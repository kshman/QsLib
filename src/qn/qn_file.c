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

#ifdef _DEBUG
//#define HFS_DEBUG_TRACE
#endif

QN_IMPL_BSTR(QnPathStr, QN_MAX_PATH, _path_str);

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

// 파일 속성 변환
#ifdef _QN_WINDOWS_
static QnFileAttr _file_attr_convert(const DWORD attr)
{
	int ret = QN_TMASK(attr, FILE_ATTRIBUTE_DIRECTORY) ? QNFATTR_DIR : QNFATTR_FILE;
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
	int ret = S_ISDIR(st->st_mode) ? QNFATTR_DIR : QNFATTR_FILE;
	if (S_ISLNK(st->st_mode))
		ret |= QNFATTR_LINK;
	return ret;
}
#endif

// 파일 속성 얻기
static QnFileAttr _internal_file_attr(const char* path)
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
static bool _internal_remove_file(const char* path)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, path, 0);
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
	if (stat(path, &st) == 0)
	{
		if (S_ISDIR(st.st_mode))
		{
			if (rmdir(path) == 0)
				ret = true;
		}
		else
		{
			if (unlink(path) == 0)
				ret = true;
		}
	}
#endif
	return ret;
}
//
static char* _internal_getcwd(void)
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
	while (true)
	{
		buf = qn_realloc(buf, len, char);
		char* res = getcwd(buf, len);
		if (res != NULL)
			return buf;
		if (errno != ERANGE)
			break;
		len *= 2;
	}
	qn_free(buf);
	return NULL;
#endif
}

// 읽기 위해 파일 열기
static void* _internal_file_open(const char* filename, size_t* size)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, filename, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, NULL);

	HANDLE fd = CreateFile(uni, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd == NULL || fd == INVALID_HANDLE_VALUE)
		return NULL;

	LARGE_INTEGER li;
	if (GetFileSizeEx(fd, &li) == FALSE || li.QuadPart <= 0)
	{
		CloseHandle(fd);
		return NULL;
	}

	*size = (size_t)li.QuadPart;
	return (void*)fd;
#else
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		return NULL;
	struct stat st;
	if (fstat(fd, &st) < 0 || st.st_size <= 0)
	{
		close(fd);
		return NULL;
	}
	*size = (size_t)st.st_size;
	return (void*)(size_t)fd;
#endif
}

// 연 파일 닫기
static void _internal_file_close(void* fd)
{
#ifdef _QN_WINDOWS_
	CloseHandle((HANDLE)fd);
#else
	close((int)(size_t)fd);
#endif
}

// 파일 읽기
static bool _internal_file_read(void* fd, void* buffer, size_t size)
{
#ifdef _QN_WINDOWS_
	DWORD dw;
	if (ReadFile((HANDLE)fd, buffer, (DWORD)size, &dw, NULL) == false || size != (size_t)dw)
		return false;
	return true;
#else
	ssize_t n = read((int)(size_t)fd, buffer, size);
	if (n < 0 || (size_t)n != size)
		return false;
	return true;
#endif
}

//
static bool _internal_chdir(const char* directory)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, directory, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, false);
	return SetCurrentDirectory(uni) != 0;
#else
	return chdir(directory) == 0;
#endif
}

//
static bool _internal_mkdir(const char* directory)
{
#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	const size_t uni_len = qn_u8to16(uni, QN_MAX_PATH, directory, 0);
	qn_return_when_fail(uni_len < QN_MAX_PATH - QN_MAX_PATH_BIAS, false);
	wchar abspath[QN_MAX_PATH];
	if (_wfullpath(abspath, uni, QN_MAX_PATH - QN_MAX_PATH_BIAS) == NULL)
		return false;
	const DWORD attr = GetFileAttributes(abspath);
	qn_return_when_fail(attr != INVALID_FILE_ATTRIBUTES, false);
	return CreateDirectory(abspath, NULL) != 0;
#else
	struct stat st;
	qn_return_when_fail(stat(directory, &st) != 0, false);
	return mkdir(directory, 0755) == 0;
#endif
}


/////////////////////////////////////////////////////////////////////////
// 스트림 + 마운트 연계 함수

//
QnFileAttr qn_get_file_attr(QnMount* mount, const char* filename)
{
	qn_return_when_fail(filename != NULL, false);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_attr(mount, filename);
	return _internal_file_attr(filename);
}

//
void* qn_file_alloc(QnMount* mount, const char* filename, int* size)
{
	qn_return_when_fail(filename != NULL, NULL);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_alloc(mount, filename, size);

	size_t len;
	void* fd = _internal_file_open(filename, &len);
	if (fd == NULL)
		return NULL;

	byte* buffer = qn_alloc(len + 4, byte);
	if (_internal_file_read(fd, buffer, len) == false)
	{
		qn_free(buffer);
		buffer = NULL;
	}

	_internal_file_close(fd);
	if (size != NULL)
		*size = (int)len;
	return buffer;
}

//
char* qn_file_alloc_text(QnMount* mount, const char* filename, int* length, int* codepage)
{
	int size;
	char* data = qn_file_alloc(mount, filename, &size);
	qn_return_when_fail(data, NULL);

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

	return data;
}

//
bool qn_remove_file(QnMount* mount, const char* path)
{
	qn_return_when_fail(path, false);
	if (mount)
	{
		qn_return_when_fail(QN_TMASK(mount->flags, QNMF_WRITE), false);
		return qn_cast_vtable(mount, QNMOUNT)->mount_remove(mount, path);
	}
	return _internal_remove_file(path);
}

//
char* qn_getcwd(QnMount* mount)
{
	if (mount)
		return qn_cast_vtable(mount, QNMOUNT)->mount_cwd(mount);
	return _internal_getcwd();
}

//
bool qn_chdir(QnMount* mount, const char* directory)
{
	qn_return_when_fail(directory, false);
	if (mount)
		return qn_cast_vtable(mount, QNMOUNT)->mount_chdir(mount, directory);
	return _internal_chdir(directory);
}

