#include "pch.h"
#include "qn.h"
#include "PatrickPowell_snprintf.h"

//////////////////////////////////////////////////////////////////////////
// 그냥 문자열

//
static void _asc_output(pointer_t ptr, size_t* currlen, size_t maxlen, int c)
{
	char* buffer = (char*)ptr;
	if (*currlen < maxlen)
		buffer[(*currlen)] = (char)c;
	(*currlen)++;
}

//
static void _asc_output_closure(pointer_t ptr, size_t currlen, size_t maxlen)
{
	char* buffer = (char*)ptr;
	if (currlen < maxlen - 1)
		buffer[currlen] = '\0';
	else if (maxlen > 0)
		buffer[maxlen - 1] = '\0';
}

/** vsnprintf */
int qn_vsnprintf(char* out, size_t len, const char* fmt, va_list va)
{
	size_t res;

	qn_retval_if_fail(fmt != NULL, -1);

	res = dopr(_asc_output, _asc_output_closure, out, len, fmt, va);

	if (len)
		len--;

	return out ? (int)QN_MIN(res, len) : (int)res;
}

/** vaprintf */
int qn_vasprintf(char** out, const char* fmt, va_list va)
{
	size_t len;

	qn_retval_if_fail(out != NULL, -2);
	qn_retval_if_fail(fmt != NULL, -1);

	len = dopr(_asc_output, _asc_output_closure, NULL, 0, fmt, va);
	if (len <= 0)
	{
		*out = NULL;
		return 0;
	}

	*out = qn_alloc(len + 1, char);
	len = qn_vsnprintf(*out, len + 1, fmt, va);

	return (int)len;
}

/** vaprintf */
char* qn_vapsprintf(const char* fmt, va_list va)
{
	size_t len;
	char* ret;

	qn_retval_if_fail(fmt != NULL, NULL);

	len = dopr(_asc_output, _asc_output_closure, NULL, 0, fmt, va);
	if (len <= 0)
		return NULL;

	ret = qn_alloc(len + 1, char);
	qn_vsnprintf(ret, len + 1, fmt, va);

	return ret;
}

/** snprintf */
int qn_snprintf(char* out, size_t len, const char* fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = qn_vsnprintf(out, len, fmt, va);
	va_end(va);

	return ret;
}

/** asprintf */
int qn_asprintf(char** out, const char* fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = qn_vasprintf(out, fmt, va);
	va_end(va);

	return ret;
}

/** asprintf */
char* qn_apsprintf(const char* fmt, ...)
{
	va_list va;
	char* ret;

	va_start(va, fmt);
	ret = qn_vapsprintf(fmt, va);
	va_end(va);

	return ret;
}

/** 글자로 채우기 */
size_t qn_strfll(char* dest, size_t pos, size_t end, int ch)
{
	size_t i, cnt;
	if (pos >= end)
		return pos;
	cnt = end - pos;
	for (i = 0; i < cnt; i++)
		dest[pos + i] = (char)ch;
	return pos + cnt;
}

/** 문자열 해시 */
size_t qn_strhash(const char* p)
{
	const char* sz = p;
	size_t h = *sz++;
	if (!h)
		return 0;
	else
	{
		size_t c;
		for (c = 0; *sz && c < 256; sz++, c++)
			h = (h << 5) - h + *sz;
		h = (h << 5) - h + c;
		return h;
	}
}

/** 문자열 해시 (대소문자구별안함) */
size_t qn_strihash(const char* p)
{
	const char* sz = p;
	size_t h = toupper(*sz);
	if (!h)
		return 0;
	else
	{
		size_t c;
		sz++;
		for (c = 0; *sz && c < 256; sz++, c++)
			h = (h << 5) - h + toupper(*sz);
		h = (h << 5) - h + c;
		return h;
	}
}

