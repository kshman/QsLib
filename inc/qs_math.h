//
// QsLib [MATH Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
//

#pragma once
#define __QS_MATH__

#ifdef __GNUC__
#pragma GCC diagnotics push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#if defined _M_AMD64 || defined _M_X64 || defined __amd64__ || defined __x86_64__ || (defined _M_IX86_FP && _M_IX86_FP >= 1) || defined __SSE__
#define QM_USE_SSE		1
#endif
#if defined _M_ARM || defined __ARM_NEON
#define QM_USE_NEON		1
#endif

#if defined QM_USE_SSE
#include <xmmintrin.h>>
#endif
#ifdef QM_USE_NEON
#include <arm_neon.h>
#endif
#include "qs_qn.h"

QN_EXTC_BEGIN

//////////////////////////////////////////////////////////////////////////
// constant
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
#define QM_PI_Q			0.78539816339744830961f								/// @brief 원주율의 사분의 일
#define QM_SQRT2		1.41421356237309504880f								/// @brief 2의 제곱근
#define QM_SQRTH		0.70710678118654752440f								/// @brief 2의 제곱근의 반
#define QM_RAD2DEG		(180.0f/QM_PI)
#define QM_DEG2RAD		(QM_PI/180.0f)


//////////////////////////////////////////////////////////////////////////
// macro & inline

#ifndef QM_FLOORF
#define QM_FLOORF		floorf
#endif
#ifndef QM_SINF
#define QM_SINF			sinf
#endif
#ifndef QM_COSF
#define QM_COSF			cosf
#endif
#ifndef QM_TANF
#define QM_TANF			tanf
#endif
#ifndef QM_ASINF
#define QM_ASINF		asinf
#endif
#ifndef QM_ACOSF
#define QM_ACOSF		acosf
#endif
#ifndef QM_ATANF
#define QM_ATANF		atanf
#endif
#ifndef QM_ATAN2F
#define QM_ATAN2F		atan2f
#endif
#ifndef QM_SQRTF
#define QM_SQRTF		sqrtf
#endif

/// @brief 실수를 정수부만 뽑기
#define QM_FRACT(f)		((f)-QM_FLOORF(f))
/// @brief 각도를 호도로 변환
#define QM_TORADIAN(d)	((d)*QM_DEG2RAD)
/// @brief 호도를 각도로 변환
#define QM_TODEGREE(r)	((r)*QM_RAD2DEG)
/// @brief 실수의 앱실론 비교
#define QM_EQEPS(a,b,e)	(((a)+(e)>(b)) && ((a)-(e)<(b)))

/// @brief 실수의 엡실론 비교
QN_INLINE bool qm_eqf(const float a, const float b)
{
	return QM_EQEPS(a, b, (float)QM_EPSILON);
}

/// @brief 두 실수의 최대값
QN_INLINE float qm_maxf(const float a, const float b)
{
	return QN_MAX(a, b);
}

/// @brief 두 실수의 최소값
QN_INLINE float qm_minf(const float a, const float b)
{
	return QN_MIN(a, b);
}

/// @brief 정수의 절대값
QN_INLINE int qm_absi(const int v)
{
	return QN_ABS(v);
}

/// @brief 실수의 절대값
QN_INLINE float qm_absf(const float v)
{
	return QN_ABS(v);
}

/// @brief 정수를 범위 내로 자르기
QN_INLINE int qm_clampi(const int v, const int min, const int max)
{
	return QN_CLAMP(v, min, max);
}

/// @brief 실수를 범위 내로 자르기
QN_INLINE float qm_clampf(const float v, const float min, const float max)
{
	return QN_CLAMP(v, min, max);
}

/// @brief 각도를 -180 ~ +180 사이로 자르기
QN_INLINE float qm_cradf(const float v)
{
	return v < (float)-QM_PI_H ? v + (float)QM_PI_H : v >(float)QM_PI_H ? v - (float)QM_PI_H : v;
}

/// @brief 실수의 보간
QN_INLINE float qm_lerpf(const float left, const float right, const float scale)
{
	return left + scale * (right - left);
}

/// @brief 사인과 코사인을 동시에 계산
QN_INLINE void qm_sincos(const float f, float* s, float* c)
{
	*s = QM_SINF(f);
	*c = QM_COSF(f);
}

/// @brief 제곱근
QN_INLINE float qm_sqrtf(float f)
{
#if defined QM_USE_SSE
	__m128 i = _mm_set_ss(f);
	__m128 o = _mm_sqrt_ss(i);
	return _mm_cvtss_f32(o);
#elif defined QM_USE_NEON
	float32x4_t i = vdupq_n_f32(f);
	float32x4_t o = vsqrtq_f32(f);
	return vgetq_lane_f32(o, 0);
#else
	return QM_SQRTF(f);
#endif
}

/// @brief 1을 나눈 제곱근
QN_INLINE float qm_inv_sqrtf(float f)
{
	return 1.0f / QM_SQRTF(f);
}


//////////////////////////////////////////////////////////////////////////
// types

/// @brief 벡터2
typedef union QmVec2
{
	struct
	{
		float X, Y;
	};
	struct
	{
		float U, V;
	};
	struct
	{
		float Pa, Pb;
	};
	struct
	{
		float Width, Height;
	};
	struct
	{
		float Near, Far;
	};
	struct
	{
		float Min, Max;
	};
	float f[2];
} QmVec2, QmCoord, QmPointF, QmSizeF, QmDepth, QmMinMaxF;

/// @brief 벡터3
typedef union QmVec3
{
	struct
	{
		float X, Y, Z;
	};
	struct
	{
		float R, G, B;
	};
	struct
	{
		float Pa, Pb, Pc;
	};
	struct
	{
		QmVec2 XY;
		float _Z;
	};
	struct
	{
		float _X;
		QmVec2 YZ;
	};
	float f[3];
} QmVec3, QmRgb;

/// @brief 벡터4
typedef union QN_ALIGN(16) QmVec4
{
	struct
	{
		union
		{
			QmVec3 XYZ;
			struct
			{
				float X, Y, Z;
			};
		};
		float W;
	};
	struct
	{
		union
		{
			QmVec3 RGB;
			struct
			{
				float R, G, B;
			};
		};
		float A;
	};
	struct
	{
		union
		{
			QmVec3 PABC;
			struct
			{
				float Pa, Pb, Pc;
			};
		};
		float Pd;
	};
	struct
	{
		float Left, Top, Right, Bottom;
	};
	struct
	{
		QmVec2 XY;
		QmVec2 _ZW;
	};
	struct
	{
		float _X;
		QmVec2 YZ;
		float _W;
	};
	struct
	{
		QmVec2 _XY;
		QmVec2 ZW;
	};
	struct
	{
		QmPointF LeftTop;
		QmPointF RightBottom;
	};
	float f[4];
#if defined QM_USE_SSE
	__m128 m128;
#endif
#ifdef QM_USE_NEON
	float32x4_t neon;
#endif
} QmVec4, QmRectF, QmColor, QmPlane;

/// @brief 행렬4x4
typedef union QN_ALIGN(16) QmMat4
{
	struct
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;
	};
	float f[4][4];
	QmVec4 rows[4];
#if defined QM_USE_SSE
	__m128 m128[4];
#endif
#ifdef QM_USE_NEON
	float32x4_t neon[4];
#endif
} QmMat4;

/// @brief 사원수
typedef union QN_ALIGN(16) QmQuat
{
	struct
	{
		union
		{
			QmVec3 XYZ;
			struct
			{
				float X, Y, Z;
			};
		};
		float W;
	};
	float f[4];
#if defined QM_USE_SSE
	__m128 m128;
#endif
#ifdef QM_USE_NEON
	float32x4_t neon;
#endif
} QmQuat;

/// @brief 정수형 벡터2
typedef union QmVecI2
{
	struct
	{
		int X, Y;
	};
	struct
	{
		int Width, Height;
	};
	int i[2];
} QmVecI2, QmPoint, QmSize;

/// @brief 정수형 벡터3
typedef union QmVecI3
{
	struct
	{
		int X, Y, Z;
	};
	struct
	{
		int R, G, B;
	};
	struct
	{
		QmPoint XY;
		QmPoint YZ;
	};
	int i[3];
} QmVecI3, QmRgbI;

/// @brief 정수형 벡터4
typedef union QN_ALIGN(16) QmVecI4
{
	struct
	{
		union
		{
			QmVec3 XYZ;
			struct
			{
				int X, Y, Z;
			};
		};
		int W;
	};
	struct
	{
		union
		{
			QmVecI3 RGB;
			struct
			{
				int R, G, B;
			};
		};
		int A;
	};
	struct
	{
		int Left, Top, Right, Bottom;
	};
	struct
	{
		QmPoint LeftTop;
		QmPoint RightBottom;
	};
	int i[4];
} QmVecI4, QmColorI, QmRect;

/// @brief 정수형 색깔
typedef struct QN_ALIGN(4) QmKolor
{
	byte B, G, R, A;
} QmKolor;

/// @brief 혼합형 정수형 색깔
typedef union QmKolorU
{
	QmKolor K;
	uint U;
} QmKolorU;

/// @brief 선분
typedef struct QmLine3
{
	QmVec3 Begin, End;
} QmLine3;

/// @brief 트랜스폼
typedef struct QmTrfm
{
	QmQuat Rotation;
	QmVec3 Location;
	QmVec3 Scale;
} QmTrfm;

/// @brief 하프 벡터2
typedef struct QmVecH2
{
	halfint X, Y;
} QmVecH2;

/// @brief 하프 벡터3
typedef struct QmVecH3
{
	halfint X, Y, Z;
} QmVecH3;

/// @brief 하프 벡터4
typedef struct QmVecH4
{
	halfint X, Y, Z, W;
} QmVecH4;


//////////////////////////////////////////////////////////////////////////
// 벡터

/// @brief 벡터2 값 설정
/// @param x,y 좌표
QN_INLINE QmVec2 qm_vec2(float x, float y)
{
	QmVec2 r = { .X = x, .Y = y };
	return r;
}

/// @brief 벡터3 값 설정
/// @param x,y,z 좌표
QN_INLINE QmVec3 qm_vec3(const float x, const float y, const float z)
{
	QmVec3 r = { .X = x, .Y = y, .Z = z };
	return r;
}

