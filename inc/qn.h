#pragma once

//////////////////////////////////////////////////////////////////////////
// version
#define _QN_VERSION_					202311L


//////////////////////////////////////////////////////////////////////////
// runtime configuration

// platform
#if defined(_WIN32)
#	define _QN_WINDOWS_					1
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__)
#	define _QN_BSD_						1
#endif

#if defined(__linux__) || defined(__gnu_linux__)
#	define _QN_LINUX_					1
#endif

#if defined(__android__) || defined(__ANDROID__)
#	define _QN_ANDROID_					1
#endif

#if defined(__unix__) || defined(__unix)
#	define _QN_UNIX_					1
#endif

#if _QN_ANDROID_
#	define _QN_MOBILE_					1
#endif

#if defined(_WINCONV) || defined(_QN_WINDOWS_)
#	define _QN_WINCONV_					1
#endif

#ifdef _CHAR_UNSIGNED
#	error "Compiler option: char type must be signed"
#endif

// api
#ifndef QNAPI
#	ifdef _LIB
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
#define QN_CONCAT_2(x,y)				x##y
#define QN_CONCAT_3(x,y,z)				x##y##z
#define QN_CONCAT(...)					QN_GET_MACRO_3(__VA_ARGS__, QN_CONCAT_3, QN_CONCAT_2)(__VA_ARGS__)
#define QN_STRING(x)					#x
#define QN_UNICODE(x)					L##x
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
#if _MSC_VER
#define QN_CONST_LONG(n)				(n##i64)
#define QN_CONST_ULONG(n)				(n##Ui64)
#else
#define QN_CONST_LONG(n)				(n##LL)
#define QN_CONST_ULONG(n)				(n##ULL)
#endif
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


#if 0
//////////////////////////////////////////////////////////////////////////
// initialize
KAPI void k_runtime(kcham use_type_library, kint v[/*4*/]);
KAPI void k_version(kint v[/*4*/]);

KAPI void k_p_lock(void);
KAPI void k_p_unlock(void);
KAPI void k_u_lock(void);
KAPI void k_u_unlock(void);

KAPI void k_exit(kint exitcode);
KAPI void k_atexit(void(*func)(kpointer), kpointer userdata);
KAPI void k_atexit_0(void(*func)());
KAPI void k_atexit_p(void(*func)(kpointer), kpointer userdata);


//////////////////////////////////////////////////////////////////////////
// protect

// atomic
KAPI kint k_atominc(kint volatile* value);
KAPI kint k_atomdec(kint volatile* value);
KAPI kint k_atomadd(kint volatile* value, kint amt);
KAPI kint k_atomexch(kint volatile* value, kint exch);
KAPI kint k_atomcmp(kint volatile* value, kint exch, kint comperand);

#define k_atomget(v)					k_atomcmp(v, 0, 0)			/** 매크로: 아토믹 얻기 연산. */
#define k_atomset(v,d)					k_atomcmp(v, d, *(v))		/** 매크로: 아토믹 쓰기 연산. */

#if _SB_64_
KAPI klong k_atominc_long(klong volatile* value);
KAPI klong k_atomdec_long(klong volatile* value);
KAPI klong k_atomadd_long(klong volatile* value, klong amt);
KAPI klong k_atomexch_long(klong volatile* value, klong exch);
KAPI klong k_atomcmp_long(klong volatile* value, klong exch, klong comperand);

#define k_atomget_long(v)				k_atomcmp_long(v, 0, 0)		/** 매크로: 아토믹 얻기 연산. */
#define k_atomset_long(v,d)				k_atomcmp_long(v, d, *(v))	/** 매크로: 아토믹 쓰기 연산. */
#endif

// intrinsic atomic
#if !SB_ATOM_IMPL && _SB_WINDOWS_ && defined(__INTRIN_H_)
#define k_atominc(v)					((kint)_InterlockedIncrement((LONG volatile *)(v)))
#define k_atomdec(v)					((kint)_InterlockedDecrement((LONG volatile *)(v)))
#define k_atomadd(v,a)					((kint)_InterlockedAdd((LONG volatile *)(v), (LONG)(a)))
#define k_atomexch(p,v)					((kint)_InterlockedExchange((LONG volatile *)(p), (LONG)(v)))
#define k_atomcmp(p,v,cmp)				((kint)_InterlockedCompareExchange((LONG volatile *)(p), (LONG)(v), (LONG)(cmp)))
#if _SB_64_
#define k_atominc_long(v)				((klong)_InterlockedIncrement64((klong volatile *)(v)))
#define k_atomdec_long(v)				((klong)_InterlockedDecrement64((klong volatile *)(v)))
#define k_atomadd_long(v,a)				((klong)_InterlockedAdd64((klong volatile *)(v), (klong)(a)))
#define k_atomexch_long(p,v)			((klong)_InterlockedExchange64((klong volatile *)(p), (klong)(v)))
#define k_atomcmp_long(p,v,cmp)			((klong)_InterlockedCompareExchange64((klong volatile *)(p), (klong)(v), (klong)(cmp)))
#endif
#endif

#if _SB_64_
#define k_atominc_ptr(v)				(uint32_tptr)k_atominc_long((klong volatile*)(v))
#define k_atomdec_ptr(v)				(uint32_tptr)k_atomdec_long((klong volatile*)(v))
#define k_atomexch_ptr(v,e)				(uint32_tptr)k_atomexch_long((klong volatile*)(v), (klong)e)
#define k_atomcmp_ptr(v,e,c)			(uint32_tptr)k_atomcmp_long((klong volatile*)(v), (klong)e, (klong)c)
#define k_atomget_ptr(v)				(uint32_tptr)k_atomcmp_long((klong volatile*)(v), 0, 0)
#define k_atomset_ptr(v,d)				(uint32_tptr)k_atomcmp_long((klong volatile*)(v), (klong)(d), *((klong*)(v)))
#else
#define k_atominc_ptr(v)				(uint32_tptr)k_atominc((kint volatile*)(v))							/** 아토믹 포인터 증가 연산 */
#define k_atomdec_ptr(v)				(uint32_tptr)k_atomdec((kint volatile*)(v))							/** 아토믹 포인터 감소 연산 */
#define k_atomexch_ptr(v,e)				(uint32_tptr)k_atomexch((kint volatile*)(v), (kint)e)					/** 아토믹 포인터 교환 연산 */
#define k_atomcmp_ptr(v,e,c)			(uint32_tptr)k_atomcmp((kint volatile*)(v), (kint)e, (kint)c)			/** 아토믹 포인터 비교 연산 */
#define k_atomget_ptr(v)				(uint32_tptr)k_atomcmp((kint volatile*)(v), 0, 0)						/** 아토믹 포인터 얻기 연산 */
#define k_atomset_ptr(v,d)				(uint32_tptr)k_atomcmp((kint volatile*)(v), (kint)(d), *((kint*)(v)))	/** 아토믹 포인터 대입 연산 */
#endif

// barrier & pause
#if _SB_WINDOWS_ && defined(_MSC_VER)
#ifndef __INTRIN_H_
extern void __cdecl __debugbreak(void);	// common
extern void _ReadWriteBarrier(void);	// common
extern void _mm_pause(void);			// x86
extern void __yield(void);				// arm
#endif
#define KSAFE_BARRIER		_ReadWriteBarrier()
#if _SB_X86_
#define KSAFE_PAUSE			_mm_pause()
#elif _SB_ARM_
#define KSAFE_PAUSE			__yield()
#else
#define KSAFE_PAUSE
#endif
#elif __GNUC__
#	if _SB_X86_
#define KSAFE_BARRIER		__asm__ __volatile__("" : : : "memory");
#define KSAFE_PAUSE			__asm__ __volatile__("pause" : : : "memory")
#define KSAFE_PAUSE_REPNOP	__asm__ __volatile__("rep; nop" : : : "memory")
#	else
#define KSAFE_BARRIER		__sync_synchronize()			// GCC 4.1
#define KSAFE_PAUSE			k_usleep(0)
#	endif
#else
KAPI void k_read_write_barrier(void);
KAPI void k_mm_pause(void);
#define KSAFE_BARRIER		k_read_write_barrier()
#define KSAFE_PAUSE			k_mm_pause()
#endif

// yield
KAPI void k_yield(kint step);

// lock
#if _SB_PTHREAD_ && !defined(__GNUC__) && (!_SB_X86_ || !_SB_ARM_)
typedef pthread_mutex_t kLock;
#define KLOCK_INIT				{PTHREAD_MUTEX_INITIALIZER}
#else
typedef kint kLock;											/** 잠그기 형 */
#define KLOCK_INIT				0							/** 잠그기의 초기 값 */
#endif

KAPI kcham k_lock_try(kLock* self);
KAPI void k_lock_enter(kLock* self);
KAPI void k_lock_leave(kLock* self);

#define QN_LOCK(x)				k_lock_enter(&(x))			/** 간편한 잠궈 보기 */
#define QN_UNLOCK(x)				k_lock_leave(&(x))			/** 간편한 잠금 풀기 */


//////////////////////////////////////////////////////////////////////////
// output
#define KLV_ALL							0					/** 출력레벨: 모든 출력 */
#define KLV_INTERNAL					1					/** 출력레벨: 내부 정보 */
#define KLV_RESULT						2					/** 출력레벨: 처리 결과 */
#define KLV_INFO						3					/** 출력레벨: 정보 */
#define KLV_WARNING						4					/** 출력레벨: 경고 */
#define KLV_ERROR						5					/** 출력레벨: 오류 */
#define KLV_CRITICAL					6					/** 출력레벨: 심각 */
#define KLV_FATAL						7					/** 출력레벨: 중단 */
#define KLV_NONE						8					/** 출력레벨: 없음 */

KAPI void k_redirect(const char* filename);
KAPI void k_settag(const char* str_len_32);
KAPI kint k_getlv(void);
KAPI void k_setlv(kint value);
KAPI void k_output(kint ch);
KAPI void k_outputs(const char* s);
KAPI void k_outputf(const char* fmt, ...);
KAPI void k_outputs_utf8(const char* s);
KAPI void k_outputf_utf8(const char* fmt, ...);
KAPI void k_debug_outputs(kint level, const char* cls, const char* text);
KAPI void k_debug_outputf(kint level, const char* cls, const char* fmt, ...);
KAPI kint k_debug_assert(const char* expr, const char* mesg, const char* filename, kint line);
KAPI kint k_debug_halt(const char* cls, const char* msg);
KAPI kcham k_syserr(char* buf, ksize_t size, kint code_or_zero);
KAPI kcham k_syserr_utf8(char* buf, ksize_t size, kint code_or_zero);
KAPI void k_apphandler(void);

