/*
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
*    add external output
**************************************************************/

#include "pch.h"
#include "qs_qn.h"
#include <ctype.h>
#include <locale.h>
#include "PatrickPowell_snprintf.h"

/*
* dopr(): poor man's version of doprintf
*/

#if !defined _QN_MOBILE_ && !defined _QN_EMSCRIPTEN_	// 모바일과 EMSCRIPTEN에서 로캘 안되는건 아닌데 안씀
#define USE_LOCALE_INFO	1
#define LOCALE_TYPE		struct lconv
#define LOCALE_FUNC		localeconv()
#else
#define USE_LOCALE_INFO	0
#define LOCALE_TYPE		void
#define LOCALE_FUNC		NULL
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

// 숫자 변환 아스키/유니코드 공용
extern const byte* qn_num_base_table(void);
extern const char* qn_char_base_table(const bool upper);
#define char_to_int(p)	(int)((p) - '0')

//
INLINE double pps_pow10(int exp)
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

//
INLINE double pps_round(double value)
{
	const ullong intpart = (ullong)value;
	value = value - (double)intpart;
	return (double)(value >= 0.5 ? intpart + 1 : intpart);
}

//
INLINE bool pps_isnan(const double value)
{
	return value != value;
}

//
INLINE bool pps_isinf(const double value)
{
	return value != 0.0 && value + value == value;
}

//
INLINE int pps_exponent(const double value)
{
	double d = value < 0.0 ? -value : value;
	int exp = 0;
	while (d < 1.0 && d > 0.0 && --exp > -99) d *= 10;
	while (d >= 10.0 && ++exp < 99) d /= 10;
	return exp;
}

//
INLINE ullong pps_intpart(const double value)
{
	if (value >= (double)UINT64_MAX)
		return UINT64_MAX;
	const ullong res = (ullong)value;
	return (const double)res <= value ? res : res - 1;
}


//////////////////////////////////////////////////////////////////////////
// 아스키 버전

// 포인터 표시를 어떤거 쓰나
#ifdef _QN_64_
#define pps_size_t		pps_int64
#else
#define pps_size_t		pps_int32
#endif

// 32비트 숫자를 문자열로
static size_t pps_atoi(uint value, char* RESTRICT buf, size_t size, nuint base, bool caps)
{
	const char* table = qn_char_base_table(caps);
	size_t pos = 0;
	do
	{
		buf[pos++] = table[value % (uint)base];
		value /= (uint)base;
	} while (value && pos < size);
	buf[pos] = '\0';
	return pos;
}

// 64비트 숫자를 문자열로
static size_t pps_atoll(ullong value, char* RESTRICT buf, size_t size, nuint base, bool caps)
{
	const char* table = qn_char_base_table(caps);
	size_t pos = 0;
	do
	{
		buf[pos++] = table[value % base];
		value /= base;
	} while (value && pos < size);
	buf[pos] = '\0';
	return pos;
}

// 숫자 분리 (보통 천단위 통화 분리자, 한자 문화권은 만단위가 좋은데)
static nint pps_quote(nint value, int flags, const LOCALE_TYPE* lc)
{
#if !USE_LOCALE_INFO
	QN_DUMMY(lc);
#endif
	if (!(flags & DP_F_QUOTE))
		return 0;
	nint ret = (value - ((value % 3) == 0 ? 1 : 0)) / 3;
#if USE_LOCALE_INFO
	if (lc->thousands_sep)
		ret *= (nint)strlen(lc->thousands_sep);
#endif
	return ret;
}

// 문자열 넣기
static void pps_puts(PatrickPowellSprintfState* state, const char* RESTRICT value, nint len, int flags)
{
	nint padlen = state->vmin - len;
	if (padlen < 0)
		padlen = 0;
	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	nint cnt = 0;
	while (padlen > 0 && cnt < state->vmax)
	{
		state->outch(state, ' ');
		--padlen;
		++cnt;
	}
	while (*value && cnt < state->vmax)
	{
		state->outch(state, *value++);
		++cnt;
	}
	while (padlen < 0 && cnt < state->vmax)
	{
		state->outch(state, ' ');
		++padlen;
		++cnt;
	}
}