/** strbrk */
char* qn_strbrk(const char* p, const char* c)
{
#if 1
	// microsoft c library 13.10
	const char* str = p;
	const char* ctn = c;
	char map[32] = { 0, };

	while (*ctn)
	{
		map[*ctn >> 3] |= (1 << (*ctn & 7));
		ctn++;
	}

	while (*str)
	{
		if (map[*str >> 3] & (1 << (*str & 7)))
			return ((char*)str);
		str++;
	}

	return NULL;
#else
	char* t;
	while (*p)
	{
		for (t = (char*)c; *t; t++)
		{
			if (*t == *p)
				return (char*)p;
		}
		p++;
	}
	return NULL;
#endif
}

/** strmid */
char* qn_strmid(char* dest, size_t destsize, const char* src, size_t pos, size_t len)
{
	size_t size = strlen(src);

	if (pos > size)
		*dest = '\0';
	else
	{
		strncpy_s(dest, destsize, src + pos, len);
		*(dest + len) = '\0';
	}

	return dest;
}

/** strltrim */
char* qn_strltrim(char* dest)
{
	char* s;
	for (s = dest; *s && isspace(*s); s++);
	if (dest != s)
		memmove(dest, s, strlen(s) + 1);
	return dest;
}

/** strrtrim */
char* qn_strrtrim(char* dest)
{
	size_t len = strlen(dest);
	while (len--)
	{
		if (!isspace(dest[len]))
			break;
		dest[len] = '\0';
	}
	return dest;
}

/** strtrim */
char* qn_strtrim(char* dest)
{
	return qn_strrtrim(qn_strltrim(dest));
}

/** strrem */
char* qn_strrem(char* p, const char* rmlist)
{
	char* p1 = p;
	char* p2 = p;

	while (*p1)
	{
		const char* ps = (const char*)rmlist;
		bool b = FALSE;

		while (*ps)
		{
			if (*p1 == *ps)
			{
				b = TRUE;
				break;
			}
			++ps;
		}

		if (!b)
		{
			*p2 = *p1;
			++p2;
		}

		++p1;
	}

	*p2 = '\0';

	return p;
}

static char* _strpcpy(char* dest, const char* src)
{
	do (*dest++ = *src);
	while (*src++ != '\0');
	return dest - 1;
}

/** 여러 문자열의 strdup */
char* qn_strcat(const char* p, ...)
{
	va_list va;
	char* str;
	char* s, *c;
	size_t size;

	size = strlen(p) + 1;
	va_start(va, p);
	s = va_arg(va, char*);
	while (s)
	{
		size += strlen(s);
		s = va_arg(va, char*);
	}
	va_end(va);

	str = qn_alloc(size, char);
	c = str;

	c = _strpcpy(c, p);
	va_start(va, p);
	s = va_arg(va, char*);
	while (s)
	{
		c = _strpcpy(c, s);
		s = va_arg(va, char*);
	}
	va_end(va);

	return str;
}


//////////////////////////////////////////////////////////////////////////
// 유니코드 문자열
//
static void _wcs_output(pointer_t ptr, size_t* currlen, size_t maxlen, int c)
{
	wchar_t* buffer = (wchar_t*)ptr;
	if (*currlen < maxlen)
		buffer[(*currlen)] = (wchar_t)c;
	(*currlen)++;
}

//
static void _wcs_output_closure(pointer_t ptr, size_t currlen, size_t maxlen)
{
	wchar_t* buffer = (wchar_t*)ptr;
	if (currlen < maxlen - 1)
		buffer[currlen] = L'\0';
	else if (maxlen > 0)
		buffer[maxlen - 1] = L'\0';
}

/** vsnprintf */
int qn_vsnwprintf(wchar_t* out, size_t len, const wchar_t* fmt, va_list va)
{
	size_t res;

	qn_retval_if_fail(fmt != NULL, -1);

	res = doprw(_wcs_output, _wcs_output_closure, out, len, fmt, va);

	if (len)
		len--;

	return out ? (int)QN_MIN(res, len) : (int)res;
}

/** vaprintf */
int qn_vaswprintf(wchar_t** out, const wchar_t* fmt, va_list va)
{
	size_t len;

	qn_retval_if_fail(out != NULL, -2);
	qn_retval_if_fail(fmt != NULL, -1);

	len = doprw(_wcs_output, _wcs_output_closure, NULL, 0, fmt, va);
	if (len <= 0)
	{
		*out = NULL;
		return 0;
	}

	*out = qn_alloc(len + 1, wchar_t);
	len = qn_vsnwprintf(*out, len + 1, fmt, va);

	return (int)len;
}

