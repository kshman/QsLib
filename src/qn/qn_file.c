//
// qn_file.c - 파일 처리. 추가로 모듈 기능(DLL, SO)
// 2023-12-27 by kim
//

// ReSharper disable CppParameterMayBeConst

#include "pch.h"
#include "qs_qn.h"
#include <fcntl.h>
#ifdef _QN_UNIX_
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _QN_BSD_
#include <sys/sysctl.h>
#endif
#endif

//////////////////////////////////////////////////////////////////////////
// 파일

// 파일 구조체
struct QnFile
{
#ifdef _QN_WINDOWS_
	HANDLE			fd;
#else
	int				fd;
#endif
	char*			name;
	QnFileAccess	acs;
	int				flag;
};

// 최대 할당 크기
static size_t max_file_alloc_size = 512ULL * 1024ULL * 1024ULL;

// 분석
static void qn_file_access_parse(const char* mode, QnFileAccess* self, int* flag)
{
#ifdef _QN_WINDOWS_
	self->mode = 0;

	if (mode)
	{
		self->access = 0;
		self->share = FILE_SHARE_READ;
		self->attr = FILE_ATTRIBUTE_NORMAL;

		for (char ch; *mode; ++mode)
		{
			switch (*mode)
			{
				case 'a':
					self->mode = OPEN_ALWAYS;
					self->access = GENERIC_WRITE;
					*flag = QNFF_WRITE | QNFF_SEEK;
					break;

				case 'r':
					self->mode = OPEN_EXISTING;
					self->access = GENERIC_READ;
					*flag = QNFF_READ | QNFF_SEEK;
					break;

				case 'w':
					self->mode = CREATE_ALWAYS;
					self->access = GENERIC_WRITE;
					*flag = QNFF_WRITE | QNFF_SEEK;
					break;

				case '+':
					self->access = GENERIC_WRITE | GENERIC_READ;
					*flag = QNFF_READ | QNFF_WRITE | QNFF_SEEK;
					break;

				case '@':
					ch = *(mode + 1);

					if (ch == 'R')
					{
						self->share = FILE_SHARE_READ;
						++mode;
					}
					else if (ch == 'W')
					{
						self->share = FILE_SHARE_WRITE;
						++mode;
					}
					else if (ch == '+')
					{
						self->share = FILE_SHARE_READ | FILE_SHARE_WRITE;
						++mode;
					}
					break;

				case '!':
					if (self->mode == CREATE_ALWAYS)
						self->attr |= FILE_FLAG_DELETE_ON_CLOSE;

					break;

				default:
					break;
			}
		}
	}

	if (self->mode == 0)
	{
		*flag = QNFF_READ | QNFF_SEEK;
		self->mode = OPEN_EXISTING;
		self->access = GENERIC_READ;
		self->attr = FILE_ATTRIBUTE_NORMAL;
		self->share = FILE_SHARE_READ;
	}
#else
	char ch, cm;

	self->mode = 0;
	self->access = 0;

	if (mode)
	{
		for (; *mode; ++mode)
		{
			switch (*mode)
			{
				case 'a':
					self->mode = O_WRONLY | O_APPEND | O_CREAT;
					*flag = QNFF_WRITE | QNFF_SEEK;
					break;

				case 'r':
					self->mode = O_RDONLY;
					*flag = QNFF_READ | QNFF_SEEK;
					break;

				case 'w':
					self->mode = O_WRONLY | O_TRUNC | O_CREAT;
					*flag = QNFF_WRITE | QNFF_SEEK;
					break;

				case '+':
					self->mode |= O_RDWR;
					*flag = QNFF_READ | QNFF_WRITE | QNFF_SEEK;
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
									self->access |= S_IRUSR;
								else if (ch == 'W')
									self->access |= S_IWUSR;
								else if (ch == 'X')
									self->access |= S_IXUSR;

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
									self->access |= S_IRGRP;
								else if (ch == 'W')
									self->access |= S_IWGRP;
								else if (ch == 'X')
									self->access |= S_IXGRP;

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
									self->access |= S_IROTH;
								else if (ch == 'W')
									self->access |= S_IWOTH;
								else if (ch == 'X')
									self->access |= S_IXOTH;

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
								strncpy(sz, (mode + 1), (size_t)(p - (mode + 1)));
								self->access = qn_strtoi(sz, 8);
							}

							mode = p;
						}
					}

					break;

				case '!':
					break;
			}
		}
	}

	if (self->mode == 0)
	{
		self->mode = O_RDONLY;
		*flag = QNFF_READ | QNFF_SEEK;
	}

	if (self->access == 0 && (self->mode & O_CREAT) != 0)
		self->access = (S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH);
