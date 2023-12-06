#pragma once
// ReSharper disable CppUnusedIncludeDirective

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)		// L4, nonstandard extension used : nameless struct/union
#endif


//////////////////////////////////////////////////////////////////////////
// includes
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <wchar.h>
#ifdef __unix__
#include <sys/time.h>
#endif


//////////////////////////////////////////////////////////////////////////
// typedefs

// union
typedef union vint16_t					vint16_t, vshort;
typedef union vint32_t					vint32_t, vint;
typedef union vint64_t					vint64_t, vlong;
typedef union any_t						any_t;

// struct
typedef struct funcparam_t				funcparam_t;
typedef struct QnDateTime				QnDateTime;
typedef struct QnTimer					QnTimer;
typedef struct QnDir					QnDir;
typedef struct QnFile					QnFile;
typedef struct QnFileInfo				QnFileInfo;
typedef struct QnFileAccess				QnFileAccess;
typedef struct QnMlu					QnMlu;
typedef struct QnMlTag					QnMlTag;
typedef struct QmGam					QmGam;


//////////////////////////////////////////////////////////////////////////
// compiler configuration

// check
#if !defined _MSC_VER && !defined __GNUC__
#error unknown compiler! (support: MSVC, CLANG, GCC)
#endif

// platform
#ifdef _WIN32
#	define _QN_WINDOWS_					1
#endif

#if defined __FreeBSD__ || defined __OpenBSD__
#	define _QN_BSD_						1
#endif

#ifdef __linux__
#	define _QN_LINUX_					1
#endif

#ifdef __android__
#	define _QN_ANDROID_					1
#	define _QN_MOBILE_					1
#endif

#ifdef __unix__
#	define _QN_UNIX_					1
#endif

#if _WIN64 || __LP64__ || __amd64__ || __x86_64__ || __aarch64__
#	define _QN_64_						1
#endif

#ifdef _CHAR_UNSIGNED
#	error "Compiler option: char type must be signed"
#endif

// compiler specific
#ifdef _MSC_VER
#	define QN_RESTRICT					__restrict
#	define QN_INLINE					__inline
#	define QN_FORCE_LINE				__forceinline
#	define QN_FUNC_NAME					__FUNCTION__
#	define QN_MEM_BARRIER()				_ReadWriteBarrier()
#	define QN_STATIC_ASSERT				static_assert
#	define QN_FALL_THROUGH
#	define QN_ALIGN(x)					__declspec(align(x))
#elif defined __GNUC__
#	define QN_RESTRICT					restrict
#ifndef __cplusplus
#	define QN_INLINE					static inline
#	define QN_FORCE_LINE				static inline __attribute__ ((always_inline))
#else
#	define QN_INLINE					inline
#	define QN_FORCE_LINE				inline __attribute__ ((always_inline))
#endif
#	define QN_FUNC_NAME					__FUNCTION__	/* __PRETTY_FUNCTION__ */
#if __X86__ || _M_X86 || __amd64__ || __x86_64__
#	define QN_MEM_BARRIER()				__asm__ __volatile__("" : : : "memory");
#elif __aarch64__
#	define QN_MEM_BARRIER()				__asm__ __volatile__("dsb sy" : : : "memory")
#else
#	define QN_MEM_BARRIER()				__sync_synchronize();
#endif
#	define QN_STATIC_ASSERT				_Static_assert
#	define QN_FALL_THROUGH				__attribute__((fallthrough))
#	define QN_ALIGN(x)					__attribute__((aligned(x)))
#endif

#ifdef __cplusplus
#	define QN_EXTC						extern "C"
#	define QN_EXTC_BEGIN				extern "C" {
#	define QN_EXTC_END					}
#else
#	define QN_EXTC
#	define QN_EXTC_BEGIN
#	define QN_EXTC_END
#endif

// path separator
#ifdef _QN_WINDOWS_
#	define QN_PATH_SEP					'\\'
#else
#	define QN_PATH_SEP					'/'
#endif

QN_EXTC_BEGIN