/** vaprintf */
wchar_t* qn_vapswprintf(const wchar_t* fmt, va_list va)
{
	size_t len;
	wchar_t* ret;

	qn_retval_if_fail(fmt != NULL, NULL);

	len = doprw(_wcs_output, _wcs_output_closure, NULL, 0, fmt, va);
	if (len <= 0)
		return NULL;

	ret = qn_alloc(len + 1, wchar_t);
	qn_vsnwprintf(ret, len + 1, fmt, va);

	return ret;
}

/** snprintf */
int qn_snwprintf(wchar_t* out, size_t len, const wchar_t* fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = qn_vsnwprintf(out, len, fmt, va);
	va_end(va);

	return ret;
}

/** asprintf */
int qn_aswprintf(wchar_t** out, const wchar_t* fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = qn_vaswprintf(out, fmt, va);
	va_end(va);

	return ret;
}

/** asprintf */
wchar_t* qn_apswprintf(const wchar_t* fmt, ...)
{
	va_list va;
	wchar_t* ret;

	va_start(va, fmt);
	ret = qn_vapswprintf(fmt, va);
	va_end(va);

	return ret;
}

/** 빈칸으로 채우기 */
size_t qn_wcsfll(wchar_t* dest, size_t pos, size_t end, int ch)
{
	size_t i, cnt;
	if (pos >= end)
		return pos;
	cnt = end - pos;
	for (i = 0; i < cnt; i++)
		dest[pos + i] = (wchar_t)ch;
	return pos + cnt;
}

/** 문자열 해시 */
size_t qn_wcshash(const wchar_t* p)
{
	const wchar_t* sz = p;
	size_t h = *sz++;
	if (!h)
		return 0;
	else
	{
		size_t c;
		for (c = 0; *sz && c < 256; sz++, c++)
			h = (h << 5) - h + *sz;
		h = (h << 5) - h + c;
		return h;
	}
}

/** 문자열 해시 (대소문자구별안함) */
size_t qn_wcsihash(const wchar_t* p)
{
	const wchar_t* sz = p;
	size_t h = towupper(*sz);
	if (!h)
		return 0;
	else
	{
		size_t c;
		sz++;
		for (c = 0; *sz && c < 256; sz++, c++)
			h = (h << 5) - h + towupper(*sz);
		h = (h << 5) - h + c;
		return h;
	}
}

/** wcsbrk */
wchar_t* qn_wcsbrk(const wchar_t* p, const wchar_t* c)
{
	wchar_t* t;
	while (*p)
	{
		for (t = (wchar_t*)c; *t; t++)
		{
			if (*t == *p)
				return (wchar_t*)p;
		}
		p++;
	}
	return NULL;
}

/** wcsmid */
wchar_t* qn_wcsmid(wchar_t* dest, size_t destsize, const wchar_t* src, size_t pos, size_t len)
{
	size_t size = wcslen(src);

	if (pos > size)
		*dest = L'\0';
	else
	{
		wcsncpy_s(dest, destsize, src + pos, len);
		*(dest + len) = L'\0';
	}

	return dest;
}

/** wcsltrim */
wchar_t* qn_wcsltrim(wchar_t* dest)
{
	wchar_t* s;
	for (s = dest; *s && iswspace(*s); s++);
	if (dest != s)
		memmove(dest, s, (wcslen(s) + 1) * sizeof(wchar_t));
	return dest;
}

/** wcsrtrim */
wchar_t* qn_wcsrtrim(wchar_t* dest)
{
	size_t len = wcslen(dest);
	while (len--)
	{
		if (!iswspace(dest[len]))
			break;
		dest[len] = L'\0';
	}
	return dest;
}

/** wcstrim */
wchar_t* qn_wcstrim(wchar_t* dest)
{
	return qn_wcsrtrim(qn_wcsltrim(dest));
}