#if _SB_DEBUG_
#define k_dmesg							k_output
#define k_dmesgs						k_outputs
#define k_dmesgf						k_outputf
#define k_dmesgs_utf8					k_outputs_utf8
#define k_dmesgf_utf8					k_outputf_utf8
#define k_assert(expr,rs_mesg)			QN_STMT_BEGIN{if (!(expr)){k_debug_assert(#expr, rs_mesg, __FILE__, __LINE__); k_breakpoint();}}QN_STMT_END
#define k_verify(expr)					QN_STMT_BEGIN{if (!(expr)){k_debug_assert(#expr, NULL, __FILE__, __LINE__); k_breakpoint();}}QN_STMT_END
#define k_test(expr)					(expr)
#else
#define k_dmesg(s)						((void)0)
#define k_dmesgs(s)						((void)0)
#define k_dmesgf(s,...)					((void)0)
#define k_dmesgs_utf8					((void)0)
#define k_dmesgf_utf8					((void)0)
#define k_assert(expr,rs_mesg)			((void)0)
#define k_verify(...)					(expr)
#define k_test(...)						((void)0)
#endif

#define k_traces(l,c,s)					QN_STMT_BEGIN{if(l>=k_getlv()) k_debug_outputs(l,c,s);}QN_STMT_END
#define k_tracef(l,c,...)				QN_STMT_BEGIN{if(l>=k_getlv()) k_debug_outputf(l,c,__VA_ARGS__);}QN_STMT_END
#define k_halt(c,s)						k_debug_halt(c,s)

#define k_return_if_fail(x)				QN_STMT_BEGIN{ if (!(x)) return; }QN_STMT_END
#define k_return_if_ok(x)				QN_STMT_BEGIN{ if ((x)) return; }QN_STMT_END
#define k_return_value_if_fail(x,r)		QN_STMT_BEGIN{ if (!(x)) return (r); }QN_STMT_END
#define k_return_value_if_ok(x,r)		QN_STMT_BEGIN{ if ((x)) return (r); }QN_STMT_END

#define k_goto_if_fail(x,lbl)			QN_STMT_BEGIN{ if (!(x)) goto lbl; }QN_STMT_END
#define k_goto_if_ok(x,lbl)				QN_STMT_BEGIN{ if ((x)) goto lbl; }QN_STMT_END

// error code
#define KERR_SUCCESS					0		// no error
#define KERR_UNKNOWN					1		// unknow error
#define KERR_FAIL						2		// operation fail
#define KERR_INTERNAL					3		// internal error
#define KERR_EXCEPTION					4		// exception
#define KERR_NEEDINIT					5		// not initialized
#define KERR_ALREADY					6		// already initialized
#define KERR_NOTABLE					7		// not a table
#define KERR_ARGUMENT					8		// invalid argument
#define KERR_NULL						9		// value is null
#define KERR_LAST						10		// value is last
#define KERR_INVAL						11		// invalid value
#define KERR_EMPTY						12		// value is empty
#define KERR_NOMEM						13		// no memory
#define KERR_NOSUPP						14		// not support function
#define KERR_SYNTAX						15		// syntax error
#define KERR_NOFILE						16		// file not found, no file
#define KERR_NODIR						17		// directory not found, no directory
#define KERR_FAIL_READ					18		// fail to read
#define KERR_FAIL_WRITE					19		// fail to write
#define KERR_FAIL_OPEN					20		// fail to open
#define KERR_FAIL_CREATE				21		// fail to create
#define KERR_OVERFLOW					22		// overflow
#define KERR_UNDERFFLOW					23		// underflow
#define KERR_EXIST						24		// already existance
#define KERR_NOEXIST					25		// not existance
#define KERR_CAST						26		// invalid cast
#define KERR_NOMEAN						27		// no mean
#define KERR_NOTIMPL					28		// not implemented yet
#define KERR_NOTARGET					29		// invalid target, no target
#define KERR_STACKOVERFLOW				30		// stack overflow
#define KERR_NONATIVE					31		// not native element
#define KERR_NOFUNCTION					32		// no function found
#define KERR_NOOPS						33		// no operator found
#define KERR_NOMNS						34		// invalid mnemonic
#define KERR_NOTYPE						35		// invalid type
#define KERR_READONLY					36		// read only, do not write
#define KERR_WRITEONLY					37		// write only, do not read
#define KERR_READDENY					38		// read deny
#define KERR_WRITEDENY					39		// write deny
#define KERR_TYPEMISS					40		// type mismatch
#define KERR_PROPERTY					41		// property
#define KERR_PARSE						42		// parse error
#define KERR_NOREF						43		// no reference found
#define KERR_UNEXPECTEXCEPTION			44		// unexpected exception
#define KERR_NOLOCK						45		// cannot lock
#define KERR_DRIVER						46		// invalid driver
#define KERR_DEVICE						47		// invalid device
#define KERR_CONTEXT					48		// invalid context
#define KERR_MAPPING					49		// invalid mapping
#define KERR_NOMULTI					50		// multi-instance not support
#define KERR_CLEAN						51		// problem(s) in clean up
#define KERR_RANGE						52		// invalid range
#define KERR_STRIDE						53		// invalid stride value


//////////////////////////////////////////////////////////////////////////
// memory

// mem
KAPI void k_free(kpointer ptr);
KAPI kpointer k_alloc(ksize_t size);
KAPI kpointer k_allocf(ksize_t size);
KAPI kpointer k_realloc(kpointer ptr, ksize_t size);
KAPI kpointer k_alloc_dbg(ksize_t size, const char* filename, kint line);
KAPI kpointer k_allocf_dbg(ksize_t size, const char* filename, kint line);
KAPI kpointer k_realloc_dbg(kpointer ptr, ksize_t size, const char* filename, kint line);

KAPI kpointer k_copymask(kpointer dest, kconstpointer src, ksize_t size, kint mask);
KAPI kpointer k_zcompress(kconstpointer src, ksize_t srcsize, ksize_t* destsize);
KAPI kpointer k_zuncompress(kconstpointer src, ksize_t srcsize, ksize_t bufsize, ksize_t* destsize);
KAPI kcham k_zucmprsb(kpointer dst, ksize_t destsize, kconstpointer src, ksize_t srcsize);

KAPI ksize_t k_memcalcblock(ksize_t size);
KAPI ksize_t k_memaligned(ksize_t size);
KAPI kint k_memudch(ksize_t size, double* out);
KAPI kint k_memuds(char* buf, kint buflen, ksize_t size);
KAPI char* k_memdmp(kconstpointer ptr, ksize_t size, char* buf, ksize_t len);
KAPI char* k_memdmp_hex(kconstpointer ptr, ksize_t size, char* buf, ksize_t len);
KAPI kcham k_memtest(kpointer ptr);

// macro
#if _SB_DEBUG_ || defined(SB_MEM_INFO)
#define k_new(cnt,type)					(type*)k_allocf_dbg((cnt)*sizeof(type), __FILE__, __LINE__)						/** 메모리 할당 */
#define k_new_zero(cnt, type)			(type*)k_alloc_dbg((cnt)*sizeof(type), __FILE__, __LINE__)						/** 메모리 할당(초기화) */
#define k_new_1(type)					(type*)k_allocf_dbg(sizeof(type), __FILE__, __LINE__)							/** 메모리 1개체 할당 */
#define k_new_0(type)					(type*)k_alloc_dbg(sizeof(type), __FILE__, __LINE__)							/** 메모리 1개체 할당(초기화) */
#define k_renew(ptr,cnt,type)			(type*)k_realloc_dbg((kpointer)(ptr), (cnt)*sizeof(type), __FILE__, __LINE__)	/** 메모리 재할당 */
#else										   
#define k_new(cnt,type)					(type*)k_allocf((cnt)*sizeof(type))
#define k_new_zero(cnt, type)			(type*)k_alloc((cnt)*sizeof(type))
#define k_new_1(type)					(type*)k_allocf(sizeof(type))
#define k_new_0(type)					(type*)k_alloc(sizeof(type))
#define k_renew(ptr,cnt,type)			(type*)k_realloc((kpointer)(ptr), (cnt)*sizeof(type))
#endif
#define k_delete(ptr)					k_free((kpointer)(ptr))															/** 메모리 삭제 */
#define k_delete_reset(pptr)			QN_STMT_BEGIN{ k_free((kpointer)*(pptr)); *(pptr)=NULL; }QN_STMT_END				/** 메모리를 삭제하고 널값으로 초기화 */

#define k_zero(ptr,cnt,type)			memset((ptr), 0, sizeof(type)*(cnt))											/** 메모리 초기화 */
#define k_zero_1(ptr)					memset((ptr), 0, sizeof(*(ptr)))												/** 메모리 1개체 초기화 */

#if _SB_BSD_ || _SB_LINUX_ /*|| !defined(_alloca)*/
#define _alloca							alloca	// "_alloca" is standard name
#endif

#define k_alloca(cnt,type)				(type*)_alloca((cnt)*sizeof(type))												/** 로컬 힙 할당 */
#define k_local_new(cnt,type)			k_alloca(cnt,type)																/** 로컬 힙 할당 */


//////////////////////////////////////////////////////////////////////////
// crts

// index
KAPI ksize_t k_nth(void);

// endian
#ifdef _SB_BIGENDIAN_
#define k_htobe16(x)					(x)
#define k_htobe32(x)					(x)
#define k_htobe64(x)					(x)
#define k_be16toh(x)					(x)
#define k_be32toh(x)					(x)
#define k_be64toh(x)					(x)

