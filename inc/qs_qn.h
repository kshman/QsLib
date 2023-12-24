﻿//
// QsLib [QN Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
//

#pragma once

//////////////////////////////////////////////////////////////////////////
// includes
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <stdnoreturn.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <wchar.h>
#ifdef _MSC_VER
#include <intrin.h>
#include <crtdbg.h>
#endif
#ifdef __unix__
#include <sys/time.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


//////////////////////////////////////////////////////////////////////////
// compiler configuration

// check
#if !defined _MSC_VER && !defined __GNUC__
#error unknown compiler! (support: MSVC, CLANG, GCC)
#endif

// platform
#ifdef _WIN32
#define _QN_WINDOWS_		1
#endif

#ifdef __unix__
#define _QN_UNIX_			1
#endif

#if defined __FreeBSD__ || defined __OpenBSD__
#define _QN_BSD_			1
#if defined __FreeBSD__
#define _QN_FREEBSD_		1
#endif
#if defined __OpenBSD__
#define _QN_OPENBSD_		1
#endif
#endif

#ifdef __linux__
#define _QN_LINUX_			1
#endif

#ifdef __android__
#define _QN_ANDROID_		1
#define _QN_MOBILE_			1
#endif

#if defined __XBOXONE__ || defined __XBOXSERIES__
#define _QN_XBOX_			1
#endif

#if defined _WIN64 || defined _M_X64 || defined __LP64__ || defined __amd64__ || defined __x86_64__ || defined __aarch64__
#define _QN_64_				1
#endif

#ifdef _CHAR_UNSIGNED
#error "Compiler option: char type must be signed"
#endif

// compiler specific
#ifdef _MSC_VER
#define QN_INLINE			_Pragma("warning(suppress:4514 4710)") __inline
#define QN_FORCE_LINE		__forceinline
#define QN_FNAME			__FUNCTION__
#define QN_FALL_THROUGH
#define QN_STRUCT_ALIGN(x)	__declspec(align(x))
#define QN_STMT_BEGIN		_Pragma("warning(suppress:4127)") do
#define QN_STMT_END			while(0)
#elif defined __GNUC__
#ifndef __cplusplus
#define QN_INLINE			static inline
#define QN_FORCE_LINE		static inline __attribute__ ((always_inline))
#else
#define QN_INLINE			inline
#define QN_FORCE_LINE		inline __attribute__ ((always_inline))
#endif
#define QN_FNAME			__FUNCTION__/*__PRETTY_FUNCTION__*/
#define QN_FALL_THROUGH		__attribute__((fallthrough))
#define QN_STRUCT_ALIGN(x)	__attribute__((aligned(x)))
#define QN_STMT_BEGIN		do
#define QN_STMT_END			while(0)
#endif

#ifdef __cplusplus
#define QN_EXTC				extern "C"
#define QN_EXTC_BEGIN		extern "C" {
#define QN_EXTC_END			}
#else
#define QN_EXTC
#define QN_EXTC_BEGIN
#define QN_EXTC_END
#endif

QN_EXTC_BEGIN

//////////////////////////////////////////////////////////////////////////
// macro

// compound & statement
#ifdef _MSC_VER
#else
#endif

// function support
#define _QN_PRAGMA(x)		_Pragma(#x)
#define _QN_STRING(x)		#x
#define _QN_UNICODE(x)		L##x
#define _QN_CONCAT_2(x,y)	x##y
#define _QN_CONCAT_3(x,y,z)	x##y##z
#define _QN_GET_MAC_2(_1,_2,N,...)		N
#define _QN_GET_MAC_3(_1,_2,_3,N,...)	N

// function
#define QN_TODO(todo)		_QN_PRAGMA(message("TODO: " #todo))
#define QN_DUMMY(dummy)		(void)dummy;
#define QN_STRING(x)		_QN_STRING(x)					/// @brief 문자열로 정의
#define QN_UNICODE(x)		_QN_UNICODE(x)					/// @brief 유니코드로 정의
#define QN_CONCAT_2(x,y)	_QN_CONCAT_2(x, y)				/// @brief 두개 문구 합침
#define QN_CONCAT_3(x,y,z)	_QN_CONCAT_3(x, y, z)			/// @brief 세개 문구 합침
#define QN_CONCAT(...)		_QN_GET_MAC_3(__VA_ARGS__, QN_CONCAT_3, QN_CONCAT_2)(__VA_ARGS__)		/// @brief 문구 합침 (세개까지)

#define QN_COUNTOF(arr)		(sizeof(arr)/sizeof((arr)[0]))	/// @brief 배열 갯수 얻기
#define QN_OFFSETOF(t,m)	((size_t)((char*)&((t*)0)->m))	/// @brief 구조체에서 위치 얻기
#define QN_MEMBEROF(t,p,o)	(*(t*)((void*)((char*)(p)+(size_t)(o))))								/// @brief 구조체에서 위치 얻기(포인터)

#define QN_MAX(a,b)			(((a)>(b))?(a):(b))				/// @brief 최대값 얻기
#define QN_MIN(a,b)			(((a)<(b))?(a):(b))				/// @brief 최소값 얻기
#define QN_ABS(v)			(((v)<0)?-(v):(v))				/// @brief 절대값 얻기
#define QN_CLAMP(v,l,h)		((v)<(l)?(l):(v)>(h)?(h):(v))	/// @brief 숫자를 최소/최대값으로 자르기

#define QN_BIT(b)			(1<<(b))						/// @brief 비트 만들기
#define QN_TBIT(v,bit)		(((v)&(1<<(bit)))!=0)			/// @brief 비트가 있나 비교
#define QN_TMASK(v,m)		(((v)&(m))!=0)					/// @brief 마스크가 있나 비교
#define QN_SBIT(pv,b,set)	((set)?((*(pv))|=(1<<(b))):((*(pv))&=~(1<<(b))))						/// @brief 비트 설정
#define QN_SMASK(pv,m,set)	((set)?((*(pv))|=(m)):((*(pv))&=~(m)))									/// @brief 마스크 설정

// constant
#define QN_USEC_PER_SEC		1000000							/// @brief 초당 마이크로초 
#define QN_NSEC_PER_SEC		1000000000						/// @brief 초당 나노초 

#define QN_MIN_HASH			11								/// @brief 최소 해시 갯수
#define QN_MAX_HASH			13845163						/// @brief 최대 해시 갯수
#define QN_MAX_RAND			0x7FFF							/// @brief 최대 난수
#define QN_MAX_PATH			1024							/// @brief 경로의 최대 길이

// path separator
#ifdef _QN_WINDOWS_
#define QN_PATH_SEP			'\\'							/// @brief 경로 분리 문자
#else
#define QN_PATH_SEP			'/'
#endif


//////////////////////////////////////////////////////////////////////////
// types

typedef void (*func_t)(void);								/// @brief 함수 핸들러
typedef void (*paramfunc_t)(void*);							/// @brief 파라미터 있는 함수 핸들러

// aliases
typedef int8_t				sbyte;							/// @brief 8비트 부호 있는 정수
typedef int64_t				llong;							/// @brief 64비트 부호 있는 정수
typedef uint8_t				byte;							/// @brief 8비트 부호 없는 정수
typedef uint16_t			ushort;							/// @brief 16비트 부호 없는 정수
typedef uint32_t			uint;							/// @brief 32비트 부호 없는 정수
typedef uint64_t			ullong;							/// @brief 64비트 부호 없는 정수

typedef intptr_t			nint;							/// @brief 플랫폼에 따른 부호 있는 포인터
typedef uintptr_t			nuint;							/// @brief 플랫폼에 따른 부호 없는 포인터

typedef wchar_t				wchar;
#ifdef _QN_WINDOWS_
typedef uint32_t			uchar4;							/// @brief 4바이트(32비트) 유니코드
typedef wchar_t				uchar2;							/// @brief 2바이트(16비트) 유니코드
#else
typedef wint_t				uchar4;
typedef uint16_t			uchar2;
#endif

typedef uint16_t			halfint;						/// @brief 16비트 부호 없는 정수(half int)
typedef int16_t				bool16;							/// @brief 16비트 참거짓
typedef int32_t				bool32;							/// @brief 32비트 참거짓

/// @brief 하위 변형 있는 16비트 정수
typedef union vint16_t
{
	struct vint16_t_byte
	{
		byte			l, h;								/// @brief 바이트 (h=msb)
	}				b;										/// @brief 바이트 집합
	ushort			w;										/// @brief 워드
} vint16_t, vshort;

/// @brief 하위 변형 있는 32비트 정수
typedef union vint32_t
{
	struct vint32_t_byte
	{
		byte			a, b, c, d;							/// @brief 바이트 (d=msb)
	}				b;										/// @brief 바이트 집합
	struct vint32_t_word
	{
		ushort			l, h;								/// @brief 워드
	}				w;										/// @brief 워드 집합
	uint			dw;										/// @brief 더블 워드
} vint32_t, vint;

/// @brief 하위 변형 있는 64비트 정수
typedef union vint64_t
{
	struct vint64_t_byte
	{
		byte			la, lb, lc, ld;						/// @brief 하위 바이트
		byte			ha, hb, hc, hd;						/// @brief 상위 바이트 (hd=msb)
	}				b;										/// @brief 바이트 집합
	struct vint64_t_word
	{
		ushort			a, b, c, d;							/// @brief 워드
	}				w;										/// @brief 워드 집합
	struct vint64_t_double_word
	{
		ushort			l, h;								/// @brief 더블 워드
	}				dw;										/// @brief 더블 워드 집합
	ullong			q;										/// @brief 쿼드 워드
} vint64_t, vllong;

/// @brief 아무값이나 넣기 위한 타입 (long double이 있으면 16, 없으면 8바이트)
typedef union any_t
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
} any_t;

/// @brief 파라미터 함수 핸들러와 파라미터 값
typedef struct funcparam_t
{
	paramfunc_t		func;
	void*			data;
} funcparam_t;

// conditions
#define qn_ret_if_fail(x)	QN_STMT_BEGIN{ if (!(x)) return; }QN_STMT_END							/// @brief 값이 거짓이면 리턴
#define qn_ret_if_ok(x)		QN_STMT_BEGIN{ if ((x)) return; }QN_STMT_END							/// @brief 값이 참이면 리턴
#define qn_val_if_fail(x,r)	QN_STMT_BEGIN{ if (!(x)) return (r); }QN_STMT_END						/// @brief 값이 거짓이변 반환
#define qn_val_if_ok(x,r)	QN_STMT_BEGIN{ if ((x)) return (r); }QN_STMT_END						/// @brief 값이 참이면 반환

#ifdef _DEBUG
#define qn_assert(expr,m)	QN_STMT_BEGIN{if (!(expr)) qn_debug_assert(#expr, m, __FILE__, __LINE__);}QN_STMT_END		/// @brief 표현이 거짓이면 메시지 출력
#define qn_verify(expr)		QN_STMT_BEGIN{if (!(expr)) qn_debug_assert(#expr, NULL, __FILE__, __LINE__);}QN_STMT_END	/// @brief 표현이 거짓이면 표현 출력
#else
#define qn_assert(expr,m)
#define qn_verify(expr)
#endif


