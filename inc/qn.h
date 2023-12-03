﻿#pragma once

#if _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)		// L4, nonstandard extension used : nameless struct/union
#endif

//////////////////////////////////////////////////////////////////////////
// version
#define _QN_VERSION_					202311L


//////////////////////////////////////////////////////////////////////////
// runtime configuration

// platform
#if _WIN32
#	define _QN_WINDOWS_					1
#endif

#if __FreeBSD__ || __OpenBSD__
#	define _QN_BSD_						1
#endif

#if __linux__
#	define _QN_LINUX_					1
#endif

#if __android__
#	define _QN_ANDROID_					1
#	define _QN_MOBILE_					1
#endif

#if __unix__
#	define _QN_UNIX_					1
#endif

#if _WIN64 || __LP64__ || __amd64__ || __x86_64__ || __aarch64__
#	define _QN_64_						1
#endif

#ifdef _CHAR_UNSIGNED
#	error "Compiler option: char type must be signed"
#endif

// compiler check
#if !(_MSC_VER || __GNUC__)
#error unknown compiler! (support: MSVC, CLANG, GCC)
#endif

// compiler specific
#if _MSC_VER
#	define QN_RESTRICT					__restrict
#	define QN_FORCELINE					__forceinline
#	define QN_INLINE					__inline
#	define QN_FUNC_NAME					__FUNCTION__
#	define QN_MEM_BARRIER()				_ReadWriteBarrier()
#	define QN_STATIC_ASSERT				static_assert
#elif __GNUC__
#	define QN_RESTRICT					restrict
#ifndef __cplusplus
#	define QN_FORCELINE					static inline __attribute__ ((always_inline))
#	define QN_INLINE					static inline
#else
#	define QN_FORCELINE					inline __attribute__ ((always_inline))
#	define QN_INLINE					inline
#endif
#	define QN_FUNC_NAME					__FUNCTION__	/* __PRETTY_FUNCTION__ */
#if __X86__ || _M_X86 || __AMD64__ || __x86_64__
#	define QN_MEM_BARRIER()				__asm__ __volatile__("" : : : "memory");
#elif __aarch64__
#	define QN_MEM_BARRIER()				__asm__ __volatile__("dsb sy" : : : "memory")
#else
#	define QN_MEM_BARRIER()				__sync_synchronize();
#endif
#	define QN_STATIC_ASSERT				_Static_assert
#endif

#if defined(__cplusplus)
#	define QN_EXTC						extern "C"
#	define QN_EXTC_BEGIN				extern "C" {
#	define QN_EXTC_END					}
#else
#	define QN_EXTC
#	define QN_EXTC_BEGIN
#	define QN_EXTC_END
#endif

// path separator
#if _QN_WINDOWS_
#	define QN_PATH_SEP					'\\'
#else
#	define QN_PATH_SEP					'/'
#endif

// api
#ifndef QNAPI
#	if defined(_LIB) || defined(_STATIC)
#		define QNAPI					extern
#	else
#		if _QN_WINDOWS_
#			define QNAPI				__declspec(dllimport)
#		elif __GNUC__
#			define QNAPI				__attribute__((__visibility__("default")))
#		else
#			define QNAPI				extern
#		endif
#	endif
#endif


//////////////////////////////////////////////////////////////////////////
// includes
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#include <signal.h>
#if _QN_UNIX_
#include <sys/time.h>
#if _QN_LINUX_
#include <linux/limits.h>
#endif
#endif


//////////////////////////////////////////////////////////////////////////
// definition

QN_EXTC_BEGIN

// compond & statement
#define QN_STMT_BEGIN					do
#define QN_STMT_END						while(0)

// function support
#define _QN_GET_MAC_2(_1,_2,N,...)		N
#define _QN_GET_MAC_3(_1,_2,_3,N,...)	N
#define _QN_STRING(x)					#x
#define _QN_UNICODE(x)					L##x
#define _QN_CONCAT_2(x,y)				x##y
#define _QN_CONCAT_3(x,y,z)				x##y##z