/// @brief 벡터4 값 설정
/// @param x,y,z,w 벡터4 요소
/// @return 만든 벡터4
QN_INLINE QmVec4 qm_vec4(const float x, const float y, const float z, const float w)
{
	QmVec4 v =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(x, y, z, w) };
#elif defined QM_USE_NEON
	{.neon = { x, y, z, w } };
#else
	{.x = x, .y = y, .z = z, .w = w };
#endif
	return v;
}

/// @brief 벡터4 값 설정
/// @param p 입력 벡터3
/// @param w w 요소
/// @return 만든 벡터4
QN_INLINE QmVec4 qm_vec4v(const QmVec3* p, const float w)
{
	QmVec4 v =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(p->X, p->Y, p->Z, w) };
#elif defined QM_USE_NEON
	{.neon = { p->X, p->Y, p->Z, w } };
#else
	{.x = p->X, .y = p->Y, .z = p->Z, .w = w };
#endif
	return v;
}

/// @brief 벡터2 초기화
QN_INLINE QmVec2 qm_vec2_rst(void)		// identify
{
	QmVec2 r = { .X = 0.0f, .Y = 0.0f };
	return r;
}

/// @brief 벡터3 초기화
QN_INLINE QmVec3 qm_vec3_rst(void)		// identify
{
	QmVec3 r = { .X = 0.0f, .Y = 0.0f, .Z = 0.0f };
	return r;
}

/// @brief 벡터4 초기화
QN_INLINE QmVec4 qm_vec4_rst(void)		// identify
{
	QmVec4 v =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 0.0f) };
#elif defined QM_USE_NEON
	{.neon = { 0.0f, 0.0f, 0.0f, 0.0f } };
#else
	{.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f };
#endif
	return v;
}

/// @brief 벡터2 대각값 설정 (모두 같은값으로 설정)
/// @param v 대각 값
QN_INLINE QmVec2 qm_vec2_diag(const float diag)
{
	QmVec2 r = { .X = diag, .Y = diag };
	return r;
}

/// @brief 벡터3 대각값 설정 (모두 같은값으로 설정)
/// @param diag 대각값
QN_INLINE QmVec3 qm_vec3_diag(const float diag)
{
	QmVec3 r = { .X = diag, .Y = diag, .Z = diag };
	return r;
}

/// @brief 벡터4 대각값 설정 (모든 요소를 같은 값을)
/// @param diag 대각값
QN_INLINE QmVec4 qm_vec4_diag(const float diag)
{
	QmVec4 v =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(diag, diag, diag, diag) };
#elif defined QM_USE_NEON
	{.neon = { diag, diag, diag, diag } };
#else
	{.x = diag, .y = diag, .z = diag, .w = diag };
#endif
	return v;
}

/// @brief 벡터2 반전
/// @param v 벡터2
QN_INLINE QmVec2 qm_vec2_ivt(const QmVec2* v)  // invert
{
	QmVec2 r = { .X = -v->X, .Y = -v->Y };
	return r;
}

/// @brief 벡터3 반전
/// @param v 원본 벡터3
QN_INLINE QmVec3 qm_vec3_ivt(const QmVec3* v)  // invert
{
	QmVec3 r = { .X = -v->X, .Y = -v->Y, .Z = -v->Z };
	return r;
}

/// @brief 벡터4 반전
/// @param v 원본 벡터4
QN_INLINE QmVec4 qm_vec4_ivt(const QmVec4* v)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(-v->X, -v->Y, -v->Z, -v->W) };
#elif defined QM_USE_NEON
	{.neon = { -v->X, -v->Y, -v->Z, -v->W } };
#else
	{.X = -v->X, .Y = -v->Y, .Z = -v->Z, .W = -v->W };
#endif
	return r;
}

/// @brief 벡터2 네거티브 (1 - 벡터2)
/// @param pv 네거티브를 담을 벡터2
/// @param v 벡터2
QN_INLINE QmVec2 qm_vec2_neg(const QmVec2* v)
{
	QmVec2 r = { .X = 1.0f - v->X, .Y = 1.0f - v->Y };
	return r;
}

/// @brief 벡터3 네거티브 (1 - 값)
/// @param v 원본 벡터3
QN_INLINE QmVec3 qm_vec3_neg(const QmVec3* v)
{
	QmVec3 r = { .X = 1.0f - v->X, .Y = 1.0f - v->Y, .Z = 1.0f - v->Z };
	return r;
}

/// @brief 벡터4 네거티브
/// @param pv 네거티브 결과를 담을 벡터4
/// @param v 원본 벡터4
QN_INLINE void qm_vec4_neg(QmVec4* pv, const QmVec4* v)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(1.0f - v->X, 1.0f - v->Y, 1.0f - v->Z, 1.0f - v->W) };
#elif defined QM_USE_NEON
	{.neon = { 1.0f - v->X, 1.0f - v->Y, 1.0f - v->Z, 1.0f - v->W } };
#else
	{.X = 1.0f - v->X, .Y = 1.0f - v->Y, .Z = 1.0f - v->Z, .W = 1.0f - v->W };
#endif
	return r;
}

/// @brief 벡터2 덧셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_add(const QmVec2* left, const QmVec2* right)
{
	QmVec2 r = { .X = left->X + right->X, .Y = left->Y + right->Y };
	return r;
}

/// @brief 벡터3 덧셈
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_add(const QmVec3* left, const QmVec3* right)
{
	QmVec3 r = { .X = left->X + right->X, .Y = left->Y + right->Y, .Z = left->Z + right->Z };
	return r;
}

/// @brief 벡터4 덧셈
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVec4 qm_vec4_add(const QmVec4* left, const QmVec4* right)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_add_ps(left->m128, right->m128) };
#elif defined QM_USE_NEON
	{.neon = vaddq_f32(left->neon, right->neon) };
#else
	{.X = left->X + right->X, .Y = left->Y + right->Y, .Z = left->Z + right->Z, .W = left->W + right->W, };
#endif
	return r;
}

/// @brief 벡터2 뺄셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_sub(const QmVec2* left, const QmVec2* right)
{
	QmVec2 r = { .X = left->X - right->X, .Y = left->Y - right->Y };
	return r;
}

/// @brief 벡터3 뺄셈
/// @param pv 뺄셈 결과를 담을 벡터3
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_sub(const QmVec3* left, const QmVec3* right)
{
	QmVec3 r = { .X = left->X - right->X, .Y = left->Y - right->Y, .Z = left->Z - right->Z };
	return r;
}

/// @brief 벡터4 뺄셈
/// @param pv 뺄셈 결과를 담을 벡터4
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVec4 qm_vec4_sub(const QmVec4* left, const QmVec4* right)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_sub_ps(left->m128, right->m128) };
#elif defined QM_USE_NEON
	{.neon = vsubq_f32(left->neon, right->neon) };
#else
	{.X = left->X - right->X, .Y = left->Y - right->Y, .Z = left->Z - right->Z, .W = left->W - right->W, };
#endif
	return r;
}

/// @brief 벡터2 확대
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 확대값
QN_INLINE QmVec2 qm_vec2_mag(const QmVec2* left, const float right)
{
	QmVec2 r = { .X = left->X * right, .Y = left->Y * right };
	return r;
}

/// @brief 벡터3 확대
/// @param pv 확대 결과를 담을 벡터3
/// @param left 원본 벡터3
/// @param right 확대값
QN_INLINE QmVec3 qm_vec3_mag(const QmVec3* left, const float right)
{
	QmVec3 r = { .X = left->X * right, .Y = left->Y * right, .Z = left->Z * right };
	return r;
}

/// @brief 벡터4 확대
/// @param left 원본 벡터4
/// @param right 확대값
QN_INLINE QmVec4 qm_vec4_mag(const QmVec4* left, const float right)
{
#if defined QM_USE_SSE
	__m128 m = _mm_set1_ps(right);
	QmVec4 r = { .m128 = _mm_mul_ps(left->m128, m) };
#elif defined QM_USE_NEON
	QmVec4 r = { .neon = vmulq_n_f32(left->neon, right) };
#else
	QmVec4 r = { .X = left->X * right, .Y = left->Y * right, .Z = left->Z * right, .W = left->W * right };
#endif
	return r;
}

/// @brief 벡터2 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec2 qm_vec2_mul(const QmVec2* left, const QmVec2* right)
{
	QmVec2 r = { .X = left->X * right->X, .Y = left->Y * right->Y };
	return r;
}

/// @brief 벡터3 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec3 qm_vec3_mul(const QmVec3* left, const QmVec3* right)
{
	QmVec3 r = { .X = left->X * right->X, .Y = left->Y * right->Y, .Z = left->Z * right->Z };
	return r;
}

/// @brief 벡터4 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec4 qm_vec4_mul(const QmVec4* left, const QmVec4* right)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_mul_ps(left->m128, right->m128) };
#elif defined QM_USE_NEON
	{.neon = vmulq_f32(left->neon, right->neon)};
#else
	{.X = left->X * right->X, .Y = left->Y * right->Y, .Z = left->Z * right->Z, .W = left->W * right->W };
#endif
	return r;
}

/// @brief 벡터2 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec2 qm_vec2_div(const QmVec2* left, const QmVec2* right)
{
	QmVec2 r = { .X = left->X / right->X, .Y = left->Y / right->Y };
	return r;
}

/// @brief 벡터3 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec3 qm_vec3_div(const QmVec3* left, const QmVec3* right)
{
	QmVec3 r = { .X = left->X / right->X, .Y = left->Y / right->Y, .Z = left->Z / right->Z };
	return r;
}

/// @brief 벡터4 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec4 qm_vec4_div(const QmVec4* left, const QmVec4* right)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_div_ps(left->m128, right->m128) };
#elif defined QM_USE_NEON
	{.neon = vdivq_f32(left->neon, right->neon)};
#else
	{.X = left->X / right->X, .Y = left->Y / right->Y, .Z = left->Z / right->Z, .W = left->W / right->W };
#endif
	return r;
}