#define k_htole16(x)					K_ENDIAN_SWAP_USHORT(x)
#define k_htole32(x)					K_ENDIAN_SWAP_UINT(x)
#define k_htole64(x)					K_ENDIAN_SWAP_ULONG(x)
#define k_le16toh(x)					K_ENDIAN_SWAP_USHORT(x)
#define k_le32toh(x)					K_ENDIAN_SWAP_UINT(x)
#define k_le64toh(x)					K_ENDIAN_SWAP_ULONG(x)
#else
#define k_htobe16(x)					K_ENDIAN_SWAP_USHORT(x)
#define k_htobe32(x)					K_ENDIAN_SWAP_UINT(x)
#define k_htobe64(x)					K_ENDIAN_SWAP_ULONG(x)
#define k_be16toh(x)					K_ENDIAN_SWAP_USHORT(x)
#define k_be32toh(x)					K_ENDIAN_SWAP_UINT(x)
#define k_be64toh(x)					K_ENDIAN_SWAP_ULONG(x)

#define k_htole16(x)					(x)
#define k_htole32(x)					(x)
#define k_htole64(x)					(x)
#define k_le16toh(x)					(x)
#define k_le32toh(x)					(x)
#define k_le64toh(x)					(x)
#endif

KAPI uint16_t k_htons(uint16_t x);
KAPI uint16_t k_ntohs(uint16_t x);
KAPI uint32_t k_htonl(uint32_t x);
KAPI uint32_t k_ntohl(uint32_t x);

// sort
KAPI void k_qsort_context(kpointer ptr, ksize_t count, ksize_t stride, kint(*compfunc)(kpointer, kconstpointer, kconstpointer), kpointer context);
KAPI void k_qsort(kpointer ptr, ksize_t count, ksize_t stride, kint(*compfunc)(kconstpointer, kconstpointer));

// hashing
KAPI uint32_t k_primeclose(uint32_t value);
KAPI uint32_t k_primeshift(uint32_t value, uint32_t min, uint32_t* shift);
KAPI uint32_t k_crc32(const uint8_t* data, ksize_t size);
KAPI kulong k_crc64(const uint8_t* data, ksize_t size);
KAPI ksize_t k_crchash(const uint8_t* data, ksize_t size);
KAPI ksize_t k_ptrhash(kpointer p);
KAPI ksize_t k_timedhash(void);
KAPI ksize_t k_cbhash(kint prime8, kfunc func, kpointer data);

// env
KAPI kint k_getusername(char* buf, ksize_t size);
KAPI kint k_getcomputername(char* buf, ksize_t size);
KAPI kint k_getenv(const char* name, char* buf, ksize_t size);
KAPI kcham k_setenv(const char* name, const char* value);


//////////////////////////////////////////////////////////////////////////
// code page & character & string

/** code page info. */
typedef struct kCpInfo
{
	kint			len;
	const char*		name;
	kint			chcp;
	kint			lang;
} kCpInfo;

KAPI void k_getcp(kCpInfo* cp);
KAPI void k_setcp(kint cp);
KAPI void k_setlcp(const char* locale);
KAPI void k_setlocale(const char* locale_nullable);

// asc table
#define KASC_ALNUM						0x0001								/** alphabet or number */
#define KASC_ALPHA						0x0002								/** alphabet */
#define KASC_CTRL						0x0004								/** control character */
#define KASC_DIGIT						0x0008								/** digit number */
#define KASC_GRAPH						0x0010								/** printing character, not space */
#define KASC_LOWER						0x0020								/** lower alphabet */
#define KASC_PRINT						0x0040								/** printing character */
#define KASC_PNCT						0x0080								/** punctuation character */
#define KASC_SPACE						0x0100								/** space */
#define KASC_UPPER						0x0200								/** uppper alphabet */
#define KASC_HEX						0x0400								/** hexa number */
#define KASC_WBRK						0x0800								/** word break character */

#define k_isalnum(ch)					((k_getatv(ch) & KASC_ALNUM) != 0)	/** 문자가 알파벳 또는 숫자 인가 */
#define k_isalpha(ch)					((k_getatv(ch) & KASC_ALPHA) != 0)	/** 문자가 알파벳인가 */
#define k_isctrl(ch)					((k_getatv(ch) & KASC_CTRL) != 0)	/** 문자가 컨트롤 문자인가 */
#define k_isdigit(ch)					((k_getatv(ch) & KASC_DIGIT) != 0)	/** 문자가 숫자인가 */
#define k_isgraph(ch)					((k_getatv(ch) & KASC_GRAPH) != 0)	/** 문자가 그래프 문자인가 */
#define k_islower(ch)					((k_getatv(ch) & KASC_LOWER) != 0)	/** 문자가 소문자인가 */
#define k_ispkint(ch)					((k_getatv(ch) & KASC_PRINT) != 0)	/** 문자가 출력가능한가 */
#define k_ispnct(ch)					((k_getatv(ch) & KASC_PNCT) != 0)	/** 문자가 구둣점인가 */
#define k_isspace(ch)					((k_getatv(ch) & KASC_SPACE) != 0)	/** 문자가 공백인가 */
#define k_isupper(ch)					((k_getatv(ch) & KASC_UPPER) != 0)	/** 문자가 대문자인가 */
#define k_ishex(ch)						((k_getatv(ch) & KASC_HEX) != 0)	/** 문자가 16진 표현 문자인가 */
#define k_iswbrk(ch)					((k_getatv(ch) & KASC_WBRK) != 0)	/** 문자가 문단 중단 표현 문자인가 */

// asc
KAPI uint16_t k_getatv(kint ch);
KAPI uint8_t k_getbtv(kint ch);
KAPI kint k_toupper(kint ch);
KAPI kint k_tolower(kint ch);
KAPI kint k_todigit(kint ch);
KAPI kint k_tohex(kint ch);

// single or multi byte string
KAPI kint k_vsnprintf(char* out, ksize_t len, const char* fmt, va_list va);
KAPI kint k_vasprintf(char** out, const char* fmt, va_list va);
KAPI kint k_vfprintf(FILE* file, const char* fmt, va_list va);
KAPI kint k_vprintf(const char* fmt, va_list va);
KAPI kint k_snprintf(char* out, ksize_t len, const char* fmt, ...);
KAPI kint k_asprintf(char** out, const char* fmt, ...);
KAPI kint k_fprintf(FILE* file, const char* fmt, ...);
KAPI kint k_printf(const char* fmt, ...);
KAPI char* k_vapsprintf(const char* fmt, va_list va);
KAPI char* k_apsprintf(const char* fmt, ...);
KAPI ksize_t k_strhash(const char* p);
KAPI ksize_t k_strihash(const char* p);
KAPI ksize_t k_strlen(const char* p);
KAPI char* k_strcpy(char* dest, const char* src);
KAPI char* k_strpcpy(char* dest, const char*  src);
KAPI char* k_strncpy(char* dest, const char* src, ksize_t len);
KAPI char* k_strcat(char* dest, const char* src);
KAPI char* k_strncat(char* dest, ksize_t len, const char* src);
KAPI char* k_strconcat(char* dest, ...);
KAPI char* k_strdup(const char* p);
KAPI char* k_strndup(const char* p, ksize_t len);
KAPI char* k_strdupcat(const char* p, ...);
KAPI char* k_strdupchr(const char* p, const char** fnd, kint ch);
KAPI char* k_strduprchr(const char* p, const char** fnd, kint ch);
KAPI char* k_strrev(char* p);
KAPI char* k_strupr(char* p);
KAPI char* k_strlwr(char* p);
KAPI char* k_strchr(const char* p, kint ch);
KAPI char* k_strrchr(const char* p, kint ch);
KAPI char* k_strstr(const char* p, const char* s);
KAPI char* k_stristr(const char* p, const char* s);
KAPI char* k_strrstr(const char* p, const char* s);
KAPI char* k_strbrk(const char* p, const char* c);
KAPI char* k_strlft(char* dest, const char* src, ksize_t len);
KAPI char* k_strrgt(char* dest, const char* src, ksize_t len);
KAPI char* k_strmid(char* dest, const char* src, ksize_t pos, ksize_t len);
KAPI char* k_strltrim(char* dest);
KAPI char* k_strrtrim(char* dest);
KAPI char* k_strtrim(char* dest);
KAPI char* k_strrem(char* p, const char* rmlist);
KAPI kcham k_streqv(const char* p1, const char* p2);
KAPI kcham k_strieqv(const char* p1, const char* p2);
KAPI kint k_strcmp(const char* p1, const char* p2);
KAPI kint k_strncmp(const char* p1, const char* p2, ksize_t len);
KAPI kint k_stricmp(const char* p1, const char* p2);
KAPI kint k_strnicmp(const char* p1, const char* p2, ksize_t len);
KAPI kint k_strwcmatch(const char* pattern, const char* str);
KAPI kint k_striwcmatch(const char* pattern, const char* str);
KAPI kint k_strfnd(const char* src, const char* find, ksize_t index);
KAPI kint k_streqlv(const char* src, ...);
KAPI kint k_strieqlv(const char* src, ...);
KAPI kint k_strisnum(const char* s);
KAPI kint k_strtoi(const char* p);
KAPI klong k_strtol(const char* p);
KAPI float k_strtof(const char* p, char** sret);
KAPI double k_strtod(const char* p, char** sret);
KAPI uint32_t k_strtoi_base(const char* p, kint base);
KAPI kulong k_strtol_base(const char* p, kint base);
KAPI kcham k_strtob(const char* p);
KAPI ksize_t k_strfll(char* dest, ksize_t pos, ksize_t end, int ch);
KAPI ksize_t k_strspc(char* dest, ksize_t pos, ksize_t end);
KAPI ksize_t k_strlin(const char* text, char* line, ksize_t* readsize, kcham removespace);
KAPI char* k_strtok(char* string, const char* control, char** context);
KAPI kint k_strnatcmp(const char* left, const char* right);
KAPI kint k_strnatcasecmp(const char* left, const char* right);
KAPI char* k_strdquote(char* p, kint quote);
KAPI char* k_strsquote(char* p, kint quote);
KAPI uint32_t k_strcrc32(const char* s);
KAPI kint k_strisupr(const char* s);
KAPI kint k_strislwr(const char* s);
KAPI kcham k_splitpath(const char* path, char* drv, kint drv_len, char* dir, kint dir_len, char* filename, kint filename_len, char* ext, kint ext_len);
KAPI kcham k_divpath(const char* path, char* dir, kint dir_len, char* filename, kint filename_len);

