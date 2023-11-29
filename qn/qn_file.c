#include "pch.h"
#include "qn.h"
#include "qnctn.h"
#include <fcntl.h>
#if _QN_UNIX_
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

//////////////////////////////////////////////////////////////////////////
// 파일

// 파일 구조체
struct qnFile
{
	char* name;
	qnFileAccess		acs;
	int					flag;
#if _MSC_VER
	HANDLE				fd;
#else
	int					fd;
#endif
};

// 최대 할당 크기
static size_t _max_file_alloc_size = 512ULL * 1024ULL * 1024ULL;

// 분석
static void _qn_file_access_parse(const char* mode, qnFileAccess* self, int* flag)
{
#if _MSC_VER
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
		*flag = QN_FF_READ | QN_FF_SEEK;
	}

	if (self->access == 0 && (self->mode & O_CREAT) != 0)
		self->access = (S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH);
#endif
}

static void _qn_file_access_parse_l(const wchar_t* mode, qnFileAccess* self, int* flag)
{
#if _MSC_VER
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

							while (iswdigit(*p))
								p++;

							if ((p - (mode + 1)) < 63)
							{
								wchar_t sz[64], * stop;
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
		*flag = QN_FF_READ | QN_FF_SEEK;
	}

	if (self->access == 0 && (self->mode & O_CREAT) != 0)
		self->access = (S_IRUSR | S_IWUSR) | (S_IRGRP) | (S_IROTH);
#endif
}

/**
 * @brief 새 파일 처리 구조를 만든다
 * @param	filename	파일의 이름
 * @param	mode		파일 처리 모드
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnFile*
 */
qnFile* qn_file_new(const char* filename, const char* mode)
{
	qn_retval_if_fail(filename, NULL);

	qnFile* self = qn_alloc_1(qnFile);
	qn_retval_if_fail(self, NULL);

	_qn_file_access_parse(mode, &self->acs, &self->flag);

#if _MSC_VER
	wchar_t uni[QN_MAX_PATH];
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

	self->name = qn_strdup(filename);

	return self;
}

/**
 * @brief 유니코드용 새 파일 구조를 만든다
 * @param	filename	파일의 이름
 * @param	mode		파일 처리 모드
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnFile*
 */
qnFile* qn_file_new_l(const wchar_t* filename, const wchar_t* mode)
{
	qn_retval_if_fail(filename, NULL);

	qnFile* self = qn_alloc_1(qnFile);
	qn_retval_if_fail(self, NULL);

	_qn_file_access_parse_l(mode, &self->acs, &self->flag);

	char asc[QN_MAX_PATH];
#if _MSC_VER
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

	self->name = qn_strdup(asc);

	return self;
}

/**
 * @brief 파일 복제. 핸들을 복제하여 따로 사용할 수 있도록 한다
 * @param[in]	src	(널값이 아닌) 원본
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnFile*
 */