//////////////////////////////////////////////////////////////////////////
// runtime

/// @brief 런타임 초기화
/// @param[out] v 버전 값. NULL 가능
QSAPI void qn_runtime(int /*NULLABLE*/v[2]);

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

/// @brief 프로퍼티를 설정한다
/// @param name 프로퍼티 이름
/// @param value 프로퍼티 값
QSAPI void qn_set_prop(const char* restrict name, const char* restrict value);

/// @brief 프로퍼티를 얻는다
/// @param name 프로퍼티 이름
/// @return 문자열의 프로퍼티 값
QSAPI const char* qn_get_prop(const char* name);

/// @brief 에러 메시지를 얻는다
/// @return 에러 메시지 문자열. 없다면 NULL
QSAPI const char* qn_get_error(void);

/// @brief 에러 메시지를 설정한다
/// @param[in] mesg 에러 메시지
QSAPI void qn_set_error(const char* mesg);

/// @brief 시스템 에러 메시지를 설정한다
/// @param errcode 에러 코드 (0으로 설정하면 현재 에러 코드를 가져옴)
/// @return 에러가 있었다면 참, 아니면 거짓
QSAPI bool qn_set_syserror(int errcode);

/// @brief 디버그용 검사 출력
/// @param[in] expr 검사한 표현
/// @param[in] mesg 오류 내용
/// @param[in] filename 파일 이름이나 함수 이름
/// @param[in] line 줄 번호
/// @return 출력한 문자열 길이
QSAPI int qn_debug_assert(const char* restrict expr, const char* restrict mesg, const char* restrict filename, int line);

/// @brief HALT 메시지
/// @param[in] head 머릿글
/// @param[in] mesg 메시지
QSAPI noreturn void qn_debug_halt(const char* restrict head, const char* restrict mesg);

/// @brief 디버그용 문자열 출력
/// @param[in] breakpoint 참이면 디버거 연결시 중단점 표시
/// @param[in] head 머릿글
/// @param[in] mesg 메시지
/// @return 출력한 문자열 길이
QSAPI int qn_debug_outputs(bool breakpoint, const char* restrict head, const char* restrict mesg);

/// @brief 디버그용 문자열 포맷 출력
/// @param[in] breakpoint 참이면 디버거 연결시 중단점 표시
/// @param[in] head 머릿글
/// @param[in] fmt 문자열 포맷
/// @return 출력한 문자열 길이
QSAPI int qn_debug_outputf(bool breakpoint, const char* restrict head, const char* restrict fmt, ...);

/// @brief 에러 메시지를 출력한다
/// @param breakpoint  참이면 디버거 연결시 중단점 표시
/// @param head 머릿글
/// @return 출력한 문자열 길이
QSAPI int qn_debug_output_error(bool breakpoint, const char* head);

/// @brief 문자열을 출력한다 (디버그 메시지 포함)
/// @param[in] mesg 출력할 내용
/// @return 출력한 문자열 길이
QSAPI int qn_outputs(const char* mesg);

