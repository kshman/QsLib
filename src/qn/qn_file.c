//
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

QN_IMPL_BSTR(QnPathStr, QN_MAX_PATH, qn_path_str);

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

// 파일 속성 변환
#ifdef _QN_WINDOWS_
static QnFileAttr _file_attr_convert(const DWORD attr)
{
	int ret = QNFATTR_FILE;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_DIRECTORY))
		ret |= QNFATTR_DIR;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_READONLY))
		ret |= QNFATTR_RDONLY;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_HIDDEN))
		ret |= QNFATTR_HIDDEN;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_SYSTEM))
		ret |= QNFATTR_SYSTEM;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_REPARSE_POINT))
		ret |= QNFATTR_LINK;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_COMPRESSED))
		ret |= QNFATTR_CMPR;
	if (QN_TMASK(attr, FILE_ATTRIBUTE_ENCRYPTED))
		ret |= QNFATTR_ENCR;
	return ret;
}
#else
static QnFileAttr _file_attr_convert(const struct stat* st)
{
	int ret = QNFATTR_FILE;
	if (S_ISDIR(st->st_mode))
		ret |= QNFATTR_DIR;
	if (S_ISLNK(st->st_mode))
		ret |= QNFATTR_LINK;
	return ret;
}
#endif

// 파일 핸들 복제
static nint _file_handle_dup(nint fd)
{
#ifdef _QN_WINDOWS_
	const HANDLE process = GetCurrentProcess();
	HANDLE dup;
	if (!DuplicateHandle(process, (HANDLE)fd, process, &dup, 0, FALSE, DUPLICATE_SAME_ACCESS))
		return -1;
	return (nint)dup;
#else
	return (nint)fcntl((int)fd, F_DUPFD_CLOEXEC, 0);
#endif
}

// 파일 속성 얻기
QnFileAttr qn_get_file_attr(const char* path)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, path, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, 0);
	const DWORD attr = GetFileAttributes(uni);
	if (attr == INVALID_FILE_ATTRIBUTES)
		return QNFATTR_NONE;
	return _file_attr_convert(attr);
#else
	struct stat st;
	int n = stat(path, &st);
	if (n < 0)
		return QNFATTR_NONE;
	return _file_attr_convert(&st);
#endif
}

//
size_t qn_filepath(const char* filename, char* dest, size_t destsize)
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
size_t qn_filename(const char* filename, char* dest, size_t destsize)
{
	qn_return_when_fail(filename != NULL, 0);

	size_t len = strlen(filename);
	const char* p = filename + len - 1;
	while (p >= filename && *p != '/' && *p != '\\')
		p--;

	if (p < filename)
	{
		if (dest)
			qn_strncpy(dest, filename, QN_MIN(destsize, len));
		return len;
	}

	len = strlen(p + 1);
	if (dest)
	{
		const size_t maxlen = QN_MIN(len, destsize - 1);
		memcpy(dest, p + 1, maxlen);
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
	size_t len = 256;
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
	DWORD len, dw = 256;
	wchar* pw = NULL;
	while (true)
	{
		pw = qn_realloc(pw, dw, wchar);
		len = GetModuleFileName(NULL, pw, dw);
		if (len < dw - 1)
			break;
		dw *= 2;
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

//
char* qn_getcwd(void)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const DWORD len = GetCurrentDirectory(QN_MAX_PATH, uni);
	if (len == 0)
		return NULL;
	return qn_u16to8_dup(uni, 0);
#else
	char* buf = NULL;
	size_t len = 256;
	ssize_t rc = -1;
	while (true)
	{
		buf = qn_realloc(buf, len, char);
		rc = getcwd(buf, len);
		if (rc != -1)
			return buf;
		if (errno != ERANGE)
			break;
		len *= 2;
	}
	qn_free(buf);
	return NULL;
#endif
}


/////////////////////////////////////////////////////////////////////////
// 스트림 연계 함수

static QnStream* _file_stream_open(QnMount* mount, const char* filename, const char* mode);

//
QnStream* qn_open_stream(QnMount* mount, const char* filename, const char* mode)
{
	qn_return_when_fail(filename != NULL, NULL);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_open(mount, filename, mode);
	return _file_stream_open(NULL, filename, mode);
}

//
QnFileAttr qn_file_attr(QnMount* mount, const char* filename)
{
	qn_return_when_fail(filename != NULL, false);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_exist(mount, filename);
	return qn_get_file_attr(filename);
}

// 파일 한번에 읽기
static void* _internal_file_alloc(const char* filename, int* size)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, filename, 0);
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

	byte* buf = qn_alloc((size_t)li.QuadPart + 4, byte);
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
	byte* buf = qn_alloc((size_t)st.st_size + 4, byte);
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
void* qn_file_alloc(QnMount* mount, const char* filename, int* size)
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
static void _internal_detect_file_encoding(char* data, int size, int* length, int* codepage)
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
		data[size] = '\0';
	}
}

//
char* qn_file_alloc_text(QnMount* mount, const char* filename, int* length, int* codepage)
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
int qn_stream_read(QnStream* self, void* buffer, int offset, int size)
{
	qn_return_when_fail(buffer != NULL, -1);
	qn_return_when_fail(size >= 0, 0);
	return qn_cast_vtable(self, QNSTREAM)->stream_read(self, buffer, offset, size);
}

//
int qn_stream_write(QnStream* self, const void* buffer, int offset, int size)
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
int qn_stream_vprintf(QnStream* self, const char* fmt, va_list va)
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
int qn_stream_printf(QnStream* self, const char* fmt, ...)
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
bool qn_dir_read_info(QnDir* self, QnFileInfo* info)
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
QnStream* qn_mount_open_stream(QnMount* g, const char* filename, const char* mode)
{
	qn_return_when_fail(filename, NULL);
	return qn_cast_vtable(g, QNMOUNT)->mount_open(g, filename, mode);
}

//
void* qn_mount_read(QnMount* g, const char* filename, int* size)
{
	qn_return_when_fail(filename, NULL);
	return qn_cast_vtable(g, QNMOUNT)->mount_read(g, filename, size);
}

//
char* qn_mount_read_text(QnMount* g, const char* filename, int* length, int* codepage)
{
	qn_return_when_fail(filename, NULL);
	return qn_cast_vtable(g, QNMOUNT)->mount_read_text(g, filename, length, codepage);
}

//
QnFileAttr qn_mount_exist(QnMount* g, const char* path)
{
	qn_return_when_fail(path, false);
	return qn_cast_vtable(g, QNMOUNT)->mount_exist(g, path);
}

