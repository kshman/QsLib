//
// qn_str.c - CRT에 없는거나, 공통이 아닌것 재구현 (mbs,wcs,utf8,utf변환,해시)
// 2023-12-27 by kim
//

// ReSharper disable CppParameterMayBeConst

#include "pch.h"
#include <ctype.h>
#include <wctype.h>
#include "PatrickPowell_snprintf.h"
#define SINFL_IMPLEMENTATION
#define SDEFL_IMPLEMENTATION
#include "vurtun/sdefl.h"
#include "vurtun/sinfl.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4706)
#endif

//////////////////////////////////////////////////////////////////////////
// 메모리 조작

//
void* qn_memenc(void* dest, const void* src, const size_t size)
{
	const byte* ps = (const byte*)src;
	byte* pd = (byte*)dest;

	for (size_t i = size; i; --i, ++pd, ++ps)
	{
		const byte z = (byte)(255 - *ps);
		*pd = (byte)(z >> 4 | (z & 0xF) << 4);
	}

	return dest;
}

//
void* qn_memdec(void* dest, const void* src, const size_t size)
{
	const byte* ps = (const byte*)src;
	byte* pd = (byte*)dest;

	for (size_t i = size; i; --i, ++pd, ++ps)
	{
		const byte z = (byte)(255 - *ps);
		*pd = (byte)(z << 4 | (z & 0xF0) >> 4);
	}

	return dest;
}

//
void* qn_memzcpr(const void* src, const size_t srcsize, /*NULLABLE*/size_t* destsize)
{
	qn_return_when_fail(src != NULL, NULL);
	qn_return_when_fail(srcsize > 0, NULL);

	struct sdefl* s = qn_alloc_zero_1(struct sdefl);
	const int bound = sdefl_bound((int)srcsize);
	byte* p = qn_alloc(bound, byte);
	const int ret = sdeflate(s, p, src, (int)srcsize, 5);	// 압축 레벨은 5
	qn_free(s);

	if (destsize)
		*destsize = (size_t)ret;
	return p;
}

//
void* qn_memzucp(const void* src, const size_t srcsize, /*NULLABLE*/size_t* destsize)
{
	qn_return_when_fail(src != NULL, NULL);
	qn_return_when_fail(srcsize > 0, NULL);

	int size = (int)(srcsize * 3 + 12);
	byte* p = NULL;
	int ret;
	do
	{
		size *= 2;
		p = qn_realloc(p, size, byte);
		ret = sinflate(p, size, src, (int)srcsize);
	} while (ret == size);

	if (destsize)
		*destsize = ret;
	return p;
}

//
void* qn_memzucp_s(const void* src, const size_t srcsize, const size_t destsize)
{
	qn_return_when_fail(src != NULL, NULL);
	qn_return_when_fail(srcsize > 0, NULL);
	qn_return_when_fail(destsize > 0, NULL);

	byte* p = qn_alloc(destsize + 4, byte);
	const int ret = sinflate(p, (int)destsize, src, (int)srcsize);
	if (ret != (int)destsize)
	{
		qn_free(p);
		return NULL;
	}
	return p;
}

//
char qn_memhrb(const size_t size, double* out)
{
	qn_return_when_fail(out != NULL, ' ');
	if (size > 1024ULL * 1024ULL * 1024ULL)
	{
		*out = (double)size / (double)(1024ULL * 1024ULL * 1024ULL);
		return 'g';
	}
	if (size > 1024ULL * 1024ULL)
	{
		*out = (double)size / (double)(1024ULL * 1024ULL);
		return 'm';
	}
	if (size > 1024ULL)
	{
		*out = (double)size / (double)(1024ULL);
		return 'k';
	}
	*out = (double)size;
	return ' ';
}