/// @brief 벡터2의 최소값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_min(const QmVec2* left, const QmVec2* right)
{
	QmVec2 r =
	{
		.X = (left->X < right->X) ? left->X : right->X,
		.Y = (left->Y < right->Y) ? left->Y : right->Y
	};
	return r;
}

/// @brief 벡터3의 최소값
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_min(const QmVec3* left, const QmVec3* right)
{
	QmVec3 r =
	{
		.X = left->X < right->X ? left->X : right->X,
		.Y = left->Y < right->Y ? left->Y : right->Y,
		.Z = left->Z < right->Z ? left->Z : right->Z,
	};
	return r;
}

/// @brief 벡터4의 최소값
/// @param pv 최소값을 담을 벡터4
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE void qm_vec4_min(QmVec4* pv, const QmVec4* left, const QmVec4* right)
{
	pv->X = (left->X < right->X) ? left->X : right->X;
	pv->Y = (left->Y < right->Y) ? left->Y : right->Y;
	pv->Z = (left->Z < right->Z) ? left->Z : right->Z;
	pv->W = (left->W < right->W) ? left->W : right->W;
}

/// @brief 벡터2의 최대값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_max(const QmVec2* left, const QmVec2* right)
{
	QmVec2 r =
	{
		.X = (left->X > right->X) ? left->X : right->X,
		.Y = (left->Y > right->Y) ? left->Y : right->Y
	};
	return r;
}

/// @brief 벡터3의 최대값
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_max(const QmVec3* left, const QmVec3* right)
{
	QmVec3 r =
	{
		.X = left->X > right->X ? left->X : right->X,
		.Y = left->Y > right->Y ? left->Y : right->Y,
		.Z = left->Z > right->Z ? left->Z : right->Z,
	};
	return r;
}

/// @brief 벡터4의 최대값
/// @param pv 최대값을 담을 벡터4
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE void qm_vec4_max(QmVec4* pv, const QmVec4* left, const QmVec4* right)
{
	pv->X = (left->X > right->X) ? left->X : right->X;
	pv->Y = (left->Y > right->Y) ? left->Y : right->Y;
	pv->Z = (left->Z > right->Z) ? left->Z : right->Z;
	pv->W = (left->W > right->W) ? left->W : right->W;
}

/// @brief 벡터2의 비교
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡트
/// @return 같으면 참
QN_INLINE bool qm_vec2_eq(const QmVec2* left, const QmVec2* right)
{
	return qm_eqf(left->X, right->X) && qm_eqf(left->Y, right->Y);
}

/// @brief 두 벡터3이 같은지 판단
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3이 같으면 참
QN_INLINE bool qm_vec3_eq(const QmVec3* left, const QmVec3* right)
{
	return qm_eqf(left->X, right->X) && qm_eqf(left->Y, right->Y) && qm_eqf(left->Z, right->Z);
}

/// @brief 두 벡터4를 비교
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터4가 같으면 참
QN_INLINE bool qm_vec4_eq(const QmVec4* left, const QmVec4* right)
{
	return
		qm_eqf(left->X, right->X) && qm_eqf(left->Y, right->Y) &&
		qm_eqf(left->Z, right->Z) && qm_eqf(left->W, right->W);
}

/// @brief 벡터2가 0인가 비교
/// @param pv 비교할 벡터2
/// @return 0이면 참
QN_INLINE bool qm_vec2_isi(const QmVec2* pv)
{
	return pv->X == 0.0f && pv->Y == 0.0f;
}

/// @brief 벡터3이 0인지 판단
/// @param pv 벡터3
/// @return 벡터3이 0이면 참
QN_INLINE bool qm_vec3_isi(const QmVec3* pv)
{
	return pv->X == 0.0f && pv->Y == 0.0f && pv->Z == 0.0f;
}

/// @brief 벡터가 0인지 비교
/// @param pv 비교할 벡터4
/// @return 벡터4가 0이면 참
QN_INLINE bool qm_vec4_isi(const QmVec4* pv)
{
	return pv->X == 0.0f && pv->Y == 0.0f && pv->Z == 0.0f && pv->W == 0.0f;
}

/// @brief 벡터2 내적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 내적 값
QN_INLINE float qm_vec2_dot(const QmVec2* left, const QmVec2* right)
{
	return left->X * right->X + left->Y * right->Y;
}

/// @brief 벡터3의 내적
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 내적값
QN_INLINE float qm_vec3_dot(const QmVec3* left, const QmVec3* right)
{
	return left->X * right->X + left->Y * right->Y + left->Z * right->Z;
}

/// @brief 벡터4 내적
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 내적값
QN_INLINE float qm_vec4_dot(const QmVec4* left, const QmVec4* right)
{
#if defined QM_USE_SSE
	__m128 p = _mm_mul_ps(left->m128, right->m128);
	__m128 u = _mm_shuffle_ps(p, p, _MM_SHUFFLE(2, 3, 0, 1));
	p = _mm_add_ps(p, u);
	u = _mm_shuffle_ps(p, p, _MM_SHUFFLE(0, 1, 2, 3));
	p = _mm_add_ps(p, u);
	float r;
	_mm_store_ss(&r, p);
	return r;
#elif defined QM_USE_NEON
	float32x4_t mul = vmulq_f32(left->neon, right->neon);
	float32x4_t half = vpaddq_f32(mul, mul);
	float32x4_t full = vpaddq_f32(half, half);
	float r = vgetq_lane_f32(full, 0);
	return r;
#else
	return left->X * right->X + left->Y * right->Y + left->Z * right->Z + left->W * right->W;
#endif
}

/// @brief 벡터2의 외적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_cross(const QmVec2* left, const QmVec2* right)
{
	QmVec2 r =
	{
		.X = left->Y * right->X - left->X * right->Y,
		.Y = left->X * right->Y - left->Y * right->X
	};
	return r;
}

/// @brief 벡터3의 외적
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_cross(const QmVec3* left, const QmVec3* right)
{
	QmVec3 r =
	{
		.X = left->Y * right->Z - left->Z * right->Y,
		.Y = left->Z * right->X - left->X * right->Z,
		.Z = left->X * right->Y - left->Y * right->X,
	};
	return r;
}

/// @brief 벡터4 외적
/// @param pv 외적 결과를 담을 벡터4
/// @param v1 첫번째 벡터4
/// @param v2 두번째 벡터4
/// @param v3 세번째 벡터4
QN_INLINE void qm_vec4_cross(QmVec4* pv, const QmVec4* v1, const QmVec4* v2, const QmVec4* v3)
{
	QmVec4 r =
	{
		.X = v1->Y * (v2->Z * v3->W - v3->Z * v2->W) - v1->Z * (v2->Y * v3->W - v3->Y * v2->W) + v1->W * (v2->Y * v3->Z - v2->Z * v3->Y),
		.Y = -(v1->X * (v2->Z * v3->W - v3->Z * v2->W) - v1->Z * (v2->X * v3->W - v3->X * v2->W) + v1->W * (v2->X * v3->Z - v3->X * v2->Z)),
		.Z = v1->X * (v2->Y * v3->W - v3->Y * v2->W) - v1->Y * (v2->X * v3->W - v3->X * v2->W) + v1->W * (v2->X * v3->Y - v3->X * v2->Y),
		.W = -(v1->X * (v2->Y * v3->Z - v3->Y * v2->Z) - v1->Y * (v2->X * v3->Z - v3->X * v2->Z) + v1->Z * (v2->X * v3->Y - v3->X * v2->Y)),
	};
	return r;
}

/// @brief 벡터2 길이의 제곱
/// @param pv 벡터2
/// @return 길이의 제곱
QN_INLINE float qm_vec2_len_sq(const QmVec2* pv)
{
	return qm_vec2_dot(pv, pv);
}

/// @brief 벡터3의 길이의 제곱
/// @param pv 벡터3
/// @return 길이의 제곱
QN_INLINE float qm_vec3_len_sq(const QmVec3* pv)
{
	return qm_vec3_dot(pv, pv);
}

/// @brief 벡터4 거리의 제곱
/// @param pv 대상 벡터4
/// @return 벡터4 거리의 제곱값
QN_INLINE float qm_vec4_len_sq(const QmVec4* pv)
{
	return qm_vec4_dot(pv, pv);
}

/// @brief 벡터2 길이
/// @param pv 벡터2
/// @return 길이
QN_INLINE float qm_vec2_len(const QmVec2* pv)
{
	return qm_sqrtf(qm_vec2_len_sq(pv));
}

/// @brief 벡터3의 길이
/// @param pv 벡터3
/// @return 길이
QN_INLINE float qm_vec3_len(const QmVec3* pv)
{
	return qm_sqrtf(qm_vec3_len_sq(pv));
}

/// @brief 벡터4 거리
/// @param pv 대상 벡터4
/// @return 벡터4 거리값
QN_INLINE float qm_vec4_len(const QmVec4* pv)
{
	return qm_sqrtf(qm_vec4_len_sq(pv));
}

/// @brief 벡터2 정규화
/// @param v 벡터2
QN_INLINE QmVec2 qm_vec2_norm(const QmVec2* v)
{
	return qm_vec2_mag(v, qm_inv_sqrtf(qm_vec2_dot(v, v)));
}

/// @brief 벡터3 정규화
/// @param v 벡터3
QN_INLINE QmVec3 qm_vec3_norm(const QmVec3* v)
{
	return qm_vec3_mag(v, qm_inv_sqrtf(qm_vec3_dot(v, v)));
}

/// @brief 벡터4 정규화
/// @param v 벡터4
QN_INLINE QmVec4 qm_vec4_norm(const QmVec4* v)
{
	return qm_vec4_mag(v, qm_inv_sqrtf(qm_vec4_dot(v, v)));
}

/// @brief 두 벡터2 거리의 제곱
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2 거리의 제곱값
QN_INLINE float qm_vec2_dist_sq(const QmVec2* left, const QmVec2* right)
{
	QmVec2 t = qm_vec2_sub(left, right);
	return qm_vec2_len_sq(&t);
}

/// @brief 두 벡터3 거리의 제곱
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3 거리의 제곱값
QN_INLINE float qm_vec3_dist_sq(const QmVec3* left, const QmVec3* right)
{
	QmVec3 t = qm_vec3_sub(left, right);
	return qm_vec3_len_sq(&t);
}

