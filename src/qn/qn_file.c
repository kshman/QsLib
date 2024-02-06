﻿//
// qn_file.c - 파일 처리
// 2023-12-27 by kim
//

#include "pch.h"
#ifdef _QN_UNIX_
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef _QN_FREEBSD_
#include <sys/sysctl.h>
#endif
#endif
#include "PatrickPowell_snprintf.h"

#define HFS_DEBUG_TRACE 1

// 최대 파일 할당 크기
static size_t max_file_alloc_size = 128ULL * 1024ULL * 1024ULL;

//
size_t qn_get_file_max_alloc_size(void)
{
	return max_file_alloc_size;
}

//
void qn_set_file_max_alloc_size(const size_t size)
{
	max_file_alloc_size = size == 0 ? (128ULL * 1024ULL * 1024ULL) : size;
}

//
size_t qn_filepath(const char* RESTRICT filename, char* RESTRICT dest, size_t destsize)
{
	qn_return_when_fail(filename != NULL, 0);

	const char* p = filename + strlen(filename) - 1;
	while (p >= filename && *p != '/' && *p != '\\')
		p--;

	if (p < filename)
	{
		if (dest)
			dest[0] = '\0';
		return 0;
	}

	const size_t len = (p - filename + 1);
	if (dest)
	{
		const size_t maxlen = QN_MIN(len, destsize - 1);
		memcpy(dest, filename, maxlen);
		dest[maxlen] = '\0';
		return maxlen;
	}
	return len;
}

//
#if !defined _QN_WINDOWS_ && !defined __EMSCRIPTEN__
static char* _read_sym_link(const char* path)
{
	char* buf = NULL;
	size_t len = 64;
	ssize_t rc = -1;
	while (true)
	{
		buf = qn_realloc(buf, len, char);
		rc = readlink(path, buf, len);
		if (rc == -1)
			break;
		else if (rc < (ssize_t)len)
		{
			buf[rc] = '\0';
			return buf;
		}
		len *= 2;
	}
	qn_free(buf);
	return NULL;
}
#endif

//
char* qn_basepath(void)
{
	char* path = NULL;
#ifdef _QN_WINDOWS_
	DWORD len = 0, dw = 128;
	wchar* pw = NULL;
	while (true)
	{
		pw = qn_realloc(pw, dw, wchar);
		len = GetModuleFileName(NULL, pw, dw);
		if (len < dw - 1)
			break;
		len *= 2;
	}
	if (len == 0)
	{
		qn_free(pw);
		return NULL;
	}
	DWORD i;
	for (i = len - 1; i > 0; i--)
	{
		if (pw[i] == L'\\')
			break;
	}
	qn_debug_assert(i > 0, "probably invalid base path!");
	pw[i + 1] = L'\0';

	path = qn_u16to8_dup(pw, 0);
	qn_free(pw);
#elif defined _QN_FREEBSD_ || defined _QN_LINUX_
#if defined _QN_FREEBSD_
	QN_STMT_BEGIN{
		char fullpath[FILENAME_MAX];
		size_t len = QN_COUNTOF(fullpath);
		const int sns[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
		if (sysctl(sns, QN_COUNTOF(sns), fullpath, &len, NULL, 0) != -1)
			path = qn_strdup(fullpath);
	}QN_STMT_END;
#endif
	if (path == NULL && (access("/proc", F_OK) == 0))
	{
#if defined _QN_FREEBSD_
		path = _read_sym_link("/proc/curproc/file");
#elif defined _QN_LINUX_
		path = _read_sym_link("/proc/self/exe");
		if (path == NULL)
		{
			char tmp[64];
			const int rc = qn_snprintf(tmp, QN_COUNTOF(tmp), "/proc/%llu/exe", (ullong)getpid());
			if (rc > 0 && rc < (int)QN_COUNTOF(tmp))
				path = _read_sym_link(tmp);
		}
#else
#error unknown platform
#endif
	}
	if (path != NULL)
	{
		char* p = qn_strrchr(path, '/');
		if (p != NULL)
			*(p + 1) = '\0';
		else
		{
			// 못찾겟어.
			qn_free(path);
			path = NULL;
		}
	}
#elif defined __EMSCRIPTEN__
	path = "/";
#endif
	return path;
}

// 파일 속성 얻기
QnFileAttr qn_get_file_attr(const char* path)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, path, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, 0);
	const DWORD attr = GetFileAttributes(uni);
	if (attr == INVALID_FILE_ATTRIBUTES)
		return QNFATTR_NONE;
	int ret = QNFATTR_FILE;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_DIRECTORY))
		ret |= QNFATTR_DIR;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_READONLY))
		ret |= QNFATTR_RDONLY;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_HIDDEN))
		ret |= QNFATTR_HIDDEN;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_SYSTEM))
		ret |= QNFATTR_SYSTEM;
#else
	struct stat st;
	int n = stat(path, &st);
	if (n < 0)
		return QNFATTR_NONE;
	int ret = QNFATTR_FILE;
	if (S_ISDIR(st.st_mode))
		ret |= QNFATTR_DIR;
	if (S_ISLNK(st.st_mode))
		ret |= QNFATTR_LINK;
#endif
	return ret;
}


/////////////////////////////////////////////////////////////////////////
// 스트림 연계 함수

static QnStream* _file_stream_open(QnMount* mount, const char* RESTRICT filename, const char* RESTRICT mode);

//
QnStream* qn_open_stream(QnMount* mount, const char* RESTRICT filename, const char* RESTRICT mode)
{
	qn_return_when_fail(filename != NULL, NULL);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_open(mount, filename, mode);
	return _file_stream_open(NULL, filename, mode);
}

//
QnFileAttr qn_file_attr(QnMount* mount, const char* RESTRICT filename)
{
	qn_return_when_fail(filename != NULL, false);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_exist(mount, filename);
	return qn_get_file_attr(filename);
}

// 파일 한번에 읽기
static void* _internal_file_alloc(const char* RESTRICT filename, int* size)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, filename, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, NULL);

	HANDLE fd = CreateFile(uni, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd == NULL || fd == INVALID_HANDLE_VALUE)
		return NULL;

	LARGE_INTEGER li;
	if (!GetFileSizeEx(fd, &li))
	{
		CloseHandle(fd);
		return NULL;
	}
	if (li.QuadPart <= 0 || (size_t)li.QuadPart > max_file_alloc_size)
	{
		CloseHandle(fd);
		return NULL;
	}

	byte* buf = qn_alloc((size_t)li.QuadPart, byte);
	DWORD ret;
	if (!ReadFile(fd, buf, (DWORD)li.QuadPart, &ret, NULL) || ret != li.QuadPart)
	{
		CloseHandle(fd);
		qn_free(buf);
		return NULL;
	}
	CloseHandle(fd);
	if (size != NULL)
		*size = (int)li.QuadPart;
#else
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		return NULL;
	struct stat st;
	if (fstat(fd, &st) < 0)
	{
		close(fd);
		return NULL;
	}
	if (st.st_size <= 0 || (size_t)st.st_size > max_file_alloc_size)
	{
		close(fd);
		return NULL;
	}
	byte* buf = qn_alloc((size_t)st.st_size, byte);
	const int n = read(fd, buf, (size_t)st.st_size);
	close(fd);
	if (n != st.st_size)
	{
		qn_free(buf);
		return NULL;
	}
	if (size != NULL)
		*size = (int)n;
#endif
	return buf;
}

//
void* qn_file_alloc(QnMount* mount, const char* RESTRICT filename, int* RESTRICT size)
{
	qn_return_when_fail(filename != NULL, NULL);
	if (mount == NULL)
		return _internal_file_alloc(filename, size);

	/*
	QnStream* stream = qn_cast_vtable(mount, QNMOUNT)->mount_open(mount, filename, "rb");
	if (stream == NULL)
		return NULL;
	const llong len = qn_cast_vtable(stream, QNSTREAM)->stream_size(stream);
	if (len <= 0 || (size_t)len > max_file_alloc_size)
	{
		qn_unload(stream);
		return NULL;
	}
	byte* buf = qn_alloc(len, byte);
	const int n = qn_cast_vtable(stream, QNSTREAM)->stream_read(stream, buf, 0, (int)len);
	qn_unload(stream);
	if (n != (int)len)
	{
		qn_free(buf);
		return NULL;
	}
	if (size != NULL)
		*size = n;
	return buf;
	*/
	return qn_cast_vtable(mount, QNMOUNT)->mount_read(mount, filename, size);
}

//
static void _internal_detect_file_encoding(char* RESTRICT data, int size, int* RESTRICT length, int* RESTRICT codepage)
{
	char *ps, *pd;
	if (*(uchar2*)data == 0xFEFF)
	{
		// UTF-16 LE
		if (length != NULL)
			*length = (size - 2) / 2;
		if (codepage != NULL)
			*codepage = 1200;
		ps = data + 2;
		pd = data;
		for (int i = 0; i < size - 2; i += 2)
		{
			*pd++ = ps[i];
			*pd++ = ps[i + 1];
		}
		*(uchar2*)pd = 0;
	}
	else if (*(uchar2*)data == 0xFFFE)
	{
		// UTF-16 BE
		if (length != NULL)
			*length = (size - 2) / 2;
		if (codepage != NULL)
			*codepage = 1201;
		ps = data + 2;
		pd = data;
		for (int i = 0; i < size - 2; i += 2)
		{
			*pd++ = ps[i + 1];
			*pd++ = ps[i];
		}
		*(uchar2*)pd = 0;
	}
	else if ((byte)data[0] == 0xEF && (byte)data[1] == 0xBB && (byte)data[2] == 0xBF)
	{
		// UTF-8
		if (length != NULL)
			*length = size - 3;
		if (codepage != NULL)
			*codepage = 65001;
		ps = data + 3;
		pd = data;
		for (int i = 0; i < size - 3; i++)
			*pd++ = ps[i];
		*pd = '\0';
	}
	else
	{
		// ANSI 아니면 BOM없는 UTF-8
		if (length != NULL)
			*length = size;
		if (codepage != NULL)
			*codepage = 0;
	}
}

