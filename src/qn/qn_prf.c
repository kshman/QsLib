﻿//
// qn_prf.c - 디버그 출력 관리자 + 메모리 관리자
// 2023-12-27 by kim
//

#include "pch.h"
#ifdef __GNUC__
#include <signal.h>
#endif
#ifdef _QN_EMSCRIPTEN_
#include <emscripten/console.h>
#endif

#ifdef DEBUG_BREAK
#error macro _DEBUG_BREAK already defined!
#endif
#ifdef _QN_WINDOWS_
#define DEBUG_BREAK(x)			if (x) DebugBreak()
#elif defined _QN_EMSCRIPTEN_
#define DEBUG_BREAK(x)
#else
#define DEBUG_BREAK(x)			if (x) raise(SIGTRAP)
#endif

//
static struct DebugImpl
{
#ifdef _QN_WINDOWS_
	HANDLE			handle;
#else
	FILE*			fp;
#endif

	char			tag[32];
	char			out_buf[MAX_DEBUG_LENGTH];
	int				out_pos;

	bool			debugger;
	bool			redirect;
	bool			__dummy1;
	bool			__dummy2;
} debug_impl =
{
#ifdef _QN_WINDOWS_
	.handle = NULL,
#else
	.fp = NULL,
#endif
	.tag = "QS",
	.out_buf = "",
	.out_pos = 0,
	.debugger = false,
	.redirect = false,
};

//
void qn_debug_up(void)
{
#ifdef _QN_WINDOWS_
	debug_impl.handle = GetStdHandle(STD_OUTPUT_HANDLE);
	debug_impl.debugger = IsDebuggerPresent();
#else
	debug_impl.fp = stdout;
#endif
}

//
void qn_debug_down(void)
{
#ifdef _QN_WINDOWS_
	if (debug_impl.redirect && debug_impl.handle != NULL)
		CloseHandle(debug_impl.handle);
#else
	if (debug_impl.redirect && debug_impl.fp != NULL)
		fclose(debug_impl.fp);
#endif
}

//
static void qn_dbg_buf_ch(const int ch)
{
	if (1 + debug_impl.out_pos > MAX_DEBUG_LENGTH - 1)
		return;
	debug_impl.out_buf[debug_impl.out_pos++] = (char)ch;
}

//
static void qn_dbg_buf_str(const char* RESTRICT s)
{
	int len = (int)strlen(s);
	if (len + debug_impl.out_pos > MAX_DEBUG_LENGTH - 1)
		len = MAX_DEBUG_LENGTH - debug_impl.out_pos - 1;
	if (len <= 0)
		return;
	memcpy(debug_impl.out_buf + debug_impl.out_pos, s, (size_t)len);
	debug_impl.out_pos += len;
}

//
static void qn_dbg_buf_va(const char* RESTRICT fmt, va_list va)
{
	const int len = qn_vsnprintf(debug_impl.out_buf + debug_impl.out_pos, MAX_DEBUG_LENGTH - (size_t)debug_impl.out_pos, fmt, va);
	debug_impl.out_pos += len;
}

//
static void qn_dbg_buf_int(const int value)
{
	const int len = qn_itoa(debug_impl.out_buf + debug_impl.out_pos, value, 10, true);
	debug_impl.out_pos += len;
}

//
static void qn_dbg_buf_head(const char* RESTRICT head)
{
	if (head == NULL)
		head = "(unknown)";
	qn_dbg_buf_ch('[');
	qn_dbg_buf_str(head);
	qn_dbg_buf_str("] ");
}