//
bool qn_mkdir(QnMount* mount, const char* directory)
{
	qn_return_when_fail(directory, false);
	if (mount)
	{
		qn_return_when_fail(QN_TMASK(mount->flags, QNMF_WRITE), false);
		return qn_cast_vtable(mount, QNMOUNT)->mount_mkdir(mount, directory);
	}
	return _internal_mkdir(directory);
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

	static const struct QNSTREAM_VTABLE _mem_stream_vt =
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
	bool at = false;

#ifdef _QN_WINDOWS_
	acs->mode = OPEN_EXISTING;
	acs->access = GENERIC_READ;
	acs->attr = FILE_ATTRIBUTE_NORMAL;
	acs->share = FILE_SHARE_READ;
	*flags = QNFF_READ | QNFF_SEEK;

	if (mode)
	{
		for (; *mode; ++mode)
		{
			if (at == false)
			{
				switch (*mode)
				{
					case 'a':
						acs->mode = OPEN_ALWAYS;
						acs->access = GENERIC_WRITE;
						*flags = QNFF_WRITE | QNFF_SEEK | QNFF_APPEND;
						break;
#if false
					case 'r':
						acs->mode = OPEN_EXISTING;
						acs->access = GENERIC_READ;
						*flags = QNFF_READ | QNFF_SEEK;
						break;
#endif
					case 'w':
						acs->mode = CREATE_ALWAYS;
						acs->access = GENERIC_WRITE;
						*flags = QNFF_WRITE | QNFF_SEEK;
						break;
					case '+':
						acs->access = GENERIC_WRITE | GENERIC_READ;
						*flags = QNFF_READ | QNFF_WRITE | QNFF_SEEK;
						break;
					case '!':
						if (acs->mode == CREATE_ALWAYS)
							acs->attr = FILE_FLAG_DELETE_ON_CLOSE;
						break;
					case 't':
						*flags |= QNFF_TEXT;
						break;
					case 'b':
						*flags |= QNFF_BINARY;
						break;
					case '@':
						at = true;
						break;
					default:
						break;
				}
			}
			else
			{
				switch (*mode)
				{
#if false
					case 'R':
						acs->share = FILE_SHARE_READ;
						break;
#endif
					case 'W':
						acs->share = FILE_SHARE_WRITE;
						break;
					case '+':
						acs->share = FILE_SHARE_READ | FILE_SHARE_WRITE;
						break;
					default:
						break;
				}
			}
		}
	}
#else
	acs->mode = O_RDONLY;
	acs->access = 0;
	*flags = QNFF_READ | QNFF_SEEK;

	if (mode)
	{
		for (; *mode; ++mode)
		{
			if (at == false)
			{
				switch (*mode)
				{
					case 'a':
						acs->mode = O_WRONLY | O_APPEND | O_CREAT;
						*flags = QNFF_WRITE | QNFF_SEEK | QNFF_APPEND;
						break;
#if false
					case 'r':
						acs->mode = O_RDONLY;
						*flags = QNFF_READ | QNFF_SEEK;
						break;
#endif
					case 'w':
						acs->mode = O_WRONLY | O_TRUNC | O_CREAT;
						*flags = QNFF_WRITE | QNFF_SEEK;
						break;
					case '+':
						acs->mode |= O_RDWR;
						*flags = QNFF_READ | QNFF_WRITE | QNFF_SEEK;
						break;
					case 't':
						*flags |= QNFF_TEXT;
						break;
					case 'b':
						*flags |= QNFF_BINARY;
						break;
					case '@':
						at = true;
						break;
					case '!':
					default:
						break;
				}
			}
			else
			{
				switch (*mode)
				{
					case 'R':
						acs->access |= S_IRUSR;
						break;
					case 'W':
						acs->access |= S_IWUSR;
						break;
					case 'X':
						acs->access |= S_IXUSR;
						break;
					case '+':
						acs->access |= S_IRUSR | S_IWUSR;
						break;
					default:
						if (*mode >= '0' && *mode <= '7')
						{
							const char* p = mode + 1;
							while (*mode >= '0' && *mode <= '7')
								p++;
							if ((p - mode) < 8)
							{
								char sz[10];
								qn_strncpy(sz, mode, (size_t)(p - mode));
								acs->access = qn_strtoi(sz, 8);
							}
							mode = p;
						}
						break;
				}
			}
		}
	}

	if (acs->access == 0)
		acs->access = QN_TMASK(acs->mode, O_CREAT) ? (S_IRUSR | S_IWUSR) | S_IRGRP | S_IROTH : S_IRUSR | S_IRGRP | S_IROTH;
#endif
}

