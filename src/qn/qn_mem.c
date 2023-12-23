#include "pch.h"
#include "qs_qn.h"
#ifndef __EMSCRIPTEN__
#include "zlib/zlib.h"
#endif

//
void* qn_memenc(void* restrict dest, const void* restrict src, size_t size)
{
	const byte* ps = (const byte*)src;
	byte* pd = (byte*)dest;

	for (; size; --size, ++pd, ++ps)
	{
		const byte z = (byte)(255 - *ps);
		*pd = (byte)(z >> 4 | (z & 0xF) << 4);
	}

	return dest;
}

//
void* qn_memdec(void* restrict dest, const void* restrict src, size_t size)
{
	const byte* ps = (const byte*)src;
	byte* pd = (byte*)dest;

	for (; size; --size, ++pd, ++ps)
	{
		const byte z = (byte)(255 - *ps);
		*pd = (byte)(z << 4 | (z & 0xF0) >> 4);
	}

	return dest;
}

//
void* qn_memzcpr(const void* src, size_t srcsize, /*NULLABLE*/size_t* destsize)
{
#ifndef __EMSCRIPTEN__
	qn_val_if_fail(src != NULL, NULL);
	qn_val_if_fail(srcsize > 0, NULL);

	uLong tmp = ((uLong)srcsize + 12) / 1000;
	tmp += tmp == 0 ? (uLong)srcsize + 13 : (uLong)srcsize + 12;

	byte* p = qn_alloc(tmp, byte);
	if (compress((Bytef*)p, &tmp, (const Bytef*)src, (uLong)srcsize) != Z_OK)
	{
		qn_free(p);
		return NULL;
	}

	if (destsize)
		*destsize = tmp;

	return p;
#else
	return NULL;
#endif
}

//
void* qn_memzucp(const void* src, size_t srcsize, size_t bufsize, /*NULLABLE*/size_t* destsize)
{
#ifndef __EMSCRIPTEN__
	qn_val_if_fail(src != NULL, NULL);
	qn_val_if_fail(srcsize > 0, NULL);

	uLong size = bufsize == 0 ? (uLong)srcsize * 5 + 12 : (uLong)bufsize;
	byte* p = qn_alloc(size, byte);
	if (uncompress((Bytef*)p, &size, (const Bytef*)src, (uLong)srcsize) != Z_OK)
	{
		qn_free(p);
		return NULL;
	}

	if (destsize)
		*destsize = size;

	return p;
#else
	return NULL;
#endif
}

//
size_t qn_memagn(size_t size)
{
	const size_t align = size % 16;
	if (align == 0)
		return 0;
	const size_t advice = (size + 16 - 1) & (size_t)~(16 - 1);
	return advice;
}

//
char qn_memhrb(size_t size, double* out)
{
	qn_val_if_fail(out != NULL, ' ');
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
char* qn_memdmp(const void* restrict ptr, size_t size, char* restrict outbuf, size_t buflen)
{
	qn_val_if_fail(ptr != NULL, NULL);
	qn_val_if_fail(outbuf != NULL, NULL);

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
		*ind = (m < 0x20 || m>0x7F) ? '.' : (const char)m;  // NOLINT
		mem++;
		ind++;
	}
	*ind = '\0';

	return outbuf;
}


//////////////////////////////////////////////////////////////////////////
// 메모리 처리

#define MEMORY_GAP				4
#define MEMORY_BLOCK_SIZE		16
#define MEMORY_PAGE				256
#define MEMORY_PAGE_SIZE		(MEMORY_BLOCK_SIZE*MEMORY_PAGE)
#define MEMORY_SIGN_HEAD		('Q' | 'M'<<8 | 'B'<<16 | '\0'<<24)
#define MEMORY_SIGN_FREE		('B' | 'A'<<8 | 'D'<<16 | '\0'<<24)