//
char* qn_file_alloc_text(QnMount* mount, const char* RESTRICT filename, int* RESTRICT length, int* RESTRICT codepage)
{
	int size;
	char* data = qn_file_alloc(mount, filename, &size);
	qn_return_when_fail(data, NULL);
	_internal_detect_file_encoding(data, size, length, codepage);
	return data;
}


//////////////////////////////////////////////////////////////////////////
// 스트림

//
int qn_stream_read(QnStream* self, void* RESTRICT buffer, int offset, int size)
{
	qn_return_when_fail(buffer != NULL, -1);
	qn_return_when_fail(size >= 0, 0);
	return qn_cast_vtable(self, QNSTREAM)->stream_read(self, buffer, offset, size);
}

//
int qn_stream_write(QnStream* self, const void* RESTRICT buffer, int offset, int size)
{
	qn_return_when_fail(buffer != NULL, -1);
	qn_return_when_fail(size >= 0, 0);
	return qn_cast_vtable(self, QNSTREAM)->stream_write(self, buffer, offset, size);
}

//
llong qn_stream_seek(QnStream* self, const llong offset, const QnSeek org)
{
	return qn_cast_vtable(self, QNSTREAM)->stream_seek(self, offset, org);
}

//
llong qn_stream_tell(QnStream* self)
{
	return qn_cast_vtable(self, QNSTREAM)->stream_tell(self);
}

//
llong qn_stream_size(QnStream* self)
{
	return qn_cast_vtable(self, QNSTREAM)->stream_size(self);
}

//
bool qn_stream_flush(QnStream* self)
{
	return qn_cast_vtable(self, QNSTREAM)->stream_flush(self);
}

//
QnStream* qn_stream_dup(QnStream* self)
{
	return qn_cast_vtable(self, QNSTREAM)->stream_dup(self);
}

// printf용 버퍼
typedef struct STREAMPRINTFBUFFER
{
	QnStream* stream;
	char buffer[8 * 1024];
	size_t loc;
} StreamPrintfBuffer;

// 정리
static void _stream_printf_finish(PatrickPowellSprintfState* state)
{
	StreamPrintfBuffer* buffer = (StreamPrintfBuffer*)state->ptr;
	if (buffer->loc > 0)
	{
		QnStream* stream = buffer->stream;
		qn_cast_vtable(stream, QNSTREAM)->stream_write(stream, buffer->buffer, 0, (int)buffer->loc);
	}
	qn_free(buffer);
}

// 한글자씩 쓰기
static void _stream_printf_outch(PatrickPowellSprintfState* state, int ch)
{
	StreamPrintfBuffer* buffer = (StreamPrintfBuffer*)state->ptr;
	qn_debug_verify(buffer->loc <= QN_COUNTOF(buffer->buffer));
	if (buffer->loc >= QN_COUNTOF(buffer->buffer))
	{
		QnStream* stream = buffer->stream;
		qn_cast_vtable(stream, QNSTREAM)->stream_write(stream, buffer->buffer, 0, (int)QN_COUNTOF(buffer->buffer));
		buffer->loc = 0;
	}
	buffer->buffer[buffer->loc++] = (char)ch;
	state->currlen++;
}

//
int qn_stream_vprintf(QnStream* self, const char* RESTRICT fmt, va_list va)
{
	qn_return_when_fail(fmt, 0);

	PatrickPowellSprintfState state =
	{
		_stream_printf_outch,
		_stream_printf_finish,
	};
	StreamPrintfBuffer* buffer = qn_alloc_1(StreamPrintfBuffer);
	buffer->stream = self;
	buffer->loc = 0;
	state.ptr = buffer;
	dopr(&state, fmt, va);
	return (int)state.currlen;
}

//
int qn_stream_printf(QnStream* self, const char* RESTRICT fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_stream_vprintf(self, fmt, va);
	va_end(va);
	return ret;
}


//////////////////////////////////////////////////////////////////////////
// 파일 리스트

//
const char* qn_dir_read(QnDir* self)
{
	return qn_cast_vtable(self, QNDIR)->dir_read(self);
}

//
bool qn_dir_read_info(QnDir* self, QnFileInfo2* info)
{
	qn_return_when_fail(info, false);
	return qn_cast_vtable(self, QNDIR)->dir_read_info(self, info);
}

//
void qn_dir_rewind(QnDir* self)
{
	qn_cast_vtable(self, QNDIR)->dir_rewind(self);
}

//
void qn_dir_seek(QnDir* self, int pos)
{
	qn_cast_vtable(self, QNDIR)->dir_seek(self, pos);
}

//
int qn_dir_tell(QnDir* self)
{
	return qn_cast_vtable(self, QNDIR)->dir_tell(self);
}


//////////////////////////////////////////////////////////////////////////
// 마운트

//
QnStream* qn_mount_open_stream(QnMount* g, const char* RESTRICT filename, const char* RESTRICT mode)
{
	qn_return_when_fail(filename, NULL);
	return qn_cast_vtable(g, QNMOUNT)->mount_open(g, filename, mode);
}

//
void* qn_mount_read(QnMount* g, const char* RESTRICT filename, int* size)
{
	qn_return_when_fail(filename, NULL);
	return qn_cast_vtable(g, QNMOUNT)->mount_read(g, filename, size);
}

//
char* qn_mount_read_text(QnMount* g, const char* RESTRICT filename, int* RESTRICT length, int* RESTRICT codepage)
{
	qn_return_when_fail(filename, NULL);
	return qn_cast_vtable(g, QNMOUNT)->mount_read_text(g, filename, length, codepage);
}

//
QnFileAttr qn_mount_exist(QnMount* g, const char* RESTRICT path)
{
	qn_return_when_fail(path, false);
	return qn_cast_vtable(g, QNMOUNT)->mount_exist(g, path);
}

//
bool qn_mount_remove(QnMount* g, const char* RESTRICT path)
{
	qn_return_when_fail(path, false);
	qn_return_when_fail(QN_TMASK(g->flags, QNMF_WRITE), false);
	return qn_cast_vtable(g, QNMOUNT)->mount_remove(g, path);
}

//
bool qn_mount_chdir(QnMount* g, const char* RESTRICT directory)
{
	qn_return_when_fail(directory, false);
	return qn_cast_vtable(g, QNMOUNT)->mount_chdir(g, directory);
}

//
bool qn_mount_mkdir(QnMount* g, const char* RESTRICT directory)
{
	qn_return_when_fail(directory, false);
	qn_return_when_fail(QN_TMASK(g->flags, QNMF_WRITE), false);
	return qn_cast_vtable(g, QNMOUNT)->mount_mkdir(g, directory);
}

//
QnDir* qn_mount_list(QnMount* g)
{
	return qn_cast_vtable(g, QNMOUNT)->mount_list(g);
}


//////////////////////////////////////////////////////////////////////////
// 메모리 스트림

//
typedef struct MEMSTREAM
{
	QnStream			base;
	size_t				capa;
	size_t				size;
	size_t				loc;
} MemStream;

//
static int _mem_stream_read(QnGam g, void* RESTRICT buffer, const int offset, const int size)
{
	MemStream* self = qn_cast_type(g, MemStream);

	size_t n = self->size - self->loc;
	n = QN_MIN(n, (size_t)size);
	if (size == 0)
		return 0;

	byte* ptr = (byte*)buffer + offset;
	memcpy(ptr, (byte*)qn_get_gam_pointer(self) + self->loc, n);
	self->loc += n;
	return (int)n;
}

//
static int _mem_stream_write(QnGam g, const void* RESTRICT buffer, const int offset, const int size)
{
	MemStream* self = qn_cast_type(g, MemStream);

	size_t sub = self->capa - self->loc;
	if ((int)sub < size)
	{
		sub = self->loc + size;
		size_t capa = self->capa == 0 ? 16 : self->capa;
		while (capa < sub)
			capa <<= 1;
		byte* data = qn_realloc(qn_get_gam_pointer(self), capa, byte);
		qn_set_gam_desc(self, data);
		self->capa = capa;
	}

	const byte* ptr = (const byte*)buffer + offset;
	memcpy((byte*)qn_get_gam_pointer(self) + self->loc, ptr, size);
	self->loc += size;
	if (self->size < self->loc)
		self->size = self->loc;
	return size;
}

//
static llong _mem_stream_seek(QnGam g, const llong offset, const QnSeek org)
{
	MemStream* self = qn_cast_type(g, MemStream);
	llong loc;
	switch (org)
	{
		case QNSEEK_BEGIN:
			loc = offset;
			break;
		case QNSEEK_CUR:
			loc = (llong)self->loc + offset;
			break;
		case QNSEEK_END:
			loc = (llong)self->size + offset;
			break;
		default:
			return -1;
	}
	self->loc = (size_t)loc <= self->size ? (size_t)loc : self->size;
	return (llong)self->loc;
}

//
static llong _mem_stream_tell(QnGam g)
{
	MemStream* self = qn_cast_type(g, MemStream);
	return (llong)self->loc;
}

//
static llong _mem_stream_size(QnGam g)
{
	MemStream* self = qn_cast_type(g, MemStream);
	return (llong)self->size;
}

//
static bool _mem_stream_flush(QnGam g)
{
	QN_DUMMY(g);
	return true;
}

