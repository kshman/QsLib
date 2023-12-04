﻿/*
* Copyright Patrick Powell 1995
* This code is based on code written by Patrick Powell (papowell@astart.com)
* It may be used for any purpose as long as this notice remains intact
* on all source code distributions
*/

/**************************************************************
* Original:
* Patrick Powell Tue Apr 11 09:48:21 PDT 1995
* A bombproof version of doprnt (dopr) included.
* Sigh.  This sort of thing is always nasty do deal with.  Note that
* the version here does not include floating point...
*
* snprintf() is used instead of sprintf() as it does limit checks
* for string length.  This covers a nasty loophole.
*
* The other functions are there to prevent NULL pointers from
* causing nast effects.
*
* More Recently:
*  Brandon Long <blong@fiction.net> 9/15/96 for mutt 0.43
*  This was ugly.  It is still ugly.  I opted out of floating point
*  numbers, but the formatter understands just about everything
*  from the normal C string format, at least as far as I can tell from
*  the Solaris 2.5 printf(3S) man page.
*
*  Brandon Long <blong@fiction.net> 10/22/97 for mutt 0.87.1
*    Ok, added some minimal floating point support, which means this
*    probably requires libm on most operating systems.  Don't yet
*    support the exponent (e,E) and sigfig (g,G).  Also, _fmt_int()
*    was pretty badly broken, it just wasn't being exercised in ways
*    which showed it, so that's been fixed.  Also, formated the code
*    to mutt conventions, and removed dead code left over from the
*    original.  Also, there is now a builtin-test, just compile with:
*           gcc -DTEST_SNPRINTF -o snprintf snprintf.c -lm
*    and run snprintf for results.
*
*  Thomas Roessler <roessler@guug.de> 01/27/98 for mutt 0.89i
*    The PGP code was using unsigned hexadecimal formats.
*    Unfortunately, unsigned formats simply didn't work.
*
*  Michael Elkins <me@cs.hmc.edu> 03/05/98 for mutt 0.90.8
*    The original code assumed that both snprintf() and vsnprintf() were
*    missing.  Some systems only have snprintf() but not vsnprintf(), so
*    the code is now broken down under HAVE_SNPRINTF and HAVE_VSNPRINTF.
*
*  Andrew Tridgell (tridge@samba.org) Oct 1998
*    fixed handling of %.0f
*    added test for HAVE_LONG_DOUBLE
*
* tridge@samba.org, idra@samba.org, April 2001
*    got rid of fcvt code (twas buggy and made testing harder)
*    added C99 semantics
*
* HT authors
*    ht_snprintf/ht_vsnprintf return number of characters actually
*      written instead of the number of characters that would
*      have been written.
*    added '%y' to allow object output using Object's toString() method.
*    added '%q[dioux]' for formatting qwords.
*    added '%b' for formatting in binary notation.
*
* hbcs version
*    add '%S', '%C', '%ls', '%lc', '%hS', '%hC' for unicode
*    add '%LnioubxX' in integer for size_t
*    remove '%y'
*    more 'C99' compatibility
*    file output
*
* qn version
*    integer types are referenced from stdint.h
*    remove file output
**************************************************************/

#include "pch.h"
#include "qs_qn.h"
#include <locale.h>

/*
* dopr(): poor man's version of doprintf
*/

#if !_QN_MOBILE_			// 모바일에서 로캘 안됨
#define USE_LOCALE_INFO	1
#else
#define USE_LOCALE_INFO	0
#endif

#if _MSC_VER				// MSVC만 와이드 로캘
#define USE_WIDE_LOCALE	1
#else
#define USE_WIDE_LOCALE	0
#endif

// format read states
#define DP_S_DEFAULT	0
#define DP_S_FLAGS		1
#define DP_S_MIN		2
#define DP_S_DOT		3
#define DP_S_MAX		4
#define DP_S_MOD		5
#define DP_S_CONV		6
#define DP_S_DONE		7

// format flags - Bits
#define DP_F_MINUS      QN_BIT(0)
#define DP_F_PLUS       QN_BIT(1)
#define DP_F_SPACE      QN_BIT(2)
#define DP_F_NUM        QN_BIT(3)
#define DP_F_ZERO       QN_BIT(4)
#define DP_F_UP         QN_BIT(5)
#define DP_F_UNSIGNED   QN_BIT(6)
#define DP_F_QUOTE		QN_BIT(7)
#define DP_F_G			QN_BIT(8)
#define DP_F_E			QN_BIT(9)
#define DP_F_UNICODE	QN_BIT(10)

// Conversion Flags
#define DP_C_SHORT		1
#define DP_C_LONG		2
#define DP_C_LLONG		3
#define DP_C_LDOUBLE	4
#define DP_C_SIZE		5
#define DP_C_INTMAX		6
#define DP_C_PTRDIFF	7
#define DP_C_CHAR		8

//
typedef union sn_any
{
	short				h;
	int					i;
	long				g;
	llong				ll;

	ushort				uh;
	uint				ui;
	unsigned long		ug;
	ullong				ull;

	double				d;

	char*				s;
	wchar*				w;

	// C99
	size_t				sz;
	long double			ld;
	intmax_t			im;
	ptrdiff_t			ff;

	//
	short*				ph;
	int*				pi;
	long*				pg;
	llong*				pll;
	size_t*				psz;
	intmax_t*			pim;
	ptrdiff_t*			pff;
} sn_any;


//////////////////////////////////////////////////////////////////////////
// 인라인 도구

QN_INLINE double pp_pow10(int exp)
{
	double result = 1;
	while (exp > 0)
	{
		result *= 10;
		exp--;
	}
	while (exp < 0)
	{
		result /= 10;
		exp++;
	}
	return result;
}

QN_INLINE double pp_round(double value)
{
	ullong intpart = (ullong)value;
	value = value - intpart;
	return (double)(value >= 0.5 ? intpart + 1 : intpart);
}

QN_INLINE bool pp_is_nan(double value)
{
	return value != value;
}

QN_INLINE bool pp_is_inf(double value)
{
	return value != 0.0 && value + value == value;
}

QN_INLINE int pp_exponent(double value)
{
	double d = value < 0.0 ? -value : value;
	int exp = 0;
	while (d < 1.0 && d > 0.0 && --exp > -99) d *= 10;
	while (d >= 10.0 && ++exp < 99) d /= 10;
	return exp;
}

QN_INLINE ullong pp_intpart(double value)
{
	if (value >= (double)UINT64_MAX)
		return UINT64_MAX;
	ullong res = (ullong)value;
	return res <= value ? res : res - 1;
}


//////////////////////////////////////////////////////////////////////////
// 아스키 버전

// 문자를 숫자로
#define char_to_int(p)	((p) - '0')

// 진수별 처리
static const char* _str_number[2] =
{
	"0123456789abcdef",
	"0123456789ABCDEF",
};

// 포인터 표시를 어떤거 쓰나
#if _QN_64_
#define _str_fmt_size		_str_fmt_long
#else
#define _str_fmt_size		_str_fmt_int
#endif

// 32비트 숫자를 문자열로
static int _str_to_int(uint value, char* buf, int size, int base, bool caps)
{
	const char* bts = _str_number[caps ? 1 : 0];
	int pos = 0;

	do
	{
		buf[pos++] = bts[value % base];
		value /= base;
	} while (value && pos < size);

	buf[pos] = '\0';

	return pos;
}

// 64비트 숫자를 문자열로
static int _str_to_long(ullong value, char* buf, int size, int base, bool caps)
{
	const char* bts = _str_number[caps ? 1 : 0];
	int pos = 0;

	do
	{
		buf[pos++] = bts[value % base];
		value /= base;
	} while (value && pos < size);

	buf[pos] = '\0';

	return pos;
}

// 숫자 분리 (보통 천단위 통화 분리자, 한자 문화권은 만단위가 좋은데)
static int _str_quote_seps(int value, int flags
#if USE_LOCALE_INFO
	, struct lconv* lc
#endif
)
{
	int ret;

	if (!(flags & DP_F_QUOTE))
		ret = 0;
	else
	{
		ret = (value - ((value % 3) == 0 ? 1 : 0)) / 3;
#if USE_LOCALE_INFO
		if (lc->thousands_sep)
			ret *= (int)strlen(lc->thousands_sep);
#endif
	}

	return ret;
}