//////////////////////////////////////////////////////////////////////////
// macro

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
#define QN_MASK(v,mask)					(((mask)==0)?(v):(v)&(mask))
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
typedef int8_t							sbyte;				/** @brief 8bit signed int */
typedef int64_t							llong;				/** @brief 64bit signed int */
typedef uint8_t							byte;				/** @brief 8bit unsigned int */
typedef uint16_t						ushort;				/** @brief 16bit unsigned int */
typedef uint32_t						uint;				/** @brief 32bit unsigned int */
typedef uint64_t						ullong;				/** @brief 64bit unsigned int */

typedef intptr_t						nint;				/** @brief platform pointer signed int */
typedef uintptr_t						nuint;				/** @brief platform pointer unsigned int */

typedef wchar_t							wchar;
#ifdef _QN_WINDOWS_
typedef uint32_t						uchar4;				/** @brief 4byte(32bit) unicode */
typedef wchar_t							uchar2;				/** @brief 2byte(16bit) unicode */
#else
typedef wchar_t							uchar4;				/** @brief 4byte(32bit) unicode */
typedef uint16_t						uchar2;				/** @brief 2byte(16bit) unicode */
#endif

typedef uint16_t						halfint;			/** @brief 16bit half int */

/** @brief variant 16bit short */
union vint16_t
{
	struct
	{
		byte			l, h;
	} b;
	ushort				w;
};

/** @brief variant 32bit int */
union vint32_t
{
	struct
	{
		byte			a, b, c, d; // d=msb
	} b;
	struct
	{
		ushort			l, h;
	} w;
	uint				dw;
};

/** @brief variant 64bit long long */
union vint64_t
{
	struct
	{
		byte			la, lb, lc, ld;
		byte			ha, hb, hc, hd; // hd=msb
	} b;
	struct
	{
		ushort			a, b, c, d;
	} w;
	struct
	{
		ushort			l, h;
	} dw;
	ullong			q;
};

/** @brief any value */
union any_t
{
	bool				b;
	int					i;
	uint				u;
	float				f;

	char*				s;
	wchar*				w;
	void*				p;
	const void*			cp;
	func_t				func;

	llong				ll;
	ullong				ull;
	double				dbl;

	byte				data[8];
};

/** @brief function parameter */
struct funcparam_t
{
	paramfunc_t			func;
	void*				data;
};

// cast
#define qn_ptrcast_int(v)				((int)(nint)(v))
#define qn_ptrcast_uint(v)				((uint)(nuint)(v))
#define qn_ptrcast_size(v)				((size_t)(nuint)(v))
#define qn_ptrcast_float(v)				(*(float*)&(v))
#define qn_castptr(v)					((nint)(v))
#define qn_castptr_float(v)				(*(nint*)&(v))
#ifdef _QN_64_
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

#ifdef _DEBUG
#define qn_assert(expr,mesg)			QN_STMT_BEGIN{if (!(expr)) qn_debug_assert(#expr, mesg, __FILE__, __LINE__);}QN_STMT_END
#define qn_verify(expr)					QN_STMT_BEGIN{if (!(expr)) qn_debug_assert(#expr, NULL, __FILE__, __LINE__);}QN_STMT_END
#else
#define qn_assert(expr,rs_mesg)			((void)0)
#define qn_verify(...)					((void)0)
#endif


//////////////////////////////////////////////////////////////////////////
// runtime
QSAPI void qn_runtime(int /*NULLABLE*/v[2]);
QSAPI void qn_atexit(paramfunc_t func, void* data);
QSAPI void qn_atexitp(paramfunc_t func, void* data);
QSAPI size_t qn_number(void);

QSAPI int qn_debug_outputs(bool breakpoint, const char* head, const char* mesg);
QSAPI int qn_debug_outputf(bool breakpoint, const char* head, const char* fmt, ...);
QSAPI int qn_debug_assert(const char* expr, const char* mesg, const char* filename, int line);
QSAPI void qn_debug_halt(const char* cls, const char* msg);