// 메모리 스트림 닫기
static void _mem_stream_dispose(QnGam g)
{
	MemStream* self = qn_cast_type(g, MemStream);
	qn_free(qn_get_gam_pointer(self));
	qn_free(self->base.name);
	qn_free(self);
}

//
static QnStream* _mem_stream_dup(QnGam g);

//
static const QN_DECL_VTABLE(QNSTREAM) _mem_stream_vt =
{
	.base.name = "MemoryStream",
	.base.dispose = _mem_stream_dispose,
	.stream_read = _mem_stream_read,
	.stream_write = _mem_stream_write,
	.stream_seek = _mem_stream_seek,
	.stream_tell = _mem_stream_tell,
	.stream_size = _mem_stream_size,
	.stream_flush = _mem_stream_flush,
	.stream_dup = _mem_stream_dup,
};

// 메모리 스트림 복제
static QnStream* _mem_stream_dup(QnGam g)
{
	MemStream* source = qn_cast_type(g, MemStream);
	MemStream* self = qn_alloc(1, MemStream);

	byte* data = qn_alloc(source->size, byte);
	memcpy(data, qn_get_gam_pointer(source), source->size);

	self->base.mount = NULL;
	self->base.name = qn_strdup(source->base.name);
	self->base.flags = source->base.flags;
	self->capa = source->size;
	self->size = source->size;
	self->loc = 0;
	qn_set_gam_desc(self, data);

	return qn_gam_init(self, _mem_stream_vt);
}

// 메모리 스트림 만들기
QnStream* qn_create_mem_stream(const char* RESTRICT name, size_t initial_capacity)
{
	MemStream* self = qn_alloc(1, MemStream);

	byte* data;
	if (initial_capacity == 0)
		data = NULL;
	else
	{
		data = qn_alloc(initial_capacity, byte);
		self->capa = initial_capacity;
	}

	self->base.mount = NULL;
	self->base.name = qn_strdup(name);
	self->base.flags = QNFF_READ | QNFF_WRITE | QNFF_SEEK | QNFFT_MEM;
	qn_set_gam_desc(self, data);

	return qn_gam_init(self, _mem_stream_vt);
}

// 메모리 스트림 외부 데이터로 만들기
QnStream* qn_create_mem_stream_stored(const char* RESTRICT name, void* RESTRICT data, size_t size)
{
	MemStream* self = qn_alloc(1, MemStream);

	self->base.mount = NULL;
	self->base.name = qn_strdup(name);
	self->base.flags = QNFF_READ | QNFF_WRITE | QNFF_SEEK | QNFFT_MEM;
	self->capa = size;
	self->size = size;
	qn_set_gam_desc(self, data);

	return qn_gam_init(self, _mem_stream_vt);
}

//
void* qn_mem_stream_get_data(QnStream* self)
{
	return QN_TMASK(self->flags, QNFFT_MEM) ? qn_get_gam_pointer(self) : NULL;
}


//////////////////////////////////////////////////////////////////////////
// 파일 스트림

//
typedef struct FILESTREAM
{
	QnStream			base;
	QnFileAccess		acs;
} FileStream;

// 분석
static void _file_stream_access_parse(const char* mode, QnFileAccess* acs, QnFileFlag* flags)
{
#ifdef _QN_WINDOWS_
	acs->mode = 0;

	if (mode)
	{
		acs->access = 0;
		acs->share = FILE_SHARE_READ;
		acs->attr = FILE_ATTRIBUTE_NORMAL;

		for (char ch; *mode; ++mode)
		{
			switch (*mode)
			{
				case 'a':
					acs->mode = OPEN_ALWAYS;
					acs->access = GENERIC_WRITE;
					*flags = QNFF_WRITE | QNFF_SEEK | QNFF_APPEND;
					break;
				case 'r':
					acs->mode = OPEN_EXISTING;
					acs->access = GENERIC_READ;
					*flags = QNFF_READ | QNFF_SEEK;
					break;
				case 'w':
					acs->mode = CREATE_ALWAYS;
					acs->access = GENERIC_WRITE;
					*flags = QNFF_WRITE | QNFF_SEEK;
					break;
				case '+':
					acs->access = GENERIC_WRITE | GENERIC_READ;
					*flags = QNFF_READ | QNFF_WRITE | QNFF_SEEK;
					break;
				case '@':
					ch = *(mode + 1);
					if (ch != '\0')
					{
						if (ch == 'R')
						{
							acs->share = FILE_SHARE_READ;
							++mode;
						}
						else if (ch == 'W')
						{
							acs->share = FILE_SHARE_WRITE;
							++mode;
						}
						else if (ch == '+')
						{
							acs->share = FILE_SHARE_READ | FILE_SHARE_WRITE;
							++mode;
						}
					} break;
				case '!':
					if (acs->mode == CREATE_ALWAYS)
						acs->attr |= FILE_FLAG_DELETE_ON_CLOSE;
					break;
				case 't':
					*flags |= QNFF_TEXT;
					break;
				case 'b':
					*flags |= QNFF_BINARY;
					break;
				default:
					break;
			}
		}
	}

	if (acs->mode == 0)
	{
		*flags = QNFF_READ | QNFF_SEEK;
		acs->mode = OPEN_EXISTING;
		acs->access = GENERIC_READ;
		acs->attr = FILE_ATTRIBUTE_NORMAL;
		acs->share = FILE_SHARE_READ;
	}
#else
	char ch;

	acs->mode = 0;
	acs->access = 0;

	if (mode)
	{
		for (; *mode; ++mode)
		{
			switch (*mode)
			{
				case 'a':
					acs->mode = O_WRONLY | O_APPEND | O_CREAT;
					*flags = QNFF_WRITE | QNFF_SEEK | QNFF_APPEND;
					break;
				case 'r':
					acs->mode = O_RDONLY;
					*flags = QNFF_READ | QNFF_SEEK;
					break;
				case 'w':
					acs->mode = O_WRONLY | O_TRUNC | O_CREAT;
					*flags = QNFF_WRITE | QNFF_SEEK;
					break;
				case '+':
					acs->mode |= O_RDWR;
					*flags = QNFF_READ | QNFF_WRITE | QNFF_SEEK;
					break;
				case '@':
					ch = *(mode + 1);
					if (ch != '\0')
					{
						if (ch == 'R')
						{
							acs->access |= S_IRUSR;
							mode++;
						}
						else if (ch == 'W')
						{
							acs->access |= S_IWUSR;
							mode++;
						}
						else if (ch == 'X')
						{
							acs->access |= S_IXUSR;
							mode++;
						}
						else if (ch == '+')
						{
							acs->access |= S_IRUSR | S_IWUSR;
							mode++;
						}
						else if (isdigit(ch))
						{
							const char* p = mode + 2;
							while (isdigit(*p))
								p++;
							if ((p - (mode + 1)) < 63)
							{
								char sz[64];
								qn_strncpy(sz, (mode + 1), (size_t)(p - (mode + 1)));
								acs->access = qn_strtoi(sz, 8);
							}
							mode = p;
						}
					} break;
				case '!':
					break;
				case 't':
					*flags |= QNFF_TEXT;
					break;
				case 'b':
					*flags |= QNFF_BINARY;
					break;
				default:
					break;
			}
		}
	}

	if (acs->mode == 0)
	{
		acs->mode = O_RDONLY;
		*flags = QNFF_READ | QNFF_SEEK;
	}

	if (acs->access == 0 && (acs->mode & O_CREAT) != 0)
		acs->access = (S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH);
#endif
}

// 실제 파일 경로 얻기
static char* _file_stream_get_real_path(char* RESTRICT dest, const QnMount* mount, const char* RESTRICT filename)
{
#ifdef _QN_WINDOWS_
	if (filename[0] != '\0' && filename[1] == ':')
	{
		qn_debug_mesg(true, "FileStream", "cannot use drive letter");
		return NULL;
	}
#endif
	if (filename[0] == '/' || filename[0] == '\\')
	{
#ifdef _DEBUG
		size_t len = strlen(filename);
		if (mount->name_len + len >= QN_MAX_PATH)
		{
			qn_mesg(true, "FileStream", "filename too long");
			return NULL;
		}
#endif
		qn_strcpy(dest, mount->name);
		qn_strncpy(dest + mount->name_len, filename + 1, QN_MAX_PATH - mount->name_len - 1);
	}
	else
	{
#ifdef _DEBUG
		size_t len = strlen(filename);
		if (mount->path.LENGTH + len >= QN_MAX_PATH)
		{
			qn_mesg(true, "FileStream", "filename too long");
			return NULL;
		}
#endif
		qn_strcpy(dest, mount->path.DATA);
		qn_strncpy(dest + mount->path.LENGTH, filename, QN_MAX_PATH - mount->path.LENGTH - 1);
	}
	return dest;
}

//
static int _file_stream_read(QnGam g, void* RESTRICT buffer, const int offset, const int size)
{
	FileStream* self = qn_cast_type(g, FileStream);
	byte* ptr = (byte*)buffer + offset;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return ReadFile(qn_get_gam_handle(self), ptr, size, &ret, NULL) ? (int)ret : -1;
#else
	return (int)read(qn_get_gam_desc_int(self), ptr, (size_t)size);
#endif
}

//
static int _file_stream_write(QnGam g, const void* RESTRICT buffer, const int offset, const int size)
{
	FileStream* self = qn_cast_type(g, FileStream);
	const byte* ptr = (const byte*)buffer + offset;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return WriteFile(qn_get_gam_handle(self), ptr, size, &ret, NULL) ? (int)ret : -1;
#else
	return (int)write(qn_get_gam_desc_int(self), ptr, (size_t)size);
#endif
}