//
char* qn_memdmp(const void* ptr, const size_t size, char* outbuf, const size_t buflen)
{
	qn_return_when_fail(ptr != NULL, NULL);
	qn_return_when_fail(outbuf != NULL, NULL);

	if (size == 0 || buflen == 0)
	{
		*outbuf = '\0';
		return outbuf;
	}

	const byte* mem = (const byte*)ptr;
	char* ind = outbuf;
	for (size_t i = 0, cnt = 1; i < size && cnt < buflen; i++, cnt++)
	{
		const byte m = *mem;
		*ind = (m < 0x20 || m > 0x7F) ? '.' : (const char)m;  // NOLINT
		mem++;
		ind++;
	}
	*ind = '\0';

	return outbuf;
}


//////////////////////////////////////////////////////////////////////////
// 공용

// 문자를 진수 숫자로 (32진수까지)
const byte* qn_num_base_table(void)
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
	return nbase_table;
}

// 숫자를 문자로 바꾸기 (32진수까지)
const char* qn_char_base_table(bool upper)
{
	static const char* nchar_table[] =
	{
		"0123456789abcdefghijklmnopqrstuv",
		"0123456789ABCDEFGHIJKLMNOPQRSTUV",
	};
	return nchar_table[upper == false ? 0 : 1];
}


//////////////////////////////////////////////////////////////////////////
// 아스키/멀티바이트 버전

// DOPR 정리 공용
static void printf_finish(PatrickPowellSprintfState* state)
{
	if (state->maxlen > 0)
		state->buf[state->currlen < state->maxlen - 1 ? state->currlen : state->maxlen - 1] = '\0';
}

// DOPR 고정 문자열 버전
static void printf_outch(PatrickPowellSprintfState* state, int ch)
{
	if (state->currlen < state->maxlen)
		state->buf[state->currlen] = (char)ch;
	state->currlen++;
}

// DOPR 할당 문자열 버전
static void printf_alloc_outch(PatrickPowellSprintfState* state, int ch)
{
	if (state->currlen >= state->maxlen)
	{
		state->maxlen = state->currlen + 256;
#ifdef QS_NO_MEMORY_PROFILE
		state->buf = (char*)qn_a_realloc(state->buf, state->maxlen);
#else
		state->buf = (char*)qn_a_i_realloc(state->buf, state->maxlen, state->desc, state->line);
#endif
	}
	state->buf[state->currlen] = (char)ch;
	state->currlen++;
}

#ifdef QS_NO_MEMORY_PROFILE
//
static int qn_a_printf(char** out, const char* fmt, va_list va)
{
	PatrickPowellSprintfState state =
	{
		printf_alloc_outch,
		printf_finish,
	};
	dopr(&state, fmt, va);
	*out = state.buf;
	return (int)state.currlen;
}

//
int qn_a_vsprintf(char** out, const char* fmt, va_list va)
{
	if (out == NULL)
		return qn_vsnprintf(NULL, 0, fmt, va);
	qn_return_when_fail(fmt != NULL, -1);
	return qn_a_printf(out, fmt, va);
}

//
char* qn_a_vpsprintf(const char* fmt, va_list va)
{
	qn_return_when_fail(fmt != NULL, NULL);
	char* buf;
	qn_a_printf(&buf, fmt, va);
	return buf;
}

//
int qn_a_sprintf(char** out, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_a_vsprintf(out, fmt, va);
	va_end(va);
	return ret;
}

//
char* qn_a_psprintf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char* buf;
	qn_a_printf(&buf, fmt, va);
	va_end(va);
	return buf;
}

//
char* qn_a_str_dup(const char* p)
{
	qn_return_when_fail(p != NULL, NULL);
	const size_t len = strlen(p) + 1;
	char* d = (char*)qn_a_alloc(len, false);
	qn_strcpy(d, p);
	return d;
}