/// @brief 문자열을 포맷하여 출력한다 (디버그 메시지 포함)
/// @param[in] fmt 출력할 포맷
/// @return 출력한 문자열 길이
QSAPI int qn_outputf(const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// memory
#define qn_alloc(c,t)		(t*)qn_memalc(NULL,(size_t)(c)*sizeof(t),false,QN_FNAME,__LINE__)		/// @brief 메모리 할당
#define qn_alloc_1(t)		(t*)qn_memalc(NULL,sizeof(t),false,QN_FNAME,__LINE__)					/// @brief 메모리를 1개만 할당
#define qn_alloc_zero(c,t)	(t*)qn_memalc(NULL,(size_t)(c)*sizeof(t),true,QN_FNAME,__LINE__)		/// @brief 메모리를 할당하고 메모리를 0으로
#define qn_alloc_zero_1(t)	(t*)qn_memalc(NULL,sizeof(t),true,QN_FNAME,__LINE__)					/// @brief 메모리를 1개만 할당하고 0으로
#define qn_realloc(p,c,t)	(t*)qn_memalc((void*)(p),(size_t)(c)*sizeof(t),false,QN_FNAME,__LINE__)	/// @brief 메모리를 다시 할당

#define qn_free(p)			qn_memfre((void*)(p))													/// @brief 메모리 해제
#define qn_free_ptr(pp)		QN_STMT_BEGIN{ qn_memfre((void*)*(pp)); *(pp)=NULL; }QN_STMT_END		/// @brief 메모리를 해제하고 포인터를 NULL로 만든다

#define qn_zero(p,c,t)		memset((p), 0, sizeof(t)*(c))											/// @brief 메모리를 0으로 채운다
#define qn_zero_1(p)		memset((p), 0, sizeof(*(p)))											/// @brief 메모리를 0으로 채운다 (주로 구조체)

/// @brief 메모리 할당 구조체
typedef struct QnAllocTable QnAllocTable;
struct QnAllocTable
{
	void*(*_alloc)(void* ptr, size_t size, bool zero, const char* desc, size_t line);
	void(*_free)(void* ptr);
};

/// @brief 메모리를 복호화한다
/// @param[out] dest 출력 대상
/// @param[in] src 복호할 메모리
/// @param[in] size 크기
/// @return dest 그대로
QSAPI void* qn_memenc(void* restrict dest, const void* restrict src, size_t size);

/// @brief 복호화한 메모리를 되돌린다
/// @param[out] dest 출력 대상
/// @param[in] src 복호화 했던 메모리
/// @param[in] size 크기
/// @return dest 그대로
QSAPI void* qn_memdec(void* restrict dest, const void* restrict src, size_t size);

/// @brief 메모리를 압축한다. qn_free()로 해제해야 한다
/// @param[in] src 메모리 원본
/// @param[in] srcsize 원본 크기
/// @param[out] destsize 압축한 크기
/// @return 압축된 메모리
QSAPI void* qn_memzcpr(const void* src, size_t srcsize, /*NULLABLE*/size_t* destsize);

/// @brief 메모리를 해제한다. qn_free()로 해제해야 한다
/// @param[in] src 압축된 메모리
/// @param[in] srcsize 압축된 메모리의 크기
/// @param[in] bufsize 버퍼 크기
/// @param[out] destsize 해제된 크기
/// @return 해제된 메모리
QSAPI void* qn_memzucp(const void* src, size_t srcsize, size_t bufsize, /*NULLABLE*/size_t* destsize);

/// @brief 입력한 크기를 메모리 정렬 기준에 맞춘 크기로 돌려준다
/// @param[in] size 메모리 크기
/// @return 정렬된 크기
QSAPI size_t qn_memagn(size_t size);

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
/// @return outbuf 그대로
QSAPI char* qn_memdmp(const void* restrict ptr, size_t size, char* restrict outbuf, size_t buflen);

/// @brief 메모리 테이블을 등록한다
/// @param[in] table 테이블
/// @return 테이블 값 중에 빈게 있으면 거짓
QSAPI bool qn_memtbl(const QnAllocTable* table);

/// @brief 메모리를 할당/재할당/해제한다
/// @param[in] ptr 재할당할 메모리 (이 값이 NULL이면 새로 할당)
/// @param[in] size 재할당할 메모리 크기 (이 값이 0이면 ptr을 해제)
/// @param[in] zero 할당할 때 메모리를 0으로 초기화 한다 (재할당일 경우 안함)
/// @param[in] desc 설명문
/// @param[in] line 줄 번호
/// @return 할당한 새로운 메모리 (메모리 주소가 안바뀔 수도 있음)
QSAPI void* qn_memalc(void* ptr, size_t size, bool zero, const char* desc, size_t line);

/// @brief 메모리를 해제한다
/// @param[in] ptr 해제할 메모리 (이 값이 NULL이면 아무것도 하지 않는다)
QSAPI void qn_memfre(void* ptr);

/// @brief 내부 메모리 관리자의 메모리의 크기를 얻는다
/// @return 관리하는 메모리의 크기
QSAPI size_t qn_mpfsize(void);

/// @brief 내부 메모리 관리자의 메모리의 갯수를 얻는다
/// @return 관리하는 메모리의 총 할당 갯수
QSAPI size_t qn_mpfcnt(void);

/// @brief 내부 메모리 관리자로 메모리를 할당/재할당/해제한다
/// @param[in] ptr 재할당할 메모리 (이 값이 NULL이면 새로 할당)
/// @param[in] size 재할당할 메모리 크기 (이 값이 0이면 ptr을 해제)
/// @param[in] zero 할당할 때 메모리를 0으로 초기화 한다 (재할당일 경우 안함)
/// @param[in] desc 설명문
/// @param[in] line 줄 번호
/// @return 할당한 새로운 메모리 (메모리 주소가 안바뀔 수도 있음)
QSAPI void* qn_mpfalloc(void* ptr, size_t size, bool zero, const char* desc, size_t line);

/// @brief 내부 메모리 관리자의 메모리를 해제한다
/// @param[in] ptr 해제할 메모리 (이 값이 NULL이면 아무것도 하지 않는다)
QSAPI void qn_mpffree(void* ptr);

/// @brief 내부 메모리 관리자로 할당하여 메모리를 복사한다
/// @param p 복사할 메모리
/// @param size_or_zero_if_string 크기를 지정하면 크기만큼 복사. 0으로 지정하면 문자열 복사로 취급한다
/// @param[in] desc 설명문
/// @param[in] line 줄 번호
/// @return 복사한 관리 메모리
void* qn_mpfdup(const void* p, size_t size_or_zero_if_string, const char* desc, size_t line);

/// @brief 디버그용 메모리 정보를 출력한다
QSAPI void qn_debug_mpfprint(void);


//////////////////////////////////////////////////////////////////////////
// hash & sort

/// @brief 포인터 해시. 일반적인 size_t 해시를 의미함
/// @param[in]	p	입력 변수
/// @return	해시 값
///
QSAPI size_t qn_hashptr(const void* p);
/// @brief 시간 해시. 현재 시각을 이용하여 해시값을 만든다
/// @return	해시 값
///
QSAPI size_t qn_hashnow(void);
/// @brief 콜백 해시. 함수 콜백을 해시값으로 만들어 준다
/// @param[in]	prime8	8비트 소수 값
/// @param[in]	func  	콜백 함수
/// @param[in]	data  	콜백 데이터
/// @return	해시 값
///
QSAPI size_t qn_hashfn(int prime8, func_t func, const void* data);
/// @brief 데이터를 crc로 해시한다
/// @param[in] data 데이터
/// @param[in] size 데이터 크기
/// @return 해시 값
///
QSAPI size_t qn_hashcrc(const byte* data, size_t size);
/// @brief 가까운 소수 얻기. 처리할 수 있는 최소 소수는 QN_MAX_HASH(11), 최대 소수는 QN_MAX_HASH(13845163)
/// @param[in] value 입력 값
/// @return 소수 값
/// @see QN_MIN_HASH, QN_MAX_HASH
///
QSAPI uint32_t qn_primenear(uint32_t value);
/// @brief 제곱 소수 얻기. 근거리에 해당하는 제곱 소수를 계산해준다
/// @param[in] value 입력 값
/// @param[in] min 최소 값
/// @param[out] shift 널이 아니면 쉬프트 크기
/// @return 소수 값
///
QSAPI uint32_t qn_primeshift(uint32_t value, uint32_t min, uint32_t* shift);

/// @brief 퀵정렬
/// @param[in,out] ptr 정렬할 데이터의 포인터
/// @param[in] count 데이터의 갯수
/// @param[in] stride 데이터의 폭
/// @param[in] compfunc 비교 연산 콜백 함수
///
QSAPI void qn_qsort(void* ptr, size_t count, size_t stride, int(*compfunc)(const void*, const void*));
/// @brief 콘텍스트 입력 받는 퀵정렬
/// @param[in,out] ptr 정렬할 데이터의 포인터
/// @param[in] count 데이터의 갯수
/// @param[in] stride 데이터의 폭
/// @param[in] compfunc 비교 연산 콜백 함수
/// @param[in] context 콜백 함수용 콘텍스트
///
QSAPI void qn_qsortc(void* ptr, size_t count, size_t stride, int(*compfunc)(void*, const void*, const void*), void* context);


//////////////////////////////////////////////////////////////////////////
// string

/// @brief 가변 포맷 문자열
/// @param[out] out 출력 문자열 버퍼. 문자열 길이만 얻으려면 NULL로 설정
/// @param[in] len 출력 문자열 버퍼 크기
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열의 길이
///
QSAPI int qn_vsnprintf(char* restrict out, size_t len, const char* restrict fmt, va_list va);
/// @brief 가변 포맷 문자열을 메모리 할당으로 만든다
/// @param[out] out 출력 문자열의 버퍼 포인터. 이 값이 NULL이면 곤린하다
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열의 길이
/// @note out 인수로 만든 문자열은 qn_free 함수로 해제해야 한다
/// @see qn_free
///
QSAPI int qn_vasprintf(char** out, const char* fmt, va_list va);
/// @brief 가변 포맷 문자열을 메모리 할당으로 만들어 반환한다
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열
/// @note 반환 값은 qn_free 함수로 해제해야 한다
/// @see qn_free
///
QSAPI char* qn_vapsprintf(const char* fmt, va_list va);
/// @brief 문자열을 포맷한다
/// @param[out] out 출력 문자열의 버퍼 포인터. 문자열 길이만 얻으려면 NULL로 설정
/// @param[in] len 출력 문자열의 버퍼 포인터의 크기
/// @param[in] fmt 포맷 문자열
/// @return 결과 문자열의 길이
///
QSAPI int qn_snprintf(char* restrict out, size_t len, const char* restrict fmt, ...);
/// @brief 문자열을 포맷하고 메모리 할당으로 만든다
/// @param[out] out 출력 문자열의 버퍼 포인터. 이 값이 NULL이면 곤린하다
/// @param[in] fmt 포맷 문자열
/// @return 결과 문자열의 길이
/// @note out 인수로 만든 문자열은 qn_free 함수로 해제해야 한다
/// @see qn_free
///
QSAPI int qn_asprintf(char** out, const char* fmt, ...);
/// @brief 문자열을 포맷하고 메모리를 반환한다
/// @param[in] fmt 포맷 문자열
/// @return 결과 문자열
/// @note 반환 값은 qn_free 함수로 해제해야 한다
/// @see qn_free
///
QSAPI char* qn_apsprintf(const char* fmt, ...);
/// @brief 글자로 채우기
/// @param[in,out] dest 채울 대상 버퍼
/// @param[in] pos 채울 시작 위치
/// @param[in] end 채울 끝 위치
/// @param[in] ch 채울 문자
/// @return 마지막으로 채운 곳 + 1의 위치
///
QSAPI size_t qn_strfll(char* dest, size_t pos, size_t end, int ch);
/// @brief 문자열 해시
/// @param[in] p 해시할 문자열
/// @return 해시 값
///
QSAPI size_t qn_strhash(const char* p);
/// @brief 문자열 해시 (대소문자 구별 안함
/// @param[in] p 해시할 문자열
/// @return 해시 값
///
QSAPI size_t qn_strihash(const char* p);
/// @brief 문자열에서 문자열을 찾는다
/// @param[in] p 대상 문자열
/// @param[in] c 찾을 문자열
/// @return 찾을 경우 대상 문자열의 위치 포인터
/// @retval NULL 못 찾았다
///
QSAPI const char* qn_strbrk(const char* p, const char* c);
/// @brief 문자열을 특정 위치에서 얻는다
/// @param[in,out] dest 얻은 문자열 버퍼
/// @param[in] destsize 얻은 문자열 버퍼 크기
/// @param[in] src 원본 문자열
/// @param[in] pos 찾을 위치
/// @param[in] len 얻을 길이
/// @return 얻은 문자열 버퍼 그대로
///
QSAPI char* qn_strmid(char* restrict dest, size_t destsize, const char* restrict src, size_t pos, size_t len);
/// @brief 문자열에서 왼쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
///
QSAPI char* qn_strltm(char* dest);
/// @brief 문자열에서 오른쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
///
QSAPI char* qn_strrtm(char* dest);
/// @brief 문자열에서 왼쪽/오른쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
///
QSAPI char* qn_strtrm(char* dest);
/// @brief 대상 문자열에서 지정한 문자들을 지운다
/// @param[in,out] p 대상 문자열
/// @param[in] rmlist 지울 문자 배열
/// @return 대상 문자열 그대로
///
QSAPI char* qn_strrem(char* restrict p, const char* restrict rmlist);
/// @brief 문자열을 복사하고 대상 문자열을 끝부분을 반환한다
/// @param[in,out] dest 대상 문자열
/// @param[in] src 복사할 문자열
/// @return 복사한 다음 대상 문자열 끝부분
/// @details strcpy(dest, src); dest = dest + strlen(src) 라고 보면 된다
///
QSAPI char* qn_stpcpy(char* restrict dest, const char* restrict src);
/// @brief 문자열을 복제한다
/// @param p 복제할 문자열
/// @return 복제한 새로운 문자열
/// @retval NULL 인수 p 가 NULL
/// @note qn_free 함수로 헤재해야 한다
///
QSAPI char* qn_strdup(const char* p);
/// @brief 여러 문자열을 이어서 붙인다
/// @param[in] p 첫 문자열
/// @note qn_free 함수로 해제해야 한다
///
QSAPI char* qn_strcat(const char* p, ...);
/// @brief 문자열에서 와일드 카드 찾기
/// @param[in] string 대상 문자열
/// @param[in] wild 찾을 와일드 카드
/// @return 대상 문자열에 있으면 참
/// @copyright Written by Jack Handy - jakkhandy@hotmail.com (http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing)
///
QSAPI bool qn_strwcm(const char* string, const char* wild);
/// @brief 문자열에서 와일드 카드 찾기 (대소문자 구분 안함)
/// @param[in] string 대상 문자열
/// @param[in] wild 찾을 와일드 카드
/// @return 대상 문자열에 있으면 참
/// @copyright Written by Jack Handy - jakkhandy@hotmail.com (http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing)
///
QSAPI bool qn_striwcm(const char* string, const char* wild);
/// @brief 문자열을 찾는다
/// @param[in] src 대상 문자열
/// @param[in] find 찾을 문자열
/// @param[in] index 찾기 시작할 대상 문자열의 위치
/// @return 찾으면 찾은 위치를 반환
/// @retval -1 찾지 못했을 때
///
QSAPI int qn_strfnd(const char* src, const char* find, size_t index);
/// @brief 문자열이 서로 같은지 비교
/// @param p1 왼쪽 문자열
/// @param p2 오른쪽 문자열
/// @return 서로 같으면 참을 반환
///
QSAPI bool qn_streqv(const char* p1, const char* p2);
/// @brief 문자열이 서로 같은지 대소문자 무시하고 비교
/// @param p1 왼쪽 문자열
/// @param p2 오른쪽 문자열
/// @return 서로 같으면 참을 반환
///
QSAPI bool qn_strieqv(const char* p1, const char* p2);
/// @brief 32비트 정수를 문자열로 변환
/// @param p 문자열 버퍼
/// @param size 문자열 버퍼 크기
/// @param n 32비트 정수
/// @param base 진수
/// @return 문자열 버퍼가 0이면 필요한 버퍼 크기, 그렇지 않으면 문자열 길이
///
QSAPI int qn_itoa(char* p, int size, int n, uint base);
/// @brief 64비트 정수를 문자열로 변환
/// @param p 문자열 버퍼
/// @param size 문자열 버퍼 크기
/// @param n 64비트 정수
/// @param base 진수
/// @return 문자열 버퍼가 0이면 필요한 버퍼 크기, 그렇지 않으면 문자열 길이
///
QSAPI int qn_lltoa(char* p, int size, llong n, uint base);
/// @brief 문자열을 32비트 정수로
/// @param p 문자열
/// @param base 진수
/// @return 바꾼 정수값
///
QSAPI uint qn_strtoi(const char* p, uint base);
/// @brief 문자열을 64비트 정수로
/// @param p 문자열
/// @param base 진수
/// @return 바꾼 정수값
///
QSAPI ullong qn_strtoll(const char* p, uint base);

#ifdef _MSC_VER
#define qn_strcpy			strcpy_s						/// @brief strcpy
#define qn_strncpy			strncpy_s						/// @brief strncpy
#define qn_strupr			_strupr_s						/// @brief strupr
#define qn_strlwr			_strlwr_s						/// @brief strlwr
#define qn_strncmp			strncmp							/// @brief strncmp
#define qn_stricmp			_stricmp						/// @brief stricmp
#define qn_strnicmp			_strnicmp						/// @brief strnicmp
#else
#define qn_strcpy(a,b,c)	strcpy(a,c)
#define qn_strncpy(a,b,c,d)	strncpy(a,c,d)
QSAPI char* qn_strupr(char* p, size_t size);
QSAPI char* qn_strlwr(char* p, size_t size);
QSAPI int qn_strncmp(const char* p1, const char* p2, size_t len);
QSAPI int qn_stricmp(const char* p1, const char* p2);
QSAPI int qn_strnicmp(const char* p1, const char* p2, size_t len);
#endif

/// @brief 가변 포맷 문자열
/// @param[out] out 출력 문자열 버퍼. 문자열 길이만 얻으려면 NULL로 설정
/// @param[in] len 출력 문자열 버퍼 크기
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열의 길이
///
QSAPI int qn_vsnwprintf(wchar* restrict out, size_t len, const wchar* restrict fmt, va_list va);
/// @brief 가변 포맷 문자열을 메모리 할당으로 만든다
/// @param[out] out 출력 문자열의 버퍼 포인터. 이 값이 NULL이면 곤린하다
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열의 길이
/// @note out 인수로 만든 문자열은 qn_free 함수로 해제해야 한다
/// @see qn_free
///
QSAPI int qn_vaswprintf(wchar** out, const wchar* fmt, va_list va);
/// @brief 가변 포맷 문자열을 메모리 할당으로 만들어 반환한다
/// @param[in] fmt 포맷 문자열
/// @param[in] va 가변 인수
/// @return 결과 문자열
/// @note 반환 값은 qn_free 함수로 해제해야 한다
/// @see qn_free
///
QSAPI wchar* qn_vapswprintf(const wchar* fmt, va_list va);
/// @brief 문자열을 포맷한다
/// @param[out] out 출력 문자열의 버퍼 포인터. 문자열 길이만 얻으려면 NULL로 설정
/// @param[in] len 출력 문자열의 버퍼 포인터의 크기
/// @param[in] fmt 포맷 문자열
/// @return 결과 문자열의 길이
///
QSAPI int qn_snwprintf(wchar* restrict out, size_t len, const wchar* restrict fmt, ...);
/// @brief 문자열을 포맷하고 메모리 할당으로 만든다
/// @param[out] out 출력 문자열의 버퍼 포인터. 이 값이 NULL이면 곤린하다
/// @param[in] fmt 포맷 문자열
/// @return 결과 문자열의 길이
/// @note out 인수로 만든 문자열은 qn_free 함수로 해제해야 한다
/// @see qn_free
///
QSAPI int qn_aswprintf(wchar** out, const wchar* restrict fmt, ...);
/// @brief 문자열을 포맷하고 메모리를 반환한다
/// @param[in] fmt 포맷 문자열
/// @return 결과 문자열
/// @note 반환 값은 qn_free 함수로 해제해야 한다
/// @see qn_free
///
QSAPI wchar* qn_apswprintf(const wchar* fmt, ...);
/// @brief 글자로 채우기
/// @param[in,out] dest 채울 대상 버퍼
/// @param[in] pos 채울 시작 위치
/// @param[in] end 채울 끝 위치
/// @param[in] ch 채울 문자
/// @return 마지막으로 채운 곳 + 1의 위치
///
QSAPI size_t qn_wcsfll(wchar* dest, size_t pos, size_t end, int ch);
/// @brief 문자열 해시
/// @param[in] p 해시할 문자열
/// @return 해시 값
///
QSAPI size_t qn_wcshash(const wchar* p);
/// @brief 문자열 해시 (대소문자 구별 안함
/// @param[in] p 해시할 문자열
/// @return 해시 값
///
QSAPI size_t qn_wcsihash(const wchar* p);
/// @brief 문자열에서 문자열을 찾는다
/// @param[in] p 대상 문자열
/// @param[in] c 찾을 문자열
/// @return 찾을 경우 대상 문자열의 위치 포인터
/// @retval NULL 찾지 못했다
///
QSAPI const wchar* qn_wcsbrk(const wchar* p, const wchar* c);
/// @brief 문자열을 특정 위치에서 얻는다
/// @param[in,out] dest 얻은 문자열 버퍼
/// @param[in] destsize 얻은 문자열 버퍼 크기
/// @param[in] src 원본 문자열
/// @param[in] pos 찾을 위치
/// @param[in] len 얻을 길이
/// @return 얻은 문자열 버퍼 그대로
///
QSAPI wchar* qn_wcsmid(wchar* restrict dest, size_t destsize, const wchar* restrict src, size_t pos, size_t len);
/// @brief 문자열에서 왼쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
///
QSAPI wchar* qn_wcsltm(wchar* dest);
/// @brief 문자열에서 오른쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
///
QSAPI wchar* qn_wcsrtm(wchar* dest);
/// @brief 문자열에서 왼쪽/오른쪽 공백을 없앤다
/// @param[in,out] dest 대상 문자열
/// @return 대상 문자열 그대로
///
QSAPI wchar* qn_wcstrm(wchar* dest);
/// @brief 대상 문자열에서 지정한 문자들을 지운다
/// @param[in,out] p 대상 문자열
/// @param[in] rmlist 지울 문자 배열
/// @return 대상 문자열 그대로
///
QSAPI wchar* qn_wcsrem(wchar* restrict p, const wchar* restrict rmlist);
/// @brief 문자열을 복사하고 대상 문자열을 끝부분을 반환한다
/// @param[in,out] dest 대상 문자열
/// @param[in] src 복사할 문자열
/// @return 복사한 다음 대상 문자열 끝부분
/// @details wcscpy(dest, src); dest = dest + wcslen(src) 라고 보면 된다
///
QSAPI wchar* qn_wcpcpy(wchar* restrict dest, const wchar* restrict src);
/// @brief 문자열을 복제한다
/// @param p 복제할 문자열
/// @return 복제한 새로운 문자열
/// @retval NULL 인수 p 가 NULL
/// @note qn_free 함수로 헤재해야 한다
///
QSAPI wchar* qn_wcsdup(const wchar* p);
/// @brief 여러 문자열을 이어서 붙인다
/// @param[in] p 첫 문자열
/// @note qn_free 함수로 해제해야 한다
///
QSAPI wchar* qn_wcscat(const wchar* p, ...);
/// @brief 문자열에서 와일드 카드 찾기
/// @param[in] string 대상 문자열
/// @param[in] wild 찾을 와일드 카드
/// @return 대상 문자열에 있으면 참
/// @copyright Written by Jack Handy - jakkhandy@hotmail.com (http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing)
///
QSAPI bool qn_wcswcm(const wchar* string, const wchar* wild);
/// @brief 문자열에서 와일드 카드 찾기 (대소문자 구분 안함)
/// @param[in] string 대상 문자열
/// @param[in] wild 찾을 와일드 카드
/// @return 대상 문자열에 있으면 참
/// @copyright Written by Jack Handy - jakkhandy@hotmail.com (http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing)
///
QSAPI bool qn_wcsiwcm(const wchar* string, const wchar* wild);
/// @brief 문자열을 찾는다
/// @param[in] src 대상 문자열
/// @param[in] find 찾을 문자열
/// @param[in] index 찾기 시작할 대상 문자열의 위치
/// @return 찾으면 찾은 위치를. 찾지 못하면 -1
///
QSAPI int qn_wcsfnd(const wchar* src, const wchar* find, size_t index);
/// @brief 문자열이 서로 같은지 비교
/// @param p1 왼쪽 문자열
/// @param p2 오른쪽 문자열
/// @return 서로 같으면 참을 반환
///
QSAPI bool qn_wcseqv(const wchar* p1, const wchar* p2);
/// @brief 문자열이 서로 같은지 대소문자 무시하고 비교
/// @param p1 왼쪽 문자열
/// @param p2 오른쪽 문자열
/// @return 서로 같으면 참을 반환
///
QSAPI bool qn_wcsieqv(const wchar* p1, const wchar* p2);
/// @brief 32비트 정수를 문자열로 변환
/// @param p 문자열 버퍼
/// @param size 문자열 버퍼 크기
/// @param n 32비트 정수
/// @param base 진수
/// @return 문자열 버퍼가 0이면 필요한 버퍼 크기, 그렇지 않으면 문자열 길이
///
QSAPI int qn_itow(wchar* p, int size, int n, uint base);
/// @brief 64비트 정수를 문자열로 변환
/// @param p 문자열 버퍼
/// @param size 문자열 버퍼 크기
/// @param n 64비트 정수
/// @param base 진수
/// @return 문자열 버퍼가 0이면 필요한 버퍼 크기, 그렇지 않으면 문자열 길이
///
QSAPI int qn_lltow(wchar* p, int size, llong n, uint base);
/// @brief 문자열을 32비트 정수로
/// @param p 문자열
/// @param base 진수
/// @return 바꾼 정수값
///
QSAPI uint qn_wcstoi(const wchar* p, uint base);
/// @brief 문자열을 64비트 정수로
/// @param p 문자열
/// @param base 진수
/// @return 바꾼 정수값
///
QSAPI ullong qn_wcstoll(const wchar* p, uint base);

#ifdef _MSC_VER
#define qn_wcscpy			wcscpy_s						/// @brief wcscpy
#define qn_wcsncpy			wcsncpy_s						/// @brief wcsncpy
#define qn_wcsupr			_wcsupr_s						/// @brief wcsupr
#define qn_wcslrw			_wcslwr_s						/// @brief wcslwr
#define qn_wcsncmp			wcsncmp							/// @brief wcsncmp
#define qn_wcsicmp			_wcsicmp						/// @brief wcsicmp
#define qn_wcsnicmp			_wcsnicmp						/// @brief wcsnicmp
#else
#define qn_wcscpy(a,b,c)	wcscpy(a,c)
#define qn_wcsncpy(a,b,c,d)	wcsncpy(a,c,d)
QSAPI wchar* qn_wcsupr(wchar* p, size_t size);
QSAPI wchar* qn_wcslwr(wchar* p, size_t size);
QSAPI int qn_wcsncmp(const wchar* p1, const wchar* p2, size_t len);
QSAPI int qn_wcsicmp(const wchar* p1, const wchar* p2);
QSAPI int qn_wcsnicmp(const wchar* p1, const wchar* p2, size_t len);
#endif

/// @brief UTF-8 문자를 UCS-4 문자로.
/// @param[in] p utf8 문자.
/// @return ucs4 문자.
///
QSAPI uchar4 qn_u8cbn(const char* p);
/// @brief UTF-8 다음 글자
/// @param[in] s utf8 문자열
/// @return 다음 글자 위치
///
QSAPI const char* qn_u8nch(const char* s);
/// @brief UTF-8 글자 길이
/// @param[in] s utf8 문자열
/// @author cp_strlen_utf8 (http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html)
///
QSAPI size_t qn_u8len(const char* s);
/// @brief UTF-8 문자열 복사
/// @param dest 대상 문자열
/// @param src 원본 문자열 
/// @param len 복사할 길이
/// @return 대상 문자열 버퍼 그대로
///
QSAPI char* qn_u8ncpy(char* restrict dest, const char* restrict src, size_t len);
/// @brief UTF-8 문자열 복사
/// @param dest 대상 문자열
/// @param src 원본 문자열
/// @param len 복사할 길이
/// @return 대상 문자열의 길이
///
QSAPI size_t qn_u8lcpy(char* restrict dest, const char* restrict src, size_t len);

/// @brief UCS-4 문자를 UTF-8 문자로
/// @param[in] c ucs4 문자.
/// @param[out] out	utf8 문자가 들어갈 배열. 최소 6개 항목의 배열이어야 한다
/// @return utf8문자를 구성하는 문자열의 길이.
///
QSAPI int qn_u32ucb(uchar4 c, char* out);
/// @brief UTF-16 문자를 UTF-8 문자로
/// @param[in] high UTF-16 상위 서로게이트
/// @param[in] low UTF-16 하위 서로게이트
/// @param[out] out	utf8 문자가 들어갈 배열. 최소 6개 항목의 배열이어야 한다
/// @return utf8문자를 구성하는 문자열의 길이.
///
QSAPI int qn_u16ucb(uchar2 high, uchar2 low, char* out);

/// @brief 멀티바이트 문자열을 와이드 문자열로 변환
/// @param[out] outwcs 출력할 와이드 문자열 버퍼 (NULL 가능)
/// @param[in] outsize 와이드 문자열 버퍼 크기
/// @param[in] inmbs 변환할 멀티바이트 문자열
/// @param[in] insize 멀티바이트 문자열의 크기 (0이어도 된다)
/// @return 와이드 문자열의 길이
/// @remark 와이드 문자열 버퍼가 NULL이면 최종 문자열 길이만 얻는다
/// @see mbstowcs
///
QSAPI size_t qn_mbstowcs(wchar* restrict outwcs, size_t outsize, const char* restrict inmbs, size_t insize);
/// @brief 와이드 문자열을 멀티바이트 문자열로 변환
/// @param[out] outmbs 출력할 멀티바이트 문자열 버퍼 (NULL 가능)
/// @param[in] outsize 멀티바이트 문자열 버퍼 크기
/// @param[in] inwcs 변환할 와이드 문자열
/// @param[in] insize 와이드 문자열의 크기 (0이어도 된다)
/// @return 멀티바이트 문자열의 길이
/// @remark 멀티바이트 문자열 버퍼가 NULL이면 최종 문자열 길이만 얻는다
/// @see wcstombs
///
QSAPI size_t qn_wcstombs(char* restrict outmbs, size_t outsize, const wchar* restrict inwcs, size_t insize);
/// @brief utf8 -> ucs4 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (ucs4)
/// @param[in]	destsize 대상 버퍼 크기
/// @param[in]	src 원본 (utf8)
/// @param[in]	srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
///
QSAPI size_t qn_u8to32(uchar4* restrict dest, size_t destsize, const char* restrict src, size_t srclen);
/// @brief utf8 -> utf16 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (utf16)
/// @param[in]	destsize 대상 버퍼 크기
/// @param[in]	src 원본 (utf8)
/// @param[in]	srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
///
QSAPI size_t qn_u8to16(uchar2* restrict dest, size_t destsize, const char* restrict src, size_t srclen);
/// @brief ucs4 -> utf8 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (utf8)
/// @param[in]	destsize 대상 버퍼 크기
/// @param[in]	src 원본 (ucs4)
/// @param[in]	srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
///
QSAPI size_t qn_u32to8(char* restrict dest, size_t destsize, const uchar4* restrict src, size_t srclen);
/// @brief utf16 -> utf8 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (utf8)
/// @param[in]	destsize 대상 버퍼 크기
/// @param[in]	src 원본 (utf16)
/// @param[in]	srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
///
QSAPI size_t qn_u16to8(char* restrict dest, size_t destsize, const uchar2* restrict src, size_t srclen);
/// @brief utf16 -> ucs4 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (ucs4)
/// @param[in]	destsize 대상 버퍼 크기
/// @param[in]	src 원본 (utf16)
/// @param[in]	srclen 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
///
QSAPI size_t qn_u16to32(uchar4* restrict dest, size_t destsize, const uchar2* restrict src, size_t srclen);
/// @brief ucs4 -> utf16 대상 버퍼가 널값이면 변환에 필요한 길이를 반환
/// @param[out]	dest (널값이 아니면) 대상 버퍼 (utf16)
/// @param[in]	destsize 대상 버퍼 크기
/// @param[in]	src 원본 (ucs4)
/// @param[in]	srclen_org 원본 길이. 0으로 지정할 수 있음
/// @return	변환한 길이 또는 변환에 필요한 길이
///
QSAPI size_t qn_u32to16(uchar2* restrict dest, size_t destsize, const uchar4* restrict src, size_t srclen);
/// @brief 멀티바이트 문자열 -> 와이드 문자열로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
///
QSAPI wchar* qn_mbstowcs_dup(const char* src, size_t srclen);
/// @brief 와이드 문자열 -> 멀티바이트 문자열로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
///
QSAPI char* qn_wcstombs_dup(const wchar* src, size_t srclen);
/// @brief utf8 -> ucs4로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
///
QSAPI uchar4* qn_u8to32_dup(const char* src, size_t srclen);
/// @brief utf8 -> utf16으로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
///
QSAPI uchar2* qn_u8to16_dup(const char* src, size_t srclen);
/// @brief ucs4 -> utf8로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
///
QSAPI char* qn_u32to8_dup(const uchar4* src, size_t srclen);
/// @brief utf16 -> utf8로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
///
QSAPI char* qn_u16to8_dup(const uchar2* src, size_t srclen);
/// @brief utf16 -> ucs4로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
///
QSAPI uchar4* qn_u16to32_dup(const uchar2* src, size_t srclen);
/// @brief ucs4 -> utf16으로 변환하고 버퍼 반환
/// @param src 원본 문자열
/// @param srclen 원본 문자열 길이
/// @return 변환한 버퍼 메모리
/// @warning 반환한 버퍼는 qn_free 함수로 지워야 한다
///
QSAPI uchar2* qn_u32to16_dup(const uchar4* src, size_t srclen);


//////////////////////////////////////////////////////////////////////////
// time

/// @brief 로컬 시간으로 변화
/// @param[out]	ptm	시간 구조체
/// @param[in]	tt time_t 형식의 시간
QSAPI void qn_localtime(struct tm* ptm, time_t tt);

/// @brief UTC 시간으로 변화
/// @param[out]	ptm	시간 구조체
/// @param[in]	tt time_t 형식의 시간
QSAPI void qn_gmtime(struct tm* ptm, time_t tt);

/// @brief 현재 시간 사이클
/// @return	현재의 사이클
QSAPI ullong qn_cycle(void);

/// @brief 프로그램 시작부터 시간 틱
/// @return	현재의 틱
QSAPI ullong qn_tick(void);

/// @brief 밀리초 슬립
/// @param[in]	milliseconds	밀리초 단위로 처리되는 millisecond
QSAPI void qn_sleep(uint milliseconds);

/// @brief 초(second) 슬립
/// @param[in]	seconds	초 단위로 처리되는 second
QSAPI void qn_ssleep(uint seconds);

/// @brief 마이크로 슬립, 정밀 시계를 이용하며 스레드 콘텍스트가 일반 슬립보다 제한된다
/// @param[in]	microseconds	마이크로초 단위로 처리되는 microsecond
QSAPI void qn_msleep(ullong microseconds);

/// @brief date time
typedef struct QnDateTime QnDateTime;
struct QnDateTime
{
	union
	{
		ullong		stamp;				/// @brief 타임 스탬프
		struct
		{
			uint	date;				/// @brief 날짜 스탬프
			uint	time;				/// @brief 시간 스탬프
		};
		struct
		{
			uint	year : 14;			/// @brief 년
			uint	month : 6;			/// @brief 월
			uint	day : 8;			/// @brief 일
			uint	dow : 4;			/// @brief 한주의 일

			uint	hour : 6;			/// @brief 시
			uint	minute : 8;			/// @brief 분
			uint	second : 8;			/// @brief 초
			uint	millisecond : 10;	/// @brief 밀리초
		};
	};
};

/// @brief 현재 날짜 시간.
/// @param[out]	dt	(널값이 아니면) 현재 날짜 시간
QSAPI void qn_now(QnDateTime* dt);

/// @brief 현재의 UTC 날짜 시간
/// @param[out]	dt	(널값이 아니면) 현재 날짜 시간
QSAPI void qn_utc(QnDateTime* dt);

/// @brief 초를 시간으로
/// @param[in]	sec   	초
/// @param[out]	dt	(널값이 아니면) 변환된 시간
QSAPI void qn_stod(double sec, QnDateTime* dt);

/// @brief 밀리초를 시간으로
/// @param[in]	msec  	밀리초
/// @param[out]	dt	(널값이 아니면) 변환된 시간
QSAPI void qn_mstod(uint msec, QnDateTime* dt);

/// @brief timer
typedef struct QnTimer QnTimer;
struct QnTimer
{
	double			abstime;								/// @brief 타이머 절대 시간
	double			runtime;								/// @brief 타이머 시작부터 수행 시간
	double			advance;								/// @brief 타이머 갱신에 따른 시간 (프레임 당 시간)
	double			fps;									/// @brief 초 당 프레임 수
};

/// @brief 타이머 만들기
/// @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 만들어진 타이머
QSAPI QnTimer* qn_timer_new(void);

/// @brief 타이머 제거
/// @param[in]	self	타이머 개체
QSAPI void qn_timer_delete(QnTimer* self);
/// @brief 타이머 리셋
/// @param[in]	self	타이머 개체

QSAPI void qn_timer_reset(QnTimer* self);
/// @brief 타이머 시작
/// @param[in]	self	타이머 개체

QSAPI void qn_timer_start(QnTimer* self);

/// @brief 타이머 정지
/// @param[in]	self	타이머 개체
QSAPI void qn_timer_stop(QnTimer* self);

/// @brief 타이머 갱신
/// @param[in] self 타이머 개체
/// @param[in] manual FPS를 자동으로 계산하려면 false, 아니면 true
/// @return	성공 여부 반화
/// @retval true 성공
/// @retval false 실패
QSAPI bool qn_timer_update(QnTimer* self, bool manual);

/// @brief 타이머의 절대 시간
/// @param[in]	self	타이머 개체
/// @return	double
QSAPI double qn_timer_get_cut(const QnTimer* self);

/// @brief 타이머 과다 수행에 따른 갱신 경과값의 설정
/// @param[in]	self	타이머 개체
/// @param	cut			제한 값
QSAPI void qn_timer_set_cut(QnTimer* self, double cut);


//////////////////////////////////////////////////////////////////////////
// disk i/o

/// @brief 디렉토리
typedef struct QnDir					QnDir;
/// @brief 파일
typedef struct QnFile					QnFile;
/// @brief 모듈
typedef struct QnModule					QnModule;

/// @brief 파일 정보
typedef struct QnFileInfo				QnFileInfo;
struct QnFileInfo
{
	int				type;									/// @brief 파일 타입
	uint			len;
	llong			size;									/// @brief 파일 크기
	llong			cmpr;									/// @brief 압축된 크기
	ullong			stc;									/// @brief 만든 날짜 타임 스탬프
	ullong			stw;									/// @brief 마지막 기록한 타임 스탬프
	char			name[256];								/// @brief 파일 이름
};

/// @brief 파일 제어
typedef struct QnFileAccess				QnFileAccess;
struct QnFileAccess
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
};