//
static llong _file_stream_seek(QnGam g, const llong offset, const QnSeek org)
{
	FileStream* self = qn_cast_type(g, FileStream);
#ifdef _QN_WINDOWS_
	static_assert(FILE_BEGIN == QNSEEK_BEGIN && FILE_CURRENT == QNSEEK_CUR && FILE_END == QNSEEK_END, "QnSeek not equal to OS seek");
	const LARGE_INTEGER* pli = (const LARGE_INTEGER*)&offset;
	LARGE_INTEGER li;
	const BOOL b = SetFilePointerEx(qn_get_gam_handle(self), *pli, &li, org);
	return b ? li.QuadPart : -1;
#else
	static_assert(SEEK_SET == QNSEEK_BEGIN && SEEK_CUR == QNSEEK_CUR && SEEK_END == QNSEEK_END, "QnSeek not equal to OS seek");
	return lseek(qn_get_gam_desc_int(self), (off_t)offset, (int)org);
#endif
}

//
static llong _file_stream_tell(QnGam g)
{
	FileStream* self = qn_cast_type(g, FileStream);
#ifdef _QN_WINDOWS_
	const LARGE_INTEGER cli = { .QuadPart = 0 };
	LARGE_INTEGER li;
	return SetFilePointerEx(qn_get_gam_handle(self), cli, &li, FILE_CURRENT) ? li.QuadPart : -1;
#else
	return lseek(qn_get_gam_desc_int(self), 0, SEEK_CUR);
#endif
}

//
static llong _file_stream_size(QnGam g)
{
	FileStream* self = qn_cast_type(g, FileStream);
#ifdef _QN_WINDOWS_
	LARGE_INTEGER li;
	return GetFileSizeEx(qn_get_gam_handle(self), &li) ? li.QuadPart : -1;
#else
	struct stat st;
	return fstat(qn_get_gam_desc_int(self), &st) < 0 ? -1 : st.st_size;
#endif
}

//
static bool _file_stream_flush(QnGam g)
{
	FileStream* self = qn_cast_type(g, FileStream);
#ifdef _QN_WINDOWS_
	return FlushFileBuffers(qn_get_gam_handle(self)) != 0;
#else
	return fsync(qn_get_gam_desc_int(self)) == 0;
#endif
}

// 파일 스트림 닫기
static void _file_stream_dispose(QnGam g)
{
	FileStream* self = qn_cast_type(g, FileStream);
#ifdef _QN_WINDOWS_
	HANDLE fd = qn_get_gam_handle(self);
	if (fd != NULL && fd != INVALID_HANDLE_VALUE)
		CloseHandle(fd);
#else
	int fd = qn_get_gam_desc_int(self);
	if (fd >= 0)
		close(fd);
#endif
	qn_unload(self->base.mount);
	qn_free(self->base.name);
	qn_free(self);
}

//
static QnStream* _file_stream_dup(QnGam g);

//
static const QN_DECL_VTABLE(QNSTREAM) _file_stream_vt =
{
	.base.name = "FileStream",
	.base.dispose = _file_stream_dispose,
	.stream_read = _file_stream_read,
	.stream_write = _file_stream_write,
	.stream_seek = _file_stream_seek,
	.stream_tell = _file_stream_tell,
	.stream_size = _file_stream_size,
	.stream_flush = _file_stream_flush,
	.stream_dup = _file_stream_dup,
};

// 파일 스트림 복제
static QnStream* _file_stream_dup(QnGam g)
{
	FileStream* source = qn_cast_type(g, FileStream);
	FileStream* self = qn_alloc(1, FileStream);

#ifdef _QN_WINDOWS_
	const HANDLE process = GetCurrentProcess();
	HANDLE fd = qn_get_gam_handle(source);
	HANDLE dup;
	if (!DuplicateHandle(process, fd, process, &dup, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		qn_free(self);
		return NULL;
	}
#else
	int fd = qn_get_gam_desc_int(source);
	int dup = fcntl(fd, F_DUPFD_CLOEXEC, 0);
	if (dup < 0)
	{
		qn_free(self);
		return NULL;
	}
#endif

	self->base.mount = qn_load(source->base.mount);
	self->base.name = qn_strdup(source->base.name);
	self->base.flags = source->base.flags;
	qn_set_gam_desc(self, dup);

	return qn_gam_init(self, _file_stream_vt);
}

// 파일 스트림 열기
static QnStream* _file_stream_open(QnMount* mount, const char* RESTRICT filename, const char* RESTRICT mode)
{
	char path[QN_MAX_PATH];
	if (mount != NULL)
	{
		filename = _file_stream_get_real_path(path, mount, filename);
		qn_return_when_fail(filename, NULL);
	}

	QnFileAccess acs;
	QnFileFlag flags;
	_file_stream_access_parse(mode, &acs, &flags);
#ifdef _QN_EMSCRIPTEN_
	if (QN_TMASK(flags, QNFF_WRITE | QNFF_APPEND))
	{
		// 쓰기 모드는 지원하지 않는다
		return NULL;
	}
#endif

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, filename, 0);
	if (uni_len >= QN_MAX_PATH - QN_MAX_PATH_BIAS)
	{
		// 파일 이름이 너무 길다
		return NULL;
	}
	HANDLE fd = CreateFile(uni, acs.access, acs.share, NULL, acs.mode, acs.attr, NULL);
	if (fd == NULL || fd == INVALID_HANDLE_VALUE)
		return NULL;
#else
	int fd = acs.access == 0 ? open(filename, acs.mode) : open(filename, acs.mode, acs.access);
	if (fd < 0)
		return NULL;
#endif

	FileStream* self = qn_alloc(1, FileStream);
	self->base.mount = qn_load(mount);
	self->base.name = qn_strdup(filename);
	self->base.flags = flags | QNFFT_FILE;
	self->acs = acs;
	qn_set_gam_desc(self, fd);

	return qn_gam_init(self, _file_stream_vt);
}


//////////////////////////////////////////////////////////////////////////
// 디스크 파일 리스트

typedef struct DISKDIR
{
	QnDir				base;
#ifdef _QN_WINDOWS_
	wchar*				wname;
	WIN32_FIND_DATA		data;
	int					stat;
#endif
	char				path[QN_MAX_PATH];
} DiskDir;

//
static const char* _disk_list_read(QnGam g)
{
	DiskDir* self = qn_cast_type(g, DiskDir);
#ifdef _QN_WINDOWS_
	for (;;)
	{
		if (self->stat < 0)
			return NULL;
		else if (self->stat == 0)
		{
			// 처음
			HANDLE fd = FindFirstFileEx(self->wname, FindExInfoStandard, &self->data, FindExSearchNameMatch, NULL, 0);
			qn_set_gam_desc(self, fd);
			self->stat = fd == INVALID_HANDLE_VALUE ? -1 : 1;
		}
		else
		{
			// 계속
			if (FindNextFile(qn_get_gam_handle(self), &self->data))
				self->stat++;
			else
			{
				FindClose(qn_get_gam_handle(self));
				qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
				self->stat = -1;
				return NULL;
			}
		}

		if (self->data.cFileName[0] == '.' && (self->data.cFileName[1] == '\0' || (self->data.cFileName[1] == '.' && self->data.cFileName[2] == '\0')))
			continue;
		qn_u16to8(self->path, QN_MAX_PATH, self->data.cFileName, 0);
		return self->path;
	}
#else
	DIR* dir = qn_get_gam_desc(self);
	struct dirent* ent = readdir(dir);
	while (ent != NULL)
	{
		if (ent->d_name[0] != '.' || (ent->d_name[1] != '\0' && (ent->d_name[1] != '.' || ent->d_name[2] != '\0')))
			break;
		ent = readdir(dir);
	}
	return ent == NULL ? NULL : ent->d_name;
#endif
}

#ifdef _QN_WINDOWS_
//
extern QnTimeStamp qn_system_time_to_timestamp(const SYSTEMTIME* pst);

//
static QnTimeStamp _filetime_to_timestamp(const FILETIME* ft)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(ft, &st);
	return qn_system_time_to_timestamp(&st);
}
#endif

#ifdef _QN_UNIX_
//
extern QnTimeStamp qn_tm_to_timestamp(struct tm* ptm);

//
static QnTimeStamp _stat_to_timestamp(time_t tt)
{
	struct tm tm;
	localtime_r(&tt, &tm);
	return qn_tm_to_timestamp(&tm);
}
#endif