//
char* qn_a_str_dup_cat(const char* p, ...)
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

	char* str = (char*)qn_a_alloc(size, false);
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
#else
static int qn_a_i_printf(const char* desc, size_t line, char** out, const char* fmt, va_list va)
{
	PatrickPowellSprintfState state =
	{
		printf_alloc_outch,
		printf_finish,
		desc, line,
	};
	dopr(&state, fmt, va);
	*out = state.buf;
	return (int)state.currlen;
}

//
int qn_a_i_vsprintf(const char* desc, size_t line, char** out, const char* fmt, va_list va)
{
	if (out == NULL)
		return qn_vsnprintf(NULL, 0, fmt, va);
	qn_return_when_fail(fmt != NULL, -1);
	return qn_a_i_printf(desc, line, out, fmt, va);
}

//
char* qn_a_i_vpsprintf(const char* desc, size_t line, const char* fmt, va_list va)
{
	qn_return_when_fail(fmt != NULL, NULL);
	char* buf;
	qn_a_i_printf(desc, line, &buf, fmt, va);
	return buf;
}

//
int qn_a_i_sprintf(const char* desc, size_t line, char** out, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_a_i_vsprintf(desc, line, out, fmt, va);
	va_end(va);
	return ret;
}

//
char* qn_a_i_psprintf(const char* desc, size_t line, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char* buf;
	qn_a_i_printf(desc, line, &buf, fmt, va);
	va_end(va);
	return buf;
}

//
char* qn_a_i_str_dup(const char* desc, size_t line, const char* p)
{
	qn_return_when_fail(p != NULL, NULL);
	const size_t len = strlen(p) + 1;
	char* d = (char*)qn_a_i_alloc(len, false, desc, line);
	qn_strcpy(d, p);
	return d;
}

//
char* qn_a_i_str_dup_cat(const char* desc, size_t line, const char* p, ...)
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

	char* str = (char*)qn_a_i_alloc(size, false, desc, line);
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
#endif

//
int qn_vsnprintf(char* out, size_t len, const char* fmt, va_list va)
{
	qn_return_when_fail(fmt != NULL, -1);
	PatrickPowellSprintfState state =
	{
		printf_outch,
		printf_finish,
	};
	state.maxlen = out != NULL ? len : 0;
	state.buf = out;
	dopr(&state, fmt, va);
	return (int)state.currlen;
}

//
int qn_snprintf(char* out, const size_t len, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_vsnprintf(out, len, fmt, va);
	va_end(va);
	return ret;
}

//
char* qn_strcpy(char* p, const char* src)
{
#ifdef __GNUC__
	return strcpy(p, src);
#else
	char* o = p;
	while ((*p++ = *src++)) {}
	return o;
#endif
}

//
char* qn_strncpy(char* p, const char* src, size_t len)
{
#ifdef __GNUC__
	return strncpy(p, src, len);
#else
	char* o = p;
	while (len && ((*p++ = *src++)))
		--len;
	*p = '\0';
#if false
	if (len)
	{
		while (--len)
			*p++ = '\0';
	}
#endif
	return o;
#endif
}

//
char* qn_stpcpy(char* dest, const char* src)
{
	do (*dest++ = *src);
	while (*src++ != '\0');
	return dest - 1;
}

//
char* qn_strcat(char* dest, const char* src)
{
#ifdef __GNUC__
	return strcat(dest, src);
#else
	char* o = dest;
	while (*dest) dest++;
	while ((*dest++ = *src++)) {}
	return o;
#endif
}

//
char* qn_strncat(char* dest, const char* src, size_t len)
{
#ifdef __GNUC__
	return strncat(dest, src, len);
#else
	char* o = dest;
	while (*dest) dest++;
	while (len && ((*dest++ = *src++)))
		--len;
	*dest = '\0';
	return o;
#endif
}

