#pragma once

#define MEMORY_GAP				4
#define MEMORY_BLOCK_SIZE		16
#define MEMORY_PAGE				256
#define MEMORY_PAGE_SIZE		(MEMORY_BLOCK_SIZE*MEMORY_PAGE)
#define MEMORY_SIGN_HEAD		('Q' | 'N'<<8 | 'M'<<16 | '\0'<<24)
#define MEMORY_SIGN_FREE		('B' | 'A'<<8 | 'D'<<16 | '\0'<<24)

//
typedef struct memBlock
{
	uint32_t			sign;
#if _QN_64_
	uint32_t			align64;
#endif

	const char*			desc;
	size_t				line;

	size_t				index;

	size_t				size;
	size_t				block;

	struct memBlock*	next;
	struct memBlock*	prev;
} memBlock;

#define _memhdr(ptr)			(((memBlock*)(ptr))-1)
#define _memptr(block)			(pointer_t)(((memBlock*)(block))+1)
#define _memsize(size)			(((sizeof(memBlock)+(size)+MEMORY_GAP)+MEMORY_BLOCK_SIZE-1)&~(MEMORY_BLOCK_SIZE-1))
