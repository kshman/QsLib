#include "pch.h"
#include "qn.h"
#ifdef _MSC_VER
#include "zlib/zlib.h"
#else
#include <zlib.h>
#endif
#include "qn_mem.h"

//
void* qn_memenc(void* dest, const void* src, size_t size)
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
void* qn_memdec(void* dest, const void* src, size_t size)
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
void* qn_memzcpr(const void* src, size_t srcsize, /*NULLABLE*/size_t* destsize)
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
void* qn_memzucp(const void* src, size_t srcsize, size_t bufsize, /*NULLABLE*/size_t* destsize)
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
char* qn_memdmp(const void* ptr, size_t size, char* outbuf, size_t buflen)
{
	qn_retval_if_fail(ptr != NULL, NULL);
	qn_retval_if_fail(outbuf != NULL, NULL);

	if (size == 0 || buflen == 0)
	{
		*outbuf = '\0';
		return outbuf;
	}

	const uint8_t* mem = (const uint8_t*)ptr;
	char* ind = outbuf;
	for (size_t i = 0, cnt = 1; i < size && cnt < buflen; i++, cnt++)
	{
		uint8_t m = *mem;
		*ind = (m < 0x20 || m>0x7F) ? '.' : (char)m;
		mem++;
		ind++;
	}
	*ind = '\0';

	return outbuf;
}


//////////////////////////////////////////////////////////////////////////
// 메모리 처리

static struct qnMemProof
{
	memBlock* frst;
	memBlock* last;
	size_t			index;
	size_t			count;
	size_t			block_size;

#if _QN_WINDOWS_
	HANDLE			heap;
#endif

#if USE_MP_LOCK
	mtx_t			lock;
#endif
} _qn_mp = { NULL, };

#if USE_MP_LOCK
#define MP_LOCK				mtx_lock(&_qn_mp.lock)
#define MP_UNLOCK			mtx_unlock(&_qn_mp.lock)
#else
#define MP_LOCK
#define MP_UNLOCK
#endif

void _qn_mp_init(void)
{
#if USE_MP_LOCK
	mtx_init(&_qn_mp.lock, mtx_plaine | mtx_recursive);
#endif

#if _QN_WINDOWS_
	_qn_mp.heap = HeapCreate(0/*|HEAP_NO_SERIALIZE*/, MEMORY_BLOCK_SIZE, 0);
	ULONG hfv = 2;
	HeapSetInformation(_qn_mp.heap, HeapCompatibilityInformation, &hfv, sizeof(ULONG));
#endif
}

static void _qn_mp_clear(void)
{
	if (_qn_mp.count == 0 && _qn_mp.frst == NULL && _qn_mp.last == NULL)
		return;

	qn_debug_outputf(false, "Memory Profiler", "found %d allocations", _qn_mp.count);

	size_t sum = 0;
	for (memBlock* next = NULL, *node = _qn_mp.frst; node; node = next)
	{
		if (node->line)
			qn_debug_outputf(false, "Memory Profiler", "\t%s(%Lu) : %Lu(%Lu) : 0x%p", node->desc, node->line, node->size, node->block, _memptr(node));
		else
			qn_debug_outputf(false, "Memory Profiler", "\t%Lu(%Lu) : 0x%p", node->size, node->block, _memptr(node));
		char sz[64];
		qn_memdmp(_memptr(node), QN_MIN(32, node->size), sz, 64 - 1);
		qn_debug_outputf(false, "Memory Profiler", "\t\t{%s}", sz);
		next = node->next;
		sum += node->block;

#if _QN_WINDOWS_
		HeapFree(_qn_mp.heap, 0, node);
#else
		free(node);
#endif
	}

	double size;
	char usage = qn_memhrd(sum, &size);
	if (usage == ' ')
		qn_debug_outputf(true, "Memory Profiler", "total block size: %Lu bytes", sum);
	else
		qn_debug_outputf(true, "Memory Profiler", "total block size: %.2f %cbytes", size, usage);
}

void _qn_mp_dispose(void)
{
	_qn_mp_clear();

#if _QN_WINDOWS_
	if (_qn_mp.heap)
	{
		SIZE_T s;
		if (HeapQueryInformation(_qn_mp.heap, HeapEnableTerminationOnCorruption, NULL, 0, &s))
			qn_debug_outputf(true, "Memory Profiler", "heap allocation left: %d", (int)s);
		HeapDestroy(_qn_mp.heap);
	}
#endif

#if USE_MP_LOCK
	mtx_destroy(&_qn_mp.lock);
#endif
}