QSAPI int qn_outputs(const char* mesg);
QSAPI int qn_outputf(const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// memory
#define qn_alloc(cnt,type)				(type*)qn_mpfalloc((cnt)*sizeof(type), false, QN_FUNC_NAME, __LINE__)
#define qn_alloc_1(type)				(type*)qn_mpfalloc(sizeof(type), false, QN_FUNC_NAME, __LINE__)
#define qn_alloc_zero(cnt, type)		(type*)qn_mpfalloc((cnt)*sizeof(type), true, QN_FUNC_NAME, __LINE__)
#define qn_alloc_zero_1(type)			(type*)qn_mpfalloc(sizeof(type), true, QN_FUNC_NAME, __LINE__)
#define qn_realloc(ptr,cnt,type)		(type*)qn_mpfreloc((void*)(ptr), (cnt)*sizeof(type), QN_FUNC_NAME, __LINE__)

#define qn_free(ptr)					qn_mpffree((void*)(ptr))
#define qn_free_ptr(pptr)				QN_STMT_BEGIN{ qn_mpffree((void*)*(pptr)); *(pptr)=NULL; }QN_STMT_END

#ifdef _MSC_VER
#define qn_alloca(cnt,type)				(type*)_malloca((cnt)*sizeof(type))
#define qn_freea(ptr)					_freea(ptr)
#else
#define qn_alloca(cnt,type)				(type*)alloca((cnt)*sizeof(type))
#define qn_freea(ptr)					((void)0)
#endif

#define qn_zero(ptr,cnt,type)			memset((ptr), 0, sizeof(type)*(cnt))
#define qn_zero_1(ptr)					memset((ptr), 0, sizeof(*(ptr)))

QSAPI void* qn_memenc(void* dest, const void* src, size_t size);
QSAPI void* qn_memdec(void* dest, const void* src, size_t size);
QSAPI void* qn_memzcpr(const void* src, size_t srcsize, /*NULLABLE*/size_t* destsize);
QSAPI void* qn_memzucp(const void* src, size_t srcsize, size_t bufsize, /*NULLABLE*/size_t* destsize);
QSAPI size_t qn_memagn(size_t size);
QSAPI char qn_memhrb(size_t size, double* out);
QSAPI char* qn_memdmp(const void* ptr, size_t size, char* outbuf, size_t buflen);

QSAPI size_t qn_mpfsize(void);
QSAPI size_t qn_mpfcnt(void);
QSAPI void* qn_mpfalloc(size_t size, bool zero, const char* desc, size_t line);
QSAPI void* qn_mpfreloc(void* ptr, size_t size, const char* desc, size_t line);
QSAPI void qn_mpffree(void* ptr);


//////////////////////////////////////////////////////////////////////////
// hash & sort
QSAPI size_t qn_hashptr(const void* p);
QSAPI size_t qn_hashnow(void);
QSAPI size_t qn_hashfn(int prime8, func_t func, const void* data);
QSAPI size_t qn_hashcrc(const byte* data, size_t size);
QSAPI uint32_t qn_primenear(uint32_t value);
QSAPI uint32_t qn_primeshift(uint32_t value, uint32_t min, uint32_t* shift);

QSAPI void qn_qsort(void* ptr, size_t count, size_t stride, int(*compfunc)(const void*, const void*));
QSAPI void qn_qsortc(void* ptr, size_t count, size_t stride, int(*compfunc)(void*, const void*, const void*), void* context);


//////////////////////////////////////////////////////////////////////////
// string
QSAPI int qn_vsnprintf(char* out, size_t len, const char* fmt, va_list va);
QSAPI int qn_vasprintf(char** out, const char* fmt, va_list va);
QSAPI char* qn_vapsprintf(const char* fmt, va_list va);
QSAPI int qn_snprintf(char* out, size_t len, const char* fmt, ...);
QSAPI int qn_asprintf(char** out, const char* fmt, ...);
QSAPI char* qn_apsprintf(const char* fmt, ...);
QSAPI size_t qn_strfll(char* dest, size_t pos, size_t end, int ch);
QSAPI size_t qn_strhash(const char* p);
QSAPI size_t qn_strihash(const char* p);
QSAPI const char* qn_strbrk(const char* p, const char* c);
QSAPI char* qn_strmid(char* dest, size_t destsize, const char* src, size_t pos, size_t len);
QSAPI char* qn_strltm(char* dest);
QSAPI char* qn_strrtm(char* dest);
QSAPI char* qn_strtrm(char* dest);
QSAPI char* qn_strrem(char* p, const char* rmlist);
QSAPI char* qn_strpcpy(char* dest, const char* src);
QSAPI char* qn_strcat(const char* p, ...);
QSAPI int qn_strwcm(const char* string, const char* wild);
QSAPI int qn_striwcm(const char* string, const char* wild);
QSAPI int qn_strfnd(const char* src, const char* find, size_t index);
#ifdef _MSC_VER
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
QSAPI char* qn_strupr(char* p, size_t size);
QSAPI char* qn_strlwr(char* p, size_t size);
QSAPI int qn_strncmp(const char* p1, const char* p2, size_t len);
QSAPI int qn_stricmp(const char* p1, const char* p2);
QSAPI int qn_strnicmp(const char* p1, const char* p2, size_t len);
#endif

QSAPI int qn_vsnwprintf(wchar* out, size_t len, const wchar* fmt, va_list va);
QSAPI int qn_vaswprintf(wchar** out, const wchar* fmt, va_list va);
QSAPI wchar* qn_vapswprintf(const wchar* fmt, va_list va);
QSAPI int qn_snwprintf(wchar* out, size_t len, const wchar* fmt, ...);
QSAPI int qn_aswprintf(wchar** out, const wchar* fmt, ...);
QSAPI wchar* qn_apswprintf(const wchar* fmt, ...);
QSAPI size_t qn_wcsfll(wchar* dest, size_t pos, size_t end, int ch);
QSAPI size_t qn_wcshash(const wchar* p);
QSAPI size_t qn_wcsihash(const wchar* p);
QSAPI const wchar* qn_wcsbrk(const wchar* p, const wchar* c);
QSAPI wchar* qn_wcsmid(wchar* dest, size_t destsize, const wchar* src, size_t pos, size_t len);
QSAPI wchar* qn_wcsltm(wchar* dest);
QSAPI wchar* qn_wcsrtm(wchar* dest);
QSAPI wchar* qn_wcstrm(wchar* dest);
QSAPI wchar* qn_wcsrem(wchar* p, const wchar* rmlist);
QSAPI wchar* qn_wcspcpy(wchar* dest, const wchar* src);
QSAPI wchar* qn_wcscat(const wchar* p, ...);
QSAPI int qn_wcswcm(const wchar* string, const wchar* wild);
QSAPI int qn_wcsiwcm(const wchar* string, const wchar* wild);
QSAPI int qn_wcsfnd(const wchar* src, const wchar* find, size_t index);
#ifdef _MSC_VER
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
QSAPI wchar* qn_wcsupr(wchar* p, size_t size);
QSAPI wchar* qn_wcslwr(wchar* p, size_t size);
QSAPI int qn_wcsncmp(const wchar* p1, const wchar* p2, size_t len);
QSAPI int qn_wcsicmp(const wchar* p1, const wchar* p2);
QSAPI int qn_wcsnicmp(const wchar* p1, const wchar* p2, size_t len);
#endif

QSAPI size_t qn_u8len(const char* s);
QSAPI uchar4 qn_u8cbn(const char* p);
QSAPI const char* qn_u8nch(const char* s);
QSAPI int qn_u8ucb(uchar4 c, char* out);

QSAPI size_t qn_mbstowcs(wchar* outwcs, size_t outsize, const char* inmbs, size_t insize);
QSAPI size_t qn_wcstombs(char* outmbs, size_t outsize, const wchar* inwcs, size_t insize);
QSAPI size_t qn_u8to32(uchar4* dest, size_t destsize, const char* src, size_t srclen);
QSAPI size_t qn_u8to16(uchar2* dest, size_t destsize, const char* src, size_t srclen);
QSAPI size_t qn_u32to8(char* dest, size_t destsize, const uchar4* src, size_t srclen);
QSAPI size_t qn_u16to8(char* dest, size_t destsize, const uchar2* src, size_t srclen);
QSAPI size_t qn_u16to32(uchar4* dest, size_t destsize, const uchar2* src, size_t srclen);
QSAPI size_t qn_u32to16(uchar2* dest, size_t destsize, const uchar4* src, size_t srclen);


//////////////////////////////////////////////////////////////////////////
// time

QSAPI void qn_localtime(struct tm* ptm, time_t tt);
QSAPI void qn_gmtime(struct tm* ptm, time_t tt);

QSAPI ullong qn_cycle(void);
QSAPI ullong qn_tick(void);
QSAPI double qn_stick(void);

QSAPI void qn_sleep(uint milliseconds);
QSAPI void qn_usleep(uint microseconds);
QSAPI void qn_ssleep(uint seconds);
QSAPI void qn_msleep(ullong microseconds);

/** @brief date time */
struct QnDateTime
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
};