//
char* qn_strconcat(size_t max_len, char* dest, ...)
{
	va_list va;
	char* c = dest;
	char* s;

	va_start(va, dest);
	s = va_arg(va, char*);
	while (s)
	{
		while (*s)
		{
			if (--max_len == 0)
				goto pos_exit;
			*c++ = *s++;
		}
		s = va_arg(va, char*);
	}
pos_exit:
	va_end(va);
	*c = '\0';
	return dest;
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
	size_t c;
	for (c = 0; *sz && c < 256; sz++, c++)
		h = (h << 5) - h + (size_t)*sz;
	h = (h << 5) - h + c;
	return h;
}

//
size_t qn_strihash(const char* p)
{
	const char* sz = p;
	size_t h = (size_t)tolower(*sz);
	if (!h)
		return 0;
	size_t c;
	sz++;
	for (c = 0; *sz && c < 256; sz++, c++)
		h = (h << 5) - h + (size_t)tolower(*sz);
	h = (h << 5) - h + c;
	return h;
}

//
uint qn_strshash(const char* p)
{
	const char* sz = p;
	uint h = (uint)tolower(*sz);
	if (!h)
		return 0;
	uint c;
	sz++;
	for (c = 0; *sz && c < 256; sz++, c++)
		h = (h << 5) - h + (uint)tolower(*sz);
	h = (h << 5) - h + c;
	return h;
}

//
size_t qn_strphash(const char** p)
{
	return qn_strhash(*p);
}

