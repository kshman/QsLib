#include "pch.h"
#include "qs_qn.h"
#include <fcntl.h>
#ifdef _QN_UNIX_
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
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
	QN_PADDING_64(4, 0)
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
								char sz[64], * stop;
								strncpy(sz, (mode + 1), p - (mode + 1));
								self->access = strtol(sz, &stop, 8);
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

static void qn_file_access_parse_l(const wchar* mode, QnFileAccess* self, int* flag)
{
#ifdef _QN_WINDOWS_
	self->mode = 0;

	if (mode)
	{
		self->access = 0;
		self->share = FILE_SHARE_READ;
		self->attr = FILE_ATTRIBUTE_NORMAL;

		for (wchar ch; *mode; ++mode)
		{
			switch (*mode)
			{
				case L'a':
					self->mode = OPEN_ALWAYS;
					self->access = GENERIC_WRITE;
					*flag = QNFF_WRITE | QNFF_SEEK;
					break;

				case L'r':
					self->mode = OPEN_EXISTING;
					self->access = GENERIC_READ;
					*flag = QNFF_READ | QNFF_SEEK;
					break;

				case L'w':
					self->mode = CREATE_ALWAYS;
					self->access = GENERIC_WRITE;
					*flag = QNFF_WRITE | QNFF_SEEK;
					break;

				case L'+':
					self->access = GENERIC_WRITE | GENERIC_READ;
					*flag = QNFF_READ | QNFF_WRITE | QNFF_SEEK;
					break;

				case L'@':
					ch = *(mode + 1);

					if (ch == L'R')
					{
						self->share = FILE_SHARE_READ;
						++mode;
					}
					else if (ch == L'W')
					{
						self->share = FILE_SHARE_WRITE;
						++mode;
					}
					else if (ch == L'+')
					{
						self->share = FILE_SHARE_READ | FILE_SHARE_WRITE;
						++mode;
					}
					break;

				case L'!':
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
	wchar ch, cm;

	self->mode = 0;
	self->access = 0;

	if (mode)
	{
		for (; *mode; ++mode)
		{
			switch (*mode)
			{
				case L'a':
					self->mode = O_WRONLY | O_APPEND | O_CREAT;
					*flag = QNFF_WRITE | QNFF_SEEK;
					break;

				case L'r':
					self->mode = O_RDONLY;
					*flag = QNFF_READ | QNFF_SEEK;
					break;

				case L'w':
					self->mode = O_WRONLY | O_TRUNC | O_CREAT;
					*flag = QNFF_WRITE | QNFF_SEEK;
					break;

				case L'+':
					self->mode |= O_RDWR;
					*flag = QNFF_READ | QNFF_WRITE | QNFF_SEEK;
					break;

				case L'@':
					cm = *(mode + 1);

					if (cm != L'\0')
					{
						if (cm == L'U')
						{
							ch = *(mode + 2);

							if (ch != L'\0')
							{
								if (ch == L'R')
									self->access |= S_IRUSR;
								else if (ch == L'W')
									self->access |= S_IWUSR;
								else if (ch == L'X')
									self->access |= S_IXUSR;

								mode++;
							}

							mode++;
						}
						else if (cm == L'G')
						{
							ch = *(mode + 2);

							if (ch != L'\0')
							{
								if (ch == L'R')
									self->access |= S_IRGRP;
								else if (ch == L'W')
									self->access |= S_IWGRP;
								else if (ch == L'X')
									self->access |= S_IXGRP;

								mode++;
							}

							mode++;
						}
						else if (cm == L'O')
						{
							ch = *(mode + 2);

							if (ch != L'\0')
							{
								if (ch == L'R')
									self->access |= S_IROTH;
								else if (ch == L'W')
									self->access |= S_IWOTH;
								else if (ch == L'X')
									self->access |= S_IXOTH;

								mode++;
							}

							mode++;
						}
						else if (iswdigit(cm))
						{
							const wchar* p = mode + 2;

							while (iswdigit(*p))
								p++;

							if ((p - (mode + 1)) < 63)
							{
								wchar sz[64], * stop;
								wcsncpy(sz, (mode + 1), p - (mode + 1));
								self->access = wcstol(sz, &stop, 8);
							}

							mode = p;
						}
					}

					break;

				case L'!':
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
		self->access = (S_IRUSR | S_IWUSR) | (S_IRGRP) | (S_IROTH);
#endif
}

//
QnFile* qn_file_new(const char* filename, const char* mode)
{
	qn_val_if_fail(filename, NULL);

	QnFile* self = qn_alloc_1(QnFile);
	qn_val_if_fail(self, NULL);

	qn_file_access_parse(mode, &self->acs, &self->flag);

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	qn_u8to16(uni, QN_MAX_PATH - 1, filename, 0);
	self->fd = CreateFile(uni, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);

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
QnFile* qn_file_new_l(const wchar* filename, const wchar* mode)
{
	qn_val_if_fail(filename, NULL);

	QnFile* self = qn_alloc_1(QnFile);
	qn_val_if_fail(self, NULL);

	qn_file_access_parse_l(mode, &self->acs, &self->flag);

	char asc[QN_MAX_PATH];
#ifdef _QN_WINDOWS_
	qn_u16to8(asc, QN_MAX_PATH - 1, filename, 0);
	self->fd = CreateFile(filename, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);

	if (!self->fd || self->fd == INVALID_HANDLE_VALUE)
#else
	qn_u32to8(asc, QN_MAX_PATH - 1, filename, 0);
	self->fd = self->acs.access == 0 ? open(asc, self->acs.mode) : open(asc, self->acs.mode, self->acs.access);

	if (self->fd < 0)
#endif
	{
		qn_free(self);
		return NULL;
	}

	self->name = qn_strdup(asc);

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
int qn_file_get_flags(const QnFile* self, int mask)
{
	return (self->flag & mask) != 0;
}

//
const char* qn_file_get_name(const QnFile* self)
{
	return self->name;
}

//
int qn_file_read(QnFile* self, void* buffer, int offset, int size)
{
	qn_val_if_fail(buffer != NULL, -1);
	qn_val_if_fail(size >= 0, 0);

	uint8_t* ptr = (uint8_t*)buffer;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return ReadFile(self->fd, ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return read(self->fd, ptr + offset, size);
#endif
}

//
int qn_file_write(QnFile* self, const void* buffer, int offset, int size)
{
	qn_val_if_fail(buffer != NULL, -1);
	qn_val_if_fail(size >= 0, 0);

	const uint8_t* ptr = (const uint8_t*)buffer;
#ifdef _QN_WINDOWS_
	DWORD ret;
	return WriteFile(self->fd, ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return write(self->fd, ptr + offset, size);
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
llong qn_file_seek(QnFile* self, llong offset, QnSeek org)
{
#ifdef _QN_WINDOWS_
	const LARGE_INTEGER* po = (LARGE_INTEGER*)&offset;
	LARGE_INTEGER ll;
	const BOOL b = SetFilePointerEx(self->fd, *po, &ll, org);
	return b ? ll.QuadPart : -1;
#else
	return lseek(self->fd, offset, org);
#endif
}

//
bool qn_file_flush(const QnFile* self)
{
#ifdef _QN_WINDOWS_
	return FlushFileBuffers(self->fd) != 0;
#else
#if 0
	sync();
#endif
	return true;
#endif
}

//
int qn_file_printf(QnFile* self, const char* fmt, ...)
{
	va_list va, vq;

	va_start(va, fmt);
	va_copy(vq, va);
	int len = qn_vsnprintf(NULL, 0, fmt, vq);
	va_end(vq);

	char* buf = qn_alloc(len + 1, char);
	qn_vsnprintf(buf, len + 1, fmt, va);
	va_end(va);
	len = qn_file_write(self, buf, 0, len);
	qn_free(buf);

	return len;
}

//
int qn_file_vprintf(QnFile* self, const char* fmt, va_list va)
{
	int len = qn_vsnprintf(NULL, 0, fmt, va);
	char* buf = qn_alloc(len + 1, char);
	qn_vsnprintf(buf, len + 1, fmt, va);
	len = qn_file_write(self, buf, 0, len);
	qn_free(buf);

	return len;
}

//
bool qn_file_exist(const char* filename, /*RET-NULLABLE*/bool* isdir)
{
	qn_val_if_fail(filename, false);

#ifdef _QN_WINDOWS_
	wchar uni[QN_MAX_PATH];
	qn_u8to16(uni, QN_MAX_PATH - 1, filename, 0);

	WIN32_FIND_DATA ffd = { 0, };
	const HANDLE h = FindFirstFileEx(uni, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);

	if (h == INVALID_HANDLE_VALUE)
		return false;
	else
	{
		FindClose(h);

		if (isdir != NULL)
			*isdir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		return true;
	}
#else
	struct stat s;
	int n = stat(filename, &s);

	if (n < 0)
		return false;
	else
	{
		if (isdir != NULL)
			*isdir = (s.st_mode & 0x4000) != 0;

		return true;
	}
#endif
}

//
bool qn_file_exist_l(const wchar* filename, /*RET-NULLABLE*/bool* isdir)
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

		if (isdir != NULL)
			*isdir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		return true;
	}
#else
	char u8[260];
	qn_u32to8(u8, 260 - 1, filename, 0);

	struct stat s;
	int n = stat(u8, &s);

	if (n < 0)
		return false;
	else
	{
		if (isdir != NULL)
			*isdir = (s.st_mode & 0x4000) != 0;

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
void qn_file_set_max_alloc_size(size_t n)
{
	max_file_alloc_size = n == 0 ? (512ULL * 1024ULL * 1024ULL) : n;
}

//
void* qn_file_alloc(const char* filename, int* size)
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

	void* buf = qn_alloc((size_t)len + 4, uint8_t);
	qn_file_read(file, buf, 0, (int)len);

	qn_file_delete(file);

	if (size)
		*size = (int)len;

	return buf;
}

//
void* qn_file_alloc_l(const wchar* filename, int* size)
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

	void* buf = qn_alloc((size_t)len + 4, uint8_t);
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
	wchar			ufile[QN_MAX_PATH];
#endif
};

//
QnDir* qn_dir_new(const char* path)
{
#ifdef _QN_WINDOWS_
	qn_val_if_fail(path != NULL, NULL);

	wchar uni[MAX_PATH];
	qn_u8to16(uni, MAX_PATH - 1, path, 0);

	WIN32_FILE_ATTRIBUTE_DATA fad = { 0, };
	if (!GetFileAttributesEx(uni, GetFileExInfoStandard, &fad))
		return NULL;
	if (!(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return NULL;

	wchar fpath[MAX_PATH];
	(void)_wfullpath(fpath, uni, MAX_PATH);  // NOLINT

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

	wchar fpath[MAX_PATH];
	(void)_wfullpath(fpath, path, MAX_PATH);

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

	char asc[260];
	qn_u32to8(asc, 260 - 1, path, 0);
	DIR* pd = opendir(asc);
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
#else
	closedir(self->pd);
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

		qn_u16to8(self->file, MAX_PATH - 1, self->ffd.cFileName, 0);

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
		qn_u8to32(self->ufile, 260 - 1, ent->d_name, 0);
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
void qn_dir_seek(QnDir* self, int pos)
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
#ifndef _QN_WINDOWS_
static char* qn_internal_read_sym_link(const char* path)
{
	char* buf = NULL;
	ssize_t len = 64;
	ssize_t rc = -1;
	while (true)
	{
		buf = qn_realloc(buf, (size_t)len, char);
		rc = readlink(path, buf, len);
		if (rc == -1)
			break;
		else if (rc < len)
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
	qn_assert(i > 0, "invalid base path");
	pw[i + 1] = L'\0';

	size_t sz = qn_u16to8(NULL, 0, pw, 0);
	path = qn_alloc(sz + 1, char);
	qn_u16to8(path, sz + 1, pw, 0);
	qn_free(pw);
#else
#if defined _QN_FREEBSD_
	QN_STMT_BEGIN{
		char fullpath[MAX_PATH];
		size_t len = QN_COUNTOF(fullpath);
		const int sns[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
		if (sysctl(sns, QN_COUNTOF(sns), fullpath, &len, NULL, 0) != -1)
			path = qn_strdup(fullpath);
	}QN_STMT_END;
#endif
	if (path == NULL && (access("/proc", F_OK) == 0))
	{
#if defined _QN_FREEBSD_
		path = qn_internal_read_sym_link("/proc/curproc/file");
#elif defined _QN_LINUX_
		path = qn_internal_read_sym_link("/proc/self/exe");
		if (path == NULL)
		{
			char path[64];
			int rc = qn_snprintf(path, QN_COUNTOF(path), "/proc/%llu/exe", (ullong)getpid());
			if (rc > 0 && rc < QN_COUNTOF(path))
				path = qn_internal_read_sym_link(path);
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
#endif
	return path;
}


//////////////////////////////////////////////////////////////////////////
// 모듈

enum QnModFlag
{
	QNMDF_INFO = QN_BIT(1),
	QNMDF_NO_CLOSURE = QN_BIT(2),
	QNMDF_RESIDENT = QN_BIT(3),
};

struct QnModule
{
	char*				filename;
	int					ref;
	int					flags;
#ifdef _QN_WINDOWS_
	HMODULE				handle;
#else
	void*				handle;
#endif

	QnModule*			next;
};

static struct QnModuleImpl
{
	BOOL				inited;
	QnModule*			self;
	QnModule*			modules;
	QnTls*				error;
	QnSpinLock			lock;
}
_qn_mod = { false, };

//
static void qn_internal_mod_delete(QnModule* self, bool dispose);

//
static void qn_module_dispose(void* dummy)
{
	qn_ret_if_fail(_qn_mod.inited);

	QnModule *next, *node = _qn_mod.modules;
	while (node)
	{
		if (QN_TMASK(node->flags, QNMDF_NO_CLOSURE | QNMDF_INFO))
			qn_debug_outputf(false, "QNMODULE", "module not deleted [%s] (ref: %d)", node->filename, node->ref);
		next = node->next;
		qn_internal_mod_delete(node, true);
		node = next;
	}

	if (_qn_mod.self != NULL)
	{
		qn_free(_qn_mod.self->filename);
		qn_free(_qn_mod.self);
	}
}

//
static void qn_module_init(void)
{
	qn_ret_if_ok(_qn_mod.inited);
	_qn_mod.inited = true;
	_qn_mod.error = qn_tls(qn_mpffree);
	qn_internal_atexit(qn_module_dispose, NULL);
}

//
static void qn_internal_mod_reset_error(void)
{
	qn_module_init();
	char* prev = (char*)qn_tlsget(_qn_mod.error);
	qn_free(prev);
	qn_tlsset(_qn_mod.error, NULL);
}

//
static void qn_internal_mod_set_error(void)
{
	qn_module_init();
	char* prev = (char*)qn_tlsget(_qn_mod.error);
	qn_free(prev);
#ifdef _QN_WINDOWS_
	qn_tlsset(_qn_mod.error, qn_syserr(0, NULL));
#else
	const char* err = dlerror();
	qn_tlsset(_qn_mod.error, err == NULL ? qn_syserr(0, NULL) : qn_strdup(err));
#endif
}

//
#ifdef _QN_WINDOWS_
static QnModule* qn_internal_find_by_handle(HMODULE handle)
#else
static QnModule* qn_internal_find_by_handle(void* handle)
#endif
{
	QnModule* find = NULL;
	QN_LOCK(_qn_mod.lock);
	if (_qn_mod.self && _qn_mod.self->handle == handle)
		find = _qn_mod.self;
	else
	{
		for (QnModule* node = _qn_mod.modules; node; node = node->next)
		{
			if (node->handle != handle)
				continue;
			find = node;
			break;
		}
	}
	QN_UNLOCK(_qn_mod.lock);
	return find;
}

//
static QnModule* qn_internal_find_by_filename(const char* filename)
{
	QnModule* find = NULL;
	QN_LOCK(_qn_mod.lock);
	for (QnModule* node = _qn_mod.modules; node; node = node->next)
	{
		if (qn_streqv(node->filename, filename) == false)
			continue;
		find = node;
		break;
	}
	QN_UNLOCK(_qn_mod.lock);
	return find;
}

//
QnModule* qn_mod_self(void)
{
	qn_internal_mod_reset_error();

	if (_qn_mod.self != NULL)
	{
		_qn_mod.self->ref++;
		return _qn_mod.self;
	}

#ifdef _QN_WINDOWS_
	HMODULE handle = (HMODULE)0x9;
#else
	void* handle = dlopen(NULL, RTLD_GLOBAL | RTLD_LAZY);
	if (handle == NULL)
	{
		qn_internal_mod_set_error();
		return NULL;
}
#endif

	QnModule* self = qn_alloc_zero_1(QnModule);
	self->handle = handle;
	self->ref = 1;
	self->flags = QNMDF_RESIDENT;

	QN_LOCK(_qn_mod.lock);
	_qn_mod.self = self;
	QN_UNLOCK(_qn_mod.lock);

	return self;
}

//
QnModule* qn_mod_open(const char* filename, int flags)
{
	qn_val_if_fail(filename != NULL, NULL);

	qn_internal_mod_reset_error();

	QnModule* self = qn_internal_find_by_filename(filename);
	if (self != NULL)
	{
		self->ref++;
		self->flags = QNMDF_NO_CLOSURE | QNMDF_RESIDENT;
		return self;
	}

#ifdef _QN_WINDOWS_
	wchar* pw;
	size_t size = qn_u8to16(NULL, 0, filename, 0) + 1;
	pw = qn_alloc(size, wchar);
	qn_u8to16(pw, size, filename, 0);
#ifdef __WINRT__
	HMODULE handle = LoadPackagedLibrary(pw, 0);
#else
	HMODULE handle = LoadLibrary(pw);
#endif
	qn_free(pw);
#else
	void* handle = dlopen(filename, RTLD_LOCAL | RTLD_NOW);
#endif
	if (handle == NULL)
	{
		qn_debug_output_syserr(true, "QNMODULE", 0);
		return NULL;
	}

	self = qn_internal_find_by_handle(handle);
	if (self != NULL)
	{
		qn_internal_mod_delete(self, false);
		self->ref++;
		self->flags |= QNMDF_NO_CLOSURE | QNMDF_RESIDENT | QNMDF_INFO;
		return self;
	}

	self = qn_alloc_1(QnModule);
	self->filename = qn_strdup(filename);
	self->ref = 1;
	self->flags = flags;
	self->handle = handle;

	QN_LOCK(_qn_mod.lock);
	self->next = _qn_mod.modules;
	_qn_mod.modules = self;
	QN_UNLOCK(_qn_mod.lock);
	return self;
}

//
static void qn_internal_mod_delete(QnModule* self, bool dispose)
{
	if (self->handle != NULL)
	{
#ifdef _QN_WINDOWS_
		if (FreeLibrary(self->handle) == FALSE)
#else
		if (dlclose(self->handle) != 0)
#endif
			qn_internal_mod_set_error();
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
	qn_val_if_fail(self->ref > 0, false);

	self->ref--;
	if (self->ref > 0)
		return true;

	QN_LOCK(_qn_mod.lock);
	for (QnModule *last = NULL, *node = _qn_mod.modules; node; )
	{
		if (node == self)
		{
			if (last)
				last->next = node->next;
			else
				_qn_mod.modules = node->next;
			break;
		}
		last = node;
		node = node->next;
	}
	self->next = NULL;
	QN_UNLOCK(_qn_mod.lock);

	qn_internal_mod_delete(self, true);
	return true;
}

//
void* qn_mod_func(QnModule* self, const char* name)
{
	qn_val_if_fail(name != NULL && *name != '\0', NULL);

	qn_internal_mod_reset_error();

	void* ptr;
#ifdef _QN_WINDOWS_
	if (self->handle != (HMODULE)0x9)
		ptr = (void*)GetProcAddress(self->handle, name);
	else
	{
#ifdef __WINRT__
		ptr = (void*)GetProcAddress(NULL, name);
#else
		ptr = (void*)GetProcAddress(GetModuleHandle(NULL), name);
#endif
	}
#else
	const char* err = dlerror();
	ptr = dlsym(self->handle, name);
#endif

	if (ptr == NULL)
		qn_internal_mod_set_error();
	return ptr;
}

//
const char* qn_mod_error(void)
{
	qn_module_init();
	return (const char*)qn_tlsget(_qn_mod.error);
}

//
int qn_mod_ref(QnModule* self)
{
	return self->ref;
}