QSAPI void qn_now(QnDateTime* dt);
QSAPI void qn_utc(QnDateTime* dt);
QSAPI void qn_stod(double sec, QnDateTime* dt);
QSAPI void qn_mstod(uint msec, QnDateTime* dt);

/** @brief timer */
struct QnTimer
{
	double				abstime;
	double				runtime;
	double				advance;
	double				fps;
};

QSAPI QnTimer* qn_timer_new(void);
QSAPI void qn_timer_delete(QnTimer* self);
QSAPI void qn_timer_reset(QnTimer* self);
QSAPI void qn_timer_start(QnTimer* self);
QSAPI void qn_timer_stop(QnTimer* self);
QSAPI bool qn_timer_update(QnTimer* self);
QSAPI double qn_timer_get_abs(const QnTimer* self);
QSAPI double qn_timer_get_run(const QnTimer* self);
QSAPI double qn_timer_get_fps(const QnTimer* self);
QSAPI double qn_timer_get_adv(const QnTimer* self);
QSAPI double qn_timer_get_cut(const QnTimer* self);
QSAPI void qn_timer_set_cut(QnTimer* self, double cut);
QSAPI void qn_timer_set_manual(QnTimer* self, bool value);


//////////////////////////////////////////////////////////////////////////
// disk i/o