/// @brief 두 벡터4 거리의 제곱
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터4 거리의 제곱값
QN_INLINE float qm_vec4_dist_sq(const QmVec4* left, const QmVec4* right)
{
	QmVec4 t = qm_vec4_sub(left, right);
	return qm_vec4_len_sq(&t);
}

/// @brief 두 벡터2의 거리
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2의 거리값
QN_INLINE float qm_vec2_dist(const QmVec2* left, const QmVec2* right)
{
	return qm_sqrtf(qm_vec2_dist_sq(left, right));
}

/// @brief 두 벡터3의 거리
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3의 거리값
QN_INLINE float qm_vec3_dist(const QmVec3* left, const QmVec3* right)
{
	return qm_sqrtf(qm_vec3_dist_sq(left, right));
}

/// @brief 두 벡터4의 거리
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터3의 거리값
QN_INLINE float qm_vec4_dist(const QmVec4* left, const QmVec4* right)
{
	return qm_sqrtf(qm_vec4_dist_sq(left, right));
}

/// @brief 벡터2 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec2 qm_vec2_interpolate(const QmVec2* left, const QmVec2* right, const float scale)
{
	QmVec2 l = qm_vec2_mag(left, 1.0f - scale);
	QmVec2 r = qm_vec2_mag(right, scale);
	return qm_vec2_add(&l, &r);
}

/// @brief 벡터3 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec3 qm_vec3_interpolate(const QmVec3* left, const QmVec3* right, const float scale)
{
	QmVec3 l = qm_vec3_mag(left, 1.0f - scale);
	QmVec3 r = qm_vec3_mag(right, scale);
	return qm_vec3_add(&l, &r);
}

/// @brief 벡터4 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec4 qm_vec4_interpolate(const QmVec4* left, const QmVec4* right, const float scale)
{
	QmVec4 l = qm_vec4_mag(left, 1.0f - scale);
	QmVec4 r = qm_vec4_mag(right, scale);
	return qm_vec4_add(&l, &r);
}

/// @brief 벡터2 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec2 qm_vec2_lerp(const QmVec2* left, const QmVec2* right, const float scale)
{
	QmVec2 sub = qm_vec2_sub(right, left);
	QmVec2 mag = qm_vec2_mag(&sub, scale);
	return qm_vec2_add(left, &mag);
}

/// @brief 벡터3 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec3 qm_vec3_lerp(const QmVec3* left, const QmVec3* right, const float scale)
{
	QmVec3 sub = qm_vec3_sub(right, left);
	QmVec3 mag = qm_vec3_mag(&sub, scale);
	return qm_vec3_add(left, &mag);
}

/// @brief 벡터4 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec4 qm_vec4_lerp(const QmVec4* left, const QmVec4* right, const float scale)
{
	QmVec4 sub = qm_vec4_sub(right, left);
	QmVec4 mag = qm_vec4_mag(&sub, scale);
	return qm_vec4_add(left, &mag);
}

/// @brief 벡터3의 방향
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_dir(const QmVec3* left, const QmVec3* right)
{
	QmVec3 t = qm_vec3_sub(left, right);
	return qm_vec3_norm(&t);
}

/// @brief 두 벡터3의 반지름의 제곱
/// @param left 왼쪽(가운데) 벡터3
/// @param right 오른쪽(바깥쪽) 벡터3
/// @return 두 벡터3의 반지름의 제곱값
QN_INLINE float qm_vec3_rad_sq(const QmVec3* left, const QmVec3* right)
{
	QmVec3 a = qm_vec3_add(left, right);
	QmVec3 m = qm_vec3_mag(&a, 0.5f);
	QmVec3 s = qm_vec3_sub(&m, left);
	return qm_vec3_len_sq(&s);
}

/// @brief 두 벡터3의 반지름
/// @param left 왼쪽(가운데) 벡터3
/// @param right 오른쪽(바깥쪽) 벡터3
/// @return 두 벡터3의 반지름
QN_INLINE float qm_vec3_rad(const QmVec3* left, const QmVec3* right)
{
	return qm_sqrtf(qm_vec3_rad_sq(left, right));
}

/// @brief 벡터3 트랜스폼
/// @param v 벡터3
/// @param trfm 변환 행렬
QN_INLINE QmVec3 qm_vec3_trfm(const QmVec3* v, const QmMat4* trfm)
{
	QmVec3 r =
	{
		.X = v->X * trfm->_11 + v->Y * trfm->_21 + v->Z * trfm->_31 + trfm->_41,
		.Y = v->X * trfm->_12 + v->Y * trfm->_22 + v->Z * trfm->_32 + trfm->_42,
		.Z = v->X * trfm->_13 + v->Y * trfm->_23 + v->Z * trfm->_33 + trfm->_43,
	};
	return r;
}

/// @brief 벡터3 정규화 트랜스폼
/// @param v 벡터3
/// @param trfm 정규화된 변환 행렬
QN_INLINE QmVec3 qm_vec3_trfm_norm(const QmVec3* v, const QmMat4* trfm)
{
	QmVec3 r =
	{
		.X = v->X * trfm->_11 + v->Y * trfm->_21 + v->Z * trfm->_31,
		.Y = v->X * trfm->_12 + v->Y * trfm->_22 + v->Z * trfm->_32,
		.Z = v->X * trfm->_13 + v->Y * trfm->_23 + v->Z * trfm->_33,
	};
	return r;
}

/// @brief 벡터4 트랜스폼
/// @param v 원본 벡터4
/// @param trfm 변환 행렬
QN_INLINE QmVec4 qm_vec4_trfm(const QmVec4* v, const QmMat4* trfm)
{
#if defined QM_USE_SSE
	QmVec4 r;
	r.m128 = _mm_mul_ps(_mm_shuffle_ps(v->m128, v->m128, 0x00), trfm->rows[0].m128);
	r.m128 = _mm_add_ps(r.m128, _mm_mul_ps(_mm_shuffle_ps(v->m128, v->m128, 0x55), trfm->rows[1].m128));
	r.m128 = _mm_add_ps(r.m128, _mm_mul_ps(_mm_shuffle_ps(v->m128, v->m128, 0xAA), trfm->rows[2].m128));
	r.m128 = _mm_add_ps(r.m128, _mm_mul_ps(_mm_shuffle_ps(v->m128, v->m128, 0xFF), trfm->rows[3].m128));
#elif defined QM_USE_NEON
	QmVec4 r;
	r.neon = vmulq_laneq_f32(trfm->rows[0].neon, v->neon, 0);
	r.neon = vfmaq_laneq_f32(r.neon, trfm->rows[1].neon, v->neon, 1);
	r.neon = vfmaq_laneq_f32(r.neon, trfm->rows[2].neon, v->neon, 2);
	r.neon = vfmaq_laneq_f32(r.neon, trfm->rows[3].neon, v->neon, 3);
#else
	QmVec4 r =
	{
		.X = v->X * trfm->_11 + v->Y * trfm->_21 + v->Z * trfm->_31 + v->W * trfm->_41,
		.Y = v->X * trfm->_12 + v->Y * trfm->_22 + v->Z * trfm->_32 + v->W * trfm->_42,
		.Z = v->X * trfm->_13 + v->Y * trfm->_23 + v->Z * trfm->_33 + v->W * trfm->_43,
		.W = v->X * trfm->_14 + v->Y * trfm->_24 + v->Z * trfm->_34 + v->W * trfm->_44,
	};
#endif
	return r;
}


//////////////////////////////////////////////////////////////////////////
// 사원수

/// @brief 사원수 값 설정
/// @param x,y,z,w 사원수 요소
/// @return 만든 사원수
QN_INLINE QmQuat qm_quat(const float x, const float y, const float z, const float w)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(x, y, z, w) };
#elif defined QM_USE_NEON
	{.neon = { x, y, z, w } };
#else
	{.x = x, .y = y, .z = z, .w = w };
#endif
	return r;
}

/// @brief 사원수 값 설정
/// @param v 벡타4
/// @return 만든 사원수
QN_INLINE QmQuat qm_quat_vec4(const QmVec4* v)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = v->m128 };
#elif defined QM_USE_NEON
	{.neon = v->neon };
#else
	{.x = v->X, .y = v->Y, .z = v->Z, .w = v->W };
#endif
	return r;
}

/// @brief 사원수 값 설정
/// @param p 입력 벡터3
/// @param w w 요소
/// @return 만든 사원수
QN_INLINE QmQuat qm_quat_vec3(const QmVec3* p, const float w)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(p->X, p->Y, p->Z, w) };
#elif defined QM_USE_NEON
	{.neon = { p->X, p->Y, p->Z, w } };
#else
	{.x = p->X, .y = p->Y, .z = p->Z, .w = w };
#endif
	return r;
}

/// @brief 사원수 초기화
QN_INLINE QmQuat qm_quat_rst(void)		// identify
{
	QmQuat v =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f) };
#elif defined QM_USE_NEON
	{.neon = { 0.0f, 0.0f, 0.0f, 1.0f } };
#else
	{.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f };
#endif
	return v;
}

/// @brief 사원수 반전
/// @param v 원본 사원수
QN_INLINE QmQuat qm_quat_ivt(const QmQuat* v)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(-v->X, -v->Y, -v->Z, -v->W) };
#elif defined QM_USE_NEON
	{.neon = { -v->X, -v->Y, -v->Z, -v->W } };
#else
	{.X = -v->X, .Y = -v->Y, .Z = -v->Z, .W = -v->W };
#endif
	return r;
}

/// @brief 사원수 네거티브
/// @param pv 네거티브 결과를 담을 사원수
/// @param v 원본 사원수
QN_INLINE void qm_quat_neg(QmQuat* pv, const QmQuat* v)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(1.0f - v->X, 1.0f - v->Y, 1.0f - v->Z, 1.0f - v->W) };
#elif defined QM_USE_NEON
	{.neon = { 1.0f - v->X, 1.0f - v->Y, 1.0f - v->Z, 1.0f - v->W } };
#else
	{.X = 1.0f - v->X, .Y = 1.0f - v->Y, .Z = 1.0f - v->Z, .W = 1.0f - v->W };
#endif
	return r;
}