//
static bool _disk_list_read_info(QnGam g, QnFileInfo2* info)
{
	DiskDir* self = qn_cast_type(g, DiskDir);
#ifdef _QN_WINDOWS_
	for (;;)
	{
		if (self->stat < 0)
			return false;
		if (self->stat == 0)
		{
			// 처음
			HANDLE fd = FindFirstFileEx(self->wname, FindExInfoStandard, &self->data, FindExSearchNameMatch, NULL, 0);
			qn_set_gam_desc(self, fd);
			if (fd == INVALID_HANDLE_VALUE)
			{
				self->stat = -1;
				return false;
			}
			self->stat = 1;
		}
		else
		{
			// 계속
			if (FindNextFile(qn_get_gam_handle(self), &self->data))
				self->stat++;
			else
			{
				FindClose(qn_get_gam_handle(self));
				qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
				self->stat = -1;
				return false;
			}
		}

		if (self->data.cFileName[0] == '.' && (self->data.cFileName[1] == '\0' || (self->data.cFileName[1] == '.' && self->data.cFileName[2] == '\0')))
			continue;

		info->type = QN_TMASK(self->data.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ? 0 : 1;
		info->len = (ushort)qn_u16to8(self->path, QN_MAX_PATH, self->data.cFileName, 0);
		info->extra = 0;
		info->size = (uint)(self->data.nFileSizeLow | ((llong)self->data.nFileSizeHigh << 32));
		info->cmpr = 0;
		info->stc = _filetime_to_timestamp(&self->data.ftCreationTime);
		info->stw = _filetime_to_timestamp(&self->data.ftLastWriteTime);
		info->name = self->path;
		return true;
	}
#else
	DIR* dir = qn_get_gam_desc(self);
	struct dirent* ent = readdir(dir);
	while (ent != NULL)
	{
		if (ent->d_name[0] != '.' || (ent->d_name[1] != '\0' && (ent->d_name[1] != '.' || ent->d_name[2] != '\0')))
			break;
		ent = readdir(dir);
	}
	if (ent == NULL)
		return false;

	qn_strconcat(self->path, self->base.name, ent->d_name, NULL);
	struct stat st;
	if (stat(self->path, &st) < 0)
		return false;

	info->type = S_ISDIR(st.st_mode) ? 0 : 1;
	info->len = (ushort)strlen(ent->d_name);
	info->extra = 0;
	info->size = (uint)st.st_size;
	info->cmpr = 0;
	info->stc = _stat_to_timestamp(st.st_ctime);
	info->stw = _stat_to_timestamp(st.st_mtime);
	info->name = ent->d_name;
	return true;
#endif
}

//
static void _disk_list_rewind(QnGam g)
{
	DiskDir* self = qn_cast_type(g, DiskDir);
#ifdef _QN_WINDOWS_
	const HANDLE fd = qn_get_gam_handle(self);
	if (fd != NULL && fd != INVALID_HANDLE_VALUE)
		FindClose(fd);
	qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
	self->stat = 0;
#else
	DIR* dir = qn_get_gam_desc(self);
	if (dir != NULL)
		rewinddir(dir);
#endif
}

//
static void _disk_list_seek(QnGam g, int position)
{
	DiskDir* self = qn_cast_type(g, DiskDir);
#ifdef _QN_WINDOWS_
	if (position < 0)
	{
		const HANDLE fd = qn_get_gam_handle(self);
		if (fd != NULL && fd != INVALID_HANDLE_VALUE)
			FindClose(fd);
		qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
		self->stat = -1;
	}
	else
	{
		_disk_list_rewind(self);
		while ((self->stat < position) && _disk_list_read(self))
		{
		}
	}
#else
#ifdef _QN_ANDROID_
	DIR* dir = qn_get_gam_desc(self);
	int cnt = 0;
	_disk_list_rewind(self);
	while (cnt < position && readdir(dir) != NULL)
		cnt++;
#else
	DIR* dir = qn_get_gam_desc(self);
	seekdir(dir, position);
#endif
#endif
}

//
static int _disk_list_tell(QnGam g)
{
	DiskDir* self = qn_cast_type(g, DiskDir);
#ifdef _QN_WINDOWS_
	return self->stat;
#else
#ifdef _QN_ANDROID_
	return -1;
#else
	DIR* dir = qn_get_gam_desc(self);
	return (int)telldir(dir);
#endif
#endif
}

//
static void _disk_list_dispose(QnGam g)
{
	DiskDir* self = qn_cast_type(g, DiskDir);
#ifdef _QN_WINDOWS_
	HANDLE fd = qn_get_gam_handle(self);
	if (fd != NULL && fd != INVALID_HANDLE_VALUE)
		FindClose(fd);
	qn_free(self->wname);
#else
	DIR* dir = qn_get_gam_desc(self);
	if (dir != NULL)
		closedir(dir);
#endif
	qn_unload(self->base.mount);
	qn_free(self->base.name);
	qn_free(self);
}

//
static QnDir* _disk_list_open(_In_ QnMount* mount)
{
#ifdef _QN_WINDOWS_
	size_t len = qn_u8to16(NULL, 0, mount->path.DATA, mount->path.LENGTH) + 1/*슬래시*/ + 1/*서픽스*/ + 1/*널*/;
	wchar* pw = qn_alloc(len, wchar);
	qn_u8to16(pw, len, mount->path.DATA, mount->path.LENGTH);
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0, };
	if (GetFileAttributesEx(pw, GetFileExInfoStandard, &fad) == FALSE ||
		QN_TMASK(fad.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) == false)
	{
		qn_free(pw);
		return NULL;
	}

	pw[len - 2] = L'\0';
	pw[len - 3] = L'*';

	DiskDir* self = qn_alloc_zero_1(DiskDir);
	self->base.mount = qn_load(mount);
	self->base.name = qn_strdup(mount->path.DATA);	// VS는 여기 널이라고 우기지만 mount는 널이 아니므로 무시
	self->wname = pw;
	qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
#else
	DIR* dir = opendir(mount->path.DATA);
	qn_return_when_fail(dir != NULL, NULL);

	QnDiskdir* self = qn_alloc_zero_1(DiskDir);
	self->base.mount = qn_load(mount);
	self->base.name = qn_strdup(mount->path.DATA);
	qn_set_gam_desc(self, dir);
#endif

	static const QN_DECL_VTABLE(QNDIR) _disk_list_vt =
	{
		.base.name = "DiskList",
		.base.dispose = _disk_list_dispose,
		.dir_read = _disk_list_read,
		.dir_read_info = _disk_list_read_info,
		.dir_rewind = _disk_list_rewind,
		.dir_seek = _disk_list_seek,
		.dir_tell = _disk_list_tell,
	};
	return qn_gam_init(self, _disk_list_vt);
}


//////////////////////////////////////////////////////////////////////////
// 디스크 파일 시스템 마운트

//
static QnStream* _disk_fs_open(QnGam g, const char* RESTRICT filename, const char* RESTRICT mode)
{
	return _file_stream_open(qn_cast_type(g, QnMount), filename, mode);
}

//
static void* _disk_fs_read(QnGam g, const char* filename, int* size)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, filename), NULL);
	char* data = _internal_file_alloc(real, size);
	return data;
}

//
static char* _disk_fs_read_text(QnGam g, const char* filename, int* length, int* codepage)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, filename), NULL);
	int size;
	char* data = _internal_file_alloc(real, &size);
	_internal_detect_file_encoding(data, size, length, codepage);
	return data;
}

//
static QnFileAttr _disk_fs_exist(QnGam g, const char* path)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, path), 0);
	return qn_get_file_attr(real);
}

//
static bool _disk_fs_remove(QnGam g, const char* path)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, path), false);

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, real, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, false);

	bool ret = false;
	const DWORD attr = GetFileAttributes(uni);
	if (attr != INVALID_FILE_ATTRIBUTES)
	{
		if (QN_TMASK(attr, FILE_ATTRIBUTE_DIRECTORY))
		{
			if (RemoveDirectory(uni) != 0)
				ret = true;
		}
		else
		{
			if (DeleteFile(uni) != 0)
				ret = true;
		}
	}
#else
	bool ret = false;
	struct stat st;
	if (stat(real, &st) == 0)
	{
		if (S_ISDIR(st.st_mode))
		{
			if (rmdir(real) == 0)
				ret = true;
		}
		else
		{
			if (unlink(real) == 0)
				ret = true;
		}
	}
#endif
	return ret;
}

//
static bool _disk_fs_ch_dir(QnGam g, const char* directory)
{
	QnMount* self = qn_cast_type(g, QnMount);

	if (directory == NULL || directory[0] == '/' || directory[0] == '\\' && directory[1] == '\0')
	{
		qn_bstr_set_len(&self->path, self->name, self->name_len);
		return true;
	}

	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, directory), false);

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, real, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, false);

	wchar abspath[QN_MAX_PATH];
	if (_wfullpath(abspath, uni, QN_MAX_PATH - QN_MAX_PATH_BIAS) == NULL)
		return false;

	DWORD attr = GetFileAttributes(abspath);
	qn_return_when_fail(attr != INVALID_FILE_ATTRIBUTES, false);
	qn_return_when_fail(QN_TMASK(attr, FILE_ATTRIBUTE_DIRECTORY), false);

	size_t len = wcslen(abspath);
	if (abspath[len - 1] != L'\\' && abspath[len - 1] != L'/')
	{
		abspath[len] = L'\\';
		abspath[len + 1] = L'\0';
		len++;
	}

	qn_u16to8(real, QN_MAX_PATH, abspath, 0);
	qn_bstr_set(&self->path, real);
#else
	char abspath[QN_MAX_PATH];
	qn_return_when_fail(realpath(real, abspath) != NULL, false);

	struct stat st;
	qn_return_when_fail(stat(abspath, &st) == 0, false);
	qn_return_when_fail(S_ISDIR(st.st_mode), false);

	size_t len = strlen(abspath);
	qn_return_when_fail(len < QN_MAX_PATH - QN_MAX_PATH_BIAS, false);

	if (abspath[len - 1] != '/' && abspath[len - 1] != '\\')
	{
		abspath[len] = '/';
		abspath[len + 1] = '\0';
		len++;
	}

	qn_bstr_set_len(&self->path, abspath, len);
#endif

	return true;
}

//
static bool _disk_fs_mk_dir(QnGam g, const char* directory)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, directory), false);

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, real, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, false);

	wchar abspath[QN_MAX_PATH];
	if (_wfullpath(abspath, uni, QN_MAX_PATH - QN_MAX_PATH_BIAS) == NULL)
		return false;

	DWORD attr = GetFileAttributes(abspath);
	qn_return_when_fail(attr == INVALID_FILE_ATTRIBUTES, false);

	return CreateDirectory(abspath, NULL) != 0;
#else
	struct stat st;
	qn_return_when_fail(stat(real, &st) != 0, false);

	return mkdir(real, 0755) == 0;
#endif
}