//
bool qn_mount_remove(QnMount* g, const char* path)
{
	qn_return_when_fail(path, false);
	qn_return_when_fail(QN_TMASK(g->flags, QNMF_WRITE), false);
	return qn_cast_vtable(g, QNMOUNT)->mount_remove(g, path);
}

//
bool qn_mount_chdir(QnMount* g, const char* directory)
{
	qn_return_when_fail(directory, false);
	return qn_cast_vtable(g, QNMOUNT)->mount_chdir(g, directory);
}

//
bool qn_mount_mkdir(QnMount* g, const char* directory)
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
static int _mem_stream_read(QnGam g, void* buffer, const int offset, const int size)
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
static int _mem_stream_write(QnGam g, const void* buffer, const int offset, const int size)
{
	MemStream* self = qn_cast_type(g, MemStream);

	size_t sub = self->capa - self->loc;
	if ((int)sub < size)
	{
		sub = self->loc + size;
		size_t capa = self->capa == 0 ? 16 : self->capa;
		while (capa < sub)
			capa <<= 1;
		byte* data = qn_get_gam_pointer(self);
		data = qn_realloc(data, capa, byte);
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
	switch ((int)org)
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
	const MemStream* self = qn_cast_type(g, MemStream);
	return (llong)self->loc;
}

//
static llong _mem_stream_size(QnGam g)
{
	const MemStream* self = qn_cast_type(g, MemStream);
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
	byte* data = qn_get_gam_pointer(self);
	qn_unload(self->base.mount);
	qn_free(data);
	qn_free(self->base.name);
	qn_free(self);
}

// 메모리 스트림 복제
static QnStream* _mem_stream_dup(QnGam g)
{
	// 윈도우든 유닉스든 파일 핸들은 복제되도 기능을 공유한다.
	// 그러므로 메모리 스트림도 똑같이 해도 된다 즉, 참조 처리면 될듯.
	return qn_load(g);
}

// 메모리 스트림 만들기 공용
static QnStream* _mem_stream_init(MemStream* self, QnMount* mount, const char* name, void* data, size_t capa, size_t size, QnFileFlag flags)
{
	self->base.mount = qn_load(mount);
	self->base.name = qn_strdup(name);
	self->base.flags = flags;
	self->capa = capa;
	self->size = size;
	self->loc = 0;
	qn_set_gam_desc(self, data);

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
	return qn_gam_init(self, _mem_stream_vt);
}

// 메모리 스트림 만들기
QnStream* qn_create_mem_stream(const char* name, size_t initial_capacity)
{
	MemStream* self = qn_alloc_1(MemStream);

	byte* data;
	if (initial_capacity == 0)
		data = NULL;
	else
	{
		data = qn_alloc(initial_capacity, byte);
		self->capa = initial_capacity;
	}

	return _mem_stream_init(self, NULL, name, data, initial_capacity, 0, QNFF_READ | QNFF_WRITE | QNFF_SEEK | QNFFT_MEM);
}

// 메모리 스트림 외부 데이터로 만들기
QnStream* qn_create_mem_stream_data(const char* name, void* data, size_t size)
{
	MemStream* self = qn_alloc_1(MemStream);
	return _mem_stream_init(self, NULL, name, data, size, size, QNFF_READ | QNFF_WRITE | QNFF_SEEK | QNFFT_MEM);
}

// HFS용 스트림 만들기
QnStream* _create_mem_stream_hfs(QnMount* mount, const char* name, void* data, size_t size)
{
	MemStream* self = qn_alloc_1(MemStream);
	return _mem_stream_init(self, mount, name, data, size, size, QNFF_READ | QNFF_SEEK | QNFFT_MEM | QNFFT_HFS);
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
static char* _file_stream_get_real_path(char* dest, const QnMount* mount, const char* filename)
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
		const size_t len = strlen(filename);
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
		const size_t len = strlen(filename);
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
static int _file_stream_read(QnGam g, void* buffer, const int offset, const int size)
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
static int _file_stream_write(QnGam g, const void* buffer, const int offset, const int size)
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

	nint fd = _file_handle_dup(qn_get_gam_desc(source, nint));
	if (fd == -1)
	{
		qn_free(self);
		return NULL;
	}

	self->base.mount = qn_load(source->base.mount);
	self->base.name = qn_strdup(source->base.name);
	self->base.flags = source->base.flags;
	qn_set_gam_desc(self, fd);

	return qn_gam_init(self, _file_stream_vt);
}

// 파일 스트림 열기
static QnStream* _file_stream_open(QnMount* mount, const char* filename, const char* mode)
{
	if (mount != NULL)
	{
		char path[QN_MAX_PATH];
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
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, filename, 0);
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
// 간접 스트림

//
typedef struct INDIRECTSTREAM
{
	QnStream			base;
	llong				offset;
	size_t				loc;
	size_t				size;
} IndirectStream;

//
static int _indirect_stream_read(QnGam g, void* buffer, const int offset, const int size)
{
	IndirectStream* self = qn_cast_type(g, IndirectStream);
	byte* ptr = (byte*)buffer + offset;
	int ret;

#ifdef _QN_WINDOWS_
	HANDLE fd = qn_get_gam_handle(self);
	const LARGE_INTEGER cli = { .QuadPart = self->offset + self->loc };
	SetFilePointerEx(fd, cli, NULL, FILE_BEGIN);

	DWORD dw;
	if (ReadFile(fd, ptr, size, &dw, NULL) == FALSE)
		return -1;
	ret = (int)dw;
#else
	int fd = qn_get_gam_desc_int(self);
	lseek(fd, (off_t)(self->offset + self->loc), SEEK_SET);

	ret = read(fd, ptr, (size_t)size);
	if (ret < 0)
		return -1;
#endif

	self->loc += ret;
	if (self->loc > self->size)
		self->loc = self->size;
	return ret;
}

//
static int _indirect_stream_write(QnGam g, const void* buffer, const int offset, const int size)
{
	QN_DUMMY(g); QN_DUMMY(buffer); QN_DUMMY(offset); QN_DUMMY(size);
	qn_mesgf(true, "IndirectStream", "write not supported");
	return -1;
}

//
static llong _indirect_stream_seek(QnGam g, const llong offset, const QnSeek org)
{
	IndirectStream* self = qn_cast_type(g, IndirectStream);
	llong loc;
	switch ((int)org)
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
static llong _indirect_stream_tell(QnGam g)
{
	IndirectStream* self = qn_cast_type(g, IndirectStream);
	return (llong)self->loc;
}

//
static llong _indirect_stream_size(QnGam g)
{
	IndirectStream* self = qn_cast_type(g, IndirectStream);
	return (llong)self->size;
}

//
static bool _indirect_stream_flush(QnGam g)
{
	QN_DUMMY(g);
	return true;
}

// 파일 스트림 닫기
static void _indirect_stream_dispose(QnGam g)
{
	IndirectStream* self = qn_cast_type(g, IndirectStream);
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
static QnStream* _indirect_stream_dup(QnGam g);

//
static const QN_DECL_VTABLE(QNSTREAM) _indirect_stream_vt =
{
	.base.name = "IndirectStream",
	.base.dispose = _indirect_stream_dispose,
	.stream_read = _indirect_stream_read,
	.stream_write = _indirect_stream_write,
	.stream_seek = _indirect_stream_seek,
	.stream_tell = _indirect_stream_tell,
	.stream_size = _indirect_stream_size,
	.stream_flush = _indirect_stream_flush,
	.stream_dup = _indirect_stream_dup,
};

// 간접 스트림 복제
static QnStream* _indirect_stream_dup(QnGam g)
{
	IndirectStream* source = qn_cast_type(g, IndirectStream);
	nint fd = _file_handle_dup(qn_get_gam_desc(source, nint));
	if (fd == -1)
		return NULL;

	IndirectStream* self = qn_alloc(1, IndirectStream);
	self->base.mount = qn_load(source->base.mount);
	self->base.name = qn_strdup(source->base.name);
	self->base.flags = source->base.flags;
	self->offset = source->offset;
	self->loc = 0;
	self->size = source->size;
	qn_set_gam_desc(self, fd);

	return qn_gam_init(self, _indirect_stream_vt);
}

// 미리 정의
struct HFS;

// 간접 스트림 열기
static QnStream* _indirect_stream_open(struct HFS* hfs, const char* filename, llong offset, size_t size)
{
	FileStream* stream = qn_get_gam_desc(hfs, FileStream*);
	nint fd = _file_handle_dup(qn_get_gam_desc(stream, nint));

	IndirectStream* self = qn_alloc(1, IndirectStream);
	self->base.mount = qn_load(hfs);
	self->base.name = qn_strdup(filename);
	self->base.flags = QNFF_READ | QNFF_SEEK | QNFFT_INDIRECT;
	self->offset = offset;
	self->loc = 0;
	self->size = size;
	qn_set_gam_desc(self, fd);

	return qn_gam_init(self, _indirect_stream_vt);
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
			const HANDLE fd = FindFirstFileEx(self->wname, FindExInfoStandard, &self->data, FindExSearchNameMatch, NULL, 0);
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
	DIR* dir = qn_get_gam_desc(self, DIR*);
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
static bool _disk_list_read_info(QnGam g, QnFileInfo* info)
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
			const HANDLE fd = FindFirstFileEx(self->wname, FindExInfoStandard, &self->data, FindExSearchNameMatch, NULL, 0);
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

		info->attr = _file_attr_convert(self->data.dwFileAttributes);
		info->len = (uint)qn_u16to8(self->path, QN_MAX_PATH, self->data.cFileName, 0);
		info->size = (llong)(self->data.nFileSizeLow | ((llong)self->data.nFileSizeHigh << 32));
		info->cmpr = 0;
		info->stc = _filetime_to_timestamp(&self->data.ftCreationTime);
		info->stw = _filetime_to_timestamp(&self->data.ftLastWriteTime);
		info->name = self->path;
		return true;
	}
#else
	DIR* dir = qn_get_gam_desc(self, DIR*);
	struct dirent* ent = readdir(dir);
	while (ent != NULL)
	{
		if (ent->d_name[0] != '.' || (ent->d_name[1] != '\0' && (ent->d_name[1] != '.' || ent->d_name[2] != '\0')))
			break;
		ent = readdir(dir);
	}
	if (ent == NULL)
		return false;

	qn_strconcat(2048, self->path, self->base.name, ent->d_name, NULL);
	struct stat st;
	if (stat(self->path, &st) < 0)
		return false;

	info->attr = _file_attr_convert(&st);
	info->len = (uint)strlen(ent->d_name);
	info->size = (llong)st.st_size;
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
	DIR* dir = qn_get_gam_desc(self, DIR*);
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
	DIR* dir = qn_get_gam_desc(self, DIR*);
	int cnt = 0;
	_disk_list_rewind(self);
	while (cnt < position && readdir(dir) != NULL)
		cnt++;
#else
	DIR* dir = qn_get_gam_desc(self, DIR*);
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
	DIR* dir = qn_get_gam_desc(self, DIR*);
	return (int)telldir(dir);
#endif
#endif
}

//
static void _disk_list_dispose(QnGam g)
{
	DiskDir* self = qn_cast_type(g, DiskDir);
#ifdef _QN_WINDOWS_
	const HANDLE fd = qn_get_gam_handle(self);
	if (fd != NULL && fd != INVALID_HANDLE_VALUE)
		FindClose(fd);
	qn_free(self->wname);
#else
	DIR* dir = qn_get_gam_desc(self, DIR*);
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
	const size_t len = qn_u8to16(NULL, 0, mount->path.DATA, mount->path.LENGTH) + 1/*슬래시*/ + 1/*서픽스*/ + 1/*널*/;
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

	DiskDir* self = qn_alloc_zero_1(DiskDir);
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
static QnStream* _disk_fs_open(QnGam g, const char* filename, const char* mode)
{
	return _file_stream_open(qn_cast_type(g, QnMount), filename, mode);
}

//
static void* _disk_fs_read(QnGam g, const char* filename, int* size)
{
	const QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, filename), NULL);
	char* data = _internal_file_alloc(real, size);
	return data;
}

//
static char* _disk_fs_read_text(QnGam g, const char* filename, int* length, int* codepage)
{
	const QnMount* self = qn_cast_type(g, QnMount);
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
	const QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, path), 0);
	return qn_get_file_attr(real);
}

