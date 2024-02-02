//
// QsLib [MATH Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: UNLICENSED
//
// 자료형 읽는법:
// - VEC1,2,3,4: 벡터 하나,둘,셋,넷
// - FLOAT2,3,4: 실수형 둘,셋,넷
//

#pragma once
#define __QS_MATH__

#ifndef __QS_QN__
#error include "qs_qn.h" first
#endif

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

QN_EXTC_BEGIN

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


//////////////////////////////////////////////////////////////////////////
// random

/// @brief 랜덤
typedef struct QMRANDOM
{
	nuint seed;
	nuint state1, state2;
} QmRandom;

/// @brief 랜덤 시드
QSAPI void qm_srand(QmRandom* r, nuint seed);

/// @brief 랜덤
QSAPI nuint qm_rand(QmRandom* r);

/// @brief 랜덤 실수 (0.0~1.0)
QSAPI float qm_randf(QmRandom* r);

/// @brief 랜덤 실수 (0.0~1.0)
QSAPI double qm_randd(QmRandom* r);


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
	int X, Y;
} QmInt2;

/// @brief INT3
typedef struct QMINT3
{
	int X, Y, Z;
} QmInt3;

/// @brief INT4
typedef struct QMINT4
{
	int X, Y, Z, W;
} QmInt4;

/// @brief UINT2
typedef struct QMUINT2
{
	uint X, Y;
} QmUint2;

/// @brief UINT3
typedef struct QMUINT3
{
	uint X, Y, Z;
} QmUint3;

/// @brief UINT4
typedef struct QMUINT4
{
	uint X, Y, Z, W;
} QmUint4;

/// @brief USHORT2
typedef union QMUSHORT2
{
	struct { ushort X, Y; };
	uint v;
} QmUshort2;

/// @brief USHORT4
typedef union QMUSHORT4
{
	struct { ushort X, Y, Z, W; };
	ullong v;
} QmUshort4;

/// @brief HALF2
typedef union QMHALF2
{
	struct { halffloat X, Y; };
	uint v;
} QmHalf2;

/// @brief HALF4
typedef union QMHALF4
{
	struct { halffloat X, Y, Z, W; };
	ullong v;
} QmHalf4;

/// @brief UINT 4/4/4/4
typedef union QMU4444
{
	struct { ushort B : 4, G : 4, R : 4, A : 4; };
	ushort v;
} QmU4444;

/// @brief UINT 5/5/5/1
typedef union QMU5551
{
	struct { ushort B : 5, G : 5, R : 5, A : 1; };
	ushort v;
} QmU5551;

/// @brief UINT 5/6/5
typedef union QMU565
{
	struct { ushort B : 5, G : 6, R : 5; };
	ushort v;
} QmU565;

/// @brief float 11/11/10
typedef union QMF111110
{
	struct { uint B : 11, G : 11, R : 10; };
	struct { uint Bm : 6, Be : 5, Gm : 6, Ge : 5, Rm : 5, Re : 5; };
	uint v;
} QmF111110;

/// @brief UINT 10/10/10/2
typedef union QMU1010102
{
	struct { uint B : 10, G : 10, R : 10, A : 2; };
	uint v;
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
} QmFloat4x4;


//////////////////////////////////////////////////////////////////////////
// types

// SIMD 벡터
#if defined QM_USE_AVX
typedef __m128			QMSVEC;
#elif defined QM_USE_NEON
typedef float32x4_t		QMSVEC;
#else
typedef union QMSVECFIU
{
	float f[4];
	int i[4];
	uint u[4];
} QMSVEC;
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
typedef union QMVECF
{
	float f[4];
	QMSVEC v;
	struct
	{
		float X, Y, Z, W;
	};
	struct
	{
		float R, G, B, A;
	};
} QmVec4, QmVecF, QmVec, QmColor;

/// @brief 벡터4
typedef union QMVECU
{
	uint u[4];
	int i[4];
	QMSVEC v;
	QmVec4 v4;
	struct
	{
		uint X, Y, Z, W;
	};
} QmVecU;

/// @brief 행렬4x4
typedef union QMMAT4
{
	float f[16];
	float m[4][4];
	QMSVEC v[4];
	QmVec4 r[4];
	struct
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;
	};
} QmMat4, QmMat;

/// @brief 위치값 (포인트)
typedef union QMPOINT
{
	int i[2];
	struct
	{
		int X, Y;
	};
} QmPoint;

/// @brief 크기
typedef union QMSIZE
{
	int i[2];
	struct
	{
		int Width, Height;
	};
	struct
	{
		int Minimum, Maximum;
	};
} QmSize;