//
static QnDir* _disk_fs_list(QnGam g)
{
	return _disk_list_open(qn_cast_type(g, QnMount));
}

//
static void _disk_fs_dispose(QnGam g)
{
	QnMount* self = qn_cast_type(g, QnMount);
	qn_free(self->name);
	qn_free(self);
}

// 디스크 파일 시스템 마운트 만들기, path에 대한 오류처리는 하고 왔을 것이다
static QnMount* _create_diskfs(char* path)
{
	QnMount* self = qn_alloc(1, QnMount);

	size_t len = strlen(path);
	if (path[len - 1] == '/' || path[len - 1] == '\\')
	{
		self->name = path;
		self->name_len = len;
	}
	else
	{
#ifdef _QN_WINDOWS_
		self->name = qn_strdupcat(path, "\\", NULL);
#else
		self->name = qn_strdupcat(path, "/", NULL);
#endif
		self->name_len = len + 1;
		qn_free(path);
	}

	qn_bstr_set_len(&self->path, self->name, self->name_len);
	self->flags = QNMFT_DISKFS;

	//
	static const QN_DECL_VTABLE(QNMOUNT) _disk_fs_vt =
	{
		.base.name = "DiskFS",
		.base.dispose = _disk_fs_dispose,
		.mount_open = _disk_fs_open,
		.mount_read = _disk_fs_read,
		.mount_read_text = _disk_fs_read_text,
		.mount_exist = _disk_fs_exist,
		.mount_remove = _disk_fs_remove,
		.mount_chdir = _disk_fs_ch_dir,
		.mount_mkdir = _disk_fs_mk_dir,
		.mount_list = _disk_fs_list,
	};
	return qn_gam_init(self, _disk_fs_vt);
}


//////////////////////////////////////////////////////////////////////////
// HFS

#define HFS_HEADER		QN_FOURCC('H', 'F', 'S', '\0')
#define HFS_VERSION		200

#define HFSAT_ROOT_STC	QN_OFFSETOF(HfsHeader, stc)
#define HFSAT_ROOT_STW	QN_OFFSETOF(HfsHeader, stw)
#define HFSAT_ROOT_REV 	QN_OFFSETOF(HfsHeader, revision)
#define HFSAT_ROOT_DESC	QN_OFFSETOF(HfsHeader, desc)
#define HFSAT_TYPE		QN_OFFSETOF(HfsSource, type)
#define HFSAT_ATTR		QN_OFFSETOF(HfsSource, attr)
#define HFSAT_SIZE		QN_OFFSETOF(HfsSource, size)
#define HFSAT_CMPR		QN_OFFSETOF(HfsSource, cmpr)
#define HFSAT_SOURCE	QN_OFFSETOF(HfsSource, source)
#define HFSAT_EXT		QN_OFFSETOF(HfsFile, ext)
#define HFSAT_LEN		QN_OFFSETOF(HfsFile, len)
#define HFSAT_HASH		QN_OFFSETOF(HfsFile, hash)
#define HFSAT_META		QN_OFFSETOF(HfsFile, meta)
#define HFSAT_NEXT		QN_OFFSETOF(HfsFile, next)
#define HFSAT_STZ		QN_OFFSETOF(HfsFile, stc)
#define HFSAT_ROOT		sizeof(HfsHeader)

#define HFS_MAX_NAME	260

typedef struct HFSHEADER
{
	uint				header;		// 헤더
	ushort				version;	// 버전
	ushort				notuse;		// 사용하지 않음
	QnDateTime			stc;		// 만든 타임스탬프
	QnDateTime			stw;		// 수정 타임스탬프
	uint				revision;	// 리비전
	char				desc[64];	// 설명
} HfsHeader;

typedef struct HFSSOURCE
{
	ushort				magic;		// 매직 (사용하지 않음)
	byte				type;		// 타입
	byte 				attr;		// 속성
	uint				size;		// 원래 크기
	uint				cmpr;		// 압축된 크기
	uint				seek;		// 파일 위치
} HfsSource;

typedef struct HFSFILE
{
	HfsSource			source;
	QnDateTime			stc;		// 만든 타임스탬프
	ushort				ext;		// ??
	ushort				len;		// 파일 이름 길이
	uint				hash;		// 파일 이름 해시
	uint				meta;		// 메타(디렉토리) 위치
	uint				next;		// 다음 파일 위치
} HfsFile;

typedef struct HFSINFO
{
	HfsFile				file;
	char				name[HFS_MAX_NAME];
} HfsInfo;
QN_DECL_ARR(HfsArrInfo, HfsInfo);

typedef struct HFS
{
	QnMount				base;
	HfsHeader			header;
	HfsArrInfo			infos;
	uint				touch;
} Hfs;

// 경로 분리
static void _hfs_split_path(const char* path, QnPathStr* dir, QnPathStr* file)
{
	qn_divpath(path, dir->DATA, file->DATA);
	qn_bstr_intern(dir);
	qn_bstr_intern(file);
}

// 파일 헤더
static bool _hfs_write_file_header(QnStream* st, HfsFile* info, const char* name, size_t name_len)
{
	info->len = (ushort)name_len;
	info->hash = qn_strshash(name);
	if (qn_stream_write(st, info, 0, sizeof(HfsFile)) != sizeof(HfsFile) ||
		qn_stream_write(st, name, 0, (int)name_len) != (int)name_len)
		return false;
	return true;
}

// 디렉토리
static ushort _hfs_write_directory(QnStream* st, const char* name, size_t name_len, uint next, uint meta, QnTimeStamp stc)
{
	if (stc == 0)
		stc = qn_now();
	HfsFile info =
	{
		.source.magic = 0,
		.source.type = QNFTYPE_SYSTEM,
		.source.attr = QNFATTR_DIR,
		.source.size = (uint)qm_rand(NULL),
		.source.cmpr = (uint)qm_rand(NULL),
		.source.seek = (uint)qm_rand(NULL),
		.ext = 0,
		.meta = meta,
		.next = next,
		.stc.stamp = stc,
	};
	return _hfs_write_file_header(st, &info, name, name_len);
}

// 디렉토리 이름 만들기
static void _hfs_make_directory_name(QnPathStr* dir, const char* name)
{
	if (name[0] == '.')
	{
		if (name[1] == '\0')
			return;
		if (name[1] == '.' && name[2] == '\0')
		{
			if (dir->DATA[0] == '/' && dir->DATA[1] == '\0')
				return;

			char* p = dir->DATA + dir->LENGTH - 1;
			char* s;
			for (s = p - 1; s != dir->DATA; s--)
				if (*s == '/')
					break;
			s[1] = '\0';
			dir->LENGTH = dir->LENGTH - (int)(p - s);
			return;
		}
	}
	qn_bstr_append(dir, name);
	qn_bstr_append_char(dir, '/');
}

// 디렉토리 찾기
static bool _hfs_find_directory(HfsInfo* info, const char* name, uint hash, QnStream* st)
{
	ushort len = (ushort)strlen(name);
	for (;;)
	{
		if (qn_stream_read(st, info, 0, sizeof(HfsFile)) != sizeof(HfsFile))
			break;
		if (info->file.source.attr == QNFATTR_DIR && info->file.len == len && info->file.hash == hash)
		{
			if (qn_stream_read(st, info->name, 0, len) == len && qn_strncmp(info->name, name, len) == 0)
				return true;
		}
		if (info->file.next == 0)
			break;
		if (qn_stream_seek(st, info->file.next, QNSEEK_BEGIN) != info->file.next)
			break;
	}
	return false;
}

//.mount_open = _disk_fs_open,
//.mount_read = _disk_fs_read,
//.mount_read_text = _disk_fs_read_text,
//.mount_exist = _disk_fs_exist,

// 디렉토리 변경
static bool _hfs_chdir(QnGam g, const char* directory)
{
	Hfs* self = qn_cast_type(g, Hfs);
	QnStream* stream = qn_get_gam_desc(self, QnStream*);
	if (qn_stricmp(directory, self->base.path.DATA) == 0)
		return true;

	llong pos = qn_stream_tell(stream);
	if (qn_pctnr_is_have(&self->infos))
	{
		HfsInfo* info = &qn_arr_nth(&self->infos, 0);
		qn_stream_seek(stream, info->file.source.seek, QNSEEK_BEGIN);
	}

	QnPathStr tmp;
	qn_bstr_set(&tmp, directory);
	if (qn_bstr_nth(&tmp, 0) == '/')
	{
		qn_stream_seek(stream, HFSAT_ROOT, QNSEEK_BEGIN);
		qn_bstr_set_char(&self->base.path, '/');
	}

	HfsInfo info;
	char* stk = NULL;
	for (char* tok = qn_strtok(tmp.DATA, "\\/\x0\n\r\t", &stk); tok; tok = qn_strtok(NULL, "\\/\x0\n\r\t", &stk))
	{
		uint hash = qn_strshash(tok);
		if (_hfs_find_directory(&info, tok, hash, stream) == false)
		{
			qn_stream_seek(stream, pos, QNSEEK_BEGIN);
			return false;
		}
		qn_stream_seek(stream, info.file.meta, QNSEEK_BEGIN);
		_hfs_make_directory_name(&self->base.path, tok);
	}
#ifdef HFS_DEBUG_TRACE
	qn_outputf("\tHFS: current directory is %s", self->base.path.DATA);
	qn_outputs("\t=type=|=attr=|=filename=====================================");
	int fc = 0;
#endif

	qn_arr_clear(&self->infos);
	pos = qn_stream_tell(stream);
	for (uint srt = (uint)pos; srt; srt = info.file.next)
	{
		if (qn_stream_read(stream, &info, 0, sizeof(HfsFile)) != sizeof(HfsFile) ||
			qn_stream_read(stream, info.name, 0, info.file.len) != info.file.len)
		{
			qn_stream_seek(stream, pos, QNSEEK_BEGIN);
			return false;
		}
		info.name[info.file.len] = '\0';

		qn_stream_seek(stream, info.file.next, QNSEEK_BEGIN);
		info.file.source.seek = srt;
		qn_arr_add(HfsArrInfo, &self->infos, info);

#ifdef HFS_DEBUG_TRACE
		qn_outputf("\t %04X | %04X | (%d/%d) %s", info.file.source.type, info.file.source.attr,
			info.file.source.seek, info.file.meta, info.name);
		fc++;
#endif
	}
#ifdef HFS_DEBUG_TRACE
	qn_outputf("\tHFS: %d files", fc);
#endif

	qn_stream_seek(stream, pos, QNSEEK_BEGIN);
	return true;
}