//
static bool _disk_fs_remove(QnGam g, const char* path)
{
	const QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, path), false);

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, real, 0);
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
		qn_path_str_set_len(&self->path, self->name, self->name_len);
		return true;
	}

	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, directory), false);

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, real, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, false);

	wchar abspath[QN_MAX_PATH];
	if (_wfullpath(abspath, uni, QN_MAX_PATH - QN_MAX_PATH_BIAS) == NULL)
		return false;

	const DWORD attr = GetFileAttributes(abspath);
	qn_return_when_fail(attr != INVALID_FILE_ATTRIBUTES, false);
	qn_return_when_fail(QN_TMASK(attr, FILE_ATTRIBUTE_DIRECTORY), false);

	size_t len = wcslen(abspath);
	if (abspath[len - 1] != L'\\' && abspath[len - 1] != L'/')
	{
		abspath[len] = L'\\';
		abspath[len + 1] = L'\0';
	}

	qn_u16to8(real, QN_MAX_PATH, abspath, 0);
	qn_path_str_set(&self->path, real);
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

	qn_path_str_set_len(&self->path, abspath, len);
#endif

	return true;
}

//
static bool _disk_fs_mk_dir(QnGam g, const char* directory)
{
	const QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, directory), false);

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, real, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, false);

	wchar abspath[QN_MAX_PATH];
	if (_wfullpath(abspath, uni, QN_MAX_PATH - QN_MAX_PATH_BIAS) == NULL)
		return false;

	const DWORD attr = GetFileAttributes(abspath);
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

	const size_t len = strlen(path);
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

	qn_path_str_set_len(&self->path, self->name, self->name_len);
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