//
static int qn_dbg_buf_flush(void)
{
	qn_val_if_fail(debug_impl.out_pos > 0, 0);
	debug_impl.out_buf[debug_impl.out_pos] = '\0';
#ifdef _QN_WINDOWS_
	if (debug_impl.handle != NULL)
	{
		DWORD wtn;
		if (debug_impl.redirect || WriteConsoleA(debug_impl.handle, debug_impl.out_buf, debug_impl.out_pos, &wtn, NULL) == 0)
			WriteFile(debug_impl.handle, debug_impl.out_buf, debug_impl.out_pos, &wtn, NULL);
	}
	if (debug_impl.debugger)
		OutputDebugStringA(debug_impl.out_buf);
#else
	if (debug_impl.fp != NULL)
		fputs(debug_impl.out_buf, debug_impl.fp);
#ifdef __EMSCRIPTEN__
	emscripten_console_log(debug_impl.out_buf);
#endif
#ifdef _QN_ANDROID_
	__android_log_print(ANDROID_LOG_VERBOSE, debug_impl.tag, debug_impl.out_buf);
#endif
#endif
	const int ret = debug_impl.out_pos;
	debug_impl.out_pos = 0;
	return ret;
}

//
int qn_debug_assert(const char* RESTRICT expr, const char* RESTRICT mesg, const char* RESTRICT filename, const int line)
{
	qn_val_if_fail(expr, -1);
	qn_dbg_buf_str("ASSERT FAILED : ");
	qn_dbg_buf_str(" (filename=\"");
	qn_dbg_buf_str(filename);
	qn_dbg_buf_str("\", line=");
	qn_dbg_buf_int(line);
	qn_dbg_buf_str(")\n\t: ");
	qn_dbg_buf_str(expr);
	qn_dbg_buf_str(">\n\t: [");
	qn_dbg_buf_str(mesg);
	qn_dbg_buf_str("]\n");
	qn_dbg_buf_flush();

	DEBUG_BREAK(debug_impl.debugger);
	return 0;
}

//
_Noreturn void qn_debug_halt(const char* RESTRICT head, const char* RESTRICT mesg)
{
	qn_dbg_buf_str("HALT ");
	qn_dbg_buf_head(head);
	qn_dbg_buf_str(mesg);
	qn_dbg_buf_ch('\n');
	qn_dbg_buf_flush();

	DEBUG_BREAK(debug_impl.debugger);
	abort();
}

//
int qn_debug_outputs(const bool breakpoint, const char* RESTRICT head, const char* RESTRICT mesg)
{
	qn_dbg_buf_head(head);
	qn_dbg_buf_str(mesg);
	qn_dbg_buf_ch('\n');
	const int len = qn_dbg_buf_flush();

	DEBUG_BREAK(breakpoint && debug_impl.debugger);
	return len;
}

//
int qn_debug_outputf(const bool breakpoint, const char* RESTRICT head, const char* RESTRICT fmt, ...)
{
	qn_dbg_buf_head(head);
	va_list va;
	va_start(va, fmt);
	qn_dbg_buf_va(fmt, va);
	va_end(va);
	qn_dbg_buf_ch('\n');
	const int len = qn_dbg_buf_flush();

	DEBUG_BREAK(breakpoint && debug_impl.debugger);
	return len;
}

//
int qn_debug_output_error(const bool breakpoint, const char* head)
{
	const char* err = qn_get_error();
	return qn_debug_outputs(breakpoint, head, err);
}

//
int qn_outputs(const char* mesg)
{
	qn_dbg_buf_str(mesg);
	qn_dbg_buf_ch('\n');
	return qn_dbg_buf_flush();
}

//
int qn_outputf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	qn_dbg_buf_va(fmt, va);
	va_end(va);
	qn_dbg_buf_ch('\n');
	return qn_dbg_buf_flush();
}


//////////////////////////////////////////////////////////////////////////
// 메모리 처리

#define MEMORY_GAP				4
#define MEMORY_BLOCK_SIZE		16
//#define MEMORY_PAGE			256
//#define MEMORY_PAGE_SIZE		(MEMORY_BLOCK_SIZE * MEMORY_PAGE)
#define MEMORY_SIGN_HEAD		('Q' | 'M'<<8 | 'B'<<16 | '\0'<<24)
#define MEMORY_SIGN_FREE		('B' | 'A'<<8 | 'D'<<16 | '\0'<<24)