/** wcsrem */
wchar_t* qn_wcsrem(wchar_t* p, const wchar_t* rmlist)
{
	wchar_t* p1 = p;
	wchar_t* p2 = p;

	while (*p1)
	{
		const wchar_t* ps = rmlist;
		bool b = FALSE;

		while (*ps)
		{
			if (*p1 == *ps)
			{
				b = TRUE;
				break;
			}
			++ps;
		}

		if (!b)
		{
			*p2 = *p1;
			++p2;
		}

		++p1;
	}

	*p2 = L'\0';

	return p;
}

/** wcspcpy */
static wchar_t* _wcspcpy(wchar_t* dest, const wchar_t*  src)
{
	do (*dest++ = *src);
	while (*src++ != L'\0');
	return dest - 1;
}

/** 여러 문자열의 strdup */
wchar_t* qn_wcscat(const wchar_t* p, ...)
{
	va_list va;
	wchar_t* str;
	wchar_t* s, *c;
	size_t size;

	size = wcslen(p) + 1;
	va_start(va, p);
	s = va_arg(va, wchar_t*);
	while (s)
	{
		size += wcslen(s);
		s = va_arg(va, wchar_t*);
	}
	va_end(va);

	str = qn_alloc(size, wchar_t);
	c = str;

	c = _wcspcpy(c, p);
	va_start(va, p);
	s = va_arg(va, wchar_t*);
	while (s)
	{
		c = _wcspcpy(c, s);
		s = va_arg(va, wchar_t*);
	}
	va_end(va);

	return str;
}


//////////////////////////////////////////////////////////////////////////
// 유니코드

/**
 * UTF-8 글자 길이
 * cp_strlen_utf8 (http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html)
 */
size_t qn_utf8len(const char* s)
{
#define MASK    ((size_t)(-1)/0xFF)
	const char* t;
	size_t cnt = 0;
	size_t i;
	uint8_t b;

	// 초기 정렬되지 않은 아무 바이트 계산
	for (t = s; (uintptr_t)t & (sizeof(size_t) - 1); t++)
	{
		b = *t;

		if (b == '\0')
			goto pos_done;

		cnt += (b >> 7) & ((~b) >> 6);
	}

	// 완전한 블럭 계산
	for (;; t += sizeof(size_t))
	{
#ifdef __GNUC__
		__builtin_prefetch(&t[256], 0, 0);
#endif

		i = *(size_t*)(t);

		if ((i - MASK) & (~i) & (MASK * 0x80))
			break;

		i = ((i & (MASK * 0x80)) >> 7) & ((~i) >> 6);
		cnt += (i * MASK) >> ((sizeof(size_t) - 1) * 8);
	}

	//
	for (;; t++)
	{
		b = *t;

		if (b == '\0')
			break;

		cnt += (b >> 7) & ((~b) >> 6);
	}

pos_done:
	return ((t - s) - cnt);
#undef MASK
}

/**
 * UTF-8 문자를 UCS-4 문자로.
 * @param	p	utf8 문자.
 * @return	ucs4 문자.
 */
uchar4_t qn_utf8ccnc(const char* p)
{
	int len, mask;
	uint8_t ch = *(uint8_t*)p;

	if (ch < 128)
	{
		len = 1;
		mask = 0x7f;
	}
	else if ((ch & 0xe0) == 0xc0)
	{
		len = 2;
		mask = 0x1f;
	}
	else if ((ch & 0xf0) == 0xe0)
	{
		len = 3;
		mask = 0x0f;
	}
	else if ((ch & 0xf8) == 0xf0)
	{
		len = 4;
		mask = 0x07;
	}
	else if ((ch & 0xfc) == 0xf8)
	{
		len = 5;
		mask = 0x03;
	}
	else if ((ch & 0xfe) == 0xfc)
	{
		len = 6;
		mask = 0x01;
	}
	else
	{
		len = -1;
		mask = 0;
	}

	if (len < 0)
	{
		// 사용하지 않는 문자 코드
		// 0xFFFFFFFF
		return (uchar4_t)-1;
	}
	else
	{
		// UCS4로 변환
		uchar4_t ret = p[0] & mask;
		for (int i = 1; i < len; i++)
		{
			if ((p[i] & 0xC0) != 0x80)
			{
				ret = (uchar4_t)-1;
				break;
			}

			ret <<= 6;
			ret |= (p[i] & 0x3F);
		}

		return ret;
	}
}