// 글자 넣기
static void _str_outch(char* buffer, size_t* currlen, size_t maxlen, char ch)
{
	if (*currlen < maxlen)
		buffer[(*currlen)] = ch;
	(*currlen)++;
}

// 문자열 널기
static void _str_outs(char* buffer, size_t* currlen, size_t maxlen, const char* value, int len, int flags, int vmin, int vmax)
{
	int padlen = vmin - len;

	if (padlen < 0)
		padlen = 0;
	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	int cnt = 0;
	while (padlen > 0 && cnt < vmax)
	{
		_str_outch(buffer, currlen, maxlen, ' ');
		--padlen;
		++cnt;
	}
	while (*value && cnt < vmax)
	{
		_str_outch(buffer, currlen, maxlen, *value++);
		++cnt;
	}
	while (padlen < 0 && cnt < vmax)
	{
		_str_outch(buffer, currlen, maxlen, ' ');
		++padlen;
		++cnt;
	}
}

// 글자 변환
static void _str_fmt_char(char* buffer, size_t* currlen, size_t maxlen, int ch, int flags)
{
	if ((flags & DP_F_UNICODE) == 0)
		_str_outch(buffer, currlen, maxlen, (char)ch);
	else
	{
		wchar_t wcs[2] = { (wchar)ch, L'\0' };
		char mbs[6];
		int len = (int)qn_wcstombs(mbs, 6, wcs);
		if (len > 0)
			_str_outs(buffer, currlen, maxlen, mbs, len, 0, 0, len);
	}
}

// 문자열 변환
static void _str_fmt_str(char* buffer, size_t* currlen, size_t maxlen, const char* value, int flags, int vmin, int vmax)
{
	if (!value)
	{
		// 널
		value = "(null)";
		if (vmax < 0)
			vmax = 6;
		if (vmin > 0 && vmin > vmax)
			vmax = vmin;
		_str_outs(buffer, currlen, maxlen, value, 6, flags, vmin, vmax);
	}
	else if ((flags & DP_F_UNICODE) == 0)
	{
		// 그냥 문자
		int len = (int)strlen(value);
		if (vmax < 0)
			vmax = len;
		if (vmin > 0 && vmin > vmax)
			vmax = vmin;
		_str_outs(buffer, currlen, maxlen, value, len, flags, vmin, vmax);
	}
	else
	{
		// 와이드 문자
		const wchar* w = (const wchar*)value;
		int len = (int)qn_wcstombs(NULL, 0, w);
		if (vmax < 0)
		{
			value = "(unicode)";
			if (vmax < 0)
				vmax = 9;
			if (vmin > 0 && vmin > vmax)
				vmax = vmin;
			_str_outs(buffer, currlen, maxlen, value, 9, flags, vmin, vmax);
		}
		else
		{
			char* s = qn_alloca(len + 1, char);
			qn_wcstombs(s, (size_t)len + 1, w);
			if (vmax < 0)
				vmax = len;
			if (vmin > 0 && vmin > vmax)
				vmax = vmin;
			_str_outs(buffer, currlen, maxlen, s, len, flags, vmin, vmax);
			qn_freea(s);
		}
	}
}