// 문자 넣기
static void pps_putch(PatrickPowellSprintfState* state, int ch, int flags)
{
	if ((flags & DP_F_UNICODE) == 0)
		state->outch(state, ch);
	else
	{
		char mbs[6];
		const wchar_t wcs[2] = { (wchar)ch, L'\0' };
		state->vmax = (nint)qn_wcstombs(mbs, 6, wcs, 1);
		if (state->vmax > 0)
		{
			state->vmin = 0;
			pps_puts(state, mbs, state->vmax, 0);
		}
	}
}

// 문자열 넣기
static void pps_string(PatrickPowellSprintfState* state, int flags)
{
	if (state->value.s == NULL)
	{
		if (state->vmax < 0)
			state->vmax = 6;
		if (state->vmin > 0 && state->vmin > state->vmax)
			state->vmax = state->vmin;
		pps_puts(state, "(null)", 6, flags);
	}
	else if ((flags & DP_F_UNICODE) == 0)
	{
		// 그냥 문자열
		const nint len = (nint)strlen(state->value.s);
		if (len > 0)
		{
			if (state->vmax < 0)
				state->vmax = len;
			if (state->vmin > 0 && state->vmin > state->vmax)
				state->vmax = state->vmin;
			pps_puts(state, state->value.s, len, flags);
		}
	}
	else
	{
		// 와이드 문자열
		char s[QN_MAX_PATH];
		const nint len = (nint)qn_wcstombs(s, QN_MAX_PATH, state->value.w, 0);
		if (len > 0)
		{
			if (state->vmax < 0)
				state->vmax = len;
			if (state->vmin > 0 && state->vmin > state->vmax)
				state->vmax = state->vmin;
			pps_puts(state, s, len, flags);
		}
	}
}