/// @brief 사원수 덧셈
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE QmQuat qm_quat_add(const QmQuat* left, const QmQuat* right)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_add_ps(left->m128, right->m128) };
#elif defined QM_USE_NEON
	{.neon = vaddq_f32(left->neon, right->neon) };
#else
	{.X = left->X + right->X, .Y = left->Y + right->Y, .Z = left->Z + right->Z, .W = left->W + right->W, };
#endif
	return r;
}

/// @brief 사원수 뺄셈
/// @param pv 뺄셈 결과를 담을 사원수
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE QmQuat qm_quat_sub(const QmQuat* left, const QmQuat* right)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_sub_ps(left->m128, right->m128) };
#elif defined QM_USE_NEON
	{.neon = vsubq_f32(left->neon, right->neon) };
#else
	{.X = left->X - right->X, .Y = left->Y - right->Y, .Z = left->Z - right->Z, .W = left->W - right->W, };
#endif
	return r;
}

/// @brief 사원수 확대
/// @param left 원본 사원수
/// @param right 확대값
QN_INLINE QmQuat qm_quat_mag(const QmQuat* left, const float right)
{
#if defined QM_USE_SSE
	__m128 m = _mm_set1_ps(right);
	QmQuat r = { .m128 = _mm_mul_ps(left->m128, m) };
#elif defined QM_USE_NEON
	QmQuat r = { .neon = vmulq_n_f32(left->neon, right) };
#else
	QmQuat r = { .X = left->X * right, .Y = left->Y * right, .Z = left->Z * right, .W = left->W * right };
#endif
	return r;
}

/// @brief 사원수 항목 곱셈
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE QmQuat qm_quat_mul(const QmQuat* left, const QmQuat* right)
{
#if defined QM_USE_SSE
	QmQuat r;
	__m128 a = _mm_xor_ps(_mm_shuffle_ps(left->m128, left->m128, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.f, -0.f, 0.f, -0.f));
	__m128 b = _mm_shuffle_ps(right->m128, right->m128, _MM_SHUFFLE(0, 1, 2, 3));
	__m128 c = _mm_mul_ps(b, a);
	a = _mm_xor_ps(_mm_shuffle_ps(left->m128, left->m128, _MM_SHUFFLE(1, 1, 1, 1)), _mm_setr_ps(0.f, 0.f, -0.f, -0.f));
	b = _mm_shuffle_ps(right->m128, right->m128, _MM_SHUFFLE(1, 0, 3, 2));
	c = _mm_add_ps(c, _mm_mul_ps(b, a));
	a = _mm_xor_ps(_mm_shuffle_ps(left->m128, left->m128, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.f, 0.f, 0.f, -0.f));
	b = _mm_shuffle_ps(right->m128, right->m128, _MM_SHUFFLE(2, 3, 0, 1));
	c = _mm_add_ps(c, _mm_mul_ps(b, a));
	a = _mm_shuffle_ps(left->m128, left->m128, _MM_SHUFFLE(3, 3, 3, 3));
	b = _mm_shuffle_ps(right->m128, right->m128, _MM_SHUFFLE(3, 2, 1, 0));
	r.m128 = _mm_add_ps(c, _mm_mul_ps(b, a));
#elif defined QM_USE_NEON
	QmQuat r;
	float32x4_t r1032 = vrev64q_f32(right->neon);
	float32x4_t r3210 = vcombine_f32(vget_high_f32(r1032), vget_low_f32(r1032));
	float32x4_t r2301 = vrev64q_f32(r3210);
	float32x4_t sign1 = { 1.0f, -1.0f, 1.0f, -1.0f };
	r.neon = vmulq_f32(r3210, vmulq_f32(vdupq_laneq_f32(left->neon, 0), sign1));
	float32x4_t sign2 = { 1.0f, 1.0f, -1.0f, -1.0f };
	r.neon = vfmaq_f32(r.neon, r2301, vmulq_f32(vdupq_laneq_f32(left->neon, 1), sign2));
	float32x4_t sign3 = { -1.0f, 1.0f, 1.0f, -1.0f };
	r.neon = vfmaq_f32(r.neon, r1032, vmulq_f32(vdupq_laneq_f32(left->neon, 2), sign3));
	r.neon = vfmaq_laneq_f32(r.neon, right->neon, left->neon, 3);
#else
	QmQuat r =
	{
		.X = left->X * right->W + left->Y * right->Z - left->Z * right->Y + left->W * right->X;
		.Y = -left->X * right->Z + left->Y * right->W + left->Z * right->X + left->W * right->Y;
		.Z = left->X * right->Y - left->Y * right->X + left->Z * right->W + left->W * right->Z;
		.W = -left->X * right->X - left->Y * right->Y - left->Z * right->Z + left->W * right->W;
	};
#endif
	return r;
}

/// @brief 사원수의 최소값
/// @param pv 최소값을 담을 사원수
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE void qm_quat_min(QmQuat* pv, const QmQuat* left, const QmQuat* right)
{
	pv->X = (left->X < right->X) ? left->X : right->X;
	pv->Y = (left->Y < right->Y) ? left->Y : right->Y;
	pv->Z = (left->Z < right->Z) ? left->Z : right->Z;
	pv->W = (left->W < right->W) ? left->W : right->W;
}

/// @brief 사원수의 최대값
/// @param pv 최대값을 담을 사원수
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE void qm_quat_max(QmQuat* pv, const QmQuat* left, const QmQuat* right)
{
	pv->X = (left->X > right->X) ? left->X : right->X;
	pv->Y = (left->Y > right->Y) ? left->Y : right->Y;
	pv->Z = (left->Z > right->Z) ? left->Z : right->Z;
	pv->W = (left->W > right->W) ? left->W : right->W;
}

/// @brief 두 사원수를 비교
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
/// @return 두 사원수가 같으면 참
QN_INLINE bool qm_quat_eq(const QmQuat* left, const QmQuat* right)
{
	return
		qm_eqf(left->X, right->X) && qm_eqf(left->Y, right->Y) &&
		qm_eqf(left->Z, right->Z) && qm_eqf(left->W, right->W);
}

/// @brief 사원수가 0인지 비교
/// @param pv 비교할 사원수
/// @return 사원수가 0이면 참
QN_INLINE bool qm_quat_isi(const QmQuat* pv)
{
	return pv->X == 0.0f && pv->Y == 0.0f && pv->Z == 0.0f && pv->W == 1.0f;
}

/// @brief 켤레 사원수
/// @param q 원본 사원수
QN_INLINE QmQuat qm_quat_conjugate(const QmQuat* q)
{
	QmQuat r = { .X = -q->X, .Y = -q->Y, .Z = -q->Z, .W = q->W, };
	return r;
}

/// @brief 역사원수를 얻는다
/// @param q 원본 사원수
QN_INLINE QmQuat qm_quat_inv(const QmQuat* q)
{
	QmQuat cjg = qm_quat_conjugate(q);
	float dot = qm_quat_dot(q, q);
	return qm_quat_mag(&cjg, 1.0f / dot);
}

/// @brief 사원수 내적
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
/// @return 내적값
QN_INLINE float qm_quat_dot(const QmQuat* left, const QmQuat* right)
{
#if defined QM_USE_SSE
	__m128 p = _mm_mul_ps(left->m128, right->m128);
	__m128 u = _mm_shuffle_ps(p, p, _MM_SHUFFLE(2, 3, 0, 1));
	p = _mm_add_ps(p, u);
	u = _mm_shuffle_ps(p, p, _MM_SHUFFLE(0, 1, 2, 3));
	p = _mm_add_ps(p, u);
	float r;
	_mm_store_ss(&r, p);
	return r;
#elif defined QM_USE_NEON
	float32x4_t mul = vmulq_f32(left->neon, right->neon);
	float32x4_t half = vpaddq_f32(mul, mul);
	float32x4_t full = vpaddq_f32(half, half);
	float r = vgetq_lane_f32(full, 0);
	return r;
#else
	return left->X * right->X + left->Y * right->Y + left->Z * right->Z + left->W * right->W;
#endif
}

/// @brief 사원수 외적
/// @param pv 외적 결과를 담을 사원수
/// @param v1 첫번째 사원수
/// @param v2 두번째 사원수
/// @param v3 세번째 사원수
QN_INLINE void qm_quat_cross(QmQuat* pv, const QmQuat* v1, const QmQuat* v2, const QmQuat* v3)
{
	QmQuat r =
	{
		.X = v1->Y * (v2->Z * v3->W - v3->Z * v2->W) - v1->Z * (v2->Y * v3->W - v3->Y * v2->W) + v1->W * (v2->Y * v3->Z - v2->Z * v3->Y),
		.Y = -(v1->X * (v2->Z * v3->W - v3->Z * v2->W) - v1->Z * (v2->X * v3->W - v3->X * v2->W) + v1->W * (v2->X * v3->Z - v3->X * v2->Z)),
		.Z = v1->X * (v2->Y * v3->W - v3->Y * v2->W) - v1->Y * (v2->X * v3->W - v3->X * v2->W) + v1->W * (v2->X * v3->Y - v3->X * v2->Y),
		.W = -(v1->X * (v2->Y * v3->Z - v3->Y * v2->Z) - v1->Y * (v2->X * v3->Z - v3->X * v2->Z) + v1->Z * (v2->X * v3->Y - v3->X * v2->Y)),
	};
	return r;
}

/// @brief 사원수 정규화
/// @param v 사원수
QN_INLINE QmQuat qm_quat_norm(const QmQuat* v)
{
	return qm_quat_mag(v, qm_inv_sqrtf(qm_quat_dot(v, v)));
}

/// @brief 사원수 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 사원수
/// @param right 대상 사원수
/// @param scale 보간값
QN_INLINE QmQuat qm_quat_interpolate(const QmQuat* left, const QmQuat* right, const float scale)
{
	QmQuat l = qm_quat_mag(left, 1.0f - scale);
	QmQuat r = qm_quat_mag(right, scale);
	return qm_quat_add(&l, &r);
}