//
typedef struct MemBlock
{
	uint				sign;

	uint				line;
	const char*			desc;

	size_t				index;

	size_t				size;
	size_t				block;

	struct MemBlock*	next;
	struct MemBlock*	prev;
} MemBlock;

#define _memhdr(ptr)			(((MemBlock*)(ptr))-1)
#define _memptr(block)			(void*)(((MemBlock*)(block))+1)
#define _memsize(size)			(((sizeof(MemBlock)+(size)+MEMORY_GAP)+MEMORY_BLOCK_SIZE-1)&(size_t)~(MEMORY_BLOCK_SIZE-1))

// 메모리 프르프
static struct MemImpl
{
	QnAllocTable	table;

	MemBlock*		frst;
	MemBlock*		last;
	size_t			index;
	size_t			count;
	size_t			block_size;

#ifdef _QN_WINDOWS_
	HANDLE			heap;
#endif

#ifndef _QN_NO_THREAD_
	QnSpinLock		lock;
#endif
} mem_impl = { {qn_mpfalloc, qn_mpffree}, };

#ifndef _QN_NO_THREAD_
#define MP_LOCK				qn_spin_enter(&mem_impl.lock)
#define MP_UNLOCK			qn_spin_leave(&mem_impl.lock)
#else
#define MP_LOCK
#define MP_UNLOCK
#endif

void qn_mpf_init(void)
{
#if _QN_WINDOWS_
	mem_impl.heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS/*|HEAP_NO_SERIALIZE*/, MEMORY_BLOCK_SIZE, 0);
	ULONG hfv = 2;
	HeapSetInformation(mem_impl.heap, HeapCompatibilityInformation, &hfv, sizeof(ULONG));
#endif
}

static void qn_mpf_clear(void)
{
	if (mem_impl.count == 0 && mem_impl.frst == NULL && mem_impl.last == NULL)
		return;

#ifndef __EMSCRIPTEN__
	qn_debug_outputf(false, "MEMORY PROFILER", "found %d allocations", mem_impl.count);
#endif

	size_t sum = 0;
	for (MemBlock* next = NULL, *node = mem_impl.frst; node; node = next)
	{
#ifndef __EMSCRIPTEN__
		if (node->line)
			qn_debug_outputf(false, "MEMORY PROFILER", "\t%s(%Lu) : %Lu(%Lu) : 0x%p", node->desc, node->line, node->size, node->block, _memptr(node));
		else
			qn_debug_outputf(false, "MEMORY PROFILER", "\t%Lu(%Lu) : 0x%p", node->size, node->block, _memptr(node));
		char sz[64];
		qn_memdmp(_memptr(node), QN_MIN(32, node->size), sz, 64 - 1);
		qn_debug_outputf(false, "MEMORY PROFILER", "\t\t{%s}", sz);
#endif
		next = node->next;
		sum += node->block;

#if _QN_WINDOWS_
		HeapFree(mem_impl.heap, 0, node);
#else
		free(node);
#endif
	}

	double size;
	const char usage = qn_memhrb(sum, &size);
	if (usage == ' ')
		qn_debug_outputf(true, "MEMORY PROFILER", "total block size: %Lu bytes", sum);
	else
		qn_debug_outputf(true, "MEMORY PROFILER", "total block size: %.2f %cbytes", size, usage);
}

void qn_mpf_dispose(void)
{
	qn_mpf_clear();

#if _QN_WINDOWS_
	if (mem_impl.heap)
	{
		SIZE_T s;
		if (HeapQueryInformation(mem_impl.heap, HeapEnableTerminationOnCorruption, NULL, 0, &s))
			qn_debug_outputf(true, "MEMORY PROFILER", "heap allocation left: %d", (int)s);
		HeapDestroy(mem_impl.heap);
	}
#endif
}