#define k_atoi(p)						k_strtoi(p)						/** 문자열을 정수로 */
#define k_atol(p)						k_strtol(p)						/** 문자열을 64비트 정수로 */
#define k_atof(p)						k_strtof(p,NULL)				/** 문자열을 단정도 실수로 */
#define k_atod(p)						k_strtod(p,NULL)				/** 문자열을 배정도 실수로 */
#define k_atoi_h(p)						k_strtoi_base(p,16)				/** 문자열을 16진수 정수로 */
#define k_atoi_o(p)						k_strtoi_base(p,8)				/** 문자열을 8진수 정수로 */
#define k_atoi_b(p)						k_strtoi_base(p,2)				/** 문자열을 2진수 정수로 */
#define k_atol_h(p)						k_strtol_base(p,16)				/** 문자열을 64비트 16진수 정수로 */
#define k_atol_o(p)						k_strtol_base(p,8)				/** 문자열을 64비트 8진수 정수로 */
#define k_atol_b(p)						k_strtol_base(p,2)				/** 문자열을 64비트 2진수 정수로 */
#define k_atob(p)						k_strtob(p)						/** 문자열을 불린 표현으로 */

// wide unicode string
KAPI kint k_vsnwprintf(kwchar* out, ksize_t len, const kwchar* fmt, va_list va);
KAPI kint k_vaswprintf(kwchar** out, const kwchar* fmt, va_list va);
KAPI kint k_vfwprintf(FILE* file, const kwchar* fmt, va_list va);
KAPI kint k_vwprintf(const kwchar* fmt, va_list va);
KAPI kint k_snwprintf(kwchar* out, ksize_t len, const kwchar* fmt, ...);
KAPI kint k_aswprintf(kwchar** out, const kwchar* fmt, ...);
KAPI kint k_fwprintf(FILE* file, const kwchar* fmt, ...);
KAPI kint k_wprintf(const kwchar* fmt, ...);
KAPI kwchar* k_vapswprintf(const kwchar* fmt, va_list va);
KAPI kwchar* k_apswprintf(const kwchar* fmt, ...);
KAPI ksize_t k_wcshash(const kwchar* p);
KAPI ksize_t k_wcsihash(const kwchar* p);
KAPI ksize_t k_wcslen(const kwchar* p);
KAPI kwchar* k_wcscpy(kwchar* dest, const kwchar* src);
KAPI kwchar* k_wcspcpy(kwchar* dest, const kwchar*  src);
KAPI kwchar* k_wcsncpy(kwchar* dest, const kwchar* src, ksize_t len);
KAPI kwchar* k_wcscat(kwchar* dest, const kwchar* src);
KAPI kwchar* k_wcsncat(kwchar* dest, ksize_t len, const kwchar* src);
KAPI kwchar* k_wcsconcat(kwchar* dest, ...);
KAPI kwchar* k_wcsdup(const kwchar* p);
KAPI kwchar* k_wcsndup(const kwchar* p, ksize_t len);
KAPI kwchar* k_wcsdupcat(const kwchar* p, ...);
KAPI kwchar* k_wcsrev(kwchar* p);
KAPI kwchar* k_wcsupr(kwchar* p);
KAPI kwchar* k_wcslwr(kwchar* p);
KAPI kwchar* k_wcschr(const kwchar* p, kint ch);
KAPI kwchar* k_wcsrchr(const kwchar* p, kint ch);
KAPI kwchar* k_wcsstr(const kwchar* p, const kwchar* s);
KAPI kwchar* k_wcsistr(const kwchar* p, const kwchar* s);
KAPI kwchar* k_wcsrstr(const kwchar* p, const kwchar* s);
KAPI kwchar* k_wcsbrk(const kwchar* p, const kwchar* c);
KAPI kwchar* k_wcslft(kwchar* dest, const kwchar* src, ksize_t len);
KAPI kwchar* k_wcsrgt(kwchar* dest, const kwchar* src, ksize_t len);
KAPI kwchar* k_wcsmid(kwchar* dest, const kwchar* src, ksize_t pos, ksize_t len);
KAPI kwchar* k_wcsltrim(kwchar* dest);
KAPI kwchar* k_wcsrtrim(kwchar* dest);
KAPI kwchar* k_wcstrim(kwchar* dest);
KAPI kwchar* k_wcsrem(kwchar* p, const kwchar* rmlist);
KAPI kcham k_wcseqv(const kwchar* p1, const kwchar* p2);
KAPI kcham k_wcsieqv(const kwchar* p1, const kwchar* p2);
KAPI kint k_wcscmp(const kwchar* p1, const kwchar* p2);
KAPI kint k_wcsncmp(const kwchar* p1, const kwchar* p2, ksize_t len);
KAPI kint k_wcsicmp(const kwchar* p1, const kwchar* p2);
KAPI kint k_wcsnicmp(const kwchar* p1, const kwchar* p2, ksize_t len);
KAPI kint k_wcswcmatch(const kwchar* pattern, const kwchar* wcs);
KAPI kint k_wcsiwcmatch(const kwchar* pattern, const kwchar* wcs);
KAPI kint k_wcsfnd(const kwchar* src, const kwchar* find, ksize_t index);
KAPI kint k_wcsisnum(const kwchar* s);
KAPI kint k_wcstoi(const kwchar* p);
KAPI klong k_wcstol(const kwchar* p);
KAPI float k_wcstof(const kwchar* p, kwchar** sret);
KAPI double k_wcstod(const kwchar* p, kwchar** sret);
KAPI uint32_t k_wcstoi_base(const kwchar* p, kint base);
KAPI kulong k_wcstol_base(const kwchar* p, kint base);
KAPI kcham k_wcstob(const kwchar* p);
KAPI ksize_t k_wcsfll(kwchar* dest, ksize_t pos, ksize_t end, int ch);
KAPI ksize_t k_wcsspc(kwchar* dest, ksize_t pos, ksize_t end);
KAPI ksize_t k_wcslin(const kwchar* text, kwchar* line, ksize_t* readsize, kcham removespace);
KAPI kwchar* k_wcstok(kwchar* string, const kwchar* control, kwchar** context);
KAPI kint k_wcsnatcmp(const kwchar* left, const kwchar* right);
KAPI kint k_wcsnatcasecmp(const kwchar* left, const kwchar* right);
KAPI kcham k_wsplitpath(const kwchar* path, kwchar* drv, kint drv_len, kwchar* dir, kint dir_len, kwchar* filename, kint filename_len, kwchar* ext, kint ext_len);
KAPI kcham k_wdivpath(const kwchar* path, kwchar* dir, kint dir_len, kwchar* filename, kint filename_len);

#define k_wtoi(p)						k_wcstoi(p)						/** 문자열을 정수로 */
#define k_wtol(p)						k_wcstol(p)						/** 문자열을 64비트 정수로 */
#define k_wtof(p)						k_wcstof(p,NULL)				/** 문자열을 단정도 실수로 */
#define k_wtod(p)						k_wcstod(p,NULL)				/** 문자열을 배정도 실수로 */
#define k_wtoi_h(p)						k_wcstoi_base(p,16)				/** 문자열을 16진수 정수로 */
#define k_wtoi_o(p)						k_wcstoi_base(p,8)				/** 문자열을 8진수 정수로 */
#define k_wtoi_b(p)						k_wcstoi_base(p,2)				/** 문자열을 2진수 정수로 */
#define k_wtol_h(p)						k_wcstol_base(p,16)				/** 문자열을 64비트 16진수 정수로 */
#define k_wtol_o(p)						k_wcstol_base(p,8)				/** 문자열을 64비트 8진수 정수로 */
#define k_wtol_b(p)						k_wcstol_base(p,2)				/** 문자열을 64비트 2진수 정수로 */
#define k_wtob(p)						k_wcstob(p)						/** 문자열을 불린 표현으로 */

// utf8 character & string
KAPI char* k_utf8fchp(const char* s, const char* p);
KAPI char* k_utf8fchn(const char* s, const char* end);
KAPI char* k_utf8chp(const char* s);
KAPI char* k_utf8chn(const char* s);
KAPI ksize_t k_utf8len(const char* s);
KAPI char* k_utf8ncpy(char* dest, const char* src, ksize_t len);
KAPI char* k_utf8mid(char* dest, const char* src, ksize_t pos, ksize_t len);
KAPI char* k_utf8chr(const char* s, const char* utf8ch);
KAPI char* k_utf8rchr(const char* s, const char* utf8ch);
KAPI char* k_utf8ucs(const char* s, uchar4_t ch);
KAPI char* k_utf8rucs(const char* s, uchar4_t ch);
KAPI char* k_utf8rev(char* s);
KAPI uchar4_t k_utf8ccnc(const char* p);
KAPI kint k_utf8cunc(uchar4_t c, char* out);

