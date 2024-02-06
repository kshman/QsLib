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
*    remove file output
**************************************************************/

#pragma once

//
typedef union PATRICKPOWELL_SNPRINTF_ANY
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
} PatrickPowellSprintfAny;

//
typedef struct PATRICKPOWELL_SNPRINTF_STATE  PatrickPowellSprintfState;

struct PATRICKPOWELL_SNPRINTF_STATE
{
	void (*outch)(PatrickPowellSprintfState*, int);
	void (*finish)(PatrickPowellSprintfState*);

#ifndef QS_NO_MEMORY_PROFILE
	const char* desc;
	size_t line;
#endif

	size_t maxlen, currlen;
	nint vmin, vmax;
	nuint base;

	PatrickPowellSprintfAny value;
	union
	{
		char* buf;
		void* ptr;
	};
};

//
extern void dopr(_In_ PatrickPowellSprintfState* state, _In_ const char* format, _In_ va_list args);