//
static bool _hfs_save_dir(Hfs* self, const QnPathStr* dir, QnPathStr* save)
{
	if (qn_bstr_is_empty(dir) || qn_bstr_compare(dir, self->base.path.DATA, true) == 0)
	{
		qn_bstr_clear(save);
		return true;
	}
	qn_bstr_set_bstr(save, dir);
	return _hfs_chdir(self, dir->DATA);
}

//
static void _hfs_restore_dir(Hfs* self, const QnPathStr* save)
{
	if (qn_bstr_is_have(save))
		_hfs_chdir(self, save->DATA);
}

//
static bool _hfs_mkdir(QnGam g, const char* directory)
{
	qn_return_on_ok(directory[0] == '.', false);
	qn_return_on_ok(directory[0] == '/' && directory[1] == '\0', true);
	qn_return_when_fail(strlen(directory) < QN_MAX_PATH, false);

	Hfs* self = qn_cast_type(g, Hfs);
	QnStream* stream = qn_get_gam_desc(self, QnStream*);

	QnPathStr dir, name, save;
	_hfs_split_path(directory, &dir, &name);
	if (name.LENGTH >= HFS_MAX_NAME)
	{
		errno = ENAMETOOLONG;
		return false;
	}
	if (_hfs_save_dir(self, &dir, &save) == false)
	{
		errno = ENOENT;
		return false;
	}

	uint hash = qn_strshash(name.DATA);
	size_t i;
	qn_arr_foreach(&self->infos, i)
	{
		HfsInfo* info = &qn_arr_nth(&self->infos, i);
		if (hash == info->file.hash && qn_bstr_compare(&name, info->name, true) == 0)
		{
			// UNDONE: 옛날 주석에 의하면 패스 잡는 방법이 틀혔다고 함
			_hfs_restore_dir(self, &save);
			errno = EEXIST;
			return false;
		}
	}

	if (qn_stream_seek(stream, 0, QNSEEK_END) == 0)
	{
		_hfs_restore_dir(self, &save);
		return false;
	}

	// 새 디렉토리
	uint next = (uint)qn_stream_tell(stream);
	_hfs_write_directory(stream, name.DATA, name.LENGTH, 0, (uint)(next + sizeof(HfsFile) + name.LENGTH), 0);

	// 현재 디렉토리
	uint curr = (uint)qn_stream_tell(stream);
	_hfs_write_directory(stream, ".", 1, (uint)(curr + sizeof(HfsFile) + 1), (uint)(next + sizeof(HfsFile) + name.LENGTH), 0);

	// 상위 디렉토리
	HfsInfo* parent = &qn_arr_nth(&self->infos, 0);
	_hfs_write_directory(stream, "..", 2, 0, parent->file.source.seek, parent->file.stc.stamp);

	// 지금꺼 갱신
	HfsInfo* last = &qn_arr_inv(&self->infos, 0);
	qn_stream_seek(stream, last->file.source.seek + HFSAT_NEXT, QNSEEK_BEGIN);
	qn_stream_write(stream, &next, 0, (int)sizeof(uint));

	//
	if (qn_bstr_is_empty(&save))
		qn_bstr_set_char(&save, '.');
	_hfs_restore_dir(self, &save);

	self->touch++;
	return true;
}

// 제거
static bool _hfs_remove(QnGam g, const char* path)
{
	Hfs* self = qn_cast_type(g, Hfs);
	QnStream* stream = qn_get_gam_desc(self, QnStream*);

	QnPathStr dir, name, save;
	_hfs_split_path(path, &dir, &name);
	if (name.LENGTH >= HFS_MAX_NAME)
	{
		errno = ENAMETOOLONG;
		return false;
	}
	if (name.DATA[0] == '.')
	{
		errno = EINVAL;
		return false;
	}
	if (_hfs_save_dir(self, &dir, &save) == false)
	{
		errno = ENOENT;
		return false;
	}

	uint hash = qn_strshash(name.DATA);
	HfsInfo* info = NULL;
	size_t i;
	qn_arr_foreach(&self->infos, i)
	{
		info = &qn_arr_nth(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.len && qn_bstr_compare(&name, info->name, true) == 0)
			break;
	}
	if (i == qn_arr_count(&self->infos))
	{
		_hfs_restore_dir(self, &save);
		return false;
	}

	uint next = info->file.next;
	HfsInfo* prev = &qn_arr_nth(&self->infos, i - 1);
	if (qn_stream_seek(stream, prev->file.source.seek + HFSAT_NEXT, QNSEEK_BEGIN) <= 0 ||
		qn_stream_write(stream, &next, 0, sizeof(uint)) != sizeof(uint))
	{
		_hfs_restore_dir(self, &save);
		return false;
	}

	qn_arr_remove_nth(HfsArrInfo, &self->infos, i);
	_hfs_restore_dir(self, &save);

	self->touch++;
	return true;
}


//.mount_list = _disk_fs_list,

// HFS 만들기
static bool _hfs_create_file(_In_ QnStream* st, char* desc)
{
	QnDateTime dt = { qn_now() };
	HfsHeader header =
	{
		.header = HFS_HEADER,
		.version = HFS_VERSION,
		.notuse = 0,
		.stc = dt.stamp,
		.stw = dt.stamp,
		.revision = 0,
		.desc = { 0 },
	};
	if (desc != NULL)
		qn_strncpy(header.desc, desc, QN_COUNTOF(header.desc) - 1);
#ifdef _QN_WINDOWS_
	else
	{
		wchar computer[MAX_COMPUTERNAME_LENGTH + 1], user[32];
		DWORD clen = MAX_COMPUTERNAME_LENGTH, ulen = 31;
		GetComputerName(computer, &clen);
		GetUserName(user, &ulen);

		if (clen + ulen + 8 + 1 > QN_COUNTOF(header.desc))
			qn_snprintf(header.desc, QN_COUNTOF(header.desc), "Created by %ls@%ls", user, computer);
		else
			qn_snprintf(header.desc, QN_COUNTOF(header.desc), "Created by %ls@%ls, %04d-%02d-%02d",
				user, computer, dt.year, dt.month, dt.day);
	}
#elif defined _QN_UNIX_ && !defined(_QN_MOBILE_)
	else
	{
		char hostname[HOST_NAME_MAX], user[LOGIN_NAME_MAX];
		gethostname(hostname, HOST_NAME_MAX);
		getlogin_r(user, LOGIN_NAME_MAX);
		if (strlen(hostname) + strlen(user) + 8 + 1 > QN_COUNTOF(header.desc))
			qn_snprintf(header.desc, QN_COUNTOF(header.desc), "Created by %s@%s", user, hostname);
		else
			qn_snprintf(header.desc, QN_COUNTOF(header.desc), "Created by %s@%s, %04d-%02d-%02d",
				user, hostname, dt.year, dt.month, dt.day);
	}
#endif
	if (qn_stream_write(st, &header, 0, sizeof(HfsHeader)) != sizeof(HfsHeader))
		return false;

	_hfs_write_directory(st, ".", 1, 0, HFSAT_ROOT, 0);

	return true;
}

// HFS 열기
static QnStream* _hfs_open_file(const char* filename, bool can_write, bool use_mem, bool is_create, HfsHeader* hdr)
{
	QnStream* st;

	if (use_mem)
	{
		// 메모리에 만들던가 읽자
		if (is_create)
		{
			// 새로 만들자
			st = qn_create_mem_stream(filename, 1024);
			qn_return_when_fail(st != NULL, NULL);

			_hfs_create_file(st, NULL);
			_mem_stream_seek(st, 0, QNSEEK_BEGIN);
		}
		else
		{
			// 파일을 메모리로 올리자
			QnStream* fs = _file_stream_open(NULL, filename, "rb");
			qn_return_when_fail(fs != NULL, NULL);

			if (qn_stream_read(fs, hdr, 0, sizeof(HfsHeader)) != sizeof(HfsHeader))
			{
				qn_unload(fs);
				return NULL;
			}

			qn_stream_seek(fs, 0, QNSEEK_BEGIN);
			llong size = qn_stream_size(fs);
			if (size < 0 || size >= (512LL * 1024LL * 1024LL))
			{
				// 최대 512MB 까지만
				qn_unload(fs);
				return NULL;
			}

			st = qn_create_mem_stream(filename, (size_t)size);
			if (st == NULL)
			{
				qn_unload(fs);
				return NULL;
			}

			byte* buffer = qn_alloc(1024LL * 1024LL, byte);
			for (;;)
			{
				int read = qn_stream_read(fs, buffer, 0, 1024LL * 1024LL);
				if (read <= 0)
					break;
				qn_stream_write(st, buffer, 0, read);
			}

			_mem_stream_seek(st, 0, QNSEEK_BEGIN);
			qn_unload(fs);
		}
	}
	else
	{
		// 파일로 열자
		qn_return_when_fail(filename != NULL, NULL);
		if (is_create == false)
		{
			st = _file_stream_open(NULL, filename, can_write ? "rb+@R" : "rb@R");
			qn_return_when_fail(st != NULL, NULL);
		}
		else
		{
			st = _file_stream_open(NULL, filename, "wb+@R");
			qn_return_when_fail(st != NULL, NULL);
			_hfs_create_file(st, NULL);
			_file_stream_flush(st);
			_file_stream_seek(st, 0, QNSEEK_BEGIN);
		}
	}

	if (qn_stream_read(st, hdr, 0, sizeof(HfsHeader)) != sizeof(HfsHeader) ||
		hdr->header != HFS_HEADER || hdr->version != HFS_VERSION)
	{
		qn_unload(st);
		return NULL;
	}

	return st;
}

