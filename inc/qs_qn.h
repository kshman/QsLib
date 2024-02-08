//
// QsLib [QN Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once
#define __QS_QN__

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

//////////////////////////////////////////////////////////////////////////
// test & platform specification

// EMSCRIPTEN
#ifdef __EMSCRIPTEN__
#ifndef QS_NO_MEMORY_PROFILE
#define QS_NO_MEMORY_PROFILE	1
#endif
#ifndef QS_NO_SPINLOCK
#define QS_NO_SPINLOCK			1
#endif
#ifndef QS_NO_THREAD
#define QS_NO_THREAD			1
#endif
#ifndef QS_SUPPRESS_VAR_CHK
#define QS_SUPPRESS_VAR_CHK		1
#endif
#endif

//////////////////////////////////////////////////////////////////////////
// compiler configuration

// check
#if !defined _MSC_VER && !defined __clang__ && !defined __GNUC__
#error unknown compiler! (support: MSVC, CLANG, GCC)
#endif
#ifdef _CHAR_UNSIGNED
#error compiler option: char type must be signed
#endif

// platform
#ifdef _WIN32
#define _QN_WINDOWS_					1
#if defined __XBOXONE__ || defined __XBOXSERIES__
#define _QN_XBOX_						1
#endif
#endif
#ifdef __unix__
#define _QN_UNIX_						1
#endif
#ifdef __FreeBSD__
#define _QN_FREEBSD_					1
#endif
#ifdef __linux__
#define _QN_LINUX_						1
#endif
#ifdef __EMSCRIPTEN__
#define _QN_EMSCRIPTEN_					1
#endif
#ifdef __android__
#define _QN_ANDROID_					1
#endif
#if defined _WIN64 || defined _M_AMD64 || defined _M_X64 || defined __LP64__ || defined __amd64__ || defined __x86_64__ || defined __aarch64__
#define _QN_64_							1
#endif
#if defined _QN_EMSCRIPTEN_ || defined _QN_ANDROID_
#define _QN_MOBILE_						1
#endif

// c standard specific
#ifndef INLINE
#if defined _MSC_VER
#define INLINE							__inline
#elif defined __GNUC__
#define INLINE							static inline
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L || defined __cplusplus
#define INLINE							inline
#else
#define INLINE							static
#endif
#endif // INLINE
#ifndef FINLINE
#if defined _MSC_VER
#define	FINLINE							__forceinline
#elif defined __GNUC__
#define	FINLINE							static inline __attribute__ ((always_inline))
#else
#define FINLINE							INLINE
#endif
#endif // FINLINE
#ifndef NORETURN
#if defined (_MSC_VER)
#define NORETURN						__declspec(noreturn)
#elif defined __GNUC__
#define NORETURN						__attribute__((noreturn))
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
#define NORETURN						_Noreturn
#else
#define NORETURN
#endif
#endif // NORETURN
#ifndef ALIGNOF
#if defined _MSC_VER
#define ALIGNOF(x)						__declspec(align(x))
#elif defined __GNUC__
#define ALIGNOF(x)						__attribute__((aligned(x)))
#elif defined __STDC_VERSION__ && __STD_VERSION__ >= 201112L
#define ALIGNOF(x)						_Alignof(x)
#else
#define ALIGNOF(x)
#endif
#endif // ALIGNOF
#ifndef DEPRECATED
#if defined _MSC_VER
#define DEPRECATED(x)					__declspec(deprecated(x))
#elif defined __GNUC__
#define DEPRECATED(x)					__attribute__((deprecated(x)))
#else
#define DEPRECATED
#endif
#endif // DEPRECATED
#ifndef FALLTHROUGH
#if defined __GNUC__
#define FALLTHROUGH						__attribute__((fallthrough))
#else
#define FALLTHROUGH
#endif
#endif // FALLTHROUGH
#ifndef PRAGMA
#define PRAGMA(x)
#endif // PRAGMA

// compiler specific
#ifdef _MSC_VER
#define QN_CONST_ANY					extern const __declspec(selectany)
#define QN_STMT_BEGIN					PRAGMA(warning(suppress:4127 4296 6011)) do
#define QN_STMT_END						while(0)
#define QN_WARN_PUSH					PRAGMA(warning(push))
#define QN_WARN_POP						PRAGMA(warning(pop))
#define QN_WARN_SIGN
#define QN_WARN_ASSIGN					PRAGMA(warning(disable:4706))
#elif defined __GNUC__
#define QN_CONST_ANY					const __attribute__((weak))
#define QN_STMT_BEGIN					do
#define QN_STMT_END						while(0)
#define QN_WARN_PUSH					PRAGMA(GCC diagnostic push)
#define QN_WARN_POP						PRAGMA(GCC diagnostic pop)
#define QN_WARN_SIGN					PRAGMA(GCC diagnostic ignored "-Wsign-conversion")
#define QN_WARN_ASSIGN
#endif


//////////////////////////////////////////////////////////////////////////
// macro

// function support
#define QN_XSTRING(x)					#x
#define QN_XUNICODE(x)					L##x
#define QN_XCCAT_2(x,y)					x##y
#define QN_XCCAT_3(x,y,z)				x##y##z
#define QN_XCCAT_4(x,y,z,w)				x##y##z##w
#define QN_XMAC_2(_1,_2,N,...)			N
#define QN_XMAC_3(_1,_2,_3,N,...)		N
#define QN_XMAC_4(_1,_2,_3,_4,N,...)	N

// function
#define QN_DUMMY(dummy)					(void)dummy
#define QN_TODO(todo)					PRAGMA(message("TODO: " #todo " (" __FUNCTION__ ")"))
#define QN_STRING(x)					QN_XSTRING(x)					/// @brief 문자열로 정의
#define QN_UNICODE(x)					QN_XUNICODE(x)					/// @brief 유니코드로 정의
#define QN_CONCAT(...)					QN_XMAC_4(__VA_ARGS__, QN_XCCAT_4, QN_XCCAT_3, QN_XCCAT_2)(__VA_ARGS__)	/// @brief 문구 합침

#define QN_COUNTOF(arr)					(sizeof(arr) / sizeof((arr)[0]))							/// @brief 배열 갯수 얻기
#define QN_OFFSETOF(type,member)		((size_t)((char*)&((type*)0)->member))						/// @brief 구조체에서 위치 얻기
#define QN_MEMBEROF(type,ptr,offset)	(*(type*)((void*)((char*)(ptr) + (size_t)(offset))))		/// @brief 구조체에서 위치 얻기(포인터)

#define QN_MAX(left,right)				(((left) > (right)) ? (left) : (right))						/// @brief 최대값 얻기
#define QN_MIN(left,right)				(((left) < (right)) ? (left) : (right))						/// @brief 최소값 얻기
#define QN_ABS(value)					(((value) < 0) ? -(value) : (value))						/// @brief 절대값 얻기
#define QN_CLAMP(value,lo,hi)			((value) < (lo) ? (lo) : (value) > (hi) ? (hi) : (value))	/// @brief 숫자를 최소/최대값으로 자르기
#define QN_FOURCC(a,b,c,d)				((uint)(a)|((uint)(b)<<8)|((uint)(c)<<16)|((uint)(d)<<24))	/// @brief 4문자 코드 만들기
#define QN_ALIGN(value,align)			(((value) + (align) - 1) & ~((align) - 1))					/// @brief 정렬하기

#define QN_BIT(bit)						(1 << (bit))					/// @brief 마스크 만들기
#define QN_TBIT(value,bit)				(((value) & (1 << (bit))) != 0)	/// @brief 비트가 있나 비교
#define QN_TMASK(value,mask)			(((value) & (mask)) != 0)		/// @brief 마스크가 있나 비교
#define QN_SBIT(value,bit,set)			QN_WARN_PUSH QN_WARN_SIGN ((set) ? ((value) |= (1 << (bit))) : ((value) &= ~(1 << (bit)))) QN_WARN_POP	/// @brief 비트 설정
#define QN_SMASK(value,mask,set)		QN_WARN_PUSH QN_WARN_SIGN ((set) ? ((value) |= (mask)) : ((value) &= ~(mask))) QN_WARN_POP				/// @brief 마스크 설정

// constant
#define QN_VERSION_MAJOR				3
#define QN_VERSION_MINER				5
#define QN_VERSION						(QN_VERSION_MAJOR * 100 + QN_VERSION_MINER)

#define QN_MSEC_PER_SEC					1000							/// @brief 초당 밀리초
#define QN_USEC_PER_SEC					1000000							/// @brief 초당 마이크로초 
#define QN_NSEC_PER_SEC					1000000000						/// @brief 초당 나노초 
#define QN_MIN_HASH						11								/// @brief 최소 해시 갯수
#define QN_MAX_HASH						13845163						/// @brief 최대 해시 갯수
#define QN_MAX_RAND						0x7FFF							/// @brief 최대 난수
#define QN_MAX_PATH						2048							/// @brief 경로의 최대 길이
#define QN_MAX_PATH_BIAS				8								/// @brief 경로의 최대 길이 보정
#ifdef _QN_WINDOWS_
#define QN_PATH_SEP						'\\'							/// @brief 경로 분리 문자
#else
#define QN_PATH_SEP						'/'
#endif


//////////////////////////////////////////////////////////////////////////
// types

typedef void* pointer_t;												/// @brief 포인터 타입
typedef void (*func_t)(void);											/// @brief 함수 핸들러
typedef void (*paramfunc_t)(void*);										/// @brief 파라미터 있는 함수 핸들러
typedef void (*paramfunc2_t)(void*,void*);								/// @brief 파라미터 2개 있는 함수 핸들러

typedef bool (*eqfunc_t)(const void*,const void*);						/// @brief 같은지 비교 함수 핸들러
typedef bool (*eqcfunc_t)(void*, const void*);							/// @brief 같은지 비교 함수 핸들러(컨텍스트)
typedef int (*compfunc_t)(const void*,const void*);						/// @brief 비교 함수 핸들러
typedef int (*sortfunc_t)(const void*,const void*);						/// @brief 정렬 함수 핸들러
typedef int (*sortcfunc_t)(void*,const void*,const void*);				/// @brief 정렬 함수 핸들러(컨텍스트)

// aliases
typedef int8_t							sbyte;							/// @brief 8비트 부호 있는 정수
typedef int64_t							llong;							/// @brief 64비트 부호 있는 정수
typedef uint8_t							byte;							/// @brief 8비트 부호 없는 정수
typedef uint16_t						ushort;							/// @brief 16비트 부호 없는 정수
typedef uint32_t						uint;							/// @brief 32비트 부호 없는 정수
typedef uint64_t						ullong;							/// @brief 64비트 부호 없는 정수

typedef intptr_t						nint;							/// @brief 플랫폼에 따른 부호 있는 포인터
typedef uintptr_t						nuint;							/// @brief 플랫폼에 따른 부호 없는 포인터

typedef wchar_t							wchar;
#ifdef _QN_WINDOWS_
typedef uint32_t						uchar4;							/// @brief 4바이트(32비트) 유니코드
typedef wchar_t							uchar2;							/// @brief 2바이트(16비트) 유니코드
#else
typedef wint_t							uchar4;
typedef uint16_t						uchar2;
#endif

typedef uint16_t						halfint;						/// @brief 16비트 부호 없는 정수(half int)
typedef uint16_t						halffloat;						/// @brief 16비트 실수(half float)
typedef int16_t							bool16;							/// @brief 16비트 참거짓
typedef int32_t							bool32;							/// @brief 32비트 참거짓

/// @brief 하위 변형 있는 16비트 정수
typedef union VINT16_T
{
	struct VINT16_T_BYTE
	{
		byte			l, h;											/// @brief 바이트 (h=msb)
	}				b;													/// @brief 바이트 집합
	ushort			w;													/// @brief 워드
} vint16_t, vshort;

/// @brief 하위 변형 있는 32비트 정수
typedef union VINT32_T
{
	struct VINT32_T_BYTE
	{
		byte			a, b, c, d;										/// @brief 바이트 (d=msb)
	}				b;													/// @brief 바이트 집합
	struct VINT32_T_WORD
	{
		ushort			l, h;											/// @brief 워드
	}				w;													/// @brief 워드 집합
	uint			dw;													/// @brief 더블 워드
} vint32_t, vint;