/// @brief 파일에서 위치 찾기
typedef enum QnSeek
{
	QNSEEK_BEGIN = 0,										/// @brief 처음부터
	QNSEEK_CUR = 1,											/// @brief 현 위치부터
	QNSEEK_END = 2,											/// @brief 끝부터
} QnSeek;

/// @brief 파일 플래그
typedef enum QnFileFlag
{
	QNFF_READ = 0x1,										/// @brief 읽기
	QNFF_WRITE = 0x2,										/// @brief 쓰기
	QNFF_SEEK = 0x4,										/// @brief 위치 찾기
	QNFF_ALL = QNFF_READ | QNFF_WRITE | QNFF_SEEK,			/// @brief 모든 플래그
	QNFF_RDONLY = QNFF_READ | QNFF_SEEK,					/// @brief 읽기 전용
} QnFileFlag;

// file

/// @brief 새 파일 처리 구조를 만든다
/// @param[in]	filename	파일의 이름
/// @param[in]	mode		파일 처리 모드
/// @return	만들어진 파일 구조
/// @retval NULL 만들지 못했다
QSAPI QnFile* qn_file_new(const char* restrict filename, const char* restrict mode);

/// @brief 파일 복제. 핸들을 복제하여 따로 사용할 수 있도록 한다
/// @param[in]	src	(널값이 아닌) 원본
/// @return	만들어진 반환 구조
/// @retval NULL 복제하지 못했다
QSAPI QnFile* qn_file_new_dup(QnFile* org);