// 32비트 정수
static void _str_fmt_int(char* buffer, size_t* currlen, size_t maxlen, sn_any* value, uint base, int vmin, int vmax, int flags)
{
#define MAX_CONVERT_PLACES 40
	uint uvalue;
	char convert[MAX_CONVERT_PLACES];
	char hexprefix;
	char signvalue;
	int place;
	int spadlen;
	int zpadlen;
	int quote;
#if USE_LOCALE_INFO
	struct lconv* lc = localeconv();	// CRT 로캘
#endif

	//
	if (vmax < 0)
		vmax = 0;
	if (flags & DP_F_UNSIGNED)
	{
		uvalue = value->ui;
		signvalue = '\0';
	}
	else
	{
		if (value->i < 0)
		{
			uvalue = -value->i;
			signvalue = '-';
		}
		else
		{
			uvalue = value->i;

			if (flags & DP_F_PLUS)  // 부호추가 (+/i)
				signvalue = '+';
			else if (flags & DP_F_SPACE)
				signvalue = ' ';
			else
				signvalue = '\0';
		}
	}

	place = _str_to_int(uvalue, convert, MAX_CONVERT_PLACES - 1, base, flags & DP_F_UP);

	// '#'이 있을때 접두사 처리
	if ((flags & DP_F_NUM) && uvalue != 0)
	{
		if (base == 16)
			hexprefix = flags & DP_F_UP ? 'X' : 'x';
		else
		{
			hexprefix = '\0';

			if (base == 8 && vmax <= place)
				vmax = place + 1;
		}
	}
	else
		hexprefix = '\0';

	//
#if USE_LOCALE_INFO
	quote = _str_quote_seps(place, flags, lc);
#else
	quote = _str_quote_seps(place, flags);
#endif
	zpadlen = vmax - place - quote;
	spadlen = vmin - quote - QN_MAX(vmax, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

	if (zpadlen < 0)
		zpadlen = 0;
	if (spadlen < 0)
		spadlen = 0;
	if (flags & DP_F_ZERO)
	{
		zpadlen = QN_MAX(zpadlen, spadlen);
		spadlen = 0;
	}
	if (flags & DP_F_MINUS)
		spadlen = -spadlen; // 왼쪽 정렬

	// 선두 공백
	while (spadlen > 0)
	{
		_str_outch(buffer, currlen, maxlen, ' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		_str_outch(buffer, currlen, maxlen, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		_str_outch(buffer, currlen, maxlen, '0');
		_str_outch(buffer, currlen, maxlen, hexprefix);
	}

	// '0' 삽입
	while (zpadlen > 0)
	{
		_str_outch(buffer, currlen, maxlen, '0');
		--zpadlen;
	}

	// 숫자
	while (place > 0)
	{
		--place;
		_str_outch(buffer, currlen, maxlen, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				_str_outch(buffer, currlen, maxlen, ',');
			else
				for (const char* psz = lc->thousands_sep; *psz; psz++)
					_str_outch(buffer, currlen, maxlen, *psz);
#else
			_str_outch(buffer, currlen, maxlen, ',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		_str_outch(buffer, currlen, maxlen, ' ');
		++spadlen;
	}
#undef MAX_CONVERT_PLACES
}

// 64비트 정수
static void _str_fmt_long(char* buffer, size_t* currlen, size_t maxlen, sn_any* value, uint base, int vmin, int vmax, int flags)
{
#define MAX_CONVERT_PLACES 80
	ullong uvalue;
	char convert[MAX_CONVERT_PLACES];
	char hexprefix;
	char signvalue;
	int place;
	int spadlen;
	int zpadlen;
	int quote;
#if USE_LOCALE_INFO
	struct lconv* lc = localeconv();
#endif

	//
	if (vmax < 0)
		vmax = 0;
	if (flags & DP_F_UNSIGNED)
	{
		uvalue = value->ull;
		signvalue = '\0';
	}
	else
	{
		if (value->ll < 0)
		{
			uvalue = -value->ll;
			signvalue = '-';
		}
		else
		{
			uvalue = value->ll;

			if (flags & DP_F_PLUS)  // 부호추가 (+/i)
				signvalue = '+';
			else if (flags & DP_F_SPACE)
				signvalue = ' ';
			else
				signvalue = '\0';
		}
	}

	place = _str_to_long(uvalue, convert, MAX_CONVERT_PLACES - 1, base, flags & DP_F_UP);

	// '#'이 있을때 접두사 처리
	if ((flags & DP_F_NUM) && uvalue != 0)
	{
		if (base == 16)
			hexprefix = flags & DP_F_UP ? 'X' : 'x';
		else
		{
			hexprefix = '\0';

			if (base == 8 && vmax <= place)
				vmax = place + 1;
		}
	}
	else
		hexprefix = '\0';

	//
#if USE_LOCALE_INFO
	quote = _str_quote_seps(place, flags, lc);
#else
	quote = _str_quote_seps(place, flags);
#endif
	zpadlen = vmax - place - quote;
	spadlen = vmin - quote - QN_MAX(vmax, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

	if (zpadlen < 0)
		zpadlen = 0;
	if (spadlen < 0)
		spadlen = 0;
	if (flags & DP_F_ZERO)
	{
		zpadlen = QN_MAX(zpadlen, spadlen);
		spadlen = 0;
	}
	if (flags & DP_F_MINUS)
		spadlen = -spadlen; // 왼쪽 정렬

	// 앞쪽 공백
	while (spadlen > 0)
	{
		_str_outch(buffer, currlen, maxlen, ' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		_str_outch(buffer, currlen, maxlen, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		_str_outch(buffer, currlen, maxlen, '0');
		_str_outch(buffer, currlen, maxlen, hexprefix);
	}

	// '0' 삽입
	if (zpadlen > 0)
	{
		while (zpadlen > 0)
		{
			_str_outch(buffer, currlen, maxlen, '0');
			--zpadlen;
		}
	}

	// 숫자
	while (place > 0)
	{
		--place;
		_str_outch(buffer, currlen, maxlen, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				_str_outch(buffer, currlen, maxlen, ',');
			else
				for (const char* psz = lc->thousands_sep; *psz; psz++)
					_str_outch(buffer, currlen, maxlen, *psz);
#else
			_str_outch(buffer, currlen, maxlen, ',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		_str_outch(buffer, currlen, maxlen, ' ');
		++spadlen;
	}
#undef MAX_CONVERT_PLACES
}

// 64비트 실수(double)
// 여기서는, 오직 16자리 정수만 지원. 원래는 9, 19, 38 자리수에 따라 각각 32, 64, 128비트용이 있어야함
static void _str_fmt_fp(char* buffer, size_t* currlen, size_t maxlen, sn_any* value, int vmin, int vmax, int flags)
{
	double fvalue;
	double uvalue;
	ullong intpart;
	ullong fracpart;
	ullong mask;
	char iconvert[311];
	char fconvert[311];
	char econvert[10];
	char signvalue;
	char esignvalue;
	int iplace;
	int fplace;
	int eplace;
	int padlen;
	int zleadfrac;
	int dotpoint;
	int quote;
	int etype;
	int exponent;
	int omitzeros;
	int omitcount;
	const char* psz;
#if USE_LOCALE_INFO
	struct lconv* lc = localeconv();
#endif

	/*
	* AIX manpage says the default is 0, but Solaris says the default
	* is 6, and sprintf on AIX defaults to 6
	*/
	if (vmax < 0)
		vmax = 6;

	fvalue = value->d;
	if (fvalue < 0)
		signvalue = '-';
	else if (flags & DP_F_PLUS) // 부호추가 (+/i)
		signvalue = '+';
	else if (flags & DP_F_SPACE)
		signvalue = ' ';
	else
		signvalue = '\0';

	// NAN, INF 처리
	if (pp_is_nan(fvalue))
		psz = flags & DP_F_UP ? "NAN" : "nan";
	else if (pp_is_inf(fvalue))
		psz = flags & DP_F_UP ? "INF" : "inf";
	else
		psz = NULL;

	if (psz)
	{
		iplace = 0;
		if (signvalue)
			iconvert[iplace++] = signvalue;
		while (*psz)
			iconvert[iplace++] = *psz++;
		iconvert[iplace] = '\0';

		_str_outs(buffer, currlen, maxlen, iconvert, iplace, flags, vmin, iplace);
		return;
	}

	// 지수 모양 처리
	if (flags & (DP_F_E | DP_F_G))
	{
		if (flags & DP_F_G)
		{
			--vmax;
			omitzeros = !(flags & DP_F_NUM);
		}
		else
			omitzeros = 0;

		exponent = pp_exponent(fvalue);
		etype = 1;
	}
	else
	{
		exponent = 0;
		omitzeros = 0;
		etype = 0;
	}

pos_exp_again: // 이 루프는 확실한 반올림 지수를 얻기 위함 -> '%g'
	if (vmax > 16)
		vmax = 16;

	// 정수로 변환
	uvalue = QN_ABS(fvalue);
	if (etype)
		uvalue /= pp_pow10(exponent);
	intpart = pp_intpart(uvalue);
	mask = (ullong)pp_pow10(vmax);
	fracpart = (ullong)pp_round(mask * (uvalue - intpart));

	if (fracpart >= mask)
	{
		// 예를 들면 uvalue=1.99952, intpart=2, temp=1000 (max=3 이므로)
		// pp_round(1000*0.99952)=1000. 그리하여, 정수부는 1증가, 실수부는 0으로 함.
		intpart++;
		fracpart = 0;

		if (etype && intpart == 10)
		{
			intpart = 1;
			exponent++;
		}
	}

	// 정확한 지수를 얻음. '%g'를 위한 지수를 재계산
	if (etype && (flags & DP_F_G) && (vmax + 1) > exponent && exponent >= -4)
	{
		vmax -= exponent;
		etype = 0;
		goto pos_exp_again;
	}

	if (etype)
	{
		if (exponent < 0)
		{
			exponent = -exponent;
			esignvalue = '-';
		}
		else
			esignvalue = '+';

		eplace = _str_to_int(exponent, econvert, 2, 10, false);

		if (eplace == 1)
		{
			// MSVC는 자리수를 3자리 씀. (001+E)
			// 여기서는 2자리만. (01+E)
			econvert[eplace++] = '0';
		}
		econvert[eplace++] = esignvalue;
		econvert[eplace++] = flags & DP_F_UP ? 'E' : 'e';
		econvert[eplace] = '\0';
	}
	else
	{
		esignvalue = '\0';
		eplace = 0;
	}

	// 정수부 처리
	iplace = _str_to_long(intpart, iconvert, 311 - 1, 10, false);

	// 실수부 처리
	// iplace=1 이면 fracpart==0 임.
	fplace = fracpart == 0 ? 0 : _str_to_long(fracpart, fconvert, 311 - 1, 10, false);

	// 지수부 처리
	zleadfrac = vmax - fplace;

	omitcount = 0;

	if (omitzeros)
	{
		if (fplace > 0)
		{
			while (omitcount < fplace && fconvert[omitcount] == '0')
				omitcount++;
		}
		else
		{
			// 실수부가 0, 걍 무시함
			omitcount = vmax;
			zleadfrac = 0;
		}
		vmax -= omitcount;
	}

	// 실수부가 0이 아니고, '#' 플래그가 있으면 정수 포인트 출력
	dotpoint = vmax > 0 || (flags & DP_F_NUM);

	//
#if USE_LOCALE_INFO
	quote = _str_quote_seps(iplace, flags, lc);
#else
	quote = _str_quote_seps(iplace, flags);
#endif

	// -1은 정수 포인트용, 부호를 출력하면 추가로 -1;
	padlen = vmin - iplace - eplace - vmax - quote - (dotpoint ? 1 : 0) - (signvalue ? 1 : 0);
	if (padlen < 0)
		padlen = 0;
	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	// 부호 및 패딩 숫자 출력
	if ((flags & DP_F_ZERO) && padlen > 0)
	{
		if (signvalue)
		{
			_str_outch(buffer, currlen, maxlen, signvalue);
			signvalue = '\0';
		}
		while (padlen > 0)
		{
			_str_outch(buffer, currlen, maxlen, '0');
			--padlen;
		}
	}

	// 남은 패딩 공백 출력
	while (padlen > 0)
	{
		_str_outch(buffer, currlen, maxlen, ' ');
		--padlen;
	}

	// 패딩에서 부호 출력 안했으면 부호 출력
	if (signvalue)
		_str_outch(buffer, currlen, maxlen, signvalue);

	// 정수부 출력
	while (iplace > 0)
	{
		--iplace;
		_str_outch(buffer, currlen, maxlen, iconvert[iplace]);

		if (quote && iplace > 0 && (iplace % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				_str_outch(buffer, currlen, maxlen, ',');
			else
				for (psz = lc->thousands_sep; *psz; psz++)
					_str_outch(buffer, currlen, maxlen, *psz);
#else
			_str_outch(buffer, currlen, maxlen, ',');
#endif
		}
	}

	// 소수점
	if (dotpoint)
	{
#if USE_LOCALE_INFO
		if (!lc->decimal_point)
			_str_outch(buffer, currlen, maxlen, '.');
		else
			for (psz = lc->decimal_point; *psz; psz++)
				_str_outch(buffer, currlen, maxlen, *psz);
#else
		_str_outch(buffer, currlen, maxlen, '.');
#endif
	}

	// 실수부 앞쪽 패딩
	while (zleadfrac > 0)
	{
		_str_outch(buffer, currlen, maxlen, '0');
		--zleadfrac;
	}

	// 실수부
	while (fplace > omitcount)
		_str_outch(buffer, currlen, maxlen, fconvert[--fplace]);

	// 지수값
	while (eplace > 0)
		_str_outch(buffer, currlen, maxlen, econvert[--eplace]);

	// 남은 패팅
	while (padlen < 0)
	{
		_str_outch(buffer, currlen, maxlen, ' ');
		++padlen;
	}
}

//
size_t dopr(char* buffer, size_t maxlen, const char* format, va_list args)
{
	int vmin = 0;
	int vmax = -1;
	int state = DP_S_DEFAULT;
	int flags = 0;
	int cflags = 0;
	int base = 0;
	size_t currlen = 0;
	sn_any value = { .ff = 0 };

	if (!buffer)
		maxlen = 0;

	char ch = *format++;
	while (state != DP_S_DONE)
	{
		if (ch == '\0')
			state = DP_S_DONE;

		switch (state)
		{
			case DP_S_DEFAULT:
				if (ch == '%')
					state = DP_S_FLAGS;
				else
					_str_outch(buffer, &currlen, maxlen, ch);
				ch = *format++;
				break;

			case DP_S_FLAGS:
				switch (ch)
				{
					case '-':
						flags |= DP_F_MINUS;
						ch = *format++;
						break;
					case '+':
						flags |= DP_F_PLUS;
						ch = *format++;
						break;
					case ' ':
						flags |= DP_F_SPACE;
						ch = *format++;
						break;
					case '#':
						flags |= DP_F_NUM;
						ch = *format++;
						break;
					case '0':
						flags |= DP_F_ZERO;
						ch = *format++;
						break;
					case '\'':
						flags |= DP_F_QUOTE;
						ch = *format++;
						break;
					default:
						state = DP_S_MIN;
						break;
				}   // switch - ch
				break;

			case DP_S_MIN:
				if (isdigit(ch))
				{
					vmin = 10 * vmin + char_to_int(ch);
					ch = *format++;
				}
				else if (ch == '*')
				{
					vmin = va_arg(args, int);
					if (vmin < 0)
					{
						flags |= DP_F_MINUS;
						vmin = -vmin;
					}
					ch = *format++;
					state = DP_S_DOT;
				}
				else
					state = DP_S_DOT;
				break;

			case DP_S_DOT:
				if (ch == '.')
				{
					state = DP_S_MAX;
					ch = *format++;
				}
				else
					state = DP_S_MOD;
				break;

			case DP_S_MAX:
				if (isdigit(ch))
				{
					if (vmax < 0)
						vmax = 0;
					vmax = 10 * vmax + char_to_int(ch);
					ch = *format++;
				}
				else if (ch == '*')
				{
					vmax = va_arg(args, int);
					if (vmax < 0)
						vmax = -1;
					ch = *format++;
					state = DP_S_MOD;
				}
				else
				{
					if (vmax < 0)
						vmax = 0;
					state = DP_S_MOD;
				}
				break;

			case DP_S_MOD:
				switch (ch)
				{
					case 'h':
						ch = *format++;
						if (ch == 'h')      // C99 %hh? 문자코드 출력
						{
							cflags = DP_C_CHAR;
							ch = *format++;
						}
						else
							cflags = DP_C_SHORT;
						break;
					case 'l':
						ch = *format++;
						if (ch == 'l')      // long long
						{
							cflags = DP_C_LLONG;
							ch = *format++;
						}
						else
							cflags = DP_C_LONG;
						break;
					case 'L':               // long double 또는 size_t
						cflags = DP_C_LDOUBLE;
						ch = *format++;
						break;
					case 'z':
						cflags = DP_C_SIZE;
						ch = *format++;
						break;
					case 'j':
						cflags = DP_C_INTMAX;
						ch = *format++;
						break;
					case 't':
						cflags = DP_C_PTRDIFF;
						ch = *format++;
						break;
					default:
						break;
				}   // switch - ch
				state = DP_S_CONV;
				break;

			case DP_S_CONV:
				switch (ch)
				{
					case 'd':
					case 'i':       // 부호있는 십진수
						switch (cflags)
						{
							case DP_C_SHORT:
								value.i = va_arg(args, int);
								_str_fmt_int(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_LONG:
								value.g = va_arg(args, long);
								_str_fmt_size(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_LLONG:
								value.ll = va_arg(args, long long);
								_str_fmt_long(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								value.sz = va_arg(args, size_t);
								_str_fmt_size(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_INTMAX:   // 최대 정수, 2023년 현재 64비트
								value.im = va_arg(args, intmax_t);
								_str_fmt_long(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_PTRDIFF:
								value.ff = va_arg(args, ptrdiff_t);
								_str_fmt_size(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_CHAR:
								value.i = va_arg(args, int);
								_str_fmt_int(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							default:
								value.i = va_arg(args, int);
								_str_fmt_int(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
						}
						break;
					case 'X':       // 대문자 부호없는 16진수
						flags |= DP_F_UP; QN_FALL_THROUGH;
					case 'x':       // 소문자 부호없는 16진수
					case 'o':       // 부호없는 8진수
					case 'b':       // 부호없는 2진수
					case 'u':       // 부호없는 십진수
						base = ch == 'b' ? 2 : ch == 'o' ? 8 : ch == 'u' ? 10 : 16;
						flags |= DP_F_UNSIGNED;
						switch (cflags)
						{
							case DP_C_SHORT:
								value.ui = (unsigned short)va_arg(args, unsigned int);
								_str_fmt_int(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_LONG:
								value.ug = va_arg(args, unsigned long);
								_str_fmt_size(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_LLONG:
								value.ull = va_arg(args, unsigned long long);
								_str_fmt_long(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								value.sz = va_arg(args, size_t);
								_str_fmt_size(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_INTMAX:   // 64비트 취급
								value.im = va_arg(args, intmax_t);
								_str_fmt_long(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_PTRDIFF:
								value.ff = va_arg(args, ptrdiff_t);
								_str_fmt_size(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_CHAR:
								value.ui = (unsigned char)va_arg(args, int);
								_str_fmt_int(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							default:
								value.ui = va_arg(args, unsigned int);
								_str_fmt_int(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
						}
						break;
					case 'A':       // 실수의 대문자 16진수 표시 (미구현 '%F'로)
					case 'F':       // 실수 대문자
						flags |= DP_F_UP; QN_FALL_THROUGH;
					case 'a':       // 실수의 소문자 16진수 표시 (미구현 '%f'로)
					case 'f':       // 실수
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							value.d = (double)va_arg(args, long double);
						else
							value.d = va_arg(args, double);
						_str_fmt_fp(buffer, &currlen, maxlen, &value, vmin, vmax, flags);
						break;
					case 'E':       // 지수 표시 (대문자)
						flags |= DP_F_UP; QN_FALL_THROUGH;
					case 'e':       // 지수 표시
						flags |= DP_F_E;
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							value.d = (double)va_arg(args, long double);
						else
							value.d = va_arg(args, double);
						_str_fmt_fp(buffer, &currlen, maxlen, &value, vmin, vmax, flags);
						break;
					case 'G':       // 지수 표시 (대문자)
						flags |= DP_F_UP; QN_FALL_THROUGH;
					case 'g':       // 지수 표시
						flags |= DP_F_G;
						if (vmax == 0) // C99
							vmax = 1;
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							value.d = (double)va_arg(args, long double);
						else
							value.d = va_arg(args, double);
						_str_fmt_fp(buffer, &currlen, maxlen, &value, vmin, vmax, flags);
						break;
					case 'c':
						if (cflags == DP_C_LONG)
							flags |= DP_F_UNICODE;
						_str_fmt_char(buffer, &currlen, maxlen, va_arg(args, int), flags);
						break;
					case 'C':
						_str_fmt_char(buffer, &currlen, maxlen, va_arg(args, int), flags | DP_F_UNICODE);
						break;
					case 's':
						if (cflags == DP_C_LONG)
							flags |= DP_F_UNICODE;
						_str_fmt_str(buffer, &currlen, maxlen, va_arg(args, char*), flags, vmin, vmax);
						break;
					case 'S':
						_str_fmt_str(buffer, &currlen, maxlen, va_arg(args, char*), flags | DP_F_UNICODE, vmin, vmax);
						break;
					case 'p':
						flags |= DP_F_UNSIGNED | DP_F_UP;
						value.s = va_arg(args, char*);
						_str_fmt_size(buffer, &currlen, maxlen, &value, 16, vmin, vmax, flags);
						break;
					case 'n':
						switch (cflags)
						{
							case DP_C_SHORT:
								value.ph = va_arg(args, short*);
								*value.ph = (short)currlen;
								break;
							case DP_C_LONG:
								value.pg = va_arg(args, long*);
								*value.pg = (long)currlen;
								break;
							case DP_C_LLONG:
								value.pll = (llong*)va_arg(args, long long*);
								*value.pll = currlen;
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:     // C99에서 원래 부호 있는 크기(ssize_t)를 요구함
								value.psz = va_arg(args, size_t*);
								*value.psz = (size_t)currlen;
								break;
							case DP_C_INTMAX:
								value.pim = va_arg(args, intmax_t*);
								*value.pim = (intmax_t)currlen;
								break;
							case DP_C_PTRDIFF:
								value.pff = va_arg(args, ptrdiff_t*);
								*value.pff = (ptrdiff_t)currlen;
								break;
							case DP_C_CHAR:
								value.s = va_arg(args, char*);
								*value.s = (char)currlen;
								break;
							default:
								value.pi = va_arg(args, int*);
								*value.pi = (int)currlen;
								break;
						}
						break;
					case '%':
						_str_outch(buffer, &currlen, maxlen, ch);
						break;
					default:    // 알수없음, 넘김
						break;
				}   // switch - ch

				ch = *format++;
				state = DP_S_DEFAULT;
				flags = cflags = vmin = base = 0;
				vmax = -1;
				break;

			case DP_S_DONE:
				break;
		}   // switch - state
	}   // while

	if (maxlen > 0)
		buffer[currlen < maxlen - 1 ? currlen : maxlen - 1] = '\0';
	return currlen;
}


//////////////////////////////////////////////////////////////////////////
// 유니코드 버전

// 문자를 숫자로
#define wcharo_int(p)	((p) - L'0')

// 진수별 처리
static const wchar* _wcs_number[2] =
{
	L"0123456789abcdef",
	L"0123456789ABCDEF",
};

// 포인터 표시를 어떤거 쓰나
#if _QN_64_
#define _wcs_fmt_size		_wcs_fmt_long
#else
#define _wcs_fmt_size		_wcs_fmt_int
#endif

// 32비트 숫자를 문자열로
static int _wcs_to_int(uint value, wchar* buf, int size, int base, bool caps)
{
	const wchar* bts = _wcs_number[caps ? 1 : 0];
	int pos = 0;

	do
	{
		buf[pos++] = bts[value % base];
		value /= base;
	} while (value && pos < size);

	buf[pos] = L'\0';

	return pos;
}

// 64비트 숫자를 문자열로
static int _wcs_to_long(ullong value, wchar* buf, int size, int base, bool caps)
{
	const wchar* bts = _wcs_number[caps ? 1 : 0];
	int pos = 0;

	do
	{
		buf[pos++] = bts[value % base];
		value /= base;
	} while (value && pos < size);

	buf[pos] = L'\0';

	return pos;
}

// 숫자 분리 (보통 천단위 통화 분리자)
static int _wcs_quote_seps(int value, int flags
#if USE_WIDE_LOCALE
	, struct lconv* lc
#endif
)
{
	int ret;

	if (!(flags & DP_F_QUOTE))
		ret = 0;
	else
	{
		ret = (value - ((value % 3) == 0 ? 1 : 0)) / 3;
#if USE_WIDE_LOCALE
		if (lc->_W_thousands_sep)
			ret *= (int)wcslen(lc->_W_thousands_sep);
#endif
	}

	return ret;
}

// 글자 넣기
static void _wcs_outch(wchar* buffer, size_t* currlen, size_t maxlen, wchar ch)
{
	if (*currlen < maxlen)
		buffer[(*currlen)] = ch;
	(*currlen)++;
}

// 문자열 널기
static void _wcs_outs(wchar* buffer, size_t* currlen, size_t maxlen, const wchar* value, int len, int flags, int vmin, int vmax)
{
	int padlen = vmin - len;

	if (padlen < 0)
		padlen = 0;
	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	int cnt = 0;
	while (padlen > 0 && cnt < vmax)
	{
		_wcs_outch(buffer, currlen, maxlen, L' ');
		--padlen;
		++cnt;
	}
	while (*value && cnt < vmax)
	{
		_wcs_outch(buffer, currlen, maxlen, *value++);
		++cnt;
	}
	while (padlen < 0 && cnt < vmax)
	{
		_wcs_outch(buffer, currlen, maxlen, L' ');
		++padlen;
		++cnt;
	}
}

// 글자 변환
static void _wcs_fmt_char(wchar* buffer, size_t* currlen, size_t maxlen, int ch, int flags)
{
	if ((flags & DP_F_UNICODE) == 0)
		_wcs_outch(buffer, currlen, maxlen, (wchar)ch);
	else
	{
		char mbs[2] = { (char)ch, '\0' };
		wchar wcs[6];
		int len = (int)qn_mbstowcs(wcs, 6, mbs);
		if (len > 0)
			_wcs_outs(buffer, currlen, maxlen, wcs, len, 0, 0, len);
	}
}

// 문자열 변환
static void _wcs_fmt_str(wchar* buffer, size_t* currlen, size_t maxlen, const wchar* value, int flags, int vmin, int vmax)
{
	if (!value)
	{
		// 널
		value = L"(null)";
		if (vmax < 0)
			vmax = 6;
		if (vmin > 0 && vmin > vmax)
			vmax = vmin;
		_wcs_outs(buffer, currlen, maxlen, value, 6, flags, vmin, vmax);
	}
	else if ((flags & DP_F_UNICODE) != 0)
	{
		// 와이드 문자
		int len = (int)wcslen(value);
		if (vmax < 0)
			vmax = len;
		if (vmin > 0 && vmin > vmax)
			vmax = vmin;
		_wcs_outs(buffer, currlen, maxlen, value, len, flags, vmin, vmax);
	}
	else
	{
		// 멀티바이트 문자
		const char* s = (const char*)value;
		int len = (int)qn_mbstowcs(NULL, 0, s);
		if (vmax < 0)
		{
			value = L"(multibyte)";
			if (vmax < 0)
				vmax = 11;
			if (vmin > 0 && vmin > vmax)
				vmax = vmin;
			_wcs_outs(buffer, currlen, maxlen, value, 11, flags, vmin, vmax);
		}
		else
		{
			wchar* w = qn_alloca(len + 1, wchar);
			qn_mbstowcs(w, (size_t)len + 1, s);
			if (vmax < 0)
				vmax = len;
			if (vmin > 0 && vmin > vmax)
				vmax = vmin;
			_wcs_outs(buffer, currlen, maxlen, w, len, flags, vmin, vmax);
			qn_freea(w);
		}
	}
}

// 32비트 정수
static void _wcs_fmt_int(wchar* buffer, size_t* currlen, size_t maxlen, sn_any* value, uint base, int vmin, int vmax, int flags)
{
#define MAX_CONVERT_PLACES 40
	uint uvalue;
	wchar convert[MAX_CONVERT_PLACES];
	wchar hexprefix;
	wchar signvalue;
	int place;
	int spadlen;
	int zpadlen;
	int quote;
#if USE_WIDE_LOCALE
	struct lconv* lc = localeconv();
#endif

	//
	if (vmax < 0)
		vmax = 0;
	if (flags & DP_F_UNSIGNED)
	{
		uvalue = value->ui;
		signvalue = L'\0';
	}
	else
	{
		if (value->i < 0)
		{
			uvalue = -value->i;
			signvalue = L'-';
		}
		else
		{
			uvalue = value->i;

			if (flags & DP_F_PLUS)  // 부호추가 (+/i)
				signvalue = L'+';
			else if (flags & DP_F_SPACE)
				signvalue = L' ';
			else
				signvalue = L'\0';
		}
	}

	place = _wcs_to_int(uvalue, convert, MAX_CONVERT_PLACES - 1, base, flags & DP_F_UP);

	// '#'이 있을때 접두사 처리
	if ((flags & DP_F_NUM) && uvalue != 0)
	{
		if (base == 16)
			hexprefix = flags & DP_F_UP ? L'X' : L'x';
		else
		{
			hexprefix = L'\0';

			if (base == 8 && vmax <= place)
				vmax = place + 1;
		}
	}
	else
		hexprefix = L'\0';

	//
#if USE_WIDE_LOCALE
	quote = _wcs_quote_seps(place, flags, lc);
#else
	quote = _wcs_quote_seps(place, flags);
#endif
	zpadlen = vmax - place - quote;
	spadlen = vmin - quote - QN_MAX(vmax, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

	if (zpadlen < 0)
		zpadlen = 0;
	if (spadlen < 0)
		spadlen = 0;
	if (flags & DP_F_ZERO)
	{
		zpadlen = QN_MAX(zpadlen, spadlen);
		spadlen = 0;
	}
	if (flags & DP_F_MINUS)
		spadlen = -spadlen; // 왼쪽 정렬

	// 선두 공백
	while (spadlen > 0)
	{
		_wcs_outch(buffer, currlen, maxlen, L' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		_wcs_outch(buffer, currlen, maxlen, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		_wcs_outch(buffer, currlen, maxlen, L'0');
		_wcs_outch(buffer, currlen, maxlen, hexprefix);
	}

	// '0' 삽입
	while (zpadlen > 0)
	{
		_wcs_outch(buffer, currlen, maxlen, L'0');
		--zpadlen;
	}

	// 숫자
	while (place > 0)
	{
		--place;
		_wcs_outch(buffer, currlen, maxlen, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_WIDE_LOCALE
			if (!lc->_W_thousands_sep)
				_wcs_outch(buffer, currlen, maxlen, L',');
			else
				for (const wchar* pwz = lc->_W_thousands_sep; *pwz; pwz++)
					_wcs_outch(buffer, currlen, maxlen, *pwz);
#else
			_wcs_outch(buffer, currlen, maxlen, L',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		_wcs_outch(buffer, currlen, maxlen, L' ');
		++spadlen;
	}
#undef MAX_CONVERT_PLACES
	}

// 64비트 정수
static void _wcs_fmt_long(wchar* buffer, size_t* currlen, size_t maxlen, sn_any* value, uint base, int vmin, int vmax, int flags)
{
#define MAX_CONVERT_PLACES 80
	ullong uvalue;
	wchar convert[MAX_CONVERT_PLACES];
	wchar hexprefix;
	wchar signvalue;
	int place;
	int spadlen;
	int zpadlen;
	int quote;
#if USE_WIDE_LOCALE
	struct lconv* lc = localeconv();
#endif

	//
	if (vmax < 0)
		vmax = 0;
	if (flags & DP_F_UNSIGNED)
	{
		uvalue = value->ull;
		signvalue = L'\0';
	}
	else
	{
		if (value->ll < 0)
		{
			uvalue = -value->ll;
			signvalue = L'-';
		}
		else
		{
			uvalue = value->ll;

			if (flags & DP_F_PLUS)  // 부호추가 (+/i)
				signvalue = L'+';
			else if (flags & DP_F_SPACE)
				signvalue = L' ';
			else
				signvalue = L'\0';
		}
	}

	place = _wcs_to_long(uvalue, convert, MAX_CONVERT_PLACES - 1, base, flags & DP_F_UP);

	// '#'이 있을때 접두사 처리
	if ((flags & DP_F_NUM) && uvalue != 0)
	{
		if (base == 16)
			hexprefix = flags & DP_F_UP ? L'X' : L'x';
		else
		{
			hexprefix = L'\0';

			if (base == 8 && vmax <= place)
				vmax = place + 1;
		}
	}
	else
		hexprefix = L'\0';

	//
#if USE_WIDE_LOCALE
	quote = _wcs_quote_seps(place, flags, lc);
#else
	quote = _wcs_quote_seps(place, flags);
#endif
	zpadlen = vmax - place - quote;
	spadlen = vmin - quote - QN_MAX(vmax, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

	if (zpadlen < 0)
		zpadlen = 0;
	if (spadlen < 0)
		spadlen = 0;
	if (flags & DP_F_ZERO)
	{
		zpadlen = QN_MAX(zpadlen, spadlen);
		spadlen = 0;
	}
	if (flags & DP_F_MINUS)
		spadlen = -spadlen; // 왼쪽 정렬

	// 앞쪽 공백
	while (spadlen > 0)
	{
		_wcs_outch(buffer, currlen, maxlen, L' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		_wcs_outch(buffer, currlen, maxlen, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		_wcs_outch(buffer, currlen, maxlen, L'0');
		_wcs_outch(buffer, currlen, maxlen, hexprefix);
	}

	// '0' 삽입
	if (zpadlen > 0)
	{
		while (zpadlen > 0)
		{
			_wcs_outch(buffer, currlen, maxlen, L'0');
			--zpadlen;
		}
	}

	// 숫자
	while (place > 0)
	{
		--place;
		_wcs_outch(buffer, currlen, maxlen, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_WIDE_LOCALE
			if (!lc->_W_thousands_sep)
				_wcs_outch(buffer, currlen, maxlen, L',');
			else
				for (const wchar* pwz = lc->_W_thousands_sep; *pwz; pwz++)
					_wcs_outch(buffer, currlen, maxlen, *pwz);
#else
			_wcs_outch(buffer, currlen, maxlen, L',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		_wcs_outch(buffer, currlen, maxlen, L' ');
		++spadlen;
	}
#undef MAX_CONVERT_PLACES
	}

// 64비트 실수(double)
// 여기서는, 오직 16자리 정수만 지원. 원래는 9, 19, 38 자리수에 따라 각각 32, 64, 128비트용이 있어야함
static void _wcs_fmt_fp(wchar* buffer, size_t* currlen, size_t maxlen, sn_any* value, int vmin, int vmax, int flags)
{
	double fvalue;
	double uvalue;
	ullong intpart;
	ullong fracpart;
	ullong mask;
	wchar iconvert[311];
	wchar fconvert[311];
	wchar econvert[10];
	wchar signvalue;
	wchar esignvalue;
	int iplace;
	int fplace;
	int eplace;
	int padlen;
	int zleadfrac;
	int dotpoint;
	int quote;
	int etype;
	int exponent;
	int omitzeros;
	int omitcount;
	const wchar* pwz;
#if USE_WIDE_LOCALE
	struct lconv* lc = localeconv();
#endif

	/*
	* AIX manpage says the default is 0, but Solaris says the default
	* is 6, and sprintf on AIX defaults to 6
	*/
	if (vmax < 0)
		vmax = 6;

	fvalue = value->d;
	if (fvalue < 0)
		signvalue = L'-';
	else if (flags & DP_F_PLUS) // 부호추가 (+/i)
		signvalue = L'+';
	else if (flags & DP_F_SPACE)
		signvalue = L' ';
	else
		signvalue = L'\0';

	// NAN, INF 처리
	if (pp_is_nan(fvalue))
		pwz = flags & DP_F_UP ? L"NAN" : L"nan";
	else if (pp_is_inf(fvalue))
		pwz = flags & DP_F_UP ? L"INF" : L"inf";
	else
		pwz = NULL;

	if (pwz)
	{
		iplace = 0;
		if (signvalue)
			iconvert[iplace++] = signvalue;
		while (*pwz)
			iconvert[iplace++] = *pwz++;
		iconvert[iplace] = L'\0';

		_wcs_outs(buffer, currlen, maxlen, iconvert, iplace, flags, vmin, iplace);
		return;
	}

	// 지수 모양 처리
	if (flags & (DP_F_E | DP_F_G))
	{
		if (flags & DP_F_G)
		{
			--vmax;
			omitzeros = !(flags & DP_F_NUM);
		}
		else
			omitzeros = 0;

		exponent = pp_exponent(fvalue);
		etype = 1;
	}
	else
	{
		exponent = 0;
		omitzeros = 0;
		etype = 0;
	}

pos_exp_again: // 이 루프는 확실한 반올림 지수를 얻기 위함 -> '%g'
	if (vmax > 16)
		vmax = 16;

	// 정수로 변환
	uvalue = QN_ABS(fvalue);
	if (etype)
		uvalue /= pp_pow10(exponent);
	intpart = pp_intpart(uvalue);
	mask = (ullong)pp_pow10(vmax);
	fracpart = (ullong)pp_round(mask * (uvalue - intpart));

	if (fracpart >= mask)
	{
		// 예를 들면 uvalue=1.99952, intpart=2, temp=1000 (max=3 이므로)
		// pp_round(1000*0.99952)=1000. 그리하여, 정수부는 1증가, 실수부는 0으로 함.
		intpart++;
		fracpart = 0;

		if (etype && intpart == 10)
		{
			intpart = 1;
			exponent++;
		}
	}

	// 정확한 지수를 얻음. '%g'를 위한 지수를 재계산
	if (etype && (flags & DP_F_G) && (vmax + 1) > exponent && exponent >= -4)
	{
		vmax -= exponent;
		etype = 0;
		goto pos_exp_again;
	}

	if (etype)
	{
		if (exponent < 0)
		{
			exponent = -exponent;
			esignvalue = L'-';
		}
		else
			esignvalue = L'+';

		eplace = _wcs_to_int(exponent, econvert, 2, 10, false);

		if (eplace == 1)
		{
			// MSVC는 자리수를 3자리 씀. (001+E)
			// 여기서는 2자리만. (01+E)
			econvert[eplace++] = L'0';
		}
		econvert[eplace++] = esignvalue;
		econvert[eplace++] = flags & DP_F_UP ? L'E' : L'e';
		econvert[eplace] = L'\0';
	}
	else
	{
		esignvalue = L'\0';
		eplace = 0;
	}

	// 정수부 처리
	iplace = _wcs_to_long(intpart, iconvert, 311 - 1, 10, false);

	// 실수부 처리
	// iplace=1 이면 fracpart==0 임.
	fplace = fracpart == 0 ? 0 : _wcs_to_long(fracpart, fconvert, 311 - 1, 10, false);

	// 지수부 처리
	zleadfrac = vmax - fplace;

	omitcount = 0;

	if (omitzeros)
	{
		if (fplace > 0)
		{
			while (omitcount < fplace && fconvert[omitcount] == L'0')
				omitcount++;
		}
		else
		{
			// 실수부가 0, 걍 무시함
			omitcount = vmax;
			zleadfrac = 0;
		}
		vmax -= omitcount;
	}

	// 실수부가 0이 아니고, '#' 플래그가 있으면 정수 포인트 출력
	dotpoint = vmax > 0 || (flags & DP_F_NUM);

	//
#if USE_WIDE_LOCALE
	quote = _wcs_quote_seps(iplace, flags, lc);
#else
	quote = _wcs_quote_seps(iplace, flags);
#endif

	// -1은 정수 포인트용, 부호를 출력하면 추가로 -1;
	padlen = vmin - iplace - eplace - vmax - quote - (dotpoint ? 1 : 0) - (signvalue ? 1 : 0);
	if (padlen < 0)
		padlen = 0;
	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	// 부호 및 패딩 숫자 출력
	if ((flags & DP_F_ZERO) && padlen > 0)
	{
		if (signvalue)
		{
			_wcs_outch(buffer, currlen, maxlen, signvalue);
			signvalue = L'\0';
		}
		while (padlen > 0)
		{
			_wcs_outch(buffer, currlen, maxlen, L'0');
			--padlen;
		}
	}

	// 남은 패딩 공백 출력
	while (padlen > 0)
	{
		_wcs_outch(buffer, currlen, maxlen, L' ');
		--padlen;
	}

	// 패딩에서 부호 출력 안했으면 부호 출력
	if (signvalue)
		_wcs_outch(buffer, currlen, maxlen, signvalue);

	// 정수부 출력
	while (iplace > 0)
	{
		--iplace;
		_wcs_outch(buffer, currlen, maxlen, iconvert[iplace]);

		if (quote && iplace > 0 && (iplace % 3) == 0)
		{
#if USE_WIDE_LOCALE
			if (!lc->_W_thousands_sep)
				_wcs_outch(buffer, currlen, maxlen, L',');
			else
				for (pwz = lc->_W_thousands_sep; *pwz; pwz++)
					_wcs_outch(buffer, currlen, maxlen, *pwz);
#else
			_wcs_outch(buffer, currlen, maxlen, L',');
#endif
		}
	}

	// 소수점
	if (dotpoint)
	{
#if USE_WIDE_LOCALE
		if (!lc->_W_decimal_point)
			_wcs_outch(buffer, currlen, maxlen, L'.');
		else
			for (pwz = lc->_W_decimal_point; *pwz; pwz++)
				_wcs_outch(buffer, currlen, maxlen, *pwz);
#else
		_wcs_outch(buffer, currlen, maxlen, L'.');
#endif
	}

	// 실수부 앞쪽 패딩
	while (zleadfrac > 0)
	{
		_wcs_outch(buffer, currlen, maxlen, '0');
		--zleadfrac;
	}

	// 실수부
	while (fplace > omitcount)
		_wcs_outch(buffer, currlen, maxlen, fconvert[--fplace]);

	// 지수값
	while (eplace > 0)
		_wcs_outch(buffer, currlen, maxlen, econvert[--eplace]);

	// 남은 패팅
	while (padlen < 0)
	{
		_wcs_outch(buffer, currlen, maxlen, ' ');
		++padlen;
	}
	}

//
size_t doprw(wchar* buffer, size_t maxlen, const wchar* format, va_list args)
{
	int vmin = 0;
	int vmax = -1;
	int state = DP_S_DEFAULT;
	int flags = 0;
	int cflags = 0;
	int base = 0;
	size_t currlen = 0;
	sn_any value = { .ff = 0 };

	if (!buffer)
		maxlen = 0;

	wchar ch = *format++;
	while (state != DP_S_DONE)
	{
		if (ch == L'\0')
			state = DP_S_DONE;

		switch (state)
		{
			case DP_S_DEFAULT:
				if (ch == L'%')
					state = DP_S_FLAGS;
				else
					_wcs_outch(buffer, &currlen, maxlen, ch);
				ch = *format++;
				break;

			case DP_S_FLAGS:
				switch (ch)
				{
					case L'-':
						flags |= DP_F_MINUS;
						ch = *format++;
						break;
					case L'+':
						flags |= DP_F_PLUS;
						ch = *format++;
						break;
					case L' ':
						flags |= DP_F_SPACE;
						ch = *format++;
						break;
					case L'#':
						flags |= DP_F_NUM;
						ch = *format++;
						break;
					case L'0':
						flags |= DP_F_ZERO;
						ch = *format++;
						break;
					case L'\'':
						flags |= DP_F_QUOTE;
						ch = *format++;
						break;
					default:
						state = DP_S_MIN;
						break;
				}   // switch - ch
				break;

			case DP_S_MIN:
				if (iswdigit(ch))
				{
					vmin = 10 * vmin + wcharo_int(ch);
					ch = *format++;
				}
				else if (ch == L'*')
				{
					vmin = va_arg(args, int);
					if (vmin < 0)
					{
						flags |= DP_F_MINUS;
						vmin = -vmin;
					}
					ch = *format++;
					state = DP_S_DOT;
				}
				else
					state = DP_S_DOT;
				break;

			case DP_S_DOT:
				if (ch == L'.')
				{
					state = DP_S_MAX;
					ch = *format++;
				}
				else
					state = DP_S_MOD;
				break;

			case DP_S_MAX:
				if (iswdigit(ch))
				{
					if (vmax < 0)
						vmax = 0;
					vmax = 10 * vmax + wcharo_int(ch);
					ch = *format++;
				}
				else if (ch == L'*')
				{
					vmax = va_arg(args, int);
					if (vmax < 0)
						vmax = -1;
					ch = *format++;
					state = DP_S_MOD;
				}
				else
				{
					if (vmax < 0)
						vmax = 0;
					state = DP_S_MOD;
				}
				break;

			case DP_S_MOD:
				switch (ch)
				{
					case L'h':
						ch = *format++;
						if (ch == L'h')
						{
							cflags = DP_C_CHAR;
							ch = *format++;
						}
						else
							cflags = DP_C_SHORT;
						break;
					case L'l':
						ch = *format++;
						if (ch == L'l')     // long long
						{
							cflags = DP_C_LLONG;
							ch = *format++;
						}
						else
							cflags = DP_C_LONG;
						break;
					case L'L':				// long double 또는 size_t
						cflags = DP_C_LDOUBLE;
						ch = *format++;
						break;
					case L'z':
						cflags = DP_C_SIZE;
						ch = *format++;
						break;
					case L'j':
						cflags = DP_C_INTMAX;
						ch = *format++;
						break;
					case L't':
						cflags = DP_C_PTRDIFF;
						ch = *format++;
						break;
					default:
						break;
				}   // switch - ch
				state = DP_S_CONV;
				break;

			case DP_S_CONV:
				switch (ch)
				{
					case L'd':
					case L'i':		// 부호있는 십진수
						switch (cflags)
						{
							case DP_C_SHORT:
								value.i = va_arg(args, int);
								_wcs_fmt_int(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_LONG:
								value.g = va_arg(args, long);
								_wcs_fmt_size(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_LLONG:
								value.ll = va_arg(args, long long);
								_wcs_fmt_long(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								value.sz = va_arg(args, size_t);
								_wcs_fmt_size(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_INTMAX:   // 최대 정수, 2023년 현재 64비트
								value.im = va_arg(args, intmax_t);
								_wcs_fmt_long(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_PTRDIFF:
								value.ff = va_arg(args, ptrdiff_t);
								_wcs_fmt_size(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							case DP_C_CHAR:
								value.i = va_arg(args, int);
								_wcs_fmt_int(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
							default:
								value.i = va_arg(args, int);
								_wcs_fmt_int(buffer, &currlen, maxlen, &value, 10, vmin, vmax, flags);
								break;
						}
						break;
					case L'X':       // 대문자 부호없는 16진수
						flags |= DP_F_UP; QN_FALL_THROUGH;
					case L'x':       // 소문자 부호없는 16진수
					case L'o':       // 부호없는 8진수
					case L'b':       // 부호없는 2진수
					case L'u':       // 부호없는 십진수
						base = ch == L'b' ? 2 : ch == L'o' ? 8 : ch == L'u' ? 10 : 16;
						flags |= DP_F_UNSIGNED;
						switch (cflags)
						{
							case DP_C_SHORT:
								value.ui = (unsigned short)va_arg(args, unsigned int);
								_wcs_fmt_int(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_LONG:
								value.ui = va_arg(args, unsigned long);
								_wcs_fmt_size(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_LLONG:
								value.ull = va_arg(args, unsigned long long);
								_wcs_fmt_long(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								value.sz = va_arg(args, size_t);
								_wcs_fmt_size(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_INTMAX:   // 64비트
								value.im = va_arg(args, intmax_t);
								_wcs_fmt_long(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_PTRDIFF:
								value.ff = va_arg(args, ptrdiff_t);
								_wcs_fmt_size(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							case DP_C_CHAR:
								value.ui = (unsigned char)va_arg(args, int);
								_wcs_fmt_int(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
							default:
								value.ui = va_arg(args, unsigned int);
								_wcs_fmt_int(buffer, &currlen, maxlen, &value, base, vmin, vmax, flags);
								break;
						}
						break;
					case L'A':       // 실수의 대문자 16진수 표시 (미구현 '%F'로)
					case L'F':       // 실수 대문자
						flags |= DP_F_UP; QN_FALL_THROUGH;
					case L'a':       // 실수의 소문자 16진수 표시 (미구현 '%f'로)
					case L'f':       // 실수
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							value.d = (double)va_arg(args, long double);
						else
							value.d = va_arg(args, double);
						_wcs_fmt_fp(buffer, &currlen, maxlen, &value, vmin, vmax, flags);
						break;
					case L'E':      // 지수 표시 (대문자)
						flags |= DP_F_UP; QN_FALL_THROUGH;
					case L'e':      // 지수 표시
						flags |= DP_F_E;
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							value.d = (double)va_arg(args, long double);
						else
							value.d = va_arg(args, double);
						_wcs_fmt_fp(buffer, &currlen, maxlen, &value, vmin, vmax, flags);
						break;
					case L'G':      // 지수 표시 (대문자)
						flags |= DP_F_UP; QN_FALL_THROUGH;
					case L'g':      // 지수 표시
						flags |= DP_F_G;
						if (vmax == 0) // C99
							vmax = 1;
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							value.d = (double)va_arg(args, long double);
						else
							value.d = va_arg(args, double);
						_wcs_fmt_fp(buffer, &currlen, maxlen, &value, vmin, vmax, flags);
						break;
					case L'c':
						if (cflags != DP_C_SHORT)
							flags |= DP_F_UNICODE;
						_wcs_fmt_char(buffer, &currlen, maxlen, va_arg(args, int), flags);
						break;
					case L'C':
						_wcs_fmt_char(buffer, &currlen, maxlen, va_arg(args, int), flags | DP_F_UNICODE);
						break;
					case L's':
						if (cflags != DP_C_SHORT)
							flags |= DP_F_UNICODE;
						_wcs_fmt_str(buffer, &currlen, maxlen, va_arg(args, wchar*), flags, vmin, vmax);
						break;
					case L'S':
						_wcs_fmt_str(buffer, &currlen, maxlen, va_arg(args, wchar*), flags | DP_F_UNICODE, vmin, vmax);
						break;
					case L'p':
						flags |= DP_F_UNSIGNED | DP_F_UP;
						value.s = va_arg(args, char*);
						_wcs_fmt_size(buffer, &currlen, maxlen, &value, 16, vmin, vmax, flags);
						break;
					case L'n':
						switch (cflags)
						{
							case DP_C_SHORT:
								value.ph = va_arg(args, short*);
								*value.ph = (short)currlen;
								break;
							case DP_C_LONG:
								value.pg = va_arg(args, long*);
								*value.pg = (long)currlen;
								break;
							case DP_C_LLONG:
								value.pll = (llong*)va_arg(args, long long*);
								*value.pll = currlen;
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:     // C99에서 원래 부호 있는 크기(ssize_t)를 요구함
								value.psz = va_arg(args, size_t*);
								*value.psz = (size_t)currlen;
								break;
							case DP_C_INTMAX:
								value.pim = va_arg(args, intmax_t*);
								*value.pim = (intmax_t)currlen;
								break;
							case DP_C_PTRDIFF:
								value.pff = va_arg(args, ptrdiff_t*);
								*value.pff = (ptrdiff_t)currlen;
								break;
							case DP_C_CHAR:
								value.s = va_arg(args, char*);
								*value.s = (char)currlen;
								break;
							default:
								value.pi = va_arg(args, int*);
								*value.pi = (int)currlen;
								break;
						}
						break;
					case L'%':
						_wcs_outch(buffer, &currlen, maxlen, ch);
						break;
					default:    // 알수없음, 넘김
						break;
				}   // switch - ch

				ch = *format++;
				state = DP_S_DEFAULT;
				flags = cflags = vmin = base = 0;
				vmax = -1;
				break;

			case DP_S_DONE:
				break;
		}   // switch - state
	}   // while

	if (maxlen > 0)
		buffer[currlen < maxlen - 1 ? currlen : maxlen - 1] = L'\0';
	return currlen;
}