// 실제 파일 경로 얻기
static char* _file_stream_get_real_path(char* dest, const QnMount* mount, const char* filename)
{
#ifdef _QN_WINDOWS_
	if (filename[0] != '\0' && filename[1] == ':')
	{
		qn_debug_mesg("FileStream", "cannot use drive letter");
		return NULL;
	}
#endif
	if (filename[0] == '/' || filename[0] == '\\')
	{
#ifdef _DEBUG
		const size_t len = strlen(filename);
		if (mount->name_len + len >= QN_MAX_PATH)
		{
			qn_mesgb("FileStream", "filename too long");
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
			qn_mesgb("FileStream", "filename too long");
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
static const struct QNSTREAM_VTABLE _file_stream_vt =
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
	{
#if defined _DEBUG && false
		WCHAR wz[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, wz, QN_COUNTOF(wz), NULL);
		char sz[256];
		qn_u16to8(sz, 256, wz, 0);
		qn_mesgf("FileStream", "CreateFile failed: %s", sz);
#endif
		return NULL;
	}
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
	qn_mesgb("IndirectStream", "write not supported");
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
static const struct QNSTREAM_VTABLE _indirect_stream_vt =
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

// 간접 스트림 열기
static QnStream* _indirect_stream_open(QnMount* hfs, const char* filename, llong offset, size_t size)
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
#else
	char*				mask;
#endif
	int					stat;
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
		if (self->stat == 0)
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

		qn_u16to8(self->path, QN_MAX_PATH, self->data.cFileName, 0);
		return self->path;
	}
#else
	DIR* dir = qn_get_gam_desc(self, DIR*);
	struct dirent* ent;
	if (self->mask == NULL)
		ent = readdir(dir);
	else
	{
		for (;;)
		{
			ent = readdir(dir);
			if (ent == NULL)
				break;
			if (qn_strwcm(ent->d_name, self->mask))
				break;
		}
	}
	if (ent == NULL)
		return NULL;
	self->stat++;
	return ent->d_name;
#endif
}

//
extern QnTimeStamp _time_to_timestamp(const time_t tt, uint ms);

#ifdef _QN_WINDOWS_
//
static QnTimeStamp _filetime_to_timestamp(const FILETIME* ft)
{
	ULARGE_INTEGER ul = { .LowPart = ft->dwLowDateTime, .HighPart = ft->dwHighDateTime };
	ul.QuadPart -= 116444736000000000ULL;
	struct timespec ts =
	{
		.tv_sec = (time_t)((ul.QuadPart) / 10000000),
		.tv_nsec = (long)((ul.QuadPart % 10000000) * 100),
	};
	return _time_to_timestamp(ts.tv_sec, ts.tv_nsec / 1000000);
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
	struct dirent* ent;
	if (self->mask == NULL)
		ent = readdir(dir);
	else
	{
		for (;;)
		{
			ent = readdir(dir);
			if (ent == NULL)
				break;
			if (qn_strwcm(ent->d_name, self->mask))
				break;
		}
	}
	if (ent == NULL)
		return false;
	self->stat++;

	qn_strconcat(2048, self->path, self->base.name, ent->d_name, NULL);
	struct stat st;
	if (stat(self->path, &st) < 0)
		return false;

	info->attr = _file_attr_convert(&st);
	info->len = (uint)strlen(ent->d_name);
	info->size = (llong)st.st_size;
	info->cmpr = 0;
	info->stc = _time_to_timestamp(st.st_ctime, 0);
	info->stw = _time_to_timestamp(st.st_mtime, 0);
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
#else
	DIR* dir = qn_get_gam_desc(self, DIR*);
	if (dir != NULL)
		rewinddir(dir);
#endif
	self->stat = 0;
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
	qn_free(self->mask);
#endif
	qn_unload(self->base.mount);
	qn_free(self->base.name);
	qn_free(self);
}

//
static QnDir* _disk_list_open(_In_ QnMount* mount, const char* directory, const char* mask)
{
#ifdef _QN_WINDOWS_
	size_t dlen = strlen(directory);
	if (dlen >= QN_MAX_PATH)
		return NULL;

	wchar* pw = qn_u8to16_dup(directory, 0);
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0, };
	bool b = GetFileAttributesEx(pw, GetFileExInfoStandard, &fad) && QN_TMASK(fad.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
	qn_free(pw);
	if (b == false)
		return NULL;

	if (mask == NULL)
		mask = "*";
	char* path;
	if (directory[dlen - 1] == '/' || directory[dlen - 1] == '\\')
		path = qn_strdupcat(directory, mask, NULL);
	else
		path = qn_strdupcat(directory, "\\", mask, NULL);
	pw = qn_u8to16_dup(path, 0);

	DiskDir* self = qn_alloc_zero_1(DiskDir);
	self->base.mount = qn_load(mount);
	self->base.name = path;
	self->wname = pw;
	qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
#else
	DIR* dir = opendir(directory);
	qn_return_when_fail(dir != NULL, NULL);

	DiskDir* self = qn_alloc_zero_1(DiskDir);
	self->base.mount = qn_load(mount);
	self->base.name = qn_strdup(directory);
	if (mask != NULL)
		self->mask = qn_strdup(mask);
	qn_set_gam_desc(self, dir);
#endif

	static const struct QNDIR_VTABLE _disk_list_vt =
	{
		.base.name = "DiskList",
		.base.dispose = _disk_list_dispose,
		.dir_read = _disk_list_read,
		.dir_read_info = _disk_list_read_info,
		.dir_rewind = _disk_list_rewind,
	};
	return qn_gam_init(self, _disk_list_vt);
}


//////////////////////////////////////////////////////////////////////////
// 마운트 공용

//
static char* _mount_cwd(QnGam g)
{
	const QnMount* self = qn_cast_type(g, QnMount);
	return qn_strdup(self->path.DATA);
}


//////////////////////////////////////////////////////////////////////////
// 디스크 파일 시스템 마운트

//
static bool _disk_fs_chdir(QnGam g, const char* directory)
{
	QnMount* self = qn_cast_type(g, QnMount);

	if (directory == NULL || directory[0] == '/' || directory[0] == '\\' && directory[1] == '\0')
	{
		_path_str_set_len(&self->path, self->name, self->name_len);
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
	_path_str_set(&self->path, real);
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

	_path_str_set_len(&self->path, abspath, len);
#endif

	return true;
}

//
static bool _disk_fs_mkdir(QnGam g, const char* directory)
{
	const QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, directory), false);
	return _internal_mkdir(real);
}

//
static bool _disk_fs_remove(QnGam g, const char* path)
{
	const QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, path), false);
	return _internal_remove_file(real);
}

//
static QnFileAttr _disk_fs_attr(QnGam g, const char* path)
{
	const QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, path), 0);
	return _internal_file_attr(real);
}

//
static QnStream* _disk_fs_stream(QnGam g, const char* filename, const char* mode)
{
	return _file_stream_open(qn_cast_type(g, QnMount), filename, mode);
}

//
static void* _disk_fs_alloc(QnGam g, const char* filename, int* size)
{
	const QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, filename), NULL);

	size_t len;
	void* fd = _internal_file_open(real, &len);
	qn_return_when_fail(fd != NULL, NULL);

	byte* buffer = qn_alloc(len + 4, byte);
	if (_internal_file_read(fd, buffer, len) == false)
	{
		qn_free(buffer);
		buffer = NULL;
	}

	_internal_file_close(fd);
	if (size != NULL)
		*size = (int)len;
	return buffer;
}

//
static QnDir* _disk_fs_list(QnGam g, const char* directory, const char* mask)
{
	QnMount* self = qn_cast_type(g, QnMount);
	char real[QN_MAX_PATH];
	qn_return_when_fail(_file_stream_get_real_path(real, self, directory), NULL);
	return _disk_list_open(self, real, mask);
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

	_path_str_set_len(&self->path, self->name, self->name_len);
#ifdef _QN_EMSCRIPTEN_
	self->flags = QNMF_READ | QNMFT_DISKFS;
#else
	self->flags = QNMF_READ | QNMF_WRITE | QNMFT_DISKFS;
#endif

	//
	static const struct QNMOUNT_VTABLE _disk_fs_vt =
	{
		.base.name = "DiskFS",
		.base.dispose = _disk_fs_dispose,
		.mount_cwd = _mount_cwd,
		.mount_chdir = _disk_fs_chdir,
		.mount_mkdir = _disk_fs_mkdir,
		.mount_remove = _disk_fs_remove,
		.mount_attr = _disk_fs_attr,
		.mount_stream = _disk_fs_stream,
		.mount_list = _disk_fs_list,
		.mount_alloc = _disk_fs_alloc,
	};
	return qn_gam_init(self, _disk_fs_vt);
	}