// 제거
static void _hfs_dispose(QnGam g)
{
	Hfs* self = qn_cast_type(g, Hfs);
	QnStream* stream = qn_get_gam_desc(self, QnStream*);

	if (self->touch > 0)
	{
		self->header.revision++;
		qn_stream_seek(stream, HFSAT_ROOT_REV, QNSEEK_BEGIN);
		qn_stream_write(stream, &self->header.revision, 0, sizeof(self->header.revision));
		self->header.stw.stamp = qn_now();
		qn_stream_seek(stream, HFSAT_ROOT_STW, QNSEEK_BEGIN);
		qn_stream_write(stream, &self->header.stw, 0, sizeof(QnDateTime));
	}

	qn_arr_disp(&self->infos);
	qn_unload(stream);
	qn_free(self->base.name);
	qn_free(self);
}

// 진짜 만들기
static QnMount* _create_hfs(const char* filename, const char* mode)
{
	bool can_write = false, use_mem = false, is_create = false;
	if (mode != NULL)
	{
		for (const char* p = mode; *p != '\0'; p++)
		{
			if (*p == 'c')
			{
				is_create = true;
				can_write = true;
			}
			else if (*p == 'm' || *p == 'i')
			{
				if (is_create == false)
					is_create = filename == NULL;
				use_mem = true;
				can_write = true;
			}
			else if (*p == '+')
				can_write = true;
		}
	}

	HfsHeader hdr;
	QnStream* stream = _hfs_open_file(filename, can_write, use_mem, is_create, &hdr);
	qn_return_when_fail(stream != NULL, NULL);

	Hfs* self = qn_alloc_zero_1(Hfs);
	self->header = hdr;
	self->base.name = qn_strdup(filename);
	self->base.name_len = filename == NULL ? 0 : strlen(filename);
	self->base.flags = QNMF_READ | QNMFT_HFS;
	if (use_mem)
		self->base.flags |= QNMFT_MEM;
	if (can_write)
		self->base.flags |= QNMF_WRITE;
	qn_set_gam_desc(self, stream);
	_hfs_chdir(qn_cast_type(self, QnMount), "/");

	static const QN_DECL_VTABLE(QNMOUNT) _hfs_vt =
	{
		.base.name = "HFS",
		.base.dispose = _hfs_dispose,
		//.mount_open = _disk_fs_open,
		//.mount_read = _disk_fs_read,
		//.mount_read_text = _disk_fs_read_text,
		//.mount_exist = _disk_fs_exist,
		.mount_remove = _hfs_remove,
		.mount_chdir = _hfs_chdir,
		.mount_mkdir = _hfs_mkdir,
		//.mount_list = _disk_fs_list,
	};
	return qn_gam_init(self, _hfs_vt);
}

// 설명 넣기
bool qn_hfs_set_desc(QnMount* mount, const char* desc)
{
	qn_return_when_fail((mount->flags & (QNMF_WRITE | QNMFT_HFS)) == (QNMF_WRITE | QNMFT_HFS), false);
	Hfs* self = qn_cast_type(mount, Hfs);
	qn_strncpy(self->header.desc, desc, QN_COUNTOF(self->header.desc) - 1);
	self->touch++;
	return true;
}

// 버퍼 넣기
bool qn_hfs_store_buffer(QnMount* mount, const char* filename, const void* data, uint size, bool cmpr, QnFileType type)
{
	if ((mount->flags & (QNMF_WRITE | QNMFT_HFS)) != (QNMF_WRITE | QNMFT_HFS))
	{
		errno = EACCES;
		return false;
	}
	if (filename == NULL || *filename == '\0' || data == NULL)
	{
		errno = EINVAL;
		return false;
	}
	if (size == 0 || size > (2040U * 1024U * 1024U))	// 1.99GB(2040MB) 이상은 안된다
	{
		errno = EINVAL;
		return false;
	}

	QnPathStr dir, name, save;
	_hfs_split_path(filename, &dir, &name);
	if (name.LENGTH >= HFS_MAX_NAME)
	{
		errno = ENAMETOOLONG;
		return false;
	}
	if (_hfs_save_dir(qn_cast_type(mount, Hfs), &dir, &save) == false)
	{
		errno = ENOENT;
		return false;
	}

	//
	Hfs* self = qn_cast_type(mount, Hfs);

	uint hash = qn_strshash(name.DATA);
	size_t i;
	qn_arr_foreach(&self->infos, i)
	{
		HfsInfo* info = &qn_arr_nth(&self->infos, i);
		if (hash == info->file.hash && qn_bstr_compare(&name, info->name, true) == 0)
		{
			_hfs_restore_dir(self, &save);
			errno = EEXIST;
			return false;
		}
	}

	//
	void* buffer = NULL;
	size_t bufsize = 0;
	if (cmpr)
	{
		buffer = qn_memzcpr(data, size, &bufsize);
		if (buffer == NULL)
		{
			bufsize = 0;
			cmpr = false;
		}
		else if (size < 10ULL * 1024ULL * 1024ULL)
		{
			// 10MB 미만일때 압축 크기가 90% 이상이면 그냥 넣는다
			double d = size * 0.9;
			if (bufsize >= d)
			{
				qn_free(buffer);
				bufsize = 0;
				cmpr = false;
			}
		}
	}

	//
	QnStream* stream = qn_get_gam_desc(self, QnStream*);
	qn_stream_seek(stream, 0, QNSEEK_END);
	uint next = (uint)qn_stream_tell(stream);

	HfsInfo file =
	{
		.file.source.magic = 0,
		.file.source.type = type,
		.file.source.attr = QNFATTR_FILE,
		.file.source.size = size,
		.file.source.cmpr = 0,
		.file.source.seek = 0,
		.file.stc.stamp = qn_now(),
		.file.ext = 0,
		//.file.len = name.LENGTH,	// len, hash는 _hfs_write_file_header()에서 채워진다
		//.file.hash = hash,
		.file.meta = 0,
		.file.next = 0,
	};

	bool isok = false;
	if (cmpr)
	{
		file.file.source.attr |= QNFATTR_CMPR;
		file.file.source.cmpr = (uint)bufsize;
		if (_hfs_write_file_header(stream, &file.file, name.DATA, name.LENGTH) &&
			qn_stream_write(stream, buffer, 0, (int)bufsize) == (int)bufsize)
			isok = true;
		qn_free(buffer);
	}
	else
	{
		if (_hfs_write_file_header(stream, &file.file, name.DATA, name.LENGTH) &&
			qn_stream_write(stream, data, 0, (int)size) == (int)size)
			isok = true;
	}
	if (isok == false)
	{
		_hfs_restore_dir(self, &save);
		return false;
	}

	// 지금꺼 갱신
	HfsInfo* last = &qn_arr_inv(&self->infos, 0);
	last->file.next = next;
	qn_stream_seek(stream, last->file.source.seek + HFSAT_NEXT, QNSEEK_BEGIN);
	qn_stream_write(stream, &next, 0, (int)sizeof(uint));

	file.file.source.seek = next;
	qn_strcpy(file.name, name.DATA);
	qn_arr_add(HfsArrInfo, &self->infos, file);

	//
	_hfs_restore_dir(self, &save);
	self->touch++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
// 만들기!

//
QnMount* qn_open_mount(const char* path, const char* mode)
{
	char* tmppath;
	bool hfs;

	if (mode != NULL)
		hfs = qn_strchr(mode, 'h') != NULL;
	else
	{
		mode = "";
		hfs = false;
	}

	if (path == NULL)
	{
		// 파일 이름이 없을 때
		if (hfs)
			return _create_hfs(NULL, mode);
		// HFS가 아니라면 현재 기본 경로를 연다
		tmppath = qn_basepath();
		return _create_diskfs(tmppath);
	}

	QnFileAttr attr = qn_get_file_attr(path);
	if (QN_TMASK(attr, QNFATTR_DIR))
	{
		// 디렉토리가 있으면 디렉토리를 연다
		if (hfs)
			return NULL;
		tmppath = qn_strdup(path);
		return _create_diskfs(tmppath);
	}
	if (attr != QNFATTR_NONE)
	{
		// 파일이 있으면 HFS로 열어본다
		return hfs ? _create_hfs(path, mode) : NULL;
	}

	// 파일이 없네
	if (hfs)
		return _create_hfs(path, mode);

	// HFS가 아니라면 디렉토리를 만든다
	if (qn_strchr(mode, 'c') != NULL)
	{
#ifdef _QN_WINDOWS_
		wchar* pwz = qn_u8to16_dup(path, 0);
		BOOL b = CreateDirectory(pwz, NULL);
		qn_free(pwz);
		if (b == FALSE)
			return NULL;
#else
		if (mkdir(path, 0755) != 0)
			return NULL;
#endif
		tmppath = qn_strdup(path);
		return _create_diskfs(tmppath);
	}

	return NULL;
}
