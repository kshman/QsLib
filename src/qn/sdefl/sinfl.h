#pragma once

#ifndef SINFL_H_INCLUDED
#define SINFL_H_INCLUDED

#define SINFL_PRE_TBL_SIZE 128
#define SINFL_LIT_TBL_SIZE 1334
#define SINFL_OFF_TBL_SIZE 402

struct sinfl
{
	const unsigned char *bitptr;
	const unsigned char *bitend;      // @raysan5: added
	unsigned long long bitbuf;
	int bitcnt;

	unsigned lits[SINFL_LIT_TBL_SIZE];
	unsigned dsts[SINFL_OFF_TBL_SIZE];
};

extern int sinflate(void *out, int cap, const void *in, int size);
extern int zsinflate(void *out, int cap, const void *in, int size);

#endif /* SINFL_H_INCLUDED */

