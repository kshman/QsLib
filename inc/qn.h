#pragma once

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
// compiler dependancies

// basic definition
#if _MSC_VER
#	define QN_ALIGN(x)					__declspec(align(x))
#	define QN_SELECTANY					__declspec(selectany)
#	define QN_FORCELINE					__forceinline
#	define QN_INLINE					__inline
#elif __GNUC__
#	define QN_ALIGN(x)					__attribute__ ((aligned(x)))
#	define QN_SELECTANY					/*__attribute__ ((selectany))*/
#ifndef __cplusplus
#	define QN_FORCELINE					static inline __attribute__ ((always_inline))
#	define QN_INLINE					static inline
#else
#	define QN_FORCELINE					inline __attribute__ ((always_inline))
#	define QN_INLINE					inline
#endif
#else
#	define QN_ALIGN(x)
#	define QN_SELECTANY
#ifndef __cplusplus
#	define QN_FORCELINE					static inline
#	define QN_INLINE					static inline
#else
#	define QN_FORCELINE					inline
#	define QN_INLINE					inline
#endif
#endif

// function
#if _MSC_VER
#	define QN_FNAME						__FUNCTION__
#elif __STDC_VERSION__ >= 199901L
#	define QN_FNAME						__func__
#elif __GNUC__
#	define QN_FNAME						__PRETTY_FUNCTION__
#else
#	define QN_FNAME						"<unknown>"
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


//////////////////////////////////////////////////////////////////////////
// includes
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#include <signal.h>
#if _QN_BSD_ || _QN_LINUX_
#include <sys/time.h>
#if _QN_LINUX_
#include <linux/limits.h>
#endif
#endif


//////////////////////////////////////////////////////////////////////////
// definition

QN_EXTC_BEGIN

#ifndef FALSE
#define FALSE							0
#endif

#ifndef TRUE
#define TRUE							1
#endif

#define QN_NOUSE(v)						(void)(v)
#define QN_GET_MACRO_3(_1,_2,_3,N, ...)	N

// compond
#define QN_STMT_BEGIN					do
#define QN_STMT_END						while(0)

// string & array & structure && etc
#define _QN_CONCAT_2(x,y)				x##y
#define _QN_CONCAT_3(x,y,z)				x##y##z
#define _QN_STRING(x)					#x
#define _QN_UNICODE(x)					L##x

#define QN_STRING(x)					_QN_STRING(x)
#define QN_UNICODE(x)					_QN_UNICODE(x)
#define QN_CONCAT_2(x,y)				_QN_CONCAT_2(x, y)
#define QN_CONCAT_3(x,y,z)				_QN_CONCAT_3(x, y, z)
#define QN_CONCAT(...)					QN_GET_MACRO_3(__VA_ARGS__, QN_CONCAT_3, QN_CONCAT_2)(__VA_ARGS__)
#define QN_BIT(n)						(1<<(n))

#define QN_COUNTOF(arr)					(sizeof(arr)/sizeof((arr)[0]))
#define QN_OFFSETOF(type,mem)			((size_t)((char*)&((type*)0)->mem))
#define QN_MEMBEROF(type,p,off)			(*(type*)((void*)((char*)(p)+(size_t)(offset))))

// basic macro function
#define QN_MAX(a,b)						(((a)>(b)) ? (a) : (b))
#define QN_MIN(a,b)						(((a)<(b)) ? (a) : (b))
#define QN_ABS(v)						(((v)<0) ? -(v) : (v))
#define QN_CLAMP(v,min,max)				((v)<(min) ? (min) : (v)>(max) ? (max) : (v))

// range
#define QN_MIN_HASH						11
#define QN_MAX_HASH						13845163
#define QN_MAX_RAND						0x7FFF
#define QN_INVALID_VALUE				(-1)
#define QN_INVALID_SIZE					((size_t)QN_INVALID_VALUE)