/// @brief 사각형
typedef union QMRECT
{
	int i[4];
	struct
	{
		int Left, Top, Right, Bottom;
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
	byte b[4];
	uint v;
	struct { byte B, G, R, A; };
} QmKolor;


//////////////////////////////////////////////////////////////////////////
// integer & float

/// @brief 두 정수의 최대값
INLINE int qm_maxi(int a, int b)
{
	return QN_MAX(a, b);
}

/// @brief 두 정수의 최소값
INLINE int qm_mini(int a, int b)
{
	return QN_MIN(a, b);
}

/// @brief 정수의 절대값
INLINE int qm_absi(int v)
{
	return QN_ABS(v);
}

/// @brief 정수를 범위 내로 자르기
INLINE int qm_clampi(int v, int min, int max)
{
	return QN_CLAMP(v, min, max);
}

/// @brief Power Of Two 계산
INLINE bool qm_pot(uint v)
{
	return (v & (v - 1)) == 0;
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
	return QN_MAX(a, b);
}

/// @brief 두 실수의 최소값
INLINE float qm_minf(float a, float b)
{
	return QN_MIN(a, b);
}

/// @brief 실수의 절대값
INLINE float qm_absf(float v)
{
	return fabsf(v);
}

/// @brief 실수를 범위 내로 자르기
INLINE float qm_clampf(float v, float min, float max)
{
	return QN_CLAMP(v, min, max);
}

/// @brief 각도를 -180 ~ +180 사이로 자르기
INLINE float qm_cradf(float v)
{
	float f = fabsf(v + QM_PI);
	f -= (QM_PI2 * (float)(int)(f / QM_PI2)) - QM_PI;
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
	QMSVEC i = _mm_set_ss(v);
	QMSVEC o = _mm_sincos_ps(&i, i);
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
	QMSVEC i = _mm_set_ss(f);
	QMSVEC o = _mm_sqrt_ss(i);
	return _mm_cvtss_f32(o);
#elif defined QM_USE_NEON
	QMSVEC i = vdupq_n_f32(f);
	QMSVEC o = vsqrtq_f32(f);
	return vgetq_lane_f32(o, 0);
#else
	return sqrtf(f);
#endif
}

/// @brief 1을 나눈 제곱근
INLINE float qm_rsqrtf(float f)
{
#if defined QM_USE_AVX
	QMSVEC i = _mm_set_ss(f);
	QMSVEC o = _mm_rsqrt_ss(i);
	return _mm_cvtss_f32(o);
#elif defined QM_USE_NEON
	QMSVEC i = vdupq_n_f32(f);
	QMSVEC o = vrsqrteq_f32(f);
	return vgetq_lane_f32(o, 0);
#else
	return 1.0f / qm_sqrtf(f);
#endif
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
INLINE halffloat qm_f2hf(float v)
{
#if defined QM_USE_AVX2
	const __m128 r = _mm_set_ss(v);
	const __m128i p = _mm_cvtps_ph(r, _MM_FROUND_TO_NEAREST_INT);
	return (halffloat)_mm_extract_epi16(p, 0);
#elif defined QM_USE_NEON && (defined _M_ARM64 || defined __aarch64__)
	QMSVEC f = vdupq_n_f32(v);
	uint16x4_t r = vcvt_f16_f32(f);
	return vgetq_lane_u16(vreinterpret_u16_f16(r), 0);
#else
	uint u = *(const uint*)&v;
	const uint s = (u & 0x80000000U) >> 16U;
	u = u & 0x7FFFFFFFU;
	uint r;
	if (u > 0x47FFEFFFU)
		r = 0x7FFFU;
	else
	{
		if (u >= 0x38800000U)
			u += 0xC8000000U;
		else
		{
			const uint t = 113U - (u >> 23U);
			u = (0x800000U | (u & 0x7FFFFFU)) >> t;
		}
		r = ((u + 0x0FFFU + ((u >> 13U) & 1U)) >> 13U) & 0x7FFFU;
	}
	return (halfint)(r | s);
#endif
}

/// @brief 16비트 실수를 32비트 실수로 변환한다
/// @param v 변환할 16비트 실수
/// @return 변환한 32비트 실수
INLINE float qm_hf2f(const halffloat v)
{
#if defined QM_USE_AVX2
	const __m128i p = _mm_cvtsi32_si128((int)v);
	const __m128 r = _mm_cvtph_ps(p);
	return _mm_cvtss_f32(r);
#elif defined QM_USE_NEON && (defined _M_ARM64 || defined __aarch64__)
	uint16x4_t u = vdup_n_u16(v);
	uint32x4_t r = vcvt_f32_f16(vreinterpret_f16_u16(u));
	return vgetq_lane_f32(r, 0);
#else
	uint m = (uint)(v & 0x03FF);
	uint e;
	if ((v & 0x7C00) != 0)
		e = (uint)((v >> 10) & 0x1F);
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
		e = (uint)-112;
	uint r = (uint)((v & 0x8000) << 16) | (uint)((e + 112) << 23) | (uint)(m << 13);
	return *(float*)&r;
#endif
}


//////////////////////////////////////////////////////////////////////////
// function

INLINE QmVec4 QM_VECTORCALL qm_vec(float x, float y, float z, float w);
INLINE QmVec4 QM_VECTORCALL qm_vec_zero(void);
INLINE QmVec4 QM_VECTORCALL qm_vec_sp(float diag);
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_x(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_y(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_z(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_w(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_xy(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_zw(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec_neg(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_rcp(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_add(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec_sub(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec_mag(const QmVec4 left, float right);
INLINE QmVec4 QM_VECTORCALL qm_vec_abr(const QmVec4 left, float right);
INLINE QmVec4 QM_VECTORCALL qm_vec_mul(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec_madd(const QmVec4 left, const QmVec4 right, const QmVec4 add);
INLINE QmVec4 QM_VECTORCALL qm_vec_div(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec_min(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec_max(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec_clamp(const QmVec4 v, const QmVec4 min, const QmVec4 max);
INLINE QmVec4 QM_VECTORCALL qm_vec_crad(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_blend(const QmVec4 left, float leftScale, const QmVec4 right, float rightScale);
INLINE QmVec4 QM_VECTORCALL qm_vec_lerp(const QmVec4 left, const QmVec4 right, float scale);
INLINE QmVec4 QM_VECTORCALL qm_vec_lerp_len(const QmVec4 left, const QmVec4 right, float scale, float len);
INLINE QmVec4 QM_VECTORCALL qm_vec_hermite(const QmVec4 pos1, const QmVec4 tan1, const QmVec4 pos2, const QmVec4 tan2, float scale);
INLINE QmVec4 QM_VECTORCALL qm_vec_catmullrom(const QmVec4 pos1, const QmVec4 pos2, const QmVec4 pos3, const QmVec4 pos4, float scale);
INLINE QmVec4 QM_VECTORCALL qm_vec_barycentric(const QmVec4 pos1, const QmVec4 pos2, const QmVec4 pos3, float f, float g);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_sqrt(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_rsqrt(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_sin(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_cos(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_tan(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_asin(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_acos(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_atan(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_atan2(const QmVec4 y, const QmVec4 x);
INLINE void QM_VECTORCALL qm_vec_simd_sincos(const QmVec4 v, QmVec4* retSin, QmVec4* retCos);
INLINE float QM_VECTORCALL qm_vec_get_x(const QmVec4 v);
INLINE float QM_VECTORCALL qm_vec_get_y(const QmVec4 v);
INLINE float QM_VECTORCALL qm_vec_get_z(const QmVec4 v);
INLINE float QM_VECTORCALL qm_vec_get_w(const QmVec4 v);
INLINE bool QM_VECTORCALL qm_vec_eq(const QmVec4 left, const QmVec4 right);
INLINE bool QM_VECTORCALL qm_vec_eps(const QmVec4 left, const QmVec4 right, float epsilon);

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

INLINE QmVec4 QM_VECTORCALL qm_vec3(float x, float y, float z);
INLINE QmVec4 QM_VECTORCALL qm_vec3_cross(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec3_norm(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec3_reflect(const QmVec4 incident, const QmVec4 normal);
INLINE QmVec4 QM_VECTORCALL qm_vec3_refract(const QmVec4 incident, const QmVec4 normal, float eta);
INLINE QmVec4 QM_VECTORCALL qm_vec3_dir(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec3_rot(const QmVec4 v, const QmVec4 rotQuat);
INLINE QmVec4 QM_VECTORCALL qm_vec3_rot_inv(const QmVec4 v, const QmVec4 rotQuat);
INLINE QmVec4 QM_VECTORCALL qm_vec3_trfm(const QmVec4 v, const QmMat4 m);
INLINE QmVec4 QM_VECTORCALL qm_vec3_trfm_norm(const QmVec4 v, const QmMat4 m);
INLINE QmVec4 QM_VECTORCALL qm_vec3_trfm_coord(const QmVec4 v, const QmMat4 m);
INLINE QmVec4 QM_VECTORCALL qm_vec3_form_norm(const QmVec4 v1, const QmVec4 v2, const QmVec4 v3);
INLINE QmVec4 QM_VECTORCALL qm_vec3_closed(const QmVec4 loc, const QmVec4 begin, const QmVec4 end);
INLINE QmVec4 QM_VECTORCALL qm_vec3_proj(const QmVec4 v, const QmMat4 proj, const QmMat4 view, const QmMat4 world, float x, float y, float width, float height, float zn, float zf);
INLINE QmVec4 QM_VECTORCALL qm_vec3_unproj(const QmVec4 v, const QmMat4 proj, const QmMat4 view, const QmMat4 world, float x, float y, float width, float height, float zn, float zf);
INLINE QmVec4 QM_VECTORCALL qm_vec3_simd_dot(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec3_dot(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec3_len_sq(const QmVec4 v);
INLINE float QM_VECTORCALL qm_vec3_len(const QmVec4 v);
INLINE float QM_VECTORCALL qm_vec3_dist_sq(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec3_dist(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec3_rad_sq(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec3_rad(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec3_angle(const QmVec4 leftNormal, const QmVec4 rightNormal);
INLINE bool QM_VECTORCALL qm_vec3_is_between(const QmVec4 queryPoint, const QmVec4 begin, const QmVec4 end);
INLINE bool QM_VECTORCALL qm_vec3_eq(const QmVec4 left, const QmVec4 right);
INLINE bool QM_VECTORCALL qm_vec3_eps(const QmVec4 left, const QmVec4 right, float epsilon);

INLINE QmVec4 QM_VECTORCALL qm_vec4(float x, float y, float z, float w);
INLINE QmVec4 QM_VECTORCALL qm_vec4v(const QmVec4 v, float w);
INLINE QmVec4 QM_VECTORCALL qm_vec4_cross(const QmVec4 a, const QmVec4 b, const QmVec4 c);
INLINE QmVec4 QM_VECTORCALL qm_vec4_norm(const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec4_reflect(const QmVec4 incident, const QmVec4 normal);
INLINE QmVec4 QM_VECTORCALL qm_vec4_refract(const QmVec4 incident, const QmVec4 normal, float eta);
INLINE QmVec4 QM_VECTORCALL qm_vec4_trfm(const QmVec4 v, const QmMat4 m);
INLINE QmVec4 QM_VECTORCALL qm_vec4_simd_dot(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec4_dot(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec4_len_sq(const QmVec4 v);
INLINE float QM_VECTORCALL qm_vec4_len(const QmVec4 v);
INLINE float QM_VECTORCALL qm_vec4_dist_sq(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_vec4_dist(const QmVec4 left, const QmVec4 right);
INLINE bool QM_VECTORCALL qm_vec4_eq(const QmVec4 left, const QmVec4 right);
INLINE bool QM_VECTORCALL qm_vec4_eps(const QmVec4 left, const QmVec4 right, float epsilon);

INLINE QmVec4 QM_VECTORCALL qm_quat(float x, float y, float z, float w);
INLINE QmVec4 QM_VECTORCALL qm_quatv(const QmVec4 v, float w);
INLINE QmVec4 QM_VECTORCALL qm_quat_unit(void);
INLINE QmVec4 QM_VECTORCALL qm_quat_mul(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_quat_norm(const QmVec4 q);
INLINE QmVec4 QM_VECTORCALL qm_quat_cjg(const QmVec4 q);
INLINE QmVec4 QM_VECTORCALL qm_quat_inv(const QmVec4 q);
INLINE QmVec4 QM_VECTORCALL qm_quat_exp(const QmVec4 q);
INLINE QmVec4 QM_VECTORCALL qm_quat_ln(const QmVec4 q);
INLINE QmVec4 QM_VECTORCALL qm_quat_slerp(const QmVec4 left, const QmVec4 right, float scale);
INLINE QmVec4 QM_VECTORCALL qm_quat_squad(const QmVec4 q1, const QmVec4 q2, const QmVec4 q3, const QmVec4 q4, float scale);
INLINE QmVec4 QM_VECTORCALL qm_quat_barycentric(const QmVec4 q1, const QmVec4 q2, const QmVec4 q3, float f, float g);
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_vec(const QmVec4 rot3);
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_axis(const QmVec4 axis3, float angle);
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_x(float rot);
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_y(float rot);
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_z(float rot);
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_mat4(const QmMat4 rot);
INLINE QmVec4 QM_VECTORCALL qm_quat_simd_dot(const QmVec4 left, const QmVec4 right);
INLINE float QM_VECTORCALL qm_quat_dot(const QmVec4 left, const QmVec4 right);
INLINE void QM_VECTORCALL qm_quat_to_axis_angle(const QmVec4 q, QmVec4* axis, float* angle);
INLINE bool QM_VECTORCALL qm_quat_isu(const QmVec4 v);

INLINE QmVec4 QM_VECTORCALL qm_plane(float a, float b, float c, float d);
INLINE QmVec4 QM_VECTORCALL qm_planev(const QmVec4 v, float d);
INLINE QmVec4 QM_VECTORCALL qm_planevv(const QmVec4 v, const QmVec4 normal);
INLINE QmVec4 QM_VECTORCALL qm_planevvv(const QmVec4 v1, const QmVec4 v2, const QmVec4 v3);
INLINE QmVec4 QM_VECTORCALL qm_plane_unit(void);
INLINE QmVec4 QM_VECTORCALL qm_plane_norm(const QmVec4 plane);
INLINE QmVec4 QM_VECTORCALL qm_plane_rnorm(const QmVec4 plane);
INLINE QmVec4 QM_VECTORCALL qm_plane_trfm(const QmVec4 plane, const QmMat4 m);
INLINE QmVec4 QM_VECTORCALL qm_plane_simd_dot(const QmVec4 plane, const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_plane_simd_dot_coord(const QmVec4 plane, const QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_plane_simd_dot_normal(const QmVec4 plane, const QmVec4 v);
INLINE float QM_VECTORCALL qm_plane_dot(const QmVec4 plane, const QmVec4 v);
INLINE float QM_VECTORCALL qm_plane_dot_coord(const QmVec4 plane, const QmVec4 v);
INLINE float QM_VECTORCALL qm_plane_dot_normal(const QmVec4 plane, const QmVec4 v);
INLINE float QM_VECTORCALL qm_plane_dist_vec3(const QmVec4 plane, const QmVec4 v);
INLINE float QM_VECTORCALL qm_plane_dist_line(const QmVec4 plane, const QmVec4 linePoint1, const QmVec4 linePoint2);
INLINE int QM_VECTORCALL qm_plane_relation(const QmVec4 plane, const QmVec4 v);
INLINE bool QM_VECTORCALL qm_plane_intersect_line(const QmVec4 plane, const QmVec4 linePoint1, const QmVec4 linePoint2, QmVec4* intersectPoint);
INLINE bool QM_VECTORCALL qm_plane_intersect_line2(const QmVec4 plane, const QmVec4 loc, const QmVec4 dir, QmVec4* intersectPoint);
INLINE bool QM_VECTORCALL qm_plane_intersect_plane(const QmVec4 plane1, const QmVec4 plane2, QmVec4* loc, QmVec4* dir);
INLINE bool QM_VECTORCALL qm_plane_intersect_planes(const QmVec4 plane1, const QmVec4 plane2, const QmVec4 plane3, QmVec4* intersectPoint);

INLINE QmVec4 QM_VECTORCALL qm_color(float r, float g, float b, float a);
INLINE QmVec4 QM_VECTORCALL qm_coloru(uint value);
INLINE QmVec4 QM_VECTORCALL qm_colork(QmKolor k);
INLINE QmVec4 QM_VECTORCALL qm_color_unit(void);
INLINE QmVec4 QM_VECTORCALL qm_color_sp(float value, float alpha);
INLINE QmVec4 QM_VECTORCALL qm_color_neg(const QmVec4 c);
INLINE QmVec4 QM_VECTORCALL qm_color_mod(const QmVec4 left, const QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_color_contrast(const QmVec4 c, float contrast);
INLINE QmVec4 QM_VECTORCALL qm_color_saturation(const QmVec4 c, float saturation);

INLINE QmMat4 QM_VECTORCALL qm_mat4_unit(void);
INLINE QmMat4 QM_VECTORCALL qm_mat4_zero(void);
INLINE QmMat4 QM_VECTORCALL qm_mat4_diag(float diag);
INLINE QmMat4 QM_VECTORCALL qm_mat4_add(const QmMat4 left, const QmMat4 right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_sub(const QmMat4 left, const QmMat4 right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_mag(const QmMat4 left, float right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_abr(const QmMat4 left, float right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_tran(const QmMat4 m);
INLINE QmMat4 QM_VECTORCALL qm_mat4_mul(const QmMat4 left, const QmMat4 right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_tmul(const QmMat4 left, const QmMat4 right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_inv(const QmMat4 m);
INLINE QmMat4 QM_VECTORCALL qm_mat4_scl(float x, float y, float z);
INLINE QmMat4 QM_VECTORCALL qm_mat4_scl_vec3(const QmVec4 v);
INLINE QmMat4 QM_VECTORCALL qm_mat4_loc(float x, float y, float z);
INLINE QmMat4 QM_VECTORCALL qm_mat4_loc_vec(const QmVec4 v);
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_rot(const QmVec4 axis, float angle);
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_vec3(const QmVec4 rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_quat(const QmVec4 rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_x(float rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_y(float rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_z(float rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_affine(const QmVec4* scl, const QmVec4* rotcenter, const QmVec4* rot, const QmVec4* loc);
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm(QmVec4 loc, QmVec4 rot, QmVec4* scl);
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm_vec(QmVec4 loc, QmVec4 rot, QmVec4* scl);
INLINE QmMat4 QM_VECTORCALL qm_mat4_reflect(const QmVec4 plane);
INLINE QmMat4 QM_VECTORCALL qm_mat4_shadow(const QmVec4 plane, const QmVec4 light);
INLINE QmMat4 QM_VECTORCALL qm_mat4_lookat_lh(QmVec4 eye, QmVec4 at, QmVec4 up);
INLINE QmMat4 QM_VECTORCALL qm_mat4_lookat_rh(QmVec4 eye, QmVec4 at, QmVec4 up);
INLINE QmMat4 QM_VECTORCALL qm_mat4_perspective_lh(float fov, float aspect, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_perspective_rh(float fov, float aspect, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_lh(float width, float height, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_rh(float width, float height, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_offcenter_lh(float left, float top, float right, float bottom, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_offcenter_rh(float left, float top, float right, float bottom, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_viewport(float x, float y, float width, float height);
INLINE bool QM_VECTORCALL qm_mat4_isu(QmMat4 m);

INLINE QmPoint qm_point(int x, int y);
INLINE QmPoint qm_pointv(const QmVec2 v);
INLINE QmPoint qm_pointv4(const QmVec4 v);
INLINE QmPoint qm_point_zero(void);
INLINE QmPoint qm_point_sp(const int diag);
INLINE QmPoint qm_point_neg(const QmPoint p);
INLINE QmPoint qm_point_add(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_sub(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_mag(const QmPoint left, int right);
INLINE QmPoint qm_point_abr(const QmPoint left, int right);
INLINE QmPoint qm_point_mul(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_div(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_min(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_max(const QmPoint left, const QmPoint right);
INLINE QmPoint qm_point_cross(const QmPoint left, const QmPoint right);
INLINE int qm_point_dot(const QmPoint left, const QmPoint right);
INLINE int qm_point_len_sq(const QmPoint pt);
INLINE int qm_point_dist_sq(const QmPoint left, const QmPoint right);
INLINE float qm_point_len(const QmPoint pt);
INLINE float qm_point_dist(const QmPoint left, const QmPoint right);
INLINE bool qm_point_eq(const QmPoint left, const QmPoint right);
INLINE bool qm_point_isz(const QmPoint pt);

INLINE QmSize qm_size(int width, int height);
INLINE QmSize qm_size_rect(const QmRect rt);
INLINE QmSize qm_size_sp(const int diag);
INLINE QmSize qm_size_add(const QmSize left, const QmSize right);
INLINE QmSize qm_size_sub(const QmSize left, const QmSize right);
INLINE QmSize qm_size_mag(const QmSize left, int right);
INLINE QmSize qm_size_abr(const QmSize left, int right);
INLINE QmSize qm_size_mul(const QmSize left, const QmSize right);
INLINE QmSize qm_size_div(const QmSize left, const QmSize right);
INLINE QmSize qm_size_min(const QmSize left, const QmSize right);
INLINE QmSize qm_size_max(const QmSize left, const QmSize right);
INLINE int qm_size_len_sq(const QmSize s);
INLINE float qm_size_len(const QmSize v);
INLINE float qm_size_get_aspect(const QmSize s);
INLINE float qm_size_calc_dpi(const QmSize pt, float horizontal, float vertical);
INLINE QmPoint qm_size_locate_center(const QmSize s, int width, int height);
INLINE bool qm_size_eq(const QmSize left, const QmSize right);

INLINE QmRect qm_rect(int left, int top, int right, int bottom);
INLINE QmRect qm_rect_size(int x, int y, int width, int height);
INLINE QmRect qm_rect_pos_size(QmPoint pos, QmSize size);
INLINE QmRect qm_rect_zero(void);
INLINE QmRect qm_rect_sp(const int diag);
INLINE QmRect qm_rect_add(const QmRect left, const QmRect right);
INLINE QmRect qm_rect_sub(const QmRect left, const QmRect right);
INLINE QmRect qm_rect_mag(const QmRect left, int right);
INLINE QmRect qm_rect_abr(const QmRect left, int right);
INLINE QmRect qm_rect_min(const QmRect left, const QmRect right);
INLINE QmRect qm_rect_max(const QmRect left, const QmRect right);
INLINE QmRect qm_rect_inflate(const QmRect rt, int left, int top, int right, int bottom);
INLINE QmRect qm_rect_deflate(const QmRect rt, int left, int top, int right, int bottom);
INLINE QmRect qm_rect_offset(const QmRect rt, int left, int top, int right, int bottom);
INLINE QmRect qm_rect_move(const QmRect rt, int left, int top);
INLINE QmRect qm_rect_set_size(const QmRect rt, int width, int height);
INLINE int qm_rect_get_width(const QmRect rt);
INLINE int qm_rect_get_height(const QmRect rt);
INLINE bool qm_rect_in(const QmRect rt, const int x, const int y);
INLINE bool qm_rect_include(const QmRect dest, const QmRect target);
INLINE bool qm_rect_intersect(const QmRect r1, const QmRect r2, QmRect* p);
INLINE bool qm_rect_eq(const QmRect left, const QmRect right);
INLINE bool qm_rect_isz(const QmRect pv);
#ifdef _WINDEF_
INLINE QmRect qm_rect_RECT(RECT rt);
INLINE RECT qm_rect_to_RECT(const QmRect rt);
#endif


//////////////////////////////////////////////////////////////////////////
// Color value

QN_CONST_ANY QmColor QMCOLOR_EMPTY = { { 0.0f, 0.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmColor QMCOLOR_BLACK = { { 0.0f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_WHITE = { { 1.0f, 1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_RED = { { 1.0f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_GREEN = { { 0.0f, 1.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_BLUE = { { 0.0f, 0.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_YELLOW = { { 1.0f, 0.92f, 0.016f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_CYAN = { { 0.0f, 1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_MAGENTA = { { 1.0f, 0.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_GRAY = { { 0.5f, 0.5f, 0.5f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_DARKGRAY = { { 0.25f, 0.25f, 0.25f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_LIGHTGRAY = { { 0.75f, 0.75f, 0.75f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_HALF_RED = { { 0.5f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_HALF_GREEN = { { 0.0f, 0.5f, 0.0f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_HALF_BLUE = { { 0.0f, 0.0f, 0.5f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_HALF_YELLOW = { { 0.5f, 0.496f, 0.01f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_HALF_CYAN = { { 0.0f, 0.5f, 0.5f, 1.0f } };
QN_CONST_ANY QmColor QMCOLOR_HALF_MAGENTA = { { 0.5f, 0.0f, 0.5f, 1.0f } };


//////////////////////////////////////////////////////////////////////////
// SIMD support

QN_CONST_ANY QmVec4 QMCONST_ZERO = { { 0.0f, 0.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_ONE = { { 1.0f, 1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_NEG = { { -1.0f, -1.0f, -1.0f, -1.0f } };
QN_CONST_ANY QmVec4 QMCONST_UNIT_R0 = { { 1.0f, 0.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_UNIT_R1 = { { 0.0f, 1.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_UNIT_R2 = { { 0.0f, 0.0f, 1.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_UNIT_R3 = { { 0.0f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMCONST_XY00 = { { 1.0f, 1.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMCONST_00ZW = { { 0.0f, 0.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVecU QMCONST_S1000 = { { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 } };
QN_CONST_ANY QmVecU QMCONST_S1100 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 } };
QN_CONST_ANY QmVecU QMCONST_S1110 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } };
QN_CONST_ANY QmVecU QMCONST_S1011 = { { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF } };

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
// vector

/// @brief 벡터 값 설정
INLINE QmVec4 QM_VECTORCALL qm_vec(float x, float y, float z, float w)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_setr_ps(x, y, z, w);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	float32x2_t xy = vcreate_f32(((ullong)(*(uint*)&x)) | (((ullong)(*(uint*)&y)) << 32));
	float32x2_t zw = vcreate_f32(((ullong)(*(uint*)&z)) | (((ullong)(*(uint*)&w)) << 32));
	QMSVEC h = vcombine_f32(xy, zw);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { x, y, z, w } };
#endif
}

/// @brief 0 벡터 얻기
INLINE QmVec4 QM_VECTORCALL qm_vec_zero(void)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_setzero_ps();
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vdupq_n_f32(0.0f);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { 0.0f, 0.0f, 0.0f, 0.0f } };
#endif
}

/// @brief 모두 같은값으로 채우기
INLINE QmVec4 QM_VECTORCALL qm_vec_sp(float diag)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_set1_ps(diag);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vdupq_n_f32(diag);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { diag, diag, diag, diag } };
#endif
}

/// @brief X값으로 모두 채우기
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_x(const QmVec4 v)
{
#if defined QM_USE_AVX2
	QMSVEC h = _mm_broadcastss_ps(v.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_AVX
	QMSVEC h = _mm_permute_ps(v.v, _MM_SHUFFLE(0, 0, 0, 0));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vdupq_lane_f32(vget_low_f32(v.v), 0);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { v.X, v.X, v.X, v.X } };
#endif
}

/// @brief Y값으로 모두 채우기
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_y(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_permute_ps(v.v, _MM_SHUFFLE(1, 1, 1, 1));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vdupq_lane_f32(vget_low_f32(v.v), 1);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { v.Y, v.Y, v.Y, v.Y } };
#endif
}

/// @brief Z값으로 모두 채우기
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_z(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_permute_ps(v.v, _MM_SHUFFLE(2, 2, 2, 2));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vdupq_lane_f32(vget_high_f32(v.v), 0);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { v.Z, v.Z, v.Z, v.Z } };
#endif
}

/// @brief W값으로 모두 채우기
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_w(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_permute_ps(v.v, _MM_SHUFFLE(3, 3, 3, 3));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vdupq_lane_f32(vget_high_f32(v.v), 1);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { v.W, v.W, v.W, v.W } };
#endif
}

/// @brief xy로 채우기
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_xy(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_unpacklo_ps(left.v, right.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vzipq_f32(left.v, right.v).val[0];
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X, right.X, left.Y, right.Y } };
#endif
}

/// @brief zw로 채우기
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_zw(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_unpackhi_ps(left.v, right.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vzipq_f32(left.v, right.v).val[1];
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.Z, right.Z, left.W, right.W } };
#endif
}

/// @brief MSB 세팅 (부호)
INLINE QmVec4 QM_VECTORCALL qm_vec_sp_msb(void)
{
#if defined QM_USE_AVX
	__m128i i = _mm_set1_epi32((int)0x80000000U);
	QMSVEC h = _mm_castsi128_ps(i);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vreinterpretq_f32_u32(vdupq_n_u32(0x80000000U));
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { 0x80000000U, 0x80000000U, 0x80000000U, 0x80000000U } };
#endif
}

/// @brief 벡터 반전
INLINE QmVec4 QM_VECTORCALL qm_vec_neg(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_sub_ps(_mm_setzero_ps(), v.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vnegq_f32(v.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { -v.X, -v.Y, -v.Z, -v.W } };
#endif
}

/// @brief 벡터 역수 (1.0f / v)
INLINE QmVec4 QM_VECTORCALL qm_vec_rcp(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_div_ps(QMCONST_ONE.v, v.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vrecpeq_f32(v.v);
	h = vmulq_f32(vrecpsq_f32(v.v, h), h);
	h = vmulq_f32(vrecpsq_f32(v.v, h), h);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { 1.0f / v.X, 1.0f / v.Y, 1.0f / v.Z, 1.0f / v.W } };
#endif
}

/// @brief 벡터 덧셈
INLINE QmVec4 QM_VECTORCALL qm_vec_add(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_add_ps(left.v, right.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vaddq_f32(left.v, right.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W } };
#endif
}

/// @brief 벡터 뺄셈
INLINE QmVec4 QM_VECTORCALL qm_vec_sub(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_sub_ps(left.v, right.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vsubq_f32(left.v, right.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W } };
#endif
}

/// @brief 벡터 확대
INLINE QmVec4 QM_VECTORCALL qm_vec_mag(const QmVec4 left, float right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_mul_ps(left.v, _mm_set1_ps(right));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmulq_n_f32(left.v, right);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X * right, left.Y * right, left.Z * right, left.W * right } };
#endif
}

/// @brief 벡터 축소
INLINE QmVec4 QM_VECTORCALL qm_vec_abr(const QmVec4 left, float right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_div_ps(left.v, _mm_set1_ps(right));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vdupq_n_f32(right);
	QMSVEC h = vdivq_f32(left.v, h);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X / right, left.Y / right, left.Z / right, left.W / right } };
#endif
}

/// @brief 벡터 곱셈
INLINE QmVec4 QM_VECTORCALL qm_vec_mul(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_mul_ps(left.v, right.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmulq_f32(left.v, right.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W } };
#endif
}

/// @brief 벡터 곱하고 더하기
INLINE QmVec4 QM_VECTORCALL qm_vec_madd(const QmVec4 left, const QmVec4 right, const QmVec4 add)
{
#if defined QM_USE_AVX
	QMSVEC h = _MM_FMADD_PS(left.v, right.v, add.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmlaq_f32(add.v, left.v, right.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X * right.X + add.X, left.Y * right.Y + add.Y, left.Z * right.Z + add.Z, left.W * right.W + add.W } };
#endif
}

/// @brief 벡터 곱하고 반대로 빼기
INLINE QmVec4 QM_VECTORCALL qm_vec_msub(const QmVec4 left, const QmVec4 right, const QmVec4 sub)
{
#if defined QM_USE_AVX
	QMSVEC h = _MM_FNMADD_PS(left.v, right.v, sub.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmlsq_f32(sub.v, left.v, right.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { sub.X - left.X * right.X, sub.Y - left.Y * right.Y, sub.Z - left.Z * right.Z, sub.W - left.W * right.W } };
#endif
}

/// @brief 벡터 나눗셈
INLINE QmVec4 QM_VECTORCALL qm_vec_div(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_div_ps(left.v, right.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vdivq_f32(left.v, right.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X / right.X, left.Y / right.Y, left.Z / right.Z, left.W / right.W } };
#endif
}

/// @brief 벡터 최소값
INLINE QmVec4 QM_VECTORCALL qm_vec_min(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_min_ps(left.v, right.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vminq_f32(left.v, right.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { QN_MIN(left.X, right.X), QN_MIN(left.Y, right.Y), QN_MIN(left.Z, right.Z), QN_MIN(left.W, right.W) } };
#endif
}

/// @brief 벡터 최대값
INLINE QmVec4 QM_VECTORCALL qm_vec_max(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_max_ps(left.v, right.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmaxq_f32(left.v, right.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { QN_MAX(left.X, right.X), QN_MAX(left.Y, right.Y), QN_MAX(left.Z, right.Z), QN_MAX(left.W, right.W) } };
#endif
}

/// @brief 벡터 범위 제한
INLINE QmVec4 QM_VECTORCALL qm_vec_clamp(const QmVec4 v, const QmVec4 min, const QmVec4 max)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_min_ps(_mm_max_ps(v.v, min.v), max.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vminq_f32(vmaxq_f32(v.v, min.v), max.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { QN_CLAMP(v.X, min.X, max.X), QN_CLAMP(v.Y, min.Y, max.Y), QN_CLAMP(v.Z, min.Z, max.Z), QN_CLAMP(v.W, min.W, max.W) } };
#endif
}

/// @brief 벡터 각도 범위 제한
INLINE QmVec4 QM_VECTORCALL qm_vec_crad(const QmVec4 v)
{
#if defined QM_USE_AVX
	static const QmVec4 rpih = { { QM_RPI_H, QM_RPI_H, QM_RPI_H, QM_RPI_H } };
	static const QmVec4 tau = { { QM_TAU, QM_TAU, QM_TAU, QM_TAU } };
	QMSVEC h = _mm_mul_ps(v.v, rpih.v);
	h = _mm_round_ps(h, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
	h = _MM_FNMADD_PS(h, tau.v, v.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	static const QmVec4 rpih = { { QM_RPI_H, QM_RPI_H, QM_RPI_H, QM_RPI_H } };
	static const QmVec4 tau = { { QM_TAU, QM_TAU, QM_TAU, QM_TAU } };
	QMSVEC h = vmulq_f32(v.v, rpih.v);
	h = vrndnq_f32(h);
	h = vmlsq_f32(v.v, h, tau.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { qm_cradf(v.X), qm_cradf(v.Y), qm_cradf(v.Z), qm_cradf(v.W) } };
#endif
}

/// @brief 벡터 블랜드
INLINE QmVec4 QM_VECTORCALL qm_vec_blend(const QmVec4 left, float leftScale, const QmVec4 right, float rightScale)
{
#if defined QM_USE_AVX
	QMSVEC h = _MM_FMADD_PS(left.v, _mm_set1_ps(leftScale), _mm_mul_ps(right.v, _mm_set1_ps(rightScale)));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmlaq_n_f32(vmulq_n_f32(right.v, rightScale), left.v, leftScale);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X * leftScale + right.X * rightScale, left.Y * leftScale + right.Y * rightScale, left.Z * leftScale + right.Z * rightScale, left.W * leftScale + right.W * rightScale } };
#endif
}

/// @brief 벡터 선형 혼합
INLINE QmVec4 QM_VECTORCALL qm_vec_lerp(const QmVec4 left, const QmVec4 right, float scale)
{
#if defined QM_USE_AVX
	QMSVEC h = _MM_FMADD_PS(_mm_sub_ps(right.v, left.v), _mm_set1_ps(scale), left.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmlaq_n_f32(left.v, vsubq_f32(right.v, left.v), scale);
	return *(QmVec4*)&h;
#else
	QmVec4 s = qm_vec_sp(scale);
	QmVec4 l = qm_vec_sub(right, left);
	return qm_vec_madd(l, s, left);
#endif
}

/// @brief 벡터 선형 혼합 + 확대
INLINE QmVec4 QM_VECTORCALL qm_vec_lerp_len(const QmVec4 left, const QmVec4 right, float scale, float len)
{
#if defined QM_USE_AVX
	QMSVEC h = _MM_FMADD_PS(_mm_sub_ps(right.v, left.v), _mm_set1_ps(scale), left.v);
	h = _mm_mul_ps(h, _mm_set1_ps(len));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmlaq_n_f32(left.v, vsubq_f32(right.v, left.v), scale);
	h = vmulq_n_f32(h, len);
	return *(QmVec4*)&h;
#else
	QmVec4 p = qm_vec_sp(scale);
	QmVec4 s = qm_vec_rcp(p);
	QmVec4 l = qm_vec_sub(right, left);
	QmVec4 h = qm_vec_madd(l, s, left);
	return qm_vec_mag(h, len);
#endif
}

/// @brief 벡터 허밋 계산
INLINE QmVec4 QM_VECTORCALL qm_vec_hermite(const QmVec4 pos1, const QmVec4 tan1, const QmVec4 pos2, const QmVec4 tan2, float scale)
{
#if defined QM_USE_AVX
	float s2 = scale * scale;
	float s3 = scale * s2;
	QMSVEC p1 = _mm_set1_ps(2.0f * s3 - 3.0f * s2 + 1.0f);
	QMSVEC t1 = _mm_set1_ps(s3 - 2.0f * s2 + scale);
	QMSVEC p2 = _mm_set1_ps(-2.0f * s3 + 3.0f * s2);
	QMSVEC t2 = _mm_set1_ps(s3 - s2);
	QMSVEC h = _mm_mul_ps(p1, pos1.v);
	h = _MM_FMADD_PS(t1, tan1.v, h);
	h = _MM_FMADD_PS(p2, pos2.v, h);
	h = _MM_FMADD_PS(t2, tan2.v, h);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	float s2 = scale * scale;
	float s3 = scale * s2;
	float p1 = 2.0f * s3 - 3.0f * s2 + 1.0f;
	float t1 = s3 - 2.0f * s2 + scale;
	float p2 = -2.0f * s3 + 3.0f * s2;
	float t2 = s3 - s2;
	QMSVEC h = vmulq_n_f32(pos1.v, p1);
	h = vmlaq_n_f32(h, tan1.v, t1);
	h = vmlaq_n_f32(h, pos2.v, p2);
	h = vmlaq_n_f32(h, tan2.v, t2);
	return *(QmVec4*)&h;
#else
	float s2 = scale * scale;
	float s3 = scale * s2;
	QmVec4 p1 = qm_vec_sp(2.0f * s3 - 3.0f * s2 + 1.0f);
	QmVec4 t1 = qm_vec_sp(s3 - 2.0f * s2 + scale);
	QmVec4 p2 = qm_vec_sp(-2.0f * s3 + 3.0f * s2);
	QmVec4 t2 = qm_vec_sp(s3 - s2);
	QmVec4 h = qm_vec_mul(p1, pos1);
	h = qm_vec_madd(t1, tan1, h);
	h = qm_vec_madd(p2, pos2, h);
	h = qm_vec_madd(t2, tan2, h);
	return h;
#endif
}

/// @brief 벡터 캣멀롬 스플라인 계산
INLINE QmVec4 QM_VECTORCALL qm_vec_catmullrom(const QmVec4 pos1, const QmVec4 pos2, const QmVec4 pos3, const QmVec4 pos4, float scale)
{
#if defined QM_USE_AVX
	float s2 = scale * scale;
	float s3 = scale * s2;
	QMSVEC p1 = _mm_set1_ps((-s3 + 2.0f * s2 - scale) * 0.5f);
	QMSVEC p2 = _mm_set1_ps((3.0f * s3 - 5.0f * s2 + 2.0f) * 0.5f);
	QMSVEC p3 = _mm_set1_ps((-3.0f * s3 + 4.0f * s2 + scale) * 0.5f);
	QMSVEC p4 = _mm_set1_ps((s3 - s2) * 0.5f);
	p2 = _mm_mul_ps(pos2.v, p2);
	p1 = _MM_FMADD_PS(pos1.v, p1, p2);
	p4 = _mm_mul_ps(pos4.v, p4);
	p3 = _MM_FMADD_PS(pos3.v, p3, p4);
	p1 = _mm_add_ps(p1, p3);
	return *(QmVec4*)&p1;
#elif defined QM_USE_NEON
	float s2 = scale * scale;
	float s3 = scale * s2;
	float p1 = (-s3 + 2.0f * s2 - scale) * 0.5f;
	float p2 = (3.0f * s3 - 5.0f * s2 + 2.0f) * 0.5f;
	float p3 = (-3.0f * s3 + 4.0f * s2 + scale) * 0.5f;
	float p4 = (s3 - s2) * 0.5f;
	p2 = vmulq_n_f32(pos2.v, p2);
	p1 = vmlaq_n_f32(p2, pos1.v, p1);
	p4 = vmulq_n_f32(pos4.v, p4);
	p3 = vmlaq_n_f32(p4, pos3.v, p3);
	p1 = vaddq_f32(p1, p3);
	return *(QmVec4*)&p1;
#else
	float s2 = scale * scale;
	float s3 = scale * s2;
	QmVec4 p1 = qm_vec_sp((-s3 + 2.0f * s2 - scale) * 0.5f);
	QmVec4 p2 = qm_vec_sp((3.0f * s3 - 5.0f * s2 + 2.0f) * 0.5f);
	QmVec4 p3 = qm_vec_sp((-3.0f * s3 + 4.0f * s2 + scale) * 0.5f);
	QmVec4 p4 = qm_vec_sp((s3 - s2) * 0.5f);
	QmVec4 h = qm_vec_mul(p1, pos1);
	h = qm_vec_madd(p2, pos2, h);
	h = qm_vec_madd(p3, pos3, h);
	h = qm_vec_madd(p4, pos4, h);
	return h;
#endif
}

/// @brief 벡터 질량 중심 좌표계 이동 계산
INLINE QmVec4 QM_VECTORCALL qm_vec_barycentric(const QmVec4 pos1, const QmVec4 pos2, const QmVec4 pos3, float f, float g)
{
#if defined QM_USE_AVX
	QMSVEC p21 = _mm_sub_ps(pos2.v, pos1.v);
	QMSVEC p31 = _mm_sub_ps(pos3.v, pos1.v);
	QMSVEC df = _mm_set1_ps(f);
	QMSVEC dg = _mm_set1_ps(g);
	QMSVEC h = _MM_FMADD_PS(p31, dg, _MM_FMADD_PS(p21, df, pos1.v));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC p21 = vsubq_f32(pos2.v, pos1.v);
	QMSVEC p31 = vsubq_f32(pos3.v, pos1.v);
	QMSVEC h = vmlaq_n_f32(vmlaq_n_f32(pos1.v, p21, f), p31, g);
	return *(QmVec4*)&h;
#else
	QmVec4 p21 = qm_vec_sub(pos2, pos1);
	QmVec4 p31 = qm_vec_sub(pos3, pos1);
	QmVec4 df = qm_vec_sp(f);
	QmVec4 dg = qm_vec_sp(g);
	return qm_vec_madd(p31, dg, qm_vec_madd(p21, df, pos1));
#endif
}

/// @brief 벡터 한번에 제곱근
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_sqrt(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_sqrt_ps(v.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vrsqrteq_f32(v.v);
	h = vmulq_f32(vrsqrtsq_f32(v.v, vmulq_f32(h, h)), h);
	return *(QmVec4*)&h;
#else
	float f = qm_sqrtf(v.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 제곱근 역수
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_rsqrt(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_rsqrt_ps(v.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vrsqrteq_f32(v.v);
	return *(QmVec4*)&h;
#else
	float f = qm_rsqrtf(v.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 사인 코사인
INLINE void QM_VECTORCALL qm_vec_simd_sincos(const QmVec4 v, QmVec4* retSin, QmVec4* retCos)
{
#if defined _MSC_VER && defined QM_USE_AVX
	retSin->v = _mm_sincos_ps(&retCos->v, v.v);
#else
	float s = sinf(v.X);
	float c = cosf(v.X);
	*retSin = (QmVec4){ { s, s, s, s } };
	*retCos = (QmVec4){ { c, c, c, c } };
#endif
}

/// @brief 벡터 한번에 사인
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_sin(const QmVec4 v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	QMSVEC h = _mm_sin_ps(v.v);
	return *(QmVec4*)&h;
#else
	float f = sinf(v.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 코사인
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_cos(const QmVec4 v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	QMSVEC h = _mm_cos_ps(v.v);
	return *(QmVec4*)&h;
#else
	float f = cosf(v.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 탄젠트
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_tan(const QmVec4 v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	QMSVEC h = _mm_tan_ps(v.v);
	return *(QmVec4*)&h;
#else
	float f = tanf(v.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 아크사인
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_asin(const QmVec4 v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	QMSVEC h = _mm_asin_ps(v.v);
	return *(QmVec4*)&h;
#else
	float f = asinf(v.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 아크코사인
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_acos(const QmVec4 v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	QMSVEC h = _mm_acos_ps(v.v);
	return *(QmVec4*)&h;
#else
	float f = acosf(v.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 아크탄젠트
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_atan(const QmVec4 v)
{
#if defined _MSC_VER && defined QM_USE_AVX
	QMSVEC h = _mm_atan_ps(v.v);
	return *(QmVec4*)&h;
#else
	float f = atanf(v.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 아크탄젠트 (y/x)
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_atan2(const QmVec4 y, const QmVec4 x)
{
#if defined _MSC_VER &&  defined QM_USE_AVX
	QMSVEC h = _mm_atan2_ps(y.v, x.v);
	return *(QmVec4*)&h;
#else
	float f = atan2f(y.X, x.X);
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터 한번에 블랜드
INLINE QmVec4 QM_VECTORCALL qm_vec_simd_blend(const QmVec4 left, const QmVec4 leftScale, const QmVec4 right, const QmVec4 rightScale)
{
#if defined QM_USE_AVX
	QMSVEC h = _MM_FMADD_PS(left.v, leftScale.v, _mm_mul_ps(right.v, rightScale.v));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmlaq_f32(vmulq_f32(right.v, rightScale.v), left.v, leftScale.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.X * leftScale.X + right.X * rightScale.X, left.Y * leftScale.Y + right.Y * rightScale.Y, left.Z * leftScale.Z + right.Z * rightScale.Z, left.W * leftScale.W + right.W * rightScale.W } };
#endif
}

/// @brief 벡터의 X
INLINE float QM_VECTORCALL qm_vec_get_x(const QmVec4 v)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(v.v);
#elif defined QM_USE_NEON
	return vgetq_lane_f32(v.v, 0);
#else
	return v.X;
#endif
}

/// @brief 벡터의 Y
INLINE float QM_VECTORCALL qm_vec_get_y(const QmVec4 v)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(1, 1, 1, 1)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(v.v, 1);
#else
	return v.Y;
#endif
}

/// @brief 벡터의 Z
INLINE float QM_VECTORCALL qm_vec_get_z(const QmVec4 v)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(2, 2, 2, 2)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(v.v, 2);
#else
	return v.Z;
#endif
}

/// @brief 벡터의 W
INLINE float QM_VECTORCALL qm_vec_get_w(const QmVec4 v)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(3, 3, 3, 3)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(v.v, 3);
#else
	return v.W;
#endif
}

/// @brief 벡터 비교
INLINE bool QM_VECTORCALL qm_vec_eq(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_cmpeq_ps(left.v, right.v);
	return (_mm_movemask_ps(h) == 0x0F) != 0;
#elif defined QM_USE_NEON
	QMSVEC h = vceqq_f32(left.v, right.v);
	return vgetq_lane_u32(vreinterpretq_u32_f32(h), 0) == 0xFFFFFFFFU;
#else
	return left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W;
#endif
}

/// @brief 벡터 비교 (입실론)
INLINE bool QM_VECTORCALL qm_vec_eps(const QmVec4 left, const QmVec4 right, float epsilon)
{
#if defined QM_USE_AVX
	QMSVEC eps = _mm_set1_ps(epsilon);
	QMSVEC v = _mm_sub_ps(left.v, right.v);
	QMSVEC h = _mm_setzero_ps();
	h = _mm_sub_ps(h, v);
	h = _mm_max_ps(h, v);
	h = _mm_cmple_ps(h, eps);
	return (_mm_movemask_ps(h) == 0xF) != 0;
#else
	// TODO: 네온 분리 해야함
	return
		qm_eqs(left.X, right.X, epsilon) && qm_eqs(left.Y, right.Y, epsilon) &&
		qm_eqs(left.Z, right.Z, epsilon) && qm_eqs(left.W, right.W, epsilon);
#endif
}

//
INLINE QmVec4 QM_VECTORCALL qm_vec_select(const QmVec4 left, const QmVec4 right, const QmVec4 control)
{
#if defined QM_USE_AVX
	QMSVEC t1 = _mm_andnot_ps(control.v, left.v);
	QMSVEC t2 = _mm_and_ps(right.v, control.v);
	QMSVEC h = _mm_or_ps(t1, t2);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vbslq_f32(vreinterpretq_u32_f32(control.v), right.v, left.v);
	return *(QmVec4*)&h;
#else
	QmVecU v;
	v.u[0] = (left.v.u[0] & ~control.v.u[0]) | (right.v.u[0] & control.v.u[0]);
	v.u[1] = (left.v.u[1] & ~control.v.u[1]) | (right.v.u[1] & control.v.u[1]);
	v.u[2] = (left.v.u[2] & ~control.v.u[2]) | (right.v.u[2] & control.v.u[2]);
	v.u[3] = (left.v.u[3] & ~control.v.u[3]) | (right.v.u[3] & control.v.u[3]);
	return *(QmVec4*)&v;
#endif
}

//
INLINE QmVec4 QM_VECTORCALL qm_vec_select_ctrl(uint i0, uint i1, uint i2, uint i3)
{
#if defined QM_USE_AVX
	__m128i l = _mm_set_epi32((int)i3, (int)i2, (int)i1, (int)i0);
	__m128i r = _mm_castps_si128(QMCONST_ZERO.v);
	QMSVEC h = _mm_castsi128_ps(_mm_cmpgt_epi32(l, r));
	return *(QmVec4*)&h;
#else
	static const uint e[2] = { 0x00000000, 0xFFFFFFFF };
	QmVecU v = { { e[i0], e[i1], e[i2], e[i3] } };
	return *(QmVec4*)&v;
#endif
}

//
INLINE QmVec4 QM_VECTORCALL qm_vec_swiz(const QmVec4 a, uint e0, uint e1, uint e2, uint e3)
{
#if defined QM_USE_AVX
	uint e[4] = { e0, e1, e2, e3 };
	__m128i o = _mm_loadu_si128((const __m128i*)e);
	QMSVEC h = _mm_permutevar_ps(a.v, o);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { a.f[e0], a.f[e1], a.f[e2], a.f[e3] } };
#endif
}

//
INLINE QmVec4 QM_VECTORCALL qm_vec_pmt(const QmVec4 a, const QmVec4 b, uint px, uint py, uint pz, uint pw)
{
#if defined QM_USE_AVX
	static const QmVecU u3 = { { 3, 3, 3, 3 } };
	ALIGNOF(16) uint e[4] = { px, py, pz, pw };
	__m128i o = _mm_loadu_si128((const __m128i*)e);
	__m128i s = _mm_cmpgt_epi32(o, _mm_castps_si128(u3.v));
	o = _mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(o), u3.v));
	__m128 u = _mm_permutevar_ps(a.v, o);
	__m128 v = _mm_permutevar_ps(b.v, o);
	u = _mm_andnot_ps(_mm_castsi128_ps(s), u);
	v = _mm_and_ps(_mm_castsi128_ps(s), v);
	QMSVEC h = _mm_or_ps(u, v);
	return *(QmVec4*)&h;
#else
	const uint* ab[2] = { (const uint*)&a.v, (const uint*)&b.v };
	QmVecU v = { { ab[px >> 2][px & 3], ab[py >> 2][py & 3], ab[pz >> 2][pz & 3], ab[pw >> 2][pw & 3] } };
	return *(QmVec4*)&v;
#endif
}

//
INLINE QmVec4 QM_VECTORCALL qm_vec_bit_shl(const QmVec4 a, const QmVec4 b, uint e)
{
	return qm_vec_pmt(a, b, e + 0, e + 1, e + 2, e + 3);
}

//
INLINE QmVec4 QM_VECTORCALL qm_vec_bit_rol(const QmVec4 a, uint e)
{
	return qm_vec_swiz(a, e & 3, (e + 1) & 3, (e + 2) & 3, (e + 3) & 3);
}

//
INLINE QmVec4 QM_VECTORCALL qm_vec_bit_ror(const QmVec4 a, uint e)
{
	return qm_vec_swiz(a, (4 - e) & 3, (5 - e) & 3, (6 - e) & 3, (7 - e) & 3);
}

//
INLINE QmVec4 QM_VECTORCALL qm_vec_bit_xor(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	__m128i v = _mm_xor_si128(_mm_castps_si128(left.v), _mm_castps_si128(right.v));
	QMSVEC h = _mm_castsi128_ps(v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(left.v), vreinterpretq_u32_f32(right.v)));
	return *(QmVec4*)&h;
#else
	QmVecU v;
	v.u[0] = left.v.u[0] ^ right.v.u[0];
	v.u[1] = left.v.u[1] ^ right.v.u[1];
	v.u[2] = left.v.u[2] ^ right.v.u[2];
	v.u[3] = left.v.u[3] ^ right.v.u[3];
	return *(QmVec4*)&v;
#endif
}

#if defined QM_USE_AVX
// 벡터 연산자 만들기
#define QM_VEC_OPERATOR(name,avx,neon,op) \
INLINE QmVec4 QM_VECTORCALL qm_vec_op_##name(const QmVec4 left, const QmVec4 right) \
	{ QMSVEC v = avx(left.v, right.v); return *(QmVec4*)&v; }
#define QM_VEC_OPERATOR2(name,avx,neon1,neon2,op) QM_VEC_OPERATOR(name,avx,,)
#elif defined QM_USE_NEON
#define QM_VEC_OPERATOR(name,avx,neon,op) \
INLINE QmVec4 QM_VECTORCALL qm_vec_op_##name(const QmVec4 left, const QmVec4 right) \
	{ QMSVEC v = vreinterpretq_f32_u32(neon(left.v, right.v)); return *(QmVec4*)&v; }
#define QM_VEC_OPERATOR2(name,avx,neon1,neon2,op)  \
INLINE QmVec4 QM_VECTORCALL qm_vec_op_##name(const QmVec4 left, const QmVec4 right) \
	{ QMSVEC v = vreinterpretq_f32_u32(neon1(neon2(left.v, right.v))); return *(QmVec4*)&v; }
#else
#define QM_VEC_OPERATOR(name,avx,neon,op) \
INLINE QmVec4 QM_VECTORCALL qm_vec_op_##name(const QmVec4 left, const QmVec4 right) \
	{ QmVecU v = { { left.f[0] op right.f[0] ? 0xFFFFFFFFU : 0, left.f[1] op right.f[1] ? 0xFFFFFFFFU : 0, left.f[2] op right.f[2] ? 0xFFFFFFFFU : 0, left.f[3] op right.f[3] ? 0xFFFFFFFFU : 0 } }; return v.v4; }
#define QM_VEC_OPERATOR2(name,avx,neon1,neon2,op) QM_VEC_OPERATOR(name,,,op)
#endif

QM_VEC_OPERATOR(eq, _mm_cmpeq_ps, vceqq_f32, == );
QM_VEC_OPERATOR2(neq, _mm_cmpneq_ps, vmvnq_u32, vceqq_f32, != );
QM_VEC_OPERATOR(lt, _mm_cmplt_ps, vcltq_f32, < );
QM_VEC_OPERATOR(leq, _mm_cmple_ps, vcleq_f32, <= );
QM_VEC_OPERATOR(gt, _mm_cmpgt_ps, vcgtq_f32, > );
QM_VEC_OPERATOR(geq, _mm_cmpge_ps, vcgeq_f32, >= );

//
INLINE QmVec4 QM_VECTORCALL qm_vec_ins(const QmVec4 a, const QmVec4 b, uint e, uint s0, uint s1, uint s2, uint s3)
{
	QmVec4 o = qm_vec_select_ctrl(s0 & 1, s1 & 1, s2 & 1, s3 & 1);
	return qm_vec_select(a, qm_vec_bit_rol(b, e), o);
}


//////////////////////////////////////////////////////////////////////////
// vector2

/// @brief 벡터2 값 설정
/// @param x,y 좌표
INLINE QmVec2 qm_vec2(float x, float y)
{
	return (QmVec2) { { x, y  } };
}

/// @brief 정수 좌표 설정
/// @param p 좌표
INLINE QmVec2 qm_vec2p(const QmPoint p)
{
	return (QmVec2) { { (float)p.X, (float)p.Y  } };
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
	return (QmVec2) { { diag, diag  } };
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
	float dot = qm_vec2_dot(incident, normal);
	float k = 1.0f - eta * eta * (1.0f - dot * dot);
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
INLINE QmVec4 QM_VECTORCALL qm_vec3(float x, float y, float z)
{
	return qm_vec(x, y, z, 0.0f);
}

/// @brief 벡터3 외적
INLINE QmVec4 QM_VECTORCALL qm_vec3_cross(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC t1 = _MM_PERMUTE_PS(left.v, _MM_SHUFFLE(3, 0, 2, 1));
	QMSVEC t2 = _MM_PERMUTE_PS(right.v, _MM_SHUFFLE(3, 1, 0, 2));
	QMSVEC h = _mm_mul_ps(t1, t2);
	t1 = _MM_PERMUTE_PS(t1, _MM_SHUFFLE(3, 0, 2, 1));
	t2 = _MM_PERMUTE_PS(t2, _MM_SHUFFLE(3, 1, 0, 2));
	h = _MM_FNMADD_PS(t1, t2, h);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	static const QmVecU mask_v3 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } };
	static const QmVecU flip_y = { { 0, 0x80000000, 0, 0 } };
	QMSVEC v1xy = vget_low_f32(left.v);
	QMSVEC v2xy = vget_low_f32(right.v);
	QMSVEC v1yx = vrev64q_f32(v1xy);
	QMSVEC v2yx = vrev64q_f32(v2xy);
	QMSVEC v1zz = vdupq_lane_f32(vget_high_f32(left.v), 0);
	QMSVEC v2zz = vdupq_lane_f32(vget_high_f32(right.v), 0);
	QMSVEC h = vmulq_f32(vcombine_f32(v1yx, v1xy), vcombine_f32(v2zz, v2yx));
	h = vmlsq_f32(h, vcombine_f32(v1zz, v1yx), vcombine_f32(v2yx, v2xy));
	h = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(h), flip_y.v));
	h = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(h), mask_v3.v));
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { left.Y * right.Z - left.Z * right.Y, left.Z * right.X - left.X * right.Z, left.X * right.Y - left.Y * right.X, 0.0f } };
#endif
}

/// @brief 벡터3 법선
INLINE QmVec4 QM_VECTORCALL qm_vec3_norm(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC d = _mm_dp_ps(v.v, v.v, 0x7F);
	QMSVEC h = _mm_rsqrt_ps(d);
	h = _mm_mul_ps(v.v, h);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmulq_f32(v.v, v.v);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vpadd_f32(v1, v1);
	v2 = vdups_lane_f32(v2, 0);
	v1 = vadd_f32(v1, v2);
	v2 = vrsqrte_f32(v1);
	h = vmulq_f32(v.v, vcombine_f32(v2, v2));
	return *(QmVec4*)&h;
#else
	float l = qm_vec3_len(v);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec_mag(v, l);
#endif
}

/// @brief 벡터3 반사
INLINE QmVec4 QM_VECTORCALL qm_vec3_reflect(const QmVec4 incident, const QmVec4 normal)
{
	QmVec4 h = qm_vec3_simd_dot(incident, normal);
	h = qm_vec_add(h, h);
	return qm_vec_msub(normal, h, incident);
}

/// @brief 벡터3 굴절
INLINE QmVec4 QM_VECTORCALL qm_vec3_refract(const QmVec4 incident, const QmVec4 normal, float eta)
{
	float dot = qm_vec3_dot(incident, normal);
	float k = 1.0f - eta * eta * (1.0f - dot * dot);
	if (k < 0.0f + QM_EPSILON)
		return qm_vec_zero();
	return qm_vec_msub(qm_vec_mag(incident, eta), qm_vec_mag(normal, eta * dot + qm_sqrtf(k)), incident);
}

/// @brief 두 벡터의 방향 (결국 법선)
INLINE QmVec4 QM_VECTORCALL qm_vec3_dir(const QmVec4 left, const QmVec4 right)
{
	//return qm_vec3_norm(qm_vec3_cross(left, right));
	return qm_vec3_norm(qm_vec_sub(left, right));
}

/// @brief 벡터3을 사원수로 회전
INLINE QmVec4 QM_VECTORCALL qm_vec3_rot(const QmVec4 v, const QmVec4 rotQuat)
{
	static const QmVecU s1110 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 } };
	QmVec4 s = qm_vec_select(s1110.v4, v, s1110.v4);
	QmVec4 q = qm_quat_cjg(rotQuat);
	QmVec4 h = qm_quat_mul(q, s);
	return qm_quat_mul(h, rotQuat);
}

/// @brief 벡터3을 사원수로 반대로 회전
INLINE QmVec4 QM_VECTORCALL qm_vec3_rot_inv(const QmVec4 v, const QmVec4 rotQuat)
{
	static const QmVecU s1110 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 } };
	QmVec4 s = qm_vec_select(s1110.v4, v, s1110.v4);
	QmVec4 h = qm_quat_mul(rotQuat, s);
	QmVec4 q = qm_quat_cjg(rotQuat);
	return qm_quat_mul(h, q);
}

/// @brief 벡터3을 행렬로 변환시킴
INLINE QmVec4 QM_VECTORCALL qm_vec3_trfm(const QmVec4 v, const QmMat4 m)
{
#if defined QM_USE_AVX
	QMSVEC x = _mm_permute_ps(v.v, _MM_SHUFFLE(0, 0, 0, 0));
	QMSVEC y = _mm_permute_ps(v.v, _MM_SHUFFLE(1, 1, 1, 1));
	QMSVEC z = _mm_permute_ps(v.v, _MM_SHUFFLE(2, 2, 2, 2));
	QMSVEC r = _MM_FMADD_PS(z, m.v[2], m.v[3]);
	r = _MM_FMADD_PS(y, m.v[1], r);
	r = _MM_FMADD_PS(x, m.v[0], r);
	return *(QmVec4*)&r;
#else
	QmVec4 x = qm_vec_sp_x(v);
	QmVec4 y = qm_vec_sp_y(v);
	QmVec4 z = qm_vec_sp_z(v);
	QmVec4 h = qm_vec_madd(z, m.r[2], m.r[3]);
	h = qm_vec_madd(y, m.r[1], h);
	h = qm_vec_madd(x, m.r[0], h);
	return h;
#endif
}

/// @brief 벡터3을 행렬로 정규화 변환시킴
INLINE QmVec4 QM_VECTORCALL qm_vec3_trfm_norm(const QmVec4 v, const QmMat4 m)
{
#if defined QM_USE_AVX
	QMSVEC x = _mm_permute_ps(v.v, _MM_SHUFFLE(0, 0, 0, 0));
	QMSVEC y = _mm_permute_ps(v.v, _MM_SHUFFLE(1, 1, 1, 1));
	QMSVEC z = _mm_permute_ps(v.v, _MM_SHUFFLE(2, 2, 2, 2));
	QMSVEC r = _mm_mul_ps(z, m.v[2]);
	r = _MM_FMADD_PS(y, m.v[1], r);
	r = _MM_FMADD_PS(x, m.v[0], r);
	return *(QmVec4*)&r;
#else
	QmVec4 x = qm_vec_sp_x(v);
	QmVec4 y = qm_vec_sp_y(v);
	QmVec4 z = qm_vec_sp_z(v);
	QmVec4 h = qm_vec_mul(z, m.r[2]);
	h = qm_vec_madd(y, m.r[1], h);
	h = qm_vec_madd(x, m.r[0], h);
	return h;
#endif
}

/// @brief 벡터3을 행렬로 변환시킴
INLINE QmVec4 QM_VECTORCALL qm_vec3_trfm_coord(const QmVec4 v, const QmMat4 m)
{
#if defined QM_USE_AVX
	QMSVEC x = _mm_permute_ps(v.v, _MM_SHUFFLE(0, 0, 0, 0));
	QMSVEC y = _mm_permute_ps(v.v, _MM_SHUFFLE(1, 1, 1, 1));
	QMSVEC z = _mm_permute_ps(v.v, _MM_SHUFFLE(2, 2, 2, 2));
	QMSVEC r = _MM_FMADD_PS(z, m.v[2], m.v[3]);
	r = _MM_FMADD_PS(y, m.v[1], r);
	r = _MM_FMADD_PS(x, m.v[0], r);
	QMSVEC w = _mm_permute_ps(r, _MM_SHUFFLE(2, 2, 2, 2));
	QMSVEC h = _mm_div_ps(r, w);
	return *(QmVec4*)&h;
#else
	QmVec4 x = qm_vec_sp_x(v);
	QmVec4 y = qm_vec_sp_y(v);
	QmVec4 z = qm_vec_sp_z(v);
	QmVec4 r = qm_vec_madd(z, m.r[2], m.r[3]);
	r = qm_vec_madd(y, m.r[1], r);
	r = qm_vec_madd(x, m.r[0], r);
	QmVec4 w = qm_vec_sp_w(r);
	return qm_vec_div(r, w);
#endif
}

/// @brief 세 벡터로 법선 벡터를 만든다
INLINE QmVec4 QM_VECTORCALL qm_vec3_form_norm(const QmVec4 v1, const QmVec4 v2, const QmVec4 v3)
{
	QmVec4 c = qm_vec3_cross(qm_vec_sub(v2, v1), qm_vec_sub(v3, v1));
	return qm_vec_mag(c, qm_vec3_len(c));
}

/// @brief 선에 가까운 점의 위치
INLINE QmVec4 QM_VECTORCALL qm_vec3_closed(const QmVec4 loc, const QmVec4 begin, const QmVec4 end)
{
	QmVec4 norm = qm_vec_sub(end, begin);
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
INLINE QmVec4 QM_VECTORCALL qm_vec3_proj(const QmVec4 v,
	const QmMat4 proj, const QmMat4 view, const QmMat4 world,
	float x, float y, float width, float height, float zn, float zf)
{
	// UNDONE: 끄덕
	QN_DUMMY(v);
	QN_DUMMY(proj); QN_DUMMY(view); QN_DUMMY(world);
	QN_DUMMY(x); QN_DUMMY(y); QN_DUMMY(width); QN_DUMMY(height); QN_DUMMY(zn); QN_DUMMY(zf);
	return qm_vec_zero();
}

/// @brief 언프로젝션
INLINE QmVec4 QM_VECTORCALL qm_vec3_unproj(const QmVec4 v,
	const QmMat4 proj, const QmMat4 view, const QmMat4 world,
	float x, float y, float width, float height, float zn, float zf)
{
	// UNDONE: 끄덕
	QN_DUMMY(v);
	QN_DUMMY(proj); QN_DUMMY(view); QN_DUMMY(world);
	QN_DUMMY(x); QN_DUMMY(y); QN_DUMMY(width); QN_DUMMY(height); QN_DUMMY(zn); QN_DUMMY(zf);
	return qm_vec_zero();
}

/// @brief 벡터3 내적 벡터형
INLINE QmVec4 QM_VECTORCALL qm_vec3_simd_dot(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_dp_ps(left.v, right.v, 0x71);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmulq_f32(left.v, right.v);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vpadd_f32(v1, v1);
	v2 = vdups_lane_f32(v2, 0);
	v1 = vadd_f32(v1, v2);
	h = vcombine_f32(v1, v1);
	return *(QmVec4*)&h;
#else
	float f = left.X * right.X + left.Y * right.Y + left.Z * right.Z;
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터3 내적
INLINE float QM_VECTORCALL qm_vec3_dot(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_dp_ps(left.v, right.v, 0x71);
	return _mm_cvtss_f32(h);
#elif defined QM_USE_NEON
	QMSVEC h = vmulq_f32(left.v, right.v);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vpadd_f32(v1, v1);
	v2 = vdups_lane_f32(v2, 0);
	v1 = vadd_f32(v1, v2);
	h = vcombine_f32(v1, v1);
	return vgetq_lane_f32(h, 0);
#else
	return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
#endif
}

/// @brief 벡터3 길이의 제곱
INLINE float QM_VECTORCALL qm_vec3_len_sq(const QmVec4 v)
{
	return qm_vec3_dot(v, v);
}

/// @brief 벡터3 길이
INLINE float QM_VECTORCALL qm_vec3_len(const QmVec4 v)
{
#ifdef QM_USE_SIMD
	QmVec4 h = qm_vec_simd_sqrt(qm_vec3_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec3_len_sq(v));
#endif
}

/// @brief 벡터3 거리의 제곱
INLINE float QM_VECTORCALL qm_vec3_dist_sq(const QmVec4 left, const QmVec4 right)
{
	return qm_vec3_len_sq(qm_vec_sub(left, right));
}

/// @brief 벡터3 거리
INLINE float QM_VECTORCALL qm_vec3_dist(const QmVec4 left, const QmVec4 right)
{
#ifdef QM_USE_SIMD
	QmVec4 v = qm_vec_sub(left, right);
	QmVec4 h = qm_vec_simd_sqrt(qm_vec3_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec3_dist_sq(left, right));
#endif
}

/// @brief 두 벡터3의 반지름의 제곱
INLINE float QM_VECTORCALL qm_vec3_rad_sq(const QmVec4 left, const QmVec4 right)
{
	return qm_vec3_len_sq(qm_vec_sub(qm_vec_mag(qm_vec_add(left, right), 0.5f), left));
}

/// @brief 두 벡터3의 반지름
INLINE float QM_VECTORCALL qm_vec3_rad(const QmVec4 left, const QmVec4 right)
{
#ifdef QM_USE_SIMD
	QmVec4 v = qm_vec_sub(qm_vec_mag(qm_vec_add(left, right), 0.5f), left);
	QmVec4 h = qm_vec_simd_sqrt(qm_vec3_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec3_rad_sq(left, right));
#endif
}

/// @brief 두 벡터3 법선의 사이각
INLINE float QM_VECTORCALL qm_vec3_angle(const QmVec4 leftNormal, const QmVec4 rightNormal)
{
	float dot = qm_vec3_dot(leftNormal, rightNormal);
	return acosf(qm_clampf(dot, -1.0f, 1.0f));
}

/// @brief 두 선분(벡터3) 사이에 벡터3이 있는지 조사
INLINE bool QM_VECTORCALL qm_vec3_is_between(const QmVec4 queryPoint, const QmVec4 begin, const QmVec4 end)
{
	const float f = qm_vec3_len_sq(qm_vec_sub(end, begin));
	return qm_vec3_dist_sq(queryPoint, begin) <= f && qm_vec3_dist_sq(queryPoint, end) <= f;
}

/// @brief 벡터3 비교
INLINE bool QM_VECTORCALL qm_vec3_eq(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_cmpeq_ps(left.v, right.v);
	return ((_mm_movemask_ps(h) & 7) == 7) != 0;
#elif defined QM_USE_NEON
	uint32x4_t v = vceqq_f32(left.v, right.v);
	uint8x8x2_t p = vzip_u8(vget_low_u8(vreinterpretq_u8_u32(v)), vget_high_u8(vreinterpretq_u8_u32(v)));
	uint16x4x2_t p2 = vzip_u16(vreinterpret_u16_u8(p.val[0]), vreinterpret_u16_u8(p.val[1]));
	return ((vget_lane_u32(vreinterpret_u32_u16(p2.val[1]), 1) & 0xFFFFFFU) == 0xFFFFFFU);
#else
	return left.X == right.X && left.Y == right.Y && left.Z == right.Z;
#endif
}

/// @brief 벡터3 비교 (입실론)
INLINE bool QM_VECTORCALL qm_vec3_eps(const QmVec4 left, const QmVec4 right, float epsilon)
{
#if defined QM_USE_AVX
	QMSVEC eps = _mm_set1_ps(epsilon);
	QMSVEC v = _mm_sub_ps(left.v, right.v);
	QMSVEC h = _mm_setzero_ps();
	h = _mm_sub_ps(h, v);
	h = _mm_max_ps(h, v);
	h = _mm_cmple_ps(h, eps);
	return ((_mm_movemask_ps(h) & 7) == 7) != 0;
#else
	// TODO: 네온 분리 해야함
	return
		qm_eqs(left.X, right.X, epsilon) &&
		qm_eqs(left.Y, right.Y, epsilon) &&
		qm_eqs(left.Z, right.Z, epsilon);
#endif
}


//////////////////////////////////////////////////////////////////////////
// vector4

/// @brief 벡터4 값 설정
INLINE QmVec4 QM_VECTORCALL qm_vec4(float x, float y, float z, float w)
{
	return qm_vec(x, y, z, w);
}

/// @brief 벡터4 값 설정 (벡터3 + w)
INLINE QmVec4 QM_VECTORCALL qm_vec4v(const QmVec4 v, float w)
{
	return qm_vec(v.X, v.Y, v.Z, w);
}

/// @brief 벡터4 외적
INLINE QmVec4 QM_VECTORCALL qm_vec4_cross(const QmVec4 a, const QmVec4 b, const QmVec4 c)
{
#if defined QM_USE_AVX
	QMSVEC u = _mm_permute_ps(b.v, _MM_SHUFFLE(2, 1, 3, 2));
	QMSVEC p = _mm_permute_ps(c.v, _MM_SHUFFLE(1, 3, 2, 3));
	u = _mm_mul_ps(u, p);
	QMSVEC o = _mm_permute_ps(b.v, _MM_SHUFFLE(1, 3, 2, 3));
	p = _mm_permute_ps(p, _MM_SHUFFLE(1, 3, 0, 1));
	u = _MM_FNMADD_PS(o, p, u);
	QMSVEC n = _mm_permute_ps(a.v, _MM_SHUFFLE(0, 0, 0, 1));
	u = _mm_mul_ps(u, n);
	o = _mm_permute_ps(b.v, _MM_SHUFFLE(2, 0, 3, 1));
	p = _mm_permute_ps(c.v, _MM_SHUFFLE(0, 3, 0, 3));
	p = _mm_mul_ps(p, o);
	o = _mm_permute_ps(o, _MM_SHUFFLE(2, 1, 2, 1));
	n = _mm_permute_ps(c.v, _MM_SHUFFLE(2, 0, 3, 1));
	p = _MM_FNMADD_PS(o, n, p);
	n = _mm_permute_ps(a.v, _MM_SHUFFLE(1, 1, 2, 2));
	u = _MM_FNMADD_PS(n, p, u);
	o = _mm_permute_ps(b.v, _MM_SHUFFLE(1, 0, 2, 1));
	p = _mm_permute_ps(c.v, _MM_SHUFFLE(0, 1, 0, 2));
	p = _mm_mul_ps(p, o);
	o = _mm_permute_ps(o, _MM_SHUFFLE(2, 0, 2, 1));
	n = _mm_permute_ps(c.v, _MM_SHUFFLE(1, 0, 2, 1));
	p = _MM_FNMADD_PS(n, o, p);
	n = _mm_permute_ps(a.v, _MM_SHUFFLE(2, 3, 3, 3));
	QMSVEC h = _MM_FMADD_PS(p, n, u);
	return *(QmVec4*)&h;
#else
	QmVec4 v;
	v.f[0] = a.f[1] * (b.f[2] * c.f[3] - c.f[2] * b.f[3]) - a.f[2] * (b.f[1] * c.f[3] - c.f[1] * b.f[3]) + a.f[3] * (b.f[1] * c.f[2] - b.f[2] * c.f[1]);
	v.f[1] = -(a.f[0] * (b.f[2] * c.f[3] - c.f[2] * b.f[3]) - a.f[2] * (b.f[0] * c.f[3] - c.f[0] * b.f[3]) + a.f[3] * (b.f[0] * c.f[2] - c.f[0] * b.f[2]));
	v.f[2] = a.f[0] * (b.f[1] * c.f[3] - c.f[1] * b.f[3]) - a.f[1] * (b.f[0] * c.f[3] - c.f[0] * b.f[3]) + a.f[3] * (b.f[0] * c.f[1] - c.f[0] * b.f[1]);
	v.f[3] = -(a.f[0] * (b.f[1] * c.f[2] - c.f[1] * b.f[2]) - a.f[1] * (b.f[0] * c.f[2] - c.f[0] * b.f[2]) + a.f[2] * (b.f[0] * c.f[1] - c.f[0] * b.f[1]));
	return v;
#endif
}

/// @brief 벡터4 법선
INLINE QmVec4 QM_VECTORCALL qm_vec4_norm(const QmVec4 v)
{
#if defined QM_USE_AVX
	QMSVEC d = _mm_dp_ps(v.v, v.v, 0xFF);
	QMSVEC h = _mm_rsqrt_ps(d);
	h = _mm_mul_ps(v.v, h);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmulq_f32(v.v, v.v);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vadd_f32(v1, v2);
	v1 = vpadd_f32(v1, v1);
	v2 = vrsqrte_f32(v1);
	h = vmulq_f32(v.v, vcombine_f32(v2, v2));
	return *(QmVec4*)&h;
#else
	float l = qm_vec4_len(v);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec_mag(v, l);
#endif
}

/// @brief 벡터4 반사
INLINE QmVec4 QM_VECTORCALL qm_vec4_reflect(const QmVec4 incident, const QmVec4 normal)
{
	QmVec4 h = qm_vec4_simd_dot(incident, normal);
	h = qm_vec_add(h, h);
	return qm_vec_msub(normal, h, incident);
}

/// @brief 벡터4 굴절
INLINE QmVec4 QM_VECTORCALL qm_vec4_refract(const QmVec4 incident, const QmVec4 normal, float eta)
{
	float dot = qm_vec4_dot(incident, normal);
	float k = 1.0f - eta * eta * (1.0f - dot * dot);
	if (k < 0.0f + QM_EPSILON)
		return qm_vec_zero();
	return qm_vec_msub(qm_vec_mag(incident, eta), qm_vec_mag(normal, eta * dot + qm_sqrtf(k)), incident);
}

/// @brief 벡터4을 행렬로 변환시킴
INLINE QmVec4 QM_VECTORCALL qm_vec4_trfm(const QmVec4 v, const QmMat4 m)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_mul_ps(_MM_PERMUTE_PS(v.v, 0x00), m.v[0]);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(v.v, 0x55), m.v[1], h);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(v.v, 0xAA), m.v[2], h);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(v.v, 0xFF), m.v[3], h);
	return *(QmVec4*)&h;
#else
	float x = v.X * m._11 + v.Y * m._21 + v.Z * m._31 + v.W * m._41;
	float y = v.X * m._12 + v.Y * m._22 + v.Z * m._32 + v.W * m._42;
	float z = v.X * m._13 + v.Y * m._23 + v.Z * m._33 + v.W * m._43;
	float w = v.X * m._14 + v.Y * m._24 + v.Z * m._34 + v.W * m._44;
	return qm_vec(x, y, z, w);
#endif
}

/// @brief 벡터4 내적 벡터형
INLINE QmVec4 QM_VECTORCALL qm_vec4_simd_dot(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_dp_ps(left.v, right.v, 0xFF);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC h = vmulq_f32(left.v, right.v);
	h = vpaddq_f32(h, h);
	h = vpaddq_f32(h, h);
	return *(QmVec4*)&h;
#else
	float f = left.X * right.X + left.Y * right.Y + left.Z * right.Z + left.W * right.W;
	return (QmVec4) { { f, f, f, f } };
#endif
}

/// @brief 벡터4 내적
INLINE float QM_VECTORCALL qm_vec4_dot(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	QMSVEC h = _mm_dp_ps(left.v, right.v, 0xFF);
	return _mm_cvtss_f32(h);
#elif defined QM_USE_NEON
	QMSVEC h = vmulq_f32(left.v, right.v);
	h = vpaddq_f32(h, h);
	h = vpaddq_f32(h, h);
	return vgetq_lane_f32(h, 0);
#else
	return left.X * right.X + left.Y * right.Y + left.Z * right.Z + left.W * right.W;
#endif
}

/// @brief 벡터4 길이의 제곱
INLINE float QM_VECTORCALL qm_vec4_len_sq(const QmVec4 v)
{
	return qm_vec4_dot(v, v);
}

/// @brief 벡터4 길이
INLINE float QM_VECTORCALL qm_vec4_len(const QmVec4 v)
{
#ifdef QM_USE_SIMD
	QmVec4 h = qm_vec_simd_sqrt(qm_vec4_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec4_len_sq(v));
#endif
}

/// @brief 벡터4 거리의 제곱
INLINE float QM_VECTORCALL qm_vec4_dist_sq(const QmVec4 left, const QmVec4 right)
{
	return qm_vec4_len_sq(qm_vec_sub(left, right));
}

/// @brief 벡터4 거리
INLINE float QM_VECTORCALL qm_vec4_dist(const QmVec4 left, const QmVec4 right)
{
#ifdef QM_USE_SIMD
	QmVec4 v = qm_vec_sub(left, right);
	QmVec4 h = qm_vec_simd_sqrt(qm_vec4_simd_dot(v, v));
	return qm_vec_get_x(h);
#else
	return qm_sqrtf(qm_vec4_dist_sq(left, right));
#endif
}

/// @brief 벡터4 비교
INLINE bool QM_VECTORCALL qm_vec4_eq(const QmVec4 left, const QmVec4 right)
{
	return qm_vec_eq(left, right);
}

/// @brief 벡터4 비교 (입실론)
INLINE bool QM_VECTORCALL qm_vec4_eps(const QmVec4 left, const QmVec4 right, float epsilon)
{
	return qm_vec_eps(left, right, epsilon);
}


//////////////////////////////////////////////////////////////////////////
// quaternion

#define qm_quat_len_sq	qm_vec4_len_sq		/// @brief 사원수 길이의 제곱
#define qm_quat_len		qm_vec4_len			/// @brief 사원수 길이
#define qm_quat_eq		qm_vec_eq			/// @brief 사원수 비교
#define qm_quat_eps		qm_vec_eps			/// @brief 사원수 비교 (입실론)

/// @brief 사원수 값 설정
INLINE QmVec4 QM_VECTORCALL qm_quat(float x, float y, float z, float w)
{
	return qm_vec(x, y, z, w);
}

/// @brief 사원수 값 설정 (벡터3 + w)
INLINE QmVec4 QM_VECTORCALL qm_quatv(const QmVec4 v, float w)
{
	return qm_vec(v.X, v.Y, v.Z, w);
}

/// @brief 단위 사원수
INLINE QmVec4 QM_VECTORCALL qm_quat_unit(void)
{
	return QMCONST_UNIT_R3;
}

/// @brief 사원수 곱셈
INLINE QmVec4 QM_VECTORCALL qm_quat_mul(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_AVX
	// TDDO: 값이 맞는지 검증해야함
	QMSVEC h = _mm_mul_ps(_MM_PERMUTE_PS(left.v, 0x00), right.v);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(left.v, 0x55), _MM_PERMUTE_PS(right.v, 0xEA), h);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(left.v, 0xAA), _MM_PERMUTE_PS(right.v, 0xD4), h);
	h = _MM_FMADD_PS(_MM_PERMUTE_PS(left.v, 0xFF), _MM_PERMUTE_PS(right.v, 0x1B), h);
	return *(QmVec4*)&h;
#else
	float x = left.W * right.X + left.X * right.W + left.Y * right.Z - left.Z * right.Y;
	float y = left.W * right.Y - left.X * right.Z + left.Y * right.W + left.Z * right.X;
	float z = left.W * right.Z + left.X * right.Y - left.Y * right.X + left.Z * right.W;
	float w = left.W * right.W - left.X * right.X - left.Y * right.Y - left.Z * right.Z;
	return (QmVec4) { { x, y, z, w } };
#endif
}

/// @brief 사원수 법선
INLINE QmVec4 QM_VECTORCALL qm_quat_norm(const QmVec4 q)
{
	return qm_vec4_norm(q);
}

/// @brief 켤레 사원수
INLINE QmVec4 QM_VECTORCALL qm_quat_cjg(const QmVec4 q)
{
#if defined QM_USE_AVX
	static const QmVec4 conjugate_mask = { { -1.0f, -1.0f, -1.0f, 1.0f } };
	QMSVEC h = _mm_mul_ps(q.v, conjugate_mask.v);
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	static const QmVec4 conjugate_mask = { { -1.0f, -1.0f, -1.0f, 1.0f } };
	QMSVEC h = vmulq_f32(q.v, conjugate_mask.v);
	return *(QmVec4*)&h;
#else
	return (QmVec4) { { -q.f[0], -q.f[1], -q.f[2], q.f[3] } };
#endif
}

/// @brief 역사원수
INLINE QmVec4 QM_VECTORCALL qm_quat_inv(const QmVec4 q)
{
	static const QmVec4 epsilon = { { FLT_EPSILON,FLT_EPSILON,FLT_EPSILON,FLT_EPSILON } };
	QmVec4 l = qm_vec4_simd_dot(q, q);
	QmVec4 c = qm_quat_cjg(q);
	QmVec4 d = qm_vec_div(c, l);
	return qm_vec_select(d, QMCONST_ZERO, qm_vec_op_leq(l, epsilon));
}

/// @brief 지수 사원수
INLINE QmVec4 QM_VECTORCALL qm_quat_exp(const QmVec4 q)
{
	float theta = qm_vec3_len(q);
	if (qm_eqf(theta, 0.0f))
		return qm_quat_unit();
	float s, c;
	qm_sincosf(theta, &s, &c);
	theta = s * (1.0f / theta);
	return qm_vec(q.X * theta, q.Y * theta, q.Z * theta, c);
}

/// @brief 로그 사원수
INLINE QmVec4 QM_VECTORCALL qm_quat_ln(const QmVec4 q)
{
	float l = qm_quat_len_sq(q);
	if (l > 1.0001f)
		return q;
	if (l < 0.9999f)
		return qm_vec_zero();
	float n = qm_vec3_len(q);
	float t = atan2f(n, q.W) / n;
	return qm_vec(q.X * t, q.Y * t, q.Z * t, 0.0f);
}

/// @brief 사원수 구형 보간 (이거 뭔가 좀 미묘해)
INLINE QmVec4 QM_VECTORCALL qm_quat_slerp(const QmVec4 left, const QmVec4 right, float scale)
{
	float d = qm_quat_dot(left, right);
	QmVec4 r;
	if (d < 0.0f - QM_EPSILON)
		d = -d, r = qm_vec_neg(right);
	else
		r = right;
	if (d < 0.9995f)
		return qm_vec_lerp(left, r, scale);
	float theta = acosf(d);
	float ls = sinf(theta * (1.0f - scale));
	float rs = sinf(theta * scale);
	QmVec4 h = qm_vec_blend(left, ls, r, rs);
	return qm_quat_norm(h);
}

/// @brief DirectXMath용 사원수 구형 보간
INLINE QmVec4 QM_VECTORCALL qm_quat_slerp_dxm(const QmVec4 Q0, const QmVec4 Q1, float scale)
{
	const QmVec4 OneMinusEpsilon = { { 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f } };
	const QmVec4 NegativeOne = { { -1.0f, -1.0f, -1.0f, -1.0f } };

	QmVec4 t = qm_vec_sp(scale);
	QmVec4 CosOmega = qm_quat_simd_dot(Q0, Q1);

	const QmVec4 Zero = qm_vec_zero();
	QmVec4 Control = qm_vec_op_lt(CosOmega, Zero);
	QmVec4 Sign = qm_vec_select(QMCONST_ONE, NegativeOne, Control);

	CosOmega = qm_vec_mul(CosOmega, Sign);

	Control = qm_vec_op_lt(CosOmega, OneMinusEpsilon);

	QmVec4 SinOmega = qm_vec_msub(CosOmega, CosOmega, QMCONST_ONE);
	SinOmega = qm_vec_simd_sqrt(SinOmega);

	QmVec4 Omega = qm_vec_simd_atan2(SinOmega, CosOmega);

	QmVec4 SignMask = qm_vec_sp_msb();
	QmVec4 V01 = qm_vec_bit_shl(t, Zero, 2);
	SignMask = qm_vec_bit_shl(SignMask, Zero, 3);
	V01 = qm_vec_bit_xor(V01, SignMask);
	V01 = qm_vec_add(QMCONST_UNIT_R0, V01);

	QmVec4 InvSinOmega = qm_vec_rcp(SinOmega);

	QmVec4 S0 = qm_vec_mul(V01, Omega);
	S0 = qm_vec_simd_sin(S0);
	S0 = qm_vec_mul(S0, InvSinOmega);

	S0 = qm_vec_select(V01, S0, Control);

	QmVec4 S1 = qm_vec_sp_y(S0);
	S0 = qm_vec_sp_x(S0);

	S1 = qm_vec_mul(S1, Sign);

	QmVec4 Result = qm_vec_mul(Q0, S0);
	Result = qm_vec_madd(Q1, S1, Result);

	return Result;
}

/// @brief 사원수 구형 사중 보간
INLINE QmVec4 QM_VECTORCALL qm_quat_squad(const QmVec4 q1, const QmVec4 q2, const QmVec4 q3, const QmVec4 q4, float scale)
{
	QmVec4 s = qm_quat_slerp(q1, q2, scale);
	QmVec4 t = qm_quat_slerp(q3, q4, scale);
	return qm_quat_slerp(s, t, 2.0f * scale * (1.0f - scale));
}

// TODO: 사원수 구형 사중 보간용 사원수 만들기...라는 함수 (qm_quat_squad_setup)

/// @brief 사원수 질량 중심 좌표계 이동 계산
INLINE QmVec4 QM_VECTORCALL qm_quat_barycentric(const QmVec4 q1, const QmVec4 q2, const QmVec4 q3, float f, float g)
{
	float s = f + g;
	if (s < QM_EPSILON && s > -QM_EPSILON)
		return q1;
	QmVec4 q12 = qm_quat_slerp(q1, q2, s);
	QmVec4 q13 = qm_quat_slerp(q1, q3, s);
	return qm_quat_slerp(q12, q13, g / s);
}

/// @brief 벡터로 회전 (롤/피치/요)
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_vec(const QmVec4 rot3)
{
	float rs, rc, ps, pc, ys, yc;
	qm_sincosf(rot3.X * 0.5f, &rs, &rc);
	qm_sincosf(rot3.Y * 0.5f, &ps, &pc);
	qm_sincosf(rot3.Z * 0.5f, &ys, &yc);
	const float pcyc = pc * yc;
	const float psyc = ps * yc;
	const float pcys = pc * ys;
	const float psys = ps * ys;
	return qm_vec(rs * pcyc - rc * psys, rc * psyc + rs * pcys, rc * pcys + rs * psyc, rc * pcyc + rs * psys);
}

/// @brief 벡터로 축 회전
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_axis(const QmVec4 axis3, float angle)
{
	float s, c;
	qm_sincosf(angle * 0.5f, &s, &c);
	return qm_quatv(qm_vec_mag(qm_vec3_norm(axis3), s), c);
}

/// @brief 사원수를 X축 회전시킨다
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_x(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
	return qm_vec(s, 0.0f, 0.0f, c);
}

/// @brief 사원수를 Y축 회전시킨다
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_y(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
	return qm_vec(0.0f, s, 0.0f, c);
}

/// @brief 사원수를 Z축 회전시킨다
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_z(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
	return qm_vec(0.0f, 0.0f, s, c);
}

/// @brief 행렬로 회전
INLINE QmVec4 QM_VECTORCALL qm_quat_rot_mat4(const QmMat4 rot)
{
#if defined QM_USE_AVX
	static const QmVec4 PMMP = { { +1.0f, -1.0f, -1.0f, +1.0f } };
	static const QmVec4 MPMP = { { -1.0f, +1.0f, -1.0f, +1.0f } };
	static const QmVec4 MMPP = { { -1.0f, -1.0f, +1.0f, +1.0f } };
	QMSVEC r0 = rot.v[0];
	QMSVEC r1 = rot.v[1];
	QMSVEC r2 = rot.v[2];
	QMSVEC r00 = _MM_PERMUTE_PS(r0, _MM_SHUFFLE(0, 0, 0, 0));
	QMSVEC r11 = _MM_PERMUTE_PS(r1, _MM_SHUFFLE(1, 1, 1, 1));
	QMSVEC r22 = _MM_PERMUTE_PS(r2, _MM_SHUFFLE(2, 2, 2, 2));
	QMSVEC r11mr00 = _mm_sub_ps(r11, r00);
	QMSVEC x2gey2 = _mm_cmple_ps(r11mr00, QMCONST_ZERO.v);
	QMSVEC r11pr00 = _mm_add_ps(r11, r00);
	QMSVEC z2gew2 = _mm_cmple_ps(r11pr00, QMCONST_ZERO.v);
	QMSVEC x2py2gez2pw2 = _mm_cmple_ps(r22, QMCONST_ZERO.v);
	QMSVEC t0 = _MM_FMADD_PS(PMMP.v, r00, QMCONST_ONE.v);
	QMSVEC t1 = _mm_mul_ps(MPMP.v, r11);
	QMSVEC t2 = _MM_FMADD_PS(MMPP.v, r22, t0);
	QMSVEC x2y2z2w2 = _mm_add_ps(t1, t2);
	t0 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1, 2, 2, 1));
	t1 = _mm_shuffle_ps(r1, r2, _MM_SHUFFLE(1, 0, 0, 0));
	t1 = _MM_PERMUTE_PS(t1, _MM_SHUFFLE(1, 3, 2, 0));
	QMSVEC xyxzyz = _mm_add_ps(t0, t1);
	t0 = _mm_shuffle_ps(r2, r1, _MM_SHUFFLE(0, 0, 0, 1));
	t1 = _mm_shuffle_ps(r1, r0, _MM_SHUFFLE(1, 2, 2, 2));
	t1 = _MM_PERMUTE_PS(t1, _MM_SHUFFLE(1, 3, 2, 0));
	QMSVEC xwywzw = _mm_sub_ps(t0, t1);
	xwywzw = _mm_mul_ps(MPMP.v, xwywzw);
	t0 = _mm_shuffle_ps(x2y2z2w2, xyxzyz, _MM_SHUFFLE(0, 0, 1, 0));
	t1 = _mm_shuffle_ps(x2y2z2w2, xwywzw, _MM_SHUFFLE(0, 2, 3, 2));
	t2 = _mm_shuffle_ps(xyxzyz, xwywzw, _MM_SHUFFLE(1, 0, 2, 1));
	QMSVEC tensor0 = _mm_shuffle_ps(t0, t2, _MM_SHUFFLE(2, 0, 2, 0));
	QMSVEC tensor1 = _mm_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 1, 1, 2));
	QMSVEC tensor2 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(2, 0, 1, 0));
	QMSVEC tensor3 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(1, 2, 3, 2));
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
	QMSVEC h = _mm_div_ps(t2, t0);
	return *(QmVec4*)&h;
#else
	float diag = rot._11 + rot._22 + rot._33 + 1.0f;
	QmVec4 q;
	if (diag > 0.0f)
	{
		float s = qm_sqrtf(diag) * 2.0f;
		float rs = 1.0f / s;
		q = qm_vec((rot._23 - rot._32) * rs, (rot._31 - rot._13) * rs, (rot._12 - rot._21) * rs, 0.25f * s);
	}
	else
	{
		// 필요한 스케일 만들기
		float s = qm_sqrtf(1.0f + rot._11 - rot._22 - rot._33) * 2.0f;
		float rs = 1.0f / s;
		if (rot._11 > rot._22 && rot._11 > rot._33)
			q = qm_vec(0.25f * s, (rot._12 + rot._21) * rs, (rot._31 + rot._13) * rs, (rot._23 - rot._32) * rs);
		else if (rot._22 > rot._33)
			q = qm_vec((rot._12 + rot._21) * rs, 0.25f * s, (rot._23 + rot._32) * rs, (rot._31 - rot._13) * rs);
		else
			q = qm_vec((rot._31 + rot._13) * rs, (rot._23 + rot._32) * rs, 0.25f * s, (rot._12 - rot._21) * rs);
	}
	float d = qm_quat_len_sq(q);
	if (qm_eqf(d, 1.0f))
		return q;
	return qm_vec_mag(q, qm_rsqrtf(d));
#endif
}

/// @brief 사원수 내적 벡터형
INLINE QmVec4 QM_VECTORCALL qm_quat_simd_dot(const QmVec4 left, const QmVec4 right)
{
	return qm_vec4_simd_dot(left, right);
}

/// @brief 사원수 내적
INLINE float QM_VECTORCALL qm_quat_dot(const QmVec4 left, const QmVec4 right)
{
	return qm_vec4_dot(left, right);
}

/// @brief 사원수를 축과 각도로
INLINE void QM_VECTORCALL qm_quat_to_axis_angle(const QmVec4 q, QmVec4* axis, float* angle)
{
	*axis = q;
	*angle = acosf(qm_vec_get_w(q)) * 2.0f;
}

/// @brief 단위 사원수인가?
INLINE bool QM_VECTORCALL qm_quat_isu(const QmVec4 v)
{
	return qm_vec_eq(v, QMCONST_UNIT_R3);
}


//////////////////////////////////////////////////////////////////////////
// plane

#define qm_plane_eq		qm_vec_eq			/// @brief 평면 비교
#define qm_plane_eps	qm_vec_eps			/// @brief 평면 비교 (입실론)
#define qm_plane_isu	qm_quat_isu			/// @brief 단위 평면인가?

/// @brief 평면 값 설정
INLINE QmVec4 QM_VECTORCALL qm_plane(float a, float b, float c, float d)
{
	return qm_vec(a, b, c, d);
}

/// @brief 평면 값 설정 (벡터3 + d)
INLINE QmVec4 QM_VECTORCALL qm_planev(const QmVec4 v, float d)
{
	return qm_vec(v.X, v.Y, v.Z, d);
}

/// @brief 점과 법선으로 면 만들기 (벡터3 + 벡터3)
INLINE QmVec4 QM_VECTORCALL qm_planevv(const QmVec4 v, const QmVec4 normal)
{
	return qm_vec(v.X, v.Y, v.Z, -qm_vec3_dot(v, normal));
}

/// @brief 점 세개로 평면 만들기 (벡터3 + 벡터3 + 벡터3)
INLINE QmVec4 QM_VECTORCALL qm_planevvv(const QmVec4 v1, const QmVec4 v2, const QmVec4 v3)
{
	QmVec4 l = qm_vec_sub(v2, v1);
	QmVec4 r = qm_vec_sub(v3, v1);
	QmVec4 n = qm_vec3_norm(qm_vec3_cross(l, r));
	n.W = -qm_vec3_dot(v1, n);
	return n;
}

/// @brief 단위 평면
INLINE QmVec4 QM_VECTORCALL qm_plane_unit(void)
{
	return QMCONST_UNIT_R3;
}

/// @brief 평면 정규화
INLINE QmVec4 QM_VECTORCALL qm_plane_norm(const QmVec4 plane)
{
#if defined QM_USE_AVX
	QMSVEC d = _mm_dp_ps(plane.v, plane.v, 0x7F);
	QMSVEC h = _mm_rsqrt_ps(d);
	h = _mm_mul_ps(plane.v, h);
	return *(QmVec4*)&h;
#else
	float l = qm_vec3_len(plane);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec_mag(plane, l);
#endif
}

/// @brief 평면 뒤집어서 정규화
INLINE QmVec4 QM_VECTORCALL qm_plane_rnorm(const QmVec4 plane)
{
#if defined QM_USE_AVX
	QMSVEC d = _mm_dp_ps(plane.v, plane.v, 0x7F);
	QMSVEC h = _mm_mul_ps(_mm_rsqrt_ps(d), QMCONST_NEG.v);
	h = _mm_mul_ps(plane.v, h);
	return *(QmVec4*)&h;
#else
	float l = qm_vec3_len(plane);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec_mag(plane, -l);
#endif
}

/// @brief 평면 트랜스폼
INLINE QmVec4 QM_VECTORCALL qm_plane_trfm(const QmVec4 plane, const QmMat4 m)
{
	QmVec4 x = qm_vec_sp_x(plane);
	QmVec4 y = qm_vec_sp_y(plane);
	QmVec4 z = qm_vec_sp_z(plane);
	QmVec4 w = qm_vec_sp_w(plane);
	QmVec4 h = qm_vec_mul(w, m.r[3]);
	h = qm_vec_madd(z, m.r[2], h);
	h = qm_vec_madd(y, m.r[1], h);
	h = qm_vec_madd(x, m.r[0], h);
	return h;
}

/// @brief 평면 내적 벡터형
INLINE QmVec4 QM_VECTORCALL qm_plane_simd_dot(const QmVec4 plane, const QmVec4 v)
{
	return qm_vec4_simd_dot(plane, v);
}

/// @brief 평면과 점의 내적 벡터형
INLINE QmVec4 QM_VECTORCALL qm_plane_simd_dot_coord(const QmVec4 plane, const QmVec4 v)
{
	QmVec4 s = qm_vec_select(QMCONST_ONE, v, QMCONST_S1110.v4);
	return qm_vec4_simd_dot(plane, s);
}

/// @brief 평면과 점의 법선 내적 벡터형
INLINE QmVec4 QM_VECTORCALL qm_plane_simd_dot_normal(const QmVec4 plane, const QmVec4 v)
{
	return qm_vec3_simd_dot(plane, v);
}

/// @brief 평면 내적
INLINE float QM_VECTORCALL qm_plane_dot(const QmVec4 plane, const QmVec4 v)
{
	return qm_vec4_dot(plane, v);
}

/// @brief 평면과 점의 내적 (= 평면과 점의 거리)
INLINE float QM_VECTORCALL qm_plane_dot_coord(const QmVec4 plane, const QmVec4 v)
{
	return qm_vec3_dot(plane, v) + plane.W;
}

/// @brief 평면과 점의 법선 내적
INLINE float QM_VECTORCALL qm_plane_dot_normal(const QmVec4 plane, const QmVec4 v)
{
	return qm_vec3_dot(plane, v);
}

/// @brief 평면과 점의 거리
INLINE float QM_VECTORCALL qm_plane_dist_vec3(const QmVec4 plane, const QmVec4 v)
{
	return qm_plane_dot_coord(plane, v);
}

/// @brief 평면과 선분의 거리
INLINE float QM_VECTORCALL qm_plane_dist_line(const QmVec4 plane, const QmVec4 linePoint1, const QmVec4 linePoint2)
{
	QmVec4 v = qm_vec_sub(linePoint2, linePoint1);
	float d = qm_vec3_dot(plane, v);
	float n = qm_vec3_dot(plane, linePoint1) + plane.W;
	if (qm_eqf(d, 0.0f))
		return n;
	return -n / d;
}

/// @brief 점과 면의 관계 (0이면 면위, 1이면 면 위쪽, -1이면 면 아래쪽)
INLINE int QM_VECTORCALL qm_plane_relation(const QmVec4 plane, const QmVec4 v)
{
	float d = qm_plane_dot_coord(plane, v);
	if (d < -QM_EPSILON)
		return -1; // 아래
	if (d > QM_EPSILON)
		return 1; // 위
	return 0; // 면위
}

/// @brief 면과 선의 충돌을 검사하고 충돌점을 구한다
INLINE bool QM_VECTORCALL qm_plane_intersect_line(const QmVec4 plane, const QmVec4 linePoint1, const QmVec4 linePoint2, QmVec4* intersectPoint)
{
	return qm_plane_intersect_line2(plane, linePoint1, qm_vec_sub(linePoint2, linePoint1), intersectPoint);
}

/// @brief 면과 선의 충돌을 검사하고 충돌점을 구한다 (선의 시작과 방향 사용)
INLINE bool QM_VECTORCALL qm_plane_intersect_line2(const QmVec4 plane, const QmVec4 loc, const QmVec4 dir, QmVec4* intersectPoint)
{
	float d = qm_vec3_dot(plane, dir);
	if (qm_eqf(d, 0.0f))
		return false;
	if (intersectPoint)
	{
		float n = qm_vec3_dot(plane, loc) + plane.W;
		QmVec4 vd = qm_vec_mag(dir, -n / d);
		*intersectPoint = qm_vec_add(loc, vd);
	}
	return true;
}

/// @brief 면과 면의 충돌을 검사하고 충돌점을 구한다
INLINE bool QM_VECTORCALL qm_plane_intersect_plane(const QmVec4 plane1, const QmVec4 plane2, QmVec4* loc, QmVec4* dir)
{
	float f0 = qm_vec3_len(plane1);
	float f1 = qm_vec3_len(plane2);
	float f2 = qm_vec3_dot(plane1, plane2);
	float det = f0 * f1 - f2 * f2;
	if (qm_eqf(det, 0.0f))
		return false;
	if (dir)
		*dir = qm_vec3_cross(plane1, plane2);
	if (loc)
	{
		float rdet = 1.0f / det;
		float fa = (f2 * plane2.W - f1 * plane1.W) * rdet;
		float fb = (f2 * plane1.W - f0 * plane2.W) * rdet;
		*loc = qm_vec_add(qm_vec_mag(plane1, fa), qm_vec_mag(plane2, fb));
	}
	return true;
}

/// @brief 세 면의 충돌을 검사하고, 충돌 지점을 구한다
INLINE bool QM_VECTORCALL qm_plane_intersect_planes(const QmVec4 plane1, const QmVec4 plane2, const QmVec4 plane3, QmVec4* intersectPoint)
{
	QmVec4 dir, loc;
	return qm_plane_intersect_plane(plane1, plane2, &loc, &dir) ? qm_plane_intersect_line2(plane3, loc, dir, intersectPoint) : false;
}


//////////////////////////////////////////////////////////////////////////
// color4

#define qm_color_eq		qm_vec_eq			/// @brief 색깔 비교
#define qm_color_eps	qm_vec_eps			/// @brief 색깔 비교 (입실론)
#define qm_color_isu	qm_quat_isu			/// @brief 검은색에 알파는 1인가?

/// @brief 색상 값 설정
INLINE QmVec4 QM_VECTORCALL qm_color(float r, float g, float b, float a)
{
	return qm_vec(r, g, b, a);
}

/// @brief 정수로 색깔 설정
INLINE QmVec4 QM_VECTORCALL qm_coloru(uint value)
{
	const float i = 1.0f / 255.0f;
	float b = (float)(value & 255) * i; value >>= 8;
	float g = (float)(value & 255) * i; value >>= 8;
	float r = (float)(value & 255) * i; value >>= 8;
	float a = (float)(value & 255) * i;
	return qm_vec(r, g, b, a);
}

/// @brief kolor 색깔 설정
INLINE QmVec4 QM_VECTORCALL qm_colork(QmKolor k)
{
	const float i = 1.0f / 255.0f;
	return qm_vec(k.R * i, k.G * i, k.B * i, k.A * i);
}

/// @brief 초기 색깔
INLINE QmVec4 QM_VECTORCALL qm_color_unit(void)
{
	return QMCONST_UNIT_R3;
}

/// @brief 같은값 설정
INLINE QmVec4 QM_VECTORCALL qm_color_sp(float value, float alpha)
{
	return qm_vec(value, value, value, alpha);
}

/// @brief 네거티브 색
INLINE QmVec4 QM_VECTORCALL qm_color_neg(const QmVec4 c)
{
#if defined QM_USE_AVX
	static const QmVec4 ONE3 = { { 1.0f, 1.0f, 1.0f, 0.0f } };
	static const QmVec4 NEG3 = { { -1.0f, -1.0f, -1.0f, 0.0f } };
	QMSVEC t = _mm_xor_ps(c.v, NEG3.v);
	return (QmVec4) { .v = _mm_add_ps(t, ONE3.v) };
#elif defined QM_USE_NEON
	uint32x4_t t = veorq_u32(vreinterpretq_u32_f32(c.v), NEG3.v);
	return (QmVec4) { .v = vaddq_f32(vreinterpretq_f32_u32(t), ONE3.v) };
#else
	return qm_color(1.0f - c.X, 1.0f - c.Y, 1.0f - c.Z, c.W);
#endif
}

/// @brief 색깔 혼합
INLINE QmVec4 QM_VECTORCALL qm_color_mod(const QmVec4 left, const QmVec4 right)
{
	return qm_vec_mul(left, right);
}

/// @brief 콘트라스트 조정
INLINE QmVec4 QM_VECTORCALL qm_color_contrast(const QmVec4 c, float contrast)
{
#if defined QM_USE_AVX
	static const QmVec4 half = { { 0.5f, 0.5f, 0.5f, 0.5f } };
	QMSVEC s = _mm_set_ps1(contrast);
	QMSVEC h = _mm_sub_ps(c.v, half.v);
	h = _MM_FMADD_PS(h, s, half.v);
	s = _mm_shuffle_ps(h, c.v, _MM_SHUFFLE(3, 2, 2, 2));
	h = _mm_shuffle_ps(h, s, _MM_SHUFFLE(3, 0, 1, 0));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	static const QmVec4 half = { { 0.5f, 0.5f, 0.5f, 0.5f } };
	QMSVEC h = vsubq_f32(c.v, half.v);
	h = vmlaq_n_f32(half.v, h, contrast);
	h = vbslq_f32(QMCONST_S1110.v4, h, half.v);
	return *(QmVec4*)&h;
#else
	float r = 0.5f + contrast * (c.X - 0.5f);
	float g = 0.5f + contrast * (c.Y - 0.5f);
	float b = 0.5f + contrast * (c.Z - 0.5f);
	return qm_color(r, g, b, c.W);
#endif
}

/// @brief 새츄레이션 조정
INLINE QmVec4 QM_VECTORCALL qm_color_saturation(const QmVec4 c, float saturation)
{
	static const QmVec4 luminance = { { 0.2125f, 0.7154f, 0.0721f, 0.0f } };
#if defined QM_USE_AVX
	QMSVEC l = qm_vec3_simd_dot(c, luminance).v;
	QMSVEC s = _mm_set_ps1(saturation);
	QMSVEC h = _mm_sub_ps(c.v, l);
	h = _MM_FMADD_PS(h, s, l);
	l = _mm_shuffle_ps(h, c.v, _MM_SHUFFLE(3, 2, 2, 2));
	h = _mm_shuffle_ps(h, l, _MM_SHUFFLE(3, 0, 1, 0));
	return *(QmVec4*)&h;
#elif defined QM_USE_NEON
	QMSVEC l = qm_vec3_simd_dot(c, luminance).v;
	QMSVEC h = vsubq_f32(c.v, l);
	h = vmlaq_n_f32(l, h, saturation);
	h = vbslq_f32(QMCONST_S1110.v4, l, c.v);
	return *(QmVec4*)&h;
#else
	float l = qm_vec3_dot(c, luminance);
	float r = ((c.X - l) * saturation) + l;
	float g = ((c.Y - l) * saturation) + l;
	float b = ((c.Z - l) * saturation) + l;
	return qm_color(r, g, b, c.W);
#endif
}


//////////////////////////////////////////////////////////////////////////
// matrix4x4

/// @brief 단위 행렬 (항등 행렬)
INLINE QmMat4 QM_VECTORCALL qm_mat4_unit(void)
{
	QmMat4 m;
	m.v[0] = QMCONST_UNIT_R0.v;
	m.v[1] = QMCONST_UNIT_R1.v;
	m.v[2] = QMCONST_UNIT_R2.v;
	m.v[3] = QMCONST_UNIT_R3.v;
	return m;
}

/// @brief 행렬을 0으로 초기화 한다
INLINE QmMat4 QM_VECTORCALL qm_mat4_zero(void)
{
	QmMat4 m;
	m.v[0] = QMCONST_ZERO.v;
	m.v[1] = QMCONST_ZERO.v;
	m.v[2] = QMCONST_ZERO.v;
	m.v[3] = QMCONST_ZERO.v;
	return m;
}

/// @brief 대각 행렬을 만든다
INLINE QmMat4 QM_VECTORCALL qm_mat4_diag(float diag)
{
	return (QmMat4) { ._11 = diag, ._22 = diag, ._33 = diag, ._44 = diag, };
}

/// @brief 두 행렬의 덧셈
INLINE QmMat4 QM_VECTORCALL qm_mat4_add(const QmMat4 left, const QmMat4 right)
{
	QmMat4 m;
	m.r[0] = qm_vec_add(left.r[0], right.r[0]);
	m.r[1] = qm_vec_add(left.r[1], right.r[1]);
	m.r[2] = qm_vec_add(left.r[2], right.r[2]);
	m.r[3] = qm_vec_add(left.r[3], right.r[3]);
	return m;
}

/// @brief 두 행렬의 뺄셈
INLINE QmMat4 QM_VECTORCALL qm_mat4_sub(const QmMat4 left, const QmMat4 right)
{
	QmMat4 m;
	m.r[0] = qm_vec_sub(left.r[0], right.r[0]);
	m.r[1] = qm_vec_sub(left.r[1], right.r[1]);
	m.r[2] = qm_vec_sub(left.r[2], right.r[2]);
	m.r[3] = qm_vec_sub(left.r[3], right.r[3]);
	return m;
}

/// @brief 행렬의 확대
INLINE QmMat4 QM_VECTORCALL qm_mat4_mag(const QmMat4 left, float right)
{
	QmMat4 m;
#if defined QM_USE_AVX
	QMSVEC mm = _mm_set1_ps(right);
	m.v[0] = _mm_mul_ps(left.v[0], mm);
	m.v[1] = _mm_mul_ps(left.v[1], mm);
	m.v[2] = _mm_mul_ps(left.v[2], mm);
	m.v[3] = _mm_mul_ps(left.v[3], mm);
#elif defined QM_USE_NEON
	m.v[0] = vmulq_n_f32(left.v[0], right);
	m.v[1] = vmulq_n_f32(left.v[1], right);
	m.v[2] = vmulq_n_f32(left.v[2], right);
	m.v[3] = vmulq_n_f32(left.v[3], right);
#else
	m.r[0] = qm_vec_mag(left.r[0], right);
	m.r[1] = qm_vec_mag(left.r[1], right);
	m.r[2] = qm_vec_mag(left.r[2], right);
	m.r[3] = qm_vec_mag(left.r[3], right);
#endif
	return m;
}

/// @brief 행렬의 줄이기
INLINE QmMat4 QM_VECTORCALL qm_mat4_abr(const QmMat4 left, float right)
{
	QmMat4 m;
#if defined QM_USE_AVX
	QMSVEC mm = _mm_set1_ps(right);
	m.v[0] = _mm_div_ps(left.v[0], mm);
	m.v[1] = _mm_div_ps(left.v[1], mm);
	m.v[2] = _mm_div_ps(left.v[2], mm);
	m.v[3] = _mm_div_ps(left.v[3], mm);
#elif defined QM_USE_NEON
	QMSVEC mm = vdupq_n_f32(right);
	m.v[0] = vdivq_f32(left.v[0], right);
	m.v[1] = vdivq_f32(left.v[1], right);
	m.v[2] = vdivq_f32(left.v[2], right);
	m.v[3] = vdivq_f32(left.v[3], right);
#else
	m.r[0] = qm_vec_abr(left.r[0], right);
	m.r[1] = qm_vec_abr(left.r[1], right);
	m.r[2] = qm_vec_abr(left.r[2], right);
	m.r[3] = qm_vec_abr(left.r[3], right);
#endif
	return m;
}

/// @brief 행렬 전치
INLINE QmMat4 QM_VECTORCALL qm_mat4_tran(const QmMat4 m)
{
#if defined QM_USE_AVX
	QMSVEC r0 = _mm_shuffle_ps(m.v[0], m.v[1], 0x44);
	QMSVEC r2 = _mm_shuffle_ps(m.v[0], m.v[1], 0xEE);
	QMSVEC r1 = _mm_shuffle_ps(m.v[2], m.v[3], 0x44);
	QMSVEC r3 = _mm_shuffle_ps(m.v[2], m.v[3], 0xEE);
	QmMat4 h;
	h.v[0] = _mm_shuffle_ps(r0, r1, 0x88);
	h.v[1] = _mm_shuffle_ps(r0, r1, 0xDD);
	h.v[2] = _mm_shuffle_ps(r2, r3, 0x88);
	h.v[3] = _mm_shuffle_ps(r2, r3, 0xDD);
	return h;
#elif defined QM_USE_NEON
	float32x4x4_t t = vld4q_f32(m.f);
	return (QmMat4) { .v[0] = t.val[0], .v[1] = t.val[1], .v[2] = t.val[2], .v[3] = t.val[3] };
#else
	return (QmMat4) { {  m._11, m._21, m._31, m._41, m._12, m._22, m._32, m._42, m._13, m._23, m._33, m._43, m._14, m._24, m._34, m._44 } };
#endif
}

/// @brief 행렬 곱셈
INLINE QmMat4 QM_VECTORCALL qm_mat4_mul(const QmMat4 left, const QmMat4 right)
{
#if defined QM_USE_AVX2
	__m256 t0 = _mm256_castps128_ps256(left.v[0]); t0 = _mm256_insertf128_ps(t0, left.v[1], 1);
	__m256 t1 = _mm256_castps128_ps256(left.v[2]); t1 = _mm256_insertf128_ps(t1, left.v[3], 1);
	__m256 u0 = _mm256_castps128_ps256(right.v[0]); u0 = _mm256_insertf128_ps(u0, right.v[1], 1);
	__m256 u1 = _mm256_castps128_ps256(right.v[2]); u1 = _mm256_insertf128_ps(u1, right.v[3], 1);
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
	QmMat4 m;
	m.v[0] = _mm256_castps256_ps128(t0);
	m.v[1] = _mm256_extractf128_ps(t0, 1);
	m.v[2] = _mm256_castps256_ps128(t1);
	m.v[3] = _mm256_extractf128_ps(t1, 1);
	return m;
#elif defined QM_USE_AVX
	QmMat4 m;
	QMSVEC vW = left.v[0];
	QMSVEC vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	QMSVEC vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	QMSVEC vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.v[0]);
	vY = _mm_mul_ps(vY, right.v[1]);
	vZ = _mm_mul_ps(vZ, right.v[2]);
	vW = _mm_mul_ps(vW, right.v[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	m.v[0] = vX;
	vW = left.v[1];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.v[0]);
	vY = _mm_mul_ps(vY, right.v[1]);
	vZ = _mm_mul_ps(vZ, right.v[2]);
	vW = _mm_mul_ps(vW, right.v[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	m.v[1] = vX;
	vW = left.v[2];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.v[0]);
	vY = _mm_mul_ps(vY, right.v[1]);
	vZ = _mm_mul_ps(vZ, right.v[2]);
	vW = _mm_mul_ps(vW, right.v[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	m.v[2] = vX;
	vW = left.v[3];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.v[0]);
	vY = _mm_mul_ps(vY, right.v[1]);
	vZ = _mm_mul_ps(vZ, right.v[2]);
	vW = _mm_mul_ps(vW, right.v[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	m.v[3] = vX;
	return m;
#elif defined QM_USE_NEON
	QMSVEC l0 = left.v[0];
	QMSVEC l1 = left.v[1];
	QMSVEC l2 = left.v[2];
	QMSVEC l3 = left.v[3];
	QmMat4 m;
	for (int i = 0; i < 4; i++)
	{
		QMSVEC v, r = right.v[i];
		v = vmulq_n_f32(l0, vgetq_lane_f32(r, 0));
		v = vmlaq_n_f32(v, l1, vgetq_lane_f32(r, 1));
		v = vmlaq_n_f32(v, l2, vgetq_lane_f32(r, 2));
		v = vmlaq_n_f32(v, l3, vgetq_lane_f32(r, 3));
		m.v[i] = v;
	}
	return m;
#else
	QmMat4 m;
	m.r[0] = qm_vec4_trfm(right.r[0], left);
	m.r[1] = qm_vec4_trfm(right.r[1], left);
	m.r[2] = qm_vec4_trfm(right.r[2], left);
	m.r[3] = qm_vec4_trfm(right.r[3], left);
	return m;
#endif
}

/// @brief 행렬의 전치곱
INLINE QmMat4 QM_VECTORCALL qm_mat4_tmul(const QmMat4 left, const QmMat4 right)
{
#if defined QM_USE_AVX2
	__m256 t0 = _mm256_castps128_ps256(left.v[0]); t0 = _mm256_insertf128_ps(t0, left.v[1], 1);
	__m256 t1 = _mm256_castps128_ps256(left.v[2]); t1 = _mm256_insertf128_ps(t1, left.v[3], 1);
	__m256 u0 = _mm256_castps128_ps256(right.v[0]); u0 = _mm256_insertf128_ps(u0, right.v[1], 1);
	__m256 u1 = _mm256_castps128_ps256(right.v[2]); u1 = _mm256_insertf128_ps(u1, right.v[3], 1);
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
	QmMat4 m;
	m.v[0] = _mm256_castps256_ps128(t0);
	m.v[1] = _mm256_extractf128_ps(t0, 1);
	m.v[2] = _mm256_castps256_ps128(t1);
	m.v[3] = _mm256_extractf128_ps(t1, 1);
	return m;
#elif defined QM_USE_AVX
	QMSVEC vW = left.v[0];
	QMSVEC vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	QMSVEC vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	QMSVEC vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.v[0]);
	vY = _mm_mul_ps(vY, right.v[1]);
	vZ = _mm_mul_ps(vZ, right.v[2]);
	vW = _mm_mul_ps(vW, right.v[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	QMSVEC r0 = vX;
	vW = left.v[1];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.v[0]);
	vY = _mm_mul_ps(vY, right.v[1]);
	vZ = _mm_mul_ps(vZ, right.v[2]);
	vW = _mm_mul_ps(vW, right.v[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	QMSVEC r1 = vX;
	vW = left.v[2];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.v[0]);
	vY = _mm_mul_ps(vY, right.v[1]);
	vZ = _mm_mul_ps(vZ, right.v[2]);
	vW = _mm_mul_ps(vW, right.v[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	QMSVEC r2 = vX;
	vW = left.v[3];
	vX = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
	vY = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
	vZ = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
	vW = _MM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
	vX = _mm_mul_ps(vX, right.v[0]);
	vY = _mm_mul_ps(vY, right.v[1]);
	vZ = _mm_mul_ps(vZ, right.v[2]);
	vW = _mm_mul_ps(vW, right.v[3]);
	vX = _mm_add_ps(vX, vZ);
	vY = _mm_add_ps(vY, vW);
	vX = _mm_add_ps(vX, vY);
	QMSVEC r3 = vX;
	// 전치
	QMSVEC p1 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1, 0, 1, 0));
	QMSVEC p3 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(3, 2, 3, 2));
	QMSVEC p2 = _mm_shuffle_ps(r2, r3, _MM_SHUFFLE(1, 0, 1, 0));
	QMSVEC p4 = _mm_shuffle_ps(r2, r3, _MM_SHUFFLE(3, 2, 3, 2));
	QmMat4 m;
	m.v[0] = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(2, 0, 2, 0));
	m.v[1] = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(3, 1, 3, 1));
	m.v[2] = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(2, 0, 2, 0));
	m.v[3] = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(3, 1, 3, 1));
	return m;
#else
	return qm_mat4_tran(qm_mat4_mul(left, right));
#endif
}

/// @brief 역행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_inv(const QmMat4 m)
{
#if defined QM_USE_AVX
	static const QmVecU PNNP = { { 0x00000000, 0x80000000, 0x80000000, 0x00000000 } };
	QMSVEC a, b, c, d;
	QMSVEC ia, ib, ic, id;
	QMSVEC dc, ab;
	QMSVEC ma, mb, mc, md;
	QMSVEC dt, d0, d1, d2;
	QMSVEC rd;
	a = _mm_movelh_ps(m.v[0], m.v[1]);
	b = _mm_movehl_ps(m.v[1], m.v[0]);
	c = _mm_movelh_ps(m.v[2], m.v[3]);
	d = _mm_movehl_ps(m.v[3], m.v[2]);
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
	rd = _mm_xor_ps(rd, PNNP.v);
	ib = _mm_sub_ps(_mm_mul_ps(c, _mm_shuffle_ps(mb, mb, 0)), ib);
	ic = _mm_sub_ps(_mm_mul_ps(b, _mm_shuffle_ps(mc, mc, 0)), ic);
	ia = _mm_div_ps(ia, rd);
	ib = _mm_div_ps(ib, rd);
	ic = _mm_div_ps(ic, rd);
	id = _mm_div_ps(id, rd);
	QmMat4 h;
	h.v[0] = _mm_shuffle_ps(ia, ib, 0x77);
	h.v[1] = _mm_shuffle_ps(ia, ib, 0x22);
	h.v[2] = _mm_shuffle_ps(ic, id, 0x77);
	h.v[3] = _mm_shuffle_ps(ic, id, 0x22);
	return m;
	// 행렬식: *(float*)&dt
#elif defined QM_USE_NEON
	float32x4_t row1 = m.v[0];
	float32x4_t row2 = m.v[1];
	float32x4_t row3 = m.v[2];
	float32x4_t row4 = m.v[3];
	float32x4_t minor0, minor1, minor2, minor3;
	float32x4_t det, tmp1;
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
	QmMat4 h;
	h.v[0] = vmulq_f32(det, minor0);
	h.v[1] = vmulq_f32(det, minor1);
	h.v[2] = vmulq_f32(det, minor2);
	h.v[3] = vmulq_f32(det, minor3);
	return h;
	// 행렬식: vgetq_lane_f32(det, 0)
#else
	QmVec4 c01 = qm_vec3_cross(m.r[0], m.r[1]);
	QmVec4 c23 = qm_vec3_cross(m.r[2], m.r[3]);
	QmVec4 s10 = qm_vec_sub(qm_vec_mag(m.r[0], m._24), qm_vec_mag(m.r[1], m._14));
	QmVec4 s32 = qm_vec_sub(qm_vec_mag(m.r[2], m._44), qm_vec_mag(m.r[3], m._34));
	QmVec4 inv = qm_vec_div(QMCONST_ONE, qm_vec_add(qm_vec3_simd_dot(c01, s32), qm_vec3_simd_dot(c23, s10)));
	c01 = qm_vec_mul(c01, inv);
	c23 = qm_vec_mul(c23, inv);
	s10 = qm_vec_mul(s10, inv);
	s32 = qm_vec_mul(s32, inv);
	QmMat4 h;
	h.r[0] = qm_vec4v(qm_vec_add(qm_vec3_cross(m.r[1], s32), qm_vec_mag(c23, m._24)), -qm_vec3_dot(m.r[1], c23));
	h.r[1] = qm_vec4v(qm_vec_sub(qm_vec3_cross(s32, m.r[0]), qm_vec_mag(c23, m._14)), +qm_vec3_dot(m.r[0], c23));
	h.r[2] = qm_vec4v(qm_vec_add(qm_vec3_cross(m.r[3], s10), qm_vec_mag(c01, m._44)), -qm_vec3_dot(m.r[3], c01));
	h.r[3] = qm_vec4v(qm_vec_sub(qm_vec3_cross(s10, m.r[2]), qm_vec_mag(c01, m._34)), +qm_vec3_dot(m.r[2], c01));
	return qm_mat4_tran(h);
	// 행렬식: 1.0f / inv
#endif
}

/// @brief 스케일 행렬을 만든다
INLINE QmMat4 QM_VECTORCALL qm_mat4_scl(float x, float y, float z)
{
	QmMat4 r = { ._11 = x, ._22 = y, ._33 = z, ._44 = 1.0f, };
	return r;
}

/// @brief 스케일 행렬을 만든다
INLINE QmMat4 QM_VECTORCALL qm_mat4_scl_vec3(const QmVec4 v)
{
	return qm_mat4_scl(v.X, v.Y, v.Z);
}

/// @brief 위치 행렬을 만든다
INLINE QmMat4 QM_VECTORCALL qm_mat4_loc(float x, float y, float z)
{
	QmMat4 r =
	{
		._11 = 1.0f, ._22 = 1.0f, ._33 = 1.0f,
		._41 = x, ._42 = y, ._43 = z, ._44 = 1.0f,
	};
	return r;
}

/// @brief 위치 행렬을 만든다
INLINE QmMat4 QM_VECTORCALL qm_mat4_loc_vec(const QmVec4 v)
{
	return qm_mat4_loc(v.X, v.Y, v.Z);
}

/// @brief 회전 행렬을 만든다
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_rot(const QmVec4 axis, float angle)
{
	QmVec4 norm = qm_vec3_norm(axis);
	float s, c;
	qm_sincosf(angle, &s, &c);
	float nc = 1.0f - c;
	QmMat4 r =
	{
		._11 = (norm.X * norm.X * nc) + c,
		._12 = (norm.X * norm.Y * nc) + (norm.Z * s),
		._13 = (norm.X * norm.Z * nc) - (norm.Y * s),
		._21 = (norm.Y * norm.X * nc) - (norm.Z * s),
		._22 = (norm.Y * norm.Y * nc) + c,
		._23 = (norm.Y * norm.Z * nc) + (norm.X * s),
		._31 = (norm.Z * norm.X * nc) + (norm.Y * s),
		._32 = (norm.Z * norm.Y * nc) - (norm.X * s),
		._33 = (norm.Z * norm.Z * nc) + c,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 회전 행렬을 만든다 (롤/피치/요)
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_vec3(const QmVec4 rot)
{
	float sr, sp, sy;
	float cr, cp, cy;
	qm_sincosf(rot.X, &sr, &cr);
	qm_sincosf(rot.Y, &sp, &cp);
	qm_sincosf(rot.Z, &sy, &cy);
	float srsp = sr * sp;
	float crsp = cr * sp;
	QmMat4 r =
	{
		._11 = cp * cy,
		._12 = cp * sy,
		._13 = -sp,
		._21 = srsp * cy - cr * sy,
		._22 = srsp * sy + cr * cy,
		._23 = sr * cp,
		._31 = crsp * cy + sr * sy,
		._32 = crsp * sy - sr * cy,
		._33 = cr * cp,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 사원수로 회전 행렬을 만든다
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_quat(const QmVec4 rot)
{
#ifdef QM_USE_AVX
	static const QmVec4 c1110 = { { 1.0f, 1.0f, 1.0f, 0.0f } };
	static const QmVecU mask3 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } };
	QMSVEC Q0 = _mm_add_ps(rot.v, rot.v);
	QMSVEC Q1 = _mm_mul_ps(rot.v, Q0);
	QMSVEC V0 = _MM_PERMUTE_PS(Q1, _MM_SHUFFLE(3, 0, 0, 1));
	V0 = _mm_and_ps(V0, mask3.v);
	QMSVEC V1 = _MM_PERMUTE_PS(Q1, _MM_SHUFFLE(3, 1, 2, 2));
	V1 = _mm_and_ps(V1, mask3.v);
	QMSVEC R0 = _mm_sub_ps(c1110.v, V0);
	R0 = _mm_sub_ps(R0, V1);
	V0 = _MM_PERMUTE_PS(rot.v, _MM_SHUFFLE(3, 1, 0, 0));
	V1 = _MM_PERMUTE_PS(Q0, _MM_SHUFFLE(3, 2, 1, 2));
	V0 = _mm_mul_ps(V0, V1);
	V1 = _MM_PERMUTE_PS(rot.v, _MM_SHUFFLE(3, 3, 3, 3));
	QMSVEC V2 = _MM_PERMUTE_PS(Q0, _MM_SHUFFLE(3, 0, 2, 1));
	V1 = _mm_mul_ps(V1, V2);
	QMSVEC R1 = _mm_add_ps(V0, V1);
	QMSVEC R2 = _mm_sub_ps(V0, V1);
	V0 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(1, 0, 2, 1));
	V0 = _MM_PERMUTE_PS(V0, _MM_SHUFFLE(1, 3, 2, 0));
	V1 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(2, 2, 0, 0));
	V1 = _MM_PERMUTE_PS(V1, _MM_SHUFFLE(2, 0, 2, 0));
	Q1 = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(1, 0, 3, 0));
	Q1 = _MM_PERMUTE_PS(Q1, _MM_SHUFFLE(1, 3, 2, 0));
	QmMat4 m;
	m.v[0] = Q1;
	Q1 = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(3, 2, 3, 1));
	Q1 = _MM_PERMUTE_PS(Q1, _MM_SHUFFLE(1, 3, 0, 2));
	m.v[1] = Q1;
	Q1 = _mm_shuffle_ps(V1, R0, _MM_SHUFFLE(3, 2, 1, 0));
	m.v[2] = Q1;
	m.v[3] = QMCONST_UNIT_R3.v;
	return m;
#else
	QmVec4 norm = qm_vec4_norm(rot);
	float XX = norm.X * norm.X;
	float YY = norm.Y * norm.Y;
	float ZZ = norm.Z * norm.Z;
	float XY = norm.X * norm.Y;
	float XZ = norm.X * norm.Z;
	float YZ = norm.Y * norm.Z;
	float WX = norm.W * norm.X;
	float WY = norm.W * norm.Y;
	float WZ = norm.W * norm.Z;
	QmMat4 r =
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
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_x(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
	QmMat4 r =
	{
		._11 = 1.0f,
		._22 = c, ._23 = s,
		._32 = -s, ._33 = c,
		._44 = 1.0f,
	};
	return r;
}

/// @brief Y축 회전 행렬을 만든다
/// @param rot Y측 회전값
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_y(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
	QmMat4 r =
	{
		._11 = c, ._13 = -s,
		._22 = 1.0f,
		._31 = s, ._33 = c,
		._44 = 1.0f,
	};
	return r;
}

/// @brief Z축 회전 행렬을 만든다
/// @param rot Z측 회전값
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot_z(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
	QmMat4 r =
	{
		._11 = c, ._12 = s,
		._21 = -s, ._22 = c,
		._33 = 1.0f,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 아핀 변환 행렬
/// @param scl 벡터3 스케일 (1일 경우 NULL)
/// @param rotcenter 벡터3 회전축(원점일 경우 NULL)
/// @param rot 사원수 회전 (고정일 경우 NULL)
/// @param loc 벡터3 위치 (원점일 경우 NULL)
INLINE QmMat4 QM_VECTORCALL qm_mat4_affine(const QmVec4* scl, const QmVec4* rotcenter, const QmVec4* rot, const QmVec4* loc)
{
	QmMat4 m = scl ? qm_mat4_scl_vec3(*scl) : qm_mat4_unit();
	QmVec4 vc = rotcenter ? *rotcenter : QMCONST_ZERO;
	QmMat4 mr = rot ? qm_mat4_rot_quat(*rot) : qm_mat4_unit();
	QmVec4 vl = loc ? *loc : QMCONST_ZERO;
	m.r[3] = qm_vec_sub(m.r[3], vc);
	m = qm_mat4_mul(m, mr);
	m.r[3] = qm_vec_add(m.r[3], vc);
	m.r[3] = qm_vec_add(m.r[3], vl);
	return m;
}

// 
INLINE void QM_VECTORCALL qm_mat4_internal_trfm_loc_scl(QmMat4* m, const QmVec4 loc, const QmVec4* scl)
{
	float* f = m->f;
	f[0] += loc.X * f[3];
	f[1] += loc.Y * f[3];
	f[2] += loc.Z * f[3];
	f[4] += loc.X * f[7];
	f[5] += loc.Y * f[7];
	f[6] += loc.Z * f[7];
	f[8] += loc.X * f[11];
	f[9] += loc.Y * f[11];
	f[10] += loc.Z * f[11];
	f[12] += loc.X * f[15];
	f[13] += loc.Y * f[15];
	f[14] += loc.Z * f[15];
	if (scl)
	{
		f[0] *= scl->X;
		f[1] *= scl->X;
		f[2] *= scl->X;
		f[3] *= scl->X;
		f[4] *= scl->Y;
		f[5] *= scl->Y;
		f[6] *= scl->Y;
		f[7] *= scl->Y;
		f[8] *= scl->Z;
		f[9] *= scl->Z;
		f[10] *= scl->Z;
		f[11] *= scl->Z;
	}
}

/// @brief 행렬 트랜스폼
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm(QmVec4 loc, QmVec4 rot, QmVec4* scl)
{
	QmMat4 m = qm_mat4_rot_quat(rot);
	qm_mat4_internal_trfm_loc_scl(&m, loc, scl);
	return m;
}

/// @brief 행렬 트랜스폼. 단 벡터 회전
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm_vec(QmVec4 loc, QmVec4 rot, QmVec4* scl)
{
	QmMat4 m = qm_mat4_rot_vec3(rot);
	qm_mat4_internal_trfm_loc_scl(&m, loc, scl);
	return m;
}

/// @brief 반사 행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_reflect(const QmVec4 plane)
{
	static const QmVec4 neg2 = { { -2.0f, -2.0f, -2.0f, 0.0f } };
	QmVec4 p = qm_plane_norm(plane);
	QmVec4 s = qm_vec_mul(p, neg2);
	QmVec4 a = qm_vec_sp_x(p);
	QmVec4 b = qm_vec_sp_y(p);
	QmVec4 c = qm_vec_sp_z(p);
	QmVec4 d = qm_vec_sp_w(p);
	QmMat4 m;
	m.r[0] = qm_vec_madd(a, s, QMCONST_UNIT_R0);
	m.r[1] = qm_vec_madd(b, s, QMCONST_UNIT_R1);
	m.r[2] = qm_vec_madd(c, s, QMCONST_UNIT_R2);
	m.r[3] = qm_vec_madd(d, s, QMCONST_UNIT_R3);
	return m;
}

/// @brief 그림자 행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_shadow(const QmVec4 plane, const QmVec4 light)
{
	static const QmVecU s0001 = { { 0, 0, 0, 0xFFFFFFFF } };
	QmVec4 p = qm_plane_norm(plane);
	QmVec4 dot = qm_plane_simd_dot(p, light);
	p = qm_vec_neg(p);
	QmVec4 a = qm_vec_sp_x(p);
	QmVec4 b = qm_vec_sp_y(p);
	QmVec4 c = qm_vec_sp_z(p);
	QmVec4 d = qm_vec_sp_w(p);
	dot = qm_vec_select(s0001.v4, dot, s0001.v4);
	QmMat4 m;
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
INLINE QmMat4 QM_VECTORCALL qm_mat4_internal_look_to(const QmVec4 eye, const QmVec4 dir, const QmVec4 up)
{
	QmVec4 r2 = qm_vec4_norm(dir);
	QmVec4 r0 = qm_vec4_norm(qm_vec3_cross(up, r2));
	QmVec4 r1 = qm_vec3_cross(r2, r0);
	QmVec4 r3 = qm_vec_neg(eye);
	QmVec4 d0 = qm_vec3_simd_dot(r0, r3);
	QmVec4 d1 = qm_vec3_simd_dot(r1, r3);
	QmVec4 d2 = qm_vec3_simd_dot(r2, r3);
	QmMat4 m;
	m.r[0] = qm_vec_select(d0, r0, QMCONST_S1110.v4);
	m.r[1] = qm_vec_select(d1, r1, QMCONST_S1110.v4);
	m.r[2] = qm_vec_select(d2, r2, QMCONST_S1110.v4);
	m.r[3] = QMCONST_UNIT_R3;
	return qm_mat4_tran(m);
}

/// @brief 보기 행렬을 만든다 (왼손 기준)
INLINE QmMat4 QM_VECTORCALL qm_mat4_lookat_lh(QmVec4 eye, QmVec4 at, QmVec4 up)
{
	QmVec4 dir = qm_vec_sub(at, eye);
	return qm_mat4_internal_look_to(eye, dir, up);
}

/// @brief 보기 행렬을 만든다 (오른손 기준)
INLINE QmMat4 QM_VECTORCALL qm_mat4_lookat_rh(QmVec4 eye, QmVec4 at, QmVec4 up)
{
	QmVec4 dir = qm_vec_sub(eye, at);
	return qm_mat4_internal_look_to(eye, dir, up);
}

/// @brief 투영 행렬을 만든다 (왼손 기준)
INLINE QmMat4 QM_VECTORCALL qm_mat4_perspective_lh(float fov, float aspect, float zn, float zf)
{
	float s, c;
	qm_sincosf(fov * 0.5f, &s, &c);
	float h = c / s;
	float w = h / aspect;
	float q = zf / (zf - zn);
	QmMat4 r =
	{
		._11 = w,
		._22 = h,
		._33 = q,
		._34 = 1.0f,
		._43 = -zn * q,
	};
	return r;
}

/// @brief 투영 행렬을 만든다 (오른손 기준)
INLINE QmMat4 QM_VECTORCALL qm_mat4_perspective_rh(float fov, float aspect, float zn, float zf)
{
	float s, c;
	qm_sincosf(fov * 0.5f, &s, &c);
	float h = c / s;
	float w = h / aspect;
	float q = zf / (zn - zf);
	QmMat4 r =
	{
		._11 = w,
		._22 = h,
		._33 = q,
		._34 = -1.0f,
		._43 = zn * q,
	};
	return r;
}

/// @brief 정규 행렬를 만든다 (왼손 기준)
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_lh(float width, float height, float zn, float zf)
{
	float q = 1.0f / (zf - zn);
	QmMat4 r =
	{
		._11 = 2.0f / width,
		._22 = 2.0f / height,
		._33 = q,
		._43 = -zn * q,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 정규 행렬을 만든다 (오른손 기준)
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_rh(float width, float height, float zn, float zf)
{
	float q = 1.0f / (zn - zf);
	QmMat4 r =
	{
		._11 = 2.0f / width,
		._22 = 2.0f / height,
		._33 = q,
		._43 = zn * q,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 사각형을 기준으로 정규 행렬을 만든다 (왼손 기준)
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_offcenter_lh(float left, float top, float right, float bottom, float zn, float zf)
{
	QmMat4 r =
	{
		._11 = 2.0f / (right - left),
		._22 = 2.0f / (top - bottom),
		._33 = 1.0f / (zf - zn),
		._41 = (left + right) / (left - right),
		._42 = (top + bottom) / (bottom - top),
		._43 = zn / (zn - zf),
		._44 = 1.0f,
	};
	return r;
}

/// @brief 사각형을 기준으로 정규 행렬을 만든다 (오른손 기준)
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_offcenter_rh(float left, float top, float right, float bottom, float zn, float zf)
{
	QmMat4 r =
	{
		._11 = 2.0f / (right - left),
		._22 = 2.0f / (top - bottom),
		._33 = 1.0f / (zn - zf),
		._41 = (left + right) / (left - right),
		._42 = (top + bottom) / (bottom - top),
		._43 = zn / (zn - zf),
		._44 = 1.0f,
	};
	return r;
}

/// @brief 뷰포트 행렬을 만든다
/// @param x,y 좌표
/// @param width,height 너비와 높이
INLINE QmMat4 QM_VECTORCALL qm_mat4_viewport(float x, float y, float width, float height)
{
	QmMat4 r =
	{
		._11 = width * 0.5f,
		._22 = height * -0.5f,
		._33 = 1.0f,
		._41 = x + width * 0.5f,
		._42 = y - height * -0.5f,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 단위 행렬인지 비교
/// @param m 비교할 행렬
/// @return 단위 행렬이면 참을 반환
INLINE bool QM_VECTORCALL qm_mat4_isu(QmMat4 m)
{
#if defined QM_USE_AVX
	QMSVEC r1 = _mm_cmpeq_ps(m.v[0], QMCONST_UNIT_R0.v);
	QMSVEC r2 = _mm_cmpeq_ps(m.v[1], QMCONST_UNIT_R1.v);
	QMSVEC r3 = _mm_cmpeq_ps(m.v[2], QMCONST_UNIT_R2.v);
	QMSVEC r4 = _mm_cmpeq_ps(m.v[3], QMCONST_UNIT_R3.v);
	r1 = _mm_and_ps(r1, r2);
	r3 = _mm_and_ps(r3, r4);
	r1 = _mm_and_ps(r1, r3);
	return (_mm_movemask_ps(r1) == 0x0f);
#elif defined QM_USE_NEON
	uint32x4_t r1 = vceqq_f32(m.v[0], QMCONST_UNIT_R0.v);
	uint32x4_t r2 = vceqq_f32(m.v[1], QMCONST_UNIT_R1.v);
	uint32x4_t r3 = vceqq_f32(m.v[2], QMCONST_UNIT_R2.v);
	uint32x4_t r4 = vceqq_f32(m.v[3], QMCONST_UNIT_R3.v);
	r1 = vandq_u32(r1, r3);
	r2 = vandq_u32(r2, r4);
	r1 = vandq_u32(r1, r2);
	uint8x8x2_t r = vzip_u8(vget_low_u8(vreinterpretq_u8_u32(r1)), vget_high_u8(vreinterpretq_u8_u32(r1)));
	uint16x4x2_t p = vzip_u16(vreinterpret_u16_u8(r.val[0]), vreinterpret_u16_u8(r.val[1]));
	uint u = vget_lane_u32(vreinterpret_u32_u16(p.val[1]), 1);
	return (u == 0xFFFFFFFFU);
#else
	const uint* p = (const uint*)&m.f[0];
	uint u1 = p[0] ^ 0x3F800000U;
	uint u0 = p[1];
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
// point 

/// @brief 점 설정
INLINE QmPoint qm_point(int x, int y)
{
	return (QmPoint) { { x, y } };
}

/// @brief 벡터로 점 설정
INLINE QmPoint qm_pointv(const QmVec2 v)
{
	return (QmPoint) { { (int)v.X, (int)v.Y } };
}

/// @brief 벡터로 점 설정
INLINE QmPoint qm_pointv4(const QmVec4 v)
{
	return (QmPoint) { { (int)v.X, (int)v.Y } };
}

/// @brief 점 초기화
INLINE QmPoint qm_point_zero(void)		// identify
{
	static const QmPoint zero = { { 0, 0 } };
	return zero;
}

/// @brief 점 대각값 설정 (모두 같은값으로 설정)
INLINE QmPoint qm_point_sp(const int diag)
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
INLINE QmPoint qm_point_mag(const QmPoint left, int right)
{
	return (QmPoint) { { left.X * right, left.Y * right } };
}

/// @brief 점 줄이기
INLINE QmPoint qm_point_abr(const QmPoint left, int right)
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
INLINE int qm_point_dot(const QmPoint left, const QmPoint right)
{
	return left.X * right.X + left.Y * right.Y;
}

/// @brief 점 길이의 제곱
INLINE int qm_point_len_sq(const QmPoint pt)
{
	return qm_point_dot(pt, pt);
}

/// @brief 두 점 거리의 제곱
INLINE int qm_point_dist_sq(const QmPoint left, const QmPoint right)
{
	QmPoint t = qm_point_sub(left, right);
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
INLINE QmSize qm_size(int width, int height)
{
	return (QmSize) { { width, height } };
}

/// @brief 사각형으로 크기를 설정한다
INLINE QmSize qm_size_rect(const QmRect rt)
{
	return (QmSize) { { rt.Right - rt.Left, rt.Bottom - rt.Top } };
}

/// @brief 사이즈 대각값 설정 (모두 같은값으로 설정)
INLINE QmSize qm_size_sp(const int diag)
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
INLINE QmSize qm_size_mag(const QmSize left, int right)
{
	return (QmSize) { { left.Width * right, left.Height * right } };
}

/// @brief 사이즈 줄이기
INLINE QmSize qm_size_abr(const QmSize left, int right)
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
INLINE int qm_size_len_sq(const QmSize s)
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
INLINE QmPoint qm_size_locate_center(const QmSize s, int width, int height)
{
	return qm_point((s.Width - width) / 2, (s.Height - height) / 2);
}

/// @brief 대각선 DPI를 구한다
INLINE float qm_size_calc_dpi(const QmSize pt, float horizontal, float vertical)
{
	float dsq = horizontal * horizontal + vertical + vertical;
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
INLINE QmRect qm_rect(int left, int top, int right, int bottom)
{
	return (QmRect) { { left, top, right, bottom } };
}

/// @brief 사각형을 좌표와 크기로 설정한다
INLINE QmRect qm_rect_size(int x, int y, int width, int height)
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
INLINE QmRect qm_rect_sp(const int diag)
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
INLINE QmRect qm_rect_mag(const QmRect left, int right)
{
	return (QmRect) { { left.Left * right, left.Top * right, left.Right * right, left.Bottom * right } };
}

/// @brief 사각형 줄이기
INLINE QmRect qm_rect_abr(const QmRect left, int right)
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
INLINE QmRect qm_rect_inflate(const QmRect rt, int left, int top, int right, int bottom)
{
	return (QmRect) { { rt.Left - left, rt.Top - top, rt.Right + right, rt.Bottom + bottom } };
}

/// @brief 사각형 크기를 줄인다 (요소가 양수일 경우)
INLINE QmRect qm_rect_deflate(const QmRect rt, int left, int top, int right, int bottom)
{
	return (QmRect) { { rt.Left + left, rt.Top + top, rt.Right - right, rt.Bottom - bottom } };
}

/// @brief 사각형을 움직인다 (요소가 양수일 경우 일괄 덧셈)
INLINE QmRect qm_rect_offset(const QmRect rt, int left, int top, int right, int bottom)
{
	return (QmRect) { { rt.Left + left, rt.Top + top, rt.Right + right, rt.Bottom + bottom } };
}

/// @brief 사각형을 움직인다
INLINE QmRect qm_rect_move(const QmRect rt, int left, int top)
{
	int dx = left - rt.Left;
	int dy = top - rt.Top;
	return (QmRect) { { rt.Left + dx, rt.Top + dy, rt.Right + dx, rt.Bottom + dy } };
}

/// @brief 사각형의 크기를 재설정한다
INLINE QmRect qm_rect_set_size(const QmRect rt, int width, int height)
{
	return (QmRect) { { rt.Left, rt.Top, rt.Left + width, rt.Top + height } };
}

/// @brief 사각형의 너비를 얻는다
INLINE int qm_rect_get_width(const QmRect rt)
{
	return rt.Right - rt.Left;
}

/// @brief 사각형의 높이를 얻는다
INLINE int qm_rect_get_height(const QmRect rt)
{
	return rt.Bottom - rt.Top;
}

/// @brief 좌표가 사각형 안에 있는지 조사한다
INLINE bool qm_rect_in(const QmRect rt, const int x, const int y)
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
INLINE void qm_rect_rotate(_In_ QmRect rt, _In_ float angle, _Out_ QmVec2* tl, _Out_ QmVec2* tr, _Out_ QmVec2* bl, _Out_ QmVec2* br)
{
	qn_verify(tl != NULL && tr != NULL && bl != NULL && br != NULL);
	float s, c;
	qm_sincosf(angle, &s, &c);
	QmVec2 dt = qm_vec2((float)(rt.Right - rt.Left) * 0.5f, (float)(rt.Bottom - rt.Top) * 0.5f);
	QmVec2 ot = qm_vec2((float)rt.Left + dt.X, (float)rt.Top + dt.Y);
	QmVec2 vo[4], vi[4] = { {{-dt.X, -dt.Y}}, {{dt.X, -dt.Y}}, {{-dt.X, dt.Y}}, {{dt.X, dt.Y}} };
	for (int i = 0; i < 4; i++)
	{
		vo[i].X = vi[i].X * c - vi[i].Y * s;
		vo[i].Y = vi[i].X * s + vi[i].Y * c;
		vo[i] = qm_vec2_add(vo[i], ot);
	}
	*tl = vo[0]; *tr = vo[1]; *bl = vo[2]; *br = vo[3];
}

#ifdef _WINDEF_
/// @brief 윈도우 RECT에서
INLINE QmRect qm_rect_RECT(RECT rt)
{
	return (QmRect) { { rt.left, rt.top, rt.right, rt.bottom } };
}

/// @brief 윈도우 RECT로
INLINE RECT qm_rect_to_RECT(const QmRect rt)
{
	return (RECT) { rt.Left, rt.Top, rt.Right, rt.Bottom };
}
#endif


//////////////////////////////////////////////////////////////////////////
// generic

/// @brief (제네릭) 덧셈
#define qm_add(l,r)		_Generic((l),\
	QmVec2: qm_vec2_add,\
	QmVec4: qm_vec_add,\
	QmMat4: qm_mat4_add,\
	QmPoint: qm_point_add,\
	QmSize: qm_size_add,\
	QmRect: qm_rect_add)(l,r)
/// @brief (제네릭) 뺄셈
#define qm_sub(l,r)		_Generic((l),\
	QmVec2: qm_vec2_sub,\
	QmVec4: qm_vec_sub,\
	QmMat4: qm_mat4_sub,\
	QmPoint: qm_point_sub,\
	QmSize: qm_size_sub,\
	QmRect: qm_rect_sub)(l,r)
/// @brief (제네릭) 확대
#define qm_mag(i,s)		_Generic((i),\
	QmVec2: qm_vec2_mag,\
	QmVec4: qm_vec_mag,\
	QmMat4: qm_mat4_mag,\
	QmPoint: qm_point_mag,\
	QmSize: qm_size_mag,\
	QmRect: qm_rect_mag)(i,s)
/// @brief (제네릭) 줄이기
#define qm_abr(i,s)		_Generic((i),\
	QmVec2: qm_vec2_abr,\
	QmVec4: qm_vec_abr,\
	QmMat4: qm_mat4_abr,\
	QmPoint: qm_point_abr,\
	QmSize: qm_size_abr,\
	QmRect: qm_rect_abr)(i,s)
/// @brief (제네릭) 최소값
#define qm_min(l,r)		_Generic((l),\
	float: qm_minf,\
	QmVec2: qm_vec2_min,\
	QmVec4: qm_vec_min,\
	QmPoint: qm_point_min,\
	QmSize: qm_size_min,\
	QmRect: qm_rect_min)(l,r)
/// @brief (제네릭) 최대값
#define qm_max(l,r)		_Generic((l),\
	float: qm_maxf,\
	QmVec2: qm_vec2_max,\
	QmVec4: qm_vec_max,\
	QmPoint: qm_point_max,\
	QmSize: qm_size_max,\
	QmRect: qm_rect_max)(l,r)
/// @brief (제네릭) 같나 비교
#define qm_eq(l,r)		_Generic((l),\
	float: qm_eqf,\
	QmVec2: qm_vec2_eq,\
	QmVec4: qm_vec_eq,\
	QmPoint: qm_point_eq,\
	QmSize: qm_size_eq,\
	QmRect: qm_rect_eq)(l,r)
/// @brief (제네릭) 선형 보간
#define qm_lerp(l,r,s)	_Generic((l),\
	float: qm_lerpf,\
	QmVec2: qm_vec2_lerp,\
	QmVec4: qm_vec_lerp)(l,r,s)
/// @brief 절대값
#define qm_abs(x)		_Generic((x),\
	float: qm_absf,\
	int: qm_absi)(x)
/// @brief 범위로 자르기
#define qm_clamp(v,n,x)	_Generic((v),\
	float: qm_clampf,\
	int: qm_clampi)(v,n,x)

QN_EXTC_END

#ifdef __clang__
#pragma clang diagnotics pop
#endif

// set reset diagnosis ivt add sub  mag mul div min max eq isi
// dot cross len_sq len dist_sq dist
// ^INLINE (.*)$ => $0;\n