/// @brief 파일 구조를 제거한다. 더 이상 파일 관리를 하지 않을 때 사용한다
/// @param[in]	self	파일 개체
/// @note 파일을 지우는게 아니다!
QSAPI void qn_file_delete(QnFile* self);

/// @brief 파일 플래그를 가져온다
/// @param[in]	self	파일 개체
/// @param[in]	mask		플래그의 마스크
/// @return	파일 플래그
QSAPI int qn_file_get_flags(const QnFile* self, int mask);

/// @brief 파일 이름을 얻는다
/// @param[in]	self	파일 개체
/// @return	파일의 이름
QSAPI const char* qn_file_get_name(const QnFile* self);

/// @brief 파일에서 읽는다
/// @param[in]	self	파일 개체
/// @param[in,out]	buffer  	읽은 데이터를 저장할 버퍼
/// @param[in]	offset  	버퍼의 옵셋
/// @param[in]	size		읽을 크기
/// @return	실제 읽은 길이를 반환
QSAPI int qn_file_read(QnFile* self, void* restrict buffer, int offset, int size);

/// @brief 파일에 쓴다
/// @param[in]	self	파일 개체
/// @param[in]	buffer  	쓸 데이터가 들어 있는 버퍼
/// @param[in]	offset  	버퍼의 옵셋
/// @param[in]	size		쓸 크기
/// @return	실제 쓴 길이를 반환
QSAPI int qn_file_write(QnFile* self, const void* restrict buffer, int offset, int size);