/** UTF-8 다음 글자 */
char* qn_utf8chn(const char* s)
{
	static const char s_skips[256] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
	};
	return (char*)(s + s_skips[*(const uint8_t*)s]);
}

/**
 * UCS-4 문자를 UTF-8 문자로
 * @param	c		   	ucs4 문자.
 * @param [반환]	out	utf8 문자가 들어갈 배열. 최소 6개 항목의 배열이어야 한다
 * @return	utf8문자를 구성하는 문자열의 길이.
 */
int qn_utf8cunc(uchar4_t c, char* out)
{
	int first, len;

	if (c < 0x80)
	{
		first = 0;
		len = 1;
	}
	else if (c < 0x800)
	{
		first = 0xc0;
		len = 2;
	}
	else if (c < 0x10000)
	{
		first = 0xe0;
		len = 3;
	}
	else if (c < 0x200000)
	{
		first = 0xf0;
		len = 4;
	}
	else if (c < 0x4000000)
	{
		first = 0xf8;
		len = 5;
	}
	else
	{
		first = 0xfc;
		len = 6;
	}

	if (out)
	{
		for (int i = len - 1; i > 0; --i)
		{
			out[i] = (c & 0x3f) | 0x80;
			c >>= 6;
		}

		out[0] = (char)(c | first);
	}

	return len;
}

// utf16 서로게이트를 ucs4로 바꿈
static uchar4_t _utf16_surrogate(uchar2_t h, uchar2_t l)
{
	return (((uchar4_t)h - 0xD800) * 0x0400 + (uchar4_t)l - 0xDC00 + 0x010000);
}


//////////////////////////////////////////////////////////////////////////
// 문자열 변환

//
size_t qn_mbstowcs(wchar_t* outwcs, size_t outsize, const char* inmbs, size_t insize)
{
#if _QN_WINDOWS_
	int len = MultiByteToWideChar(CP_THREAD_ACP, 0, inmbs, insize == 0 ? -1 : (int)insize, outwcs, (int)outsize);
	if (outwcs && len >= 0)
		outwcs[len] = L'\0';
	return len;
#else
	return mbstowcs(outwcs, inmbs, outsize);
#endif
}

//
size_t qn_wcstombs(char* outmbs, size_t outsize, const wchar_t* inwcs, size_t insize)
{
#if _QN_WINDOWS_
	int len = WideCharToMultiByte(CP_THREAD_ACP, 0, inwcs, insize == 0 ? -1 : (int)insize, outmbs, (int)outsize, NULL, NULL);
	if (outmbs && len >= 0)
		outmbs[len] = L'\0';
	return len;
#else
	return wcstombs(outmbs, inwcs, outsize);
#endif
}


/**
 * utf8 -> ucs4.
 * 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
 * @param [반환]	dest	(널값이 아니면) 대상 버퍼 (ucs4).
 * @param	destsize		대상 버퍼 크기.
 * @param	src				원본 (utf8).
 * @param	srclen			원본 길이. 0으로 지정할 수 있음.
 * @return	변환한 길이 또는 변환에 필요한 길이.
 */
size_t qn_utf8to32(uchar4_t* dest, size_t destsize, const char* src, size_t srclen)
{
	qn_retval_if_fail(src, 0);

	if (destsize == 0 || !dest)
	{
		// utf-8 -> ucs-4의 길이는.... 그냥 utf-8의 길이.
		return qn_utf8len(src);
	}
	else
	{
		const char* t;
		size_t i, size;

		if (srclen == 0)
			srclen = qn_utf8len(src);

		size = QN_MIN(destsize, srclen);

		for (t = src, i = 0; i < size; i++)
		{
			dest[i] = qn_utf8ccnc(t);
			t = qn_utf8chn(t);
		}

		dest[i] = (uchar4_t)'\0';

		return size;
	}
}

