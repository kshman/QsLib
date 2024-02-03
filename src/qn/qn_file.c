﻿//
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

//////////////////////////////////////////////////////////////////////////
// 파일

// 파일 구조체
struct QNFILE
{
	QnGam			base;
	char*			name;
	QnFileAccess	acs;
	int				flag;
};

// 최대 할당 크기
static size_t max_file_alloc_size = 512ULL * 1024ULL * 1024ULL;

//
static void qn_file_dispose(QnGam* gam);

//
static qn_gam_vt(QNGAM) qn_file_vt =
{
	"FILE",
	qn_file_dispose,
};

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
								qn_strncpy(sz, (mode + 1), (size_t)(p - (mode + 1)));
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
QnFile* qn_open_file(const char* RESTRICT filename, const char* RESTRICT mode)
{
	qn_val_if_fail(filename, NULL);

	QnFile* self = qn_alloc_1(QnFile);
	qn_val_if_fail(self, NULL);

	qn_file_access_parse(mode, &self->acs, &self->flag);

#ifdef _QN_WINDOWS_
	wchar* uni = (wchar*)qn_u8to16_dup(filename, 0);
	HANDLE fd = CreateFile(uni, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);
	qn_free(uni);
	if (!fd || fd == INVALID_HANDLE_VALUE)
#else
	int fd = self->acs.access == 0 ? open(filename, self->acs.mode) : open(filename, self->acs.mode, self->acs.access);
	if (fd < 0)
#endif
	{
		qn_free(self);
		return NULL;
	}

	self->name = qn_strdup(filename);
	qn_set_gam_desc(self, fd);

	return qn_gam_init(self, QnFile, &qn_file_vt);
}

//
QnFile* qn_file_new_l(const wchar* RESTRICT filename, const wchar* RESTRICT mode)
{
	qn_val_if_fail(filename, NULL);

	QnFile* self = qn_alloc_1(QnFile);
	qn_val_if_fail(self, NULL);

	char cmode[64];
#ifdef _QN_WINDOWS_
	qn_u16to8(cmode, QN_COUNTOF(cmode), mode, 0);
#else
	qn_u32to8(cmode, QN_COUNTOF(cmode), (const uchar4*)mode, 0);
#endif

	qn_file_access_parse(cmode, &self->acs, &self->flag);

	char* u8;
#ifdef _QN_WINDOWS_
	u8 = qn_u16to8_dup((const uchar2*)filename, 0);
	HANDLE fd = CreateFile(filename, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);
	if (!fd || fd == INVALID_HANDLE_VALUE)
#else
	u8 = qn_u32to8_dup((const uchar4*)filename, 0);
	int fd = self->acs.access == 0 ? open(u8, self->acs.mode) : open(u8, self->acs.mode, self->acs.access);
	if (fd < 0)
#endif
	{
		qn_free(u8);
		qn_free(self);
		return NULL;
	}

	self->name = u8;
	qn_set_gam_desc(self, fd);

	return qn_gam_init(self, QnFile, &qn_file_vt);
}

//
QnFile* qn_file_dup(const QnFile* src)
{
	QnFile* self = qn_alloc_1(QnFile);
	qn_val_if_fail(self, NULL);

	memcpy(self, src, sizeof(QnFile));

#ifdef _QN_WINDOWS_
	const HANDLE p = GetCurrentProcess();
	HANDLE fd;
	if (!DuplicateHandle(p, qn_get_gam_handle(self), p, &fd, 0, false, DUPLICATE_SAME_ACCESS))
	{
		qn_free(self);
		return NULL;
	}
#else
	int fd = dup(qn_get_gam_desc_int(self));
	if (fd < 0)
	{
		qn_free(self);
		return NULL;
	}
#endif

	self->name = qn_strdup(src->name);
	qn_set_gam_desc(self, fd);

	return qn_gam_init(self, QnFile, &qn_file_vt);
}

