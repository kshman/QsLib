#include "pch.h"
#include "qn.h"
#ifdef _MSC_VER
#include "zlib/zlib.h"
#else
#include <zlib.h>
#endif

//
pointer_t qn_memenc(pointer_t dest, const pointer_t src, size_t size)
{
	uint8_t* pd = (uint8_t*)dest;
	uint8_t* ps = (uint8_t*)src;

	for (; size; --size, ++pd, ++ps)
	{
		uint8_t z = (uint8_t)(255 - *ps);
		*pd = (uint8_t)(z >> 4 | (z & 0xF) << 4);
	}

	return dest;
}

//
pointer_t qn_memdec(pointer_t dest, const pointer_t src, size_t size)
{
	uint8_t* pd = (uint8_t*)dest;
	uint8_t* ps = (uint8_t*)src;

	for (; size; --size, ++pd, ++ps)
	{
		uint8_t z = (uint8_t)(255 - *ps);
		*pd = (uint8_t)(z << 4 | (z & 0xF0) >> 4);
	}

	return dest;
}

//
pointer_t qn_memzcpr(const pointer_t src, size_t srcsize, /*RET_NULLABLE*/size_t* destsize)
{
	qn_retval_if_fail(src != NULL, NULL);
	qn_retval_if_fail(srcsize > 0, NULL);

	uLong tmp = ((uLong)srcsize + 12) / 1000;
	tmp += tmp == 0 ? (uLong)srcsize + 13 : (uLong)srcsize + 12;

	uint8_t* p = qn_alloc(tmp, uint8_t);
	if (compress((Bytef*)p, &tmp, (Bytef*)src, (uLong)srcsize) != Z_OK)
	{
		qn_free(p);
		return NULL;
	}

	if (destsize)
		*destsize = tmp;

	return p;
}

//
pointer_t qn_memzucp(const pointer_t src, size_t srcsize, size_t bufsize, /*RET_NULLABLE*/size_t* destsize)
{
	qn_retval_if_fail(src != NULL, NULL);
	qn_retval_if_fail(srcsize > 0, NULL);

	uLong size = bufsize == 0 ? (uLong)srcsize * 5 + 12 : (uLong)bufsize;
	uint8_t* p = qn_alloc(size, uint8_t);
	if (uncompress((Bytef*)p, &size, (const Bytef*)src, (uLong)srcsize) != Z_OK)
	{
		qn_free(p);
		return NULL;
	}

	if (destsize)
		*destsize = size;

	return p;
}

//
size_t qn_memagn(size_t size)
{
	size_t align = size % 16;
	if (align == 0)
		return 0;
	size_t advice = (size + 16 - 1) & ~(16 - 1);
	return advice;
}

//
char qn_memhrd(size_t size, double* out)
{
	qn_retval_if_fail(out != NULL, ' ');
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
char* qn_memdmp(const pointer_t ptr, size_t size, char* outbuf, size_t buflen)
{
	qn_retval_if_fail(ptr != NULL, NULL);
	qn_retval_if_fail(outbuf != NULL, NULL);

	if (size = 0 || buflen == 0)
	{
		*outbuf = '\0';
		return outbuf;
	}

	const uint8_t* mem = (const uint8_t*)ptr;
	char* ind = outbuf;
	for (size_t i = 0, cnt = 1; i < size && cnt < buflen; i++, cnt++)
	{
		uint8_t m = *mem;
		*ind = (m < 0x20 || m>0x7F) ? ' ' : (char)m;
		mem++;
		ind++;
	}
	*ind = '\0';

	return outbuf;
}