// constant
#define QN_CONST_EPSILON				0.0001
#define QN_CONST_E						2.7182818284590452353602874713526624977572470937000
#define QN_CONST_LOG2E					1.44269504088896340736
#define QN_CONST_LOG10E					0.434294481903251827651
#define QN_CONST_LOG2B10				0.30102999566398119521
#define QN_CONST_LN2					0.6931471805599453094172321214581765680755001343603
#define QN_CONST_LN10					2.3025850929940456840179914546843642076011014886288
#define QN_CONST_PI						3.1415926535897932384626433832795028841971693993751
#define QN_CONST_PI2					6.2831853071795864769252867665590057683943387987502
#define QN_CONST_PI_2					1.5707963267948966192313216916397514420985846996876
#define QN_CONST_PI_4					0.7853981633974483096156608458198757210492923498438
#define QN_CONST_SQRT2					1.4142135623730950488016887242096980785696718753769
#define QN_CONST_SQRT1_2				0.7071067811865475244008443621048490392848359376884
#define QN_CONST_USEC_PER_SEC			1000000
#define QN_CONST_NSEC_PER_SEC			1000000000

#ifdef PATH_MAX
#define QN_MAX_PATH						PATH_MAX
#else
#define QN_MAX_PATH						1024
#endif


//////////////////////////////////////////////////////////////////////////
// types

// handle
typedef void*							pointer_t;

// unicode
#if _MSC_VER
typedef uint32_t						uchar4_t;
typedef wchar_t							uchar2_t;
#elif _SB_UNIX_
typedef wchar_t							uchar4_t;
typedef uint16_t						uchar2_t;
#else
typedef uint32_t						uchar4_t;
typedef uint16_t						uchar2_t;
#endif

// bool
#if !__cplusplus && (_MSC_VER || __STDC_VERSION__ < 201710L) && !defined(bool)
typedef _Bool							bool;
#endif

// specific
typedef uint16_t						half_t;
typedef void (*func_t)(void);

// 
typedef union vshort_t
{
	struct
	{
		uint8_t			l, h;
	} b;
	uint16_t			w;
} vshort_t;

//
typedef union vint_t
{
	struct
	{
		uint8_t			a, b, c, d; // d=msb
	} b;
	struct
	{
		uint16_t		l, h;
	} w;
	uint32_t				dw;
} vint_t;

//
typedef union vlong_t
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
} vlong_t;

//
typedef union any_t
{
	bool				b;
	int32_t				i;
	uint32_t			u;
	float				f;

	char*				s;
	wchar_t*			w;
	pointer_t			p;
	const pointer_t		cp;
	func_t				func;

	int64_t				i64;
	uint64_t			u64;
	double				dbl;

	uint8_t				data[8];
} any_t;

//
typedef struct funcparam_t
{
	func_t				func;
	pointer_t			data;
} funcparam_t;

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
QNAPI void qn_runtime(int /*nullable*/v[]);
QNAPI _Noreturn void qn_exit(const int exitcode);
QNAPI void qn_atexit(void(*func)(pointer_t), pointer_t data);
QNAPI void qn_atexit0(void(*func)(void));
QNAPI void qn_atexitp(void(*func)(pointer_t), pointer_t data);

QNAPI int qn_debug_assert(const char* expr, const char* mesg, const char* filename, int line);
QNAPI int qn_debug_halt(const char* cls, const char* msg);

//////////////////////////////////////////////////////////////////////////
// memory
#define qn_alloc(cnt,type)				(type*)malloc((cnt)*sizeof(type))
#define qn_alloc_1(type)				(type*)malloc(sizeof(type))
#define qn_alloc_zero(cnt, type)		(type*)calloc(cnt, sizeof(type))
#define qn_alloc_zero_1(type)			(type*)calloc(1, sizeof(type))
#define qn_realloc(ptr,cnt,type)		(type*)realloc((pointer_t)(ptr), (cnt)*sizeof(type))
#define qn_free(ptr)					free((pointer_t)(ptr))
#define qn_free_ptr(pptr)				QN_STMT_BEGIN{ free((pointer_t)*(pptr)); *(pptr)=NULL; }QN_STMT_END
#define qn_zero(ptr,cnt,type)			memset((ptr), 0, sizeof(type)*(cnt))
#define qn_zero_1(ptr)					memset((ptr), 0, sizeof(*(ptr)))
#if _MSC_VER
#define qn_alloca(cnt,type)				(type*)_malloca((cnt)*sizeof(type))
#define qn_freea(ptr)					_freea(ptr)
#else
#define qn_alloca(cnt,type)				(type*)_alloca((cnt)*sizeof(type))
#define qn_freea(ptr)					((void)0)
#endif

