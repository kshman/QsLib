//
// qn_file.c - 파일 처리
// 2023-12-27 by kim
//

// ReSharper disable CppParameterMayBeConst

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

// 최대 파일 할당 크기
static size_t max_file_alloc_size = 128ULL * 1024ULL * 1024ULL;

//
size_t qn_file_get_max_alloc_size(void)
{
	return max_file_alloc_size;
}

//
void qn_file_set_max_alloc_size(const size_t size)
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
static char* _qn_read_sym_link(const char* path)
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
		path = _qn_read_sym_link("/proc/curproc/file");
#elif defined _QN_LINUX_
		path = _qn_read_sym_link("/proc/self/exe");
		if (path == NULL)
		{
			char tmp[64];
			const int rc = qn_snprintf(tmp, QN_COUNTOF(tmp), "/proc/%llu/exe", (ullong)getpid());
			if (rc > 0 && rc < (int)QN_COUNTOF(tmp))
				path = _qn_read_sym_link(tmp);
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


/////////////////////////////////////////////////////////////////////////
// 스트림 연계 함수

static QnStream* qn_file_stream_open(QnMount* mount, const char* RESTRICT filename, const char* RESTRICT mode);
static int qn_file_stream_exist(const char* filename);

//
QnStream* qn_open_stream(QnMount* mount, const char* RESTRICT filename, const char* RESTRICT mode)
{
	qn_return_when_fail(filename != NULL, NULL);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_open(mount, filename, mode);
	return qn_file_stream_open(NULL, filename, mode);
}

//
int qn_file_exist(QnMount* mount, const char* RESTRICT filename)
{
	qn_return_when_fail(filename != NULL, false);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_exist(mount, filename);
	return qn_file_stream_exist(filename);
}

// 파일 한번에 읽기
static void* qn_internal_file_alloc(const char* RESTRICT filename, int* size)
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
		*size = n;
#endif
	return buf;
}

//
void* qn_file_alloc(QnMount* mount, const char* RESTRICT filename, int* RESTRICT size)
{
	qn_return_when_fail(filename != NULL, NULL);
	if (mount == NULL)
		return qn_internal_file_alloc(filename, size);

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
static void qn_internal_detect_file_encoding(char* RESTRICT data, int size, int* RESTRICT length, int* RESTRICT codepage)
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
	qn_internal_detect_file_encoding(data, size, length, codepage);
	return data;
}


//////////////////////////////////////////////////////////////////////////
// 스트림

//
int qn_stream_read(QnStream* self, void* RESTRICT buffer, const int offset, const int size)
{
	qn_return_when_fail(buffer != NULL, -1);
	qn_return_when_fail(size >= 0, 0);
	return qn_cast_vtable(self, QNSTREAM)->stream_read(self, buffer, offset, size);
}

//
int qn_stream_write(QnStream* self, const void* RESTRICT buffer, const int offset, const int size)
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
	char buffer[4096];
	size_t loc;
} StreamPrintfBuffer;

// 정리
static void stream_printf_finish(PatrickPowellSprintfState* state)
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
static void stream_printf_outch(PatrickPowellSprintfState* state, int ch)
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
		stream_printf_outch,
		stream_printf_finish,
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
const char* qn_dir_read(QnDir * self)
{
	return qn_cast_vtable(self, QNDIR)->read(self);
}

//
bool qn_dir_read_info(QnDir * self, QnFileInfo * info)
{
	qn_return_when_fail(info, false);
	return qn_cast_vtable(self, QNDIR)->read_info(self, info);
}

//
void qn_dir_rewind(QnDir * self)
{
	qn_cast_vtable(self, QNDIR)->rewind(self);
}

//
void qn_dir_seek(QnDir * self, const int pos)
{
	qn_cast_vtable(self, QNDIR)->seek(self, pos);
}

