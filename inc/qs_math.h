//
// QsLib [MATH Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: BSD-2-Clause
//
// 자료형 읽는법:
// - VEC1,2,3,4: 벡터 하나,둘,셋,넷
// - FLOAT2,3,4: 실수형 둘,셋,넷
//

#pragma once
#define __QS_MATH__

#ifdef __clang__
#pragma clang diagnotics push
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

//#define QS_NO_SIMD	// SIMD 사용 안함 테스트

#if !defined __EMSCRIPTEN__ && !defined QS_NO_SIMD
#if defined __AVX__
#define QM_USE_AVX		1
#endif
#if defined __AVX2__
#define QM_USE_AVX2		1
#endif
#if defined _M_ARM || defined __ARM_NEON
#define QM_USE_NEON		1
#endif
#if defined QM_USE_AVX || defined QM_USE_AVX2 || defined QM_USE_NEON
#define QM_USE_SIMD		1
#endif
#endif // !__EMSCRIPTEN__ && !QS_NO_SIMD

#if (defined _MSC_VER || defined __clang__) && !defined _M_ARM && !defined _M_ARM64 && !defined __EMSCRIPTEN__
#define QM_VECTORCALL	__vectorcall
#define QM_VECTORDECL	__vectorcall
#elif defined __GNUC__
#define QM_VECTORCALL
#define QM_VECTORDECL
#else
#define QM_VECTORCALL	__fastcall
#define QM_VECTORDECL
#endif

#ifdef QM_USE_SIMD
#ifdef _MSC_VER
#define QM_ALIGN(x)		__declspec(align(x))
#define QM_AUNION(x)	__declspec(align(x)) union
#define QM_ASTRUCT(x)	__declspec(align(x)) struct
#elif defined __GNUC__
#define QM_ALIGN(x)		__attribute__((aligned(x)))
#define QM_AUNION(x)	union __attribute__((aligned(x)))
#define QM_ASTRUCT(x)	struct __attribute__((aligned(x)))
#endif
#else
#define QM_ALIGN(x)
#define QM_AUNION(x)	union
#define QM_ASTRUCT(x)	struct
#endif

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif
#ifdef QM_USE_AVX
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#endif
#ifdef QM_USE_AVX2
#include <immintrin.h>
#endif
#ifdef QM_USE_NEON
#include <arm_neon.h>
#endif

//////////////////////////////////////////////////////////////////////////
// constant & macro

#define QM_L_E			2.7182818284590452353602874713526624977572470937000
#define QM_L_LOG2_E		1.44269504088896340736							
#define QM_L_LOG10_E	0.434294481903251827651							
#define QM_L_LOG2_B10	0.30102999566398119521							
#define QM_L_LN2		0.6931471805599453094172321214581765680755001343603
#define QM_L_LN10		2.3025850929940456840179914546843642076011014886288
#define QM_L_PI			3.1415926535897932384626433832795028841971693993751
#define QM_L_SQRT2		1.4142135623730950488016887242096980785696718753769

#define QM_EPSILON		0.0001f												/// @brief 엡실론
#define QM_PI			3.14159265358979323846f								/// @brief 원주율 (180도)
#define QM_PI2			6.28318530717958647692f								/// @brief 원주율 두배 (360도)
#define QM_PI_H			1.57079632679489661923f								/// @brief 원주울의 반 (90도)
#define QM_PI_Q			0.78539816339744830961f								/// @brief 원주율의 반의 반 (45도)
#define QM_RPI			0.31830988618379067154f								/// @brief 원주율의 역수
#define QM_RPI_H		0.15915494309189533577f								/// @brief 원주율의 역수의 반
#define QM_SQRT2		1.41421356237309504880f								/// @brief 2의 제곱근
#define QM_SQRT_H		0.70710678118654752440f								/// @brief 2의 제곱근의 반
#define QM_TAU			6.28318530717958647692f								/// @brief 타우, 원주율의 두배 (360도)
#define QM_TAU2			12.56637061435917295384f							/// @brief 타우 두배 두배 (720도)
#define QM_TAU_H		3.14159265358979323846f								/// @brief 타우의 반 (180도)
#define QM_TAU_Q		1.57079632679489661923f								/// @brief 타우의 반의 반 (90도)
#define QM_RTAU			0.15915494309189533577f								/// @brief 타우의 역수
#define QM_RTAU_H		0.07957747154594766788f								/// @brief 타우의 두배의 역수
#define QM_DEG_45		0.78539816339744830961f								/// @brief 45도
#define QM_DEG_90		1.57079632679489661923f								/// @brief 90도
#define QM_DEG_180		3.14159265358979323846f								/// @brief 180도
#define QM_DEG_270		4.71238898038468985769f								/// @brief 270도
#define QM_DEG_360		6.28318530717958647692f								/// @brief 360도
#define QM_RAD2DEG		(180.0f/QM_PI)
#define QM_DEG2RAD		(QM_PI/180.0f)

typedef uint16_t		half_t;												/// @brief 16비트 실수형


//////////////////////////////////////////////////////////////////////////
// pack types

/// @brief FLOAT2
typedef struct QMFLOAT2
{
	float X, Y;
} QmFloat2;

/// @brief FLOAT3
typedef struct QMFLOAT3
{
	float X, Y, Z;
} QmFloat3;

/// @brief FLOAT4
typedef struct QMFLOAT4
{
	float X, Y, Z, W;
} QmFloat4;

/// @brief INT2
typedef struct QMINT2
{
	int32_t X, Y;
} QmInt2;

/// @brief INT3
typedef struct QMINT3
{
	int32_t X, Y, Z;
} QmInt3;

/// @brief INT4
typedef struct QMINT4
{
	int32_t X, Y, Z, W;
} QmInt4;

/// @brief UINT2
typedef struct QMUINT2
{
	uint32_t X, Y;
} QmUint2;

/// @brief UINT3
typedef struct QMUINT3
{
	uint32_t X, Y, Z;
} QmUint3;

/// @brief UINT4
typedef struct QMUINT4
{
	uint32_t X, Y, Z, W;
} QmUint4;

/// @brief USHORT2
typedef union QMUSHORT2
{
	struct { uint16_t X, Y; };
	uint32_t v;
} QmUshort2;

/// @brief USHORT4
typedef union QMUSHORT4
{
	struct { uint16_t X, Y, Z, W; };
	uint64_t v;
} QmUshort4;

/// @brief HALF2
typedef union QMHALF2
{
	struct { half_t X, Y; };
	uint32_t v;
} QmHalf2;

/// @brief HALF4
typedef union QMHALF4
{
	struct { half_t X, Y, Z, W; };
	uint64_t v;
} QmHalf4;

/// @brief UINT 4/4/4/4
typedef union QMU4444
{
	struct { uint16_t B : 4, G : 4, R : 4, A : 4; };
	uint16_t v;
} QmU4444;

/// @brief UINT 5/5/5/1
typedef union QMU5551
{
	struct { uint16_t B : 5, G : 5, R : 5, A : 1; };
	uint16_t v;
} QmU5551;

/// @brief UINT 5/6/5
typedef union QMU565
{
	struct { uint16_t B : 5, G : 6, R : 5; };
	uint16_t v;
} QmU565;

/// @brief float 11/11/10
typedef union QMF111110
{
	struct { uint32_t B : 11, G : 11, R : 10; };
	struct { uint32_t Bm : 6, Be : 5, Gm : 6, Ge : 5, Rm : 5, Re : 5; };
	uint32_t v;
} QmF111110;

/// @brief UINT 10/10/10/2
typedef union QMU1010102
{
	struct { uint32_t B : 10, G : 10, R : 10, A : 2; };
	uint32_t v;
} QmU1010102;

/// @brief FLOAT4X4
typedef union QMFLOAT4X4
{
	struct
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;
	};
	float m[4][4];
	float f[16];
} QmFloat4x4;

/// @brief 정렬된 FLOAT3
typedef QM_ASTRUCT(16) QMFLOAT3A
{
	float X, Y, Z;
} QmFloat3A;

/// @brief 정렬된 FLOAT4
typedef QM_ASTRUCT(16) QMFLOAT4A
{
	float X, Y, Z, W;
} QmFloat4A;

/// @brief 정렬된 INT3
typedef QM_ASTRUCT(16) QMINT3A
{
	int32_t X, Y, Z;
} QmInt3A;

/// @brief 정렬된 INT4
typedef QM_ASTRUCT(16) QMINT4A
{
	int32_t X, Y, Z, W;
} QmInt4A;

/// @brief 정렬된 UINT3
typedef QM_ASTRUCT(16) QMUINT3A
{
	uint32_t X, Y, Z;
} QmUint3A;

/// @brief 정렬된 UINT4
typedef QM_ASTRUCT(16) QMUINT4A
{
	uint32_t X, Y, Z, W;
} QmUint4A;

/// @brief 정렬된 FLOAT4X4
typedef QM_AUNION(16) QMFLOAT4X4A
{
	struct
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;
	};
	float m[4][4];
	float f[16];
} QmFloat4x4A;


//////////////////////////////////////////////////////////////////////////
// types

// SIMD 벡터
#if defined QM_USE_AVX
typedef __m128			QMVEC;
typedef __m128i			QMIVEC;
#elif defined QM_USE_NEON
typedef float32x4_t		QMVEC;
typedef int32x4_t		QMIVEC;
#else
typedef QM_AUNION(16) QMVEC_FIU
{
	float f[4];
	int32_t i[4];
	uint32_t u[4];
} QMVEC, QMIVEC;
#endif

// SIMD 행렬
#ifdef QM_USE_SIMD
typedef QM_ASTRUCT(16) QMMAT_SIMD
{
	QMVEC r[4];
} QMMAT;
#else
typedef QM_AUNION(16) QMMAT_FMR
{
	float f[16];
	float m[4][4];
	QMVEC r[4];
	struct
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;
	};
} QMMAT;
#endif

/// @brief 벡터2
typedef union QMVEC2
{
	float f[2];
	struct
	{
		float X, Y;
	};
} QmVec2;

/// @brief 벡터4
typedef QM_AUNION(16) QMVECF
{
	float f[4];
	QMVEC s;
	struct
	{
		float X, Y, Z, W;
	};
	struct
	{
		float R, G, B, A;
	};
} QmVec4, QmVecF;

/// @brief 벡터4
typedef QM_AUNION(16) QMVECU
{
	uint32_t u[4];
	int32_t i[4];
	QMVEC s;
	struct
	{
		uint32_t X, Y, Z, W;
	};
} QmVecU;

/// @brief 행렬4x4
typedef QM_AUNION(16) QMMAT4
{
	float f[16];
	float m[4][4];
	QMVEC r[4];
	QMMAT s;
	struct
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;
	};
} QmMat4;

/// @brief 위치값 (포인트)
typedef union QMPOINT
{
	int32_t i[2];
	struct
	{
		int32_t X, Y;
	};
} QmPoint;

/// @brief 크기
typedef union QMSIZE
{
	int32_t i[2];
	struct
	{
		int32_t Width, Height;
	};
	struct
	{
		int32_t Minimum, Maximum;
	};
} QmSize;

/// @brief 사각형
typedef union QMRECT
{
	int32_t i[4];
	struct
	{
		int32_t Left, Top, Right, Bottom;
	};
	struct
	{
		QmPoint LeftTop;
		QmPoint RightBottom;
	};
} QmRect;

/// @brief 정수형 색깔
typedef union QMKOLOR
{
	uint32_t v;
	struct { uint8_t B, G, R, A; };
} QmKolor;


//////////////////////////////////////////////////////////////////////////
// integer & float

/// @brief 두 정수의 최대값
INLINE int32_t qm_maxi(int32_t a, int32_t b)
{
	return (((a) > (b)) ? (a) : (b));
}

/// @brief 두 정수의 최소값
INLINE int32_t qm_mini(int32_t a, int32_t b)
{
	return (((a) < (b)) ? (a) : (b));
}

/// @brief 정수의 절대값
INLINE int32_t qm_absi(int32_t v)
{
	return (((v) < 0) ? -(v) : (v));
}

/// @brief 정수를 범위 내로 자르기
INLINE int32_t qm_clampi(int32_t v, int32_t min, int32_t max)
{
	return ((v) < (min) ? (min) : (v) > (max) ? (max) : (v));
}

/// @brief Power Of Two 계산
INLINE bool qm_pot(uint32_t v)
{
	return (v & (v - 1)) == 0;
}

/// @brief power of 10
INLINE double qm_pow10(int32_t n)
{
	return exp(n * QM_L_LN10);
}

/// @brief 실수의 앱실론 비교
INLINE bool qm_eqs(float a, float b, float epsilon)
{
	return fabsf(a - b) < epsilon;
}

/// @brief 실수의 엡실론 비교
INLINE bool qm_eqf(float a, float b)
{
	return fabsf(a - b) < QM_EPSILON;
}

/// @brief 두 실수의 최대값
INLINE float qm_maxf(float a, float b)
{
	return (((a) > (b)) ? (a) : (b));
}

/// @brief 두 실수의 최소값
INLINE float qm_minf(float a, float b)
{
	return (((a) < (b)) ? (a) : (b));
}

/// @brief 실수의 절대값
INLINE float qm_absf(float v)
{
	return fabsf(v);
}

/// @brief 실수를 범위 내로 자르기
INLINE float qm_clampf(float v, float min, float max)
{
	return ((v) < (min) ? (min) : (v) > (max) ? (max) : (v));
}

/// @brief 각도를 -180 ~ +180 사이로 자르기
INLINE float qm_cradf(float v)
{
	float f = fabsf(v + QM_PI);
	f -= (QM_PI2 * (float)(int32_t)(f / QM_PI2)) - QM_PI;
	return v < 0.0f ? -f : f;
}

/// @brief 실수의 보간
INLINE float qm_lerpf(float left, float right, float scale)
{
	return left + scale * (right - left);
}

/// @brief 실수에서 실수부만 뽑기
INLINE float qm_fractf(float f)
{
	return f - floorf(f);
}

/// @brief 사인과 코사인을 동시에 계산
INLINE void qm_sincosf(float v, float* s, float* c)
{
#if defined _MSC_VER && defined QM_USE_AVX
	QMVEC i = _mm_set_ss(v);
	QMVEC o = _mm_sincos_ps(&i, i);
	*s = _mm_cvtss_f32(o);
	*c = _mm_cvtss_f32(i);
#else
	*s = sinf(v);
	*c = cosf(v);
#endif
}

/// @brief 제곱근
INLINE float qm_sqrtf(float f)
{
#if defined QM_USE_AVX
	QMVEC i = _mm_set_ss(f);
	QMVEC o = _mm_sqrt_ss(i);
	return _mm_cvtss_f32(o);
#elif defined QM_USE_NEON
	QMVEC i = vdupq_n_f32(f);
	QMVEC o = vsqrtq_f32(f);
	return vgetq_lane_f32(o, 0);
#else
	return sqrtf(f);
#endif
}

/// @brief 1을 나눈 제곱근
INLINE float qm_rsqrtf(float f)
{
#if defined QM_USE_AVX
	QMVEC i = _mm_set_ss(f);
	QMVEC o = _mm_rsqrt_ss(i);
	return _mm_cvtss_f32(o);
#elif defined QM_USE_NEON
	QMVEC i = vdupq_n_f32(f);
	QMVEC o = vrsqrteq_f32(f);
	return vgetq_lane_f32(o, 0);
#else
	return 1.0f / qm_sqrtf(f);
#endif
}

/// @brief power of 10
INLINE float qm_pow10f(int32_t n)
{
	return expf((float)n * (float)QM_L_LN10);
}

/// @brief XMATH 반올림
INLINE float qm_roundf(float f)
{
	const float i = floorf(f);
	f -= i;
	if (f < 0.5f)
		return i;
	if (f > 0.5f)
		return i + 1.0f;
	float int_part;
	(void)modff(i / 2.0f, &int_part);
	if ((2.0f * int_part) == i)
		return i;
	return i + 1.0f;
}

/// @brief 각도를 호도로 변환
INLINE float qm_d2rf(float d)
{
	return d * QM_DEG2RAD;
}

/// @brief 호도를 각도로 변환
INLINE float qm_r2df(float r)
{
	return r * QM_RAD2DEG;
}

/// @brief 32비트 실수를 16비트 실수로 변환한다
/// @param v 32비트 실수
/// @return 변환한 16비트 실수
INLINE half_t qm_f2hf(float v)
{
#if defined QM_USE_AVX2
	const QMVEC r = _mm_set_ss(v);
	const QMIVEC p = _mm_cvtps_ph(r, _MM_FROUND_TO_NEAREST_INT);
	return (half_t)_mm_extract_epi16(p, 0);
#elif defined QM_USE_NEON && (defined _M_ARM64 || defined __aarch64__)
	QMVEC f = vdupq_n_f32(v);
	uint16x4_t r = vcvt_f16_f32(f);
	return vgetq_lane_u16(vreinterpret_u16_f16(r), 0);
#else
	uint32_t u = *(const uint32_t*)&v;
	const uint32_t s = (u & 0x80000000U) >> 16U;
	u = u & 0x7FFFFFFFU;
	uint32_t r;
	if (u > 0x47FFEFFFU)
		r = 0x7FFFU;
	else
	{
		if (u >= 0x38800000U)
			u += 0xC8000000U;
		else
		{
			const uint32_t t = 113U - (u >> 23U);
			u = (0x800000U | (u & 0x7FFFFFU)) >> t;
		}
		r = ((u + 0x0FFFU + ((u >> 13U) & 1U)) >> 13U) & 0x7FFFU;
	}
	return (half_t)(r | s);
#endif
}

/// @brief 16비트 실수를 32비트 실수로 변환한다
/// @param v 변환할 16비트 실수
/// @return 변환한 32비트 실수
INLINE float qm_hf2f(const half_t v)
{
#if defined QM_USE_AVX2
	const QMIVEC p = _mm_cvtsi32_si128((int32_t)v);
	const QMVEC r = _mm_cvtph_ps(p);
	return _mm_cvtss_f32(r);
#elif defined QM_USE_NEON && (defined _M_ARM64 || defined __aarch64__)
	uint16x4_t u = vdup_n_u16(v);
	uint32x4_t r = vcvt_f32_f16(vreinterpret_f16_u16(u));
	return vgetq_lane_f32(r, 0);
#else
	uint32_t m = (uint32_t)(v & 0x03FF);
	uint32_t e;
	if ((v & 0x7C00) != 0)
		e = (uint32_t)((v >> 10) & 0x1F);
	else if (m != 0)
	{
		e = 1;
		do
		{
			e--;
			m <<= 1;
		} while ((m & 0x0400) == 0);
		m &= 0x03FF;
	}
	else
		e = (uint32_t)-112;
	uint32_t r = (uint32_t)((v & 0x8000) << 16) | (uint32_t)((e + 112) << 23) | (uint32_t)(m << 13);
	return *(float*)&r;
#endif
}


//////////////////////////////////////////////////////////////////////////
// function