/**
 * utf8 -> utf16.
 * 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
 * @param [반환]	dest	(널값이 아니면) 대상 버퍼 (utf16).
 * @param	destsize		대상 버퍼 크기.
 * @param	src				원본 (utf8).
 * @param	srclen			원본 길이. 0으로 지정할 수 있음.
 * @return	변환한 길이 또는 변환에 필요한 길이.
 */
size_t qn_utf8to16(uchar2_t* dest, size_t destsize, const char* src, size_t srclen)
{
	qn_retval_if_fail(src, 0);

	if (destsize == 0 || !dest)
	{
		// utf-8 -> ucs-4의 길이는.... 그냥 utf-8의 길이.
		// 다만 서로게이트 문제는 어떻게 하나... 일단 이걸로 하고 나중에 고치자
		return qn_utf8len(src);
	}
	else
	{
		const char* t;
		size_t i, size;

		if (srclen == 0)
			srclen = qn_utf8len(src);

		size = QN_MIN(destsize, srclen);

		for (t = src, i = 0; i < size;)
		{
			uchar4_t ch = qn_utf8ccnc(t);

			if (ch < 0x010000)
				dest[i++] = (uchar2_t)ch;
			else
			{
				dest[i++] = (uchar2_t)((ch - 0x010000) / 0x0400 + 0xD800);
				dest[i++] = (uchar2_t)((ch - 0x010000) % 0x0400 + 0xDC00);
			}

			t = qn_utf8chn(t);
		}

		dest[i] = (uchar2_t)'\0';

		return size;
	}
}

/**
 * ucs4 -> utf8.
 * 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
 * @param [반환]	dest	(널값이 아니면) 대상 버퍼 (utf8).
 * @param	destsize		대상 버퍼 크기.
 * @param	src				원본 (ucs4).
 * @param	srclen			원본 길이. 0으로 지정할 수 있음.
 * @return	변환한 길이 또는 변환에 필요한 길이.
 */
size_t qn_utf32to8(char* dest, size_t destsize, const uchar4_t* src, size_t srclen_org)
{
	size_t z, n, size;
	intptr_t i, srclen;
	uchar4_t uc;

	qn_retval_if_fail(src, 0);

	srclen = (intptr_t)srclen_org;

	if (destsize > 0)
	{
		for (z = 0, size = 0, i = 0; srclen == 0 || i < srclen; i++)
		{
			uc = src[i];

			if (!uc)
				break;

			if ((uc & 0x80000000) != 0)	// if (uc >= 0x80000000)
			{
				// 변경할 수 없는 문자가 포함됨..
				// 일단 그냥 못한다고 보내자
				return 0;
			}

			n = (uc < 0x80) ? 1 :
				(uc < 0x0800) ? 2 :
				(uc < 0x010000) ? 3 :
				(uc < 0x200000) ? 4 :
				(uc < 0x04000000) ? 5 :
				6;

			z = size + n;

			if (z > destsize)
				break;

			size = z;
		}
	}
	else
	{
		for (size = 0, i = 0; srclen == 0 || i < srclen; i++)
		{
			uc = src[i];

			if (!uc)
				break;

			if ((uc & 0x80000000) != 0)	// if (uc >= 0x80000000)
			{
				// 변경할 수 없는 문자가 포함됨..
				// 일단 그냥 못한다고 보내자
				return 0;
			}

			n = (uc < 0x80) ? 1 :
				(uc < 0x0800) ? 2 :
				(uc < 0x010000) ? 3 :
				(uc < 0x200000) ? 4 :
				(uc < 0x04000000) ? 5 :
				6;

			size += n;
		}
	}

	if (dest && destsize > 0)
	{
		char* p = dest;

		for (i = 0; p < (dest + size); i++)
			p += qn_utf8cunc(src[i], p);

		*p = '\0';
	}

	return size;
}

