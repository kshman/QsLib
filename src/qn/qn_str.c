//
// qn_str.c - CRT에 없는거나, 공통이 아닌것 재구현 (mbs,wcs,utf-8,utf 변환)
// 2023-12-27 by kim
//

// ReSharper disable CppParameterMayBeConst

#include "pch.h"
#include "qs_qn.h"

//////////////////////////////////////////////////////////////////////////
// 공용

// 문자를 진수 숫자로 (32진수까지)
byte qn_char_to_int_base(const uint n)
{
	static const byte nbase_table[] =
	{
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,	// 0~15
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 16~31
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 32~47
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255, // 48~63
		255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 64~79
		25, 26, 27, 28, 29, 30, 31, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 80~95
		255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 96~111
		25, 26, 27, 28, 29, 30, 31, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 112~127
	};
	return n < (uint)QN_COUNTOF(nbase_table) ? nbase_table[n] : 255;
}

// 숫자를 문자로 바꾸기 (32진수까지)
char qn_int_base_to_char(const uint n, bool upper)
{
	static const char* nchar_table[] =
	{
		"0123456789abcdefghijklmnopqrstuv",
		"0123456789ABCDEFGHIJKLMNOPQRSTUV",
	};
#if false
	return n < (uint)QN_COUNTOF(nchar_table) ? nchar_table[n] : '\0';
#else
	// 오류 검사할 필요 없다
	return nchar_table[upper][n];
#endif
}

//////////////////////////////////////////////////////////////////////////
// 그냥 문자열

extern size_t dopr(char* restrict buffer, size_t maxlen, const char* restrict format, va_list args);

//
int qn_vsnprintf(char* restrict out, size_t len, const char* restrict fmt, va_list va)
{
	qn_val_if_fail(fmt != NULL, -1);

	const size_t res = dopr(out, len, fmt, va);
	if (len)
		len--;

	return out ? (int)QN_MIN(res, len) : (int)res;
}

//
int qn_vasprintf(char** out, const char* fmt, va_list va)
{
	qn_val_if_fail(out != NULL, -2);
	qn_val_if_fail(fmt != NULL, -1);

	va_list vq;
	va_copy(vq, va);
	size_t len = dopr(NULL, 0, fmt, vq);
	va_end(vq);
	if (len == 0)
		*out = NULL;
	else
	{
		*out = qn_alloc(len + 1, char);
		len = dopr(*out, len + 1, fmt, va);
	}

	return (int)len;
}

//
char* qn_vapsprintf(const char* fmt, va_list va)
{
	qn_val_if_fail(fmt != NULL, NULL);

	va_list vq;
	va_copy(vq, va);
	const size_t len = dopr(NULL, 0, fmt, vq);
	va_end(vq);
	if (len == 0)
		return NULL;

	char* ret = qn_alloc(len + 1, char);
	dopr(ret, len + 1, fmt, va);

	return ret;
}

//
int qn_snprintf(char* restrict out, const size_t len, const char* restrict fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_vsnprintf(out, len, fmt, va);
	va_end(va);

	return ret;
}

//
int qn_asprintf(char** out, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_vasprintf(out, fmt, va);
	va_end(va);

	return ret;
}

//
char* qn_apsprintf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char* ret = qn_vapsprintf(fmt, va);
	va_end(va);

	return ret;
}

//
size_t qn_strfll(char* dest, const size_t pos, const size_t end, const int ch)
{
	if (pos >= end)
		return pos;
	const size_t cnt = end - pos;
	for (size_t i = 0; i < cnt; i++)
		dest[pos + i] = (char)ch;
	return pos + cnt;
}

//
size_t qn_strhash(const char* p)
{
	const char* sz = p;
	size_t h = (size_t)*sz++;
	if (!h)
		return 0;
	else
	{
		size_t c;
		for (c = 0; *sz && c < 256; sz++, c++)
			h = (h << 5) - h + (size_t)*sz;
		h = (h << 5) - h + c;
		return h;
	}
}

//
size_t qn_strihash(const char* p)
{
	const char* sz = p;
	size_t h = (size_t)toupper(*sz);
	if (!h)
		return 0;
	else
	{
		size_t c;
		sz++;
		for (c = 0; *sz && c < 256; sz++, c++)
			h = (h << 5) - h + (size_t)toupper(*sz);
		h = (h << 5) - h + c;
		return h;
	}
}