INLINE QMVEC QM_VECTORCALL qm_vec(float x, float y, float z, float w);
INLINE QMVEC QM_VECTORCALL qm_vec_zero(void);
INLINE QMVEC QM_VECTORCALL qm_vec_one(void);
INLINE QMVEC QM_VECTORCALL qm_vec_sp(float diag);
INLINE QMVEC QM_VECTORCALL qm_vec_sp_x(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_sp_y(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_sp_z(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_sp_w(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_sp_xy(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec_sp_zw(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec_sp_msb(void);
INLINE QMVEC QM_VECTORCALL qm_vec_neg(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_rcp(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_add(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec_sub(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec_mag(const QMVEC left, float right);
INLINE QMVEC QM_VECTORCALL qm_vec_abr(const QMVEC left, float right);
INLINE QMVEC QM_VECTORCALL qm_vec_mul(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec_madd(const QMVEC left, const QMVEC right, const QMVEC add);
INLINE QMVEC QM_VECTORCALL qm_vec_msub(const QMVEC left, const QMVEC right, const QMVEC sub);
INLINE QMVEC QM_VECTORCALL qm_vec_div(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec_min(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec_max(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec_clamp(const QMVEC v, const QMVEC min, const QMVEC max);
INLINE QMVEC QM_VECTORCALL qm_vec_csat(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_crad(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_blend(const QMVEC left, float leftScale, const QMVEC right, float rightScale);
INLINE QMVEC QM_VECTORCALL qm_vec_lerp(const QMVEC left, const QMVEC right, float scale);
INLINE QMVEC QM_VECTORCALL qm_vec_lerp_len(const QMVEC left, const QMVEC right, float scale, float len);
INLINE QMVEC QM_VECTORCALL qm_vec_hermite(const QMVEC pos1, const QMVEC tan1, const QMVEC pos2, const QMVEC tan2, float scale);
INLINE QMVEC QM_VECTORCALL qm_vec_catmullrom(const QMVEC pos1, const QMVEC pos2, const QMVEC pos3, const QMVEC pos4, float scale);
INLINE QMVEC QM_VECTORCALL qm_vec_barycentric(const QMVEC pos1, const QMVEC pos2, const QMVEC pos3, float f, float g);
INLINE float QM_VECTORCALL qm_vec_get_x(const QMVEC v);
INLINE float QM_VECTORCALL qm_vec_get_y(const QMVEC v);
INLINE float QM_VECTORCALL qm_vec_get_z(const QMVEC v);
INLINE float QM_VECTORCALL qm_vec_get_w(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_set_x(const QMVEC v, float x);
INLINE QMVEC QM_VECTORCALL qm_vec_set_y(const QMVEC v, float y);
INLINE QMVEC QM_VECTORCALL qm_vec_set_z(const QMVEC v, float z);
INLINE QMVEC QM_VECTORCALL qm_vec_set_w(const QMVEC v, float w);
INLINE bool QM_VECTORCALL qm_vec_eq(const QMVEC left, const QMVEC right);
INLINE bool QM_VECTORCALL qm_vec_eps(const QMVEC left, const QMVEC right, float epsilon);
INLINE bool QM_VECTORCALL qm_vec_near(const QMVEC left, const QMVEC right);
INLINE void QM_VECTORCALL qm_vec_st_float2(QmFloat2* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_float3(QmFloat3* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_float4(QmFloat4* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_int2(QmInt2* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_int3(QmInt3* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_int4(QmInt4* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_uint2(QmUint2* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_uint3(QmUint3* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_uint4(QmUint4* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_ushort2(QmUshort2* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_ushort4(QmUshort4* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_half2(QmHalf2* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_half4(QmHalf4* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_u4444(QmU4444* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_u565(QmU565* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_u5551(QmU5551* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_f111110(QmF111110* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_u1010102(QmU1010102* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_kolor(QmKolor* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_float3a(QmFloat3A* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_float4a(QmFloat4A* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_ushort2n(QmUshort2* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_st_ushort4n(QmUshort4* p, const QMVEC v);
INLINE void QM_VECTORCALL qm_vec_simd_sincos(const QMVEC v, QMVEC* ret_sin, QMVEC* ret_cos);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_sqrt(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_rsqrt(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_sin(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_cos(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_tan(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_asin(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_acos(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_atan(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_atan2(const QMVEC y, const QMVEC x);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_round(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec_simd_blend(const QMVEC left, const QMVEC left_scale, const QMVEC right, const QMVEC right_scale);

INLINE QmVec2 qm_vec2(float x, float y);
INLINE QmVec2 qm_vec2p(const QmPoint p);
INLINE QmVec2 qm_vec2_zero(void);
INLINE QmVec2 qm_vec2_sp(float diag);
INLINE QmVec2 qm_vec2_neg(const QmVec2 v);
INLINE QmVec2 qm_vec2_rcp(const QmVec2 v);
INLINE QmVec2 qm_vec2_add(const QmVec2 left, const QmVec2 right);
INLINE QmVec2 qm_vec2_sub(const QmVec2 left, const QmVec2 right);
INLINE QmVec2 qm_vec2_mag(const QmVec2 left, float right);
INLINE QmVec2 qm_vec2_abr(const QmVec2 left, float right);
INLINE QmVec2 qm_vec2_mul(const QmVec2 left, const QmVec2 right);
INLINE QmVec2 qm_vec2_div(const QmVec2 left, const QmVec2 right);
INLINE QmVec2 qm_vec2_min(const QmVec2 left, const QmVec2 right);
INLINE QmVec2 qm_vec2_max(const QmVec2 left, const QmVec2 right);
INLINE QmVec2 qm_vec2_norm(const QmVec2 v);
INLINE QmVec2 qm_vec2_cross(const QmVec2 left, const QmVec2 right);
INLINE QmVec2 qm_vec2_lerp(const QmVec2 left, const QmVec2 right, float scale);
INLINE QmVec2 qm_vec2_reflect(const QmVec2 incident, const QmVec2 normal);
INLINE QmVec2 qm_vec2_refract(const QmVec2 incident, const QmVec2 normal, float eta);
INLINE float qm_vec2_dot(const QmVec2 left, const QmVec2 right);
INLINE float qm_vec2_len_sq(const QmVec2 v);
INLINE float qm_vec2_len(const QmVec2 v);
INLINE float qm_vec2_dist_sq(const QmVec2 left, const QmVec2 right);
INLINE float qm_vec2_dist(const QmVec2 left, const QmVec2 right);
INLINE bool qm_vec2_eq(const QmVec2 left, const QmVec2 right);
INLINE bool qm_vec2_eps(const QmVec2 left, const QmVec2 right, float epsilon);
INLINE bool qm_vec2_isz(const QmVec2 v);

INLINE QMVEC QM_VECTORCALL qm_vec3(float x, float y, float z);
INLINE QMVEC QM_VECTORCALL qm_vec3_cross(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec3_norm(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec3_reflect(const QMVEC incident, const QMVEC normal);
INLINE QMVEC QM_VECTORCALL qm_vec3_refract(const QMVEC incident, const QMVEC normal, float eta);
INLINE QMVEC QM_VECTORCALL qm_vec3_dir(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec3_rot(const QMVEC v, const QMVEC rot_quat);
INLINE QMVEC QM_VECTORCALL qm_vec3_rot_inv(const QMVEC v, const QMVEC rot_quat);
INLINE QMVEC QM_VECTORCALL qm_vec3_trfm(const QMVEC v, const QMMAT m);
INLINE QMVEC QM_VECTORCALL qm_vec3_trfm_norm(const QMVEC v, const QMMAT m);
INLINE QMVEC QM_VECTORCALL qm_vec3_trfm_coord(const QMVEC v, const QMMAT m);
INLINE QMVEC QM_VECTORCALL qm_vec3_form_norm(const QMVEC v1, const QMVEC v2, const QMVEC v3);
INLINE QMVEC QM_VECTORCALL qm_vec3_closed(const QMVEC loc, const QMVEC begin, const QMVEC end);
INLINE QMVEC QM_VECTORCALL qm_vec3_proj(const QMVEC v, const QMMAT proj, const QMMAT view, const QMMAT world, float x, float y, float width, float height, float zn, float zf);
INLINE QMVEC QM_VECTORCALL qm_vec3_unproj(const QMVEC v, const QMMAT proj, const QMMAT view, const QMMAT world, float x, float y, float width, float height, float zn, float zf);
INLINE float QM_VECTORCALL qm_vec3_dot(const QMVEC left, const QMVEC right);
INLINE float QM_VECTORCALL qm_vec3_len_sq(const QMVEC v);
INLINE float QM_VECTORCALL qm_vec3_len(const QMVEC v);
INLINE float QM_VECTORCALL qm_vec3_dist_sq(const QMVEC left, const QMVEC right);
INLINE float QM_VECTORCALL qm_vec3_dist(const QMVEC left, const QMVEC right);
INLINE float QM_VECTORCALL qm_vec3_rad_sq(const QMVEC left, const QMVEC right);
INLINE float QM_VECTORCALL qm_vec3_rad(const QMVEC left, const QMVEC right);
INLINE float QM_VECTORCALL qm_vec3_angle(const QMVEC left_normal, const QMVEC right_normal);
INLINE bool QM_VECTORCALL qm_vec3_is_between(const QMVEC query_point, const QMVEC begin, const QMVEC end);
INLINE bool QM_VECTORCALL qm_vec3_eq(const QMVEC left, const QMVEC right);
INLINE bool QM_VECTORCALL qm_vec3_eps(const QMVEC left, const QMVEC right, float epsilon);
INLINE bool QM_VECTORCALL qm_vec3_near(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec3_simd_dot(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec3_simd_len_sq(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec3_simd_len(const QMVEC v);

INLINE QMVEC QM_VECTORCALL qm_vec4(float x, float y, float z, float w);
INLINE QMVEC QM_VECTORCALL qm_vec4v(const QMVEC v, float w);
INLINE QMVEC QM_VECTORCALL qm_vec4_cross(const QMVEC a, const QMVEC b, const QMVEC c);
INLINE QMVEC QM_VECTORCALL qm_vec4_norm(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec4_reflect(const QMVEC incident, const QMVEC normal);
INLINE QMVEC QM_VECTORCALL qm_vec4_refract(const QMVEC incident, const QMVEC normal, float eta);
INLINE QMVEC QM_VECTORCALL qm_vec4_trfm(const QMVEC v, const QMMAT m);
INLINE float QM_VECTORCALL qm_vec4_dot(const QMVEC left, const QMVEC right);
INLINE float QM_VECTORCALL qm_vec4_len_sq(const QMVEC v);
INLINE float QM_VECTORCALL qm_vec4_len(const QMVEC v);
INLINE float QM_VECTORCALL qm_vec4_dist_sq(const QMVEC left, const QMVEC right);
INLINE float QM_VECTORCALL qm_vec4_dist(const QMVEC left, const QMVEC right);
INLINE bool QM_VECTORCALL qm_vec4_eq(const QMVEC left, const QMVEC right);
INLINE bool QM_VECTORCALL qm_vec4_eps(const QMVEC left, const QMVEC right, float epsilon);
INLINE QMVEC QM_VECTORCALL qm_vec4_simd_dot(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_vec4_simd_len_sq(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_vec4_simd_len(const QMVEC v);

INLINE QMVEC QM_VECTORCALL qm_quat(float x, float y, float z, float w);
INLINE QMVEC QM_VECTORCALL qm_quatv(const QMVEC v, float w);
INLINE QMVEC QM_VECTORCALL qm_quat_unit(void);
INLINE QMVEC QM_VECTORCALL qm_quat_mul(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_quat_norm(const QMVEC q);
INLINE QMVEC QM_VECTORCALL qm_quat_cjg(const QMVEC q);
INLINE QMVEC QM_VECTORCALL qm_quat_inv(const QMVEC q);
INLINE QMVEC QM_VECTORCALL qm_quat_exp(const QMVEC q);
INLINE QMVEC QM_VECTORCALL qm_quat_ln(const QMVEC q);
INLINE QMVEC QM_VECTORCALL qm_quat_slerp(const QMVEC left, const QMVEC right, float scale);
INLINE QMVEC QM_VECTORCALL qm_quat_slerp_dxm(const QMVEC Q0, const QMVEC Q1, float scale);
INLINE QMVEC QM_VECTORCALL qm_quat_squad(const QMVEC q1, const QMVEC q2, const QMVEC q3, const QMVEC q4, float scale);
INLINE QMVEC QM_VECTORCALL qm_quat_barycentric(const QMVEC q1, const QMVEC q2, const QMVEC q3, float f, float g);
INLINE QMVEC QM_VECTORCALL qm_quat_rot_vec(const QMVEC rot3);
INLINE QMVEC QM_VECTORCALL qm_quat_rot_axis(const QMVEC axis3, float angle);
INLINE QMVEC QM_VECTORCALL qm_quat_rot_x(float rot);
INLINE QMVEC QM_VECTORCALL qm_quat_rot_y(float rot);
INLINE QMVEC QM_VECTORCALL qm_quat_rot_z(float rot);
INLINE QMVEC QM_VECTORCALL qm_quat_rot_mat4(const QMMAT rot);
INLINE QMVEC QM_VECTORCALL qm_quat_angle(const QMVEC v1, const QMVEC v2);
INLINE float QM_VECTORCALL qm_quat_dot(const QMVEC left, const QMVEC right);
INLINE void QM_VECTORCALL qm_quat_to_axis_angle(const QMVEC q, QMVEC* axis, float* angle);
INLINE bool QM_VECTORCALL qm_quat_isu(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_quat_simd_dot(const QMVEC left, const QMVEC right);

INLINE QMVEC QM_VECTORCALL qm_plane(float a, float b, float c, float d);
INLINE QMVEC QM_VECTORCALL qm_planev(const QMVEC v, float d);
INLINE QMVEC QM_VECTORCALL qm_planevv(const QMVEC v, const QMVEC normal);
INLINE QMVEC QM_VECTORCALL qm_planevvv(const QMVEC v1, const QMVEC v2, const QMVEC v3);
INLINE QMVEC QM_VECTORCALL qm_plane_unit(void);
INLINE QMVEC QM_VECTORCALL qm_plane_norm(const QMVEC plane);
INLINE QMVEC QM_VECTORCALL qm_plane_rnorm(const QMVEC plane);
INLINE QMVEC QM_VECTORCALL qm_plane_trfm(const QMVEC plane, const QMMAT m);
INLINE float QM_VECTORCALL qm_plane_dot(const QMVEC plane, const QMVEC v);
INLINE float QM_VECTORCALL qm_plane_dot_coord(const QMVEC plane, const QMVEC v);
INLINE float QM_VECTORCALL qm_plane_dot_normal(const QMVEC plane, const QMVEC v);
INLINE float QM_VECTORCALL qm_plane_dist_vec3(const QMVEC plane, const QMVEC v);
INLINE float QM_VECTORCALL qm_plane_dist_line(const QMVEC plane, const QMVEC line_point1, const QMVEC line_point2);
INLINE int32_t QM_VECTORCALL qm_plane_relation(const QMVEC plane, const QMVEC v);
INLINE bool QM_VECTORCALL qm_plane_intersect_line(const QMVEC plane, const QMVEC line_point1, const QMVEC line_point2, QMVEC* intersect_point);
INLINE bool QM_VECTORCALL qm_plane_intersect_line2(const QMVEC plane, const QMVEC loc, const QMVEC dir, QMVEC* intersect_point);
INLINE bool QM_VECTORCALL qm_plane_intersect_plane(const QMVEC plane1, const QMVEC plane2, QMVEC* loc, QMVEC* dir);
INLINE bool QM_VECTORCALL qm_plane_intersect_planes(const QMVEC plane1, const QMVEC plane2, const QMVEC plane3, QMVEC* intersect_point);
INLINE bool QM_VECTORCALL qm_plane_eq(const QMVEC left, const QMVEC right);
INLINE bool QM_VECTORCALL qm_plane_eps(const QMVEC left, const QMVEC right, float epsilon);
INLINE bool QM_VECTORCALL qm_plane_isu(const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_plane_simd_dot(const QMVEC plane, const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_plane_simd_dot_coord(const QMVEC plane, const QMVEC v);
INLINE QMVEC QM_VECTORCALL qm_plane_simd_dot_normal(const QMVEC plane, const QMVEC v);

INLINE QMVEC QM_VECTORCALL qm_color(float r, float g, float b, float a);
INLINE QMVEC QM_VECTORCALL qm_coloru(uint32_t value);
INLINE QMVEC QM_VECTORCALL qm_colork(QmKolor k);
INLINE QMVEC QM_VECTORCALL qm_color_unit(void);
INLINE QMVEC QM_VECTORCALL qm_color_sp(float value, float alpha);
INLINE QMVEC QM_VECTORCALL qm_color_neg(const QMVEC c);
INLINE QMVEC QM_VECTORCALL qm_color_mod(const QMVEC left, const QMVEC right);
INLINE QMVEC QM_VECTORCALL qm_color_contrast(const QMVEC c, float contrast);
INLINE QMVEC QM_VECTORCALL qm_color_saturate(const QMVEC c, float saturation);

INLINE QMMAT QM_VECTORCALL qm_mat4_unit(void);
INLINE QMMAT QM_VECTORCALL qm_mat4_zero(void);
INLINE QMMAT QM_VECTORCALL qm_mat4_diag(float diag);
INLINE QMMAT QM_VECTORCALL qm_mat4_add(const QMMAT left, const QMMAT right);
INLINE QMMAT QM_VECTORCALL qm_mat4_sub(const QMMAT left, const QMMAT right);
INLINE QMMAT QM_VECTORCALL qm_mat4_mag(const QMMAT left, float right);
INLINE QMMAT QM_VECTORCALL qm_mat4_abr(const QMMAT left, float right);
INLINE QMMAT QM_VECTORCALL qm_mat4_tran(const QMMAT m);
INLINE QMMAT QM_VECTORCALL qm_mat4_mul(const QMMAT left, const QMMAT right);
INLINE QMMAT QM_VECTORCALL qm_mat4_tmul(const QMMAT left, const QMMAT right);
INLINE QMMAT QM_VECTORCALL qm_mat4_inv(const QMMAT m);
INLINE QMMAT QM_VECTORCALL qm_mat4_scl(float x, float y, float z);
INLINE QMMAT QM_VECTORCALL qm_mat4_scl_vec3(const QMVEC v);
INLINE QMMAT QM_VECTORCALL qm_mat4_loc(float x, float y, float z);
INLINE QMMAT QM_VECTORCALL qm_mat4_loc_vec(const QMVEC v);
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_rot(const QMVEC axis, float angle);
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_vec3(const QMVEC rot);
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_quat(const QMVEC rot);
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_x(float rot);
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_y(float rot);
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_z(float rot);
INLINE QMMAT QM_VECTORCALL qm_mat4_affine(const QMVEC scl, const QMVEC rotcenter, const QMVEC rot, const QMVEC loc);
INLINE QMMAT QM_VECTORCALL qm_mat4_trfm(const QMVEC loc, const QMVEC rot, const QMVEC scl);
INLINE QMMAT QM_VECTORCALL qm_mat4_trfm_vec(const QMVEC loc, const QMVEC rot, const QMVEC scl);
INLINE QMMAT QM_VECTORCALL qm_mat4_reflect(const QMVEC plane);
INLINE QMMAT QM_VECTORCALL qm_mat4_shadow(const QMVEC plane, const QMVEC light);
INLINE QMMAT QM_VECTORCALL qm_mat4_internal_look_to(const QMVEC eye, const QMVEC dir, const QMVEC up);
INLINE QMMAT QM_VECTORCALL qm_mat4_lookat_lh(QMVEC eye, QMVEC at, QMVEC up);
INLINE QMMAT QM_VECTORCALL qm_mat4_lookat_rh(QMVEC eye, QMVEC at, QMVEC up);
INLINE QMMAT QM_VECTORCALL qm_mat4_perspective_lh(float fov, float aspect, float zn, float zf);
INLINE QMMAT QM_VECTORCALL qm_mat4_perspective_rh(float fov, float aspect, float zn, float zf);
INLINE QMMAT QM_VECTORCALL qm_mat4_ortho_lh(float width, float height, float zn, float zf);
INLINE QMMAT QM_VECTORCALL qm_mat4_ortho_rh(float width, float height, float zn, float zf);
INLINE QMMAT QM_VECTORCALL qm_mat4_ortho_offcenter_lh(float left, float top, float right, float bottom, float zn, float zf);
INLINE QMMAT QM_VECTORCALL qm_mat4_ortho_offcenter_rh(float left, float top, float right, float bottom, float zn, float zf);
INLINE QMMAT QM_VECTORCALL qm_mat4_viewport(float x, float y, float width, float height);
INLINE bool QM_VECTORCALL qm_mat4_isu(QMMAT m);

INLINE QmPoint qm_point(int32_t x, int32_t y);
INLINE QmPoint qm_pointv(const QmVec2 v);
INLINE QmPoint qm_pointv4(const QMVEC v);
INLINE QmPoint qm_point_zero(void);
INLINE QmPoint qm_point_sp(const int32_t diag);
INLINE QmPoint qm_point_neg(const QmPoint p);
INLINE QmPoint qm_point_add(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_sub(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_mag(const QmPoint left, int32_t right);
INLINE QmPoint qm_point_abr(const QmPoint left, int32_t right);
INLINE QmPoint qm_point_mul(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_div(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_min(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_max(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_cross(const QmPoint left, const QmPoint right);
INLINE int32_t qm_point_dot(const QmPoint left, const QmPoint right);
INLINE int32_t qm_point_len_sq(const QmPoint pt);
INLINE int32_t qm_point_dist_sq(const QmPoint left, const QmPoint right);
INLINE float qm_point_len(const QmPoint pt);
INLINE float qm_point_dist(const QmPoint left, const QmPoint right);
INLINE bool qm_point_eq(const QmPoint left, const QmPoint right);
INLINE bool qm_point_isz(const QmPoint pt);

INLINE QmSize qm_size(int32_t width, int32_t height);
INLINE QmSize qm_size_rect(const QmRect rt);
INLINE QmSize qm_size_sp(const int32_t diag);
INLINE QmSize qm_size_add(const QmSize left, const QmSize right);
INLINE QmSize qm_size_sub(const QmSize left, const QmSize right);
INLINE QmSize qm_size_mag(const QmSize left, int32_t right);
INLINE QmSize qm_size_abr(const QmSize left, int32_t right);
INLINE QmSize qm_size_mul(const QmSize left, const QmSize right);
INLINE QmSize qm_size_div(const QmSize left, const QmSize right);
INLINE QmSize qm_size_min(const QmSize left, const QmSize right);
INLINE QmSize qm_size_max(const QmSize left, const QmSize right);
INLINE int32_t qm_size_len_sq(const QmSize s);
INLINE float qm_size_len(const QmSize v);
INLINE float qm_size_get_aspect(const QmSize s);
INLINE QmPoint qm_size_locate_center(const QmSize s, int32_t width, int32_t height);
INLINE float qm_size_calc_dpi(const QmSize pt, float horizontal, float vertical);
INLINE bool qm_size_eq(const QmSize left, const QmSize right);

INLINE QmRect qm_rect(int32_t left, int32_t top, int32_t right, int32_t bottom);
INLINE QmRect qm_rect_size(int32_t x, int32_t y, int32_t width, int32_t height);
INLINE QmRect qm_rect_pos_size(QmPoint pos, QmSize size);
INLINE QmRect qm_rect_zero(void);
INLINE QmRect qm_rect_sp(const int32_t diag);
INLINE QmRect qm_rect_add(const QmRect left, const QmRect right);
INLINE QmRect qm_rect_sub(const QmRect left, const QmRect right);
INLINE QmRect qm_rect_mag(const QmRect left, int32_t right);
INLINE QmRect qm_rect_abr(const QmRect left, int32_t right);
INLINE QmRect qm_rect_min(const QmRect left, const QmRect right);
INLINE QmRect qm_rect_max(const QmRect left, const QmRect right);
INLINE QmRect qm_rect_inflate(const QmRect rt, int32_t left, int32_t top, int32_t right, int32_t bottom);
INLINE QmRect qm_rect_deflate(const QmRect rt, int32_t left, int32_t top, int32_t right, int32_t bottom);
INLINE QmRect qm_rect_offset(const QmRect rt, int32_t left, int32_t top, int32_t right, int32_t bottom);
INLINE QmRect qm_rect_move(const QmRect rt, int32_t left, int32_t top);
INLINE QmRect qm_rect_set_size(const QmRect rt, int32_t width, int32_t height);
INLINE int32_t qm_rect_get_width(const QmRect rt);
INLINE int32_t qm_rect_get_height(const QmRect rt);
INLINE bool qm_rect_in(const QmRect rt, const int32_t x, const int32_t y);
INLINE bool qm_rect_include(const QmRect dest, const QmRect target);
INLINE bool qm_rect_intersect(const QmRect r1, const QmRect r2, QmRect* p);
INLINE bool qm_rect_eq(const QmRect left, const QmRect right);
INLINE bool qm_rect_isz(const QmRect pv);
INLINE void qm_rect_rotate(QmRect rt, float angle, QmVec2* tl, QmVec2* tr, QmVec2* bl, QmVec2* br);
#ifdef _WINDEF_
INLINE QmRect qm_rect_win_rect(RECT rt);
INLINE RECT qm_rect_to_win_rect(const QmRect rt);
#endif


//////////////////////////////////////////////////////////////////////////
// Color value

QN_CONST_ANY QmKolor QMKOLOR_EMPTY = { 0x00000000 };
QN_CONST_ANY QmKolor QMKOLOR_BLACK = { 0xFF000000 };
QN_CONST_ANY QmKolor QMKOLOR_WHITE = { 0xFFFFFFFF };
QN_CONST_ANY QmKolor QMKOLOR_RED = { 0xFFFF0000 };
QN_CONST_ANY QmKolor QMKOLOR_GREEN = { 0xFF00FF00 };
QN_CONST_ANY QmKolor QMKOLOR_BLUE = { 0xFF0000FF };
QN_CONST_ANY QmKolor QMKOLOR_YELLOW = { 0xFFFFFF00 };
QN_CONST_ANY QmKolor QMKOLOR_CYAN = { 0xFF00FFFF };
QN_CONST_ANY QmKolor QMKOLOR_MAGENTA = { 0xFFFF00FF };
QN_CONST_ANY QmKolor QMKOLOR_GRAY = { 0xFF808080 };
QN_CONST_ANY QmKolor QMKOLOR_DARKGRAY = { 0xFF404040 };
QN_CONST_ANY QmKolor QMKOLOR_LIGHTGRAY = { 0xFFC0C0C0 };
QN_CONST_ANY QmKolor QMKOLOR_HALF_RED = { 0x7F000000 };
QN_CONST_ANY QmKolor QMKOLOR_HALF_GREEN = { 0x7F0000FF };
QN_CONST_ANY QmKolor QMKOLOR_HALF_BLUE = { 0x7F00FF00 };
QN_CONST_ANY QmKolor QMKOLOR_HALF_YELLOW = { 0x7FFF0000 };
QN_CONST_ANY QmKolor QMKOLOR_HALF_CYAN = { 0x7FFFFF00 };
QN_CONST_ANY QmKolor QMKOLOR_HALF_MAGENTA = { 0x7F00FFFF };

QN_CONST_ANY QmVec4 QMCONST_EMPTY = { { 0.0f, 0.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_BLACK = { { 0.0f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_WHITE = { { 1.0f, 1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_RED = { { 1.0f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_GREEN = { { 0.0f, 1.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_BLUE = { { 0.0f, 0.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_YELLOW = { { 1.0f, 0.92f, 0.016f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_CYAN = { { 0.0f, 1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_MAGENTA = { { 1.0f, 0.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_GRAY = { { 0.5f, 0.5f, 0.5f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_DARKGRAY = { { 0.25f, 0.25f, 0.25f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_LIGHTGRAY = { { 0.75f, 0.75f, 0.75f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_HALF_RED = { { 0.5f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_HALF_GREEN = { { 0.0f, 0.5f, 0.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_HALF_BLUE = { { 0.0f, 0.0f, 0.5f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_HALF_YELLOW = { { 0.5f, 0.496f, 0.01f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_HALF_CYAN = { { 0.0f, 0.5f, 0.5f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_HALF_MAGENTA = { { 0.5f, 0.0f, 0.5f, 1.0f } };


//////////////////////////////////////////////////////////////////////////
// SIMD support

QN_CONST_ANY QmVec4 QMCONST_ZERO = { { 0.0f, 0.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_ONE = { { 1.0f, 1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_HALF = { { 0.5f, 0.5f, 0.5f, 0.5f } };
QN_CONST_ANY QmVec4 QMCONST_NEG = { { -1.0f, -1.0f, -1.0f, -1.0f } };
QN_CONST_ANY QmVec4 QMCONST_UNIT_R0 = { { 1.0f, 0.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_UNIT_R1 = { { 0.0f, 1.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_UNIT_R2 = { { 0.0f, 0.0f, 1.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_UNIT_R3 = { { 0.0f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_XY00 = { { 1.0f, 1.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_00ZW = { { 0.0f, 0.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_EPSILON = { { QM_EPSILON, QM_EPSILON, QM_EPSILON, QM_EPSILON } };	// FLT_EPSILON
QN_CONST_ANY QmVec4 QMCONST_MAX_INT = { { 65536.0f * 32768.0f - 128.0f, 65536.0f * 32768.0f - 128.0f, 65536.0f * 32768.0f - 128.0f, 65536.0f * 32768.0f - 128.0f } };
QN_CONST_ANY QmVec4 QMCONST_MAX_UINT = { { 65536.0f * 65536.0f - 256.0f, 65536.0f * 65536.0f - 256.0f, 65536.0f * 65536.0f - 256.0f, 65536.0f * 65536.0f - 256.0f } };
QN_CONST_ANY QmVec4 QMCONST_MAX_USHORT = { { 65535.0f, 65535.0f, 65535.0f, 65535.0f } };
QN_CONST_ANY QmVec4 QMCONST_MAX_UBYTE = { { 255.0f, 255.0f, 255.0f, 255.0f } };
QN_CONST_ANY QmVec4 QMCONST_UINT_SIGN_FIX = { { 32768.0f * 65536.0f, 32768.0f * 65536.0f, 32768.0f * 65536.0f, 32768.0f * 65536.0f } };
QN_CONST_ANY QmVecU QMCONST_S1000 = { { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 } };
QN_CONST_ANY QmVecU QMCONST_S0100 = { { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 } };
QN_CONST_ANY QmVecU QMCONST_S0010 = { { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 } };
QN_CONST_ANY QmVecU QMCONST_S0001 = { { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF } };
QN_CONST_ANY QmVecU QMCONST_S0111 = { { 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } };
QN_CONST_ANY QmVecU QMCONST_S1100 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 } };
QN_CONST_ANY QmVecU QMCONST_S1101 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF } };
QN_CONST_ANY QmVecU QMCONST_S1110 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } };
QN_CONST_ANY QmVecU QMCONST_S1011 = { { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF } };
QN_CONST_ANY QmVecU QMCONST_MSB = { { 0x80000000, 0x80000000, 0x80000000, 0x80000000 } };
QN_CONST_ANY QmVecU QMCONST_MASK_ABS = { { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF } };

#if defined QM_USE_SIMD && !defined QM_USE_NEON
#ifdef QM_USE_AVX
#define _MM_PERMUTE_PS(v, c)	_mm_permute_ps((v), (c))
#else
#define _MM_PERMUTE_PS(v, c)	_mm_shuffle_ps((v), (v), (c))
#endif
#ifdef QM_USE_AVX2
#define _MM_FMADD_PS(a,b,c)		_mm_fmadd_ps((a),(b),(c))
#define _MM_FNMADD_PS(a,b,c)	_mm_fnmadd_ps((a),(b),(c))
#else
#define _MM_FMADD_PS(a,b,c)		_mm_add_ps(_mm_mul_ps((a),(b)),(c))
#define _MM_FNMADD_PS(a,b,c)	_mm_sub_ps(c,_mm_mul_ps((a),(b)))
#endif
#endif


//////////////////////////////////////////////////////////////////////////
// vector op

//
INLINE QMVEC QM_VECTORCALL qm_vec_select(const QMVEC left, const QMVEC right, const QMVEC control)
{
#if defined QM_USE_AVX
	QMVEC t1 = _mm_andnot_ps(control, left);
	QMVEC t2 = _mm_and_ps(right, control);
	return _mm_or_ps(t1, t2);
#elif defined QM_USE_NEON
	return vbslq_f32(vreinterpretq_u32_f32(control), right, left);
#else
	QMVEC v;
	v.u[0] = (left.u[0] & ~control.u[0]) | (right.u[0] & control.u[0]);	
	v.u[1] = (left.u[1] & ~control.u[1]) | (right.u[1] & control.u[1]);
	v.u[2] = (left.u[2] & ~control.u[2]) | (right.u[2] & control.u[2]);
	v.u[3] = (left.u[3] & ~control.u[3]) | (right.u[3] & control.u[3]);
	return v;
#endif
}

//
INLINE QMVEC QM_VECTORCALL qm_vec_select_ctrl(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3)
{
#if defined QM_USE_AVX
	QMIVEC l = _mm_set_epi32((int32_t)i3, (int32_t)i2, (int32_t)i1, (int32_t)i0);
	QMIVEC r = _mm_castps_si128(QMCONST_ZERO.s);
	return _mm_castsi128_ps(_mm_cmpgt_epi32(l, r));
#else
	static const uint32_t e[2] = { 0x00000000, 0xFFFFFFFF };
	const QmVecU v = { { e[i0], e[i1], e[i2], e[i3] } };
	return v.s;
#endif
}

//
INLINE QMVEC QM_VECTORCALL qm_vec_swiz(const QMVEC a, uint32_t e0, uint32_t e1, uint32_t e2, uint32_t e3)
{
#if defined QM_USE_AVX
	uint32_t e[4] = { e0, e1, e2, e3 };
	QMIVEC o = _mm_loadu_si128((const QMIVEC*)e);
	return _mm_permutevar_ps(a, o);
#elif defined QM_USE_NEON
	static const uint32_t ce[4] = { 0x03020100, 0x07060504, 0x0B0A0908, 0x0F0E0D0C };
	uint8x8x2_t t = { .val[0] = vreinterpret_u8_u32(vget_low_u32(a)), .val[1] = vreinterpret_u8_u32(vget_high_u32(a)) };
	uint32x2_t il = vcreate_u32((uint64_t)ce[e0] | ((uint64_t)ce[e1] << 32));
	uint8x8_t lo = vtbl2_u8(t, vreinterpret_u8_u32(il));
	uint32x2_t ih = vcreate_u32((uint64_t)ce[e2] | ((uint64_t)ce[e3] << 32));
	uint8x8_t hi = vtbl2_u8(t, vreinterpret_u8_u32(ih));
	return vcombine_f32(vreinterpret_f32_u8(lo), vreinterpret_f32_u8(hi));
#else
	return (QMVEC) { { a.f[e0], a.f[e1], a.f[e2], a.f[e3] } };
#endif
}

//
INLINE QMVEC QM_VECTORCALL qm_vec_pmt(const QMVEC a, const QMVEC b, uint32_t px, uint32_t py, uint32_t pz, uint32_t pw)
{
#if defined QM_USE_AVX
	static const QmVecU u3 = { { 3, 3, 3, 3 } };
	ALIGNOF(16) uint32_t e[4] = { px, py, pz, pw };
	QMIVEC o = _mm_loadu_si128((const QMIVEC*)e);
	QMIVEC s = _mm_cmpgt_epi32(o, _mm_castps_si128(u3.s));
	o = _mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(o), u3.s));
	QMVEC u = _mm_permutevar_ps(a, o);
	QMVEC v = _mm_permutevar_ps(b, o);
	u = _mm_andnot_ps(_mm_castsi128_ps(s), u);
	v = _mm_and_ps(_mm_castsi128_ps(s), v);
	return _mm_or_ps(u, v);
#elif defined QM_USE_NEON
	static const uint32_t ce[8] = { 0x03020100, 0x07060504, 0x0B0A0908, 0x0F0E0D0C, 0x13121110, 0x17161514, 0x1B1A1918, 0x1F1E1D1C };
	uint8x8x4_t t = {
		.val[0] = vreinterpret_u8_u32(vget_low_u32(a)),
		.val[1] = vreinterpret_u8_u32(vget_high_u32(a)),
		.val[2] = vreinterpret_u8_u32(vget_low_u32(b)),
		.val[3] = vreinterpret_u8_u32(vget_high_u32(b))
	};
	uint32x2_t il = vcreate_u32((uint64_t)ce[px] | ((uint64_t)ce[py] << 32));
	uint8x8_t lo = vtbl4_u8(t, vreinterpret_u8_u32(il));
	uint32x2_t ih = vcreate_u32((uint64_t)ce[pz] | ((uint64_t)ce[pw] << 32));
	uint8x8_t hi = vtbl4_u8(t, vreinterpret_u8_u32(ih));
	return vcombine_f32(vreinterpret_f32_u8(lo), vreinterpret_f32_u8(hi));
#else
	const uint32_t* ab[2] = { a.u, b.u };
	QmVecU v = { { ab[px >> 2][px & 3], ab[py >> 2][py & 3], ab[pz >> 2][pz & 3], ab[pw >> 2][pw & 3] } };
	return v.s;
#endif
}

#ifdef QM_USE_AVX
#define qm_vec_pmt_mask(ret,a,b,mask,s)\
	do{\
		QMVEC s1 = _MM_PERMUTE_PS(a, s);\
		QMVEC s2 = _MM_PERMUTE_PS(b, s);\
		QMVEC m1 = _mm_andnot_ps(mask, s1);\
		QMVEC m2 = _mm_and_ps(mask, s2);\
		ret = _mm_or_ps(m1, m2);\
	} while (0)
#define qm_vec_pmt_anao(ret,a,b,s,hx,hy,hz,hw)\
	do{\
		if (!hx && !hy && !hz && !hw)		/*false,false,false,false*/\
			ret = _MM_PERMUTE_PS(a, s);\
		else if (hx && hy && hz && hw)		/*true, true, true, true*/\
			ret = _MM_PERMUTE_PS(b, s);\
		else if (!hx && !hy && hz && hw)	/*false,false,true, true*/\
			ret = _mm_shuffle_ps(a, b, s);\
		else if (hx && hy && !hz && !hw)	/*true, true,false,false*/\
			ret = _mm_shuffle_ps(a, b, s);\
		else {\
			const QmVecU mask = { { hx ? 0xFFFFFFFF : 0, hy ? 0xFFFFFFFF : 0, hz ? 0xFFFFFFFF : 0, hw ? 0xFFFFFFFF : 0 } };\
			qm_vec_pmt_mask(ret, a, b, mask.s, s);\
		}\
	} while (0)
#endif

//
INLINE QMVEC QM_VECTORCALL qm_vec_bit_shl(const QMVEC a, const QMVEC b, uint32_t e)
{
	return qm_vec_pmt(a, b, e + 0, e + 1, e + 2, e + 3);
}

//
INLINE QMVEC QM_VECTORCALL qm_vec_bit_rol(const QMVEC a, uint32_t e)
{
	return qm_vec_swiz(a, e & 3, (e + 1) & 3, (e + 2) & 3, (e + 3) & 3);
}

//
INLINE QMVEC QM_VECTORCALL qm_vec_bit_ror(const QMVEC a, uint32_t e)
{
	return qm_vec_swiz(a, (4 - e) & 3, (5 - e) & 3, (6 - e) & 3, (7 - e) & 3);
}

//
INLINE QMVEC QM_VECTORCALL qm_vec_bit_xor(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	QMIVEC v = _mm_xor_si128(_mm_castps_si128(left), _mm_castps_si128(right));
	return _mm_castsi128_ps(v);
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(left), vreinterpretq_u32_f32(right)));
#else
	QMVEC v;
	v.u[0] = left.u[0] ^ right.u[0];
	v.u[1] = left.u[1] ^ right.u[1];
	v.u[2] = left.u[2] ^ right.u[2];
	v.u[3] = left.u[3] ^ right.u[3];
	return v;
#endif
}

#if defined QM_USE_AVX
// 벡터 연산자 만들기
#define QM_VEC_OPERATOR(name,avx,neon,op) \
INLINE QMVEC QM_VECTORCALL qm_vec_op_##name(const QMVEC left, const QMVEC right) \
	{ return avx(left, right); }
#define QM_VEC_OPERATOR2(name,avx,neon1,neon2,op) QM_VEC_OPERATOR(name,avx,,)
#elif defined QM_USE_NEON
#define QM_VEC_OPERATOR(name,avx,neon,op) \
INLINE QMVEC QM_VECTORCALL qm_vec_op_##name(const QMVEC left, const QMVEC right) \
	{ return vreinterpretq_f32_u32(neon(left, right)); }
#define QM_VEC_OPERATOR2(name,avx,neon1,neon2,op)  \
INLINE QMVEC QM_VECTORCALL qm_vec_op_##name(const QMVEC left, const QMVEC right) \
	{ return vreinterpretq_f32_u32(neon1(neon2(left, right))); }
#else
#define QM_VEC_OPERATOR(name,avx,neon,op) \
INLINE QMVEC QM_VECTORCALL qm_vec_op_##name(const QMVEC left, const QMVEC right) \
	{ QmVecU v = { { left.f[0] op right.f[0] ? 0xFFFFFFFFU : 0, left.f[1] op right.f[1] ? 0xFFFFFFFFU : 0, left.f[2] op right.f[2] ? 0xFFFFFFFFU : 0, left.f[3] op right.f[3] ? 0xFFFFFFFFU : 0 } }; return v.s; }
#define QM_VEC_OPERATOR2(name,avx,neon1,neon2,op) QM_VEC_OPERATOR(name,,,op)
#endif

QM_VEC_OPERATOR(eq, _mm_cmpeq_ps, vceqq_f32, == );
QM_VEC_OPERATOR2(neq, _mm_cmpneq_ps, vmvnq_u32, vceqq_f32, != );
QM_VEC_OPERATOR(lt, _mm_cmplt_ps, vcltq_f32, < );
QM_VEC_OPERATOR(leq, _mm_cmple_ps, vcleq_f32, <= );
QM_VEC_OPERATOR(gt, _mm_cmpgt_ps, vcgtq_f32, > );
QM_VEC_OPERATOR(geq, _mm_cmpge_ps, vcgeq_f32, >= );

/// @brief 벡터 연산자 입실론 비교
INLINE QMVEC QM_VECTORCALL qm_vec_op_eps(const QMVEC left, const QMVEC right, QMVEC epsilon)
{
#if defined QM_USE_AVX
	QMVEC v = _mm_sub_ps(left, right);
	QMVEC h = _mm_setzero_ps();
	h = _mm_sub_ps(h, v);
	h = _mm_max_ps(h, v);
	return _mm_cmple_ps(h, epsilon);
#elif defined QM_USE_NEON
	QMVEC v = vsubq_f32(left, right);
	return = vreinterpretq_f32_u32(vcleq_f32(vabsq_f32(v), epsilon));
#else
	const float x = qm_absf(left.f[0] - right.f[0]);
	const float y = qm_absf(left.f[1] - right.f[1]);
	const float z = qm_absf(left.f[2] - right.f[2]);
	const float w = qm_absf(left.f[3] - right.f[3]);
	QMVEC u;
	u.u[0] = x <= epsilon.f[0] ? 0xFFFFFFFFU : 0;
	u.u[1] = y <= epsilon.f[1] ? 0xFFFFFFFFU : 0;
	u.u[2] = z <= epsilon.f[2] ? 0xFFFFFFFFU : 0;
	u.u[3] = w <= epsilon.f[3] ? 0xFFFFFFFFU : 0;
	return u;
#endif
}

/// @brief 벡터 연산 바운드
INLINE QMVEC QM_VECTORCALL qm_vec_op_in_bound(const QMVEC v, const QMVEC bound)
{
#if defined QM_USE_AVX
	QMVEC t1 = _mm_cmple_ps(v, bound);
	QMVEC t2 = _mm_mul_ps(bound, QMCONST_NEG.s);
	QMVEC t3 = _mm_cmple_ps(t2, v);
	return _mm_and_ps(t1, t3);
#elif defined QM_USE_NEON
	QMVEC t1 = vcleq_f32(v, bound);
	QMVEC t2 = vreinterpretq_u32_f32(vnegq_f32(bound));
	QMVEC t3 = vcleq_f32(vreinterpretq_f32_u32(t2), v);
	return vreinterpretq_f32_u32(vandq_u32(t1, t3));
#else
	QMVEC u;
	u.u[0] = (v.f[0] <= bound.f[0] && v.f[0] >= -bound.f[0]) ? 0xFFFFFFFFU : 0;
	u.u[1] = (v.f[1] <= bound.f[1] && v.f[1] >= -bound.f[1]) ? 0xFFFFFFFFU : 0;
	u.u[2] = (v.f[2] <= bound.f[2] && v.f[2] >= -bound.f[2]) ? 0xFFFFFFFFU : 0;
	u.u[3] = (v.f[3] <= bound.f[3] && v.f[3] >= -bound.f[3]) ? 0xFFFFFFFFU : 0;
	return u;
#endif
}

//
INLINE QMVEC QM_VECTORCALL qm_vec_ins(const QMVEC a, const QMVEC b, uint32_t e, uint32_t s0, uint32_t s1, uint32_t s2, uint32_t s3)
{
	const QMVEC o = qm_vec_select_ctrl(s0 & 1, s1 & 1, s2 & 1, s3 & 1);
	return qm_vec_select(a, qm_vec_bit_rol(b, e), o);
}


//////////////////////////////////////////////////////////////////////////
// vector

/// @brief 벡터 값 설정
INLINE QMVEC QM_VECTORCALL qm_vec(float x, float y, float z, float w)
{
#if defined QM_USE_AVX
	return _mm_setr_ps(x, y, z, w);
#elif defined QM_USE_NEON
	float32x2_t xy = vcreate_f32(((uint64_t)(*(uint32_t*)&x)) | (((uint64_t)(*(uint32_t*)&y)) << 32));
	float32x2_t zw = vcreate_f32(((uint64_t)(*(uint32_t*)&z)) | (((uint64_t)(*(uint32_t*)&w)) << 32));
	return vcombine_f32(xy, zw);
#else
	return (QMVEC) { { x, y, z, w } };
#endif
}

/// @brief 0 벡터 얻기
INLINE QMVEC QM_VECTORCALL qm_vec_zero(void)
{
#if defined QM_USE_AVX
	return _mm_setzero_ps();
#else
	return QMCONST_ZERO.s;
#endif
}

/// @brief 1 벡터 얻기
INLINE QMVEC QM_VECTORCALL qm_vec_one(void)
{
	return QMCONST_ONE.s;
}

/// @brief 모두 같은값으로 채우기
INLINE QMVEC QM_VECTORCALL qm_vec_sp(float diag)
{
#if defined QM_USE_AVX
	return _mm_set1_ps(diag);
#elif defined QM_USE_NEON
	return vdupq_n_f32(diag);
#else
	return (QMVEC) { { diag, diag, diag, diag } };
#endif
}

/// @brief X값으로 모두 채우기
INLINE QMVEC QM_VECTORCALL qm_vec_sp_x(const QMVEC v)
{
#if defined QM_USE_AVX2
	return _mm_broadcastss_ps(v);
#elif defined QM_USE_AVX
	return _mm_permute_ps(v, _MM_SHUFFLE(0, 0, 0, 0));
#elif defined QM_USE_NEON
	return vdupq_lane_f32(vget_low_f32(v), 0);
#else
	return (QMVEC) { { v.f[0], v.f[0], v.f[0], v.f[0] } };
#endif
}

/// @brief Y값으로 모두 채우기
INLINE QMVEC QM_VECTORCALL qm_vec_sp_y(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_permute_ps(v, _MM_SHUFFLE(1, 1, 1, 1));
#elif defined QM_USE_NEON
	return vdupq_lane_f32(vget_low_f32(v), 1);
#else
	return (QMVEC) { { v.f[1], v.f[1], v.f[1], v.f[1] } };
#endif
}

/// @brief Z값으로 모두 채우기
INLINE QMVEC QM_VECTORCALL qm_vec_sp_z(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_permute_ps(v, _MM_SHUFFLE(2, 2, 2, 2));
#elif defined QM_USE_NEON
	return vdupq_lane_f32(vget_high_f32(v), 0);
#else
	return (QMVEC) { { v.f[2], v.f[2], v.f[2], v.f[2] } };
#endif
}

/// @brief W값으로 모두 채우기
INLINE QMVEC QM_VECTORCALL qm_vec_sp_w(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_permute_ps(v, _MM_SHUFFLE(3, 3, 3, 3));
#elif defined QM_USE_NEON
	return vdupq_lane_f32(vget_high_f32(v), 1);
#else
	return (QMVEC) { { v.f[3], v.f[3], v.f[3], v.f[3] } };
#endif
}

/// @brief xy로 채우기
INLINE QMVEC QM_VECTORCALL qm_vec_sp_xy(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_unpacklo_ps(left, right);
#elif defined QM_USE_NEON
	return vzipq_f32(left, right).val[0];
#else
	return (QMVEC) { { left.f[0], right.f[0], left.f[1], right.f[1] } };
#endif
}

/// @brief zw로 채우기
INLINE QMVEC QM_VECTORCALL qm_vec_sp_zw(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_unpackhi_ps(left, right);
#elif defined QM_USE_NEON
	return vzipq_f32(left, right).val[1];
#else
	return (QMVEC) { { left.f[2], right.f[2], left.f[3], right.f[3] } };
#endif
}

/// @brief MSB 세팅 (부호)
INLINE QMVEC QM_VECTORCALL qm_vec_sp_msb(void)
{
	return QMCONST_MSB.s;
}

/// @brief 벡터 반전
INLINE QMVEC QM_VECTORCALL qm_vec_neg(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_sub_ps(_mm_setzero_ps(), v);
#elif defined QM_USE_NEON
	return vnegq_f32(v);
#else
	return (QMVEC) { { -v.f[0], -v.f[1], -v.f[2], -v.f[3] } };
#endif
}

/// @brief 벡터 역수 (1.0f / v)
INLINE QMVEC QM_VECTORCALL qm_vec_rcp(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_div_ps(QMCONST_ONE.s, v);
#elif defined QM_USE_NEON
	QMVEC h = vrecpeq_f32(v);
	h = vmulq_f32(vrecpsq_f32(v, h), h);
	h = vmulq_f32(vrecpsq_f32(v, h), h);
	return h;
#else
	return (QMVEC) { { 1.0f / v.f[0], 1.0f / v.f[1], 1.0f / v.f[2], 1.0f / v.f[3] } };
#endif
}

/// @brief 벡터 덧셈
INLINE QMVEC QM_VECTORCALL qm_vec_add(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_add_ps(left, right);
#elif defined QM_USE_NEON
	return vaddq_f32(left, right);
#else
	QMVEC u;
	u.f[0] = left.f[0] + right.f[0];
	u.f[1] = left.f[1] + right.f[1];
	u.f[2] = left.f[2] + right.f[2];
	u.f[3] = left.f[3] + right.f[3];
	return u;
#endif
}

/// @brief 벡터 뺄셈
INLINE QMVEC QM_VECTORCALL qm_vec_sub(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_sub_ps(left, right);
#elif defined QM_USE_NEON
	return vsubq_f32(left, right);
#else
	QMVEC u;
	u.f[0] = left.f[0] - right.f[0];
	u.f[1] = left.f[1] - right.f[1];
	u.f[2] = left.f[2] - right.f[2];
	u.f[3] = left.f[3] - right.f[3];
	return u;
#endif
}

/// @brief 벡터 확대
INLINE QMVEC QM_VECTORCALL qm_vec_mag(const QMVEC left, float right)
{
#if defined QM_USE_AVX
	return _mm_mul_ps(left, _mm_set1_ps(right));
#elif defined QM_USE_NEON
	return vmulq_n_f32(left, right);
#else
	return (QMVEC) { { left.f[0] * right, left.f[1] * right, left.f[2] * right, left.f[3] * right } };
#endif
}

/// @brief 벡터 축소
INLINE QMVEC QM_VECTORCALL qm_vec_abr(const QMVEC left, float right)
{
#if defined QM_USE_AVX
	return _mm_div_ps(left, _mm_set1_ps(right));
#elif defined QM_USE_NEON
	QMVEC h = vdupq_n_f32(right);
	QMVEC h = vdivq_f32(left, h);
	return h;
#else
	return (QMVEC) { { left.f[0] / right, left.f[1] / right, left.f[2] / right, left.f[3] / right } };
#endif
}

/// @brief 벡터 곱셈
INLINE QMVEC QM_VECTORCALL qm_vec_mul(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_mul_ps(left, right);
#elif defined QM_USE_NEON
	return vmulq_f32(left, right);
#else
	QMVEC u;
	u.f[0] = left.f[0] * right.f[0];
	u.f[1] = left.f[1] * right.f[1];
	u.f[2] = left.f[2] * right.f[2];
	u.f[3] = left.f[3] * right.f[3];
	return u;
#endif
}

/// @brief 벡터 곱하고 더하기
INLINE QMVEC QM_VECTORCALL qm_vec_madd(const QMVEC left, const QMVEC right, const QMVEC add)
{
#if defined QM_USE_AVX
	return _MM_FMADD_PS(left, right, add);
#elif defined QM_USE_NEON
	return vmlaq_f32(add, left, right);
#else
	QMVEC u;
	u.f[0] = left.f[0] * right.f[0] + add.f[0];
	u.f[1] = left.f[1] * right.f[1] + add.f[1];
	u.f[2] = left.f[2] * right.f[2] + add.f[2];
	u.f[3] = left.f[3] * right.f[3] + add.f[3];
	return u;
#endif
}

/// @brief 벡터 곱하고 반대로 빼기
INLINE QMVEC QM_VECTORCALL qm_vec_msub(const QMVEC left, const QMVEC right, const QMVEC sub)
{
#if defined QM_USE_AVX
	return _MM_FNMADD_PS(left, right, sub);
#elif defined QM_USE_NEON
	return vmlsq_f32(sub, left, right);
#else
	QMVEC u;
	u.f[0] = sub.f[0] - left.f[0] * right.f[0];
	u.f[1] = sub.f[1] - left.f[1] * right.f[1];
	u.f[2] = sub.f[2] - left.f[2] * right.f[2];
	u.f[3] = sub.f[3] - left.f[3] * right.f[3];
	return u;
#endif
}

/// @brief 벡터 나눗셈
INLINE QMVEC QM_VECTORCALL qm_vec_div(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_div_ps(left, right);
#elif defined QM_USE_NEON
	return vdivq_f32(left, right);
#else
	QMVEC u;
	u.f[0] = left.f[0] / right.f[0];
	u.f[1] = left.f[1] / right.f[1];
	u.f[2] = left.f[2] / right.f[2];
	u.f[3] = left.f[3] / right.f[3];
	return u;
#endif
}

/// @brief 벡터 최소값
INLINE QMVEC QM_VECTORCALL qm_vec_min(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_min_ps(left, right);
#elif defined QM_USE_NEON
	return vminq_f32(left, right);
#else
	QMVEC u;
	u.f[0] = QN_MIN(left.f[0], right.f[0]);
	u.f[1] = QN_MIN(left.f[1], right.f[1]);
	u.f[2] = QN_MIN(left.f[2], right.f[2]);
	u.f[3] = QN_MIN(left.f[3], right.f[3]);
	return u;
#endif
}

/// @brief 벡터 최대값
INLINE QMVEC QM_VECTORCALL qm_vec_max(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_max_ps(left, right);
#elif defined QM_USE_NEON
	return vmaxq_f32(left, right);
#else
	QMVEC u;
	u.f[0] = QN_MAX(left.f[0], right.f[0]);
	u.f[1] = QN_MAX(left.f[1], right.f[1]);
	u.f[2] = QN_MAX(left.f[2], right.f[2]);
	u.f[3] = QN_MAX(left.f[3], right.f[3]);
	return u;
#endif
}

/// @brief 벡터 범위 제한
INLINE QMVEC QM_VECTORCALL qm_vec_clamp(const QMVEC v, const QMVEC min, const QMVEC max)
{
#if defined QM_USE_AVX
	return _mm_min_ps(_mm_max_ps(v, min), max);
#elif defined QM_USE_NEON
	return vminq_f32(vmaxq_f32(v, min), max);
#else
	QMVEC u;
	u.f[0] = QN_CLAMP(v.f[0], min.f[0], max.f[0]);
	u.f[1] = QN_CLAMP(v.f[1], min.f[1], max.f[1]);
	u.f[2] = QN_CLAMP(v.f[2], min.f[2], max.f[2]);
	u.f[3] = QN_CLAMP(v.f[3], min.f[3], max.f[3]);
	return u;
#endif
}

/// @brief 벡터 범위 제한 (0~1)
INLINE QMVEC QM_VECTORCALL qm_vec_csat(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_min_ps(_mm_max_ps(v, QMCONST_ZERO.s), QMCONST_ONE.s);
#elif defined QM_USE_NEON
	return vminq_f32(vmaxq_f32(v, QMCONST_ZERO.s), QMCONST_ONE.s);
#else
	return qm_vec_clamp(v, QMCONST_ZERO.s, QMCONST_ONE.s);
#endif
}

/// @brief 벡터 각도 범위 제한
INLINE QMVEC QM_VECTORCALL qm_vec_crad(const QMVEC v)
{
#if defined QM_USE_AVX
	static const QmVec4 rpih = { { QM_RPI_H, QM_RPI_H, QM_RPI_H, QM_RPI_H } };
	static const QmVec4 tau = { { QM_TAU, QM_TAU, QM_TAU, QM_TAU } };
	QMVEC h = _mm_mul_ps(v, rpih.s);
	h = _mm_round_ps(h, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
	h = _MM_FNMADD_PS(h, tau.s, v);
	return h;
#elif defined QM_USE_NEON
	static const QmVec4 rpih = { { QM_RPI_H, QM_RPI_H, QM_RPI_H, QM_RPI_H } };
	static const QmVec4 tau = { { QM_TAU, QM_TAU, QM_TAU, QM_TAU } };
	QMVEC h = vmulq_f32(v, rpih.s);
	h = vrndnq_f32(h);
	h = vmlsq_f32(v, h, tau.s);
	return h;
#else
	QMVEC u;
	u.f[0] = qm_cradf(v.f[0]);
	u.f[1] = qm_cradf(v.f[1]);
	u.f[2] = qm_cradf(v.f[2]);
	u.f[3] = qm_cradf(v.f[3]);
	return u;
#endif
}

/// @brief 벡터 블랜드
INLINE QMVEC QM_VECTORCALL qm_vec_blend(const QMVEC left, float leftScale, const QMVEC right, float rightScale)
{
#if defined QM_USE_AVX
	return _MM_FMADD_PS(left, _mm_set1_ps(leftScale), _mm_mul_ps(right, _mm_set1_ps(rightScale)));
#elif defined QM_USE_NEON
	return vmlaq_n_f32(vmulq_n_f32(right, rightScale), left, leftScale);
#else
	QMVEC u;
	u.f[0] = left.f[0] * leftScale + right.f[0] * rightScale;
	u.f[1] = left.f[1] * leftScale + right.f[1] * rightScale;
	u.f[2] = left.f[2] * leftScale + right.f[2] * rightScale;
	u.f[3] = left.f[3] * leftScale + right.f[3] * rightScale;
	return u;
#endif
}

/// @brief 벡터 선형 혼합
INLINE QMVEC QM_VECTORCALL qm_vec_lerp(const QMVEC left, const QMVEC right, float scale)
{
#if defined QM_USE_AVX
	return _MM_FMADD_PS(_mm_sub_ps(right, left), _mm_set1_ps(scale), left);
#elif defined QM_USE_NEON
	return vmlaq_n_f32(left, vsubq_f32(right, left), scale);
#else
	const QMVEC s = qm_vec_sp(scale);
	const QMVEC l = qm_vec_sub(right, left);
	return qm_vec_madd(l, s, left);
#endif
}

/// @brief 벡터 선형 혼합 + 확대
INLINE QMVEC QM_VECTORCALL qm_vec_lerp_len(const QMVEC left, const QMVEC right, float scale, float len)
{
#if defined QM_USE_AVX
	QMVEC h = _MM_FMADD_PS(_mm_sub_ps(right, left), _mm_set1_ps(scale), left);
	return _mm_mul_ps(h, _mm_set1_ps(len));
#elif defined QM_USE_NEON
	QMVEC h = vmlaq_n_f32(left, vsubq_f32(right, left), scale);
	return vmulq_n_f32(h, len);
#else
	const QMVEC p = qm_vec_sp(scale);
	const QMVEC s = qm_vec_rcp(p);
	const QMVEC l = qm_vec_sub(right, left);
	const QMVEC h = qm_vec_madd(l, s, left);
	return qm_vec_mag(h, len);
#endif
}

/// @brief 벡터 허밋 계산
INLINE QMVEC QM_VECTORCALL qm_vec_hermite(const QMVEC pos1, const QMVEC tan1, const QMVEC pos2, const QMVEC tan2, float scale)
{
#if defined QM_USE_AVX
	float s2 = scale * scale;
	float s3 = scale * s2;
	QMVEC p1 = _mm_set1_ps(2.0f * s3 - 3.0f * s2 + 1.0f);
	QMVEC t1 = _mm_set1_ps(s3 - 2.0f * s2 + scale);
	QMVEC p2 = _mm_set1_ps(-2.0f * s3 + 3.0f * s2);
	QMVEC t2 = _mm_set1_ps(s3 - s2);
	QMVEC h = _mm_mul_ps(p1, pos1);
	h = _MM_FMADD_PS(t1, tan1, h);
	h = _MM_FMADD_PS(p2, pos2, h);
	h = _MM_FMADD_PS(t2, tan2, h);
	return h;
#elif defined QM_USE_NEON
	float s2 = scale * scale;
	float s3 = scale * s2;
	float p1 = 2.0f * s3 - 3.0f * s2 + 1.0f;
	float t1 = s3 - 2.0f * s2 + scale;
	float p2 = -2.0f * s3 + 3.0f * s2;
	float t2 = s3 - s2;
	QMVEC h = vmulq_n_f32(pos1, p1);
	h = vmlaq_n_f32(h, tan1, t1);
	h = vmlaq_n_f32(h, pos2, p2);
	h = vmlaq_n_f32(h, tan2, t2);
	return h;
#else
	float s2 = scale * scale;
	float s3 = scale * s2;
	QMVEC p1 = qm_vec_sp(2.0f * s3 - 3.0f * s2 + 1.0f);
	QMVEC t1 = qm_vec_sp(s3 - 2.0f * s2 + scale);
	QMVEC p2 = qm_vec_sp(-2.0f * s3 + 3.0f * s2);
	QMVEC t2 = qm_vec_sp(s3 - s2);
	QMVEC h = qm_vec_mul(p1, pos1);
	h = qm_vec_madd(t1, tan1, h);
	h = qm_vec_madd(p2, pos2, h);
	h = qm_vec_madd(t2, tan2, h);
	return h;
#endif
}

/// @brief 벡터 캣멀롬 스플라인 계산
INLINE QMVEC QM_VECTORCALL qm_vec_catmullrom(const QMVEC pos1, const QMVEC pos2, const QMVEC pos3, const QMVEC pos4, float scale)
{
#if defined QM_USE_AVX
	float s2 = scale * scale;
	float s3 = scale * s2;
	QMVEC p1 = _mm_set1_ps((-s3 + 2.0f * s2 - scale) * 0.5f);
	QMVEC p2 = _mm_set1_ps((3.0f * s3 - 5.0f * s2 + 2.0f) * 0.5f);
	QMVEC p3 = _mm_set1_ps((-3.0f * s3 + 4.0f * s2 + scale) * 0.5f);
	QMVEC p4 = _mm_set1_ps((s3 - s2) * 0.5f);
	p2 = _mm_mul_ps(pos2, p2);
	p1 = _MM_FMADD_PS(pos1, p1, p2);
	p4 = _mm_mul_ps(pos4, p4);
	p3 = _MM_FMADD_PS(pos3, p3, p4);
	p1 = _mm_add_ps(p1, p3);
	return p1;
#elif defined QM_USE_NEON
	float s2 = scale * scale;
	float s3 = scale * s2;
	float p1 = (-s3 + 2.0f * s2 - scale) * 0.5f;
	float p2 = (3.0f * s3 - 5.0f * s2 + 2.0f) * 0.5f;
	float p3 = (-3.0f * s3 + 4.0f * s2 + scale) * 0.5f;
	float p4 = (s3 - s2) * 0.5f;
	p2 = vmulq_n_f32(pos2, p2);
	p1 = vmlaq_n_f32(p2, pos1, p1);
	p4 = vmulq_n_f32(pos4, p4);
	p3 = vmlaq_n_f32(p4, pos3, p3);
	p1 = vaddq_f32(p1, p3);
	return p1;
#else
	float s2 = scale * scale;
	float s3 = scale * s2;
	QMVEC p1 = qm_vec_sp((-s3 + 2.0f * s2 - scale) * 0.5f);
	QMVEC p2 = qm_vec_sp((3.0f * s3 - 5.0f * s2 + 2.0f) * 0.5f);
	QMVEC p3 = qm_vec_sp((-3.0f * s3 + 4.0f * s2 + scale) * 0.5f);
	QMVEC p4 = qm_vec_sp((s3 - s2) * 0.5f);
	QMVEC h = qm_vec_mul(p1, pos1);
	h = qm_vec_madd(p2, pos2, h);
	h = qm_vec_madd(p3, pos3, h);
	h = qm_vec_madd(p4, pos4, h);
	return h;
#endif
}

/// @brief 벡터 질량 중심 좌표계 이동 계산
INLINE QMVEC QM_VECTORCALL qm_vec_barycentric(const QMVEC pos1, const QMVEC pos2, const QMVEC pos3, float f, float g)
{
#if defined QM_USE_AVX
	QMVEC p21 = _mm_sub_ps(pos2, pos1);
	QMVEC p31 = _mm_sub_ps(pos3, pos1);
	QMVEC df = _mm_set1_ps(f);
	QMVEC dg = _mm_set1_ps(g);
	return _MM_FMADD_PS(p31, dg, _MM_FMADD_PS(p21, df, pos1));
#elif defined QM_USE_NEON
	QMVEC p21 = vsubq_f32(pos2, pos1);
	QMVEC p31 = vsubq_f32(pos3, pos1);
	return vmlaq_n_f32(vmlaq_n_f32(pos1, p21, f), p31, g);
#else
	QMVEC p21 = qm_vec_sub(pos2, pos1);
	QMVEC p31 = qm_vec_sub(pos3, pos1);
	QMVEC df = qm_vec_sp(f);
	QMVEC dg = qm_vec_sp(g);
	return qm_vec_madd(p31, dg, qm_vec_madd(p21, df, pos1));
#endif
}

/// @brief 벡터의 X
INLINE float QM_VECTORCALL qm_vec_get_x(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(v);
#elif defined QM_USE_NEON
	return vgetq_lane_f32(v, 0);
#else
	return v.f[0];
#endif
}

/// @brief 벡터의 Y
INLINE float QM_VECTORCALL qm_vec_get_y(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(v, 1);
#else
	return v.f[1];
#endif
}

/// @brief 벡터의 Z
INLINE float QM_VECTORCALL qm_vec_get_z(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(v, 2);
#else
	return v.f[2];
#endif
}

/// @brief 벡터의 W
INLINE float QM_VECTORCALL qm_vec_get_w(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(v, 3);
#else
	return v.f[3];
#endif
}

/// @brief 벡터 X 설정
INLINE QMVEC QM_VECTORCALL qm_vec_set_x(const QMVEC v, float x)
{
#if defined QM_USE_AVX
	return _mm_move_ss(v, _mm_set_ss(x));
#elif defined QM_USE_NEON
	return vsetq_lane_f32(x, v, 0);
#else
	return (QMVEC) { { x, v.f[1], v.f[2], v.f[3] } };
#endif
}

/// @brief 벡터 Y 설정
INLINE QMVEC QM_VECTORCALL qm_vec_set_y(const QMVEC v, float y)
{
#if defined QM_USE_AVX
	return _mm_insert_ps(v, _mm_set_ss(y), 0x10);
#elif defined QM_USE_NEON
	return vsetq_lane_f32(y, v, 1);
#else
	return (QMVEC) { { v.f[0], y, v.f[2], v.f[3] } };
#endif
}

/// @brief 벡터 Z 설정
INLINE QMVEC QM_VECTORCALL qm_vec_set_z(const QMVEC v, float z)
{
#if defined QM_USE_AVX
	return _mm_insert_ps(v, _mm_set_ss(z), 0x20);
#elif defined QM_USE_NEON
	return vsetq_lane_f32(z, v, 2);
#else
	return (QMVEC) { { v.f[0], v.f[1], z, v.f[3] } };
#endif
}

/// @brief 벡터 W 설정
INLINE QMVEC QM_VECTORCALL qm_vec_set_w(const QMVEC v, float w)
{
#if defined QM_USE_AVX
	return _mm_insert_ps(v, _mm_set_ss(w), 0x30);
#elif defined QM_USE_NEON
	return vsetq_lane_f32(w, v, 3);
#else
	return (QMVEC) { { v.f[0], v.f[1], v.f[2], w } };
#endif
}

/// @brief 벡터 비교
INLINE bool QM_VECTORCALL qm_vec_eq(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	QMVEC h = _mm_cmpeq_ps(left, right);
	return (_mm_movemask_ps(h) == 0x0F) != 0;
#elif defined QM_USE_NEON
	QMVEC h = vceqq_f32(left, right);
	return vgetq_lane_u32(vreinterpretq_u32_f32(h), 0) == 0xFFFFFFFFU;
#else
	return
		left.f[0] == right.f[0] && left.f[1] == right.f[1] &&
		left.f[2] == right.f[2] && left.f[3] == right.f[3];
#endif
}

/// @brief 벡터 비교 (입실론)
INLINE bool QM_VECTORCALL qm_vec_eps(const QMVEC left, const QMVEC right, float epsilon)
{
#if defined QM_USE_AVX
	QMVEC eps = _mm_set1_ps(epsilon);
	QMVEC v = _mm_sub_ps(left, right);
	QMVEC h = _mm_setzero_ps();
	h = _mm_sub_ps(h, v);
	h = _mm_max_ps(h, v);
	h = _mm_cmple_ps(h, eps);
	return (_mm_movemask_ps(h) == 0xF) != 0;
#elif defined QM_USE_NEON
	float32x2_t d = vsub_f32(vget_low_f32(left), vget_low_f32(right));
	uint32x2_t t = vcle_f32(vabs_f32(d), vget_low_f32(vdup_n_f32(epsilon)));
	uint64_t r = vget_lane_u64(vreinterpret_u64_u32(t), 0);
	return r == 0xFFFFFFFFFFFFFFFFULL;
#else
	return
		qm_eqs(left.f[0], right.f[0], epsilon) && qm_eqs(left.f[1], right.f[1], epsilon) &&
		qm_eqs(left.f[2], right.f[2], epsilon) && qm_eqs(left.f[3], right.f[3], epsilon);
#endif
}

/// @brief 벡터 비교 (입실론)
INLINE bool QM_VECTORCALL qm_vec_near(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	QMVEC v = _mm_sub_ps(left, right);
	QMVEC h = _mm_setzero_ps();
	h = _mm_sub_ps(h, v);
	h = _mm_max_ps(h, v);
	h = _mm_cmple_ps(h, QMCONST_EPSILON.s);
	return (_mm_movemask_ps(h) == 0xF) != 0;
#elif defined QM_USE_NEON
	float32x2_t d = vsub_f32(vget_low_f32(left), vget_low_f32(right));
	uint32x2_t t = vcle_f32(vabs_f32(d), vget_low_f32(QMCONST_EPSILON.s));
	uint64_t r = vget_lane_u64(vreinterpret_u64_u32(t), 0);
	return r == 0xFFFFFFFFFFFFFFFFULL;
#else
	return
		qm_eqs(left.f[0], right.f[0], QM_EPSILON) && qm_eqs(left.f[1], right.f[1], QM_EPSILON) &&
		qm_eqs(left.f[2], right.f[2], QM_EPSILON) && qm_eqs(left.f[3], right.f[3], QM_EPSILON);
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_float2(QmFloat2* p, const QMVEC v)
{
#if defined QM_USE_AVX
	_mm_store_sd((double*)p, _mm_castps_pd(v));
#elif defined QM_USE_NEON
	float32x2_t lo = vget_low_f32(v);
	vst1_f32((float*)p, lo);
#else
	p->X = v.f[0];
	p->Y = v.f[1];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_float3(QmFloat3* p, const QMVEC v)
{
#if defined QM_USE_AVX
	*(int32_t*)&p->X = _mm_extract_ps(v, 0);
	*(int32_t*)&p->Y = _mm_extract_ps(v, 1);
	*(int32_t*)&p->Z = _mm_extract_ps(v, 2);
#elif defined QM_USE_NEON
	float32x2_t lo = vget_low_f32(v);
	vst1_f32((float*)p, lo);
	vst1q_lane_f32((float*)p + 2, v, 2);
#else
	p->X = v.f[0];
	p->Y = v.f[1];
	p->Z = v.f[2];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_float4(QmFloat4* p, const QMVEC v)
{
#if defined QM_USE_AVX
	_mm_storeu_ps((float*)p, v);
#elif defined QM_USE_NEON
	vst1q_f32((float*)p, v);
#else
	p->X = v.f[0];
	p->Y = v.f[1];
	p->Z = v.f[2];
	p->W = v.f[3];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_int2(QmInt2* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC w = _mm_cmpgt_ps(v, QMCONST_MAX_INT.s);
	QMIVEC h = _mm_cvttps_epi32(v);
	QMVEC r = _mm_and_ps(w, QMCONST_MASK_ABS.s);
	w = _mm_andnot_ps(w, _mm_castsi128_ps(h));
	w = _mm_or_ps(w, r);
	_mm_store_sd((double*)p, _mm_castps_pd(w));
#elif defined QM_USE_NEON
	float32x2_t lo = vget_low_f32(v);
	int32x2_t i = vcvt_s32_f32(lo);
	vst1_s32((int32_t*)p, i);
#else
	p->X = (int32_t)v.f[0];
	p->Y = (int32_t)v.f[1];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_int3(QmInt3* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC w = _mm_cmpgt_ps(v, QMCONST_MAX_INT.s);
	QMIVEC h = _mm_cvttps_epi32(v);
	QMVEC r = _mm_and_ps(w, QMCONST_MASK_ABS.s);
	w = _mm_andnot_ps(w, _mm_castsi128_ps(h));
	w = _mm_or_ps(w, r);
	_mm_store_sd((double*)p, _mm_castps_pd(w));
	QMVEC z = _MM_PERMUTE_PS(w, _MM_SHUFFLE(2, 2, 2, 2));
	_mm_store_ss((float*)&p->Z, z);
#elif defined QM_USE_NEON
	QMIVEC i = vcvtq_s32_f32(v);
	QMIVEC lo = vget_low_s32(i);
	vst1_s32((int32_t*)p, lo);
	vst1q_lane_s32((int32_t*)p + 2, i, 2);
#else
	p->X = (int32_t)v.f[0];
	p->Y = (int32_t)v.f[1];
	p->Z = (int32_t)v.f[2];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_int4(QmInt4* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC w = _mm_cmpgt_ps(v, QMCONST_MAX_INT.s);
	QMIVEC h = _mm_cvttps_epi32(v);
	QMVEC r = _mm_and_ps(w, QMCONST_MASK_ABS.s);
	w = _mm_andnot_ps(w, _mm_castsi128_ps(h));
	w = _mm_or_ps(w, r);
	_mm_storeu_si128((QMIVEC*)p, _mm_castps_si128(w));
#elif defined QM_USE_NEON
	QMIVEC i = vcvtq_s32_f32(v);
	vst1q_s32((int32_t*)p, i);
#else
	p->X = (int32_t)v.f[0];
	p->Y = (int32_t)v.f[1];
	p->Z = (int32_t)v.f[2];
	p->W = (int32_t)v.f[3];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_uint2(QmUint2* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC r = _mm_max_ps(v, QMCONST_ZERO.s);
	QMVEC w = _mm_cmpgt_ps(r, QMCONST_MAX_UINT.s);
	QMVEC m = _mm_cmpge_ps(r, QMCONST_UINT_SIGN_FIX.s);
	QMVEC t = _mm_and_ps(QMCONST_UINT_SIGN_FIX.s, m);
	r = _mm_sub_ps(r, t);
	QMIVEC h = _mm_cvttps_epi32(r);
	m = _mm_and_ps(m, QMCONST_MSB.s);
	r = _mm_xor_ps(_mm_castsi128_ps(h), m);
	r = _mm_or_ps(r, w);
	_mm_store_sd((double*)p, _mm_castps_pd(r));
#elif defined QM_USE_NEON
	float32x2_t lo = vget_low_f32(v);
	uint32x2_t u = vcvt_u32_f32(lo);
	vst1_u32((uint32_t*)p, u);
#else
	p->X = (uint32_t)v.f[0];
	p->Y = (uint32_t)v.f[1];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_uint3(QmUint3* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC r = _mm_max_ps(v, QMCONST_ZERO.s);
	QMVEC w = _mm_cmpgt_ps(r, QMCONST_MAX_UINT.s);
	QMVEC m = _mm_cmpge_ps(r, QMCONST_UINT_SIGN_FIX.s);
	QMVEC t = _mm_and_ps(QMCONST_UINT_SIGN_FIX.s, m);
	r = _mm_sub_ps(r, t);
	QMIVEC h = _mm_cvttps_epi32(r);
	m = _mm_and_ps(m, QMCONST_MSB.s);
	r = _mm_xor_ps(_mm_castsi128_ps(h), m);
	r = _mm_or_ps(r, w);
	_mm_store_sd((double*)p, _mm_castps_pd(r));
	QMVEC z = _MM_PERMUTE_PS(r, _MM_SHUFFLE(2, 2, 2, 2));
	_mm_store_ss((float*)&p->Z, z);
#elif defined QM_USE_NEON
	uint32x4_t i = vcvtq_u32_f32(v);
	uint32x4_t lo = vget_low_u32(i);
	vst1_u32((uint32_t*)p, lo);
	vst1q_lane_u32((uint32_t*)p + 2, i, 2);
#else
	p->X = (uint32_t)v.f[0];
	p->Y = (uint32_t)v.f[1];
	p->Z = (uint32_t)v.f[2];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_uint4(QmUint4* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC r = _mm_max_ps(v, QMCONST_ZERO.s);
	QMVEC w = _mm_cmpgt_ps(r, QMCONST_MAX_UINT.s);
	QMVEC m = _mm_cmpge_ps(r, QMCONST_UINT_SIGN_FIX.s);
	QMVEC t = _mm_and_ps(QMCONST_UINT_SIGN_FIX.s, m);
	r = _mm_sub_ps(r, t);
	QMIVEC h = _mm_cvttps_epi32(r);
	m = _mm_and_ps(m, QMCONST_MSB.s);
	r = _mm_xor_ps(_mm_castsi128_ps(h), m);
	r = _mm_or_ps(r, w);
	_mm_storeu_si128((QMIVEC*)p, _mm_castps_si128(r));
#elif defined QM_USE_NEON
	uint32x4_t i = vcvtq_u32_f32(v);
	vst1q_u32((uint32_t*)p, i);
#else
	p->X = (uint32_t)v.f[0];
	p->Y = (uint32_t)v.f[1];
	p->Z = (uint32_t)v.f[2];
	p->W = (uint32_t)v.f[3];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_ushort2(QmUshort2* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMIVEC i = _mm_cvtps_epi32(v);
	p->X = (uint16_t)_mm_extract_epi16(i, 0);
	p->Y = (uint16_t)_mm_extract_epi16(i, 2);
#elif defined QM_USE_NEON
	uint32x4_t i = vcvtq_u32_f32(v);
	uint16x4_t lo = vqmovn_u32(i);
	vst1_u16((uint16_t*)p, lo);
#else
	p->X = (uint16_t)v.f[0];
	p->Y = (uint16_t)v.f[1];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_ushort4(QmUshort4* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMIVEC i = _mm_cvtps_epi32(v);
	p->X = (uint16_t)_mm_extract_epi16(i, 0);
	p->Y = (uint16_t)_mm_extract_epi16(i, 2);
	p->Z = (uint16_t)_mm_extract_epi16(i, 4);
	p->W = (uint16_t)_mm_extract_epi16(i, 6);
#elif defined QM_USE_NEON
	uint32x4_t i = vcvtq_u32_f32(v);
	uint16x4_t lo = vqmovn_u32(i);
	uint16x8_t h = vcombine_u16(lo, lo);
	vst1_u16((uint16_t*)p, h);
#else
	p->X = (uint16_t)v.f[0];
	p->Y = (uint16_t)v.f[1];
	p->Z = (uint16_t)v.f[2];
	p->W = (uint16_t)v.f[3];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_half2(QmHalf2* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMIVEC i = _mm_cvtps_ph(v, _MM_FROUND_TO_NEAREST_INT);
	_mm_store_ss((float*)p, _mm_castsi128_ps(i));
#else
	p->X = qm_f2hf(qm_vec_get_x(v));
	p->Y = qm_f2hf(qm_vec_get_y(v));
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_half4(QmHalf4* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMIVEC i = _mm_cvtps_ph(v, _MM_FROUND_TO_NEAREST_INT);
	_mm_storel_epi64((QMIVEC*)p, i);
#else
	p->X = qm_f2hf(qm_vec_get_x(v));
	p->Y = qm_f2hf(qm_vec_get_y(v));
	p->Z = qm_f2hf(qm_vec_get_z(v));
	p->W = qm_f2hf(qm_vec_get_w(v));
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_u4444(QmU4444* p, const QMVEC v)
{
#if defined QM_USE_AVX
	static const QmVec4 max = { { 15.0f, 15.0f, 15.0f, 15.0f } };
	QMVEC h = _mm_min_ps(v, max.s);
	QMIVEC i = _mm_cvtps_epi32(h);
	uint16_t x = (uint16_t)_mm_extract_epi16(i, 0);
	uint16_t y = (uint16_t)_mm_extract_epi16(i, 2);
	uint16_t z = (uint16_t)_mm_extract_epi16(i, 4);
	uint16_t w = (uint16_t)_mm_extract_epi16(i, 6);
	p->v = (uint16_t)(((int32_t)x & 0x0F) | (((int32_t)y & 0x0F) << 4) | (((int32_t)z & 0x0F) << 8) | (((int32_t)w & 0x0F) << 12));
#else
	p->v = (uint16_t)(
		((int32_t)(qm_vec_get_x(v) * 15.0f) & 0x0F) |
		(((int32_t)(qm_vec_get_y(v) * 15.0f) & 0x0F) << 4) |
		(((int32_t)(qm_vec_get_z(v) * 15.0f) & 0x0F) << 8) |
		(((int32_t)(qm_vec_get_w(v) * 15.0f) & 0x0F) << 12));
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_u565(QmU565* p, const QMVEC v)
{
#if defined QM_USE_AVX
	static const QmVec4 max = { { 31.0f, 63.0f, 31.0f, 0.0f } };
	QMVEC h = _mm_min_ps(v, max.s);
	QMIVEC i = _mm_cvtps_epi32(h);
	uint16_t x = (uint16_t)_mm_extract_epi16(i, 0);
	uint16_t y = (uint16_t)_mm_extract_epi16(i, 2);
	uint16_t z = (uint16_t)_mm_extract_epi16(i, 4);
	p->v = (uint16_t)(((int32_t)x & 0x1F) | (((int32_t)y & 0x3F) << 5) | (((int32_t)z & 0x1F) << 11));
#else
	p->v = (uint16_t)(
		((int32_t)(qm_vec_get_x(v) * 31.0f) & 0x1F) |
		(((int32_t)(qm_vec_get_y(v) * 63.0f) & 0x3F) << 5) |
		(((int32_t)(qm_vec_get_z(v) * 31.0f) & 0x1F) << 11));
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_u5551(QmU5551* p, const QMVEC v)
{
#if defined QM_USE_AVX
	static const QmVec4 max = { { 31.0f, 31.0f, 31.0f, 1.0f } };
	QMVEC h = _mm_min_ps(v, max.s);
	QMIVEC i = _mm_cvtps_epi32(h);
	uint16_t x = (uint16_t)_mm_extract_epi16(i, 0);
	uint16_t y = (uint16_t)_mm_extract_epi16(i, 2);
	uint16_t z = (uint16_t)_mm_extract_epi16(i, 4);
	uint16_t w = (uint16_t)_mm_extract_epi16(i, 6);
	p->v = (uint16_t)(((int32_t)x & 0x1F) | (((int32_t)y & 0x1F) << 5) | (((int32_t)z & 0x1F) << 10) | ((int32_t)w ? 0x8000 : 01));
#else
	p->v = (uint16_t)(
		((int32_t)(qm_vec_get_x(v) * 31.0f) & 0x1F) |
		(((int32_t)(qm_vec_get_y(v) * 31.0f) & 0x1F) << 5) |
		(((int32_t)(qm_vec_get_z(v) * 31.0f) & 0x1F) << 10) |
		(((int32_t)(qm_vec_get_w(v) * 1.0f) & 0x01) << 15));
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_f111110(QmF111110* p, const QMVEC v)
{
	QM_ALIGN(16) uint32_t i[4];
	qm_vec_st_float3a((QmFloat3A*)i, v);
	uint32_t res[3], sign, u;
	for (uint32_t z = 0; z < 2; z++)
	{
		sign = i[z] & 0x80000000U;
		u = i[z] & 0x7F800000U;
		if ((u & 0x7F800000U) == 0x7F800000U)
		{
			res[z] = 0x7C0U;
			if ((u & 0x007FFFFFU) != 0U)
				res[z] |= 0x7FFU;
			else if (sign)
				res[z] = 0U;
		}
		else if (sign || u < 0x35800000U)
			res[z] = 0U;
		else if (u > 0x477FE000U)
			res[z] = 0x7BFU;
		else
		{
			if (u < 0x38000000U)
			{
				const uint32_t shift = 113U - (u >> 23U);
				u = (0x800000U | (i[z] & 0x007FFFFFU)) >> shift;
			}
			else
				u += 0xC8000000U;
			res[z] = ((u + 0xFFFFU + ((u >> 17U) & 1)) >> 17U) & 0x7FFU;
		}
	}
	sign = i[2] & 0x80000000U;
	u = i[2] & 0x7FFFFFFFU;
	if ((u & 0x7F800000U) == 0x7F800000U)
	{
		res[2] = 0x3E0U;
		if (u & 0x007FFFFFU)
			res[2] |= 0x3FFU;
		else if (sign && u < 0x36000000U)
			res[2] = 0U;
	}
	else if (sign)
		res[2] = 0U;
	else if (u > 0x477C0000U)
		res[2] = 0x3DFU;
	else
	{
		if (u < 0x38800000U)
		{
			const uint32_t shift = 113U - (u >> 23U);
			u = (0x800000U | (i[2] & 0x007FFFFFU)) >> shift;
		}
		else
			u += 0xC8000000U;
		res[2] = ((u + 0x1FFFFU + ((u >> 18U) & 1U)) >> 18U) & 0x3FFU;
	}
	p->v = res[0] | ((res[1] & 0x7FFU) << 10U) | ((res[2] & 0x3FFU) << 21U);
}

//
INLINE void QM_VECTORCALL qm_vec_st_u1010102(QmU1010102* p, const QMVEC v)
{
	const int32_t x = (int32_t)(qm_vec_get_x(v) * 1023) & 0x3FF;
	const int32_t y = (int32_t)(qm_vec_get_y(v) * 1023) & 0x3FF;
	const int32_t z = (int32_t)(qm_vec_get_z(v) * 1023) & 0x3FF;
	const int32_t w = (int32_t)(qm_vec_get_w(v) * 3) & 0x03;
	p->v = (uint32_t)(x | (y << 10) | (z << 20) | (w << 30));
}

//
INLINE void QM_VECTORCALL qm_vec_st_kolor(QmKolor* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC h = _mm_mul_ps(v, QMCONST_MAX_UBYTE.s);
	h = _MM_PERMUTE_PS(h, _MM_SHUFFLE(3, 0, 1, 2));
	QMIVEC i = _mm_cvtps_epi32(h);
	i = _mm_packus_epi32(i, i);
	i = _mm_packus_epi16(i, i);
	_mm_store_ss((float*)&p->v, _mm_castsi128_ps(i));
#elif defined QM_USE_NEON
	uint32x4_t i = vcvtq_u32_f32(v);
	uint16x4_t lo = vqmovn_u32(i);
	uint8x8_t h = vqmovn_u16(vcombine_u16(lo, lo));
	vst1_u8((uint8_t*)&p->v, h);
#else
	p->R = (uint8_t)(v.f[0] * 255.0f);
	p->G = (uint8_t)(v.f[1] * 255.0f);
	p->B = (uint8_t)(v.f[2] * 255.0f);
	p->A = (uint8_t)(v.f[3] * 255.0f);
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_float3a(QmFloat3A* p, const QMVEC v)
{
#if defined QM_USE_AVX
	_mm_store_sd((double*)p, _mm_castps_pd(v));
	*(int32_t*)&p->Z = _mm_extract_ps(v, 2);
#elif defined QM_USE_NEON
	float32x2_t lo = vget_low_f32(v);
	vst1_f32((float*)p, lo);
	vst1q_lane_f32((float*)p + 2, v, 2);
#else
	p->X = v.f[0];
	p->Y = v.f[1];
	p->Z = v.f[2];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_float4a(QmFloat4A* p, const QMVEC v)
{
#if defined QM_USE_AVX
	_mm_storeu_ps((float*)p, v);
	*(int32_t*)&p->W = _mm_extract_ps(v, 3);
#elif defined QM_USE_NEON
	vst1q_f32((float*)p, v);
#else
	p->X = v.f[0];
	p->Y = v.f[1];
	p->Z = v.f[2];
	p->W = v.f[3];
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_ushort2n(QmUshort2* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC r = _mm_add_ps(_mm_mul_ps(v, QMCONST_MAX_USHORT.s), QMCONST_HALF.s);
	QMIVEC i = _mm_cvttps_epi32(r);
	p->X = (uint16_t)_mm_extract_epi16(i, 0);
	p->Y = (uint16_t)_mm_extract_epi16(i, 2);
#elif defined QM_USE_NEON
	QMVEC r = vaddq_f32(vmulq_n_f32(v, 65535.0f), QMCONST_HALF.s);
	uint32x4_t u32 = vcvtq_u32_f32(r);
	uint16x4_t u16 = vqmovn_u32(u32);
	vst1_lane_u32(&p->v, vreinterpretq_f32_u16(u16), 0);
#else
	p->X = (uint16_t)(v.f[0] * 65535.0f);
	p->Y = (uint16_t)(v.f[1] * 65535.0f);
#endif
}

//
INLINE void QM_VECTORCALL qm_vec_st_ushort4n(QmUshort4* p, const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC r = _mm_add_ps(_mm_mul_ps(v, QMCONST_MAX_USHORT.s), QMCONST_HALF.s);
	QMIVEC i = _mm_cvttps_epi32(r);
	p->X = (uint16_t)_mm_extract_epi16(i, 0);
	p->Y = (uint16_t)_mm_extract_epi16(i, 2);
	p->Z = (uint16_t)_mm_extract_epi16(i, 4);
	p->W = (uint16_t)_mm_extract_epi16(i, 6);
#elif defined QM_USE_NEON
	QMVEC r = vaddq_f32(vmulq_n_f32(v, 65535.0f), QMCONST_HALF.s);
	uint16x4_t u16 = vmovn_u32(vcvtq_u32_f32(r));
	vst1q_u16((uint16_t*)p, u16);
#else
	p->X = (uint16_t)(v.f[0] * 65535.0f);
	p->Y = (uint16_t)(v.f[1] * 65535.0f);
	p->Z = (uint16_t)(v.f[2] * 65535.0f);
	p->W = (uint16_t)(v.f[3] * 65535.0f);
#endif
}

/// @brief 벡터 한번에 제곱근
INLINE QMVEC QM_VECTORCALL qm_vec_simd_sqrt(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_sqrt_ps(v);
#elif defined QM_USE_NEON
	QMVEC h = vrsqrteq_f32(v);
	return vmulq_f32(vrsqrtsq_f32(v, vmulq_f32(h, h)), h);
#else
	const float f = qm_sqrtf(v.f[0]);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 제곱근 역수
INLINE QMVEC QM_VECTORCALL qm_vec_simd_rsqrt(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_rsqrt_ps(v);
#elif defined QM_USE_NEON
	return vrsqrteq_f32(v);
#else
	const float f = qm_rsqrtf(v.f[0]);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 사인 코사인
INLINE void QM_VECTORCALL qm_vec_simd_sincos(const QMVEC v, QMVEC* ret_sin, QMVEC* ret_cos)
{
#if defined _MSC_VER && defined QM_USE_AVX
	*ret_sin = _mm_sincos_ps(ret_cos, v);
#else
	float sx, cx, sy, cy, sz, cz, sw, cw;
	qm_sincosf(qm_vec_get_x(v), &sx, &cx);
	qm_sincosf(qm_vec_get_y(v), &sy, &cy);
	qm_sincosf(qm_vec_get_z(v), &sz, &cz);
	qm_sincosf(qm_vec_get_w(v), &sw, &cw);
	*ret_sin = (QMVEC){ { sx, sy, sz, sw } };
	*ret_cos = (QMVEC){ { cx, cy, cz, cw } };
#endif
}

/// @brief 벡터 한번에 사인
INLINE QMVEC QM_VECTORCALL qm_vec_simd_sin(const QMVEC v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	return _mm_sin_ps(v);
#else
	const float x = qm_vec_get_x(v);
	const float f = sinf(x);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 코사인
INLINE QMVEC QM_VECTORCALL qm_vec_simd_cos(const QMVEC v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	return _mm_cos_ps(v);
#else
	const float x = qm_vec_get_x(v);
	const float f = cosf(x);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 탄젠트
INLINE QMVEC QM_VECTORCALL qm_vec_simd_tan(const QMVEC v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	return _mm_tan_ps(v);
#else
	const float x = qm_vec_get_x(v);
	const float f = tanf(x);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 아크사인
INLINE QMVEC QM_VECTORCALL qm_vec_simd_asin(const QMVEC v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	return _mm_asin_ps(v);
#else
	const float x = qm_vec_get_x(v);
	const float f = asinf(x);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 아크코사인
INLINE QMVEC QM_VECTORCALL qm_vec_simd_acos(const QMVEC v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	return _mm_acos_ps(v);
#else
	const float x = qm_vec_get_x(v);
	const float f = acosf(x);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 아크탄젠트
INLINE QMVEC QM_VECTORCALL qm_vec_simd_atan(const QMVEC v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	return _mm_atan_ps(v);
#else
	const float x = qm_vec_get_x(v);
	const float f = atanf(x);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 아크탄젠트 (y/x)
INLINE QMVEC QM_VECTORCALL qm_vec_simd_atan2(const QMVEC y, const QMVEC x)
{
#if defined _MSC_VER &&  defined QM_USE_AVX
	return _mm_atan2_ps(y, x);
#else
	const float yy = qm_vec_get_x(y);
	const float xx = qm_vec_get_x(x);
	const float f = atan2f(yy, xx);
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 반올림
INLINE QMVEC QM_VECTORCALL qm_vec_simd_round(const QMVEC v)
{
#if defined QM_USE_AVX
	return _mm_round_ps(v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#elif defined QM_USE_NEON
#if defined _M_ARM64 || defined _M_ARM64EC || defined __aarch64__
	return vrndnq_f32(v);
#else
	static const QmVec4 NoFraction = { { 8388608.0f, 8388608.0f, 8388608.0f, 8388608.0f } };
	uint32x4_t sign = vandq_u32(vreinterpretq_u32_f32(v), QMCONST_MSB.s);
	QMVEC magic = vreinterpretq_f32_u32(vorrq_u32(NoFraction.s, sign));
	QMVEC r1 = vsubq_f32(vaddq_f32(v, magic), magic);
	QMVEC r2 = vabsq_f32(vsubq_f32(v, magic), v);
	uint32x4_t m = vcleq_f32(r2, NoFraction, s);
	return vbslq_f32(m, r1, v);
#endif
#else
	return (QMVEC) { { qm_roundf(v.f[0]), qm_roundf(v.f[1]), qm_roundf(v.f[2]), qm_roundf(v.f[3]) } };
#endif
}

/// @brief 벡터 한번에 블랜드
INLINE QMVEC QM_VECTORCALL qm_vec_simd_blend(const QMVEC left, const QMVEC left_scale, const QMVEC right, const QMVEC right_scale)
{
#if defined QM_USE_AVX
	return _MM_FMADD_PS(left, left_scale, _mm_mul_ps(right, right_scale));
#elif defined QM_USE_NEON
	return vmlaq_f32(vmulq_f32(right, right_scale), left, left_scale);
#else
	QMVEC u;
	u.f[0] = left.f[0] * left_scale.f[0] + right.f[0] * right_scale.f[0];
	u.f[1] = left.f[1] * left_scale.f[1] + right.f[1] * right_scale.f[1];
	u.f[2] = left.f[2] * left_scale.f[2] + right.f[2] * right_scale.f[2];
	u.f[3] = left.f[3] * left_scale.f[3] + right.f[3] * right_scale.f[3];
	return u;
#endif
}


//////////////////////////////////////////////////////////////////////////
// vector2

/// @brief 벡터2 값 설정
/// @param x,y 좌표
INLINE QmVec2 qm_vec2(float x, float y)
{
	return (QmVec2) { { x, y } };
}

/// @brief 정수 좌표 설정
/// @param p 좌표
INLINE QmVec2 qm_vec2p(const QmPoint p)
{
	return (QmVec2) { { (float)p.X, (float)p.Y } };
}

/// @brief 벡터2 초기화
INLINE QmVec2 qm_vec2_zero(void)
{
	static const QmVec2 ZERO = { { 0.0f, 0.0f } };
	return ZERO;
}

/// @brief 벡터2 대각값 설정 (모두 같은값으로 설정)
/// @param diag 대각 값
INLINE QmVec2 qm_vec2_sp(float diag)
{
	return (QmVec2) { { diag, diag } };
}

/// @brief 벡터2 부호 반전
/// @param v 벡터2
INLINE QmVec2 qm_vec2_neg(const QmVec2 v)
{
	return (QmVec2) { { -v.X, -v.Y } };
}

/// @brief 벡터2 역수 (1.0f / v)
/// @param v 벡터2
INLINE QmVec2 qm_vec2_rcp(const QmVec2 v)
{
	return (QmVec2) { { 1.0f / v.X, 1.0f / v.Y } };
}

/// @brief 벡터2 덧셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_add(const QmVec2 left, const QmVec2 right)
{
	return (QmVec2) { { left.X + right.X, left.Y + right.Y } };
}

/// @brief 벡터2 뺄셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_sub(const QmVec2 left, const QmVec2 right)
{
	return (QmVec2) { { left.X - right.X, left.Y - right.Y } };
}

/// @brief 벡터2 확대
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 확대값
INLINE QmVec2 qm_vec2_mag(const QmVec2 left, float right)
{
	return (QmVec2) { { left.X * right, left.Y * right } };
}

/// @brief 벡터2 줄이기
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 줄일값
INLINE QmVec2 qm_vec2_abr(const QmVec2 left, float right)
{
	return (QmVec2) { { left.X / right, left.Y / right } };
}

/// @brief 벡터2 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmVec2 qm_vec2_mul(const QmVec2 left, const QmVec2 right)
{
	return (QmVec2) { { left.X * right.X, left.Y * right.Y } };
}

/// @brief 벡터2 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmVec2 qm_vec2_div(const QmVec2 left, const QmVec2 right)
{
	return (QmVec2) { { left.X / right.X, left.Y / right.Y } };
}

/// @brief 벡터2의 최소값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_min(const QmVec2 left, const QmVec2 right)
{
	return (QmVec2) { { QN_MIN(left.X, right.X), QN_MIN(left.Y, right.Y) } };
}

/// @brief 벡터2의 최대값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_max(const QmVec2 left, const QmVec2 right)
{
	return (QmVec2) { { QN_MAX(left.X, right.X), QN_MAX(left.Y, right.Y) } };
}

/// @brief 벡터2 정규화
/// @param v 벡터2
INLINE QmVec2 qm_vec2_norm(const QmVec2 v)
{
	float l = qm_vec2_len(v);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec2_mag(v, l);
}

/// @brief 벡터2의 외적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_cross(const QmVec2 left, const QmVec2 right)
{
	return (QmVec2) { { left.Y * right.X - left.X * right.Y, left.X * right.Y - left.Y * right.X } };
}

/// @brief 벡터2 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmVec2 qm_vec2_lerp(const QmVec2 left, const QmVec2 right, float scale)
{
	return qm_vec2_add(left, qm_vec2_mag(qm_vec2_sub(right, left), scale));
}

/// @brief 벡터2 반사
INLINE QmVec2 qm_vec2_reflect(const QmVec2 incident, const QmVec2 normal)
{
	return qm_vec2_sub(incident, qm_vec2_mag(normal, 2.0f * qm_vec2_dot(incident, normal)));
}

/// @brief 벡터2 굴절
INLINE QmVec2 qm_vec2_refract(const QmVec2 incident, const QmVec2 normal, float eta)
{
	const float dot = qm_vec2_dot(incident, normal);
	const float k = 1.0f - eta * eta * (1.0f - dot * dot);
	if (k < 0.0f + QM_EPSILON)
		return qm_vec2_zero();
	return qm_vec2_sub(qm_vec2_mag(incident, eta), qm_vec2_mag(normal, eta * dot + qm_sqrtf(k)));
}

/// @brief 벡터2 내적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 내적 값
INLINE float qm_vec2_dot(const QmVec2 left, const QmVec2 right)
{
	return left.X * right.X + left.Y * right.Y;
}

/// @brief 벡터2 길이의 제곱
/// @param v 벡터2
/// @return 길이의 제곱
INLINE float qm_vec2_len_sq(const QmVec2 v)
{
	return qm_vec2_dot(v, v);
}

/// @brief 벡터2 길이
/// @param v 벡터2
/// @return 길이
INLINE float qm_vec2_len(const QmVec2 v)
{
	return qm_sqrtf(qm_vec2_len_sq(v));
}

/// @brief 두 벡터2 거리의 제곱
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2 거리의 제곱값
INLINE float qm_vec2_dist_sq(const QmVec2 left, const QmVec2 right)
{
	return qm_vec2_len_sq(qm_vec2_sub(left, right));
}

/// @brief 두 벡터2의 거리
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2의 거리값
INLINE float qm_vec2_dist(const QmVec2 left, const QmVec2 right)
{
	return qm_sqrtf(qm_vec2_dist_sq(left, right));
}

/// @brief 벡터2의 비교
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡트
/// @return 같으면 참
INLINE bool qm_vec2_eq(const QmVec2 left, const QmVec2 right)
{
	return qm_eqf(left.X, right.X) && qm_eqf(left.Y, right.Y);
}

/// @brief 벡터2 비교 (입실론)
INLINE bool qm_vec2_eps(const QmVec2 left, const QmVec2 right, float epsilon)
{
	return qm_eqs(left.X, right.X, epsilon) && qm_eqs(left.Y, right.Y, epsilon);
}

/// @brief 벡터2가 0인가 비교
/// @param v 비교할 벡터2
/// @return 0이면 참
INLINE bool qm_vec2_isz(const QmVec2 v)
{
	return v.X == 0.0f && v.Y == 0.0f;
}


//////////////////////////////////////////////////////////////////////////
// vector3 (data type is vector4)

/// @brief 벡터3 값 설정
INLINE QMVEC QM_VECTORCALL qm_vec3(float x, float y, float z)
{
	return qm_vec(x, y, z, 0.0f);
}

/// @brief 벡터3 외적
INLINE QMVEC QM_VECTORCALL qm_vec3_cross(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	QMVEC t1 = _MM_PERMUTE_PS(left, _MM_SHUFFLE(3, 0, 2, 1));
	QMVEC t2 = _MM_PERMUTE_PS(right, _MM_SHUFFLE(3, 1, 0, 2));
	QMVEC h = _mm_mul_ps(t1, t2);
	t1 = _MM_PERMUTE_PS(t1, _MM_SHUFFLE(3, 0, 2, 1));
	t2 = _MM_PERMUTE_PS(t2, _MM_SHUFFLE(3, 1, 0, 2));
	return _MM_FNMADD_PS(t1, t2, h);
#elif defined QM_USE_NEON
	static const QmVecU flip_y = { { 0, 0x80000000, 0, 0 } };
	QMVEC v1xy = vget_low_f32(left);
	QMVEC v2xy = vget_low_f32(right);
	QMVEC v1yx = vrev64q_f32(v1xy);
	QMVEC v2yx = vrev64q_f32(v2xy);
	QMVEC v1zz = vdupq_lane_f32(vget_high_f32(left), 0);
	QMVEC v2zz = vdupq_lane_f32(vget_high_f32(right), 0);
	QMVEC h = vmulq_f32(vcombine_f32(v1yx, v1xy), vcombine_f32(v2zz, v2yx));
	h = vmlsq_f32(h, vcombine_f32(v1zz, v1yx), vcombine_f32(v2yx, v2xy));
	h = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(h), flip_y.s));
	return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(h), QMCONST_S1110.s));
#else
	QMVEC u;
	u.f[0] = left.f[1] * right.f[2] - left.f[2] * right.f[1];
	u.f[1] = left.f[2] * right.f[0] - left.f[0] * right.f[2];
	u.f[2] = left.f[0] * right.f[1] - left.f[1] * right.f[0];
	u.f[3] = 0.0f;
	return u;
#endif
}

/// @brief 벡터3 법선
INLINE QMVEC QM_VECTORCALL qm_vec3_norm(const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC d = _mm_dp_ps(v, v, 0x7F);
	QMVEC h = _mm_rsqrt_ps(d);
	return _mm_mul_ps(v, h);
#elif defined QM_USE_NEON
	QMVEC h = vmulq_f32(v, v);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vpadd_f32(v1, v1);
	v2 = vdups_lane_f32(v2, 0);
	v1 = vadd_f32(v1, v2);
	v2 = vrsqrte_f32(v1);
	return vmulq_f32(v, vcombine_f32(v2, v2));
#else
	float l = qm_vec3_len(v);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec_mag(v, l);
#endif
}

/// @brief 벡터3 반사
INLINE QMVEC QM_VECTORCALL qm_vec3_reflect(const QMVEC incident, const QMVEC normal)
{
	QMVEC h = qm_vec3_simd_dot(incident, normal);
	h = qm_vec_add(h, h);
	return qm_vec_msub(normal, h, incident);
}

/// @brief 벡터3 굴절
INLINE QMVEC QM_VECTORCALL qm_vec3_refract(const QMVEC incident, const QMVEC normal, float eta)
{
	const float dot = qm_vec3_dot(incident, normal);
	const float k = 1.0f - eta * eta * (1.0f - dot * dot);
	if (k < 0.0f + QM_EPSILON)
		return qm_vec_zero();
	return qm_vec_msub(qm_vec_mag(incident, eta), qm_vec_mag(normal, eta * dot + qm_sqrtf(k)), incident);
}

/// @brief 두 벡터의 방향 (결국 법선)
INLINE QMVEC QM_VECTORCALL qm_vec3_dir(const QMVEC left, const QMVEC right)
{
	//return qm_vec3_norm(qm_vec3_cross(left, right));
	return qm_vec3_norm(qm_vec_sub(left, right));
}

/// @brief 벡터3을 사원수로 회전
INLINE QMVEC QM_VECTORCALL qm_vec3_rot(const QMVEC v, const QMVEC rot_quat)
{
	const QMVEC s = qm_vec_select(QMCONST_S1110.s, v, QMCONST_S1110.s);
	const QMVEC q = qm_quat_cjg(rot_quat);
	const QMVEC h = qm_quat_mul(q, s);
	return qm_quat_mul(h, rot_quat);
}

/// @brief 벡터3을 사원수로 반대로 회전
INLINE QMVEC QM_VECTORCALL qm_vec3_rot_inv(const QMVEC v, const QMVEC rot_quat)
{
	const QMVEC s = qm_vec_select(QMCONST_S1110.s, v, QMCONST_S1110.s);
	const QMVEC h = qm_quat_mul(rot_quat, s);
	const QMVEC q = qm_quat_cjg(rot_quat);
	return qm_quat_mul(h, q);
}

/// @brief 벡터3을 행렬로 변환시킴
INLINE QMVEC QM_VECTORCALL qm_vec3_trfm(const QMVEC v, const QMMAT m)
{
#if defined QM_USE_AVX
	QMVEC x = _mm_permute_ps(v, _MM_SHUFFLE(0, 0, 0, 0));
	QMVEC y = _mm_permute_ps(v, _MM_SHUFFLE(1, 1, 1, 1));
	QMVEC z = _mm_permute_ps(v, _MM_SHUFFLE(2, 2, 2, 2));
	QMVEC r = _MM_FMADD_PS(z, m.r[2], m.r[3]);
	r = _MM_FMADD_PS(y, m.r[1], r);
	return _MM_FMADD_PS(x, m.r[0], r);
#else
	QMVEC x = qm_vec_sp_x(v);
	QMVEC y = qm_vec_sp_y(v);
	QMVEC z = qm_vec_sp_z(v);
	QMVEC h = qm_vec_madd(z, m.r[2], m.r[3]);
	h = qm_vec_madd(y, m.r[1], h);
	return qm_vec_madd(x, m.r[0], h);
#endif
}

/// @brief 벡터3을 행렬로 정규화 변환시킴
INLINE QMVEC QM_VECTORCALL qm_vec3_trfm_norm(const QMVEC v, const QMMAT m)
{
#if defined QM_USE_AVX
	QMVEC x = _mm_permute_ps(v, _MM_SHUFFLE(0, 0, 0, 0));
	QMVEC y = _mm_permute_ps(v, _MM_SHUFFLE(1, 1, 1, 1));
	QMVEC z = _mm_permute_ps(v, _MM_SHUFFLE(2, 2, 2, 2));
	QMVEC r = _mm_mul_ps(z, m.r[2]);
	r = _MM_FMADD_PS(y, m.r[1], r);
	return _MM_FMADD_PS(x, m.r[0], r);
#else
	QMVEC x = qm_vec_sp_x(v);
	QMVEC y = qm_vec_sp_y(v);
	QMVEC z = qm_vec_sp_z(v);
	QMVEC h = qm_vec_mul(z, m.r[2]);
	h = qm_vec_madd(y, m.r[1], h);
	return qm_vec_madd(x, m.r[0], h);
#endif
}

/// @brief 벡터3을 행렬로 변환시킴
INLINE QMVEC QM_VECTORCALL qm_vec3_trfm_coord(const QMVEC v, const QMMAT m)
{
#if defined QM_USE_AVX
	QMVEC x = _mm_permute_ps(v, _MM_SHUFFLE(0, 0, 0, 0));
	QMVEC y = _mm_permute_ps(v, _MM_SHUFFLE(1, 1, 1, 1));
	QMVEC z = _mm_permute_ps(v, _MM_SHUFFLE(2, 2, 2, 2));
	QMVEC r = _MM_FMADD_PS(z, m.r[2], m.r[3]);
	r = _MM_FMADD_PS(y, m.r[1], r);
	r = _MM_FMADD_PS(x, m.r[0], r);
	QMVEC w = _mm_permute_ps(r, _MM_SHUFFLE(2, 2, 2, 2));
	return _mm_div_ps(r, w);
#else
	QMVEC x = qm_vec_sp_x(v);
	QMVEC y = qm_vec_sp_y(v);
	QMVEC z = qm_vec_sp_z(v);
	QMVEC r = qm_vec_madd(z, m.r[2], m.r[3]);
	r = qm_vec_madd(y, m.r[1], r);
	r = qm_vec_madd(x, m.r[0], r);
	QMVEC w = qm_vec_sp_w(r);
	return qm_vec_div(r, w);
#endif
}

/// @brief 세 벡터로 법선 벡터를 만든다
INLINE QMVEC QM_VECTORCALL qm_vec3_form_norm(const QMVEC v1, const QMVEC v2, const QMVEC v3)
{
	const QMVEC c = qm_vec3_cross(qm_vec_sub(v2, v1), qm_vec_sub(v3, v1));
	return qm_vec_mag(c, qm_vec3_len(c));
}

/// @brief 선에 가까운 점의 위치
INLINE QMVEC QM_VECTORCALL qm_vec3_closed(const QMVEC loc, const QMVEC begin, const QMVEC end)
{
	QMVEC norm = qm_vec_sub(end, begin);
	const float d = qm_vec3_len(norm);
	norm = qm_vec_mag(norm, 1.0f / d);
	const float t = qm_vec3_dot(norm, qm_vec_sub(loc, begin));
	if (t < 0.0f + QM_EPSILON)
		return begin;
	if (t > d)
		return end;
	return qm_vec_add(begin, qm_vec_mag(norm, t));
}

/// @brief 프로젝션
INLINE QMVEC QM_VECTORCALL qm_vec3_proj(const QMVEC v,
	const QMMAT proj, const QMMAT view, const QMMAT world,
	float x, float y, float width, float height, float zn, float zf)
{
	// UNDONE: 끄덕
	QN_DUMMY(v);
	QN_DUMMY(proj); QN_DUMMY(view); QN_DUMMY(world);
	QN_DUMMY(x); QN_DUMMY(y); QN_DUMMY(width); QN_DUMMY(height); QN_DUMMY(zn); QN_DUMMY(zf);
	return qm_vec_zero();
}

/// @brief 언프로젝션
INLINE QMVEC QM_VECTORCALL qm_vec3_unproj(const QMVEC v,
	const QMMAT proj, const QMMAT view, const QMMAT world,
	float x, float y, float width, float height, float zn, float zf)
{
	// UNDONE: 끄덕
	QN_DUMMY(v);
	QN_DUMMY(proj); QN_DUMMY(view); QN_DUMMY(world);
	QN_DUMMY(x); QN_DUMMY(y); QN_DUMMY(width); QN_DUMMY(height); QN_DUMMY(zn); QN_DUMMY(zf);
	return qm_vec_zero();
}

/// @brief 벡터3 내적
INLINE float QM_VECTORCALL qm_vec3_dot(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	QMVEC h = _mm_dp_ps(left, right, 0x71);
	return _mm_cvtss_f32(h);
#elif defined QM_USE_NEON
	QMVEC h = vmulq_f32(left, right);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vpadd_f32(v1, v1);
	v2 = vdups_lane_f32(v2, 0);
	v1 = vadd_f32(v1, v2);
	h = vcombine_f32(v1, v1);
	return vgetq_lane_f32(h, 0);
#else
	return left.f[0] * right.f[0] + left.f[1] * right.f[1] + left.f[2] * right.f[2];
#endif
}

/// @brief 벡터3 길이의 제곱
INLINE float QM_VECTORCALL qm_vec3_len_sq(const QMVEC v)
{
	return qm_vec3_dot(v, v);
}

/// @brief 벡터3 길이
INLINE float QM_VECTORCALL qm_vec3_len(const QMVEC v)
{
#ifdef QM_USE_SIMD
	QMVEC h = qm_vec_simd_sqrt(qm_vec3_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec3_len_sq(v));
#endif
}

/// @brief 벡터3 거리의 제곱
INLINE float QM_VECTORCALL qm_vec3_dist_sq(const QMVEC left, const QMVEC right)
{
	return qm_vec3_len_sq(qm_vec_sub(left, right));
}

/// @brief 벡터3 거리
INLINE float QM_VECTORCALL qm_vec3_dist(const QMVEC left, const QMVEC right)
{
#ifdef QM_USE_SIMD
	QMVEC v = qm_vec_sub(left, right);
	QMVEC h = qm_vec_simd_sqrt(qm_vec3_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec3_dist_sq(left, right));
#endif
}

/// @brief 두 벡터3의 반지름의 제곱
INLINE float QM_VECTORCALL qm_vec3_rad_sq(const QMVEC left, const QMVEC right)
{
	return qm_vec3_len_sq(qm_vec_sub(qm_vec_mag(qm_vec_add(left, right), 0.5f), left));
}

/// @brief 두 벡터3의 반지름
INLINE float QM_VECTORCALL qm_vec3_rad(const QMVEC left, const QMVEC right)
{
#ifdef QM_USE_SIMD
	QMVEC v = qm_vec_sub(qm_vec_mag(qm_vec_add(left, right), 0.5f), left);
	QMVEC h = qm_vec_simd_sqrt(qm_vec3_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec3_rad_sq(left, right));
#endif
}

/// @brief 두 벡터3 법선의 사이각
INLINE float QM_VECTORCALL qm_vec3_angle(const QMVEC left_normal, const QMVEC right_normal)
{
	const float dot = qm_vec3_dot(left_normal, right_normal);
	return acosf(qm_clampf(dot, -1.0f, 1.0f));
}

/// @brief 두 선분(벡터3) 사이에 벡터3이 있는지 조사
INLINE bool QM_VECTORCALL qm_vec3_is_between(const QMVEC query_point, const QMVEC begin, const QMVEC end)
{
	const float f = qm_vec3_len_sq(qm_vec_sub(end, begin));
	return qm_vec3_dist_sq(query_point, begin) <= f && qm_vec3_dist_sq(query_point, end) <= f;
}

/// @brief 벡터3 비교
INLINE bool QM_VECTORCALL qm_vec3_eq(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	QMVEC h = _mm_cmpeq_ps(left, right);
	return ((_mm_movemask_ps(h) & 7) == 7) != 0;
#elif defined QM_USE_NEON
	uint32x4_t v = vceqq_f32(left, right);
	uint8x8x2_t p = vzip_u8(vget_low_u8(vreinterpretq_u8_u32(v)), vget_high_u8(vreinterpretq_u8_u32(v)));
	uint16x4x2_t p2 = vzip_u16(vreinterpret_u16_u8(p.val[0]), vreinterpret_u16_u8(p.val[1]));
	return ((vget_lane_u32(vreinterpret_u32_u16(p2.val[1]), 1) & 0xFFFFFFU) == 0xFFFFFFU);
#else
	return left.f[0] == right.f[0] && left.f[1] == right.f[1] && left.f[2] == right.f[2];
#endif
}

/// @brief 벡터3 비교 (입실론)
INLINE bool QM_VECTORCALL qm_vec3_eps(const QMVEC left, const QMVEC right, float epsilon)
{
#if defined QM_USE_AVX
	QMVEC eps = _mm_set1_ps(epsilon);
	QMVEC v = _mm_sub_ps(left, right);
	QMVEC h = _mm_setzero_ps();
	h = _mm_sub_ps(h, v);
	h = _mm_max_ps(h, v);
	h = _mm_cmple_ps(h, eps);
	return ((_mm_movemask_ps(h) & 7) == 7) != 0;
#elif defined QM_USE_NEON
	float32x2_t d = vsub_f32(vget_low_f32(left), vget_low_f32(right));
	uint32x2_t t = vcle_f32(vabs_f32(d), vget_low_f32(vdup_n_f32(epsilon)));
	uint64_t r = vget_lane_u64(vreinterpret_u64_u32(t), 0);
	return r == 0xFFFFFFFFFFFFFFFFULL;
#else
	return
		qm_eqs(left.f[0], right.f[0], epsilon) &&
		qm_eqs(left.f[1], right.f[1], epsilon) &&
		qm_eqs(left.f[2], right.f[2], epsilon);
#endif
}

/// @brief 벡터3 비교 (입실론)
INLINE bool QM_VECTORCALL qm_vec3_near(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	QMVEC v = _mm_sub_ps(left, QMCONST_EPSILON.s);
	QMVEC h = _mm_setzero_ps();
	h = _mm_sub_ps(h, v);
	h = _mm_max_ps(h, v);
	h = _mm_cmple_ps(h, QMCONST_EPSILON.s);
	return ((_mm_movemask_ps(h) & 7) == 7) != 0;
#elif defined QM_USE_NEON
	float32x2_t d = vsub_f32(vget_low_f32(left), vget_low_f32(right));
	uint32x2_t t = vcle_f32(vabs_f32(d), vget_low_f32(QMCONST_EPSILON.s));
	uint64_t r = vget_lane_u64(vreinterpret_u64_u32(t), 0);
	return r == 0xFFFFFFFFFFFFFFFFULL;
#else
	return
		qm_eqs(left.f[0], right.f[0], QM_EPSILON) &&
		qm_eqs(left.f[1], right.f[1], QM_EPSILON) &&
		qm_eqs(left.f[2], right.f[2], QM_EPSILON);
#endif
}

/// @brief 벡터3 내적 벡터형
INLINE QMVEC QM_VECTORCALL qm_vec3_simd_dot(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_dp_ps(left, right, 0x71);
#elif defined QM_USE_NEON
	QMVEC h = vmulq_f32(left, right);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vpadd_f32(v1, v1);
	v2 = vdups_lane_f32(v2, 0);
	v1 = vadd_f32(v1, v2);
	return vcombine_f32(v1, v1);
#else
	const float f = left.f[0] * right.f[0] + left.f[1] * right.f[1] + left.f[2] * right.f[2];
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터3 길이의 제곱 벡터형
INLINE QMVEC QM_VECTORCALL qm_vec3_simd_len_sq(const QMVEC v)
{
	return qm_vec3_simd_dot(v, v);
}

/// @brief 벡터3 길이 벡터형
INLINE QMVEC QM_VECTORCALL qm_vec3_simd_len(const QMVEC v)
{
	return qm_vec_simd_sqrt(qm_vec3_simd_dot(v, v));
}


//////////////////////////////////////////////////////////////////////////
// vector4

/// @brief 벡터4 값 설정
INLINE QMVEC QM_VECTORCALL qm_vec4(float x, float y, float z, float w)
{
	return qm_vec(x, y, z, w);
}

/// @brief 벡터4 값 설정 (벡터3 + w)
INLINE QMVEC QM_VECTORCALL qm_vec4v(const QMVEC v, float w)
{
	return qm_vec_set_w(v, w);
}

/// @brief 벡터4 외적
INLINE QMVEC QM_VECTORCALL qm_vec4_cross(const QMVEC a, const QMVEC b, const QMVEC c)
{
#if defined QM_USE_AVX
	QMVEC u = _mm_permute_ps(b, _MM_SHUFFLE(2, 1, 3, 2));
	QMVEC p = _mm_permute_ps(c, _MM_SHUFFLE(1, 3, 2, 3));
	u = _mm_mul_ps(u, p);
	QMVEC o = _mm_permute_ps(b, _MM_SHUFFLE(1, 3, 2, 3));
	p = _mm_permute_ps(p, _MM_SHUFFLE(1, 3, 0, 1));
	u = _MM_FNMADD_PS(o, p, u);
	QMVEC n = _mm_permute_ps(a, _MM_SHUFFLE(0, 0, 0, 1));
	u = _mm_mul_ps(u, n);
	o = _mm_permute_ps(b, _MM_SHUFFLE(2, 0, 3, 1));
	p = _mm_permute_ps(c, _MM_SHUFFLE(0, 3, 0, 3));
	p = _mm_mul_ps(p, o);
	o = _mm_permute_ps(o, _MM_SHUFFLE(2, 1, 2, 1));
	n = _mm_permute_ps(c, _MM_SHUFFLE(2, 0, 3, 1));
	p = _MM_FNMADD_PS(o, n, p);
	n = _mm_permute_ps(a, _MM_SHUFFLE(1, 1, 2, 2));
	u = _MM_FNMADD_PS(n, p, u);
	o = _mm_permute_ps(b, _MM_SHUFFLE(1, 0, 2, 1));
	p = _mm_permute_ps(c, _MM_SHUFFLE(0, 1, 0, 2));
	p = _mm_mul_ps(p, o);
	o = _mm_permute_ps(o, _MM_SHUFFLE(2, 0, 2, 1));
	n = _mm_permute_ps(c, _MM_SHUFFLE(1, 0, 2, 1));
	p = _MM_FNMADD_PS(n, o, p);
	n = _mm_permute_ps(a, _MM_SHUFFLE(2, 3, 3, 3));
	return _MM_FMADD_PS(p, n, u);
#else
	QMVEC v;
	v.f[0] = a.f[1] * (b.f[2] * c.f[3] - c.f[2] * b.f[3]) - a.f[2] * (b.f[1] * c.f[3] - c.f[1] * b.f[3]) + a.f[3] * (b.f[1] * c.f[2] - b.f[2] * c.f[1]);
	v.f[1] = -(a.f[0] * (b.f[2] * c.f[3] - c.f[2] * b.f[3]) - a.f[2] * (b.f[0] * c.f[3] - c.f[0] * b.f[3]) + a.f[3] * (b.f[0] * c.f[2] - c.f[0] * b.f[2]));
	v.f[2] = a.f[0] * (b.f[1] * c.f[3] - c.f[1] * b.f[3]) - a.f[1] * (b.f[0] * c.f[3] - c.f[0] * b.f[3]) + a.f[3] * (b.f[0] * c.f[1] - c.f[0] * b.f[1]);
	v.f[3] = -(a.f[0] * (b.f[1] * c.f[2] - c.f[1] * b.f[2]) - a.f[1] * (b.f[0] * c.f[2] - c.f[0] * b.f[2]) + a.f[2] * (b.f[0] * c.f[1] - c.f[0] * b.f[1]));
	return v;
#endif
}

/// @brief 벡터4 법선
INLINE QMVEC QM_VECTORCALL qm_vec4_norm(const QMVEC v)
{
#if defined QM_USE_AVX
	QMVEC d = _mm_dp_ps(v, v, 0xFF);
	QMVEC h = _mm_rsqrt_ps(d);
	return _mm_mul_ps(v, h);
#elif defined QM_USE_NEON
	QMVEC h = vmulq_f32(v, v);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vadd_f32(v1, v2);
	v1 = vpadd_f32(v1, v1);
	v2 = vrsqrte_f32(v1);
	return vmulq_f32(v, vcombine_f32(v2, v2));
#else
	float l = qm_vec4_len(v);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec_mag(v, l);
#endif
}

/// @brief 벡터4 반사
INLINE QMVEC QM_VECTORCALL qm_vec4_reflect(const QMVEC incident, const QMVEC normal)
{
	QMVEC h = qm_vec4_simd_dot(incident, normal);
	h = qm_vec_add(h, h);
	return qm_vec_msub(normal, h, incident);
}

/// @brief 벡터4 굴절
INLINE QMVEC QM_VECTORCALL qm_vec4_refract(const QMVEC incident, const QMVEC normal, float eta)
{
	const float dot = qm_vec4_dot(incident, normal);
	const float k = 1.0f - eta * eta * (1.0f - dot * dot);
	if (k < 0.0f + QM_EPSILON)
		return qm_vec_zero();
	return qm_vec_msub(qm_vec_mag(incident, eta), qm_vec_mag(normal, eta * dot + qm_sqrtf(k)), incident);
}

/// @brief 벡터4을 행렬로 변환시킴
INLINE QMVEC QM_VECTORCALL qm_vec4_trfm(const QMVEC v, const QMMAT m)
{
#if defined QM_USE_AVX
	QMVEC h = _mm_mul_ps(_MM_PERMUTE_PS(v, 0x00), m.r[0]);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(v, 0x55), m.r[1], h);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(v, 0xAA), m.r[2], h);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(v, 0xFF), m.r[3], h);
	return h;
#else
	const float x = v.f[0] * m._11 + v.f[1] * m._21 + v.f[2] * m._31 + v.f[3] * m._41;
	const float y = v.f[0] * m._12 + v.f[1] * m._22 + v.f[2] * m._32 + v.f[3] * m._42;
	const float z = v.f[0] * m._13 + v.f[1] * m._23 + v.f[2] * m._33 + v.f[3] * m._43;
	const float w = v.f[0] * m._14 + v.f[1] * m._24 + v.f[2] * m._34 + v.f[3] * m._44;
	return qm_vec(x, y, z, w);
#endif
}

/// @brief 벡터4 내적
INLINE float QM_VECTORCALL qm_vec4_dot(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	QMVEC h = _mm_dp_ps(left, right, 0xFF);
	return _mm_cvtss_f32(h);
#elif defined QM_USE_NEON
	QMVEC h = vmulq_f32(left, right);
	h = vpaddq_f32(h, h);
	h = vpaddq_f32(h, h);
	return vgetq_lane_f32(h, 0);
#else
	return left.f[0] * right.f[0] + left.f[1] * right.f[1] + left.f[2] * right.f[2] + left.f[3] * right.f[3];
#endif
}

/// @brief 벡터4 길이의 제곱
INLINE float QM_VECTORCALL qm_vec4_len_sq(const QMVEC v)
{
	return qm_vec4_dot(v, v);
}

/// @brief 벡터4 길이
INLINE float QM_VECTORCALL qm_vec4_len(const QMVEC v)
{
#ifdef QM_USE_SIMD
	QMVEC h = qm_vec_simd_sqrt(qm_vec4_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec4_len_sq(v));
#endif
}

/// @brief 벡터4 거리의 제곱
INLINE float QM_VECTORCALL qm_vec4_dist_sq(const QMVEC left, const QMVEC right)
{
	return qm_vec4_len_sq(qm_vec_sub(left, right));
}

/// @brief 벡터4 거리
INLINE float QM_VECTORCALL qm_vec4_dist(const QMVEC left, const QMVEC right)
{
#ifdef QM_USE_SIMD
	QMVEC v = qm_vec_sub(left, right);
	QMVEC h = qm_vec_simd_sqrt(qm_vec4_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec4_dist_sq(left, right));
#endif
}

/// @brief 벡터4 비교
INLINE bool QM_VECTORCALL qm_vec4_eq(const QMVEC left, const QMVEC right)
{
	return qm_vec_eq(left, right);
}

/// @brief 벡터4 비교 (입실론)
INLINE bool QM_VECTORCALL qm_vec4_eps(const QMVEC left, const QMVEC right, float epsilon)
{
	return qm_vec_eps(left, right, epsilon);
}

/// @brief 벡터4 내적 벡터형
INLINE QMVEC QM_VECTORCALL qm_vec4_simd_dot(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	return _mm_dp_ps(left, right, 0xFF);
#elif defined QM_USE_NEON
	QMVEC h = vmulq_f32(left, right);
	h = vpaddq_f32(h, h);
	h = vpaddq_f32(h, h);
	return h;
#else
	const float f = left.f[0] * right.f[0] + left.f[1] * right.f[1] + left.f[2] * right.f[2] + left.f[3] * right.f[3];
	return (QMVEC) { { f, f, f, f } };
#endif
}

/// @brief 벡터4 길이의 제곱 벡터형
INLINE QMVEC QM_VECTORCALL qm_vec4_simd_len_sq(const QMVEC v)
{
	return qm_vec4_simd_dot(v, v);
}

/// @brief 벡터4 길이 벡터형
INLINE QMVEC QM_VECTORCALL qm_vec4_simd_len(const QMVEC v)
{
	return qm_vec_simd_sqrt(qm_vec4_simd_dot(v, v));
}


//////////////////////////////////////////////////////////////////////////
// quaternion

#define qm_quat_len_sq	qm_vec4_len_sq		/// @brief 사원수 길이의 제곱
#define qm_quat_len		qm_vec4_len			/// @brief 사원수 길이
#define qm_quat_eq		qm_vec_eq			/// @brief 사원수 비교
#define qm_quat_eps		qm_vec_eps			/// @brief 사원수 비교 (입실론)

/// @brief 사원수 값 설정
INLINE QMVEC QM_VECTORCALL qm_quat(float x, float y, float z, float w)
{
	return qm_vec(x, y, z, w);
}

/// @brief 사원수 값 설정 (벡터3 + w)
INLINE QMVEC QM_VECTORCALL qm_quatv(const QMVEC v, float w)
{
	return qm_vec_set_w(v, w);
}

/// @brief 단위 사원수
INLINE QMVEC QM_VECTORCALL qm_quat_unit(void)
{
	return QMCONST_UNIT_R3.s;
}

/// @brief 사원수 곱셈
INLINE QMVEC QM_VECTORCALL qm_quat_mul(const QMVEC left, const QMVEC right)
{
#if defined QM_USE_AVX
	// TDDO: 값이 맞는지 검증해야함
	QMVEC h = _mm_mul_ps(_MM_PERMUTE_PS(left, 0x00), right);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(left, 0x55), _MM_PERMUTE_PS(right, 0xEA), h);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(left, 0xAA), _MM_PERMUTE_PS(right, 0xD4), h);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(left, 0xFF), _MM_PERMUTE_PS(right, 0x1B), h);
	return h;
#else
	const float x = left.f[3] * right.f[0] + left.f[0] * right.f[3] + left.f[1] * right.f[2] - left.f[2] * right.f[1];
	const float y = left.f[3] * right.f[1] - left.f[0] * right.f[2] + left.f[1] * right.f[3] + left.f[2] * right.f[0];
	const float z = left.f[3] * right.f[2] + left.f[0] * right.f[1] - left.f[1] * right.f[0] + left.f[2] * right.f[3];
	const float w = left.f[3] * right.f[3] - left.f[0] * right.f[0] - left.f[1] * right.f[1] - left.f[2] * right.f[2];
	return qm_vec(x, y, z, w);
#endif
}

/// @brief 사원수 법선
INLINE QMVEC QM_VECTORCALL qm_quat_norm(const QMVEC q)
{
	return qm_vec4_norm(q);
}

/// @brief 켤레 사원수
INLINE QMVEC QM_VECTORCALL qm_quat_cjg(const QMVEC q)
{
#if defined QM_USE_AVX
	static const QmVec4 conjugate_mask = { { -1.0f, -1.0f, -1.0f, 1.0f } };
	return _mm_mul_ps(q, conjugate_mask.s);
#elif defined QM_USE_NEON
	static const QmVec4 conjugate_mask = { { -1.0f, -1.0f, -1.0f, 1.0f } };
	return vmulq_f32(q, conjugate_mask.s);
#else
	return (QMVEC) { { -q.f[0], -q.f[1], -q.f[2], q.f[3] } };
#endif
}

/// @brief 역사원수
INLINE QMVEC QM_VECTORCALL qm_quat_inv(const QMVEC q)
{
	const QMVEC l = qm_vec4_simd_dot(q, q);
	const QMVEC c = qm_quat_cjg(q);
	const QMVEC d = qm_vec_div(c, l);
	return qm_vec_select(d, QMCONST_ZERO.s, qm_vec_op_leq(l, QMCONST_EPSILON.s));
}

/// @brief 지수 사원수
INLINE QMVEC QM_VECTORCALL qm_quat_exp(const QMVEC q)
{
	const QMVEC t = qm_vec3_simd_len(q);
	QMVEC s, c;
	qm_vec_simd_sincos(t, &s, &c);
	const QMVEC p = qm_vec_div(s, t);
	QMVEC h = qm_vec_mul(q, p);
	const QMVEC o = qm_vec_op_eps(t, QMCONST_ZERO.s, QMCONST_EPSILON.s);
	h = qm_vec_select(h, q, o);
	h = qm_vec_select(o, h, QMCONST_S1110.s);
	return h;
}

/// @brief 로그 사원수
INLINE QMVEC QM_VECTORCALL qm_quat_ln(const QMVEC q)
{
	static const QmVec4 ome = { { 1.0f - QM_EPSILON, 1.0f - QM_EPSILON, 1.0f - QM_EPSILON, 1.0f - QM_EPSILON } };
	const QMVEC w = qm_vec_sp_w(q);
	const QMVEC z = qm_vec_select(QMCONST_S1110.s, q, QMCONST_S1110.s);
	const QMVEC c = qm_vec_op_in_bound(w, ome.s);
	const QMVEC t = qm_vec_simd_acos(w);
	const QMVEC s = qm_vec_simd_sin(t);
	const QMVEC p = qm_vec_div(t, s);
	const QMVEC h = qm_vec_mul(q, p);
	return qm_vec_select(z, h, c);
}

/// @brief 사원수 구형 보간 (이거 뭔가 좀 미묘해)
INLINE QMVEC QM_VECTORCALL qm_quat_slerp(const QMVEC left, const QMVEC right, float scale)
{
	float d = qm_quat_dot(left, right);
	QMVEC r;
	if (d < 0.0f - QM_EPSILON)
		d = -d, r = qm_vec_neg(right);
	else
		r = right;
	if (d < 0.9995f)
		return qm_vec_lerp(left, r, scale);
	const float theta = acosf(d);
	const float ls = sinf(theta * (1.0f - scale));
	const float rs = sinf(theta * scale);
	const QMVEC h = qm_vec_blend(left, ls, r, rs);
	return qm_quat_norm(h);
}

/// @brief DirectXMath용 사원수 구형 보간
INLINE QMVEC QM_VECTORCALL qm_quat_slerp_dxm(const QMVEC Q0, const QMVEC Q1, float scale)
{
	const QMVEC OneMinusEpsilon = { { 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f } };
	const QMVEC NegativeOne = { { -1.0f, -1.0f, -1.0f, -1.0f } };

	const QMVEC t = qm_vec_sp(scale);
	QMVEC CosOmega = qm_quat_simd_dot(Q0, Q1);

	const QMVEC Zero = qm_vec_zero();
	QMVEC Control = qm_vec_op_lt(CosOmega, Zero);
	const QMVEC Sign = qm_vec_select(QMCONST_ONE.s, NegativeOne, Control);

	CosOmega = qm_vec_mul(CosOmega, Sign);

	Control = qm_vec_op_lt(CosOmega, OneMinusEpsilon);

	QMVEC SinOmega = qm_vec_msub(CosOmega, CosOmega, QMCONST_ONE.s);
	SinOmega = qm_vec_simd_sqrt(SinOmega);

	const QMVEC Omega = qm_vec_simd_atan2(SinOmega, CosOmega);

	QMVEC SignMask = qm_vec_sp_msb();
	QMVEC V01 = qm_vec_bit_shl(t, Zero, 2);
	SignMask = qm_vec_bit_shl(SignMask, Zero, 3);
	V01 = qm_vec_bit_xor(V01, SignMask);
	V01 = qm_vec_add(QMCONST_UNIT_R0.s, V01);

	const QMVEC InvSinOmega = qm_vec_rcp(SinOmega);

	QMVEC S0 = qm_vec_mul(V01, Omega);
	S0 = qm_vec_simd_sin(S0);
	S0 = qm_vec_mul(S0, InvSinOmega);

	S0 = qm_vec_select(V01, S0, Control);

	QMVEC S1 = qm_vec_sp_y(S0);
	S0 = qm_vec_sp_x(S0);

	S1 = qm_vec_mul(S1, Sign);

	QMVEC Result = qm_vec_mul(Q0, S0);
	Result = qm_vec_madd(Q1, S1, Result);

	return Result;
}

/// @brief 사원수 구형 사중 보간
INLINE QMVEC QM_VECTORCALL qm_quat_squad(const QMVEC q1, const QMVEC q2, const QMVEC q3, const QMVEC q4, float scale)
{
	const QMVEC s = qm_quat_slerp(q1, q2, scale);
	const QMVEC t = qm_quat_slerp(q3, q4, scale);
	return qm_quat_slerp(s, t, 2.0f * scale * (1.0f - scale));
}

// TODO: 사원수 구형 사중 보간용 사원수 만들기...라는 함수 (qm_quat_squad_setup)

/// @brief 사원수 질량 중심 좌표계 이동 계산
INLINE QMVEC QM_VECTORCALL qm_quat_barycentric(const QMVEC q1, const QMVEC q2, const QMVEC q3, float f, float g)
{
	const float s = f + g;
	if (s < QM_EPSILON && s > -QM_EPSILON)
		return q1;
	const QMVEC q12 = qm_quat_slerp(q1, q2, s);
	const QMVEC q13 = qm_quat_slerp(q1, q3, s);
	return qm_quat_slerp(q12, q13, g / s);
}

/// @brief 벡터로 회전 (롤/피치/요)
INLINE QMVEC QM_VECTORCALL qm_quat_rot_vec(const QMVEC rot3)
{
#if defined QM_USE_AVX && false
	static const QmVec4 sign = { { 1.0f, -1.0f, -1.0f, 1.0f } };
	QMVEC s, c;
	qm_vec_simd_sincos(_mm_mul_ps(rot3, QMCONST_HALF.s), &s, &c);
	QMVEC p0, y0, r0, p1, y1, r1;
	qm_vec_pmt_mask(p0, s, c, QMCONST_S0111.s, (_MM_SHUFFLE(0, 0, 0, 0)));
	qm_vec_pmt_mask(y0, s, c, QMCONST_S1011.s, (_MM_SHUFFLE(1, 1, 1, 1)));
	qm_vec_pmt_mask(r0, s, c, QMCONST_S1101.s, (_MM_SHUFFLE(2, 2, 2, 2)));
	qm_vec_pmt_mask(p1, s, c, QMCONST_S0111.s, (_MM_SHUFFLE(0, 0, 0, 0)));
	qm_vec_pmt_mask(y1, s, c, QMCONST_S1011.s, (_MM_SHUFFLE(1, 1, 1, 1)));
	qm_vec_pmt_mask(r1, s, c, QMCONST_S1101.s, (_MM_SHUFFLE(2, 2, 2, 2)));
	QMVEC o = _mm_mul_ps(p1, sign.s);
	QMVEC z = _mm_mul_ps(p0, y0);
	return _MM_FMADD_PS(_mm_mul_ps(o, y1), r1, _mm_mul_ps(z, r0));
#else
	QmVec4 s, c;
	qm_vec_simd_sincos(qm_vec_mul(rot3, QMCONST_HALF.s), &s.s, &c.s);
	const float pcyc = c.Y * c.Z;
	const float psyc = s.Y * c.Z;
	const float pcys = c.Y * s.Z;
	const float psys = s.Y * s.Z;
	return qm_vec(s.X * pcyc - c.X * psys, c.X * psyc + s.X * pcys, c.X * pcys + s.X * psyc, c.X * pcyc + s.X * psys);
#endif
}

/// @brief 벡터로 축 회전
INLINE QMVEC QM_VECTORCALL qm_quat_rot_axis(const QMVEC axis3, float angle)
{
	float s, c;
	qm_sincosf(angle * 0.5f, &s, &c);
	return qm_quatv(qm_vec_mag(qm_vec3_norm(axis3), s), c);
}

/// @brief 사원수를 X축 회전시킨다
INLINE QMVEC QM_VECTORCALL qm_quat_rot_x(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
	return qm_vec(s, 0.0f, 0.0f, c);
}

/// @brief 사원수를 Y축 회전시킨다
INLINE QMVEC QM_VECTORCALL qm_quat_rot_y(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
	return qm_vec(0.0f, s, 0.0f, c);
}

/// @brief 사원수를 Z축 회전시킨다
INLINE QMVEC QM_VECTORCALL qm_quat_rot_z(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
	return qm_vec(0.0f, 0.0f, s, c);
}

/// @brief 행렬로 회전
INLINE QMVEC QM_VECTORCALL qm_quat_rot_mat4(const QMMAT rot)
{
#if defined QM_USE_AVX
	static const QmVec4 PMMP = { { +1.0f, -1.0f, -1.0f, +1.0f } };
	static const QmVec4 MPMP = { { -1.0f, +1.0f, -1.0f, +1.0f } };
	static const QmVec4 MMPP = { { -1.0f, -1.0f, +1.0f, +1.0f } };
	QMVEC r0 = rot.r[0];
	QMVEC r1 = rot.r[1];
	QMVEC r2 = rot.r[2];
	QMVEC r00 = _MM_PERMUTE_PS(r0, _MM_SHUFFLE(0, 0, 0, 0));
	QMVEC r11 = _MM_PERMUTE_PS(r1, _MM_SHUFFLE(1, 1, 1, 1));
	QMVEC r22 = _MM_PERMUTE_PS(r2, _MM_SHUFFLE(2, 2, 2, 2));
	QMVEC r11mr00 = _mm_sub_ps(r11, r00);
	QMVEC x2gey2 = _mm_cmple_ps(r11mr00, QMCONST_ZERO.s);
	QMVEC r11pr00 = _mm_add_ps(r11, r00);
	QMVEC z2gew2 = _mm_cmple_ps(r11pr00, QMCONST_ZERO.s);
	QMVEC x2py2gez2pw2 = _mm_cmple_ps(r22, QMCONST_ZERO.s);
	QMVEC t0 = _MM_FMADD_PS(PMMP.s, r00, QMCONST_ONE.s);
	QMVEC t1 = _mm_mul_ps(MPMP.s, r11);
	QMVEC t2 = _MM_FMADD_PS(MMPP.s, r22, t0);
	QMVEC x2y2z2w2 = _mm_add_ps(t1, t2);
	t0 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1, 2, 2, 1));
	t1 = _mm_shuffle_ps(r1, r2, _MM_SHUFFLE(1, 0, 0, 0));
	t1 = _MM_PERMUTE_PS(t1, _MM_SHUFFLE(1, 3, 2, 0));
	QMVEC xyxzyz = _mm_add_ps(t0, t1);
	t0 = _mm_shuffle_ps(r2, r1, _MM_SHUFFLE(0, 0, 0, 1));
	t1 = _mm_shuffle_ps(r1, r0, _MM_SHUFFLE(1, 2, 2, 2));
	t1 = _MM_PERMUTE_PS(t1, _MM_SHUFFLE(1, 3, 2, 0));
	QMVEC xwywzw = _mm_sub_ps(t0, t1);
	xwywzw = _mm_mul_ps(MPMP.s, xwywzw);
	t0 = _mm_shuffle_ps(x2y2z2w2, xyxzyz, _MM_SHUFFLE(0, 0, 1, 0));
	t1 = _mm_shuffle_ps(x2y2z2w2, xwywzw, _MM_SHUFFLE(0, 2, 3, 2));
	t2 = _mm_shuffle_ps(xyxzyz, xwywzw, _MM_SHUFFLE(1, 0, 2, 1));
	QMVEC tensor0 = _mm_shuffle_ps(t0, t2, _MM_SHUFFLE(2, 0, 2, 0));
	QMVEC tensor1 = _mm_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 1, 1, 2));
	QMVEC tensor2 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(2, 0, 1, 0));
	QMVEC tensor3 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(1, 2, 3, 2));
	t0 = _mm_and_ps(x2gey2, tensor0);
	t1 = _mm_andnot_ps(x2gey2, tensor1);
	t0 = _mm_or_ps(t0, t1);
	t1 = _mm_and_ps(z2gew2, tensor2);
	t2 = _mm_andnot_ps(z2gew2, tensor3);
	t1 = _mm_or_ps(t1, t2);
	t0 = _mm_and_ps(x2py2gez2pw2, t0);
	t1 = _mm_andnot_ps(x2py2gez2pw2, t1);
	t2 = _mm_or_ps(t0, t1);
	t1 = _mm_dp_ps(t2, t2, 0xFF);
	t0 = _mm_sqrt_ps(t1);
	return _mm_div_ps(t2, t0);
#else
	const float diag = rot._11 + rot._22 + rot._33 + 1.0f;
	QMVEC q;
	if (diag > 0.0f)
	{
		float s = qm_sqrtf(diag) * 2.0f;
		const float rs = 1.0f / s;
		q = qm_vec((rot._23 - rot._32) * rs, (rot._31 - rot._13) * rs, (rot._12 - rot._21) * rs, 0.25f * s);
	}
	else
	{
		// 필요한 스케일 만들기
		float s = qm_sqrtf(1.0f + rot._11 - rot._22 - rot._33) * 2.0f;
		const float rs = 1.0f / s;
		if (rot._11 > rot._22 && rot._11 > rot._33)
			q = qm_vec(0.25f * s, (rot._12 + rot._21) * rs, (rot._31 + rot._13) * rs, (rot._23 - rot._32) * rs);
		else if (rot._22 > rot._33)
			q = qm_vec((rot._12 + rot._21) * rs, 0.25f * s, (rot._23 + rot._32) * rs, (rot._31 - rot._13) * rs);
		else
			q = qm_vec((rot._31 + rot._13) * rs, (rot._23 + rot._32) * rs, 0.25f * s, (rot._12 - rot._21) * rs);
	}
	const float d = qm_quat_len_sq(q);
	if (qm_eqf(d, 1.0f))
		return q;
	return qm_vec_mag(q, qm_rsqrtf(d));
#endif
}

/// @brief 벡터 사이각
INLINE QMVEC QM_VECTORCALL qm_quat_angle(const QMVEC v1, const QMVEC v2)
{
	const QMVEC t = qm_vec3_simd_dot(v1, v2);
	const QMVEC c = qm_vec3_cross(v1, v2);
	const QMVEC h = qm_vec_set_w(c, qm_vec_get_w(t) + 1.0f);
	const QMVEC l = qm_vec4_simd_len(h);
	return qm_vec_div(h, l);
}

/// @brief 사원수 내적
INLINE float QM_VECTORCALL qm_quat_dot(const QMVEC left, const QMVEC right)
{
	return qm_vec4_dot(left, right);
}

/// @brief 사원수를 축과 각도로
INLINE void QM_VECTORCALL qm_quat_to_axis_angle(const QMVEC q, QMVEC* axis, float* angle)
{
	*axis = q;
	*angle = acosf(qm_vec_get_w(q)) * 2.0f;
}

/// @brief 단위 사원수인가?
INLINE bool QM_VECTORCALL qm_quat_isu(const QMVEC v)
{
	return qm_vec_eq(v, QMCONST_UNIT_R3.s);
}

/// @brief 사원수 내적 벡터형
INLINE QMVEC QM_VECTORCALL qm_quat_simd_dot(const QMVEC left, const QMVEC right)
{
	return qm_vec4_simd_dot(left, right);
}


//////////////////////////////////////////////////////////////////////////
// plane

/// @brief 평면 값 설정
INLINE QMVEC QM_VECTORCALL qm_plane(float a, float b, float c, float d)
{
	return qm_vec(a, b, c, d);
}

/// @brief 평면 값 설정 (벡터3 + d)
INLINE QMVEC QM_VECTORCALL qm_planev(const QMVEC v, float d)
{
	return qm_vec_set_w(v, d);
}

/// @brief 점과 법선으로 면 만들기 (벡터3 + 벡터3)
INLINE QMVEC QM_VECTORCALL qm_planevv(const QMVEC v, const QMVEC normal)
{
	return qm_vec_set_w(v, -qm_vec3_dot(v, normal));
}

/// @brief 점 세개로 평면 만들기 (벡터3 + 벡터3 + 벡터3)
INLINE QMVEC QM_VECTORCALL qm_planevvv(const QMVEC v1, const QMVEC v2, const QMVEC v3)
{
	const QMVEC l = qm_vec_sub(v2, v1);
	const QMVEC r = qm_vec_sub(v3, v1);
	const QMVEC n = qm_vec3_norm(qm_vec3_cross(l, r));
	return qm_vec_set_w(n, -qm_vec3_dot(v1, n));
}

/// @brief 단위 평면
INLINE QMVEC QM_VECTORCALL qm_plane_unit(void)
{
	return QMCONST_UNIT_R3.s;
}

/// @brief 평면 정규화
INLINE QMVEC QM_VECTORCALL qm_plane_norm(const QMVEC plane)
{
#if defined QM_USE_AVX
	QMVEC d = _mm_dp_ps(plane, plane, 0x7F);
	QMVEC h = _mm_rsqrt_ps(d);
	h = _mm_mul_ps(plane, h);
	return h;
#else
	float l = qm_vec3_len(plane);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec_mag(plane, l);
#endif
}

/// @brief 평면 뒤집어서 정규화
INLINE QMVEC QM_VECTORCALL qm_plane_rnorm(const QMVEC plane)
{
#if defined QM_USE_AVX
	QMVEC d = _mm_dp_ps(plane, plane, 0x7F);
	QMVEC h = _mm_mul_ps(_mm_rsqrt_ps(d), QMCONST_NEG.s);
	h = _mm_mul_ps(plane, h);
	return h;
#else
	float l = qm_vec3_len(plane);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec_mag(plane, -l);
#endif
}

/// @brief 평면 트랜스폼
INLINE QMVEC QM_VECTORCALL qm_plane_trfm(const QMVEC plane, const QMMAT m)
{
	const QMVEC x = qm_vec_sp_x(plane);
	const QMVEC y = qm_vec_sp_y(plane);
	const QMVEC z = qm_vec_sp_z(plane);
	const QMVEC w = qm_vec_sp_w(plane);
	QMVEC h = qm_vec_mul(w, m.r[3]);
	h = qm_vec_madd(z, m.r[2], h);
	h = qm_vec_madd(y, m.r[1], h);
	h = qm_vec_madd(x, m.r[0], h);
	return h;
}

/// @brief 평면 내적
INLINE float QM_VECTORCALL qm_plane_dot(const QMVEC plane, const QMVEC v)
{
	return qm_vec4_dot(plane, v);
}

/// @brief 평면과 점의 내적 (= 평면과 점의 거리)
INLINE float QM_VECTORCALL qm_plane_dot_coord(const QMVEC plane, const QMVEC v)
{
	return qm_vec3_dot(plane, v) + qm_vec_get_w(plane);
}

/// @brief 평면과 점의 법선 내적
INLINE float QM_VECTORCALL qm_plane_dot_normal(const QMVEC plane, const QMVEC v)
{
	return qm_vec3_dot(plane, v);
}

/// @brief 평면과 점의 거리
INLINE float QM_VECTORCALL qm_plane_dist_vec3(const QMVEC plane, const QMVEC v)
{
	return qm_plane_dot_coord(plane, v);
}

/// @brief 평면과 선분의 거리
INLINE float QM_VECTORCALL qm_plane_dist_line(const QMVEC plane, const QMVEC line_point1, const QMVEC line_point2)
{
	const QMVEC v = qm_vec_sub(line_point2, line_point1);
	const float d = qm_vec3_dot(plane, v);
	const float n = qm_vec3_dot(plane, line_point1) + qm_vec_get_w(plane);
	if (qm_eqf(d, 0.0f))
		return n;
	return -n / d;
}

/// @brief 점과 면의 관계 (0이면 면위, 1이면 면 위쪽, -1이면 면 아래쪽)
INLINE int32_t QM_VECTORCALL qm_plane_relation(const QMVEC plane, const QMVEC v)
{
	const float d = qm_plane_dot_coord(plane, v);
	if (d < -QM_EPSILON)
		return -1; // 아래
	if (d > QM_EPSILON)
		return 1; // 위
	return 0; // 면위
}

/// @brief 면과 선의 충돌을 검사하고 충돌점을 구한다
INLINE bool QM_VECTORCALL qm_plane_intersect_line(const QMVEC plane, const QMVEC line_point1, const QMVEC line_point2, QMVEC* intersect_point)
{
	return qm_plane_intersect_line2(plane, line_point1, qm_vec_sub(line_point2, line_point1), intersect_point);
}

/// @brief 면과 선의 충돌을 검사하고 충돌점을 구한다 (선의 시작과 방향 사용)
INLINE bool QM_VECTORCALL qm_plane_intersect_line2(const QMVEC plane, const QMVEC loc, const QMVEC dir, QMVEC* intersect_point)
{
	const float d = qm_vec3_dot(plane, dir);
	if (qm_eqf(d, 0.0f))
		return false;
	if (intersect_point)
	{
		const float n = qm_vec3_dot(plane, loc) + qm_vec_get_w(plane);
		const QMVEC vd = qm_vec_mag(dir, -n / d);
		*intersect_point = qm_vec_add(loc, vd);
	}
	return true;
}

/// @brief 면과 면의 충돌을 검사하고 충돌점을 구한다
INLINE bool QM_VECTORCALL qm_plane_intersect_plane(const QMVEC plane1, const QMVEC plane2, QMVEC* loc, QMVEC* dir)
{
	const float f0 = qm_vec3_len(plane1);
	const float f1 = qm_vec3_len(plane2);
	const float f2 = qm_vec3_dot(plane1, plane2);
	const float det = f0 * f1 - f2 * f2;
	if (qm_eqf(det, 0.0f))
		return false;
	if (dir)
		*dir = qm_vec3_cross(plane1, plane2);
	if (loc)
	{
		const float rdet = 1.0f / det;
		const float w1 = qm_vec_get_w(plane1), w2 = qm_vec_get_w(plane2);
		const float fa = (f2 * w2 - f1 * w1) * rdet;
		const float fb = (f2 * w1 - f0 * w2) * rdet;
		*loc = qm_vec_add(qm_vec_mag(plane1, fa), qm_vec_mag(plane2, fb));
	}
	return true;
}

/// @brief 세 면의 충돌을 검사하고, 충돌 지점을 구한다
INLINE bool QM_VECTORCALL qm_plane_intersect_planes(const QMVEC plane1, const QMVEC plane2, const QMVEC plane3, QMVEC* intersect_point)
{
	QMVEC dir, loc;
	return qm_plane_intersect_plane(plane1, plane2, &loc, &dir) ? qm_plane_intersect_line2(plane3, loc, dir, intersect_point) : false;
}

/// @brief 평면 비교
INLINE bool QM_VECTORCALL qm_plane_eq(const QMVEC left, const QMVEC right)
{
	return qm_vec_eq(left, right);
}

/// @brief 평면 비교 (입실론)
INLINE bool QM_VECTORCALL qm_plane_eps(const QMVEC left, const QMVEC right, float epsilon)
{
	return qm_vec_eps(left, right, epsilon);
}

/// @brief 단위 평면인가?
INLINE bool QM_VECTORCALL qm_plane_isu(const QMVEC v)
{
	return qm_vec_eq(v, QMCONST_UNIT_R3.s);
}

/// @brief 평면 내적 벡터형
INLINE QMVEC QM_VECTORCALL qm_plane_simd_dot(const QMVEC plane, const QMVEC v)
{
	return qm_vec4_simd_dot(plane, v);
}

/// @brief 평면과 점의 내적 벡터형
INLINE QMVEC QM_VECTORCALL qm_plane_simd_dot_coord(const QMVEC plane, const QMVEC v)
{
	const QMVEC s = qm_vec_select(QMCONST_ONE.s, v, QMCONST_S1110.s);
	return qm_vec4_simd_dot(plane, s);
}

/// @brief 평면과 점의 법선 내적 벡터형
INLINE QMVEC QM_VECTORCALL qm_plane_simd_dot_normal(const QMVEC plane, const QMVEC v)
{
	return qm_vec3_simd_dot(plane, v);
}


//////////////////////////////////////////////////////////////////////////
// color4

#define qm_color_eq		qm_vec_eq			/// @brief 색깔 비교
#define qm_color_eps	qm_vec_eps			/// @brief 색깔 비교 (입실론)
#define qm_color_isu	qm_quat_isu			/// @brief 검은색에 알파는 1인가?

/// @brief 색상 값 설정
INLINE QMVEC QM_VECTORCALL qm_color(float r, float g, float b, float a)
{
	return qm_vec(r, g, b, a);
}

/// @brief 정수로 색깔 설정
INLINE QMVEC QM_VECTORCALL qm_coloru(uint32_t value)
{
	const float i = 1.0f / 255.0f;
	const float b = (float)(value & 255) * i; value >>= 8;
	const float g = (float)(value & 255) * i; value >>= 8;
	const float r = (float)(value & 255) * i; value >>= 8;
	const float a = (float)(value & 255) * i;
	return qm_vec(r, g, b, a);
}

/// @brief kolor 색깔 설정
INLINE QMVEC QM_VECTORCALL qm_colork(QmKolor k)
{
	const float i = 1.0f / 255.0f;
	return qm_vec((float)k.R * i, (float)k.G * i, (float)k.B * i, (float)k.A * i);
}

/// @brief 초기 색깔
INLINE QMVEC QM_VECTORCALL qm_color_unit(void)
{
	return QMCONST_UNIT_R3.s;
}

/// @brief 같은값 설정
INLINE QMVEC QM_VECTORCALL qm_color_sp(float value, float alpha)
{
	return qm_vec(value, value, value, alpha);
}

/// @brief 네거티브 색
INLINE QMVEC QM_VECTORCALL qm_color_neg(const QMVEC c)
{
#if defined QM_USE_AVX
	static const QmVec4 ONE3 = { { 1.0f, 1.0f, 1.0f, 0.0f } };
	static const QmVec4 NEG3 = { { -1.0f, -1.0f, -1.0f, 0.0f } };
	QMVEC t = _mm_xor_ps(c, NEG3.s);
	return _mm_add_ps(t, ONE3.s);
#elif defined QM_USE_NEON
	static const QmVec4 ONE3 = { { 1.0f, 1.0f, 1.0f, 0.0f } };
	static const QmVec4 NEG3 = { { -1.0f, -1.0f, -1.0f, 0.0f } };
	uint32x4_t t = veorq_u32(vreinterpretq_u32_f32(c), NEG3.s);
	return vaddq_f32(vreinterpretq_f32_u32(t), ONE3.s);
#else
	return qm_vec(1.0f - c.f[0], 1.0f - c.f[1], 1.0f - c.f[2], c.f[3]);
#endif
}

/// @brief 색깔 혼합
INLINE QMVEC QM_VECTORCALL qm_color_mod(const QMVEC left, const QMVEC right)
{
	return qm_vec_mul(left, right);
}

/// @brief 콘트라스트 조정
INLINE QMVEC QM_VECTORCALL qm_color_contrast(const QMVEC c, float contrast)
{
#if defined QM_USE_AVX
	QMVEC s = _mm_set_ps1(contrast);
	QMVEC h = _mm_sub_ps(c, QMCONST_HALF.s);
	h = _MM_FMADD_PS(h, s, QMCONST_HALF.s);
	s = _mm_shuffle_ps(h, c, _MM_SHUFFLE(3, 2, 2, 2));
	h = _mm_shuffle_ps(h, s, _MM_SHUFFLE(3, 0, 1, 0));
	return h;
#elif defined QM_USE_NEON
	QMVEC h = vsubq_f32(c, QMCONST_HALF.s);
	h = vmlaq_n_f32(half.s, h, contrast);
	h = vbslq_f32(QMCONST_S1110.s, h, QMCONST_HALF.s);
	return h;
#else
	const float r = 0.5f + contrast * (c.f[0] - 0.5f);
	const float g = 0.5f + contrast * (c.f[1] - 0.5f);
	const float b = 0.5f + contrast * (c.f[2] - 0.5f);
	return qm_color(r, g, b, c.f[3]);
#endif
}

/// @brief 새츄레이션 조정
INLINE QMVEC QM_VECTORCALL qm_color_saturate(const QMVEC c, float saturation)
{
	static const QmVec4 luminance = { { 0.2125f, 0.7154f, 0.0721f, 0.0f } };
#if defined QM_USE_AVX
	QMVEC l = qm_vec3_simd_dot(c, luminance.s);
	QMVEC s = _mm_set_ps1(saturation);
	QMVEC h = _mm_sub_ps(c, l);
	h = _MM_FMADD_PS(h, s, l);
	l = _mm_shuffle_ps(h, c, _MM_SHUFFLE(3, 2, 2, 2));
	h = _mm_shuffle_ps(h, l, _MM_SHUFFLE(3, 0, 1, 0));
	return h;
#elif defined QM_USE_NEON
	QMVEC l = qm_vec3_simd_dot(c, luminance);
	QMVEC h = vsubq_f32(c, l);
	h = vmlaq_n_f32(l, h, saturation);
	h = vbslq_f32(QMCONST_S1110.s, l, c);
	return h;
#else
	float l = qm_vec3_dot(c, luminance.s);
	const float r = ((c.f[0] - l) * saturation) + l;
	const float g = ((c.f[1] - l) * saturation) + l;
	const float b = ((c.f[2] - l) * saturation) + l;
	return qm_color(r, g, b, c.f[3]);
#endif
}

INLINE QmKolor QM_VECTORCALL qm_color_to_kolor(const QMVEC c)
{
	QmKolor k;
	qm_vec_st_kolor(&k, c);
	return k;
}


//////////////////////////////////////////////////////////////////////////
// matrix4x4

/// @brief 단위 행렬 (항등 행렬)
INLINE QMMAT QM_VECTORCALL qm_mat4_unit(void)
{
	QMMAT m;
	m.r[0] = QMCONST_UNIT_R0.s;
	m.r[1] = QMCONST_UNIT_R1.s;
	m.r[2] = QMCONST_UNIT_R2.s;
	m.r[3] = QMCONST_UNIT_R3.s;
	return m;
}

/// @brief 행렬을 0으로 초기화 한다
INLINE QMMAT QM_VECTORCALL qm_mat4_zero(void)
{
	QMMAT m;
	m.r[0] = QMCONST_ZERO.s;
	m.r[1] = QMCONST_ZERO.s;
	m.r[2] = QMCONST_ZERO.s;
	m.r[3] = QMCONST_ZERO.s;
	return m;
}

/// @brief 대각 행렬을 만든다
INLINE QMMAT QM_VECTORCALL qm_mat4_diag(float diag)
{
	QMMAT m;
	m.r[0] = qm_vec(diag, 0.0f, 0.0f, 0.0f);
	m.r[1] = qm_vec(0.0f, diag, 0.0f, 0.0f);
	m.r[2] = qm_vec(0.0f, 0.0f, diag, 0.0f);
	m.r[3] = qm_vec(0.0f, 0.0f, 0.0f, diag);
	return m;
}

/// @brief 두 행렬의 덧셈
INLINE QMMAT QM_VECTORCALL qm_mat4_add(const QMMAT left, const QMMAT right)
{
	QMMAT m;
	m.r[0] = qm_vec_add(left.r[0], right.r[0]);
	m.r[1] = qm_vec_add(left.r[1], right.r[1]);
	m.r[2] = qm_vec_add(left.r[2], right.r[2]);
	m.r[3] = qm_vec_add(left.r[3], right.r[3]);
	return m;
}

/// @brief 두 행렬의 뺄셈
INLINE QMMAT QM_VECTORCALL qm_mat4_sub(const QMMAT left, const QMMAT right)
{
	QMMAT m;
	m.r[0] = qm_vec_sub(left.r[0], right.r[0]);
	m.r[1] = qm_vec_sub(left.r[1], right.r[1]);
	m.r[2] = qm_vec_sub(left.r[2], right.r[2]);
	m.r[3] = qm_vec_sub(left.r[3], right.r[3]);
	return m;
}

/// @brief 행렬의 확대
INLINE QMMAT QM_VECTORCALL qm_mat4_mag(const QMMAT left, float right)
{
	QMMAT m;
#if defined QM_USE_AVX
	QMVEC mm = _mm_set1_ps(right);
	m.r[0] = _mm_mul_ps(left.r[0], mm);
	m.r[1] = _mm_mul_ps(left.r[1], mm);
	m.r[2] = _mm_mul_ps(left.r[2], mm);
	m.r[3] = _mm_mul_ps(left.r[3], mm);
#elif defined QM_USE_NEON
	m.r[0] = vmulq_n_f32(left.r[0], right);
	m.r[1] = vmulq_n_f32(left.r[1], right);
	m.r[2] = vmulq_n_f32(left.r[2], right);
	m.r[3] = vmulq_n_f32(left.r[3], right);
#else
	m.r[0] = qm_vec_mag(left.r[0], right);
	m.r[1] = qm_vec_mag(left.r[1], right);
	m.r[2] = qm_vec_mag(left.r[2], right);
	m.r[3] = qm_vec_mag(left.r[3], right);
#endif
	return m;
}

/// @brief 행렬의 줄이기
INLINE QMMAT QM_VECTORCALL qm_mat4_abr(const QMMAT left, float right)
{
	QMMAT m;
#if defined QM_USE_AVX
	QMVEC mm = _mm_set1_ps(right);
	m.r[0] = _mm_div_ps(left.r[0], mm);
	m.r[1] = _mm_div_ps(left.r[1], mm);
	m.r[2] = _mm_div_ps(left.r[2], mm);
	m.r[3] = _mm_div_ps(left.r[3], mm);
#elif defined QM_USE_NEON
	QMVEC mm = vdupq_n_f32(right);
	m.r[0] = vdivq_f32(left.r[0], right);
	m.r[1] = vdivq_f32(left.r[1], right);
	m.r[2] = vdivq_f32(left.r[2], right);
	m.r[3] = vdivq_f32(left.r[3], right);
#else
	m.r[0] = qm_vec_abr(left.r[0], right);
	m.r[1] = qm_vec_abr(left.r[1], right);
	m.r[2] = qm_vec_abr(left.r[2], right);
	m.r[3] = qm_vec_abr(left.r[3], right);
#endif
	return m;
}

/// @brief 행렬 전치
INLINE QMMAT QM_VECTORCALL qm_mat4_tran(const QMMAT m)
{
#if defined QM_USE_AVX
	QMVEC r0 = _mm_shuffle_ps(m.r[0], m.r[1], 0x44);
	QMVEC r2 = _mm_shuffle_ps(m.r[0], m.r[1], 0xEE);
	QMVEC r1 = _mm_shuffle_ps(m.r[2], m.r[3], 0x44);
	QMVEC r3 = _mm_shuffle_ps(m.r[2], m.r[3], 0xEE);
	QMMAT h;
	h.r[0] = _mm_shuffle_ps(r0, r1, 0x88);
	h.r[1] = _mm_shuffle_ps(r0, r1, 0xDD);
	h.r[2] = _mm_shuffle_ps(r2, r3, 0x88);
	h.r[3] = _mm_shuffle_ps(r2, r3, 0xDD);
	return h;
#elif defined QM_USE_NEON
	float32x4x4_t t = vld4q_f32(m.f);
	return (QMMAT) { .r[0] = t.val[0], .r[1] = t.val[1], .r[2] = t.val[2], .r[3] = t.val[3] };
#else
	return (QMMAT) { { m._11, m._21, m._31, m._41, m._12, m._22, m._32, m._42, m._13, m._23, m._33, m._43, m._14, m._24, m._34, m._44 } };
#endif
}

/// @brief 행렬 곱셈
INLINE QMMAT QM_VECTORCALL qm_mat4_mul(const QMMAT left, const QMMAT right)
{
#if defined QM_USE_AVX2
	__m256 t0 = _mm256_castps128_ps256(left.r[0]); t0 = _mm256_insertf128_ps(t0, left.r[1], 1);
	__m256 t1 = _mm256_castps128_ps256(left.r[2]); t1 = _mm256_insertf128_ps(t1, left.r[3], 1);
	__m256 u0 = _mm256_castps128_ps256(right.r[0]); u0 = _mm256_insertf128_ps(u0, right.r[1], 1);
	__m256 u1 = _mm256_castps128_ps256(right.r[2]); u1 = _mm256_insertf128_ps(u1, right.r[3], 1);
	__m256 a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(0, 0, 0, 0));
	__m256 a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(0, 0, 0, 0));
	__m256 b0 = _mm256_permute2f128_ps(u0, u0, 0x00);
	__m256 c0 = _mm256_mul_ps(a0, b0);
	__m256 c1 = _mm256_mul_ps(a1, b0);
	a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(1, 1, 1, 1));
	a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(1, 1, 1, 1));
	b0 = _mm256_permute2f128_ps(u0, u0, 0x11);
	__m256 c2 = _mm256_fmadd_ps(a0, b0, c0);
	__m256 c3 = _mm256_fmadd_ps(a1, b0, c1);
	a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(2, 2, 2, 2));
	a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2));
	__m256 b1 = _mm256_permute2f128_ps(u1, u1, 0x00);
	__m256 c4 = _mm256_mul_ps(a0, b1);
	__m256 c5 = _mm256_mul_ps(a1, b1);
	a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(3, 3, 3, 3));
	a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 3, 3, 3));
	b1 = _mm256_permute2f128_ps(u1, u1, 0x11);
	__m256 c6 = _mm256_fmadd_ps(a0, b1, c4);
	__m256 c7 = _mm256_fmadd_ps(a1, b1, c5);
	t0 = _mm256_add_ps(c2, c6);
	t1 = _mm256_add_ps(c3, c7);
	QMMAT m;
	m.r[0] = _mm256_castps256_ps128(t0);
	m.r[1] = _mm256_extractf128_ps(t0, 1);
	m.r[2] = _mm256_castps256_ps128(t1);
	m.r[3] = _mm256_extractf128_ps(t1, 1);
	return m;
#elif defined QM_USE_AVX
	QMMAT m;
	QMVEC vW = left.r[0];
	QMVEC vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	QMVEC vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	QMVEC vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.r[0]);
	vY = _mm_mul_ps(vY, right.r[1]);
	vZ = _mm_mul_ps(vZ, right.r[2]);
	vW = _mm_mul_ps(vW, right.r[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	m.r[0] = vX;
	vW = left.r[1];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.r[0]);
	vY = _mm_mul_ps(vY, right.r[1]);
	vZ = _mm_mul_ps(vZ, right.r[2]);
	vW = _mm_mul_ps(vW, right.r[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	m.r[1] = vX;
	vW = left.r[2];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.r[0]);
	vY = _mm_mul_ps(vY, right.r[1]);
	vZ = _mm_mul_ps(vZ, right.r[2]);
	vW = _mm_mul_ps(vW, right.r[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	m.r[2] = vX;
	vW = left.r[3];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.r[0]);
	vY = _mm_mul_ps(vY, right.r[1]);
	vZ = _mm_mul_ps(vZ, right.r[2]);
	vW = _mm_mul_ps(vW, right.r[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	m.r[3] = vX;
	return m;
#elif defined QM_USE_NEON
	QMVEC l0 = left.r[0];
	QMVEC l1 = left.r[1];
	QMVEC l2 = left.r[2];
	QMVEC l3 = left.r[3];
	QMMAT m;
	for (int32_t i = 0; i < 4; i++)
	{
		QMVEC v, r = right.r[i];
		v = vmulq_n_f32(l0, vgetq_lane_f32(r, 0));
		v = vmlaq_n_f32(v, l1, vgetq_lane_f32(r, 1));
		v = vmlaq_n_f32(v, l2, vgetq_lane_f32(r, 2));
		v = vmlaq_n_f32(v, l3, vgetq_lane_f32(r, 3));
		m.r[i] = v;
	}
	return m;
#else
	QMMAT m;
	m.r[0] = qm_vec4_trfm(right.r[0], left);
	m.r[1] = qm_vec4_trfm(right.r[1], left);
	m.r[2] = qm_vec4_trfm(right.r[2], left);
	m.r[3] = qm_vec4_trfm(right.r[3], left);
	return m;
#endif
}

/// @brief 행렬의 전치곱
INLINE QMMAT QM_VECTORCALL qm_mat4_tmul(const QMMAT left, const QMMAT right)
{
#if defined QM_USE_AVX2
	__m256 t0 = _mm256_castps128_ps256(left.r[0]); t0 = _mm256_insertf128_ps(t0, left.r[1], 1);
	__m256 t1 = _mm256_castps128_ps256(left.r[2]); t1 = _mm256_insertf128_ps(t1, left.r[3], 1);
	__m256 u0 = _mm256_castps128_ps256(right.r[0]); u0 = _mm256_insertf128_ps(u0, right.r[1], 1);
	__m256 u1 = _mm256_castps128_ps256(right.r[2]); u1 = _mm256_insertf128_ps(u1, right.r[3], 1);
	__m256 a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(0, 0, 0, 0));
	__m256 a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(0, 0, 0, 0));
	__m256 b0 = _mm256_permute2f128_ps(u0, u0, 0x00);
	__m256 c0 = _mm256_mul_ps(a0, b0);
	__m256 c1 = _mm256_mul_ps(a1, b0);
	a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(1, 1, 1, 1));
	a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(1, 1, 1, 1));
	b0 = _mm256_permute2f128_ps(u0, u0, 0x11);
	__m256 c2 = _mm256_fmadd_ps(a0, b0, c0);
	__m256 c3 = _mm256_fmadd_ps(a1, b0, c1);
	a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(2, 2, 2, 2));
	a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2));
	__m256 b1 = _mm256_permute2f128_ps(u1, u1, 0x00);
	__m256 c4 = _mm256_mul_ps(a0, b1);
	__m256 c5 = _mm256_mul_ps(a1, b1);
	a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(3, 3, 3, 3));
	a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 3, 3, 3));
	b1 = _mm256_permute2f128_ps(u1, u1, 0x11);
	__m256 c6 = _mm256_fmadd_ps(a0, b1, c4);
	__m256 c7 = _mm256_fmadd_ps(a1, b1, c5);
	t0 = _mm256_add_ps(c2, c6);
	t1 = _mm256_add_ps(c3, c7);
	// 전치
	__m256 p = _mm256_unpacklo_ps(t0, t1);
	__m256 p2 = _mm256_unpackhi_ps(t0, t1);
	__m256 p3 = _mm256_permute2f128_ps(p, p2, 0x20);
	__m256 p4 = _mm256_permute2f128_ps(p, p2, 0x31);
	p = _mm256_unpacklo_ps(p3, p4);
	p2 = _mm256_unpackhi_ps(p3, p4);
	t0 = _mm256_permute2f128_ps(p, p2, 0x20);
	t1 = _mm256_permute2f128_ps(p, p2, 0x31);
	QMMAT m;
	m.r[0] = _mm256_castps256_ps128(t0);
	m.r[1] = _mm256_extractf128_ps(t0, 1);
	m.r[2] = _mm256_castps256_ps128(t1);
	m.r[3] = _mm256_extractf128_ps(t1, 1);
	return m;
#elif defined QM_USE_AVX
	QMVEC vW = left.r[0];
	QMVEC vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	QMVEC vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	QMVEC vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.r[0]);
	vY = _mm_mul_ps(vY, right.r[1]);
	vZ = _mm_mul_ps(vZ, right.r[2]);
	vW = _mm_mul_ps(vW, right.r[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	QMVEC r0 = vX;
	vW = left.r[1];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.r[0]);
	vY = _mm_mul_ps(vY, right.r[1]);
	vZ = _mm_mul_ps(vZ, right.r[2]);
	vW = _mm_mul_ps(vW, right.r[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	QMVEC r1 = vX;
	vW = left.r[2];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.r[0]);
	vY = _mm_mul_ps(vY, right.r[1]);
	vZ = _mm_mul_ps(vZ, right.r[2]);
	vW = _mm_mul_ps(vW, right.r[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	QMVEC r2 = vX;
	vW = left.r[3];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.r[0]);
	vY = _mm_mul_ps(vY, right.r[1]);
	vZ = _mm_mul_ps(vZ, right.r[2]);
	vW = _mm_mul_ps(vW, right.r[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	QMVEC r3 = vX;
	// 전치
	QMVEC p1 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1, 0, 1, 0));
	QMVEC p3 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(3, 2, 3, 2));
	QMVEC p2 = _mm_shuffle_ps(r2, r3, _MM_SHUFFLE(1, 0, 1, 0));
	QMVEC p4 = _mm_shuffle_ps(r2, r3, _MM_SHUFFLE(3, 2, 3, 2));
	QMMAT m;
	m.r[0] = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(2, 0, 2, 0));
	m.r[1] = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(3, 1, 3, 1));
	m.r[2] = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(2, 0, 2, 0));
	m.r[3] = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(3, 1, 3, 1));
	return m;
#else
	return qm_mat4_tran(qm_mat4_mul(left, right));
#endif
}

/// @brief 역행렬
INLINE QMMAT QM_VECTORCALL qm_mat4_inv(const QMMAT m)
{
#if defined QM_USE_AVX
	static const QmVecU PNNP = { { 0x00000000, 0x80000000, 0x80000000, 0x00000000 } };
	QMVEC a, b, c, d;
	QMVEC ia, ib, ic, id;
	QMVEC dc, ab;
	QMVEC ma, mb, mc, md;
	QMVEC dt, d0, d1, d2;
	QMVEC rd;
	a = _mm_movelh_ps(m.r[0], m.r[1]);
	b = _mm_movehl_ps(m.r[1], m.r[0]);
	c = _mm_movelh_ps(m.r[2], m.r[3]);
	d = _mm_movehl_ps(m.r[3], m.r[2]);
	ab = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x0F), b);
	ab = _mm_sub_ps(ab, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xA5), _mm_shuffle_ps(b, b, 0x4E)));
	dc = _mm_mul_ps(_mm_shuffle_ps(d, d, 0x0F), c);
	dc = _mm_sub_ps(dc, _mm_mul_ps(_mm_shuffle_ps(d, d, 0xA5), _mm_shuffle_ps(c, c, 0x4E)));
	ma = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x5F), a);
	ma = _mm_sub_ss(ma, _mm_movehl_ps(ma, ma));
	mb = _mm_mul_ps(_mm_shuffle_ps(b, b, 0x5F), b);
	mb = _mm_sub_ss(mb, _mm_movehl_ps(mb, mb));
	mc = _mm_mul_ps(_mm_shuffle_ps(c, c, 0x5F), c);
	mc = _mm_sub_ss(mc, _mm_movehl_ps(mc, mc));
	md = _mm_mul_ps(_mm_shuffle_ps(d, d, 0x5F), d);
	md = _mm_sub_ss(md, _mm_movehl_ps(md, md));
	d0 = _mm_mul_ps(_mm_shuffle_ps(dc, dc, 0xD8), ab);
	id = _mm_mul_ps(_mm_shuffle_ps(c, c, 0xA0), _mm_movelh_ps(ab, ab));
	id = _mm_add_ps(id, _mm_mul_ps(_mm_shuffle_ps(c, c, 0xF5), _mm_movehl_ps(ab, ab)));
	ia = _mm_mul_ps(_mm_shuffle_ps(b, b, 0xA0), _mm_movelh_ps(dc, dc));
	ia = _mm_add_ps(ia, _mm_mul_ps(_mm_shuffle_ps(b, b, 0xF5), _mm_movehl_ps(dc, dc)));
	d0 = _mm_add_ps(d0, _mm_movehl_ps(d0, d0));
	d0 = _mm_add_ss(d0, _mm_shuffle_ps(d0, d0, 1));
	d1 = _mm_mul_ps(ma, md);
	d2 = _mm_mul_ps(mb, mc);
	id = _mm_sub_ps(_mm_mul_ps(d, _mm_shuffle_ps(ma, ma, 0)), id);
	ia = _mm_sub_ps(_mm_mul_ps(a, _mm_shuffle_ps(md, md, 0)), ia);
	dt = _mm_sub_ss(_mm_add_ss(d1, d2), d0);
	rd = _mm_div_ss(_mm_set_ss(1.0f), dt);
	// ZERO_SINGULAR
	//rd=_mm_and_ps(_mm_cmpneq_ss(det,_mm_setzero_ps()),rd);
	ib = _mm_mul_ps(d, _mm_shuffle_ps(ab, ab, 0x33));
	ib = _mm_sub_ps(ib, _mm_mul_ps(_mm_shuffle_ps(d, d, 0xB1), _mm_shuffle_ps(ab, ab, 0x66)));
	ic = _mm_mul_ps(a, _mm_shuffle_ps(dc, dc, 0x33));
	ic = _mm_sub_ps(ic, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xB1), _mm_shuffle_ps(dc, dc, 0x66)));
	rd = _mm_shuffle_ps(rd, rd, 0);
	rd = _mm_xor_ps(rd, PNNP.s);
	ib = _mm_sub_ps(_mm_mul_ps(c, _mm_shuffle_ps(mb, mb, 0)), ib);
	ic = _mm_sub_ps(_mm_mul_ps(b, _mm_shuffle_ps(mc, mc, 0)), ic);
	ia = _mm_div_ps(ia, rd);
	ib = _mm_div_ps(ib, rd);
	ic = _mm_div_ps(ic, rd);
	id = _mm_div_ps(id, rd);
	QMMAT h;
	h.r[0] = _mm_shuffle_ps(ia, ib, 0x77);
	h.r[1] = _mm_shuffle_ps(ia, ib, 0x22);
	h.r[2] = _mm_shuffle_ps(ic, id, 0x77);
	h.r[3] = _mm_shuffle_ps(ic, id, 0x22);
	return m;
	// 행렬식: *(float*)&dt
#elif defined QM_USE_NEON
	QMVEC row1 = m.r[0];
	QMVEC row2 = m.r[1];
	QMVEC row3 = m.r[2];
	QMVEC row4 = m.r[3];
	QMVEC minor0, minor1, minor2, minor3;
	QMVEC det, tmp1;
	tmp1 = vmulq_lane_f32(row1, vget_high_f32(row2), 1);
	minor0 = vmulq_lane_f32(row3, vget_low_f32(row4), 0);
	minor0 = vmlsq_lane_f32(minor0, row3, vget_high_f32(row4), 1);
	minor0 = vmlaq_lane_f32(minor0, row2, vget_low_f32(row4), 1);
	minor0 = vmlsq_lane_f32(minor0, tmp1, vget_low_f32(row3), 0);
	minor1 = vmulq_lane_f32(row0, vget_high_f32(row2), 1);
	minor1 = vmlsq_lane_f32(minor1, row3, vget_high_f32(row4), 0);
	minor1 = vmlaq_lane_f32(minor1, row3, vget_low_f32(row4), 1);
	minor1 = vmlsq_lane_f32(minor1, row2, vget_low_f32(row4), 1);
	minor1 = vmlsq_lane_f32(minor1, tmp1, vget_high_f32(row3), 0);
	tmp1 = vmulq_lane_f32(row1, vget_low_f32(row2), 1);
	minor2 = vmulq_lane_f32(row3, vget_high_f32(row4), 0);
	minor2 = vmlaq_lane_f32(minor2, row3, vget_low_f32(row4), 1);
	minor2 = vmlsq_lane_f32(minor2, row2, vget_high_f32(row4), 1);
	minor2 = vmlaq_lane_f32(minor2, tmp1, vget_low_f32(row3), 0);
	minor3 = vmulq_lane_f32(row0, vget_low_f32(row2), 1);
	minor3 = vmlaq_lane_f32(minor3, row3, vget_high_f32(row4), 0);
	minor3 = vmlsq_lane_f32(minor3, row3, vget_low_f32(row4), 1);
	minor3 = vmlaq_lane_f32(minor3, row2, vget_high_f32(row4), 1);
	minor3 = vmlaq_lane_f32(minor3, tmp1, vget_high_f32(row3), 0);
	tmp1 = vmulq_lane_f32(row1, vget_low_f32(row3), 0);
	tmp1 = vmlaq_lane_f32(tmp1, row0, vget_low_f32(row2), 1);
	tmp1 = vmlsq_lane_f32(tmp1, row0, vget_high_f32(row2), 0);
	tmp1 = vmulq_lane_f32(tmp1, vget_high_f32(row3), 1);
	det = vmlaq_lane_f32(det, row0, minor0, 0);
	det = vmlsq_lane_f32(det, row1, minor1, 0);
	det = vmlaq_lane_f32(det, row2, minor2, 0);
	det = vmlsq_lane_f32(det, row3, minor3, 0);
	det = vrecpeq_f32(det);
	det = vmulq_f32(vrecpsq_f32(det, det), det);
	det = vmulq_f32(vrecpsq_f32(det, det), det);
	QMMAT h;
	h.r[0] = vmulq_f32(det, minor0);
	h.r[1] = vmulq_f32(det, minor1);
	h.r[2] = vmulq_f32(det, minor2);
	h.r[3] = vmulq_f32(det, minor3);
	return h;
	// 행렬식: vgetq_lane_f32(det, 0)
#else
	QMVEC c01 = qm_vec3_cross(m.r[0], m.r[1]);
	QMVEC c23 = qm_vec3_cross(m.r[2], m.r[3]);
	QMVEC s10 = qm_vec_sub(qm_vec_mag(m.r[0], m._24), qm_vec_mag(m.r[1], m._14));
	QMVEC s32 = qm_vec_sub(qm_vec_mag(m.r[2], m._44), qm_vec_mag(m.r[3], m._34));
	const QMVEC inv = qm_vec_div(QMCONST_ONE.s, qm_vec_add(qm_vec3_simd_dot(c01, s32), qm_vec3_simd_dot(c23, s10)));
	c01 = qm_vec_mul(c01, inv);
	c23 = qm_vec_mul(c23, inv);
	s10 = qm_vec_mul(s10, inv);
	s32 = qm_vec_mul(s32, inv);
	QMMAT h;
	h.r[0] = qm_vec4v(qm_vec_add(qm_vec3_cross(m.r[1], s32), qm_vec_mag(c23, m._24)), -qm_vec3_dot(m.r[1], c23));
	h.r[1] = qm_vec4v(qm_vec_sub(qm_vec3_cross(s32, m.r[0]), qm_vec_mag(c23, m._14)), +qm_vec3_dot(m.r[0], c23));
	h.r[2] = qm_vec4v(qm_vec_add(qm_vec3_cross(m.r[3], s10), qm_vec_mag(c01, m._44)), -qm_vec3_dot(m.r[3], c01));
	h.r[3] = qm_vec4v(qm_vec_sub(qm_vec3_cross(s10, m.r[2]), qm_vec_mag(c01, m._34)), +qm_vec3_dot(m.r[2], c01));
	return qm_mat4_tran(h);
	// 행렬식: 1.0f / inv
#endif
}

/// @brief 스케일 행렬을 만든다
INLINE QMMAT QM_VECTORCALL qm_mat4_scl(float x, float y, float z)
{
#if defined QM_USE_AVX
	QMMAT r;
	r.r[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, x);
	r.r[1] = _mm_set_ps(0.0f, 0.0f, y, 0.0f);
	r.r[2] = _mm_set_ps(0.0f, z, 0.0f, 0.0f);
	r.r[3] = QMCONST_UNIT_R3.s;
#elif defined QM_USE_NEON
	QMMAT r;
	r.r[0] = vsetq_lane_f32(x, QMCONST_ZERO.s, 0);
	r.r[1] = vsetq_lane_f32(y, QMCONST_ZERO.s, 1);
	r.r[2] = vsetq_lane_f32(z, QMCONST_ZERO.s, 2);
	r.r[3] = QMCONST_UNIT_R3.s;
#else
	QMMAT r = { ._11 = x, ._22 = y, ._33 = z, ._44 = 1.0f, };
#endif
	return r;
}

/// @brief 스케일 행렬을 만든다
INLINE QMMAT QM_VECTORCALL qm_mat4_scl_vec3(const QMVEC v)
{
#if defined QM_USE_AVX
	QMMAT r;
	r.r[0] = _mm_and_ps(v, QMCONST_S1000.s);
	r.r[1] = _mm_and_ps(v, QMCONST_S0100.s);
	r.r[2] = _mm_and_ps(v, QMCONST_S0010.s);
	r.r[3] = QMCONST_UNIT_R3.s;
#elif defined QM_USE_NEON
	QMMAT r;
	r.r[0] = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(v), QMCONST_S1000.s));
	r.r[1] = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(v), QMCONST_S0100.s));
	r.r[2] = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(v), QMCONST_S0010.s));
	r.r[3] = QMCONST_UNIT_R3.s;
#else
	QMMAT r = { ._11 = v.f[0], ._22 = v.f[1], ._33 = v.f[2], ._44 = 1.0f, };
#endif
	return r;
}

/// @brief 위치 행렬을 만든다
INLINE QMMAT QM_VECTORCALL qm_mat4_loc(float x, float y, float z)
{
	QMMAT r;
	r.r[0] = QMCONST_UNIT_R0.s;
	r.r[1] = QMCONST_UNIT_R1.s;
	r.r[2] = QMCONST_UNIT_R2.s;
	r.r[3] = qm_vec(x, y, z, 1.0f);
	return r;
}

/// @brief 위치 행렬을 만든다
INLINE QMMAT QM_VECTORCALL qm_mat4_loc_vec(const QMVEC v)
{
	QMMAT r;
	r.r[0] = QMCONST_UNIT_R0.s;
	r.r[1] = QMCONST_UNIT_R1.s;
	r.r[2] = QMCONST_UNIT_R2.s;
	r.r[3] = qm_vec_select(QMCONST_UNIT_R3.s, v, QMCONST_S1110.s);
	return r;
}

/// @brief 회전 행렬을 만든다
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_rot(const QMVEC axis, float angle)
{
	QMVEC norm = qm_vec3_norm(axis);
	float s, c;
	qm_sincosf(angle, &s, &c);
#ifdef QM_USE_AVX
	QMVEC c2 = _mm_set_ps1(1.0f - c);
	QMVEC c1 = _mm_set_ps1(c);
	QMVEC c0 = _mm_set_ps1(s);
	QMVEC n0 = _MM_PERMUTE_PS(norm, _MM_SHUFFLE(3, 0, 2, 1));
	QMVEC n1 = _MM_PERMUTE_PS(norm, _MM_SHUFFLE(3, 1, 0, 2));
	QMVEC v0 = _mm_mul_ps(n1, _mm_mul_ps(c2, n0));
	QMVEC r0 = _mm_add_ps(c1, _mm_mul_ps(_mm_mul_ps(c2, norm), norm));
	QMVEC r1 = _mm_add_ps(v0, _mm_mul_ps(c0, norm));
	QMVEC r2 = _mm_sub_ps(v0, _mm_mul_ps(c0, norm));
	v0 = _mm_and_ps(r0, QMCONST_S1110.s);
	QMVEC v1 = _MM_PERMUTE_PS(_mm_shuffle_ps(r1, r2, _MM_SHUFFLE(2, 1, 2, 0)), _MM_SHUFFLE(0, 3, 2, 1));
	QMVEC v2 = _MM_PERMUTE_PS(_mm_shuffle_ps(r1, r2, _MM_SHUFFLE(0, 0, 1, 1)), _MM_SHUFFLE(2, 0, 2, 0));
	QMVEC h0 = _MM_PERMUTE_PS(_mm_shuffle_ps(v0, v1, _MM_SHUFFLE(1, 0, 3, 0)), _MM_SHUFFLE(1, 3, 2, 0));
	QMVEC h1 = _MM_PERMUTE_PS(_mm_shuffle_ps(v0, v1, _MM_SHUFFLE(3, 2, 3, 1)), _MM_SHUFFLE(1, 3, 0, 2));
	QMVEC h2 = _mm_shuffle_ps(v2, v0, _MM_SHUFFLE(3, 2, 1, 0));
	QMMAT r;
	r.r[0] = h0;
	r.r[1] = h1;
	r.r[2] = h2;
	r.r[3] = QMCONST_UNIT_R3.s;
#else
	const float nc = 1.0f - c;
	QMMAT r =
	{
		._11 = (norm.f[0] * norm.f[0] * nc) + c,
		._12 = (norm.f[0] * norm.f[1] * nc) + (norm.f[2] * s),
		._13 = (norm.f[0] * norm.f[2] * nc) - (norm.f[1] * s),
		._21 = (norm.f[1] * norm.f[0] * nc) - (norm.f[2] * s),
		._22 = (norm.f[1] * norm.f[1] * nc) + c,
		._23 = (norm.f[1] * norm.f[2] * nc) + (norm.f[0] * s),
		._31 = (norm.f[2] * norm.f[0] * nc) + (norm.f[1] * s),
		._32 = (norm.f[2] * norm.f[1] * nc) - (norm.f[0] * s),
		._33 = (norm.f[2] * norm.f[2] * nc) + c,
		._44 = 1.0f,
	};
#endif
	return r;
}

/// @brief 회전 행렬을 만든다 (롤/피치/요)
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_vec3(const QMVEC rot)
{
	float sr, sp, sy;
	float cr, cp, cy;
	qm_sincosf(qm_vec_get_x(rot), &sr, &cr);
	qm_sincosf(qm_vec_get_y(rot), &sp, &cp);
	qm_sincosf(qm_vec_get_z(rot), &sy, &cy);
	const float srsp = sr * sp;
	const float crsp = cr * sp;
	QMMAT r;
	r.r[0] = qm_vec4(cp * cy, cp * sy, -sp, 0.0f);
	r.r[1] = qm_vec4(srsp * cy - cr * sy, srsp * sy + cr * cy, sr * cp, 0.0f);
	r.r[2] = qm_vec4(crsp * cy + sr * sy, crsp * sy - sr * cy, cr * cp, 0.0f);
	r.r[3] = QMCONST_UNIT_R3.s;
	return r;
}

/// @brief 사원수로 회전 행렬을 만든다
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_quat(const QMVEC rot)
{
#ifdef QM_USE_AVX
	static const QmVec4 c1110 = { { 1.0f, 1.0f, 1.0f, 0.0f } };
	static const QmVecU mask3 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } };
	QMVEC Q0 = _mm_add_ps(rot, rot);
	QMVEC Q1 = _mm_mul_ps(rot, Q0);
	QMVEC V0 = _MM_PERMUTE_PS(Q1, _MM_SHUFFLE(3, 0, 0, 1));
	V0 = _mm_and_ps(V0, mask3.s);
	QMVEC V1 = _MM_PERMUTE_PS(Q1, _MM_SHUFFLE(3, 1, 2, 2));
	V1 = _mm_and_ps(V1, mask3.s);
	QMVEC R0 = _mm_sub_ps(c1110.s, V0);
	R0 = _mm_sub_ps(R0, V1);
	V0 = _MM_PERMUTE_PS(rot, _MM_SHUFFLE(3, 1, 0, 0));
	V1 = _MM_PERMUTE_PS(Q0, _MM_SHUFFLE(3, 2, 1, 2));
	V0 = _mm_mul_ps(V0, V1);
	V1 = _MM_PERMUTE_PS(rot, _MM_SHUFFLE(3, 3, 3, 3));
	QMVEC V2 = _MM_PERMUTE_PS(Q0, _MM_SHUFFLE(3, 0, 2, 1));
	V1 = _mm_mul_ps(V1, V2);
	QMVEC R1 = _mm_add_ps(V0, V1);
	QMVEC R2 = _mm_sub_ps(V0, V1);
	V0 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(1, 0, 2, 1));
	V0 = _MM_PERMUTE_PS(V0, _MM_SHUFFLE(1, 3, 2, 0));
	V1 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(2, 2, 0, 0));
	V1 = _MM_PERMUTE_PS(V1, _MM_SHUFFLE(2, 0, 2, 0));
	Q1 = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(1, 0, 3, 0));
	Q1 = _MM_PERMUTE_PS(Q1, _MM_SHUFFLE(1, 3, 2, 0));
	QMMAT m;
	m.r[0] = Q1;
	Q1 = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(3, 2, 3, 1));
	Q1 = _MM_PERMUTE_PS(Q1, _MM_SHUFFLE(1, 3, 0, 2));
	m.r[1] = Q1;
	Q1 = _mm_shuffle_ps(V1, R0, _MM_SHUFFLE(3, 2, 1, 0));
	m.r[2] = Q1;
	m.r[3] = QMCONST_UNIT_R3.s;
	return m;
#else
	const QMVEC norm = qm_vec4_norm(rot);
	const float XX = norm.f[0] * norm.f[0];
	const float YY = norm.f[1] * norm.f[1];
	const float ZZ = norm.f[2] * norm.f[2];
	const float XY = norm.f[0] * norm.f[1];
	const float XZ = norm.f[0] * norm.f[2];
	const float YZ = norm.f[1] * norm.f[2];
	const float WX = norm.f[3] * norm.f[0];
	const float WY = norm.f[3] * norm.f[1];
	const float WZ = norm.f[3] * norm.f[2];
	QMMAT r =
	{
		._11 = 1.0f - 2.0f * (YY + ZZ),
		._12 = 2.0f * (XY + WZ),
		._13 = 2.0f * (XZ - WY),
		._21 = 2.0f * (XY - WZ),
		._22 = 1.0f - 2.0f * (XX + ZZ),
		._23 = 2.0f * (YZ + WX),
		._31 = 2.0f * (XZ + WY),
		._32 = 2.0f * (YZ - WX),
		._33 = 1.0f - 2.0f * (XX + YY),
		._44 = 1.0f,
	};
	return r;
#endif
}

/// @brief X축 회전 행렬을 만든다
/// @param rot X측 회전값
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_x(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
	QMMAT r;
	r.r[0] = QMCONST_UNIT_R0.s;
	r.r[1] = qm_vec4(0.0f, c, s, 0.0f);
	r.r[2] = qm_vec4(0.0f, -s, c, 0.0f);
	r.r[3] = QMCONST_UNIT_R3.s;
	return r;
}

/// @brief Y축 회전 행렬을 만든다
/// @param rot Y측 회전값
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_y(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
	QMMAT r;
	r.r[0] = qm_vec4(c, 0.0f, -s, 0.0f);
	r.r[1] = QMCONST_UNIT_R1.s;
	r.r[2] = qm_vec4(s, 0.0f, c, 0.0f);
	r.r[3] = QMCONST_UNIT_R3.s;
	return r;
}

/// @brief Z축 회전 행렬을 만든다
/// @param rot Z측 회전값
INLINE QMMAT QM_VECTORCALL qm_mat4_rot_z(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
	QMMAT r;
	r.r[0] = qm_vec4(c, s, 0.0f, 0.0f);
	r.r[1] = qm_vec4(-s, c, 0.0f, 0.0f);
	r.r[2] = QMCONST_UNIT_R2.s;
	r.r[3] = QMCONST_UNIT_R3.s;
	return r;
}

/// @brief 아핀 변환 행렬
/// @param scl 벡터3 스케일 (1일 경우 NULL)
/// @param rotcenter 벡터3 회전축(원점일 경우 NULL)
/// @param rot 사원수 회전 (고정일 경우 NULL)
/// @param loc 벡터3 위치 (원점일 경우 NULL)
INLINE QMMAT QM_VECTORCALL qm_mat4_affine(const QMVEC scl, const QMVEC rotcenter, const QMVEC rot, const QMVEC loc)
{
	QMMAT m = qm_mat4_scl_vec3(scl);
	const QMMAT mr = qm_mat4_rot_quat(rot);
	m.r[3] = qm_vec_sub(m.r[3], rotcenter);
	m = qm_mat4_mul(m, mr);
	m.r[3] = qm_vec_add(m.r[3], rotcenter);
	m.r[3] = qm_vec_add(m.r[3], loc);
	return m;
}

/// @brief 행렬 트랜스폼
INLINE QMMAT QM_VECTORCALL qm_mat4_trfm(const QMVEC loc, const QMVEC rot, const QMVEC scl)
{
	QMMAT m = qm_mat4_scl_vec3(scl);
	const QMMAT mr = qm_mat4_rot_quat(rot);
	m = qm_mat4_mul(m, mr);
	m.r[3] = qm_vec_add(m.r[3], loc);
	return m;
}

/// @brief 행렬 트랜스폼. 단 벡터 회전
INLINE QMMAT QM_VECTORCALL qm_mat4_trfm_vec(const QMVEC loc, const QMVEC rot, const QMVEC scl)
{
	QMMAT m = qm_mat4_scl_vec3(scl);
	const QMMAT mr = qm_mat4_rot_vec3(rot);
	m = qm_mat4_mul(m, mr);
	m.r[3] = qm_vec_add(m.r[3], loc);
	return m;
}

/// @brief 반사 행렬
INLINE QMMAT QM_VECTORCALL qm_mat4_reflect(const QMVEC plane)
{
	static const QMVEC neg2 = { { -2.0f, -2.0f, -2.0f, 0.0f } };
	const QMVEC p = qm_plane_norm(plane);
	const QMVEC s = qm_vec_mul(p, neg2);
	const QMVEC a = qm_vec_sp_x(p);
	const QMVEC b = qm_vec_sp_y(p);
	const QMVEC c = qm_vec_sp_z(p);
	const QMVEC d = qm_vec_sp_w(p);
	QMMAT m;
	m.r[0] = qm_vec_madd(a, s, QMCONST_UNIT_R0.s);
	m.r[1] = qm_vec_madd(b, s, QMCONST_UNIT_R1.s);
	m.r[2] = qm_vec_madd(c, s, QMCONST_UNIT_R2.s);
	m.r[3] = qm_vec_madd(d, s, QMCONST_UNIT_R3.s);
	return m;
}

/// @brief 그림자 행렬
INLINE QMMAT QM_VECTORCALL qm_mat4_shadow(const QMVEC plane, const QMVEC light)
{
	static const QmVecU s0001 = { { 0, 0, 0, 0xFFFFFFFF } };
	QMVEC p = qm_plane_norm(plane);
	QMVEC dot = qm_plane_simd_dot(p, light);
	p = qm_vec_neg(p);
	const QMVEC a = qm_vec_sp_x(p);
	const QMVEC b = qm_vec_sp_y(p);
	const QMVEC c = qm_vec_sp_z(p);
	const QMVEC d = qm_vec_sp_w(p);
	dot = qm_vec_select(s0001.s, dot, s0001.s);
	QMMAT m;
	m.r[3] = qm_vec_madd(d, light, dot);
	dot = qm_vec_bit_rol(dot, 1);
	m.r[2] = qm_vec_madd(c, light, dot);
	dot = qm_vec_bit_rol(dot, 1);
	m.r[1] = qm_vec_madd(b, light, dot);
	dot = qm_vec_bit_rol(dot, 1);
	m.r[0] = qm_vec_madd(a, light, dot);
	return m;
}

// 보기 행렬을 만든다 (왼손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_internal_look_to(const QMVEC eye, const QMVEC dir, const QMVEC up)
{
	const QMVEC r2 = qm_vec4_norm(dir);
	const QMVEC r0 = qm_vec4_norm(qm_vec3_cross(up, r2));
	const QMVEC r1 = qm_vec3_cross(r2, r0);
	const QMVEC r3 = qm_vec_neg(eye);
	const QMVEC d0 = qm_vec3_simd_dot(r0, r3);
	const QMVEC d1 = qm_vec3_simd_dot(r1, r3);
	const QMVEC d2 = qm_vec3_simd_dot(r2, r3);
	QMMAT m;
	m.r[0] = qm_vec_select(d0, r0, QMCONST_S1110.s);
	m.r[1] = qm_vec_select(d1, r1, QMCONST_S1110.s);
	m.r[2] = qm_vec_select(d2, r2, QMCONST_S1110.s);
	m.r[3] = QMCONST_UNIT_R3.s;
	return qm_mat4_tran(m);
}

/// @brief 보기 행렬을 만든다 (왼손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_lookat_lh(QMVEC eye, QMVEC at, QMVEC up)
{
	const QMVEC dir = qm_vec_sub(at, eye);
	return qm_mat4_internal_look_to(eye, dir, up);
}

/// @brief 보기 행렬을 만든다 (오른손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_lookat_rh(QMVEC eye, QMVEC at, QMVEC up)
{
	const QMVEC dir = qm_vec_sub(eye, at);
	return qm_mat4_internal_look_to(eye, dir, up);
}

/// @brief 투영 행렬을 만든다 (왼손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_perspective_lh(float fov, float aspect, float zn, float zf)
{
	float s, c;
	qm_sincosf(fov * 0.5f, &s, &c);
	const float h = c / s;
	const float w = h / aspect;
	const float q = zf / (zf - zn);
	QMMAT r;
	r.r[0] = qm_vec4(w, 0.0f, 0.0f, 0.0f);
	r.r[1] = qm_vec4(0.0f, h, 0.0f, 0.0f);
	r.r[2] = qm_vec4(0.0f, 0.0f, q, 1.0f);
	r.r[3] = qm_vec4(0.0f, 0.0f, -zn * q, 0.0f);
	return r;
}

/// @brief 투영 행렬을 만든다 (오른손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_perspective_rh(float fov, float aspect, float zn, float zf)
{
	float s, c;
	qm_sincosf(fov * 0.5f, &s, &c);
	const float h = c / s;
	const float w = h / aspect;
	const float q = zf / (zn - zf);
	QMMAT r;
	r.r[0] = qm_vec4(w, 0.0f, 0.0f, 0.0f);
	r.r[1] = qm_vec4(0.0f, h, 0.0f, 0.0f);
	r.r[2] = qm_vec4(0.0f, 0.0f, q, -1.0f);
	r.r[3] = qm_vec4(0.0f, 0.0f, zn * q, 0.0f);
	return r;
}

/// @brief 정규 행렬를 만든다 (왼손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_ortho_lh(float width, float height, float zn, float zf)
{
	const float q = 1.0f / (zf - zn);
	QMMAT r;
	r.r[0] = qm_vec4(2.0f / width, 0.0f, 0.0f, 0.0f);
	r.r[1] = qm_vec4(0.0f, 2.0f / height, 0.0f, 0.0f);
	r.r[2] = qm_vec4(0.0f, 0.0f, q, 0.0f);
	r.r[3] = qm_vec4(0.0f, 0.0f, -zn * q, 1.0f);
	return r;
}

/// @brief 정규 행렬을 만든다 (오른손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_ortho_rh(float width, float height, float zn, float zf)
{
	const float q = 1.0f / (zn - zf);
	QMMAT r;
	r.r[0] = qm_vec4(2.0f / width, 0.0f, 0.0f, 0.0f);
	r.r[1] = qm_vec4(0.0f, 2.0f / height, 0.0f, 0.0f);
	r.r[2] = qm_vec4(0.0f, 0.0f, q, 0.0f);
	r.r[3] = qm_vec4(0.0f, 0.0f, zn * q, 1.0f);
	return r;
}

/// @brief 사각형을 기준으로 정규 행렬을 만든다 (왼손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_ortho_offcenter_lh(float left, float top, float right, float bottom, float zn, float zf)
{
	QMMAT r;
	r.r[0] = qm_vec4(2.0f / (right - left), 0.0f, 0.0f, 0.0f);
	r.r[1] = qm_vec4(0.0f, 2.0f / (top - bottom), 0.0f, 0.0f);
	r.r[2] = qm_vec4(0.0f, 0.0f, 1.0f / (zf - zn), 0.0f);
	r.r[3] = qm_vec4((left + right) / (left - right), (top + bottom) / (bottom - top), zn / (zn - zf), 1.0f);
	return r;
}

/// @brief 사각형을 기준으로 정규 행렬을 만든다 (오른손 기준)
INLINE QMMAT QM_VECTORCALL qm_mat4_ortho_offcenter_rh(float left, float top, float right, float bottom, float zn, float zf)
{
	QMMAT r;
	r.r[0] = qm_vec4(2.0f / (right - left), 0.0f, 0.0f, 0.0f);
	r.r[1] = qm_vec4(0.0f, 2.0f / (top - bottom), 0.0f, 0.0f);
	r.r[2] = qm_vec4(0.0f, 0.0f, 1.0f / (zn - zf), 0.0f);
	r.r[3] = qm_vec4((left + right) / (left - right), (top + bottom) / (bottom - top), zn / (zn - zf), 1.0f);
	return r;
}

/// @brief 뷰포트 행렬을 만든다
/// @param x,y 좌표
/// @param width,height 너비와 높이
INLINE QMMAT QM_VECTORCALL qm_mat4_viewport(float x, float y, float width, float height)
{
	QMMAT r;
	r.r[0] = qm_vec4(width * 0.5f, 0.0f, 0.0f, 0.0f);
	r.r[1] = qm_vec4(0.0f, height * -0.5f, 0.0f, 0.0f);
	r.r[2] = QMCONST_UNIT_R2.s;
	r.r[3] = qm_vec4(x + width * 0.5f, y - height * -0.5f, 0.0f, 1.0f);
	return r;
}

/// @brief 단위 행렬인지 비교
/// @param m 비교할 행렬
/// @return 단위 행렬이면 참을 반환
INLINE bool QM_VECTORCALL qm_mat4_isu(QMMAT m)
{
#if defined QM_USE_AVX
	QMVEC r1 = _mm_cmpeq_ps(m.r[0], QMCONST_UNIT_R0.s);
	QMVEC r2 = _mm_cmpeq_ps(m.r[1], QMCONST_UNIT_R1.s);
	QMVEC r3 = _mm_cmpeq_ps(m.r[2], QMCONST_UNIT_R2.s);
	QMVEC r4 = _mm_cmpeq_ps(m.r[3], QMCONST_UNIT_R3.s);
	r1 = _mm_and_ps(r1, r2);
	r3 = _mm_and_ps(r3, r4);
	r1 = _mm_and_ps(r1, r3);
	return (_mm_movemask_ps(r1) == 0x0f);
#elif defined QM_USE_NEON
	uint32x4_t r1 = vceqq_f32(m.r[0], QMCONST_UNIT_R0.s);
	uint32x4_t r2 = vceqq_f32(m.r[1], QMCONST_UNIT_R1.s);
	uint32x4_t r3 = vceqq_f32(m.r[2], QMCONST_UNIT_R2.s);
	uint32x4_t r4 = vceqq_f32(m.r[3], QMCONST_UNIT_R3.s);
	r1 = vandq_u32(r1, r3);
	r2 = vandq_u32(r2, r4);
	r1 = vandq_u32(r1, r2);
	uint8x8x2_t r = vzip_u8(vget_low_u8(vreinterpretq_u8_u32(r1)), vget_high_u8(vreinterpretq_u8_u32(r1)));
	uint16x4x2_t p = vzip_u16(vreinterpret_u16_u8(r.val[0]), vreinterpret_u16_u8(r.val[1]));
	uint32_t u = vget_lane_u32(vreinterpret_u32_u16(p.val[1]), 1);
	return (u == 0xFFFFFFFFU);
#else
	const uint32_t* p = (const uint32_t*)&m.f[0];
	uint32_t u1 = p[0] ^ 0x3F800000U;
	uint32_t u0 = p[1];
	u0 |= p[2];
	u0 |= p[3];
	u0 |= p[4];
	u1 |= p[5] ^ 0x3F800000U;
	u0 |= p[6];
	u0 |= p[7];
	u0 |= p[8];
	u0 |= p[9];
	u1 |= p[10] ^ 0x3F800000U;
	u0 |= p[11];
	u0 |= p[12];
	u0 |= p[13];
	u0 |= p[14];
	u1 |= p[15] ^ 0x3F800000U;
	u0 &= 0x7FFFFFFF;
	u1 |= u0;
	return (u1 == 0);
#endif
}


//////////////////////////////////////////////////////////////////////////
// point32_t 

/// @brief 점 설정
INLINE QmPoint qm_point(int32_t x, int32_t y)
{
	return (QmPoint) { { x, y } };
}

/// @brief 벡터로 점 설정
INLINE QmPoint qm_pointv(const QmVec2 v)
{
	return (QmPoint) { { (int32_t)v.X, (int32_t)v.Y } };
}

/// @brief 벡터로 점 설정
INLINE QmPoint qm_pointv4(const QMVEC v)
{
	QmPoint p;
	qm_vec_st_int2((QmInt2*)&p, v);
	return p;
}

/// @brief 점 초기화
INLINE QmPoint qm_point_zero(void)		// identify
{
	static const QmPoint zero = { { 0, 0 } };
	return zero;
}

/// @brief 점 대각값 설정 (모두 같은값으로 설정)
INLINE QmPoint qm_point_sp(const int32_t diag)
{
	return (QmPoint) { { diag, diag } };
}

/// @brief 점 반전
INLINE QmPoint qm_point_neg(const QmPoint p)  // invert
{
	return (QmPoint) { { -p.X, -p.Y } };
}

/// @brief 점 덧셈
INLINE QmPoint qm_point_add(const QmPoint left, const QmPoint right)
{
	return (QmPoint) { { left.X + right.X, left.Y + right.Y } };
}

/// @brief 점 뺄셈
INLINE QmPoint qm_point_sub(const QmPoint left, const QmPoint right)
{
	return (QmPoint) { { left.X - right.X, left.Y - right.Y } };
}

/// @brief 점 확대
INLINE QmPoint qm_point_mag(const QmPoint left, int32_t right)
{
	return (QmPoint) { { left.X * right, left.Y * right } };
}

/// @brief 점 줄이기
INLINE QmPoint qm_point_abr(const QmPoint left, int32_t right)
{
	return (QmPoint) { { left.X / right, left.Y / right } };
}

/// @brief 점 항목 곱셈
INLINE QmPoint qm_point_mul(const QmPoint left, const QmPoint right)
{
	return (QmPoint) { { left.X * right.X, left.Y * right.Y } };
}

/// @brief 점 항목 나눗셈
INLINE QmPoint qm_point_div(const QmPoint left, const QmPoint right)
{
	return (QmPoint) { { left.X / right.X, left.Y / right.Y } };
}

/// @brief 점의 최소값
INLINE QmPoint qm_point_min(const QmPoint left, const QmPoint right)
{
	return (QmPoint) { { QN_MIN(left.X, right.X), QN_MIN(left.Y, right.Y) } };
}

/// @brief 점의 최대값
INLINE QmPoint qm_point_max(const QmPoint left, const QmPoint right)
{
	return (QmPoint) { { QN_MAX(left.X, right.X), QN_MAX(left.Y, right.Y) } };
}

/// @brief 점의 외적
INLINE QmPoint qm_point_cross(const QmPoint left, const QmPoint right)
{
	return qm_point(left.Y * right.X - left.X * right.Y, left.X * right.Y - left.Y * right.X);
}

/// @brief 점 내적
INLINE int32_t qm_point_dot(const QmPoint left, const QmPoint right)
{
	return left.X * right.X + left.Y * right.Y;
}

/// @brief 점 길이의 제곱
INLINE int32_t qm_point_len_sq(const QmPoint pt)
{
	return qm_point_dot(pt, pt);
}

/// @brief 두 점 거리의 제곱
INLINE int32_t qm_point_dist_sq(const QmPoint left, const QmPoint right)
{
	const QmPoint t = qm_point_sub(left, right);
	return qm_point_len_sq(t);
}

/// @brief 점 길이
INLINE float qm_point_len(const QmPoint pt)
{
	return qm_sqrtf((float)qm_point_len_sq(pt));	// NOLINT
}

/// @brief 두 점의 거리
INLINE float qm_point_dist(const QmPoint left, const QmPoint right)
{
	return qm_sqrtf((float)qm_point_dist_sq(left, right));		// NOLINT
}

/// @brief 점의 비교
INLINE bool qm_point_eq(const QmPoint left, const QmPoint right)
{
	return left.X == right.X && left.Y == right.Y;
}

/// @brief 점가 0인가 비교
INLINE bool qm_point_isz(const QmPoint pt)
{
	return pt.X == 0 && pt.Y == 0;
}


//////////////////////////////////////////////////////////////////////////
// size

/// @brief 사이즈 값 설정
INLINE QmSize qm_size(int32_t width, int32_t height)
{
	return (QmSize) { { width, height } };
}

/// @brief 사각형으로 크기를 설정한다
INLINE QmSize qm_size_rect(const QmRect rt)
{
	return (QmSize) { { rt.Right - rt.Left, rt.Bottom - rt.Top } };
}

/// @brief 사이즈 대각값 설정 (모두 같은값으로 설정)
INLINE QmSize qm_size_sp(const int32_t diag)
{
	return (QmSize) { { diag, diag } };
}

/// @brief 사이즈 덧셈
INLINE QmSize qm_size_add(const QmSize left, const QmSize right)
{
	return (QmSize) { { left.Width + right.Width, left.Height + right.Height } };
}

/// @brief 사이즈 뺄셈
INLINE QmSize qm_size_sub(const QmSize left, const QmSize right)
{
	return (QmSize) { { left.Width - right.Width, left.Height - right.Height } };
}

/// @brief 사이즈 확대
INLINE QmSize qm_size_mag(const QmSize left, int32_t right)
{
	return (QmSize) { { left.Width * right, left.Height * right } };
}

/// @brief 사이즈 줄이기
INLINE QmSize qm_size_abr(const QmSize left, int32_t right)
{
	return (QmSize) { { left.Width / right, left.Height / right } };
}

/// @brief 사이즈 항목 곱셈
INLINE QmSize qm_size_mul(const QmSize left, const QmSize right)
{
	return (QmSize) { { left.Width * right.Width, left.Height * right.Height } };
}

/// @brief 사이즈 항목 나눗셈
INLINE QmSize qm_size_div(const QmSize left, const QmSize right)
{
	return (QmSize) { { left.Width / right.Width, left.Height / right.Height } };
}

/// @brief 사이즈의 최소값
INLINE QmSize qm_size_min(const QmSize left, const QmSize right)
{
	return (QmSize) { { QN_MIN(left.Width, right.Width), QN_MIN(left.Height, right.Height) } };
}

/// @brief 사이즈의 최대값
INLINE QmSize qm_size_max(const QmSize left, const QmSize right)
{
	return (QmSize) { { QN_MAX(left.Width, right.Width), QN_MAX(left.Height, right.Height) } };
}

/// @brief 사이즈 크기의 제곱
INLINE int32_t qm_size_len_sq(const QmSize s)
{
	return s.Width * s.Width + s.Height * s.Height;
}

/// @brief 사이즈 크기
INLINE float qm_size_len(const QmSize v)
{
	return qm_sqrtf((float)qm_size_len_sq(v));	// NOLINT
}

/// @brief 종횡비(너비 나누기 높이)를 계신한다
INLINE float qm_size_get_aspect(const QmSize s)
{
	return (float)s.Width / (float)s.Height;
}

/// @brief 가운데 위치하도록 위치를 구한다
INLINE QmPoint qm_size_locate_center(const QmSize s, int32_t width, int32_t height)
{
	return qm_point((s.Width - width) / 2, (s.Height - height) / 2);
}

/// @brief 대각선 DPI를 구한다
INLINE float qm_size_calc_dpi(const QmSize pt, float horizontal, float vertical)
{
	const float dsq = horizontal * horizontal + vertical + vertical;
	if (dsq <= 0.0f)
		return 0.0f;
	return qm_sqrtf((float)(pt.Width * pt.Width + pt.Height * pt.Height)) / qm_sqrtf(dsq);
}

/// @brief 사이즈의 비교
INLINE bool qm_size_eq(const QmSize left, const QmSize right)
{
	return left.Width == right.Width && left.Height == right.Height;
}


//////////////////////////////////////////////////////////////////////////
// rectangle

/// @brief 사각형 값 설정
INLINE QmRect qm_rect(int32_t left, int32_t top, int32_t right, int32_t bottom)
{
	return (QmRect) { { left, top, right, bottom } };
}

/// @brief 사각형을 좌표와 크기로 설정한다
INLINE QmRect qm_rect_size(int32_t x, int32_t y, int32_t width, int32_t height)
{
	return (QmRect) { { x, y, x + width, y + height } };
}

/// @brief 사각형을 좌표와 크기 타입을 사용하여 설정한다
INLINE QmRect qm_rect_pos_size(QmPoint pos, QmSize size)
{
	return (QmRect) { { pos.X, pos.Y, pos.X + size.Width, pos.Y + size.Height } };
}

/// @brief 사각형 초기화
INLINE QmRect qm_rect_zero(void)		// identify
{
	static const QmRect zero = { { 0, 0, 0, 0 } };
	return zero;
}

/// @brief 사각형 대각값 설정 (모든 요소를 같은 값을)
INLINE QmRect qm_rect_sp(const int32_t diag)
{
	return (QmRect) { { diag, diag, diag, diag } };
}

/// @brief 사각형 덧셈
INLINE QmRect qm_rect_add(const QmRect left, const QmRect right)
{
	return (QmRect) { { left.Left + right.Left, left.Top + right.Top, left.Right + right.Right, left.Bottom + right.Bottom } };
}

/// @brief 사각형 뺄셈
INLINE QmRect qm_rect_sub(const QmRect left, const QmRect right)
{
	return (QmRect) { { left.Left - right.Left, left.Top - right.Top, left.Right - right.Right, left.Bottom - right.Bottom } };
}

/// @brief 사각형 확대
INLINE QmRect qm_rect_mag(const QmRect left, int32_t right)
{
	return (QmRect) { { left.Left * right, left.Top * right, left.Right * right, left.Bottom * right } };
}

/// @brief 사각형 줄이기
INLINE QmRect qm_rect_abr(const QmRect left, int32_t right)
{
	return (QmRect) { { left.Left / right, left.Top / right, left.Right / right, left.Bottom / right } };
}

/// @brief 사각형의 최소값
INLINE QmRect qm_rect_min(const QmRect left, const QmRect right)
{
	return (QmRect) { { QN_MIN(left.Left, right.Left), QN_MIN(left.Top, right.Top), QN_MIN(left.Right, right.Right), QN_MIN(left.Bottom, right.Bottom) } };
}

/// @brief 사각형의 최대값
INLINE QmRect qm_rect_max(const QmRect left, const QmRect right)
{
	return (QmRect) { { QN_MAX(left.Left, right.Left), QN_MAX(left.Top, right.Top), QN_MAX(left.Right, right.Right), QN_MAX(left.Bottom, right.Bottom) } };
}

/// @brief 사각형 크기를 키운다 (요소가 양수일 경우)
INLINE QmRect qm_rect_inflate(const QmRect rt, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
	return (QmRect) { { rt.Left - left, rt.Top - top, rt.Right + right, rt.Bottom + bottom } };
}

/// @brief 사각형 크기를 줄인다 (요소가 양수일 경우)
INLINE QmRect qm_rect_deflate(const QmRect rt, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
	return (QmRect) { { rt.Left + left, rt.Top + top, rt.Right - right, rt.Bottom - bottom } };
}

/// @brief 사각형을 움직인다 (요소가 양수일 경우 일괄 덧셈)
INLINE QmRect qm_rect_offset(const QmRect rt, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
	return (QmRect) { { rt.Left + left, rt.Top + top, rt.Right + right, rt.Bottom + bottom } };
}

/// @brief 사각형을 움직인다
INLINE QmRect qm_rect_move(const QmRect rt, int32_t left, int32_t top)
{
	const int32_t dx = left - rt.Left;
	const int32_t dy = top - rt.Top;
	return (QmRect) { { rt.Left + dx, rt.Top + dy, rt.Right + dx, rt.Bottom + dy } };
}

/// @brief 사각형의 크기를 재설정한다
INLINE QmRect qm_rect_set_size(const QmRect rt, int32_t width, int32_t height)
{
	return (QmRect) { { rt.Left, rt.Top, rt.Left + width, rt.Top + height } };
}

/// @brief 사각형의 너비를 얻는다
INLINE int32_t qm_rect_get_width(const QmRect rt)
{
	return rt.Right - rt.Left;
}

/// @brief 사각형의 높이를 얻는다
INLINE int32_t qm_rect_get_height(const QmRect rt)
{
	return rt.Bottom - rt.Top;
}

/// @brief 좌표가 사각형 안에 있는지 조사한다
INLINE bool qm_rect_in(const QmRect rt, const int32_t x, const int32_t y)
{
	return x >= rt.Left && x <= rt.Right && y >= rt.Top && y <= rt.Bottom;
}

/// @brief 대상 사각형이 원본 사각형안에 있는지 조사한다
INLINE bool qm_rect_include(const QmRect dest, const QmRect target)
{
	if (dest.Left > target.Left || dest.Top > target.Top)
		return false;
	if (dest.Right < target.Right || dest.Bottom < target.Bottom)
		return false;
	return true;
}

/// @brief 두 사각형이 충돌하는지 비교하고 충돌 사각형을 만든다
INLINE bool qm_rect_intersect(const QmRect r1, const QmRect r2, QmRect* p)
{
	const bool b = r2.Left < r1.Right && r2.Right > r1.Left && r2.Top < r1.Bottom && r2.Bottom > r1.Top;
	if (p)
	{
		if (!b)
			*p = qm_rect_zero();
		else
			*p = qm_rect(
				QN_MAX(r1.Left, r2.Left), QN_MAX(r1.Top, r2.Top),
				QN_MIN(r1.Right, r2.Right), QN_MIN(r1.Bottom, r2.Bottom));
	}
	return b;
}

/// @brief 두 사각형를 비교
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
/// @return 두 사각형가 같으면 참
INLINE bool qm_rect_eq(const QmRect left, const QmRect right)
{
	return
		left.Left == right.Left && left.Top == right.Top &&
		left.Right == right.Right && left.Bottom == right.Bottom;
}

/// @brief 사각형가 0인지 비교
/// @param pv 비교할 사각형
/// @return 사각형가 0이면 참
INLINE bool qm_rect_isz(const QmRect pv)
{
	return pv.Left == 0 && pv.Top == 0 && pv.Right == 0 && pv.Bottom == 0;
}

/// @brief 사각형을 회전한다
/// @param rt 원본 사각형
/// @param angle 회전 각도(호도)
/// @param tl 왼쪽 위
/// @param tr 오른쪽 위
/// @param bl 왼쪽 아래
/// @param br 오른쪽 아래
INLINE void qm_rect_rotate(QmRect rt, float angle, QmVec2* tl, QmVec2* tr, QmVec2* bl, QmVec2* br)
{
	assert(tl != NULL && tr != NULL && bl != NULL && br != NULL);
	float s, c;
	qm_sincosf(angle, &s, &c);
	const QmVec2 dt = qm_vec2((float)(rt.Right - rt.Left) * 0.5f, (float)(rt.Bottom - rt.Top) * 0.5f);
	const QmVec2 ot = qm_vec2((float)rt.Left + dt.X, (float)rt.Top + dt.Y);
	const QmVec2 vi[4] = { {{-dt.X, -dt.Y}}, {{dt.X, -dt.Y}}, {{-dt.X, dt.Y}}, {{dt.X, dt.Y}} };
	QmVec2 vo[4];
	for (int32_t i = 0; i < 4; i++)
	{
		vo[i].X = vi[i].X * c - vi[i].Y * s;
		vo[i].Y = vi[i].X * s + vi[i].Y * c;
		vo[i] = qm_vec2_add(vo[i], ot);
	}
#ifdef _DEBUG
	*tl = vo[0]; *tr = vo[1]; *bl = vo[2]; *br = vo[3];
#else
	*tl = vo[0]; *tr = vo[1]; *bl = vo[2]; *br = vo[3];
#endif
}

#ifdef _WINDEF_
/// @brief 윈도우 RECT에서
INLINE QmRect qm_rect_win_rect(RECT rt)
{
	return (QmRect) { { rt.left, rt.top, rt.right, rt.bottom } };
}

/// @brief 윈도우 RECT로
INLINE RECT qm_rect_to_win_rect(const QmRect rt)
{
	return (RECT) { rt.Left, rt.Top, rt.Right, rt.Bottom };
}
#endif


//////////////////////////////////////////////////////////////////////////
// generic

/// @brief (제네릭) 덧셈
#define qm_add(l,r)		_Generic((l),\
	QmVec2: qm_vec2_add,\
	QMVEC: qm_vec_add,\
	QMMAT: qm_mat4_add,\
	QmPoint: qm_point_add,\
	QmSize: qm_size_add,\
	QmRect: qm_rect_add)(l,r)
/// @brief (제네릭) 뺄셈
#define qm_sub(l,r)		_Generic((l),\
	QmVec2: qm_vec2_sub,\
	QMVEC: qm_vec_sub,\
	QMMAT: qm_mat4_sub,\
	QmPoint: qm_point_sub,\
	QmSize: qm_size_sub,\
	QmRect: qm_rect_sub)(l,r)
/// @brief (제네릭) 확대
#define qm_mag(i,s)		_Generic((i),\
	QmVec2: qm_vec2_mag,\
	QMVEC: qm_vec_mag,\
	QMMAT: qm_mat4_mag,\
	QmPoint: qm_point_mag,\
	QmSize: qm_size_mag,\
	QmRect: qm_rect_mag)(i,s)
/// @brief (제네릭) 줄이기
#define qm_abr(i,s)		_Generic((i),\
	QmVec2: qm_vec2_abr,\
	QMVEC: qm_vec_abr,\
	QMMAT: qm_mat4_abr,\
	QmPoint: qm_point_abr,\
	QmSize: qm_size_abr,\
	QmRect: qm_rect_abr)(i,s)
/// @brief (제네릭) 최소값
#define qm_min(l,r)		_Generic((l),\
	float: qm_minf,\
	QmVec2: qm_vec2_min,\
	QMVEC: qm_vec_min,\
	QmPoint: qm_point_min,\
	QmSize: qm_size_min,\
	QmRect: qm_rect_min)(l,r)
/// @brief (제네릭) 최대값
#define qm_max(l,r)		_Generic((l),\
	float: qm_maxf,\
	QmVec2: qm_vec2_max,\
	QMVEC: qm_vec_max,\
	QmPoint: qm_point_max,\
	QmSize: qm_size_max,\
	QmRect: qm_rect_max)(l,r)
/// @brief (제네릭) 같나 비교
#define qm_eq(l,r)		_Generic((l),\
	float: qm_eqf,\
	QmVec2: qm_vec2_eq,\
	QMVEC: qm_vec_eq,\
	QmPoint: qm_point_eq,\
	QmSize: qm_size_eq,\
	QmRect: qm_rect_eq)(l,r)
/// @brief (제네릭) 선형 보간
#define qm_lerp(l,r,s)	_Generic((l),\
	float: qm_lerpf,\
	QmVec2: qm_vec2_lerp,\
	QMVEC: qm_vec_lerp)(l,r,s)
/// @brief 절대값
#define qm_abs(x)		_Generic((x),\
	float: qm_absf,\
	int32_t: qm_absi)(x)
/// @brief 범위로 자르기
#define qm_clamp(v,n,x)	_Generic((v),\
	float: qm_clampf,\
	int32_t: qm_clampi)(v,n,x)

#ifdef __clang__
#pragma clang diagnotics pop
#endif

// set reset diagnosis ivt add sub  mag mul div min max eq isi
// dot cross len_sq len dist_sq dist
// ^INLINE (.*)$ => $0;\n