/// @brief 두 사원수를 블렌딩한다
/// @param left 왼쪽 사원수
/// @param left_scale 왼쪽 값
/// @param right 오른쪽 사원수
/// @param right_scale 오른쪽 값
QN_INLINE QmQuat qm_quat_blend(const QmQuat* left, const float left_scale, const QmQuat* right, const float right_scale)
{
#if defined QM_USE_SSE
	__m128 ls = _mm_set1_ps(left_scale);
	__m128 rs = _mm_set1_ps(right_scale);
	__m128 p = _mm_mul_ps(left->m128, ls);
	__m128 u = _mm_mul_ps(right->m128, rs);
	QmQuat r = { .m128 = _mm_add_ps(p, u) };
#elif defined QM_USE_NEON
	float32x4_t ls = vmulq_n_f32(left->neon, left_scale);
	float32x4_t rs = vmulq_n_f32(right->neon, right_scale);
	QmQuat r = { .neon = vaddq_f32(ls, rs) };
#else
	QmQuat r =
	{
		.X = left->X * left_scale + right->X * right_scale;
		.Y = left->Y * left_scale + right->Y * right_scale;
		.Z = left->Z * left_scale + right->Z * right_scale;
		.W = left->W * left_scale + right->W * right_scale;
	}
#endif
	return r;
}

/// @brief 사원수 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 사원수
/// @param right 대상 사원수
/// @param scale 보간값
QN_INLINE QmQuat qm_quat_lerp(const QmQuat* left, const QmQuat* right, const float scale)
{
	QmQuat b = qm_quat_blend(left, 1.0f - scale, right, scale);
	return qm_quat_norm(&b);
}

/// @brief 사원수의 스플라인 lerp
/// @param left 기준 사원수
/// @param right 대상 사원수
/// @param change 변화량
QN_INLINE QmQuat qm_quat_slerp(const QmQuat* restrict left, const QmQuat* restrict right, const float scale)
{
	float dot = qm_quat_dot(left, right);
	QmQuat q1, q2;
	if (dot < 0.0f)
	{
		dot = -dot;
		q1 = *left;
		q2 = qm_quat(-right->X, -right->Y, -right->Z, -right->W);
	}
	else
	{
		q1 = *left;
		q2 = *right;
	}
	if (dot > 0.9995f)
		return qm_quat_lerp(&q1, &q2, scale);
	float angle = QM_ACOSF(dot);
	float ls = QM_SINF((1.0f - scale) * angle);
	float rs = QM_SINF(scale * angle);
	QmQuat b = qm_quat_blend(&q1, ls, &q2, rs);
	return qm_quat_norm(&b);
}

/// @brief 행렬로 회전 (왼손)
/// @param m 회전 행렬
/// @return 
QN_INLINE QmQuat qm_quat_mat_lh(const QmMat4* m)
{
	float f;
	QmQuat q;
	if (m->_33 < 0.0f)
	{
		if (m->_11 > m->_22)
		{
			f = 1 + m->_11 - m->_22 - m->_33;
			q = qm_quat(f, m->_12 + m->_21, m->_31 + m->_13, m->_32 - m->_23);
		}
		else {
			f = 1 - m->_11 + m->_22 - m->_33;
			q = qm_quat(m->_12 + m->_21, f, m->_23 + m->_32, m->_13 - m->_31);
		}
	}
	else
	{
		if (m->_11 < -m->_22)
		{
			f = 1 - m->_11 - m->_22 + m->_33;
			q = qm_quat(m->_31 + m->_13, m->_23 + m->_32, f, m->_21 - m->_12);
		}
		else
		{
			f = 1 + m->_11 + m->_22 + m->_33;
			q = qm_quat(m->_32 - m->_23, m->_13 - m->_31, m->_21 - m->_13, f);
		}
	}
	return qm_quat_mag(&q, 0.5f / qm_sqrtf(f));
}

/// @brief 행렬로 회전 (오른손)
/// @param m 회전 행렬
/// @return 
QN_INLINE QmQuat qm_quat_mat_rh(const QmMat4* m)
{
	float f;
	QmQuat q;
	if (m->_33 < 0.0f)
	{
		if (m->_11 > m->_22)
		{
			f = 1 + m->_11 - m->_22 - m->_33;
			q = qm_quat(f, m->_12 + m->_21, m->_31 + m->_13, m->_23 - m->_32);
		}
		else
		{
			f = 1 - m->_11 + m->_22 - m->_33;
			q = qm_quat(m->_12 + m->_21, f, m->_23 + m->_32, m->_31 - m->_13);
		}
	}
	else
	{
		if (m->_11 < -m->_22)
		{
			f = 1 - m->_11 - m->_22 + m->_33;
			q = qm_quat(m->_31 + m->_13, m->_23 + m->_32, f, m->_12 - m->_21);
		}
		else
		{
			f = 1 + m->_11 + m->_22 + m->_33;
			q = qm_quat(m->_23 - m->_32, m->_31 - m->_13, m->_12 - m->_21, f);
		}
	}
	return qm_quat_mag(&q, 0.5f / qm_sqrtf(f));
}

/// @brief 사원수를 벡터3 축으로 회전시킨다 (왼손)
/// @param v 벡터3 회전축
/// @param angle 회전값
QN_INLINE QmQuat qm_quat_axis_vec_lh(const QmVec3* v, const float angle)
{
	float s, c;
	qm_sincos(angle * -0.5f, &s, &c);
	QmVec3 norm = qm_vec3_norm(v);
	QmVec3 mag = qm_vec3_mag(&norm, s);
	return qm_quat_vec3(&mag, c);
}

/// @brief 사원수를 벡터3 축으로 회전시킨다 (오른손)
/// @param v 벡터3 회전축
/// @param angle 회전값
QN_INLINE QmQuat qm_quat_axis_vec_rh(const QmVec3* v, const float angle)
{
	float s, c;
	qm_sincos(angle * 0.5f, &s, &c);
	QmVec3 norm = qm_vec3_norm(v);
	QmVec3 mag = qm_vec3_mag(&norm, s);
	return qm_quat_vec3(&mag, c);
}



//////////////////////////////////////////////////////////////////////////
// 행렬

/// @brief 행렬을 0으로 초기화 한다
/// @param pm 초기화할 대상 행렬
QN_INLINE QmMat4 qm_mat4_zero(void)
{
	QmMat4 r = { 0.0f };
	return r;
}

/// @brief 단위 행렬을 만든다
/// @param pm 단위 행렬로 만들 행렬
QN_INLINE QmMat4 qm_mat4_rst(void)		// identify
{
	QmMat4 r =
	{
		._11 = 1.0f, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = 1.0f, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f, ._34 = 0.0f,
		._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = 1.0f,
	};
	return r;
}

/// @brief 대각 행렬을 만든다
/// @param diag 대각값
QN_INLINE QmMat4 qm_mat4_diag(const float diag)
{
	QmMat4 r =
	{
		._11 = diag, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = diag, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = diag, ._34 = 0.0f,
		._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = diag,
	};
	return r;
}

/// @brief 단위 행렬인지 비교
/// @param pm 비교할 행렬
/// @return 단위 행렬이면 참을 반환
QN_INLINE bool qm_mat4_isi(const QmMat4* pm)
{
	return
		pm->_11 == 1.0f && pm->_12 == 0.0f && pm->_13 == 0.0f && pm->_14 == 0.0f &&
		pm->_21 == 0.0f && pm->_22 == 1.0f && pm->_23 == 0.0f && pm->_24 == 0.0f &&
		pm->_31 == 0.0f && pm->_32 == 0.0f && pm->_33 == 1.0f && pm->_34 == 0.0f &&
		pm->_41 == 0.0f && pm->_42 == 0.0f && pm->_43 == 0.0f && pm->_44 == 1.0f;
}

/// @brief 행렬 전치
/// @param m 전치할 행렬
QN_INLINE QmMat4 qm_mat4_tran(const QmMat4* m)
{
#if defined QM_USE_SSE
	QmMat4 r = *m;
	_MM_TRANSPOSE4_PS(r.rows[0].m128, r.rows[1].m128, r.rows[2].m128, r.rows[3].m128);
#elif defined QM_USE_NEON
	QmMat4 r;
	float32x4x4_t t = vld4q_f32((float*)m->rows);
	r.rows[0].neon = t.val[0];
	r.rows[1].neon = t.val[1];
	r.rows[2].neon = t.val[2];
	r.rows[3].neon = t.val[3];
#else
	QmMat4 r =
	{
		._11 = m->_11, ._12 = m->_21, ._13 = m->_31, ._14 = m->_41,
		._21 = m->_12, ._22 = m->_22, ._23 = m->_32, ._24 = m->_42,
		._31 = m->_13, ._32 = m->_23, ._33 = m->_33, ._34 = m->_43,
		._41 = m->_14, ._42 = m->_24, ._43 = m->_34, ._44 = m->_44,
	};
#endif
	return r;
}

/// @brief 두 행렬의 덧셈
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
QN_INLINE QmMat4 qm_mat4_add(const QmMat4* left, const QmMat4* right)
{
	QmMat4 r;
	r.rows[0] = qm_vec4_add(&left->rows[0], &right->rows[0]);
	r.rows[1] = qm_vec4_add(&left->rows[1], &right->rows[1]);
	r.rows[2] = qm_vec4_add(&left->rows[2], &right->rows[2]);
	r.rows[3] = qm_vec4_add(&left->rows[3], &right->rows[3]);
	return r;
}

/// @brief 두 행렬의 뺄셈
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
QN_INLINE QmMat4 qm_mat4_sub(const QmMat4* left, const QmMat4* right)
{
	QmMat4 r;
	r.rows[0] = qm_vec4_sub(&left->rows[0], &right->rows[0]);
	r.rows[1] = qm_vec4_sub(&left->rows[1], &right->rows[1]);
	r.rows[2] = qm_vec4_sub(&left->rows[2], &right->rows[2]);
	r.rows[3] = qm_vec4_sub(&left->rows[3], &right->rows[3]);
	return r;
}