// code convert
KAPI ksize_t k_getuni(kwchar* destuni, ksize_t destsize, const char* srcasc);
KAPI ksize_t k_getasc(char* destasc, ksize_t destsize, const kwchar* srcuni);
KAPI ksize_t k_conv_asc_to_uni(kwchar* destuni, ksize_t destsize, const char* srcasc, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_uni_to_asc(char* destasc, ksize_t destsize, const kwchar* srcuni, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_utf8_to_utf16(uchar2_t* dest, ksize_t destsize, const char* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_utf8_to_ucs4(uchar4_t* dest, ksize_t destsize, const char* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_uchar4_to_utf8(char* dest, ksize_t destsize, const uchar4_t* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_utf16_to_utf8(char* dest, ksize_t destsize, const uchar2_t* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_utf16_to_ucs4(uchar4_t* dest, ksize_t destsize, const uchar2_t* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_uchar4_to_utf16(uchar2_t* dest, ksize_t destsize, const uchar4_t* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_utf8_to_asc(char* dest, ksize_t destsize, const char* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_utf8_to_uni(kwchar* dest, ksize_t destsize, const char* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_asc_to_utf8(char* dest, ksize_t destsize, const char* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_conv_uni_to_utf8(char* dest, ksize_t destsize, const kwchar* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_base64_encrypt(char* dest, ksize_t destsize, const char* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_base64_decrypt(char* dest, ksize_t destsize, const char* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_base64_encrypt_alloc(char** dest, const char* src, ksize_t srclen_could_be_zero);
KAPI ksize_t k_base64_decrypt_alloc(char** dest, const char* src, ksize_t srclen_could_be_zero);

// symbol
KAPI uint32_t k_symcount(void);
KAPI void k_symprint(void);
KAPI uint32_t k_symflx(const char* string);
KAPI uint32_t k_symstt(const char* string);
KAPI const char* k_syminflx(const char* string);
KAPI const char* k_syminstt(const char* string);
KAPI const char* k_symbothflx(const char* string, uint32_t* sym);
KAPI const char* k_symbothstt(const char* string, uint32_t* sym);
KAPI uint32_t k_symtry(const char* string);
KAPI uint32_t k_symlet(const char* string, const char** ret);
KAPI ksize_t k_symlen(uint32_t symbol);
KAPI const char* k_symstr(uint32_t symbol);
KAPI kcham k_symvalue(uint32_t symbol, const char** str, ksize_t* len);
KAPI kcham k_symunvalue(uint32_t symbol, const char** str, ksize_t* len);
KAPI ksize_t k_symunlen(uint32_t symbol);
KAPI const char* k_symunstr(uint32_t symbol);


//////////////////////////////////////////////////////////////////////////
// time

/** date time. */
typedef struct kDateTime
{
	union
	{
		kulong				stamp;
		struct
		{
			uint32_t			date;
			uint32_t			time;
		};
		struct
		{
			uint32_t			year : 14;
			uint32_t			month : 6;
			uint32_t			day : 8;
			uint32_t			dow : 4;

			uint32_t			hour : 6;
			uint32_t			minute : 8;
			uint32_t			second : 8;
			uint32_t			millisecond : 10;
		};
	};
} kDateTime;

/** guid. */
typedef struct kGuid
{
	uint32_t A, B, C, D;
} kGuid;

#define KGUID_UNIQUE				1						/** GUID를 유니크 문자열로 */
#define KGUID_DIGIT					2						/** GUID를 숫자로 */
#define KGUID_DIGIT_BRACE			3						/** GUID를 숫자와 브라켓으로 */
#define KGUID_GIDIT_PARENTHESE		4						/** GUID를 숫자와 괄호로 */

/** timer. */
typedef struct kTimer
{
	double				abstime;
	double				runtime;
	double				advance;
	double				fps;
} kTimer;

#if _SB_WINDOWS_
/** timeval. */
struct timeval;
#endif

// time
KAPI void k_localtime(struct tm* ptm, const time_t ptt);
KAPI void k_gmtime(struct tm* ptm, const time_t tt);
KAPI void k_timeval_now(struct timeval* tv);
KAPI void k_timeval_add_micro(struct timeval* tv, kint microseconds);
KAPI void k_timeval_add_milli(struct timeval* tv, kint milliseconds);

// date time
KAPI void k_now(kDateTime* dt);
KAPI void k_stod(double seconds, kDateTime* dt);
KAPI void k_mstod(uint32_t milliseconds, kDateTime* dt);
KAPI void k_dtos(const kDateTime* dt_null_for_now, char* buf, kint size, kcham date, kcham time, kcham milli);

// guid
KAPI void k_guid(kGuid* guid);
KAPI kint k_guid_eq(const kGuid* l, const kGuid* r);
KAPI kint k_guid_comp(const kGuid* l, const kGuid* r);
KAPI char* k_guid_tos(const kGuid* guid_nullable, kint mtd, char* buf, kint size);

// tick
KAPI uint32_t k_cycle(void);
KAPI kulong k_cycle_long(void);
KAPI double k_cycle_per_sec(void);
KAPI uint32_t k_tick(void);
KAPI kulong k_tick_long(void);
KAPI double k_tick_second(void);

// sleep
KAPI void k_sleep(uint32_t milliseconds);
KAPI void k_ssleep(uint32_t seconds);
KAPI void k_usleep(uint32_t microseconds);
KAPI void k_msleep(kulong microseconds);

// timer
KAPI kTimer* k_timer_new(void);
KAPI void k_timer_delete(kTimer* self);
KAPI void k_timer_reset(kTimer* self);
KAPI void k_timer_start(kTimer* self);
KAPI void k_timer_stop(kTimer* self);
KAPI kcham k_timer_update(kTimer* self);
KAPI double k_timer_get_adv(kTimer* self);
KAPI double k_timer_get_abs(kTimer* self);
KAPI double k_timer_get_run(kTimer* self);
KAPI double k_timer_get_fps(kTimer* self);
KAPI double k_timer_get_cut(kTimer* self);
KAPI void k_timer_set_cut(kTimer* self, double cut);
KAPI void k_timer_set_manual(kTimer* self, kcham value);


//////////////////////////////////////////////////////////////////////////
// disk

/** directory. */
typedef struct kDir kDir;

/** file. */
typedef struct kFile kFile;

/** file io. */
typedef struct kFileIoDesc
{
	kint(*read)(kpointer handle, kpointer buffer, kint offset, kint size);
	kint(*write)(kpointer handle, kconstpointer buffer, kint offset, kint size);
	klong(*tell)(kpointer handle);
	klong(*seek)(kpointer handle, klong offset, kint org);
} kFileIoDesc;

/** file info. */
typedef struct kFileInfo
{
	kshort				type;
	uint16_t				len;
	klong				size;
	klong				cmpr;
	kulong				stc;			// creation stamp
	kulong				stw;			// last written stamp
	char				name[256];
} kFileInfo;

/** file access */
typedef struct kFileAccess
{
#if _SB_WINDOWS_
	uint32_t				mode;
	uint32_t				share;
	uint32_t				access;
	uint32_t				attr;
#else
	kint				mode;
	kint				access;
#endif
} kFileAccess;

/** file source. */
typedef struct kFileSource
{
	kshort				spec;
	kshort				type;
	uint32_t				size;
	uint32_t				cmpr;
	uint32_t				srt;
} kFileSource;

/** HFS info. */
typedef struct kHfsFile
{
	kFileSource			src;
	uint16_t				ext;
	uint16_t				len;
	uint32_t				hash;
	uint32_t				meta;
	uint32_t				next;
	kulong				stz;
	kchar				name[256];
} kHfsFile;

// seek
#define KSEEK_BEGIN						0					/** 파일의 처음부터 끝 방향으로 */
#define KSEEK_CUR						1					/** 파일의 현재부터 끝 방향으로 */
#define KSEEK_END						2					/** 파일의 끝부터 처음 방향으로 */

// store
#define KSTORE_AUTO						0					/** 파일 저장을 자동으로 */
#define KSTORE_NORM						1					/** 파일 저장을 순수하게 */
#define KSTORE_ENCR						2					/** 파일 저장을 암호화하여 */
#define KSTORE_CMPR						3					/** 파일 저장을 압축하여 */

// file attribute
#define KFAS_NONE						0					/** 파일 속성 없음 */
#define KFAS_FILE						0x0001				/** 파일 속성이 파일임 */
#define KFAS_DIR						0x0002				/** 파일 속성이 디렉터리임 */
#define KFAS_LINK						0x0008				/** 파일 속성이 링크 또는 정션임 */
#define KFAS_CMPR						0x0100				/** 파일 속성이 압축됨 */
#define KFAS_ENCR						0x0200				/** 파일 속성이 암호화됨 */

// file flags
#define KFF_READ						0x01				/** 파일이 읽을 수 있음 */
#define KFF_WRITE						0x02				/** 파일이 쓸 수 있음 */
#define KFF_SEEK						0x04				/** 파일 읽고 쓰는 위치를 찾거나 바꿀 수 있음 */

#define KFF_ALL							(KFF_READ|KFF_WRITE|KFF_SEEK)	/** 파일이 모든 속성을 갖고 있음: 읽기, 쓰기, 찾기 */
#define KFF_RDONLY						(KFF_READ|KFF_SEEK)				/** 파일이 읽을 수 만 있음: 읽기, 찾기 */

// file
KAPI kFile* k_file_new(const char* filename, const char* mode);
KAPI kFile* k_file_new_dup(kFile* org);
KAPI void k_file_delete(kFile* self);
KAPI kint k_file_flags(kFile* self, kint mask);
KAPI const char* k_file_name(kFile* self);
KAPI kint k_file_read(kFile* self, kpointer buffer, kint offset, kint size);
KAPI kint k_file_write(kFile* self, kconstpointer buffer, kint offset, kint size);
KAPI klong k_file_size(kFile* self);
KAPI klong k_file_tell(kFile* self);
KAPI klong k_file_seek(kFile* self, klong offset, kint org);
KAPI kcham k_file_flush(kFile* self);
KAPI kint k_file_printf(kFile* self, const char* fmt, ...);
KAPI kint k_file_vprintf(kFile* self, const char* fmt, va_list va);
KAPI kcham k_file_exist(const char* filename, kint* res);
KAPI kpointer k_file_alloc(const char* filename, kint* size);
KAPI kFile* k_file_new_l(const kwchar* filename, const kwchar* mode);
KAPI kcham k_file_exist_l(const kwchar* filename, kint* res);
KAPI kpointer k_file_alloc_l(const kwchar* filename, kint* size);
KAPI ksize_t k_file_get_max_alloc_size(void);
KAPI void k_file_set_max_alloc_size(ksize_t n);
KAPI void k_file_access_parse(const char* mode, kFileAccess* self, kint* flag);
KAPI void k_file_access_parse_l(const kwchar* mode, kFileAccess* self, kint* flag);

// directory
KAPI kDir* k_dir_new(const char* path);
KAPI void k_dir_delete(kDir* self);
KAPI const char* k_dir_read(kDir* self);
KAPI void k_dir_rewind(kDir* self);
KAPI void k_dir_seek(kDir* self, kint pos);
KAPI kint k_dir_tell(kDir* self);
KAPI kDir* k_dir_new_l(const kwchar* path);
KAPI const kwchar* k_dir_read_l(kDir* self);


//////////////////////////////////////////////////////////////////////////
// network

// poll
#define KPOLL_IN						0x0001				/** 폴링 입력 */
#define KPOLL_PRI						0x0002				/** 폴링 PRI */
#define KPOLL_OUT						0x0004				/** 폴링 출력 */
#define KPOLL_ERR						0x0008				/** 폴링 오류 */
#define KPOLL_HUP						0x0010				/** 폴링 HUP */
#define KPOLL_NVAL						0x0020				/** 폴링 잘못됨 */

// family
#define KNAF_UNSPEC						0					/**  */
#define KNAF_UNIX						1					/**  */
#define KNAF_INET						2					/**  */
#define KNAF_INET6						23					/**  */
#define KNAF_IRDA						26					/**  */
#define KNAF_BTH						32					/**  */

// socket type
#define KNST_STREAM						1					/**  */
#define KNST_DATAGRAM					2					/**  */
#define KNST_RAW						3					/**  */
#define KNST_RDM						4					/**  */
#define KNST_SEQPACKET					5					/**  */

// protocol
#define KNPROT_IP						0					/**  */
#define KNPROT_ICMP						1					/**  */
#define KNPROT_IGMP						2					/**  */
#define KNPROT_GGP						3					/**  */
#define KNPROT_IPV4						4					/**  */
#define KNPROT_TCP						6					/**  */
#define KNPROT_UDP						17					/**  */
#define KNPROT_IPV6						41					/**  */
#define KNPROT_ROUTING					43					/**  */
#define KNPROT_FRAGMENT					44					/**  */
#define KNPROT_ESP						50					/**  */
#define KNPROT_AH						51					/**  */
#define KNPROT_ICMPV6					58					/**  */
#define KNPROT_NONE						59					/**  */
#define KNPROT_DSTOPTS					60					/**  */
#define KNPROT_ND						77					/**  */
#define KNPROT_RAW						255					/**  */

// opt level
#define KNSOL_SOCKET					65535				/**  */
#define KNSOL_IP						0					/**  */
#define KNSOL_TCP						6					/**  */
#define KNSOL_UDP						17					/**  */
#define KNSOL_IPV6						41					/**  */

// opt name
#define KNSON_DEBUG						1					/**  */
#define KNSON_ACCEPTCONN				2					/**  */
#define KNSON_REUSEADDR					4					/**  */
#define KNSON_KEEPALIVE					8					/**  */
#define KNSON_DONTROUTE					16					/**  */
#define KNSON_BROADCAST					32					/**  */
#define KNSON_USELOOPBACK				64					/**  */
#define KNSON_LINGER					128					/**  */
#define KNSON_OUTOFBANDINLINE			256					/**  */
#define KNSON_DONTLINGER				-129				/**  */
#define KNSON_EXCLUSIVEADDRUSE			-5					/**  */
#define KNSON_SENDBUF					4097				/**  */
#define KNSON_RECVBUF					4098				/**  */
#define KNSON_SENDLOWWATER				4099				/**  */
#define KNSON_RECVLOWWATER				4100				/**  */
#define KNSON_SENDTIMEOUT				4101				/**  */
#define KNSON_RECVTIMEOUT				4102				/**  */
#define KNSON_ERROR						4103				/**  */
#define KNSON_TYPE						4104				/**  */
#define KNSON_MAXCONN					2147483647			/**  */
#define KNSON_IPOPTIONS					1					/**  */
#define KNSON_HEADERINCL				2					/**  */
#define KNSON_TYPEOFSERVICE				3					/**  */
#define KNSON_IPTIMETOLIVE				4					/**  */
#define KNSON_MULTICASTINTERFACE		9					/**  */
#define KNSON_MULTICASTTIMETOLIVE		10					/**  */
#define KNSON_MULTICASTLOOPBACK			11					/**  */
#define KNSON_ADDMEMBERSHIP				12					/**  */
#define KNSON_DROPMEMBERSHIP			13					/**  */
#define KNSON_DONTFRAGMENT				14					/**  */
#define KNSON_ADDSRCMEMBERSHIP			15					/**  */
#define KNSON_DROPSRCMEMBERSHIP			16					/**  */
#define KNSON_BLOCKSRC					17					/**  */
#define KNSON_UNBLOCKSRC				18					/**  */
#define KNSON_PACKETINFO				19					/**  */
#define KNSON_NODELAY					1					/**  */
#define KNSON_BSDURGENT					2					/**  */
#define KNSON_EXPEDITED					2					/**  */
#define KNSON_NOCHECKSUM				1					/**  */
#define KNSON_CHECKSUMCOVERAGE			20					/**  */

// socket flags
#define KNSF_NONE						0					/**  */
#define KNSF_OUTOFBAND					0x0001				/**  */
#define KNSF_PEEK						0x0002				/**  */
#define KNSF_DONTROUTE					0x0004				/**  */
#define KNSF_MAXIOVECTLEN				0x0010				/**  */
#define KNSF_FULLDATA					0x8000				/**  */

// socket where
#define KNSW_NONE						0					/**  */
#define KNSW_ANY						1					/**  */
#define KNSW_LOOPBACK					2					/**  */
#define KNSW_BROADCAST					3					/**  */

// addressinfo flag
#define KNAI_PASSIVE					0x0001				/**  */
#define KNAI_CANONNAME					0x0002				/**  */
#define KNAI_NUMERICHOST				0x0004				/**  */
#if _SB_WINDOWS_
#define KNAI_NUMERICSERV				0x0008				/**  */
#define KNAI_ALL						0x0100				/**  */
#define KNAI_ADDRCONFIG					0x0400				/**  */
#define KNAI_V4MAPPED					0x0800				/**  */
#else
#define KNAI_V4MAPPED					0x0008				/**  */
#define KNAI_ALL						0x0010				/**  */
#define KNAI_ADDRCONFIG					0x0020				/**  */
#define KNAI_NUMERICSERV				0x0400				/**  */
#endif

/** ipv4. */
typedef struct kNpV4
{
	union
	{
		uint32_t			addr;
		kvint			vaddr;
	};
	uint8_t				zero[8];
} kNpV4;

/** ipv6. */
typedef struct kNpV6
{
	uint32_t				flowinfo;
	union
	{
		uint16_t			addr[8];
		uint8_t			vaddr[16];
	};
	union
	{
		uint32_t			scopeid;
		union
		{
			struct
			{
				uint32_t	zone : 28;
				uint32_t	level : 4;
			};
			uint32_t		value;
		}				scopetype;
	};
} kNpV6;

/** address. */
typedef struct kNar
{
	uint16_t				family;
	uint16_t				port;
	union
	{
		kNpV4			v4;
		kNpV6			v6;
	};
} kNar;

/** address info. */
typedef struct kNai
{
	kint				flags;
	kint				family;			// KNAF_
	kint				socktype;		// KNST_
	kint				protocol;		// KNPROT_
	ksize_t				len;
	kNar				nar;
	char*				cname;			// canonical name
} kNai;

/** socket. */
typedef struct kSocket kSocket;

// network address resolver
KAPI kcham k_nar_init_table(kNar* self, kint count, uint8_t* tbl, uint16_t port);
KAPI kcham k_nar_init4(kNar* self, uint32_t addr, uint16_t port);
KAPI kcham k_nar_init4_resolve(kNar* self, const char* addr, uint16_t port);
KAPI kcham k_nar_init4_specify(kNar* self, kint where, uint16_t port);
KAPI kcham k_nar_init6(kNar* self, uint16_t* addr, uint16_t port);
KAPI kcham k_nar_init6_byte(kNar* self, uint8_t* addr, uint16_t port);
KAPI kcham k_nar_init6_specify(kNar* self, kint where, uint16_t port);

KAPI kint k_nar_numeric_addr(const kNar* self, char* buffer, kint size);
KAPI kint k_nar_resolve_addr(const kNar* self, char* buffer, kint size);
KAPI uint32_t k_nar_get_addr4(const kNar* self);
KAPI uint16_t k_nar_get_port(const kNar* self);
KAPI kint k_nar_get_family(const kNar* self);
KAPI kint k_nar_get_size(const kNar* self);
KAPI kcham k_nar_test(const kNar* self);
KAPI kcham k_nar_eq(const kNar* left, const kNar* right);

KAPI void k_nar_set_port(kNar* self, uint16_t port);

KAPI kcham k_nar_look_host_name(char* buffer, kint size);

// socket
KAPI kSocket* k_sock_new(kint family, kint socktype, kint protocol);
KAPI kSocket* k_sock_new_ai(const kNai* nai);
KAPI void k_sock_delete(kSocket* self);

KAPI kcham k_sock_reuse(kSocket* self);
KAPI kcham k_sock_close(kSocket* self);

KAPI kcham k_sock_shutdown(kSocket* self, kcham isread, kcham iswrite);
KAPI kcham k_sock_connect(kSocket* self, const kNar* nar);
KAPI kSocket* k_sock_accept(kSocket* self);
KAPI kcham k_sock_bind(kSocket* self, const kNar* nar);
KAPI kcham k_sock_bind_ai(kSocket* self, const kNai* nai);
KAPI kcham k_sock_listen(kSocket* self, kint maxconn);
KAPI kint k_sock_poll(kSocket* self, kint mode, kint milliseconds);

KAPI kint k_sock_send(kSocket* self, kconstpointer buffer, kint offset, kint size, kint flags);
KAPI kint k_sock_send_to(kSocket* self, kconstpointer buffer, kint offset, kint size, const kNar* nar, kint flags);
KAPI kint k_sock_send_nof(kSocket* self, kconstpointer buffer, kint offset, kint size);
KAPI kint k_sock_recv(kSocket* self, kpointer buffer, kint offset, kint size, kint flags);
KAPI kint k_sock_recv_from(kSocket* self, kpointer buffer, kint offset, kint size, kNar* nar, kint flags);
KAPI kint k_sock_recv_nof(kSocket* self, kpointer buffer, kint offset, kint size);

KAPI kcham k_sock_test(kSocket* self);
KAPI kcham k_sock_is_conn(kSocket* self);
KAPI kcham k_sock_is_listen(kSocket* self);
KAPI kcham k_sock_get_local(kSocket* self, kNar* nar);
KAPI kcham k_sock_get_remote(kSocket* self, kNar* nar);
KAPI kcham k_sock_get_block(kSocket* self);
KAPI kint k_sock_get_select(kSocket* self);
KAPI kint k_sock_get_family(kSocket* self);
KAPI kint k_sock_get_sock_type(kSocket* self);
KAPI kint k_sock_get_protocol(kSocket* self);
KAPI kint k_sock_get_avail(kSocket* self);
KAPI kcham k_sock_get_opt(kSocket* self, kint level, kint name, kpointer data, kint size);
KAPI kcham k_sock_get_dont_frag(kSocket* self);
KAPI kcham k_sock_get_broadcast(kSocket* self);
KAPI kcham k_sock_get_exaddr_use(kSocket* self);
KAPI kcham k_sock_get_linger(kSocket* self, kint* onoff, kint* linger);
KAPI kcham k_sock_get_mclb(kSocket* self);
KAPI kcham k_sock_get_nodelay(kSocket* self);
KAPI kshort k_sock_get_ttl(kSocket* self);
KAPI kint k_sock_get_recv_buf_size(kSocket* self);
KAPI kint k_sock_get_send_buf_size(kSocket* self);
KAPI kint k_sock_get_send_timeout(kSocket* self);
KAPI kint k_sock_get_recv_timeout(kSocket* self);

KAPI kcham k_sock_set_block(kSocket* self, kcham value);
KAPI kcham k_sock_set_opt(kSocket* self, kint level, kint name, kconstpointer data, kint size);
KAPI kcham k_sock_set_dont_frag(kSocket* self, kcham value);
KAPI kcham k_sock_set_broadcast(kSocket* self, kcham value);
KAPI kcham k_sock_set_exaddr_use(kSocket* self, kcham value);
KAPI kcham k_sock_set_linger(kSocket* self, kint onoff, kint linger);
KAPI kcham k_sock_set_mclb(kSocket* self, kcham value);
KAPI kcham k_sock_set_nodelay(kSocket* self, kcham value);
KAPI kcham k_sock_set_ttl(kSocket* self, kshort value);
KAPI kcham k_sock_set_recv_buf_size(kSocket* self, kint size);
KAPI kcham k_sock_set_send_buf_size(kSocket* self, kint size);
KAPI kcham k_sock_set_send_timeout(kSocket* self, kint len);
KAPI kcham k_sock_set_recv_timeout(kSocket* self, kint len);

KAPI kint k_sock_select(kSocket** socks, kint count, kint mode, kint milliseconds);

KAPI void k_sock_set_userdata(kSocket* self, kpointer data);
KAPI kpointer k_sock_get_userdata(kSocket* self);

//
KAPI void k_net_trace_errno(kcham isenable);
KAPI kint k_nai_query(const char* nodename, const char* servname, const kNai* hint, kNai** nhis);
KAPI kint k_nai_tos(const kNai* self, char* buffer, kint size);


//////////////////////////////////////////////////////////////////////////
// serial

// parity
#define KSERPAR_NONE					0					/**  */
#define KSERPAR_ODD						1					/**  */
#define KSERPAR_EVEN					2					/**  */
#define KSERPAR_MARK					3					/**  */
#define KSERPAR_SPACE					4					/**  */

// stop bit
#define KSERSTOP_1						0					/**  */
#define KSERSTOP_1_5					1					/**  */
#define KSERSTOP_2						2					/**  */

// handshake
#define KSERHS_NONE						0					/**  */
#define KSERHS_XONXOFF					1					/**  */
#define KSERHS_REQTOSEND				2					/**  */
#define KSERHS_REQTOSENDXONXOFF			3					/**  */

// flags
#define KSERF_NODTR						0x01				/**  */
#define KSERF_NORTS						0x02				/**  */
#define KSERF_NOBREAK					0x04				/**  */

/** sric. */
typedef struct kSerialCom kSerialCom;

// sric
KAPI kSerialCom* k_sric_new(int flags);
KAPI void k_sric_delete(kSerialCom* self);

KAPI kcham k_sric_set_port(kSerialCom* self, const char* name);
KAPI kcham k_sric_set_baud(kSerialCom* self, kint baudrate);
KAPI kcham k_sric_set_databit(kSerialCom* self, kint databit);
KAPI kcham k_sric_set_stopbit(kSerialCom* self, kint stopbit);
KAPI kcham k_sric_set_parity(kSerialCom* self, kint parity);
KAPI kcham k_sric_set_handshake(kSerialCom* self, kint handshake);
KAPI kcham k_sric_set_break_state(kSerialCom* self, kcham isbreak);
KAPI kcham k_sric_set_dtr_enable(kSerialCom* self, kcham value);
KAPI kcham k_sric_set_rts_enable(kSerialCom* self, kcham value);
KAPI kcham k_sric_set_recv_timeout(kSerialCom* self, kint value);
KAPI kcham k_sric_set_send_timeout(kSerialCom* self, kint value);
KAPI kcham k_sric_set_recv_buf_size(kSerialCom* self, kint value);
KAPI kcham k_sric_set_send_buf_size(kSerialCom* self, kint value);

KAPI kintptr k_sric_get_handle(kSerialCom* self);
KAPI const char* k_sric_get_port(kSerialCom* self);
KAPI kint k_sric_get_baud(kSerialCom* self);
KAPI kint k_sric_get_databit(kSerialCom* self);
KAPI kint k_sric_get_stopbit(kSerialCom* self);
KAPI kint k_sric_get_parity(kSerialCom* self);
KAPI kint k_sric_get_handshake(kSerialCom* self);
KAPI kcham k_sric_get_break_state(kSerialCom* self);
KAPI kcham k_sric_get_dtr_enable(kSerialCom* self);
KAPI kcham k_sric_get_rts_enable(kSerialCom* self);
KAPI kint k_sric_get_recv_timeout(kSerialCom* self);
KAPI kint k_sric_get_send_timeout(kSerialCom* self);
KAPI kint k_sric_get_recv_buf_size(kSerialCom* self);
KAPI kint k_sric_get_send_buf_size(kSerialCom* self);
KAPI kint k_sric_get_size_to_recv(kSerialCom* self);
KAPI kint k_sric_get_size_to_send(kSerialCom* self);
KAPI kcham k_sric_get_cd_holding(kSerialCom* self);
KAPI kcham k_sric_get_cts_holding(kSerialCom* self);
KAPI kcham k_sric_get_dsr_holding(kSerialCom* self);
KAPI kcham k_sric_is_open(kSerialCom* self);

KAPI kcham k_sric_open(kSerialCom* self);
KAPI void k_sric_close(kSerialCom* self);
KAPI kcham k_sric_discard_send(kSerialCom* self);
KAPI kcham k_sric_discard_recv(kSerialCom* self);

KAPI kint k_sric_send(kSerialCom* self, kconstpointer data, kint offset, kint size);
KAPI kint k_sric_recv(kSerialCom* self, kpointer data, kint offset, kint size);
KAPI kint k_sric_recv_byte(kSerialCom* self);
KAPI uint8_t* k_sric_recv_alloc(kSerialCom* self, kint* outrecv);
KAPI kcham k_sric_set_use_event(kSerialCom* self, kcham value);
KAPI kcham k_sric_wait_event(kSerialCom* self, kint waitelapse);

KAPI char* k_sric_enum_ports(kint* count);

#define k_sric_recv_exist			k_sric_recv_alloc


//////////////////////////////////////////////////////////////////////////
// system information

/** system information. */
typedef struct kSysInfo
{
	struct
	{
		kint			os;			// KSIPF_*
		kint			sub;		// KSIPFS_WINDOWS_*

		kint			ft;

		char			desc[64];
	} os;

	struct
	{
		kint			cpu;
		uint8_t			family;
		uint8_t			model;
		uint8_t			step;
		uint8_t			type;
		uint8_t			ext_model;
		uint8_t			ext_family;

		uint8_t			brand_id;
		uint8_t			apic_id;

		kint			count;
		kint			cc[4];

		uint32_t			ft[2];

		char			vendor[32];
		char			desc[64];
	} cpu;

	struct
	{
		klong			left;
		klong			phy;
	} mem;

	struct
	{
		char			host[64];
		uint8_t			addr[64];
	} net;
} kSysInfo;

KAPI void k_siqm(kSysInfo* p);
KAPI void k_sibf(const kSysInfo* p);
KAPI void k_sibm(const kSysInfo* si_nullable, char* buf, ksize_t size);
KAPI const kSysInfo* k_sisd(void);
KAPI kint k_sisd_ossub(void);


//////////////////////////////////////////////////////////////////////////
// break
#if _SB_WINDOWS_ && (!defined(_WINBASE_) || !defined(_WINBASE_H))
__declspec(dllimport) void __stdcall DebugBreak(void);
__declspec(dllimport) kint __stdcall IsDebuggerPresent(void);
#endif

#if _SB_WINDOWS_
#define k_candebug()					IsDebuggerPresent()
#if _SB_WINDOWS_DESKTOP_
#define k_breakpoint()					DebugBreak()
#else
#if _SB_X86_
#define k_breakpoint()					__debugbreak()
#else
#define k_breakpoint()					QN_STMT_BEGIN{ void* p=NULL; *p=NULL; }QN_STMT_END
#endif
#endif
#else
#if __GNUC__
#if _SB_X86_
#define k_candebug()					FALSE
#define k_breakpoint()					QN_STMT_BEGIN{ __asm__ __volatile__ ("int $03"); }QN_STMT_END
#else
#define k_candebug()					FALSE
#define k_breakpoint()					raise(SIGTRAP)
#endif
#else
#define k_candebug()					FALSE
#define k_breakpoint()					raise(SIGTRAP)
#endif
#endif


//////////////////////////////////////////////////////////////////////////
// module

/** load information. */
typedef struct kModule
{
	char*				filename;
	kint				ref;
} kModule;

// flags
#define KMDB_LAZY						0x0001					/** 모듈 플래그: LAZY */
#define KMDB_LOCAL						0x0002					/** 모듈 플래그: LOCAL */
#define KMDB_MASK						(KMDB_LAZY|KMDB_LOCAL)	/** 모듈 플래그: LAZY, LOCAL */

#define KMDB_EXT_INFO					0x2000					/** 모듈 상태: 정보 있음 */
#define KMDB_EXT_NOCLOSURE				0x4000					/** 모듈 상태: 닫기 기능 없음 */
#define KMDB_EXT_RESIDENT				0x8000					/** 모듈 상태: 항상 상주 */

// callback
typedef kcham(*kModInitFunc)(kModule*, kpointer, const char**);	/** 모듈이 읽을 때 콜백 */
typedef void(*kModDispFunc)(kModule*);							/** 모듈이 해제될 때 콜백 */

// global
KAPI kcham k_mod_set_suffix(const char* suffix);
KAPI void k_mod_set_error(const char* error);
KAPI const char* k_mod_get_error(void);

// 
KAPI kModule* k_mod_open(const char* filename, kpointer data, kint flags);
KAPI kcham k_mod_close(kModule* self);
KAPI kcham k_mod_sym_ptr(kModule* self, const char* name, kpointer* ptr);
KAPI kcham k_mod_sym_func(kModule* self, const char* name, kfunc* func);
KAPI kint k_mod_ref(kModule* self);
KAPI const char* k_mod_get_name(kModule* self);
KAPI kpointer k_mod_get_handle(kModule* self);


//////////////////////////////////////////////////////////////////////////
// thread

// callback
typedef kpointer(*kThreadFunc)(kpointer);			/** 스레드 콜백 */
typedef void(*kNotifyFunc)(kpointer);				/** 알림 콜백 */
typedef void(*kResultFunc)(kpointer, kint, kint);	/** 결과 콜백{userdata(result), code, bytes} */
typedef kint(*kIntervalFunc)(kpointer, kint);		/** 인터벌 콜백 */

// type
typedef struct kMutex kMutex;						/** 뮤텍스 */
typedef struct kCond kCond;							/** 조건 */
typedef struct kSem kSem;							/** 세마포어 */
typedef struct kTls kTls;							/** TLS */
typedef struct kThread kThread;						/** 스레드 */

/** once. */
typedef struct kOnce
{
	volatile kint		status;
	volatile kpointer	value;
} kOnce;

KAPI kpointer k_once_run(kOnce* self, kThreadFunc func, kpointer data);
KAPI kcham k_once_enter(volatile uint32_tptr* value);
KAPI void k_once_leave(volatile uint32_tptr* value, uint32_tptr init);

#define k_once_try(value)		(k_atomget_ptr(value) != 0 ? FALSE : k_once_enter(value))

// mutex, basic overlapped lock
KAPI kMutex* k_mutex_new(void);
KAPI kMutex* k_mutex_new_count(kint count);
KAPI void k_mutex_delete(kMutex* self);
KAPI void k_mutex_enter(kMutex* self);
KAPI void k_mutex_leave(kMutex* self);

// condition
KAPI kCond* k_cond_new(void);
KAPI void k_cond_delete(kCond* self);
KAPI void k_cond_signal(kCond* self);
KAPI void k_cond_broadcast(kCond* self);
KAPI void k_cond_wait(kCond* self, kMutex* enterlock);
KAPI kcham k_cond_wait_for(kCond* self, kMutex* enterlock, kint milliseconds);

// semaphore
KAPI kSem* k_sem_new(kint initial);
KAPI void k_sem_delete(kSem* self);
KAPI kcham k_sem_wait(kSem* self);
KAPI kcham k_sem_wait_for(kSem* self, kint milliseconds);
KAPI kcham k_sem_try_wait(kSem* self);
KAPI kcham k_sem_post(kSem* self);
KAPI kint k_sem_value(kSem* self);

// tls
KAPI kTls* k_tls(kNotifyFunc func);
KAPI void k_settls(kTls* self, kpointer value);
KAPI kpointer k_gettls(kTls* self);

// thread
KAPI kThread* k_thread_try(void);
KAPI kThread* k_thread_self(void);
KAPI kcham k_thread_run(kThreadFunc func, kpointer userdata, uint32_t stacksize, uint32_t busytype);
KAPI void k_thread_exit(kpointer ret);

KAPI kThread* k_thread_new(kThreadFunc func, kpointer userdata, uint32_t stacksize, uint32_t busytype);
KAPI void k_thread_delete(kThread* self);
KAPI kcham k_thread_start(kThread* self);
KAPI kpointer k_thread_wait(kThread* self);
KAPI kcham k_thread_set_busy(kThread* self, kint busytype);
KAPI kcham k_thread_set_stack(kThread* self, uint32_t stacksize);
KAPI kcham k_thread_set_cb(kThread* self, kThreadFunc func, kpointer data);
KAPI kcham k_thread_can_wait(kThread* self);
KAPI kint k_thread_get_busy(kThread* self);
KAPI uint32_t k_thread_get_stack(kThread* self);
KAPI uint32_t k_thread_get_id(kThread* self);


//////////////////////////////////////////////////////////////////////////
// xml

/** ml unit. */
typedef struct kMlu kMlu;

/** ml tag. */
typedef struct kMlTag
{
	char*				name;
	char*				cntx;

	kint				nlen;
	kint				clen;
	kint				line;

#if _SB_WINDOWS_ && _SB_DEBUG_
	// only for windows in debug mode. do not use
	kwchar*			uname;
	kwchar*			ucntx;
#endif
} kMlTag;

// ml unit
KAPI kMlu* k_mlu_new(void);
KAPI kMlu* k_mlu_new_file(const char* filename);
KAPI kMlu* k_mlu_new_file_l(const kwchar* filename);
KAPI kMlu* k_mlu_new_buffer(kconstpointer data, kint size);
KAPI void k_mlu_delete(kMlu* self);

KAPI void k_mlu_clean_tags(kMlu* self);
KAPI void k_mlu_clean_errs(kMlu* self);
KAPI kcham k_mlu_load_buffer(kMlu* self, kconstpointer data, kint size);
KAPI kcham k_mlu_write_file(kMlu* self, const char* filename);

KAPI kint k_mlu_get_count(kMlu* self);
KAPI const char* k_mlu_get_filename(kMlu* self);
KAPI const char* k_mlu_get_err(kMlu* self, kint at);
KAPI kMlTag* k_mlu_get_tag(kMlu* self, const char* name);
KAPI kMlTag* k_mlu_get_tag_nth(kMlu* self, kint at);
KAPI const char* k_mlu_get_context(kMlu* self, const char* name, const char* ifnotexist);
KAPI const char* k_mlu_get_context_nth(kMlu* self, kint at, const char* ifnotexist);
KAPI kint k_mlu_contains(kMlu* self, kMlTag* tag);

KAPI void k_mlu_foreach(kMlu* self, void(*func)(kpointer userdata, kMlTag* tag), kpointer userdata);
KAPI void k_mlu_loopeach(kMlu* self, void(*func)(kMlTag* tag));

KAPI kMlTag* k_mlu_add(kMlu* self, const char* name, const char* context, kint line);
KAPI kMlTag* k_mlu_add_tag(kMlu* self, kMlTag* tag);
KAPI kint k_mlu_remove(kMlu* self, const char* name, kcham isall);
KAPI kcham k_mlu_remove_nth(kMlu* self, kint at);
KAPI kcham k_mlu_remove_tag(kMlu* self, kMlTag* tag, kcham isdelete);

KAPI void k_mlu_add_err(kMlu* self, const char* msg);
KAPI void k_mlu_add_errf(kMlu* self, const char* fmt, ...);
KAPI void k_mlu_print_err(kMlu* self);

KAPI void k_mlu_print(kMlu* self);

// tag
KAPI kMlTag* k_mltag_new(const char* name);
KAPI void k_mltag_delete(kMlTag* self);

// tag - context
KAPI void k_mltag_add_context(kMlTag* ptr, const char* cntx, kint size);
KAPI void k_mltag_set_context(kMlTag* ptr, const char* cntx, kint size);

// tag - sub
KAPI kint k_mltag_get_sub_count(kMlTag* ptr);
KAPI kMlTag* k_mltag_get_sub(kMlTag* ptr, const char* name);
KAPI kMlTag* k_mltag_get_sub_nth(kMlTag* ptr, kint at);
KAPI const char* k_mltag_get_sub_context(kMlTag* ptr, const char* name, const char* ifnotexist);
KAPI const char* k_mltag_get_sub_context_nth(kMlTag* ptr, kint at, const char* ifnotexist);
KAPI kint k_mltag_contains_sub(kMlTag* ptr, kMlTag* tag);
KAPI kMlTag* k_mltag_add_sub(kMlTag* ptr, const char* name, const char* context, kint line);
KAPI kMlTag* k_mltag_add_sub_tag(kMlTag* ptr, kMlTag* tag);
KAPI kint k_mltag_remove_sub(kMlTag* ptr, const char* name, kcham isall);
KAPI kcham k_mltag_remove_sub_nth(kMlTag* ptr, kint at);
KAPI kcham k_mltag_remove_sub_tag(kMlTag* ptr, kMlTag* tag, kcham isdelete);

KAPI void k_mltag_foreach_sub(kMlTag* ptr, void(*func)(kpointer userdata, kMlTag* tag), kpointer userdata);
KAPI void k_mltag_loopeach_sub(kMlTag* ptr, void(*func)(kMlTag* tag));

// tag - arg
KAPI kint k_mltag_get_arity(kMlTag* ptr);
KAPI const char* k_mltag_get_arg(kMlTag* ptr, const char* name, const char* ifnotexist);
KAPI kcham k_mltag_next_arg(kMlTag* ptr, kpointer* index, const char** name, const char** data);
KAPI kcham k_mltag_contains_arg(kMlTag* ptr, const char* name);

KAPI void k_mltag_foreach_arg(kMlTag* ptr, void(*func)(kpointer userdata, const char* name, const char* data), kpointer userdata);
KAPI void k_mltag_loopeach_arg(kMlTag* ptr, void(*func)(const char* name, const char* data));

KAPI void k_mltag_set_arg(kMlTag* ptr, const char* name, const char* value);
KAPI kcham k_mltag_remove_arg(kMlTag* ptr, const char* name);


//////////////////////////////////////////////////////////////////////////
// win32 registry
#if _SB_WINDOWS_DESKTOP_
/** Defines an alias representing registry. */
typedef struct kRegistry				kRegistry;

KAPI kRegistry* k_regi_new(const char* container, const char* name, kcham canwrite);
KAPI void k_regi_delete(kRegistry* key);
KAPI kcham k_regi_enum(kRegistry* key, kint index, char* buffer, kint size);
KAPI kcham k_regi_query_type(kRegistry* key, const char* name, kint* rettype, kint* retsize);
KAPI uint8_t* k_regi_get_unknown(kRegistry* key, const char* name, kint* rettype, kint* retsize);
KAPI uint8_t* k_regi_get_binary(kRegistry* key, const char* name, kint* retsize);
KAPI kchar* k_regi_get_multi_string(kRegistry* key, const char* name, kint* retsize);
KAPI kcham k_regi_get_string(kRegistry* key, const char* name, char* buffer, kint size);
KAPI kcham k_regi_get_int(kRegistry* key, const char* name, kint* value);
KAPI kcham k_regi_get_long(kRegistry* key, const char* name, klong* value);
#endif
#endif

QN_EXTC_END