//
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

//
typedef struct HFSSOURCE
{
	byte				type;		// 타입
	byte 				attr;		// 속성
	ushort				len;		// 파일 이름 길이
	uint				size;		// 원래 크기
	uint				cmpr;		// 압축된 크기
	uint				seek;		// 파일 위치
} HfsSource;

//
typedef struct HFSFILE
{
	HfsSource			source;
	QnDateTime			stc;		// 만든 타임스탬프
	uint				hash;		// 파일 이름 해시
	uint				meta;		// 메타(디렉토리) 위치
	uint				next;		// 다음 파일 위치
} HfsFile;

//
typedef struct HFSINFO
{
	HfsFile				file;
	char				name[HFS_MAX_NAME];
} HfsInfo;
QN_DECL_ARRAY(HfsArrInfo, HfsInfo);
QN_IMPL_ARRAY(HfsArrInfo, HfsInfo, _hfs_infos);

//
typedef struct HFSDIR
{
	QnDir				base;
	HfsArrInfo			infos;
} HfsDir;

//
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
	qn_path_str_intern(dir);
	qn_path_str_intern(file);
}

// 파일 헤더
static bool _hfs_write_file_header(QnStream* st, HfsFile* info, const char* name, size_t name_len)
{
	info->source.len = (ushort)name_len;
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
		.source.type = QNFTYPE_SYSTEM,
		.source.attr = QNFATTR_DIR,
		.source.size = (uint)qm_rand(NULL),
		.source.cmpr = (uint)qm_rand(NULL),
		.source.seek = (uint)qm_rand(NULL),
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
	qn_path_str_append(dir, name);
	qn_path_str_append_char(dir, '/');
}