#ifdef QS_NO_MEMORY_PROFILE
#define _memsize(size)			(((size)+MEMORY_GAP+MEMORY_BLOCK_SIZE-1)&(size_t)~(MEMORY_BLOCK_SIZE-1))

static void* qn_internal_alloc(size_t size, bool zero);
#ifdef _QN_WINDOWS_
static void* qn_internal_realloc(void* ptr, size_t size);
static void qn_internal_free(void* ptr);
#else
#define qn_internal_realloc		realloc
#define qn_internal_free		free
#endif
#else
//
typedef struct MemBlock
{
	uint				sign;

	uint				line;
	const char*			desc;

	size_t				index;
#ifdef _QN_64_
	size_t				align64;
#endif

	size_t				block;
	size_t				size;

	struct MemBlock*	next;
	struct MemBlock*	prev;
} MemBlock;

static_assert(sizeof(MemBlock) % MEMORY_BLOCK_SIZE == 0, "MemBlock size is not aligned!");

#define _memhdr(ptr)			(((MemBlock*)(ptr))-1)
#define _memptr(block)			(void*)(((MemBlock*)(block))+1)
#define _memsize(size)			(((sizeof(MemBlock)+(size))+MEMORY_GAP+MEMORY_BLOCK_SIZE-1)&(size_t)~(MEMORY_BLOCK_SIZE-1))

static void* qn_mpf_alloc(size_t size, bool zero, const char* desc, size_t line);
static void* qn_mpf_realloc(void* ptr, size_t size, const char* desc, size_t line);
static void qn_mpf_free(void* ptr);
#endif

// 메모리 프르프
static struct MemImpl
{
	QnAllocTable	table;

#ifdef _QN_WINDOWS_
	HANDLE			heap;
#endif

#ifndef QS_NO_MEMORY_PROFILE
	MemBlock*		frst;
	MemBlock*		last;
	size_t			index;
	size_t			count;
	size_t			block_size;

#ifndef QS_NO_SPINLOCK
	QnSpinLock		lock;
#endif
#endif
	char			dbg_buf[MAX_DEBUG_LENGTH];
} mem_impl =
{
#ifdef QS_NO_MEMORY_PROFILE
	.table = { qn_internal_alloc, qn_internal_realloc, qn_internal_free, },
#else
	.table = { qn_mpf_alloc, qn_mpf_realloc, qn_mpf_free, },
#endif
};

//
void qn_mpf_up(void)
{
#ifdef _QN_WINDOWS_
	mem_impl.heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS/*|HEAP_NO_SERIALIZE*/, MEMORY_BLOCK_SIZE, 0);
	ULONG hfv = 2;
	HeapSetInformation(mem_impl.heap, HeapCompatibilityInformation, &hfv, sizeof(ULONG));
#endif
}