/** @brief file info */
struct QnFileInfo
{
	short				type;
	ushort				len;
	llong				size;
	llong				cmpr;
	ullong				stc;			// creation stamp
	ullong				stw;			// last written stamp
	char				name[256];
};

/** @brief file access */
struct QnFileAccess
{
#ifdef _MSC_VER
	uint				mode;
	uint				share;
	uint				access;
	uint				attr;
#else
	int					mode;
	int					access;
#endif
};

/** @brief seek */
enum QnSeek
{
	QNSEEK_BEGIN = 0,
	QNSEEK_CUR = 1,
	QNSEEK_END = 2,
};
typedef enum QnSeek						QnSeek;

/** @brief file flag */
enum QnFileFlag
{
	QNFF_READ = 0x1,
	QNFF_WRITE = 0x2,
	QNFF_SEEK = 0x4,
	QNFF_ALL = QNFF_READ | QNFF_WRITE | QNFF_SEEK,
	QNFF_RDONLY = QNFF_READ | QNFF_SEEK,
};
typedef enum QnFileFlag					QnFileFlag;

// file
QSAPI QnFile* qn_file_new(const char* filename, const char* mode);
QSAPI QnFile* qn_file_new_dup(QnFile* org);
QSAPI void qn_file_delete(QnFile* self);
QSAPI int qn_file_get_flags(const QnFile* self, int mask);
QSAPI const char* qn_file_get_name(const QnFile* self);
QSAPI int qn_file_read(QnFile* self, void* buffer, int offset, int size);
QSAPI int qn_file_write(QnFile* self, const void* buffer, int offset, int size);
QSAPI int64_t qn_file_size(QnFile* self);
QSAPI int64_t qn_file_tell(QnFile* self);
QSAPI int64_t qn_file_seek(QnFile* self, llong offset, int org);
QSAPI bool qn_file_flush(const QnFile* self);
QSAPI int qn_file_printf(QnFile* self, const char* fmt, ...);
QSAPI int qn_file_vprintf(QnFile* self, const char* fmt, va_list va);
QSAPI bool qn_file_exist(const char* filename, /*RET-NULLABLE*/bool* isdir);
QSAPI void* qn_file_alloc(const char* filename, int* size);
QSAPI QnFile* qn_file_new_l(const wchar* filename, const wchar* mode);
QSAPI bool qn_file_exist_l(const wchar* filename, /*RET-NULLABLE*/bool* isdir);
QSAPI void* qn_file_alloc_l(const wchar* filename, int* size);
QSAPI size_t qn_file_get_max_alloc_size(void);
QSAPI void qn_file_set_max_alloc_size(size_t n);