QNAPI pointer_t qn_memenc(pointer_t dest, const pointer_t src, size_t size);
QNAPI pointer_t qn_memdec(pointer_t dest, const pointer_t src, size_t size);
QNAPI pointer_t qn_memzcpr(const pointer_t src, size_t srcsize, /*RET_NULLABLE*/size_t* destsize);
QNAPI pointer_t qn_memzucp(const pointer_t src, size_t srcsize, size_t bufsize, /*RET_NULLABLE*/size_t* destsize);
QNAPI size_t qn_memagn(size_t size);
QNAPI char qn_memhrd(size_t size, double* out);
QNAPI char* qn_memdmp(const pointer_t ptr, size_t size, char* outbuf, size_t buflen);

//////////////////////////////////////////////////////////////////////////
// hash & sort
QNAPI size_t qn_hashptr(const pointer_t p);
QNAPI size_t qn_hashtime(void);
QNAPI size_t qn_hashfunc(int32_t prime8, func_t func, pointer_t data);
QNAPI size_t qn_hashdata(const uint8_t* data, size_t size);
QNAPI uint32_t qn_primenear(uint32_t value);
QNAPI uint32_t qn_primeshift(uint32_t value, uint32_t min, uint32_t* shift);
QNAPI void qn_qsort(pointer_t ptr, size_t count, size_t stride, int(*compfunc)(const pointer_t, const pointer_t));
QNAPI void qn_qsort_context(pointer_t ptr, size_t count, size_t stride, int(*compfunc)(pointer_t, const pointer_t, const pointer_t), pointer_t context);

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

/** date time. */
typedef struct qnDateTime
{
	union
	{
		uint64_t		stamp;
		struct
		{
			uint32_t	date;
			uint32_t	time;
		};
		struct
		{
			uint32_t	year : 14;
			uint32_t	month : 6;
			uint32_t	day : 8;
			uint32_t	dow : 4;

			uint32_t	hour : 6;
			uint32_t	minute : 8;
			uint32_t	second : 8;
			uint32_t	millisecond : 10;
		};
	};
} qnDateTime;

QNAPI void qn_now(qnDateTime* dt);
QNAPI void qn_utc(qnDateTime* dt);
QNAPI void qn_stod(double sec, qnDateTime* dt);
QNAPI void qn_mstod(uint32_t msec, qnDateTime* dt);

/** timer. */
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
QNAPI char* qn_strltrim(char* dest);
QNAPI char* qn_strrtrim(char* dest);
QNAPI char* qn_strtrim(char* dest);
QNAPI char* qn_strrem(char* p, const char* rmlist);
QNAPI char* qn_strcat(const char* p, ...);

QNAPI int qn_vsnwprintf(wchar_t* out, size_t len, const wchar_t* fmt, va_list va);
QNAPI int qn_vaswprintf(wchar_t** out, const wchar_t* fmt, va_list va);
QNAPI wchar_t* qn_vapswprintf(const wchar_t* fmt, va_list va);
QNAPI int qn_snwprintf(wchar_t* out, size_t len, const wchar_t* fmt, ...);
QNAPI int qn_aswprintf(wchar_t** out, const wchar_t* fmt, ...);
QNAPI wchar_t* qn_apswprintf(const wchar_t* fmt, ...);
QNAPI size_t qn_wcsfll(wchar_t* dest, size_t pos, size_t end, int ch);
QNAPI size_t qn_wcshash(const wchar_t* p);
QNAPI size_t qn_wcsihash(const wchar_t* p);
QNAPI wchar_t* qn_wcsbrk(const wchar_t* p, const wchar_t* c);
QNAPI wchar_t* qn_wcsmid(wchar_t* dest, size_t destsize, const wchar_t* src, size_t pos, size_t len);
QNAPI wchar_t* qn_wcsltrim(wchar_t* dest);
QNAPI wchar_t* qn_wcsrtrim(wchar_t* dest);
QNAPI wchar_t* qn_wcstrim(wchar_t* dest);
QNAPI wchar_t* qn_wcsrem(wchar_t* p, const wchar_t* rmlist);
QNAPI wchar_t* qn_wcscat(const wchar_t* p, ...);