//
static void qn_file_dispose(QnGam* gam)
{
	QnFile* self = qn_cast_type(gam, QnFile);

#ifdef _QN_WINDOWS_
	HANDLE fd = qn_get_gam_handle(self);
	if (fd && fd != INVALID_HANDLE_VALUE)
		CloseHandle(fd);
#else
	int fd = qn_get_gam_desc_int(self);
	if (fd >= 0)
		close(fd);
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
int qn_file_read(QnFile* self, void* RESTRICT buffer, const int offset, const int size)
{
	qn_val_if_fail(buffer != NULL, -1);
	qn_val_if_fail(size >= 0, 0);

	byte* ptr = (byte*)buffer;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return ReadFile(qn_get_gam_handle(self), ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return (int)read(qn_get_gam_desc_int(self), ptr + offset, (size_t)size);
#endif
}

//
int qn_file_write(QnFile* self, const void* RESTRICT buffer, const int offset, const int size)
{
	qn_val_if_fail(buffer != NULL, -1);
	qn_val_if_fail(size >= 0, 0);

	const byte* ptr = (const byte*)buffer;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return WriteFile(qn_get_gam_handle(self), ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return (int)write(qn_get_gam_desc_int(self), ptr + offset, (size_t)size);
#endif
}

//
llong qn_file_size(QnFile* self)
{
#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	return GetFileSizeEx(qn_get_gam_handle(self), &ll) ? ll.QuadPart : -1;
#else
	struct stat s;
	int n = fstat(qn_get_gam_desc_int(self), &s);
	return n < 0 ? -1 : s.st_size;
#endif
}

//
llong qn_file_tell(QnFile* self)
{
#ifdef _QN_WINDOWS_
	const LARGE_INTEGER pl = { .QuadPart = 0 };
	LARGE_INTEGER ll;
	return SetFilePointerEx(qn_get_gam_handle(self), pl, &ll, FILE_CURRENT) ? ll.QuadPart : -1;
#else
	return lseek(qn_get_gam_desc_int(self), 0, SEEK_CUR);
#endif
}

//
llong qn_file_seek(QnFile* self, const llong offset, const QnSeek org)
{
#ifdef _QN_WINDOWS_
	static_assert(FILE_BEGIN == QNSEEK_BEGIN && FILE_CURRENT == QNSEEK_CUR && FILE_END == QNSEEK_END, "QnSeek not equal to OS seek");
	const LARGE_INTEGER* po = (const LARGE_INTEGER*)&offset;
	LARGE_INTEGER ll;
	const BOOL b = SetFilePointerEx(qn_get_gam_handle(self), *po, &ll, org);
	return b ? ll.QuadPart : -1;
#else
	static_assert(SEEK_SET == QNSEEK_BEGIN && SEEK_CUR == QNSEEK_CUR && SEEK_END == QNSEEK_END, "QnSeek not equal to OS seek");
	return lseek(qn_get_gam_desc_int(self), offset, (int)org);
#endif
}

//
bool qn_file_flush(const QnFile* self)
{
#ifdef _QN_WINDOWS_
	return FlushFileBuffers(qn_get_gam_handle(self)) != 0;
#else
	return fsync(qn_get_gam_desc_int(self)) == 0;
#endif
}

//
int qn_file_printf(QnFile* self, const char* RESTRICT fmt, ...)
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
int qn_file_vprintf(QnFile* self, const char* RESTRICT fmt, va_list va)
{
	int len = qn_vsnprintf(NULL, 0, fmt, va);
	char* buf = qn_alloc(len + 1, char);
	qn_vsnprintf(buf, (size_t)len + 1, fmt, va);
	len = qn_file_write(self, buf, 0, len);
	qn_free(buf);

	return len;
}

//
bool qn_file_exist(const char* RESTRICT filename, /*RET-NULLABLE*/bool* is_dir)
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
bool qn_file_exist_l(const wchar* RESTRICT filename, /*RET-NULLABLE*/bool* is_dir)
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
_Success_(return != NULL)
void* qn_file_alloc(_In_ const char* RESTRICT filename, _Out_opt_ int* size)
{
	qn_val_if_fail(filename != NULL, NULL);

#ifdef _QN_WINDOWS_
	wchar* uni = (wchar*)qn_u8to16_dup(filename, 0);
	HANDLE fd = CreateFile(uni, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	qn_free(uni);
	if (fd == NULL || fd == INVALID_HANDLE_VALUE)
		return NULL;
	LARGE_INTEGER ll;
	if (!GetFileSizeEx(fd, &ll))
	{
		CloseHandle(fd);
		return NULL;
	}
	void* buf = qn_alloc((size_t)ll.QuadPart, byte);
	DWORD len;
	if (!ReadFile(fd, buf, (DWORD)ll.QuadPart, &len, NULL))
	{
		qn_free(buf);
		CloseHandle(fd);
		return NULL;
	}
	len = (DWORD)ll.QuadPart;
#else
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		return NULL;
	struct stat s;
	if (fstat(fd, &s) < 0)
	{
		close(fd);
		return NULL;
	}
	int len = (int)s.st_size;
	void* buf = qn_alloc(len, byte);
	if (read(fd, buf, len) < 0)
	{
		qn_free(buf);
		close(fd);
		return NULL;
	}
#endif

	if (size)
		*size = (int)len;
	return buf;
}

//
_Success_(return != NULL)
void* qn_file_alloc_l(_In_ const wchar * RESTRICT filename, _Out_opt_ int* size)
{
	qn_val_if_fail(filename != NULL, NULL);

#ifdef _QN_WINDOWS_
	HANDLE fd = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd == NULL || fd == INVALID_HANDLE_VALUE)
		return NULL;
	LARGE_INTEGER ll;
	if (!GetFileSizeEx(fd, &ll))
	{
		CloseHandle(fd);
		return NULL;
	}
	void* buf = qn_alloc((size_t)ll.QuadPart, byte);
	DWORD len;
	if (!ReadFile(fd, buf, (DWORD)ll.QuadPart, &len, NULL))
	{
		qn_free(buf);
		CloseHandle(fd);
		return NULL;
	}
	len = (DWORD)ll.QuadPart;
#else
	char* u8 = qn_u32to8_dup((const uchar4*)filename, 0);
	int fd = open(u8, O_RDONLY);
	qn_free(u8);
	if (fd < 0)
		return NULL;
	struct stat s;
	if (fstat(fd, &s) < 0)
	{
		close(fd);
		return NULL;
	}
	int len = (int)s.st_size;
	void* buf = qn_alloc(len, byte);
	if (read(fd, buf, len) < 0)
	{
		qn_free(buf);
		close(fd);
		return NULL;
	}
#endif

	if (size)
		*size = (int)len;
	return buf;
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


//////////////////////////////////////////////////////////////////////////
// 디렉토리

// 디렉토리 구조체.
struct QNDIR
{
	QnGam			base;
#ifdef _QN_WINDOWS_
	WIN32_FIND_DATA	ffd;
	wchar*			name;
	int				stat;
	char			file[MAX_PATH];
#else
	wchar			ufile[FILENAME_MAX];
#endif
};

//
static void qn_dir_dispose(QnGam * gam);

//
static qn_gam_vt(QNGAM) qn_dir_vt =
{
	"DIR",
	qn_dir_dispose,
};

//
QnDir* qn_open_dir(const char* path)
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
	qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
#else
	qn_val_if_fail(path, NULL);

	DIR* pd = opendir(path);
	qn_val_if_fail(pd != NULL, NULL);

	QnDir* self = qn_alloc_zero_1(QnDir);
	qn_set_gam_desc(self, pd);
#endif

	return qn_gam_init(self, QnDir, &qn_dir_vt);
}

//
QnDir* qn_dir_new_l(const wchar * path)
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
	qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
#else
	qn_val_if_fail(path, NULL);

	char* u8 = qn_u32to8_dup((const uchar4*)path, 0);
	DIR* pd = opendir(u8);
	qn_free(u8);
	qn_val_if_fail(pd != NULL, NULL);

	QnDir* self = qn_alloc_zero_1(QnDir);
	qn_set_gam_desc(self, pd);
#endif

	return self;
}

//
static void qn_dir_dispose(QnGam * gam)
{
	QnDir* self = qn_cast_type(gam, QnDir);

#ifdef _QN_WINDOWS_
	const HANDLE handle = qn_get_gam_handle(self);
	if (handle != INVALID_HANDLE_VALUE)
		FindClose(handle);

	qn_free(self->name);
	qn_free(self->file);
#else
	DIR* pd = qn_get_gam_desc(self, DIR*);
	closedir(pd);
	qn_free(self->ufile);
#endif

	qn_free(self);
}

//
const char* qn_dir_read(QnDir * self)
{
#ifdef _QN_WINDOWS_
	for (;;)
	{
		if (self->stat < 0)
			return NULL;
		else if (self->stat == 0)
		{
			// 처음
			const HANDLE handle = FindFirstFileEx(self->name, FindExInfoStandard, &self->ffd, FindExSearchNameMatch, NULL, 0);
			qn_set_gam_desc(self, handle);
			self->stat = handle == INVALID_HANDLE_VALUE ? -1 : 1;
		}
		else
		{
			// 계속
			if (FindNextFile(qn_get_gam_handle(self), &self->ffd))
				self->stat++;
			else
			{
				FindClose(qn_get_gam_handle(self));
				qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
				self->stat = -1;

				return NULL;
			}
		}

		if (qn_wcseqv(self->ffd.cFileName, L".") || qn_wcseqv(self->ffd.cFileName, L".."))
			continue;

		qn_u16to8(self->file, QN_COUNTOF(self->file) - 1, self->ffd.cFileName, 0);
		return self->file;
	}
#else
	DIR* pd = qn_get_gam_desc(self, DIR*);
	struct dirent* ent = readdir(pd);

	while (ent && (qn_streqv(ent->d_name, ".") || qn_streqv(ent->d_name, "..")))
		ent = readdir(pd);

	return ent ? ent->d_name : NULL;
#endif
}

//
const wchar* qn_dir_read_l(QnDir * self)
{
#ifdef _QN_WINDOWS_
	for (;;)
	{
		if (self->stat < 0)
			return NULL;
		else if (self->stat == 0)
		{
			// 처음
			const HANDLE handle = FindFirstFileEx(self->name, FindExInfoStandard, &self->ffd, FindExSearchNameMatch, NULL, 0);
			qn_set_gam_desc(self, handle);
			self->stat = handle == INVALID_HANDLE_VALUE ? -1 : 1;
		}
		else
		{
			// 계속
			if (FindNextFile(qn_get_gam_handle(self), &self->ffd))
				self->stat++;
			else
			{
				FindClose(qn_get_gam_handle(self));
				qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
				self->stat = -1;

				return NULL;
			}
		}

		if (wcscmp(self->ffd.cFileName, L".") != 0 || wcscmp(self->ffd.cFileName, L"..") != 0)
			continue;

		return self->ffd.cFileName;
	}
#else
	DIR* pd = qn_get_gam_desc(self, DIR*);
	struct dirent* ent = readdir(pd);

	while (ent && (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0))
		ent = readdir(pd);

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
void qn_dir_rewind(QnDir * self)
{
#ifdef _QN_WINDOWS_
	const HANDLE handle = qn_get_gam_handle(self);
	if (handle != INVALID_HANDLE_VALUE)
		FindClose(handle);

	qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
	self->stat = 0;
#else
	rewinddir(qn_get_gam_desc(self, DIR*));
#endif
}

//
int qn_dir_tell(QnDir * self)
{
#ifdef _QN_WINDOWS_
	return self->stat;
#elif defined _QN_ANDROID_
	return -1;
#else
	return (int)telldir(qn_get_gam_desc(self, DIR*));
#endif
}

//
void qn_dir_seek(QnDir * self, const int pos)
{
#ifdef _QN_WINDOWS_
	if (pos < 0)
	{
		const HANDLE handle = qn_get_gam_handle(self);
		if (handle != INVALID_HANDLE_VALUE)
			FindClose(handle);

		qn_set_gam_desc(self, INVALID_HANDLE_VALUE);
		self->stat = -1;
	}
	else
	{
		qn_dir_rewind(self);

		while ((self->stat < pos) && qn_dir_read(self))
		{
			// 아무것도 안함
		}
	}
#elif _QN_ANDROID_
	DIR* pd = qn_get_gam_desc(self, DIR*);
	int cnt = 0;
	rewinddir(pd);

	while (cnt < pos && readdir(pd))
		cnt++;
#else
	DIR* pd = qn_get_gam_desc(self, DIR*);
	seekdir(pd, pos);
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
