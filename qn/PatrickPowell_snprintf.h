#pragma once

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
*    * ht_snprintf/ht_vsnprintf return number of characters actually
*      written instead of the number of characters that would
*      have been written.
*    * added '%y' to allow object output using Object's toString() method.
*    * added '%q[dioux]' for formatting qwords.
*    * added '%b' for formatting in binary notation.
*
* hbcs version
*    * add '%S', '%C', '%ls', '%lc', '%hS', '%hC' for unicode
*    * add '%LnioubxX' in integer for size_t
*    * remove '%y'
*    * more 'C99' compatibility
*    * file output
**************************************************************/

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
#define DP_F_MINUS      (1 << 0)
#define DP_F_PLUS       (1 << 1)
#define DP_F_SPACE      (1 << 2)
#define DP_F_NUM        (1 << 3)
#define DP_F_ZERO       (1 << 4)
#define DP_F_UP         (1 << 5)
#define DP_F_UNSIGNED   (1 << 6)
#define DP_F_QUOTE		(1 << 7)
#define DP_F_G			(1 << 8)
#define DP_F_E			(1 << 9)

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
typedef void(*_sn_outfunc)(pointer_t, size_t*, size_t, int);
typedef void(*_sn_clfunc)(pointer_t, size_t, size_t);

//
typedef union _sn_anyval
{
	int16_t				h;
	int32_t				i;
	int64_t				l;

	uint16_t			uh;
	uint32_t			ui;
	uint64_t			ul;

	double				d;

	char*				s;
	wchar_t*			w;

	// hb only
	size_t				sz;

	// C99
	long double			ld;
	intmax_t			im;
	ptrdiff_t			ff;

	//
	int16_t*			ph;
	int32_t*			pi;
	int64_t*			pl;
	size_t*				psz;
	intmax_t*			pim;
	ptrdiff_t*			pff;
} _sn_anyval;

//
extern size_t dopr(_sn_outfunc outfn, _sn_clfunc clfn, pointer_t ptr, size_t maxlen, const char* format, va_list args);
extern size_t doprw(_sn_outfunc outfn, _sn_clfunc clfn, pointer_t ptr, size_t maxlen, const wchar_t* format, va_list args);

#if !_QN_ANDROID_ && !_QN_IOS_		// [2014-10-15 ksh] 안드로이드랑 IOS는 로캘이 안됨
#define USE_LOCALE_INFO	1
#else
#define USE_LOCALE_INFO	0
#endif

#if _MSC_VER						// [2014-12-15 ksh] 윈도우만 유니코드 로캘 지원
#define USE_WIDE_LOCALE	1
#else
#define USE_WIDE_LOCALE	0
#endif
