#include "pch.h"
#include "qn.h"
#include "PatrickPowell_snprintf.h"

//////////////////////////////////////////////////////////////////////////
// 그냥 문자열

//
static void _asc_output(pointer_t ptr, size_t *currlen, size_t maxlen, int c)
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

	qn_value_if_fail(fmt != NULL, -1);

	res = dopr(_asc_output, _asc_output_closure, out, len, fmt, va);

	if (len)
		len--;

	return out ? (int)QN_MIN(res, len) : (int)res;
}

/** vaprintf */
int qn_vasprintf(char** out, const char* fmt, va_list va)
{
	size_t len;

	qn_value_if_fail(out != NULL, -2);
	qn_value_if_fail(fmt != NULL, -1);

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

	qn_value_if_fail(fmt != NULL, NULL);

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
	char map[32] = {0, };

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
		for (t=(char*)c; *t; t++)
		{
			if (*t==*p)
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


//////////////////////////////////////////////////////////////////////////
// 유니코드 문자열
//
static void _wcs_output(pointer_t ptr, size_t *currlen, size_t maxlen, int c)
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

	qn_value_if_fail(fmt != NULL, -1);

	res = doprw(_wcs_output, _wcs_output_closure, out, len, fmt, va);

	if (len)
		len--;

	return out ? (int)K_MIN(res, len) : (int)res;
}

/** vaprintf */
int qn_vaswprintf(wchar_t** out, const wchar_t* fmt, va_list va)
{
	size_t len;

	qn_value_if_fail(out != NULL, -2);
	qn_value_if_fail(fmt != NULL, -1);

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

	qn_value_if_fail(fmt != NULL, NULL);

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
		memmove(dest, s, (wcslen(s) + 1)*sizeof(wchar_t));
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