static void _qn_mp_add(memBlock* node)
{
	MP_LOCK;
	if (_qn_mp.frst)
		_qn_mp.frst->prev = node;
	else
		_qn_mp.last = node;
	node->next = _qn_mp.frst;
	node->prev = NULL;

	_qn_mp.frst = node;
	_qn_mp.count++;
	_qn_mp.index++;
	_qn_mp.block_size += node->block;
	MP_UNLOCK;
}

static void _qn_mp_del(memBlock* node)
{
	MP_LOCK;
	if (node->next)
		node->next->prev = node->prev;
	else
		_qn_mp.last = node->prev;
	if (node->prev)
		node->prev->next = node->next;
	else
		_qn_mp.frst = node->next;
	_qn_mp.count--;
	_qn_mp.block_size -= node->block;
	MP_UNLOCK;
}

//
size_t qn_mpfsize(void)
{
	return _qn_mp.block_size;
}

//
size_t qn_mpfcnt(void)
{
	return _qn_mp.count;
}

//
void* qn_mpfalloc(size_t size, bool zero, const char* desc, size_t line)
{
	qn_retval_if_fail(size, NULL);

	size_t block = _memsize(size);
	memBlock* node = (memBlock*)
#if _QN_WINDOWS_
		HeapAlloc(_qn_mp.heap, zero ? HEAP_ZERO_MEMORY : 0, block);
#else
		(zero ? calloc(block, 1) : malloc(block));
#endif
	if (!node)
	{
		qn_debug_outputf(false, "Memory Profiler", "cannot allocate memory : %s(%Lu) : %Lu(%Lu)", desc, line, size, block);
		return NULL;
	}

	node->sign = MEMORY_SIGN_HEAD;
#if _QN_64_
	node->align64 = 0;
#endif
	node->desc = desc;
	node->line = line;
	node->index = _qn_mp.index;
	node->size = size;
	node->block = block;
	_qn_mp_add(node);

	return _memptr(node);
}

//
void* qn_mpfreloc(void* ptr, size_t size, const char* desc, size_t line)
{
	if (!ptr)
		return qn_mpfalloc(size, false, desc, line);
	if (size == 0)
	{
		qn_mpffree(ptr);
		return NULL;
	}

	memBlock* node = _memhdr(ptr);
	if (node == NULL)
	{
		qn_debug_outputf(true, "Memory Profiler", "try to realloc null memory node : 0x%p", ptr);
		return NULL;
	}
#if _QN_WINDOWS_
	if (HeapValidate(_qn_mp.heap, 0, node) == 0 || node->sign != MEMORY_SIGN_HEAD)
	{
		qn_debug_outputf(false, "Memory Profiler", "try to realloc invalid memory : 0x%p", ptr);
		char sz[260];
		qn_memdmp(ptr, 19, sz, 260 - 1);
		qn_debug_outputf(true, "Memory Profiler", "\t\t{%s}", sz);
		return NULL;
	}
#endif

	size_t block = _memsize(size);
	if (block <= node->block)
	{
		// 블락이 같으면 그냥 써. 근데 이럴일이 없을껄
		node->size = size;
		return ptr;
	}

	// 재할당 하거나 새 메모리
	node = (memBlock*)
#if _QN_WINDOWS_
		HeapReAlloc(_qn_mp.heap, 0, node, block);
#else
		realloc(node, block);
#endif
		if (!node)
		{
			qn_debug_outputf(false, "Memory Profiler", "cannot reallocate memory : %s(%Lu) : %Lu(%Lu)", desc, line, size, block);
			return NULL;
		}

		node->desc = desc;
		node->line = line;
		node->size = size;
		node->block = block;

		return _memptr(node);
}

//
void qn_mpffree(void* ptr)
{
	qn_ret_if_fail(ptr);

	memBlock* node = _memhdr(ptr);
	if (node == NULL)
	{
		qn_debug_outputf(true, "Memory Profiler", "try to free null memory node : 0x%p", ptr);
		return;
	}
#if _QN_WINDOWS_
	if (HeapValidate(_qn_mp.heap, 0, node) == 0 || node->sign != MEMORY_SIGN_HEAD)
	{
		qn_debug_outputf(false, "Memory Profiler", "try to free invalid memory : 0x%p", ptr);
		char sz[260];
		qn_memdmp(ptr, 19, sz, 260 - 1);
		qn_debug_outputf(true, "Memory Profiler", "\t\t{%s}", sz);
		return;
	}
#endif

	node->sign = MEMORY_SIGN_FREE;
	_qn_mp_del(node);

#if _QN_WINDOWS_
	HeapFree(_qn_mp.heap, 0, node);
#else
	free(node);
#endif
}