#ifndef QS_NO_MEMORY_PROFILE
//
static void qn_mpf_clear(void)
{
	if (mem_impl.count == 0 && mem_impl.frst == NULL && mem_impl.last == NULL)
		return;

	qn_debug_outputf(false, "MEMORY PROFILER", "found %d allocations", mem_impl.count);

	size_t sum = 0;
	for (MemBlock* next = NULL, *node = mem_impl.frst; node; node = next)
	{
		if (node->line)
			qn_debug_outputf(false, "MEMORY PROFILER", "\t%s(%Lu) : %Lu(%Lu) : 0x%p", node->desc, node->line, node->size, node->block, _memptr(node));
		else
			qn_debug_outputf(false, "MEMORY PROFILER", "\t%Lu(%Lu) : 0x%p", node->size, node->block, _memptr(node));
		qn_memdmp(_memptr(node), QN_MIN(32, node->size), mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1);
		qn_debug_outputf(false, "MEMORY PROFILER", "\t\t{%s}", mem_impl.dbg_buf);

		next = node->next;
		sum += node->block;

#ifdef _QN_WINDOWS_
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
#endif

//
void qn_mpf_down(void)
{
#ifndef QS_NO_MEMORY_PROFILE
	qn_mpf_clear();
#endif

#ifdef _QN_WINDOWS_
	if (mem_impl.heap)
	{
		SIZE_T s;
		if (HeapQueryInformation(mem_impl.heap, HeapEnableTerminationOnCorruption, NULL, 0, &s))
			qn_debug_outputf(true, "MEMORY PROFILER", "heap allocation left: %Lu bytes", s);
		HeapDestroy(mem_impl.heap);
	}
#endif
}

#ifdef QS_NO_MEMORY_PROFILE
#ifdef _QN_WINDOWS_
#pragma warning(disable: 4702)
//
static DWORD qn_internal_memory_exception(const DWORD ex, const size_t size, const size_t block)
{
	if (ex == STATUS_NO_MEMORY)
	{
		qn_snprintf(mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1, "out of memory : size=%Lu(%Lu)", size, block);
		qn_debug_halt("MEMORY PROFILER", mem_impl.dbg_buf);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	if (ex == STATUS_ACCESS_VIOLATION)
	{
		qn_snprintf(mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1, "access violation : size=%Lu(%Lu)", size, block);
		qn_debug_halt("MEMORY PROFILER", mem_impl.dbg_buf);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
#pragma warning(default: 4702)
#endif

//
static void* qn_internal_alloc(size_t size, bool zero)
{
	qn_val_if_fail(size > 0, NULL);

#ifdef _QN_WINDOWS_
	size_t block = _memsize(size);
	void* ptr;
	__try
	{
		ptr = HeapAlloc(mem_impl.heap, zero ? HEAP_ZERO_MEMORY : 0, block);
	}
	__except (qn_internal_memory_exception(_exception_code(), size, block))
	{
		ptr = NULL;
	}
	if (ptr == NULL)
	{
		qn_snprintf(mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1, "out of memory : size=%Lu(%Lu)", size, block);
		qn_debug_halt("MEMORY PROFILER", mem_impl.dbg_buf);
	}
	return ptr;
#else
	return zero ? calloc(1, size) : malloc(size);
#endif
}

#ifdef _QN_WINDOWS_
//
static void* qn_internal_realloc(void* ptr, size_t size)
{
	if (ptr == NULL && size != 0)
		return qn_internal_alloc(size, false);
	if (size == 0)
	{
		qn_internal_free(ptr);
		return NULL;
	}

	if (HeapValidate(mem_impl.heap, 0, ptr) == FALSE)
	{
		qn_snprintf(mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1, "try to realloc invalid memory : 0x%p", ptr);
		qn_debug_halt("MEMORY PROFILER", mem_impl.dbg_buf);
	}

	size_t block = _memsize(size);
	void* newptr;
	__try
	{
		newptr = HeapReAlloc(mem_impl.heap, 0, ptr, block);
	}
	__except (qn_internal_memory_exception(_exception_code(), size, block))
	{
		newptr = NULL;
	}
	if (newptr == NULL)
	{
		HeapFree(mem_impl.heap, 0, ptr);
		qn_snprintf(mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1, "out of memory : size=%Lu(%Lu)", size, block);
		qn_debug_halt("MEMORY PROFILER", mem_impl.dbg_buf);
	}
	return newptr;
}

//
static void qn_internal_free(void* ptr)
{
	qn_ret_if_fail(ptr);

	if (HeapValidate(mem_impl.heap, 0, ptr) == FALSE)
	{
		qn_debug_outputf(false, "MEMORY PROFILER", "try to free invalid memory : 0x%p", ptr);
		qn_memdmp(ptr, 19, mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1);
		qn_debug_outputf(true, "MEMORY PROFILER", "\t\t{%s}", mem_impl.dbg_buf);
		return;
	}
	HeapFree(mem_impl.heap, 0, ptr);
}
#endif
#else
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

//
void qn_mpfdbgprint(void)
{
	if (mem_impl.count == 0 && mem_impl.frst == NULL && mem_impl.last == NULL)
		return;

	qn_debug_outputf(false, "MEMORY PROFILER", "found %d allocations", mem_impl.count);
	qn_debug_outputf(false, "MEMORY PROFILER", " %-8s | %-8s | %-8s | %-s", "no", "size", "block", "desc");

	QN_LOCK(mem_impl.lock);
	size_t sum = 0, cnt = 1;
	for (MemBlock* next, *node = mem_impl.frst; node; node = next, cnt++)
	{
		if (node->line)
		{
			qn_debug_outputf(false, "MEMORY PROFILER", " %-8d | %-8zu | % -8zu | \"%s:%d\"",
				cnt, node->size, node->block, node->desc, (int)node->line);
		}
		else
		{
			qn_memdmp(_memptr(node), QN_MIN(32, node->size), mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1);
			qn_debug_outputf(false, "MEMORY PROFILER", " %-8d | %-8zu | % -8zu | <%s>",
				cnt, node->size, node->block, mem_impl.dbg_buf);
		}
		next = node->next;
		sum += node->block;
	}
	QN_UNLOCK(mem_impl.lock);

	double size;
	const char usage = qn_memhrb(sum, &size);
	qn_debug_outputf(false, "MEMORY PROFILER", "block size: %.3g%cbytes", size, usage);
}

//
static void qn_mpf_node_link(MemBlock* node)
{
	QN_LOCK(mem_impl.lock);
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
	QN_UNLOCK(mem_impl.lock);
}

//
static void qn_mpf_node_unlink(const MemBlock* node)
{
	QN_LOCK(mem_impl.lock);
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
	QN_UNLOCK(mem_impl.lock);
}

//
_Noreturn static void qn_mpf_out_of_memory(const char* desc, const size_t line, const size_t size, const size_t block)
{
	qn_snprintf(mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1, "out of memory : %s(%Lu) : %Lu(%Lu)", desc, line, size, block);
	qn_debug_halt("MEMORY PROFILER", mem_impl.dbg_buf);
}

//
_Noreturn static void qn_mpf_access_violation(const char* desc, const size_t line, const size_t size, const size_t block)
{
	qn_snprintf(mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1, "access violation : %s(%Lu) : %Lu(%Lu)", desc, line, size, block);
	qn_debug_halt("MEMORY PROFILER", mem_impl.dbg_buf);
}

#ifdef _QN_WINDOWS_
#pragma warning(disable: 4702)
//
static DWORD qn_mpf_windows_exception(const DWORD ex, const char* desc, const size_t line, const size_t size, const size_t block)
{
	if (ex == STATUS_NO_MEMORY)
	{
		qn_mpf_out_of_memory(desc, line, size, block);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	if (ex == STATUS_ACCESS_VIOLATION)
	{
		qn_mpf_access_violation(desc, line, size, block);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
#pragma warning(default: 4702)
#endif

//
static void* qn_mpf_alloc(size_t size, bool zero, const char* desc, size_t line)
{
	qn_val_if_fail(size > 0, NULL);

	size_t block = _memsize(size);
	MemBlock* node;
#ifdef _QN_WINDOWS_
	__try
	{
		node = (MemBlock*)HeapAlloc(mem_impl.heap, zero ? HEAP_ZERO_MEMORY : 0, block);
	}
	__except (qn_mpf_windows_exception(_exception_code(), desc, line, size, block))
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
	if (ptr == NULL && size != 0)
		return qn_mpf_alloc(size, false, desc, line);
	if (size == 0)
	{
		qn_mpf_free(ptr);
		return NULL;
	}

	MemBlock* node = _memhdr(ptr);
	if (node == NULL ||
#ifdef _QN_WINDOWS_
		HeapValidate(mem_impl.heap, 0, node) == FALSE ||
#endif
		node->sign != MEMORY_SIGN_HEAD)
	{
		qn_debug_outputf(false, "MEMORY PROFILER", "try to realloc invalid memory : 0x%p", ptr);
		qn_mpf_access_violation(desc, line, size, 0);
	}

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
	__except (qn_mpf_windows_exception(_exception_code(), desc, line, size, block))
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
static void qn_mpf_free(void* ptr)
{
	qn_ret_if_fail(ptr);

	MemBlock* node = _memhdr(ptr);
	if (node == NULL ||
#ifdef _QN_WINDOWS_
		HeapValidate(mem_impl.heap, 0, node) == FALSE ||
#endif
		node->sign != MEMORY_SIGN_HEAD)
	{
		qn_debug_outputf(false, "MEMORY PROFILER", "try to realloc invalid memory : 0x%p", ptr);
		qn_memdmp(ptr, 19, mem_impl.dbg_buf, QN_COUNTOF(mem_impl.dbg_buf) - 1);
		qn_debug_outputf(true, "MEMORY PROFILER", "\t\t{%s}", mem_impl.dbg_buf);
		return;
	}

	node->sign = MEMORY_SIGN_FREE;
	qn_mpf_node_unlink(node);

#if _QN_WINDOWS_
	HeapFree(mem_impl.heap, 0, node);
#else
	free(node);
#endif
}
#endif


//////////////////////////////////////////////////////////////////////////
// 테이블 기반 메모리

//
bool qn_memtbl(const QnAllocTable* table)
{
	if (
#ifndef QS_NO_MEMORY_PROFILE
		mem_impl.index != 0 ||
#endif
		table == NULL ||
		table->_alloc == NULL ||
		table->_realloc == NULL ||
		table->_free == NULL)
		return false;
	mem_impl.table = *table;
	return true;
}

//
void qn_mem_free(void* ptr)
{
	mem_impl.table._free(ptr);
}

#ifdef QS_NO_MEMORY_PROFILE
//
void* qn_a_alloc(const size_t size, const bool zero)
{
	return mem_impl.table._alloc(size, zero);
}

//
void* qn_a_realloc(void* ptr, const size_t size)
{
	return mem_impl.table._realloc(ptr, size);
}

//
void* qn_a_mem_dup(const void* ptr, size_t size_or_zero_if_psz)
{
	qn_val_if_fail(ptr != NULL, NULL);
	if (size_or_zero_if_psz > 0)
	{
		byte* m = (byte*)qn_a_alloc(size_or_zero_if_psz, false);
		memcpy(m, ptr, size_or_zero_if_psz);
		return m;
	}
	const size_t len = strlen((const char*)ptr) + 1;
	char* d = (char*)qn_a_alloc(len, false);
	qn_strcpy(d, (const char*)ptr);
	return d;
}
#else
//
void* qn_a_i_alloc(const size_t size, const bool zero, const char* desc, const size_t line)
{
	return mem_impl.table._alloc(size, zero, desc, line);
}

//
void* qn_a_i_realloc(void* ptr, const size_t size, const char* desc, const size_t line)
{
	return mem_impl.table._realloc(ptr, size, desc, line);
}

//
void* qn_a_i_mem_dup(const void* ptr, size_t size_or_zero_if_psz, const char* desc, size_t line)
{
	qn_val_if_fail(ptr != NULL, NULL);
	if (size_or_zero_if_psz > 0)
	{
		byte* m = (byte*)qn_a_i_alloc(size_or_zero_if_psz, false, desc, line);
		memcpy(m, ptr, size_or_zero_if_psz);
		return m;
	}
	const size_t len = strlen((const char*)ptr) + 1;
	char* d = (char*)qn_a_i_alloc(len, false, desc, line);
	qn_strcpy(d, (const char*)ptr);
	return d;
}
#endif
