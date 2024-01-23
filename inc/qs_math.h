//
// QsLib [MATH Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: UNLICENSED
//
// 본 라이브러리의 일부는 다음과 같은 라이브러리를 소스의 일부로 포함하고 있습니다.
// HandmadeMath (https://github.com/HandmadeMath/HandmadeMath)
// DirectXMath (https://github.com/microsoft/DirectXMath)
// 
// 그 밖에 오래되어 출처를 알 수 없는 코드가 포함되어 있습니다.
//

// ReSharper disable CppLocalVariableMayBeConst
#pragma once
#define __QS_MATH__

#ifndef __QS_QN__
#error include "qs_qn.h" first
#endif

#ifdef __GNUC__
#pragma GCC diagnotics push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define QS_NO_SIMD

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

#if (defined _MSC_VER || defined __clang__) && !defined _M_ARM && !defined _M_ARM64
#define QM_VECTORCALL	__vectorcall
#elif defined __GNUC__
#define QM_VECTORCALL
#else
#define QM_VECTORCALL	__fastcall
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
#define QM_PI			3.14159265358979323846f								/// @brief 원주율
#define QM_PI2			6.28318530717958647692f								/// @brief 원주율 두배
#define QM_PI_H			1.57079632679489661923f								/// @brief 원주울의 반
#define QM_PI_Q			0.78539816339744830961f								/// @brief 원주율의 반의 반
#define QM_INV_PI		0.31830988618379067154f								/// @brief 원주율의 역수
#define QM_INV_PI_H		0.15915494309189533577f								/// @brief 원주율 두배의 역수
#define QM_SQRT2		1.41421356237309504880f								/// @brief 2의 제곱근
#define QM_SQRT_H		0.70710678118654752440f								/// @brief 2의 제곱근의 반
#define QM_TAU			6.28318530717958647692f								/// @brief 타우, 원주율의 두배
#define QM_TAU2			12.56637061435917295384f							/// @brief 타우 두배
#define QM_TAU_H		3.14159265358979323846f								/// @brief 타우의 반
#define QM_TAU_Q		1.57079632679489661923f								/// @brief 타우의 반의 반
#define QM_INV_TAU		0.15915494309189533577f								/// @brief 타우의 역수
#define QM_INV_TAU_H	0.07957747154594766788f								/// @brief 타우의 두배의 역수
#define QM_RAD2DEG		(180.0f/QM_PI)
#define QM_DEG2RAD		(QM_PI/180.0f)


//////////////////////////////////////////////////////////////////////////
// random

/// @brief 랜덤 시드
QSAPI void qm_srand(nuint seed);

/// @brief 랜덤
QSAPI nuint qm_rand(void);

/// @brief 랜덤 실수
QSAPI float qm_randf(void);

/// @brief 랜덤 실수
QSAPI double qm_randd(void);


//////////////////////////////////////////////////////////////////////////
// types

// SIMD 벡터
#if defined QM_USE_AVX
typedef __m128			QMVECTOR;
#elif defined QM_USE_NEON
typedef float32x4_t		QMVECTOR;
#else
typedef union QMVECTOR
{
	float f[4];
	int i[4];
	uint u[4];
} QMVECTOR;
#endif

/// @brief 벡터2
typedef union QMVEC2
{
	float f[2];
	struct
	{
		float X, Y;
	};
} QmVec2, QmPointF;

/// @brief 벡터4 / 사원수 / 평면
typedef union QMVEC4
{
	float f[4];
	QMVECTOR v;
	struct
	{
		float X, Y, Z, W;
	};
	struct
	{
		float A, B, C, D;
	};
} QmVec4, QmQuat, QmPlane;

/// @brief 행렬4x4
typedef union QMMAT4
{
	float f[16];
	float m[4][4];
	QMVECTOR v[4];
	struct
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;
	};
} QmMat4;

/// @brief 색깔
typedef union QMCOLOR
{
	float f[4];
	QMVECTOR v;
	struct
	{
		float R, G, B, A;
	};
} QmColor;

/// @brief 정수형 색깔
typedef union QMKOLOR
{
	byte b[4];
	uint U;
	struct
	{
		byte B, G, R, A;
	};
} QmKolor;

/// @brief 정수형 벡터2 (포인트)
typedef union QMVECI2
{
	int i[2];
	struct
	{
		int X, Y;
	};
} QmVecI2, QmPoint;

/// @brief 정수형 벡터4
typedef union QMVECI4
{
	int i[4];
	QMVECTOR v;
	struct
	{
		int X, Y, Z, W;
	};
} QmVecI4;

/// @brief 부호없는 정수형 벡터4
typedef union QMVECU4
{
	uint u[4];
	QMVECTOR v;
	struct
	{
		uint X, Y, Z, W;
	};
} QmVecU4;

/// @brief 사이즈
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

/// @brief 하프 벡터2
typedef union QMVECH2
{
	halffloat h[2];
	struct
	{
		halffloat X, Y;
	};
} QmVecH2;

/// @brief 하프 벡터4
typedef union QMVECH4
{
	halffloat h[4];
	struct
	{
		halffloat X, Y, Z, W;
	};
} QmVecH4;

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

/// @brief FLOAT2X2
typedef union QMFLOAT2X2
{
	struct
	{
		float _11, _12;
		float _21, _22;
	};
	float m[2][2];
} QmFloat2x2;

/// @brief FLOAT3X3
typedef union QMFLOAT3X3
{
	struct
	{
		float _11, _12, _13;
		float _21, _22, _23;
		float _31, _32, _33;
	};
	float m[3][3];
} QmFloat3x3;

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
// integer & float

/// @brief 두 실수의 최대값
INLINE int qm_maxi(int a, int b)
{
	return QN_MAX(a, b);
}

/// @brief 두 실수의 최소값
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

/// @brief 실수를 정수부만 뽑기
INLINE float qm_fractf(float f)
{
	return f - floorf(f);
}

/// @brief 사인과 코사인을 동시에 계산
INLINE void qm_sincosf(float v, float* s, float* c)
{
#ifdef QM_SINCOSF
	QM_SINCOSF(v, s, c);
#elif true
	* s = sinf(v);
	*c = cosf(v);
#endif
}

/// @brief 제곱근
INLINE float qm_sqrtf(float f)
{
#if defined QM_USE_AVX
	QMVECTOR i = _mm_set_ss(f);
	QMVECTOR o = _mm_sqrt_ss(i);
	return _mm_cvtss_f32(o);
#elif defined QM_USE_NEON
	QMVECTOR i = vdupq_n_f32(f);
	QMVECTOR o = vsqrtq_f32(f);
	return vgetq_lane_f32(o, 0);
#else
	return sqrtf(f);
#endif
}

/// @brief 1을 나눈 제곱근
INLINE float qm_inv_sqrtf(float f)
{
	return 1.0f / qm_sqrtf(f);
}

/// @brief 각도를 호도로 변환
INLINE float qm_deg2rad(float d)
{
	return d * QM_DEG2RAD;
}

/// @brief 호도를 각도로 변환
INLINE float qm_rad2deg(float r)
{
	return r * QM_RAD2DEG;
}

#ifdef _PREFAST_
#pragma prefast(push)
#pragma prefast(disable : 26015 26019, "PREfast noise: Esp:1307" )
#endif

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
	float32x4_t f = vdupq_n_f32(v);
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

#ifdef _PREFAST_
#pragma prefast(pop)
#endif


//////////////////////////////////////////////////////////////////////////
// function

INLINE QmVec2 qm_vec2(float x, float y);
INLINE QmVec2 qm_vec2_zero(void);
INLINE QmVec2 qm_vec2_diag(float diag);
INLINE QmVec2 qm_vec2_neg(QmVec2 v);
INLINE QmVec2 qm_vec2_add(QmVec2 left, QmVec2 right);
INLINE QmVec2 qm_vec2_sub(QmVec2 left, QmVec2 right);
INLINE QmVec2 qm_vec2_mag(QmVec2 left, float right);
INLINE QmVec2 qm_vec2_abr(QmVec2 left, float right);
INLINE QmVec2 qm_vec2_mul(QmVec2 left, QmVec2 right);
INLINE QmVec2 qm_vec2_div(QmVec2 left, QmVec2 right);
INLINE QmVec2 qm_vec2_min(QmVec2 left, QmVec2 right);
INLINE QmVec2 qm_vec2_max(QmVec2 left, QmVec2 right);
INLINE QmVec2 qm_vec2_norm(QmVec2 v);
INLINE QmVec2 qm_vec2_cross(QmVec2 left, QmVec2 right);
INLINE QmVec2 qm_vec2_interpolate(QmVec2 left, QmVec2 right, float scale);
INLINE QmVec2 qm_vec2_lerp(QmVec2 left, QmVec2 right, float scale);
INLINE float qm_vec2_dot(QmVec2 left, QmVec2 right);
INLINE float qm_vec2_len_sq(QmVec2 v);
INLINE float qm_vec2_dist_sq(QmVec2 left, QmVec2 right);
INLINE float qm_vec2_len(QmVec2 v);
INLINE float qm_vec2_dist(QmVec2 left, QmVec2 right);
INLINE bool qm_vec2_eq(QmVec2 left, QmVec2 right);
INLINE bool qm_vec2_isz(QmVec2 v);