qnFile* qn_file_new_dup(qnFile* src)
{
	qnFile* self = qn_alloc_1(qnFile);
	qn_retval_if_fail(self, NULL);

	memcpy(self, src, sizeof(qnFile));

#if _MSC_VER
	HANDLE d, p = GetCurrentProcess();
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

/**
 * @brief 파일 구조를 제거한다. 더 이상 파일 관리를 하지 않을 때 사용한다
 * @param[in]	self	파일 개체
 */
void qn_file_delete(qnFile* self)
{
#if _MSC_VER
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
 * @brief 파일 플래그를 가져온다
 * @param[in]	self	파일 개체
 * @param	mask		플래그의 마스크
 * @return	파일 플래그
 */
int qn_file_flags(qnFile* self, int mask)
{
	return (self->flag & mask) != 0;
}

/**
 * @brief 파일 이름을 얻는다
 * @param[in]	self	파일 개체
 * @return	파일의 이름
 */
const char* qn_file_name(qnFile* self)
{
	return self->name;
}

/**
 * @brief 파일에서 읽는다
 * @param[in]	self	파일 개체
 * @param	buffer  	읽은 데이터를 저장할 버퍼
 * @param	offset  	버퍼의 옵셋
 * @param	size		읽을 크기
 * @return	실제 읽은 길이를 반환
 */
int qn_file_read(qnFile* self, pointer_t buffer, int offset, int size)
{
	qn_retval_if_fail(buffer != NULL, -1);
	qn_retval_if_fail(size >= 0, 0);

	uint8_t* ptr = (uint8_t*)buffer;
#if _MSC_VER
	DWORD ret;
	return ReadFile(self->fd, ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return read(self->fd, ptr + offset, size);
#endif
}

/**
 * @brief 파일에 쓴다
 * @param[in]	self	파일 개체
 * @param	buffer  	쓸 데이터가 들어 있는 버퍼
 * @param	offset  	버퍼의 옵셋
 * @param	size		쓸 크기
 * @return	실제 쓴 길이를 반환
 */
int qn_file_write(qnFile* self, cpointer_t buffer, int offset, int size)
{
	qn_retval_if_fail(buffer != NULL, -1);
	qn_retval_if_fail(size >= 0, 0);

	const uint8_t* ptr = (const uint8_t*)buffer;
#if _MSC_VER
	DWORD ret;
	return WriteFile(self->fd, ptr + offset, size, &ret, NULL) ? (int)ret : 0;
#else
	return write(self->fd, ptr + offset, size);
#endif
}

/**
 * @brief 파일 크기를 얻는다
 * @param[in]	self	파일 개체
 * @return	파일의 길이
 */
int64_t qn_file_size(qnFile* self)
{
#if _MSC_VER
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
 * @brief 파일 위치 얻기
 * @param[in]	self	파일 개체
 * @return	현재 파일의 읽고 쓰는 위치
 */
int64_t qn_file_tell(qnFile* self)
{
#if _MSC_VER
	LARGE_INTEGER ll, pl = { .QuadPart = 0 };
	return SetFilePointerEx(self->fd, pl, &ll, FILE_CURRENT) ? ll.QuadPart : -1;
#else
	return lseek(self->fd, 0, SEEK_CUR);
#endif
}

/**
 * @brief 파일 위치 변경
 * @param[in]	self	파일 개체
 * @param	offset  	이동할 위치
 * @param	org			방식 (C형식 SEEK와 동일)
 * @return	변경된 위치.
 */
int64_t qn_file_seek(qnFile* self, int64_t offset, int org)
{
#if _MSC_VER
	LARGE_INTEGER ll;
	LARGE_INTEGER* po = (LARGE_INTEGER*)&offset;
	BOOL b = SetFilePointerEx(self->fd, *po, &ll, org);
	return b ? ll.QuadPart : -1;
#else
	return lseek(self->fd, offset, org);
#endif
}

/**
 * @brief 파일 갱신. 파일 내용을 갱신한다. 쓰기 버퍼의 남은 데이터를 모두 쓴다
 * @param[in]	self	파일 개체
 * @return	성공하면 참, 실패하면 거짓
 */
bool qn_file_flush(qnFile* self)
{
#if _MSC_VER
	return FlushFileBuffers(self->fd) != 0;
#else
#if 0
	sync();
#endif
	return true;
#endif
}

/**
 * @brief 파일에 포맷된 문자열 쓰기
 * @param[in]	self	파일 개체
 * @param	fmt			포맷 문자열
 * @return	실제 쓴 길이
 */
int qn_file_printf(qnFile* self, const char* fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	int len = qn_vsnprintf(NULL, 0, fmt, va);
	va_end(va);

	char* buf = qn_alloca(len + 1, char);
	va_start(va, fmt);
	qn_vsnprintf(buf, len + 1, fmt, va);
	va_end(va);

	len = qn_file_write(self, buf, 0, len);

	qn_freea(buf);

	return len;
}

/**
 * @brief 파일에 포맷된 문자열 쓰기 가변형
 * @param[in]	self	파일 개체
 * @param	fmt			포맷 문자열
 * @param	va			가변 인수
 * @return	실제 쓴 길이
 */
int qn_file_vprintf(qnFile* self, const char* fmt, va_list va)
{
	int len = qn_vsnprintf(NULL, 0, fmt, va);

	char* buf = qn_alloca(len + 1, char);
	qn_vsnprintf(buf, len + 1, fmt, va);

	len = qn_file_write(self, buf, 0, len);

	qn_freea(buf);

	return len;
}

/**
 * @brief 파일이 있나 조사한다
 * @param	filename	파일의 이름
 * @param[out]	res 	(널값이 아니면) 파일 처리 플래그로 KFAS_로 시작하는 마스크 플래그
 * @return	성공하면 참, 실패하면 거짓
 */
bool qn_file_exist(const char* filename, /*RET-NULLABLE*/bool* isdir)
{
	qn_retval_if_fail(filename, false);

#if _MSC_VER
	wchar_t uni[QN_MAX_PATH];
	qn_utf8to16(uni, QN_MAX_PATH - 1, filename, 0);

	WIN32_FIND_DATA ffd = { 0, };
	HANDLE h = FindFirstFileEx(uni, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);

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

/**
 * @brief 파일이 있나 조사한다. 유니코드 버전
 * @param	filename	파일의 이름
 * @param[out]	res 	(널값이 아니면) 파일 처리 플래그로 KFAS_로 시작하는 마스크 플래그
 * @return	성공하면 참, 실패하면 거짓
 */
bool qn_file_exist_l(const wchar_t* filename, /*RET-NULLABLE*/bool* isdir)
{
	qn_retval_if_fail(filename, false);

#if _MSC_VER
	WIN32_FIND_DATA ffd = { 0, };
	HANDLE h = FindFirstFileEx(filename, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);

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
	qn_utf32to8(u8, 260 - 1, filename, 0);

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

/**
 * @brief qn_file_alloc 함수에서 사용하는 파일 읽기 최대 할당 크기
 * @return	최대 할당 크기
 */
size_t qn_file_get_max_alloc_size(void)
{
	return _max_file_alloc_size;
}

/**
 * @brief qn_file_alloc 함수에서 사용할 파일 읽기 최대 할당 크기 설정
 * @param	n	할당할 크기
 */
void qn_file_set_max_alloc_size(size_t n)
{
	_max_file_alloc_size = n == 0 ? (512ULL * 1024ULL * 1024ULL) : n;
}

/**
 * @brief 파일 할당. 즉, 파일 전체를 읽어 메모리에 할당한 후 반환한다
 * @param	filename	파일의 이름
 * @param[out]	size	(널값이 아니면) 읽은 파일의 크기
 * @return	읽은 버퍼. 사용한 다음 k_free 함수로 해제해야한다
 */
pointer_t qn_file_alloc(const char* filename, int* size)
{
	qn_retval_if_fail(filename != NULL, NULL);

	qnFile* file = qn_file_new(filename, "rb");
	qn_retval_if_fail(file, NULL);

	int64_t len = qn_file_size(file);
	if (!len || len > (int64_t)_max_file_alloc_size)
	{
		qn_file_delete(file);
		return NULL;
	}

	pointer_t buf = qn_alloc((size_t)len + 4, uint8_t);
	qn_file_read(file, buf, 0, (int)len);

	qn_file_delete(file);

	if (size)
		*size = (int)len;

	return buf;
}

/**
 * @brief 파일 할당. 즉, 파일 전체를 읽어 메모리에 할당한 후 반환한다. 유니코드 버전
 * @param	filename	파일의 이름
 * @param[out]	size	(널값이 아니면) 읽은 파일의 크기
 * @return	읽은 버퍼. 사용한 다음 k_free 함수로 해제해야한다
 */
pointer_t qn_file_alloc_l(const wchar_t* filename, int* size)
{
	qn_retval_if_fail(filename, NULL);

	qnFile* file = qn_file_new_l(filename, L"rb");
	qn_retval_if_fail(file, NULL);

	int64_t len = qn_file_size(file);

	if (len <= 0 || (size_t)len > _max_file_alloc_size)
	{
		qn_file_delete(file);
		return NULL;
	}

	pointer_t buf = qn_alloc((size_t)len + 4, uint8_t);
	qn_file_read(file, buf, 0, (int)len);

	qn_file_delete(file);

	if (size)
		*size = (int)len;

	return buf;
}


//////////////////////////////////////////////////////////////////////////
// 디렉토리

// 디렉토리 구조체.
struct qnDir
{
#if _MSC_VER
	wchar_t* name;
	int					stat;
	HANDLE				handle;
	WIN32_FIND_DATA		ffd;
	char				file[MAX_PATH];
#else
	DIR* pd;
	wchar_t				ufile[QN_MAX_PATH];
#endif
};

/**
 * @brief 디렉토리를 새로 만든다
 * @param	path 	디렉토리의 완전한 경로 이름
 * @param	flags	사용하지 않음
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 만들어진 개체
 */
qnDir* qn_dir_new(const char* path)
{
#if _MSC_VER
	qn_retval_if_fail(path != NULL, NULL);

	wchar_t uni[MAX_PATH];
	qn_utf8to16(uni, MAX_PATH - 1, path, 0);

	WIN32_FILE_ATTRIBUTE_DATA fad = { 0, };
	if (!GetFileAttributesEx(uni, GetFileExInfoStandard, &fad))
		return NULL;
	if (!(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return NULL;

	wchar_t fpath[MAX_PATH];
	(void)_wfullpath(fpath, uni, MAX_PATH);

	size_t len = wcslen(fpath) + 1/*슬래시*/ + 1/*서픽스*/ + 1/*널*/;
	wchar_t* suffix = qn_alloc(len, wchar_t);
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

	qnDir* self = qn_alloc_zero_1(qnDir);
	if (!self)
	{
		qn_free(suffix);
		return NULL;
	}

	self->name = suffix;
	self->handle = INVALID_HANDLE_VALUE;
#else
	qn_retval_if_fail(path, NULL);

	DIR* pd = opendir(path);
	qn_retval_if_fail(pd != NULL, NULL);

	qnDir* self = qn_alloc_zero_1(qnDir);
	self->pd = pd;
#endif

	return self;
}

/**
 * @brief 디렉토리를 새로 만든다 (유니코드 사용)
 * @param	path 	디렉토리의 완전한 경로 이름
 * @param	flags	사용하지 않음
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 만들어진 개체
 */
qnDir* qn_dir_new_l(const wchar_t* path)
{
#if _MSC_VER
	qn_retval_if_fail(path != NULL, NULL);

	WIN32_FILE_ATTRIBUTE_DATA fad = { 0, };
	if (!GetFileAttributesEx(path, GetFileExInfoStandard, &fad))
		return NULL;
	if (!(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return NULL;

	wchar_t fpath[MAX_PATH];
	(void)_wfullpath(fpath, path, MAX_PATH);

	size_t len = wcslen(fpath) + 1/*슬래시*/ + 1/*서픽스*/ + 1/*널*/;
	wchar_t* suffix = qn_alloc(len, wchar_t);
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

	qnDir* self = qn_alloc_zero_1(qnDir);
	if (!self)
	{
		qn_free(suffix);
		return NULL;
	}

	self->name = suffix;
	self->handle = INVALID_HANDLE_VALUE;
#else
	qn_retval_if_fail(path, NULL);

	char asc[260];
	qn_utf32to8(asc, 260 - 1, path, 0);
	DIR* pd = opendir(asc);
	qn_retval_if_fail(pd != NULL, NULL);

	qnDir* self = qn_alloc_zero_1(qnDir);
	self->pd = pd;
#endif

	return self;
}

/**
 * @brief 디렉토리 개체 제거
 * @param[in]	self	디렉토리 개체
 */
void qn_dir_delete(qnDir* self)
{
#if _MSC_VER
	if (self->handle != INVALID_HANDLE_VALUE)
		FindClose(self->handle);

	qn_free(self->name);
#else
	closedir(self->pd);
#endif

	qn_free(self);
}

/**
 * @brief 디렉토리에서 항목 읽기
 * @param[in]	self	디렉토리 개체
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 파일 이름
 */
const char* qn_dir_read(qnDir* self)
{
#if _MSC_VER
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

		qn_utf16to8(self->file, MAX_PATH - 1, self->ffd.cFileName, 0);

		return self->file;
	}
#else
	struct dirent* ent = readdir(self->pd);

	while (ent && (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0))
		ent = readdir(self->pd);

	return ent ? ent->d_name : NULL;
#endif
}

/**
 * @brief 디렉토리에서 항목 읽기 (유니코드)
 * @param[in]	self	디렉토리 개체
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 파일 이름
 */
const wchar_t* qn_dir_read_l(qnDir* self)
{
#if _MSC_VER
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
		qn_utf8to32(self->ufile, 260 - 1, ent->d_name, 0);
		return self->ufile;
	}
#endif
}

/**
 * @brief 디렉토리를 첫 항목으로 감기
 * @param[in]	self	디렉토리 개체
 */
void qn_dir_rewind(qnDir* self)
{
#if _MSC_VER
	if (self->handle != INVALID_HANDLE_VALUE)
		FindClose(self->handle);

	self->handle = INVALID_HANDLE_VALUE;
	self->stat = 0;
#else
	rewinddir(self->pd);
#endif
}

/**
 * @brief 디렉토리에서 몇번째 항목인지 얻기
 * @param[in]	self	디렉토리 개체
 * @return	항목 순서
 */
int qn_dir_tell(qnDir* self)
{
#if _MSC_VER
	return self->stat;
#elif _QN_ANDROID_
	return -1;
#else
	return (int)telldir(self->pd);
#endif
}

/**
 * @brief 디렉토리에서 순서 항목으로 찾아가기
 * @param[in]	self	디렉토리 개체
 * @param	pos			찾아갈 위치
 */
void qn_dir_seek(qnDir* self, int pos)
{
#if _MSC_VER
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