/// @brief 파일 크기를 얻는다
/// @param[in]	self	파일 개체
/// @return	파일의 길이
QSAPI int64_t qn_file_size(QnFile* self);

/// @brief 파일 위치 얻기
/// @param[in]	self	파일 개체
/// @return	현재 파일의 읽고 쓰는 위치
QSAPI int64_t qn_file_tell(QnFile* self);

/// @brief 파일 위치 변경
/// @param[in]	self	파일 개체
/// @param[in]	offset  	이동할 위치
/// @param[in]	org			방식 (C 형식 SEEK와 동일)
/// @return	변경된 위치.
QSAPI int64_t qn_file_seek(QnFile* self, llong offset, QnSeek org);

/// @brief 파일 갱신. 파일 내용을 갱신한다. 쓰기 버퍼의 남은 데이터를 모두 쓴다
/// @param[in]	self	파일 개체
/// @return	갱신 성공 여부
/// @retval true 갱신 성공
/// @retval false 갱신 실패
/// @remark 반환값은 무시해도 좋다
QSAPI bool qn_file_flush(const QnFile* self);

/// @brief 파일에 포맷된 문자열 쓰기
/// @param[in]	self	파일 개체
/// @param[in]	fmt			포맷 문자열
/// @return	실제 쓴 길이
QSAPI int qn_file_printf(QnFile* self, const char* restrict fmt, ...);

/// @brief 파일에 포맷된 문자열 쓰기 가변형
/// @param[in]	self	파일 개체
/// @param[in]	fmt			포맷 문자열
/// @param[in]	va			가변 인수
/// @return	실제 쓴 길이
QSAPI int qn_file_vprintf(QnFile* self, const char* restrict fmt, va_list va);

/// @brief 파일이 있나 조사한다
/// @param[in]	filename	파일의 이름
/// @param[out]	isdir 	(널값이 아니면) 파일 처리 플래그로 KFAS_로 시작하는 마스크 플래그
/// @return 파일 조사 여부
/// @retval true 파알이 있다
/// @retval false 파일이 없다
QSAPI bool qn_file_exist(const char* restrict filename, /*RET-NULLABLE*/bool* isdir);

/// @brief 파일 할당. 즉, 파일 전체를 읽어 메모리에 할당한 후 반환한다
/// @param[in]	filename	파일의 이름
/// @param[out]	size	(널값이 아니면) 읽은 파일의 크기
/// @return	읽은 버퍼. 사용한 다음 k_free 함수로 해제해야한다
QSAPI void* qn_file_alloc(const char* restrict filename, int* size);

/// @brief 유니코드용 새 파일 구조를 만든다
/// @param[in]	filename	파일의 이름
/// @param[in]	mode		파일 처리 모드
/// @return	만들어진 파일 구조
/// @retval NULL 문제가 있거나 실패했다
QSAPI QnFile* qn_file_new_l(const wchar* restrict filename, const wchar* restrict mode);

/// @brief 파일이 있나 조사한다. 유니코드 버전
/// @param[in]	filename	파일의 이름
/// @param[out]	isdir 	(널값이 아니면) 파일 처리 플래그로 KFAS_로 시작하는 마스크 플래그
/// @return	파일 존재 여부 반환
/// @retval true 파일이 있따
/// @retval false 파일이 없다
QSAPI bool qn_file_exist_l(const wchar* restrict filename, /*RET-NULLABLE*/bool* isdir);

/// @brief 파일 할당. 즉, 파일 전체를 읽어 메모리에 할당한 후 반환한다. 유니코드 버전
/// @param[in]	filename	파일의 이름
/// @param[out]	size	(널값이 아니면) 읽은 파일의 크기
/// @return	읽은 버퍼. 사용한 다음 k_free 함수로 해제해야한다
QSAPI void* qn_file_alloc_l(const wchar* restrict filename, int* size);

/// @brief qn_file_alloc 함수에서 사용하는 파일 읽기 최대 할당 크기
/// @return	최대 할당 크기
QSAPI size_t qn_file_get_max_alloc_size(void);

/// @brief qn_file_alloc 함수에서 사용할 파일 읽기 최대 할당 크기 설정
/// @param[in]	n	할당할 크기
QSAPI void qn_file_set_max_alloc_size(size_t n);

// directory

/// @brief 디렉토리를 새로 만든다
/// @param[in]	path 	디렉토리의 완전한 경로 이름
/// @return 디렉토리 관리 개체 반환
/// @retval NULL 문제가 있거나 실패했다
QSAPI QnDir* qn_dir_new(const char* path);

/// @brief 디렉토리 개체 제거
/// @param[in]	self	디렉토리 개체
QSAPI void qn_dir_delete(QnDir* self);

/// @brief 디렉토리에서 항목 읽기
/// @param[in]	self	디렉토리 개체
/// @return 읽은 디렉토리 이름
/// @retval NULL 문제가 있더나 실패했을때. 아니면 더 이상 파일이 없다
QSAPI const char* qn_dir_read(QnDir* self);

/// @brief 디렉토리를 첫 항목으로 감기
/// @param[in]	self	디렉토리 개체
QSAPI void qn_dir_rewind(QnDir* self);

/// @brief 디렉토리에서 순서 항목으로 찾아가기
/// @param[in]	self	디렉토리 개체
/// @param	pos			찾아갈 위치
QSAPI void qn_dir_seek(QnDir* self, int pos);

/// @brief 디렉토리에서 몇번째 항목인지 얻기
/// @param[in]	self	디렉토리 개체
/// @return	항목 순서
QSAPI int qn_dir_tell(QnDir* self);

/// @brief 디렉토리를 새로 만든다 (유니코드 사용)
/// @param	path 	디렉토리의 완전한 경로 이름
/// @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 만들어진 개체
QSAPI QnDir* qn_dir_new_l(const wchar* path);

/// @brief 디렉토리에서 항목 읽기 (유니코드)
/// @param[in]	self	디렉토리 개체
/// @return 디렉토리 관리 개체 반환
/// @retval NULL 문제가 있거나 실패했다
QSAPI const wchar_t* qn_dir_read_l(QnDir* self);

/// @brief 프로그램 기본 패스를 얻는다
/// @return 기본 패스
/// @warning 반환 값은 qn_free 함수로 해제해야한다
QSAPI char* qn_dir_base_path(void);

// module

/// @brief 프로그램을 모듈로 얻는다
/// @return 프로그람 자체 모듈
QSAPI QnModule* qn_mod_self(void);

/// @brief 모듈을 연다
/// @param filename 파일 이름 (대소문자 구별한다!)
/// @param flags 플래그. 지금은 사용하지 않는다 0으로
/// @return 만들어진 모듈
/// @note 파일 이름은 대소문자를 구별하며 오류를 막기위해 윈도우에서는 대문자로 하는게 좋아요
QSAPI QnModule* qn_mod_load(const char* filename, int flags);

/// @brief 모듈 사용을 그만한다
/// @param self 모듈
/// @return 성공적으로 언로드하면 참을 반환
/// @details 모듈은 참조 방식으로 다른 곳에서 같은 모듈을 사용할 수도 있다.
/// @see qn_mod_ref
QSAPI bool qn_mod_unload(QnModule* self);

/// @brief 함수를 읽는다
/// @param self 모듈
/// @param name 읽을 함수 이름
/// @return 함수 포인터
QSAPI void* qn_mod_func(QnModule* self, const char* restrict name);

/// @brief 모듈의 참조값을 얻는다
/// @param self 모듈
/// @return 모듈의 참조값
QSAPI int qn_mod_ref(QnModule* self);


//////////////////////////////////////////////////////////////////////////
// xml

typedef struct QnMlu		QnMlu;

//
typedef struct QnMlTag		QnMlTag;
struct QnMlTag
{
	char*			name;
	char*			context;
	int				name_len;
	int				context_len;
	int				line;
	bool32			sibling;
};

// ml unit

/// @brief RML을 만든다
/// @return 만들어진 RML 개체
/// @retval NULL 문제가 있거나 실패했을 때
///
QSAPI QnMlu* qn_mlu_new(void);
/// @brief 파일에서 RML을 만든다
/// @param[in]	filename	파일의 이름
/// @return 만들어진 RML 개체
/// @retval NULL 문제가 있거나 실패했을 때
///
QSAPI QnMlu* qn_mlu_new_file(const char* filename);
/// @brief 파일에서 RML을 만든다. 유니코드 파일 이름을 사용한다
/// @param[in]	filename	파일의 이름
/// @return 만들어진 RML 개체
/// @retval NULL 문제가 있거나 실패했을 때
///
QSAPI QnMlu* qn_mlu_new_file_l(const wchar* filename);
/// @brief 버퍼에서 RML을 만든다
/// @param[in]	data	버퍼
/// @param[in]	size	버퍼 크기
/// @return 만들어진 RML 개체
/// @retval NULL 문제가 있거나 실패했을 때
///
QSAPI QnMlu* qn_mlu_new_buffer(const void* restrict data, int size);
/// @brief RML을 제거한다
/// @param[in]	self	Mlu 개체
///
QSAPI void qn_mlu_delete(QnMlu* self);