// 디렉토리 찾기
static bool _hfs_find_directory(HfsInfo* info, const char* name, uint hash, QnStream* st)
{
	const ushort len = (ushort)strlen(name);
	while (qn_stream_read(st, info, 0, sizeof(HfsFile)) == sizeof(HfsFile))
	{
		if (info->file.source.attr == QNFATTR_DIR && info->file.source.len == len && info->file.hash == hash)
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

// 디렉토리 변경
static bool _hfs_chdir(QnGam g, const char* directory)
{
	Hfs* self = qn_cast_type(g, Hfs);
	QnStream* stream = qn_get_gam_desc(self, QnStream*);
	if (qn_stricmp(directory, self->base.path.DATA) == 0)
		return true;

	if (_hfs_infos_is_have(&self->infos))
	{
		const HfsInfo* info = _hfs_infos_nth_ptr(&self->infos, 0);
		qn_stream_seek(stream, info->file.source.seek, QNSEEK_BEGIN);
	}

	QnPathStr tmp;
	qn_path_str_set(&tmp, directory);
	if (qn_path_str_nth(&tmp, 0) == '/')
	{
		qn_stream_seek(stream, HFSAT_ROOT, QNSEEK_BEGIN);
		qn_path_str_set_char(&self->base.path, '/');
	}

	HfsInfo info;
	char* stk = NULL;
	for (const char* tok = qn_strtok(tmp.DATA, "\\/\x0\n\r\t", &stk); tok; tok = qn_strtok(NULL, "\\/\x0\n\r\t", &stk))
	{
		const uint hash = qn_strshash(tok);
		if (_hfs_find_directory(&info, tok, hash, stream) == false)
			return false;
		qn_stream_seek(stream, info.file.meta, QNSEEK_BEGIN);
		_hfs_make_directory_name(&self->base.path, tok);
	}
#if HFS_DEBUG_TRACE
	qn_outputf("\tHFS: current directory is %s", self->base.path.DATA);
	qn_outputs("\t=type=|=attr=|=filename=====================================");
	int fc = 0;
#endif

	_hfs_infos_clear(&self->infos);
	for (uint srt = (uint)qn_stream_tell(stream); srt; srt = info.file.next)
	{
		if (qn_stream_read(stream, &info, 0, sizeof(HfsFile)) != sizeof(HfsFile) ||
			qn_stream_read(stream, info.name, 0, info.file.source.len) != info.file.source.len)
			return false;
		info.name[info.file.source.len] = '\0';

		qn_stream_seek(stream, info.file.next, QNSEEK_BEGIN);
		info.file.source.seek = srt;
		_hfs_infos_add(&self->infos, info);

#if HFS_DEBUG_TRACE
		qn_outputf("\t %04X | %04X | (%d/%d) %s", info.file.source.type, info.file.source.attr,
			info.file.source.seek, info.file.meta, info.name);
		fc++;
#endif
	}
#if HFS_DEBUG_TRACE
	qn_outputf("\tHFS: %d files", fc);
#endif

	return true;
}

//
static bool _hfs_save_dir(Hfs* self, const QnPathStr* dir, QnPathStr* save)
{
	if (qn_path_str_is_empty(dir) || qn_path_str_icmp_bstr(dir, &self->base.path) == 0)
	{
		if (save != NULL)
			qn_path_str_clear(save);
		return true;
	}
	if (save != NULL)
		qn_path_str_set_bstr(save, &self->base.path);
	return _hfs_chdir(self, dir->DATA);
}

//
static void _hfs_restore_dir(Hfs* self, const QnPathStr* save)
{
	if (save != NULL && qn_path_str_is_have(save))
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

	const uint hash = qn_path_str_shash(&name);
	size_t i;
	QN_CTNR_FOREACH(self->infos, i)
	{
		const HfsInfo* info = _hfs_infos_nth_ptr(&self->infos, i);
		if (hash == info->file.hash && qn_path_str_icmp(&name, info->name) == 0)
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
	const uint next = (uint)qn_stream_tell(stream);
	_hfs_write_directory(stream, name.DATA, name.LENGTH, 0, (uint)(next + sizeof(HfsFile) + name.LENGTH), 0);

	// 현재 디렉토리
	const uint curr = (uint)qn_stream_tell(stream);
	_hfs_write_directory(stream, ".", 1, (uint)(curr + sizeof(HfsFile) + 1), (uint)(next + sizeof(HfsFile) + name.LENGTH), 0);

	// 상위 디렉토리
	const HfsInfo* parent = _hfs_infos_nth_ptr(&self->infos, 0);
	_hfs_write_directory(stream, "..", 2, 0, parent->file.source.seek, parent->file.stc.stamp);

	// 지금꺼 갱신
	const HfsInfo* last = _hfs_infos_inv_ptr(&self->infos, 0);
	qn_stream_seek(stream, HFSAT_NEXT + last->file.source.seek, QNSEEK_BEGIN);
	qn_stream_write(stream, &next, 0, (int)sizeof(uint));

	//
	if (qn_path_str_is_empty(&save))
		qn_path_str_set_char(&save, '.');
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

	const uint hash = qn_path_str_shash(&name);
	const HfsInfo* found = NULL;
	size_t i;
	QN_CTNR_FOREACH(self->infos, i)
	{
		const HfsInfo* info = _hfs_infos_nth_ptr(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && qn_path_str_icmp(&name, info->name) == 0)
		{
			found = info;
			break;
		}
	}
	if (found == NULL)
	{
		_hfs_restore_dir(self, &save);
		return false;
	}

	const uint next = found->file.next;
	const HfsInfo* prev = _hfs_infos_nth_ptr(&self->infos, i - 1);
	if (qn_stream_seek(stream, HFSAT_NEXT + prev->file.source.seek, QNSEEK_BEGIN) <= 0 ||
		qn_stream_write(stream, &next, 0, sizeof(uint)) != sizeof(uint))
	{
		_hfs_restore_dir(self, &save);
		return false;
	}

	_hfs_infos_remove_nth(&self->infos, i);
	_hfs_restore_dir(self, &save);

	self->touch++;
	return true;
}

// 소스로 읽기
static void* _hfs_source_read(Hfs* hfs, HfsSource* source)
{
	QnStream* stream = qn_get_gam_desc(hfs, QnStream*);
	if (qn_stream_seek(stream, (llong)(source->seek + sizeof(HfsFile) + source->len), QNSEEK_BEGIN) < 0)
		return NULL;

	byte* data;
	if (QN_TMASK(source->attr, QNFATTR_CMPR))
	{
		// 압축
		byte* cmpr = qn_alloc(source->cmpr, byte);
		if (qn_stream_read(stream, cmpr, 0, (int)source->cmpr) != (int)source->cmpr)
		{
			qn_free(cmpr);
			return NULL;
		}
		data = qn_memzucp_s(cmpr, source->cmpr, source->size);
		qn_free(cmpr);
	}
	else
	{
		// 압축 아님
		data = qn_alloc(source->size + 4, byte);
		if (qn_stream_read(stream, data, 0, (int)source->size) != (int)source->size)
		{
			qn_free(data);
			data = NULL;
		}
	}
	return data;
}

// 파일 읽기
static void* _hfs_read(QnGam g, const char* filename, int* size)
{
	Hfs* self = qn_cast_type(g, Hfs);

	QnPathStr dir, name, keep, *save = QN_TMASK(self->base.flags, QNMFT_NORESTORE) ? NULL : &keep;
	_hfs_split_path(filename, &dir, &name);
	if (qn_path_str_is_empty(&name) || name.DATA[0] == '.')
		return NULL;
	if (_hfs_save_dir(self, &dir, save) == false)
		return NULL;

	const uint hash = qn_path_str_shash(&name);
	HfsInfo* found = NULL;
	size_t i;
	QN_CTNR_FOREACH(self->infos, i)
	{
		HfsInfo* info = _hfs_infos_nth_ptr(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && qn_path_str_icmp(&name, info->name) == 0)
		{
			found = info;
			break;
		}
	}
	if (found == NULL)
	{
		_hfs_restore_dir(self, save);
		return NULL;
	}

	void* data = _hfs_source_read(self, &found->file.source);
	if (size)
		*size = (int)found->file.source.size;

	_hfs_restore_dir(self, save);
	return data;
}

// 텍스트 읽기
static char* _hfs_read_text(QnGam g, const char* filename, int* length, int* codepage)
{
	int size;
	char* data = (char*)_hfs_read(g, filename, &size);
	_internal_detect_file_encoding(data, size, length, codepage);
	return data;
}

// 파일 열기
static QnStream* _hfs_open_stream(QnGam g, const char* filename, const char* mode)
{
	Hfs* self = qn_cast_type(g, Hfs);
	QnStream* hfs_stream = qn_get_gam_desc(self, QnStream*);
	bool indirect = QN_TMASK(hfs_stream->flags, QNFFT_FILE);

	if (mode)
	{
		for (const char* p = mode; *p; p++)
		{
			if (*p == 'w' || *p == 'a' || *p == '+')
			{
				qn_mesgf(true, "Hfs", "file write mode is not supported: %s (%s)", filename, mode);
				return NULL;
			}
			if (*p == 'm')
			{
				// m 모드는 메모리 스트림으로
				indirect = false;
			}
		}
	}

	QnPathStr dir, name, keep, *save = QN_TMASK(self->base.flags, QNMFT_NORESTORE) ? NULL : &keep;
	_hfs_split_path(filename, &dir, &name);
	if (qn_path_str_is_empty(&name) || name.DATA[0] == '.')
		return NULL;
	if (_hfs_save_dir(self, &dir, save) == false)
		return NULL;

	const uint hash = qn_path_str_shash(&name);
	HfsInfo* found = NULL;
	size_t i;
	QN_CTNR_FOREACH(self->infos, i)
	{
		HfsInfo* info = _hfs_infos_nth_ptr(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && qn_path_str_icmp(&name, info->name) == 0)
		{
			found = info;
			break;
		}
	}
	if (found == NULL)
	{
		_hfs_restore_dir(self, save);
		return NULL;
	}

	QnStream* stream;
	if (QN_TMASK(found->file.source.attr, QNFATTR_CMPR) || indirect == false)
	{
		void* data = _hfs_source_read(self, &found->file.source);
		stream = data == NULL ? NULL : _create_mem_stream_hfs(qn_cast_type(self, QnMount), filename, data, found->file.source.size);
	}
	else
	{
		llong offset = found->file.source.seek + sizeof(HfsFile) + found->file.source.len;
		stream = _indirect_stream_open(self, filename, offset, found->file.source.size);
	}

	_hfs_restore_dir(self, save);
	return stream;
}

// 파일 있나
static QnFileAttr _hfs_exist(QnGam g, const char* path)
{
	Hfs* self = qn_cast_type(g, Hfs);

	QnPathStr dir, name, keep, *save = QN_TMASK(self->base.flags, QNMFT_NORESTORE) ? NULL : &keep;
	_hfs_split_path(path, &dir, &name);
	if (_hfs_save_dir(self, &dir, save) == false)
		return 0;

	const uint hash = qn_path_str_shash(&name);
	QnFileAttr attr = QNFATTR_NONE;
	size_t i;
	QN_CTNR_FOREACH(self->infos, i)
	{
		const HfsInfo* info = _hfs_infos_nth_ptr(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && qn_path_str_icmp(&name, info->name) == 0)
		{
			attr = info->file.source.attr;
			break;
		}
	}

	_hfs_restore_dir(self, save);
	return attr;
}

//
static const HfsInfo* _hfs_list_internal_read_info(HfsDir* self)
{
	if (self->base.base.desc >= _hfs_infos_count(&self->infos))
		return NULL;
	return _hfs_infos_nth_ptr(&self->infos, self->base.base.desc++);
}

//
static const char* _hfs_list_read(QnGam g)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	const HfsInfo* hi = _hfs_list_internal_read_info(self);
	return hi == NULL ? NULL : hi->name;
}

//
static bool _hfs_list_read_info(QnGam g, QnFileInfo* info)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	const HfsInfo* hi = _hfs_list_internal_read_info(self);
	if (hi == NULL)
		return false;

	info->attr = hi->file.source.attr;
	info->len = hi->file.source.len;
	info->size = hi->file.source.size;
	info->cmpr = hi->file.source.cmpr;
	info->stc = hi->file.stc.stamp;
	info->stw = hi->file.stc.stamp;
	info->name = hi->name;
	return true;
}

//
static void _hfs_list_rewind(QnGam g)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	self->base.base.desc = 0;
}