// 32비트 정수
static void pps_int32(PatrickPowellSprintfState* state, int flags, const LOCALE_TYPE* lc)
{
#define MAX_CONVERT_PLACES 40
	uint uvalue;
	char convert[MAX_CONVERT_PLACES];
	char hexprefix;
	char signvalue;
	nint place;
	nint spadlen;
	nint zpadlen;
	nint quote;

	//
	if (state->vmax < 0)
		state->vmax = 0;
	if (flags & DP_F_UNSIGNED)
	{
		uvalue = state->value.ui;
		signvalue = '\0';
	}
	else
	{
		if (state->value.i < 0)
		{
			uvalue = (uint)-state->value.i;
			signvalue = '-';
		}
		else
		{
			uvalue = (uint)state->value.i;
			if (flags & DP_F_PLUS)  // 부호추가 (+/i)
				signvalue = '+';
			else if (flags & DP_F_SPACE)
				signvalue = ' ';
			else
				signvalue = '\0';
		}
	}

	place = pps_atoi(uvalue, convert, MAX_CONVERT_PLACES - 1, state->base, flags & DP_F_UP);

	// '#'이 있을때 접두사 처리
	if (!((flags & DP_F_NUM) && uvalue != 0))
		hexprefix = '\0';
	else
	{
		if (state->base == 16)
			hexprefix = flags & DP_F_UP ? 'X' : 'x';
		else
		{
			hexprefix = '\0';
			if (state->base == 8 && state->vmax <= place)
				state->vmax = place + 1;
		}
	}

	//
	quote = pps_quote(place, flags, lc);
	zpadlen = state->vmax - place - quote;
	spadlen = state->vmin - quote - QN_MAX(state->vmax, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

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
		state->outch(state, ' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		state->outch(state, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		state->outch(state, '0');
		state->outch(state, hexprefix);
	}

	// '0' 삽입
	while (zpadlen > 0)
	{
		state->outch(state, '0');
		--zpadlen;
	}

	// 숫자
	while (place > 0)
	{
		--place;
		state->outch(state, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				state->outch(state, ',');
			else
				for (const char* psz = lc->thousands_sep; *psz; psz++)
					state->outch(state, *psz);
#else
			state->outch(state, ',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		state->outch(state, ' ');
		++spadlen;
	}
#undef MAX_CONVERT_PLACES
}

// 64비트 정수
static void pps_int64(PatrickPowellSprintfState* state, int flags, const LOCALE_TYPE* lc)
{
#define MAX_CONVERT_PLACES 80
	ullong uvalue;
	char convert[MAX_CONVERT_PLACES];
	char hexprefix;
	char signvalue;
	nint place;
	nint spadlen;
	nint zpadlen;
	nint quote;

	//
	if (state->vmax < 0)
		state->vmax = 0;
	if (flags & DP_F_UNSIGNED)
	{
		uvalue = state->value.ull;
		signvalue = '\0';
	}
	else
	{
		if (state->value.ll < 0)
		{
			uvalue = (ullong)-state->value.ll;
			signvalue = '-';
		}
		else
		{
			uvalue = (ullong)state->value.ll;
			if (flags & DP_F_PLUS)  // 부호추가 (+/i)
				signvalue = '+';
			else if (flags & DP_F_SPACE)
				signvalue = ' ';
			else
				signvalue = '\0';
		}
	}

	place = pps_atoll(uvalue, convert, MAX_CONVERT_PLACES - 1, state->base, flags & DP_F_UP);

	// '#'이 있을때 접두사 처리
	if (!((flags & DP_F_NUM) && uvalue != 0))
		hexprefix = '\0';
	{
		if (state->base == 16)
			hexprefix = flags & DP_F_UP ? 'X' : 'x';
		else
		{
			hexprefix = '\0';
			if (state->base == 8 && state->vmax <= place)
				state->vmax = place + 1;
		}
	}

	//
	quote = pps_quote(place, flags, lc);
	zpadlen = state->vmax - place - quote;
	spadlen = state->vmin - quote - QN_MAX(state->vmax, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

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
		state->outch(state, ' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		state->outch(state, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		state->outch(state, '0');
		state->outch(state, hexprefix);
	}

	// '0' 삽입
	if (zpadlen > 0)
	{
		while (zpadlen > 0)
		{
			state->outch(state, '0');
			--zpadlen;
		}
	}

	// 숫자
	while (place > 0)
	{
		--place;
		state->outch(state, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				state->outch(state, ',');
			else
				for (const char* psz = lc->thousands_sep; *psz; psz++)
					state->outch(state, *psz);
#else
			state->outch(state, ',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		state->outch(state, ' ');
		++spadlen;
	}
#undef MAX_CONVERT_PLACES
}

// 64비트 실수(double)
// 여기서는, 오직 16자리 정수만 지원. 원래는 9, 19, 38 자리수에 따라 각각 32, 64, 128비트용이 있어야함
static void pps_double(PatrickPowellSprintfState* state, int flags, const LOCALE_TYPE* lc)
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
	nint iplace;
	nint fplace;
	nint eplace;
	nint padlen;
	nint zleadfrac;
	nint dotpoint;
	nint quote;
	nint etype;
	int exponent;
	int omitzeros;
	nint omitcount;
	const char* psz;

	/*
	* AIX manpage says the default is 0, but Solaris says the default
	* is 6, and sprintf on AIX defaults to 6
	*/
	if (state->vmax < 0)
		state->vmax = 6;

	fvalue = state->value.d;
	if (fvalue < 0)
		signvalue = '-';
	else if (flags & DP_F_PLUS) // 부호추가 (+/i)
		signvalue = '+';
	else if (flags & DP_F_SPACE)
		signvalue = ' ';
	else
		signvalue = '\0';

	// NAN, INF 처리
	if (pps_isnan(fvalue))
		psz = flags & DP_F_UP ? "NAN" : "nan";
	else if (pps_isinf(fvalue))
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

		state->vmax = iplace;
		pps_puts(state, iconvert, iplace, flags);
		return;
	}

	// 지수 모양 처리
	if (flags & (DP_F_E | DP_F_G))
	{
		if (flags & DP_F_G)
		{
			--state->vmax;
			omitzeros = !(flags & DP_F_NUM);
		}
		else
			omitzeros = 0;
		exponent = pps_exponent(fvalue);
		etype = 1;
	}
	else
	{
		exponent = 0;
		omitzeros = 0;
		etype = 0;
	}

pos_exp_again: // 이 루프는 확실한 반올림 지수를 얻기 위함 -> '%g'
	if (state->vmax > 16)
		state->vmax = 16;

	// 정수로 변환
	uvalue = QN_ABS(fvalue);
	if (etype)
		uvalue /= pps_pow10(exponent);
	intpart = pps_intpart(uvalue);
	mask = (ullong)pps_pow10((int)state->vmax);
	fracpart = (ullong)pps_round((double)mask * (uvalue - (double)intpart));

	if (fracpart >= mask)
	{
		// 예를 들면 uvalue=1.99952, intpart=2, temp=1000 (max=3 이므로)
		// pps_round(1000*0.99952)=1000. 그리하여, 정수부는 1증가, 실수부는 0으로 함.
		intpart++;
		fracpart = 0;

		if (etype && intpart == 10)
		{
			intpart = 1;
			exponent++;
		}
	}

	// 정확한 지수를 얻음. '%g'를 위한 지수를 재계산
	if (etype && (flags & DP_F_G) && (state->vmax + 1) > exponent && exponent >= -4)
	{
		state->vmax -= exponent;
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

		eplace = pps_atoi((uint)exponent, econvert, 2, 10, false);

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
	iplace = pps_atoll(intpart, iconvert, 311 - 1, 10, false);

	// 실수부 처리
	// iplace=1 이면 fracpart==0 임.
	fplace = fracpart == 0 ? 0 : pps_atoll(fracpart, fconvert, 311 - 1, 10, false);

	// 지수부 처리
	zleadfrac = state->vmax - fplace;

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
			omitcount = state->vmax;
			zleadfrac = 0;
		}
		state->vmax -= omitcount;
	}

	// 실수부가 0이 아니고, '#' 플래그가 있으면 정수 포인트 출력
	dotpoint = state->vmax > 0 || (flags & DP_F_NUM);

	//
	quote = pps_quote(iplace, flags, lc);

	// -1은 정수 포인트용, 부호를 출력하면 추가로 -1;
	padlen = state->vmin - iplace - eplace - state->vmax - quote - (dotpoint ? 1 : 0) - (signvalue ? 1 : 0);
	if (padlen < 0)
		padlen = 0;
	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	// 부호 및 패딩 숫자 출력
	if ((flags & DP_F_ZERO) && padlen > 0)
	{
		if (signvalue)
		{
			state->outch(state, signvalue);
			signvalue = '\0';
		}
		while (padlen > 0)
		{
			state->outch(state, '0');
			--padlen;
		}
	}

	// 남은 패딩 공백 출력
	while (padlen > 0)
	{
		state->outch(state, ' ');
		--padlen;
	}

	// 패딩에서 부호 출력 안했으면 부호 출력
	if (signvalue)
		state->outch(state, signvalue);

	// 정수부 출력
	while (iplace > 0)
	{
		--iplace;
		state->outch(state, iconvert[iplace]);
		if (quote && iplace > 0 && (iplace % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				state->outch(state, ',');
			else
				for (psz = lc->thousands_sep; *psz; psz++)
					state->outch(state, *psz);
#else
			state->outch(state, ',');
#endif
		}
	}

	// 소수점
	if (dotpoint)
	{
#if USE_LOCALE_INFO
		if (!lc->decimal_point)
			state->outch(state, '.');
		else
			for (psz = lc->decimal_point; *psz; psz++)
				state->outch(state, *psz);
#else
		state->outch(state, '.');
#endif
	}

	// 실수부 앞쪽 패딩
	while (zleadfrac > 0)
	{
		state->outch(state, '0');
		--zleadfrac;
	}

	// 실수부
	while (fplace > omitcount)
		state->outch(state, fconvert[--fplace]);

	// 지수값
	while (eplace > 0)
		state->outch(state, econvert[--eplace]);

	// 남은 패팅
	while (padlen < 0)
	{
		state->outch(state, ' ');
		++padlen;
	}
}

//
void dopr(_In_ PatrickPowellSprintfState* state, _In_ const char* RESTRICT format, _In_ va_list args)
{
	qn_debug_verify(state != NULL);
	int cmd = DP_S_DEFAULT;
	int flags = 0;
	int cflags = 0;
	const LOCALE_TYPE* lc = LOCALE_FUNC;

	state->vmax = -1;

	char ch = *format++;
	while (cmd != DP_S_DONE)
	{
		if (ch == '\0')
			cmd = DP_S_DONE;

		switch (cmd)
		{
			case DP_S_DEFAULT:
				if (ch == '%')
					cmd = DP_S_FLAGS;
				else
					state->outch(state, ch);
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
						cmd = DP_S_MIN;
						break;
				}   // switch - ch
				break;

			case DP_S_MIN:
				if (isdigit(ch))
				{
					state->vmin = 10 * state->vmin + char_to_int(ch);
					ch = *format++;
				}
				else if (ch == '*')
				{
					state->vmin = va_arg(args, int);
					if (state->vmin < 0)
					{
						flags |= DP_F_MINUS;
						state->vmin = -state->vmin;
					}
					ch = *format++;
					cmd = DP_S_DOT;
				}
				else
					cmd = DP_S_DOT;
				break;

			case DP_S_DOT:
				if (ch == '.')
				{
					cmd = DP_S_MAX;
					ch = *format++;
				}
				else
					cmd = DP_S_MOD;
				break;

			case DP_S_MAX:
				if (isdigit(ch))
				{
					if (state->vmax < 0)
						state->vmax = 0;
					state->vmax = 10 * state->vmax + char_to_int(ch);
					ch = *format++;
				}
				else if (ch == '*')
				{
					state->vmax = va_arg(args, int);
					if (state->vmax < 0)
						state->vmax = -1;
					ch = *format++;
					cmd = DP_S_MOD;
				}
				else
				{
					if (state->vmax < 0)
						state->vmax = 0;
					cmd = DP_S_MOD;
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
				cmd = DP_S_CONV;
				break;

			case DP_S_CONV:
				switch (ch)
				{
					case 'd':
					case 'i':       // 부호있는 십진수
						state->base = 10;
						switch (cflags)
						{
							case DP_C_SHORT:
								state->value.i = va_arg(args, int);
								pps_int32(state, flags, lc);
								break;
							case DP_C_LONG:
								state->value.g = va_arg(args, long);
								pps_size_t(state, flags, lc);
								break;
							case DP_C_LLONG:
								state->value.ll = va_arg(args, long long);
								pps_int64(state, flags, lc);
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								state->value.sz = va_arg(args, size_t);
								pps_size_t(state, flags, lc);
								break;
							case DP_C_INTMAX:   // 최대 정수, 2023년 현재 64비트
								state->value.im = va_arg(args, intmax_t);
								pps_int64(state, flags, lc);
								break;
							case DP_C_PTRDIFF:
								state->value.ff = va_arg(args, ptrdiff_t);
								pps_size_t(state, flags, lc);
								break;
							case DP_C_CHAR:
							default:
								state->value.i = va_arg(args, int);
								pps_int32(state, flags, lc);
								break;
						}
						break;
					case 'X':       // 대문자 부호없는 16진수
						flags |= DP_F_UP; FALLTHROUGH;
					case 'x':       // 소문자 부호없는 16진수 / NOLINT
					case 'o':       // 부호없는 8진수
					case 'b':       // 부호없는 2진수
					case 'u':       // 부호없는 십진수
						state->base = ch == 'b' ? 2 : ch == 'o' ? 8 : ch == 'u' ? 10 : 16;
						flags |= DP_F_UNSIGNED;
						switch (cflags)
						{
							case DP_C_SHORT:
								state->value.ui = (unsigned short)va_arg(args, unsigned int);
								pps_int32(state, flags, lc);
								break;
							case DP_C_LONG:
								state->value.ug = va_arg(args, unsigned long);
								pps_size_t(state, flags, lc);
								break;
							case DP_C_LLONG:
								state->value.ull = va_arg(args, unsigned long long);
								pps_int64(state, flags, lc);
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								state->value.sz = va_arg(args, size_t);
								pps_size_t(state, flags, lc);
								break;
							case DP_C_INTMAX:   // 64비트 취급
								state->value.im = va_arg(args, intmax_t);
								pps_int64(state, flags, lc);
								break;
							case DP_C_PTRDIFF:
								state->value.ff = va_arg(args, ptrdiff_t);
								pps_size_t(state, flags, lc);
								break;
							case DP_C_CHAR:
								state->value.ui = (unsigned char)va_arg(args, int);
								pps_int32(state, flags, lc);
								break;
							default:
								state->value.ui = va_arg(args, unsigned int);
								pps_int32(state, flags, lc);
								break;
						}
						break;
					case 'A':       // 실수의 대문자 16진수 표시 (미구현 '%F'로)
						flags |= DP_F_UP; FALLTHROUGH;
					case 'a':       // 실수의 소문자 16진수 표시 (미구현 '%f'로) / NOLINT
						flags |= DP_F_UNSIGNED;
						state->base = 16;
						if (cflags == DP_C_LDOUBLE)
							state->value.d = (double)va_arg(args, long double);
						else
							state->value.d = va_arg(args, double);
						pps_int64(state, flags, lc);
						break;
					case 'F':       // 실수 대문자
						flags |= DP_F_UP; FALLTHROUGH;
					case 'f':       // 실수
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							state->value.d = (double)va_arg(args, long double);
						else
							state->value.d = va_arg(args, double);
						pps_double(state, flags, lc);
						break;
					case 'E':       // 지수 표시 (대문자)
						flags |= DP_F_UP; FALLTHROUGH;
					case 'e':       // 지수 표시 / NOLINT
						flags |= DP_F_E;
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							state->value.d = (double)va_arg(args, long double);
						else
							state->value.d = va_arg(args, double);
						pps_double(state, flags, lc);
						break;
					case 'G':       // 지수 표시 (대문자)
						flags |= DP_F_UP; FALLTHROUGH;
					case 'g':       // 지수 표시 / NOLINT
						flags |= DP_F_G;
						if (state->vmax == 0) // C99
							state->vmax = 1;
						// long double은 지원 안함...
						if (cflags == DP_C_LDOUBLE)
							state->value.d = (double)va_arg(args, long double);
						else
							state->value.d = va_arg(args, double);
						pps_double(state, flags, lc);
						break;
					case 'c':
						if (cflags == DP_C_LONG)
							flags |= DP_F_UNICODE;
						pps_putch(state, va_arg(args, int), flags);
						break;
					case 'C':
						pps_putch(state, va_arg(args, int), flags | DP_F_UNICODE);
						break;
					case 's':
						if (cflags != DP_C_LONG)
							state->value.s = va_arg(args, char*);
						else
						{
							flags |= DP_F_UNICODE;
							state->value.w = va_arg(args, wchar*);
						}
						pps_string(state, flags);
						break;
					case 'S':
						state->value.w = va_arg(args, wchar*);
						pps_string(state, flags | DP_F_UNICODE);
						break;
					case 'p':
						flags |= DP_F_UNSIGNED | DP_F_UP;
						state->value.s = va_arg(args, char*);
						state->base = 16;
						pps_size_t(state, flags, lc);
						break;
					case 'n':
					{
						PatrickPowellSprintfAny value;
						switch (cflags)
						{
							case DP_C_SHORT:
								value.ph = va_arg(args, short*);
								*value.ph = (short)state->currlen;
								break;
							case DP_C_LONG:
								value.pg = va_arg(args, long*);
								*value.pg = (long)state->currlen;
								break;
							case DP_C_LLONG:
								value.pll = (llong*)va_arg(args, long long*);
								*value.pll = (llong)state->currlen;
								break;
							case DP_C_LDOUBLE:
							case DP_C_SIZE:     // C99에서 원래 부호 있는 크기(ssize_t)를 요구함
								value.psz = va_arg(args, size_t*);
								*value.psz = (size_t)state->currlen;
								break;
							case DP_C_INTMAX:
								value.pim = va_arg(args, intmax_t*);
								*value.pim = (intmax_t)state->currlen;
								break;
							case DP_C_PTRDIFF:
								value.pff = va_arg(args, ptrdiff_t*);
								*value.pff = (ptrdiff_t)state->currlen;
								break;
							case DP_C_CHAR:
								value.s = va_arg(args, char*);
								*value.s = (char)state->currlen;
								break;
							default:
								value.pi = va_arg(args, int*);
								*value.pi = (int)state->currlen;
								break;
						}
					} break;
					case '%':
						state->outch(state, ch);
						break;
					default:    // 알수없음, 넘김
						break;
				}   // switch - ch
				ch = *format++;
				cmd = DP_S_DEFAULT;
				flags = cflags = 0;
				state->base = 0;
				state->vmin = 0;
				state->vmax = -1;
				break;

			case DP_S_DONE:
			default:
				break;
		}   // switch - state
	}   // while

	state->finish(state);
}