//
int qn_strcmp(const char* p1, const char* p2)
{
#if defined _MSC_VER || defined __GNUC__
	return strcmp(p1, p2);
#else
	while (*p1 && *p1 == *p2)
		++p1, ++p2;
	return *p1 - *p2;
#endif
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
int qn_strpcmp(const char** p1, const char** p2)
{
	return qn_strcmp(*p1, *p2);
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
bool qn_strwcm(const char* string, const char* wild)
{
	const char* cp = NULL, * mp = NULL;
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
	return *wild == '\0';
}

//
bool qn_striwcm(const char* string, const char* wild)
{
	const char* cp = NULL, * mp = NULL;
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
	return *wild == '\0';
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
const char* qn_strbrk(const char* p, const char* c)
{
#if defined _MSC_VER || defined __GNUC__
	return strpbrk(p, c);
#else
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
#endif
}

//
char* qn_strchr(const char* p, int ch)
{
#if defined _MSC_VER || defined __GNUC__
	return strchr(p, ch);
#else
	while (*p && *p != (char)ch) p++;
	return (*p == (char)ch) ? (char*)p : NULL;
#endif
}

//
char* qn_strrchr(const char* p, int ch)
{
#if defined _MSC_VER || defined __GNUC__
	return strrchr(p, ch);
#else
	char* s = (char*)p;
	while (*s++);
	while (--s != p && *s != (char)ch);
	return (*s == (char)ch) ? (char*)s : NULL;
#endif
}

//
char* qn_strtok(_Inout_opt_z_ char* p, _In_z_ const char* sep, _Inout_ char** ctx)
{
#if defined _MSC_VER
	return strtok_s(p, sep, ctx);
#elif defined __GNUC__
	return strtok_r(p, sep, ctx);
#else
	qn_return_when_fail(p != NULL, NULL);
	qn_return_when_fail(sep != NULL, NULL);
	qn_return_when_fail(ctx != NULL, NULL);
	if (p)
		*ctx = (char*)p;
	else if (!*ctx)
		return NULL;
	char* s = *ctx;
	while (*s)
	{
		const char* d = sep;
		while (*d)
		{
			if (*s == *d)
			{
				*s++ = '\0';
				*ctx = s;
				return s - 1;
			}
			++d;
		}
		++s;
	}
	*ctx = NULL;
	return NULL;
#endif
}

//
const char* qn_strext(const char* p, const char* name, int separator)
{
	const size_t len = strlen(name);
	for (;;)
	{
		const char* t = strstr(p, name);
		if (t == NULL)
			break;
		if (t == p || *(t - 1) == separator)
		{
			const char* e = t + len;
			if (*e == '\0' || *e == separator)
				return t;
		}
		p = t + len;
	}
	return NULL;
}

//
char* qn_strmid(char* dest, const char* src, const size_t pos, const size_t len)
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
char* qn_strrem(char* p, const char* rmlist)
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
char* qn_strupr(char* p)
{
	for (char* s = p; *s; ++s)
		if ((*s >= 'a') && (*s <= 'z'))
			*s -= 'a' - 'A';
	return p;
}

//
char* qn_strlwr(char* p)
{
	for (char* s = p; *s; ++s)
		if ((*s >= 'A') && (*s <= 'Z'))
			*s += 'a' - 'A';
	return p;
}

//
int qn_strtoi(const char* p, const uint base)
{
	qn_return_when_fail(p != NULL, 0);
	qn_return_when_fail(base >= 2 && base < 32, 0);
	const byte* table = qn_num_base_table();
	uint v = 0;
	int sign = 1;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	if (*p == '+')
		p++;
	else if (*p == '-')
	{
		sign = -1;
		p++;
	}
	uint ch = table[(byte)*p++];
	while (ch < base)
	{
		v = v * base + ch;
		ch = table[(byte)*p++];
	}
	return sign * (int)v;
}

//
llong qn_strtoll(const char* p, const uint base)
{
	qn_return_when_fail(p != NULL, 0);
	qn_return_when_fail(base >= 2 && base < 32, 0);
	const byte* table = qn_num_base_table();
	ullong v = 0;
	llong sign = 1LL;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	if (*p == '+')
		p++;
	else if (*p == '-')
	{
		sign = -1LL;
		p++;
	}
	uint ch = table[(byte)*p++];
	while (ch < base)
	{
		v = v * base + ch;
		ch = table[(byte)*p++];
	}
	return sign * (llong)v;
}

//
float qn_strtof(const char* p)
{
	float f = 0.0f;
	int rsign = 1;
	int e = 0;
	int ch;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	if (*p == '+')
		p++;
	else if (*p == '-')
	{
		rsign = -1;
		p++;
	}
	while ((ch = (int)*p++) != '\0' && isdigit(ch))
		f = f * 10.0f + (float)(ch - '0');
	if (ch == '.')
	{
		while ((ch = (int)*p++) != '\0' && isdigit(ch))
		{
			f = f * 10.0f + (float)(ch - '0');
			e--;
		}
	}
	if (ch == 'e' || ch == 'E')
	{
		int sign = 1;
		int n = 0;
		ch = (int)*p++;
		if (ch == '+')
			ch = (int)*p++;
		else if (ch == '-')
		{
			ch = (int)*p++;
			sign = -1;
		}
		while (isdigit(ch))
		{
			n = n * 10 + (ch - '0');
			ch = (int)*p++;
		}
		e += n * sign;
	}
	while (e > 0)
	{
		f *= 10.0f;
		e--;
	}
	while (e < 0)
	{
		f *= 0.1f;
		e++;
	}
	return rsign < 0 ? -f : f;
}

//
double qn_strtod(const char* p)
{
	double d = 0.0;
	int rsign = 1;
	int e = 0;
	int ch;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	if (*p == '+')
		p++;
	else if (*p == '-')
	{
		rsign = -1;
		p++;
	}
	while ((ch = (int)*p++) != '\0' && isdigit(ch))
		d = d * 10.0 + (ch - '0');
	if (ch == '.')
	{
		while ((ch = (int)*p++) != '\0' && isdigit(ch))
		{
			d = d * 10.0 + (ch - '0');
			e--;
		}
	}
	if (ch == 'e' || ch == 'E')
	{
		int sign = 1;
		int n = 0;
		ch = (int)*p++;
		if (ch == '+')
			ch = (int)*p++;
		else if (ch == '-')
		{
			ch = (int)*p++;
			sign = -1;
		}
		while (isdigit(ch))
		{
			n = n * 10 + (ch - '0');
			ch = (int)*p++;
		}
		e += n * sign;
	}
	while (e > 0)
	{
		d *= 10.0;
		e--;
	}
	while (e < 0)
	{
		d *= 0.1;
		e++;
	}
	return rsign < 0 ? -d : d;
}

//
int qn_itoa(char* p, const int n, const uint base, bool upper)
{
	qn_return_when_fail(base <= 32, -1);
	const char* table = qn_char_base_table(upper);
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
		conv[place++] = table[uvalue % base];
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
	qn_return_when_fail(base <= 32, -1);
	const char* table = qn_char_base_table(upper);
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
		conv[place++] = table[uvalue % base];
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
void qn_divpath(const char* p, char* dir, char* filename)
{
	const char* end = p;
	const char* sep = NULL;
	while (*end)
	{
		if (*end == '/' || *end == '\\')
			sep = end;
		++end;
	}
	if (sep)
	{
		if (dir)
			qn_strncpy(dir, p, sep - p + 1);
		if (filename)
			qn_strcpy(filename, sep + 1);
	}
	else
	{
		if (dir)
			*dir = '\0';
		if (filename)
			qn_strcpy(filename, p);
	}
}

//
void qn_splitpath(const char* p, char* drive, char* dir, char* name, char* ext)
{
	const char* path = p;
	const char* s = path;
	const char* end = path;
	const char* dot = NULL;
	const char* sep = NULL;
	while (*end)
	{
		if (*end == '/' || *end == '\\')
			sep = end;
		else if (*end == '.')
			dot = end;
		++end;
	}
	if (sep)
	{
		if (drive)
		{
			if (sep - s > 1 && s[1] == ':')
			{
				if (drive)
					qn_strncpy(drive, path, 2);
			}
			else
				*drive = '\0';
		}
		if (dir)
			qn_strncpy(dir, path, sep - path + 1);
	}
	else
	{
		if (drive)
			*drive = '\0';
		if (dir)
			*dir = '\0';
	}
	if (dot && dot >= sep)
	{
		if (name)
			qn_strncpy(name, sep + 1, dot - sep - 1);
		if (ext)
		{
			char* x = qn_stpcpy(ext, dot);
			*x = '\0';
		}
	}
	else
	{
		if (name)
			qn_strcpy(name, sep + 1);
		if (ext)
			*ext = '\0';
	}
}


//////////////////////////////////////////////////////////////////////////
// 유니코드 변환

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
		// 사용하지 않는 문자 코드
		// 0xFFFFFFFF
		return (uchar4)-1;
	}

	// UCS4로 변환
	uchar4 ret = (uchar4)(p[0] & mask);
	for (int i = 1; i < len; i++)
	{
		if ((p[i] & 0xC0) != 0x80)
			return 0;

		ret <<= 6;
		ret |= (p[i] & 0x3F);
	}

	return ret;
}

//
uchar4 qn_u8cbc(const char* p, int* len)
{
	int mask;
	const byte ch = (byte)*p;

	if (ch < 128)
	{
		*len = 1;
		mask = 0x7f;
	}
	else if ((ch & 0xe0) == 0xc0)
	{
		*len = 2;
		mask = 0x1f;
	}
	else if ((ch & 0xf0) == 0xe0)
	{
		*len = 3;
		mask = 0x0f;
	}
	else if ((ch & 0xf8) == 0xf0)
	{
		*len = 4;
		mask = 0x07;
	}
	else if ((ch & 0xfc) == 0xf8)
	{
		*len = 5;
		mask = 0x03;
	}
	else if ((ch & 0xfe) == 0xfc)
	{
		*len = 6;
		mask = 0x01;
	}
	else
	{
		// 사용하지 않는 문자 코드
		// 0xFFFFFFFF
		return (uchar4)-1;
	}

	// UCS4로 변환
	uchar4 ret = (uchar4)(p[0] & mask);
	for (int i = 1; i < *len; i++)
	{
		if ((p[i] & 0xC0) != 0x80)
			return 0;

		ret <<= 6;
		ret |= (p[i] & 0x3F);
	}

	return ret;
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
#if false
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
char* qn_u8ncpy(char* dest, const char* src, size_t len)
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
size_t qn_u8lcpy(char* dest, const char* src, size_t len)
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
		out[len] = '\0';
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
size_t qn_mbstowcs(wchar* outwcs, const size_t outsize, const char* inmbs, const size_t insize)
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
size_t qn_wcstombs(char* outmbs, const size_t outsize, const wchar* inwcs, const size_t insize)
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
size_t qn_u8to32(uchar4* dest, const size_t destsize, const char* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	if (dest == NULL || destsize == 0)
	{
		// utf-8 -> ucs-4의 길이는.... 그냥 utf-8의 길이.
		return qn_u8len(src);
	}
	else
	{
		const size_t slen = srclen == 0 ? qn_u8len(src) : srclen;
		const size_t size = QN_MIN(destsize - 1, slen);

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
size_t qn_u8to16(uchar2* dest, const size_t destsize, const char* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	if (dest == NULL || destsize == 0)
	{
		// utf-8 -> ucs-4의 길이는.... 그냥 utf-8의 길이.
		// 다만 서로게이트 문제는 어떻게 하나... 일단 이걸로 하고 나중에 고치자
		// .... 서로게이트 문제 원래 발생 안한다 [2024-01-12 kim]
		return qn_u8len(src);
	}
	else
	{
		const size_t slen = srclen == 0 ? qn_u8len(src) : srclen;
		const size_t size = QN_MIN(destsize - 1, slen);

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
size_t qn_u32to8(char* dest, size_t destsize, const uchar4* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	const nint slen = (nint)srclen;
	size_t n, size;
	intptr_t i;
	uchar4 uc;

	if (destsize > 0)
	{
		destsize--;
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
size_t qn_u16to8(char* dest, size_t destsize, const uchar2* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	const uchar2* cp = src;
	uchar4 hsg = 0;
	size_t size = 0;
	uchar4 uc;
	uchar2 ch;
	size_t n;

	if (destsize > 0)
	{
		destsize--;
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
size_t qn_u16to32(uchar4* dest, size_t destsize, const uchar2* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	uchar4 hsg = 0;
	size_t size = 0;

	if (destsize > 0)
	{
		destsize--;
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
size_t qn_u32to16(uchar2* dest, size_t destsize, const uchar4* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	size_t size = 0;
	const intptr_t slen = (intptr_t)srclen;
	size_t z;
	intptr_t i;

	if (destsize > 0)
	{
		destsize--;
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

#ifdef QS_NO_MEMORY_PROFILE
#define DEF_UTF_DUP(name, in_type, out_type)\
	out_type* qn_a_##name(const in_type* src, size_t srclen) {\
		size_t len=qn_##name(NULL,0,src,srclen)+1; qn_return_on_ok(len<2,NULL);\
		out_type* buf=qn_a_alloc(len*sizeof(out_type), false); qn_##name(buf,len,src,srclen);/* NOLINT */\
		return buf; }
#else
#define DEF_UTF_DUP(name, in_type, out_type)\
	out_type* qn_a_i_##name(const in_type* src, size_t srclen, const char* desc, size_t line) {\
		size_t len=qn_##name(NULL,0,src,srclen)+1; qn_return_on_ok(len<2,NULL);\
		out_type* buf=qn_a_i_alloc(len*sizeof(out_type), false, desc, line); qn_##name(buf,len,src,srclen);/* NOLINT */\
		return buf; }
#endif
DEF_UTF_DUP(mbstowcs, char, wchar)
DEF_UTF_DUP(wcstombs, wchar, char)
DEF_UTF_DUP(u8to32, char, uchar4)
DEF_UTF_DUP(u8to16, char, uchar2)
DEF_UTF_DUP(u32to8, uchar4, char)
DEF_UTF_DUP(u16to8, uchar2, char)
DEF_UTF_DUP(u16to32, uchar2, uchar4)
DEF_UTF_DUP(u32to16, uchar4, uchar2)
#undef DEF_UTF_DUP


//////////////////////////////////////////////////////////////////////////
// 한글 분석

// 초성: ㄱㄲㄴㄷㄸㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ
// 중성: ㅏㅐㅑㅒㅓⅩⅩㅔㅕㅖㅗㅘㅙⅩⅩㅚㅛㅜㅝㅞㅟⅩⅩㅠㅡㅢㅣ<아><어>
// 종성: ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁ<ㅎㅎ>ㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ<ㅇ>


//
bool qn_hangul_dcp(uchar4 code, int* cho_jung_jong)
{
	qn_return_when_fail(cho_jung_jong, false);
	if ((code >= 0xAC00) && (code <= 0xD7A3))
	{
		static const char cho_index[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20 };
		static const char jung_index[] = { 3, 4, 5, 6, 7,10,11,12, 13,14,15,18,19,20,21,22,23,26,27,28,29 };
		static const char jong_index[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,19,20,21,22,23,24,25,26,27,28,29 };
		uchar4 c = code - 0xAC00;
		cho_jung_jong[0] = cho_index[(int)(c / 588)];
		cho_jung_jong[1] = jung_index[(int)((c % 588) / 28)];
		cho_jung_jong[2] = jong_index[(int)(c % 28)];
		return true;
	}
	if ((code >= 0x3131) && code <= 0x314E)
	{
		// 자음만
		// ㄱㄲ　ㄴ　　ㄷㄸㄹ　　　　　　　ㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ
		// ㄱㄲㄳㄴㄵㄶㄷㄸㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅃㅄㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ
		static const char jaum_index[] = { 2, 3, -1, 4, -1, -1, 5, 6, 7, -1, -1, -1, -1, -1, -1, -1, 8, 9, 10, -1, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
		byte c = (byte)(code - 0x3131);
		cho_jung_jong[0] = jaum_index[c];
		cho_jung_jong[1] = -1;
		cho_jung_jong[2] = -1;
		return true;
	}
	if ((code >= 0x314F) && code <= 0x3163)
	{
		// 모음만
		// ㅏㅐㅑㅒㅓⅩⅩㅔㅕㅖㅗㅘㅙⅩⅩㅚㅛㅜㅝㅞㅟⅩⅩㅠㅡㅢㅣ
		// ㅏㅐㅑㅒㅓ　　ㅔㅕㅖㅗㅘㅙ　　ㅚㅛㅜㅝㅞㅟ　　ㅠㅡㅢㅣ
		static const char moum_index[] = { 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 18, 19, 20, 21, 22, 23, 26, 27, 28, 29 };
		byte c = (byte)(code - 0x314F);
		cho_jung_jong[0] = -1;
		cho_jung_jong[1] = moum_index[c];
		cho_jung_jong[2] = -1;
		return true;
	}
	/*
	if ((code >= 0xffa1) && (code <= 0xffdc))
	{
		// 한글 반각인데 취급 안함
		return false;
	}
	*/
	cho_jung_jong[0] = cho_jung_jong[1] = cho_jung_jong[2] = -1;
	return false;
}

//
uchar4 qn_hangul_josa(uchar4 code, int josa_type)
{
	uchar4 c;
	if (code > 0xAC00 && code <= 0xD7A3)
	{
		c = code - 0xAC00;
		c = (uchar4)(c % 28);
	}
	else
		c = 0;
	static const char* josadeul[][2] =
	{
		{ "은", "는" },
		{ "이", "가" },
		{ "을", "를" },
		{ "과", "와" },
	};
	const char* josa = josadeul[josa_type][c > 0 ? 0 : 1];
	return qn_u8cbn(josa);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