/// @brief 하위 변형 있는 64비트 정수
typedef union VINT64_T
{
	struct VINT64_T_BYTE
	{
		byte			la, lb, lc, ld;									/// @brief 하위 바이트
		byte			ha, hb, hc, hd;									/// @brief 상위 바이트 (hd=msb)
	}				b;													/// @brief 바이트 집합
	struct VINT64_T_WORD
	{
		ushort			a, b, c, d;										/// @brief 워드
	}				w;													/// @brief 워드 집합
	struct VINT64_T_DOUBLE_WORD
	{
		ushort			l, h;											/// @brief 더블 워드
	}				dw;													/// @brief 더블 워드 집합
	ullong			q;													/// @brief 쿼드 워드
} vint64_t, vllong;

/// @brief 아무값이나 넣기 위한 타입 (long double이 있으면 16, 없으면 8바이트)
typedef union ANY_T
{
	bool			b;
	int				i;
	uint			u;
	float			f;

	char*			s;
	wchar*			w;
	void*			p;
	const void*		cp;
	func_t			func;

	llong			ll;
	ullong			ull;
	double			dbl;

	byte			data[8];
} any_t, vany;

/// @brief 파라미터 함수 핸들러와 파라미터 값
typedef struct FUNCPARAM_T
{
	paramfunc_t		func;
	void*			data;
} funcparam_t;

// conditions
#define qn_return_when_fail(expr,ret)	QN_STMT_BEGIN{ if (!(expr)) return ret; }QN_STMT_END	/// @brief 값이 거짓이변 반환
#define qn_return_on_ok(expr,ret)		QN_STMT_BEGIN{ if ((expr)) return ret; }QN_STMT_END		/// @brief 값이 참이면 반환
#ifdef _DEBUG
#define qn_debug_assert(expr,msg)		QN_STMT_BEGIN{ if (!(expr)) qn_asrt(#expr, msg, __FUNCTION__, __LINE__); }QN_STMT_END	/// @brief 표현이 거짓이면 메시지 출력
#define qn_debug_verify(expr)			QN_STMT_BEGIN{ if (!(expr)) qn_asrt(#expr, NULL, __FUNCTION__, __LINE__); }QN_STMT_END	/// @brief 표현이 거짓이면 메시지 출력
#define qn_debug_mesg(b,head,mesg)		qn_mesg(b, head, mesg)									/// @brief 디버그용 메시지 출력
#define qn_debug_mesgf(b,head,fmt,...)	qn_mesgf(b, head, fmt, __VA_ARGS__)						/// @brief 디버그용 포맷 메시지 출력
#define qn_debug_outputs(msg)			qn_outputs(msg)											/// @brief 디버그용 출력
#define qn_debug_outputf(fmt,...)		qn_outputf(fmt, __VA_ARGS__)							/// @brief 디버그용 포맷 출력
#else
#define qn_debug_assert(expr,msg)
#define qn_debug_verify(expr)
#define qn_debug_mesg(b,head,mesg)
#define qn_debug_mesgf(b,head,fmt,...)
#define qn_debug_outputs(msg)
#define qn_debug_outputf(fmt,...)
#endif


//////////////////////////////////////////////////////////////////////////
// runtime

/// @brief 런타임 초기화
QSAPI void qn_runtime(void);

/// @brief 버전 문자열을 얻는다
/// @return 버전 문자열 "QS VERSION major.minor" 형식
QSAPI const char* qn_version(void);

/// @brief 프로그램 종료할 때 실행할 함수 등록
/// @param[in] func 실핼할 함수
/// @param[in] data 함수에 전달할 데이터
QSAPI void qn_atexit(paramfunc_t func, void* data);

/// @brief 프로그램 종료할 때 실행할 함수 등록 (우선 순위 높음)
/// @param[in] func 실핼할 함수
/// @param[in] data 함수에 전달할 데이터
QSAPI void qn_p_atexit(paramfunc_t func, void* data);

/// @brief size_t형으로 순서 값을 얻는다
/// @return 순서값
/// @retval 1 맨첨에 불렸을 때
QSAPI size_t qn_p_index(void);

/// @brief 뭔가 모를 때 쓰는 함수
/// @param value 값
/// @param hex 16진수 표시 여부
/// @return "UNKNOWN(??)" 또는 "UNKNOWN(0x??)" 형식의 문자열
QSAPI const char* qn_p_unknown(int value, bool hex);

/// @brief 프로퍼티를 설정한다
/// @param name 프로퍼티 이름
/// @param value 프로퍼티 값
QSAPI void qn_set_prop(const char* name, const char* value);

/// @brief 프로퍼티를 얻는다
/// @param name 프로퍼티 이름
/// @return 문자열의 프로퍼티 값
QSAPI const char* qn_get_prop(const char* name);

/// @brief 프로퍼티를 얻고 정수로 바꾼다
/// @param name 프로퍼티 이름
/// @param default_value 프로퍼티가 없을 경우 기본 값
/// @param min_value 최소값
/// @param max_value 최대값
/// @return 얻은 정수값
QSAPI int qn_get_prop_int(const char* name, int default_value, int min_value, int max_value);

/// @brief 프로퍼티를 얻고 실수로 바꾼다
/// @param name 프로퍼티 이름
/// @param default_value 프로퍼티가 없을 경우 기본 값
/// @param min_value 최소값
/// @param max_value 최대값
/// @return 얻은 실수값
QSAPI float qn_get_prop_float(const char* name, float default_value, float min_value, float max_value);

/// @brief 시스템 심볼을 등록한다
/// @param names 심볼 이름 배열
/// @param count 심볼 갯수
/// @param start_sym 시작 심볼 값
QSAPI void qn_syssym(const char** names, int count, nint start_sym);

/// @brief 심볼을 얻는다
/// @param name 심볼 이름
///	@return 심볼 값	
QSAPI nint qn_sym(const char* name);

/// @brief 심볼 디버그 출력
QSAPI void qn_symdbgout(void);

/// @brief 디버그용 검사 출력
/// @param[in] expr 검사한 표현
/// @param[in] mesg 메시지
/// @param[in] filename 파일 이름이나 함수 이름
/// @param[in] line 줄 번호
/// @return 출력한 문자열 길이
QSAPI int qn_asrt(const char* expr, const char* mesg, const char* filename, int line);

/// @brief HALT 메시지
/// @param[in] head 머릿글
/// @param[in] mesg 메시지
QSAPI NORETURN void qn_halt(const char* head, const char* mesg);

/// @brief 디버그용 문자열 출력. 출력할 문자열의 끝에 개행문자가 붙는다
/// @param[in] breakpoint 참이면 디버거 연결시 중단점 표시
/// @param[in] head 머릿글
/// @param[in] mesg 메시지
/// @return 출력한 문자열 길이
QSAPI int qn_mesg(bool breakpoint, const char* head, const char* mesg);

/// @brief 디버그용 문자열 포맷 출력. 출력할 문자열의 끝에 개행문자가 붙는다
/// @param[in] breakpoint 참이면 디버거 연결시 중단점 표시
/// @param[in] head 머릿글
/// @param[in] fmt 문자열 포맷
/// @param ... 인수
/// @return 출력한 문자열 길이
QSAPI int qn_mesgf(bool breakpoint, const char* head, const char* fmt, ...);

/// @brief 문자열을 콘솔 또는 지정한 리다이렉션으로 출력한다. 출력할 문자열의 끝에 개행문자가 붙는다
/// @param[in] mesg 출력할 내용
/// @return 출력한 문자열 길이
QSAPI int qn_outputs(const char* mesg);