/**
 * utf16 -> utf8.
 * 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
 * @param [반환]	dest	(널값이 아니면) 대상 버퍼 (utf8).
 * @param	destsize		대상 버퍼 크기.
 * @param	src				원본 (utf16).
 * @param	srclen			원본 길이. 0으로 지정할 수 있음.
 * @return	변환한 길이 또는 변환에 필요한 길이.
 */
size_t qn_utf16to8(char* dest, size_t destsize, const uchar2_t* src, size_t srclen)
{
	const uchar2_t* cp;
	char* p;
	uchar4_t hsg, uc;
	uchar2_t ch;
	size_t size, n, z;

	qn_retval_if_fail(src, 0);

	cp = src;
	hsg = 0;
	size = 0;

	if (destsize > 0)
	{
		for (z = 0; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					uc = _utf16_surrogate((uchar2_t)hsg, ch);
					hsg = 0;
				}
				else
				{
					// 음... 변경할 수 없는 문자가 있네...
					// 일단 못한다고 보내자
					return 0;
				}
			}
			else
			{
				// 일반
				if (hsg)
				{
					// 상위 서로게이트가 있네?
					// 일단.. 문제가 있다고 판단
					return 0;
				}

				if (ch >= 0xD800 && ch < 0xDC00)
				{
					// 상위 서로게이트
					hsg = ch;
					continue;
				}

				uc = ch;
			}

			n = (uc < 0x80) ? 1 :
				(uc < 0x0800) ? 2 :
				(uc < 0x010000) ? 3 :
				(uc < 0x200000) ? 4 :
				(uc < 0x04000000) ? 5 :
				6;

			z = size + n;

			if (z > destsize)
				break;

			size = z;
		}
	}
	else
	{
		for (; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					uc = _utf16_surrogate((uchar2_t)hsg, ch);
					hsg = 0;
				}
				else
				{
					// 음... 변경할 수 없는 문자가 있네...
					// 일단 못한다고 보내자
					return 0;
				}
			}
			else
			{
				// 일반
				if (hsg)
				{
					// 상위 서로게이트가 있네?
					// 일단.. 문제가 있다고 판단
					return 0;
				}

				if (ch >= 0xD800 && ch < 0xDC00)
				{
					// 상위 서로게이트
					hsg = ch;
					continue;
				}

				uc = ch;
			}

			n = (uc < 0x80) ? 1 :
				(uc < 0x0800) ? 2 :
				(uc < 0x010000) ? 3 :
				(uc < 0x200000) ? 4 :
				(uc < 0x04000000) ? 5 :
				6;

			size += n;
		}
	}

	if (hsg)
	{
		// 하위 서로게이트가 남음...
		// 못함
		return 0;
	}

	if (destsize > 0 && dest)
	{
		hsg = 0;
		p = dest;
		cp = src;

		for (; p < dest + size; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				uc = _utf16_surrogate((uchar2_t)hsg, ch);
				hsg = 0;
			}
			else if (ch >= 0xD800 && ch < 0xDC00)
			{
				// 상위 서로게이트
				hsg = ch;
				continue;
			}
			else
			{
				// 일반 글자
				uc = ch;
			}

			p += qn_utf8cunc(uc, p);
		}

		*p = '\0';
	}

	return size;
}

/**
 * utf16 -> ucs4.
 * 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
 * @param [반환]	dest	(널값이 아니면) 대상 버퍼 (ucs4).
 * @param	destsize		대상 버퍼 크기.
 * @param	src				원본 (utf16).
 * @param	srclen			원본 길이. 0으로 지정할 수 있음.
 * @return	변환한 길이 또는 변환에 필요한 길이.
 */
