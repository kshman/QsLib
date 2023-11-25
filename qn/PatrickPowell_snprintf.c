#include "pch.h"
#include "qn.h"
#include "PatrickPowell_snprintf.h"
#include <locale.h>

/*
* dopr(): poor man's version of doprintf
*/

#if _QN_64_
#define _str_fmt_size		_str_fmt_long
#define _wcs_fmt_size		_wcs_fmt_long
#else
#define _str_fmt_size		_str_fmt_int
#define _wcs_fmt_size		_wcs_fmt_int
#endif

//////////////////////////////////////////////////////////////////////////
// 인라인 도구

static double pp_pow10(int exp)
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

static double pp_round(double value)
{
	uint64_t intpart = (uint64_t)value;
	value = value - intpart;
	return (double)(value >= 0.5 ? intpart + 1 : intpart);
}

static uint64_t pp_floor(double value)
{
	uint64_t intpart = (uint64_t)value;
	return intpart;
}

static bool pp_is_nan(double value)
{
	return value != value;
}

static bool pp_is_inf(double value)
{
	return value != 0.0 && value + value == value;
}

static int pp_exponent(double value)
{
	double d = value < 0.0 ? -value : value;
	int exp = 0;
	while (d < 1.0 && d > 0.0 && --exp > -99) d *= 10;
	while (d >= 10.0 && ++exp < 99) d /= 10;
	return exp;
}

static uint64_t pp_intpart(double value)
{
	if (value >= UINT_FAST64_MAX)
		return UINT_FAST64_MAX;
	uint64_t res = (uint64_t)value;
	return res <= value ? res : res - 1;
}

static size_t pp_getasc(char* outasc, size_t outsize, const wchar_t* inuni)
{
	size_t cnt;
#if _MSC_VER
	wcstombs_s(&cnt, outasc, outsize, inuni, outsize);
#else
	cnt = wcstombs(outasc, inuni, outsize);
#endif
	outasc[cnt] = '\0';
	return cnt;
}

static size_t pp_getuni(wchar_t* outuni, size_t outsize, const char* inasc)
{
	size_t cnt;
#if _MSC_VER
	mbstowcs_s(&cnt, outuni, outsize, inasc, outsize);
#else
	cnt = mbstowcs(outuni, inasc, outsize);
#endif
	outuni[cnt] = L'\0';
	return cnt;
}


//////////////////////////////////////////////////////////////////////////
// 아스키 버전

// 문자를 숫자로
#define char_to_int(p)	((p) - '0')

// 진수별 처리
static const char* _base_to_str[2] =
{
	"0123456789abcdef",
	"0123456789ABCDEF",
};

// 숫자를 문자열로
static int _conv_int_to_str(uint32_t value, char* buf, int size, int base, bool isup)
{
	const char* bts = _base_to_str[isup ? 1 : 0];
	int pos = 0;

	do
	{
		buf[pos++] = bts[value % base];
		value /= base;
	} while (value && pos < size);

	buf[pos] = '\0';

	return pos;
}