/// @brief 행렬의 확대
/// @param m 대상 행렬
/// @param scale 확대값
QN_INLINE QmMat4 qm_mat4_mag(const QmMat4* m, const float scale)
{
#if defined QM_USE_SSE
	QmMat4 r;
	__m128 mm = _mm_set1_ps(scale);
	r.rows[0].m128 = _mm_mul_ps(m->rows[0].m128, mm);
	r.rows[1].m128 = _mm_mul_ps(m->rows[1].m128, mm);
	r.rows[2].m128 = _mm_mul_ps(m->rows[2].m128, mm);
	r.rows[3].m128 = _mm_mul_ps(m->rows[3].m128, mm);
#elif defined QM_USE_NEON
	QmMat4 r;
	r.rows[0].neon = vmulq_n_f32(m->rows[0].neon, scale);
	r.rows[1].neon = vmulq_n_f32(m->rows[1].neon, scale);
	r.rows[2].neon = vmulq_n_f32(m->rows[2].neon, scale);
	r.rows[3].neon = vmulq_n_f32(m->rows[3].neon, scale);
#else
	QmMat4 r =
	{
		._11 = m->_11 * scale, ._12 = m->_12 * scale, ._13 = m->_13 * scale, ._14 = m->_14 * scale,
		._21 = m->_21 * scale, ._22 = m->_22 * scale, ._23 = m->_23 * scale, ._24 = m->_24 * scale,
		._31 = m->_31 * scale, ._32 = m->_32 * scale, ._33 = m->_33 * scale, ._34 = m->_34 * scale,
		._41 = m->_41 * scale, ._42 = m->_42 * scale, ._43 = m->_43 * scale, ._44 = m->_44 * scale,
	};
#endif
	return r;
}

#if defined QM_USE_SSE
// SSE
QSAPI void qm_sse_mat4_mul(QmMat4* pm, const QmMat4* restrict left, const QmMat4* restrict right);
QSAPI void qm_sse_mat4_inv(QmMat4* restrict pm, const QmMat4* restrict m);
QSAPI float qm_sse_mat4_det(const QmMat4* m);
#endif

/// @brief 행렬 곱
/// @param left 좌측 행렬
/// @param right 우측 행렬
QN_INLINE QmMat4 qm_mat4_mul(const QmMat4* restrict left, const QmMat4* restrict right)
{
	QmMat4 r;
#if defined QM_USE_SSE
	qm_sse_mat4_mul(&r, left, right);
#else
	r.rows[0] = qm_vec4_trfm(&right->rows[0], left);
	r.rows[1] = qm_vec4_trfm(&right->rows[1], left);
	r.rows[2] = qm_vec4_trfm(&right->rows[2], left);
	r.rows[3] = qm_vec4_trfm(&right->rows[3], left);
#endif
	return r;
}

/// @brief 역행렬
/// @param pm 반환 행렬
/// @param m 입력 행렬
QN_INLINE QmMat4 qm_mat4_inv(const QmMat4* m)
{
#if defined QM_USE_SSE
	QmMat4 r;
	qm_sse_mat4_inv(&r, m);
	return r;
#else
	QmVec3 c01 = qm_vec3_cross(&m->rows[0].XYZ, &m->rows[1].XYZ);
	QmVec3 c23 = qm_vec3_cross(&m->rows[2].XYZ, &m->rows[3].XYZ);
	QmVec3 m01 = qm_vec3_mag(&m->rows[0].XYZ, m->rows[1].W);
	QmVec3 m10 = qm_vec3_mag(&m->rows[1].XYZ, m->rows[0].W);
	QmVec3 m23 = qm_vec3_mag(&m->rows[2].XYZ, m->rows[3].W);
	QmVec3 m32 = qm_vec3_mag(&m->rows[3].XYZ, m->rows[2].W);
	QmVec3 s10 = qm_vec3_sub(&m10, &m10);
	QmVec3 s32 = qm_vec3_sub(&m23, &m32);
	float inv = 1.0f / (qm_vec3_dot(&c01, &s32) + qm_vec3_dot(&c23, &s10));
	c01 = qm_vec3_mag(&c01, inv);
	c23 = qm_vec3_mag(&c23, inv);
	s10 = qm_vec3_mag(&s10, inv);
	s32 = qm_vec3_mag(&s32, inv);
	QmMat4 r;
	QmVec3 a1, a2, a3;
	a1 = qm_vec3_cross(&m->rows[1].XYZ, &s32);
	a2 = qm_vec3_mag(&c23, m->rows[1].W);
	a3 = qm_vec3_add(&a1, &a2);
	r.rows[0] = qm_vec4v(&a3, -qm_vec3_dot(&m->rows[1].XYZ, &c23));
	a1 = qm_vec3_cross(&s32, &m->rows[0].XYZ);
	a2 = qm_vec3_mag(&c23, m->rows[0].W);
	a3 = qm_vec3_sub(&a1, &a2);
	r.rows[1] = qm_vec4v(&a3, +qm_vec3_dot(&m->rows[0].XYZ, &c23));
	a1 = qm_vec3_cross(&m->rows[3].XYZ, &s10);
	a2 = qm_vec3_mag(&c01, m->rows[3].W);
	a3 = qm_vec3_add(&a1, &a2);
	r.rows[0] = qm_vec4v(&a3, -qm_vec3_dot(&m->rows[3].XYZ, &c01));
	a1 = qm_vec3_cross(&s10, &m->rows[2].XYZ);
	a2 = qm_vec3_mag(&c01, m->rows[2].W);
	a3 = qm_vec3_sub(&a1, &a2);
	r.rows[1] = qm_vec4v(&a3, +qm_vec3_dot(&m->rows[2].XYZ, &c01));
	return qm_mat4_tran(&r);
#endif
}

/// @brief 행렬식
/// @param m 행렬
/// @return 행렬식
QN_INLINE float qm_mat4_det(const QmMat4* m)
{
#if defined QM_USE_SSE
	return qm_sse_mat4_det(m);
#else
	QmVec3 c01 = qm_vec3_cross(&m->rows[0].XYZ, &m->rows[1].XYZ);
	QmVec3 c23 = qm_vec3_cross(&m->rows[2].XYZ, &m->rows[3].XYZ);
	QmVec3 m01 = qm_vec3_mag(&m->rows[0].XYZ, m->rows[1].W);
	QmVec3 m10 = qm_vec3_mag(&m->rows[1].XYZ, m->rows[0].W);
	QmVec3 m23 = qm_vec3_mag(&m->rows[2].XYZ, m->rows[3].W);
	QmVec3 m32 = qm_vec3_mag(&m->rows[3].XYZ, m->rows[2].W);
	QmVec3 s10 = qm_vec3_sub(&m10, &m10);
	QmVec3 s32 = qm_vec3_sub(&m23, &m32);
	return qm_vec3_dot(&c01, &s32) + qm_vec3_dot(&c23, &s10);
#endif
}

/// @brief 전치곱
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
QN_INLINE QmMat4 qm_mat4_tmul(const QmMat4* restrict left, const QmMat4* restrict right)
{
	QmMat4 m = qm_mat4_mul(left, right);
	return qm_mat4_tran(&m);
}

/// @brief 보기 행렬을 만든다 (왼손 기준)
/// @param eye 시선의 위치
/// @param at 바라보는 방향
/// @param up 시선의 윗쪽 방향
QN_INLINE QmMat4 qm_mat4_lookat_lh(const QmVec3* eye, const QmVec3* at, const QmVec3* up)
{
	QmVec3 vx, vy, vz;
	vz = qm_vec3_sub(at, eye);
	vz = qm_vec3_norm(&vz);
	vx = qm_vec3_cross(up, &vz);
	vx = qm_vec3_norm(&vx);
	vy = qm_vec3_cross(&vz, &vx);
	QmMat4 r =
	{
		._11 = vx.X, ._12 = vy.X, ._13 = vz.X, ._14 = 0.0f,
		._21 = vx.Y, ._22 = vy.Y, ._23 = vz.Y, ._24 = 0.0f,
		._31 = vx.Z, ._32 = vy.Z, ._33 = vz.Z, ._34 = 0.0f,
		._41 = -qm_vec3_dot(&vx, eye),
		._42 = -qm_vec3_dot(&vy, eye),
		._43 = -qm_vec3_dot(&vz, eye),
		._44 = 1.0f,
	};
	return r;
}

/// @brief 보기 행렬을 만든다 (오른손 기준)
/// @param eye 시선의 위치
/// @param at 바라보는 방향
/// @param up 시선의 윗쪽 방향
/// @return
QN_INLINE QmMat4 qm_mat4_lookat_rh(const QmVec3* eye, const QmVec3* at, const QmVec3* up)
{
	QmVec3 vx, vy, vz;
	vz = qm_vec3_sub(eye, at);
	vz = qm_vec3_norm(&vz);
	vx = qm_vec3_cross(up, &vz);
	vx = qm_vec3_norm(&vx);
	vy = qm_vec3_cross(&vz, &vx);
	QmMat4 r =
	{
		._11 = vx.X, ._12 = vy.X, ._13 = vz.X, ._14 = 0.0f,
		._21 = vx.Y, ._22 = vy.Y, ._23 = vz.Y, ._24 = 0.0f,
		._31 = vx.Z, ._32 = vy.Z, ._33 = vz.Z, ._34 = 0.0f,
		._41 = -qm_vec3_dot(&vx, eye),
		._42 = -qm_vec3_dot(&vy, eye),
		._43 = -qm_vec3_dot(&vz, eye),
		._44 = 1.0f,
	};
	return r;
}

/// @brief 보기 행렬의 역행렬 
/// @param lm 보기 행렬
/// @return 
QN_INLINE QmMat4 qm_mat4_inv_lookat(const QmMat4* lm)
{
	QmMat4 t = qm_mat4_tran(lm);
	QmMat4 r;
	r.rows[0] = qm_vec4v(&t.rows[0].XYZ, 0.0f);
	r.rows[1] = qm_vec4v(&t.rows[1].XYZ, 0.0f);
	r.rows[2] = qm_vec4v(&t.rows[2].XYZ, 0.0f);
	r.rows[3] = qm_vec4v(&t.rows[3].XYZ, -1.0f);
	r._41 = -1.0f * lm->_41 / (t._11 + t._12 + t._13);
	r._42 = -1.0f * lm->_42 / (t._21 + t._22 + t._23);
	r._43 = -1.0f * lm->_43 / (t._31 + t._32 + t._33);
	r._44 = 1.0f;
	return r;
}