/// @brief 모든 RML 태그를 삭제한다
/// @param[in]	self	Mlu 개체
///
QSAPI void qn_mlu_clean_tags(QnMlu* self);
/// @brief 모든 RML 오류를 삭제한다
/// @param[in]	self	Mlu 개체
///
QSAPI void qn_mlu_clean_errs(QnMlu* self);
/// @brief 버퍼에서 RML을 분석하여 읽는다
/// @param[in]	self	Mlu 개체
/// @param[in]	data		버퍼
/// @param[in]	size		버퍼 크기
/// @return 분석하고 읽는 것 성공 여부
/// @retval true 버퍼에서 읽는데 성공
/// @retval false 버퍼에서 읽을 수가 없다
///
QSAPI bool qn_mlu_load_buffer(QnMlu* self, const void* restrict data, int size);
/// @brief RML 내용을 파일로 저장한다
/// @param[in]	self	Mlu 개체
/// @param[in]	filename	파일의 이름
/// @return 파일로 저장 성공 여부
/// @retval true 저장 성공
/// @retval false 저장할 수 없었다
///
QSAPI bool qn_mlu_write_file(const QnMlu* self, const char* restrict filename);

/// @brief 갖고 있는 최상위 태그의 갯수를 반환한다
/// @param[in]	self	Mlu 개체
/// @return	최상위 태그 갯수
///
QSAPI int qn_mlu_get_count(const QnMlu* self);
/// @brief 갖고 있는 오류를 순번으로 얻는다
/// @param[in]	self	Mlu 개체
/// @param[in]	at			오류 순번
/// @return at 번째 오류 메시지
/// @retval NULL 더 이상 오류가 없다
///
QSAPI const char* qn_mlu_get_err(const QnMlu* self, int at);
/// @brief 최상위 태그를 찾는다
/// @param[in]	self	Mlu 개체
/// @param[in]	name		태그 이름
/// @return	이름에 해당하는 태그 개체
/// @retval NULL 이름에 해당하는 태그가 없다
///
QSAPI QnMlTag* qn_mlu_get_tag(const QnMlu* self, const char* restrict name);
/// @brief 최상위 태그를 순번으로 얻는다
/// @param[in]	self	Mlu 개체
/// @param[in]	at			순번
/// @return at 번째 태그
/// @retval NULL 해당하는 순번에 개체가 없거나, at 범위 밖이다
///
QSAPI QnMlTag* qn_mlu_get_tag_nth(const QnMlu* self, int at);
/// @brief 최상위 태그가 갖고 있는 컨텍스트를 얻는다
/// @param[in]	self  	개체나 인터페이스의 자기 자신 값
/// @param[in]	name	  	태그 이름
/// @param[in]	ifnotexist	태그가 존재하지 않을 경우 반환할 값
/// @return 태그 컨텍스트
/// @retval ifnotexist 이름에 해당하는 태그가 없거나 문제가 있다
///
QSAPI const char* qn_mlu_get_context(const QnMlu* self, const char* restrict name, const char* restrict ifnotexist);
/// @brief 최상위 태그가 갖고 있는 컨텍스트를 얻는다. 태그는 순번으로 검사한다
/// @param[in]	self  	개체나 인터페이스의 자기 자신 값
/// @param[in]	at		  	순번
/// @param[in]	ifnotexist	태그가 존재하지 않을 경우 반환할 값
/// @return 태그 컨텍스트
/// @retval ifnotexist 이름에 해당하는 태그가 없거나 문제가 있다
///
QSAPI const char* qn_mlu_get_context_nth(const QnMlu* self, int at, const char* restrict ifnotexist);
/// @brief 지정한 태그가 있나 검사한다
/// @param[in]	self	Mlu 개체
/// @param[in]	tag	(널값이 아님) 지정한 태그
/// @return 지정한 태그가 몇번째 인가 반환
/// @retval -1 지정한 태그가 없다
///
QSAPI int qn_mlu_contains(const QnMlu* self, QnMlTag* restrict tag);

/// @brief 최상위 태그에 대해 ForEach를 수행한다
/// @param[in]	self		개체나 인터페이스의 자기 자신 값
/// @param[in]	func	콜백 함수
/// @param	userdata		콜백 데이터
///
QSAPI void qn_mlu_foreach(const QnMlu* self, void(*func)(void*, QnMlTag*), void* userdata);
/// @brief 최상위 태그에 대해 LoopEach를 수행한다
/// @param[in]	self		개체나 인터페이스의 자기 자신 값
/// @param[in]	func	콜백 함수
///
QSAPI void qn_mlu_loopeach(const QnMlu* self, void(*func)(QnMlTag* tag));

/// @brief 태그를 추가한다
/// @param[in]	self	Mlu 개체
/// @param[in]	name		태그 이름
/// @param[in]	context 	태그 컨텍스트
/// @param[in]	line		줄번호
/// @return 추가한 새 태그
/// @retval NULL 태그를 추가할 수 없었다
///
QSAPI QnMlTag* qn_mlu_add(QnMlu* self, const char* restrict name, const char* restrict context, int line);
/// @brief 태그를 추가한다
/// @param[in]	self   	Ml 개체
/// @param[in]	tag	(널값이 아님) 추가할 태그
/// @return 추가한 새 태그
/// @retval NULL 태그를 추가할 수 없었다
///
QSAPI QnMlTag* qn_mlu_add_tag(QnMlu* self, QnMlTag* restrict tag);
/// @brief 태그를 제거한다
/// @param[in]	self	Mlu 개체
/// @param[in]	name		태그 이름
/// @param[in]	isall   	같은 이름 태그를 모두 지우려면 참으로 넣는다
/// @return	지운 태그의 갯수
///
QSAPI int qn_mlu_remove(QnMlu* self, const char* restrict name, bool isall);
/// @brief 태그를 순번으로 제거한다
/// @param[in]	self	Mlu 개체
/// @param[in]	at			순번
/// @return 태그 제거 여부
/// @retval true 제거 성공
/// @retval false 제거 실패
///
QSAPI bool qn_mlu_remove_nth(QnMlu* self, int at);
/// @brief 태그를 제거한다
/// @param[in]	self	Mlu 개체
/// @param[in]	tag	(널값이 아님) 지울 태그
/// @param[in]	isdelete   	태그를 삭제하려면 참으로 넣는다
/// @return	태그 제거 여부
/// @retval true 태그 제거 성공
/// @retval false 태그 제거 실패
///
QSAPI bool qn_mlu_remove_tag(QnMlu* self, QnMlTag* restrict tag, bool isdelete);

/// @brief 오류값을 추가한다
/// @param[in]	self	Mlu 개체
/// @param[in]	msg			메시지
///
QSAPI void qn_mlu_add_err(QnMlu* self, const char* restrict msg);
/// @brief 오류값을 포맷 방식으로 추가한다
/// @param[in]	self	Mlu 개체
/// @param[in]	fmt			포맷 문자열
/// @param ...
///
QSAPI void qn_mlu_add_errf(QnMlu* self, const char* restrict fmt, ...);
/// @brief 갖고 있는 오류를 디버그 콘솔로 출력한다
/// @param[in]	self	Mlu 개체
///
QSAPI void qn_mlu_print_err(const QnMlu* self);

/// @brief RML 정보 구성 내용을 디버그 콘솔로 출력한다
/// @param[in]	self	Mlu 개체
///
QSAPI void qn_mlu_print(QnMlu* self);

// tag

/// @brief 태그 노드를 만든다
/// @param[in]	name	태그 이름
/// @return	만들어지느 태그 노드
/// @retval NULL 태그 노드를 만들 수가 없었다
///
QSAPI QnMlTag* qn_mltag_new(const char* name);
/// @brief 태그 노드를 제거한다
/// @param[in]	self	MlTag 개체
///
QSAPI void qn_mltag_delete(QnMlTag* self);

// tag - context

/// @brief 태그 내용을 추가한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	cntx	   	컨텍스트
/// @param[in]	size	   	컨텍스트의 크기
///
QSAPI void qn_mltag_add_context(QnMlTag* ptr, const char* restrict cntx, int size);
/// @brief 태그에 내용을 쓴다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	cntx 컨텍스트
/// @param[in]	size 컨텍스트의 크기
///
QSAPI void qn_mltag_set_context(QnMlTag* ptr, const char* restrict cntx, int size);

// tag - sub

/// @brief 하부 태그의 갯수를 얻는다
/// @param[in]	ptr	MlTag 개체
/// @return	하부 태그의 갯수
///
QSAPI int qn_mltag_get_sub_count(QnMlTag* ptr);
/// @brief 하부 태그를 찾는다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	name	   	찾을 태그 이름
/// @return 이름에 해당하는 태그
/// @retval NULL 이름에 해당하는 태그가 없다
///
QSAPI QnMlTag* qn_mltag_get_sub(QnMlTag* ptr, const char* restrict name);
/// @brief 하부 태그를 순번으로 찾는다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	at		   	순번
/// @return 순번에 해당하는 태그
/// @retval NULL 순번에 해당하는 태그가 없거나 범위 밖이다
///
QSAPI QnMlTag* qn_mltag_get_sub_nth(QnMlTag* ptr, int at);
/// @brief 하부 태그의 컨텍스트를 얻는다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	name	   	태그 이름
/// @param[in]	ifnotexist 	태그를 찾을 수 없으면 반환할 값
/// @return 태그의 컨텍스트
/// @retval ifnotexist 태그가 찾을 수 없다
///
QSAPI const char* qn_mltag_get_sub_context(QnMlTag* ptr, const char* restrict name, const char* restrict ifnotexist);
/// @brief 하부 태그의 컨텍스트를 순번으로 얻는다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	at			순번
/// @param[in]	ifnotexist 	태그를 찾을 수 없으면 반환할 값
/// @return	서브 컨텍스트 내용
/// @retval ifnotexist 서브 컨텍스트를 찾을 수 없다
///
QSAPI const char* qn_mltag_get_sub_context_nth(QnMlTag* ptr, int at, const char* restrict ifnotexist);
/// @brief 지정한 태그를 하부 태그로 갖고 있나 조사
/// @param[in]	ptr	MlTag 개체
/// @param[in]	tag	찾을 태그
/// @return 하부 태그의 인덱스 순번
/// @retval -1 해당하는 태그가 없다
///
QSAPI int qn_mltag_contains_sub(QnMlTag* restrict ptr, QnMlTag* restrict tag);
/// @brief 하부 태그를 추가한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	name	   	태그 이름
/// @param[in]	context	   	태그 콘텍스트
/// @param[in]	line	   	줄 번호
/// @return 추가한 하부 태그
/// @retval NULL 하부 태그를 추가할 수 없다
///
QSAPI QnMlTag* qn_mltag_add_sub(QnMlTag* ptr, const char* restrict name, const char* restrict context, int line);
/// @brief 하부 태그를 추가한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	tag	추가할 태그
/// @return 추가한 하부 태그
/// @retval NULL 하부 태그를 추가할 수 없다
///
QSAPI QnMlTag* qn_mltag_add_sub_tag(QnMlTag* restrict ptr, QnMlTag* restrict tag);
/// @brief 지정한 이름의 태그를 제거한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	name	   	태그 이름
/// @param[in]	isall	   	같은 이름의 모든 태그를 지우려면 참으로 넣는다
/// @return	지운 태그의 갯수
///
QSAPI int qn_mltag_remove_sub(QnMlTag* ptr, const char* restrict name, bool isall);
/// @brief 지정한 순번의 하부 태그를 삭제한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	at			순번
/// @return 하부 태그 삭제 여부
/// @retval true 태그를 지웠다
/// @retval false 태그를 지울 수 없다
///
QSAPI bool qn_mltag_remove_sub_nth(QnMlTag* ptr, int at);
/// @brief 지정한 하부 태그를 삭제한다
/// @param[in]	ptr	MlTag 개체
/// @param[in,out]	tag	(널값이 아님) 지울 태그
/// @param[in]	isdelete   	태그 자체를 삭제하려면 참으로 넣는다
/// @return 하부 태그 삭제 여부
/// @retval true 태그를 지웠다
/// @retval false 태그를 지울 수 없다
///
QSAPI bool qn_mltag_remove_sub_tag(QnMlTag* restrict ptr, QnMlTag* restrict tag, bool isdelete);