QNAPI size_t qn_utf8len(const char* s);
QNAPI uchar4_t qn_utf8ccnc(const char* p);
QNAPI char* qn_utf8chn(const char* s);
QNAPI int qn_utf8cunc(uchar4_t c, char* out);

QNAPI size_t qn_mbstowcs(wchar_t* outwcs, size_t outsize, const char* inmbs, size_t insize);
QNAPI size_t qn_wcstombs(char* outmbs, size_t outsize, const wchar_t* inwcs, size_t insize);
QNAPI size_t qn_utf8to32(uchar4_t* dest, size_t destsize, const char* src, size_t srclen);
QNAPI size_t qn_utf8to16(uchar2_t* dest, size_t destsize, const char* src, size_t srclen);
QNAPI size_t qn_utf32to8(char* dest, size_t destsize, const uchar4_t* src, size_t srclen);
QNAPI size_t qn_utf16to8(char* dest, size_t destsize, const uchar2_t* src, size_t srclen);
QNAPI size_t qn_utf16to32(uchar4_t* dest, size_t destsize, const uchar2_t* src, size_t srclen);
QNAPI size_t qn_utf32to16(uchar2_t* dest, size_t destsize, const uchar4_t* src, size_t srclen);


//////////////////////////////////////////////////////////////////////////
// i/o

/** file io. */
typedef struct qnIoFuncDesc
{
	int(*read)(pointer_t handle, pointer_t buffer, int offset, int size);
	int(*write)(pointer_t handle, const pointer_t buffer, int offset, int size);
	int64_t(*tell)(pointer_t handle);
	int64_t(*seek)(pointer_t handle, int64_t offset, int org);
} qnIoFuncDesc;


//////////////////////////////////////////////////////////////////////////
// disk i/o

/** directory. */
typedef struct qnDir qnDir;

/** file. */
typedef struct qnFile qnFile;

/** file info. */
typedef struct qnFileInfo
{
	int16_t				type;
	uint16_t			len;
	int64_t				size;
	int64_t				cmpr;
	uint64_t			stc;			// creation stamp
	uint64_t			stw;			// last written stamp
	char				name[256];
} qnFileInfo;

/** file access */
typedef struct qnFileAccess
{
#if _QN_WINDOWS_
	uint32_t			mode;
	uint32_t			share;
	uint32_t			access;
	uint32_t			attr;
#else
	int					mode;
	int					access;
#endif
} qnFileAccess;

// seek
typedef enum qnSeek
{
	QN_SEEK_BEGIN	= 0,
	QN_SEEK_CUR		= 1,
	QN_SEEK_END		= 2,
} qnSeek;

// file flag
typedef enum qnFileFlag
{
	QN_FF_READ		= 0x1,
	QN_FF_WRITE		= 0x2,
	QN_FF_SEEK		= 0x4,
	QN_FF_ALL		= QN_FF_READ | QN_FF_WRITE | QN_FF_SEEK,
	QN_FF_RDONLY	= QN_FF_READ | QN_FF_SEEK,
} qnFileFlag;

// file
QNAPI qnFile* qn_file_new(const char* filename, const char* mode);
QNAPI qnFile* qn_file_new_dup(qnFile* org);
QNAPI void qn_file_delete(qnFile* self);
QNAPI int qn_file_flags(qnFile* self, int mask);
QNAPI const char* qn_file_name(qnFile* self);
QNAPI int qn_file_read(qnFile* self, pointer_t buffer, int offset, int size);
QNAPI int qn_file_write(qnFile* self, const pointer_t buffer, int offset, int size);
QNAPI int64_t qn_file_size(qnFile* self);
QNAPI int64_t qn_file_tell(qnFile* self);
QNAPI int64_t qn_file_seek(qnFile* self, int64_t offset, int org);
QNAPI bool qn_file_flush(qnFile* self);
QNAPI int qn_file_printf(qnFile* self, const char* fmt, ...);
QNAPI int qn_file_vprintf(qnFile* self, const char* fmt, va_list va);
QNAPI bool qn_file_exist(const char* filename, /*RET-NULLABLE*/bool* isdir);
QNAPI pointer_t qn_file_alloc(const char* filename, int* size);
QNAPI qnFile* qn_file_new_l(const wchar_t* filename, const wchar_t* mode);
QNAPI bool qn_file_exist_l(const wchar_t* filename, /*RET-NULLABLE*/bool* isdir);
QNAPI pointer_t qn_file_alloc_l(const wchar_t* filename, int* size);
QNAPI size_t qn_file_get_max_alloc_size(void);
QNAPI void qn_file_set_max_alloc_size(size_t n);
QNAPI void qn_file_access_parse(const char* mode, qnFileAccess* self, int* flag);
QNAPI void qn_file_access_parse_l(const wchar_t* mode, qnFileAccess* self, int* flag);

