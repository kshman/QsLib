#include "pch.h"
#include "qn.h"
#include <fcntl.h>
#if _QN_UNIX_
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

//////////////////////////////////////////////////////////////////////////
// ����

// ���� ����ü
struct qnFile
{
	char* name;
	qnFileAccess		acs;
	int					flag;
#if _QN_WINDOWS_
	HANDLE				fd;
#else
	int					fd;
#endif
};

// �ִ� �Ҵ� ũ��
static size_t _max_file_alloc_size = 512 * 1024 * 1024;

// �м�
void qn_file_access_parse(const char* mode, qnFileAccess* self, int* flag)
{
#if _QN_WINDOWS_
	char ch;

	self->mode = 0;

	if (mode)
	{
		self->access = 0;
		self->share = FILE_SHARE_READ;
		self->attr = FILE_ATTRIBUTE_NORMAL;

		for (; *mode; ++mode)
		{
			switch (*mode)
			{
			case 'a':
				self->mode = OPEN_ALWAYS;
				self->access = GENERIC_WRITE;
				*flag = QN_FF_WRITE | QN_FF_SEEK;
				break;

			case 'r':
				self->mode = OPEN_EXISTING;
				self->access = GENERIC_READ;
				*flag = QN_FF_READ | QN_FF_SEEK;
				break;

			case 'w':
				self->mode = CREATE_ALWAYS;
				self->access = GENERIC_WRITE;
				*flag = QN_FF_WRITE | QN_FF_SEEK;
				break;

			case '+':
				self->access = GENERIC_WRITE | GENERIC_READ;
				*flag = QN_FF_READ | QN_FF_WRITE | QN_FF_SEEK;
				break;

			case '@':
				ch = *(mode + 1);

				if (ch == 'R')
					self->share = FILE_SHARE_READ, ++mode;
				else if (ch == 'W')
					self->share = FILE_SHARE_WRITE, ++mode;
				else if (ch == '+')
					self->share = FILE_SHARE_READ | FILE_SHARE_WRITE, ++mode;

				break;

			case '!':
				if (self->mode == CREATE_ALWAYS)
					self->attr |= FILE_FLAG_DELETE_ON_CLOSE;

				break;
			}
		}
	}

	if (self->mode == 0)
	{
		*flag = QN_FF_READ | QN_FF_SEEK;
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
				*flag = QN_FF_WRITE | QN_FF_SEEK;
				break;

			case 'r':
				self->mode = O_RDONLY;
				*flag = QN_FF_READ | QN_FF_SEEK;
				break;

			case 'w':
				self->mode = O_WRONLY | O_TRUNC | O_CREAT;
				*flag = QN_FF_WRITE | QN_FF_SEEK;
				break;

			case '+':
				self->mode |= O_RDWR;
				*flag = QN_FF_READ | QN_FF_WRITE | QN_FF_SEEK;
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
							strncpy_s(sz, 64 - 1, (mode + 1), p - (mode + 1));
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
		*flag = QN_FF_READ | QN_FF_SEEK;
	}

	if (self->access == 0 && (self->mode & O_CREAT) != 0)
		self->access = (S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH);
#endif
}