// function
#define QN_STRING(x)					_QN_STRING(x)
#define QN_UNICODE(x)					_QN_UNICODE(x)
#define QN_CONCAT_2(x,y)				_QN_CONCAT_2(x, y)
#define QN_CONCAT_3(x,y,z)				_QN_CONCAT_3(x, y, z)
#define QN_CONCAT(...)					_QN_GET_MAC_3(__VA_ARGS__, QN_CONCAT_3, QN_CONCAT_2)(__VA_ARGS__)

#define QN_COUNTOF(arr)					(sizeof(arr)/sizeof((arr)[0]))
#define QN_OFFSETOF(type,mem)			((size_t)((char*)&((type*)0)->mem))
#define QN_MEMBEROF(type,p,off)			(*(type*)((void*)((char*)(p)+(size_t)(offset))))

#define QN_MAX(a,b)						(((a)>(b))?(a):(b))
#define QN_MIN(a,b)						(((a)<(b))?(a):(b))
#define QN_ABS(v)						(((v)<0)?-(v):(v))
#define QN_CLAMP(v,vmin,vmax)			((v)<(vmin)?(vmin):(v)>(vmax)?(vmax):(v))

#define QN_BIT(bit)						(1<<(bit))
#define QN_MASK(v,mask)					((mask==0)?(v):(v)&(mask))
#define QN_SET_BIT(pv,bit,isset)		((isset)?((*(pv))|=(1<<(bit))):((*(pv))&=~(1<<(bit))))
#define QN_SET_MASK(pv,mask,isset)		((isset)?((*(pv))|=(mask)):((*(pv))&=~(mask)))
#define QN_TEST_BIT(v,bit)				(((v)&(1<<(bit)))!=0)
#define QN_TEST_MASK(v,mask)			(((v)&(mask))!=0)

// constant
#define QN_USEC_PER_SEC					1000000
#define QN_NSEC_PER_SEC					1000000000

#define QN_MIN_HASH						11
#define QN_MAX_HASH						13845163
#define QN_MAX_RAND						0x7FFF
#define QN_MAX_PATH						1024


//////////////////////////////////////////////////////////////////////////
// types

typedef void (*func_t)(void);								/** @brief function handle */
typedef void (*paramfunc_t)(void*);							/** @brief parameter function handle */

// aliases
typedef int8_t							sbyte;
typedef int64_t							llong;
typedef uint8_t							byte;
typedef uint16_t						ushort;
typedef uint32_t						uint;
typedef uint64_t						ullong;

typedef intptr_t						nint;
typedef uintptr_t						nuint;

typedef wchar_t							wchar;
#if _QN_WINDOWS_
typedef uint32_t						uchar4;				/** @brief 4byte(32bit) unicode */
typedef wchar_t							uchar2;				/** @brief 2byte(16bit) unicode */
#else
typedef wchar_t							uchar4;				/** @brief 4byte(32bit) unicode */
typedef uint16_t						uchar2;				/** @brief 2byte(16bit) unicode */
#endif

typedef uint16_t						halfint;

/** @brief variant 16bit short */
typedef union vint16_t
{
	struct
	{
		uint8_t			l, h;
	} b;
	uint16_t			w;
} vint16_t, vshort;

/** @brief variant 32bit int */
typedef union vint32_t
{
	struct
	{
		uint8_t			a, b, c, d; // d=msb
	} b;
	struct
	{
		uint16_t		l, h;
	} w;
	uint32_t			dw;
} vint32_t, vint;

/** @brief variant 64bit long long */
typedef union vint64_t
{
	struct
	{
		uint8_t			la, lb, lc, ld;
		uint8_t			ha, hb, hc, hd; // hd=msb
	} b;
	struct
	{
		uint16_t		a, b, c, d;
	} w;
	struct
	{
		uint32_t		l, h;
	} dw;
	uint64_t			q;
} vint64_t, vlong;

/** @brief any value */
typedef union any_t
{
	bool				b;
	int32_t				i;
	uint32_t			u;
	float				f;

	char*				s;
	wchar_t*			w;
	void*			p;
	const void*			cp;
	func_t				func;

	int64_t				i64;
	uint64_t			u64;
	double				dbl;

	uint8_t				data[8];
} any_t;