/// @brief 투영 행렬을 만든다 (왼손 기준)
/// @param fov 포브(Field Of View)값
/// @param aspect 화면 종횡비(가로 나누기 세로)
/// @param depth 뎁스 너비
QN_INLINE QmMat4 qm_mat4_perspective_lh(const float fov, const float aspect, const float zn, const float zf)
{
	const float f = 1.0f / QM_TANF(fov * 0.5f);
	const float q = zf / (zf - zn);
	QmMat4 r =
	{
		._11 = f / aspect,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = f,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = q,
		._34 = 1.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = -zf * q,
		._44 = 0.0f,
	};
	return r;
}

/// @brief 투영 행렬을 만든다 (오른손 기준)
/// @param fov 포브(Field Of View)값
/// @param aspect 화면 종횡비(가로 나누기 세로)
/// @param depth 뎁스 너비
QN_INLINE QmMat4 qm_mat4_perspective_rh(const float fov, const float aspect, const float zn, const float zf)
{
	const float f = 1.0f / QM_TANF(fov * 0.5f);
	const float q = zf / (zf - zn);
	QmMat4 r =
	{
		._11 = f / aspect,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = f,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = q,
		._34 = -1.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = zn * q,
		._44 = 0.0f,
	};
	return r;
}

/// @brief 투영 행렬의 역행렬 (왼손)
/// @param om 투영 행렬
QN_INLINE QmMat4 qm_mat4_inv_perspective_lh(const QmMat4* pm)
{
	float f = 1.0f / pm->_43;
	QmMat4 r =
	{
		._11 = 1.0f / pm->_11,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = 1.0f / pm->_22,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 0.0f,
		._34 = f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = pm->_34,
		._44 = pm->_33 * -f,
	};
	return r;
}

/// @brief 투영 행렬의 역행렬 (오른손)
/// @param om 투영 행렬
QN_INLINE QmMat4 qm_mat4_inv_perspective_rh(const QmMat4* pm)
{
	float f = 1.0f / pm->_43;
	QmMat4 r =
	{
		._11 = 1.0f / pm->_11,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = 1.0f / pm->_22,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 0.0f,
		._34 = f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = pm->_34,
		._44 = pm->_33 * f,
	};
	return r;
}

/// @brief 정규 행렬를 만든다 (왼손 기준)
/// @param width 너비
/// @param height 높이
/// @param zn 깊이 가까운곳
/// @param zf 깊이 먼곳
QN_INLINE QmMat4 qm_mat4_ortho_lh(const float width, const float height, const float zn, const float zf)
{
	QmMat4 r =
	{
		._11 = 2.0f / width,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = 2.0f / height,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 1.0f / (zf - zn),
		._34 = 0.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = zn / (zn - zf),
		._44 = 1.0f,
	};
	return r;
}

/// @brief 정규 행렬을 만든다 (오른손 기준)
/// @param width 너비
/// @param height 높이
/// @param zn 깊이 가까운곳
/// @param zf 깊이 먼곳
QN_INLINE QmMat4 qm_mat4_ortho_rh(const float width, const float height, const float zn, const float zf)
{
	QmMat4 r =
	{
		._11 = 2.0f / width,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = 2.0f / height,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 1.0f / (zn - zf),
		._34 = 0.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = zn / (zn - zf),
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
QN_INLINE QmMat4 qm_mat4_ortho_offcenter_lh(const float left, const float top, const float right, const float bottom,
	const float zn, const float zf)
{
	QmMat4 r =
	{
		._11 = 2.0f / (right - left),
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = 2.0f / (top - bottom),
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 1.0f / (zf - zn),
		._34 = 0.0f,
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
QN_INLINE QmMat4 qm_mat4_ortho_offcenter_rh(const float left, const float top, const float right, const float bottom,
	const float zn, const float zf)
{
	QmMat4 r =
	{
		._11 = 2.0f / (right - left),
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = 2.0f / (top - bottom),
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 1.0f / (zn - zf),
		._34 = 0.0f,
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
QN_INLINE QmMat4 qm_mat4_viewport(const float x, const float y, const float width, const float height)
{
	QmMat4 r =
	{
		._11 = width * 0.5f,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = height * -0.5f,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 1.0f,
		._34 = 0.0f,
		._41 = x + width * 0.5f,
		._42 = y - height * -0.5f,
		._43 = 0.0f,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 정규 행렬의 역행렬
/// @param om 정규 행렬
QN_INLINE QmMat4 qm_mat4_inv_ortho(const QmMat4* om)
{
	QmVec3 diag = { .X = 1.0f / om->_11, .Y = 1.0f / om->_22, .Z = 1.0f / om->_33 };
	QmMat4 r =
	{
		._11 = diag.X,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = diag.Y,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = diag.Z,
		._34 = 0.0f,
		._41 = -om->_41 * diag.X,
		._42 = -om->_42 * diag.Y,
		._43 = -om->_43 * diag.Z,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 위치 행렬을 만든다
/// @param x,y,z 좌표
QN_INLINE QmMat4 qm_mat4_loc(const float x, const float y, const float z)
{
	QmMat4 r =
	{
		._11 = 1.0f, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = 1.0f, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f, ._34 = 0.0f,
		._41 = x, ._42 = y, ._43 = z, ._44 = 1.0f,
	};
	return r;
}

/// @brief 위치 행렬을 만든다
/// @param v 좌표 벡터
QN_INLINE QmMat4 qm_mat4_loc_vec(const QmVec3* v)
{
	return qm_mat4_loc(v->X, v->Y, v->Z);
}

/// @brief 위치 행렬의 역행렬
/// @param lm 위치 행렬
QN_INLINE QmMat4 qm_mat4_inv_loc(const QmMat4* lm)
{
	QmMat4 r =
	{
		._11 = 1.0f, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = 1.0f, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f, ._34 = 0.0f,
		._41 = -lm->_41, ._42 = -lm->_42, ._43 = -lm->_43, ._44 = 1.0f,
	};
	return r;
}

/// @brief 오른손 회전
/// @param angle 회전 각도
/// @param axis 회전 축
QN_INLINE QmMat4 qm_mat4_rot_lh(const float angle, const QmVec3* axis)
{
	QmVec3 norm = qm_vec3_norm(axis);
	float s, c;
	qm_sincos(-angle, &s, &c);
	float nc = 1.0f - c;
	QmMat4 r =
	{
		._11 = (norm.X * norm.X * nc) + c,
		._12 = (norm.X * norm.Y * nc) + (norm.Z * s),
		._13 = (norm.X * norm.Z * nc) - (norm.Y * s),
		._14 = 0.0f,
		._21 = (norm.Y * norm.X * nc) - (norm.Z * s),
		._22 = (norm.Y * norm.Y * nc) + c,
		._23 = (norm.Y * norm.Z * nc) + (norm.X * s),
		._24 = 0.0f,
		._31 = (norm.Z * norm.X * nc) + (norm.Y * s),
		._32 = (norm.Z * norm.Y * nc) - (norm.X * s),
		._33 = (norm.Z * norm.Z * nc) + c,
		._34 = 0.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = 0.0f,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 왼손 회전
/// @param angle 회전 각도
/// @param axis 회전 축
QN_INLINE QmMat4 qm_mat4_rot_rh(const float angle, const QmVec3* axis)
{
	QmVec3 norm = qm_vec3_norm(axis);
	float s, c;
	qm_sincos(angle, &s, &c);
	float nc = 1.0f - c;
	QmMat4 r =
	{
		._11 = (norm.X * norm.X * nc) + c,
		._12 = (norm.X * norm.Y * nc) + (norm.Z * s),
		._13 = (norm.X * norm.Z * nc) - (norm.Y * s),
		._14 = 0.0f,
		._21 = (norm.Y * norm.X * nc) - (norm.Z * s),
		._22 = (norm.Y * norm.Y * nc) + c,
		._23 = (norm.Y * norm.Z * nc) + (norm.X * s),
		._24 = 0.0f,
		._31 = (norm.Z * norm.X * nc) + (norm.Y * s),
		._32 = (norm.Z * norm.Y * nc) - (norm.X * s),
		._33 = (norm.Z * norm.Z * nc) + c,
		._34 = 0.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = 0.0f,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 회전 행렬의 역행렬
/// @param rm 회전 행렬
QN_INLINE QmMat4 qm_mat4_inv_rot(const QmMat4* rm)
{
	return qm_mat4_tran(rm);
}

/// @brief 스케일 행렬을 만든다
/// @param x,y,z 각 축 별 스케일 값
QN_INLINE QmMat4 qm_mat4_scl(const float x, const float y, const float z)
{
	QmMat4 r =
	{
		._11 = x, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = y, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = z, ._34 = 0.0f,
		._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = 1.0f,
	};
	return r;
}

/// @brief 스케일 행렬을 만든다
/// @param v 스케일 벡터
QN_INLINE QmMat4 qm_mat4_scl_vec(const QmVec3* v)
{
	return qm_mat4_scl(v->X, v->Y, v->Z);
}

/// @brief 스케일 행렬의 역행렬
/// @param sm 스케일 행렬
QN_INLINE QmMat4 qm_mat4_inv_scl(const QmMat4* sm)
{
	QmMat4 r =
	{
		._11 = 1.0f / sm->_11, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = 1.0f / sm->_22, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f / sm->_33, ._34 = 0.0f,
		._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = 1.0f,
	};
	return r;
}

/// @brief 사원수로 회전 행렬을 만든다
/// @param rot 사원수 회전값
/// @return
QN_INLINE QmMat4 qm_mat4_quat(const QmQuat* rot)
{
	QmQuat norm = qm_quat_norm(rot);
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
		._14 = 0.0f,
		._21 = 2.0f * (XY - WZ),
		._22 = 1.0f - 2.0f * (XX + ZZ),
		._23 = 2.0f * (YZ + WX),
		._24 = 0.0f,
		._31 = 2.0f * (XZ + WY),
		._32 = 2.0f * (YZ - WX),
		._33 = 1.0f - 2.0f * (XX + YY),
		._34 = 0.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = 0.0f,
		._44 = 1.0f,
	};
	return r;
}


QN_EXTC_END

#ifdef __GNUC__
#pragma GCC diagnotics pop
#endif