INLINE QmVec4 QM_VECTORCALL qm_vec4(float x, float y, float z, float w);
INLINE QmVec4 QM_VECTORCALL qm_vec4_zero(void);
INLINE QmVec4 QM_VECTORCALL qm_vec4_diag(float diag);
INLINE QmVec4 QM_VECTORCALL qm_vec4_neg(QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec4_add(QmVec4 left, QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec4_sub(QmVec4 left, QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec4_mag(QmVec4 left, float right);
INLINE QmVec4 QM_VECTORCALL qm_vec4_abr(QmVec4 left, float right);
INLINE QmVec4 QM_VECTORCALL qm_vec4_mul(QmVec4 left, QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec4_div(QmVec4 left, QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec4_min(QmVec4 left, QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec4_max(QmVec4 left, QmVec4 right);
INLINE QmVec4 QM_VECTORCALL qm_vec4_norm(QmVec4 v);
INLINE QmVec4 QM_VECTORCALL qm_vec4_cross(QmVec4 v1, QmVec4 v2, QmVec4 v3);
INLINE QmVec4 QM_VECTORCALL qm_vec4_interpolate(QmVec4 left, QmVec4 right, float scale);
INLINE QmVec4 QM_VECTORCALL qm_vec4_lerp(QmVec4 left, QmVec4 right, float scale);
INLINE QmVec4 QM_VECTORCALL qm_vec4_trfm(QmVec4 v, QmMat4 trfm);
INLINE float QM_VECTORCALL qm_vec4_dot(QmVec4 left, QmVec4 right);
INLINE float QM_VECTORCALL qm_vec4_len_sq(QmVec4 v);
INLINE float QM_VECTORCALL qm_vec4_dist_sq(QmVec4 left, QmVec4 right);
INLINE float QM_VECTORCALL qm_vec4_len(QmVec4 v);
INLINE float QM_VECTORCALL qm_vec4_dist(QmVec4 left, QmVec4 right);
INLINE bool QM_VECTORCALL qm_vec4_eq(QmVec4 left, QmVec4 right);
INLINE bool QM_VECTORCALL qm_vec4_isz(QmVec4 v);

INLINE QmQuat QM_VECTORCALL qm_quat_unit(void);
INLINE QmQuat QM_VECTORCALL qm_quat_mul(QmQuat left, QmQuat right);
INLINE QmQuat QM_VECTORCALL qm_quat_cjg(QmQuat q);
INLINE QmQuat QM_VECTORCALL qm_quat_inv(QmQuat q);
INLINE QmQuat QM_VECTORCALL qm_quat_lerp(QmQuat left, QmQuat right, float scale);
INLINE QmQuat QM_VECTORCALL qm_quat_slerp(QmQuat left, QmQuat right, float scale);
INLINE QmQuat QM_VECTORCALL qm_quat_mat(QmMat4 rot);
INLINE QmQuat QM_VECTORCALL qm_quat_axis_vec(QmVec4 v, float angle);
INLINE QmQuat QM_VECTORCALL qm_quat_vec(QmVec4 rot);
INLINE QmQuat QM_VECTORCALL qm_quat_x(float rot);
INLINE QmQuat QM_VECTORCALL qm_quat_y(float rot);
INLINE QmQuat QM_VECTORCALL qm_quat_z(float rot);
INLINE QmQuat QM_VECTORCALL qm_quat_exp(QmQuat q);
INLINE QmVec4 QM_VECTORCALL qm_quat_ln(QmQuat q);
INLINE bool QM_VECTORCALL qm_quat_isu(QmQuat q);

INLINE QmPlane QM_VECTORCALL qm_planev(QmVec4 v, float d);
INLINE QmPlane QM_VECTORCALL qm_planevv(QmVec4 v, QmVec4 n);
INLINE QmPlane QM_VECTORCALL q4_planevvv(QmVec4 v1, QmVec4 v2, QmVec4 v3);
INLINE QmPlane QM_VECTORCALL qm_plane_unit(void);
INLINE QmPlane QM_VECTORCALL qm_plane_norm(QmPlane p);
INLINE QmPlane QM_VECTORCALL qm_plane_rev_norm(QmPlane p);
INLINE QmPlane QM_VECTORCALL qm_plane_trfm(QmPlane plane, QmMat4 trfm);
INLINE float QM_VECTORCALL qm_plane_dot_coord(QmPlane p, QmVec4 v);
INLINE float QM_VECTORCALL qm_plane_dot_normal(QmPlane p, QmVec4 v);
INLINE float QM_VECTORCALL qm_plane_dist(QmPlane p, QmVec4 v);
INLINE float QM_VECTORCALL qm_plane_distance_line(QmPlane p, QmVec4 begin, QmVec4 end);
INLINE int QM_VECTORCALL qm_plane_rel_point(QmPlane p, QmVec4 v);
INLINE bool QM_VECTORCALL qm_plane_intersect(QmPlane p, QmPlane o, QmVec4* loc, QmVec4* dir);
INLINE bool QM_VECTORCALL qm_plane_intersect_line(QmPlane plane, QmVec4 loc, QmVec4 dir, QmVec4* pv);
INLINE bool QM_VECTORCALL qm_plane_intersect_planes(QmPlane plane, QmPlane other1, QmPlane other2, QmVec4 * pv);
INLINE bool QM_VECTORCALL qm_plane_intersect_between_point(QmPlane plane, QmVec4 v1, QmVec4 v2, QmVec4* pv);
INLINE bool QM_VECTORCALL qm_plane_isu(QmPlane p);

INLINE QmMat4 QM_VECTORCALL qm_mat4_unit(void);
INLINE QmMat4 QM_VECTORCALL qm_mat4_zero(void);
INLINE QmMat4 QM_VECTORCALL qm_mat4_diag(float diag);
INLINE QmMat4 QM_VECTORCALL qm_mat4_tran(QmMat4 m);
INLINE QmMat4 QM_VECTORCALL qm_mat4_add(QmMat4 left, QmMat4 right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_sub(QmMat4 left, QmMat4 right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_mag(QmMat4 m, float scale);
INLINE QmMat4 QM_VECTORCALL qm_mat4_abr(QmMat4 m, float scale);
INLINE QmMat4 QM_VECTORCALL qm_mat4_mul(QmMat4 left, QmMat4 right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_inv(QmMat4 m);
INLINE float QM_VECTORCALL qm_mat4_det(QmMat4 m);
INLINE QmMat4 QM_VECTORCALL qm_mat4_tmul(QmMat4 left, QmMat4 right);
INLINE QmMat4 QM_VECTORCALL qm_mat4_tolook(QMVECTOR eye, QMVECTOR dir, QMVECTOR up);
INLINE QmMat4 QM_VECTORCALL qm_mat4_lookat_lh(QmVec4 eye, QmVec4 at, QmVec4 up);
INLINE QmMat4 QM_VECTORCALL qm_mat4_lookat_rh(QmVec4 eye, QmVec4 at, QmVec4 up);
INLINE QmMat4 QM_VECTORCALL qm_mat4_perspective_lh(float fov, float aspect, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_perspective_rh(float fov, float aspect, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_lh(float width, float height, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_rh(float width, float height, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_offcenter_lh(float left, float top, float right, float bottom, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_ortho_offcenter_rh(float left, float top, float right, float bottom, float zn, float zf);
INLINE QmMat4 QM_VECTORCALL qm_mat4_viewport(float x, float y, float width, float height);
INLINE QmMat4 QM_VECTORCALL qm_mat4_scl(float x, float y, float z);
INLINE QmMat4 QM_VECTORCALL qm_mat4_scl_vec(QmVec4 v);
INLINE QmMat4 QM_VECTORCALL qm_mat4_loc(float x, float y, float z);
INLINE QmMat4 QM_VECTORCALL qm_mat4_loc_vec(QmVec4 v);
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot(float angle, QmVec4 axis);
INLINE QmMat4 QM_VECTORCALL qm_mat4_vec(QmVec4 rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_quat(QmQuat rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_x(float rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_y(float rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_z(float rot);
INLINE QmMat4 QM_VECTORCALL qm_mat4_shadow(QmVec4 light, QmPlane plane);
INLINE QmMat4 QM_VECTORCALL qm_mat4_affine(QmVec4 * scl, QmVec4* rotcenter, QmQuat* rot, QmVec4* loc);
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm_loc_scl(QmMat4 m, QmVec4 loc, QmVec4* scl);
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm(QmVec4 loc, QmQuat rot, QmVec4* scl);
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm_vec(QmVec4 loc, QmVec4 rot, QmVec4* scl);
INLINE bool QM_VECTORCALL qm_mat4_isu(QmMat4 m);

INLINE QmColor QM_VECTORCALL qm_color(float r, float g, float b, float a);
INLINE QmColor QM_VECTORCALL qm_coloru(uint value);
INLINE QmColor QM_VECTORCALL qm_colork(QmKolor cu);
INLINE QmColor QM_VECTORCALL qm_color_unit(void);
INLINE QmColor QM_VECTORCALL qm_color_diag(float diag, float alpha);
INLINE QmColor QM_VECTORCALL qm_color_neg(QmColor c);
INLINE QmColor QM_VECTORCALL qm_color_mod(QmColor left, QmColor right);
INLINE QmColor QM_VECTORCALL qm_color_interpolate(QmColor left, QmColor right, float scale);
INLINE QmColor QM_VECTORCALL qm_color_lerp(QmColor left, QmColor right, float scale);
INLINE QmColor QM_VECTORCALL qm_color_contrast(QmColor c, float scale);
INLINE QmColor QM_VECTORCALL qm_color_saturation(QmColor c, float scale);
INLINE uint QM_VECTORCALL qm_color_to_uint(QmColor c);
INLINE uint QM_VECTORCALL qm_color_to_uint_check(QmColor c);
INLINE bool QM_VECTORCALL qm_color_eq(QmColor left, QmColor right);

INLINE QmKolor qm_kolor(const byte r, const byte g, const byte b, const byte a);
INLINE QmKolor qm_kolorf(float r, float g, float b, float a);
INLINE QmKolor qm_koloru(const uint value);
INLINE QmKolor QM_VECTORCALL qm_kolorc(QmColor cr);
INLINE QmKolor qm_kolor_add(QmKolor left, QmKolor right);
INLINE QmKolor qm_kolor_sub(QmKolor left, QmKolor right);
INLINE QmKolor qm_kolor_mag(QmKolor left, float scale);
INLINE QmKolor qm_kolor_neg(QmKolor c);
INLINE QmKolor qm_kolor_mul(QmKolor left, QmKolor right);
INLINE QmKolor qm_kolor_div(QmKolor left, QmKolor right);
INLINE QmKolor qm_kolor_interpolate(QmKolor left, QmKolor right, float scale);
INLINE QmKolor qm_kolor_lerp(QmKolor left, QmKolor right, float scale);
INLINE bool qm_kolor_eq(QmKolor left, QmKolor right);

INLINE QmPoint qm_point(int x, int y);
INLINE QmPoint qm_point_zero(void);
INLINE QmPoint qm_point_diag(const int diag);
INLINE QmPoint qm_point_neg(QmPoint p);
INLINE QmPoint qm_point_add(QmPoint left, QmPoint right);
INLINE QmPoint qm_point_sub(QmPoint left, QmPoint right);
INLINE QmPoint qm_point_mag(QmPoint left, const int right);
INLINE QmPoint qm_point_mul(QmPoint left, QmPoint right);
INLINE QmPoint qm_point_div(QmPoint left, QmPoint right);
INLINE QmPoint qm_point_min(QmPoint left, QmPoint right);
INLINE QmPoint qm_point_max(QmPoint left, QmPoint right);
INLINE QmPoint qm_point_cross(QmPoint left, QmPoint right);
INLINE int qm_point_dot(QmPoint left, QmPoint right);
INLINE int qm_point_len_sq(QmPoint pt);
INLINE int qm_point_dist_sq(QmPoint left, QmPoint right);
INLINE float qm_point_len(QmPoint pt);
INLINE float qm_point_dist(QmPoint left, QmPoint right);
INLINE bool qm_point_eq(QmPoint left, QmPoint right);
INLINE bool qm_point_isz(QmPoint pt);

INLINE QmSize qm_size(int width, int height);
INLINE QmSize qm_size_rect(QmRect rt);
INLINE QmSize qm_size_zero(void);
INLINE QmSize qm_size_diag(const int diag);
INLINE QmSize qm_size_add(QmSize left, QmSize right);
INLINE QmSize qm_size_sub(QmSize left, QmSize right);
INLINE QmSize qm_size_mag(QmSize left, const int right);
INLINE QmSize qm_size_mul(QmSize left, QmSize right);
INLINE QmSize qm_size_div(QmSize left, QmSize right);
INLINE QmSize qm_size_min(QmSize left, QmSize right);
INLINE QmSize qm_size_max(QmSize left, QmSize right);
INLINE int qm_size_len_sq(QmSize s);
INLINE float qm_size_len(QmSize v);
INLINE float qm_size_aspect(QmSize s);
INLINE float qm_size_diag_dpi(QmSize pt, float horizontal, float vertical);
INLINE bool qm_size_eq(QmSize left, QmSize right);
INLINE bool qm_size_isz(QmSize s);

INLINE QmRect qm_rect(const int left, const int top, const int right, const int bottom);
INLINE QmRect qm_rect_size(const int x, const int y, const int width, const int height);
INLINE QmRect qm_rect_pos_size(QmPoint pos, QmSize size);
INLINE QmRect qm_rect_zero(void);
INLINE QmRect qm_rect_diag(const int diag);
INLINE QmRect qm_rect_add(QmRect left, QmRect right);
INLINE QmRect qm_rect_sub(QmRect left, QmRect right);
INLINE QmRect qm_rect_mag(QmRect left, const int right);
INLINE QmRect qm_rect_min(QmRect left, QmRect right);
INLINE QmRect qm_rect_max(QmRect left, QmRect right);
INLINE QmRect qm_rect_inflate(QmRect rt, const int left, const int top, const int right, const int bottom);
INLINE QmRect qm_rect_deflate(QmRect rt, const int left, const int top, const int right, const int bottom);
INLINE QmRect qm_rect_offset(QmRect rt, const int left, const int top, const int right, const int bottom);
INLINE QmRect qm_rect_move(QmRect rt, const int left, const int top);
INLINE QmRect qm_rect_resize(QmRect rt, const int width, const int height);
INLINE int qm_rect_width(QmRect rt);
INLINE int qm_rect_height(QmRect rt);
INLINE bool qm_rect_in(QmRect rt, const int x, const int y);
INLINE bool qm_rect_include(QmRect dest, QmRect target);
INLINE bool qm_rect_intersect(QmRect r1, QmRect r2, QmRect * p);
INLINE bool qm_rect_eq(QmRect left, QmRect right);
INLINE bool qm_rect_isz(QmRect pv);
#ifdef _WINDEF_
INLINE QmRect qm_rect_RECT(RECT rt);
INLINE RECT qm_rect_to_RECT(QmRect rt);
#endif

INLINE QmVecH2 qm_vec2h(float x, float y);
INLINE QmVecH4 qm_vec4h(float x, float y, float z, float w);


//////////////////////////////////////////////////////////////////////////
// support data & function + QMVECTOR

// AVX2
#ifdef QM_USE_AVX2
#define _MM_FMADD_PS(a,b,c)		_mm_fmadd_ps((a),(b),(c))
#define _MM_FNMADD_PS(a,b,c)	_mm_fnmadd_ps((a),(b),(c))
#else
#define _MM_FMADD_PS(a,b,c)		_mm_add_ps(_mm_mul_ps((a),(b)),(c))
#define _MM_FNMADD_PS(a,b,c)	_mm_sub_ps(c,_mm_mul_ps((a),(b)))
#endif

QN_CONST_ANY QmVec4 QMC_ZERO = { { 0.0f, 0.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMC_ONE_V4 = { { 1.0f, 1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMC_ONE_V3 = { { 1.0f, 1.0f, 1.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMC_UNIT_R0 = { { 1.0f, 0.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMC_UNIT_R1 = { { 0.0f, 1.0f, 0.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMC_UNIT_R2 = { { 0.0f, 0.0f, 1.0f, 0.0f } };
QN_CONST_ANY QmVec4 QMC_UNIT_R3 = { { 0.0f, 0.0f, 0.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMC_UNIT_N3 = { { 0.0f, 0.0f, 0.0f, -1.0f } };
QN_CONST_ANY QmVec4 QMC_NEG = { { -1.0f, -1.0f, -1.0f, -1.0f } };
QN_CONST_ANY QmVec4 QMC_NEG_X = { { -1.0f, 1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMC_NEG_Y = { { 1.0f, -1.0f, 1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMC_NEG_Z = { { 1.0f, 1.0f, -1.0f, 1.0f } };
QN_CONST_ANY QmVec4 QMC_NEG_W = { { 1.0f, 1.0f, 1.0f, -1.0f } };
QN_CONST_ANY QmVec4 QMC_EPSILON = { { FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON } };
QN_CONST_ANY QmVec4 QMC_NFRAC = { { 8388608.0f, 8388608.0f, 8388608.0f, 8388608.0f } };

QN_CONST_ANY QmVecU4 QMC_INF = { { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 } };
QN_CONST_ANY QmVecU4 QMC_NAN = { { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 } };
QN_CONST_ANY QmVecU4 QMC_NEG_V3 = { { 0x80000000, 0x80000000, 0x80000000, 0x00000000 } };
QN_CONST_ANY QmVecU4 QMC_NEG_V4 = { { 0x80000000, 0x80000000, 0x80000000, 0x80000000 } };
QN_CONST_ANY QmVecU4 QMC_MASK_V3 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } };
QN_CONST_ANY QmVecU4 QMC_S1000 = { { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 } };
QN_CONST_ANY QmVecU4 QMC_S1100 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 } };
QN_CONST_ANY QmVecU4 QMC_S1110 = { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 } };
QN_CONST_ANY QmVecU4 QMC_S1011 = { { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF } };

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SELECT(QMVECTOR a, QMVECTOR b, QMVECTOR mask)
{
#if defined QM_USE_AVX
	QMVECTOR t1 = _mm_andnot_ps(mask, a);
	QMVECTOR t2 = _mm_and_ps(b, mask);
	return _mm_or_ps(t1, t2);
#elif defined QM_USE_NEON
	return vbslq_f32(vreinterpretq_u32_f32(mask), b, a);
#else
	QMVECTOR v;
	v.u[0] = (a.u[0] & ~mask.u[0]) | (b.u[0] & mask.u[0]);
	v.u[1] = (a.u[1] & ~mask.u[1]) | (b.u[1] & mask.u[1]);
	v.u[2] = (a.u[2] & ~mask.u[2]) | (b.u[2] & mask.u[2]);
	v.u[3] = (a.u[3] & ~mask.u[3]) | (b.u[3] & mask.u[3]);
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SELECT_CTRL(uint i0, uint i1, uint i2, uint i3)
{
#if defined QM_USE_AVX
	__m128i l = _mm_set_epi32((int)i3, (int)i2, (int)i1, (int)i0);
	__m128i r = _mm_castps_si128(QMC_ZERO.v);
	return _mm_castsi128_ps(_mm_cmpgt_epi32(l, r));
#elif defined QM_USE_NEON
	int32x2_t l = vcreate_s32(((ullong)(i0)) | (((ullong)(i1)) << 32));
	int32x2_t r = vcreate_s32(((ullong)(i2)) | (((ullong)(i3)) << 32));
	return vreinterpretq_f32_u32(vcltq_s32(vcombine_s32(l, r), vreinterpretq_s32_f32(QMC_ZERO.v)));
#else
	static const uint e[2] = { 0x00000000, 0xFFFFFFFF };
	QMVECTOR v;
	v.u[0] = e[i0];
	v.u[1] = e[i1];
	v.u[2] = e[i2];
	v.u[3] = e[i3];
	return v;
#endif
}

//
INLINE float QM_VECTORCALL QMVECTOR_GET_X(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(a);
#elif defined QM_USE_NEON
	return vgetq_lane_f32(a, 0);
#else
	return a.f[0];
#endif
}

//
INLINE float QM_VECTORCALL QMVECTOR_GET_Y(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_permute_ps(a, _MM_SHUFFLE(1, 1, 1, 1)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(a, 1);
#else
	return a.f[1];
#endif
}

//
INLINE float QM_VECTORCALL QMVECTOR_GET_Z(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_permute_ps(a, _MM_SHUFFLE(2, 2, 2, 2)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(a, 2);
#else
	return a.f[2];
#endif
}

//
INLINE float QM_VECTORCALL QMVECTOR_GET_W(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_cvtss_f32(_mm_permute_ps(a, _MM_SHUFFLE(3, 3, 3, 3)));
#elif defined QM_USE_NEON
	return vgetq_lane_f32(a, 3);
#else
	return a.f[3];
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SET(float x, float y, float z, float w)
{
#if defined QM_USE_AVX
	return _mm_setr_ps(x, y, z, w);
#elif defined QM_USE_NEON
	float32x2_t xy = vcreate_f32(((ullong)(*(uint*)&x)) | (((ullong)(*(uint*)&y)) << 32));
	float32x2_t zw = vcreate_f32(((ullong)(*(uint*)&z)) | (((ullong)(*(uint*)&w)) << 32));
	return vcombine_f32(xy, zw);
#else
	return (QMVECTOR) { { x, y, z, w } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SET_X(QMVECTOR a, float x)
{
#if defined QM_USE_AVX
	return _mm_move_ss(a, _mm_set_ss(x));
#elif defined QM_USE_NEON
	return vsetq_lane_f32(x, a, 0);
#else
	return (QMVECTOR) { { x, a.f[1], a.f[2], a.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SET_Y(QMVECTOR a, float y)
{
#if defined QM_USE_AVX
	return _mm_insert_ps(a, _mm_set_ss(y), 0x10);
#elif defined QM_USE_NEON
	return vsetq_lane_f32(y, a, 1);
#else
	return (QMVECTOR) { { a.f[0], y, a.f[2], a.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SET_Z(QMVECTOR a, float z)
{
#if defined QM_USE_AVX
	return _mm_insert_ps(a, _mm_set_ss(z), 0x20);
#elif defined QM_USE_NEON
	return vsetq_lane_f32(z, a, 2);
#else
	return (QMVECTOR) { { a.f[0], a.f[1], z, a.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SET_W(QMVECTOR a, float w)
{
#if defined QM_USE_AVX
	return _mm_insert_ps(a, _mm_set_ss(w), 0x30);
#elif defined QM_USE_NEON
	return vsetq_lane_f32(w, a, 3);
#else
	return (QMVECTOR) { { a.f[0], a.f[1], a.f[2], w } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG(float diag)
{
#if defined QM_USE_AVX
	return _mm_set1_ps(diag);
#elif defined QM_USE_NEON
	return vdupq_n_f32(diag);
#else
	return (QMVECTOR) { {diag, diag, diag, diag} };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_X(QMVECTOR a)
{
#if defined QM_USE_AVX2
	return _mm_broadcastss_ps(a);
#elif defined QM_USE_AVX
	return _mm_permute_ps(a, _MM_SHUFFLE(0, 0, 0, 0));
#elif defined QM_USE_NEON
	return vdupq_lane_f32(vget_low_f32(a), 0);
#else
	return (QMVECTOR) { { a.f[0], a.f[0], a.f[0], a.f[0] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_Y(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_permute_ps(a, _MM_SHUFFLE(1, 1, 1, 1));
#elif defined QM_USE_NEON
	return vdupq_lane_f32(vget_low_f32(a), 1);
#else
	return (QMVECTOR) { { a.f[1], a.f[1], a.f[1], a.f[1] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_Z(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_permute_ps(a, _MM_SHUFFLE(2, 2, 2, 2));
#elif defined QM_USE_NEON
	return vdupq_lane_f32(vget_high_f32(a), 0);
#else
	return (QMVECTOR) { { a.f[2], a.f[2], a.f[2], a.f[2] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_W(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_permute_ps(a, _MM_SHUFFLE(3, 3, 3, 3));
#elif defined QM_USE_NEON
	return vdupq_lane_f32(vget_high_f32(a), 1);
#else
	return (QMVECTOR) { { a.f[3], a.f[3], a.f[3], a.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_ZERO(void)
{
#if defined QM_USE_AVX
	return QMC_ZERO.v;
#elif defined QM_USE_NEON
	return vdupq_n_f32(0.0f);
#else
	return (QMVECTOR) { { 0.0f, 0.0f, 0.0f, 0.0f } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_ONE(void)
{
#if defined QM_USE_AVX
	return QMC_ONE_V4.v;
#elif defined QM_USE_NEON
	return vdupq_n_f32(1.0f);
#else
	return (QMVECTOR) { { 1.0f, 1.0f, 1.0f, 1.0f } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_INF(void)
{
#if defined QM_USE_AVX
	return QMC_INF.v;
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vdupq_n_u32(0x7F800000));
#else
	QMVECTOR v;
	v.u[0] = v.u[1] = v.u[2] = v.u[3] = 0x7F800000;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_NAN(void)
{
#if defined QM_USE_AVX
	return QMC_NAN.v;
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vdupq_n_u32(0x7FC00000));
#else
	QMVECTOR v;
	v.u[0] = v.u[1] = v.u[2] = v.u[3] = 0x7FC00000;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_EPSILON(void)
{
#if defined QM_USE_AVX
	return QMC_EPSILON.v;
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vdupq_n_u32(0x34000000));
#else
	QMVECTOR v;
	v.u[0] = v.u[1] = v.u[2] = v.u[3] = 0x34000000;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIAG_MSB(void)
{
#if defined QM_USE_AVX
	__m128i v = _mm_set1_epi32((int)0x80000000);
	return _mm_castsi128_ps(v);
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vdupq_n_u32(0x80000000U));
#else
	QMVECTOR v;
	v.u[0] = v.u[1] = v.u[2] = v.u[3] = 0x80000000U;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_JOIN_XY(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_unpacklo_ps(a, b);
#elif defined QM_USE_NEON
	return vzipq_f32(a, b).val[0];
#else
	return (QMVECTOR) { { a.f[0], b.f[0], a.f[1], b.f[1] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_JOIN_ZW(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_unpackhi_ps(a, b);
#elif defined QM_USE_NEON
	return vzipq_f32(a, b).val[1];
#else
	return (QMVECTOR) { { a.f[2], b.f[2], a.f[3], b.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SWIZZLE(QMVECTOR a, uint e0, uint e1, uint e2, uint e3)
{
#if defined QM_USE_AVX
	uint e[4] = { e0, e1, e2, e3 };
	__m128i o = _mm_loadu_si128((const __m128i*)e);
	return _mm_permutevar_ps(a, o);
#elif defined QM_USE_NEON
	static const uint e[4] = { 0x03020100, 0x07060504, 0x0B0A0908, 0x0F0E0D0C };
	uint8x8x2_t t;
	t.val[0] = vreinterpret_u8_f32(vget_low_f32(a));
	t.val[1] = vreinterpret_u8_f32(vget_high_f32(a));
	uint32x2_t i = vcreate_u32(((ullong)(e[e0])) | (((ullong)(e[e1])) << 32));
	const uint8x8_t l = vtbl2_u8(t, vreinterpret_u8_u32(i));
	i = vcreate_u32(((ullong)(e[e2])) | (((ullong)(e[e3])) << 32));
	const uint8x8_t h = vtbl2_u8(t, vreinterpret_u8_u32(i));
	return vcombine_f32(vreinterpret_f32_u8(l), vreinterpret_f32_u8(h));
#else
	return (QMVECTOR) { { a.f[e0], a.f[e1], a.f[e2], a.f[e3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_PERMUTE(QMVECTOR a, QMVECTOR b, uint px, uint py, uint pz, uint pw)
{
#if defined QM_USE_AVX
	static const QmVecU4 u3 = { { 3, 3, 3, 3 } };
	ALIGNOF(16) uint e[4] = { px, py, pz, pw };
	__m128i o = _mm_loadu_si128((const __m128i*)e);
	__m128i s = _mm_cmpgt_epi32(o, _mm_castps_si128(u3.v));
	o = _mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(o), u3.v));
	__m128 u = _mm_permutevar_ps(a, o);
	__m128 v = _mm_permutevar_ps(b, o);
	u = _mm_andnot_ps(_mm_castsi128_ps(s), u);
	v = _mm_and_ps(_mm_castsi128_ps(s), v);
	return _mm_or_ps(u, v);
#elif defined QM_USE_NEON
	static const uint e[8] = { 0x03020100, 0x07060504, 0x0B0A0908, 0x0F0E0D0C, 0x13121110, 0x17161514, 0x1B1A1918, 0x1F1E1D1C };
	uint8x8x4_t t;
	t.val[0] = vreinterpret_u8_f32(vget_low_f32(a));
	t.val[1] = vreinterpret_u8_f32(vget_high_f32(a));
	t.val[2] = vreinterpret_u8_f32(vget_low_f32(b));
	t.val[3] = vreinterpret_u8_f32(vget_high_f32(b));
	uint32x2_t i = vcreate_u32(((ullong)(e[px])) | (((ullong)(e[py])) << 32));
	const uint8x8_t l = vtbl4_u8(t, vreinterpret_u8_u32(i));
	i = vcreate_u32(((ullong)(e[pz])) | (((ullong)(e[pw])) << 32));
	const uint8x8_t h = vtbl4_u8(t, vreinterpret_u8_u32(i));
	return vcombine_f32(vreinterpret_f32_u8(l), vreinterpret_f32_u8(h));
#else
	const uint* ab[2] = { (const uint*)&a, (const uint*)&b };
	QMVECTOR v;
	uint* vp = (uint*)&v;
	vp[0] = ab[(px >> 2)][(px & 3)];
	vp[1] = ab[(py >> 2)][(py & 3)];
	vp[2] = ab[(pz >> 2)][(pz & 3)];
	vp[3] = ab[(pw >> 2)][(pw & 3)];
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SHIFT_LEFT(QMVECTOR a, QMVECTOR b, uint e)
{
	return QMVECTOR_PERMUTE(a, b, e + 0, e + 1, e + 2, e + 3);
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_ROTATE_LEFT(QMVECTOR a, uint e)
{
	return QMVECTOR_SWIZZLE(a, e & 3, (e + 1) & 3, (e + 2) & 3, (e + 3) & 3);
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_ROTATE_RIGHT(QMVECTOR a, uint e)
{
	return QMVECTOR_SWIZZLE(a, (4 - e) & 3, (5 - e) & 3, (6 - e) & 3, (7 - e) & 3);
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_INSERT(QMVECTOR a, QMVECTOR b, uint e, uint s0, uint s1, uint s2, uint s3)
{
	QMVECTOR o = QMVECTOR_SELECT_CTRL(s0 & 1, s1 & 1, s2 & 1, s3 & 1);
	return QMVECTOR_SELECT(a, QMVECTOR_ROTATE_LEFT(b, e), o);
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_NEG(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_sub_ps(_mm_setzero_ps(), a);
#elif defined QM_USE_NEON
	return vnegq_f32(a);
#else
	return (QMVECTOR) { { -a.f[0], -a.f[1], -a.f[2], -a.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_ONE_NEG(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_sub_ps(QMC_ONE_V4.v, a);
#elif defined QM_USE_NEON
	return vsubq_f32(QMC_ONE_V4.v, a);
#else
	return (QMVECTOR) { { 1.0f - a.f[0], 1.0f - a.f[1], 1.0f - a.f[2], 1.0f - a.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_ADD(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_add_ps(a, b);
#elif defined QM_USE_NEON
	return vaddq_f32(a, b);
#else
	return (QMVECTOR) { { a.f[0] + b.f[0], a.f[1] + b.f[1], a.f[2] + b.f[2], a.f[3] + b.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SUB(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_sub_ps(a, b);
#elif defined QM_USE_NEON
	return vsubq_f32(a, b);
#else
	return (QMVECTOR) { {a.f[0] - b.f[0], a.f[1] - b.f[1], a.f[2] - b.f[2], a.f[3] - b.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_MUL(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_mul_ps(a, b);
#elif defined QM_USE_NEON
	return vmulq_f32(a, b);
#else
	return (QMVECTOR) { {a.f[0] * b.f[0], a.f[1] * b.f[1], a.f[2] * b.f[2], a.f[3] * b.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_MUL_ADD(QMVECTOR a, QMVECTOR b, QMVECTOR c)
{
#if defined QM_USE_AVX
	return _MM_FMADD_PS(a, b, c);
#elif defined QM_USE_NEON
#if defined _M_ARM64 || defined __aarch64__
	return vfmaq_f32(c, a, b);
#else
	return vmlaq_f32(c, a, b);
#endif
#else
	return (QMVECTOR) { { a.f[0] * b.f[0] + c.f[0], a.f[1] * b.f[1] + c.f[1], a.f[2] * b.f[2] + c.f[2], a.f[3] * b.f[3] + c.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_NEG_MUL_SUB(QMVECTOR a, QMVECTOR b, QMVECTOR c)
{
#if defined QM_USE_AVX
	return _MM_FNMADD_PS(a, b, c);
#elif defined QM_USE_NEON
#if defined _M_ARM64 || defined __aarch64__
	return vfmsq_f32(c, a, b);
#else
	return vmlsq_f32(c, a, b);
#endif
#else
	return (QMVECTOR) { { c.f[0] - (a.f[0] * b.f[0]), c.f[1] - (a.f[1] * b.f[1]), c.f[2] - (a.f[2] * b.f[2]), c.f[3] - (a.f[3] * b.f[3]) } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DIV(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_div_ps(a, b);
#elif defined QM_USE_NEON
	return vdivq_f32(a, b);
#else
	return (QMVECTOR) { { a.f[0] / b.f[0], a.f[1] / b.f[1], a.f[2] / b.f[2], a.f[3] / b.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_MAG(QMVECTOR a, float b)
{
#if defined QM_USE_AVX
	QMVECTOR m = _mm_set1_ps(b);
	return _mm_mul_ps(a, m);
#elif defined QM_USE_NEON
	return vmulq_n_f32(a, b);
#else
	return (QMVECTOR) { { a.f[0] * b, a.f[1] * b, a.f[2] * b, a.f[3] * b } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_ABR(QMVECTOR a, float b)
{
#if defined QM_USE_AVX
	QMVECTOR m = _mm_set1_ps(b);
	return _mm_div_ps(a, m);
#elif defined QM_USE_NEON
	QMVECTOR m = vdupq_n_f32(b);
	return vdivq_f32(a, m);
#else
	return (QMVECTOR) { { a.f[0] / b, a.f[1] / b, a.f[2] / b, a.f[3] / b } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_MIN(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_min_ps(a, b);
#elif defined QM_USE_NEON
	return vminq_f32(a, b);
#else
	return (QMVECTOR) { { QN_MIN(a.f[0], b.f[0]), QN_MIN(a.f[1], b.f[1]), QN_MIN(a.f[2], b.f[2]), QN_MIN(a.f[3], b.f[3]) } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_MAX(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_max_ps(a, b);
#elif defined QM_USE_NEON
	return vmaxq_f32(a, b);
#else
	return (QMVECTOR) { { QN_MAX(a.f[0], b.f[0]), QN_MAX(a.f[1], b.f[1]), QN_MAX(a.f[2], b.f[2]), QN_MAX(a.f[3], b.f[3]) } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_SQRT(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_sqrt_ps(a);
#elif defined QM_USE_NEON
	return vsqrtq_f32(a);
#else
	return (QMVECTOR) { { qm_sqrtf(a.f[0]), qm_sqrtf(a.f[1]), qm_sqrtf(a.f[2]), qm_sqrtf(a.f[3]) } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DOT3(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_dp_ps(a, b, 0x7f);
#elif defined QM_USE_AVX
	QMVECTOR t = _mm_mul_ps(a, b);
	t = _mm_and_ps(t, QMC_MASK_V3.v);
	t = _mm_hadd_ps(t, t);
	return _mm_hadd_ps(t, t);
#elif defined QM_USE_NEON
	float32x4_t t = vmulq_f32(a, b);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vpadd_f32(v1, v1);
	v2 = vdup_lane_f32(v2, 0);
	v1 = vadd_f32(v1, v2);
	return vcombine_f32(v1, v1);
#else
	float f = a.f[0] * b.f[0] + a.f[1] * b.f[1] + a.f[2] * b.f[2];
	return (QMVECTOR) { { f, f, f, f} };
#endif
}
//
INLINE float QM_VECTORCALL QMVECTOR_DOT3F(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	QMVECTOR t = _mm_dp_ps(a, b, 0x7f);
	return _mm_cvtss_f32(t);
#elif defined QM_USE_NEON
	QMVECTOR t = vmulq_f32(a, b);
	float32x2_t v1 = vget_low_f32(t);
	float32x2_t v2 = vget_high_f32(t);
	v1 = vpadd_f32(v1, v1);
	v2 = vdup_lane_f32(v2, 0);
	v1 = vadd_f32(v1, v2);
	t = vcombine_f32(v1, v1);
	return vgetq_lane_f32(t, 0);
#else
	return a.f[0] * b.f[0] + a.f[1] * b.f[1] + a.f[2] * b.f[2];
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_DOT4(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_dp_ps(a, b, 0xff);	// 0xf1 ?
#elif defined QM_USE_NEON
	QMVECTOR t = vmulq_f32(a, b);
	t = vpaddq_f32(t, t);
	return vpaddq_f32(t, t);
#else
	float f = a.f[0] * b.f[0] + a.f[1] * b.f[1] + a.f[2] * b.f[2] + a.f[3] * b.f[3];
	return (QMVECTOR) { { f, f, f, f} };
#endif
}

//
INLINE float QM_VECTORCALL QMVECTOR_DOT4F(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	QMVECTOR t = _mm_dp_ps(a, b, 0xff);	// 0xf1 ?
	return _mm_cvtss_f32(t);
#elif defined QM_USE_NEON
	QMVECTOR t = vmulq_f32(a, b);
	t = vpaddq_f32(t, t);
	t = vpaddq_f32(t, t);
	return vgetq_lane_f32(t, 0);
#else
	return a.f[0] * b.f[0] + a.f[1] * b.f[1] + a.f[2] * b.f[2] + a.f[3] * b.f[3];
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_CROSS3(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	QMVECTOR u = _mm_permute_ps(a, _MM_SHUFFLE(3, 0, 2, 1));
	QMVECTOR p = _mm_permute_ps(b, _MM_SHUFFLE(3, 1, 0, 2));
	QMVECTOR m = _mm_mul_ps(u, p);
	u = _mm_permute_ps(u, _MM_SHUFFLE(3, 0, 1, 2));
	p = _mm_permute_ps(p, _MM_SHUFFLE(3, 1, 0, 2));
	m = _MM_FNMADD_PS(u, p, m);
	return _mm_and_ps(m, QMC_MASK_V3.v);
#elif defined QM_USE_NEON
	QMVECTOR u = vextq_f32(a, a, 1);
	QMVECTOR p = vextq_f32(b, b, 2);
	QMVECTOR m = vmulq_f32(u, p);
	u = vextq_f32(u, u, 2);
	p = vextq_f32(p, p, 1);
	m = vmlsq_f32(m, u, p);
	return vandq_f32(m, QMC_MASK_V3.v);
#else
	QMVECTOR v;
	v.f[0] = a.f[1] * b.f[2] - a.f[2] * b.f[1];
	v.f[1] = a.f[2] * b.f[0] - a.f[0] * b.f[2];
	v.f[2] = a.f[0] * b.f[1] - a.f[1] * b.f[0];
	v.f[3] = 0.0f;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_CROSS4(QMVECTOR a, QMVECTOR b, QMVECTOR c)
{
#if defined QM_USE_AVX
	QMVECTOR u = _mm_permute_ps(b, _MM_SHUFFLE(2, 1, 3, 2));
	QMVECTOR p = _mm_permute_ps(c, _MM_SHUFFLE(1, 3, 2, 3));
	u = _mm_mul_ps(u, p);
	QMVECTOR o = _mm_permute_ps(b, _MM_SHUFFLE(1, 3, 2, 3));
	p = _mm_permute_ps(p, _MM_SHUFFLE(1, 3, 0, 1));
	u = _MM_FNMADD_PS(o, p, u);
	QMVECTOR n = _mm_permute_ps(a, _MM_SHUFFLE(0, 0, 0, 1));
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
	QMVECTOR v;
	v.f[0] = a.f[1] * (b.f[2] * c.f[3] - c.f[2] * b.f[3]) - a.f[2] * (b.f[1] * c.f[3] - c.f[1] * b.f[3]) + a.f[3] * (b.f[1] * c.f[2] - b.f[2] * c.f[1]);
	v.f[1] = -(a.f[0] * (b.f[2] * c.f[3] - c.f[2] * b.f[3]) - a.f[2] * (b.f[0] * c.f[3] - c.f[0] * b.f[3]) + a.f[3] * (b.f[0] * c.f[2] - c.f[0] * b.f[2]));
	v.f[2] = a.f[0] * (b.f[1] * c.f[3] - c.f[1] * b.f[3]) - a.f[1] * (b.f[0] * c.f[3] - c.f[0] * b.f[3]) + a.f[3] * (b.f[0] * c.f[1] - c.f[0] * b.f[1]);
	v.f[3] = -(a.f[0] * (b.f[1] * c.f[2] - c.f[1] * b.f[2]) - a.f[1] * (b.f[0] * c.f[2] - c.f[0] * b.f[2]) + a.f[2] * (b.f[0] * c.f[1] - c.f[0] * b.f[1]));
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_LEN_SQ3(QMVECTOR a)
{
	return QMVECTOR_DOT3(a, a);
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_LEN3(QMVECTOR a)
{
	return QMVECTOR_SQRT(QMVECTOR_DOT3(a, a));
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_LEN_SQ4(QMVECTOR a)
{
	return QMVECTOR_DOT4(a, a);
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_LEN4(QMVECTOR a)
{
	return QMVECTOR_SQRT(QMVECTOR_DOT4(a, a));
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_NORM3(QMVECTOR a)
{
#if defined QM_USE_AVX
	QMVECTOR lq = _mm_dp_ps(a, a, 0x7f);
	QMVECTOR u = _mm_sqrt_ps(lq);
	QMVECTOR z = _mm_setzero_ps();
	z = _mm_cmpneq_ps(z, u);
	lq = _mm_cmpneq_ps(lq, QMC_INF.v);
	u = _mm_div_ps(a, u);
	u = _mm_and_ps(u, z);
	QMVECTOR t1 = _mm_andnot_ps(lq, QMC_NAN.v);
	QMVECTOR t2 = _mm_and_ps(u, lq);
	return _mm_or_ps(t1, t2);
#elif defined QM_USE_NEON
	float32x4_t vv = vmulq_f32(a, a);
	float32x2_t v1 = vget_low_f32(vv);
	float32x2_t v2 = vget_high_f32(vv);
	v1 = vadd_f32(v1, v1);
	v2 = vdup_lane_f32(v2, 0);
	v1 = vpadd_f32(v1, v2);
	uint32x2_t ez = vceq_f32(v1, vdup_n_f32(0));
	uint32x2_t ei = vceq_f32(v1, vget_low_f32(QMC_INF.v));
	float32x2_t s0 = vrsqrte_f32(v1);
	float32x2_t p0 = vmul_f32(v1, s0);
	float32x2_t r0 = vrsqrts_f32(p0, s0);
	float32x2_t s1 = vmul_f32(s0, r0);
	float32x2_t p1 = vmul_f32(v1, s1);
	float32x2_t r1 = vrsqrts_f32(p1, s1);
	v2 = vmul_f32(s1, r1);
	QMVECTOR n = vmulq_f32(V, vcombine_f32(v2, v2));
	n = vbslq_f32(vcombine_u32(ez, ez), vdupq_n_f32(0), n);
	return vbslq_f32(vcombine_u32(ei, ei), QMC_NAN.v, n);
#else
	QMVECTOR v = QMVECTOR_LEN3(a);
	float l = v.f[0];
	if (l > 0.0f)
		l = 1.0f / l;
	return QMVECTOR_MAG(a, l);
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_NORM4(QMVECTOR a)
{
#if defined QM_USE_AVX
	QMVECTOR lq = _mm_dp_ps(a, a, 0xff);
	QMVECTOR u = _mm_sqrt_ps(lq);
	QMVECTOR z = _mm_setzero_ps();
	z = _mm_cmpneq_ps(z, u);
	lq = _mm_cmpneq_ps(lq, QMC_INF.v);
	u = _mm_div_ps(a, u);
	u = _mm_and_ps(u, z);
	QMVECTOR t1 = _mm_andnot_ps(lq, QMC_NAN.v);
	QMVECTOR t2 = _mm_and_ps(u, lq);
	return _mm_or_ps(t1, t2);
#elif defined QM_USE_NEON
	float32x4_t vv = vmulq_f32(a, a);
	float32x2_t v1 = vget_low_f32(vv);
	float32x2_t v2 = vget_high_f32(vv);
	v1 = vadd_f32(v1, v2);
	v1 = vpadd_f32(v1, v1);
	uint32x2_t ez = vceq_f32(v1, vdup_n_f32(0));
	uint32x2_t ei = vceq_f32(v1, vget_low_f32(QMC_INF.v));
	float32x2_t s0 = vrsqrte_f32(v1);
	float32x2_t p0 = vmul_f32(v1, s0);
	float32x2_t r0 = vrsqrts_f32(p0, s0);
	float32x2_t s1 = vmul_f32(s0, r0);
	float32x2_t p1 = vmul_f32(v1, s1);
	float32x2_t r1 = vrsqrts_f32(p1, s1);
	v2 = vmul_f32(s1, r1);
	QMVECTOR n = vmulq_f32(V, vcombine_f32(v2, v2));
	n = vbslq_f32(vcombine_u32(ez, ez), vdupq_n_f32(0), n);
	return vbslq_f32(vcombine_u32(ei, ei), QMC_NAN.v, n);
#else
	QMVECTOR v = QMVECTOR_LEN4(a);
	float l = v.f[0];
	if (l > 0.0f)
		l = 1.0f / l;
	return QMVECTOR_MAG(a, l);
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_CJG(QMVECTOR a)
{
#if defined QM_USE_AVX
	static const QmVec4 CJGMASK = { { -1.0f, -1.0f, -1.0f, 1.0f } };
	return _mm_mul_ps(a, CJGMASK.v);
#elif defined QM_USE_NEON
	static const QmVec4 CJGMASK = { { -1.0f, -1.0f, -1.0f, 1.0f } };
	return vmulq_f32(a, CJGMASK.v);
#else
	return (QMVECTOR) { { -a.f[0], -a.f[1], -a.f[2], a.f[3] } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_BLEND(QMVECTOR left, float left_scale, QMVECTOR right, float right_scale)
{
#if defined QM_USE_AVX
	QMVECTOR ls = _mm_set1_ps(left_scale);
	QMVECTOR rs = _mm_set1_ps(right_scale);
	return _mm_add_ps(_mm_mul_ps(left, ls), _mm_mul_ps(right, rs));
#elif defined QM_USE_NEON
	QMVECTOR ls = vmulq_n_f32(left, left_scale);
	QMVECTOR rs = vmulq_n_f32(right, right_scale);
	return vaddq_f32(ls, rs);
#else
	QMVECTOR ls = QMVECTOR_MAG(left, left_scale);
	QMVECTOR rs = QMVECTOR_MAG(right, right_scale);
	return QMVECTOR_ADD(ls, rs);
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_TRFM(QMVECTOR v, QmMat4 m)
{
#if defined QM_USE_AVX
	QMVECTOR p = _mm_permute_ps(v, _MM_SHUFFLE(3, 3, 3, 3));
	QMVECTOR o = _mm_permute_ps(v, _MM_SHUFFLE(2, 2, 2, 2));
	p = _mm_mul_ps(p, m.v[3]);
	p = _MM_FMADD_PS(o, m.v[2], p);
	o = _mm_permute_ps(v, _MM_SHUFFLE(1, 1, 1, 1));
	p = _MM_FMADD_PS(o, m.v[1], p);
	o = _mm_permute_ps(v, _MM_SHUFFLE(0, 0, 0, 0));
	return _MM_FMADD_PS(o, m.v[0], p);
#elif defined QM_USE_NEON
	QmVec4 p = vmulq_laneq_f32(m.v[0], v, 0);
	p = vfmaq_laneq_f32(p, m.v[1], v, 1);
	p = vfmaq_laneq_f32(p, m.v[2], v, 2);
	return vfmaq_laneq_f32(r.v, m.v[3], v, 3);
#else
	QMVECTOR r;
	r.f[0] = v.f[0] * m._11 + v.f[1] * m._21 + v.f[2] * m._31 + v.f[3] * m._41;
	r.f[1] = v.f[0] * m._12 + v.f[1] * m._22 + v.f[2] * m._32 + v.f[3] * m._42;
	r.f[2] = v.f[0] * m._13 + v.f[1] * m._23 + v.f[2] * m._33 + v.f[3] * m._43;
	r.f[3] = v.f[0] * m._14 + v.f[1] * m._24 + v.f[2] * m._34 + v.f[3] * m._44;
	return r;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_EQ(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_cmpeq_ps(a, b);
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vceqq_f32(a, b));
#else
	QMVECTOR v;
	v.u[0] = a.f[0] == b.f[0] ? 0xFFFFFFFF : 0;
	v.u[1] = a.f[1] == b.f[1] ? 0xFFFFFFFF : 0;
	v.u[2] = a.f[2] == b.f[2] ? 0xFFFFFFFF : 0;
	v.u[3] = a.f[3] == b.f[3] ? 0xFFFFFFFF : 0;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_EQ_EPS(QMVECTOR a, QMVECTOR b, QMVECTOR eps)
{
#if defined QM_USE_AVX
	QMVECTOR v = _mm_sub_ps(a, b);
	QMVECTOR t = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), v), v);
	return _mm_cmple_ps(t, eps);
#elif defined QM_USE_NEON
	float32x4_t v = vsubq_f32(a, b);
#if defined _MSC_VER && !defined __clang__ && !defined _ARM64_DISTINCT_NEON_TYPES
	return vacleq_f32(v, eps);
#else
	return vreinterpretq_f32_u32(vcleq_f32(vabsq_f32(v), eps));
#endif
#else
	QMVECTOR v;
	v.u[0] = qm_absf(a.f[0] - b.f[0]) <= eps.f[0] ? 0xFFFFFFFF : 0;
	v.u[1] = qm_absf(a.f[1] - b.f[1]) <= eps.f[1] ? 0xFFFFFFFF : 0;
	v.u[2] = qm_absf(a.f[2] - b.f[2]) <= eps.f[2] ? 0xFFFFFFFF : 0;
	v.u[3] = qm_absf(a.f[3] - b.f[3]) <= eps.f[3] ? 0xFFFFFFFF : 0;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_NEQ(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_cmpneq_ps(a, b);
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(a, b)));
#else
	QMVECTOR v;
	v.u[0] = a.f[0] != b.f[0] ? 0xFFFFFFFF : 0;
	v.u[1] = a.f[1] != b.f[1] ? 0xFFFFFFFF : 0;
	v.u[2] = a.f[2] != b.f[2] ? 0xFFFFFFFF : 0;
	v.u[3] = a.f[3] != b.f[3] ? 0xFFFFFFFF : 0;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_GT(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_cmpgt_ps(a, b);
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vcgtq_f32(a, b));
#else
	QMVECTOR v;
	v.u[0] = a.f[0] > b.f[0] ? 0xFFFFFFFF : 0;
	v.u[1] = a.f[1] > b.f[1] ? 0xFFFFFFFF : 0;
	v.u[2] = a.f[2] > b.f[2] ? 0xFFFFFFFF : 0;
	v.u[3] = a.f[3] > b.f[3] ? 0xFFFFFFFF : 0;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_GEQ(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_cmpge_ps(a, b);
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vcgeq_f32(a, b));
#else
	QMVECTOR v;
	v.u[0] = a.f[0] >= b.f[0] ? 0xFFFFFFFF : 0;
	v.u[1] = a.f[1] >= b.f[1] ? 0xFFFFFFFF : 0;
	v.u[2] = a.f[2] >= b.f[2] ? 0xFFFFFFFF : 0;
	v.u[3] = a.f[3] >= b.f[3] ? 0xFFFFFFFF : 0;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_LT(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_cmplt_ps(a, b);
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vcltq_f32(a, b));
#else
	QMVECTOR v;
	v.u[0] = a.f[0] < b.f[0] ? 0xFFFFFFFF : 0;
	v.u[1] = a.f[1] < b.f[1] ? 0xFFFFFFFF : 0;
	v.u[2] = a.f[2] < b.f[2] ? 0xFFFFFFFF : 0;
	v.u[3] = a.f[3] < b.f[3] ? 0xFFFFFFFF : 0;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_LEQ(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	return _mm_cmple_ps(a, b);
#elif defined QM_USE_NEON
	return vreinterpretq_f32_u32(vcleq_f32(a, b));
#else
	QMVECTOR v;
	v.u[0] = a.f[0] <= b.f[0] ? 0xFFFFFFFF : 0;
	v.u[1] = a.f[1] <= b.f[1] ? 0xFFFFFFFF : 0;
	v.u[2] = a.f[2] <= b.f[2] ? 0xFFFFFFFF : 0;
	v.u[3] = a.f[3] <= b.f[3] ? 0xFFFFFFFF : 0;
	return v;
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_INBOUNDS(QMVECTOR a, QMVECTOR bounds)
{
#if defined QM_USE_AVX
	QMVECTOR t = _mm_cmpge_ps(a, bounds);
	QMVECTOR u = _mm_mul_ps(bounds, QMC_NEG.v);
	return _mm_and_ps(t, _mm_cmple_ps(u, a));
#elif defined QM_USE_NEON
	QMVECTOR t = vcleq_f32(a, bounds);
	QMVECTOR u = vreinterpretq_u32_f32(vnegq_f32(bounds));
	QMVECTOR v = vandq_u32(t, vcleq_f32(vreinterpretq_f32_u32(u), a));
	return vreinterpretq_f32_u32(v);
#else
	QMVECTOR v;
	v.u[0] = a.f[0] >= bounds.f[0] && a.f[0] <= -bounds.f[0] ? 0xFFFFFFFF : 0;
	v.u[1] = a.f[1] >= bounds.f[1] && a.f[1] <= -bounds.f[1] ? 0xFFFFFFFF : 0;
	v.u[2] = a.f[2] >= bounds.f[2] && a.f[2] <= -bounds.f[2] ? 0xFFFFFFFF : 0;
	v.u[3] = a.f[3] >= bounds.f[3] && a.f[3] <= -bounds.f[3] ? 0xFFFFFFFF : 0;
	return v;
#endif
}

//
INLINE bool QM_VECTORCALL QMVECTOR_EQ_B(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	QMVECTOR t = _mm_cmpeq_ps(a, b);
	return (_mm_movemask_ps(t) == 0x0f) != 0;
#elif defined QM_USE_NEON
	uint32x4_t t = vceqq_f32(a, b);
	uint8x8x2_t u = vzip_u8(vget_low_u8(vreinterpret_u8_u32(t)), vget_high_u8(vreinterpret_u8_u32(t)));
	uint16x4x2_t v = vzip_u16(vreinterpret_u16_u8(u.val[0]), vreinterpret_u16_u8(u.val[1]));
	return vget_lane_u32(vreinterpret_u32_u16(v.val[1]), 1) == 0xFFFFFFFFU;
#else
	return
		qm_eqf(a.f[0], b.f[0]) && qm_eqf(a.f[1], b.f[1]) &&
		qm_eqf(a.f[2], b.f[2]) && qm_eqf(a.f[3], b.f[3]);
#endif
}

//
INLINE bool QM_VECTORCALL QMVECTOR_NEQ_B(QMVECTOR a, QMVECTOR b)
{
#if defined QM_USE_AVX
	QMVECTOR t = _mm_cmpneq_ps(a, b);
	return (_mm_movemask_ps(t)) != 0;
#elif defined QM_USE_NEON
	uint32x4_t t = vceqq_f32(a, b);
	uint8x8x2_t u = vzip_u8(vget_low_u8(vreinterpret_u8_u32(t)), vget_high_u8(vreinterpret_u8_u32(t)));
	uint16x4x2_t v = vzip_u16(vreinterpret_u16_u8(u.val[0]), vreinterpret_u16_u8(u.val[1]));
	return vget_lane_u32(vreinterpret_u32_u16(v.val[1]), 1) != 0xFFFFFFFFU;
#else
	return
		!qm_eqf(a.f[0], b.f[0]) || !qm_eqf(a.f[1], b.f[1]) ||
		!qm_eqf(a.f[2], b.f[2]) || !qm_eqf(a.f[3], b.f[3]);
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_ROUND(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_round_ps(a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#elif defined QM_USE_NEON
#if defined _M_ARM64 || defined _M_ARM64EC || defined __aarch64__
	return vrndnq_f32(a);
#else
	uint32x4_t s = vandq_u32(vreinterpretq_u32_f32(a), QMC_NEG_V4.v);
	float32x4_t m = vreinterpretq_f32_u32(vorrq_u32(QMC_NFRAC.v, s));
	float32x4_t t1 = vsubq_f32(vaddq_f32(a, m), m);
	float32x4_t t2 = vabsq_f32(a);
	uint32x4_t mask = vcleq_f32(t2, QMC_NFRAC.v);
	return vbslq_f32(mask, t1, a);
#endif
#else
	return (QMVECTOR) { { roundf(a.f[0]), roundf(a.f[1]), roundf(a.f[2]), roundf(a.f[3]) } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_TRUNC(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_round_ps(a, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
#elif defined QM_USE_NEON
#if defined _M_ARM64 || defined _M_ARM64EC || defined __aarch64__
	return vrndq_f32(a);
#else
	float32x4_t t = vreinterpretq_f32_u32(vcltq_f32(vabsq_f32(a), QMC_NFRAC.v));
	float32x4_t r = vcvtq_f32_s32(vcvtq_s32_f32(a));
	return vbslq_f32(vreinterpretq_u32_f32(t), r, a);
#endif
#else
	return (QMVECTOR) { { truncf(a.f[0]), truncf(a.f[1]), truncf(a.f[2]), truncf(a.f[3]) } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_FLOOR(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_floor_ps(a);
#elif defined QM_USE_NEON
#if defined _M_ARM64 || defined _M_ARM64EC || defined __aarch64__
	return vrndmq_f32(a);
#else
	float32x4_t t = vreinterpretq_f32_u32(vcltq_f32(vabsq_f32(a), QMC_NFRAC.v));
	float32x4_t r = vcvtq_f32_s32(vcvtq_s32_f32(a));
	r = vsubq_f32(r, vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(r), vreinterpretq_u32_f32(t))));
	return vbslq_f32(vreinterpretq_u32_f32(t), r, a);
#endif
#else
	return (QMVECTOR) { { floorf(a.f[0]), floorf(a.f[1]), floorf(a.f[2]), floorf(a.f[3]) } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_CEIL(QMVECTOR a)
{
#if defined QM_USE_AVX
	return _mm_ceil_ps(a);
#elif defined QM_USE_NEON
#if defined _M_ARM64 || defined _M_ARM64EC || defined __aarch64__
	return vrndpq_f32(a);
#else
	float32x4_t t = vreinterpretq_f32_u32(vcltq_f32(vabsq_f32(a), QMC_NFRAC.v));
	float32x4_t r = vcvtq_f32_s32(vcvtq_s32_f32(a));
	r = vaddq_f32(r, vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(r), vreinterpretq_u32_f32(t))));
	return vbslq_f32(vreinterpretq_u32_f32(t), r, a);
#endif
#else
	return (QMVECTOR) { { ceilf(a.f[0]), ceilf(a.f[1]), ceilf(a.f[2]), ceilf(a.f[3]) } };
#endif
}

//
INLINE QMVECTOR QM_VECTORCALL QMVECTOR_CLAMP(QMVECTOR a, QMVECTOR min, QMVECTOR max)
{
#if defined QM_USE_AVX
	QMVECTOR t = _mm_min_ps(a, max);
	return _mm_max_ps(t, min);
#elif defined QM_USE_NEON
	QMVECTOR t = vminq_f32(a, max);
	return vmaxq_f32(t, min);
#else
	return QMVECTOR_MIN(QMVECTOR_MAX(a, min), max);
#endif
}


//////////////////////////////////////////////////////////////////////////
// 벡터2

/// @brief 벡터2 값 설정
/// @param x,y 좌표
INLINE QmVec2 qm_vec2(float x, float y)
{
	return (QmVec2) { { x, y  } };
}

/// @brief 벡터2 초기화
INLINE QmVec2 qm_vec2_zero(void)
{
	static const QmVec2 ZERO = { { 0.0f, 0.0f } };
	return ZERO;
}

/// @brief 벡터2 대각값 설정 (모두 같은값으로 설정)
/// @param diag 대각 값
INLINE QmVec2 qm_vec2_diag(float diag)
{
	return (QmVec2) { { diag, diag  } };
}

/// @brief 벡터2 부호 반전
/// @param v 벡터2
INLINE QmVec2 qm_vec2_neg(QmVec2 v)
{
	return (QmVec2) { { -v.X, -v.Y } };
}

/// @brief 벡터2 덧셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_add(QmVec2 left, QmVec2 right)
{
	return (QmVec2) { { left.X + right.X, left.Y + right.Y } };
}

/// @brief 벡터2 뺄셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_sub(QmVec2 left, QmVec2 right)
{
	return (QmVec2) { { left.X - right.X, left.Y - right.Y } };
}

/// @brief 벡터2 확대
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 확대값
INLINE QmVec2 qm_vec2_mag(QmVec2 left, float right)
{
	return (QmVec2) { { left.X * right, left.Y * right } };
}

/// @brief 벡터2 줄이기
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 확대값
INLINE QmVec2 qm_vec2_abr(QmVec2 left, float right)
{
	return (QmVec2) { { left.X / right, left.Y / right } };
}

/// @brief 벡터2 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmVec2 qm_vec2_mul(QmVec2 left, QmVec2 right)
{
	return (QmVec2) { { left.X * right.X, left.Y * right.Y } };
}

/// @brief 벡터2 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmVec2 qm_vec2_div(QmVec2 left, QmVec2 right)
{
	return (QmVec2) { { left.X / right.X, left.Y / right.Y } };
}

/// @brief 벡터2의 최소값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_min(QmVec2 left, QmVec2 right)
{
	return (QmVec2) { { QN_MIN(left.X, right.X), QN_MIN(left.Y, right.Y) } };
}

/// @brief 벡터2의 최대값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_max(QmVec2 left, QmVec2 right)
{
	return (QmVec2) { { QN_MAX(left.X, right.X), QN_MAX(left.Y, right.Y) } };
}

/// @brief 벡터2 정규화
/// @param v 벡터2
INLINE QmVec2 qm_vec2_norm(QmVec2 v)
{
	return qm_vec2_mag(v, qm_inv_sqrtf(qm_vec2_dot(v, v)));
}

/// @brief 벡터2의 외적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
INLINE QmVec2 qm_vec2_cross(QmVec2 left, QmVec2 right)
{
	return (QmVec2) { { left.Y * right.X - left.X * right.Y, left.X * right.Y - left.Y * right.X } };
}

/// @brief 벡터2 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmVec2 qm_vec2_interpolate(QmVec2 left, QmVec2 right, float scale)
{
	return qm_vec2_add(qm_vec2_mag(left, 1.0f - scale), qm_vec2_mag(right, scale));
}

/// @brief 벡터2 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmVec2 qm_vec2_lerp(QmVec2 left, QmVec2 right, float scale)
{
	return qm_vec2_add(left, qm_vec2_mag(qm_vec2_sub(right, left), scale));		// NOLINT
}

/// @brief 벡터2 내적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 내적 값
INLINE float qm_vec2_dot(QmVec2 left, QmVec2 right)
{
	return left.X * right.X + left.Y * right.Y;
}

/// @brief 벡터2 길이의 제곱
/// @param v 벡터2
/// @return 길이의 제곱
INLINE float qm_vec2_len_sq(QmVec2 v)
{
	return qm_vec2_dot(v, v);
}

/// @brief 두 벡터2 거리의 제곱
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2 거리의 제곱값
INLINE float qm_vec2_dist_sq(QmVec2 left, QmVec2 right)
{
	return qm_vec2_len_sq(qm_vec2_sub(left, right));
}

/// @brief 벡터2 길이
/// @param v 벡터2
/// @return 길이
INLINE float qm_vec2_len(QmVec2 v)
{
	return qm_sqrtf(qm_vec2_len_sq(v));
}

/// @brief 두 벡터2의 거리
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2의 거리값
INLINE float qm_vec2_dist(QmVec2 left, QmVec2 right)
{
	return qm_sqrtf(qm_vec2_dist_sq(left, right));
}

/// @brief 벡터2의 비교
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡트
/// @return 같으면 참
INLINE bool qm_vec2_eq(QmVec2 left, QmVec2 right)
{
	return qm_eqf(left.X, right.X) && qm_eqf(left.Y, right.Y);
}

/// @brief 벡터2가 0인가 비교
/// @param v 비교할 벡터2
/// @return 0이면 참
INLINE bool qm_vec2_isz(QmVec2 v)
{
	return v.X == 0.0f && v.Y == 0.0f;
}


//////////////////////////////////////////////////////////////////////////
// 벡터4

/// @brief 벡터4 값 설정
/// @param x,y,z,w 벡터4 요소
/// @return 만든 벡터4
INLINE QmVec4 QM_VECTORCALL qm_vec4(float x, float y, float z, float w)
{
	return (QmVec4) { .v = QMVECTOR_SET(x, y, z, w) };
}

/// @brief 벡터4 초기화
INLINE QmVec4 QM_VECTORCALL qm_vec4_zero(void)
{
	return QMC_ZERO;
}

/// @brief 벡터4 대각값 설정 (모든 요소를 같은 값을)
/// @param diag 대각값
INLINE QmVec4 QM_VECTORCALL qm_vec4_diag(float diag)
{
	return (QmVec4) { .v = QMVECTOR_DIAG(diag) };
}

/// @brief 벡터4 반전
/// @param v 원본 벡터4
INLINE QmVec4 QM_VECTORCALL qm_vec4_neg(QmVec4 v)
{
	return (QmVec4) { .v = QMVECTOR_NEG(v.v) };
}

/// @brief 벡터4 덧셈
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
INLINE QmVec4 QM_VECTORCALL qm_vec4_add(QmVec4 left, QmVec4 right)
{
	return (QmVec4) { .v = QMVECTOR_ADD(left.v, right.v) };
}

/// @brief 벡터4 뺄셈
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
INLINE QmVec4 QM_VECTORCALL qm_vec4_sub(QmVec4 left, QmVec4 right)
{
	return (QmVec4) { .v = QMVECTOR_SUB(left.v, right.v) };
}

/// @brief 벡터4 확대
/// @param left 원본 벡터4
/// @param right 확대값
INLINE QmVec4 QM_VECTORCALL qm_vec4_mag(QmVec4 left, float right)
{
	return (QmVec4) { .v = QMVECTOR_MAG(left.v, right) };
}

/// @brief 벡터4 줄임
/// @param left 원본 벡터4
/// @param right 줄일값
INLINE QmVec4 QM_VECTORCALL qm_vec4_abr(QmVec4 left, float right)
{
	return (QmVec4) { .v = QMVECTOR_ABR(left.v, right) };
}

/// @brief 벡터4 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmVec4 QM_VECTORCALL qm_vec4_mul(QmVec4 left, QmVec4 right)
{
	return (QmVec4) { .v = QMVECTOR_MUL(left.v, right.v) };
}

/// @brief 벡터4 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmVec4 QM_VECTORCALL qm_vec4_div(QmVec4 left, QmVec4 right)
{
	return (QmVec4) { .v = QMVECTOR_DIV(left.v, right.v) };
}

/// @brief 벡터4의 최소값
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
INLINE QmVec4 QM_VECTORCALL qm_vec4_min(QmVec4 left, QmVec4 right)
{
	return (QmVec4) { .v = QMVECTOR_MIN(left.v, right.v) };
}

/// @brief 벡터4의 최대값
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
INLINE QmVec4 QM_VECTORCALL qm_vec4_max(QmVec4 left, QmVec4 right)
{
	return (QmVec4) { .v = QMVECTOR_MAX(left.v, right.v) };
}

/// @brief 벡터4 정규화
/// @param v 벡터4
INLINE QmVec4 QM_VECTORCALL qm_vec4_norm(QmVec4 v)
{
	return (QmVec4) { .v = QMVECTOR_NORM4(v.v) };
}

/// @brief 벡터4 외적
/// @param v1 첫번째 벡터4
/// @param v2 두번째 벡터4
/// @param v3 세번째 벡터4
INLINE QmVec4 QM_VECTORCALL qm_vec4_cross(QmVec4 v1, QmVec4 v2, QmVec4 v3)
{
	return (QmVec4) { .v = QMVECTOR_CROSS4(v1.v, v2.v, v3.v) };
}

/// @brief 벡터4 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmVec4 QM_VECTORCALL qm_vec4_interpolate(QmVec4 left, QmVec4 right, float scale)
{
	QMVECTOR l = QMVECTOR_MAG(left.v, 1.0f - scale);
	QMVECTOR r = QMVECTOR_MAG(right.v, scale);
	return (QmVec4) { .v = QMVECTOR_ADD(l, r) };
}

/// @brief 벡터4 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmVec4 QM_VECTORCALL qm_vec4_lerp(QmVec4 left, QmVec4 right, float scale)
{
	QMVECTOR s = QMVECTOR_SUB(right.v, left.v);
	QMVECTOR m = QMVECTOR_MAG(s, scale);
	return (QmVec4) { .v = QMVECTOR_ADD(left.v, m) };
}

/// @brief 벡터4 트랜스폼
/// @param v 원본 벡터4
/// @param trfm 변환 행렬
INLINE QmVec4 QM_VECTORCALL qm_vec4_trfm(QmVec4 v, QmMat4 trfm)
{
	return (QmVec4) { .v = QMVECTOR_TRFM(v.v, trfm) };
}

/// @brief 벡터4 내적
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 내적값
INLINE float QM_VECTORCALL qm_vec4_dot(QmVec4 left, QmVec4 right)
{
	return QMVECTOR_DOT4F(left.v, right.v);
}

/// @brief 벡터4 거리의 제곱
/// @param v 대상 벡터4
/// @return 벡터4 거리의 제곱값
INLINE float QM_VECTORCALL qm_vec4_len_sq(QmVec4 v)
{
	return QMVECTOR_DOT4F(v.v, v.v);
}

/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터4 거리의 제곱값
INLINE float QM_VECTORCALL qm_vec4_dist_sq(QmVec4 left, QmVec4 right)
{
	QMVECTOR t = QMVECTOR_SUB(left.v, right.v);
	return QMVECTOR_DOT4F(t, t);
}

/// @brief 벡터4 거리
/// @param v 대상 벡터4
/// @return 벡터4 거리값
INLINE float QM_VECTORCALL qm_vec4_len(QmVec4 v)
{
	return qm_sqrtf(QMVECTOR_DOT4F(v.v, v.v));
}

/// @brief 두 벡터4의 거리
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터3의 거리값
INLINE float QM_VECTORCALL qm_vec4_dist(QmVec4 left, QmVec4 right)
{
	QMVECTOR t = QMVECTOR_SUB(left.v, right.v);
	return qm_sqrtf(QMVECTOR_DOT4F(t, t));
}

/// @brief 두 벡터4를 비교
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터4가 같으면 참
INLINE bool QM_VECTORCALL qm_vec4_eq(QmVec4 left, QmVec4 right)
{
	return QMVECTOR_EQ_B(left.v, right.v);
}

/// @brief 벡터가 0인지 비교
/// @param v 비교할 벡터4
/// @return 벡터4가 0이면 참
INLINE bool QM_VECTORCALL qm_vec4_isz(QmVec4 v)
{
	return QMVECTOR_EQ_B(v.v, QMC_ZERO.v);
}


//////////////////////////////////////////////////////////////////////////
// 사원수

/// @brief 사원수 초기화
INLINE QmQuat QM_VECTORCALL qm_quat_unit(void)
{
	return QMC_UNIT_R3;
}

/// @brief 사원수 항목 곱셈
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
INLINE QmQuat QM_VECTORCALL qm_quat_mul(QmQuat left, QmQuat right)
{
#if defined QM_USE_AVX
	static const QmVec4 sign1 = { { 1.0f, -1.0f, 1.0f, -1.0f } };
	static const QmVec4 sign2 = { { 1.0f, 1.0f, -1.0f, -1.0f } };
	static const QmVec4 sign3 = { { -1.0f, 1.0f, 1.0f, -1.0f } };
	QMVECTOR p = _mm_permute_ps(right.v, _MM_SHUFFLE(3, 3, 3, 3));
	QMVECTOR rx = _mm_permute_ps(right.v, _MM_SHUFFLE(0, 0, 0, 0));
	QMVECTOR ry = _mm_permute_ps(right.v, _MM_SHUFFLE(1, 1, 1, 1));
	QMVECTOR rz = _mm_permute_ps(right.v, _MM_SHUFFLE(2, 2, 2, 2));
	QMVECTOR s = _mm_permute_ps(left.v, _MM_SHUFFLE(0, 1, 2, 3));
	p = _mm_mul_ps(p, left.v);
	rx = _mm_mul_ps(rx, s);
	s = _mm_permute_ps(s, _MM_SHUFFLE(2, 3, 0, 1));
	p = _MM_FMADD_PS(rx, sign1.v, p);
	ry = _mm_mul_ps(ry, s);
	s = _mm_permute_ps(s, _MM_SHUFFLE(0, 1, 2, 3));
	ry = _mm_mul_ps(ry, sign2.v);
	rz = _mm_mul_ps(rz, s);
	ry = _MM_FMADD_PS(rz, sign3.v, ry);
	return (QmQuat) { .v = _mm_add_ps(p, ry) };
#elif defined QM_USE_NEON
	static const QmVec4 sign1 = { { 1.0f, -1.0f, 1.0f, -1.0f } };
	static const QmVec4 sign2 = { { 1.0f, 1.0f, -1.0f, -1.0f } };
	static const QmVec4 sign3 = { { -1.0f, 1.0f, 1.0f, -1.0f } };
	QMVECTOR r1032 = vrev64q_f32(right.v);
	QMVECTOR r3210 = vcombine_f32(vget_high_f32(r1032), vget_low_f32(r1032));
	QMVECTOR r2301 = vrev64q_f32(r3210);
	QMVECTOR q = vmulq_f32(r3210, vmulq_f32(vdupq_laneq_f32(left.v, 0), sign1.v));
	q = vfmaq_f32(q.v, r2301, vmulq_f32(vdupq_laneq_f32(left.v, 1), sign2.v));
	q = vfmaq_f32(q.v, r1032, vmulq_f32(vdupq_laneq_f32(left.v, 2), sign3.v));
	return (QmQuat) { .v = vfmaq_laneq_f32(q.v, right.v, left.v, 3) };
#else
	return qm_vec4(
		left.X * right.W + left.Y * right.Z - left.Z * right.Y + left.W * right.X,
		-left.X * right.Z + left.Y * right.W + left.Z * right.X + left.W * right.Y,
		left.X * right.Y - left.Y * right.X + left.Z * right.W + left.W * right.Z,
		-left.X * right.X - left.Y * right.Y - left.Z * right.Z + left.W * right.W);
#endif
}

/// @brief 켤레 사원수
/// @param q 원본 사원수
INLINE QmQuat QM_VECTORCALL qm_quat_cjg(QmQuat q)
{
	return (QmQuat) { .v = QMVECTOR_CJG(q.v) };
}

/// @brief 역사원수를 얻는다
/// @param q 원본 사원수
INLINE QmQuat QM_VECTORCALL qm_quat_inv(QmQuat q)
{
	QMVECTOR l = QMVECTOR_LEN_SQ4(q.v);
	QMVECTOR c = QMVECTOR_CJG(q.v);
	QMVECTOR o = QMVECTOR_LEQ(l, QMC_EPSILON.v);
	QMVECTOR p = QMVECTOR_DIV(c, l);
	return (QmQuat) { .v = QMVECTOR_SELECT(p, QMC_ZERO.v, o) };
}

/// @brief 사원수 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 사원수
/// @param right 대상 사원수
/// @param scale 보간값
INLINE QmQuat QM_VECTORCALL qm_quat_lerp(QmQuat left, QmQuat right, float scale)
{
	return (QmQuat) { .v = QMVECTOR_NORM4(QMVECTOR_BLEND(left.v, 1.0f - scale, right.v, scale)) };
}

/// @brief 사원수의 원형 보간
/// @param left 기준 사원수
/// @param right 대상 사원수
/// @param scale 변화량
INLINE QmQuat QM_VECTORCALL qm_quat_slerp(QmQuat left, QmQuat right, float scale)
{
	float dot = QMVECTOR_DOT4F(left.v, right.v);
	QMVECTOR q1, q2;
	if (dot < 0.0f)
	{
		dot = -dot;
		q1 = left.v;
		q2 = QMVECTOR_NEG(right.v);
	}
	else
	{
		q1 = left.v;
		q2 = right.v;
	}
	float angle = acosf(dot);
	float ls = sinf((1.0f - scale) * angle);
	float rs = sinf(scale * angle);
	QMVECTOR r = QMVECTOR_BLEND(q1, ls, q2, rs);
	return (QmQuat) { .v = QMVECTOR_NORM4(r) };
}

/// @brief 행렬로 사원수 회전
/// @param rot 회전할 행렬
INLINE QmQuat QM_VECTORCALL qm_quat_mat(QmMat4 rot)
{
	float diag = rot._11 + rot._22 + rot._33 + 1.0f;
	QMVECTOR q;
	if (diag > 0.0f)
	{
		// 진단값에서 안전치
		float scale = qm_sqrtf(diag) * 2.0f;
		float iscl = 1.0f / scale;
		q = QMVECTOR_SET((rot._23 - rot._32) * iscl, (rot._31 - rot._13) * iscl, (rot._12 - rot._21) * iscl, 0.25f * scale);
	}
	else
	{
		// 필요한 스케일 만들기
		float scale = qm_sqrtf(1.0f + rot._11 - rot._22 - rot._33) * 2.0f;
		float iscl = 1.0f / scale;
		if (rot._11 > rot._22 && rot._11 > rot._33)
			q = QMVECTOR_SET(0.25f * scale, (rot._12 + rot._21) * iscl, (rot._31 + rot._13) * iscl, (rot._23 - rot._32) * iscl);
		else if (rot._22 > rot._33)
			q = QMVECTOR_SET((rot._12 + rot._21) * iscl, 0.25f * scale, (rot._23 + rot._32) * iscl, (rot._31 - rot._13) * iscl);
		else
			q = QMVECTOR_SET((rot._31 + rot._13) * iscl, (rot._23 + rot._32) * iscl, 0.25f * scale, (rot._12 - rot._21) * iscl);
	}
	QMVECTOR d = QMVECTOR_DOT4(q, q);
	if (QMVECTOR_EQ_B(d, QMC_ONE_V4.v))
		return (QmQuat) { .v = q };
	d = QMVECTOR_DIV(QMC_ONE_V4.v, QMVECTOR_SQRT(d));
	return (QmQuat) { .v = QMVECTOR_MUL(q, d) };
}

/// @brief 사원수를 벡터3 축으로 회전시킨다
/// @param v 벡터3 회전축
/// @param angle 회전값
INLINE QmQuat QM_VECTORCALL qm_quat_axis_vec(QmVec4 v, float angle)
{
	float s, c;
	qm_sincosf(angle * 0.5f, &s, &c);
	QMVECTOR n = QMVECTOR_SELECT(QMC_ONE_V4.v, QMVECTOR_NORM3(v.v), QMC_S1110.v);
	QMVECTOR m = QMVECTOR_SET(s, s, s, c);
	return (QmQuat) { .v = QMVECTOR_MUL(n, m) };
}

/// @brief 벡터로 화전
/// @param rot 벡터3 회전 행렬
INLINE QmQuat QM_VECTORCALL qm_quat_vec(QmVec4 rot)
{
	float rs, rc, ps, pc, ys, yc;
	qm_sincosf(rot.X * 0.5f, &rs, &rc);
	qm_sincosf(rot.Y * 0.5f, &ps, &pc);
	qm_sincosf(rot.Z * 0.5f, &ys, &yc);
	float pcyc = pc * yc;
	float psyc = ps * yc;
	float pcys = pc * ys;
	float psys = ps * ys;
	return qm_vec4(rs * pcyc - rc * psys, rc * psyc + rs * pcys, rc * pcys + rs * psyc, rc * pcyc + rs * psys);
}

/// @brief 사원수를 X축 회전시킨다
/// @param rot X축 회전값
INLINE QmQuat QM_VECTORCALL qm_quat_x(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
#if defined QM_USE_AVX
	QMVECTOR r = _mm_setr_ps(s, 0.0f, 0.0f, c);
	return (QmQuat) { .v = _mm_shuffle_ps(r, r, _MM_SHUFFLE(0, 1, 2, 3)) };
#else
	return (QmQuat) { { s, 0.0f, 0.0f, c } };
#endif
}

/// @brief 사원수를 Y축 회전시킨다
/// @param rot Y축 회전값
INLINE QmQuat QM_VECTORCALL qm_quat_y(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
#if defined QM_USE_AVX
	QMVECTOR r = _mm_setr_ps(0.0f, s, 0.0f, c);
	return (QmQuat) { .v = _mm_shuffle_ps(r, r, _MM_SHUFFLE(0, 1, 2, 3)) };
#else
	return (QmQuat) { { 0.0f, s, 0.0f, c } };
#endif
}

/// @brief 사원수를 Z축 회전시킨다
/// @param rot Z축 회전값
INLINE QmQuat QM_VECTORCALL qm_quat_z(float rot)
{
	float s, c;
	qm_sincosf(rot * 0.5f, &s, &c);
#if defined QM_USE_AVX
	QMVECTOR r = _mm_setr_ps(0.0f, 0.0f, s, c);
	return (QmQuat) { .v = _mm_shuffle_ps(r, r, _MM_SHUFFLE(0, 1, 2, 3)) };
#else
	return (QmQuat) { { 0.0f, 0.0f, s, c } };
#endif
}

/// @brief 지수 사원수 값을 얻는다
/// @param q 원본 사원수
INLINE QmQuat QM_VECTORCALL qm_quat_exp(QmQuat q)
{
	QmVec4 t = (QmVec4){ .v = QMVECTOR_LEN3(q.v) };
	float n = t.X;
	if (n == 0.0)
		return QMC_UNIT_R3;
	float sn, cn;
	qm_sincosf(n, &sn, &cn);
	n = 1.0f / n;
	return (QmQuat) { .v = QMVECTOR_SET(sn * q.X * n, sn * q.Y * n, sn * q.Z * n, cn) };
}

/// @brief 사원수 로그
/// @param q 입력 사원수
INLINE QmVec4 QM_VECTORCALL qm_quat_ln(QmQuat q)
{
	QmVec4 t = (QmVec4){ .v = QMVECTOR_LEN_SQ3(q.v) };
	float n = t.X;
	if (n > 1.0001f)
		return q;
	if (n > 0.99999f)
	{
		float nv = qm_sqrtf(n);
		float at = atan2f(nv, q.W) / nv;
		return (QmVec4) { .v = QMVECTOR_MAG(q.v, at) };
	}
	// 법선이 1보다 작다. 이런일은 생기지 않는다!!!!
	qn_assert(false, "법선이 1보다 작은데? 어째서???");
	return QMC_ZERO;
}

/// @brief 사원수가 단위 사원수인지 조사
/// @param q 비교할 사원수
/// @return 사원수가 단위 사원수면 참
INLINE bool QM_VECTORCALL qm_quat_isu(QmQuat q)
{
	return QMVECTOR_EQ_B(q.v, QMC_UNIT_R3.v);
}


//////////////////////////////////////////////////////////////////////////
// 평면

/// @brief 벡터로 면을 만든다
/// @param v 벡터3
/// @param d 면의 법선
INLINE QmPlane QM_VECTORCALL qm_planev(QmVec4 v, float d)
{
	return (QmPlane) { .v = QMVECTOR_SET(v.X, v.Y, v.Z, d) };
}

/// @brief 점과 점의 법선으로 면을 만든다
/// @param v 벡터3 점
/// @param n 벡터3 점의 법선
INLINE QmPlane QM_VECTORCALL qm_planevv(QmVec4 v, QmVec4 n)
{
	float d = QMVECTOR_DOT3F(v.v, n.v);
	return (QmPlane) { .v = QMVECTOR_SET(v.X, v.Y, v.Z, d) };
}

/// @brief 점 세개로 평면을 만든다
/// @param v1 벡터3 점1
/// @param v2 벡터3 점2
/// @param v3 벡터3 점3
INLINE QmPlane QM_VECTORCALL q4_planevvv(QmVec4 v1, QmVec4 v2, QmVec4 v3)
{
	QMVECTOR l = QMVECTOR_SUB(v2.v, v1.v);
	QMVECTOR r = QMVECTOR_SUB(v3.v, v2.v);
	QmPlane n = (QmPlane){ .v = QMVECTOR_NORM3(QMVECTOR_CROSS3(l, r)) };
	n.D = -QMVECTOR_DOT3F(v1.v, n.v);
	return n;
}

/// @brief 면을 초기화한다
INLINE QmPlane QM_VECTORCALL qm_plane_unit(void)
{
	return QMC_UNIT_R3;
}

/// @brief 면을 정규화 한다
/// @param p 대상 면
INLINE QmPlane QM_VECTORCALL qm_plane_norm(QmPlane p)
{
	QMVECTOR t = QMVECTOR_LEN_SQ3(p.v);
	QMVECTOR u = QMVECTOR_DIV(QMC_ONE_V4.v, QMVECTOR_SQRT(t));
	return (QmPlane) { .v = QMVECTOR_MUL(p.v, u) };
}

/// @brief 면을 뒤집어서 정규화 한다
/// @param p 대상 면
INLINE QmPlane QM_VECTORCALL qm_plane_rev_norm(QmPlane p)
{
	QMVECTOR t = QMVECTOR_LEN_SQ3(p.v);
	QMVECTOR u = QMVECTOR_NEG(QMVECTOR_DIV(QMC_ONE_V4.v, QMVECTOR_SQRT(t)));
	return (QmPlane) { .v = QMVECTOR_MUL(p.v, u) };
}

/// @brief 면 트랜스폼
/// @param plane 대상 면
/// @param trfm 트랜스폼 행렬
INLINE QmPlane QM_VECTORCALL qm_plane_trfm(QmPlane plane, QmMat4 trfm)
{
	QMVECTOR x = QMVECTOR_DIAG_X(plane.v);
	QMVECTOR y = QMVECTOR_DIAG_Y(plane.v);
	QMVECTOR z = QMVECTOR_DIAG_Z(plane.v);
	QMVECTOR w = QMVECTOR_DIAG_W(plane.v);
	QMVECTOR t = QMVECTOR_MUL(w, trfm.v[3]);
	t = QMVECTOR_MUL_ADD(z, trfm.v[2], t);
	t = QMVECTOR_MUL_ADD(y, trfm.v[1], t);
	t = QMVECTOR_MUL_ADD(x, trfm.v[0], t);
	return (QmPlane) { .v = t };
}

/// @brief 면과 점(벡터3)의 내적
/// @param p 대상 면
/// @param v 대상 벡터3
/// @return 면과 점의 내적
INLINE float QM_VECTORCALL qm_plane_dot_coord(QmPlane p, QmVec4 v)
{
	QMVECTOR t = QMVECTOR_SELECT(QMC_ONE_V4.v, v.v, QMC_S1110.v);
	return QMVECTOR_DOT4F(p.v, t);
}

/// @brief 면과 법선의 내적
/// @param p 대상 면
/// @param v 대상 벡터3
/// @return 면과 법선의 내적
INLINE float QM_VECTORCALL qm_plane_dot_normal(QmPlane p, QmVec4 v)
{
	return QMVECTOR_DOT3F(p.v, v.v);
}

/// @brief 면과 점의 거리를 얻는다
/// @param p 대상 면
/// @param v 대상 벡터3 점
/// @return 면과 점의 거리
INLINE float QM_VECTORCALL qm_plane_dist(QmPlane p, QmVec4 v)
{
	return QMVECTOR_DOT3F(v.v, p.v) + p.D;
}

/// @brief 면과 선분의 거리를 얻는다
/// @param p 대상 면
/// @param begin 대상 선의 시작
/// @param end 대상 선의 끝
/// @return 면과 선분의 거리
INLINE float QM_VECTORCALL qm_plane_distance_line(QmPlane p, QmVec4 begin, QmVec4 end)
{
	float f = 1.0f / QMVECTOR_DOT3F(p.v, QMVECTOR_SUB(end.v, begin.v));
	return -(QMVECTOR_DOT3F(p.v, begin.v) + p.D) * f;
}

/// @brief 점과 면의 관계를 얻는다
/// @param p 대상 면
/// @param v 관계를 얻을 벡터3 점
/// @return 관계 값을 얻는다
/// @retval 0 점이 면 위에 있다
/// @retval 1 점이 면 앞에 있다
/// @retval -1 점이 면 뒤에 있다
INLINE int QM_VECTORCALL qm_plane_rel_point(QmPlane p, QmVec4 v)
{
	float f = QMVECTOR_DOT3F(p.v, v.v) + p.D;
	if (f < -QM_EPSILON)
		return -1;  // back
	if (f > QM_EPSILON)
		return 1;   // front
	return 0;       // on
}

/// @brief 면과 면의 충돌 평면을 만든다
/// @param p 대상 평면
/// @param o 검사할 평면
/// @param loc 반환 시작 벡터3 (반환, 널가능)
/// @param dir 반환 방향 벡터3 (반환, 널가능)
/// @return 만들 수 있으면 TRUE
INLINE bool QM_VECTORCALL qm_plane_intersect(QmPlane p, QmPlane o, QmVec4* loc, QmVec4* dir)
{
	float f0 = qm_sqrtf(QMVECTOR_DOT3F(p.v, p.v));
	float f1 = qm_sqrtf(QMVECTOR_DOT3F(o.v, o.v));
	float f2 = QMVECTOR_DOT3F(p.v, o.v);
	float det = f0 * f1 - f2 * f2;
	if (qm_absf(det) < QM_EPSILON)
		return false;
	if (dir)
		dir->v = QMVECTOR_CROSS3(p.v, o.v);
	if (loc)
	{
		float inv = 1.0f / det;
		float fa = (f1 * -p.D + f2 * o.D) * inv;
		float fb = (f0 * -o.D + f2 * p.D) * inv;
		loc->v = QMVECTOR_SET(p.A * fa + o.A * fb, p.B * fa + o.B * fb, p.C * fa + o.C * fb, 0.0f);
	}
	return true;
}

/// @brief 면과 선분 충돌 여부
/// @param plane 면
/// @param loc 선의 시작
/// @param dir 선의 방량
/// @param pv 충돌 위치 (반환, 널가능)
/// @return 충돌하면 참
INLINE bool QM_VECTORCALL qm_plane_intersect_line(QmPlane plane, QmVec4 loc, QmVec4 dir, QmVec4* pv)
{
	// v2.pl<-v1
	float dot = QMVECTOR_DOT3F(plane.v, dir.v);
	if (qm_eqf(dot, 0.0f))
	{
		if (pv)
			pv->v = QMC_ZERO.v;
		return false;
	}
	if (pv)
	{
		float tmp = (plane.D + QMVECTOR_DOT3F(plane.v, loc.v)) / dot;
		pv->X = loc.X - tmp * dir.X;
		pv->Y = loc.Y - tmp * dir.Y;
		pv->Z = loc.Z - tmp * dir.Z;
	}
	return true;
}

/// @brief 세 면이 충돌 하면 참
/// @param plane 기준 면
/// @param other1 대상 면1
/// @param other2 대상 면2
/// @param pv 충돌 위치 (반환, 널가능)
/// @return 충돌하면 참
INLINE bool QM_VECTORCALL qm_plane_intersect_planes(QmPlane plane, QmPlane other1, QmPlane other2, QmVec4 * pv)
{
	QmVec4 dir, loc;
	return (qm_plane_intersect(plane, other1, &loc, &dir)) ? qm_plane_intersect_line(other2, loc, dir, pv) : false;
}

/// @brief 이거 뭔지 기억이 안난다. 뭐에 쓰던거지. 기본적으로 qm_plane_intersect_line 에다 방향 벡터와의 거리 계산 추가
/// @param plane 면
/// @param v1 벡터3 첫째 점
/// @param v2 벡터3 둘째 점
/// @param pv 충돌 지점 (반환, 널가능)
/// @return 충돌하면서 방향 벡터의 거리 안쪽(?)이면 참
INLINE bool QM_VECTORCALL qm_plane_intersect_between_point(QmPlane plane, QmVec4 v1, QmVec4 v2, QmVec4* pv)
{
	QmVec4 dir = qm_vec4_sub(v2, v1);
	QmVec4 point;
	if (!qm_plane_intersect_line(plane, v1, dir, &point))
		return false;
	float f = QMVECTOR_DOT3F(dir.v, dir.v);
	if (pv)
		pv->v = point.v;
	return QMVECTOR_DOT3F(point.v, v1.v) <= f && QMVECTOR_DOT3F(point.v, v2.v) >= 0.0f;
}

/// @brief 벡터가 0인지 비교
/// @param p 비교할 면
/// @return 면가 0이면 참
INLINE bool QM_VECTORCALL qm_plane_isu(QmPlane p)
{
	return QMVECTOR_EQ_B(p.v, QMC_UNIT_R3.v);
}


//////////////////////////////////////////////////////////////////////////
// 행렬

/// @brief 단위 행렬 (항등 행렬)
INLINE QmMat4 QM_VECTORCALL qm_mat4_unit(void)
{
	QmMat4 m;
	m.v[0] = QMC_UNIT_R0.v;
	m.v[1] = QMC_UNIT_R1.v;
	m.v[2] = QMC_UNIT_R2.v;
	m.v[3] = QMC_UNIT_R3.v;
	return m;
}

/// @brief 행렬을 0으로 초기화 한다
INLINE QmMat4 QM_VECTORCALL qm_mat4_zero(void)
{
	QmMat4 m;
	m.v[0] = QMC_ZERO.v;
	m.v[1] = QMC_ZERO.v;
	m.v[2] = QMC_ZERO.v;
	m.v[3] = QMC_ZERO.v;
	return m;
}

/// @brief 대각 행렬을 만든다
/// @param diag 대각값
INLINE QmMat4 QM_VECTORCALL qm_mat4_diag(float diag)
{
	return (QmMat4) { ._11 = diag, ._22 = diag, ._33 = diag, ._44 = diag, };
}

/// @brief 행렬 전치
/// @param m 전치할 행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_tran(QmMat4 m)
{
#if defined QM_USE_AVX
	QMVECTOR r0 = _mm_shuffle_ps(m.v[0], m.v[1], 0x44);
	QMVECTOR r2 = _mm_shuffle_ps(m.v[0], m.v[1], 0xEE);
	QMVECTOR r1 = _mm_shuffle_ps(m.v[2], m.v[3], 0x44);
	QMVECTOR r3 = _mm_shuffle_ps(m.v[2], m.v[3], 0xEE);
	return (QmMat4) { .v[0] = _mm_shuffle_ps(r0, r1, 0x88), .v[1] = _mm_shuffle_ps(r0, r1, 0xDD), .v[2] = _mm_shuffle_ps(r2, r3, 0x88), .v[3] = _mm_shuffle_ps(r2, r3, 0xDD) };
#elif defined QM_USE_NEON
	float32x4x4_t t = vld4q_f32(m.f);
	return (QmMat4) { .v[0] = t.val[0], .v[1] = t.val[1], .v[2] = t.val[2], .v[3] = t.val[3] };
#else
	return (QmMat4) { {  m._11, m._21, m._31, m._41, m._12, m._22, m._32, m._42, m._13, m._23, m._33, m._43, m._14, m._24, m._34, m._44 } };
#endif
}

/// @brief 두 행렬의 덧셈
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_add(QmMat4 left, QmMat4 right)
{
	QmMat4 m;
	m.v[0] = QMVECTOR_ADD(left.v[0], right.v[0]);
	m.v[1] = QMVECTOR_ADD(left.v[1], right.v[1]);
	m.v[2] = QMVECTOR_ADD(left.v[2], right.v[2]);
	m.v[3] = QMVECTOR_ADD(left.v[3], right.v[3]);
	return m;
}

/// @brief 두 행렬의 뺄셈
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_sub(QmMat4 left, QmMat4 right)
{
	QmMat4 m;
	m.v[0] = QMVECTOR_SUB(left.v[0], right.v[0]);
	m.v[1] = QMVECTOR_SUB(left.v[1], right.v[1]);
	m.v[2] = QMVECTOR_SUB(left.v[2], right.v[2]);
	m.v[3] = QMVECTOR_SUB(left.v[3], right.v[3]);
	return m;
}

/// @brief 행렬의 확대
/// @param m 대상 행렬
/// @param scale 확대값
INLINE QmMat4 QM_VECTORCALL qm_mat4_mag(QmMat4 m, float scale)
{
	QmMat4 r;
#if defined QM_USE_AVX
	QMVECTOR mm = _mm_set1_ps(scale);
	r.v[0] = _mm_mul_ps(m.v[0], mm);
	r.v[1] = _mm_mul_ps(m.v[1], mm);
	r.v[2] = _mm_mul_ps(m.v[2], mm);
	r.v[3] = _mm_mul_ps(m.v[3], mm);
#elif defined QM_USE_NEON
	r.v[0] = vmulq_n_f32(m.v[0], scale);
	r.v[1] = vmulq_n_f32(m.v[1], scale);
	r.v[2] = vmulq_n_f32(m.v[2], scale);
	r.v[3] = vmulq_n_f32(m.v[3], scale);
#else
	r.v[0] = QMVECTOR_MAG(m.v[0], scale);
	r.v[1] = QMVECTOR_MAG(m.v[1], scale);
	r.v[2] = QMVECTOR_MAG(m.v[2], scale);
	r.v[3] = QMVECTOR_MAG(m.v[3], scale);
#endif
	return r;
}

/// @brief 행렬의 줄이기
/// @param m 대상 행렬
/// @param scale 줄일값
INLINE QmMat4 QM_VECTORCALL qm_mat4_abr(QmMat4 m, float scale)
{
	QmMat4 r;
#if defined QM_USE_AVX
	QMVECTOR mm = _mm_set1_ps(scale);
	r.v[0] = _mm_div_ps(m.v[0], mm);
	r.v[1] = _mm_div_ps(m.v[1], mm);
	r.v[2] = _mm_div_ps(m.v[2], mm);
	r.v[3] = _mm_div_ps(m.v[3], mm);
#elif defined QM_USE_NEON
	QMVECTOR mm = vdupq_n_f32(scale);
	r.v[0] = vdivq_f32(m.v[0], mm);
	r.v[1] = vdivq_f32(m.v[1], mm);
	r.v[2] = vdivq_f32(m.v[2], mm);
	r.v[3] = vdivq_f32(m.v[3], mm);
#else
	r.v[0] = QMVECTOR_MAG(m.v[0], scale);
	r.v[1] = QMVECTOR_MAG(m.v[1], scale);
	r.v[2] = QMVECTOR_MAG(m.v[2], scale);
	r.v[3] = QMVECTOR_MAG(m.v[3], scale);
#endif
	return r;
}

/// @brief 행렬 곱
/// @param left 좌측 행렬
/// @param right 우측 행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_mul(QmMat4 left, QmMat4 right)
{
#if defined QM_USE_AVX
	register QMVECTOR r1, r2, b1, b2, b3, b4;
	QmMat4 m;
	b1 = _mm_loadu_ps(&right._11);
	b2 = _mm_loadu_ps(&right._21);
	b3 = _mm_loadu_ps(&right._31);
	b4 = _mm_loadu_ps(&right._41);
	r1 = _mm_mul_ps(_mm_set_ps1(left._11), b1);
	r2 = _mm_mul_ps(_mm_set_ps1(left._21), b1);
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left._12), b2));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left._22), b2));
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left._13), b3));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left._23), b3));
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left._14), b4));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left._24), b4));
	_mm_storeu_ps(&m._11, r1);
	_mm_storeu_ps(&m._21, r2);
	r1 = _mm_mul_ps(_mm_set_ps1(left._31), b1);
	r2 = _mm_mul_ps(_mm_set_ps1(left._41), b1);
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left._32), b2));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left._42), b2));
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left._33), b3));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left._43), b3));
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left._34), b4));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left._44), b4));
	_mm_storeu_ps(&m._31, r1);
	_mm_storeu_ps(&m._41, r2);
	return m;
#elif defined QM_USE_NEON
	QMVECTOR l0 = left.v[0];
	QMVECTOR l1 = left.v[1];
	QMVECTOR l2 = left.v[2];
	QMVECTOR l3 = left.v[3];
	QmMat4 m;
	for (int i = 0; i < 4; i++)
	{
		QMVECTOR v, r = right.v[i];
		v = vmulq_n_f32(l0, vgetq_lane_f32(r, 0));
		v = vmlaq_n_f32(v, l1, vgetq_lane_f32(r, 1));
		v = vmlaq_n_f32(v, l2, vgetq_lane_f32(r, 2));
		v = vmlaq_n_f32(v, l3, vgetq_lane_f32(r, 3));
		m.v[i] = v;
	}
	return m;
#else
	QmMat4 m;
	m.v[0] = QMVECTOR_TRFM(right.v[0], left);
	m.v[1] = QMVECTOR_TRFM(right.v[1], left);
	m.v[2] = QMVECTOR_TRFM(right.v[2], left);
	m.v[3] = QMVECTOR_TRFM(right.v[3], left);
	return m;
#endif
}

#if defined QM_USE_AVX
QSAPI QmMat4 QM_VECTORCALL qm_sse_mat4_inv(QmMat4 m);
QSAPI float QM_VECTORCALL qm_sse_mat4_det(QmMat4 m);
#endif
#if defined QM_USE_NEON
QSAPI QmMat4 QM_VECTORCALL qm_neon_mat4_inv(QmMat4 m);
QSAPI float QM_VECTORCALL qm_neon_mat4_det(QmMat4 m);
#endif

/// @brief 역행렬
/// @param m 입력 행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_inv(QmMat4 m)
{
#if defined QM_USE_AVX
	return qm_sse_mat4_inv(m);
#elif defined QM_USE_NEON
	return qm_neon_mat4_inv(m);
#else
	QMVECTOR c01 = QMVECTOR_CROSS3(m.v[0], m.v[1]);
	QMVECTOR c23 = QMVECTOR_CROSS3(m.v[2], m.v[3]);
	QMVECTOR s10 = QMVECTOR_SUB(QMVECTOR_MAG(m.v[0], m._24), QMVECTOR_MAG(m.v[1], m._14));
	QMVECTOR s32 = QMVECTOR_SUB(QMVECTOR_MAG(m.v[2], m._44), QMVECTOR_MAG(m.v[3], m._34));
	QMVECTOR inv = QMVECTOR_DIV(QMC_ONE_V4.v, QMVECTOR_ADD(QMVECTOR_DOT3(c01, s32), QMVECTOR_DOT3(c23, s10)));
	c01 = QMVECTOR_MUL(c01, inv);
	c23 = QMVECTOR_MUL(c23, inv);
	s10 = QMVECTOR_MUL(s10, inv);
	s32 = QMVECTOR_MUL(s32, inv);
	QmMat4 r;
	r.v[0] = QMVECTOR_ADD(QMVECTOR_CROSS3(m.v[1], s32), QMVECTOR_MAG(c23, m._24));
	r.v[1] = QMVECTOR_SUB(QMVECTOR_CROSS3(s32, m.v[0]), QMVECTOR_MAG(c23, m._14));
	r.v[2] = QMVECTOR_ADD(QMVECTOR_CROSS3(m.v[3], s10), QMVECTOR_MAG(c01, m._44));
	r.v[3] = QMVECTOR_SUB(QMVECTOR_CROSS3(s10, m.v[2]), QMVECTOR_MAG(c01, m._34));
	r._14 = -QMVECTOR_DOT3F(m.v[1], c23);
	r._24 = +QMVECTOR_DOT3F(m.v[0], c23);
	r._34 = -QMVECTOR_DOT3F(m.v[3], c01);
	r._44 = +QMVECTOR_DOT3F(m.v[2], c01);
	return qm_mat4_tran(r);
#endif
}

/// @brief 행렬식
/// @param m 행렬
/// @return 행렬식
INLINE float QM_VECTORCALL qm_mat4_det(QmMat4 m)
{
#if defined QM_USE_AVX
	return qm_sse_mat4_det(m);
#elif defined QM_USE_NEON
	return qm_neon_mat4_det(m);
#else
	QMVECTOR c01 = QMVECTOR_CROSS3(m.v[0], m.v[1]);
	QMVECTOR c23 = QMVECTOR_CROSS3(m.v[2], m.v[3]);
	QMVECTOR s10 = QMVECTOR_SUB(QMVECTOR_MAG(m.v[0], m._24), QMVECTOR_MAG(m.v[1], m._14));
	QMVECTOR s32 = QMVECTOR_SUB(QMVECTOR_MAG(m.v[2], m._44), QMVECTOR_MAG(m.v[3], m._34));
	return QMVECTOR_DOT3F(c01, s32) + QMVECTOR_DOT3F(c23, s10);
#endif
}

/// @brief 전치곱
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
INLINE QmMat4 QM_VECTORCALL qm_mat4_tmul(QmMat4 left, QmMat4 right)
{
	return qm_mat4_tran(qm_mat4_mul(left, right));
}

/// @brief 보기 행렬을 만든다 (왼손 기준)
/// @param eye 시선의 위치
/// @param dir 시선의 방향
/// @param up 윗쪽 방향
INLINE QmMat4 QM_VECTORCALL qm_mat4_tolook(QMVECTOR eye, QMVECTOR dir, QMVECTOR up)
{
	QMVECTOR r2 = QMVECTOR_NORM4(dir);
	QMVECTOR r0 = QMVECTOR_NORM4(QMVECTOR_CROSS3(up, r2));
	QMVECTOR r1 = QMVECTOR_CROSS3(r2, r0);
	QMVECTOR r3 = QMVECTOR_NEG(eye);
	QMVECTOR d0 = QMVECTOR_DOT3(r0, r3);
	QMVECTOR d1 = QMVECTOR_DOT3(r1, r3);
	QMVECTOR d2 = QMVECTOR_DOT3(r2, r3);
	QmMat4 m;
	m.v[0] = QMVECTOR_SELECT(d0, r0, QMC_S1110.v);
	m.v[1] = QMVECTOR_SELECT(d1, r1, QMC_S1110.v);
	m.v[2] = QMVECTOR_SELECT(d2, r2, QMC_S1110.v);
	m.v[3] = QMC_UNIT_R3.v;
	return qm_mat4_tran(m);
}

/// @brief 보기 행렬을 만든다 (왼손 기준)
/// @param eye 시선의 위치
/// @param at 바라보는 방향
/// @param up 시선의 윗쪽 방향
INLINE QmMat4 QM_VECTORCALL qm_mat4_lookat_lh(QmVec4 eye, QmVec4 at, QmVec4 up)
{
	QMVECTOR dir = QMVECTOR_SUB(at.v, eye.v);
	return qm_mat4_tolook(eye.v, dir, up.v);
}

/// @brief 보기 행렬을 만든다 (오른손 기준)
/// @param eye 시선의 위치
/// @param at 바라보는 방향
/// @param up 시선의 윗쪽 방향
/// @return
INLINE QmMat4 QM_VECTORCALL qm_mat4_lookat_rh(QmVec4 eye, QmVec4 at, QmVec4 up)
{
	QMVECTOR dir = QMVECTOR_SUB(eye.v, at.v);
	return qm_mat4_tolook(eye.v, dir, up.v);
}

/// @brief 투영 행렬을 만든다 (왼손 기준)
/// @param fov 포브(Field Of View)값
/// @param aspect 화면 종횡비(가로 나누기 세로)
/// @param zn,zf 뎁스 너비
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
/// @param fov 포브(Field Of View)값
/// @param aspect 화면 종횡비(가로 나누기 세로)
/// @param zn,zf 뎁스 너비
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
/// @param width 너비
/// @param height 높이
/// @param zn 깊이 가까운곳
/// @param zf 깊이 먼곳
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
/// @param width 너비
/// @param height 높이
/// @param zn 깊이 가까운곳
/// @param zf 깊이 먼곳
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

/// @brief 사각형을 준으로 정규 행렬을 만든다 (왼손 기준)
/// @param left 사각형의 왼쪽
/// @param top 사각형의 윗쪽
/// @param right 사각형의 오른쪽
/// @param bottom 사각형의 아래쪽
/// @param zn 깊이 가까운곳
/// @param zf 깊이 먼곳
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

/// @brief 사각형을 준으로 정규 행렬을 만든다 (오른손 기준)
/// @param left 사각형의 왼쪽
/// @param top 사각형의 윗쪽
/// @param right 사각형의 오른쪽
/// @param bottom 사각형의 아래쪽
/// @param zn 깊이 가까운곳
/// @param zf 깊이 먼곳
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

/// @brief 스케일 행렬을 만든다
/// @param x,y,z 각 축 별 스케일 값
INLINE QmMat4 QM_VECTORCALL qm_mat4_scl(float x, float y, float z)
{
	QmMat4 r = { ._11 = x, ._22 = y, ._33 = z, ._44 = 1.0f, };
	return r;
}

/// @brief 스케일 행렬을 만든다
/// @param v 스케일 벡터
INLINE QmMat4 QM_VECTORCALL qm_mat4_scl_vec(QmVec4 v)
{
	return qm_mat4_scl(v.X, v.Y, v.Z);
}

/// @brief 위치 행렬을 만든다
/// @param x,y,z 좌표
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
/// @param v 좌표 벡터
INLINE QmMat4 QM_VECTORCALL qm_mat4_loc_vec(QmVec4 v)
{
	return qm_mat4_loc(v.X, v.Y, v.Z);
}

/// @brief 회전 행렬을 만든다
/// @param angle 회전 각도
/// @param axis 벡터3 회전 축
INLINE QmMat4 QM_VECTORCALL qm_mat4_rot(float angle, QmVec4 axis)
{
	QmVec4 norm = (QmVec4){ .v = QMVECTOR_NORM3(axis.v) };
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

/// @brief 회전 행렬을 만든다 (Roll/Pitch/Yaw)
/// @param rot 벡터3 회전값
INLINE QmMat4 QM_VECTORCALL qm_mat4_vec(QmVec4 rot)
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
/// @param rot 사원수 회전값
/// @return
INLINE QmMat4 QM_VECTORCALL qm_mat4_quat(QmQuat rot)
{
	QmQuat norm = qm_vec4_norm(rot);
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
}

/// @brief X축 회전 행렬을 만든다
/// @param rot X측 회전값
INLINE QmMat4 QM_VECTORCALL qm_mat4_x(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
#if defined QM_USE_AVX
	QmMat4 r;
	QMVECTOR vs = _mm_set1_ps(s);
	QMVECTOR vc = _mm_set1_ps(c);
	vc = _mm_shuffle_ps(vc, vs, _MM_SHUFFLE(3, 0, 0, 3));
	r.v[0] = QMC_UNIT_R0.v;
	r.v[1] = vc;
	vc = _mm_permute_ps(vc, _MM_SHUFFLE(3, 1, 2, 0));
	vc = _mm_mul_ps(vc, QMC_NEG_Y.v);
	r.v[2] = vc;
	r.v[3] = QMC_UNIT_R3.v;
#else
	QmMat4 r =
	{
		._11 = 1.0f,
		._22 = c, ._23 = s,
		._32 = -s, ._33 = c,
		._44 = 1.0f,
	};
#endif
	return r;
}

/// @brief Y축 회전 행렬을 만든다
/// @param rot Y측 회전값
INLINE QmMat4 QM_VECTORCALL qm_mat4_y(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
#if defined QM_USE_AVX
	QmMat4 r;
	QMVECTOR vs = _mm_set1_ps(s);
	QMVECTOR vc = _mm_set1_ps(c);
	vs = _mm_shuffle_ps(vs, vc, _MM_SHUFFLE(3, 0, 3, 0));
	r.v[2] = vs;
	r.v[1] = QMC_UNIT_R1.v;
	vs = _mm_permute_ps(vs, _MM_SHUFFLE(3, 0, 1, 2));
	vs = _mm_mul_ps(vs, QMC_NEG_Z.v);
	r.v[0] = vs;
	r.v[3] = QMC_UNIT_R3.v;
#else
	QmMat4 r =
	{
		._11 = c, ._13 = -s,
		._22 = 1.0f,
		._31 = s, ._33 = c,
		._44 = 1.0f,
	};
#endif
	return r;
}

/// @brief Z축 회전 행렬을 만든다
/// @param rot Z측 회전값
INLINE QmMat4 QM_VECTORCALL qm_mat4_z(float rot)
{
	float s, c;
	qm_sincosf(rot, &s, &c);
#if defined QM_USE_AVX
	QmMat4 r;
	QMVECTOR vs = _mm_set1_ps(s);
	QMVECTOR vc = _mm_set1_ps(c);
	vc = _mm_unpacklo_ps(vc, vs);
	r.v[0] = vc;
	vc = _mm_permute_ps(vc, _MM_SHUFFLE(3, 2, 0, 1));
	vc = _mm_mul_ps(vc, QMC_NEG_X.v);
	r.v[1] = vc;
	r.v[2] = QMC_UNIT_R2.v;
	r.v[3] = QMC_UNIT_R3.v;
#else
	QmMat4 r =
	{
		._11 = c, ._12 = s,
		._21 = -s, ._22 = c,
		._33 = 1.0f,
		._44 = 1.0f,
	};
#endif
	return r;
}

/// @brief 그림자 행렬을 만든다
/// @param light 빛의 방향
/// @param plane 투영될 면
INLINE QmMat4 QM_VECTORCALL qm_mat4_shadow(QmVec4 light, QmPlane plane)
{
	float d = QMVECTOR_DOT3F(plane.v, light.v);
	if (qm_eqf(light.W, 0.0f))
	{
		QmMat4 r =
		{
			._11 = -plane.A * light.X + d,
			._12 = -plane.A * light.Y,
			._13 = -plane.A * light.Z,
			._21 = -plane.B * light.X,
			._22 = -plane.B * light.Y + d,
			._23 = -plane.B * light.Z,
			._31 = -plane.C * light.X,
			._32 = -plane.C * light.Y,
			._33 = -plane.C * light.Z + d,
			._41 = -plane.D * light.X,
			._42 = -plane.D * light.Y,
			._43 = -plane.D * light.Z,
			._44 = d,
		};
		return r;
	}
	QmMat4 r =
	{
		._11 = -plane.A * light.X + d,
		._12 = -plane.A * light.Y,
		._13 = -plane.A * light.Z,
		._14 = -plane.A * light.W,
		._21 = -plane.B * light.X,
		._22 = -plane.B * light.Y + d,
		._23 = -plane.B * light.Z,
		._24 = -plane.B * light.W,
		._31 = -plane.C * light.X,
		._32 = -plane.C * light.Y,
		._33 = -plane.C * light.Z + d,
		._34 = -plane.C * light.W,
		._41 = -plane.D * light.X,
		._42 = -plane.D * light.Y,
		._43 = -plane.D * light.Z,
		._44 = -plane.D * light.W + d,
	};
	return r;
}

/// @brief 아핀 변환 행렬
/// @param scl 벡터3 스케일 (1일 경우 NULL)
/// @param rotcenter 벡터3 회전축(원점일 경우 NULL)
/// @param rot 사원수 회전 (고정일 경우 NULL)
/// @param loc 벡터3 위치 (원점일 경우 NULL)
INLINE QmMat4 QM_VECTORCALL qm_mat4_affine(QmVec4 * scl, QmVec4* rotcenter, QmQuat* rot, QmVec4* loc)
{
	QmMat4 m1 = scl ? qm_mat4_scl_vec(*scl) : qm_mat4_unit();
	QmMat4 m2 = rotcenter ? qm_mat4_loc(-rotcenter->X, -rotcenter->Y, -rotcenter->Z) : qm_mat4_unit();
	QmMat4 m4 = rotcenter ? qm_mat4_loc_vec(*rotcenter) : qm_mat4_unit();
	QmMat4 m3 = rot ? qm_mat4_quat(*rot) : qm_mat4_unit();
	QmMat4 m5 = loc ? qm_mat4_loc_vec(*loc) : qm_mat4_unit();
	QmMat4 m = qm_mat4_mul(m1, m2);
	m = qm_mat4_mul(m, m3);
	m = qm_mat4_mul(m, m4);
	return qm_mat4_mul(m, m5);
}

//
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm_loc_scl(QmMat4 m, QmVec4 loc, QmVec4* scl)
{
	float* f = m.f;
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
	return m;
}

/// @brief 행렬 트랜스폼
/// @param loc 벡터3 위치
/// @param rot 사원수 회전
/// @param scl 벡터3 스케일 (1일 경우 NULL)
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm(QmVec4 loc, QmQuat rot, QmVec4* scl)
{
	QmMat4 m = qm_mat4_quat(rot);
	return qm_mat4_trfm_loc_scl(m, loc, scl);
}

/// @brief 행렬 트랜스폼. 단 벡터 회전
/// @param loc 벡터3 위치
/// @param rot 벡터3 회전
/// @param scl 벡터3 스케일 (1일 경우 NULL)
INLINE QmMat4 QM_VECTORCALL qm_mat4_trfm_vec(QmVec4 loc, QmVec4 rot, QmVec4* scl)
{
	QmMat4 m = qm_mat4_vec(rot);
	return qm_mat4_trfm_loc_scl(m, loc, scl);
}

/// @brief 단위 행렬인지 비교
/// @param m 비교할 행렬
/// @return 단위 행렬이면 참을 반환
INLINE bool QM_VECTORCALL qm_mat4_isu(QmMat4 m)
{
#if defined QM_USE_AVX
	QMVECTOR r1 = _mm_cmpeq_ps(m.v[0], QMC_UNIT_R0.v);
	QMVECTOR r2 = _mm_cmpeq_ps(m.v[1], QMC_UNIT_R1.v);
	QMVECTOR r3 = _mm_cmpeq_ps(m.v[2], QMC_UNIT_R2.v);
	QMVECTOR r4 = _mm_cmpeq_ps(m.v[3], QMC_UNIT_R3.v);
	r1 = _mm_and_ps(r1, r2);
	r3 = _mm_and_ps(r3, r4);
	r1 = _mm_and_ps(r1, r3);
	return (_mm_movemask_ps(r1) == 0x0f);
#elif defined QM_USE_NEON
	uint32x4_t r1 = vceqq_f32(m.v[0], QMC_UNIT_R0.v);
	uint32x4_t r2 = vceqq_f32(m.v[1], QMC_UNIT_R1.v);
	uint32x4_t r3 = vceqq_f32(m.v[2], QMC_UNIT_R2.v);
	uint32x4_t r4 = vceqq_f32(m.v[3], QMC_UNIT_R3.v);
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
// 색깔

/// @brief 색깔 값 설정
/// @param r,g,b,a 색깔 요소
/// @return 만든 색깔
INLINE QmColor QM_VECTORCALL qm_color(float r, float g, float b, float a)
{
	return (QmColor) { .v = QMVECTOR_SET(r, g, b, a) };
}

/// @brief 32비트 RGBA 정수로 색깔을 설정한다
/// @param value 32비트 RGBA 정수
INLINE QmColor QM_VECTORCALL qm_coloru(uint value)
{
	float f = 1.0f / 255.0f;
	QmColor c;
	c.B = (float)(value & 255) * f; value >>= 8;
	c.G = (float)(value & 255) * f; value >>= 8;
	c.R = (float)(value & 255) * f; value >>= 8;
	c.A = (float)(value & 255) * f;
	return c;
}

/// @brief 정수형 색깔로 색깔을 설정한다
/// @param cu 정수형 색깔
INLINE QmColor QM_VECTORCALL qm_colork(QmKolor cu)
{
	float f = 1.0f / 255.0f;
	QmColor c;
	c.A = (float)cu.A * f;
	c.R = (float)cu.R * f;
	c.G = (float)cu.G * f;
	c.B = (float)cu.B * f;
	return c;
}

/// @brief 색깔 초기화(즉, 깜장. 알파는 1)
INLINE QmColor QM_VECTORCALL qm_color_unit(void)		// identify
{
	return (QmColor) { .v = QMC_UNIT_R3.v };
}

/// @brief 색깔 대각값 설정 (모든 요소를 같은 값을)
///	@param c 반환 색깔
/// @param diag 대각값
/// @param alpha 알파값
INLINE QmColor QM_VECTORCALL qm_color_diag(float diag, float alpha)
{
	return (QmColor) { .v = QMVECTOR_SET(diag, diag, diag, alpha) };
}

/// @brief 색깔 네거티브
/// @param c 원본 색깔
INLINE QmColor QM_VECTORCALL qm_color_neg(QmColor c)
{
#if defined QM_USE_AVX
	QMVECTOR t = _mm_xor_ps(c.v, QMC_NEG_V3.v);
	return (QmColor) { .v = _mm_add_ps(t, QMC_ONE_V3.v) };
#elif defined QM_USE_NEON
	uint32x4_t t = veorq_u32(vreinterpretq_u32_f32(c.v), QMC_NEG_V3.v);
	return (QmColor) { .v = vaddq_f32(vreinterpretq_f32_u32(t), QMC_ONE_V3.v) };
#else
	return qm_color(1.0f - c.R, 1.0f - c.G, 1.0f - c.B, c.A);
#endif
}

/// @brief 색깔 항목 곱셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
INLINE QmColor QM_VECTORCALL qm_color_mod(QmColor left, QmColor right)
{
	return (QmColor) { .v = QMVECTOR_MUL(left.v, right.v) };
}

/// @brief 색깔 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 색깔
/// @param right 대상 색깔
/// @param scale 보간값
INLINE QmColor QM_VECTORCALL qm_color_interpolate(QmColor left, QmColor right, float scale)
{
	QMVECTOR l = QMVECTOR_MAG(left.v, 1.0f - scale);
	QMVECTOR r = QMVECTOR_MAG(right.v, scale);
	return (QmColor) { .v = QMVECTOR_ADD(l, r) };
}

/// @brief 색깔 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 색깔
/// @param right 대상 색깔
/// @param scale 보간값
INLINE QmColor QM_VECTORCALL qm_color_lerp(QmColor left, QmColor right, float scale)
{
	QMVECTOR s = QMVECTOR_SUB(right.v, left.v);
	QMVECTOR m = QMVECTOR_MAG(s, scale);
	return (QmColor) { .v = QMVECTOR_ADD(left.v, m) };
}

/// @brief 색깔의 콘트라스트를 조정한다
/// @param c 원본 색깔
/// @param scale 조정값
INLINE QmColor QM_VECTORCALL qm_color_contrast(QmColor c, float scale)
{
	return qm_color(
		0.5f + scale * (c.R - 0.5f),
		0.5f + scale * (c.G - 0.5f),
		0.5f + scale * (c.B - 0.5f),
		c.A);
}

/// @brief 색깔의 새츄레이션을 조정한다
/// @param c 원본 색깔
/// @param scale 조정값
INLINE QmColor QM_VECTORCALL qm_color_saturation(QmColor c, float scale)
{
	float g = c.R * 0.2125f + c.G * 0.7154f + c.B * 0.0721f;
	return qm_color(
		g + scale * (c.R - g),
		g + scale * (c.G - g),
		g + scale * (c.B - g),
		c.A);
}

/// @brief 색깔을 32비트 정수로 만든다 (빠른 버전)
/// @param c 대상 색깔
/// @return 32비트 정수
INLINE uint QM_VECTORCALL qm_color_to_uint(QmColor c)
{
	const byte R = (byte)(c.R * 255.0f + 0.5f);
	const byte G = (byte)(c.G * 255.0f + 0.5f);
	const byte B = (byte)(c.B * 255.0f + 0.5f);
	const byte A = (byte)(c.A * 255.0f + 0.5f);
	return ((uint)A << 24) | ((uint)R << 16) | ((uint)G << 8) | (uint)B;
}

/// @brief 색깔을 32비트 정수로 만든다
/// @param c 대상 색깔
/// @return 32비트 정수
INLINE uint QM_VECTORCALL qm_color_to_uint_check(QmColor c)
{
	const byte R = (c.R >= 1.0f) ? 0xff : (c.R <= 0.0f) ? 0x00 : (byte)(c.R * 255.0f + 0.5f);	// NOLINT
	const byte G = (c.G >= 1.0f) ? 0xff : (c.G <= 0.0f) ? 0x00 : (byte)(c.G * 255.0f + 0.5f);	// NOLINT
	const byte B = (c.B >= 1.0f) ? 0xff : (c.B <= 0.0f) ? 0x00 : (byte)(c.B * 255.0f + 0.5f);	// NOLINT
	const byte A = (c.A >= 1.0f) ? 0xff : (c.A <= 0.0f) ? 0x00 : (byte)(c.A * 255.0f + 0.5f);	// NOLINT
	return ((uint)A << 24) | ((uint)R << 16) | ((uint)G << 8) | (uint)B;
}

/// @brief 두 색깔를 비교
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
/// @return 두 색깔가 같으면 참
INLINE bool QM_VECTORCALL qm_color_eq(QmColor left, QmColor right)
{
	return QMVECTOR_EQ_B(left.v, right.v);
}


//////////////////////////////////////////////////////////////////////////
// 정수형 8비트 색깔

/// @brief 색깔을 설정한다
/// @param r 빨강
/// @param g 녹색
/// @param b 파랑
/// @param a 알파
INLINE QmKolor qm_kolor(const byte r, const byte g, const byte b, const byte a)
{
	return (QmKolor) { { r, g, b, a } };
}

/// @brief 색깔을 설정한다
/// @param r 빨강
/// @param g 녹색
/// @param b 파랑
/// @param a 알파
INLINE QmKolor qm_kolorf(float r, float g, float b, float a)
{
	return (QmKolor) { { (byte)(r * 255.0f), (byte)(g * 255.0f), (byte)(b * 255.0f), (byte)(a * 255.0f) } };
}

/// @brief 색깔을 설정한다
/// @param value 32비트 RGBA 정수
INLINE QmKolor qm_koloru(const uint value)
{
	return (QmKolor) { .U = value };
}

/// @brief 색깔을 설정한다
/// @param cr 실수형 색깔
INLINE QmKolor QM_VECTORCALL qm_kolorc(QmColor cr)
{
	return (QmKolor) { { (byte)(cr.R * 255.0f), (byte)(cr.G * 255.0f), (byte)(cr.B * 255.0f), (byte)(cr.A * 255.0f) } };
}

/// @brief 두 색깔의 덧셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
/// @return
INLINE QmKolor qm_kolor_add(QmKolor left, QmKolor right)
{
	return (QmKolor) { { left.R + right.R, left.G + right.G, left.B + right.B, left.A + right.A } };
}

/// @brief 두 색깔의 뺄셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
/// @return
INLINE QmKolor qm_kolor_sub(QmKolor left, QmKolor right)
{
	return (QmKolor) { { left.R - right.R, left.G - right.G, left.B - right.B, left.A - right.A } };
}

/// @brief 색깔을 확대한다 (밝기가 아니다)
/// @param left 왼쪽 색깔
/// @param scale 확대값
INLINE QmKolor qm_kolor_mag(QmKolor left, float scale)
{
	return (QmKolor) { { (byte)((float)left.R * scale), (byte)((float)left.G * scale), (byte)((float)left.B * scale), (byte)((float)left.A * scale) } };
}

/// @brief 네거티브 색깔을 만든다
/// @param c 원본 색깔
INLINE QmKolor qm_kolor_neg(QmKolor c)
{
	return (QmKolor) { { 255 - c.R, 255 - c.G, 255 - c.B, c.A } };
}

/// @brief 두 색깔의 곱셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
INLINE QmKolor qm_kolor_mul(QmKolor left, QmKolor right)
{
	return (QmKolor) { { left.R * right.R, left.G * right.G, left.B * right.B, left.A * right.A } };
}

/// @brief 두 색깔의 나눗셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
INLINE QmKolor qm_kolor_div(QmKolor left, QmKolor right)
{
	return (QmKolor) { { left.R / right.R, left.G / right.G, left.B / right.B, left.A / right.A } };
}

/// @brief 색깔을 보간한다
/// @param left 시작 색깔
/// @param right 끝 색깔
/// @param scale 보간값
INLINE QmKolor qm_kolor_interpolate(QmKolor left, QmKolor right, float scale)
{
	float f = 1.0f - scale;
	QmKolor k;
	k.R = (byte)((float)left.R * f + (float)right.R * scale);
	k.G = (byte)((float)left.G * f + (float)right.G * scale);
	k.B = (byte)((float)left.B * f + (float)right.B * scale);
	k.A = (byte)((float)left.A * f + (float)right.A * scale);
	return k;
}

/// @brief 색깔을 선형 보간한다
/// @param left 시작 색깔
/// @param right 끝 색깔
/// @param scale 보간값
/// @return
INLINE QmKolor qm_kolor_lerp(QmKolor left, QmKolor right, float scale)
{
	QmKolor k;
	k.R = (byte)((float)left.R + scale * (float)(right.R - left.R));
	k.G = (byte)((float)left.G + scale * (float)(right.G - left.G));
	k.B = (byte)((float)left.B + scale * (float)(right.B - left.B));
	k.A = (byte)((float)left.A + scale * (float)(right.A - left.A));
	return k;
}

/// @brief 두 색깔이 같은지 비교한다
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
/// @return 두 색깔이 같으면 참
INLINE bool qm_kolor_eq(QmKolor left, QmKolor right)
{
	return left.U == right.U;
}


//////////////////////////////////////////////////////////////////////////
// 정수 벡터2, 점

/// @brief 점 설정
/// @param x,y 좌표
INLINE QmPoint qm_point(int x, int y)
{
	return (QmPoint) { { x, y } };
}

/// @brief 점 초기화
INLINE QmPoint qm_point_zero(void)		// identify
{
	static const QmPoint zero = { { 0, 0 } };
	return zero;
}

/// @brief 점 대각값 설정 (모두 같은값으로 설정)
/// @param pt 반환 점
/// @param diag 대각 값
INLINE QmPoint qm_point_diag(const int diag)
{
	return (QmPoint) { { diag, diag } };
}

/// @brief 점 반전
/// @param p 점
INLINE QmPoint qm_point_neg(QmPoint p)  // invert
{
	return (QmPoint) { { -p.X, -p.Y } };
}

/// @brief 점 덧셈
/// @param left 왼쪽 점
/// @param right 오른쪽 점
INLINE QmPoint qm_point_add(QmPoint left, QmPoint right)
{
	return (QmPoint) { { left.X + right.X, left.Y + right.Y } };
}

/// @brief 점 뺄셈
/// @param left 왼쪽 점
/// @param right 오른쪽 점
INLINE QmPoint qm_point_sub(QmPoint left, QmPoint right)
{
	return (QmPoint) { { left.X - right.X, left.Y - right.Y } };
}

/// @brief 점 확대
/// @param left 왼쪽 점
/// @param right 오른쪽 확대값
INLINE QmPoint qm_point_mag(QmPoint left, const int right)
{
	return (QmPoint) { { left.X * right, left.Y * right } };
}

/// @brief 점 항목 곱셈
/// @param left 왼쪽 점
/// @param right 오른쪽 점
INLINE QmPoint qm_point_mul(QmPoint left, QmPoint right)
{
	return (QmPoint) { { left.X * right.X, left.Y * right.Y } };
}

/// @brief 점 항목 나눗셈
/// @param left 왼쪽 점
/// @param right 오른쪽 점
INLINE QmPoint qm_point_div(QmPoint left, QmPoint right)
{
	return (QmPoint) { { left.X / right.X, left.Y / right.Y } };
}

/// @brief 점의 최소값
/// @param left 왼쪽 점
/// @param right 오른쪽 점
INLINE QmPoint qm_point_min(QmPoint left, QmPoint right)
{
	return (QmPoint) { { QN_MIN(left.X, right.X), QN_MIN(left.Y, right.Y) } };
}

/// @brief 점의 최대값
/// @param left 왼쪽 점
/// @param right 오른쪽 점
INLINE QmPoint qm_point_max(QmPoint left, QmPoint right)
{
	return (QmPoint) { { QN_MAX(left.X, right.X), QN_MAX(left.Y, right.Y) } };
}

/// @brief 점의 외적
/// @param left 왼쪽 점
/// @param right 오른쪽 점
INLINE QmPoint qm_point_cross(QmPoint left, QmPoint right)
{
	return qm_point(left.Y * right.X - left.X * right.Y, left.X * right.Y - left.Y * right.X);
}

/// @brief 점 내적
/// @param left 왼쪽 점
/// @param right 오른쪽 점
/// @return 내적 값
INLINE int qm_point_dot(QmPoint left, QmPoint right)
{
	return left.X * right.X + left.Y * right.Y;
}

/// @brief 점 길이의 제곱
/// @param pt 점
/// @return 길이의 제곱
INLINE int qm_point_len_sq(QmPoint pt)
{
	return qm_point_dot(pt, pt);
}

/// @brief 두 점 거리의 제곱
/// @param left 왼쪽 점
/// @param right 오른쪽 점
/// @return 두 점 거리의 제곱값
INLINE int qm_point_dist_sq(QmPoint left, QmPoint right)
{
	QmPoint t = qm_point_sub(left, right);
	return qm_point_len_sq(t);
}

/// @brief 점 길이
/// @param pt 점
/// @return 길이
INLINE float qm_point_len(QmPoint pt)
{
	return qm_sqrtf((float)qm_point_len_sq(pt));	// NOLINT
}

/// @brief 두 점의 거리
/// @param left 왼쪽 점
/// @param right 오른쪽 점
/// @return 두 점의 거리값
INLINE float qm_point_dist(QmPoint left, QmPoint right)
{
	return qm_sqrtf((float)qm_point_dist_sq(left, right));		// NOLINT
}

/// @brief 점의 비교
/// @param left 왼쪽 점
/// @param right 오른쪽 벡트
/// @return 같으면 참
INLINE bool qm_point_eq(QmPoint left, QmPoint right)
{
	return left.X == right.X && left.Y == right.Y;
}

/// @brief 점가 0인가 비교
/// @param pt 비교할 점
/// @return 0이면 참
INLINE bool qm_point_isz(QmPoint pt)
{
	return pt.X == 0 && pt.Y == 0;
}


//////////////////////////////////////////////////////////////////////////
// size

/// @brief 사이즈 값 설정
/// @param width,height 너비와 높이
INLINE QmSize qm_size(int width, int height)
{
	return (QmSize) { { width, height } };
}

/// @brief 사각형으로 크기를 설정한다
/// @param rt 대상 사각형
INLINE QmSize qm_size_rect(QmRect rt)
{
	return (QmSize) { { rt.Right - rt.Left, rt.Bottom - rt.Top } };
}

/// @brief 사이즈 초기화
INLINE QmSize qm_size_zero(void)		// identify
{
	static const QmSize zero = { { 0, 0 } };
	return zero;
}

/// @brief 사이즈 대각값 설정 (모두 같은값으로 설정)
///	@param s 반환 사이즈
/// @param diag 대각 값
INLINE QmSize qm_size_diag(const int diag)
{
	return (QmSize) { { diag, diag } };
}

/// @brief 사이즈 덧셈
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
INLINE QmSize qm_size_add(QmSize left, QmSize right)
{
	return (QmSize) { { left.Width + right.Width, left.Height + right.Height } };
}

/// @brief 사이즈 뺄셈
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
INLINE QmSize qm_size_sub(QmSize left, QmSize right)
{
	return (QmSize) { { left.Width - right.Width, left.Height - right.Height } };
}

/// @brief 사이즈 확대
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 확대값
INLINE QmSize qm_size_mag(QmSize left, const int right)
{
	return (QmSize) { { left.Width * right, left.Height * right } };
}

/// @brief 사이즈 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmSize qm_size_mul(QmSize left, QmSize right)
{
	return (QmSize) { { left.Width * right.Width, left.Height * right.Height } };
}

/// @brief 사이즈 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmSize qm_size_div(QmSize left, QmSize right)
{
	return (QmSize) { { left.Width / right.Width, left.Height / right.Height } };
}

/// @brief 사이즈의 최소값
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
INLINE QmSize qm_size_min(QmSize left, QmSize right)
{
	return (QmSize) { { QN_MIN(left.Width, right.Width), QN_MIN(left.Height, right.Height) } };
}

/// @brief 사이즈의 최대값
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
INLINE QmSize qm_size_max(QmSize left, QmSize right)
{
	return (QmSize) { { QN_MAX(left.Width, right.Width), QN_MAX(left.Height, right.Height) } };
}

/// @brief 사이즈 길이의 제곱
/// @param s 사이즈
/// @return 길이의 제곱
INLINE int qm_size_len_sq(QmSize s)
{
	return s.Width * s.Width + s.Height * s.Height;
}

/// @brief 사이즈 길이
/// @param v 사이즈
/// @return 길이
INLINE float qm_size_len(QmSize v)
{
	return qm_sqrtf((float)qm_size_len_sq(v));	// NOLINT
}

/// @brief 종횡비(너비 나누기 높이)를 계신한다
/// @param s 계산할 크기
/// @return 종횡비값
INLINE float qm_size_aspect(QmSize s)
{
	return (float)s.Width / (float)s.Height;
}

/// @brief 대각선 DPI를 구한다
/// @param pt 너비와 높이
/// @param horizontal 너비 DPI
/// @param vertical 높이 DPI
/// @return 대각선 DPI
INLINE float qm_size_diag_dpi(QmSize pt, float horizontal, float vertical)
{
	float dsq = horizontal * horizontal + vertical + vertical;
	if (dsq <= 0.0f)
		return 0.0f;
	return qm_sqrtf((float)(pt.Width * pt.Width + pt.Height * pt.Height)) / qm_sqrtf(dsq);
}

/// @brief 사이즈의 비교
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
/// @return 같으면 참
INLINE bool qm_size_eq(QmSize left, QmSize right)
{
	return left.Width == right.Width && left.Height == right.Height;
}

/// @brief 사이즈가 0인가 비교
/// @param s 비교할 사이즈
/// @return 0이면 참
INLINE bool qm_size_isz(QmSize s)
{
	return s.Width == 0 && s.Height == 0;
}


//////////////////////////////////////////////////////////////////////////
// 사각형

/// @brief 사각형 값 설정
/// @param left,top,right,bottom 사각형 요소
/// @return 만든 사각형
INLINE QmRect qm_rect(const int left, const int top, const int right, const int bottom)
{
	return (QmRect) { { left, top, right, bottom } };
}

/// @brief 사각형을 좌표와 크기로 설정한다
/// @param x,y 좌표
/// @param width,height 크기
INLINE QmRect qm_rect_size(const int x, const int y, const int width, const int height)
{
	return (QmRect) { { x, y, x + width, y + height } };
}

/// @brief 사각형을 좌표와 크기 타입을 사용하여 설정한다
/// @param pos 좌표
/// @param size 크기
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
/// @param diag 대각값
INLINE QmRect qm_rect_diag(const int diag)
{
	return (QmRect) { { diag, diag, diag, diag } };
}

/// @brief 사각형 덧셈
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
INLINE QmRect qm_rect_add(QmRect left, QmRect right)
{
	return (QmRect) { { left.Left + right.Left, left.Top + right.Top, left.Right + right.Right, left.Bottom + right.Bottom } };
}

/// @brief 사각형 뺄셈
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
INLINE QmRect qm_rect_sub(QmRect left, QmRect right)
{
	return (QmRect) { { left.Left - right.Left, left.Top - right.Top, left.Right - right.Right, left.Bottom - right.Bottom } };
}

/// @brief 사각형 확대
/// @param left 원본 사각형
/// @param right 확대값
INLINE QmRect qm_rect_mag(QmRect left, const int right)
{
	return (QmRect) { { left.Left * right, left.Top * right, left.Right * right, left.Bottom * right } };
}

/// @brief 사각형의 최소값
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
INLINE QmRect qm_rect_min(QmRect left, QmRect right)
{
	return (QmRect) { { QN_MIN(left.Left, right.Left), QN_MIN(left.Top, right.Top), QN_MIN(left.Right, right.Right), QN_MIN(left.Bottom, right.Bottom) } };
}

/// @brief 사각형의 최대값
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
INLINE QmRect qm_rect_max(QmRect left, QmRect right)
{
	return (QmRect) { { QN_MAX(left.Left, right.Left), QN_MAX(left.Top, right.Top), QN_MAX(left.Right, right.Right), QN_MAX(left.Bottom, right.Bottom) } };
}

/// @brief 사각형 크기를 키운다 (요소가 양수일 경우)
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른쪽
/// @param bottom 아래쪽
INLINE QmRect qm_rect_inflate(QmRect rt, const int left, const int top, const int right, const int bottom)
{
	return (QmRect) { { rt.Left - left, rt.Top - top, rt.Right + right, rt.Bottom + bottom } };
}

/// @brief 사각형 크기를 줄인다 (요소가 양수일 경우)
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른족
/// @param bottom 아래쪽
INLINE QmRect qm_rect_deflate(QmRect rt, const int left, const int top, const int right, const int bottom)
{
	return (QmRect) { { rt.Left + left, rt.Top + top, rt.Right - right, rt.Bottom - bottom } };
}

/// @brief 사각형을 움직인다
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른쪽
/// @param bottom 아래쪽
INLINE QmRect qm_rect_offset(QmRect rt, const int left, const int top, const int right, const int bottom)
{
	return (QmRect) { { rt.Left + left, rt.Top + top, rt.Right + right, rt.Bottom + bottom } };
}

/// @brief 사각형을 움직인다
/// @param rt 원본 사각형
/// @param left 새 왼쪽
/// @param top 새 오른쪽
/// @return
INLINE QmRect qm_rect_move(QmRect rt, const int left, const int top)
{
	const int dx = left - rt.Left;
	const int dy = top - rt.Top;
	return (QmRect) { { rt.Left + dx, rt.Top + dy, rt.Right + dx, rt.Bottom + dy } };
}

/// @brief 사각형의 크기를 재설정한다
/// @param rt 원본 사각형
/// @param width 너비
/// @param height 높이
INLINE QmRect qm_rect_resize(QmRect rt, const int width, const int height)
{
	return (QmRect) { { rt.Left, rt.Top, rt.Left + width, rt.Top + height } };
}

/// @brief 사각형의 너비를 얻는다
/// @param rt 대상 사각형
/// @return 사각형의 너비값
INLINE int qm_rect_width(QmRect rt)
{
	return rt.Right - rt.Left;
}

/// @brief 사각형의 높이를 얻는다
/// @param rt 대상 사각형
/// @return 사각형의 높이값
INLINE int qm_rect_height(QmRect rt)
{
	return rt.Bottom - rt.Top;
}

/// @brief 좌표가 사각형 안에 있는지 조사한다
/// @param rt 대상 사각형
/// @param x,y 좌표
/// @return 좌표가 사각형 안에 있으면 참
INLINE bool qm_rect_in(QmRect rt, const int x, const int y)
{
	return x >= rt.Left && x <= rt.Right && y >= rt.Top && y <= rt.Bottom;
}

/// @brief 대상 사각형이 원본 사각형안에 있는지 조사한다
/// @param dest 원본 사각형
/// @param target 대상 사각형
/// @return 대상이 원본 안에 있으면 참
INLINE bool qm_rect_include(QmRect dest, QmRect target)
{
	if (dest.Left > target.Left || dest.Top > target.Top)
		return false;
	if (dest.Right < target.Right || dest.Bottom < target.Bottom)
		return false;
	return true;
}

/// @brief 두 사각형이 충돌하는지 비교하고 충돌 사각형을 만든다
/// @param r1 왼쪽 사각형
/// @param r2 오른쪽 사각형
/// @param p 충돌 사각형이 담길 사각형 (NULL 가능)
/// @return 두 사각형이 충돌했다면 참
INLINE bool qm_rect_intersect(QmRect r1, QmRect r2, QmRect * p)
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
INLINE bool qm_rect_eq(QmRect left, QmRect right)
{
	return
		left.Left == right.Left && left.Top == right.Top &&
		left.Right == right.Right && left.Bottom == right.Bottom;
}

/// @brief 사각형가 0인지 비교
/// @param pv 비교할 사각형
/// @return 사각형가 0이면 참
INLINE bool qm_rect_isz(QmRect pv)
{
	return pv.Left == 0 && pv.Top == 0 && pv.Right == 0 && pv.Bottom == 0;
}

#ifdef _WINDEF_
/// @brief 윈도우 RECT에서
INLINE QmRect qm_rect_RECT(RECT rt)
{
	return (QmRect) { { rt.left, rt.top, rt.right, rt.bottom } };
}

/// @brief 윈도우 RECT로
INLINE RECT qm_rect_to_RECT(QmRect rt)
{
	return (RECT) { rt.Left, rt.Top, rt.Right, rt.Bottom };
}
#endif


//////////////////////////////////////////////////////////////////////////
// half

/// @brief 하프 벡터2를 설정한다
/// @param x,y 좌표
INLINE QmVecH2 qm_vec2h(float x, float y)
{
	return (QmVecH2) { { qm_f2hf(x), qm_f2hf(y) } };
}

/// @brief 하프 벡터4를 설정한다
/// @param x,y,z,w 좌표
INLINE QmVecH4 qm_vec4h(float x, float y, float z, float w)
{
	return (QmVecH4) { { qm_f2hf(x), qm_f2hf(y), qm_f2hf(z), qm_f2hf(w) } };
}


//////////////////////////////////////////////////////////////////////////
// 제네릭

/// @brief (제네릭) 부호 반전
#define qm_neg(x)		_Generic((x),\
	QmVec2: qm_vec2_neg,\
	QmVec4: qm_vec4_neg,\
	QmColor: qm_color_neg,\
	QmKolor: qm_kolor_neg,\
	QmPoint*: qm_point_neg)(x)
/// @brief (제네릭) 덧셈
#define qm_add(l,r)		_Generic((l),\
	QmVec2: qm_vec2_add,\
	QmVec4: qm_vec4_add,\
	QmMat4: qm_mat4_add,\
	QmPoint: qm_point_add,\
	QmSize: qm_size_add,\
	QmRect: qm_rect_add,\
	QmKolor: qm_kolor_add)(l,r)
/// @brief (제네릭) 뺄셈
#define qm_sub(l,r)		_Generic((l),\
	QmVec2: qm_vec2_sub,\
	QmVec4: qm_vec4_sub,\
	QmMat4: qm_mat4_sub,\
	QmPoint: qm_point_sub,\
	QmSize: qm_size_sub,\
	QmRect: qm_rect_sub,\
	QmKolor: qm_kolor_sub)(l,r)
/// @brief (제네릭) 확대
#define qm_mag(i,s)		_Generic((i),\
	QmVec2: qm_vec2_mag,\
	QmVec4: qm_vec4_mag,\
	QmMat4: qm_mat4_mag,\
	QmPoint: qm_point_mag,\
	QmSize: qm_size_mag,\
	QmRect: qm_rect_mag,\
	QmKolor: qm_kolor_mag)(i,s)
/// @brief (제네릭) 곱셈
#define qm_mul(l,r)		_Generic((l),\
	QmVec2: qm_vec2_mul,\
	QmVec4: qm_vec4_mul,\
	QmMat4: qm_mat4_mul,\
	QmColor: qm_color_mod,\
	QmPoint: qm_point_mul,\
	QmSize: qm_size_mul,\
	QmKolor: qm_kolor_mul)(l,r)
/// @brief (제네릭) 나눗셈
#define qm_div(l,r)		_Generic((l),\
	QmVec2: qm_vec2_div,\
	QmVec4: qm_vec4_div,\
	QmPoint: qm_point_div,\
	QmSize: qm_size_div,\
	QmKolor: qm_kolor_div)(l,r)
/// @brief (제네릭) 최소값
#define qm_min(l,r)		_Generic((l),\
	float: qm_minf,\
	QmVec2: qm_vec2_min,\
	QmVec4: qm_vec4_min,\
	QmPoint: qm_point_min,\
	QmSize: qm_size_min,\
	QmRect: qm_rect_min)(l,r)
/// @brief (제네릭) 최대값
#define qm_max(l,r)		_Generic((l),\
	float: qm_maxf,\
	QmVec2: qm_vec2_max,\
	QmVec4: qm_vec4_max,\
	QmPoint: qm_point_max,\
	QmSize: qm_size_max,\
	QmRect: qm_rect_max)(l,r)
/// @brief (제네릭) 같나 비교
#define qm_eq(l,r)		_Generic((l),\
	float: qm_eqf,\
	QmVec2: qm_vec2_eq,\
	QmVec4: qm_vec4_eq,\
	QmColor: qm_color_eq,\
	QmPoint: qm_point_eq,\
	QmSize: qm_size_eq,\
	QmRect: qm_rect_eq,\
	QmKolor: qm_kolor_eq)(l,r)
/// @brief (제네릭) 단위 요소인가 확인
#define qm_isu(x)		_Generic((x),\
	QmVec2: qm_vec2_isz,\
	QmVec4: qm_vec4_isz,\
	QmQuat: qm_quat_isu,\
	QmMat4: qm_mat4_isu,\
	QmPlane: qm_plane_isu,\
	QmPoint: qm_point_isz,\
	QmSize: qm_size_isz,\
	QmRect: qm_rect_isz)(x)
/// @brief (제네릭) 내적
#define qm_dot(l,r)		_Generic((l),\
	QmVec2: qm_vec2_dot,\
	QmVec4: qm_vec4_dot,\
	QmPoint: qm_point_dot)(l,r)
/// @brief (제네릭) 외적
#define qm_cross(l,r)		_Generic((l),\
	QmVec2: qm_vec2_cross,\
	QmVec4: qm_vec4_cross,\
	QmPoint: qm_point_cross)(l,r)
/// @brief (제네릭) 길의 제곱
#define qm_len_sq(x)	_Generic((x),\
	QmVec2: qm_vec2_len_sq,\
	QmVec4: qm_vec4_len_sq,\
	QmPoint: qm_point_len_sq,\
	QmSize: qm_size_len_sq)(x)
/// @brief (제네릭) 길이
#define qm_len(x)		_Generic((x),\
	QmVec2: qm_vec2_len,\
	QmVec4: qm_vec4_len,\
	QmPoint: qm_point_len,\
	QmSize: qm_size_len)(x)
/// @brief (제네릭) 거리의 제곱
#define qm_dist_sq(x,y)	_Generic((x),\
	QmVec2: qm_vec2_dist_sq,\
	QmVec4: qm_vec4_dist_sq,\
	QmPoint : qm_point_dist_sq)(x,y)
/// @brief (제네릭) 거리
#define qm_dist(x,y)	_Generic((x),\
	QmVec2: qm_vec2_dist,\
	QmVec4: qm_vec4_dist,\
	QmPlane: qm_plane_dist,\
	QmPoint: qm_point_dist)(x,y)
/// @brief (제네릭) 정규화
#define qm_norm(x)		_Generic((x),\
	QmVec2: qm_vec2_norm,\
	QmVec4: qm_vec4_norm,\
	QmPlane: qm_plane_norm)(o)
/// @brief (제네릭) 원본에서 대상으로 보간
#define qm_interpolate(l,r,s)	_Generic((l),\
	QmVec2: qm_vec2_interpolate,\
	QmVec4: qm_vec4_interpolate,\
	QmColor: qm_color_interpolate,\
	QmKolor: qm_kolor_interpolate)(l,r,s)
/// @brief (제네릭) 선형 보간
#define qm_lerp(l,r,s)	_Generic((l),\
	float: qm_lerpf,\
	QmVec2: qm_vec2_lerp,\
	QmVec4: qm_vec4_lerp,\
	QmColor: qm_color_lerp,\
	QmKolor: qm_kolor_lerp)(l,r,s)
/// @brief (제네릭) 구면 보간
#define qm_slerp(l,r,s)	_Generic((l),\
	QmQuat: qm_quat_lerp)(l,r,s)
/// @brief (제네릭) 역함수
#define qm_inv(x)		_Generic((x),\
	QmQuat: qm_quat_inv,\
	QmMat4: qm_mat4_inv)(x)
/// @brief (제네릭) 전치
#define qm_tran(x)		_Generic((x),\
	QmMat4: qm_mat4_tran)(x)
/// @brief 절대값
#define qm_abs(x)		_Generic((x),\
	float: qm_absf,\
	int: qm_absi)(x)
/// @brief 범위로 자르기
#define qm_clamp(v,n,x)	_Generic((v),\
	float: qm_clampf,\
	int: qm_clampi)(v,n,x)
/// @brief 종횡비
#define qm_aspect(s)	_Generic((s),\
	QmSize: qm_size_aspect)(s)

QN_EXTC_END

#ifdef __GNUC__
#pragma GCC diagnotics pop
#endif

// set reset diagnosis ivt add sub  mag mul div min max eq isi
// dot cross len_sq len dist_sq dist
// ^INLINE (.*)$ => $0;\n