/** @brief function parameter */
typedef struct funcparam_t
{
	paramfunc_t			func;
	void*				data;
} funcparam_t;

// cast
#define qn_ptrcast_int(v)				((int)(nint)(v))
#define qn_ptrcast_uint(v)				((uint)(nuint)(v))
#define qn_ptrcast_size(v)				((size_t)(nuint)(v))
#define qn_ptrcast_float(v)				(*(float*)&(v))
#define qn_castptr(v)					((nint)(v))
#define qn_castptr_float(v)				(*(nint*)&(v))
#if _QN_64_
#define qn_ptrcast_llong(v)				((llong)(nint)(v))
#define qn_ptrcast_ullong(v)			((ullong)(nuint)(v))
#define qn_ptrcast_double(v)			(*(double*)&(v))
#define qn_castptr_double(v)			(*(nint*)&(v))
#endif

// conditions
#define qn_ret_if_fail(x)				QN_STMT_BEGIN{ if (!(x)) return; }QN_STMT_END
#define qn_ret_if_ok(x)					QN_STMT_BEGIN{ if ((x)) return; }QN_STMT_END
#define qn_retval_if_fail(x,r)			QN_STMT_BEGIN{ if (!(x)) return (r); }QN_STMT_END
#define qn_retval_if_ok(x,r)			QN_STMT_BEGIN{ if ((x)) return (r); }QN_STMT_END

#if _DEBUG
#define qn_assert(expr,mesg)			QN_STMT_BEGIN{if (!(expr)) qn_debug_assert(#expr, mesg, __FILE__, __LINE__);}QN_STMT_END
#define qn_verify(expr)					QN_STMT_BEGIN{if (!(expr)) qn_debug_assert(#expr, NULL, __FILE__, __LINE__);}QN_STMT_END
#else
#define qn_assert(expr,rs_mesg)			((void)0)
#define qn_verify(...)					((void)0)
#endif


//////////////////////////////////////////////////////////////////////////
// runtime
QNAPI void qn_runtime(int /*NULLABLE*/v[]);
QNAPI void qn_exit(const int exitcode);
QNAPI void qn_atexit(paramfunc_t func, void* data);
QNAPI void qn_atexitp(paramfunc_t func, void* data);

QNAPI int qn_debug_assert(const char* expr, const char* mesg, const char* filename, int line);
QNAPI void qn_debug_halt(const char* cls, const char* msg);
QNAPI int qn_debug_outputs(bool breakpoint, const char* head, const char* mesg);
QNAPI int qn_debug_outputf(bool breakpoint, const char* head, const char* fmt, ...);