// directory
QNAPI qnDir* qn_dir_new(const char* path);
QNAPI void qn_dir_delete(qnDir* self);
QNAPI const char* qn_dir_read(qnDir* self);
QNAPI void qn_dir_rewind(qnDir* self);
QNAPI void qn_dir_seek(qnDir* self, int pos);
QNAPI int qn_dir_tell(qnDir* self);
QNAPI qnDir* qn_dir_new_l(const wchar_t* path);
QNAPI const wchar_t* qn_dir_read_l(qnDir* self);


//////////////////////////////////////////////////////////////////////////
// xml

/** ml unit. */
typedef struct qnMlu qnMlu;

/** ml tag. */
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
QNAPI qnMlu* qn_mlu_new_file_l(const wchar_t* filename);
QNAPI qnMlu* qn_mlu_new_buffer(const pointer_t data, int size);
QNAPI void qn_mlu_delete(qnMlu* self);

QNAPI void qn_mlu_clean_tags(qnMlu* self);
QNAPI void qn_mlu_clean_errs(qnMlu* self);
QNAPI bool qn_mlu_load_buffer(qnMlu* self, const pointer_t data, int size);
QNAPI bool qn_mlu_write_file(qnMlu* self, const char* filename);

QNAPI int qn_mlu_get_count(qnMlu* self);
QNAPI const char* qn_mlu_get_err(qnMlu* self, int at);
QNAPI qnMlTag* qn_mlu_get_tag(qnMlu* self, const char* name);
QNAPI qnMlTag* qn_mlu_get_tag_nth(qnMlu* self, int at);
QNAPI const char* qn_mlu_get_context(qnMlu* self, const char* name, const char* ifnotexist);
QNAPI const char* qn_mlu_get_context_nth(qnMlu* self, int at, const char* ifnotexist);
QNAPI int qn_mlu_contains(qnMlu* self, qnMlTag* tag);

QNAPI void qn_mlu_foreach(qnMlu* self, void(*func)(pointer_t userdata, qnMlTag* tag), pointer_t userdata);
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

QNAPI void qn_mltag_foreach_sub(qnMlTag* ptr, void(*func)(pointer_t userdata, qnMlTag* tag), pointer_t userdata);
QNAPI void qn_mltag_loopeach_sub(qnMlTag* ptr, void(*func)(qnMlTag* tag));

// tag - arg
QNAPI int qn_mltag_get_arity(qnMlTag* ptr);
QNAPI const char* qn_mltag_get_arg(qnMlTag* ptr, const char* name, const char* ifnotexist);
QNAPI bool qn_mltag_next_arg(qnMlTag* ptr, pointer_t* index, const char** name, const char** data);
QNAPI bool qn_mltag_contains_arg(qnMlTag* ptr, const char* name);

QNAPI void qn_mltag_foreach_arg(qnMlTag* ptr, void(*func)(pointer_t userdata, const char* name, const char* data), pointer_t userdata);
QNAPI void qn_mltag_loopeach_arg(qnMlTag* ptr, void(*func)(const char* name, const char* data));

QNAPI void qn_mltag_set_arg(qnMlTag* ptr, const char* name, const char* value);
QNAPI bool qn_mltag_remove_arg(qnMlTag* ptr, const char* name);

QN_EXTC_END