static void qn_mpf_node_link(MemBlock* node)
{
	MP_LOCK;
	if (mem_impl.frst)
		mem_impl.frst->prev = node;
	else
		mem_impl.last = node;
	node->next = mem_impl.frst;
	node->prev = NULL;

	mem_impl.frst = node;
	mem_impl.count++;
	mem_impl.index++;
	mem_impl.block_size += node->block;
	MP_UNLOCK;
}

static void qn_mpf_node_unlink(const MemBlock* node)
{
	MP_LOCK;
	if (node->next)
		node->next->prev = node->prev;
	else
		mem_impl.last = node->prev;
	if (node->prev)
		node->prev->next = node->next;
	else
		mem_impl.frst = node->next;
	mem_impl.count--;
	mem_impl.block_size -= node->block;
	MP_UNLOCK;
}

//
size_t qn_mpfsize(void)
{
	return mem_impl.block_size;
}

//
size_t qn_mpfcnt(void)
{
	return mem_impl.count;
}

#ifdef _QN_WINDOWS_
//
static DWORD qn_windows_mpf_exception(DWORD ex, const char* desc, size_t line, size_t size, size_t block)
{
	char sz[1024];
	if (ex == STATUS_NO_MEMORY)
		qn_snprintf(sz, QN_COUNTOF(sz) - 1, "out of memory : %s(%Lu) : %Lu(%Lu)", desc, line, size, block);
	else if (ex == STATUS_ACCESS_VIOLATION)
		qn_snprintf(sz, QN_COUNTOF(sz) - 1, "access violation : %s(%Lu) : %Lu(%Lu)", desc, line, size, block);
	else
		return EXCEPTION_CONTINUE_SEARCH;
	qn_debug_halt("MEMORY PROFILER", sz);
	//return EXCEPTION_EXECUTE_HANDLER;
}
#endif

//
static void qn_mpf_out_of_memory(const char* desc, size_t line, size_t size, size_t block)
{
	char sz[1024];
	qn_snprintf(sz, QN_COUNTOF(sz) - 1, "out of memory : %s(%Lu) : %Lu(%Lu)", desc, line, size, block);
	qn_debug_halt("MEMORY PROFILER", sz);
}

//
static void* qn_mpf_alloc(size_t size, bool zero, const char* desc, size_t line)
{
	size_t block = _memsize(size);
	MemBlock* node;

#ifdef _QN_WINDOWS_
	__try
	{
		node = (MemBlock*)HeapAlloc(mem_impl.heap, zero ? HEAP_ZERO_MEMORY : 0, block);
	}
	__except (qn_windows_mpf_exception(_exception_code(), desc, line, size, block))
	{
		node = NULL;
	}
#else
	node = (MemBlock*)(zero ? calloc(block, 1) : malloc(block));
#endif
	if (node == NULL)
		qn_mpf_out_of_memory(desc, line, size, block);
	else
	{
		node->sign = MEMORY_SIGN_HEAD;
		node->desc = desc;
		node->line = (uint)line;
		node->index = mem_impl.index;
		node->size = size;
		node->block = block;
		qn_mpf_node_link(node);
	}

	return _memptr(node);
}