//
static void _hfs_list_seek(QnGam g, int position)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	if ((size_t)position < _hfs_infos_count(&self->infos))
		self->base.base.desc = position;
}

//
static int _hfs_list_tell(QnGam g)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	return (int)self->base.base.desc;
}

//
static void _hfs_list_dispose(QnGam g)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	_hfs_infos_dispose(&self->infos);
	qn_unload(self->base.mount);
	qn_free(self->base.name);
	qn_free(self);
}

//
static QnDir* _hfs_list_open(_In_ QnMount* mount)
{
	Hfs* hfs = qn_cast_type(mount, Hfs);
	HfsDir* self = qn_alloc_zero_1(HfsDir);

	_hfs_infos_init_copy(&self->infos, &hfs->infos);

	self->base.mount = qn_load(mount);
	self->base.name = qn_strdup(mount->path.DATA);	// VS는 여기 널이라고 우기지만 mount는 널이 아니므로 무시
	qn_set_gam_desc(self, 0);

	static const QN_DECL_VTABLE(QNDIR) _hfs_list_vt =
	{
		.base.name = "HfsList",
		.base.dispose = _hfs_list_dispose,
		.dir_read = _hfs_list_read,
		.dir_read_info = _hfs_list_read_info,
		.dir_rewind = _hfs_list_rewind,
		.dir_seek = _hfs_list_seek,
		.dir_tell = _hfs_list_tell,
	};
	return qn_gam_init(self, _hfs_list_vt);
}