//////////////////////////////////////////////////////////////////////////
// HFS

#define HFS_HEADER		QN_FOURCC('H', 'F', 'S', '\0')
#define HFS_VERSION		14

#define HFSAT_ROOT		sizeof(HfsHeader)
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
#define HFSAT_SUBP		QN_OFFSETOF(HfsFile, subp)
#define HFSAT_NEXT		QN_OFFSETOF(HfsFile, next)
#define HFSAT_STZ		QN_OFFSETOF(HfsFile, stc)

#define HFS_MAX_NAME	260

// HFS 헤더
typedef struct HFSHEADER
{
	uint				header;				// 헤더
	ushort				version;			// 버전
	ushort				notuse;				// 사용하지 않음
	QnDateTime			stc;				// 만든 타임스탬프
	QnDateTime			stw;				// 수정 타임스탬프
	uint				revision;			// 리비전
	char				desc[64];			// 설명
} HfsHeader;

// HFS 파일 소스
typedef struct HFSSOURCE
{
	byte 				attr;				// 속성
	byte				type;				// 타입
	ushort				len;				// 파일 이름 길이
	uint				size;				// 원래 크기
	uint				cmpr;				// 압축된 크기
	uint				seek;				// 파일 위치
} HfsSource;

// HFS 파일
typedef struct HFSFILE
{
	HfsSource			source;
	QnDateTime			stc;				// 만든 타임스탬프
	uint				hash;				// 파일 이름 해시
	uint				zzzz;				// 메타 데이터
	uint				subp;				// 디렉토리 위치
	uint				next;				// 다음 파일 위치
} HfsFile;

// HFS 파일 정보
typedef struct HFSINFO
{
	HfsFile				file;
	char				name[HFS_MAX_NAME];
} HfsInfo;
QN_DECLIMPL_ARRAY(HfsInfoArray, HfsInfo, _hfs_infos);

//
typedef struct HFSDIR
{
	QnDir				base;
	HfsInfoArray		infos;
} HfsDir;

//
typedef struct HFS
{
	QnMount				base;
	HfsHeader			header;
	HfsInfoArray		infos;
	uint				touch;
} Hfs;

// 경로 분리
static void _hfs_split_path(const char* path, QnPathStr* dir, QnPathStr* file)
{
	qn_divpath(path, dir->DATA, file->DATA);
	_path_str_intern(dir);
	_path_str_intern(file);
}

// 파일 헤더
static bool _hfs_write_file_header(QnStream* stream, HfsFile* file, const char* name, size_t name_len, uint hash)
{
	file->hash = hash == 0 ? qn_strshash(name) : hash;
	file->source.len = (ushort)(name_len == 0 ? strlen(name) : name_len);
	if (qn_stream_write(stream, file, 0, sizeof(HfsFile)) != sizeof(HfsFile) ||
		qn_stream_write(stream, name, 0, (int)name_len) != (int)name_len)
		return false;
	return true;
}

// 디렉토리
//static ushort _hfs_write_directory(QnStream* st, const char* name, size_t name_len, uint next, uint meta, QnTimeStamp stc)
static ushort _hfs_write_directory(QnStream* stream, const char* name, size_t name_len, uint hash, QnTimeStamp stc, uint subp, uint next)
{
	if (stc == 0)
		stc = qn_now();
	HfsFile info =
	{
		.source.attr = QNFATTR_DIR,
		.source.type = QNFTYPE_DIR,
		.source.size = (uint)qn_rand(NULL),
		.source.cmpr = (uint)qn_rand(NULL),
		.source.seek = (uint)qn_rand(NULL),
		.stc.stamp = stc,
		.subp = subp,
		.next = next,
	};
	return _hfs_write_file_header(stream, &info, name, name_len, hash);
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
	_path_str_add(dir, name);
	_path_str_add_char(dir, '/');
}

// 디렉토리 찾기
static bool _hfs_find_directory(QnStream* stream, HfsInfo* info, const char* name, size_t name_len, uint hash)
{
	while (qn_stream_read(stream, info, 0, sizeof(HfsFile)) == sizeof(HfsFile))
	{
		if (info->file.source.attr == QNFATTR_DIR && info->file.hash == hash && info->file.source.len == name_len)
		{
			if (qn_stream_read(stream, info->name, 0, (int)name_len) == (int)name_len && qn_strncmp(info->name, name, name_len) == 0)
				return true;
		}
		if (info->file.next == 0)
			break;
		if (qn_stream_seek(stream, info->file.next, QNSEEK_BEGIN) != info->file.next)
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
		const HfsInfo* info = _hfs_infos_ptr_nth(&self->infos, 0);
		qn_stream_seek(stream, info->file.source.seek, QNSEEK_BEGIN);
	}

	QnPathStr tmp;
	_path_str_set(&tmp, directory);
	if (_path_str_nth(&tmp, 0) == '/')
	{
		qn_stream_seek(stream, HFSAT_ROOT, QNSEEK_BEGIN);
		_path_str_set_char(&self->base.path, '/');
	}

	HfsInfo info;
	char* stk = NULL;
	for (const char* tok = qn_strtok(tmp.DATA, "\\/\x0\n\r\t", &stk); tok; tok = qn_strtok(NULL, "\\/\x0\n\r\t", &stk))
	{
		const uint hash = qn_strshash(tok);
		if (_hfs_find_directory(stream, &info, tok, strlen(tok), hash) == false)
			return false;
		qn_stream_seek(stream, info.file.subp, QNSEEK_BEGIN);
		_hfs_make_directory_name(&self->base.path, tok);
	}
#ifdef HFS_DEBUG_TRACE
	qn_outputf("\tHFS: current directory is %s", self->base.path.DATA);
	qn_outputs("\t\t=attr=|=type=|=filename=====================================");
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

#ifdef HFS_DEBUG_TRACE
		if (info.file.source.type == QNFTYPE_DIR)
			qn_outputf("\t\t %04X | %4d | [%s] <%d>", info.file.source.attr, info.file.source.type, info.name, info.file.subp);
		else
			qn_outputf("\t\t %04X | %4d | %s <%d>", info.file.source.attr, info.file.source.type, info.name, info.file.source.seek);
		fc++;
#endif
	}