//
int qn_dir_tell(QnDir * self)
{
	return qn_cast_vtable(self, QNDIR)->tell(self);
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
int qn_mount_exist(QnMount* g, const char* RESTRICT path)
{
	qn_return_when_fail(path, false);
	return qn_cast_vtable(g, QNMOUNT)->mount_exist(g, path);
}

//
bool qn_mount_remove(QnMount* g, const char* RESTRICT path)
{
	qn_return_when_fail(path, false);
	return qn_cast_vtable(g, QNMOUNT)->mount_remove(g, path);
}

//
bool qn_mount_chdir(QnMount* g, const char* RESTRICT directory)
{
	return qn_cast_vtable(g, QNMOUNT)->mount_chdir(g, directory);
}

//
bool qn_mount_mkdir(QnMount* g, const char* RESTRICT directory)
{
	qn_return_when_fail(directory, false);
	return qn_cast_vtable(g, QNMOUNT)->mount_mkdir(g, directory);
}

//
QnDir* qn_mount_directory(QnMount* g)
{
	return qn_cast_vtable(g, QNMOUNT)->mount_directory(g);
}


//////////////////////////////////////////////////////////////////////////
// 메모리 스트림

//
typedef struct QNMEMSTREAM
{
	QnStream			base;
	size_t				capa;
	size_t				size;
	size_t				loc;
} QnMemStream;

static void qn_mem_stream_dispose(QnGam g);
static int qn_mem_stream_read(QnGam g, void* RESTRICT buffer, const int offset, const int size);
static int qn_mem_stream_write(QnGam g, const void* RESTRICT buffer, const int offset, const int size);
static llong qn_mem_stream_seek(QnGam g, const llong offset, const QnSeek org);
static llong qn_mem_stream_tell(QnGam g);
static llong qn_mem_stream_size(QnGam g);
static bool qn_mem_stream_flush(QnGam g);
static QnStream* qn_mem_stream_dup(QnGam g);

static QN_DECL_VTABLE(QNSTREAM) qn_mem_stream_vt =
{
	.base.name = "MemoryStream",
	.base.dispose = qn_mem_stream_dispose,
	.stream_read = qn_mem_stream_read,
	.stream_write = qn_mem_stream_write,
	.stream_seek = qn_mem_stream_seek,
	.stream_tell = qn_mem_stream_tell,
	.stream_size = qn_mem_stream_size,
	.stream_flush = qn_mem_stream_flush,
	.stream_dup = qn_mem_stream_dup,
};

// 메모리 스트림 만들기
QnStream* qn_create_mem_stream(const char* RESTRICT name, size_t initial_capacity)
{
	QnMemStream* self = qn_alloc(1, QnMemStream);

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
	self->base.flags = QNFF_ALL | QNFFT_MEM;
	qn_set_gam_desc(self, data);

	return qn_gam_init(self, qn_mem_stream_vt);
}

// 메모리 스트림 외부 데이터로 만들기
QnStream* qn_create_mem_stream_stored(const char* RESTRICT name, void* RESTRICT data, size_t size)
{
	QnMemStream* self = qn_alloc(1, QnMemStream);

	self->base.mount = NULL;
	self->base.name = qn_strdup(name);
	self->base.flags = QNFF_ALL | QNFFT_MEM;
	self->capa = size;
	self->size = size;
	qn_set_gam_desc(self, data);

	return qn_gam_init(self, qn_mem_stream_vt);
}

// 메모리 스트림 닫기
static void qn_mem_stream_dispose(QnGam g)
{
	QnMemStream* self = qn_cast_type(g, QnMemStream);
	qn_free(qn_get_gam_pointer(self));
	qn_free(self->base.name);
	qn_free(self);
}

//
static int qn_mem_stream_read(QnGam g, void* RESTRICT buffer, const int offset, const int size)
{
	QnMemStream* self = qn_cast_type(g, QnMemStream);

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
static int qn_mem_stream_write(QnGam g, const void* RESTRICT buffer, const int offset, const int size)
{
	QnMemStream* self = qn_cast_type(g, QnMemStream);

	size_t capa = QN_MAX(self->capa, 64);
	size_t sub = self->capa - self->loc;
	if ((int)sub < size)
	{
		sub = self->loc + size;
		if (capa == 0)
			capa = 16;
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
static llong qn_mem_stream_seek(QnGam g, const llong offset, const QnSeek org)
{
	QnMemStream* self = qn_cast_type(g, QnMemStream);
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
static llong qn_mem_stream_tell(QnGam g)
{
	QnMemStream* self = qn_cast_type(g, QnMemStream);
	return (llong)self->loc;
}

//
static llong qn_mem_stream_size(QnGam g)
{
	QnMemStream* self = qn_cast_type(g, QnMemStream);
	return (llong)self->size;
}

//
static bool qn_mem_stream_flush(QnGam g)
{
	QN_DUMMY(g);
	return true;
}

//
static QnStream* qn_mem_stream_dup(QnGam g)
{
	QnMemStream* source = qn_cast_type(g, QnMemStream);
	QnMemStream* self = qn_alloc(1, QnMemStream);

	byte* data = qn_alloc(source->size, byte);
	memcpy(data, qn_get_gam_pointer(source), source->size);

	self->base.mount = NULL;
	self->base.name = qn_strdup(source->base.name);
	self->base.flags = source->base.flags;
	self->capa = source->size;
	self->size = source->size;
	self->loc = 0;
	qn_set_gam_desc(self, data);

	return qn_gam_init(self, qn_mem_stream_vt);
}

//
const void* qn_mem_stream_get_data(QnStream* self)
{
	return QN_TMASK(self->flags, QNFFT_MEM) ? qn_get_gam_pointer(self) : NULL;
}


//////////////////////////////////////////////////////////////////////////
// 파일 스트림

//
typedef struct QNFILESTREAM
{
	QnStream			base;
	QnFileAccess		acs;
} QnFileStream;

static void qn_file_stream_access_parse(const char* mode, QnFileAccess* acs, QnFileFlag* flags);
static char* qn_file_stream_get_real_path(char* RESTRICT dest, const QnMount* mount, const char* RESTRICT filename);
static void qn_file_stream_dispose(QnGam g);
static int qn_file_stream_read(QnGam g, void* RESTRICT buffer, const int offset, const int size);
static int qn_file_stream_write(QnGam g, const void* RESTRICT buffer, const int offset, const int size);
static llong qn_file_stream_seek(QnGam g, const llong offset, const QnSeek org);
static llong qn_file_stream_tell(QnGam g);
static llong qn_file_stream_size(QnGam g);
static bool qn_file_stream_flush(QnGam g);
static QnStream* qn_file_stream_dup(QnGam g);

static QN_DECL_VTABLE(QNSTREAM) qn_file_stream_vt =
{
	.base.name = "FileStream",
	.base.dispose = qn_file_stream_dispose,
	.stream_read = qn_file_stream_read,
	.stream_write = qn_file_stream_write,
	.stream_seek = qn_file_stream_seek,
	.stream_tell = qn_file_stream_tell,
	.stream_size = qn_file_stream_size,
	.stream_flush = qn_file_stream_flush,
	.stream_dup = qn_file_stream_dup,
};

// 파일 스트림 열기
static QnStream* qn_file_stream_open(QnMount* mount, const char* RESTRICT filename, const char* RESTRICT mode)
{
	char path[QN_MAX_PATH];
	if (mount != NULL)
	{
		filename = qn_file_stream_get_real_path(path, mount, filename);
		qn_return_when_fail(filename, NULL);
	}
	QnFileStream* self = qn_alloc(1, QnFileStream);

	qn_file_stream_access_parse(mode, &self->acs, &self->base.flags);
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, filename, 0);
	if (uni_len >= QN_MAX_PATH - QN_MAX_PATH_BIAS)
	{
		// 파일 이름이 너무 길다
		qn_free(self);
		return NULL;
	}
	HANDLE fd = CreateFile(uni, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);
	if (fd == NULL || fd == INVALID_HANDLE_VALUE)
	{
		qn_free(self);
		return NULL;
	}
#else
	int fd = self->acs.access == 0 ? open(filename, self->acs.mode) : open(filename, self->acs.mode, self->acs.access);
	if (fd < 0)
	{
		qn_free(self);
		return NULL;
	}
#endif

	self->base.mount = qn_load(mount);
	self->base.name = qn_strdup(filename);
	self->base.flags |= QNFFT_FILE;
	qn_set_gam_desc(self, fd);

	return qn_gam_init(self, qn_file_stream_vt);
}

// 파일 스트림이 있나
static int qn_file_stream_exist(const char* path)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, path, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, 0);
	const DWORD attr = GetFileAttributes(uni);
	int ret = attr == INVALID_FILE_ATTRIBUTES ? 0 : QN_TMASK(attr, FILE_ATTRIBUTE_DIRECTORY) ? 2 : 1;
#else
	struct stat st;
	int n = stat(path, &st);
	int ret = n < 0 ? 0 : S_ISDIR(st.st_mode) ? 2 : 1;
#endif
	return ret;
}

// 분석
static void qn_file_stream_access_parse(const char* mode, QnFileAccess* acs, QnFileFlag* flags)
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
					break;
				case '!':
					if (acs->mode == CREATE_ALWAYS)
						acs->attr |= FILE_FLAG_DELETE_ON_CLOSE;
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
	char ch, cm;

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
					cm = *(mode + 1);

					if (cm != '\0')
					{
						if (cm == 'U')
						{
							ch = *(mode + 2);
							if (ch != '\0')
							{
								if (ch == 'R')
									acs->access |= S_IRUSR;
								else if (ch == 'W')
									acs->access |= S_IWUSR;
								else if (ch == 'X')
									acs->access |= S_IXUSR;
								mode++;
							}
							mode++;
						}
						else if (cm == 'G')
						{
							ch = *(mode + 2);
							if (ch != '\0')
							{
								if (ch == 'R')
									acs->access |= S_IRGRP;
								else if (ch == 'W')
									acs->access |= S_IWGRP;
								else if (ch == 'X')
									acs->access |= S_IXGRP;
								mode++;
							}
							mode++;
						}
						else if (cm == 'O')
						{
							ch = *(mode + 2);
							if (ch != '\0')
							{
								if (ch == 'R')
									acs->access |= S_IROTH;
								else if (ch == 'W')
									acs->access |= S_IWOTH;
								else if (ch == 'X')
									acs->access |= S_IXOTH;
								mode++;
							}
							mode++;
						}
						else if (isdigit(cm))
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
static char* qn_file_stream_get_real_path(char* RESTRICT dest, const QnMount* mount, const char* RESTRICT filename)
{
#ifdef _QN_WINDOWS_
	if (filename[0] != '\0' && filename[1] == ':')
	{
		qn_mesg(true, "FileStream", "cannot use drive letter");
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

// 파일 스트림 닫기
static void qn_file_stream_dispose(QnGam g)
{
	QnFileStream* self = qn_cast_type(g, QnFileStream);
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
static int qn_file_stream_read(QnGam g, void* RESTRICT buffer, const int offset, const int size)
{
	QnFileStream* self = qn_cast_type(g, QnFileStream);
	byte* ptr = (byte*)buffer + offset;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return ReadFile(qn_get_gam_handle(self), ptr, size, &ret, NULL) ? (int)ret : -1;
#else
	return (int)read(qn_get_gam_desc_int(self), ptr, (size_t)size);
#endif
}

//
static int qn_file_stream_write(QnGam g, const void* RESTRICT buffer, const int offset, const int size)
{
	QnFileStream* self = qn_cast_type(g, QnFileStream);
	const byte* ptr = (const byte*)buffer + offset;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return WriteFile(qn_get_gam_handle(self), ptr, size, &ret, NULL) ? (int)ret : -1;
#else
	return (int)write(qn_get_gam_desc_int(self), ptr, (size_t)size);
#endif
}

//
static llong qn_file_stream_seek(QnGam g, const llong offset, const QnSeek org)
{
	QnFileStream* self = qn_cast_type(g, QnFileStream);
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
static llong qn_file_stream_tell(QnGam g)
{
	QnFileStream* self = qn_cast_type(g, QnFileStream);
#ifdef _QN_WINDOWS_
	const LARGE_INTEGER cli = { .QuadPart = 0 };
	LARGE_INTEGER li;
	return SetFilePointerEx(qn_get_gam_handle(self), cli, &li, FILE_CURRENT) ? li.QuadPart : -1;
#else
	return lseek(qn_get_gam_desc_int(self), 0, SEEK_CUR);
#endif
}

//
static llong qn_file_stream_size(QnGam g)
{
	QnFileStream* self = qn_cast_type(g, QnFileStream);
#ifdef _QN_WINDOWS_
	LARGE_INTEGER li;
	return GetFileSizeEx(qn_get_gam_handle(self), &li) ? li.QuadPart : -1;
#else
	struct stat st;
	return fstat(qn_get_gam_desc_int(self), &st) < 0 ? -1 : st.st_size;
#endif
}

//
static bool qn_file_stream_flush(QnGam g)
{
	QnFileStream* self = qn_cast_type(g, QnFileStream);
#ifdef _QN_WINDOWS_
	return FlushFileBuffers(qn_get_gam_handle(self)) != 0;
#else
	return fsync(qn_get_gam_desc_int(self)) == 0;
#endif
}

//
static QnStream* qn_file_stream_dup(QnGam g)
{
	QnFileStream* source = qn_cast_type(g, QnFileStream);
	QnFileStream* self = qn_alloc(1, QnFileStream);

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

	return qn_gam_init(self, qn_file_stream_vt);
}


//////////////////////////////////////////////////////////////////////////
// 디스크 파일 리스트


//////////////////////////////////////////////////////////////////////////
// 디스크 파일 시스템 마운트

static void qn_diskfs_dispose(QnGam g);
static QnStream* qn_diskfs_open(QnGam g, const char* RESTRICT filename, const char* RESTRICT mode);
static void* qn_diskfs_read(QnGam g, const char* filename, int* size);
static char* qn_diskfs_read_text(QnGam g, const char* filename, int* length, int* codepage);
static int qn_diskfs_exist(QnGam g, const char* path);
static bool qn_diskfs_remove(QnGam g, const char* path);
static bool qn_diskfs_ch_dir(QnGam g, const char* directory);
static bool qn_diskfs_mk_dir(QnGam g, const char* directory);

static QN_DECL_VTABLE(QNMOUNT) qn_diskfs_vt =
{
	.base.name = "DiskFS",
	.base.dispose = qn_diskfs_dispose,
	.mount_open = qn_diskfs_open,
	.mount_read = qn_diskfs_read,
	.mount_read_text = qn_diskfs_read_text,
	.mount_exist = qn_diskfs_exist,
	.mount_remove = qn_diskfs_remove,
	.mount_chdir = qn_diskfs_ch_dir,
	.mount_mkdir = qn_diskfs_mk_dir,
};

// 디스크 파일 시스템 마운트 만들기, path에 대한 오류처리는 하고 왔을 것이다
static QnMount* qn_create_diskfs(char* path, QnMountFlag flags)
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
	self->flags = flags | QNMFT_DISKFS;

	return qn_gam_init(self, qn_diskfs_vt);
}

//
static void qn_diskfs_dispose(QnGam g)
{
	QnMount* self = qn_cast_type(g, QnMount);
	qn_free(self->name);
	qn_free(self);
}

//
static QnStream* qn_diskfs_open(QnGam g, const char* RESTRICT filename, const char* RESTRICT mode)
{
	return qn_file_stream_open(qn_cast_type(g, QnMount), filename, mode);
}

//
static void* qn_diskfs_read(QnGam g, const char* filename, int* size)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(qn_file_stream_get_real_path(real, self, filename), NULL);
	char* data = qn_internal_file_alloc(real, size);
	return data;
}

//
static char* qn_diskfs_read_text(QnGam g, const char* filename, int* length, int* codepage)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(qn_file_stream_get_real_path(real, self, filename), NULL);
	int size;
	char* data = qn_internal_file_alloc(real, &size);
	qn_internal_detect_file_encoding(data, size, length, codepage);
	return data;
}

//
static int qn_diskfs_exist(QnGam g, const char* path)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(qn_file_stream_get_real_path(real, self, path), 0);
	int ret = qn_file_stream_exist(real);
	return ret;
}

//
static bool qn_diskfs_remove(QnGam g, const char* path)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(qn_file_stream_get_real_path(real, self, path), false);

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
static bool qn_diskfs_ch_dir(QnGam g, const char* directory)
{
	QnMount* self = qn_cast_type(g, QnMount);

	if (directory == NULL || directory[0] == '/' || directory[0] == '\\' && directory[1] == '\0')
	{
		qn_bstr_set_len(&self->path, self->name, self->name_len);
		return true;
	}

	char real[QN_MAX_PATH];
	qn_return_when_fail(qn_file_stream_get_real_path(real, self, directory), false);

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
static bool qn_diskfs_mk_dir(QnGam g, const char* directory)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(qn_file_stream_get_real_path(real, self, directory), false);

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


//////////////////////////////////////////////////////////////////////////
//

//
QnMount* qn_open_mount(const char* path, QnMountFlag flags)
{
	char* tmppath;

	if (path == NULL)
	{
		tmppath = qn_basepath();
		return qn_create_diskfs(tmppath, flags);
	}

	int exist = qn_file_stream_exist(path);
	if (exist == 0)
		return NULL;
	if (exist == 2)
	{
		tmppath = qn_strdup(path);
		return qn_create_diskfs(tmppath, flags);
	}
	if (exist == 1)
	{
		size_t len = strlen(path);
		if (len < 5)
			return NULL;
		char ext[4];
		qn_strncpy(ext, path + len - 3, 3);
		if (qn_stricmp(ext, "hfs") == 0)
		{
			// 여기서 HFS를 열어야함
			return NULL;
		}
	}

	return NULL;
}