// HFS 만들기
static bool _hfs_create_file(_In_ QnStream* st, char* desc)
{
	QnDateTime dt = { qn_now() };
	HfsHeader header =
	{
		.header = HFS_HEADER,
		.version = HFS_VERSION,
		.notuse = 0,
		.stc = dt,
		.stw = dt,
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
	QnStream* stream;

	if (use_mem)
	{
		// 메모리에 만들던가 읽자
		if (is_create)
		{
			// 새로 만들자
			stream = qn_create_mem_stream(filename, 1024);
			qn_return_when_fail(stream != NULL, NULL);

			_hfs_create_file(stream, NULL);
			_mem_stream_seek(stream, 0, QNSEEK_BEGIN);
		}
		else
		{
			// 파일을 메모리로 올리자
			QnStream* filestream = _file_stream_open(NULL, filename, "rb");
			qn_return_when_fail(filestream != NULL, NULL);

			if (_file_stream_read(filestream, hdr, 0, sizeof(HfsHeader)) != sizeof(HfsHeader) ||
				hdr->header != HFS_HEADER || hdr->version != HFS_VERSION)
			{
				qn_unload(filestream);
				return NULL;
			}

			_file_stream_seek(filestream, 0, QNSEEK_BEGIN);
			const llong size = _file_stream_size(filestream);
			if (size < 0 || size >= (512LL * 1024LL * 1024LL))
			{
				// 최대 512MB 까지만
				qn_unload(filestream);
				return NULL;
			}

			stream = qn_create_mem_stream(filename, (size_t)size);
			if (stream == NULL)
			{
				qn_unload(filestream);
				return NULL;
			}

			byte* buffer = qn_alloc(2LL * 1024LL * 1024LL, byte);
			for (;;)
			{
				const int read = _file_stream_read(filestream, buffer, 0, 2LL * 1024LL * 1024LL);
				if (read <= 0)
					break;
				_mem_stream_write(stream, buffer, 0, read);
			}
			qn_free(buffer);

			_mem_stream_seek(stream, 0, QNSEEK_BEGIN);
			qn_unload(filestream);
		}
	}
	else
	{
		// 파일로 열자
		qn_return_when_fail(filename != NULL, NULL);
		if (is_create == false)
		{
			stream = _file_stream_open(NULL, filename, can_write ? "rb+@R" : "rb@R");
			qn_return_when_fail(stream != NULL, NULL);
		}
		else
		{
			stream = _file_stream_open(NULL, filename, "wb+@R");
			qn_return_when_fail(stream != NULL, NULL);
			_hfs_create_file(stream, NULL);
			_file_stream_flush(stream);
			_file_stream_seek(stream, 0, QNSEEK_BEGIN);
		}
	}

	if (qn_stream_read(stream, hdr, 0, sizeof(HfsHeader)) != sizeof(HfsHeader) ||
		hdr->header != HFS_HEADER || hdr->version != HFS_VERSION)
	{
		qn_unload(stream);
		return NULL;
	}

	return stream;
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

	_hfs_infos_dispose(&self->infos);
	qn_unload(stream);
	qn_free(self->base.name);
	qn_free(self);
}

// 진짜 만들기
static QnMount* _create_hfs(const char* filename, const char* mode)
{
	bool can_write = false, use_mem = false, is_create = false, no_restore = false;
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
			else if (*p == 'f')
				no_restore = true;
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
	if (no_restore)
		self->base.flags |= QNMFT_NORESTORE;
	qn_set_gam_desc(self, stream);
	_hfs_chdir(qn_cast_type(self, QnMount), "/");

	static const QN_DECL_VTABLE(QNMOUNT) _hfs_vt =
	{
		.base.name = "Hfs",
		.base.dispose = _hfs_dispose,
		.mount_open = _hfs_open_stream,
		.mount_read = _hfs_read,
		.mount_read_text = _hfs_read_text,
		.mount_exist = _hfs_exist,
		.mount_remove = _hfs_remove,
		.mount_chdir = _hfs_chdir,
		.mount_mkdir = _hfs_mkdir,
		.mount_list = _hfs_list_open,
	};
	return qn_gam_init(self, _hfs_vt);
}

#ifndef _QN_MOBILE_
// 설명 넣기
bool qn_hfs_set_desc(QnMount* mount, const char* desc)
{
	qn_return_when_fail((mount->flags & (QNMF_WRITE | QNMFT_HFS)) == (QNMF_WRITE | QNMFT_HFS), false);
	Hfs* self = qn_cast_type(mount, Hfs);
	qn_strncpy(self->header.desc, desc, QN_COUNTOF(self->header.desc) - 1);
	self->touch++;
	return true;
}

// 버퍼 넣기 메인
static bool _hfs_store_buffer(Hfs* self, const char* filename, const void* data, uint size, bool cmpr, QnFileType type)
{
	QnPathStr dir, name, save;
	_hfs_split_path(filename, &dir, &name);
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

	//
	const uint hash = qn_path_str_shash(&name);
	size_t i;
	QN_CTNR_FOREACH(self->infos, i)
	{
		const HfsInfo* info = _hfs_infos_nth_ptr(&self->infos, i);
		if (hash == info->file.hash && qn_path_str_icmp(&name, info->name) == 0)
		{
			_hfs_restore_dir(self, &save);
			errno = EEXIST;
			return false;
		}
	}

	//
	void* bufcmpr = NULL;
	size_t sizecmpr = 0;
	if (cmpr)
	{
		bufcmpr = qn_memzcpr(data, size, &sizecmpr);
		if (bufcmpr == NULL)
		{
			sizecmpr = 0;
			cmpr = false;
		}
		else
		{
#if defined _DEBUG && false
			byte* tmp = qn_memzucp_s(bufcmpr, sizecmpr, size);
			qn_free(tmp);
#endif

			// 압축 크기가 96% 이상이면 그냥 넣는다 (예컨데 압축파일)
			const double d = size * 0.96;
			if ((double)sizecmpr >= d)
			{
				qn_free(bufcmpr);
				sizecmpr = 0;
				cmpr = false;
			}
		}
	}

	//
	QnStream* stream = qn_get_gam_desc(self, QnStream*);
	qn_stream_seek(stream, 0, QNSEEK_END);
	const uint next = (uint)qn_stream_tell(stream);

	HfsInfo file =
	{
		.file.source.type = (byte)type,
		.file.source.attr = QNFATTR_FILE,
		.file.source.size = size,
		.file.source.cmpr = 0,
		.file.source.seek = 0,
		.file.stc.stamp = qn_now(),
		//.file.len = name.LENGTH,	// len, hash는 _hfs_write_file_header()에서 채워진다
		//.file.hash = hash,
		.file.meta = 0,
		.file.next = 0,
	};

	bool isok = false;
	if (cmpr)
	{
		file.file.source.attr |= QNFATTR_CMPR;
		file.file.source.cmpr = (uint)sizecmpr;
		if (_hfs_write_file_header(stream, &file.file, name.DATA, name.LENGTH) &&
			qn_stream_write(stream, bufcmpr, 0, (int)sizecmpr) == (int)sizecmpr)
			isok = true;
		qn_free(bufcmpr);
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
	HfsInfo* last = _hfs_infos_inv_ptr(&self->infos, 0);
	last->file.next = next;
	qn_stream_seek(stream, HFSAT_NEXT + last->file.source.seek, QNSEEK_BEGIN);
	qn_stream_write(stream, &next, 0, (int)sizeof(uint));

	file.file.source.seek = next;
	qn_strcpy(file.name, name.DATA);
	_hfs_infos_add(&self->infos, file);

	//
	_hfs_restore_dir(self, &save);
	self->touch++;
	return true;
}

// 버퍼 넣기
bool qn_hfs_store_data(QnMount* mount, const char* filename, const void* data, uint size, bool cmpr, QnFileType type)
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

	Hfs* self = qn_cast_type(mount, Hfs);
	return _hfs_store_buffer(self, filename, data, size, cmpr, type);
}

//
bool qn_hfs_store_stream(QnMount* mount, const char* filename, QnStream* stream, bool cmpr, QnFileType type)
{
	if ((mount->flags & (QNMF_WRITE | QNMFT_HFS)) != (QNMF_WRITE | QNMFT_HFS))
	{
		errno = EACCES;
		return false;
	}
	if (filename == NULL || *filename == '\0' || stream == NULL)
	{
		errno = EINVAL;
		return false;
	}

	const int size = (int)qn_stream_size(stream);
	if (size == 0 || (uint)size > (2040U * 1024U * 1024U))	// 1.99GB(2040MB) 이상은 안된다
	{
		errno = EINVAL;
		return false;
	}

	void *data = qn_alloc(size, byte);
	if (qn_stream_seek(stream, 0, QNSEEK_BEGIN) < 0 || qn_stream_read(stream, data, 0, size) != size)
	{
		qn_free(data);
		return false;
	}

	Hfs* self = qn_cast_type(mount, Hfs);
	const bool ret = _hfs_store_buffer(self, filename, data, size, cmpr, type);
	qn_free(data);
	return ret;
}

//
bool qn_hfs_store_file(QnMount* mount, const char* filename, const char* srcfile, bool cmpr, QnFileType type)
{
	if ((mount->flags & (QNMF_WRITE | QNMFT_HFS)) != (QNMF_WRITE | QNMFT_HFS))
	{
		errno = EACCES;
		return false;
	}
	if (srcfile == NULL || *srcfile == '\0')
	{
		errno = EINVAL;
		return false;
	}

	const QnFileAttr attr = qn_get_file_attr(srcfile);
	if (attr == QNFATTR_NONE)
	{
		errno = ENOENT;
		return false;
	}
	if (QN_TMASK(attr, QNFATTR_DIR))
	{
		errno = EISDIR;
		return false;
	}

	QnStream* stream = _file_stream_open(NULL, srcfile, "rb");
	if (stream == NULL)
		return false;

	const int size = (int)qn_stream_size(stream);
	if (size == 0 || (uint)size > (2040U * 1024U * 1024U))	// 1.99GB(2040MB) 이상은 안된다
	{
		qn_unload(stream);
		errno = EINVAL;
		return false;
	}

	void *data = qn_alloc(size, byte);
	if (qn_stream_read(stream, data, 0, size) != size)
	{
		qn_free(data);
		qn_unload(stream);
		return false;
	}

	if (filename == NULL)
		filename = srcfile;

	Hfs* self = qn_cast_type(mount, Hfs);
	const bool ret = _hfs_store_buffer(self, filename, data, size, cmpr, type);
	qn_free(data);
	qn_unload(stream);
	return ret;
}

// 최적화용 파일 읽기
static void* _hfs_optimize_read(Hfs* hfs, HfsSource* source, uint* ret_size)
{
	QnStream* stream = qn_get_gam_desc(hfs, QnStream*);
	if (qn_stream_seek(stream, (llong)(source->seek + sizeof(HfsFile) + source->len), QNSEEK_BEGIN) < 0)
		return NULL;

	uint size = QN_TMASK(source->attr, QNFATTR_CMPR) ? source->cmpr : source->size;
	byte* data = qn_alloc(size, byte);
	if (qn_stream_read(stream, data, 0, (int)size) != (int)size)
	{
		qn_free(data);
		return NULL;
	}
	*ret_size = size;
	return data;
}

// 최적화 진행
static bool _hfs_optimize_process(HfsOptimizeData* od)
{
	Hfs* input = qn_cast_type(od->input, Hfs);
	Hfs* output = qn_cast_type(od->output, Hfs);
	HfsOptimizeParam* param = od->param;

	HfsArrInfo infos;
	_hfs_infos_init_copy(&infos, &input->infos);

	if (param->callback)
	{
		char path[QN_MAX_PATH];
		qn_strncpy(path, input->base.path.DATA, input->base.path.LENGTH - 1);
		qn_divpath(path, NULL, od->name);
		od->size = 0;
		od->count++;
		param->callback(param->userdata, od);
	}

	size_t i;
	QN_CTNR_FOREACH(infos, i)
	{
		HfsInfo* info = _hfs_infos_nth_ptr(&infos, i);
		if (info->name[0] == '.')
			continue;
		if (QN_TMASK(info->file.source.attr, QNFATTR_DIR))
		{
			_hfs_mkdir(output, info->name);
			_hfs_chdir(output, info->name);
			_hfs_chdir(input, info->name);

			od->stack++;
			if (_hfs_optimize_process(od) == false)
			{
				_hfs_infos_dispose(&infos);
				return false;
			}

			od->stack--;
			_hfs_chdir(output, "..");
			_hfs_chdir(input, "..");
		}
		else
		{
			if (param->callback)
			{
				qn_strcpy(od->name, info->name);
				od->size = info->file.source.size;
				od->count++;
				param->callback(param->userdata, od);
			}

			uint size;
			void* data = _hfs_optimize_read(input, &info->file.source, &size);
			if (data == NULL)
			{
				_hfs_infos_dispose(&infos);
				return false;
			}

			QnStream* stream = qn_get_gam_desc(output, QnStream*);
			qn_stream_seek(stream, 0, QNSEEK_END);
			const uint next = (uint)qn_stream_tell(stream);

			HfsInfo file;
			memcpy(&file.file, &info->file, sizeof(HfsFile));

			if (_hfs_write_file_header(stream, &file.file, info->name, info->file.source.len) == false ||
				qn_stream_write(stream, data, 0, size) != (int)size)
			{
				qn_free(data);
				_hfs_infos_dispose(&infos);
				return false;
			}
			qn_free(data);

			HfsInfo* last = _hfs_infos_inv_ptr(&output->infos, 0);
			last->file.next = next;
			qn_stream_seek(stream, HFSAT_NEXT + last->file.source.seek, QNSEEK_BEGIN);
			qn_stream_write(stream, &next, 0, (int)sizeof(uint));

			file.file.source.seek = next;
			qn_strcpy(file.name, info->name);
			_hfs_infos_add(&output->infos, file);
		}
	}

	_hfs_infos_dispose(&infos);
	return true;
}

//
bool qn_hfs_optimize(QnMount* mount, HfsOptimizeParam* param)
{
	qn_return_when_fail(QN_TMASK(mount->flags, QNMFT_HFS), false);
	qn_return_when_fail(param != NULL, false);

	Hfs* outhfs = qn_cast_type(_create_hfs(param->filename, "c"), Hfs);
	qn_return_when_fail(outhfs != NULL, false);

	if (param->desc[0] != '\0')
		qn_hfs_set_desc(qn_cast_type(outhfs, QnMount), param->desc);

	HfsOptimizeData od =
	{
		.stack = 1,
		.input = mount,
		.output = outhfs,
		.param = param,
	};

	Hfs* self = qn_cast_type(mount, Hfs);
	QnPathStr dir, save;
	qn_path_str_set_char(&dir, '/');
	_hfs_save_dir(self, &dir, &save);

	bool ret = _hfs_optimize_process(&od);

	_hfs_restore_dir(self, &save);

	qn_unload(outhfs);
	return ret;
}
#endif


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
#if false
		tmppath = qn_basepath();
#else
		tmppath = qn_getcwd();
#endif
		return _create_diskfs(tmppath);
	}

	const QnFileAttr attr = qn_get_file_attr(path);
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
		const BOOL b = CreateDirectory(pwz, NULL);
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