//
const char* qn_strbrk(const char* p, const char* c)
{
#if 1
	// microsoft c library 13.10
	const char* str = p;
	const char* ctn = c;
	int map[32] = { 0, };

	while (*ctn)
	{
		map[*ctn >> 3] |= 1 << (*ctn & 7);
		ctn++;
	}

	while (*str)
	{
		if (map[*str >> 3] & (1 << (*str & 7)))
			return str;
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

//
char* qn_strmid(char* restrict dest, const char* restrict src, const size_t pos, const size_t len)
{
	const size_t size = strlen(src);

	if (pos > size)
		*dest = '\0';
	else
	{
		qn_strncpy(dest, src + pos, len);
		*(dest + len) = '\0';
	}

	return dest;
}

//
char* qn_strltm(char* dest)
{
	char* s;
	for (s = dest; *s && isspace(*s); s++) {}
	if (dest != s)
		memmove(dest, s, strlen(s) + 1);
	return dest;
}

//
char* qn_strrtm(char* dest)
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

//
char* qn_strtrm(char* dest)
{
	return qn_strrtm(qn_strltm(dest));
}

//
char* qn_strrem(char* restrict p, const char* restrict rmlist)
{
	const char* p1 = p;
	char* p2 = p;

	while (*p1)
	{
		const char* ps = (const char*)rmlist;
		bool b = false;

		while (*ps)
		{
			if (*p1 == *ps)
			{
				b = true;
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

//
char* qn_stpcpy(char* restrict dest, const char* restrict src)
{
	do (*dest++ = *src);
	while (*src++ != '\0');
	return dest - 1;
}

//
char* qn_strdup(const char* p)
{
	qn_val_if_fail(p != NULL, NULL);
	const size_t len = strlen(p) + 1;
	char* d = qn_alloc(len, char);
	qn_strcpy(d, p);
	return d;
}

//
char* qn_strcat(const char* p, ...)
{
	va_list va, vq;
	va_start(va, p);
	va_copy(vq, va);

	size_t size = strlen(p) + 1;
	const char* s = va_arg(va, char*);
	while (s)
	{
		size += strlen(s);
		s = va_arg(va, char*);
	}

	char* str = qn_alloc(size, char);
	char* c = qn_stpcpy(str, p);
	s = va_arg(vq, char*);
	while (s)
	{
		c = qn_stpcpy(c, s);
		s = va_arg(vq, char*);
	}
	va_end(vq);

	va_end(va);
	return str;
}

//
bool qn_strwcm(const char* string, const char* wild)
{
	const char *cp = NULL, *mp = NULL;

	while ((*string) && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
			return false;
		wild++;
		string++;
	}

	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
				return 1;
			mp = wild;
			cp = string + 1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
		wild++;

	return *wild != '\0';
}

//
bool qn_striwcm(const char* string, const char* wild)
{
	const char *cp = NULL, *mp = NULL;

	while ((*string) && (*wild != '*'))
	{
		if ((toupper(*wild) != toupper(*string)) && (*wild != '?'))
			return false;
		wild++;
		string++;
	}

	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
				return 1;
			mp = wild;
			cp = string + 1;
		}
		else if ((toupper(*wild) == toupper(*string)) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
		wild++;

	return *wild != '\0';
}

//
int qn_strfnd(const char* src, const char* find, const size_t index)
{
	const char* p = src + index;

	while (*p)
	{
		const char* s1 = p;
		const char* s2 = find;

		while (*s1 && *s2 && !(*s1 - *s2))
		{
			++s1;
			++s2;
		}

		if (!*s2)
			return (int)(src + index - p);
	}

	return -1;
}

//
bool qn_streqv(const char* p1, const char* p2)
{
	return p1 == p2 ? true : strcmp(p1, p2) == 0;
}

//
bool qn_strieqv(const char* p1, const char* p2)
{
	return p1 == p2 ? true : qn_stricmp(p1, p2) == 0;
}

//
char* qn_strcpy(char* p, const char* src)
{
#ifdef __GNUC__
	return strcpy(p, src);
#else
	char* o = p;
	while ((*p++ = *src++));
	return o;
#endif
}

//
char* qn_strpcpy(char* p, const char* src)
{
	do (*p++ = *src);
	while (*src++ != '\0');
	return p - 1;
}

//
char* qn_strncpy(char* p, const char* src, size_t len)
{
#ifdef __GNUC__
	return strncpy(p, src, len);
#else
	char* o = p;
	while (len && (*p++ = *src++))
		--len;
	*p = '\0';
	if (len)
	{
		while (--len)
			*p++ = '\0';
	}
	return o;
#endif
}

//
char* qn_strupr(char* p)
{
	char* s = p;
	for (; *s; ++s)
		if ((*s >= 'a') && (*s <= 'z'))
			*s -= 'a' - 'A';
	return p;
}

//
char* qn_strlwr(char* p)
{
	char* s = p;
	for (; *s; ++s)
		if ((*s >= 'A') && (*s <= 'Z'))
			*s += 'a' - 'A';
	return p;
}

//
int qn_strncmp(const char* p1, const char* p2, size_t len)
{
#if defined _MSC_VER || defined __GNUC__
	return strncmp(p1, p2, len);
#else
	if (!len)
		return 0;
	while (--len && *p1 && *p1 == *p2)
		++p1, ++p2;
	return *p1 - *p2;
#endif
}

//
int qn_stricmp(const char* p1, const char* p2)
{
#ifdef _MSC_VER
	return _stricmp(p1, p2);
#elif defined __GNUC__
	return strcasecmp(p1, p2);
#else
	int	f, l;
	do
	{
		f = (unsigned char)(*(p1++));
		l = (unsigned char)(*(p2++));
		if (f >= 'A' && f <= 'Z')	f -= 'A' - 'a';
		if (l >= 'A' && l <= 'Z')	l -= 'A' - 'a';
	} while (f && (f == l));
	return (f - l);
#endif
}

//
int qn_strnicmp(const char* p1, const char* p2, size_t len)
{
#ifdef _MSC_VER
	return _strnicmp(p1, p2, len);
#elif defined __GNUC__
	return strncasecmp(p1, p2, len);
#else
	int	f, l;
	while (len && *p1 && *p2)
	{
		--len;

		f = (unsigned char)(*p1);
		l = (unsigned char)(*p2);
		if (f >= 'A' && f <= 'Z')	f -= 'A' - 'a';
		if (l >= 'A' && l <= 'Z')	l -= 'A' - 'a';

		if (f != l) return (f - l);
		++p1;
		++p2;
	}
	return (len) ? (int)(p1 - p2) : 0;
#endif
}

//
int qn_itoa(char* p, const int n, const uint base, bool upper)
{
	qn_val_if_fail(base <= 32, -1);
	if (upper > 1)
		upper = 1;
	char conv[32];
	int place = 0;
	uint uvalue;
	if (n >= 0 || base != 10)
		uvalue = (uint)n;
	else
	{
		conv[place++] = '-';
		uvalue = (uint)-n;
	}
	do
	{
		conv[place++] = qn_int_base_to_char(uvalue % base, true);
		uvalue = uvalue / base;
	} while (uvalue && place < (int)QN_COUNTOF(conv));
	if (place == (int)QN_COUNTOF(conv))
		place--;
	conv[place] = '\0';
	if (p != NULL)
		qn_strcpy(p, conv);
	return place;
}

//
int qn_lltoa(char* p, const llong n, const uint base, bool upper)
{
	qn_val_if_fail(base <= 32, -1);
	if (upper > 1)
		upper = 1;
	char conv[64];
	int place = 0;
	ullong uvalue;
	if (n >= 0 || base != 10)
		uvalue = (ullong)n;
	else
	{
		conv[place++] = '-';
		uvalue = (ullong)-n;
	}
	do
	{
		conv[place++] = qn_int_base_to_char(uvalue % base, upper);
		uvalue = uvalue / base;
	} while (uvalue && place < (int)QN_COUNTOF(conv));
	if (place == (int)QN_COUNTOF(conv))
		place--;
	conv[place] = '\0';
	if (p != NULL)
		qn_strcpy(p, conv);
	return place;
}

//
uint qn_strtoi(const char* p, const uint base)
{
	qn_val_if_fail(p != NULL, 0);
	qn_val_if_fail(base >= 2 && base < 32, 0);
	uint v = 0;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	uint ch = qn_char_to_int_base((uint)*p++);
	while (ch < base)
	{
		v = v * base + ch;
		ch = qn_char_to_int_base((uint)*p++);
	}
	return v;
}

//
ullong qn_strtoll(const char* p, const uint base)
{
	qn_val_if_fail(p != NULL, 0);
	qn_val_if_fail(base >= 2 && base < 32, 0);
	ullong v = 0;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	uint ch = qn_char_to_int_base((uint)*p++);
	while (ch < base)
	{
		v = v * base + ch;
		ch = qn_char_to_int_base((uint)*p++);
	}
	return v;
}


//////////////////////////////////////////////////////////////////////////
// 유니코드

//
uchar4 qn_u8cbn(const char* p)
{
	int len, mask;
	const byte ch = (byte)*p;

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
		return (uchar4)-1;
	}
	else
	{
		// UCS4로 변환
		uchar4 ret = (uchar4)(p[0] & mask);
		for (int i = 1; i < len; i++)
		{
			if ((p[i] & 0xC0) != 0x80)
			{
				ret = (uchar4)-1;
				break;
			}

			ret <<= 6;
			ret |= (p[i] & 0x3F);
		}

		return ret;
	}
}

//
const char* qn_u8nch(const char* s)
{
	static const char utf8_skips[256] =
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
	return s + utf8_skips[*(const byte*)s];
}

//
size_t qn_u8len(const char* s)
{
#if true
	// 새니타이저에 안걸리는 방법
	size_t cnt = 0;
	while (*s)
		cnt += (*s++ & 0xC0) != 0x80;
	return cnt;
#else
	// 이 루틴은 새니타이저에 걸린다
#define MASK    ((size_t)(-1)/0xFF)
	const char* t;
	size_t cnt;

	// 초기 정렬되지 않은 아무 바이트 계산
	for (cnt = 0, t = s; (uintptr_t)t & (sizeof(size_t) - 1); t++)
	{
		const byte b = (byte)*t;

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

		size_t i = *(const size_t*)t;	// NOLINT

		if ((i - MASK) & (~i) & (MASK * 0x80))
			break;

		i = ((i & (MASK * 0x80)) >> 7) & ((~i) >> 6);
		cnt += (i * MASK) >> ((sizeof(size_t) - 1) * 8);
	}

	//
	for (;; t++)
	{
		const byte b = (byte)*t;

		if (b == '\0')
			break;

		cnt += (b >> 7) & ((~b) >> 6);
	}

pos_done:
	return ((size_t)(t - s) - cnt);
#undef MASK
#endif
}

//
char* qn_u8ncpy(char* restrict dest, const char* restrict src, size_t len)
{
	const char* t = src;

	while (len && *t)
	{
		t = qn_u8nch(t);
		len--;
	}

	len = (size_t)(t - src);
	memcpy(dest, src, len);
	dest[len] = '\0';

	return dest;
}

//
size_t qn_u8lcpy(char* restrict dest, const char* restrict src, size_t len)
{
	const char* t = src;

	while (len && *t)
	{
		t = qn_u8nch(t);
		len--;
	}

	len = (size_t)(t - src);
	memcpy(dest, src, len);
	dest[len] = '\0';

	return len;
}

//
int qn_u32ucb(uchar4 c, char* out)
{
	uchar4 first;
	int len;

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
			out[i] = (char)((c & 0x3f) | 0x80);
			c >>= 6;
		}

		out[0] = (char)(c | first);
	}

	return len;
}

// utf16 서로게이트를 ucs4로 바꿈
static uchar4 _utf16_surrogate(const uchar2 h, const uchar2 l)
{
	return (((uchar4)h - 0xD800) * 0x0400 + (uchar4)l - 0xDC00 + 0x010000);
}

//
int qn_u16ucb(const uchar2 high, const uchar2 low, char* out)
{
	const uchar4 ucs4 = _utf16_surrogate(high, low);
	return qn_u32ucb(ucs4, out);
}


//////////////////////////////////////////////////////////////////////////
// 문자열 변환

//
size_t qn_mbstowcs(wchar* restrict outwcs, const size_t outsize, const char* restrict inmbs, const size_t insize)
{
#ifdef _QN_WINDOWS_
	int len = MultiByteToWideChar(CP_THREAD_ACP, 0, inmbs, insize == 0 ? -1 : (int)insize, outwcs, (int)outsize);
	if (outwcs && len >= 0 && outsize > 0)
		outwcs[QN_MIN((int)outsize - 1, len)] = L'\0';
	return (int)len;
#else
	const size_t len = mbstowcs(outwcs, inmbs, outsize);
	if (outwcs && len >= 0 && outsize > 0)
		outwcs[QN_MIN(outsize - 1, len)] = L'\0';
	return (size_t)len;
#endif
}

//
size_t qn_wcstombs(char* restrict outmbs, const size_t outsize, const wchar* restrict inwcs, const size_t insize)
{
#ifdef _QN_WINDOWS_
	int len = WideCharToMultiByte(CP_THREAD_ACP, 0, inwcs, insize == 0 ? -1 : (int)insize, outmbs, (int)outsize, NULL, NULL);
	if (outmbs && len >= 0 && outsize > 0)
		outmbs[QN_MIN((int)outsize - 1, len)] = '\0';
	return (int)len;
#else
	size_t len = wcstombs(outmbs, inwcs, outsize);
	if (outmbs && len >= 0 && outsize > 0)
		outmbs[QN_MIN(outsize - 1, len)] = L'\0';
	return (size_t)len;
#endif
}

//
size_t qn_u8to32(uchar4* restrict dest, const size_t destsize, const char* restrict src, const size_t srclen)
{
	qn_val_if_fail(src, 0);

	if (destsize == 0 || !dest)
	{
		// utf-8 -> ucs-4의 길이는.... 그냥 utf-8의 길이.
		return qn_u8len(src);
	}
	else
	{
		const size_t slen = srclen == 0 ? qn_u8len(src) : srclen;
		const size_t size = QN_MIN(destsize, slen);

		const char* t;
		size_t i;
		for (t = src, i = 0; i < size; i++)
		{
			dest[i] = qn_u8cbn(t);
			t = qn_u8nch(t);
		}

		dest[i] = (uchar4)'\0';

		return size;
	}
}

//
size_t qn_u8to16(uchar2* restrict dest, const size_t destsize, const char* restrict src, const size_t srclen)
{
	qn_val_if_fail(src, 0);

	if (destsize == 0 || !dest)
	{
		// utf-8 -> ucs-4의 길이는.... 그냥 utf-8의 길이.
		// 다만 서로게이트 문제는 어떻게 하나... 일단 이걸로 하고 나중에 고치자
		return qn_u8len(src);
	}
	else
	{
		const size_t slen = srclen == 0 ? qn_u8len(src) : srclen;
		const size_t size = QN_MIN(destsize, slen);

		const char* t;
		size_t i;
		for (t = src, i = 0; i < size;)
		{
			const uchar4 ch = qn_u8cbn(t);

			if (ch < 0x010000)
				dest[i++] = (uchar2)ch;
			else
			{
				dest[i++] = (uchar2)((ch - 0x010000) / 0x0400 + 0xD800);
				dest[i++] = (uchar2)((ch - 0x010000) % 0x0400 + 0xDC00);
			}

			t = qn_u8nch(t);
		}

		dest[i] = (uchar2)'\0';

		return size;
	}
}

//
size_t qn_u32to8(char* restrict dest, const size_t destsize, const uchar4* restrict src, const size_t srclen)
{
	qn_val_if_fail(src, 0);

	const nint slen = (nint)srclen;
	size_t n, size;
	intptr_t i;
	uchar4 uc;

	if (destsize > 0)
	{
		for (size = 0, i = 0; slen == 0 || i < slen; i++)
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

			const size_t z = size + n;

			if (z > destsize)
				break;

			size = z;
		}
	}
	else
	{
		for (size = 0, i = 0; slen == 0 || i < slen; i++)
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
			p += qn_u32ucb(src[i], p);

		*p = '\0';
	}

	return size;
}

//
size_t qn_u16to8(char* restrict dest, const size_t destsize, const uchar2* restrict src, const size_t srclen)
{
	qn_val_if_fail(src, 0);

	const uchar2* cp = src;
	uchar4 hsg = 0;
	size_t size = 0;
	uchar4 uc;
	uchar2 ch;
	size_t n;

	if (destsize > 0)
	{
		for (; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					uc = _utf16_surrogate((uchar2)hsg, ch);
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

			const size_t z = size + n;
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
					uc = _utf16_surrogate((uchar2)hsg, ch);
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
		char* p = dest;
		hsg = 0;
		cp = src;

		for (; p < dest + size; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				uc = _utf16_surrogate((uchar2)hsg, ch);
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

			p += qn_u32ucb(uc, p);
		}

		*p = '\0';
	}

	return size;
}

//
size_t qn_u16to32(uchar4* restrict dest, const size_t destsize, const uchar2* restrict src, const size_t srclen)
{
	qn_val_if_fail(src, 0);

	uchar4 hsg = 0;
	size_t size = 0;

	if (destsize > 0)
	{
		for (const uchar2* cp = src; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			const uchar2 ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					_utf16_surrogate((uchar2)hsg, ch);
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
			}

			const size_t z = size + 1;

			if (z > destsize)
				break;

			size = z;
		}
	}
	else
	{
		for (const uchar2* cp = src; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			const uchar2 ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					_utf16_surrogate((uchar2)hsg, ch);
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
		uchar4* p = dest;
		uchar4 uc;

		hsg = 0;

		for (const uchar2* cp = src; p < dest + size; cp++)
		{
			const uchar2 ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				uc = _utf16_surrogate((uchar2)hsg, ch);
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

//
size_t qn_u32to16(uchar2* restrict dest, const size_t destsize, const uchar4* restrict src, const size_t srclen)
{
	qn_val_if_fail(src, 0);

	size_t size = 0;
	const intptr_t slen = (intptr_t)srclen;
	size_t z;
	intptr_t i;

	if (destsize > 0)
	{
		for (i = 0; (slen == 0 || i < slen) && src[i]; i++)
		{
			const uchar4 uc = src[i];

			if (uc < 0xD800)
				z = 1;		// NOLINT(bugprone-branch-clone)
			else if (uc < 0xE000)
				return 0;	// NOLINT(bugprone-branch-clone)
			else if (uc < 0x010000)
				z = 1;
			else if (uc < 0x110000)
				z = 2;
			else
				return 0;

			z += size;
			if (z > destsize)
				break;
			size = z;
		}
	}
	else
	{
		for (i = 0; (slen == 0 || i < slen) && src[i]; i++)
		{
			const uchar4 uc = src[i];

			if (uc < 0xD800)
				size++;		// NOLINT(bugprone-branch-clone)
			else if (uc < 0xE000)
				return 0;	// NOLINT(bugprone-branch-clone)
			else if (uc < 0x010000)
				size++;
			else if (uc < 0x110000)
				size += 2;
			else
				return 0;
		}
	}

	if (destsize > 0 && dest)
	{
		uchar2* p;
		for (p = dest, i = 0, z = 0; z < size; i++)
		{
			const uchar4 uc = src[i];

			if (uc < 0x010000)
				p[z++] = (uchar2)uc;
			else
			{
				p[z++] = (uchar2)((uc - 0x010000) / 0x0400 + 0xD800);
				p[z++] = (uchar2)((uc - 0x010000) % 0x0400 + 0xDC00);
			}
		}

		p[z] = (uchar2)'\0';
	}

	return size;
}

#define DEF_UTF_DUP(name, in_type, out_type)\
	out_type* name##_dup(const in_type* src, size_t srclen) {\
		size_t len=name(NULL,0,src,srclen)+1; qn_val_if_ok(len<2,NULL);\
		out_type* buf=qn_alloc(len, out_type); name(buf,len,src,srclen);/* NOLINT */\
		return buf; }
DEF_UTF_DUP(qn_mbstowcs, char, wchar)
DEF_UTF_DUP(qn_wcstombs, wchar, char)
DEF_UTF_DUP(qn_u8to32, char, uchar4)
DEF_UTF_DUP(qn_u8to16, char, uchar2)
DEF_UTF_DUP(qn_u32to8, uchar4, char)
DEF_UTF_DUP(qn_u16to8, uchar2, char)
DEF_UTF_DUP(qn_u16to32, uchar2, uchar4)
DEF_UTF_DUP(qn_u32to16, uchar4, uchar2)
#undef DEF_UTF_DUP