/// @brief 문자열을 포맷하여 콘솔 또는 지정한 리다이렉션으로 출력한다. 출력할 문자열의 끝에 개행문자가 붙는다
/// @param[in] fmt 출력할 포맷
/// @param ... 인수
/// @return 출력한 문자열 길이
QSAPI int qn_outputf(const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// memory
#ifdef QS_NO_MEMORY_PROFILE
#define qn_alloc(count,type)			(type*)qn_a_alloc((size_t)(count)*sizeof(type),false)									/// @brief 메모리 할당
#define qn_alloc_1(type)				(type*)qn_a_alloc(sizeof(type),false)													/// @brief 메모리를 1개만 할당
#define qn_alloc_zero(count,type)		(type*)qn_a_alloc((size_t)(count)*sizeof(type),true)									/// @brief 메모리를 할당하고 메모리를 0으로
#define qn_alloc_zero_1(type)			(type*)qn_a_alloc(sizeof(type),true)													/// @brief 메모리를 1개만 할당하고 0으로
#define qn_realloc(ptr,count,type)		(type*)qn_a_realloc((void*)(ptr),(size_t)(count)*sizeof(type))							/// @brief 메모리를 다시 할당
#define qn_memdup(ptr,size_or_zero)		qn_a_mem_dup(ptr, size_or_zero)
#else
#define qn_alloc(count,type)			(type*)qn_a_i_alloc((size_t)(count)*sizeof(type),false,__FUNCTION__,__LINE__)			/// @brief 메모리 할당
#define qn_alloc_1(type)				(type*)qn_a_i_alloc(sizeof(type),false,__FUNCTION__,__LINE__)							/// @brief 메모리를 1개만 할당
#define qn_alloc_zero(count,type)		(type*)qn_a_i_alloc((size_t)(count)*sizeof(type),true,__FUNCTION__,__LINE__)			/// @brief 메모리를 할당하고 메모리를 0으로
#define qn_alloc_zero_1(type)			(type*)qn_a_i_alloc(sizeof(type),true,__FUNCTION__,__LINE__)							/// @brief 메모리를 1개만 할당하고 0으로
#define qn_realloc(ptr,count,type)		(type*)qn_a_i_realloc((void*)(ptr),(size_t)(count)*sizeof(type),__FUNCTION__,__LINE__)	/// @brief 메모리를 다시 할당
#define qn_memdup(ptr,size_or_zero)		qn_a_i_mem_dup(ptr, size_or_zero, __FUNCTION__, __LINE__)
#endif
#define qn_free(ptr)					qn_mem_free((void*)(ptr))					/// @brief 메모리 해제
#define qn_zero(ptr,count,type)			memset((ptr), 0, sizeof(type)*(count))		/// @brief 메모리를 0으로 채운다
#define qn_zero_1(ptr)					memset((ptr), 0, sizeof(*(ptr)))			/// @brief 메모리를 0으로 채운다 (주로 구조체)
#define qn_copy(dest,src,count,type)	memcpy((dest), (src), sizeof(type)*(count))	/// @brief 메모리를 복사한다
#define qn_copy_1(dest,src)				memcpy((dest), (src), sizeof(*(dest)))		/// @brief 메모리를 복사한다 (주로 구조체)

/// @brief 메모리 할당 구조체
typedef struct QNALLOCTABLE
{
#ifdef QS_NO_MEMORY_PROFILE
	void*(*_alloc)(size_t size, bool zero);
	void*(*_realloc)(void* ptr, size_t size);
#else
	void*(*_alloc)(size_t size, bool zero, const char* desc, size_t line);
	void*(*_realloc)(void* ptr, size_t size, const char* desc, size_t line);
#endif
	void(*_free)(void* ptr);
} QnAllocTable;

/// @brief 메모리를 복호화한다
/// @param[out] dest 출력 대상
/// @param[in] src 복호할 메모리
/// @param[in] size 크기
/// @return dest 그대로
QSAPI void* qn_memenc(void* dest, const void* src, size_t size);

/// @brief 복호화한 메모리를 되돌린다
/// @param[out] dest 출력 대상
/// @param[in] src 복호화 했던 메모리
/// @param[in] size 크기
/// @return dest 그대로
QSAPI void* qn_memdec(void* dest, const void* src, size_t size);

/// @brief 메모리를 압축한다. qn_free()로 해제해야 한다
/// @param[in] src 메모리 원본
/// @param[in] srcsize 원본 크기
/// @param[out] destsize 압축한 크기 (널 가능)
/// @return 압축된 메모리
QSAPI void* qn_memzcpr(const void* src, size_t srcsize, size_t* destsize);

/// @brief 메모리를 해제한다. qn_free()로 해제해야 한다
/// @param[in] src 압축된 메모리
/// @param[in] srcsize 압축된 메모리의 크기
/// @param[out] destsize 해제된 크기 (널 가능)
/// @return 해제된 메모리
QSAPI void* qn_memzucp(const void* src, size_t srcsize, size_t* destsize);

/// @brief 메모리를 해제한다. 해제 후 메모리 크기를 알아야 쓸 수 있다. 해제된 메모리는 qn_free()로 해제해야 한다
/// @param[in] src 압축된 메모리
/// @param[in] srcsize 압축된 메모리의 크기
/// @param[out] destsize 해제된 크기
/// @return 해제된 메모리
QSAPI void* qn_memzucp_s(const void* src, size_t srcsize, size_t destsize);

/// @brief 메모리 크기를 사람이 읽을 수 있는 포맷으로(human readable)
/// @param[in] size 메모리 크기
/// @param[out] out 읽을 수 있는 크기
/// @return 메모리 크기 헤더
/// @retval k 킬로 바이트
/// @retval m 메가 바이트
/// @retval g 기가 바이트
QSAPI char qn_memhrb(size_t size, double* out);

/// @brief 메모리 내용을 문자열에 담느다
/// @param[in] ptr 문자열에 담을 메모리
/// @param[in] size 메모리의 크기
/// @param[out] outbuf 출력 버퍼
/// @param[in] buflen 출력 버퍼의 크기
/// @return outbuf 인수 그대로
QSAPI char* qn_memdmp(const void* ptr, size_t size, char* outbuf, size_t buflen);

/// @brief 메모리 테이블을 등록한다
/// @param[in] table 테이블
/// @return 테이블 값 중에 빈게 있으면 거짓
/// @warning qn_runtime() 호출 이전에 사용하지 않으면 이 기능은 사용할 수 없다
QSAPI bool qn_memtbl(const QnAllocTable* table);

#ifdef QS_NO_MEMORY_PROFILE
/// @brief 메모리를 할당한다
/// @param[in] size 할당할 메모리 크기
/// @param[in] zero 할당한 메모리를 0으로 초기화 한다
/// @return 할당한 메모리
QSAPI void* qn_a_alloc(size_t size, bool zero);

/// @brief 메모리를 할당/재할당/해제한다
/// @param[in] ptr 재할당할 메모리 (이 값이 NULL이면 새로 할당)
/// @param[in] size 재할당할 메모리 크기 (이 값이 0이면 ptr을 해제)
/// @return 할당한 새로운 메모리 (메모리 주소가 안바뀔 수도 있음)
QSAPI void* qn_a_realloc(void* ptr, size_t size);

/// @brief 메모리를 복제한다
/// @param ptr 복제할 메모리
/// @param size_or_zero_if_psz 0이 아니면 그 값만금 복사, 0이면 문자열 취급
/// @return 복사된 메모리
QSAPI void* qn_a_mem_dup(const void* ptr, size_t size_or_zero_if_psz);
#else
/// @brief 내부 메모리 관리자의 메모리의 크기를 얻는다
/// @return 관리하는 메모리의 크기
QSAPI size_t qn_mpfsize(void);

/// @brief 내부 메모리 관리자의 메모리의 갯수를 얻는다
/// @return 관리하는 메모리의 총 할당 갯수
QSAPI size_t qn_mpfcnt(void);

/// @brief 내부 메모리 관리자의 내용을 디버그로 출력한다
QSAPI void qn_mpfdbgout(void);

/// @brief 메모리를 할당한다
/// @param[in] size 할당할 메모리 크기
/// @param[in] zero 할당한 메모리를 0으로 초기화 한다
/// @param[in] desc 설명문
/// @param[in] line 줄 번호
/// @return 할당한 메모리
QSAPI void* qn_a_i_alloc(size_t size, bool zero, const char* desc, size_t line);

/// @brief 메모리를 할당/재할당/해제한다
/// @param[in] ptr 재할당할 메모리 (이 값이 NULL이면 새로 할당)
/// @param[in] size 재할당할 메모리 크기 (이 값이 0이면 ptr을 해제)
/// @param[in] desc 설명문
/// @param[in] line 줄 번호
/// @return 할당한 새로운 메모리 (메모리 주소가 안바뀔 수도 있음)
QSAPI void* qn_a_i_realloc(void* ptr, size_t size, const char* desc, size_t line);

/// @brief 메모리를 복제한다
/// @param[in] ptr 복제할 메모리
/// @param[in] size_or_zero_if_psz 0이 아니면 그 값만금 복사, 0이면 문자열 취급
/// @param[in] desc 설명문
/// @param[in] line 줄 번호
/// @return 복사된 메모리
QSAPI void* qn_a_i_mem_dup(const void* ptr, size_t size_or_zero_if_psz, const char* desc, size_t line);
#endif

/// @brief 메모리를 해제한다
/// @param[in] ptr 해제할 메모리 (이 값이 NULL이면 아무것도 하지 않는다)
QSAPI void qn_mem_free(void* ptr);

/// @brief 메모리를 해제한다
///	@param[in] pptr 해제할 메모리 포인터의 포인터 (이 값이 NULL이면 아무것도 하지 않는다)
///	@warning 포인터의 포인터를 입력 받는다!
QSAPI void qn_mem_free_ptr(void* pptr);


//////////////////////////////////////////////////////////////////////////
// hash & sort

/// @brief 포인터 해시. 일반적인 size_t 해시를 의미함
/// @param[in] ptr 입력 변수
/// @return	해시 값
QSAPI size_t qn_hash_ptr(const void* ptr);

/// @brief 시간 해시. 현재 시각을 이용하여 해시값을 만든다
/// @return	해시 값
QSAPI size_t qn_hash_now(void);

/// @brief 콜백 해시. 함수 콜백을 해시값으로 만들어 준다
/// @param[in] prime8 8비트 소수 값
/// @param[in] func 콜백 함수
/// @param[in] data 콜백 데이터
/// @return	해시 값
QSAPI size_t qn_hash_func(int prime8, func_t func, const void* data);

/// @brief 데이터를 crc로 해시한다
/// @param[in] data 데이터
/// @param[in] size 데이터 크기
/// @return 해시 값
QSAPI size_t qn_hash_crc(const byte* data, size_t size);

/// @brief 가까운 소수 얻기. 처리할 수 있는 최소 소수는 QN_MAX_HASH(11), 최대 소수는 QN_MAX_HASH(13845163)
/// @param[in] value 입력 값
/// @return 소수 값
/// @see QN_MIN_HASH, QN_MAX_HASH
QSAPI uint qn_prime_near(uint value);

/// @brief 제곱 소수 얻기. 근거리에 해당하는 제곱 소수를 계산해준다
/// @param[in] value 입력 값
/// @param[in] min 최소 값
/// @param[out] shift 널이 아니면 쉬프트 크기
/// @return 소수 값
QSAPI uint qn_prime_shift(uint value, uint min, uint* shift);

/// @brief 퀵정렬
/// @param[in,out] ptr 정렬할 데이터의 포인터
/// @param[in] count 데이터의 갯수
/// @param[in] stride 데이터의 폭
/// @param[in] compfunc 비교 연산 콜백 함수
QSAPI void qn_qsort(void* ptr, size_t count, size_t stride, sortfunc_t compfunc);

/// @brief 콘텍스트 입력 받는 퀵정렬
/// @param[in,out] ptr 정렬할 데이터의 포인터
/// @param[in] count 데이터의 갯수
/// @param[in] stride 데이터의 폭
/// @param[in] compfunc 비교 연산 콜백 함수
/// @param[in] context 콜백 함수용 콘텍스트
QSAPI void qn_qsortc(void* ptr, size_t count, size_t stride, sortcfunc_t compfunc, void* context);


//////////////////////////////////////////////////////////////////////////
// string

#ifdef QS_NO_MEMORY_PROFILE
/// @brief 가변 포맷 문자열을 메모리 할당으로 만든다
/// @param[out] out 출력 문자열의 버퍼 포인터. 이 값이 NULL이면 곤린하다
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열의 길이
/// @note out 인수로 만든 문자열은 qn_free 함수로 해제해야 한다
/// @see qn_free
#define qn_vasprintf(out,fmt,va)	qn_a_vsprintf(out, fmt, va)

/// @brief 가변 포맷 문자열을 메모리 할당으로 만들어 반환한다
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열
/// @note 반환 값은 qn_free 함수로 해제해야 한다
/// @see qn_free
#define qn_vapsprintf(fmt, va)		qn_a_vpsprintf(fmt, va)

/// @brief 문자열을 포맷하고 메모리 할당으로 만든다
/// @param[out] out 출력 문자열의 버퍼 포인터. 이 값이 NULL이면 곤린하다
/// @param[in] fmt 포맷 문자열
/// @param ... 인수
/// @return 결과 문자열의 길이
/// @note out 인수로 만든 문자열은 qn_free 함수로 해제해야 한다
/// @see qn_free
#define qn_asprintf(out,fmt,...)	qn_a_sprintf(out, fmt, __VA_ARGS__)

/// @brief 문자열을 포맷하고 메모리를 반환한다
/// @param[in] fmt 포맷 문자열
/// @param ... 인수
/// @return 결과 문자열
/// @note 반환 값은 qn_free 함수로 해제해야 한다
/// @see qn_free
#define qn_apsprintf(fmt, ...)		qn_a_psprintf(fmt, __VA_ARGS__)

/// @brief 문자열을 복제한다
/// @param p 복제할 문자열
/// @return 복제한 새로운 문자열
/// @retval NULL 인수 p 가 NULL
/// @note qn_free 함수로 헤재해야 한다
#define qn_strdup(p)				qn_a_str_dup(p)

/// @brief 여러 문자열을 이어서 붙인다
/// @param[in] p 첫 문자열
/// @param ... 인수
/// @note qn_free 함수로 해제해야 한다
/// @warning 마지막 인수는 반드시 NULL이어야 한다
#define qn_strdupcat(p,...)			qn_a_str_dup_cat(p,__VA_ARGS__)

// qn_vasprintf 안쪽 
QSAPI int qn_a_vsprintf(char** out, const char* fmt, va_list va);
// qn_vapsprintf 안쪽
QSAPI char* qn_a_vpsprintf(const char* fmt, va_list va);
// qn_asprintf 안쪽
QSAPI int qn_a_sprintf(char** out, const char* fmt, ...);
// qn_apsprintf 안쪽
QSAPI char* qn_a_psprintf(const char* fmt, ...);
// qn_strdup 안쪽
QSAPI char* qn_a_str_dup(const char* p);
// qn_strcat 안쪽
QSAPI char* qn_a_str_dup_cat(const char* p, ...);
#else
/// @brief 가변 포맷 문자열을 메모리 할당으로 만든다
/// @param[out] out 출력 문자열의 버퍼 포인터. 이 값이 NULL이면 곤린하다
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열의 길이
/// @note out 인수로 만든 문자열은 qn_free 함수로 해제해야 한다
/// @see qn_free
#define qn_vasprintf(out,fmt,va)	qn_a_i_vsprintf(__FUNCTION__, __LINE__, out, fmt, va)

/// @brief 가변 포맷 문자열을 메모리 할당으로 만들어 반환한다
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열
/// @note 반환 값은 qn_free 함수로 해제해야 한다
/// @see qn_free
#define qn_vapsprintf(fmt, va)		qn_a_i_vpsprintf(__FUNCTION__, __LINE__, fmt, va)

/// @brief 문자열을 포맷하고 메모리 할당으로 만든다
/// @param[out] out 출력 문자열의 버퍼 포인터. 이 값이 NULL이면 곤린하다
/// @param[in] fmt 포맷 문자열
/// @param ... 인수
/// @return 결과 문자열의 길이
/// @note out 인수로 만든 문자열은 qn_free 함수로 해제해야 한다
/// @see qn_free
#define qn_asprintf(out,fmt,...)	qn_a_i_sprintf(__FUNCTION__, __LINE__, out, fmt, __VA_ARGS__)

/// @brief 문자열을 포맷하고 메모리를 반환한다
/// @param[in] fmt 포맷 문자열
/// @param ... 인수
/// @return 결과 문자열
/// @note 반환 값은 qn_free 함수로 해제해야 한다
/// @see qn_free
#define qn_apsprintf(fmt, ...)		qn_a_i_psprintf(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)

/// @brief 문자열을 복제한다
/// @param p 복제할 문자열
/// @return 복제한 새로운 문자열
/// @retval NULL 인수 p 가 NULL
/// @note qn_free 함수로 헤재해야 한다
#define qn_strdup(p)				qn_a_i_str_dup(__FUNCTION__, __LINE__, p)

/// @brief 여러 문자열을 이어서 붙인다
/// @param[in] p 첫 문자열
/// @param ... 인수
/// @note qn_free 함수로 해제해야 한다
/// @warning 마지막 인수는 반드시 NULL이어야 한다
#define qn_strdupcat(p,...)			qn_a_i_str_dup_cat(__FUNCTION__, __LINE__, p, __VA_ARGS__)

// qn_vasprintf 안쪽 
QSAPI int qn_a_i_vsprintf(const char* desc, size_t line, char** out, const char* fmt, va_list va);
// qn_vapsprintf 안쪽
QSAPI char* qn_a_i_vpsprintf(const char* desc, size_t line, const char* fmt, va_list va);
// qn_asprintf 안쪽
QSAPI int qn_a_i_sprintf(const char* desc, size_t line, char** out, const char* fmt, ...);
// qn_apsprintf 안쪽
QSAPI char* qn_a_i_psprintf(const char* desc, size_t line, const char* fmt, ...);
// qn_strdup 안쪽
QSAPI char* qn_a_i_str_dup(const char* desc, size_t line, const char* p);
// qn_strcat 안쪽
QSAPI char* qn_a_i_str_dup_cat(const char* desc, size_t line, const char* p, ...);
#endif

/// @brief 가변 포맷 문자열
/// @param[out] out 출력 문자열 버퍼. 문자열 길이만 얻으려면 NULL로 설정
/// @param[in] len 출력 문자열 버퍼 크기
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열의 길이
QSAPI int qn_vsnprintf(char* out, size_t len, const char* fmt, va_list va);

/// @brief 문자열을 포맷한다
/// @param[out] out 출력 문자열의 버퍼 포인터. 문자열 길이만 얻으려면 NULL로 설정
/// @param[in] len 출력 문자열의 버퍼 포인터의 크기
/// @param[in] fmt 포맷 문자열
/// @param ... 인수
/// @return 결과 문자열의 길이
QSAPI int qn_snprintf(char* out, size_t len, const char* fmt, ...);

/// @brief 문자열 복사
/// @param p 대상
/// @param src 원본
/// @return 대상 포인터
QSAPI char* qn_strcpy(char* p, const char* src);

/// @brief 문자열 길이 만큼 복사
/// @param p 대상
/// @param src 원본
/// @param len 복사할 길이
/// @return 대상 포인터
QSAPI char* qn_strncpy(char* p, const char* src, size_t len);

/// @brief 문자열을 복사하고 대상 문자열을 끝부분을 반환한다
/// @param[in,out] dest 대상 문자열
/// @param[in] src 복사할 문자열
/// @return 복사한 다음 대상 문자열 끝부분
/// @details strcpy(dest, src); dest = dest + strlen(src) 라고 보면 된다
QSAPI char* qn_stpcpy(char* dest, const char* src);

/// @brief 문자열을 뒤에 덧붙인다
/// @param[in,out] dest 대상 문자열
/// @param[in] src 덧붙일 문자열
/// @return 대상 문자열
QSAPI char* qn_strcat(char* dest, const char* src);

/// @brief 문자열 길이 만큼 뒤에 덧붙인다
/// @param[in,out] dest 대상 문자열
/// @param[in] src 덧붙일 문자열
/// @param[in] len 덧붙일 길이
/// @return 대상 문자열
QSAPI char* qn_strncat(char* dest, const char* src, size_t len);

/// @brief 문자열을 뒤에 덧붙인다
/// @param[in] max_len 최대 길이
/// @param[in,out] dest 대상 문자열
/// @param[in] ... 덧붙일 문자열
/// @return 대상 문자열
QSAPI char* qn_strconcat(size_t max_len, char* dest, ...);

/// @brief 글자로 채우기
/// @param[in,out] dest 채울 대상 버퍼
/// @param[in] pos 채울 시작 위치
/// @param[in] end 채울 끝 위치
/// @param[in] ch 채울 문자
/// @return 마지막으로 채운 곳 + 1의 위치
QSAPI size_t qn_strfll(char* dest, size_t pos, size_t end, int ch);

/// @brief 문자열 해시
/// @param[in] p 해시할 문자열
/// @return 해시 값
QSAPI size_t qn_strhash(const char* p);

/// @brief 문자열 해시 (대소문자 구별 안함)
/// @param[in] p 해시할 문자열
/// @return 해시 값
QSAPI size_t qn_strihash(const char* p);

/// @brief 문자열 해시 (대소문자 구별 안함)
/// @param[in] p 해시할 문자열
/// @return 해시 값
QSAPI uint qn_strshash(const char* p);

/// @brief 문자열 비교
/// @param p1 문자열1
/// @param p2 문자열2
/// @return 문자열 비교 값 (0보다 작으면 문자열1이, 0보다 크면 문자열2가 크다. 0이면 같음)
QSAPI int qn_strcmp(const char* p1, const char* p2);

/// @brief 문자열 길이 만큼 비교
/// @param p1 문자열1
/// @param p2 문자열2
/// @param len 비교할 길이
/// @return 문자열 비교 값 (0보다 작으면 문자열1이, 0보다 크면 문자열2가 크다. 0이면 같음)
QSAPI int qn_strncmp(const char* p1, const char* p2, size_t len);

/// @brief 문자열을 대소문자 구별없이 비교
/// @param p1 문자열1
/// @param p2 문자열2
/// @return 문자열 비교 값 (0보다 작으면 문자열1이, 0보다 크면 문자열2가 크다. 0이면 같음)
QSAPI int qn_stricmp(const char* p1, const char* p2);

/// @brief 문자열을 길이 만큼 대소문자 구별없이 비교
/// @param p1 문자열1
/// @param p2 문자열2
/// @param len 비교할 길이
/// @return 문자열 비교 값 (0보다 작으면 문자열1이, 0보다 크면 문자열2가 크다. 0이면 같음)
QSAPI int qn_strnicmp(const char* p1, const char* p2, size_t len);

/// @brief 문자열을 찾는다
/// @param[in] src 대상 문자열
/// @param[in] find 찾을 문자열
/// @param[in] index 찾기 시작할 대상 문자열의 위치
/// @return 찾으면 찾은 위치를 반환
/// @retval -1 찾지 못했을 때
QSAPI int qn_strfnd(const char* src, const char* find, size_t index);

/// @brief 문자열에서 와일드 카드 찾기
/// @param[in] string 대상 문자열
/// @param[in] wild 찾을 와일드 카드
/// @return 대상 문자열에 있으면 참
/// @copyright Written by Jack Handy - jakkhandy@hotmail.com (http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing)
QSAPI bool qn_strwcm(const char* string, const char* wild);

/// @brief 문자열에서 와일드 카드 찾기 (대소문자 구분 안함)
/// @param[in] string 대상 문자열
/// @param[in] wild 찾을 와일드 카드
/// @return 대상 문자열에 있으면 참
/// @copyright Written by Jack Handy - jakkhandy@hotmail.com (http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing)
QSAPI bool qn_striwcm(const char* string, const char* wild);

/// @brief 문자열이 서로 같은지 비교
/// @param p1 왼쪽 문자열
/// @param p2 오른쪽 문자열
/// @return 서로 같으면 참을 반환
QSAPI bool qn_streqv(const char* p1, const char* p2);

/// @brief 문자열이 서로 같은지 대소문자 무시하고 비교
/// @param p1 왼쪽 문자열
/// @param p2 오른쪽 문자열
/// @return 서로 같으면 참을 반환
QSAPI bool qn_strieqv(const char* p1, const char* p2);

/// @brief 문자열에서 문자열을 찾는다
/// @param[in] p 대상 문자열
/// @param[in] c 찾을 문자열
/// @return 찾을 경우 대상 문자열의 위치 포인터
/// @retval NULL 못 찾았다
QSAPI const char* qn_strbrk(const char* p, const char* c);

/// @brief 문자열에서 문자의 위치를 찾는다
/// @param[in] p 대상 문자열
/// @param[in] ch 찾을 문자
/// @return 찾은 위치 포인터 (못 찾으면 NULL)
QSAPI char* qn_strchr(const char* p, int ch);

/// @brief 문자열에서 뒤에서부터 문자의 위치를 찾는다
/// @param[in] p 대상 문자열
/// @param[in] ch 찾을 문자
/// @return 찾은 위치 포인터 (못 찾으면 NULL)
QSAPI char* qn_strrchr(const char* p, int ch);

/// @brief 문자열 토큰
/// @param[in,out] p 대상 문자열
/// @param[in] sep 구분자
/// @param[in,out] ctx 토큰화된 문자열의 위치
/// @return 토큰화된 문자열
QSAPI char* qn_strtok(_Inout_opt_z_ char* p, _In_z_ const char* sep, _Inout_ char** ctx);

/// @brief 구분자로 분리된 문자열에서 이름에 해당하는 문자열이 있는지 확인한다
/// @param[in] p 대상 문자열
/// @param[in] name 이름
/// @param[in] separator 구분자
/// @return 찾은 위치 포인터 (못 찾으면 NULL)
QSAPI const char* qn_strext(const char* p, const char* name, int separator);

/// @brief 문자열을 특정 위치에서 얻는다
/// @param[in,out] dest 얻은 문자열 버퍼
/// @param[in] src 원본 문자열
/// @param[in] pos 찾을 위치
/// @param[in] len 얻을 길이
/// @return 얻은 문자열 버퍼 그대로
QSAPI char* qn_strmid(char* dest, const char* src, size_t pos, size_t len);

/// @brief 문자열에서 왼쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
QSAPI char* qn_strltm(char* dest);

/// @brief 문자열에서 오른쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
QSAPI char* qn_strrtm(char* dest);

/// @brief 문자열에서 왼쪽/오른쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
QSAPI char* qn_strtrm(char* dest);

/// @brief 대상 문자열에서 지정한 문자들을 지운다
/// @param[in,out] p 대상 문자열
/// @param[in] rmlist 지울 문자 배열
/// @return 대상 문자열 그대로
QSAPI char* qn_strrem(char* p, const char* rmlist);

/// @brief 문자열을 대문자로
/// @param p 대문자로 바꿀 문자열
/// @return 대상 포인터
QSAPI char* qn_strupr(char* p);

/// @brief 문자열을 소문자로
/// @param p 소문자로 바꿀 문자열
/// @return 대상 포인터
QSAPI char* qn_strlwr(char* p);

/// @brief 문자열을 32비트 정수로
/// @param p 문자열
/// @param base 진수
/// @return 바꾼 정수값
QSAPI int qn_strtoi(const char* p, uint base);

/// @brief 문자열을 64비트 정수로
/// @param p 문자열
/// @param base 진수
/// @return 바꾼 정수값
QSAPI llong qn_strtoll(const char* p, uint base);

/// @brief 문자열을 32비트 실수로
/// @param p 문자열
/// @return 바꾼 실수값
QSAPI float qn_strtof(const char* p);

/// @brief 문자열을 64비트 실수로
/// @param p 문자열
/// @return 바꾼 실수값
QSAPI double qn_strtod(const char* p);

/// @brief 32비트 정수를 문자열로 변환
/// @param p 문자열 버퍼
/// @param n 32비트 정수
/// @param base 진수
/// @param upper 대문자로 출력
/// @return 문자열 버퍼가 0이면 필요한 버퍼 크기, 그렇지 않으면 문자열 길이
QSAPI int qn_itoa(char* p, int n, uint base, bool upper);

/// @brief 64비트 정수를 문자열로 변환
/// @param p 문자열 버퍼
/// @param n 64비트 정수
/// @param base 진수
/// @param upper 대문자로 출력
/// @return 문자열 버퍼가 0이면 필요한 버퍼 크기, 그렇지 않으면 문자열 길이
QSAPI int qn_lltoa(char* p, llong n, uint base, bool upper);

/// @brief 경로를 디렉토리와 파일 이름으로 나눈다
/// @param p 경로
/// @param dir 디렉터리 이름이 들어갈 버퍼 (널 가능)
/// @param filename 파일 이름이 들어갈 버퍼 (널 가능)
void qn_divpath(const char* p, char* dir, char* filename);

/// @brief 경로를 드라이브, 디렉토리, 파일 이름, 확장자로 나눈다
/// @param p 경로
/// @param drive 드라이브 이름이 들어갈 버퍼 (널 가능)
/// @param dir 디렉터리 이름이 들어갈 버퍼 (널 가능)
/// @param name 파일 이름이 들어갈 버퍼 (널 가능)
/// @param ext 확장자 이름이 들어갈 버퍼 (널 가능)
void qn_splitpath(const char* p, char* drive, char* dir, char* name, char* ext);

/// @brief UTF-8 문자를 UCS-4 문자로.
/// @param[in] p utf8 문자.
/// @return ucs4 문자.
QSAPI uchar4 qn_u8cbn(const char* p);

/// @brief UTF-8 다음 글자
/// @param[in] s utf8 문자열
/// @return 다음 글자 위치
QSAPI const char* qn_u8nch(const char* s);

/// @brief UTF-8 글자 길이
/// @param[in] s utf8 문자열
/// @author cp_strlen_utf8 (http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html)
QSAPI size_t qn_u8len(const char* s);

/// @brief UTF-8 문자열 복사
/// @param dest 대상 문자열
/// @param src 원본 문자열 
/// @param len 복사할 길이
/// @return 대상 문자열 버퍼 그대로
QSAPI char* qn_u8ncpy(char* dest, const char* src, size_t len);

/// @brief UTF-8 문자열 복사
/// @param dest 대상 문자열
/// @param src 원본 문자열
/// @param len 복사할 길이
/// @return 대상 문자열의 길이
QSAPI size_t qn_u8lcpy(char* dest, const char* src, size_t len);

/// @brief UCS-4 문자를 UTF-8 문자로
/// @param[in] c ucs4 문자.
/// @param[out] out	utf8 문자가 들어갈 배열. 최소 6개 항목의 배열이어야 한다
/// @return utf8문자를 구성하는 문자열의 길이.
QSAPI int qn_u32ucb(uchar4 c, char* out);

/// @brief UTF-16 문자를 UTF-8 문자로
/// @param[in] high UTF-16 상위 서로게이트
/// @param[in] low UTF-16 하위 서로게이트
/// @param[out] out	utf8 문자가 들어갈 배열. 최소 6개 항목의 배열이어야 한다
/// @return utf8문자를 구성하는 문자열의 길이.
QSAPI int qn_u16ucb(uchar2 high, uchar2 low, char* out);

/// @brief 멀티바이트 문자열을 와이드 문자열로 변환
/// @param[out] outwcs 출력할 와이드 문자열 버퍼 (NULL 가능)
/// @param[in] outsize 와이드 문자열 버퍼 크기
/// @param[in] inmbs 변환할 멀티바이트 문자열
/// @param[in] insize 멀티바이트 문자열의 크기 (0이어도 된다)
/// @return 와이드 문자열의 길이
/// @remark 와이드 문자열 버퍼가 NULL이면 최종 문자열 길이만 얻는다
/// @see mbstowcs
QSAPI size_t qn_mbstowcs(wchar* outwcs, size_t outsize, const char* inmbs, size_t insize);

/// @brief 와이드 문자열을 멀티바이트 문자열로 변환
/// @param[out] outmbs 출력할 멀티바이트 문자열 버퍼 (NULL 가능)
/// @param[in] outsize 멀티바이트 문자열 버퍼 크기
/// @param[in] inwcs 변환할 와이드 문자열
/// @param[in] insize 와이드 문자열의 크기 (0이어도 된다)
/// @return 멀티바이트 문자열의 길이
/// @remark 멀티바이트 문자열 버퍼가 NULL이면 최종 문자열 길이만 얻는다
/// @see wcstombs
QSAPI size_t qn_wcstombs(char* outmbs, size_t outsize, const wchar* inwcs, size_t insize);

/// @brief utf8 -> ucs4 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (ucs4)
/// @param[in] destsize 대상 버퍼 크기
/// @param[in] src 원본 (utf8)
/// @param[in] srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
QSAPI size_t qn_u8to32(uchar4* dest, size_t destsize, const char* src, size_t srclen);

/// @brief utf8 -> utf16 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (utf16)
/// @param[in] destsize 대상 버퍼 크기
/// @param[in] src 원본 (utf8)
/// @param[in] srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
QSAPI size_t qn_u8to16(uchar2* dest, size_t destsize, const char* src, size_t srclen);

/// @brief ucs4 -> utf8 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (utf8)
/// @param[in] destsize 대상 버퍼 크기
/// @param[in] src 원본 (ucs4)
/// @param[in] srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
QSAPI size_t qn_u32to8(char* dest, size_t destsize, const uchar4* src, size_t srclen);

/// @brief utf16 -> utf8 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (utf8)
/// @param[in] destsize 대상 버퍼 크기
/// @param[in] src 원본 (utf16)
/// @param[in] srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
QSAPI size_t qn_u16to8(char* dest, size_t destsize, const uchar2* src, size_t srclen);

/// @brief utf16 -> ucs4 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (ucs4)
/// @param[in] destsize 대상 버퍼 크기
/// @param[in] src 원본 (utf16)
/// @param[in] srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
QSAPI size_t qn_u16to32(uchar4* dest, size_t destsize, const uchar2* src, size_t srclen);

/// @brief ucs4 -> utf16 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (utf16)
/// @param[in] destsize 대상 버퍼 크기
/// @param[in] src 원본 (ucs4)
/// @param[in] srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
QSAPI size_t qn_u32to16(uchar2* dest, size_t destsize, const uchar4* src, size_t srclen);

#ifdef QS_NO_MEMORY_PROFILE
/// @brief 멀티바이트 문자열 -> 와이드 문자열로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_mbstowcs_dup(src,srclen)		qn_a_mbstowcs(src,srclen)

/// @brief 와이드 문자열 -> 멀티바이트 문자열로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_wcstombs_dup(src,srclen)		qn_a_wcstombs(src,srclen)

/// @brief utf8 -> ucs4로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u8to32_dup(src,srclen)		qn_a_u8to32(src,srclen)

/// @brief utf8 -> utf16으로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u8to16_dup(src,srclen)		qn_a_u8to16(src,srclen)

/// @brief ucs4 -> utf8로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u32to8_dup(src,srclen)		qn_a_u32to8(src,srclen)

/// @brief utf16 -> utf8로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u16to8_dup(src,srclen)		qn_a_u16to8(src,srclen)

/// @brief utf16 -> ucs4로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u16to32_dup(src,srclen)		qn_a_u16to32(src,srclen)

/// @brief ucs4 -> utf16으로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u32to16_dup(src,srclen)		qn_a_u32to16(src,srclen)

QSAPI wchar* qn_a_mbstowcs(const char* src, size_t srclen);
QSAPI char* qn_a_wcstombs(const wchar* src, size_t srclen);
QSAPI uchar4* qn_a_u8to32(const char* src, size_t srclen);
QSAPI uchar2* qn_a_u8to16(const char* src, size_t srclen);
QSAPI char* qn_a_u32to8(const uchar4* src, size_t srclen);
QSAPI char* qn_a_u16to8(const uchar2* src, size_t srclen);
QSAPI uchar4* qn_a_u16to32(const uchar2* src, size_t srclen);
QSAPI uchar2* qn_a_u32to16(const uchar4* src, size_t srclen);
#else
/// @brief 멀티바이트 문자열 -> 와이드 문자열로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_mbstowcs_dup(src,srclen)		qn_a_i_mbstowcs(src,srclen,__FUNCTION__,__LINE__)

/// @brief 와이드 문자열 -> 멀티바이트 문자열로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_wcstombs_dup(src,srclen)		qn_a_i_wcstombs(src,srclen,__FUNCTION__,__LINE__)

/// @brief utf8 -> ucs4로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u8to32_dup(src,srclen)		qn_a_i_u8to32(src,srclen,__FUNCTION__,__LINE__)

/// @brief utf8 -> utf16으로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u8to16_dup(src,srclen)		qn_a_i_u8to16(src,srclen,__FUNCTION__,__LINE__)

/// @brief ucs4 -> utf8로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u32to8_dup(src,srclen)		qn_a_i_u32to8(src,srclen,__FUNCTION__,__LINE__)

/// @brief utf16 -> utf8로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u16to8_dup(src,srclen)		qn_a_i_u16to8(src,srclen,__FUNCTION__,__LINE__)

/// @brief utf16 -> ucs4로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u16to32_dup(src,srclen)		qn_a_i_u16to32(src,srclen,__FUNCTION__,__LINE__)

/// @brief ucs4 -> utf16으로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
#define qn_u32to16_dup(src,srclen)		qn_a_i_u32to16(src,srclen,__FUNCTION__,__LINE__)

QSAPI wchar* qn_a_i_mbstowcs(const char* src, size_t srclen, const char* desc, size_t line);
QSAPI char* qn_a_i_wcstombs(const wchar* src, size_t srclen, const char* desc, size_t line);
QSAPI uchar4* qn_a_i_u8to32(const char* src, size_t srclen, const char* desc, size_t line);
QSAPI uchar2* qn_a_i_u8to16(const char* src, size_t srclen, const char* desc, size_t line);
QSAPI char* qn_a_i_u32to8(const uchar4* src, size_t srclen, const char* desc, size_t line);
QSAPI char* qn_a_i_u16to8(const uchar2* src, size_t srclen, const char* desc, size_t line);
QSAPI uchar4* qn_a_i_u16to32(const uchar2* src, size_t srclen, const char* desc, size_t line);
QSAPI uchar2* qn_a_i_u32to16(const uchar4* src, size_t srclen, const char* desc, size_t line);
#endif


//////////////////////////////////////////////////////////////////////////
// gam

/// @brief GAM 가상 테이블 이름을 얻는다
#define QN_DECL_VTABLE(TYPE)		struct _VT_##TYPE
/// @brief GAME 부모 선언
#define QN_GAM_BASE(TYPE)			struct TYPE base
/// @brief GAM 가상 테이블 부모 선언
#define QN_GAM_VTABLE(TYPE)			struct _VT_##TYPE base

/// @brief GAM 포인터를 다른 타입으로 캐스팅한다
#define qn_cast_type(g,type)		((type*)(g))
/// @brief GAM 가상 테이블로 GAM 포인터를 캐스팅한다
#define qn_cast_vtable(g,TYPE)		((QN_DECL_VTABLE(TYPE)*)((QnGamBase*)(g))->vt)

/// @brief GAM 포인터
typedef void *QNGAM, *QnGam;

/// @brief GAM 할거리를 의미. 영어로 OBJECT 개념
typedef struct QNGAMBASE			QnGamBase;
/// @brief GAM 함수 테이블
typedef struct _VT_QNGAMBASE		QnGamBaseVT;

//
QN_DECL_VTABLE(QNGAMBASE)
{
	const char* name;
	void (*dispose)(QnGam);
};

//
struct QNGAMBASE
{
	const QnGamBaseVT*	vt;
	volatile nint		ref;
	nuint				desc;
};

/// @brief GAM 가상 테이블을 초기화하고 GAM 포인터 반환
/// @param g 현재 오브젝트
/// @param vt 가상 테이블
/// @return 현재 오브젝트 그대로
QSAPI QnGam qn_sc_init(QnGam g, const void* vt);

/// @brief 참조를 추가한다.
/// @param g 현재 오브젝트
/// @return 현재 오브젝트 그대로
QSAPI QnGam qn_sc_load(QnGam g);

/// @brief 참조를 제거한다. 참조가 0이 되면 제거한다
/// @param g 현재 오브젝트
/// @return 현재 오브젝트 그대로
QSAPI QnGam qn_sc_unload(QnGam g);

/// @brief 참조를 얻는다
/// @param g 현재 오브젝트
/// @return 현재 참조값
QSAPI nint qn_sc_get_ref(QnGam g);

/// @brief 표현자(디스크립터)를 얻는다
/// @param g 현재 오브젝트
/// @return 현재 표현자
QSAPI nuint qn_sc_get_desc(QnGam g);

/// @brief 표현자(디스크립터)를 쓴다
/// @param g 현재 오브젝트
/// @param ptr 표현자(디스크립터)
/// @return 설정하기 전에 갖고 있던 이전 표현자
QSAPI nuint qn_sc_set_desc(QnGam g, nuint ptr);

/// @brief GAM 가상 테이블을 초기화하고 GAM 포인터 반환
#define qn_gam_init(g,vt)			(qn_sc_init((QnGam)(g), &(vt)))
/// @brief 참조를 얻는다
#define qn_gam_ref(g)				qn_sc_get_ref((QnGam)(g))
/// @brief 참조를 추가한다
#define qn_load(g)					((g) ? qn_sc_load((QnGam)(g)) : NULL)
/// @brief 참조를 제거한다. 참조가 0이 되면 제거한다
#define qn_unload(g)				((g) ? qn_sc_unload((QnGam)(g)) : NULL)
/// @brief 참조를 추가한다 (타입 변환)
#define qn_loadc(g,type)			((g) ? (type*)(qn_sc_load((QnGam)(g))) : NULL)
/// @brief 참조를 제거한다. 참조가 0이 되면 제거한다
#define qn_unloadc(g)				((g) ? qn_sc_unload((QnGam)(g)) : NULL)
/// @brief 참조를 추가한다. 널 검사 안한다 (타입 변환)
#define qn_loadu(g,type)			(type*)qn_sc_load((QnGam)(g))
/// @brief 참조를 제거한다. 참조가 0이 되면 제거한다. 널 검사 안한다
#define qn_unloadu(g)				qn_sc_unload((QnGam)(g))
/// @brief 표현자(디스크립터)를 얻는다
#define qn_get_gam_desc(g,type)		(type)qn_sc_get_desc((QnGam)(g))
/// @brief 표현자(디스크립터)를 쓴다
#define qn_set_gam_desc(g,ptr)		qn_sc_set_desc((QnGam)(g),(nuint)(ptr))
#ifdef _QN_WINDOWS_
/// @brief (WIN32) 핸들을 표현자로 읽는다
#define qn_get_gam_handle(g)		qn_get_gam_desc(g,HANDLE)
#endif
/// @brief 포인터를 표현자로 읽는다
#define qn_get_gam_pointer(p)		qn_get_gam_desc(p,void*)
/// @brief 정수를 표현자로 읽는다
#define qn_get_gam_desc_int(p)		qn_get_gam_desc(p,int)


//////////////////////////////////////////////////////////////////////////
// time

/// @brief 로컬 시간으로 변화
/// @param[out]	ptm	시간 구조체
/// @param[in] tt time_t 형식의 시간
QSAPI void qn_localtime(struct tm* ptm, time_t tt);

/// @brief UTC 시간으로 변화
/// @param[out]	ptm	시간 구조체
/// @param[in] tt time_t 형식의 시간
QSAPI void qn_gmtime(struct tm* ptm, time_t tt);

/// @brief 현재 시간 사이클
/// @return	현재의 사이클
QSAPI ullong qn_cycle(void);

/// @brief 프로그램 시작부터 시간 틱
/// @return	현재의 틱
QSAPI ullong qn_tick(void);

/// @brief 프로그램 시작부터 시간 틱 (32비트 버전)
/// @return	현재의 틱
QSAPI uint qn_tick32(void);

/// @brief 밀리초 슬립
/// @param[in] milliseconds	밀리초 단위로 처리되는 millisecond
QSAPI void qn_sleep(uint milliseconds);

/// @brief 초(second) 슬립
/// @param[in] seconds	초 단위로 처리되는 second
QSAPI void qn_ssleep(double seconds);

/// @brief 마이크로 슬립, 정밀 시계를 이용하며 스레드 콘텍스트가 일반 슬립보다 제한된다
/// @param[in] microseconds	마이크로초 단위로 처리되는 microsecond
QSAPI void qn_msleep(ullong microseconds);

/// @brief time stamp
typedef ullong QnTimeStamp;

/// @brief date time
typedef union QNDATETIME
{
	QnTimeStamp			stamp;				/// @brief 타임 스탬프
	struct
	{
		uint			date;				/// @brief 날짜 스탬프
		uint			time;				/// @brief 시간 스탬프
	};
	struct
	{
		uint			year : 14;			/// @brief 년
		uint			month : 6;			/// @brief 월
		uint			day : 8;			/// @brief 일
		uint			dow : 4;			/// @brief 한주의 일

		uint			hour : 6;			/// @brief 시
		uint			minute : 8;			/// @brief 분
		uint			second : 8;			/// @brief 초
		uint			millisecond : 10;	/// @brief 밀리초
	};
} QnDateTime;

/// @brief 현재 시간 날짜를 포함하는 타임스탬프
/// @returns 현재 타임스탬프. QnDateTime 으로 컨버전해서 사용할 수 있다
QSAPI QnTimeStamp qn_now(void);

/// @brief 현재의 UTC 시간 날짜를 포함하는 타임스탬프
/// @returns 현재 타임스탬프. QnDateTime 으로 컨버전해서 사용할 수 있다
QSAPI QnTimeStamp qn_utc(void);

/// @brief 초를 시간으로
/// @param[in] sec 초
/// @return 계산된 타임스탬프
QSAPI QnTimeStamp qn_stod(double sec);

/// @brief 밀리초를 시간으로
/// @param msec 밀리초 
/// @return 계산된 타임스탬프
QSAPI QnTimeStamp qn_mstod(uint msec);

/// @brief timer
typedef struct QNTIMER
{
	QN_GAM_BASE(QNGAMBASE);
	double				abstime;			/// @brief 타이머 절대 시간
	double				runtime;			/// @brief 타이머 시작부터 수행 시간
	double				advance;			/// @brief 타이머 갱신에 따른 시간 (프레임 당 시간)
	double				fps;				/// @brief 초 당 프레임 수
} QnTimer;

/// @brief 타이머 만들기
/// @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 만들어진 타이머
QSAPI QnTimer* qn_create_timer(void);

/// @brief 타이머 리셋
/// @param[in] self 타이머 개체
QSAPI void qn_timer_reset(QnTimer* self);

/// @brief 타이머 시작
/// @param[in] self 타이머 개체
QSAPI void qn_timer_start(QnTimer* self);

/// @brief 타이머 정지
/// @param[in] self 타이머 개체
QSAPI void qn_timer_stop(QnTimer* self);

/// @brief 타이머 갱신
/// @param[in] self 타이머 개체
/// @param[in] manual FPS를 자동으로 계산하려면 false, 아니면 true
/// @return	성공 여부 반화
/// @retval true 성공
/// @retval false 실패
QSAPI bool qn_timer_update(QnTimer* self, bool manual);

/// @brief 타이머 갱신
/// @param self 타이머 개체
/// @param manual FPS를 자동으로 계산하려면 false, 아니면 true
/// @param target_fps 목표 FPS 
/// @return 성공 여부
bool qn_timer_update_fps(QnTimer* self, bool manual, double target_fps);

/// @brief 타이머의 절대 시간
/// @param[in] self 타이머 개체
/// @return	double
QSAPI double qn_timer_get_cut(const QnTimer* self);

/// @brief 타이머 과다 수행에 따른 갱신 경과값의 설정
/// @param[in] self 타이머 개체
/// @param	cut 제한 값
QSAPI void qn_timer_set_cut(QnTimer* self, double cut);

/// @brief 타이머의 절대 시간
/// @param[in] self 타이머 개체
/// @return	double
INLINE double qn_timer_get_abstime(const QnTimer* self) { return self->abstime; }

/// @brief 타이머의 수행 시간
/// @param[in] self 타이머 개체
/// @return	double
INLINE double qn_timer_get_runtime(const QnTimer* self) { return self->runtime; }

/// @brief 타이머의 갱신 시간
/// @param[in] self 타이머 개체
/// @return	double
INLINE double qn_timer_get_advance(const QnTimer* self) { return self->advance; }

/// @brief 타이머의 초당 프레임 수
/// @param[in] self 타이머 개체
/// @return	double
INLINE double qn_timer_get_fps(const QnTimer* self) { return self->fps; }


//////////////////////////////////////////////////////////////////////////
// disk i/o

/// @brief 패스 전용 문자열
typedef struct QNPATHSTR
{
	size_t LENGTH;
	char DATA[QN_MAX_PATH];
} QnPathStr;

/// @brief 파일 정보
typedef struct QNFILEINFO
{
	ushort			type;									/// @brief 파일 타입
	ushort			len;									/// @brief 파일 이름 길이
	uint			extra;
	uint			size;									/// @brief 파일 크기
	uint			cmpr;									/// @brief 압축된 크기
	QnTimeStamp		stc;									/// @brief 만든 날짜 타임 스탬프
	QnTimeStamp		stw;									/// @brief 마지막 기록한 타임 스탬프
} QnFileInfo;

/// @brief 파일 정보2
typedef struct QNFILEINFO2
{
	ushort			type;									/// @brief 파일 타입 (0=디렉토리, 1=파일)
	ushort			len;									/// @brief 파일 이름 길이
	uint			extra;
	uint			size;									/// @brief 파일 크기
	uint			cmpr;									/// @brief 압축된 크기
	QnTimeStamp		stc;									/// @brief 만든 날짜 타임 스탬프
	QnTimeStamp		stw;									/// @brief 마지막 기록한 타임 스탬프
	const char*		name;									/// @brief 파일 이름
} QnFileInfo2;

/// @brief 파일 제어
typedef struct QNFILEACCESS
{
#ifdef _MSC_VER
	uint			mode;									/// @brief 제어 모드
	uint			share;									/// @brief 공유 모드
	uint			access;									/// @brief 제어 방법
	uint			attr;									/// @brief 파일 속성
#else
	int				mode;
	uint			access;
#endif
} QnFileAccess;

/// @brief 파일에서 위치 찾기
typedef enum QNSEEK
{
	QNSEEK_BEGIN = 0,										/// @brief 처음부터
	QNSEEK_CUR = 1,											/// @brief 현 위치부터
	QNSEEK_END = 2,											/// @brief 끝부터
} QnSeek;

/// @brief 파일 플래그
typedef enum QNFILEFLAG
{
	QNFF_NONE = 0,											/// @brief 없음
	QNFF_READ = QN_BIT(0),									/// @brief 읽기
	QNFF_WRITE = QN_BIT(1),									/// @brief 쓰기
	QNFF_SEEK = QN_BIT(2),									/// @brief 위치 찾기
	QNFF_APPEND = QN_BIT(3),								/// @brief 끝에 추가
	QNFF_TEXT = QN_BIT(4),									/// @brief 텍스트 파일
	QNFF_BINARY = QN_BIT(5),								/// @brief 바이너리 파일

	QNFFT_FILE = QN_BIT(16),								/// @brief 디스크 파일 시스템 파일
	QNFFT_HFS = QN_BIT(17),									/// @brief HFS 파일
	QNFFT_MEM = QN_BIT(18),									/// @brief 메모리 파일
} QnFileFlag;

/// @brief 파일 타입
typedef enum QNFILETYPE
{
	QNFTYPE_UNKNOWN = 0,									/// @brief 알 수 없음
	QNFTYPE_SYSTEM = 1,										/// @brief 시스템
	QNFTYPE_TEXT = 4,										/// @brief 텍스트
	QNFTYPE_PICTURE = 5,									/// @brief 그림
	QNFTYPE_SOUND = 6,										/// @brief 소리
	QNFTYPE_VIDEO = 7,										/// @brief 비디오
	QNFTYPE_CODE = 8,										/// @brief 코드
	QNFTYPE_PROGRAM = 9,									/// @brief 프로그램
	QNFTYPE_SCRIPT = 10,									/// @brief 스크립트
	QNFTYPE_ARCHIVE = 11,									/// @brief 아카이브
	QNFTYPE_MARKUP = 12,									/// @brief 마크업
	QNFTYPE_MARKDOWN = 13,									/// @brief 마크다운
	QNFTYPE_JSON = 14,										/// @brief JSON
	QNFTYPE_TOML = 15,										/// @brief TOML
} QnFileType;

/// @brief 파일 속성
typedef enum QNFILEATTR
{
	QNFATTR_NONE = 0,										/// @brief 파일이 없음
	QNFATTR_FILE = QN_BIT(0),								/// @brief 파일
	QNFATTR_DIR = QN_BIT(1),								/// @brief 디렉토리
	QNFATTR_SYSTEM = QN_BIT(2),								/// @brief 시스템
	QNFATTR_HIDDEN = QN_BIT(3),								/// @brief 숨김 (윈도우만)
	QNFATTR_RDONLY = QN_BIT(4),								/// @brief 읽기 전용 (윈도우만)
	QNFATTR_LINK = QN_BIT(5),								/// @brief 링크
	QNFATTR_CMPR = QN_BIT(6),								/// @brief 압축 (윈도우/HFS만)
	QNFATTR_ENCR = QN_BIT(7),								/// @brief 암호화 (윈도우/HFS만)
} QnFileAttr;

/// @brief 마운트 플래그
typedef enum QNMOUNTFLAG
{
	QNMF_NONE = 0,											/// @brief 없음
	QNMF_READ = QN_BIT(0),									/// @brief 읽기
	QNMF_WRITE = QN_BIT(1),									/// @brief 쓰기

	QNMFT_DISKFS = QN_BIT(16),								/// @brief 디스크 파일 시스템
	QNMFT_MEM = QN_BIT(17),									/// @brief 메모리 파일 시스템
	QNMFT_HFS = QN_BIT(18),									/// @brief HFS 파일 시스템
} QnMountFlag;

/// @brief 한번에 파일을 읽는 기능들에 대해 최대 허용 크기 (초기값은 128MB)
QSAPI size_t qn_get_file_max_alloc_size(void);

/// @brief 한번에 파일을 읽는 기능들의 최대 허용 크기 지정 (0으로 지정하면 128MB로 초기화)
QSAPI void qn_set_file_max_alloc_size(size_t size);

/// @brief 파일 속성을 얻는다
/// @param path 파일 경로
/// @return 파일 속성
/// @see QnFileAttr
QSAPI QnFileAttr qn_get_file_attr(const char* path);

/// @brief 파일 이름에서 경로를 뽑느다
/// @param filename 대상 파일 이름
/// @param dest 경로를 넣을 버퍼 (길이를 얻고 싶으면 이 값을 널로 지정)
/// @param destsize 버퍼의 크기
/// @return 경로의 길이. 버퍼의 크기보다 길더라도 버퍼의 크기-1 만큼만 반환한다
QSAPI size_t qn_filepath(const char* filename, char* dest, size_t destsize);

/// @brief 프로그램 기본 패스를 얻는다
/// @return 기본 패스
/// @warning 반환 값은 qn_free 함수로 해제해야한다
QSAPI char* qn_basepath(void);


//////////////////////////////////////////////////////////////////////////
// stream + directory + mount

/// @brief 스트림
typedef struct QNSTREAM		QnStream;
/// @brief 파일 목록
typedef struct QNDIR		QnDir;
/// @brief 마운트
typedef struct QNMOUNT		QnMount;

/// @brief 스트림 타입
struct QNSTREAM
{
	QN_GAM_BASE(QNGAMBASE);
	QnMount*			mount;
	char*				name;
	QnFileFlag			flags;
};

QN_DECL_VTABLE(QNSTREAM)
{
	QN_GAM_VTABLE(QNGAMBASE);
	int (*stream_read)(/*QnStream*/QnGam, void*, int, int);
	int (*stream_write)(/*QnStream*/QnGam, const void*, int, int);
	llong(*stream_seek)(/*QnStream*/QnGam, llong, QnSeek);
	llong(*stream_tell)(/*QnStream*/QnGam);
	llong(*stream_size)(/*QnStream*/QnGam);
	bool (*stream_flush)(/*QnStream*/QnGam);
	QnStream* (*stream_dup)(/*QnStream*/QnGam);
};

/// @brief 디렉토리 정보
struct QNDIR
{
	QN_GAM_BASE(QNGAMBASE);
	QnMount*			mount;
	char*				name;
};

QN_DECL_VTABLE(QNDIR)
{
	QN_GAM_VTABLE(QNGAMBASE);
	const char* (*dir_read)(/*QnDir*/QnGam);
	bool (*dir_read_info)(/*QnDir*/QnGam, QnFileInfo2*);
	void (*dir_rewind)(/*QnDir*/QnGam);
	void (*dir_seek)(/*QnDir*/QnGam, int);
	int (*dir_tell)(/*QnDir*/QnGam);
};

/// @brief 마운트 타입
struct QNMOUNT
{
	QN_GAM_BASE(QNGAMBASE);
	char*				name;
	size_t				name_len;
	QnPathStr			path;
	QnMountFlag			flags;
};

QN_DECL_VTABLE(QNMOUNT)
{
	QN_GAM_VTABLE(QNGAMBASE);
	QnStream* (*mount_open)(/*QnMount*/QnGam, const char*, const char*);
	void* (*mount_read)(/*QnMount*/QnGam, const char*, int*);
	char* (*mount_read_text)(/*QnMount*/QnGam, const char*, int*, int*);
	QnFileAttr (*mount_exist)(/*QnMount*/QnGam, const char*);
	bool (*mount_remove)(/*QnMount*/QnGam, const char*);
	bool (*mount_chdir)(/*QnMount*/QnGam, const char*);
	bool (*mount_mkdir)(/*QnMount*/QnGam, const char*);
	QnDir* (*mount_list)(/*QnMount*/QnGam);
};

// 스트림 보조 함수

/// @brief 파일이 존재하는지 확인
/// @param mount 마운트 (널이면 디스크 파일 시스템)
/// @param filename 파일 이름
/// @return 파일 속성
/// @see QnFileAttr
QSAPI QnFileAttr qn_file_attr(QnMount* mount, const char* filename);

/// @brief 파일을 한번에 읽는다
/// @param mount 마운트 (널이면 디스크 파일 시스템)
/// @param filename 파일이름
/// @param size 파일의 크기 (널 가능)
/// @return 파일 데이터
QSAPI void* qn_file_alloc(QnMount* mount, const char* filename, int* size);

/// @brief 텍스트 파일을 한번에 읽는다
/// @param mount 마운트 (널이면 디스크 파일 시스템)
/// @param filename 파일이름
/// @param length 텍스트의 길이
/// @param codepage 코드 페이지 (1200=UTF-16LE, 1201=UTF-16BE, 65001=UTF-8, 0=ANSI/시스템로캘 또는 UTF-8) 
/// @return 텍스트 데이터
QSAPI char* qn_file_alloc_text(QnMount* mount, const char* filename, int* length, int* codepage);

// 스트림

/// @brief 파일 스트림 열기
/// @param mount 마운트 (널이면 디스크 파일 시스템)
/// @param filename 파일 이름
/// @param mode 모드
/// @return 스트림
/// mount 인수가 널이면 파일 시스템에서 파일을 연다
QSAPI QnStream* qn_open_stream(QnMount* mount, const char* filename, const char* mode);

/// @brief 스트림 읽기
/// @param self 스트림
/// @param buffer 버퍼
/// @param offset 오프셋
/// @param size 크기
/// @return 읽은 크기
QSAPI int qn_stream_read(QnStream* self, void* buffer, int offset, int size);

/// @brief 스트림 쓰기
/// @param self 스트림
/// @param buffer 버퍼
/// @param offset 오프셋
/// @param size 크기
/// @return 쓴 크기
QSAPI int qn_stream_write(QnStream* self, const void* buffer, int offset, int size);

/// @brief 스트림 위치 찾기
/// @param self 스트림
/// @param offset 오프셋
/// @param org 기준
/// @return 새로운 위치
QSAPI llong qn_stream_seek(QnStream* self, llong offset, QnSeek org);

/// @brief 스트림 위치 얻기
/// @param self 스트림
/// @return 현재 위치
QSAPI llong qn_stream_tell(QnStream* self);

/// @brief 스트림 크기 얻기
/// @param self 스트림
/// @return 크기
QSAPI llong qn_stream_size(QnStream* self);

/// @brief 스트림 갱신
/// @param self 스트림
/// @return 성공 여부
QSAPI bool qn_stream_flush(QnStream* self);

/// @brief 스트림 복제
/// @param self 스트림
/// @return 복제된 스트림
QSAPI QnStream* qn_stream_dup(QnStream* self);

/// @brief 스트림에 포맷하여 쓰기
/// @param self 스트림
/// @param fmt 포맷
/// @param va 가변 인수
/// @return 쓴 크기
QSAPI int qn_stream_vprintf(QnStream* self, const char* fmt, va_list va);

/// @brief 스트림에 포맷하여 쓰기
/// @param self 스트림
/// @param fmt 포맷
/// @param ... 가변 인수
/// @return 쓴 크기
QSAPI int qn_stream_printf(QnStream* self, const char* fmt, ...);

/// @brief 스트림 이름 얻기
/// @param self 스트림
/// @return 이름
INLINE const char* qn_stream_get_name(QnStream* self) { return self->name; }

/// @brief 스트림 읽을 수 있나
/// @param self 스트림
/// @return 참이면 읽을 수 있다
INLINE bool qn_stream_can_read(QnStream* self) { return QN_TMASK(self->flags, QNFF_READ); }

/// @brief 스트림 쓸 수 있나
/// @param self 스트림
/// @return 참이면 쓸 수 있다
INLINE bool qn_stream_can_write(QnStream* self) { return QN_TMASK(self->flags, QNFF_WRITE); }

/// @brief 스트림 위치 찾을 수 있나
/// @param self 스트림
/// @return 참이면 위치 찾을 수 있다
INLINE bool qn_stream_can_seek(QnStream* self) { return QN_TMASK(self->flags, QNFF_SEEK); }

// 메모리 스트림

/// @brief 메모리 스트림 만들기
/// @param name 스트림 이름
/// @param initial_capacity 초기 용량
/// @return 만든 메모리 스트림
QSAPI QnStream* qn_create_mem_stream(const char* name, size_t initial_capacity);

/// @brief 메모리 스트림 외부 데이터로 만들기
/// @param name 스트림 이름
/// @param data 데이터로 사용할 버퍼
/// @param size 버퍼의 크기
/// @return 만든 메모리 스트림
QSAPI QnStream* qn_create_mem_stream_stored(const char* name, void* data, size_t size);

/// @brief 메모리 스트림의 데이터 얻기
/// @param self 스트림
/// @return 메모리 포인터
QSAPI void* qn_mem_stream_get_data(QnStream* self);

// 디렉토리

/// @brief 파일 목록에서 항목 읽기
/// @param self 디렉토리
/// @return 항목 이름
QSAPI const char* qn_dir_read(QnDir* self);

/// @brief 파일 목록에서 항목의 정보 읽기
/// @param self 디렉토리
/// @param info 항목 정보
QSAPI bool qn_dir_read_info(QnDir* self, QnFileInfo2* info);

/// @brief 파일 목록을 처음으로 감기
/// @param self 디렉토리
QSAPI void qn_dir_rewind(QnDir* self);

/// @brief 파일 목록에서 순서 항목으로 찾아가기
/// @param self 디렉토리
/// @param pos 위치
QSAPI void qn_dir_seek(QnDir* self, int pos);

/// @brief 파일 목록에서 몇번째 항목인지 얻기
/// @param self 디렉토리
/// @return 위치
QSAPI int qn_dir_tell(QnDir* self);

// 마운트

/// @brief 마운트 열기
/// @param path 마운트 경로
/// @param mode 모드 (h=HFS, m=메모리, c=만들기, +=읽고 쓰기)
/// @return 만든 마운트
/// @note path를 널 값으로 하여 파일 시스템을 열 때는 실행 파일이 있는 경로를 기준으로 한다
QSAPI QnMount* qn_open_mount(const char* path, const char* mode);

/// @brief 마운트에서 파일 열기
/// @param self 마운트
/// @param filename 파일 이름
/// @param mode 모드
/// @return 파일 스트림
QSAPI QnStream* qn_mount_open_stream(QnMount* self, const char* filename, const char* mode);

/// @brief 마운트에서 파일 한꺼번에 읽기
/// @param self 마운트
/// @param filename 파일 이름
/// @param size 파일 크기 (널 가능)
/// @return 파일 데이터
QSAPI void* qn_mount_read(QnMount* self, const char* filename, int* size);

/// @brief 마운트에서 텍스트 파일 한꺼번에 읽기
/// @param self 마운트
/// @param filename 파일 이름
/// @param length 텍스트의 길이
/// @param codepage 코드 페이지 (1200=UTF-16LE, 1201=UTF-16BE, 65001=UTF-8, 0=ANSI 또는 UTF-8)
/// @return 텍스트 데이터
/// UTF-32로 텍스트를 저장하는 일은 없어서 지원 안넣었슴
QSAPI char* qn_mount_read_text(QnMount* self, const char* filename, int* length, int* codepage);

/// @brief 마운트에서 파일이 있나 조사
/// @param self 마운트
/// @param path 파일 경로
/// @return 파일 속성
/// @see QnFileAttr
QSAPI QnFileAttr qn_mount_exist(QnMount* self, const char* path);

/// @brief 마운트에서 파일 제거
/// @param self 마운트
/// @param path 파일 경로
/// @return 성공했으면 참을 반환
QSAPI bool qn_mount_remove(QnMount* self, const char* path);

/// @brief 마운트에서 디렉토리 변경
/// @param self 마운트
/// @param directory 변경할 디렉토리 (널이면 기본 디렉토리로 변경)
/// @return 성공했으면 참을 반환
QSAPI bool qn_mount_chdir(QnMount* self, const char* directory);

/// @brief 마운트에서 디렉토리 만들기
/// @param self 마운트
/// @param directory 디렉토리 이름
/// @return 성공했으면 참을 반환
QSAPI bool qn_mount_mkdir(QnMount* self, const char* directory);

/// @brief 마운트에서 파일 목록 얻기
/// @param self 마운트
/// @return 파일 목록 (널이면 실패)
QSAPI QnDir* qn_mount_list(QnMount* self);

/// @brief 마운트 이름 얻기
/// @param self 마운트
/// @return 이름
INLINE const char* qn_mount_get_name(QnMount* self) { return qn_cast_type(self, QnMount)->name; }

/// @brief 마운트 경로 얻기
/// @param self 마운트
/// @return 경로
INLINE const char* qn_mount_get_path(QnMount* self) { return qn_cast_type(self, QnMount)->path.DATA; }

// HFS

#ifndef _QN_MOBILE_
/// @brief HFS에 설명을 넣는다 (63자까지)
/// @param mount 마운트
/// @param desc 설명
/// @return 실패 했다면 마운트가 HFS가 아니거나 쓰기 모드가 아님
QSAPI bool qn_hfs_set_desc(QnMount* mount, const char* desc);

/// @brief HFS에 버퍼를 파일로 저장한다
/// @param mount 마운트
/// @param filename 파일 이름
/// @param data 데이터
/// @param size 크기 (최대 크기는 1.99GB로 2040MB)
/// @param cmpr 압축 여부
/// @param type 파일 타입
/// @return 성공했으면 참을 반환
QSAPI bool qn_hfs_store_data(QnMount* mount, const char* filename, const void* data, uint size, bool cmpr, QnFileType type);

/// @brief HFS에 스트림을 파일로 저장한다
/// @param mount 마운트
/// @param filename 파일 이름
/// @param stream 스트림
/// @param cmpr 압축 여부
/// @param type 파일 타입
/// @return 성공했으면 참을 반환
QSAPI bool qn_hfs_store_stream(QnMount* mount, const char* filename, QnStream* stream, bool cmpr, QnFileType type);

/// @brief HFS에 파일을 파일로 저장한다
/// @param mount 마운트
/// @param filename 파일 이름 (널이면 소스 파일 이름을 사용)
/// @param srcfile 소스 파일 이름
/// @param cmpr 압축 여부
/// @param type 파일 타입
/// @return 성공했으면 참을 반환
QSAPI bool qn_hfs_store_file(QnMount* mount, const char* filename, const char* srcfile, bool cmpr, QnFileType type);
#endif


//////////////////////////////////////////////////////////////////////////
// thread

/// @brief 스핀락
typedef int volatile	QnSpinLock;
/// @brief TLS
typedef int				QnTls;
/// @brief 뮤텍스
typedef struct QNMUTEX	QnMutex;
/// @brief 컨디션
typedef struct QNCOND	QnCond;
/// @brief 세마포어
typedef struct QNSEM	QnSem;

// spin lock

/// @brief 스핀락을 걸어본다
/// @param lock 스핀락
/// @return 걸렸으면 참
QSAPI bool qn_spin_try(QnSpinLock* lock);

/// @brief 스핀락을 건다
/// @param lock 스핀락
/// @returns 스핀락이 들어갈 때까지 걸린 횟수
/// @note 걸릴 때까지 대기한다
QSAPI uint qn_spin_enter(QnSpinLock* lock);

/// @brief 스핀락을 푼다
/// @param lock 스핀락
QSAPI void qn_spin_leave(QnSpinLock* lock);

#ifndef QS_NO_SPINLOCK
/// @brief 스핀락 들어간다
#define QN_LOCK(sp)		qn_spin_enter(&sp)
/// @brief 스핀락 나온다
#define QN_UNLOCK(sp)	qn_spin_leave(&sp)
#else
#define QN_LOCK(sp)
#define QN_UNLOCK(sp)
#endif

// tls

/// @brief TLS를 만든다
/// @param callback TLS가 제거될 때 필요한 콜백 (NULL 허용)
/// @return TLS 값
QSAPI QnTls qn_tls(paramfunc_t callback);

/// @brief TLS에 값을 쓴다
/// @param tls 대상 TLS
/// @param data 넣을 값
QSAPI void qn_tlsset(QnTls tls, void* data);

/// @brief TLS에서 값을 읽는다
/// @param tls 대상 TLS
/// @return 읽은 값
QSAPI void* qn_tlsget(QnTls tls);

// mutex

/// @brief 뮤텍스를 만든다. 이 뮤텍스는 RECURSIVE 타입이다
/// @return 만들어진 뮤텍스
QSAPI QnMutex* qn_new_mutex(void);

/// @brief 뮤텍스를 삭제한다
/// @param self 뮤텍스
QSAPI void qn_delete_mutex(QnMutex* self);

/// @brief 뮤텍스를 잠궈 본다
/// @param self 뮤텍스
/// @return 잠금 상태
/// @retval true 잠겼다
/// @retval false 잠기지 않았다
QSAPI bool qn_mutex_try(QnMutex* self);

/// @brief 뮤텍스를 잠근다. 다른 스레드에서 잠겨있으면 잠글 수 있을 때까지 대기한다
/// @param self 뮤텍스
QSAPI void qn_mutex_enter(QnMutex* self);

/// @brief 뮤텍스 잠김을 푼다
/// @param self 뮤텍스
QSAPI void qn_mutex_leave(QnMutex* self);

// condition

/// @brief 조건을 만든다
/// @return 만들어진 조건
QSAPI QnCond* qn_new_cond(void);

/// @brief 조건을 삭제한다
/// @param self 조건
QSAPI void qn_delete_cond(QnCond* self);

/// @brief 조건에 신호를 1번만 보낸다
/// @param self 조건
/// @note 가장 최근 수행한 스레드가 신호를 받는다
QSAPI void qn_cond_signal(QnCond* self);

/// @brief 모든 스레드에 신호를 보낸다.
/// @param self 조건
QSAPI void qn_cond_broadcast(QnCond* self);

/// @brief 각자 스레드가 조건에 해당하는지 기다린다
/// @param self 조건
/// @param lock 조건을 잠글 뮤텍스
/// @param milliseconds 기다릴 시간 (밀리초)
/// @return 조건 대기 및 시간 제한을 반환
/// @retval true 조건이 들어왔다
/// @retval false 조건이 들어오지 않았고. 시간 제한이 됐다
QSAPI bool qn_cond_wait_for(QnCond* self, QnMutex* lock, uint milliseconds);

/// @brief 각자 스레드가 조건을 기다린다
/// @param self 조건
/// @param lock 조건을 잠글 뮤텍스
QSAPI void qn_cond_wait(QnCond* self, QnMutex* lock);

// semaphore

/// @brief 세마포어를 만든다
/// @param initial 초기값
/// @return 만들어진 세마포어
QSAPI QnSem* qn_new_sem(int initial);

/// @brief 세마포어를 삭제한다
/// @param self 세마포어
QSAPI void qn_delete_sem(QnSem* self);

/// @brief 세마포어를 지정한 시간 만큼 대기해 본다
/// @param self 세마포어
/// @param milliseconds 대기할 시간 (밀리초)
/// @return 대기 결과
/// @retval true 신호가 떴다
/// @retval false 신호가 오지 않았고 시간 제한을 넘었다
QSAPI bool qn_sem_wait_for(QnSem* self, uint milliseconds);

/// @brief 세마포어를 대기한다
/// @param self 세마포어
/// @return 대기 결과. 다만 별로 신경 쓰지 않아도 좋다
QSAPI bool qn_sem_wait(QnSem* self);

/// @brief 세마포어가 기다릴 수 있는지 조시한다
/// @param self 세마포어
/// @return 기다릴 수 있는지 여부
/// @retval true 기다릴 수 있다
/// @retval false 기다릴 수 없다
QSAPI bool qn_sem_try(QnSem* self);

/// @brief 세마포어 갯수를 반환한다
/// @param self 세마포어
/// @return 세마포어 갯수
QSAPI int qn_sem_count(QnSem* self);

/// @brief 세마포어를 알린다
/// @param self 세마포어
/// @return 성공하면 참
QSAPI bool qn_sem_post(QnSem* self);

// thread

/// @brief 스레드 콜백
typedef void* (*QnThreadCallback)(void*);

/// @brief 스레드
typedef struct QNTHREAD
{
	char*				name;
	bool32				canwait;
	bool32				managed;

	int					busy;
	uint				stack_size;

	QnThreadCallback	cb_func;
	void*				cb_data;
	void*				cb_ret;
} QnThread;

/// @brief 현재 스레드를 얻는다
/// @return 현재 스레드 포인터
QSAPI QnThread* qn_thread_self(void);

/// @brief 스레드를 만든다
/// @param name 스레드 이름
/// @param func 스레드 콜백 함수
/// @param data 콜백 데이터
/// @param stack_size 스택 크기 (보통 0을 넣어도 무방)
/// @param busy 우선순위
/// @return 만들어진 스레드
/// *
/// * | busy | 우선순위  |
/// * | ----:|:-------- |
/// * | -2   | 아주 낮음 |
/// * | -1   | 낮음     |
/// * | 0    | 보통     |
/// * | 1    | 높음     |
/// * | 2    | 아주 높음 |
QSAPI QnThread* qn_new_thread(const char* name, QnThreadCallback func, void* data, uint stack_size, int busy);

/// @brief 스레드를 제거한다. 실행 중이면 끝날 때 까지 대기한다
/// @param self 스레드
QSAPI void qn_delete_thread(QnThread* self);

/// @brief 스레드를 바로 실행한다
/// @param name 스레드 이름
/// @param func 스레드 콜백 함수
/// @param data 콜백 데이터
/// @param stack_size 스택 크기 (보통 0을 넣어도 된다)
/// @param busy 우선 순위
/// @return 성공적으로 실행했으면 참
/// @see qn_thread_new qn_thread_start
QSAPI bool qn_thread_once(const char* name, QnThreadCallback func, void* data, uint stack_size, int busy);

/// @brief 스레드를 실행한다
/// @param self 스레드
/// @return 성공적으로 실행했으면 참
/// @see qn_thread_new
QSAPI bool qn_thread_start(QnThread* self);

/// @brief 스레드가 끝날 때까지 대기한다
/// @param self 스레드
/// @return 스레드가 보내온 반환값
QSAPI void* qn_thread_wait(QnThread* self);

/// @brief 스레드를 끝낸다. 이 함수는 스레드 안에서 사용해야한다
/// @param ret 스레드 반환 값
/// @warning 메인 스레드에서 사용하면 안된다
QSAPI void qn_thread_exit(void* ret);

/// @brief 스레드 우선 순위를 얻는다
/// @param self 스레드
/// @return 우선 순위 (-2 ~ 2)
QSAPI int qn_thread_get_busy(const QnThread* self);

/// @brief 스레드 우선 순위를 설정한다
/// @param self 스레드
/// @param busy 우선 순위 (-2 ~ 2)
/// @return 성공하면 참
QSAPI bool qn_thread_set_busy(QnThread* self, int busy);

// module

/// @brief 모듈
typedef struct QNMODULE QnModule;

/// @brief 프로그램을 모듈로 얻는다
/// @return 프로그람 자체 모듈
QSAPI QnModule* qn_mod_self(void);

/// @brief 모듈을 연다
/// @param filename 파일 이름 (대소문자 구별한다!)
/// @param flags 플래그. 지금은 사용하지 않는다 0으로
/// @return 만들어진 모듈
/// @note 파일 이름은 대소문자를 구별하며 오류를 막기위해 윈도우에서는 대문자로 하는게 좋아요
QSAPI QnModule* qn_open_mod(const char* filename, int flags);

/// @brief 함수를 읽는다
/// @param self 모듈
/// @param name 읽을 함수 이름
/// @return 함수 포인터
QSAPI void* qn_mod_func(QnModule* self, const char* name);


//////////////////////////////////////////////////////////////////////////
// type check
#define QN_ASSERT_SIZE(t,s)	static_assert(sizeof(t) == s, #t " type size must be " #s "")
QN_ASSERT_SIZE(bool32, 4);
QN_ASSERT_SIZE(byte, 1);
QN_ASSERT_SIZE(ushort, 2);
QN_ASSERT_SIZE(uint, 4);
QN_ASSERT_SIZE(ullong, 8);
QN_ASSERT_SIZE(halfint, 2);
QN_ASSERT_SIZE(vshort, 2);
QN_ASSERT_SIZE(vint, 4);
QN_ASSERT_SIZE(vllong, 8);
QN_ASSERT_SIZE(any_t, 8);
#ifdef _QN_64_
QN_ASSERT_SIZE(nuint, 8);
QN_ASSERT_SIZE(funcparam_t, 16);
#else
QN_ASSERT_SIZE(nuint, 4);
QN_ASSERT_SIZE(funcparam_t, 8);
#endif
#undef QN_ASSERT_SIZE