/// @brief 하부 태그에 대해 ForEach 연산을 수행한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	func	콜백 함수
/// @param	userdata		콜백 데이터
///
QSAPI void qn_mltag_foreach_sub(QnMlTag* ptr, void(*func)(void* userdata, QnMlTag* tag), void* userdata);
/// @brief 하부 태그에 대해 LoopEach 연산을 수행한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	func	콜백 함수
///
QSAPI void qn_mltag_loopeach_sub(QnMlTag* ptr, void(*func)(QnMlTag* tag));

// tag - arg

/// @brief 태그의 인수의 개수를 반환한다
/// @param[in]	ptr	MlTag 개체
/// @return	인수의 개수
///
QSAPI int qn_mltag_get_arity(QnMlTag* ptr);
/// @brief 인수를 이름으로 찾는다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	name	   	인수 이름
/// @param[in]	ifnotexist 	인수를 찾지 못하면 반환할 값
/// @return 이름에 해당하는 태그의 인수
/// @retval ifnotexist 이름에 해당하는 인수 값이 없다
///
QSAPI const char* qn_mltag_get_arg(QnMlTag* ptr, const char* restrict name, const char* restrict ifnotexist);
/// @brief 다음 인수를 찾는다
/// @param[in]	ptr	MlTag 개체
/// @param[in,out]	index	(널값이 아님) 내부 찾기 인덱스 데이터
/// @param[in]	name		 	인수 이름
/// @param[in]	data		 	인수 자료
/// @return 다음 인수의 존재 여부
/// @retval true 다음 인수가 있다
/// @retval false 다음 인수가 없다
///
QSAPI bool qn_mltag_next_arg(QnMlTag* ptr, void** restrict index, const char** restrict name, const char** restrict data);
/// @brief 이름에 해당하는 인수가 있는지 조사한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	name	   	인수 이름
/// @return 이름에 해당하는 인수 존재 여부
/// @retval true 인수가 있다
/// @retval false 인수가 없다
///
QSAPI bool qn_mltag_contains_arg(QnMlTag* ptr, const char* restrict name);

/// @brief 인수에 대해 ForEach 연산을 수행한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	func	콜백 함수
/// @param	userdata		콜백 데이터
///
QSAPI void qn_mltag_foreach_arg(QnMlTag* ptr, void(*func)(void* userdata, char* const* name, char* const* data), void* userdata);
/// @brief 인수에 대해 LoopEach 연산을 수행한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	func	콜백 함수
///
QSAPI void qn_mltag_loopeach_arg(QnMlTag* ptr, void(*func)(char* const* name, char* const* data));

/// @brief 인수를 추가한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	name	   	인수 이름
/// @param[in]	value	   	인수 값
///
QSAPI void qn_mltag_set_arg(QnMlTag* ptr, const char* restrict name, const char* restrict value);
/// @brief 인수를 제거한다
/// @param[in]	ptr	MlTag 개체
/// @param[in]	name	   	제거할 인수 이름
/// @return	인수 제거 여부
/// @retval true 인수 제거의 성공
/// @retval false 인수 제거의 실패
///
QSAPI bool qn_mltag_remove_arg(QnMlTag* ptr, const char* restrict name);


//////////////////////////////////////////////////////////////////////////
// thread

#if defined _MSC_VER
#define qn_barrier()		_ReadWriteBarrier()				/// @brief 메모리 바리어
#if defined _M_IX86 || defined _M_X64
#define qn_pause()			_mm_pause()						/// @brief CPU 포즈
#elif defined _M_ARM || defined _M_ARM64
#define qn_pause()			_yield()
#endif
#elif defined __GNUC__
#if defined __i386__ || defined __x86_64__
#define qn_barrier()		__asm__ __volatile__("" : : : "memory");
#define qn_pause()			__asm__ __volatile__ ("pause\n")
#elif defined __aarch64__
#define qn_barrier()		__asm__ __volatile__("dsb sy" : : : "memory")
#define qn_pause()			__asm__ __volatile__ ("yield" ::: "memory")
#elif defined __EMSCRIPTEN__
#define qn_barrier()
#define qn_pause()
#else
#error unknown platform
#endif
#else
#error unknown compiler
#endif

typedef int volatile		QnSpinLock;						/// @brief 스핀락
typedef void				QnTls;							/// @brief TLS
typedef struct QnMutex		QnMutex;						/// @brief 뮤텍스
typedef struct QnCond		QnCond;							/// @brief 컨디션
typedef struct QnSem		QnSem;							/// @brief 세마포어
typedef struct QnThread		QnThread;						/// @brief 스레드
typedef void* (*QnThreadCallback)(void*);					/// @brief 스레드 콜백


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

/// @brief 스핀락 들어간다
#define QN_LOCK(sp)			qn_spin_enter(&sp)
/// @brief 스핀락 나온다
#define QN_UNLOCK(sp)		qn_spin_leave(&sp);


/// @brief 스레드
struct QnThread
{
	char*				name;
	bool32				canwait;
	bool32				managed;

	int					busy;
	uint				stack_size;

	QnThreadCallback	cb_func;
	void*				cb_data;
	void*				cb_ret;
};

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
QSAPI QnThread* qn_thread_new(const char* restrict name, QnThreadCallback func, void* data, uint stack_size, int busy);

/// @brief 스레드를 제거한다. 실행 중이면 끝날 때 까지 대기한다
/// @param self 스레드
QSAPI void qn_thread_delete(QnThread* self);

/// @brief 스레드를 바로 실행한다
/// @param name 스레드 이름
/// @param func 스레드 콜백 함수
/// @param data 콜백 데이터
/// @param stack_size 스택 크기 (보통 0을 넣어도 된다)
/// @param busy 우선 순위
/// @return 성공적으로 실행했으면 참
/// @see qn_thread_new qn_thread_start
QSAPI bool qn_thread_once(const char* restrict name, QnThreadCallback func, void* data, uint stack_size, int busy);

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
QSAPI int qn_thread_get_busy(QnThread* self);

/// @brief 스레드 우선 순위를 설정한다
/// @param self 스레드
/// @param busy 우선 순위 (-2 ~ 2)
/// @return 성공하면 참
QSAPI bool qn_thread_set_busy(QnThread* self, int busy);

// tls

/// @brief TLS를 만든다
/// @param callback TLS가 제거될 때 필요한 콜백 (NULL 허용)
/// @return TLS 값
QSAPI QnTls* qn_tls(paramfunc_t callback);

/// @brief TLS에 값을 쓴다
/// @param tls 대상 TLS
/// @param data 넣을 값
QSAPI void qn_tlsset(QnTls* tls, void* restrict data);

/// @brief TLS에서 값을 읽는다
/// @param tls 대상 TLS
/// @return 읽은 값
QSAPI void* qn_tlsget(QnTls* tls);

// mutex

/// @brief 뮤텍스를 만든다. 이 뮤텍스는 RECURSIVE 타입이다
/// @return 만들어진 뮤텍스
QSAPI QnMutex* qn_mutex_new(void);

/// @brief 뮤텍스를 삭제한다
/// @param self 뮤텍스
QSAPI void qn_mutex_delete(QnMutex* self);

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
QSAPI QnCond* qn_cond_new(void);

/// @brief 조건을 삭제한다
/// @param self 조건
QSAPI void qn_cond_delete(QnCond* self);

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
QSAPI QnSem* qn_sem_new(int initial);

/// @brief 세마포어를 삭제한다
/// @param self 세마포어
QSAPI void qn_sem_delete(QnSem* self);

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


//////////////////////////////////////////////////////////////////////////
// gam 

/// @brief GAM 할거리를 의미. 영어로 OBJECT
typedef struct QsGam		QsGam;

/// @brief GAM 가상 테이블 이름을 얻는다
#define qv_name(type)		struct _vt_##type
/// @brief GAM 가상 테이블로 GAM 포인터를 캐스팅한다
#define qv_cast(g,type)		((struct _vt_##type*)((QsGam*)(g))->vt)

/// @brief GAM 포인터를 다른 타입으로 캐스팅한다
#define qs_cast(g,type)		((type*)(g))

qv_name(QsGam)
{
	const char* name;
	void (*dispose)(QsGam*);
};

struct QsGam
{
	qv_name(QsGam)*	vt;
	volatile nint	ref;
	nuint			desc;
};

/// @brief GAM 가상 테이블을 초기화하고 GAM 포인터 반환
/// @param g 현재 오브젝트
/// @param vt 가상 테이블
/// @return 현재 오브젝트 그대로
QSAPI QsGam* qs_sc_init(QsGam* restrict g, void* restrict vt);

/// @brief 참조를 추가한다.
/// @param g 현재 오브젝트
/// @return 현재 오브젝트 그대로
QSAPI QsGam* qs_sc_load(QsGam* restrict g);

/// @brief 참조를 제거한다. 참조가 0이 되면 제거한다
/// @param g 현재 오브젝트
/// @return 현재 오브젝트 그대로
QSAPI QsGam* qs_sc_unload(QsGam* restrict g);

/// @brief 참조를 얻는다
/// @param g 현재 오브젝트
/// @return 현재 참조값
QSAPI nint qs_sc_get_ref(QsGam* restrict g);

/// @brief 표현자(디스크립터)를 얻는다
/// @param g 현재 오브젝트
/// @return 현재 표현자
QSAPI nuint qs_sc_get_desc(const QsGam* restrict g);

/// @brief 표현자(디스크립터)를 쓴다
/// @param g 현재 오브젝트
/// @param ptr 표현자(디스크립터)
/// @return 설정하기 전에 갖고 있던 이전 표현자
QSAPI nuint qs_sc_set_desc(QsGam* restrict g, nuint ptr);

/// @brief GAM 가상 테이블을 초기화하고 GAM 포인터 반환
#define qs_init(g,type,pvt)	((type*)qs_sc_init((QsGam*)(g), pvt))
/// @brief 참조를 추가한다
#define qs_load(g)			((g) ? qs_sc_load((QsGam*)(g)) : NULL)
/// @brief 참조를 제거한다. 참조가 0이 되면 제거한다
#define qs_unload(g)		((g) ? qs_sc_unload((QsGam*)(g)) : NULL)
/// @brief 참조를 얻는다
#define qs_get_ref(g)		qs_sc_get_ref((QsGam*)(g))
/// @brief 표현자(디스크립터)를 얻는다
#define qs_get_desc(g,type)	(type)qs_sc_get_desc((QsGam*)(g))
/// @brief 표현자(디스크립터)를 쓴다
#define qs_set_desc(g,ptr)	qs_sc_set_desc((QsGam*)(g),(nuint)(ptr))


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

QN_EXTC_END