#ifdef HFS_DEBUG_TRACE
	qn_outputf("\tHFS: %d files", fc);
#endif

	return true;
}

//
static bool _hfs_save_dir(Hfs* self, const QnPathStr* dir, QnPathStr* save)
{
	if (_path_str_is_empty(dir) || _path_str_icmp_bstr(dir, &self->base.path) == 0)
	{
		if (save != NULL)
			_path_str_clear(save);
		return true;
	}
	if (save != NULL)
		_path_str_set_bstr(save, &self->base.path);
	return _hfs_chdir(self, dir->DATA);
}

//
static void _hfs_restore_dir(Hfs* self, const QnPathStr* save)
{
	if (save != NULL && _path_str_is_have(save))
		_hfs_chdir(self, save->DATA);
}

//
static bool _hfs_mkdir(QnGam g, const char* directory)
{
	if (directory[0] == '.')
	{
		errno = EINVAL;
		return false;
	}
	if (directory[0] == '/' && directory[1] == '\0')
		return true;
	if (strlen(directory) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
		return false;
	}

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

	const uint hash = _path_str_shash(&name);
	size_t i;
	QN_CTNR_FOREACH(self->infos, 1, i)	// 0번은 현재 디렉토리
	{
		const HfsInfo* info = _hfs_infos_ptr_nth(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && _path_str_icmp(&name, info->name) == 0)
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

	// 디렉토리 만들고 "." 추가
	const uint next = (uint)qn_stream_tell(stream);
	const uint subp = (uint)(next + sizeof(HfsFile) + name.LENGTH);
	const QnTimeStamp stc = qn_now();
	_hfs_write_directory(stream, name.DATA, name.LENGTH, hash, stc, subp, 0);
	_hfs_write_directory(stream, ".", 1, 0, stc, subp, (uint)(subp + sizeof(HfsFile) + 1));

	// ".." 디렉토리
	const HfsInfo* parent = _hfs_infos_ptr_nth(&self->infos, 0);
	_hfs_write_directory(stream, "..", 2, 0, parent->file.stc.stamp, parent->file.source.seek, 0);

	// 지금꺼 갱신
	const HfsInfo* last = _hfs_infos_ptr_inv(&self->infos, 0);
	qn_stream_seek(stream, HFSAT_NEXT + last->file.source.seek, QNSEEK_BEGIN);
	qn_stream_write(stream, &next, 0, (int)sizeof(uint));

	//
	if (_path_str_is_empty(&save))
		_path_str_set_char(&save, '.');
	_hfs_restore_dir(self, &save);

	self->touch++;
	return true;
}

// 제거
static bool _hfs_remove(QnGam g, const char* path)
{
	if (strlen(path) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
		return false;
	}

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

	const uint hash = _path_str_shash(&name);
	const HfsInfo* found = NULL;
	size_t i;
	QN_CTNR_FOREACH(self->infos, 1, i)	// 0번은 현재 디렉토리
	{
		const HfsInfo* info = _hfs_infos_ptr_nth(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && _path_str_icmp(&name, info->name) == 0)
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
	const HfsInfo* prev = _hfs_infos_ptr_nth(&self->infos, i - 1);
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

// 소스로 열기
static QnStream* _hfs_source_open(Hfs* hfs, HfsSource* source, const char* filename)
{
	QnStream* stream;
	if (QN_TMASK(source->attr, QNFATTR_CMPR) || QN_TMASK(source->attr, QNFATTR_INDIRECT) == false)
	{
		void* data = _hfs_source_read(hfs, source);
		stream = data == NULL ? NULL : _create_mem_stream_hfs(qn_cast_type(hfs, QnMount), filename, data, source->size);
	}
	else
	{
		llong offset = source->seek + sizeof(HfsFile) + source->len;
		stream = _indirect_stream_open(qn_cast_type(hfs, QnMount), filename, offset, source->size);
	}
	return stream;
}

// 파일 읽기
static void* _hfs_alloc(QnGam g, const char* filename, int* size)
{
	if (strlen(filename) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
		return NULL;
	}

	Hfs* self = qn_cast_type(g, Hfs);

	QnPathStr dir, name, keep, *save = QN_TMASK(self->base.flags, QNMFT_NORESTORE) ? NULL : &keep;
	_hfs_split_path(filename, &dir, &name);
	if (_path_str_is_empty(&name) || name.DATA[0] == '.')
	{
		errno = EINVAL;
		return NULL;
	}
	if (_hfs_save_dir(self, &dir, save) == false)
	{
		errno = ENOENT;
		return NULL;
	}

	const uint hash = _path_str_shash(&name);
	HfsInfo* found = NULL;
	size_t i;
	QN_CTNR_FOREACH(self->infos, 1, i)	// 0번은 현재 디렉토리
	{
		HfsInfo* info = _hfs_infos_ptr_nth(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && _path_str_icmp(&name, info->name) == 0)
		{
			found = info;
			break;
		}
	}
	if (found == NULL)
	{
		_hfs_restore_dir(self, save);
		errno = ENOENT;
		return NULL;
	}

	void* data = _hfs_source_read(self, &found->file.source);
	if (size)
		*size = (int)found->file.source.size;

	_hfs_restore_dir(self, save);
	return data;
}

// 파일 열기
static QnStream* _hfs_stream(QnGam g, const char* filename, const char* mode)
{
	if (strlen(filename) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
		return NULL;
	}
	if (mode)
	{
		for (const char* p = mode; *p; p++)
		{
			if (*p == 'w' || *p == 'a' || *p == '+')
			{
				qn_mesgfb("Hfs", "file write mode is not supported: %s (%s)", filename, mode);
				errno = EACCES;
				return NULL;
			}
		}
	}

	Hfs* self = qn_cast_type(g, Hfs);

	QnPathStr dir, name, keep, *save = QN_TMASK(self->base.flags, QNMFT_NORESTORE) ? NULL : &keep;
	_hfs_split_path(filename, &dir, &name);
	if (_path_str_is_empty(&name) || name.DATA[0] == '.')
	{
		errno = EINVAL;
		return NULL;
	}
	if (_hfs_save_dir(self, &dir, save) == false)
	{
		errno = ENOENT;
		return NULL;
	}

	const uint hash = _path_str_shash(&name);
	HfsInfo* found = NULL;
	size_t i;
	QN_CTNR_FOREACH(self->infos, 1, i)	// 0번은 현재 디렉토리
	{
		HfsInfo* info = _hfs_infos_ptr_nth(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && _path_str_icmp(&name, info->name) == 0)
		{
			found = info;
			break;
		}
	}
	if (found == NULL)
	{
		_hfs_restore_dir(self, save);
		errno = ENOENT;
		return NULL;
	}

	QnStream* stream = _hfs_source_open(self, &found->file.source, filename);
	_hfs_restore_dir(self, save);
	return stream;
}

// 파일 있나
static QnFileAttr _hfs_attr(QnGam g, const char* path)
{
	if (strlen(path) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
		return QNFATTR_NONE;
	}

	Hfs* self = qn_cast_type(g, Hfs);

	QnPathStr dir, name, keep, *save = QN_TMASK(self->base.flags, QNMFT_NORESTORE) ? NULL : &keep;
	_hfs_split_path(path, &dir, &name);
	if (_hfs_save_dir(self, &dir, save) == false)
	{
		errno = ENOENT;
		return QNFATTR_NONE;
	}

	const uint hash = _path_str_shash(&name);
	QnFileAttr attr = QNFATTR_NONE;
	size_t i;
	QN_CTNR_FOREACH(self->infos, 1, i)	// 0번은 현재 디렉토리
	{
		const HfsInfo* info = _hfs_infos_ptr_nth(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && _path_str_icmp(&name, info->name) == 0)
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
	return _hfs_infos_ptr_nth(&self->infos, self->base.base.desc++);
}

//
static const char* _hfs_list_read(QnGam g)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	const HfsInfo* info = _hfs_list_internal_read_info(self);
	return info == NULL ? NULL : info->name;
}

//
static bool _hfs_list_read_info(QnGam g, QnFileInfo* ret)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	const HfsInfo* info = _hfs_list_internal_read_info(self);
	if (info == NULL)
		return false;

	ret->attr = info->file.source.attr;
	ret->len = info->file.source.len;
	ret->size = info->file.source.size;
	ret->cmpr = info->file.source.cmpr;
	ret->stc = info->file.stc.stamp;
	ret->stw = info->file.stc.stamp;
	ret->name = info->name;
	return true;
}

//
static void _hfs_list_rewind(QnGam g)
{
	HfsDir* self = qn_cast_type(g, HfsDir);
	self->base.base.desc = 0;
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
static QnDir* _hfs_list(_In_ QnGam g, const char* directory, const char* mask)
{
	if (strlen(directory) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
		return NULL;
	}

	Hfs* hfs = qn_cast_type(g, Hfs);
	QnPathStr dir, save;
	_path_str_set(&dir, directory);
	if (_hfs_save_dir(hfs, &dir, &save) == false)
	{
		errno = ENOENT;
		return false;
	}

	HfsDir* self = qn_alloc_zero_1(HfsDir);
	if (mask == NULL || (mask[0] == '*' && mask[1] == '\0'))
		_hfs_infos_init_copy(&self->infos, &hfs->infos);
	else
	{
		_hfs_infos_init(&self->infos, 0);
		size_t i;
		QN_ARRAY_FOREACH(hfs->infos, 0, i)
		{
			HfsInfo* info = _hfs_infos_ptr_nth(&hfs->infos, i);
			if (qn_striwcm(info->name, mask))
				_hfs_infos_ptr_add(&self->infos, info);
		}
	}

	self->base.mount = qn_load(hfs);
	self->base.name = qn_strdup(hfs->base.path.DATA);	// VS는 여기 널이라고 우기지만 mount는 널이 아니므로 무시
	qn_set_gam_desc(self, 0);

	_hfs_restore_dir(hfs, &save);

	static const struct QNDIR_VTABLE _hfs_list_vt =
	{
		.base.name = "HfsList",
		.base.dispose = _hfs_list_dispose,
		.dir_read = _hfs_list_read,
		.dir_read_info = _hfs_list_read_info,
		.dir_rewind = _hfs_list_rewind,
	};
	return qn_gam_init(self, _hfs_list_vt);
}

// HFS 만들기
static bool _hfs_create_file(_In_ QnStream* stream, char* desc)
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
	if (qn_stream_write(stream, &header, 0, sizeof(HfsHeader)) != sizeof(HfsHeader))
		return false;

	_hfs_write_directory(stream, ".", 1, 0, 0, HFSAT_ROOT, 0);
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

			// 버퍼 사이즈에 관한 참고: https://github.com/dotnet/runtime/discussions/74405
			byte* buffer = qn_alloc(16LL * 1024LL, byte);
			for (;;)
			{
				const int read = _file_stream_read(filestream, buffer, 0, 16LL * 1024LL);
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

	static const struct QNMOUNT_VTABLE _hfs_vt =
	{
		.base.name = "Hfs",
		.base.dispose = _hfs_dispose,
		.mount_cwd = _mount_cwd,
		.mount_chdir = _hfs_chdir,
		.mount_mkdir = _hfs_mkdir,
		.mount_remove = _hfs_remove,
		.mount_attr = _hfs_attr,
		.mount_stream = _hfs_stream,
		.mount_list = _hfs_list,
		.mount_alloc = _hfs_alloc,
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
	const uint hash = _path_str_shash(&name);
	size_t i;
	QN_CTNR_FOREACH(self->infos, 1, i)	// 0번은 현재 디렉토리
	{
		const HfsInfo* info = _hfs_infos_ptr_nth(&self->infos, i);
		if (hash == info->file.hash && name.LENGTH == info->file.source.len && _path_str_icmp(&name, info->name) == 0)
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
		.file.source.attr = QNFATTR_FILE,
		.file.source.type = (byte)type,
		.file.source.size = size,
		.file.source.cmpr = 0,
		.file.source.seek = 0,
		.file.stc.stamp = qn_now(),
		.file.subp = 0,
		.file.next = 0,
		// len, hash는 _hfs_write_file_header()에서 채워진다
	};

	bool isok = false;
	if (cmpr)
	{
		file.file.source.attr |= QNFATTR_CMPR;
		file.file.source.cmpr = (uint)sizecmpr;
		if (_hfs_write_file_header(stream, &file.file, name.DATA, name.LENGTH, hash) &&
			qn_stream_write(stream, bufcmpr, 0, (int)sizecmpr) == (int)sizecmpr)
			isok = true;
		qn_free(bufcmpr);
	}
	else
	{
		if (_hfs_write_file_header(stream, &file.file, name.DATA, name.LENGTH, hash) &&
			qn_stream_write(stream, data, 0, (int)size) == (int)size)
			isok = true;
	}
	if (isok == false)
	{
		_hfs_restore_dir(self, &save);
		return false;
	}

	// 지금꺼 갱신
	HfsInfo* last = _hfs_infos_ptr_inv(&self->infos, 0);
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
	if (strlen(filename) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
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
	if (strlen(filename) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
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

	if (filename == NULL)
		filename = srcfile;
	if (strlen(filename) >= QN_MAX_PATH - 1)
	{
		errno = ENAMETOOLONG;
		return false;
	}

	const QnFileAttr attr = qn_get_file_attr(NULL, srcfile);
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

	HfsInfoArray infos;
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
	QN_CTNR_FOREACH(infos, 1, i)	// 0번은 현재 디렉토리
	{
		HfsInfo* info = _hfs_infos_ptr_nth(&infos, i);
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

			if (_hfs_write_file_header(stream, &file.file, info->name, info->file.source.len, info->file.hash) == false ||
				qn_stream_write(stream, data, 0, size) != (int)size)
			{
				qn_free(data);
				_hfs_infos_dispose(&infos);
				return false;
			}
			qn_free(data);

			HfsInfo* last = _hfs_infos_ptr_inv(&output->infos, 0);
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
	_path_str_set_char(&dir, '/');
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
		tmppath = qn_getcwd(NULL);
#endif
		return _create_diskfs(tmppath);
}

	const QnFileAttr attr = qn_get_file_attr(NULL, path);
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
		if (_internal_mkdir(path) == false)
			return NULL;
		tmppath = qn_strdup(path);
		return _create_diskfs(tmppath);
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// 퓨즈

// 퓨즈 소스
typedef struct FUSESOURCE
{
	Hfs*			hfs;
	HfsSource		source;
	char			name[260];
} FuseSource;

// Hfs 언로드
INLINE void _hfs_unload_ptr(Hfs** hfs)
{
	qn_unload(*hfs);
}

// 파일 소스 해시
QN_DECLIMPL_MUKUM(FsMukum, char*, FuseSource, qn_strphash, qn_strpcmp, (void), (void), _fs_mukum);
// 마운트 해시
QN_DECLIMPL_MUKUM(HfsMukum, char*, Hfs*, qn_strphash, qn_strpcmp, (void), _hfs_unload_ptr, _hfs_mukum);

// 퓨즈
typedef struct FUSE
{
	QnMount				base;

	HfsMukum			hfss;
	FsMukum				fss;
	QnSpinLock			lock;
	bool				diskfs;
} Fuse;

//
static QnStream* _fuse_stream(QnGam g, const char* filename, const char* mode)
{
	Fuse* self = qn_cast_type(g, Fuse);
	QnStream* stream;
	QN_LOCK(self->lock);
	if (self->diskfs)
	{
		stream = _file_stream_open(qn_cast_type(self, QnMount), filename, mode);
		if (stream != NULL)
		{
			QN_UNLOCK(self->lock);
			return stream;
		}
	}

	FuseSource* pfs = _fs_mukum_get(&self->fss, filename);
	stream = pfs == NULL ? NULL : _hfs_source_open(pfs->hfs, &pfs->source, pfs->name);
	QN_UNLOCK(self->lock);
	return stream;
}

//
static void* _fuse_alloc(QnGam g, const char* filename, int* size)
{
	Fuse* self = qn_cast_type(g, Fuse);
	void* data;
	QN_LOCK(self->lock);
	if (self->diskfs)
	{
		data = _disk_fs_alloc(self, filename, size);
		if (data != NULL)
		{
			QN_UNLOCK(self->lock);
			return data;
		}
	}

	FuseSource* pfs = _fs_mukum_get(&self->fss, filename);
	if (pfs == NULL)
		data = NULL;
	else
	{
		data = _hfs_source_read(pfs->hfs, &pfs->source);
		if (data != NULL)
			*size = pfs->source.size;
	}
	QN_UNLOCK(self->lock);
	return data;
}

//
static QnFileAttr _fuse_attr(QnGam g, const char* path)
{
	Fuse* self = qn_cast_type(g, Fuse);
	QnFileAttr attr;
	QN_LOCK(self->lock);
	if (self->diskfs)
	{
		attr = _disk_fs_attr(self, path);
		if (attr != QNFATTR_NONE)
		{
			QN_UNLOCK(self->lock);
			return attr;
		}
	}
	FuseSource* pfs = _fs_mukum_get(&self->fss, path);
	attr = pfs != NULL ? pfs->source.attr : QNFATTR_NONE;
	QN_UNLOCK(self->lock);
	return attr;
}

//
static bool _fuse_remove(QnGam g, const char* path)
{
	Fuse* self = qn_cast_type(g, Fuse);
	QN_LOCK(self->lock);
	if (self->diskfs)
	{
		if (_disk_fs_remove(self, path))
		{
			QN_UNLOCK(self->lock);
			return true;
		}
	}

	bool ret;
	FuseSource* pfs = _fs_mukum_get(&self->fss, path);
	if (pfs == NULL)
		ret = false;
	else
	{
		_hfs_remove(pfs->hfs, path);
		ret = _fs_mukum_remove(&self->fss, path);
	}
	QN_UNLOCK(self->lock);
	return ret;
}

//
static void _fuse_dispose(QnGam g)
{
	Fuse* self = qn_cast_type(g, Fuse);

	_fs_mukum_dispose(&self->fss);
	_hfs_mukum_dispose(&self->hfss);
	qn_free(self->base.name);
	qn_free(self);
}

// HFS 분석
static void _fuse_parse_hfs(Fuse* self, Hfs* hfs, const char* dir)
{
	HfsInfoArray infos;
	_hfs_infos_init_copy(&infos, &hfs->infos);

	QnPathStr bs;
	_path_str_set(&bs, dir);
	size_t bpos = _path_str_len(&bs);

	size_t i;
	QN_ARRAY_FOREACH(infos, 1, i)	// 0번은 현재 디렉토리
	{
		HfsInfo* info = _hfs_infos_ptr_nth(&infos, i);
		if (info->name[0] == '.')
			continue;

		_path_str_add(&bs, info->name);
		if (QN_TMASK(info->file.source.attr, QNFATTR_DIR))
		{
			_path_str_add_char(&bs, '/');
			_hfs_chdir(hfs, bs.DATA);
			_fuse_parse_hfs(self, hfs, bs.DATA);
			_hfs_chdir(hfs, "..");
		}
		else
		{
			FsMukumNode* node = qn_alloc_1(FsMukumNode);
			node->VALUE.hfs = hfs;
			node->VALUE.source = info->file.source;
			qn_strcpy(node->VALUE.name, bs.DATA);
			node->KEY = node->VALUE.name;
			if (!_fs_mukum_add(&self->fss, node))
				qn_free(node);
		}
		_path_str_trunc(&bs, bpos);
	}

	_hfs_infos_dispose(&infos);
}

// HFS 추가
static bool _fuse_add_hfs(Fuse* self, const char* name)
{
	Hfs** phfs = _hfs_mukum_get(&self->hfss, name);
	qn_return_when_fail(phfs == NULL, false);

	Hfs* hfs = (Hfs*)_create_hfs(name, "f");
	if (hfs == NULL)
		return false;
	_hfs_mukum_set(&self->hfss, hfs->base.name, hfs);
	_fuse_parse_hfs(self, hfs, "/");
	return true;
}

//
bool qn_fuse_add_hfs(QnMount* mount, const char* name)
{
	qn_return_when_fail(QN_TMASK(mount->flags, QNMFT_FUSE), false);

	Fuse* self = qn_cast_type(mount, Fuse);
	QN_LOCK(self->lock);
	bool ret = _fuse_add_hfs(self, name);
	QN_UNLOCK(self->lock);
	return ret;
}

//
int qn_fuse_get_hfs_count(QnMount* mount)
{
	qn_return_when_fail(QN_TMASK(mount->flags, QNMFT_FUSE), -1);
	Fuse* self = qn_cast_type(mount, Fuse);
	return (int)_hfs_mukum_count(&self->hfss);
}

//
bool qn_fuse_get_disk_fs_enabled(QnMount* mount)
{
	qn_return_when_fail(QN_TMASK(mount->flags, QNMFT_FUSE), false);
	Fuse* self = qn_cast_type(mount, Fuse);
	return self->diskfs;
}

//
void qn_fuse_set_disk_fs_enabled(QnMount* mount, bool enabled)
{
	qn_return_when_fail(QN_TMASK(mount->flags, QNMFT_FUSE), );
	Fuse* self = qn_cast_type(mount, Fuse);
	self->diskfs = enabled;
}

// 디스크 파일 시스템 마운트 만들기, path에 대한 오류처리는 하고 왔을 것이다
QnMount* qn_create_fuse(const char* path, bool diskfs, bool loadall)
{
	Fuse* self = qn_alloc_zero_1(Fuse);

	if (path == NULL)
	{
		char* cwd = qn_getcwd(NULL);
#ifdef _QN_WINDOWS_
		self->base.name = qn_strdupcat(cwd, "\\", NULL);
#else
		self->base.name = qn_strdupcat(cwd, "/", NULL);
#endif
		self->base.name_len = strlen(self->base.name);
		qn_free(cwd);
	}
	else
	{
		const size_t len = strlen(path);
		if (path[len - 1] == '/' || path[len - 1] == '\\')
		{
			self->base.name = qn_strdup(path);
			self->base.name_len = len;
		}
		else
		{
#ifdef _QN_WINDOWS_
			self->base.name = qn_strdupcat(path, "\\", NULL);
#else
			self->base.name = qn_strdupcat(path, "/", NULL);
#endif
			self->base.name_len = len + 1;
		}
		}

	_hfs_mukum_init_fast(&self->hfss);
	_fs_mukum_init_fast(&self->fss);
	_path_str_set_len(&self->base.path, self->base.name, self->base.name_len);
	self->base.flags = QNMFT_DISKFS | QNMFT_FUSE;
	self->diskfs = diskfs;

	//
	static const struct QNMOUNT_VTABLE _fuse_vt =
	{
		.base.name = "DiskFS",
		.base.dispose = _fuse_dispose,
		.mount_cwd = _mount_cwd,
		.mount_chdir = _disk_fs_chdir,
		.mount_mkdir = _disk_fs_mkdir,
		.mount_remove = _fuse_remove,
		.mount_attr = _fuse_attr,
		.mount_stream = _fuse_stream,
		.mount_list = _disk_fs_list,
		.mount_alloc = _fuse_alloc,
	};
	qn_gam_init(self, _fuse_vt);

	if (loadall)
	{
		// 모든 파일을 읽어들인다
		QnDir* dir = _disk_fs_list(self, "", "*.hfs");
		if (dir != NULL)
		{
			for (;;)
			{
				const char* filename = _disk_list_read(dir);
				if (filename == NULL)
					break;
				if (_fuse_add_hfs(self, filename) == false)
					qn_mesgfb("Fuse", "Failed to load HFS file: %s (check opened by other program)", filename);
			}
			qn_unload(dir);
		}
	}

	return qn_cast_type(self, QnMount);
	}


//////////////////////////////////////////////////////////////////////////
// 스트림 실체

//
QnStream* qn_open_stream(QnMount* mount, const char* filename, const char* mode)
{
	qn_return_when_fail(filename != NULL, NULL);
	if (mount != NULL)
		return qn_cast_vtable(mount, QNMOUNT)->mount_stream(mount, filename, mode);
	return _file_stream_open(NULL, filename, mode);
}

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
QnDir* qn_open_dir(QnMount* mount, const char* directory, const char* mask)
{
	if (directory == NULL)
		directory = ".";
	return qn_cast_vtable(mount, QNMOUNT)->mount_list(mount, directory, mask);
}

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