void qn_file_access_parse_l(const wchar_t* mode, qnFileAccess* self, int* flag)
{
#if _QN_WINDOWS_
	wchar_t ch;

	self->mode = 0;

	if (mode)
	{
		self->access = 0;
		self->share = FILE_SHARE_READ;
		self->attr = FILE_ATTRIBUTE_NORMAL;

		for (; *mode; ++mode)
		{
			switch (*mode)
			{
			case L'a':
				self->mode = OPEN_ALWAYS;
				self->access = GENERIC_WRITE;
				*flag = QN_FF_WRITE | QN_FF_SEEK;
				break;

			case L'r':
				self->mode = OPEN_EXISTING;
				self->access = GENERIC_READ;
				*flag = QN_FF_READ | QN_FF_SEEK;
				break;

			case L'w':
				self->mode = CREATE_ALWAYS;
				self->access = GENERIC_WRITE;
				*flag = QN_FF_WRITE | QN_FF_SEEK;
				break;

			case L'+':
				self->access = GENERIC_WRITE | GENERIC_READ;
				*flag = QN_FF_READ | QN_FF_WRITE | QN_FF_SEEK;
				break;

			case L'@':
				ch = *(mode + 1);

				if (ch == L'R')
					self->share = FILE_SHARE_READ, ++mode;
				else if (ch == L'W')
					self->share = FILE_SHARE_WRITE, ++mode;
				else if (ch == L'+')
					self->share = FILE_SHARE_READ | FILE_SHARE_WRITE, ++mode;

				break;

			case L'!':
				if (self->mode == CREATE_ALWAYS)
					self->attr |= FILE_FLAG_DELETE_ON_CLOSE;

				break;
			}
		}
	}

	if (self->mode == 0)
	{
		*flag = QN_FF_READ | QN_FF_SEEK;
		self->mode = OPEN_EXISTING;
		self->access = GENERIC_READ;
		self->attr = FILE_ATTRIBUTE_NORMAL;
		self->share = FILE_SHARE_READ;
	}
#else
	wchar_t ch, cm;

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
				*flag = QN_FF_WRITE | QN_FF_SEEK;
				break;

			case L'r':
				self->mode = O_RDONLY;
				*flag = QN_FF_READ | QN_FF_SEEK;
				break;

			case L'w':
				self->mode = O_WRONLY | O_TRUNC | O_CREAT;
				*flag = QN_FF_WRITE | QN_FF_SEEK;
				break;

			case L'+':
				self->mode |= O_RDWR;
				*flag = QN_FF_READ | QN_FF_WRITE | QN_FF_SEEK;
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
						const wchar_t* p = mode + 2;

						while (_k_iswdigit(*p))
							p++;

						if ((p - (mode + 1)) < 63)
						{
							wchar_t sz[64], * stop;
							wcsncpy_s(sz, 64 - 1, (mode + 1), p - (mode + 1));
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
		*flag = QN_FF_READ | QN_FF_SEEK;
	}

	if (self->access == 0 && (self->mode & O_CREAT) != 0)
		self->access = (S_IRUSR | S_IWUSR) | (S_IRGRP) | (S_IROTH);
#endif
}

/**
 * �� ���� ó�� ������ �����.
 * @param	filename	������ �̸�.
 * @param	mode		���� ó�� ���.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnFile*.
 */
qnFile* qn_file_new(const char* filename, const char* mode)
{
	qnFile* self;
#if _QN_WINDOWS_
	wchar_t uni[QN_MAX_PATH];
#endif

	qn_retval_if_fail(filename, NULL);

	self = qn_alloc_1(qnFile);
	qn_retval_if_fail(self, NULL);

	qn_file_access_parse(mode, &self->acs, &self->flag);

#if _QN_WINDOWS_
	qn_utf8to16(uni, QN_MAX_PATH - 1, filename, 0);
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

	self->name = _strdup(filename);

	return self;
}

/**
 * �����ڵ�� �� ���� ������ �����.
 * @param	filename	������ �̸�.
 * @param	mode		���� ó�� ���.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnFile*.
 */
qnFile* qn_file_new_l(const wchar_t* filename, const wchar_t* mode)
{
	qnFile* self;
	char asc[QN_MAX_PATH];

	qn_retval_if_fail(filename, NULL);

	self = qn_alloc_1(qnFile);
	qn_retval_if_fail(self, NULL);

	qn_file_access_parse_l(mode, &self->acs, &self->flag);

#if _QN_WINDOWS_
	qn_utf16to8(asc, QN_MAX_PATH - 1, filename, 0);
	self->fd = CreateFile(filename, self->acs.access, self->acs.share, NULL, self->acs.mode, self->acs.attr, NULL);

	if (!self->fd || self->fd == INVALID_HANDLE_VALUE)
#else
	qn_utf32to8(asc, QN_MAX_PATH - 1, filename, 0);
	self->fd = self->acs.access == 0 ? open(asc, self->acs.mode) : open(asc, self->acs.mode, self->acs.access);

	if (self->fd < 0)
#endif
	{
		qn_free(self);
		return NULL;
	}

	self->name = _strdup(asc);

	return self;
}

/**
 * ���� ����. �ڵ��� �����Ͽ� ���� ����� �� �ֵ��� �Ѵ�.
 * @param [�Է�,��ȯ]	src	(�ΰ��� �ƴ�) ����.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnFile*.
 */
qnFile* qn_file_new_dup(qnFile* src)
{
	qnFile* self = qn_alloc_1(qnFile);
	qn_retval_if_fail(self, NULL);

	memcpy(self, src, sizeof(qnFile));

#if _QN_WINDOWS_
	HANDLE d, p = GetCurrentProcess();
	if (!DuplicateHandle(p, self->fd, p, &d, 0, FALSE, DUPLICATE_SAME_ACCESS))
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

	self->name = _strdup(src->name);

	return self;
}