size_t qn_utf16to32(uchar4_t* dest, size_t destsize, const uchar2_t* src, size_t srclen)
{
	const uchar2_t* cp;
	uchar4_t* p;
	uchar4_t hsg, uc;
	uchar2_t ch;
	size_t size, z;

	qn_retval_if_fail(src, 0);

	cp = src;
	hsg = 0;
	size = 0;

	if (destsize > 0)
	{
		for (z = 0; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					uc = _utf16_surrogate((uchar2_t)hsg, ch);
					hsg = 0;
				}
				else
				{
					// 음... 변경할 수 없는 문자가 있네...
					// 일단 못한다고 보내자
					return 0;
				}
			}
			else
			{
				// 일반
				if (hsg)
				{
					// 상위 서로게이트가 있네?
					// 일단.. 문제가 있다고 판단
					return 0;
				}

				if (ch >= 0xD800 && ch < 0xDC00)
				{
					// 상위 서로게이트
					hsg = ch;
					continue;
				}

				uc = ch;
			}

			z = size + 1;

			if (z > destsize)
				break;

			size = z;
		}
	}
	else
	{
		for (; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					uc = _utf16_surrogate((uchar2_t)hsg, ch);
					hsg = 0;
				}
				else
				{
					// 음... 변경할 수 없는 문자가 있네...
					// 일단 못한다고 보내자
					return 0;
				}
			}
			else
			{
				// 일반
				if (hsg)
				{
					// 상위 서로게이트가 있네?
					// 일단.. 문제가 있다고 판단
					return 0;
				}

				if (ch >= 0xD800 && ch < 0xDC00)
				{
					// 상위 서로게이트
					hsg = ch;
					continue;
				}

				uc = ch;
			}


			size++;
		}
	}

	if (hsg)
	{
		// 하위 서로게이트가 남음...
		// 못함
		return 0;
	}

	if (destsize > 0 && dest)
	{
		hsg = 0;
		p = dest;
		cp = src;

		for (; p < dest + size; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				uc = _utf16_surrogate((uchar2_t)hsg, ch);
				hsg = 0;
			}
			else if (ch >= 0xD800 && ch < 0xDC00)
			{
				// 상위 서로게이트
				hsg = ch;
				continue;
			}
			else
			{
				// 일반 글자
				uc = ch;
			}

			*p = uc;
			p++;
		}

		*p = '\0';
	}

	return size;
}

/**
 * utf16 -> ucs4.
 * 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
 * @param [반환]	dest	(널값이 아니면) 대상 버퍼 (utf16).
 * @param	destsize		대상 버퍼 크기.
 * @param	src				원본 (ucs4).
 * @param	srclen_org		원본 길이. 0으로 지정할 수 있음.
 * @return	변환한 길이 또는 변환에 필요한 길이.
 */
size_t qn_utf32to16(uchar2_t* dest, size_t destsize, const uchar4_t* src, size_t srclen_org)
{
	uchar2_t* p;
	uchar4_t uc;
	size_t size, z;
	intptr_t i, srclen;

	qn_retval_if_fail(src, 0);

	size = 0;
	srclen = (intptr_t)srclen_org;

	if (destsize > 0)
	{
		for (i = 0; (srclen == 0 || i < srclen) && src[i]; i++)
		{
			uc = src[i];

			if (uc < 0xD800)
				z = 1;
			else if (uc < 0xE000)
			{
				// 음..
				return 0;
			}
			else if (uc < 0x010000)
				z = 1;
			else if (uc < 0x110000)
				z = 2;
			else
			{
				// 음...
				return 0;
			}

			z += size;

			if (z > destsize)
				break;

			size = z;
		}
	}
	else
	{
		for (i = 0; (srclen == 0 || i < srclen) && src[i]; i++)
		{
			uc = src[i];

			if (uc < 0xD800)
				size++;
			else if (uc < 0xE000)
			{
				// 음..
				return 0;
			}
			else if (uc < 0x010000)
				size++;
			else if (uc < 0x110000)
				size += 2;
			else
			{
				// 음...
				return 0;
			}
		}
	}

	if (destsize > 0 && dest)
	{
		for (p = dest, i = 0, z = 0; z < size; i++)
		{
			uc = src[i];

			if (uc < 0x010000)
				p[z++] = (uchar2_t)uc;
			else
			{
				p[z++] = (uchar2_t)((uc - 0x010000) / 0x0400 + 0xD800);
				p[z++] = (uchar2_t)((uc - 0x010000) % 0x0400 + 0xDC00);
			}
		}

		p[z] = (uchar2_t)'\0';
	}

	return size;
}