// 숫자를 문자열로
static int _conv_long_to_str(uint64_t value, char* buf, int size, int base, bool isup)
{
	const char* bts = _base_to_str[isup ? 1 : 0];
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
static int _get_quote_seps_str(int value, int flags
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

// 문자열
static void _str_fmt_str(_sn_outfunc outfn, pointer_t ptr, size_t* currlen, size_t maxlen,
	const char* value, int flags, int min, int max)
{
	int padlen;
	int strln;
	int cnt;

	strln = (int)strlen(value);

	padlen = min - strln;

	if (padlen < 0)
		padlen = 0;

	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	cnt = 0;

	while (padlen > 0 && cnt < max)
	{
		outfn(ptr, currlen, maxlen, ' ');
		--padlen;
		++cnt;
	}

	while (*value && cnt < max)
	{
		outfn(ptr, currlen, maxlen, *value++);
		++cnt;
	}

	while (padlen < 0 && cnt < max)
	{
		outfn(ptr, currlen, maxlen, ' ');
		++padlen;
		++cnt;
	}
}

// 32비트 정수
static void _str_fmt_int(_sn_outfunc outfn, pointer_t ptr, size_t* currlen, size_t maxlen, _sn_anyval* value, uint32_t base, int min, int max, int flags)
{
#define MAX_CONVERT_PLACES 40
	uint32_t uvalue;
	char convert[MAX_CONVERT_PLACES];
	int hexprefix;
	int signvalue;
	int place;
	int spadlen;
	int zpadlen;
	int quote;
#if USE_LOCALE_INFO
	struct lconv* lc = localeconv();	// CRT 로캘
#endif

	//
	if (max < 0)
		max = 0;

	//
	if (flags & DP_F_UNSIGNED)
	{
		uvalue = value->ui;
		signvalue = 0;
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
				signvalue = 0;
		}
	}

	//
	place = _conv_int_to_str(uvalue, convert, MAX_CONVERT_PLACES - 1, base, flags & DP_F_UP);

	// '#'이 있을때 접두사 처리
	if ((flags & DP_F_NUM) && uvalue != 0)
	{
		if (base == 16)
			hexprefix = flags & DP_F_UP ? 'X' : 'x';
		else
		{
			hexprefix = 0;

			if (base == 8 && max <= place)
				max = place + 1;
		}
	}
	else
	{
		hexprefix = 0;
	}

	//
#if USE_LOCALE_INFO
	quote = _get_quote_seps_str(place, flags, lc);
#else
	quote = _get_quote_seps_str(place, flags);
#endif

	//
	zpadlen = max - place - quote;
	spadlen = min - quote - QN_MAX(max, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

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
		outfn(ptr, currlen, maxlen, ' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		outfn(ptr, currlen, maxlen, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		outfn(ptr, currlen, maxlen, '0');
		outfn(ptr, currlen, maxlen, hexprefix);
	}

	// '0' 삽입
	while (zpadlen > 0)
	{
		outfn(ptr, currlen, maxlen, '0');
		--zpadlen;
	}

	// 숫자
	while (place > 0)
	{
		--place;
		outfn(ptr, currlen, maxlen, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				outfn(ptr, currlen, maxlen, ',');
			else
			{
				const char* psz;
				for (psz = lc->thousands_sep; *psz; psz++)
					outfn(ptr, currlen, maxlen, *psz);
			}
#else
			outfn(ptr, currlen, maxlen, ',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		outfn(ptr, currlen, maxlen, ' ');
		++spadlen;
	}
#undef MAX_CONVERT_PLACES
}

// 64비트 정수
static void _str_fmt_long(_sn_outfunc outfn, pointer_t ptr, size_t* currlen, size_t maxlen, _sn_anyval* value, uint32_t base, int min, int max, int flags)
{
#define MAX_CONVERT_PLACES 80
	uint64_t uvalue;
	char convert[MAX_CONVERT_PLACES];
	int hexprefix;
	int signvalue;
	int place;
	int spadlen;
	int zpadlen;
	int quote;
#if USE_LOCALE_INFO
	struct lconv* lc = localeconv();
#endif

	//
	if (max < 0)
		max = 0;

	//
	if (flags & DP_F_UNSIGNED)
	{
		uvalue = value->ul;
		signvalue = 0;
	}
	else
	{
		if (value->l < 0)
		{
			uvalue = -value->l;
			signvalue = '-';
		}
		else
		{
			uvalue = value->l;

			if (flags & DP_F_PLUS)  // 부호추가 (+/i)
				signvalue = '+';
			else if (flags & DP_F_SPACE)
				signvalue = ' ';
			else
				signvalue = 0;
		}
	}

	//
	place = _conv_long_to_str(uvalue, convert, MAX_CONVERT_PLACES - 1, base, flags & DP_F_UP);

	// '#'이 있을때 접두사 처리
	if ((flags & DP_F_NUM) && uvalue != 0)
	{
		if (base == 16)
			hexprefix = flags & DP_F_UP ? 'X' : 'x';
		else
		{
			hexprefix = 0;

			if (base == 8 && max <= place)
				max = place + 1;
		}
	}
	else
	{
		hexprefix = 0;
	}

	//
#if USE_LOCALE_INFO
	quote = _get_quote_seps_str(place, flags, lc);
#else
	quote = _get_quote_seps_str(place, flags);
#endif

	//
	zpadlen = max - place - quote;
	spadlen = min - quote - QN_MAX(max, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

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

	// 공백
	while (spadlen > 0)
	{
		outfn(ptr, currlen, maxlen, ' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		outfn(ptr, currlen, maxlen, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		outfn(ptr, currlen, maxlen, '0');
		outfn(ptr, currlen, maxlen, hexprefix);
	}

	// '0' 삽입
	if (zpadlen > 0)
	{
		while (zpadlen > 0)
		{
			outfn(ptr, currlen, maxlen, '0');
			--zpadlen;
		}
	}

	// 숫자
	while (place > 0)
	{
		--place;
		outfn(ptr, currlen, maxlen, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				outfn(ptr, currlen, maxlen, ',');
			else
			{
				const char* psz;
				for (psz = lc->thousands_sep; *psz; psz++)
					outfn(ptr, currlen, maxlen, *psz);
			}
#else
			outfn(ptr, currlen, maxlen, ',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		outfn(ptr, currlen, maxlen, ' ');
		++spadlen;
	}
#undef MAX_CONVERT_PLACES
}

// 64비트 실수(double)
static void _str_fmt_fp(_sn_outfunc outfn, pointer_t ptr, size_t* currlen, size_t maxlen, _sn_anyval* value, int min, int max, int flags)
{
	double fvalue;
	double uvalue;
	uint64_t intpart;
	uint64_t fracpart;
	uint64_t mask;
	char iconvert[311];
	char fconvert[311];
	char econvert[10];
	int signvalue;
	int esignvalue;
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
	if (max < 0)
		max = 6;

	//
	fvalue = value->d;

	if (fvalue < 0)
		signvalue = '-';
	else if (flags & DP_F_PLUS) // 부호추가 (+/i)
		signvalue = '+';
	else if (flags & DP_F_SPACE)
		signvalue = ' ';
	else
		signvalue = 0;

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
			iconvert[iplace++] = (char)signvalue;

		while (*psz)
			iconvert[iplace++] = *psz++;

		iconvert[iplace] = '\0';

		_str_fmt_str(outfn, ptr, currlen, maxlen, iconvert, flags, min, iplace);

		// 여기서 끝냄
		return;
	}

	// 지수 모양 처리
	if (flags & (DP_F_E | DP_F_G))
	{
		if (flags & DP_F_G)
		{
			--max;
			omitzeros = !(flags & DP_F_NUM);
		}
		else
		{
			omitzeros = 0;
		}

		exponent = pp_exponent(fvalue);
		etype = 1;
	}
	else
	{
		exponent = 0;
		omitzeros = 0;
		etype = 0;
	}

pos_exp_again:
	// 이 루프는 확실한 반올림 지수를 얻기 위함 -> '%g'

	// 오직 16자리 정수만 지원
	// 왜냠시롱 여기서는 그것만 지원
	// 원래는 9, 19, 38 자리수, 각각 32, 64, 128비트용
	if (max > 16)
		max = 16;

	// 정수로 변환
	uvalue = QN_ABS(fvalue);

	if (etype)
		uvalue /= pp_pow10(exponent);

	intpart = pp_intpart(uvalue);

	// 오버플로우, 현재 사용 안함
	//if (intpart==UINT_FAST64_MAX) return FALSE;

	//
	mask = (uint64_t)pp_pow10(max);
	fracpart = (uint64_t)pp_round(mask * (uvalue - intpart));

	if (fracpart >= mask)
	{
		// 예를 들면 uvalue=1.99952, intpart=2, temp=1000 (max=3 이므로)
		// SNP_ROUND(1000*0.99952)=1000. 그리하여, 정수부는 1증가, 실수부는 0으로 함.
		intpart++;
		fracpart = 0;

		if (etype && intpart == 10)
		{
			intpart = 1;
			exponent++;
		}
	}

	// 정확한 지수를 얻음. '%g'를 위한 지수를 재계산
	if (etype && (flags & DP_F_G) && (max + 1) > exponent && exponent >= -4)
	{
		max -= exponent;
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
		{
			esignvalue = '+';
		}

		eplace = _conv_int_to_str(exponent, econvert, 2, 10, FALSE);

		if (eplace == 1)
		{
			// MSVC는 자리수를 3자리 씀. (001+E)
			// 여기서는 2자리만. (01+E)
			econvert[eplace++] = '0';
		}

		econvert[eplace++] = (char)esignvalue;
		econvert[eplace++] = flags & DP_F_UP ? 'E' : 'e';
		econvert[eplace] = '\0';
	}
	else
	{
		esignvalue = 0;
		eplace = 0;
	}

	// 정수부 처리
	iplace = _conv_long_to_str(intpart, iconvert, 311 - 1, 10, FALSE);

	// 실수부 처리
	// iplace=1 이면 fracpart==0 임.
	fplace = fracpart == 0 ? 0 : _conv_long_to_str(fracpart, fconvert, 311 - 1, 10, FALSE);

	// 지수부 처리
	zleadfrac = max - fplace;

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
			omitcount = max;
			zleadfrac = 0;
		}

		max -= omitcount;
	}

	// 실수부가 0이 아니고, '#' 플래그가 있으면 정수 포인트 출력
	dotpoint = max > 0 || (flags & DP_F_NUM);

	//
#if USE_LOCALE_INFO
	quote = _get_quote_seps_str(iplace, flags, lc);
#else
	quote = _get_quote_seps_str(iplace, flags);
#endif

	// -1은 정수 포인트용, 부호를 출력하면 추가로 -1;
	padlen = min - iplace - eplace - max - quote - (dotpoint ? 1 : 0) - (signvalue ? 1 : 0);

	if (padlen < 0)
		padlen = 0;

	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	// 부호 및 패딩 숫자 출력
	if ((flags & DP_F_ZERO) && padlen > 0)
	{
		if (signvalue)
		{
			outfn(ptr, currlen, maxlen, signvalue);
			signvalue = 0;
		}

		while (padlen > 0)
		{
			outfn(ptr, currlen, maxlen, '0');
			--padlen;
		}
	}

	// 남은 패딩 공백 출력
	while (padlen > 0)
	{
		outfn(ptr, currlen, maxlen, ' ');
		--padlen;
	}

	// 패딩에서 부호 출력 안했으면 부호 출력
	if (signvalue)
		outfn(ptr, currlen, maxlen, signvalue);

	// 정수부 출력
	while (iplace > 0)
	{
		--iplace;
		outfn(ptr, currlen, maxlen, iconvert[iplace]);

		if (quote && iplace > 0 && (iplace % 3) == 0)
		{
#if USE_LOCALE_INFO
			if (!lc->thousands_sep)
				outfn(ptr, currlen, maxlen, ',');
			else
			{
				for (psz = lc->thousands_sep; *psz; psz++)
					outfn(ptr, currlen, maxlen, *psz);
			}
#else
			outfn(ptr, currlen, maxlen, ',');
#endif
		}
	}

	// 소수점
	if (dotpoint)
	{
#if USE_LOCALE_INFO
		if (!lc->decimal_point)
			outfn(ptr, currlen, maxlen, '.');
		else
		{
			for (psz = lc->decimal_point; *psz; psz++)
				outfn(ptr, currlen, maxlen, *psz);
		}
#else
		outfn(ptr, currlen, maxlen, '.');
#endif
	}

	// 실수부 앞쪽 패딩
	while (zleadfrac > 0)
	{
		outfn(ptr, currlen, maxlen, '0');
		--zleadfrac;
	}

	// 실수부
	while (fplace > omitcount)
		outfn(ptr, currlen, maxlen, fconvert[--fplace]);

	// 지수값
	while (eplace > 0)
		outfn(ptr, currlen, maxlen, econvert[--eplace]);

	// 남은 패팅
	while (padlen < 0)
	{
		outfn(ptr, currlen, maxlen, ' ');
		++padlen;
	}
}

// 여기가 진짜 asc용 처리
size_t dopr(_sn_outfunc outfn, _sn_clfunc clfn, pointer_t ptr, size_t maxlen, const char* format, va_list args)
{
	char ch;
	int min;
	int max;
	int state;
	int flags;
	int cflags;
	int base;
	size_t currlen;

	_sn_anyval value;
	char tmp[1024];
	wchar_t wtmp[3];

	state = DP_S_DEFAULT;
	currlen = flags = cflags = min = base = 0;
	max = -1;
	ch = *format++;

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
					outfn(ptr, &currlen, maxlen, ch);

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
					min = 10 * min + char_to_int(ch);
					ch = *format++;
				}
				else if (ch == '*')
				{
					min = va_arg(args, int);

					if (min < 0)
					{
						flags |= DP_F_MINUS;
						min = -min;
					}

					ch = *format++;
					state = DP_S_DOT;
				}
				else
				{
					state = DP_S_DOT;
				}

				break;

			case DP_S_DOT:
				if (ch == '.')
				{
					state = DP_S_MAX;
					ch = *format++;
				}
				else
				{
					state = DP_S_MOD;
				}

				break;

			case DP_S_MAX:
				if (isdigit(ch))
				{
					if (max < 0)
						max = 0;

					max = 10 * max + char_to_int(ch);
					ch = *format++;
				}
				else if (ch == '*')
				{
					max = va_arg(args, int);

					if (max < 0)
						max = -1;

					ch = *format++;
					state = DP_S_MOD;
				}
				else
				{
					if (max < 0)
						max = 0;

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
						{
							cflags = DP_C_SHORT;
						}

						break;

					case 'l':
						ch = *format++;

						if (ch == 'l')      // long long, int64_t, 64비트 정수
						{
							cflags = DP_C_LLONG;
							ch = *format++;
						}
						else
						{
							cflags = DP_C_LONG;
						}

						break;

					case 'L':               // long double, 지원 안함. 또는 size_t
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
								value.i = (short)va_arg(args, int);
								_str_fmt_int(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_LONG:
#if !_QN_WINDOWS_
								value.l = va_arg(args, long);
								_str_fmt_long(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
#else
								value.i = va_arg(args, int);
								_str_fmt_int(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
#endif
								break;

							case DP_C_LLONG:
								value.l = va_arg(args, long long);
								_str_fmt_long(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								value.sz = va_arg(args, size_t);
								_str_fmt_size(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_INTMAX:   // 64비트
								value.im = va_arg(args, intmax_t);
								_str_fmt_long(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_PTRDIFF:
								value.ff = va_arg(args, ptrdiff_t);
								_str_fmt_size(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_CHAR:
								value.i = (char)va_arg(args, int);
								_str_fmt_int(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							default:
								value.i = va_arg(args, int);
								_str_fmt_int(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;
						}

						break;

					case 'X':       // 대문자 부호없는 16진수
						flags |= DP_F_UP;

					case 'x':       // 소문자 부호없는 16진수
						base = 16;

					case 'o':       // 부호없는 8진수
						if (base == 0)
							base = 8;

					case 'b':       // 부호없는 2진수
						if (base == 0)
							base = 2;

					case 'u':       // 부호없는 십진수
						if (base == 0)
							base = 10;

						flags |= DP_F_UNSIGNED;

						switch (cflags)
						{
							case DP_C_SHORT:
								value.ui = (unsigned short)va_arg(args, unsigned int);
								_str_fmt_int(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_LONG:
#if !_QN_WINDOWS_
								value.ul = va_arg(args, unsigned long);
								_str_fmt_long(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
#else
								value.ui = va_arg(args, unsigned int);
								_str_fmt_int(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
#endif
								break;

							case DP_C_LLONG:
								value.ul = va_arg(args, unsigned long long);
								_str_fmt_long(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								value.sz = va_arg(args, size_t);
								_str_fmt_size(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_INTMAX:   // 64비트
								value.im = va_arg(args, intmax_t);
								_str_fmt_long(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_PTRDIFF:
								value.ff = va_arg(args, ptrdiff_t);
								_str_fmt_size(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_CHAR:
								value.ui = (unsigned char)va_arg(args, int);
								_str_fmt_int(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							default:
								value.ui = va_arg(args, unsigned int);
								_str_fmt_int(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;
						}

						break;

					case 'A':       // 실수의 대문자 16진수 표시 (미구현 '%F'로)

					case 'F':       // 실수 대문자
						flags |= DP_F_UP;

					case 'a':       // 실수의 소문자 16진수 표시 (미구현 '%f'로)

					case 'f':       // 실수
						if (cflags == DP_C_LDOUBLE)
						{
#if 0
							// long double은 보통 지원 안함...
							value.ld = va_arg(args, long double);
							_str_fmt_fplong(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#else
							value.d = (double)va_arg(args, long double);
							_str_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#endif
						}
						else
						{
							value.d = va_arg(args, double);
							_str_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
						}

						break;

					case 'E':       // 지수 표시 (대문자)
						flags |= DP_F_UP;

					case 'e':       // 지수 표시
						flags |= DP_F_E;

						if (cflags == DP_C_LDOUBLE)
						{
#if 0
							value.ld = va_arg(args, long double);
							_str_fmt_fplong(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#else
							value.d = (double)va_arg(args, long double);
							_str_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#endif
						}
						else
						{
							value.d = va_arg(args, double);
							_str_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
						}

						break;

					case 'G':       // 지수 표시 (대문자)
						flags |= DP_F_UP;

					case 'g':       // 지수 표시
						flags |= DP_F_G;

						if (max == 0) // C99
							max = 1;

						if (cflags == DP_C_LDOUBLE)
						{
#if 0
							value.ld = va_arg(args, long double);
							_str_fmt_fplong(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#else
							value.d = (double)va_arg(args, long double);
							_str_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#endif
						}
						else
						{
							value.d = va_arg(args, double);
							_str_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
						}

						break;

					case 'c':
						if (cflags != DP_C_LONG)
							outfn(ptr, &currlen, maxlen, va_arg(args, int));
						else
						{
							wtmp[0] = va_arg(args, wchar_t);
							wtmp[1] = L'\0';
							max = (int)pp_getasc(tmp, 30, wtmp);
							_str_fmt_str(outfn, ptr, &currlen, maxlen, tmp, 0, 0, max);
						}

						break;

					case 'C':
						if (cflags == DP_C_SHORT)
							outfn(ptr, &currlen, maxlen, va_arg(args, int));
						else
						{
							wtmp[0] = va_arg(args, wchar_t);
							wtmp[1] = L'\0';
							max = (int)pp_getasc(tmp, 30, wtmp);
							_str_fmt_str(outfn, ptr, &currlen, maxlen, tmp, 0, 0, max);
						}

						break;

					case 's':
						if (cflags != DP_C_LONG)
						{
							value.s = va_arg(args, char*);

							if (!value.s)
								value.s = "(null)";

							if (max < 0)
								max = (int)strlen(value.s);

							if (min > 0 && max >= 0 && min > max)
								max = min;

							_str_fmt_str(outfn, ptr, &currlen, maxlen, value.s, flags, min, max);
						}
						else
						{
							value.w = va_arg(args, wchar_t*);

							if (!value.w)
							{
								value.s = "(null)";

								if (max == -1)
									max = 6;
							}
							else
							{
								pp_getasc(tmp, 1024 - 1, value.w);
								value.s = tmp;

								if (max == -1)
									max = (int)strlen(value.s);
							}

							if (min > 0 && max >= 0 && min > max)
								max = min;

							_str_fmt_str(outfn, ptr, &currlen, maxlen, value.s, flags, min, max);
						}

						break;

					case 'S':
						if (cflags == DP_C_SHORT)
						{
							value.s = va_arg(args, char*);

							if (!value.s)
								value.s = "(null)";

							if (max == -1)
								max = (int)strlen(value.s);

							if (min > 0 && max >= 0 && min > max)
								max = min;

							_str_fmt_str(outfn, ptr, &currlen, maxlen, value.s, flags, min, max);
						}
						else
						{
							value.w = va_arg(args, wchar_t*);

							if (!value.w)
							{
								value.s = "(null)";

								if (max == -1)
									max = 6;
							}
							else
							{
								pp_getasc(tmp, 1024 - 1, value.w);
								value.s = tmp;

								if (max == -1)
									max = (int)strlen(value.s);
							}

							if (min > 0 && max >= 0 && min > max)
								max = min;

							_str_fmt_str(outfn, ptr, &currlen, maxlen, value.s, flags, min, max);
						}

						break;

					case 'p':
						flags |= DP_F_UNSIGNED | DP_F_UP;
						value.s = va_arg(args, char*);
						_str_fmt_size(outfn, ptr, &currlen, maxlen, &value, 16, min, max, flags);
						break;

					case 'n':
						switch (cflags)
						{
							case DP_C_SHORT:
								value.ph = va_arg(args, short*);
								*value.ph = (int16_t)currlen;
								break;

							case DP_C_LONG:
#if !_QN_WINDOWS_
								value.pl = va_arg(args, long*);
								*value.pl = (int64_t)currlen;
#else
								value.pi = va_arg(args, int*);
								*value.pi = (int)currlen;
#endif
								break;

							case DP_C_LLONG:
								value.pl = va_arg(args, long long*);
								*value.pl = (int64_t)currlen;
								break;

#if 0

							case DP_C_LDOUBLE:  // 기능 없음
								break;
#endif

							case DP_C_SIZE:     // C99에서 원래 부호 있는 크기(ssize_t)를 요구함
								value.psz = va_arg(args, size_t*);
								*value.psz = (size_t)currlen;
								break;

							case DP_C_INTMAX:   // 64비트
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
						outfn(ptr, &currlen, maxlen, ch);
						break;

					default:    // 알수없음, 넘김
						break;
				}   // switch - ch

				ch = *format++;
				state = DP_S_DEFAULT;
				flags = cflags = min = base = 0;
				max = -1;
				break;

			case DP_S_DONE:
				break;

			default:    // 음?
				break; // 일부 엉뚱한 컴파일러는 break를 요구함... 이라고 써있었음
		}   // switch - state
	}   // while

	if (clfn && maxlen != 0)
		clfn(ptr, currlen, maxlen);

	return currlen;
}


//////////////////////////////////////////////////////////////////////////
// 유니코드 버전

// 문자를 숫자로
#define wch_to_int(p)	((p) - L'0')

// 진수별 처리
static const wchar_t* _base_to_wcs[2] =
{
	L"0123456789abcdef",
	L"0123456789ABCDEF",
};

// 숫자를 문자열로
static int _conv_int_to_wcs(uint32_t value, wchar_t* buf, int size, int base, bool isup)
{
	const wchar_t* bts = _base_to_wcs[isup ? 1 : 0];
	int pos = 0;

	do
	{
		buf[pos++] = bts[value % base];
		value /= base;
	} while (value && pos < size);

	buf[pos] = L'\0';

	return pos;
}

// 숫자를 문자열로
static int _conv_long_to_wcs(uint64_t value, wchar_t* buf, int size, int base, bool isup)
{
	const wchar_t* bts = _base_to_wcs[isup ? 1 : 0];
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
static int _get_quote_seps_wcs(int value, int flags
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

#if USE_WIDE_LOCALE
		if (lc->_W_thousands_sep)
			ret *= (int)wcslen(lc->_W_thousands_sep);
#endif
	}

	return ret;
}

// 문자열
static void _wcs_fmt_wcs(_sn_outfunc outfn, pointer_t ptr, size_t* currlen, size_t maxlen,
	const wchar_t* value, int flags, int min, int max)
{
	int padlen;
	int wcsln;
	int cnt;

	wcsln = (int)wcslen(value);

	padlen = min - wcsln;

	if (padlen < 0)
		padlen = 0;

	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	cnt = 0;

	while (padlen > 0 && cnt < max)
	{
		outfn(ptr, currlen, maxlen, L' ');
		--padlen;
		++cnt;
	}

	while (*value && cnt < max)
	{
		outfn(ptr, currlen, maxlen, *value++);
		++cnt;
	}

	while (padlen < 0 && cnt < max)
	{
		outfn(ptr, currlen, maxlen, L' ');
		++padlen;
		++cnt;
	}
}

// 32비트 정수
static void _wcs_fmt_int(_sn_outfunc outfn, pointer_t ptr, size_t* currlen, size_t maxlen, _sn_anyval* value, uint32_t base, int min, int max, int flags)
{
#define MAX_CONVERT_PLACES 40
	uint32_t uvalue;
	wchar_t convert[MAX_CONVERT_PLACES];
	int hexprefix;
	int signvalue;
	int place;
	int spadlen;
	int zpadlen;
	int quote;
#if USE_LOCALE_INFO
	struct lconv* lc = localeconv();
#endif

	if (max < 0)
		max = 0;

	if (flags & DP_F_UNSIGNED)
	{
		uvalue = value->ui;
		signvalue = 0;
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
				signvalue = 0;
		}
	}

	place = _conv_int_to_wcs(uvalue, convert, MAX_CONVERT_PLACES - 1, base, flags & DP_F_UP);

	if ((flags & DP_F_NUM) && uvalue != 0)
	{
		if (base == 16)
			hexprefix = flags & DP_F_UP ? L'X' : L'x';
		else
		{
			hexprefix = 0;

			if (base == 8 && max <= place)
				max = place + 1;
		}
	}
	else
	{
		hexprefix = 0;
	}

#if USE_LOCALE_INFO
	quote = _get_quote_seps_wcs(place, flags, lc);
#else
	quote = _get_quote_seps_wcs(place, flags);
#endif

	zpadlen = max - place - quote;
	spadlen = min - quote - QN_MAX(max, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

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

	// 공백
	while (spadlen > 0)
	{
		outfn(ptr, currlen, maxlen, L' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		outfn(ptr, currlen, maxlen, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		outfn(ptr, currlen, maxlen, L'0');
		outfn(ptr, currlen, maxlen, hexprefix);
	}

	// '0' 삽입
	while (zpadlen > 0)
	{
		outfn(ptr, currlen, maxlen, L'0');
		--zpadlen;
	}

	// 숫자
	while (place > 0)
	{
		--place;
		outfn(ptr, currlen, maxlen, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_WIDE_LOCALE
			if (!lc->_W_thousands_sep)
				outfn(ptr, currlen, maxlen, L',');
			else
			{
				const wchar_t* pwz;
				for (pwz = lc->_W_thousands_sep; *pwz; pwz++)
					outfn(ptr, currlen, maxlen, *pwz);
			}
#else
			// 아띠... 유니코드가 엄씀
			outfn(ptr, currlen, maxlen, L',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		outfn(ptr, currlen, maxlen, L' ');
		++spadlen;
	}

#undef MAX_CONVERT_PLACES
}

// 64비트 정수
static void _wcs_fmt_long(_sn_outfunc outfn, pointer_t ptr, size_t* currlen, size_t maxlen, _sn_anyval* value, uint32_t base, int min, int max, int flags)
{
#define MAX_CONVERT_PLACES 80
	uint64_t uvalue;
	wchar_t convert[MAX_CONVERT_PLACES];
	int hexprefix;
	int signvalue;
	int place;
	int spadlen;
	int zpadlen;
	int quote;
#if USE_LOCALE_INFO
	struct lconv* lc = localeconv();
#endif

	if (max < 0)
		max = 0;

	if (flags & DP_F_UNSIGNED)
	{
		uvalue = value->ul;
		signvalue = 0;
	}
	else
	{
		if (value->l < 0)
		{
			uvalue = -value->l;
			signvalue = L'-';
		}
		else
		{
			uvalue = value->l;

			if (flags & DP_F_PLUS)  // 부호추가 (+/i)
				signvalue = L'+';
			else if (flags & DP_F_SPACE)
				signvalue = L' ';
			else
				signvalue = 0;
		}
	}

	place = _conv_long_to_wcs(uvalue, convert, MAX_CONVERT_PLACES - 1, base, flags & DP_F_UP);

	if ((flags & DP_F_NUM) && uvalue != 0)
	{
		if (base == 16)
			hexprefix = flags & DP_F_UP ? L'X' : L'x';
		else
		{
			hexprefix = 0;

			if (base == 8 && max <= place)
				max = place + 1;
		}
	}
	else
	{
		hexprefix = 0;
	}

#if USE_LOCALE_INFO
	quote = _get_quote_seps_wcs(place, flags, lc);
#else
	quote = _get_quote_seps_wcs(place, flags);
#endif

	zpadlen = max - place - quote;
	spadlen = min - quote - QN_MAX(max, place) - (signvalue ? 1 : 0) - (hexprefix ? 2 : 0);

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

	// 공백
	while (spadlen > 0)
	{
		outfn(ptr, currlen, maxlen, L' ');
		--spadlen;
	}

	// 부호
	if (signvalue)
		outfn(ptr, currlen, maxlen, signvalue);

	// 16진수 접두사
	if (hexprefix)
	{
		outfn(ptr, currlen, maxlen, L'0');
		outfn(ptr, currlen, maxlen, hexprefix);
	}

	// '0' 삽입
	if (zpadlen > 0)
	{
		while (zpadlen > 0)
		{
			outfn(ptr, currlen, maxlen, L'0');
			--zpadlen;
		}
	}

	// 숫자
	while (place > 0)
	{
		--place;
		outfn(ptr, currlen, maxlen, convert[place]);

		if (quote && place > 0 && (place % 3) == 0)
		{
#if USE_WIDE_LOCALE
			if (!lc->_W_thousands_sep)
				outfn(ptr, currlen, maxlen, L',');
			else
			{
				const wchar_t* pwz;
				for (pwz = lc->_W_thousands_sep; *pwz; pwz++)
					outfn(ptr, currlen, maxlen, *pwz);
			}
#else
			// 크흑.. 유니코드
			outfn(ptr, currlen, maxlen, L',');
#endif
		}
	}

	// 왼쪽 정렬에 따른 공백
	while (spadlen < 0)
	{
		outfn(ptr, currlen, maxlen, L' ');
		++spadlen;
	}

#undef MAX_CONVERT_PLACES
}

// 64비트 실수(double)
static void _wcs_fmt_fp(_sn_outfunc outfn, pointer_t ptr, size_t* currlen, size_t maxlen, _sn_anyval* value, int min, int max, int flags)
{
	double fvalue;
	double uvalue;
	uint64_t intpart;
	uint64_t fracpart;
	uint64_t mask;
	wchar_t iconvert[311];
	wchar_t fconvert[311];
	wchar_t econvert[10];
	int signvalue;
	int esignvalue;
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
	const wchar_t* pwz;
#if USE_LOCALE_INFO
	struct lconv* lc = localeconv();
#endif

	/*
	* AIX manpage says the default is 0, but Solaris says the default
	* is 6, and sprintf on AIX defaults to 6
	*/
	if (max < 0)
		max = 6;

	fvalue = value->d;

	if (fvalue < 0)
		signvalue = L'-';
	else if (flags & DP_F_PLUS) // 부호추가 (+/i)
		signvalue = L'+';
	else if (flags & DP_F_SPACE)
		signvalue = L' ';
	else
		signvalue = 0;

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
			iconvert[iplace++] = (char)signvalue;

		while (*pwz)
			iconvert[iplace++] = *pwz++;

		iconvert[iplace] = '\0';

		_wcs_fmt_wcs(outfn, ptr, currlen, maxlen, iconvert, flags, min, iplace);

		// 여기서 끝냄
		return;
	}

	// 지수 모양 처리
	if (flags & (DP_F_E | DP_F_G))
	{
		if (flags & DP_F_G)
		{
			--max;
			omitzeros = !(flags & DP_F_NUM);
		}
		else
		{
			omitzeros = 0;
		}

		exponent = pp_exponent(fvalue);
		etype = 1;
	}
	else
	{
		exponent = 0;
		omitzeros = 0;
		etype = 0;
	}

pos_exp_again:
	// 이 루프는 확실한 반올림 지수를 얻기 위함 -> '%g'

	// 오직 16자리 정수만 지원
	// 왜냠시롱 여기서는 그것만 지원
	// 원래는 9, 19, 38 자리수, 각각 32, 64, 128비트용
	if (max > 16)
		max = 16;

	// 정수로 변환
	uvalue = QN_ABS(fvalue);

	if (etype)
		uvalue /= pp_pow10(exponent);

	intpart = pp_intpart(uvalue);

	// 오버플로우, 현재 사용 안함
	//if (intpart==UINT_FAST64_MAX) return FALSE;

	//
	mask = (uint64_t)pp_pow10(max);
	fracpart = (uint64_t)pp_round(mask * (uvalue - intpart));

	if (fracpart >= mask)
	{
		// 예를 들면 uvalue=1.99952, intpart=2, temp=1000 (max=3 이므로)
		// SNP_ROUND(1000*0.99952)=1000. 그리하여, 정수부는 1증가, 실수부는 0으로 함.
		intpart++;
		fracpart = 0;

		if (etype && intpart == 10)
		{
			intpart = 1;
			exponent++;
		}
	}

	// 정확한 지수를 얻음. '%g'를 위한 지수를 재계산
	if (etype && (flags & DP_F_G) && (max + 1) > exponent && exponent >= -4)
	{
		max -= exponent;
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
		{
			esignvalue = L'+';
		}

		eplace = _conv_int_to_wcs(exponent, econvert, 2, 10, FALSE);

		if (eplace == 1)
		{
			// MSVC는 자리수를 3자리 씀. (001+E)
			// 여기서는 2자리만. (01+E)
			econvert[eplace++] = L'0';
		}

		econvert[eplace++] = (wchar_t)esignvalue;
		econvert[eplace++] = flags & DP_F_UP ? L'E' : L'e';
		econvert[eplace] = L'\0';
	}
	else
	{
		esignvalue = 0;
		eplace = 0;
	}

	// 정수부 처리
	iplace = _conv_long_to_wcs(intpart, iconvert, 311 - 1, 10, FALSE);

	// 실수부 처리
	// iplace=1 이면 fracpart==0 임.
	fplace = fracpart == 0 ? 0 : _conv_long_to_wcs(fracpart, fconvert, 311 - 1, 10, FALSE);

	// 지수부 처리
	zleadfrac = max - fplace;

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
			omitcount = max;
			zleadfrac = 0;
		}

		max -= omitcount;
	}

	// 실수부가 0이 아니고, '#' 플래그가 있으면 정수 포인트 출력
	dotpoint = max > 0 || (flags & DP_F_NUM);

	//
#if USE_LOCALE_INFO
	quote = _get_quote_seps_wcs(iplace, flags, lc);
#else
	quote = _get_quote_seps_wcs(iplace, flags);
#endif

	// -1은 정수 포인트용, 부호를 출력하면 추가로 -1;
	padlen = min - iplace - eplace - max - quote - (dotpoint ? 1 : 0) - (signvalue ? 1 : 0);

	if (padlen < 0)
		padlen = 0;

	if (flags & DP_F_MINUS)
		padlen = -padlen; // 왼쪽 정렬

	if ((flags & DP_F_ZERO) && padlen > 0)
	{
		if (signvalue)
		{
			outfn(ptr, currlen, maxlen, signvalue);
			signvalue = 0;
		}

		while (padlen > 0)
		{
			outfn(ptr, currlen, maxlen, L'0');
			--padlen;
		}
	}

	while (padlen > 0)
	{
		outfn(ptr, currlen, maxlen, L' ');
		--padlen;
	}

	if (signvalue)
		outfn(ptr, currlen, maxlen, signvalue);

	while (iplace > 0)
	{
		--iplace;
		outfn(ptr, currlen, maxlen, iconvert[iplace]);

		if (quote && iplace > 0 && (iplace % 3) == 0)
		{
#if USE_WIDE_LOCALE
			if (!lc->_W_thousands_sep)
				outfn(ptr, currlen, maxlen, L',');
			else
			{
				for (pwz = lc->_W_thousands_sep; *pwz; pwz++)
					outfn(ptr, currlen, maxlen, *pwz);
			}
#else
			outfn(ptr, currlen, maxlen, L',');
#endif
		}
	}

	if (dotpoint)
	{
#if USE_WIDE_LOCALE
		if (!lc->_W_decimal_point)
			outfn(ptr, currlen, maxlen, L'.');
		else
		{
			for (pwz = lc->_W_decimal_point; *pwz; pwz++)
				outfn(ptr, currlen, maxlen, *pwz);
		}
#else
		outfn(ptr, currlen, maxlen, L'.');
#endif
	}

	while (zleadfrac > 0)
	{
		outfn(ptr, currlen, maxlen, '0');
		--zleadfrac;
	}

	while (fplace > omitcount)
		outfn(ptr, currlen, maxlen, fconvert[--fplace]);

	while (eplace > 0)
		outfn(ptr, currlen, maxlen, econvert[--eplace]);

	while (padlen < 0)
	{
		outfn(ptr, currlen, maxlen, ' ');
		++padlen;
	}
}

// 여기가 진짜 asc용 처리
size_t doprw(_sn_outfunc outfn, _sn_clfunc clfn, pointer_t ptr, size_t maxlen, const wchar_t* format, va_list args)
{
	wchar_t ch;
	int min;
	int max;
	int state;
	int flags;
	int cflags;
	int base;
	size_t currlen;

	_sn_anyval value;
	wchar_t tmp[1024];
	char stmp[3];

	state = DP_S_DEFAULT;
	currlen = flags = cflags = min = base = 0;
	max = -1;
	ch = *format++;

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
					outfn(ptr, &currlen, maxlen, ch);

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
					min = 10 * min + wch_to_int(ch);
					ch = *format++;
				}
				else if (ch == L'*')
				{
					min = va_arg(args, int);

					if (min < 0)
					{
						flags |= DP_F_MINUS;
						min = -min;
					}

					ch = *format++;
					state = DP_S_DOT;
				}
				else
				{
					state = DP_S_DOT;
				}

				break;

			case DP_S_DOT:
				if (ch == L'.')
				{
					state = DP_S_MAX;
					ch = *format++;
				}
				else
				{
					state = DP_S_MOD;
				}

				break;

			case DP_S_MAX:
				if (iswdigit(ch))
				{
					if (max < 0)
						max = 0;

					max = 10 * max + wch_to_int(ch);
					ch = *format++;
				}
				else if (ch == L'*')
				{
					max = va_arg(args, int);

					if (max < 0)
						max = -1;

					ch = *format++;
					state = DP_S_MOD;
				}
				else
				{
					if (max < 0)
						max = 0;

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
						{
							cflags = DP_C_SHORT;
						}

						break;

					case L'l':
						ch = *format++;

						if (ch == L'l')     // long long, int64_t, 64비트 정수
						{
							cflags = DP_C_LLONG;
							ch = *format++;
						}
						else
						{
							cflags = DP_C_LONG;
						}

						break;

					case L'L':              // long double, 지원 안함. 또는 size_t
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
					case L'i':      // 부호있는 십진수
						switch (cflags)
						{
							case DP_C_SHORT:
								value.i = (short)va_arg(args, int);
								_wcs_fmt_int(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_LONG:
#if !_QN_WINDOWS_
								value.l = va_arg(args, long);
								_wcs_fmt_long(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
#else
								value.i = va_arg(args, int);
								_wcs_fmt_int(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
#endif
								break;

							case DP_C_LLONG:
								value.l = va_arg(args, long long);
								_wcs_fmt_long(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								value.sz = va_arg(args, size_t);
								_wcs_fmt_size(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_INTMAX:   // 64비트
								value.im = va_arg(args, intmax_t);
								_wcs_fmt_long(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_PTRDIFF:
								value.ff = va_arg(args, ptrdiff_t);
								_wcs_fmt_size(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							case DP_C_CHAR:
								value.i = (char)va_arg(args, int);
								_wcs_fmt_int(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;

							default:
								value.i = va_arg(args, int);
								_wcs_fmt_int(outfn, ptr, &currlen, maxlen, &value, 10, min, max, flags);
								break;
						}

						break;

					case L'X':       // 대문자 부호없는 16진수
						flags |= DP_F_UP;

					case L'x':       // 소문자 부호없는 16진수
						base = 16;

					case L'o':       // 부호없는 8진수
						if (base == 0)
							base = 8;

					case L'b':       // 부호없는 2진수
						if (base == 0)
							base = 2;

					case L'u':       // 부호없는 십진수
						if (base == 0)
							base = 10;

						flags |= DP_F_UNSIGNED;

						switch (cflags)
						{
							case DP_C_SHORT:
								value.ui = (unsigned short)va_arg(args, unsigned int);
								_wcs_fmt_int(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_LONG:
#if !_QN_WINDOWS_
								value.ul = va_arg(args, unsigned long);
								_wcs_fmt_long(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
#else
								value.ui = va_arg(args, unsigned int);
								_wcs_fmt_int(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
#endif
								break;

							case DP_C_LLONG:
								value.ul = va_arg(args, unsigned long long);
								_wcs_fmt_long(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_LDOUBLE:
							case DP_C_SIZE:
								value.sz = va_arg(args, size_t);
								_wcs_fmt_size(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_INTMAX:   // 64비트
								value.im = va_arg(args, intmax_t);
								_wcs_fmt_long(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_PTRDIFF:
								value.ff = va_arg(args, ptrdiff_t);
								_wcs_fmt_size(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							case DP_C_CHAR:
								value.ui = (unsigned char)va_arg(args, unsigned int);
								_wcs_fmt_int(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;

							default:
								value.ui = va_arg(args, unsigned int);
								_wcs_fmt_int(outfn, ptr, &currlen, maxlen, &value, base, min, max, flags);
								break;
						}

						break;

					case L'A':       // 실수의 대문자 16진수 표시 (미구현 '%F'로)

					case L'F':       // 실수 대문자
						flags |= DP_F_UP;

					case L'a':       // 실수의 소문자 16진수 표시 (미구현 '%f'로)

					case L'f':       // 실수
						if (cflags == DP_C_LDOUBLE)
						{
#if 0
							// long double은 보통 지원 안함...
							value.ld = va_arg(args, long double);
							_wcs_fmt_fplong(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#else
							value.d = (double)va_arg(args, long double);
							_wcs_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#endif
						}
						else
						{
							value.d = va_arg(args, double);
							_wcs_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
						}

						break;

					case L'E':      // 지수 표시 (대문자)
						flags |= DP_F_UP;

					case L'e':      // 지수 표시
						flags |= DP_F_E;

						if (cflags == DP_C_LDOUBLE)
						{
#if 0
							value.ld = va_arg(args, long double);
							_wcs_fmt_fplong(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#else
							value.d = (double)va_arg(args, long double);
							_wcs_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#endif
						}
						else
						{
							value.d = va_arg(args, double);
							_wcs_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
						}

						break;

					case L'G':      // 지수 표시 (대문자)
						flags |= DP_F_UP;

					case L'g':      // 지수 표시
						flags |= DP_F_G;

						if (max == 0) // C99
							max = 1;

						if (cflags == DP_C_LDOUBLE)
						{
#if 0
							value.ld = va_arg(args, long double);
							_wcs_fmt_fplong(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#else
							value.d = (double)va_arg(args, long double);
							_wcs_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
#endif
						}
						else
						{
							value.d = va_arg(args, double);
							_wcs_fmt_fp(outfn, ptr, &currlen, maxlen, &value, min, max, flags);
						}

						break;

					case L'c':
						if (cflags != DP_C_SHORT)
							outfn(ptr, &currlen, maxlen, va_arg(args, int));        // 이거 wchar_t로 해야하나?
						else
						{
							stmp[0] = (char)va_arg(args, int);
							stmp[1] = '\0';
							max = (int)pp_getuni(tmp, 30, stmp);
							_wcs_fmt_wcs(outfn, ptr, &currlen, maxlen, tmp, 0, 0, max);
						}

						break;

					case L'C':
						if (cflags != DP_C_SHORT)
							outfn(ptr, &currlen, maxlen, va_arg(args, int));        // 이것도
						else
						{
							stmp[0] = (char)va_arg(args, int);
							stmp[1] = '\0';
							max = (int)pp_getuni(tmp, 30, stmp);
							_wcs_fmt_wcs(outfn, ptr, &currlen, maxlen, tmp, 0, 0, max);
						}

						break;

					case L's':
						if (cflags != DP_C_SHORT)
						{
							value.w = va_arg(args, wchar_t*);

							if (!value.w)
								value.w = L"(null)";

							if (max == -1)
								max = (int)wcslen(value.w);

							if (min > 0 && max >= 0 && min > max)
								max = min;

							_wcs_fmt_wcs(outfn, ptr, &currlen, maxlen, value.w, flags, min, max);
						}
						else
						{
							value.s = va_arg(args, char*);

							if (!value.s)
							{
								value.w = L"(null)";

								if (max == -1)
									max = 6;
							}
							else
							{
								pp_getuni(tmp, 1024 - 1, value.s);
								value.w = tmp;

								if (max == -1)
									max = (int)wcslen(value.w);
							}

							if (min > 0 && max >= 0 && min > max)
								max = min;

							_wcs_fmt_wcs(outfn, ptr, &currlen, maxlen, value.w, flags, min, max);
						}

						break;

					case L'S':
						if (cflags != DP_C_SHORT)
						{
							value.w = va_arg(args, wchar_t*);

							if (!value.w)
								value.w = L"(null)";

							if (max == -1)
								max = (int)wcslen(value.w);

							if (min > 0 && max >= 0 && min > max)
								max = min;

							_wcs_fmt_wcs(outfn, ptr, &currlen, maxlen, value.w, flags, min, max);
						}
						else
						{
							value.s = va_arg(args, char*);

							if (!value.s)
							{
								value.w = L"(null)";

								if (max == -1)
									max = 6;
							}
							else
							{
								pp_getuni(tmp, 1024 - 1, value.s);
								value.w = tmp;

								if (max == -1)
									max = (int)wcslen(value.w);
							}

							if (min > 0 && max >= 0 && min > max)
								max = min;

							_wcs_fmt_wcs(outfn, ptr, &currlen, maxlen, value.w, flags, min, max);
						}

						break;

					case L'p':
						flags |= DP_F_UNSIGNED | DP_F_UP;
						value.s = va_arg(args, char*);
						_wcs_fmt_size(outfn, ptr, &currlen, maxlen, &value, 16, min, max, flags);
						break;

					case L'n':
						switch (cflags)
						{
							case DP_C_SHORT:
								value.ph = va_arg(args, short*);
								*value.ph = (short)currlen;
								break;

							case DP_C_LONG:
#if !_QN_WINDOWS_
								value.pl = va_arg(args, long*);
								*value.pl = (long)currlen;
#else
								value.pi = va_arg(args, int*);
								*value.pi = (int)currlen;
#endif
								break;

							case DP_C_LLONG:
								value.pl = va_arg(args, long long*);
								*value.pl = (long long)currlen;
								break;

#if 0

							case DP_C_LDOUBLE:  // 기능 없음
								break;
#endif

							case DP_C_SIZE:     // C99에서 원래 부호 있는 크기(ssize_t)를 요구함
								value.psz = va_arg(args, size_t*);
								*value.psz = (size_t)currlen;
								break;

							case DP_C_INTMAX:   // 64비트
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
						outfn(ptr, &currlen, maxlen, ch);
						break;

					default:
						// 알수없음, 넘김
						break;
				}   // switch - ch

				ch = *format++;
				state = DP_S_DEFAULT;
				flags = cflags = min = base = 0;
				max = -1;
				break;

			case DP_S_DONE:
				break;

			default:	// 음?
				break; // 일부 엉뚱한 컴파일러는 break를 요구함
		}   // switch - state
	}   // while

	if (clfn && maxlen != 0)
		clfn(ptr, currlen, maxlen);

	return currlen;
}
