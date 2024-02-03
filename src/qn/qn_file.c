//
// qn_file.c - 파일 처리. 추가로 모듈 기능(DLL, SO)
// 2023-12-27 by kim
//

// ReSharper disable CppParameterMayBeConst

#include "pch.h"
#ifdef _QN_UNIX_
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#endif

// 최대 파일 할당 크기
static size_t max_file_alloc_size = 512ULL * 1024ULL * 1024ULL;

//
size_t qn_file_get_max_alloc_size(void)
{
	return max_file_alloc_size;
}

//
void qn_file_set_max_alloc_size(const size_t n)
{
	max_file_alloc_size = n == 0 ? (512ULL * 1024ULL * 1024ULL) : n;
}

//
size_t qn_get_file_path(const char* filename, char* dest, size_t destsize)
{
	qn_val_if_fail(filename != NULL, 0);

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
char* qn_get_base_path(void)
{
	char* path = NULL;
#ifdef _QN_WINDOWS_
	DWORD len = 128;
	wchar* pw = NULL;
	while (true)
	{
		pw = qn_realloc(pw, len, wchar);
		len = GetModuleFileName(NULL, pw, len);
		if (len < len - 1)
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
	qn_assert(i > 0, "probably invalid base path!");
	pw[i + 1] = L'\0';

	const size_t sz = qn_u16to8(NULL, 0, pw, 0);
	path = qn_alloc(sz + 1, char);
	qn_u16to8(path, sz + 1, pw, 0);
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


//////////////////////////////////////////////////////////////////////////
// 파일 스트림

//
typedef struct QNFILESTREAM
{
	QnStream			base;
	QnFileAccess		acs;
} QnFileStream;

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
					*flags = QNFF_WRITE | QNFF_SEEK;
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
					*flags = QNFF_WRITE | QNFF_SEEK;
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
		*flag = QNFF_READ | QNFF_SEEK;
	}

	if (acs->access == 0 && (acs->mode & O_CREAT) != 0)
		acs->access = (S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH);
#endif
}

// 실제 파일 경로 얻기
static char* qn_file_stream_get_real_path(const QnMount* mount, const char* RESTRICT filename)
{
	char* s;
	if (*filename != '/' && *filename != '\\')
		s = qn_strcat(mount->name, mount->path, filename);
	else
		s = qn_strcat(mount->name, filename);
	return s;
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
	ifd(fd >= 0)
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
		qn_syserr(EBADF, false);
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
	qn_set_gam_desc(self, dup);

	return qn_gam_init_dup(self, source);
}


// 파일 스트림 열기
QnStream* qn_open_file_stream(QnMount* mount, const char* RESTRICT filename, const char* RESTRICT mode)
{
	qn_val_if_fail(filename != NULL, NULL);

	char* path;
	if (mount == NULL)
		path = qn_strdup(filename);
	else
	{
		if (QN_TMASK(mount->flags, QNMFT_DISKFS) == false)
		{
			qn_syserr(EPERM, true);
			return NULL;
		}
		path = qn_file_stream_get_real_path(mount, filename);
	}

	QnFileStream* self = qn_alloc(1, QnFileStream);

	qn_file_stream_access_parse(mode, &self->acs, &self->base.flags);
#ifdef _QN_WINDOWS_
	wchar* uni = (wchar*)qn_u8to16_dup(path, 0);
	HANDLE fd = CreateFile(uni, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);
	qn_free(uni);
	if (fd == NULL || fd == INVALID_HANDLE_VALUE)
#else
	int fd = self->acs.access == 0 ? open(path, self->acs.mode) : open(path, self->acs.mode, self->acs.access);
	if (fd < 0)
#endif
	{
		qn_syserr(ENOENT, false);
		qn_free(path);
		qn_free(self);
		return NULL;
	}

	self->base.mount = mount;
	self->base.name = path;
	qn_set_gam_desc(self, fd);

	qn_gam_vt(QNSTREAM) qn_file_stream_vt =
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
	return qn_gam_init(self, qn_file_stream_vt);
}


//////////////////////////////////////////////////////////////////////////
// 스트림

//
QnStream* qn_open_stream(QnMount* mount, const char* RESTRICT filename, const char* RESTRICT mode)
{
	if (mount != NULL)
		return qn_cast_vt(mount, QNMOUNT)->open(mount, filename, mode);

	return NULL;
}

//
int qn_stream_read(QnGam self, void* RESTRICT buffer, const int offset, const int size)
{
	qn_val_if_fail(buffer != NULL, -1);
	qn_val_if_fail(size >= 0, 0);

	return qn_cast_vt(self, QNSTREAM)->stream_read(self, buffer, offset, size);
}

//
int qn_stream_write(QnGam self, const void* RESTRICT buffer, const int offset, const int size)
{
	qn_val_if_fail(buffer != NULL, -1);
	qn_val_if_fail(size >= 0, 0);

	return qn_cast_vt(self, QNSTREAM)->stream_write(self, buffer, offset, size);
}

//
llong qn_stream_seek(QnGam self, const llong offset, const QnSeek org)
{
	return qn_cast_vt(self, QNSTREAM)->stream_seek(self, offset, org);
}

//
llong qn_stream_tell(QnGam self)
{
	return qn_cast_vt(self, QNSTREAM)->stream_tell(self);
}

//
llong qn_stream_size(QnGam self)
{
	return qn_cast_vt(self, QNSTREAM)->stream_size(self);
}

//
bool qn_stream_flush(QnGam self)
{
	return qn_cast_vt(self, QNSTREAM)->stream_flush(self);
}

//
QnStream* qn_stream_dup(/*QnStream*/QnGam self)
{
	return qn_cast_vt(self, QNSTREAM)->stream_dup(self);
}



//////////////////////////////////////////////////////////////////////////
// 파일 리스트

//
const char* qn_file_list_read(QnFileList* self)
{
	return qn_cast_vt(self, QNFILELIST)->read(self);
}

//
bool qn_file_list_read_info(QnFileList* self, QnFileInfo* info)
{
	qn_val_if_fail(info, false);
	return qn_cast_vt(self, QNFILELIST)->read_info(self, info);
}

//
void qn_file_list_rewind(QnFileList* self)
{
	qn_cast_vt(self, QNFILELIST)->rewind(self);
}

//
void qn_file_list_seek(QnFileList* self, const int pos)
{
	qn_cast_vt(self, QNFILELIST)->seek(self, pos);
}

//
int qn_file_list_tell(QnFileList* self)
{
	return qn_cast_vt(self, QNFILELIST)->tell(self);
}


//////////////////////////////////////////////////////////////////////////
// 마운트

//
QnStream* qn_mount_open_file(QnGam g, const char* RESTRICT filename, const char* RESTRICT mode)
{
	qn_val_if_fail(filename, NULL);
	return qn_cast_vt(g, QNMOUNT)->open(g, filename, mode);
}

//
void* qn_mount_read_file(QnGam g, const char* RESTRICT filename, int* size)
{
	qn_val_if_fail(filename, NULL);
	return qn_cast_vt(g, QNMOUNT)->read(g, filename, size);
}

//
char* qn_mount_read_text(QnGam g, const char* RESTRICT filename, int* length)
{
	qn_val_if_fail(filename, NULL);
	return qn_cast_vt(g, QNMOUNT)->read_text(g, filename, length);
}

//
bool qn_mount_file_exist(QnGam g, const char* RESTRICT filename)
{
	qn_val_if_fail(filename, false);
	return qn_cast_vt(g, QNMOUNT)->exist(g, filename);
}

//
bool qn_mount_remove(QnGam g, const char* RESTRICT filename)
{
	qn_val_if_fail(filename, false);
	return qn_cast_vt(g, QNMOUNT)->remove(g, filename);
}

//
bool qn_mount_chdir(QnGam g, const char* RESTRICT path)
{
	qn_val_if_fail(path, false);
	return qn_cast_vt(g, QNMOUNT)->ch_dir(g, path);
}

//
bool qn_mount_mkdir(QnGam g, const char* RESTRICT path)
{
	qn_val_if_fail(path, false);
	return qn_cast_vt(g, QNMOUNT)->mk_dir(g, path);
}


//////////////////////////////////////////////////////////////////////////
// 디스크 파일 시스템 마운트

// QnStream* (*open)(/*QnMount*/QnGam, const char* RESTRICT, const char* RESTRICT);
QnStream* qn_diskfs_open_file(QnGam g, const char* RESTRICT filename, const char* RESTRICT mode)
{
	qn_val_if_fail(filename, NULL);

	return NULL;
}

// void* (*read)(/*QnMount*/QnGam, const char*, int*);
// char* (*read_text)(/*QnMount*/QnGam, const char*, int*);
// bool (*exist)(/*QnMount*/QnGam, const char*);
// bool (*remove)(/*QnMount*/QnGam, const char*);
// bool (*ch_dir)(/*QnMount*/QnGam, const char*);
// bool (*mk_dir)(/*QnMount*/QnGam, const char*);