/**
 * ���� ������ �����Ѵ�. �� �̻� ���� ������ ���� ���� �� ����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 */
void qn_file_delete(qnFile* self)
{
#if _QN_WINDOWS_
	if (self->fd && self->fd != INVALID_HANDLE_VALUE)
		CloseHandle(self->fd);
#else
	if (self->fd >= 0)
		close(self->fd);
#endif

	qn_free(self->name);
	qn_free(self);
}

/**
 * ���� �÷��׸� �����´�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	mask		�÷����� ����ũ.
 * @return	���� �÷���.
 */
int qn_file_flags(qnFile* self, int mask)
{
	return (self->flag & mask) != 0;
}

/**
 * ���� �̸��� ��´�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @return	������ �̸�.
 */
const char* qn_file_name(qnFile* self)
{
	return self->name;
}

/**
 * ���Ͽ��� �д´�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	buffer  	���� �����͸� ������ ����.
 * @param	offset  	������ �ɼ�.
 * @param	size		���� ũ��.
 * @return	���� ���� ���̸� ��ȯ.
 */
int qn_file_read(qnFile* self, pointer_t buffer, int offset, int size)
{
	uint8_t* ptr;

	qn_retval_if_fail(buffer != NULL, -1);
	qn_retval_if_fail(size >= 0, 0);

	ptr = (uint8_t*)buffer;

#if _QN_WINDOWS_
	DWORD ret;
	return ReadFile(self->fd, ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return read(self->fd, ptr + offset, size);
#endif
}

/**
 * ���Ͽ� ����.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	buffer  	�� �����Ͱ� ��� �ִ� ����.
 * @param	offset  	������ �ɼ�.
 * @param	size		�� ũ��.
 * @return	���� �� ���̸� ��ȯ.
 */
int qn_file_write(qnFile* self, const pointer_t buffer, int offset, int size)
{
	const uint8_t* ptr;

	qn_retval_if_fail(buffer != NULL, -1);
	qn_retval_if_fail(size >= 0, 0);

	ptr = (const uint8_t*)buffer;

#if _QN_WINDOWS_
	DWORD ret;
	return WriteFile(self->fd, ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return write(self->fd, ptr + offset, size);
#endif
}

/**
 * ���� ũ�⸦ ��´�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @return	������ ����.
 */
int64_t qn_file_size(qnFile* self)
{
#if _QN_WINDOWS_
	LARGE_INTEGER ll;
	return GetFileSizeEx(self->fd, &ll) ? ll.QuadPart : -1;
#else
#if _QN_LINUX_
	off_t save, last;
#elif _QN_BSD_
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

/**
 * ���� ��ġ ���.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @return	���� ������ �а� ���� ��ġ.
 */
int64_t qn_file_tell(qnFile* self)
{
#if _QN_WINDOWS_
	LARGE_INTEGER ll, pl;
	pl.QuadPart = 0;
	return SetFilePointerEx(self->fd, pl, &ll, FILE_CURRENT) ? ll.QuadPart : -1;
#else
	return lseek(self->fd, 0, SEEK_CUR);
#endif
}

/**
 * ���� ��ġ ����.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	offset  	�̵��� ��ġ.
 * @param	org			��� (C���� SEEK�� ����).
 * @return	����� ��ġ.
 */
int64_t qn_file_seek(qnFile* self, int64_t offset, int org)
{
#if _QN_WINDOWS_
	LARGE_INTEGER ll;
	LARGE_INTEGER* po = (LARGE_INTEGER*)&offset;

	BOOL b = SetFilePointerEx(self->fd, *po, &ll, org);

	return b ? ll.QuadPart : -1;
#else
	return lseek(self->fd, offset, org);
#endif
}

/**
 * ���� ����. ���� ������ �����Ѵ�. ���� ������ ���� �����͸� ��� ����.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @return	�����ϸ� ��, �����ϸ� ����.
 */
bool qn_file_flush(qnFile* self)
{
#if _QN_WINDOWS_
	return FlushFileBuffers(self->fd) != 0;
#else
#if 0
	sync();
#endif
	return TRUE;
#endif
}

/**
 * ���Ͽ� ����. ���� ���.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	fmt			���� ���ڿ�.
 * @return	���� �� ����.
 */
int qn_file_printf(qnFile* self, const char* fmt, ...)
{
	char* buf;
	int len;
	va_list va;

	va_start(va, fmt);
	len = qn_vsnprintf(NULL, 0, fmt, va);
	va_end(va);

	buf = qn_alloca(len + 1, char);
	va_start(va, fmt);
	qn_vsnprintf(buf, len + 1, fmt, va);
	va_end(va);

	len = qn_file_write(self, buf, 0, len);

	qn_freea(buf);

	return len;
}

/**
 * ���Ͽ� ����, ���� ���.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	fmt			���� ���ڿ�.
 * @param	va			���� �μ�.
 * @return	���� �� ����.
 */
int qn_file_vprintf(qnFile* self, const char* fmt, va_list va)
{
	char* buf;
	int len;

	len = qn_vsnprintf(NULL, 0, fmt, va);

	buf = qn_alloca(len + 1, char);
	qn_vsnprintf(buf, len + 1, fmt, va);

	len = qn_file_write(self, buf, 0, len);

	qn_freea(buf);

	return len;
}

/**
 * ������ �ֳ� �����Ѵ�.
 * @param	filename	������ �̸�.
 * @param [��ȯ]	res 	(�ΰ��� �ƴϸ�) ���� ó�� �÷��׷� KFAS_�� �����ϴ� ����ũ �÷���.
 * @return	�����ϸ� ��, �����ϸ� ����.
 */
bool qn_file_exist(const char* filename, /*RET-NULLABLE*/bool* isdir)
{
	qn_retval_if_fail(filename, FALSE);

#if _QN_WINDOWS_
	WIN32_FIND_DATA ffd;
	HANDLE h;
	wchar_t uni[QN_MAX_PATH];

	qn_utf8to16(uni, QN_MAX_PATH - 1, filename, 0);

	h = FindFirstFileEx(uni, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);

	if (h == INVALID_HANDLE_VALUE)
		return FALSE;
	else
	{
		FindClose(h);

		if (isdir != NULL)
			*isdir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		return TRUE;
	}
#else
	struct stat s;
	kint n;

	n = stat(filename, &s);

	if (n < 0)
		return FALSE;
	else
	{
		if (isdir != NULL)
			*isdir = (s.st_mode & 0x4000) != 0;

		return TRUE;
	}
#endif
}

/**
 * ������ �ֳ� �����Ѵ�. �����ڵ� ����.
 * @param	filename	������ �̸�.
 * @param [��ȯ]	res 	(�ΰ��� �ƴϸ�) ���� ó�� �÷��׷� KFAS_�� �����ϴ� ����ũ �÷���.
 * @return	�����ϸ� ��, �����ϸ� ����.
 */
bool qn_file_exist_l(const wchar_t* filename, /*RET-NULLABLE*/bool* isdir)
{
	qn_retval_if_fail(filename, FALSE);

#if _QN_WINDOWS_
	WIN32_FIND_DATA ffd;
	HANDLE h;

	h = FindFirstFileEx(filename, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);

	if (h == INVALID_HANDLE_VALUE)
		return FALSE;
	else
	{
		FindClose(h);

		if (isdir != NULL)
			*isdir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		return TRUE;
	}
#else
	struct stat s;
	char asc[260];
	kint n;

	k_conv_uni_to_utf8(asc, 260 - 1, filename, 0);
	n = stat(asc, &s);

	if (n < 0)
		return FALSE;
	else
	{
		if (isdir != NULL)
			*isdir = (s.st_mode & 0x4000) != 0;

		return TRUE;
	}
#endif
}

/**
 * qn_file_alloc �Լ����� ����ϴ� ���� �б� �ִ� �Ҵ� ũ��.
 * @return	�ִ� �Ҵ� ũ��.
 */
size_t qn_file_get_max_alloc_size(void)
{
	return _max_file_alloc_size;
}

/**
 * qn_file_alloc �Լ����� ����� ���� �б� �ִ� �Ҵ� ũ�� ����.
 * @param	n	�Ҵ��� ũ��.
 */
void qn_file_set_max_alloc_size(size_t n)
{
	_max_file_alloc_size = n == 0 ? (512ULL * 1024ULL * 1024ULL) : n;
}

/**
 * ���� �Ҵ�. �� ���� ��ü�� �о� �޸𸮿� �Ҵ��� �� ��ȯ�Ѵ�.
 * @param	filename	������ �̸�.
 * @param [��ȯ]	size	(�ΰ��� �ƴϸ�) ���� ������ ũ��.
 * @return	���� ����. ����� ���� k_free �Լ��� �����ؾ��Ѵ�.
 */
pointer_t qn_file_alloc(const char* filename, int* size)
{
	qnFile* file;
	pointer_t buf;
	int64_t len;

	qn_retval_if_fail(filename != NULL, NULL);

	file = qn_file_new(filename, "rb");

	if (!file)
		return NULL;

	len = qn_file_size(file);

	if (!len || len > (int64_t)_max_file_alloc_size)
	{
		qn_file_delete(file);
		return NULL;
	}

	buf = qn_alloc((size_t)len + 4, uint8_t);
	qn_file_read(file, buf, 0, (int)len);

	qn_file_delete(file);

	if (size)
		*size = (int)len;

	return buf;
}

/**
 * ���� �Ҵ�. �� ���� ��ü�� �о� �޸𸮿� �Ҵ��� �� ��ȯ�Ѵ�. �����ڵ� ����.
 * @param	filename	������ �̸�.
 * @param [��ȯ]	size	(�ΰ��� �ƴϸ�) ���� ������ ũ��.
 * @return	���� ����. ����� ���� k_free �Լ��� �����ؾ��Ѵ�.
 */
pointer_t qn_file_alloc_l(const wchar_t* filename, int* size)
{
	qnFile* file;
	pointer_t buf;
	uint64_t len;

	qn_retval_if_fail(filename, NULL);

	file = qn_file_new_l(filename, L"rb");
	qn_retval_if_fail(file, NULL);

	len = qn_file_size(file);

	if (!len || len > _max_file_alloc_size)
	{
		qn_file_delete(file);
		return NULL;
	}

	buf = qn_alloc((size_t)len + 4, uint8_t);
	qn_file_read(file, buf, 0, (int)len);

	qn_file_delete(file);

	if (size)
		*size = (int)len;

	return buf;
}


//////////////////////////////////////////////////////////////////////////
// ���͸�

// ���͸� ����ü.
struct qnDir
{
#if _QN_WINDOWS_
	wchar_t*			name;
	int					stat;
	HANDLE				handle;
	WIN32_FIND_DATA		ffd;
	char				file[MAX_PATH];
#else
	DIR*				pd;
	wchar_t				ufile[QN_MAX_PATH];
#endif
};

/**
 * ���͸��� ���� �����.
 * @param	path 	���͸��� ������ ��� �̸�.
 * @param	flags	������� ����.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� ������� ��ü.
 */
qnDir* qn_dir_new(const char* path)
{
	qnDir* self;

#if _QN_WINDOWS_
	size_t len;
	wchar_t* suffix;
	wchar_t fpath[MAX_PATH];
	wchar_t uni[MAX_PATH];
	WIN32_FILE_ATTRIBUTE_DATA fad;

	qn_retval_if_fail(path != NULL, NULL);

	qn_utf8to16(uni, MAX_PATH - 1, path, 0);

	if (!GetFileAttributesEx(uni, GetFileExInfoStandard, &fad))
		return NULL;

	if (!(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return NULL;

	(void)_wfullpath(fpath, uni, MAX_PATH);

	len = wcslen(fpath) + 1/*������*/ + 1/*���Ƚ�*/ + 1/*��*/;
	suffix = qn_alloc(len, wchar_t);
	qn_retval_if_fail(suffix, NULL);

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

	self = qn_alloc_zero_1(qnDir);
	if (!self) 
	{
		qn_free(suffix);
		return NULL;
	}

	self->name = suffix;
	self->handle = INVALID_HANDLE_VALUE;
#else
	DIR* pd;

	qn_value_if_fail(path, NULL);

	pd = opendir(path);
	qn_value_if_fail(pd != NULL, NULL);

	self = qn_alloc_zero_1(qnDir);
	self->pd = pd;
#endif

	return self;
}

/**
 * ���͸��� ���� �����. (�����ڵ� ���)
 * @param	path 	���͸��� ������ ��� �̸�.
 * @param	flags	������� ����.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� ������� ��ü.
 */
qnDir* qn_dir_new_l(const wchar_t* path)
{
	qnDir* self;

#if _QN_WINDOWS_
	size_t len;
	wchar_t* suffix;
	wchar_t fpath[MAX_PATH];
	WIN32_FILE_ATTRIBUTE_DATA fad;

	qn_retval_if_fail(path != NULL, NULL);

	if (!GetFileAttributesEx(path, GetFileExInfoStandard, &fad))
		return NULL;

	if (!(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return NULL;

	(void)_wfullpath(fpath, path, MAX_PATH);

	len = wcslen(fpath) + 1/*������*/ + 1/*���Ƚ�*/ + 1/*��*/;
	suffix = qn_alloc(len, wchar_t);
	qn_retval_if_fail(suffix, NULL);

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

	self = qn_alloc_zero_1(qnDir);
	if (!self) 
	{
		qn_free(suffix);
		return NULL;
	}

	self->name = suffix;
	self->handle = INVALID_HANDLE_VALUE;
#else
	DIR* pd;
	char asc[260];

	qn_value_if_fail(path, NULL);

	qn_utf32to8(asc, 260 - 1, path, 0);
	pd = opendir(asc);
	qn_value_if_fail(pd != NULL, NULL);

	self = qn_alloc_0(qnDir);
	self->pd = pd;
#endif

	return self;
}

/**
 * ���͸� ��ü ����.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 */
void qn_dir_delete(qnDir* self)
{
#if _QN_WINDOWS_
	if (self->handle != INVALID_HANDLE_VALUE)
		FindClose(self->handle);

	qn_free(self->name);
#else
	closedir(self->pd);
#endif

	qn_free(self);
}

/**
 * ���͸����� �׸� �б�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� ���� �̸�.
 */
const char* qn_dir_read(qnDir* self)
{
#if _QN_WINDOWS_
	for (;;)
	{
		if (self->stat < 0)
			return NULL;
		else if (self->stat == 0)
		{
			// ó��
			self->handle = FindFirstFileEx(self->name, FindExInfoStandard, &self->ffd, FindExSearchNameMatch, NULL, 0);
			self->stat = self->handle == INVALID_HANDLE_VALUE ? -1 : 1;
		}
		else
		{
			// ���
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

		qn_utf16to8(self->file, MAX_PATH - 1, self->ffd.cFileName, 0);

		return self->file;
	}
#else
	struct dirent* ent;

	ent = readdir(self->pd);

	while (ent && (_strcmp(ent->d_name, ".") == 0 || _strcmp(ent->d_name, "..") == 0))
		ent = readdir(self->pd);

	return ent ? ent->d_name : NULL;
#endif
}

/**
 * ���͸����� �׸� �б�. (�����ڵ�)
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� ���� �̸�.
 */
const wchar_t* qn_dir_read_l(qnDir* self)
{
#if _QN_WINDOWS_
	for (;;)
	{
		if (self->stat < 0)
			return NULL;
		else if (self->stat == 0)
		{
			// ó��
			self->handle = FindFirstFileEx(self->name, FindExInfoStandard, &self->ffd, FindExSearchNameMatch, NULL, 0);
			self->stat = self->handle == INVALID_HANDLE_VALUE ? -1 : 1;
		}
		else
		{
			// ���
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
	struct dirent* ent;

	ent = readdir(self->pd);

	while (ent && (_strcmp(ent->d_name, ".") == 0 || _strcmp(ent->d_name, "..") == 0))
		ent = readdir(self->pd);

	if (!ent)
		return NULL;
	else
	{
		qn_utf8to32(self->ufile, 260 - 1, ent->d_name, 0);
		return self->ufile;
	}
#endif
}

/**
 * ���͸��� ù �׸����� ����.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 */
void qn_dir_rewind(qnDir* self)
{
#if _QN_WINDOWS_
	if (self->handle != INVALID_HANDLE_VALUE)
		FindClose(self->handle);

	self->handle = INVALID_HANDLE_VALUE;
	self->stat = 0;
#else
	rewinddir(self->pd);
#endif
}

/**
 * ���͸����� ���° �׸����� ���.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @return	�׸� ����.
 */
int qn_dir_tell(qnDir* self)
{
#if _QN_WINDOWS_
	return self->stat;
#elif _SB_ANDROID_
	return -1;
#else
	return (int)telldir(self->pd);
#endif
}

/**
 * ���͸����� ���� �׸����� ã�ư���.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	pos			ã�ư� ��ġ.
 */
void qn_dir_seek(qnDir* self, int pos)
{
#if _QN_WINDOWS_
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
#elif _SB_ANDROID_
	int cnt = 0;
	rewinddir(self->pd);

	while (cnt < pos && readdir(self->pd))
		cnt++;

#else
	seekdir(self->pd, pos);
#endif
}