QNAPI int qn_outputs(const char* mesg);
QNAPI int qn_outputf(const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// memory
#define qn_alloc(cnt,type)				(type*)qn_mpfalloc((cnt)*sizeof(type), false, QN_FUNC_NAME, __LINE__)
#define qn_alloc_1(type)				(type*)qn_mpfalloc(sizeof(type), false, QN_FUNC_NAME, __LINE__)
#define qn_alloc_zero(cnt, type)		(type*)qn_mpfalloc((cnt)*sizeof(type), true, QN_FUNC_NAME, __LINE__)
#define qn_alloc_zero_1(type)			(type*)qn_mpfalloc(sizeof(type), true, QN_FUNC_NAME, __LINE__)
#define qn_realloc(ptr,cnt,type)		(type*)qn_mpfreloc((void*)(ptr), (cnt)*sizeof(type), QN_FUNC_NAME, __LINE__)

#define qn_free(ptr)					qn_mpffree((void*)(ptr))
#define qn_free_ptr(pptr)				QN_STMT_BEGIN{ qn_mpffree((void*)*(pptr)); *(pptr)=NULL; }QN_STMT_END

#if _MSC_VER
#define qn_alloca(cnt,type)				(type*)_malloca((cnt)*sizeof(type))
#define qn_freea(ptr)					_freea(ptr)
#else
#define qn_alloca(cnt,type)				(type*)alloca((cnt)*sizeof(type))
#define qn_freea(ptr)					((void)0)
#endif

#define qn_zero(ptr,cnt,type)			memset((ptr), 0, sizeof(type)*(cnt))
#define qn_zero_1(ptr)					memset((ptr), 0, sizeof(*(ptr)))

QNAPI void* qn_memenc(void* dest, const void* src, size_t size);
QNAPI void* qn_memdec(void* dest, const void* src, size_t size);
QNAPI void* qn_memzcpr(const void* src, size_t srcsize, /*NULLABLE*/size_t* destsize);
QNAPI void* qn_memzucp(const void* src, size_t srcsize, size_t bufsize, /*NULLABLE*/size_t* destsize);
QNAPI size_t qn_memagn(size_t size);
QNAPI char qn_memhrd(size_t size, double* out);
QNAPI char* qn_memdmp(const void* ptr, size_t size, char* outbuf, size_t buflen);

QNAPI size_t qn_mpfsize(void);
QNAPI size_t qn_mpfcnt(void);
QNAPI void* qn_mpfalloc(size_t size, bool zero, const char* desc, size_t line);
QNAPI void* qn_mpfreloc(void* ptr, size_t size, const char* desc, size_t line);
QNAPI void qn_mpffree(void* ptr);


//////////////////////////////////////////////////////////////////////////
// hash & sort
QNAPI size_t qn_hashptr(const void* p);
QNAPI size_t qn_hashnow(void);
QNAPI size_t qn_hashfn(int prime8, func_t func, void* data);
QNAPI size_t qn_hashcrc(const byte* data, size_t size);
QNAPI uint32_t qn_primenear(uint32_t value);
QNAPI uint32_t qn_primeshift(uint32_t value, uint32_t min, uint32_t* shift);

QNAPI void qn_qsort(void* ptr, size_t count, size_t stride, int(*compfunc)(const void*, const void*));
QNAPI void qn_qsortc(void* ptr, size_t count, size_t stride, int(*compfunc)(void*, const void*, const void*), void* context);


//////////////////////////////////////////////////////////////////////////
// string
QNAPI int qn_vsnprintf(char* out, size_t len, const char* fmt, va_list va);
QNAPI int qn_vasprintf(char** out, const char* fmt, va_list va);
QNAPI char* qn_vapsprintf(const char* fmt, va_list va);
QNAPI int qn_snprintf(char* out, size_t len, const char* fmt, ...);
QNAPI int qn_asprintf(char** out, const char* fmt, ...);
QNAPI char* qn_apsprintf(const char* fmt, ...);
QNAPI size_t qn_strfll(char* dest, size_t pos, size_t end, int ch);
QNAPI size_t qn_strhash(const char* p);
QNAPI size_t qn_strihash(const char* p);
QNAPI char* qn_strbrk(const char* p, const char* c);
QNAPI char* qn_strmid(char* dest, size_t destsize, const char* src, size_t pos, size_t len);
QNAPI char* qn_strltm(char* dest);
QNAPI char* qn_strrtm(char* dest);
QNAPI char* qn_strtrm(char* dest);
QNAPI char* qn_strrem(char* p, const char* rmlist);
QNAPI char* qn_strpcpy(char* dest, const char* src);
QNAPI char* qn_strcat(const char* p, ...);
QNAPI int qn_strwcm(const char* string, const char* wild);
QNAPI int qn_striwcm(const char* string, const char* wild);
QNAPI int qn_strfnd(const char* src, const char* find, size_t index);
#if _MSC_VER
#define qn_strcpy						strcpy_s
#define qn_strncpy						strncpy_s
#define qn_strdup						_strdup
#define qn_strupr						_strupr_s
#define qn_strlwr						_strlwr_s
#define qn_strncmp						strncmp
#define qn_stricmp						_stricmp
#define qn_strnicmp						_strnicmp
#else
#define qn_strcpy(a,b,c)				strcpy(a,c)
#define qn_strncpy(a,b,c,d)				strncpy(a,c,d)
#define qn_strdup						strdup
QNAPI char* qn_strupr(char* p, size_t size);
QNAPI char* qn_strupr(char* p, size_t size);
QNAPI int qn_strncmp(const char* p1, const char* p2, size_t len);
QNAPI int qn_stricmp(const char* p1, const char* p2);
QNAPI int qn_strnicmp(const char* p1, const char* p2, size_t len);
#endif

QNAPI int qn_vsnwprintf(wchar* out, size_t len, const wchar* fmt, va_list va);
QNAPI int qn_vaswprintf(wchar** out, const wchar* fmt, va_list va);
QNAPI wchar* qn_vapswprintf(const wchar* fmt, va_list va);
QNAPI int qn_snwprintf(wchar* out, size_t len, const wchar* fmt, ...);
QNAPI int qn_aswprintf(wchar** out, const wchar* fmt, ...);
QNAPI wchar* qn_apswprintf(const wchar* fmt, ...);
QNAPI size_t qn_wcsfll(wchar* dest, size_t pos, size_t end, int ch);
QNAPI size_t qn_wcshash(const wchar* p);
QNAPI size_t qn_wcsihash(const wchar* p);
QNAPI wchar* qn_wcsbrk(const wchar* p, const wchar* c);
QNAPI wchar* qn_wcsmid(wchar* dest, size_t destsize, const wchar* src, size_t pos, size_t len);
QNAPI wchar* qn_wcsltm(wchar* dest);
QNAPI wchar* qn_wcsrtm(wchar* dest);
QNAPI wchar* qn_wcstrm(wchar* dest);
QNAPI wchar* qn_wcsrem(wchar* p, const wchar* rmlist);
QNAPI wchar* qn_wcspcpy(wchar* dest, const wchar* src);
QNAPI wchar* qn_wcscat(const wchar* p, ...);
QNAPI int qn_wcswcm(const wchar* string, const wchar* wild);
QNAPI int qn_wcsiwcm(const wchar* string, const wchar* wild);
QNAPI int qn_wcsfnd(const wchar* src, const wchar* find, size_t index);
#if _MSC_VER
#define qn_wcscpy						wcscpy_s
#define qn_wcsncpy						wcsncpy_s
#define qn_wcsdup						_wcsdup
#define qn_wcsupr						_wcsupr_s
#define qn_wcslrw						_wcslwr_s
#define qn_wcsncmp						wcsncmp
#define qn_wcsicmp						_wcsicmp
#define qn_wcsnicmp						_wcsnicmp
#else
#define qn_wcscpy(a,b,c)				wcscpy(a,c)
#define qn_wcsncpy(a,b,c,d)				wcsncpy(a,c,d)
#define qn_wcsdup						qwcsdup
QNAPI wchar* qn_wcsupr(wchar* p, size_t size);
QNAPI wchar* qn_wcslwr(wchar* p, size_t size);
QNAPI int qn_wcsncmp(const wchar* p1, const wchar* p2, size_t len);
QNAPI int qn_wcsicmp(const wchar* p1, const wchar* p2);
QNAPI int qn_wcsnicmp(const wchar* p1, const wchar* p2, size_t len);
#endif

QNAPI size_t qn_u8len(const char* s);
QNAPI uchar4 qn_u8cbn(const char* p);
QNAPI char* qn_u8nch(const char* s);
QNAPI int qn_u8ucb(uchar4 c, char* out);

QNAPI size_t qn_mbstowcs(wchar* outwcs, size_t outsize, const char* inmbs, size_t insize);
QNAPI size_t qn_wcstombs(char* outmbs, size_t outsize, const wchar* inwcs, size_t insize);
QNAPI size_t qn_u8to32(uchar4* dest, size_t destsize, const char* src, size_t srclen);
QNAPI size_t qn_u8to16(uchar2* dest, size_t destsize, const char* src, size_t srclen);
QNAPI size_t qn_u32to8(char* dest, size_t destsize, const uchar4* src, size_t srclen);
QNAPI size_t qn_u16to8(char* dest, size_t destsize, const uchar2* src, size_t srclen);
QNAPI size_t qn_u16to32(uchar4* dest, size_t destsize, const uchar2* src, size_t srclen);
QNAPI size_t qn_u32to16(uchar2* dest, size_t destsize, const uchar4* src, size_t srclen);


//////////////////////////////////////////////////////////////////////////
// time

QNAPI void qn_localtime(struct tm* ptm, const time_t tt);
QNAPI void qn_gmtime(struct tm* ptm, const time_t tt);

QNAPI uint64_t qn_cycle(void);
QNAPI uint64_t qn_tick(void);
QNAPI double qn_stick(void);

QNAPI void qn_sleep(uint32_t milliseconds);
QNAPI void qn_usleep(uint32_t microseconds);
QNAPI void qn_ssleep(uint32_t seconds);
QNAPI void qn_msleep(uint64_t microseconds);

/** @brief date time */
typedef struct qnDateTime
{
	union
	{
		ullong		stamp;
		struct
		{
			uint	date;
			uint	time;
		};
		struct
		{
			uint	year : 14;
			uint	month : 6;
			uint	day : 8;
			uint	dow : 4;

			uint	hour : 6;
			uint	minute : 8;
			uint	second : 8;
			uint	millisecond : 10;
		};
	};
} qnDateTime;

QNAPI void qn_now(qnDateTime* dt);
QNAPI void qn_utc(qnDateTime* dt);
QNAPI void qn_stod(double sec, qnDateTime* dt);
QNAPI void qn_mstod(uint msec, qnDateTime* dt);

/** @brief timer */
typedef struct qnTimer
{
	double				abstime;
	double				runtime;
	double				advance;
	double				fps;
} qnTimer;

QNAPI qnTimer* qn_timer_new(void);
QNAPI void qn_timer_delete(qnTimer* self);
QNAPI void qn_timer_reset(qnTimer* self);
QNAPI void qn_timer_start(qnTimer* self);
QNAPI void qn_timer_stop(qnTimer* self);
QNAPI bool qn_timer_update(qnTimer* self);
QNAPI double qn_timer_get_abs(qnTimer* self);
QNAPI double qn_timer_get_run(qnTimer* self);
QNAPI double qn_timer_get_fps(qnTimer* self);
QNAPI double qn_timer_get_adv(qnTimer* self);
QNAPI double qn_timer_get_cut(qnTimer* self);
QNAPI void qn_timer_set_cut(qnTimer* self, double cut);
QNAPI void qn_timer_set_manual(qnTimer* self, bool value);


//////////////////////////////////////////////////////////////////////////
// i/o

/** @brief file io */
typedef struct qnIoFuncDesc
{
	int(*read)(void* handle, void* buffer, int offset, int size);
	int(*write)(void* handle, const void* buffer, int offset, int size);
	int64_t(*tell)(void* handle);
	int64_t(*seek)(void* handle, int64_t offset, int org);
} qnIoFuncDesc;


//////////////////////////////////////////////////////////////////////////
// disk i/o

/** @brief directory */
typedef struct qnDir qnDir;

/** @brief file */
typedef struct qnFile qnFile;

/** @brief file info */
typedef struct qnFileInfo
{
	short				type;
	ushort				len;
	llong				size;
	llong				cmpr;
	ullong				stc;			// creation stamp
	ullong				stw;			// last written stamp
	char				name[256];
} qnFileInfo;

/** @brief file access */
typedef struct qnFileAccess
{
#if _MSC_VER
	uint				mode;
	uint				share;
	uint				access;
	uint				attr;
#else
	int					mode;
	int					access;
#endif
} qnFileAccess;

/** @brief seek */
typedef enum qnSeek
{
	QN_SEEK_BEGIN = 0,
	QN_SEEK_CUR = 1,
	QN_SEEK_END = 2,
} qnSeek;

/** @brief file flag */
typedef enum qnFileFlag
{
	QN_FF_READ = 0x1,
	QN_FF_WRITE = 0x2,
	QN_FF_SEEK = 0x4,
	QN_FF_ALL = QN_FF_READ | QN_FF_WRITE | QN_FF_SEEK,
	QN_FF_RDONLY = QN_FF_READ | QN_FF_SEEK,
} qnFileFlag;

// file
QNAPI qnFile* qn_file_new(const char* filename, const char* mode);
QNAPI qnFile* qn_file_new_dup(qnFile* org);
QNAPI void qn_file_delete(qnFile* self);
QNAPI int qn_file_flags(qnFile* self, int mask);
QNAPI const char* qn_file_name(qnFile* self);
QNAPI int qn_file_read(qnFile* self, void* buffer, int offset, int size);
QNAPI int qn_file_write(qnFile* self, const void* buffer, int offset, int size);
QNAPI int64_t qn_file_size(qnFile* self);
QNAPI int64_t qn_file_tell(qnFile* self);
QNAPI int64_t qn_file_seek(qnFile* self, llong offset, int org);
QNAPI bool qn_file_flush(qnFile* self);
QNAPI int qn_file_printf(qnFile* self, const char* fmt, ...);
QNAPI int qn_file_vprintf(qnFile* self, const char* fmt, va_list va);
QNAPI bool qn_file_exist(const char* filename, /*RET-NULLABLE*/bool* isdir);
QNAPI void* qn_file_alloc(const char* filename, int* size);
QNAPI qnFile* qn_file_new_l(const wchar* filename, const wchar* mode);
QNAPI bool qn_file_exist_l(const wchar* filename, /*RET-NULLABLE*/bool* isdir);
QNAPI void* qn_file_alloc_l(const wchar* filename, int* size);
QNAPI size_t qn_file_get_max_alloc_size(void);
QNAPI void qn_file_set_max_alloc_size(size_t n);

// directory
QNAPI qnDir* qn_dir_new(const char* path);
QNAPI void qn_dir_delete(qnDir* self);
QNAPI const char* qn_dir_read(qnDir* self);
QNAPI void qn_dir_rewind(qnDir* self);
QNAPI void qn_dir_seek(qnDir* self, int pos);
QNAPI int qn_dir_tell(qnDir* self);
QNAPI qnDir* qn_dir_new_l(const wchar* path);
QNAPI const wchar_t* qn_dir_read_l(qnDir* self);


//////////////////////////////////////////////////////////////////////////
// xml

/** @brief ml unit */
typedef struct qnMlu qnMlu;

/** @brief ml tag */
typedef struct qnMlTag
{
	char*				name;
	char*				cntx;

	int					nlen;
	int					clen;
	int					line;
} qnMlTag;

// ml unit
QNAPI qnMlu* qn_mlu_new(void);
QNAPI qnMlu* qn_mlu_new_file(const char* filename);
QNAPI qnMlu* qn_mlu_new_file_l(const wchar* filename);
QNAPI qnMlu* qn_mlu_new_buffer(const void* data, int size);
QNAPI void qn_mlu_delete(qnMlu* self);

QNAPI void qn_mlu_clean_tags(qnMlu* self);
QNAPI void qn_mlu_clean_errs(qnMlu* self);
QNAPI bool qn_mlu_load_buffer(qnMlu* self, const void* data, int size);
QNAPI bool qn_mlu_write_file(qnMlu* self, const char* filename);

QNAPI int qn_mlu_get_count(qnMlu* self);
QNAPI const char* qn_mlu_get_err(qnMlu* self, int at);
QNAPI qnMlTag* qn_mlu_get_tag(qnMlu* self, const char* name);
QNAPI qnMlTag* qn_mlu_get_tag_nth(qnMlu* self, int at);
QNAPI const char* qn_mlu_get_context(qnMlu* self, const char* name, const char* ifnotexist);
QNAPI const char* qn_mlu_get_context_nth(qnMlu* self, int at, const char* ifnotexist);
QNAPI int qn_mlu_contains(qnMlu* self, qnMlTag* tag);

QNAPI void qn_mlu_foreach(qnMlu* self, void(*func)(void*, qnMlTag*), void* userdata);
QNAPI void qn_mlu_loopeach(qnMlu* self, void(*func)(qnMlTag* tag));

QNAPI qnMlTag* qn_mlu_add(qnMlu* self, const char* name, const char* context, int line);
QNAPI qnMlTag* qn_mlu_add_tag(qnMlu* self, qnMlTag* tag);
QNAPI int qn_mlu_remove(qnMlu* self, const char* name, bool isall);
QNAPI bool qn_mlu_remove_nth(qnMlu* self, int at);
QNAPI bool qn_mlu_remove_tag(qnMlu* self, qnMlTag* tag, bool isdelete);

QNAPI void qn_mlu_add_err(qnMlu* self, const char* msg);
QNAPI void qn_mlu_add_errf(qnMlu* self, const char* fmt, ...);
QNAPI void qn_mlu_print_err(qnMlu* self);

QNAPI void qn_mlu_print(qnMlu* self);

// tag
QNAPI qnMlTag* qn_mltag_new(const char* name);
QNAPI void qn_mltag_delete(qnMlTag* self);

// tag - context
QNAPI void qn_mltag_add_context(qnMlTag* ptr, const char* cntx, int size);
QNAPI void qn_mltag_set_context(qnMlTag* ptr, const char* cntx, int size);

// tag - sub
QNAPI int qn_mltag_get_sub_count(qnMlTag* ptr);
QNAPI qnMlTag* qn_mltag_get_sub(qnMlTag* ptr, const char* name);
QNAPI qnMlTag* qn_mltag_get_sub_nth(qnMlTag* ptr, int at);
QNAPI const char* qn_mltag_get_sub_context(qnMlTag* ptr, const char* name, const char* ifnotexist);
QNAPI const char* qn_mltag_get_sub_context_nth(qnMlTag* ptr, int at, const char* ifnotexist);
QNAPI int qn_mltag_contains_sub(qnMlTag* ptr, qnMlTag* tag);
QNAPI qnMlTag* qn_mltag_add_sub(qnMlTag* ptr, const char* name, const char* context, int line);
QNAPI qnMlTag* qn_mltag_add_sub_tag(qnMlTag* ptr, qnMlTag* tag);
QNAPI int qn_mltag_remove_sub(qnMlTag* ptr, const char* name, bool isall);
QNAPI bool qn_mltag_remove_sub_nth(qnMlTag* ptr, int at);
QNAPI bool qn_mltag_remove_sub_tag(qnMlTag* ptr, qnMlTag* tag, bool isdelete);

QNAPI void qn_mltag_foreach_sub(qnMlTag* ptr, void(*func)(void* userdata, qnMlTag* tag), void* userdata);
QNAPI void qn_mltag_loopeach_sub(qnMlTag* ptr, void(*func)(qnMlTag* tag));

// tag - arg
QNAPI int qn_mltag_get_arity(qnMlTag* ptr);
QNAPI const char* qn_mltag_get_arg(qnMlTag* ptr, const char* name, const char* ifnotexist);
QNAPI bool qn_mltag_next_arg(qnMlTag* ptr, void** index, const char** name, const char** data);
QNAPI bool qn_mltag_contains_arg(qnMlTag* ptr, const char* name);

QNAPI void qn_mltag_foreach_arg(qnMlTag* ptr, void(*func)(void* userdata, const char* name, const char* data), void* userdata);
QNAPI void qn_mltag_loopeach_arg(qnMlTag* ptr, void(*func)(const char* name, const char* data));

QNAPI void qn_mltag_set_arg(qnMlTag* ptr, const char* name, const char* value);
QNAPI bool qn_mltag_remove_arg(qnMlTag* ptr, const char* name);


//////////////////////////////////////////////////////////////////////////
// qm(qn gam)

#define qvt_name(type)					struct _vt##type
#define qvt_cast(g,type)				((struct _vt##type*)((qnGam*)(g))->vt)
#define qm_cast(g,type)					((type*)(g))

qvt_name(qnGam)
{
	const char* name;
	void (*dispose)(void*);
};

typedef struct qnGam
{
	qvt_name(qnGam)*	vt;
	volatile nint		ref;
	nuint				desc;
} qnGam;

QNAPI void* qm_init(void* g, void* vt);
QNAPI void* qm_load(void* g);
QNAPI void* qm_unload(void* g);

QNAPI int qm_get_ref(void* g);
QNAPI nuint qm_get_desc(void* g);
QNAPI nuint qm_set_desc(void* g, nuint ptr);

QN_EXTC_END

#if _MSC_VER
#pragma warning(pop)
#endif