#endif
}

//
QnFile* qn_file_new(const char* restrict filename, const char* restrict mode)
{
	qn_val_if_fail(filename, NULL);

	QnFile* self = qn_alloc_1(QnFile);
	qn_val_if_fail(self, NULL);

	qn_file_access_parse(mode, &self->acs, &self->flag);

#ifdef _QN_WINDOWS_
	wchar* uni = (wchar*)qn_u8to16_dup(filename, 0);
	self->fd = CreateFile(uni, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);
	qn_free(uni);

	if (!self->fd || self->fd == INVALID_HANDLE_VALUE)
#else
	self->fd = self->acs.access == 0 ? open(filename, self->acs.mode) : open(filename, self->acs.mode, self->acs.access);

	if (self->fd < 0)
#endif
	{
		qn_free(self);
		return NULL;
	}

	self->name = qn_strdup(filename);

	return self;
}

//
QnFile* qn_file_new_l(const wchar* restrict filename, const wchar* restrict mode)
{
	qn_val_if_fail(filename, NULL);

	QnFile* self = qn_alloc_1(QnFile);
	qn_val_if_fail(self, NULL);

	char cmode[64];
#ifdef _QN_WINDOWS_
	qn_u16to8(cmode, QN_COUNTOF(cmode), mode, 0);
#else
	qn_u32to8(cmode, QN_COUNTOF(cmode), mode, 0);
#endif

	qn_file_access_parse(cmode, &self->acs, &self->flag);

	char* u8;
#ifdef _QN_WINDOWS_
	u8 = qn_u16to8_dup((const uchar2*)filename, 0);
	self->fd = CreateFile(filename, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);

	if (!self->fd || self->fd == INVALID_HANDLE_VALUE)
#else
	u8 = qn_u32to8_dup((const uchar4*)filename, 0);
	self->fd = self->acs.access == 0 ? open(u8, self->acs.mode) : open(u8, self->acs.mode, self->acs.access);

	if (self->fd < 0)
#endif
	{
		qn_free(u8);
		qn_free(self);
		return NULL;
	}

	self->name = u8;

	return self;
}

//
QnFile* qn_file_new_dup(QnFile* src)
{
	QnFile* self = qn_alloc_1(QnFile);
	qn_val_if_fail(self, NULL);

	memcpy(self, src, sizeof(QnFile));

#ifdef _QN_WINDOWS_
	const HANDLE p = GetCurrentProcess();
	HANDLE d;
	if (!DuplicateHandle(p, self->fd, p, &d, 0, false, DUPLICATE_SAME_ACCESS))
	{
		qn_free(self);
		return NULL;
	}

	self->fd = d;
#else
	self->fd = dup(src->fd);

	if (self->fd < 0)
	{
		qn_free(self);
		return NULL;
	}
#endif

	self->name = qn_strdup(src->name);

	return self;
}

//
void qn_file_delete(QnFile* self)
{
#ifdef _QN_WINDOWS_
	if (self->fd && self->fd != INVALID_HANDLE_VALUE)
		CloseHandle(self->fd);
#else
	if (self->fd >= 0)
		close(self->fd);
#endif

	qn_free(self->name);
	qn_free(self);
}

//
int qn_file_get_flags(const QnFile* self, const int mask)
{
	return (self->flag & mask) != 0;
}

//
const char* qn_file_get_name(const QnFile* self)
{
	return self->name;
}