// directory
QSAPI QnDir* qn_dir_new(const char* path);
QSAPI void qn_dir_delete(QnDir* self);
QSAPI const char* qn_dir_read(QnDir* self);
QSAPI void qn_dir_rewind(QnDir* self);
QSAPI void qn_dir_seek(QnDir* self, int pos);
QSAPI int qn_dir_tell(QnDir* self);
QSAPI QnDir* qn_dir_new_l(const wchar* path);
QSAPI const wchar_t* qn_dir_read_l(QnDir* self);


//////////////////////////////////////////////////////////////////////////
// xml

struct QnMlTag
{
	char*				name;
	char*				cntx;

	int					nlen;
	int					clen;
	int					line;
};

// ml unit
QSAPI QnMlu* qn_mlu_new(void);
QSAPI QnMlu* qn_mlu_new_file(const char* filename);
QSAPI QnMlu* qn_mlu_new_file_l(const wchar* filename);
QSAPI QnMlu* qn_mlu_new_buffer(const void* data, int size);
QSAPI void qn_mlu_delete(QnMlu* self);

QSAPI void qn_mlu_clean_tags(QnMlu* self);
QSAPI void qn_mlu_clean_errs(QnMlu* self);
QSAPI bool qn_mlu_load_buffer(QnMlu* self, const void* data, int size);
QSAPI bool qn_mlu_write_file(const QnMlu* self, const char* filename);

QSAPI int qn_mlu_get_count(const QnMlu* self);
QSAPI const char* qn_mlu_get_err(const QnMlu* self, int at);
QSAPI QnMlTag* qn_mlu_get_tag(const QnMlu* self, const char* name);
QSAPI QnMlTag* qn_mlu_get_tag_nth(const QnMlu* self, int at);
QSAPI const char* qn_mlu_get_context(const QnMlu* self, const char* name, const char* ifnotexist);
QSAPI const char* qn_mlu_get_context_nth(const QnMlu* self, int at, const char* ifnotexist);
QSAPI int qn_mlu_contains(const QnMlu* self, QnMlTag* tag);

QSAPI void qn_mlu_foreach(const QnMlu* self, void(*func)(void*, QnMlTag*), void* userdata);
QSAPI void qn_mlu_loopeach(const QnMlu* self, void(*func)(QnMlTag* tag));

QSAPI QnMlTag* qn_mlu_add(QnMlu* self, const char* name, const char* context, int line);
QSAPI QnMlTag* qn_mlu_add_tag(QnMlu* self, QnMlTag* tag);
QSAPI int qn_mlu_remove(QnMlu* self, const char* name, bool isall);
QSAPI bool qn_mlu_remove_nth(QnMlu* self, int at);
QSAPI bool qn_mlu_remove_tag(QnMlu* self, QnMlTag* tag, bool isdelete);

QSAPI void qn_mlu_add_err(QnMlu* self, const char* msg);
QSAPI void qn_mlu_add_errf(QnMlu* self, const char* fmt, ...);
QSAPI void qn_mlu_print_err(const QnMlu* self);

QSAPI void qn_mlu_print(QnMlu* self);

// tag
QSAPI QnMlTag* qn_mltag_new(const char* name);
QSAPI void qn_mltag_delete(QnMlTag* self);

// tag - context
QSAPI void qn_mltag_add_context(QnMlTag* ptr, const char* cntx, int size);
QSAPI void qn_mltag_set_context(QnMlTag* ptr, const char* cntx, int size);