//
static void* qn_mpf_realloc(void* ptr, size_t size, const char* desc, size_t line)
{
	MemBlock* node = _memhdr(ptr);
	if (node == NULL)
	{
		qn_debug_outputf(true, "MEMORY PROFILER", "try to realloc null memory node : 0x%p", ptr);
		return NULL;
	}
#if _QN_WINDOWS_
	if (HeapValidate(mem_impl.heap, 0, node) == 0 || node->sign != MEMORY_SIGN_HEAD)
	{
		qn_debug_outputf(false, "MEMORY PROFILER", "try to realloc invalid memory : 0x%p", ptr);
		char sz[260];
		qn_memdmp(ptr, 19, sz, 260 - 1);
		qn_debug_outputf(true, "MEMORY PROFILER", "\t\t{%s}", sz);
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
	qn_mpf_node_unlink(node);

#if _QN_WINDOWS_
	__try
	{
		node = (MemBlock*)HeapReAlloc(mem_impl.heap, 0, node, block);
	}
	__except (qn_windows_mpf_exception(_exception_code(), desc, line, size, block))
	{
		node = NULL;
	}
#else
	node = (MemBlock*)realloc(node, block);
#endif
	if (node == NULL)
		qn_mpf_out_of_memory(desc, line, size, block);
	else
	{
		node->desc = desc;
		node->line = (uint)line;
		node->size = size;
		node->block = block;
		qn_mpf_node_link(node);
	}

	return _memptr(node);
}

//
void* qn_mpfalloc(void* ptr, size_t size, bool zero, const char* desc, size_t line)
{
	if (ptr == NULL && size != 0)
		return qn_mpf_alloc(size, zero, desc, line);
	if (size != 0)
		return qn_mpf_realloc(ptr, size, desc, line);
	qn_mpffree(ptr);
	return NULL;
}

//
void qn_mpffree(void* ptr)
{
	qn_ret_if_fail(ptr);

	MemBlock* node = _memhdr(ptr);
	if (node == NULL)
	{
		qn_debug_outputf(true, "MEMORY PROFILER", "try to free null memory node : 0x%p", ptr);
		return;
	}
#if _QN_WINDOWS_
	if (HeapValidate(mem_impl.heap, 0, node) == 0 || node->sign != MEMORY_SIGN_HEAD)
	{
		qn_debug_outputf(false, "MEMORY PROFILER", "try to free invalid memory : 0x%p", ptr);
		char sz[260];
		qn_memdmp(ptr, 19, sz, 260 - 1);
		qn_debug_outputf(true, "MEMORY PROFILER", "\t\t{%s}", sz);
		return;
	}
#endif

	node->sign = MEMORY_SIGN_FREE;
	qn_mpf_node_unlink(node);

#if _QN_WINDOWS_
	HeapFree(mem_impl.heap, 0, node);
#else
	free(node);
#endif
}

//
void qn_debug_mpfprint(void)
{
#ifndef __EMSCRIPTEN__
	if (mem_impl.count == 0 && mem_impl.frst == NULL && mem_impl.last == NULL)
		return;

	qn_debug_outputf(false, "MEMORY PROFILER", "found %d allocations", mem_impl.count);
	qn_debug_outputf(false, "MEMORY PROFILER", " %-8s | %-8s | %-8s | %-s", "no", "size", "block", "desc");

	MP_LOCK;
	size_t sum = 0, cnt = 1;
	for (MemBlock* next = NULL, *node = mem_impl.frst; node; node = next, cnt++)
	{
		if (node->line)
		{
			qn_debug_outputf(false, "MEMORY PROFILER", " %-8d | %-8zu | % -8zu | \"%s:%d\"",
				cnt, node->size, node->block, node->desc, (int)node->line);
		}
		else
		{
			char sz[64];
			qn_memdmp(_memptr(node), QN_MIN(32, node->size), sz, 64 - 1);
			qn_debug_outputf(false, "MEMORY PROFILER", " %-8d | %-8zu | % -8zu | <%s>",
				cnt, node->size, node->block, sz);
		}
		next = node->next;
		sum += node->block;
	}
	MP_UNLOCK;

	double size;
	const char usage = qn_memhrb(sum, &size);
	qn_debug_outputf(false, "MEMORY PROFILER", "block size: %.3g%cbytes", size, usage);
#endif
}

//
bool qn_memtbl(const QnAllocTable* table)
{
	if (table == NULL ||
		table->_alloc == NULL ||
		table->_free == NULL)
		return false;
	mem_impl.table = *table;
	return true;
}

//
void* qn_memalc(void* ptr, size_t size, bool zero, const char* desc, size_t line)
{
	return mem_impl.table._alloc(ptr, size, zero, desc, line);
}

//
void qn_memfre(void* ptr)
{
	mem_impl.table._free(ptr);
}