//
int qn_file_read(QnFile* self, void* restrict buffer, const int offset, const int size)
{
	qn_val_if_fail(buffer != NULL, -1);
	qn_val_if_fail(size >= 0, 0);

	byte* ptr = (byte*)buffer;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return ReadFile(self->fd, ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return (int)read(self->fd, ptr + offset, (size_t)size);
#endif
}

//
int qn_file_write(QnFile* self, const void* restrict buffer, const int offset, const int size)
{
	qn_val_if_fail(buffer != NULL, -1);
	qn_val_if_fail(size >= 0, 0);

	const byte* ptr = (const byte*)buffer;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return WriteFile(self->fd, ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return (int)write(self->fd, ptr + offset, (size_t)size);
#endif
}

//
llong qn_file_size(QnFile* self)
{
#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	return GetFileSizeEx(self->fd, &ll) ? ll.QuadPart : -1;
#else
#ifdef _QN_LINUX_
	off_t save, last;
#elif defined _QN_BSD_
	int save, last;
#else
	long save, last;
#endif
	save = lseek(self->fd, 0, SEEK_CUR);
	last = lseek(self->fd, 0, SEEK_END);
	lseek(self->fd, save, SEEK_SET);
	return last;
#endif
}

//
llong qn_file_tell(QnFile* self)
{
#ifdef _QN_WINDOWS_
	const LARGE_INTEGER pl = { .QuadPart = 0 };
	LARGE_INTEGER ll;
	return SetFilePointerEx(self->fd, pl, &ll, FILE_CURRENT) ? ll.QuadPart : -1;
#else
	return lseek(self->fd, 0, SEEK_CUR);
#endif
}

//
llong qn_file_seek(QnFile* self, const llong offset, const QnSeek org)
{
#ifdef _QN_WINDOWS_
	static_assert(FILE_BEGIN == QNSEEK_BEGIN && FILE_CURRENT == QNSEEK_CUR && FILE_END == QNSEEK_END, "QnSeek not equal to OS seek");
	const LARGE_INTEGER* po = (const LARGE_INTEGER*)&offset;
	LARGE_INTEGER ll;
	const BOOL b = SetFilePointerEx(self->fd, *po, &ll, org);
	return b ? ll.QuadPart : -1;
#else
	static_assert(SEEK_SET == QNSEEK_BEGIN && SEEK_CUR == QNSEEK_CUR && SEEK_END == QNSEEK_END, "QnSeek not equal to OS seek");
	return lseek(self->fd, offset, (int)org);
#endif
}

//
bool qn_file_flush(const QnFile* self)
{
#ifdef _QN_WINDOWS_
	return FlushFileBuffers(self->fd) != 0;
#else
	return fsync(self->fd) == 0;
#endif
}

//
int qn_file_printf(QnFile* self, const char* restrict fmt, ...)
{
	va_list va, vq;

	va_start(va, fmt);
	va_copy(vq, va);
	int len = qn_vsnprintf(NULL, 0, fmt, vq);
	va_end(vq);

	char* buf = qn_alloc(len + 1, char);
	qn_vsnprintf(buf, (size_t)len + 1, fmt, va);
	va_end(va);
	len = qn_file_write(self, buf, 0, len);
	qn_free(buf);

	return len;
}

//
int qn_file_vprintf(QnFile* self, const char* restrict fmt, va_list va)
{
	int len = qn_vsnprintf(NULL, 0, fmt, va);
	char* buf = qn_alloc(len + 1, char);
	qn_vsnprintf(buf, (size_t)len + 1, fmt, va);
	len = qn_file_write(self, buf, 0, len);
	qn_free(buf);

	return len;
}

//
bool qn_file_exist(const char* restrict filename, /*RET-NULLABLE*/bool* is_dir)
{
	qn_val_if_fail(filename, false);

#ifdef _QN_WINDOWS_
	wchar* pw = qn_u8to16_dup(filename, 0);
	WIN32_FIND_DATA ffd = { 0, };
	const HANDLE h = FindFirstFileEx(pw, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);
	qn_free(pw);

	if (h == INVALID_HANDLE_VALUE)
		return false;
	else
	{
		FindClose(h);

		if (is_dir != NULL)
			*is_dir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		return true;
	}
#else
	struct stat s;
	int n = stat(filename, &s);

	if (n < 0)
		return false;
	else
	{
		if (is_dir != NULL)
			*is_dir = (s.st_mode & 0x4000) != 0;

		return true;
	}
#endif
}

//
bool qn_file_exist_l(const wchar* restrict filename, /*RET-NULLABLE*/bool* is_dir)
{
	qn_val_if_fail(filename, false);

#ifdef _QN_WINDOWS_
	WIN32_FIND_DATA ffd = { 0, };
	const HANDLE h = FindFirstFileEx(filename, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);

	if (h == INVALID_HANDLE_VALUE)
		return false;
	else
	{
		FindClose(h);

		if (is_dir != NULL)
			*is_dir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		return true;
	}
#else
	char* u8 = qn_u32to8_dup((const uchar4*)filename, 0);
	struct stat s;
	int n = stat(u8, &s);
	qn_free(u8);

	if (n < 0)
		return false;
	else
	{
		if (is_dir != NULL)
			*is_dir = (s.st_mode & 0x4000) != 0;

		return true;
	}
#endif
}

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
void* qn_file_alloc(const char* restrict filename, int* size)
{
	qn_val_if_fail(filename != NULL, NULL);

	QnFile* file = qn_file_new(filename, "rb");
	qn_val_if_fail(file, NULL);

	const llong len = qn_file_size(file);
	if (!len || len > (llong)max_file_alloc_size)
	{
		qn_file_delete(file);
		return NULL;
	}

	void* buf = qn_alloc((size_t)len + 4, byte);
	qn_file_read(file, buf, 0, (int)len);

	qn_file_delete(file);

	if (size)
		*size = (int)len;

	return buf;
}

//
void* qn_file_alloc_l(const wchar* restrict filename, int* size)
{
	qn_val_if_fail(filename, NULL);

	QnFile* file = qn_file_new_l(filename, L"rb");
	qn_val_if_fail(file, NULL);

	const llong len = qn_file_size(file);

	if (len <= 0 || (size_t)len > max_file_alloc_size)
	{
		qn_file_delete(file);
		return NULL;
	}

	void* buf = qn_alloc((size_t)len + 4, byte);
	qn_file_read(file, buf, 0, (int)len);

	qn_file_delete(file);

	if (size)
		*size = (int)len;

	return buf;
}


//////////////////////////////////////////////////////////////////////////
// 디렉토리

// 디렉토리 구조체.
struct QnDir
{
#ifdef _QN_WINDOWS_
	HANDLE			handle;
	WIN32_FIND_DATA	ffd;
	wchar*			name;
	int				stat;
	char			file[MAX_PATH];
#else
	DIR*			pd;
	wchar			ufile[FILENAME_MAX];
#endif
};

//
QnDir* qn_dir_new(const char* path)
{
#ifdef _QN_WINDOWS_
	qn_val_if_fail(path != NULL, NULL);

	wchar* pw = qn_u8to16_dup(path, 0);
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0, };
	if (!GetFileAttributesEx(pw, GetFileExInfoStandard, &fad))
		return NULL;
	if (!(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return NULL;

	wchar fpath[1024];
	(void)_wfullpath(fpath, pw, QN_COUNTOF(fpath) - 1);  // NOLINT
	qn_free(pw);

	size_t len = wcslen(fpath) + 1/*슬래시*/ + 1/*서픽스*/ + 1/*널*/;
	wchar* suffix = qn_alloc(len, wchar);
	qn_val_if_fail(suffix, NULL);

	wcscpy_s(suffix, len, fpath);
	len = wcslen(suffix);

	if (suffix[0] != L'\0' && suffix[len - 1] != L'/' && suffix[len - 1] != L'\\')
	{
		suffix[len] = QN_UNICODE(QN_PATH_SEP);
		suffix[len + 1] = L'\0';
		len++;
	}

	suffix[len] = L'*';
	suffix[len + 1] = L'\0';

	QnDir* self = qn_alloc_zero_1(QnDir);
	if (!self)
	{
		qn_free(suffix);
		return NULL;
	}

	self->name = suffix;
	self->handle = INVALID_HANDLE_VALUE;
#else
	qn_val_if_fail(path, NULL);

	DIR* pd = opendir(path);
	qn_val_if_fail(pd != NULL, NULL);

	QnDir* self = qn_alloc_zero_1(QnDir);
	self->pd = pd;
#endif

	return self;
}

//
QnDir* qn_dir_new_l(const wchar* path)
{
#ifdef _QN_WINDOWS_
	qn_val_if_fail(path != NULL, NULL);

	WIN32_FILE_ATTRIBUTE_DATA fad = { 0, };
	if (!GetFileAttributesEx(path, GetFileExInfoStandard, &fad))
		return NULL;
	if (!(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return NULL;

	wchar fpath[1024];
	(void)_wfullpath(fpath, path, QN_COUNTOF(fpath) - 1);

	size_t len = wcslen(fpath) + 1/*슬래시*/ + 1/*서픽스*/ + 1/*널*/;
	wchar* suffix = qn_alloc(len, wchar);
	qn_val_if_fail(suffix, NULL);

	wcscpy_s(suffix, len, fpath);
	len = wcslen(suffix);

	if (suffix[0] != L'\0' && suffix[len - 1] != L'/' && suffix[len - 1] != L'\\')
	{
		suffix[len] = QN_UNICODE(QN_PATH_SEP);
		suffix[len + 1] = L'\0';
		len++;
	}

	suffix[len] = L'*';
	suffix[len + 1] = L'\0';

	QnDir* self = qn_alloc_zero_1(QnDir);
	if (!self)
	{
		qn_free(suffix);
		return NULL;
	}

	self->name = suffix;
	self->handle = INVALID_HANDLE_VALUE;
#else
	qn_val_if_fail(path, NULL);

	char* u8 = qn_u32to8_dup((const uchar4*)path, 0);
	DIR* pd = opendir(u8);
	qn_free(u8);
	qn_val_if_fail(pd != NULL, NULL);

	QnDir* self = qn_alloc_zero_1(QnDir);
	self->pd = pd;
#endif

	return self;
}

//
void qn_dir_delete(QnDir* self)
{
#ifdef _QN_WINDOWS_
	if (self->handle != INVALID_HANDLE_VALUE)
		FindClose(self->handle);

	qn_free(self->name);
	qn_free(self->file);
#else
	closedir(self->pd);
	qn_free(self->ufile);
#endif

	qn_free(self);
}

//
const char* qn_dir_read(QnDir* self)
{
#ifdef _QN_WINDOWS_
	for (;;)
	{
		if (self->stat < 0)
			return NULL;
		else if (self->stat == 0)
		{
			// 처음
			self->handle = FindFirstFileEx(self->name, FindExInfoStandard, &self->ffd, FindExSearchNameMatch, NULL, 0);
			self->stat = self->handle == INVALID_HANDLE_VALUE ? -1 : 1;
		}
		else
		{
			// 계속
			if (FindNextFile(self->handle, &self->ffd))
				self->stat++;
			else
			{
				FindClose(self->handle);
				self->handle = INVALID_HANDLE_VALUE;
				self->stat = -1;

				return NULL;
			}
		}

		if (wcscmp(self->ffd.cFileName, L".") == 0 || wcscmp(self->ffd.cFileName, L"..") == 0)
			continue;

		qn_u16to8(self->file, QN_COUNTOF(self->file) - 1, self->ffd.cFileName, 0);
		return self->file;
	}
#else
	struct dirent* ent = readdir(self->pd);

	while (ent && (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0))
		ent = readdir(self->pd);

	return ent ? ent->d_name : NULL;
#endif
}

//
const wchar* qn_dir_read_l(QnDir* self)
{
#ifdef _QN_WINDOWS_
	for (;;)
	{
		if (self->stat < 0)
			return NULL;
		else if (self->stat == 0)
		{
			// 처음
			self->handle = FindFirstFileEx(self->name, FindExInfoStandard, &self->ffd, FindExSearchNameMatch, NULL, 0);
			self->stat = self->handle == INVALID_HANDLE_VALUE ? -1 : 1;
		}
		else
		{
			// 계속
			if (FindNextFile(self->handle, &self->ffd))
				self->stat++;
			else
			{
				FindClose(self->handle);
				self->handle = INVALID_HANDLE_VALUE;
				self->stat = -1;

				return NULL;
			}
		}

		if (wcscmp(self->ffd.cFileName, L".") != 0 || wcscmp(self->ffd.cFileName, L"..") != 0)
			continue;

		return self->ffd.cFileName;
	}
#else
	struct dirent* ent = readdir(self->pd);

	while (ent && (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0))
		ent = readdir(self->pd);

	if (!ent)
		return NULL;
	else
	{
		qn_u8to32((uchar4*)self->ufile, QN_COUNTOF(self->ufile) - 1, ent->d_name, 0);
		return self->ufile;
	}
#endif
}

//
void qn_dir_rewind(QnDir* self)
{
#ifdef _QN_WINDOWS_
	if (self->handle != INVALID_HANDLE_VALUE)
		FindClose(self->handle);

	self->handle = INVALID_HANDLE_VALUE;
	self->stat = 0;
#else
	rewinddir(self->pd);
#endif
}

//
int qn_dir_tell(QnDir* self)
{
#ifdef _QN_WINDOWS_
	return self->stat;
#elif defined _QN_ANDROID_
	return -1;
#else
	return (int)telldir(self->pd);
#endif
}

//
void qn_dir_seek(QnDir* self, const int pos)
{
#ifdef _QN_WINDOWS_
	if (pos < 0)
	{
		if (self->handle != INVALID_HANDLE_VALUE)
			FindClose(self->handle);

		self->handle = INVALID_HANDLE_VALUE;
		self->stat = -1;
	}
	else
	{
		qn_dir_rewind(self);

		while ((self->stat < pos) && qn_dir_read(self))
			;
	}
#elif _QN_ANDROID_
	int cnt = 0;
	rewinddir(self->pd);

	while (cnt < pos && readdir(self->pd))
		cnt++;

#else
	seekdir(self->pd, pos);
#endif
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
char* qn_dir_base_path(void)
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
	qn_assert(i > 0 && "베이스 패스가 잘못된거 같은데요!");
	pw[i + 1] = L'\0';

	const size_t sz = qn_u16to8(NULL, 0, pw, 0);
	path = qn_alloc(sz + 1, char);
	qn_u16to8(path, sz + 1, pw, 0);
	qn_free(pw);
#elif defined _QN_BSD_ || defined _QN_LINUX_
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
		char* p = strrchr(path, '/');
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
// 모듈

// 모듈 플래그
typedef enum QnModFlag
{
	QNMDF_SYSTEM = QN_BIT(0),			// 시스템 모듈 (load 인수로 true)를 입력했을 때 반응하도록 함
	QNMDF_RESIDENT = QN_BIT(1),			// 로드된 모듈로 만들어졌을 때
	QNMDF_MULTIPLE = QN_BIT(2),			// 한번 이상 모듈이 만들어졌을 때
	QNMDF_SELF = QN_BIT(7),				// 실행파일
} QnModFlag;

// 단위 모듈 내용
struct QnModule
{
	char*				filename;
	size_t				hash;

	enum QnModFlag		flags;
	int					ref;
#ifdef _QN_WINDOWS_
	HMODULE				handle;
#else
	void*				handle;
#endif

	QnModule*			next;
};

// 모듈 관리 구현
static struct ModuleImpl
{
	QnSpinLock			lock;
	QnModule*			self;
	QnModule*			modules;
} module_impl = { false, };

//
void qn_module_up(void)
{
	// 딱히 할건 없다
}

//
void qn_module_down(void)
{
	QnModule* node = module_impl.modules;
	while (node)
	{
		QnModule* next = node->next;
		if (QN_TMASK(node->flags, QNMDF_SYSTEM | QNMDF_RESIDENT | QNMDF_MULTIPLE) == false)
			qn_debug_outputf(false, "MODULE", "'%s' not unloaded (ref: %d)", node->filename, node->ref);
		if (QN_TMASK(node->flags, QNMDF_RESIDENT) == false && node->handle != NULL)
#ifdef _QN_WINDOWS_
			FreeLibrary(node->handle);
#else
			dlclose(node->handle);
#endif
		qn_free(node->filename);
		qn_free(node);
		node = next;
	}

	if (module_impl.self != NULL)
	{
		qn_free(module_impl.self->filename);
		qn_free(module_impl.self);
	}
}

//
static QnModule* _qn_module_find(const char* filename, const size_t hash)
{
	QnModule* find = NULL;
	QN_LOCK(module_impl.lock);
	for (QnModule* node = module_impl.modules; node; node = node->next)
	{
		if (hash != node->hash || qn_streqv(node->filename, filename) == false)
			continue;
		find = node;
		break;
	}
	QN_UNLOCK(module_impl.lock);
	return find;
}

// dlerror 처리용
static void _qn_module_set_error(void)
{
#ifndef _QN_WINDOWS_
	const char* err = dlerror();
	if (err != NULL)
		qn_set_error(err);
	else
#endif
		qn_set_syserror(0);
}

//
QnModule* qn_mod_self(void)
{
	qn_set_error(NULL);

	if (module_impl.self != NULL)
	{
		module_impl.self->ref++;
		return module_impl.self;
	}

#ifdef _QN_WINDOWS_
	HMODULE handle =
#ifdef __WINRT__
		NULL;
#else
		GetModuleHandle(NULL);
#endif
#else
	void* handle = dlopen(NULL, RTLD_GLOBAL | RTLD_LAZY);	// 이 호출에 오류가 발생하면 프로그램은 끝이다
#endif

	QnModule* self = qn_alloc_zero_1(QnModule);
	self->handle = handle;
	self->ref = 1;
	self->flags = (QnModFlag)(QNMDF_RESIDENT | QNMDF_SELF);

	QN_LOCK(module_impl.lock);
	module_impl.self = self;
	QN_UNLOCK(module_impl.lock);

	return self;
}

//
QnModule* qn_mod_load(const char* filename, const int flags)
{
	qn_val_if_fail(filename != NULL, NULL);

	qn_set_error(NULL);

	const size_t hash = qn_strhash(filename);
	QnModule* self = _qn_module_find(filename, hash);
	if (self != NULL)
	{
		self->ref++;
		QN_SMASK(&self->flags, QNMDF_MULTIPLE, true);
		return self;
	}

	QnModFlag mod_flag = (QnModFlag)flags;
#ifdef _QN_WINDOWS_
	wchar* pw = qn_u8to16_dup(filename, 0);
#ifdef __WINRT__
	HMODULE handle = LoadPackagedLibrary(pw, 0);
#else
	HMODULE handle = GetModuleHandle(pw);
	if (handle != NULL)
		mod_flag |= QNMDF_RESIDENT;
	else
		handle = LoadLibrary(pw);
#endif
	qn_free(pw);
#else
	void* handle = dlopen(filename, RTLD_LOCAL | RTLD_NOW);
#endif
	if (handle == NULL)
	{
		_qn_module_set_error();
		qn_debug_output_error(true, "MODULE");
		return NULL;
	}

	self = qn_alloc_1(QnModule);
	self->hash = hash;
	self->filename = qn_strdup(filename);
	self->ref = 1;
	self->flags = mod_flag;
	self->handle = handle;

	QN_LOCK(module_impl.lock);
	self->next = module_impl.modules;
	module_impl.modules = self;
	QN_UNLOCK(module_impl.lock);
	return self;
}

//
static void _qn_mod_free(QnModule * self, const bool dispose)
{
	if (self->handle != NULL)
	{
#ifdef _QN_WINDOWS_
		if (FreeLibrary(self->handle) == FALSE)
#else
		if (dlclose(self->handle) != 0)
#endif
			_qn_module_set_error();
	}
	if (dispose)
	{
		qn_free(self->filename);
		qn_free(self);
	}
}

//
bool qn_mod_unload(QnModule* self)
{
	qn_val_if_ok(QN_TMASK(self->flags, QNMDF_SELF), false);
	qn_val_if_fail(self->ref > 0, false);

	self->ref--;
	if (self->ref > 0)
		return true;

	QN_LOCK(module_impl.lock);
	for (QnModule *last = NULL, *node = module_impl.modules; node; )
	{
		if (node == self)
		{
			if (last)
				last->next = node->next;
			else
				module_impl.modules = node->next;
			break;
		}
		last = node;
		node = node->next;
	}
	self->next = NULL;
	QN_UNLOCK(module_impl.lock);

	_qn_mod_free(self, true);
	return true;
}

//
void* qn_mod_func(QnModule* self, const char* restrict name)
{
	qn_val_if_fail(name != NULL && *name != '\0', NULL);
#ifdef _QN_WINDOWS_
#ifdef __WINRT__
	void* ptr = (void*)GetProcAddress(NULL, name);
#else
	void* ptr = (void*)GetProcAddress(self->handle, name);
#endif
#else
	void* ptr = dlsym(self->handle, name);
#endif
	if (ptr == NULL)
		_qn_module_set_error();
	return ptr;
}

//
int qn_mod_ref(const QnModule* self)
{
	return self->ref;
}