// tag - sub
QSAPI int qn_mltag_get_sub_count(QnMlTag* ptr);
QSAPI QnMlTag* qn_mltag_get_sub(QnMlTag* ptr, const char* name);
QSAPI QnMlTag* qn_mltag_get_sub_nth(QnMlTag* ptr, int at);
QSAPI const char* qn_mltag_get_sub_context(QnMlTag* ptr, const char* name, const char* ifnotexist);
QSAPI const char* qn_mltag_get_sub_context_nth(QnMlTag* ptr, int at, const char* ifnotexist);
QSAPI int qn_mltag_contains_sub(QnMlTag* ptr, QnMlTag* tag);
QSAPI QnMlTag* qn_mltag_add_sub(QnMlTag* ptr, const char* name, const char* context, int line);
QSAPI QnMlTag* qn_mltag_add_sub_tag(QnMlTag* ptr, QnMlTag* tag);
QSAPI int qn_mltag_remove_sub(QnMlTag* ptr, const char* name, bool isall);
QSAPI bool qn_mltag_remove_sub_nth(QnMlTag* ptr, int at);
QSAPI bool qn_mltag_remove_sub_tag(QnMlTag* ptr, QnMlTag* tag, bool isdelete);

QSAPI void qn_mltag_foreach_sub(QnMlTag* ptr, void(*func)(void* userdata, QnMlTag* tag), void* userdata);
QSAPI void qn_mltag_loopeach_sub(QnMlTag* ptr, void(*func)(QnMlTag* tag));

// tag - arg
QSAPI int qn_mltag_get_arity(QnMlTag* ptr);
QSAPI const char* qn_mltag_get_arg(QnMlTag* ptr, const char* name, const char* ifnotexist);
QSAPI bool qn_mltag_next_arg(QnMlTag* ptr, void** index, const char** name, const char** data);
QSAPI bool qn_mltag_contains_arg(QnMlTag* ptr, const char* name);

QSAPI void qn_mltag_foreach_arg(QnMlTag* ptr, void(*func)(void* userdata, const char* name, const char* data), void* userdata);
QSAPI void qn_mltag_loopeach_arg(QnMlTag* ptr, void(*func)(const char* name, const char* data));

QSAPI void qn_mltag_set_arg(QnMlTag* ptr, const char* name, const char* value);
QSAPI bool qn_mltag_remove_arg(QnMlTag* ptr, const char* name);


//////////////////////////////////////////////////////////////////////////
// qm(qn gam)

#define qvt_name(type)					struct _vt_##type
#define qvt_cast(g,type)				((struct _vt_##type*)((QmGam*)(g))->vt)

#define qm_cast(g,type)					((type*)(g))

qvt_name(QmGam)
{
	const char* name;
	void (*dispose)(QmGam*);
};

struct QmGam
{
	qvt_name(QmGam)*	vt;
	volatile nint		ref;
	nuint				desc;
};

QSAPI QmGam* qm_stc_init(QmGam* g, void* vt);
QSAPI QmGam* qm_stc_load(QmGam* g);
QSAPI QmGam* qm_stc_unload(QmGam* g);
QSAPI size_t qm_stc_get_ref(QmGam* g);
QSAPI nuint qm_stc_get_desc(const QmGam* g);
QSAPI nuint qm_stc_set_desc(QmGam* g, nuint ptr);

/**
 * @brief init gam vtable
 * @param g gam object
 * @param type type of g
 * @param vt vtable
 * @return g self
 */
#define qm_init(g,type,vt)				((type*)qm_stc_init((QmGam*)(g), vt))
/**
 * @brief load reference
 * @param g gam object
 * @return g self
 */
#define qm_load(g)						((g) ? qm_stc_load((QmGam*)(g)) : NULL)
/**
 * @brief unload reference. if reference is zero, will be disposing
 * @param g gam object
 * @return g self
 */
#define qm_unload(g)					((g) ? qm_stc_unload((QmGam*)(g)) : NULL)
/**
 * @brief return reference
 * @param g gam object
 * @return reference value
 */
#define qm_get_ref(g)					qm_stc_get_ref((QmGam*)(g))
/**
 * @brief return description value
 * @param g gam object
 * @param type return type
 * @return description value
 */
#define qm_get_desc(g,type)				(type)qm_stc_get_desc((QmGam*)(g))
/**
 * @brief set description value
 * @param g gam object
 * @param ptr description(nint)
 * @return previous description value
 */
#define qm_set_desc(g,ptr)				qm_stc_set_desc((QmGam*)(g),(nuint)(ptr))

QN_EXTC_END

#ifdef _MSC_VER
#pragma warning(pop)
#endif

