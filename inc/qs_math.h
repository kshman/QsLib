//
// QsLib [MATH Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
//

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

#ifndef __EMSCRIPTEN__
#if defined _M_AMD64 || defined _M_X64 || defined __amd64__ || defined __x86_64__ || (defined _M_IX86_FP && _M_IX86_FP >= 1) || defined __SSE__
#define QM_USE_SSE		1
#endif
#if defined _M_ARM || defined __ARM_NEON
#define QM_USE_NEON		1
#endif
#endif

#include <math.h>
#if defined QM_USE_SSE
#include <emmintrin.h>
#endif
#ifdef QM_USE_NEON
#include <arm_neon.h>
#endif

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
#ifndef QM_CEILF
#define QM_CEILF		ceilf
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
} QmVec4, QmRectF, QmColor;

/// @brief 벡터4
typedef union QN_ALIGN(16) QmPlane
{
	struct
	{
		union
		{
			QmVec3 XYZ;
			struct
			{
				float A, B, C;
			};
		};
		float D;
	};
	struct
	{
		QmVec2 AB;
		QmVec2 _CD;
	};
	struct
	{
		float _A;
		QmVec2 BC;
		float _D;
	};
	struct
	{
		QmVec2 _AB;
		QmVec2 CD;
	};
	float f[4];
#if defined QM_USE_SSE
	__m128 m128;
#endif
#ifdef QM_USE_NEON
	float32x4_t neon;
#endif
} QmPlane;

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
	float l[16];
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
		float _Z;
	};
	struct
	{
		float _X;
		QmPoint YZ;
	};
	int i[3];
} QmVecI3, QmRgbI;

/// @brief 정수형 벡터4
typedef union QmVecI4
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
		int Left, Top, Right, Bottom;
	};
	struct
	{
		QmPoint LeftTop;
		QmPoint RightBottom;
	};
	int i[4];
#ifdef QM_USE_NEON
	int32x4_t neon;
#endif
} QmVecI4, QmRect;

/// @brief 정수형 색깔
typedef union QmKolor
{
	struct
	{
		byte B, G, R, A;
	};
	uint U;
} QmKolor;

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
typedef union QmVecH2
{
	struct
	{
		halfint X, Y;
	};
	struct
	{
		halfint U, V;
	};
	struct
	{
		halfint Width, Height;
	};
} QmVecH2, QmCoordH, QmPointH, QmSizeH;

/// @brief 하프 벡터3
typedef struct QmVecH3
{
	halfint X, Y, Z;
} QmVecH3;

/// @brief 하프 벡터4
typedef union QmVecH4
{
	struct
	{
		halfint X, Y, Z, W;
	};
	struct
	{
		halfint Left, Top, Right, Bottom;
	};
} QmVecH4, QmRectH;


//////////////////////////////////////////////////////////////////////////
// 인라인 함수

// 벡터2

/// @brief 벡터2 값 설정
/// @param x,y 좌표
QN_INLINE QmVec2 qm_vec2(float x, float y)
{
	QmVec2 r = { .X = x, .Y = y };
	return r;
}

/// @brief 정수 벡터2 설정
/// @param iv 정수 벡터
QN_INLINE QmVec2 qm_vec2v(const QmVecI2 iv)
{
	QmVec2 r = { .X = (float)iv.X, .Y = (float)iv.Y };
	return r;
}

/// @brief 벡터2 값 설정
///	@param v 반환 벡터
/// @param x,y 좌표
QN_INLINE void qm_vec2_set(QmVec2* v, float x, float y)
{
	v->X = x;
	v->Y = y;
}

/// @brief 정수 벡터2 설정
///	@param v 반환 벡터
/// @param iv 정수 벡터
QN_INLINE void qm_vec2_setv(QmVec2* v, const QmVecI2 iv)
{
	v->X = (float)iv.X;
	v->Y = (float)iv.Y;
}

/// @brief 벡터2 초기화
QN_INLINE void qm_vec2_rst(QmVec2* v)		// identify
{
	v->X = 0.0f;
	v->Y = 0.0f;
}

/// @brief 벡터2 대각값 설정 (모두 같은값으로 설정)
///	@param v 반환 벡터
/// @param diag 대각 값
QN_INLINE void qm_vec2_diag(QmVec2* v, const float diag)
{
	v->X = diag;
	v->Y = diag;
}

/// @brief 벡터2 반전
/// @param v 벡터2
QN_INLINE QmVec2 qm_vec2_ivt(const QmVec2 v)  // invert
{
	QmVec2 r = { .X = -v.X, .Y = -v.Y };
	return r;
}

/// @brief 벡터2 네거티브 (1 - 벡터2)
/// @param v 벡터2
QN_INLINE QmVec2 qm_vec2_neg(const QmVec2 v)
{
	QmVec2 r = { .X = 1.0f - v.X, .Y = 1.0f - v.Y };
	return r;
}

/// @brief 벡터2 덧셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_add(const QmVec2 left, const QmVec2 right)
{
	QmVec2 r = { .X = left.X + right.X, .Y = left.Y + right.Y };
	return r;
}

/// @brief 벡터2 뺄셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_sub(const QmVec2 left, const QmVec2 right)
{
	QmVec2 r = { .X = left.X - right.X, .Y = left.Y - right.Y };
	return r;
}

/// @brief 벡터2 확대
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 확대값
QN_INLINE QmVec2 qm_vec2_mag(const QmVec2 left, const float right)
{
	QmVec2 r = { .X = left.X * right, .Y = left.Y * right };
	return r;
}

/// @brief 벡터2 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec2 qm_vec2_mul(const QmVec2 left, const QmVec2 right)
{
	QmVec2 r = { .X = left.X * right.X, .Y = left.Y * right.Y };
	return r;
}

/// @brief 벡터2 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec2 qm_vec2_div(const QmVec2 left, const QmVec2 right)
{
	QmVec2 r = { .X = left.X / right.X, .Y = left.Y / right.Y };
	return r;
}

/// @brief 벡터2의 최소값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_min(const QmVec2 left, const QmVec2 right)
{
	QmVec2 r =
	{
		.X = (left.X < right.X) ? left.X : right.X,
		.Y = (left.Y < right.Y) ? left.Y : right.Y
	};
	return r;
}

/// @brief 벡터2의 최대값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_max(const QmVec2 left, const QmVec2 right)
{
	QmVec2 r =
	{
		.X = (left.X > right.X) ? left.X : right.X,
		.Y = (left.Y > right.Y) ? left.Y : right.Y
	};
	return r;
}

/// @brief 벡터2의 비교
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡트
/// @return 같으면 참
QN_INLINE bool qm_vec2_eq(const QmVec2 left, const QmVec2 right)
{
	return qm_eqf(left.X, right.X) && qm_eqf(left.Y, right.Y);
}

/// @brief 벡터2가 0인가 비교
/// @param pv 비교할 벡터2
/// @return 0이면 참
QN_INLINE bool qm_vec2_isi(const QmVec2 pv)
{
	return pv.X == 0.0f && pv.Y == 0.0f;
}

/// @brief 벡터2 내적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 내적 값
QN_INLINE float qm_vec2_dot(const QmVec2 left, const QmVec2 right)
{
	return left.X * right.X + left.Y * right.Y;
}

/// @brief 벡터2의 외적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVec2 qm_vec2_cross(const QmVec2 left, const QmVec2 right)
{
	QmVec2 r =
	{
		.X = left.Y * right.X - left.X * right.Y,
		.Y = left.X * right.Y - left.Y * right.X
	};
	return r;
}

/// @brief 벡터2 길이의 제곱
/// @param v 벡터2
/// @return 길이의 제곱
QN_INLINE float qm_vec2_len_sq(const QmVec2 v)
{
	return qm_vec2_dot(v, v);
}

/// @brief 벡터2 길이
/// @param v 벡터2
/// @return 길이
QN_INLINE float qm_vec2_len(const QmVec2 v)
{
	return qm_sqrtf(qm_vec2_len_sq(v));
}

/// @brief 벡터2 정규화
/// @param v 벡터2
QN_INLINE QmVec2 qm_vec2_norm(const QmVec2 v)
{
	return qm_vec2_mag(v, qm_inv_sqrtf(qm_vec2_dot(v, v)));
}

/// @brief 두 벡터2 거리의 제곱
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2 거리의 제곱값
QN_INLINE float qm_vec2_dist_sq(const QmVec2 left, const QmVec2 right)
{
	return qm_vec2_len_sq(qm_vec2_sub(left, right));
}

/// @brief 두 벡터2의 거리
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2의 거리값
QN_INLINE float qm_vec2_dist(const QmVec2 left, const QmVec2 right)
{
	return qm_sqrtf(qm_vec2_dist_sq(left, right));
}

/// @brief 벡터2 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec2 qm_vec2_interpolate(const QmVec2 left, const QmVec2 right, const float scale)
{
	return qm_vec2_add(qm_vec2_mag(left, 1.0f - scale), qm_vec2_mag(right, scale));
}

/// @brief 벡터2 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec2 qm_vec2_lerp(const QmVec2 left, const QmVec2 right, const float scale)
{
	return qm_vec2_add(left, qm_vec2_mag(qm_vec2_sub(right, left), scale));
}


// 벡터3

/// @brief 벡터3 값 설정
/// @param x,y,z 좌표
QN_INLINE QmVec3 qm_vec3(const float x, const float y, const float z)
{
	QmVec3 r = { .X = x, .Y = y, .Z = z };
	return r;
}

/// @brief 벡터3 값 설정
///	@param v 반환 벡터
/// @param x,y,z 좌표
QN_INLINE void qm_vec3_set(QmVec3* v, const float x, const float y, const float z)
{
	v->X = x;
	v->Y = y;
	v->Z = z;
}

/// @brief 벡터3 초기화
///	@param v 반환 벡터
QN_INLINE void qm_vec3_rst(QmVec3* v)		// identify
{
	v->X = 0.0f;
	v->Y = 0.0f;
	v->Z = 0.0f;
}

/// @brief 벡터3 대각값 설정 (모두 같은값으로 설정)
///	@param v 반환 벡터
/// @param diag 대각값
QN_INLINE void qm_vec3_diag(QmVec3* v, const float diag)
{
	v->X = diag;
	v->Y = diag;
	v->Z = diag;
}

/// @brief 벡터3 반전
/// @param v 원본 벡터3
QN_INLINE QmVec3 qm_vec3_ivt(const QmVec3 v)  // invert
{
	QmVec3 r = { .X = -v.X, .Y = -v.Y, .Z = -v.Z };
	return r;
}

/// @brief 벡터3 네거티브 (1 - 값)
/// @param v 원본 벡터3
QN_INLINE QmVec3 qm_vec3_neg(const QmVec3 v)
{
	QmVec3 r = { .X = 1.0f - v.X, .Y = 1.0f - v.Y, .Z = 1.0f - v.Z };
	return r;
}

/// @brief 벡터3 덧셈
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_add(const QmVec3 left, const QmVec3 right)
{
	QmVec3 r = { .X = left.X + right.X, .Y = left.Y + right.Y, .Z = left.Z + right.Z };
	return r;
}

/// @brief 벡터3 뺄셈
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_sub(const QmVec3 left, const QmVec3 right)
{
	QmVec3 r = { .X = left.X - right.X, .Y = left.Y - right.Y, .Z = left.Z - right.Z };
	return r;
}

/// @brief 벡터3 확대
/// @param left 원본 벡터3
/// @param right 확대값
QN_INLINE QmVec3 qm_vec3_mag(const QmVec3 left, const float right)
{
	QmVec3 r = { .X = left.X * right, .Y = left.Y * right, .Z = left.Z * right };
	return r;
}

/// @brief 벡터3 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec3 qm_vec3_mul(const QmVec3 left, const QmVec3 right)
{
	QmVec3 r = { .X = left.X * right.X, .Y = left.Y * right.Y, .Z = left.Z * right.Z };
	return r;
}

/// @brief 벡터3 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec3 qm_vec3_div(const QmVec3 left, const QmVec3 right)
{
	QmVec3 r = { .X = left.X / right.X, .Y = left.Y / right.Y, .Z = left.Z / right.Z };
	return r;
}

/// @brief 벡터3의 최소값
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_min(const QmVec3 left, const QmVec3 right)
{
	QmVec3 r =
	{
		.X = left.X < right.X ? left.X : right.X,
		.Y = left.Y < right.Y ? left.Y : right.Y,
		.Z = left.Z < right.Z ? left.Z : right.Z,
	};
	return r;
}

/// @brief 벡터3의 최대값
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_max(const QmVec3 left, const QmVec3 right)
{
	QmVec3 r =
	{
		.X = left.X > right.X ? left.X : right.X,
		.Y = left.Y > right.Y ? left.Y : right.Y,
		.Z = left.Z > right.Z ? left.Z : right.Z,
	};
	return r;
}

/// @brief 두 벡터3이 같은지 판단
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3이 같으면 참
QN_INLINE bool qm_vec3_eq(const QmVec3 left, const QmVec3 right)
{
	return qm_eqf(left.X, right.X) && qm_eqf(left.Y, right.Y) && qm_eqf(left.Z, right.Z);
}

/// @brief 벡터3이 0인지 판단
/// @param pv 벡터3
/// @return 벡터3이 0이면 참
QN_INLINE bool qm_vec3_isi(const QmVec3 pv)
{
	return pv.X == 0.0f && pv.Y == 0.0f && pv.Z == 0.0f;
}

/// @brief 벡터3의 내적
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 내적값
QN_INLINE float qm_vec3_dot(const QmVec3 left, const QmVec3 right)
{
	return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
}

/// @brief 벡터3의 외적
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_cross(const QmVec3 left, const QmVec3 right)
{
	QmVec3 r =
	{
		.X = left.Y * right.Z - left.Z * right.Y,
		.Y = left.Z * right.X - left.X * right.Z,
		.Z = left.X * right.Y - left.Y * right.X,
	};
	return r;
}

/// @brief 벡터3의 길이의 제곱
/// @param pv 벡터3
/// @return 길이의 제곱
QN_INLINE float qm_vec3_len_sq(const QmVec3 pv)
{
	return qm_vec3_dot(pv, pv);
}

/// @brief 벡터3의 길이
/// @param pv 벡터3
/// @return 길이
QN_INLINE float qm_vec3_len(const QmVec3 pv)
{
	return qm_sqrtf(qm_vec3_len_sq(pv));
}

/// @brief 벡터3 정규화
/// @param v 벡터3
QN_INLINE QmVec3 qm_vec3_norm(const QmVec3 v)
{
	return qm_vec3_mag(v, qm_inv_sqrtf(qm_vec3_dot(v, v)));
}

/// @brief 두 벡터3 거리의 제곱
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3 거리의 제곱값
QN_INLINE float qm_vec3_dist_sq(const QmVec3 left, const QmVec3 right)
{
	return qm_vec3_len_sq(qm_vec3_sub(left, right));
}

/// @brief 두 벡터3의 거리
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3의 거리값
QN_INLINE float qm_vec3_dist(const QmVec3 left, const QmVec3 right)
{
	return qm_sqrtf(qm_vec3_dist_sq(left, right));
}

/// @brief 벡터3 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec3 qm_vec3_interpolate(const QmVec3 left, const QmVec3 right, const float scale)
{
	return qm_vec3_add(qm_vec3_mag(left, 1.0f - scale), qm_vec3_mag(right, scale));
}

/// @brief 벡터3 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec3 qm_vec3_lerp(const QmVec3 left, const QmVec3 right, const float scale)
{
	return qm_vec3_add(left, qm_vec3_mag(qm_vec3_sub(right, left), scale));
}

/// @brief 벡터3의 방향
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVec3 qm_vec3_dir(const QmVec3 left, const QmVec3 right)
{
	return qm_vec3_norm(qm_vec3_sub(left, right));
}

/// @brief 두 벡터3의 반지름의 제곱
/// @param left 왼쪽(가운데) 벡터3
/// @param right 오른쪽(바깥쪽) 벡터3
/// @return 두 벡터3의 반지름의 제곱값
QN_INLINE float qm_vec3_rad_sq(const QmVec3 left, const QmVec3 right)
{
	return qm_vec3_len_sq(qm_vec3_sub(qm_vec3_mag(qm_vec3_add(left, right), 0.5f), left));
}

/// @brief 두 벡터3의 반지름
/// @param left 왼쪽(가운데) 벡터3
/// @param right 오른쪽(바깥쪽) 벡터3
/// @return 두 벡터3의 반지름
QN_INLINE float qm_vec3_rad(const QmVec3 left, const QmVec3 right)
{
	return qm_sqrtf(qm_vec3_rad_sq(left, right));
}

/// @brief 벡터3 트랜스폼
/// @param v 벡터3
/// @param trfm 변환 행렬
QN_INLINE QmVec3 qm_vec3_trfm(const QmVec3 v, const QmMat4 trfm)
{
	QmVec3 r =
	{
		.X = v.X * trfm._11 + v.Y * trfm._21 + v.Z * trfm._31 + trfm._41,
		.Y = v.X * trfm._12 + v.Y * trfm._22 + v.Z * trfm._32 + trfm._42,
		.Z = v.X * trfm._13 + v.Y * trfm._23 + v.Z * trfm._33 + trfm._43,
	};
	return r;
}

/// @brief 벡터3 정규화 트랜스폼
/// @param v 벡터3
/// @param trfm 정규화된 변환 행렬
QN_INLINE QmVec3 qm_vec3_trfm_norm(const QmVec3 v, const QmMat4 trfm)
{
	QmVec3 r =
	{
		.X = v.X * trfm._11 + v.Y * trfm._21 + v.Z * trfm._31,
		.Y = v.X * trfm._12 + v.Y * trfm._22 + v.Z * trfm._32,
		.Z = v.X * trfm._13 + v.Y * trfm._23 + v.Z * trfm._33,
	};
	return r;
}

/// @brief 벡터3 사원수 회전
/// @param rot 사원수
QN_INLINE QmVec3 qm_vec3_quat(const QmQuat rot)
{
	QmQuat q = { .X = rot.X * rot.X, .Y = rot.Y * rot.Y, .Z = rot.Z * rot.Z, .W = rot.W * rot.W };
	QmVec3 r =
	{
		.X = QM_ATAN2F(2.0f * (rot.Y * rot.Z + rot.X * rot.W), -q.X - q.Y + q.Z + q.W),
		.Y = QM_ASINF(qm_clampf(-2.0f * (rot.X * rot.Z + rot.Y * rot.W), -1.0f, 1.0f)),
		.Z = QM_ATAN2F(2.0f * (rot.X * rot.Y + rot.Z * rot.W), q.X - q.Y - q.Z + q.W),
	};
	return r;
}

/// @brief 벡터3 행렬 회전
/// @param rot 행렬
QN_INLINE QmVec3 qm_vec3_mat4(const QmMat4 rot)
{
	QmVec3 r;
	if (rot._31 == 0.0f && rot._33 == 0.0f)
	{
		r.X = rot._32 > 0.0f ? (float)(QM_PI_H + QM_PI) : (float)QM_PI_H;
		r.Z = QM_ATAN2F(rot._21, rot._23);
		r.Y = 0.0f;
	}
	else
	{
		r.Y = -QM_ATAN2F(rot._31, rot._33);
		r.X = -QM_ATAN2F(rot._32, qm_sqrtf(rot._31 * rot._31 + rot._33 * rot._33));
		r.Z = QM_ATAN2F(rot._12, qm_sqrtf(rot._11 * rot._11 + rot._13 * rot._13));
	}
	return r;
}

/// @brief 사원수와 벡터3 각도로 회전
/// @param rot 사원수
/// @param angle 각도 벡터3
QN_INLINE QmVec3 qm_vec3_quat_vec3(const QmQuat rot, const QmVec3 angle)
{
	QmQuat q1 =
	{
		.X = angle.X * rot.W,
		.Y = angle.Y * rot.W,
		.Z = angle.Z * rot.W,
		.W = -qm_vec3_dot(rot.XYZ, angle),
	};
	QmVec3 t = qm_vec3_cross(rot.XYZ, angle);
	q1.X += t.X;
	q1.Y += t.Y;
	q1.Z += t.Z;
	const QmQuat q2 =
	{
		.X = -rot.X,
		.Y = -rot.Y,
		.Z = -rot.Z,
		.W = rot.W,
	};
	QmVec3 r =
	{
		.X = q1.X * q2.W + q1.Y * q2.X - q1.Z * q2.Y + q1.W * q2.X,
		.Y = -q1.X * q2.Z + q1.Y * q2.W + q1.Z * q2.X + q1.W * q2.Y,
		.Z = q1.X * q2.Y - q1.Y * q2.X + q1.Z * q2.W + q1.W * q2.Z,
	};
	return r;
}

/// @brief 축 회전(yaw)
/// @param pv 벡터3
/// @return 축 회전 각도
QN_INLINE float qm_vec3_yaw(const QmVec3 pv)
{
	return -QM_ATANF(pv.X / pv.Y) + ((pv.X > 0.0f) ? (float)-QM_PI_H : (float)QM_PI_H);
}

/// @brief 선에 가까운 점의 위치
///	@param loc 조사할 점
/// @param begin 선의 시작 점
/// @param end 선의 끝점
QN_INLINE QmVec3 qm_vec3_closed(const QmVec3 loc, const QmVec3 begin, const QmVec3 end)
{
	QmVec3 norm = qm_vec3_sub(end, begin);
	const float d = qm_vec3_len(norm);
	norm = qm_vec3_mag(norm, 1.0f / d);
	const float t = qm_vec3_dot(norm, qm_vec3_sub(loc, begin));
	if (t < 0.0f)
		return begin;
	if (t > d)
		return end;
	return qm_vec3_add(begin, qm_vec3_mag(norm, t));
}

/// @brief 단위 벡터로 만들어 크기를 조정하고 길이 만큼 혼합
/// @param left 시작 벡터
/// @param right 끝 벡터
/// @param scale 크기 변화량
/// @param len 길이
QN_INLINE QmVec3 qm_vec3_lerp_len(const QmVec3 left, const QmVec3 right, const float scale, const float len)
{
	QmVec3 r = qm_vec3_lerp(qm_vec3_norm(left), qm_vec3_norm(right), scale);
	return qm_vec3_mag(r, len);
}

/// @brief 세 벡터로 법선 벡터를 만든다
/// @param v1 벡터 1
/// @param v2 벡터 2
/// @param v3 벡터 3
QN_INLINE QmVec3 qm_vec3_form_norm(const QmVec3 v1, const QmVec3 v2, const QmVec3 v3)
{
	QmVec3 c = qm_vec3_cross(qm_vec3_sub(v2, v1), qm_vec3_sub(v3, v1));
	return qm_vec3_mag(c, qm_vec3_len(c));
}

/// @brief 반사 벡터를 만든다
/// @param in 입력 벡터
/// @param dir 법선 벡터
QN_INLINE QmVec3 qm_vec3_reflect(const QmVec3 in, const QmVec3 dir)
{
	const float len = qm_vec3_len(in);
	QmVec3 t;
	if (qm_eqf(len, 0.0f))
		qm_vec3_rst(&t);
	else
		t = qm_vec3_mag(in, 1.0f / len);
	float dot = qm_vec3_dot(t, dir);
	/*if (dot + QM_EPSILON > 0.0f)
		return qm_vec3_rst();*/
	return qm_vec3_mag(qm_vec3(-2.0f * dot * dir.X + t.X, -2.0f * dot * dir.Y + t.Y, -2.0f * dot * dir.Z + t.Z), len);
}

/// @brief 두 벡터3 사이에 벡터3이 있는지 조사
/// @param p 조사할 벡터3
/// @param begin 선분의 시작점 벡터3
/// @param end 선분이 끝점 벡터3
/// @return 벡터 사이에 있었다면 참
QN_INLINE bool qm_vec3_between(const QmVec3 p, const QmVec3 begin, const QmVec3 end)
{
	const float f = qm_vec3_len_sq(qm_vec3_sub(end, begin));
	return qm_vec3_dist_sq(p, begin) <= f && qm_vec3_dist_sq(p, end) <= f;
}


// 벡터4

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
	{.X = x, .Y = y, .Z = z, .W = w };
#endif
	return v;
}

/// @brief 벡터4 값 설정
/// @param v3 입력 벡터3
/// @param w w 요소
/// @return 만든 벡터4
QN_INLINE QmVec4 qm_vec4v(const QmVec3 v3, const float w)
{
	QmVec4 v =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(v3.X, v3.Y, v3.Z, w) };
#elif defined QM_USE_NEON
	{.neon = { v3.X, v3.Y, v3.Z, w } };
#else
	{.X = v3.X, .Y = v3.Y, .Z = v3.Z, .W = w };
#endif
	return v;
}

/// @brief 벡터4 값 설정
///	@param v 반환 벡터
/// @param x,y,z,w 벡터4 요소
QN_INLINE void qm_vec4_set(QmVec4* v, const float x, const float y, const float z, const float w)
{
#if defined QM_USE_SSE
	v->m128 = _mm_setr_ps(x, y, z, w);
#elif defined QM_USE_NEON
	QmVec4 t = { .neon = { x, y, z, w } };
	v->neon = t.neon;
#else
	v->X = x, v->Y = y, v->Z = z, v->W = w;
#endif
}

/// @brief 벡터4 값 설정
///	@param v 반환 벡터
/// @param v3 입력 벡터3
/// @param w w 요소
/// @return 만든 벡터4
QN_INLINE void qm_vec4_setv(QmVec4* v, const QmVec3 v3, const float w)
{
#if defined QM_USE_SSE
	v->m128 = _mm_setr_ps(v3.X, v3.Y, v3.Z, w);
#elif defined QM_USE_NEON
	QmVec4 t = { .neon = { v3.X, v3.Y, v3.Z, w } };
	v->neon = t.neon;
#else
	v->X = v3.X, v->Y = v3.Y, v->Z = v3.Z, v->W = w;
#endif
}

/// @brief 벡터4 초기화
QN_INLINE void qm_vec4_rst(QmVec4* v)		// identify
{
#if defined QM_USE_SSE
	v->m128 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 0.0f);
#elif defined QM_USE_NEON
	QmVec4 t = { .neon = { 0.0f, 0.0f, 0.0f, 0.0f } };
	v->neon = t.neon;
#else
	v->X = 0.0f, v->Y = 0.0f, v->Z = 0.0f, v->W = 0.0f;
#endif
}

/// @brief 벡터4 대각값 설정 (모든 요소를 같은 값을)
///	@param v 반환 벡터
/// @param diag 대각값
QN_INLINE void qm_vec4_diag(QmVec4* v, const float diag)
{
#if defined QM_USE_SSE
	v->m128 = _mm_setr_ps(diag, diag, diag, diag);
#elif defined QM_USE_NEON
	QmVec4 t = { .neon = { diag, diag, diag, diag } };
	v->neon = t.neon;
#else
	v->X = diag, v->Y = diag, v->Z = diag, v->W = diag;
#endif
}

/// @brief 벡터4 반전
/// @param v 원본 벡터4
QN_INLINE QmVec4 qm_vec4_ivt(const QmVec4 v)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_xor_ps(v.m128, _mm_set1_ps(-0.0f)) };
#elif defined QM_USE_NEON
	{.neon = { -v.X, -v.Y, -v.Z, -v.W } };
#else
	{.X = -v.X, .Y = -v.Y, .Z = -v.Z, .W = -v.W };
#endif
	return r;
}

/// @brief 벡터4 네거티브
/// @param v 원본 벡터4
QN_INLINE QmVec4 qm_vec4_neg(const QmVec4 v)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(1.0f - v.X, 1.0f - v.Y, 1.0f - v.Z, 1.0f - v.W) };
#elif defined QM_USE_NEON
	{.neon = { 1.0f - v.X, 1.0f - v.Y, 1.0f - v.Z, 1.0f - v.W } };
#else
	{.X = 1.0f - v.X, .Y = 1.0f - v.Y, .Z = 1.0f - v.Z, .W = 1.0f - v.W };
#endif
	return r;
}

/// @brief 벡터4 덧셈
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVec4 qm_vec4_add(const QmVec4 left, const QmVec4 right)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_add_ps(left.m128, right.m128) };
#elif defined QM_USE_NEON
	{.neon = vaddq_f32(left.neon, right.neon) };
#else
	{.X = left.X + right.X, .Y = left.Y + right.Y, .Z = left.Z + right.Z, .W = left.W + right.W, };
#endif
	return r;
}

/// @brief 벡터4 뺄셈
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVec4 qm_vec4_sub(const QmVec4 left, const QmVec4 right)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_sub_ps(left.m128, right.m128) };
#elif defined QM_USE_NEON
	{.neon = vsubq_f32(left.neon, right.neon) };
#else
	{.X = left.X - right.X, .Y = left.Y - right.Y, .Z = left.Z - right.Z, .W = left.W - right.W, };
#endif
	return r;
}

/// @brief 벡터4 확대
/// @param left 원본 벡터4
/// @param right 확대값
QN_INLINE QmVec4 qm_vec4_mag(const QmVec4 left, const float right)
{
#if defined QM_USE_SSE
	__m128 m = _mm_set1_ps(right);
	QmVec4 r = { .m128 = _mm_mul_ps(left.m128, m) };
#elif defined QM_USE_NEON
	QmVec4 r = { .neon = vmulq_n_f32(left.neon, right) };
#else
	QmVec4 r = { .X = left.X * right, .Y = left.Y * right, .Z = left.Z * right, .W = left.W * right };
#endif
	return r;
}

/// @brief 벡터4 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec4 qm_vec4_mul(const QmVec4 left, const QmVec4 right)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_mul_ps(left.m128, right.m128) };
#elif defined QM_USE_NEON
	{.neon = vmulq_f32(left.neon, right.neon)};
#else
	{.X = left.X * right.X, .Y = left.Y * right.Y, .Z = left.Z * right.Z, .W = left.W * right.W };
#endif
	return r;
}

/// @brief 벡터4 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVec4 qm_vec4_div(const QmVec4 left, const QmVec4 right)
{
	QmVec4 r =
#if defined QM_USE_SSE
	{ .m128 = _mm_div_ps(left.m128, right.m128) };
#elif defined QM_USE_NEON
	{.neon = vdivq_f32(left.neon, right.neon)};
#else
	{.X = left.X / right.X, .Y = left.Y / right.Y, .Z = left.Z / right.Z, .W = left.W / right.W };
#endif
	return r;
}

/// @brief 벡터4의 최소값
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVec4 qm_vec4_min(const QmVec4 left, const QmVec4 right)
{
	QmVec4 r =
	{
		.X = (left.X < right.X) ? left.X : right.X,
		.Y = (left.Y < right.Y) ? left.Y : right.Y,
		.Z = (left.Z < right.Z) ? left.Z : right.Z,
		.W = (left.W < right.W) ? left.W : right.W,
	};
	return r;
}

/// @brief 벡터4의 최대값
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVec4 qm_vec4_max(const QmVec4 left, const QmVec4 right)
{
	QmVec4 r =
	{
		.X = (left.X > right.X) ? left.X : right.X,
		.Y = (left.Y > right.Y) ? left.Y : right.Y,
		.Z = (left.Z > right.Z) ? left.Z : right.Z,
		.W = (left.W > right.W) ? left.W : right.W,
	};
	return r;
}

/// @brief 두 벡터4를 비교
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터4가 같으면 참
QN_INLINE bool qm_vec4_eq(const QmVec4 left, const QmVec4 right)
{
	return
		qm_eqf(left.X, right.X) && qm_eqf(left.Y, right.Y) &&
		qm_eqf(left.Z, right.Z) && qm_eqf(left.W, right.W);
}

/// @brief 벡터가 0인지 비교
/// @param pv 비교할 벡터4
/// @return 벡터4가 0이면 참
QN_INLINE bool qm_vec4_isi(const QmVec4 pv)
{
	return pv.X == 0.0f && pv.Y == 0.0f && pv.Z == 0.0f && pv.W == 0.0f;
}

/// @brief 벡터4 내적
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 내적값
QN_INLINE float qm_vec4_dot(const QmVec4 left, const QmVec4 right)
{
#if defined QM_USE_SSE
	__m128 p = _mm_mul_ps(left.m128, right.m128);
	__m128 u = _mm_shuffle_ps(p, p, _MM_SHUFFLE(2, 3, 0, 1));
	p = _mm_add_ps(p, u);
	u = _mm_shuffle_ps(p, p, _MM_SHUFFLE(0, 1, 2, 3));
	p = _mm_add_ps(p, u);
	float r;
	_mm_store_ss(&r, p);
	return r;
#elif defined QM_USE_NEON
	float32x4_t mul = vmulq_f32(left.neon, right.neon);
	float32x4_t half = vpaddq_f32(mul, mul);
	float32x4_t full = vpaddq_f32(half, half);
	return vgetq_lane_f32(full, 0);
#else
	return left.X * right.X + left.Y * right.Y + left.Z * right.Z + left.W * right.W;
#endif
}

#if defined QM_USE_SSE
QSAPI void qm_sse_vec4_cross(QmVec4* pv, const QmVec4* v1, const QmVec4* v2, const QmVec4* v3);
#endif
#if defined QM_USE_NEON
QSAPI void qm_neon_vec4_cross(QmVec4* pv, const QmVec4* v1, const QmVec4* v2, const QmVec4* v3);
#endif

/// @brief 벡터4 외적
/// @param v1 첫번째 벡터4
/// @param v2 두번째 벡터4
/// @param v3 세번째 벡터4
QN_INLINE QmVec4 qm_vec4_cross(const QmVec4 v1, const QmVec4 v2, const QmVec4 v3)
{
#if defined QM_USE_SSE
	QmVec4 r;
	qm_sse_vec4_cross(&r, &v1, &v2, &v3);
#elif defined QM_USE_NEON
	QmVec4 r;
	qm_neon_vec4_cross(&r, &v1, &v2, &v3);
#else
	QmVec4 r =
	{
		.X = v1.Y * (v2.Z * v3.W - v3.Z * v2.W) - v1.Z * (v2.Y * v3.W - v3.Y * v2.W) + v1.W * (v2.Y * v3.Z - v2.Z * v3.Y),
		.Y = -(v1.X * (v2.Z * v3.W - v3.Z * v2.W) - v1.Z * (v2.X * v3.W - v3.X * v2.W) + v1.W * (v2.X * v3.Z - v3.X * v2.Z)),
		.Z = v1.X * (v2.Y * v3.W - v3.Y * v2.W) - v1.Y * (v2.X * v3.W - v3.X * v2.W) + v1.W * (v2.X * v3.Y - v3.X * v2.Y),
		.W = -(v1.X * (v2.Y * v3.Z - v3.Y * v2.Z) - v1.Y * (v2.X * v3.Z - v3.X * v2.Z) + v1.Z * (v2.X * v3.Y - v3.X * v2.Y)),
	};
#endif
	return r;
}

/// @brief 벡터4 거리의 제곱
/// @param pv 대상 벡터4
/// @return 벡터4 거리의 제곱값
QN_INLINE float qm_vec4_len_sq(const QmVec4 pv)
{
	return qm_vec4_dot(pv, pv);
}

/// @brief 벡터4 거리
/// @param pv 대상 벡터4
/// @return 벡터4 거리값
QN_INLINE float qm_vec4_len(const QmVec4 pv)
{
	return qm_sqrtf(qm_vec4_len_sq(pv));
}

/// @brief 벡터4 정규화
/// @param v 벡터4
QN_INLINE QmVec4 qm_vec4_norm(const QmVec4 v)
{
	return qm_vec4_mag(v, qm_inv_sqrtf(qm_vec4_dot(v, v)));
}

/// @brief 두 벡터4 거리의 제곱
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터4 거리의 제곱값
QN_INLINE float qm_vec4_dist_sq(const QmVec4 left, const QmVec4 right)
{
	QmVec4 t = qm_vec4_sub(left, right);
	return qm_vec4_len_sq(t);
}

/// @brief 두 벡터4의 거리
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터3의 거리값
QN_INLINE float qm_vec4_dist(const QmVec4 left, const QmVec4 right)
{
	return qm_sqrtf(qm_vec4_dist_sq(left, right));
}

/// @brief 벡터4 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec4 qm_vec4_interpolate(const QmVec4 left, const QmVec4 right, const float scale)
{
	return qm_vec4_add(qm_vec4_mag(left, 1.0f - scale), qm_vec4_mag(right, scale));
}

/// @brief 벡터4 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
QN_INLINE QmVec4 qm_vec4_lerp(const QmVec4 left, const QmVec4 right, const float scale)
{
	return qm_vec4_add(left, qm_vec4_mag(qm_vec4_sub(right, left), scale));
}

/// @brief 벡터4 트랜스폼
/// @param v 원본 벡터4
/// @param trfm 변환 행렬
QN_INLINE QmVec4 qm_vec4_trfm(const QmVec4 v, const QmMat4 trfm)
{
#if defined QM_USE_SSE
	QmVec4 r;
	r.m128 = _mm_mul_ps(_mm_shuffle_ps(v.m128, v.m128, 0x00), trfm.rows[0].m128);
	r.m128 = _mm_add_ps(r.m128, _mm_mul_ps(_mm_shuffle_ps(v.m128, v.m128, 0x55), trfm.rows[1].m128));
	r.m128 = _mm_add_ps(r.m128, _mm_mul_ps(_mm_shuffle_ps(v.m128, v.m128, 0xAA), trfm.rows[2].m128));
	r.m128 = _mm_add_ps(r.m128, _mm_mul_ps(_mm_shuffle_ps(v.m128, v.m128, 0xFF), trfm.rows[3].m128));
#elif defined QM_USE_NEON
	QmVec4 r;
	r.neon = vmulq_laneq_f32(trfm.rows[0].neon, v.neon, 0);
	r.neon = vfmaq_laneq_f32(r.neon, trfm.rows[1].neon, v.neon, 1);
	r.neon = vfmaq_laneq_f32(r.neon, trfm.rows[2].neon, v.neon, 2);
	r.neon = vfmaq_laneq_f32(r.neon, trfm.rows[3].neon, v.neon, 3);
#else
	QmVec4 r =
	{
		.X = v.X * trfm._11 + v.Y * trfm._21 + v.Z * trfm._31 + v.W * trfm._41,
		.Y = v.X * trfm._12 + v.Y * trfm._22 + v.Z * trfm._32 + v.W * trfm._42,
		.Z = v.X * trfm._13 + v.Y * trfm._23 + v.Z * trfm._33 + v.W * trfm._43,
		.W = v.X * trfm._14 + v.Y * trfm._24 + v.Z * trfm._34 + v.W * trfm._44,
	};
#endif
	return r;
}


// 정수 벡터2

/// @brief 벡터2 값 설정
/// @param x,y 좌표
QN_INLINE QmVecI2 qm_veci2(int x, int y)
{
	QmVecI2 r = { .X = x, .Y = y };
	return r;
}

/// @brief 벡터2 설정
QN_INLINE void qm_veci2_set(QmVecI2* v, int x, int y)
{
	v->X = x;
	v->Y = y;
}

/// @brief 벡터2 초기화
QN_INLINE void qm_veci2_rst(QmVecI2* v)		// identify
{
	v->X = 0;
	v->Y = 0;
}

/// @brief 벡터2 대각값 설정 (모두 같은값으로 설정)
/// @param v 반환 벡터
/// @param diag 대각 값
QN_INLINE void qm_veci2_diag(QmVecI2* v, const int diag)
{
	v->X = diag;
	v->Y = diag;
}

/// @brief 벡터2 반전
/// @param v 벡터2
QN_INLINE QmVecI2 qm_veci2_ivt(const QmVecI2 v)  // invert
{
	QmVecI2 r = { .X = -v.X, .Y = -v.Y };
	return r;
}

/// @brief 벡터2 덧셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVecI2 qm_veci2_add(const QmVecI2 left, const QmVecI2 right)
{
	QmVecI2 r = { .X = left.X + right.X, .Y = left.Y + right.Y };
	return r;
}

/// @brief 벡터2 뺄셈
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVecI2 qm_veci2_sub(const QmVecI2 left, const QmVecI2 right)
{
	QmVecI2 r = { .X = left.X - right.X, .Y = left.Y - right.Y };
	return r;
}

/// @brief 벡터2 확대
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 확대값
QN_INLINE QmVecI2 qm_veci2_mag(const QmVecI2 left, const int right)
{
	QmVecI2 r = { .X = left.X * right, .Y = left.Y * right };
	return r;
}

/// @brief 벡터2 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVecI2 qm_veci2_mul(const QmVecI2 left, const QmVecI2 right)
{
	QmVecI2 r = { .X = left.X * right.X, .Y = left.Y * right.Y };
	return r;
}

/// @brief 벡터2 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVecI2 qm_veci2_div(const QmVecI2 left, const QmVecI2 right)
{
	QmVecI2 r = { .X = left.X / right.X, .Y = left.Y / right.Y };
	return r;
}

/// @brief 벡터2의 최소값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVecI2 qm_veci2_min(const QmVecI2 left, const QmVecI2 right)
{
	QmVecI2 r =
	{
		.X = (left.X < right.X) ? left.X : right.X,
		.Y = (left.Y < right.Y) ? left.Y : right.Y
	};
	return r;
}

/// @brief 벡터2의 최대값
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVecI2 qm_veci2_max(const QmVecI2 left, const QmVecI2 right)
{
	QmVecI2 r =
	{
		.X = (left.X > right.X) ? left.X : right.X,
		.Y = (left.Y > right.Y) ? left.Y : right.Y
	};
	return r;
}

/// @brief 벡터2의 비교
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡트
/// @return 같으면 참
QN_INLINE bool qm_veci2_eq(const QmVecI2 left, const QmVecI2 right)
{
	return left.X == right.X && left.Y == right.Y;
}

/// @brief 벡터2가 0인가 비교
/// @param pv 비교할 벡터2
/// @return 0이면 참
QN_INLINE bool qm_veci2_isi(const QmVecI2 pv)
{
	return pv.X == 0 && pv.Y == 0;
}

/// @brief 벡터2 내적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 내적 값
QN_INLINE int qm_veci2_dot(const QmVecI2 left, const QmVecI2 right)
{
	return left.X * right.X + left.Y * right.Y;
}

/// @brief 벡터2의 외적
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
QN_INLINE QmVecI2 qm_veci2_cross(const QmVecI2 left, const QmVecI2 right)
{
	QmVecI2 r =
	{
		.X = left.Y * right.X - left.X * right.Y,
		.Y = left.X * right.Y - left.Y * right.X
	};
	return r;
}

/// @brief 벡터2 길이의 제곱
/// @param pv 벡터2
/// @return 길이의 제곱
QN_INLINE int qm_veci2_len_sq(const QmVecI2 pv)
{
	return qm_veci2_dot(pv, pv);
}

/// @brief 벡터2 길이
/// @param pv 벡터2
/// @return 길이
QN_INLINE float qm_veci2_len(const QmVecI2 pv)
{
	return qm_sqrtf((float)qm_veci2_len_sq(pv));
}

/// @brief 두 벡터2 거리의 제곱
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2 거리의 제곱값
QN_INLINE int qm_veci2_dist_sq(const QmVecI2 left, const QmVecI2 right)
{
	const QmVecI2 t = qm_veci2_sub(left, right);
	return qm_veci2_len_sq(t);
}

/// @brief 두 벡터2의 거리
/// @param left 왼쪽 벡터2
/// @param right 오른쪽 벡터2
/// @return 두 벡터2의 거리값
QN_INLINE float qm_veci2_dist(const QmVecI2 left, const QmVecI2 right)
{
	return qm_sqrtf((float)qm_veci2_dist_sq(left, right));
}


// 정수 벡터3

/// @brief 벡터3 값 설정
/// @param x,y,z 좌표
QN_INLINE QmVecI3 qm_veci3(const int x, const int y, const int z)
{
	QmVecI3 r = { .X = x, .Y = y, .Z = z };
	return r;
}

/// @brief 벡터3 값 설정
///	@param v 반환 벡터
/// @param x,y,z 좌표
QN_INLINE void qm_veci3_set(QmVecI3* v, const int x, const int y, const int z)
{
	v->X = x;
	v->Y = y;
	v->Z = z;
}

/// @brief 벡터3 초기화
///	@param v 반환 벡터
QN_INLINE void qm_veci3_rst(QmVecI3* v)		// identify
{
	v->X = 0;
	v->Y = 0;
	v->Z = 0;
}

/// @brief 벡터3 대각값 설정 (모두 같은값으로 설정)
///	@param v 반환 벡터
/// @param diag 대각값
QN_INLINE void qm_veci3_diag(QmVecI3* v, const int diag)
{
	v->X = diag;
	v->Y = diag;
	v->Z = diag;
}

/// @brief 벡터3 반전
/// @param v 원본 벡터3
QN_INLINE QmVecI3 qm_veci3_ivt(const QmVecI3 v)  // invert
{
	QmVecI3 r = { .X = -v.X, .Y = -v.Y, .Z = -v.Z };
	return r;
}

/// @brief 벡터3 덧셈
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVecI3 qm_veci3_add(const QmVecI3 left, const QmVecI3 right)
{
	QmVecI3 r = { .X = left.X + right.X, .Y = left.Y + right.Y, .Z = left.Z + right.Z };
	return r;
}

/// @brief 벡터3 뺄셈
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVecI3 qm_veci3_sub(const QmVecI3 left, const QmVecI3 right)
{
	QmVecI3 r = { .X = left.X - right.X, .Y = left.Y - right.Y, .Z = left.Z - right.Z };
	return r;
}

/// @brief 벡터3 확대
/// @param left 원본 벡터3
/// @param right 확대값
QN_INLINE QmVecI3 qm_veci3_mag(const QmVecI3 left, const int right)
{
	QmVecI3 r = { .X = left.X * right, .Y = left.Y * right, .Z = left.Z * right };
	return r;
}

/// @brief 벡터3 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVecI3 qm_veci3_mul(const QmVecI3 left, const QmVecI3 right)
{
	QmVecI3 r = { .X = left.X * right.X, .Y = left.Y * right.Y, .Z = left.Z * right.Z };
	return r;
}

/// @brief 벡터3 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVecI3 qm_veci3_div(const QmVecI3 left, const QmVecI3 right)
{
	QmVecI3 r = { .X = left.X / right.X, .Y = left.Y / right.Y, .Z = left.Z / right.Z };
	return r;
}

/// @brief 벡터3의 최소값
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVecI3 qm_veci3_min(const QmVecI3 left, const QmVecI3 right)
{
	QmVecI3 r =
	{
		.X = left.X < right.X ? left.X : right.X,
		.Y = left.Y < right.Y ? left.Y : right.Y,
		.Z = left.Z < right.Z ? left.Z : right.Z,
	};
	return r;
}

/// @brief 벡터3의 최대값
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVecI3 qm_veci3_max(const QmVecI3 left, const QmVecI3 right)
{
	QmVecI3 r =
	{
		.X = left.X > right.X ? left.X : right.X,
		.Y = left.Y > right.Y ? left.Y : right.Y,
		.Z = left.Z > right.Z ? left.Z : right.Z,
	};
	return r;
}

/// @brief 두 벡터3이 같은지 판단
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3이 같으면 참
QN_INLINE bool qm_veci3_eq(const QmVecI3 left, const QmVecI3 right)
{
	return left.X == right.X && left.Y == right.Y && left.Z == right.Z;
}

/// @brief 벡터3이 0인지 판단
/// @param pv 벡터3
/// @return 벡터3이 0이면 참
QN_INLINE bool qm_veci3_isi(const QmVecI3 pv)
{
	return pv.X == 0 && pv.Y == 0 && pv.Z == 0;
}

/// @brief 벡터3의 내적
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 내적값
QN_INLINE int qm_veci3_dot(const QmVecI3 left, const QmVecI3 right)
{
	return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
}

/// @brief 벡터3의 외적
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
QN_INLINE QmVecI3 qm_veci3_cross(const QmVecI3 left, const QmVecI3 right)
{
	QmVecI3 r =
	{
		.X = left.Y * right.Z - left.Z * right.Y,
		.Y = left.Z * right.X - left.X * right.Z,
		.Z = left.X * right.Y - left.Y * right.X,
	};
	return r;
}

/// @brief 벡터3의 길이의 제곱
/// @param pv 벡터3
/// @return 길이의 제곱
QN_INLINE int qm_veci3_len_sq(const QmVecI3 pv)
{
	return qm_veci3_dot(pv, pv);
}

/// @brief 벡터3의 길이
/// @param pv 벡터3
/// @return 길이
QN_INLINE float qm_veci3_len(const QmVecI3 pv)
{
	return qm_sqrtf((float)qm_veci3_len_sq(pv));
}

/// @brief 두 벡터3 거리의 제곱
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3 거리의 제곱값
QN_INLINE int qm_veci3_dist_sq(const QmVecI3 left, const QmVecI3 right)
{
	const QmVecI3 t = qm_veci3_sub(left, right);
	return qm_veci3_len_sq(t);
}

/// @brief 두 벡터3의 거리
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3의 거리값
QN_INLINE float qm_veci3_dist(const QmVecI3 left, const QmVecI3 right)
{
	return qm_sqrtf((float)qm_veci3_dist_sq(left, right));
}


// 정수 벡터4

/// @brief 벡터4 값 설정
/// @param x,y,z,w 벡터4 요소
/// @return 만든 벡터4
QN_INLINE QmVecI4 qm_veci4(const int x, const int y, const int z, const int w)
{
	QmVecI4 v = { .X = x, .Y = y, .Z = z, .W = w };
	return v;
}

/// @brief 벡터4 값 설정
/// @param v3 입력 벡터3
/// @param w w 요소
/// @return 만든 벡터4
QN_INLINE QmVecI4 qm_veci4v(const QmVecI3 v3, const int w)
{
	QmVecI4 v = { .X = v3.X, .Y = v3.Y, .Z = v3.Z, .W = w };
	return v;
}

/// @brief 벡터4 값 설정
///	@param v 반환 벡터
/// @param x,y,z,w 벡터4 요소
QN_INLINE void qm_veci4_set(QmVecI4* v, const int x, const int y, const int z, const int w)
{
	v->X = x;
	v->Y = y;
	v->Z = z;
	v->W = w;
}

/// @brief 벡터4 값 설정
///	@param v 반환 벡터
/// @param v3 입력 벡터3
/// @param w w 요소
QN_INLINE void qm_veci4_setv(QmVecI4* v, const QmVecI3 v3, const int w)
{
	v->X = v3.X;
	v->Y = v3.Y;
	v->Z = v3.Z;
	v->W = w;
}

/// @brief 벡터4 초기화
QN_INLINE void qm_veci4_rst(QmVecI4* v)		// identify
{
	v->X = 0;
	v->Y = 0;
	v->Z = 0;
	v->W = 0;
}

/// @brief 벡터4 대각값 설정 (모든 요소를 같은 값을)
///	@param v 반환 벡터
/// @param diag 대각값
QN_INLINE void qm_veci4_diag(QmVecI4* v, const int diag)
{
	v->X = diag;
	v->Y = diag;
	v->Z = diag;
	v->W = diag;
}

/// @brief 벡터4 반전
/// @param v 원본 벡터4
QN_INLINE QmVecI4 qm_veci4_ivt(const QmVecI4 v)
{
	QmVecI4 r = { .X = -v.X, .Y = -v.Y, .Z = -v.Z, .W = -v.W };
	return r;
}

/// @brief 벡터4 덧셈
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVecI4 qm_veci4_add(const QmVecI4 left, const QmVecI4 right)
{
	QmVecI4 r =
#if defined QM_USE_NEON
	{ .neon = vaddq_s32(left.neon, right.neon) };
#else
	{.X = left.X + right.X, .Y = left.Y + right.Y, .Z = left.Z + right.Z, .W = left.W + right.W, };
#endif
	return r;
}

/// @brief 벡터4 뺄셈
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVecI4 qm_veci4_sub(const QmVecI4 left, const QmVecI4 right)
{
	QmVecI4 r =
#if defined QM_USE_NEON
	{ .neon = vsubq_s32(left.neon, right.neon) };
#else
	{.X = left.X - right.X, .Y = left.Y - right.Y, .Z = left.Z - right.Z, .W = left.W - right.W, };
#endif
	return r;
}

/// @brief 벡터4 확대
/// @param left 원본 벡터4
/// @param right 확대값
QN_INLINE QmVecI4 qm_veci4_mag(const QmVecI4 left, const int right)
{
#if defined QM_USE_NEON
	QmVecI4 r = { .neon = vmulq_n_s32(left.neon, right) };
#else
	QmVecI4 r = { .X = left.X * right, .Y = left.Y * right, .Z = left.Z * right, .W = left.W * right };
#endif
	return r;
}

/// @brief 벡터4 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVecI4 qm_veci4_mul(const QmVecI4 left, const QmVecI4 right)
{
	QmVecI4 r =
#if defined QM_USE_NEON
	{ .neon = vmulq_s32(left.neon, right.neon) };
#else
	{.X = left.X * right.X, .Y = left.Y * right.Y, .Z = left.Z * right.Z, .W = left.W * right.W };
#endif
	return r;
}

/// @brief 벡터4 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
QN_INLINE QmVecI4 qm_veci4_div(const QmVecI4 left, const QmVecI4 right)
{
	QmVecI4 r =
#if defined QM_USE_NEON
	{ .neon = vdivq_s32(left.neon, right.neon) };
#else
	{.X = left.X / right.X, .Y = left.Y / right.Y, .Z = left.Z / right.Z, .W = left.W / right.W };
#endif
	return r;
}

/// @brief 벡터4의 최소값
/// @param pv 최소값을 담을 벡터4
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVecI4 qm_veci4_min(const QmVecI4 left, const QmVecI4 right)
{
	QmVecI4 r =
	{
		.X = (left.X < right.X) ? left.X : right.X,
		.Y = (left.Y < right.Y) ? left.Y : right.Y,
		.Z = (left.Z < right.Z) ? left.Z : right.Z,
		.W = (left.W < right.W) ? left.W : right.W,
	};
	return r;
}

/// @brief 벡터4의 최대값
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
QN_INLINE QmVecI4 qm_veci4_max(const QmVecI4 left, const QmVecI4 right)
{
	QmVecI4 r =
	{
		.X = (left.X > right.X) ? left.X : right.X,
		.Y = (left.Y > right.Y) ? left.Y : right.Y,
		.Z = (left.Z > right.Z) ? left.Z : right.Z,
		.W = (left.W > right.W) ? left.W : right.W,
	};
	return r;
}

/// @brief 두 벡터4를 비교
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터4가 같으면 참
QN_INLINE bool qm_veci4_eq(const QmVecI4 left, const QmVecI4 right)
{
	return left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W;
}

/// @brief 벡터가 0인지 비교
/// @param pv 비교할 벡터4
/// @return 벡터4가 0이면 참
QN_INLINE bool qm_veci4_isi(const QmVecI4 pv)
{
	return pv.X == 0 && pv.Y == 0 && pv.Z == 0 && pv.W == 0;
}

/// @brief 벡터4 내적
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 내적값
QN_INLINE int qm_veci4_dot(const QmVecI4 left, const QmVecI4 right)
{
	return left.X * right.X + left.Y * right.Y + left.Z * right.Z + left.W * right.W;
}

/// @brief 벡터4 외적
/// @param pv 외적 결과를 담을 벡터4
/// @param v1 첫번째 벡터4
/// @param v2 두번째 벡터4
/// @param v3 세번째 벡터4
QN_INLINE QmVecI4 qm_veci4_cross(const QmVecI4 v1, const QmVecI4 v2, const QmVecI4 v3)
{
	QmVecI4 r =
	{
		.X = v1.Y * (v2.Z * v3.W - v3.Z * v2.W) - v1.Z * (v2.Y * v3.W - v3.Y * v2.W) + v1.W * (v2.Y * v3.Z - v2.Z * v3.Y),
		.Y = -(v1.X * (v2.Z * v3.W - v3.Z * v2.W) - v1.Z * (v2.X * v3.W - v3.X * v2.W) + v1.W * (v2.X * v3.Z - v3.X * v2.Z)),
		.Z = v1.X * (v2.Y * v3.W - v3.Y * v2.W) - v1.Y * (v2.X * v3.W - v3.X * v2.W) + v1.W * (v2.X * v3.Y - v3.X * v2.Y),
		.W = -(v1.X * (v2.Y * v3.Z - v3.Y * v2.Z) - v1.Y * (v2.X * v3.Z - v3.X * v2.Z) + v1.Z * (v2.X * v3.Y - v3.X * v2.Y)),
	};
	return r;
}


/// @brief 벡터4 거리의 제곱
/// @param pv 대상 벡터4
/// @return 벡터4 거리의 제곱값
QN_INLINE int qm_veci4_len_sq(const QmVecI4 pv)
{
	return qm_veci4_dot(pv, pv);
}

/// @brief 벡터4 거리
/// @param pv 대상 벡터4
/// @return 벡터4 거리값
QN_INLINE float qm_veci4_len(const QmVecI4 pv)
{
	return qm_sqrtf((float)qm_veci4_len_sq(pv));
}


/// @brief 두 벡터4 거리의 제곱
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터4 거리의 제곱값
QN_INLINE int qm_veci4_dist_sq(const QmVecI4 left, const QmVecI4 right)
{
	return qm_veci4_len_sq(qm_veci4_sub(left, right));
}

/// @brief 두 벡터4의 거리
/// @param left 왼쪽 벡터4
/// @param right 오른쪽 벡터4
/// @return 두 벡터3의 거리값
QN_INLINE float qm_veci4_dist(const QmVecI4 left, const QmVecI4 right)
{
	return qm_sqrtf((float)qm_veci4_dist_sq(left, right));
}


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
	{.X = x, .Y = y, .Z = z, .W = w };
#endif
	return r;
}

/// @brief 사원수 값 설정
/// @param v 벡타4
/// @return 만든 사원수
QN_INLINE QmQuat qm_quatv4(const QmVec4 v)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = v.m128 };
#elif defined QM_USE_NEON
	{.neon = v.neon };
#else
	{.X = v.X, .Y = v.Y, .Z = v.Z, .W = v.W };
#endif
	return r;
}

/// @brief 사원수 값 설정
/// @param p 입력 벡터3
/// @param w w 요소
/// @return 만든 사원수
QN_INLINE QmQuat qm_quatv3(const QmVec3 p, const float w)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(p.X, p.Y, p.Z, w) };
#elif defined QM_USE_NEON
	{.neon = { p.X, p.Y, p.Z, w } };
#else
	{.X = p.X, .Y = p.Y, .Z = p.Z, .W = w };
#endif
	return r;
}

/// @brief 사원수 값 설정
///	@param q 반환 사원수
/// @param x,y,z,w 사원수 요소
QN_INLINE void qm_quat_set(QmQuat* q, const float x, const float y, const float z, const float w)
{
#if defined QM_USE_SSE
	q->m128 = _mm_setr_ps(x, y, z, w);
#elif defined QM_USE_NEON
	QmVec4 t = { .neon = { x, y, z, w } };
	q->neon = t.neon;
#else
	q->X = x, q->Y = y, q->Z = z, q->W = w;
#endif
}

/// @brief 사원수 값 설정
///	@param q 반환 사원수
/// @param v 벡타4
QN_INLINE void qm_quat_setv4(QmQuat* q, const QmVec4 v)
{
#if defined QM_USE_SSE
	q->m128 = v.m128;
#elif defined QM_USE_NEON
	q->neon = v.neon;
#else
	q->X = v.X, q->Y = v.Y, q->Z = v.Z, q->W = v.W;
#endif
}

/// @brief 사원수 값 설정
///	@param q 반환 사원수
/// @param p 입력 벡터3
/// @param w w 요소
QN_INLINE void qm_quat_setv3(QmQuat* q, const QmVec3 p, const float w)
{
#if defined QM_USE_SSE
	q->m128 = _mm_setr_ps(p.X, p.Y, p.Z, w);
#elif defined QM_USE_NEON
	q->neon = { p.X, p.Y, p.Z, w };
#else
	q->X = p.X, q->Y = p.Y, q->Z = p.Z, q->W = w;
#endif
}

/// @brief 사원수 초기화
///	@param q 반환 사원수
QN_INLINE void qm_quat_rst(QmQuat* q)		// identify
{
#if defined QM_USE_SSE
	q->m128 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
#elif defined QM_USE_NEON
	q->neon = { 0.0f, 0.0f, 0.0f, 1.0f };
#else
	q->X = q->Y = q->Z = 0.0f, q->W = 1.0f;
#endif
}

/// @brief 사원수 대각값 설정 (모든 요소를 같은 값을)
///	@param q 반환 사원수
/// @param diag 대각값
QN_INLINE void qm_quat_diag(QmQuat* q, const float diag)
{
#if defined QM_USE_SSE
	q->m128 = _mm_setr_ps(diag, diag, diag, diag);
#elif defined QM_USE_NEON
	q->neon = { diag, diag, diag, diag };
#else
	q->X = q->Y = q->Z = q->W = diag;
#endif
}

/// @brief 사원수 반전
/// @param v 원본 사원수
QN_INLINE QmQuat qm_quat_ivt(const QmQuat v)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(-v.X, -v.Y, -v.Z, -v.W) };
#elif defined QM_USE_NEON
	{.neon = { -v.X, -v.Y, -v.Z, -v.W } };
#else
	{.X = -v.X, .Y = -v.Y, .Z = -v.Z, .W = -v.W };
#endif
	return r;
}

/// @brief 사원수 네거티브
/// @param v 원본 사원수
QN_INLINE QmQuat qm_quat_neg(const QmQuat v)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(1.0f - v.X, 1.0f - v.Y, 1.0f - v.Z, 1.0f - v.W) };
#elif defined QM_USE_NEON
	{.neon = { 1.0f - v.X, 1.0f - v.Y, 1.0f - v.Z, 1.0f - v.W } };
#else
	{.X = 1.0f - v.X, .Y = 1.0f - v.Y, .Z = 1.0f - v.Z, .W = 1.0f - v.W };
#endif
	return r;
}

/// @brief 사원수 덧셈
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE QmQuat qm_quat_add(const QmQuat left, const QmQuat right)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_add_ps(left.m128, right.m128) };
#elif defined QM_USE_NEON
	{.neon = vaddq_f32(left.neon, right.neon) };
#else
	{.X = left.X + right.X, .Y = left.Y + right.Y, .Z = left.Z + right.Z, .W = left.W + right.W, };
#endif
	return r;
}

/// @brief 사원수 뺄셈
/// @param pv 뺄셈 결과를 담을 사원수
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE QmQuat qm_quat_sub(const QmQuat left, const QmQuat right)
{
	QmQuat r =
#if defined QM_USE_SSE
	{ .m128 = _mm_sub_ps(left.m128, right.m128) };
#elif defined QM_USE_NEON
	{.neon = vsubq_f32(left.neon, right.neon) };
#else
	{.X = left.X - right.X, .Y = left.Y - right.Y, .Z = left.Z - right.Z, .W = left.W - right.W, };
#endif
	return r;
}

/// @brief 사원수 확대
/// @param left 원본 사원수
/// @param right 확대값
QN_INLINE QmQuat qm_quat_mag(const QmQuat left, const float right)
{
#if defined QM_USE_SSE
	__m128 m = _mm_set1_ps(right);
	QmQuat r = { .m128 = _mm_mul_ps(left.m128, m) };
#elif defined QM_USE_NEON
	QmQuat r = { .neon = vmulq_n_f32(left.neon, right) };
#else
	QmQuat r = { .X = left.X * right, .Y = left.Y * right, .Z = left.Z * right, .W = left.W * right };
#endif
	return r;
}

/// @brief 사원수 항목 곱셈
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE QmQuat qm_quat_mul(const QmQuat left, const QmQuat right)
{
#if defined QM_USE_SSE
	QmQuat r;
	__m128 a = _mm_xor_ps(_mm_shuffle_ps(left.m128, left.m128, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.f, -0.f, 0.f, -0.f));
	__m128 b = _mm_shuffle_ps(right.m128, right.m128, _MM_SHUFFLE(0, 1, 2, 3));
	__m128 c = _mm_mul_ps(b, a);
	a = _mm_xor_ps(_mm_shuffle_ps(left.m128, left.m128, _MM_SHUFFLE(1, 1, 1, 1)), _mm_setr_ps(0.f, 0.f, -0.f, -0.f));
	b = _mm_shuffle_ps(right.m128, right.m128, _MM_SHUFFLE(1, 0, 3, 2));
	c = _mm_add_ps(c, _mm_mul_ps(b, a));
	a = _mm_xor_ps(_mm_shuffle_ps(left.m128, left.m128, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.f, 0.f, 0.f, -0.f));
	b = _mm_shuffle_ps(right.m128, right.m128, _MM_SHUFFLE(2, 3, 0, 1));
	c = _mm_add_ps(c, _mm_mul_ps(b, a));
	a = _mm_shuffle_ps(left.m128, left.m128, _MM_SHUFFLE(3, 3, 3, 3));
	b = _mm_shuffle_ps(right.m128, right.m128, _MM_SHUFFLE(3, 2, 1, 0));
	r.m128 = _mm_add_ps(c, _mm_mul_ps(b, a));
#elif defined QM_USE_NEON
	QmQuat r;
	float32x4_t r1032 = vrev64q_f32(right.neon);
	float32x4_t r3210 = vcombine_f32(vget_high_f32(r1032), vget_low_f32(r1032));
	float32x4_t r2301 = vrev64q_f32(r3210);
	float32x4_t sign1 = { 1.0f, -1.0f, 1.0f, -1.0f };
	r.neon = vmulq_f32(r3210, vmulq_f32(vdupq_laneq_f32(left.neon, 0), sign1));
	float32x4_t sign2 = { 1.0f, 1.0f, -1.0f, -1.0f };
	r.neon = vfmaq_f32(r.neon, r2301, vmulq_f32(vdupq_laneq_f32(left.neon, 1), sign2));
	float32x4_t sign3 = { -1.0f, 1.0f, 1.0f, -1.0f };
	r.neon = vfmaq_f32(r.neon, r1032, vmulq_f32(vdupq_laneq_f32(left.neon, 2), sign3));
	r.neon = vfmaq_laneq_f32(r.neon, right.neon, left.neon, 3);
#else
	QmQuat r =
	{
		.X = left.X * right.W + left.Y * right.Z - left.Z * right.Y + left.W * right.X,
		.Y = -left.X * right.Z + left.Y * right.W + left.Z * right.X + left.W * right.Y,
		.Z = left.X * right.Y - left.Y * right.X + left.Z * right.W + left.W * right.Z,
		.W = -left.X * right.X - left.Y * right.Y - left.Z * right.Z + left.W * right.W,
	};
#endif
	return r;
}

/// @brief 사원수의 최소값
/// @param pv 최소값을 담을 사원수
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE void qm_quat_min(QmQuat pv, const QmQuat left, const QmQuat right)
{
	pv.X = (left.X < right.X) ? left.X : right.X;
	pv.Y = (left.Y < right.Y) ? left.Y : right.Y;
	pv.Z = (left.Z < right.Z) ? left.Z : right.Z;
	pv.W = (left.W < right.W) ? left.W : right.W;
}

/// @brief 사원수의 최대값
/// @param pv 최대값을 담을 사원수
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
QN_INLINE void qm_quat_max(QmQuat pv, const QmQuat left, const QmQuat right)
{
	pv.X = (left.X > right.X) ? left.X : right.X;
	pv.Y = (left.Y > right.Y) ? left.Y : right.Y;
	pv.Z = (left.Z > right.Z) ? left.Z : right.Z;
	pv.W = (left.W > right.W) ? left.W : right.W;
}

/// @brief 두 사원수를 비교
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
/// @return 두 사원수가 같으면 참
QN_INLINE bool qm_quat_eq(const QmQuat left, const QmQuat right)
{
	return
		qm_eqf(left.X, right.X) && qm_eqf(left.Y, right.Y) &&
		qm_eqf(left.Z, right.Z) && qm_eqf(left.W, right.W);
}

/// @brief 사원수가 0인지 비교
/// @param pv 비교할 사원수
/// @return 사원수가 0이면 참
QN_INLINE bool qm_quat_isi(const QmQuat pv)
{
	return pv.X == 0.0f && pv.Y == 0.0f && pv.Z == 0.0f && pv.W == 1.0f;
}

/// @brief 사원수 내적
/// @param left 왼쪽 사원수
/// @param right 오른쪽 사원수
/// @return 내적값
QN_INLINE float qm_quat_dot(const QmQuat left, const QmQuat right)
{
#if defined QM_USE_SSE
	__m128 p = _mm_mul_ps(left.m128, right.m128);
	__m128 u = _mm_shuffle_ps(p, p, _MM_SHUFFLE(2, 3, 0, 1));
	p = _mm_add_ps(p, u);
	u = _mm_shuffle_ps(p, p, _MM_SHUFFLE(0, 1, 2, 3));
	p = _mm_add_ps(p, u);
	float r;
	_mm_store_ss(&r, p);
	return r;
#elif defined QM_USE_NEON
	float32x4_t mul = vmulq_f32(left.neon, right.neon);
	float32x4_t half = vpaddq_f32(mul, mul);
	float32x4_t full = vpaddq_f32(half, half);
	float r = vgetq_lane_f32(full, 0);
	return r;
#else
	return left.X * right.X + left.Y * right.Y + left.Z * right.Z + left.W * right.W;
#endif
}

/// @brief 사원수 거리의 제곱
/// @param pv 대상 사원수
/// @return 사원수 거리의 제곱값
QN_INLINE float qm_quat_len_sq(const QmQuat pv)
{
	return qm_quat_dot(pv, pv);
}

/// @brief 사원수 거리
/// @param pv 대상 사원수
/// @return 사원수 거리값
QN_INLINE float qm_quat_len(const QmQuat pv)
{
	return qm_sqrtf(qm_quat_len_sq(pv));
}

/// @brief 켤레 사원수
/// @param q 원본 사원수
QN_INLINE QmQuat qm_quat_conjugate(const QmQuat q)
{
	QmQuat r = { .X = -q.X, .Y = -q.Y, .Z = -q.Z, .W = q.W, };
	return r;
}

/// @brief 역사원수를 얻는다
/// @param q 원본 사원수
QN_INLINE QmQuat qm_quat_inv(const QmQuat q)
{
	return qm_quat_mag(qm_quat_conjugate(q), 1.0f / qm_quat_dot(q, q));
}

/// @brief 사원수 정규화
/// @param v 사원수
QN_INLINE QmQuat qm_quat_norm(const QmQuat v)
{
	return qm_quat_mag(v, qm_inv_sqrtf(qm_quat_dot(v, v)));
}

/// @brief 사원수 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 사원수
/// @param right 대상 사원수
/// @param scale 보간값
QN_INLINE QmQuat qm_quat_interpolate(const QmQuat left, const QmQuat right, const float scale)
{
	return qm_quat_add(qm_quat_mag(left, 1.0f - scale), qm_quat_mag(right, scale));
}

/// @brief 두 사원수를 블렌딩한다
/// @param left 왼쪽 사원수
/// @param left_scale 왼쪽 값
/// @param right 오른쪽 사원수
/// @param right_scale 오른쪽 값
QN_INLINE QmQuat qm_quat_blend(const QmQuat left, const float left_scale, const QmQuat right, const float right_scale)
{
#if defined QM_USE_SSE
	__m128 ls = _mm_set1_ps(left_scale);
	__m128 rs = _mm_set1_ps(right_scale);
	__m128 p = _mm_mul_ps(left.m128, ls);
	__m128 u = _mm_mul_ps(right.m128, rs);
	QmQuat r = { .m128 = _mm_add_ps(p, u) };
#elif defined QM_USE_NEON
	float32x4_t ls = vmulq_n_f32(left.neon, left_scale);
	float32x4_t rs = vmulq_n_f32(right.neon, right_scale);
	QmQuat r = { .neon = vaddq_f32(ls, rs) };
#else
	QmQuat r =
	{
		.X = left.X * left_scale + right.X * right_scale,
		.Y = left.Y * left_scale + right.Y * right_scale,
		.Z = left.Z * left_scale + right.Z * right_scale,
		.W = left.W * left_scale + right.W * right_scale,
	};
#endif
	return r;
}

/// @brief 사원수 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 사원수
/// @param right 대상 사원수
/// @param scale 보간값
QN_INLINE QmQuat qm_quat_lerp(const QmQuat left, const QmQuat right, const float scale)
{
	return qm_quat_norm(qm_quat_blend(left, 1.0f - scale, right, scale));
}

/// @brief 사원수의 스플라인 lerp
/// @param left 기준 사원수
/// @param right 대상 사원수
/// @param change 변화량
QN_INLINE QmQuat qm_quat_slerp(const QmQuat left, const QmQuat right, const float scale)
{
	float dot = qm_quat_dot(left, right);
	QmQuat q1, q2;
	if (dot < 0.0f)
	{
		dot = -dot;
		q1 = left;
		q2 = qm_quat(-right.X, -right.Y, -right.Z, -right.W);
	}
	else
	{
		q1 = left;
		q2 = right;
	}
	if (dot > 0.9995f)
		return qm_quat_lerp(q1, q2, scale);
	float angle = QM_ACOSF(dot);
	float ls = QM_SINF((1.0f - scale) * angle);
	float rs = QM_SINF(scale * angle);
	QmQuat b = qm_quat_blend(q1, ls, q2, rs);
	return qm_quat_norm(b);
}

/// @brief 행렬로 회전 (왼손)
/// @param m 회전 행렬
/// @return 
QN_INLINE QmQuat qm_quat_mat_lh(const QmMat4 m)
{
	float f;
	QmQuat q;
	if (m._33 < 0.0f)
	{
		if (m._11 > m._22)
		{
			f = 1 + m._11 - m._22 - m._33;
			q = qm_quat(f, m._12 + m._21, m._31 + m._13, m._32 - m._23);
		}
		else {
			f = 1 - m._11 + m._22 - m._33;
			q = qm_quat(m._12 + m._21, f, m._23 + m._32, m._13 - m._31);
		}
	}
	else
	{
		if (m._11 < -m._22)
		{
			f = 1 - m._11 - m._22 + m._33;
			q = qm_quat(m._31 + m._13, m._23 + m._32, f, m._21 - m._12);
		}
		else
		{
			f = 1 + m._11 + m._22 + m._33;
			q = qm_quat(m._32 - m._23, m._13 - m._31, m._21 - m._13, f);
		}
	}
	return qm_quat_mag(q, 0.5f / qm_sqrtf(f));
}

/// @brief 행렬로 회전 (오른손)
/// @param m 회전 행렬
/// @return 
QN_INLINE QmQuat qm_quat_mat_rh(const QmMat4 m)
{
	float f;
	QmQuat q;
	if (m._33 < 0.0f)
	{
		if (m._11 > m._22)
		{
			f = 1 + m._11 - m._22 - m._33;
			q = qm_quat(f, m._12 + m._21, m._31 + m._13, m._23 - m._32);
		}
		else
		{
			f = 1 - m._11 + m._22 - m._33;
			q = qm_quat(m._12 + m._21, f, m._23 + m._32, m._31 - m._13);
		}
	}
	else
	{
		if (m._11 < -m._22)
		{
			f = 1 - m._11 - m._22 + m._33;
			q = qm_quat(m._31 + m._13, m._23 + m._32, f, m._12 - m._21);
		}
		else
		{
			f = 1 + m._11 + m._22 + m._33;
			q = qm_quat(m._23 - m._32, m._31 - m._13, m._12 - m._21, f);
		}
	}
	return qm_quat_mag(q, 0.5f / qm_sqrtf(f));
}

/// @brief 행렬로 사원수 회전
/// @param pq 반환 사원수
/// @param rot 회전할 행렬
QSAPI void qm_quat_mat_extend(QmQuat* pq, const QmMat4* rot);

/// @brief 사원수를 벡터3 축으로 회전시킨다
/// @param v 벡터3 회전축
/// @param angle 회전값
QN_INLINE QmQuat qm_quat_axis_vec(const QmVec3 v, const float angle)
{
	float s, c;
	qm_sincos(angle * 0.5f, &s, &c);
	return qm_quatv3(qm_vec3_mag(qm_vec3_norm(v), s), c);
}

/// @brief 벡터로 화전
/// @param pq 반환 사원수
/// @param rot 회전 행렬
QN_INLINE QmQuat qm_quat_vec(const QmVec3 rot)
{
	float rs, rc, ps, pc, ys, yc;
	qm_sincos(rot.X * 0.5f, &rs, &rc);
	qm_sincos(rot.Y * 0.5f, &ps, &pc);
	qm_sincos(rot.Z * 0.5f, &ys, &yc);
	const float pcyc = pc * yc;
	const float psyc = ps * yc;
	const float pcys = pc * ys;
	const float psys = ps * ys;
	QmQuat r =
	{
		.X = rs * pcyc - rc * psys,
		.Y = rc * psyc + rs * pcys,
		.Z = rc * pcys + rs * psyc,
		.W = rc * pcyc + rs * psys,
	};
	return r;
}

/// @brief 사원수를 X축 회전시킨다
/// @param pq 결과를 담을 사원수
/// @param rot_x X축 회전값
QN_INLINE QmQuat qm_quat_x(const float rot_x)
{
	QmQuat r;
	r.Y = r.Z = 0.0f;
	qm_sincos(rot_x * 0.5f, &r.X, &r.W);
	return r;
}

/// @brief 사원수를 Y축 회전시킨다
/// @param pq 결과를 담을 사원수
/// @param rot_y Y축 회전값
QN_INLINE QmQuat qm_quat_y(const float rot_y)
{
	QmQuat r;
	r.X = r.Z = 0.0f;
	qm_sincos(rot_y * 0.5f, &r.Y, &r.W);
	return r;
}

/// @brief 사원수를 Z축 회전시킨다
/// @param pq 결과를 담을 사원수
/// @param rot_z Z축 회전값
QN_INLINE QmQuat qm_quat_z(const float rot_z)
{
	QmQuat r;
	r.X = r.Y = 0.0f;
	qm_sincos(rot_z * 0.5f, &r.Z, &r.W);
	return r;
}

/// @brief 지수 사원수 값을 얻는다
/// @param q 원본 사원수
QN_INLINE QmQuat qm_quat_exp(const QmQuat q)
{
	float n = qm_sqrtf(q.X * q.X + q.Y * q.Y + q.Z * q.Z);
	if (n == 0.0)
	{
		QmQuat t; qm_quat_rst(&t);
		return t;
	}
	float sn, cn;
	qm_sincos(n, &sn, &cn);
	n = 1.0f / n;
	return qm_quat(sn * q.X * n, sn * q.Y * n, sn * q.Z * n, cn);
}

/// @brief 사원수 로그
/// @param q 입력 사원수
QN_INLINE QmVec3 qm_quat_ln(const QmQuat q)
{
	const float n = qm_quat_len_sq(q);
	if (n > 1.0001f)
		return q.XYZ;
	if (n > 0.99999f)
	{
		const float nv = qm_vec3_len(q.XYZ);
		const float t = QM_ATAN2F(nv, q.W) / nv;
		return qm_vec3(t * q.X, t * q.Y, t * q.Z);
	}
	// 법선이 1보다 작다. 이런일은 생기지 않는다!!!!
	qn_assert(true && "법선이 1보다 작은데? 어째서???");
	QmVec3 v; qm_vec3_rst(&v);
	return v;
}


// 행렬

/// @brief 행렬을 0으로 초기화 한다
/// @param pm 초기화할 대상 행렬
QN_INLINE void qm_mat4_zero(QmMat4* m)
{
	m->_11 = m->_12 = m->_13 = m->_14 = 0.0f;
	m->_21 = m->_22 = m->_23 = m->_24 = 0.0f;
	m->_31 = m->_32 = m->_33 = m->_34 = 0.0f;
	m->_41 = m->_42 = m->_43 = m->_44 = 0.0f;
}

/// @brief 단위 행렬을 만든다
/// @param pm 단위 행렬로 만들 행렬
QN_INLINE void qm_mat4_rst(QmMat4* m)		// identify
{
	m->_12 = m->_13 = m->_14 = 0.0f;
	m->_21 = m->_23 = m->_24 = 0.0f;
	m->_31 = m->_32 = m->_34 = 0.0f;
	m->_41 = m->_42 = m->_43 = 0.0f;
	m->_11 = m->_22 = m->_33 = m->_44 = 1.0f;
}

/// @brief 대각 행렬을 만든다
/// @param diag 대각값
QN_INLINE void qm_mat4_diag(QmMat4* m, const float diag)
{
	m->_12 = m->_13 = m->_14 = 0.0f;
	m->_21 = m->_23 = m->_24 = 0.0f;
	m->_31 = m->_32 = m->_34 = 0.0f;
	m->_41 = m->_42 = m->_43 = 0.0f;
	m->_11 = m->_22 = m->_33 = m->_44 = diag;
}

/// @brief 단위 행렬인지 비교
/// @param pm 비교할 행렬
/// @return 단위 행렬이면 참을 반환
QN_INLINE bool qm_mat4_isi(const QmMat4 pm)
{
	return
		pm._11 == 1.0f && pm._12 == 0.0f && pm._13 == 0.0f && pm._14 == 0.0f &&
		pm._21 == 0.0f && pm._22 == 1.0f && pm._23 == 0.0f && pm._24 == 0.0f &&
		pm._31 == 0.0f && pm._32 == 0.0f && pm._33 == 1.0f && pm._34 == 0.0f &&
		pm._41 == 0.0f && pm._42 == 0.0f && pm._43 == 0.0f && pm._44 == 1.0f;
}

/// @brief 행렬 전치
/// @param m 전치할 행렬
QN_INLINE QmMat4 qm_mat4_tran(const QmMat4 m)
{
#if defined QM_USE_SSE
	QmMat4 r = m;
	_MM_TRANSPOSE4_PS(r.rows[0].m128, r.rows[1].m128, r.rows[2].m128, r.rows[3].m128);
#elif defined QM_USE_NEON
	QmMat4 r;
	float32x4x4_t t = vld4q_f32((float*)m.rows);
	r.rows[0].neon = t.val[0];
	r.rows[1].neon = t.val[1];
	r.rows[2].neon = t.val[2];
	r.rows[3].neon = t.val[3];
#else
	QmMat4 r =
	{
		._11 = m._11, ._12 = m._21, ._13 = m._31, ._14 = m._41,
		._21 = m._12, ._22 = m._22, ._23 = m._32, ._24 = m._42,
		._31 = m._13, ._32 = m._23, ._33 = m._33, ._34 = m._43,
		._41 = m._14, ._42 = m._24, ._43 = m._34, ._44 = m._44,
	};
#endif
	return r;
}

/// @brief 두 행렬의 덧셈
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
QN_INLINE QmMat4 qm_mat4_add(const QmMat4 left, const QmMat4 right)
{
	QmMat4 r;
	r.rows[0] = qm_vec4_add(left.rows[0], right.rows[0]);
	r.rows[1] = qm_vec4_add(left.rows[1], right.rows[1]);
	r.rows[2] = qm_vec4_add(left.rows[2], right.rows[2]);
	r.rows[3] = qm_vec4_add(left.rows[3], right.rows[3]);
	return r;
}

/// @brief 두 행렬의 뺄셈
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
QN_INLINE QmMat4 qm_mat4_sub(const QmMat4 left, const QmMat4 right)
{
	QmMat4 r;
	r.rows[0] = qm_vec4_sub(left.rows[0], right.rows[0]);
	r.rows[1] = qm_vec4_sub(left.rows[1], right.rows[1]);
	r.rows[2] = qm_vec4_sub(left.rows[2], right.rows[2]);
	r.rows[3] = qm_vec4_sub(left.rows[3], right.rows[3]);
	return r;
}

/// @brief 행렬의 확대
/// @param m 대상 행렬
/// @param scale 확대값
QN_INLINE QmMat4 qm_mat4_mag(const QmMat4 m, const float scale)
{
#if defined QM_USE_SSE
	QmMat4 r;
	__m128 mm = _mm_set1_ps(scale);
	r.rows[0].m128 = _mm_mul_ps(m.rows[0].m128, mm);
	r.rows[1].m128 = _mm_mul_ps(m.rows[1].m128, mm);
	r.rows[2].m128 = _mm_mul_ps(m.rows[2].m128, mm);
	r.rows[3].m128 = _mm_mul_ps(m.rows[3].m128, mm);
#elif defined QM_USE_NEON
	QmMat4 r;
	r.rows[0].neon = vmulq_n_f32(m.rows[0].neon, scale);
	r.rows[1].neon = vmulq_n_f32(m.rows[1].neon, scale);
	r.rows[2].neon = vmulq_n_f32(m.rows[2].neon, scale);
	r.rows[3].neon = vmulq_n_f32(m.rows[3].neon, scale);
#else
	QmMat4 r =
	{
		._11 = m._11 * scale, ._12 = m._12 * scale, ._13 = m._13 * scale, ._14 = m._14 * scale,
		._21 = m._21 * scale, ._22 = m._22 * scale, ._23 = m._23 * scale, ._24 = m._24 * scale,
		._31 = m._31 * scale, ._32 = m._32 * scale, ._33 = m._33 * scale, ._34 = m._34 * scale,
		._41 = m._41 * scale, ._42 = m._42 * scale, ._43 = m._43 * scale, ._44 = m._44 * scale,
	};
#endif
	return r;
}

#if defined QM_USE_SSE
QSAPI void qm_sse_mat4_mul(QmMat4* pm, const QmMat4* left, const QmMat4* right);
QSAPI void qm_sse_mat4_inv(QmMat4* pm, const QmMat4* m);
QSAPI float qm_sse_mat4_det(const QmMat4* m);
#endif
#if defined QM_USE_NEON
QSAPI void qm_neon_mat4_mul(QmMat4* pm, const QmMat4* left, const QmMat4* right);
QSAPI void qm_neon_mat4_inv(QmMat4* pm, const QmMat4* m);
QSAPI float qm_neon_mat4_det(const QmMat4* m);
#endif

/// @brief 행렬 곱
/// @param left 좌측 행렬
/// @param right 우측 행렬
QN_INLINE QmMat4 qm_mat4_mul(const QmMat4 left, const QmMat4 right)
{
	QmMat4 r;
#if defined QM_USE_SSE
	qm_sse_mat4_mul(&r, &left, &right);
#elif defined QM_USE_NEON
	qm_neon_mat4_mul(&r, &left, &right);
#else
	r.rows[0] = qm_vec4_trfm(right.rows[0], left);
	r.rows[1] = qm_vec4_trfm(right.rows[1], left);
	r.rows[2] = qm_vec4_trfm(right.rows[2], left);
	r.rows[3] = qm_vec4_trfm(right.rows[3], left);
#endif
	return r;
}

/// @brief 역행렬
/// @param m 입력 행렬
QN_INLINE QmMat4 qm_mat4_inv(const QmMat4 m)
{
#if defined QM_USE_SSE
	QmMat4 r;
	qm_sse_mat4_inv(&r, &m);
	return r;
#elif defined QM_USE_NEON
	QmMat4 r;
	qm_neon_mat4_inv(&r, &m);
	return r;
#else
	QmVec3 c01 = qm_vec3_cross(m.rows[0].XYZ, m.rows[1].XYZ);
	QmVec3 c23 = qm_vec3_cross(m.rows[2].XYZ, m.rows[3].XYZ);
	QmVec3 s10 = qm_vec3_sub(qm_vec3_mag(m.rows[0].XYZ, m.rows[1].W), qm_vec3_mag(m.rows[1].XYZ, m.rows[0].W));
	QmVec3 s32 = qm_vec3_sub(qm_vec3_mag(m.rows[2].XYZ, m.rows[3].W), qm_vec3_mag(m.rows[3].XYZ, m.rows[2].W));
	float inv = 1.0f / (qm_vec3_dot(c01, s32) + qm_vec3_dot(c23, s10));
	c01 = qm_vec3_mag(c01, inv);
	c23 = qm_vec3_mag(c23, inv);
	s10 = qm_vec3_mag(s10, inv);
	s32 = qm_vec3_mag(s32, inv);
	QmMat4 r;
	r.rows[0] = qm_vec4v(qm_vec3_add(qm_vec3_cross(m.rows[1].XYZ, s32), qm_vec3_mag(c23, m.rows[1].W)), -qm_vec3_dot(m.rows[1].XYZ, c23));
	r.rows[1] = qm_vec4v(qm_vec3_sub(qm_vec3_cross(s32, m.rows[0].XYZ), qm_vec3_mag(c23, m.rows[0].W)), +qm_vec3_dot(m.rows[0].XYZ, c23));
	r.rows[0] = qm_vec4v(qm_vec3_add(qm_vec3_cross(m.rows[3].XYZ, s10), qm_vec3_mag(c01, m.rows[3].W)), -qm_vec3_dot(m.rows[3].XYZ, c01));
	r.rows[1] = qm_vec4v(qm_vec3_sub(qm_vec3_cross(s10, m.rows[2].XYZ), qm_vec3_mag(c01, m.rows[2].W)), +qm_vec3_dot(m.rows[2].XYZ, c01));
	return qm_mat4_tran(r);
#endif
}

/// @brief 행렬식
/// @param m 행렬
/// @return 행렬식
QN_INLINE float qm_mat4_det(const QmMat4 m)
{
#if defined QM_USE_SSE
	return qm_sse_mat4_det(&m);
#elif defined QM_USE_NEON
	return qm_neon_mat4_det(&m);
#else
	QmVec3 c01 = qm_vec3_cross(m.rows[0].XYZ, m.rows[1].XYZ);
	QmVec3 c23 = qm_vec3_cross(m.rows[2].XYZ, m.rows[3].XYZ);
	QmVec3 s10 = qm_vec3_sub(qm_vec3_mag(m.rows[0].XYZ, m.rows[1].W), qm_vec3_mag(m.rows[1].XYZ, m.rows[0].W));
	QmVec3 s32 = qm_vec3_sub(qm_vec3_mag(m.rows[2].XYZ, m.rows[3].W), qm_vec3_mag(m.rows[3].XYZ, m.rows[2].W));
	return qm_vec3_dot(c01, s32) + qm_vec3_dot(c23, s10);
#endif
}

/// @brief 전치곱
/// @param left 왼쪽 행렬
/// @param right 오른쪽 행렬
QN_INLINE QmMat4 qm_mat4_tmul(const QmMat4 left, const QmMat4 right)
{
	return qm_mat4_tran(qm_mat4_mul(left, right));
}

/// @brief 보기 행렬을 만든다 (왼손 기준)
/// @param eye 시선의 위치
/// @param at 바라보는 방향
/// @param up 시선의 윗쪽 방향
QN_INLINE QmMat4 qm_mat4_lookat_lh(const QmVec3 eye, const QmVec3 at, const QmVec3 up)
{
	QmVec3 vz = qm_vec3_norm(qm_vec3_sub(at, eye));
	QmVec3 vx = qm_vec3_norm(qm_vec3_cross(up, vz));
	QmVec3 vy = qm_vec3_cross(vz, vx);
	QmMat4 r =
	{
		._11 = vx.X, ._12 = vy.X, ._13 = vz.X, ._14 = 0.0f,
		._21 = vx.Y, ._22 = vy.Y, ._23 = vz.Y, ._24 = 0.0f,
		._31 = vx.Z, ._32 = vy.Z, ._33 = vz.Z, ._34 = 0.0f,
		._41 = -qm_vec3_dot(vx, eye),
		._42 = -qm_vec3_dot(vy, eye),
		._43 = -qm_vec3_dot(vz, eye),
		._44 = 1.0f,
	};
	return r;
}

/// @brief 보기 행렬을 만든다 (오른손 기준)
/// @param eye 시선의 위치
/// @param at 바라보는 방향
/// @param up 시선의 윗쪽 방향
/// @return
QN_INLINE QmMat4 qm_mat4_lookat_rh(const QmVec3 eye, const QmVec3 at, const QmVec3 up)
{
	QmVec3 vz = qm_vec3_norm(qm_vec3_sub(eye, at));
	QmVec3 vx = qm_vec3_norm(qm_vec3_cross(up, vz));
	QmVec3 vy = qm_vec3_cross(vz, vx);
	QmMat4 r =
	{
		._11 = vx.X, ._12 = vy.X, ._13 = vz.X, ._14 = 0.0f,
		._21 = vx.Y, ._22 = vy.Y, ._23 = vz.Y, ._24 = 0.0f,
		._31 = vx.Z, ._32 = vy.Z, ._33 = vz.Z, ._34 = 0.0f,
		._41 = -qm_vec3_dot(vx, eye),
		._42 = -qm_vec3_dot(vy, eye),
		._43 = -qm_vec3_dot(vz, eye),
		._44 = 1.0f,
	};
	return r;
}

/// @brief 보기 행렬의 역행렬 
/// @param lm 보기 행렬
/// @return 
QN_INLINE QmMat4 qm_mat4_inv_lookat(const QmMat4 lm)
{
	QmMat4 t = qm_mat4_tran(lm);
	QmMat4 r;
	r.rows[0] = qm_vec4v(t.rows[0].XYZ, 0.0f);
	r.rows[1] = qm_vec4v(t.rows[1].XYZ, 0.0f);
	r.rows[2] = qm_vec4v(t.rows[2].XYZ, 0.0f);
	r.rows[3] = qm_vec4v(t.rows[3].XYZ, -1.0f);
	r._41 = -1.0f * lm._41 / (t._11 + t._12 + t._13);
	r._42 = -1.0f * lm._42 / (t._21 + t._22 + t._23);
	r._43 = -1.0f * lm._43 / (t._31 + t._32 + t._33);
	r._44 = 1.0f;
	return r;
}

/// @brief 투영 행렬을 만든다 (왼손 기준)
/// @param fov 포브(Field Of View)값
/// @param aspect 화면 종횡비(가로 나누기 세로)
/// @param zn,zf 뎁스 너비
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
/// @param zn,zf 뎁스 너비
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
/// @param pm 투영 행렬
QN_INLINE QmMat4 qm_mat4_inv_perspective_lh(const QmMat4 pm)
{
	const float f = 1.0f / pm._43;
	QmMat4 r =
	{
		._11 = 1.0f / pm._11,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = 1.0f / pm._22,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 0.0f,
		._34 = f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = pm._34,
		._44 = pm._33 * -f,
	};
	return r;
}

/// @brief 투영 행렬의 역행렬 (오른손)
/// @param pm 투영 행렬
QN_INLINE QmMat4 qm_mat4_inv_perspective_rh(const QmMat4 pm)
{
	const float f = 1.0f / pm._43;
	QmMat4 r =
	{
		._11 = 1.0f / pm._11,
		._12 = 0.0f,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = 0.0f,
		._22 = 1.0f / pm._22,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = 0.0f,
		._32 = 0.0f,
		._33 = 0.0f,
		._34 = f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = pm._34,
		._44 = pm._33 * f,
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
QN_INLINE QmMat4 qm_mat4_inv_ortho(const QmMat4 om)
{
	const QmVec3 diag = { .X = 1.0f / om._11, .Y = 1.0f / om._22, .Z = 1.0f / om._33 };
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
		._41 = -om._41 * diag.X,
		._42 = -om._42 * diag.Y,
		._43 = -om._43 * diag.Z,
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
QN_INLINE QmMat4 qm_mat4_loc_vec(const QmVec3 v)
{
	return qm_mat4_loc(v.X, v.Y, v.Z);
}

/// @brief 위치 행렬의 역행렬
/// @param lm 위치 행렬
QN_INLINE QmMat4 qm_mat4_inv_loc(const QmMat4 lm)
{
	QmMat4 r =
	{
		._11 = 1.0f, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = 1.0f, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f, ._34 = 0.0f,
		._41 = -lm._41, ._42 = -lm._42, ._43 = -lm._43, ._44 = 1.0f,
	};
	return r;
}

/// @brief 오른손 회전
/// @param angle 회전 각도
/// @param axis 회전 축
QN_INLINE QmMat4 qm_mat4_rot(const float angle, const QmVec3 axis)
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
QN_INLINE QmMat4 qm_mat4_inv_rot(const QmMat4 rm)
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
QN_INLINE QmMat4 qm_mat4_scl_vec(const QmVec3 v)
{
	return qm_mat4_scl(v.X, v.Y, v.Z);
}

/// @brief 스케일 행렬의 역행렬
/// @param sm 스케일 행렬
QN_INLINE QmMat4 qm_mat4_inv_scl(const QmMat4 sm)
{
	QmMat4 r =
	{
		._11 = 1.0f / sm._11, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = 1.0f / sm._22, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f / sm._33, ._34 = 0.0f,
		._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = 1.0f,
	};
	return r;
}

/// @brief 변환 행렬을 만든다
/// @param rot 벡터3 회전값
QN_INLINE QmMat4 qm_mat4_vec(const QmVec3 rot)
{
	float sr, sp, sy;
	float cr, cp, cy;
	qm_sincos(rot.X, &sr, &cr);
	qm_sincos(rot.Y, &sp, &cp);
	qm_sincos(rot.Z, &sy, &cy);
	const float srsp = sr * sp;
	const float crsp = cr * sp;
	QmMat4 r =
	{
		._11 = cp * cy,
		._12 = cp * sy,
		._13 = -sp,
		._14 = 0.0f,
		._21 = srsp * cy - cr * sy,
		._22 = srsp * sy + cr * cy,
		._23 = sr * cp,
		._24 = 0.0f,
		._31 = crsp * cy + sr * sy,
		._32 = crsp * sy - sr * cy,
		._33 = cr * cp,
		._34 = 0.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = 0.0f,
		._44 = 1.0f,
	};
	return r;
}

/// @brief 사원수로 회전 행렬을 만든다
/// @param rot 사원수 회전값
/// @return
QN_INLINE QmMat4 qm_mat4_quat(const QmQuat rot)
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

/// @brief X축 회전 행렬을 만든다
/// @param rot X측 회전값
QN_INLINE QmMat4 qm_mat4_x(const float rot)
{
	float vsin, vcos;
	qm_sincos(rot, &vsin, &vcos);
	QmMat4 r =
	{
		._11 = 1.0f, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
		._21 = 0.0f, ._22 = vcos, ._23 = vsin, ._24 = 0.0f,
		._31 = 0.0f, ._32 = -vsin, ._33 = vcos, ._34 = 0.0f,
		._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = 1.0f,
	};
	return r;
}

/// @brief Y축 회전 행렬을 만든다
/// @param rot Y측 회전값
QN_INLINE QmMat4 qm_mat4_y(const float rot)
{
	float vsin, vcos;
	qm_sincos(rot, &vsin, &vcos);
	QmMat4 r =
	{
		._11 = vcos, ._12 = 0.0f, ._13 = -vsin, ._14 = 0.0f,
		._21 = 0.0f, ._22 = 1.0f, ._23 = 0.0f, ._24 = 0.0f,
		._31 = vsin, ._32 = 0.0f, ._33 = vcos, ._34 = 0.0f,
		._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = 1.0f,
	};
	return r;
}

/// @brief Z축 회전 행렬을 만든다
/// @param rot Z측 회전값
QN_INLINE QmMat4 qm_mat4_z(const float rot)
{
	float vsin, vcos;
	qm_sincos(rot, &vsin, &vcos);
	QmMat4 r =
	{
		._11 = vcos, ._12 = vsin, ._13 = 0.0f, ._14 = 0.0f,
		._21 = -vsin, ._22 = vcos, ._23 = 0.0f, ._24 = 0.0f,
		._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f, ._34 = 0.0f,
		._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = 1.0f,
	};
	return r;
}

/// @brief 그림자 행렬을 만든다
/// @param light 빛의 방향
/// @param plane 투영될 면
QN_INLINE QmMat4 qm_mat4_shadow(const QmVec4 light, const QmPlane plane)
{
	const float d = qm_vec3_dot(plane.XYZ, light.XYZ);
	if (qm_eqf(light.W, 0.0f))
	{
		QmMat4 r =
		{
			._11 = -plane.A * light.X + d,
			._12 = -plane.A * light.Y,
			._13 = -plane.A * light.Z,
			._14 = 0.0f,
			._21 = -plane.B * light.X,
			._22 = -plane.B * light.Y + d,
			._23 = -plane.B * light.Z,
			._24 = 0.0f,
			._31 = -plane.C * light.X,
			._32 = -plane.C * light.Y,
			._33 = -plane.C * light.Z + d,
			._34 = 0.0f,
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
/// @param scl 스케일 (1일 경우 NULL)
/// @param rotcenter 회전축(원점일 경우 NULL)
/// @param rot 회전 (고정일 경우 NULL)
/// @param loc 위치 (원점일 경우 NULL)
QN_INLINE QmMat4 qm_mat4_affine(const QmVec3* scl, const QmVec3* rotcenter, const QmQuat* rot, const QmVec3* loc)
{
	QmMat4 m1, m2, m3, m4, m5;
	if (scl)
		qm_mat4_scl_vec(*scl);
	else
		qm_mat4_rst(&m1);
	if (rotcenter)
	{
		m2 = qm_mat4_loc(-rotcenter->X, -rotcenter->Y, -rotcenter->Z);
		m4 = qm_mat4_loc_vec(*rotcenter);
	}
	else
	{
		qm_mat4_rst(&m2);
		qm_mat4_rst(&m4);
	}
	if (rot)
		m3 = qm_mat4_quat(*rot);
	else
		qm_mat4_rst(&m3);
	if (loc)
		m5 = qm_mat4_loc_vec(*loc);
	else
		qm_mat4_rst(&m5);
	QmMat4 m = qm_mat4_mul(m1, m2);
	m = qm_mat4_mul(m, m3);
	m = qm_mat4_mul(m, m4);
	return qm_mat4_mul(m, m5);
}

//
QN_INLINE QmMat4 qm_mat4_trfm_loc_scl(QmMat4 m, const QmVec3 loc, const QmVec3* scl)
{
	float* f = m.l;
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
/// @param loc 위치
/// @param rot 회전
/// @param scl 스케일 (1일 경우 NULL)
QN_INLINE QmMat4 qm_mat4_trfm(const QmVec3 loc, const QmQuat rot, const QmVec3* scl)
{
	QmMat4 m = qm_mat4_quat(rot);
	return qm_mat4_trfm_loc_scl(m, loc, scl);
}

/// @brief 행렬 트랜스폼. 단 벡터 회전
/// @param loc 위치
/// @param rot 회전
/// @param scl 스케일 (1일 경우 NULL)
QN_INLINE QmMat4 qm_mat4_trfm_vec(const QmVec3 loc, const QmVec3 rot, const QmVec3* scl)
{
	QmMat4 m = qm_mat4_vec(rot);
	return qm_mat4_trfm_loc_scl(m, loc, scl);
}


// plane

/// @brief 면을 만든다
/// @param a,b,c,d 면의 설정값
QN_INLINE QmPlane qm_plane(const float a, const float b, const float c, const float d)
{
	QmPlane r =
#if defined QM_USE_SSE
	{ .m128 = _mm_setr_ps(a, b, c, d) };
#elif defined QM_USE_NEON
	{.neon = { a, b, c, d } };
#else
	{.A = a, .B = b, .C = c, .D = d, };
#endif
	return r;
}

/// @brief 벡터로 면을 만든다
/// @param v 벡터
/// @param d 면의 법선
QN_INLINE QmPlane qm_planev(const QmVec3 v, const float d)
{
	return qm_plane(v.X, v.Y, v.Z, d);
}

/// @brief 점과 점의 법선으로 면을 만든다
/// @param pv 점
/// @param pn 점의 법선
QN_INLINE QmPlane qm_planevv(const QmVec3 pv, const QmVec3 pn)
{
	return qm_planev(pn, -qm_vec3_dot(pv, pn));
}

/// @brief 점 세개로 평면을 만든다
/// @param v1 점1
/// @param v2 점2
/// @param v3 점3
QN_INLINE QmPlane qm_planevvv(const QmVec3 v1, const QmVec3 v2, const QmVec3 v3)
{
	QmVec3 t = qm_vec3_norm(qm_vec3_cross(qm_vec3_sub(v2, v1), qm_vec3_sub(v3, v2)));
	return qm_planev(t, -qm_vec3_dot(v1, t));
}

/// @brief 면을 만든다
///	@param p 반환 면
/// @param a,b,c,d 면의 설정값
QN_INLINE void qm_plane_set(QmPlane* p, const float a, const float b, const float c, const float d)
{
#if defined QM_USE_SSE
	p->m128 = _mm_setr_ps(a, b, c, d);
#elif defined QM_USE_NEON
	p->neon = { a, b, c, d };
#else
	p->A = a, p->B = b, p->C = c, p->D = d;
#endif
}

/// @brief 벡터로 면을 만든다
///	@param p 반환 면
/// @param v 벡터
/// @param d 면의 법선
QN_INLINE void qm_plane_setv(QmPlane* p, const QmVec3 v, const float d)
{
	qm_plane_set(p, v.X, v.Y, v.Z, d);
}

/// @brief 점과 점의 법선으로 면을 만든다
///	@param p 반환 면
/// @param pv 점
/// @param pn 점의 법선
QN_INLINE void qm_plane_setvv(QmPlane* p, const QmVec3 pv, const QmVec3 pn)
{
	qm_plane_setv(p, pn, -qm_vec3_dot(pv, pn));
}

/// @brief 점 세개로 평면을 만든다
///	@param p 반환 면
/// @param v1 점1
/// @param v2 점2
/// @param v3 점3
QN_INLINE void qm_plane_setvvv(QmPlane* p, const QmVec3 v1, const QmVec3 v2, const QmVec3 v3)
{
	QmVec3 t = qm_vec3_norm(qm_vec3_cross(qm_vec3_sub(v2, v1), qm_vec3_sub(v3, v2)));
	qm_plane_setv(p, t, -qm_vec3_dot(v1, t));
}

/// @brief 면을 초기화한다
///	@param p 반환 면
QN_INLINE void qm_plane_rst(QmPlane* p)
{
	qm_plane_set(p, 0.0f, 0.0f, 0.0f, 1.0f);
}

/// @brief 면 확대
/// @param left 원본 면
/// @param right 확대값
QN_INLINE QmPlane qm_plane_mag(const QmPlane left, const float right)
{
#if defined QM_USE_SSE
	__m128 m = _mm_set1_ps(right);
	QmPlane r = { .m128 = _mm_mul_ps(left.m128, m) };
#elif defined QM_USE_NEON
	QmPlane r = { .neon = vmulq_n_f32(left.neon, right) };
#else
	QmPlane r = { .A = left.A * right, .B = left.B * right, .C = left.C * right, .D = left.D * right };
#endif
	return r;
}

/// @brief 두 면를 비교
/// @param left 왼쪽 면
/// @param right 오른쪽 면
/// @return 두 면가 같으면 참
QN_INLINE bool qm_plane_eq(const QmPlane left, const QmPlane right)
{
	return
		qm_eqf(left.A, right.A) && qm_eqf(left.B, right.B) &&
		qm_eqf(left.C, right.C) && qm_eqf(left.D, right.D);
}

/// @brief 벡터가 0인지 비교
/// @param p 비교할 면
/// @return 면가 0이면 참
QN_INLINE bool qm_plane_isi(const QmPlane p)
{
	return p.A == 0.0f && p.B == 0.0f && p.C == 0.0f && p.D == 1.0f;
}

/// @brief 면과 점(벡터3)의 내적
/// @param p 대상 면
/// @param v 대상 벡터3
/// @return 면과 점의 내적
QN_INLINE float qm_plane_dot_coord(const QmPlane p, const QmVec3 v)
{
	return p.A * v.X + p.B * v.Y + p.C * v.Z + p.D;
}

/// @brief 점과 면의 관계를 얻는다
/// @param p 대상 면
/// @param v 관계를 얻을 점
/// @return 관계 값을 얻는다
/// @retval 0 점이 면 위에 있다
/// @retval 1 점이 면 앞에 있다
/// @retval -1 점이 면 뒤에 있다
QN_INLINE int qm_plane_rel_point(const QmPlane p, const QmVec3 v)
{
	const float f = qm_vec3_dot(p.XYZ, v) + p.D;
	if (f < -QM_EPSILON)
		return -1;  // back
	if (f > QM_EPSILON)
		return 1;   // front
	return 0;       // on
}

/// @brief 면과 점의 거리를 얻는다
/// @param p 대상 면
/// @param v 대상 점
/// @return 면과 점의 거리
QN_INLINE float qm_plane_dist(const QmPlane p, const QmVec3 v)
{
	return qm_vec3_dot(v, p.XYZ) + p.D;
}

/// @brief 면을 정규화 한다
/// @param p 대상 면
QN_INLINE QmPlane qm_plane_norm(const QmPlane p)
{
	const float f = qm_inv_sqrtf(qm_vec3_len_sq(p.XYZ));
	return qm_plane_mag(p, f);
}

/// @brief 면을 뒤집어서 정규화 한다
/// @param p 대상 면
QN_INLINE QmPlane qm_plane_rev_norm(const QmPlane p)
{
	const float f = -qm_inv_sqrtf(qm_vec3_len_sq(p.XYZ));
	return qm_plane_mag(p, f);
}

/// @brief 면과 선분의 거리를 얻는다
/// @param p 대상 면
/// @param begin 대상 선의 시작
/// @param end 대상 선의 끝
/// @return 면과 선분의 거리
QN_INLINE float qm_plane_distance_line(const QmPlane p, const QmVec3 begin, const QmVec3 end)
{
	const float f = 1.0f / qm_vec3_dot(p.XYZ, qm_vec3_sub(end, begin));
	return -(qm_vec3_dot(p.XYZ, begin) + p.D) * f;
}

/// @brief 면과 면의 충돌 평면을 만든다
/// @param p 대상 평면
/// @param o 검사할 평면
/// @param loc 반환 시작 벡터 (반환, 널가능)
/// @param dir 반환 방향 벡터 (반환, 널가능)
/// @return 만들 수 있으면 TRUE
QN_INLINE bool qm_plane_intersect(const QmPlane p, const QmPlane o, QmVec3* loc, QmVec3* dir)
{
	const float f0 = qm_vec3_len(p.XYZ);
	const float f1 = qm_vec3_len(o.XYZ);
	const float f2 = qm_vec3_dot(p.XYZ, o.XYZ);
	const float det = f0 * f1 - f2 * f2;
	if (qm_absf(det) < QM_EPSILON)
		return false;
	if (dir)
		*dir = qm_vec3_cross(p.XYZ, o.XYZ);
	if (loc)
	{
		const float inv = 1.0f / det;
		const float fa = (f1 * -p.D + f2 * o.D) * inv;
		const float fb = (f0 * -o.D + f2 * p.D) * inv;
		*loc = qm_vec3(p.A * fa + o.A * fb, p.B * fa + o.B * fb, p.C * fa + o.C * fb);
	}
	return true;
}

/// @brief 면과 선분 충돌 여부
/// @param plane 면
/// @param loc 선의 시작
/// @param dir 선의 방량
/// @param pv 충돌 위치 (반환, 널가능)
/// @return 충돌하면 참
QN_INLINE bool qm_plane_intersect_line(const QmPlane plane, const QmVec3 loc, const QmVec3 dir, QmVec3* pv)
{
	// v2.pl<-v1
	const float dot = qm_vec3_dot(plane.XYZ, dir);
	if (qm_eqf(dot, 0.0f))
	{
		if (pv)
			qm_vec3_rst(pv);
		return false;
	}
	if (pv)
	{
		const float tmp = (plane.D + qm_vec3_dot(plane.XYZ, loc)) / dot;
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
QN_INLINE bool qm_plane_intersect_planes(const QmPlane plane, const QmPlane other1, const QmPlane other2, QmVec3* pv)
{
	QmVec3 dir, loc;
	return (qm_plane_intersect(plane, other1, &loc, &dir)) ? qm_plane_intersect_line(other2, loc, dir, pv) : false;
}

/// @brief 이거 뭔지 기억이 안난다. 뭐에 쓰던거지. 기본적으로 qm_plane_intersect_line 에다 방향 벡터와의 거리 계산 추가
/// @param plane 면
/// @param v1 벡터1
/// @param v2 벡터2
/// @param pv 충돌 지점 (반환, 널가능)
/// @return 충돌하면서 방향 벡터의 거리 안쪽(?)이면 참
QN_INLINE bool qm_plane_intersect_between_point(const QmPlane plane, const QmVec3 v1, const QmVec3 v2, QmVec3* pv)
{
	QmVec3 dir = qm_vec3_sub(v2, v1), point;
	if (!qm_plane_intersect_line(plane, v1, dir, &point))
		return false;
	const float f = qm_vec3_len_sq(dir);
	if (pv)
		*pv = point;
	return qm_vec3_dist_sq(point, v1) <= f && qm_vec3_dist_sq(point, v2) <= f;
}

/// @brief 면 트랜스폼
/// @param plane 대상 면
/// @param trfm 트랜스폼 행렬
QN_INLINE QmPlane qm_plane_trfm(const QmPlane plane, const QmMat4 trfm)
{
	QmVec3 vt = qm_vec3_trfm(qm_vec3_mag(plane.XYZ, -plane.D), trfm);
	QmVec3 nm = qm_vec3_norm(plane.XYZ);
	QmVec3 dg = qm_vec3(trfm._11, trfm._22, trfm._33);
	if (!qm_eqf(dg.X, 0.0f) && !qm_eqf(dg.Y, 0.0f) && !qm_eqf(dg.Z, 0.0f) &&
		(qm_eqf(dg.X, 1.0f) || qm_eqf(dg.Y, 1.0f) || qm_eqf(dg.Z, 1.0f)))
		nm = qm_vec3_div(nm, qm_vec3_mul(dg, dg));
	QmVec3 tn = qm_vec3_norm(qm_vec3_trfm_norm(nm, trfm));
	return qm_planev(tn, -qm_vec3_dot(vt, tn));
}


// line3

/// @brief 선분을 만든다
/// @param bx,by,bz 시작 점
/// @param ex,ey,ez 끝 점
QN_INLINE QmLine3 qm_line3(const float bx, const float by, const float bz, const float ex, const float ey, const float ez)
{
	QmLine3 r = { .Begin = qm_vec3(bx, by, bz),.End = qm_vec3(ex, ey, ez), };
	return r;
}

/// @brief 벡터3 두개로 선분을 만든다
/// @param begin 시작 벡터3
/// @param end 끝 벡터3
QN_INLINE QmLine3 qm_line3vv(const QmVec3 begin, const QmVec3 end)
{
	QmLine3 r = { .Begin = begin, .End = end };
	return r;
}

/// @brief 선분을 만든다
///	@param l 반환 선분
/// @param bx,by,bz 시작 점
/// @param ex,ey,ez 끝 점
QN_INLINE void qm_line3_set(QmLine3* l, const float bx, const float by, const float bz, const float ex, const float ey, const float ez)
{
	l->Begin = qm_vec3(bx, by, bz);
	l->End = qm_vec3(ex, ey, ez);
}

/// @brief 벡터3 두개로 선분을 만든다
///	@param l 반환 선분
/// @param begin 시작 벡터3
/// @param end 끝 벡터3
QN_INLINE void qm_line3_setvv(QmLine3* l, const QmVec3 begin, const QmVec3 end)
{
	l->Begin = begin;
	l->End = end;
}

/// @brief 선분을 이동한다 (덧셈)
/// @param l 원본 선분
/// @param v 이동 거리 벡터3
QN_INLINE QmLine3 qm_line3_add(const QmLine3 l, const QmVec3 v)
{
	QmLine3 r = { .Begin = qm_vec3_add(l.Begin, v), .End = qm_vec3_add(l.End, v) };
	return r;
}

/// @brief 선분을 이동한다 (뺄셈)
/// @param l 원본 선분
/// @param v 이동 거리 벡터3
QN_INLINE QmLine3 qm_line3_sub(const QmLine3 l, const QmVec3 v)
{
	QmLine3 r = { .Begin = qm_vec3_sub(l.Begin, v), .End = qm_vec3_sub(l.End, v) };
	return r;
}

/// @brief 선분 길이의 제곱을 얻는다
/// @param l 대상 선분
/// @return 선분 길이의 제곱
QN_INLINE float qm_line3_len_sq(const QmLine3 l)
{
	return qm_vec3_dist_sq(l.Begin, l.End);
}

/// @brief 선분 길이를 얻는다
/// @param l 대상 선분
/// @return 선분 길이
QN_INLINE float qm_line3_len(const QmLine3 l)
{
	return qm_vec3_dist(l.Begin, l.End);
}

/// @brief 선분의 중심을 얻는다
/// @param l 대상 선분
QN_INLINE QmVec3 qm_line3_center(const QmLine3 l)
{
	return qm_vec3_mag(qm_vec3_add(l.Begin, l.End), 0.5f);
}

/// @brief 선분의 벡터를 얻는다
/// @param l 대상 선분
QN_INLINE QmVec3 qm_line3_vec(const QmLine3 l)
{
	return qm_vec3_sub(l.End, l.Begin);
}

/// @brief 점(벡터3)이 선분 위에 있는지 판단한다
/// @param l 대상 선분
/// @param v 대상 점
/// @return 점이 선분 위에 있으면 참
QN_INLINE bool qm_line3_in(const QmLine3 l, const QmVec3 v)
{
	return qm_vec3_between(v, l.Begin, l.End);
}

/// @brief 구와 충돌하는 선 판정
/// @param l 처리할 선
/// @param org 구의 중점
/// @param rad 구의 반지름
/// @param dist 충돌 거리
/// @return 충돌하면 true
QN_INLINE bool qm_line3_intersect_sphere(const QmLine3 l, const QmVec3 org, const float rad, float* dist)
{
	QmVec3 v = qm_vec3_norm(qm_line3_vec(l));
	QmVec3 t = qm_vec3_sub(org, l.Begin);
	const float c = qm_vec3_len(t);
	const float z = qm_vec3_dot(t, v);
	const float d = rad * rad - (c * c - z * z);
	if (d < 0.0f)
		return false;
	if (dist)
		*dist = z - qm_sqrtf(d);
	return true;
}


// half

/// @brief 32비트 실수를 16비트 실수로 변환한다
/// @param v 32비트 실수
/// @return 변환한 16비트 실수
QN_INLINE halfint qm_f2hf(const float v)
{
	qn_barrier();
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
}

/// @brief 16비트 실수를 32비트 실수로 변환한다
/// @param v 변환할 16비트 실수
/// @return 변환한 32비트 실수
QN_INLINE float qm_hf2f(const halfint v)
{
	qn_barrier();
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
}

/// @brief 하프 벡터2를 설정한다
/// @param x,y 좌표
QN_INLINE QmVecH2 qm_vec2h(const float x, const float y)
{
	QmVecH2 r = { .X = qm_f2hf(x), .Y = qm_f2hf(y) };
	return r;
}

/// @brief 하프 벡터2를 설정한다
///	@param v 반환 벡터
/// @param x,y 좌표
QN_INLINE void qm_vec2h_set(QmVecH2* v, const float x, const float y)
{
	v->X = qm_f2hf(x);
	v->Y = qm_f2hf(y);
}

/// @brief 하프 벡터3를 설정한다
///	@param v 반환 벡터
/// @param x,y,z 좌표
QN_INLINE QmVecH3 qm_vec3h(const float x, const float y, const float z)
{
	QmVecH3 r = { .X = qm_f2hf(x), .Y = qm_f2hf(y), .Z = qm_f2hf(z) };
	return r;
}

/// @brief 하프 벡터3를 설정한다
/// @param x,y,z 좌표
QN_INLINE void qm_vec3h_set(QmVecH3* v, const float x, const float y, const float z)
{
	v->X = qm_f2hf(x);
	v->Y = qm_f2hf(y);
	v->Z = qm_f2hf(z);
}

/// @brief 하프 벡터4를 설정한다
/// @param x,y,z,w 좌표
QN_INLINE QmVecH4 qm_vec4h(const float x, const float y, const float z, const float w)
{
	QmVecH4 r = { .X = qm_f2hf(x), .Y = qm_f2hf(y), .Z = qm_f2hf(z), .W = qm_f2hf(w) };
	return r;
}

/// @brief 하프 벡터4를 설정한다
///	@param v 반환 벡터
/// @param x,y,z,w 좌표
QN_INLINE void qm_vec4h_set(QmVecH4* v, const float x, const float y, const float z, const float w)
{
	v->X = qm_f2hf(x);
	v->Y = qm_f2hf(y);
	v->Z = qm_f2hf(z);
	v->W = qm_f2hf(w);
}


// 실수형 점 

#define qm_pointf				qm_vec2
#define qm_pointf_set			qm_vec2_set
#define qm_pointf_rst			qm_vec2_rst
#define qm_pointf_add			qm_vec2_add
#define qm_pointf_sub			qm_vec2_sub
#define qm_pointf_mag			qm_vec2_mag
#define qm_pointf_mul			qm_vec2_mul
#define qm_pointf_div			qm_vec2_div
#define qm_pointf_min			qm_vec2_min
#define qm_pointf_max			qm_vec2_max
#define qm_pointf_eq			qm_vec2_eq
#define qm_pointf_len			qm_vec2_len
#define qm_pointf_len_sq		qm_vec2_len_sq
#define qm_pointf_dist			qm_vec2_dist
#define qm_pointf_dist_sq		qm_vec2_dist_sq
#define qm_pointf_interpolete	qm_vec2_interpolate
#define qm_pointf_lerp			qm_vec2_lerp


// 실수형 크기

#define qm_sizef				qm_vec2
#define qm_sizef_set			qm_vec2_set
#define qm_sizef_rst			qm_vec2_rst
#define qm_sizef_add			qm_vec2_add
#define qm_sizef_sub			qm_vec2_sub
#define qm_sizef_mag			qm_vec2_mag
#define qm_sizef_mul			qm_vec2_mul
#define qm_sizef_div			qm_vec2_div
#define qm_sizef_min			qm_vec2_min
#define qm_sizef_max			qm_vec2_max
#define qm_sizef_eq				qm_vec2_eq
#define qm_sizef_len			qm_vec2_len
#define qm_sizef_len_sq			qm_vec2_len_sq
#define qm_sizef_dist			qm_vec2_dist
#define qm_sizef_dist_sq		qm_vec2_dist_sq
#define qm_sizef_interpolete	qm_vec2_interpolate
#define qm_sizef_lerp			qm_vec2_lerp

/// @brief 사각형으로 크기를 설정한다
/// @param rt 대상 사각형
QN_INLINE QmSizeF qm_sizef_set_rect(const QmRectF rt)
{
	return qm_vec2(rt.Right - rt.Left, rt.Bottom - rt.Top);
}

/// @brief 종횡비(너비 나누기 높이)를 계신한다
/// @param pt 계산할 크기
/// @return 종횡비값
QN_INLINE float qm_sizef_aspect(const QmSizeF pt)
{
	return pt.Width / pt.Height;
}

/// @brief 대각선 DPI를 구한다
/// @param pt 너비와 높이
/// @param horizontal 너비 DPI
/// @param vertical 높이 DPI
/// @return 대각선 DPI
QN_INLINE float qm_sizef_diag_dpi(const QmSizeF pt, const float horizontal, const float vertical)
{
	const float dsq = horizontal * horizontal + vertical + vertical;
	if (dsq <= 0.0f)
		return 0.0f;
	return qm_sqrtf(pt.Width * pt.Width + pt.Height * pt.Height) / qm_sqrtf(dsq);
}


// 깊이

#define qm_depth				qm_vec2
#define qm_depth_set			qm_vec2_set


// 최소 최대

#define qm_minmaxf				qm_vec2
#define qm_minmaxf_set			qm_vec2_set


// 실수형 사각형

#define qm_rectf				qm_vec4
#define qm_rectf_set			qm_vec4_set
#define qm_rectf_rst			qm_vec4_rst
#define qm_rectf_add			qm_vec4_add
#define qm_rectf_sub			qm_vec4_sub
#define qm_rectf_mag			qm_vec4_mag
#define qm_rectf_min			qm_vec4_min
#define qm_rectf_max			qm_vec4_max
#define qm_rectf_eq				qm_vec4_eq
#define qm_rectf_interpolete	qm_vec4_interpolate
#define qm_rectf_lerp			qm_vec4_lerp

/// @brief 사각형을 좌표와 크기로 설정한다
/// @param x,y 좌표
/// @param width,height 크기
QN_INLINE QmRectF qm_rectf_set_size(const float x, const float y, const float width, const float height)
{
	return qm_vec4(x, y, x + width, y + height);
}

/// @brief 사각형을 좌표와 크기 타입을 사용하여 설정한다
/// @param pos 좌표
/// @param size 크기
QN_INLINE QmRectF qm_rectf_set_pos_size(const QmPointF pos, const QmSizeF size)
{
	return qm_vec4(pos.X, pos.Y, pos.X + size.Width, pos.Y + size.Height);
}

/// @brief 사각형 크기를 키운다 (요소가 양수일 경우)
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른쪽
/// @param bottom 아래쪽
QN_INLINE QmRectF qm_rectf_inflate(const QmRectF rt, const float left, const float top, const float right, const float bottom)
{
	return qm_vec4(rt.Left - left, rt.Top - top, rt.Right + right, rt.Bottom + bottom);
}

/// @brief 사각형 크기를 줄인다 (요소가 양수일 경우)
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른족
/// @param bottom 아래쪽
QN_INLINE QmRectF qm_rectf_deflate(const QmRectF rt, const float left, const float top, const float right, const float bottom)
{
	return qm_vec4(rt.Left + left, rt.Top + top, rt.Right - right, rt.Bottom - bottom);
}

/// @brief 사각형을 움직인다
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른쪽
/// @param bottom 아래쪽
QN_INLINE QmRectF qm_rectf_offset(const QmRectF rt, const float left, const float top, const float right, const float bottom)
{
	return qm_vec4(rt.Left + left, rt.Top + top, rt.Right + right, rt.Bottom + bottom);
}

/// @brief 사각형을 움직인다
/// @param rt 원본 사각형
/// @param left 새 왼쪽
/// @param top 새 오른쪽
/// @return
QN_INLINE QmRectF qm_rectf_move(const QmRectF rt, const float left, const float top)
{
	const float dx = left - rt.Left;
	const float dy = top - rt.Top;
	return qm_vec4(rt.Left + dx, rt.Top + dy, rt.Right + dx, rt.Bottom + dy);
}

/// @brief 사각형의 크기를 재설정한다
/// @param rt 원본 사각형
/// @param width 너비
/// @param height 높이
QN_INLINE QmRectF qm_rectf_resize(const QmRectF rt, const float width, const float height)
{
	return qm_vec4(rt.Left, rt.Top, rt.Left + width, rt.Top + height);
}

/// @brief 사각형의 너비를 얻는다
/// @param rt 대상 사각형
/// @return 사각형의 너비값
QN_INLINE float qm_rectf_width(const QmRectF rt)
{
	return rt.Right - rt.Left;
}

/// @brief 사각형의 높이를 얻는다
/// @param rt 대상 사각형
/// @return 사각형의 높이값
QN_INLINE float qm_rectf_height(const QmRectF rt)
{
	return rt.Bottom - rt.Top;
}

/// @brief 좌표가 사각형 안에 있는지 조사한다
/// @param rt 대상 사각형
/// @param x,y 좌표
/// @return 좌표가 사각형 안에 있으면 참
QN_INLINE bool qm_rectf_in(const QmRectF rt, const float x, const float y)
{
	return (x >= rt.Left && x <= rt.Right && y >= rt.Top && y <= rt.Bottom);
}

/// @brief 두 사각형이 충돌하는지 비교하고 충돌 사각형을 만든다
/// @param r1 왼쪽 사각형
/// @param r2 오른쪽 사각형
/// @param p 충돌 사각형이 담길 사각형 (NULL 가능)
/// @return 두 사각형이 충돌했다면 참
QN_INLINE bool qm_rectf_intersect(const QmRectF r1, const QmRectF r2, QmRectF* p)
{
	const bool b = r2.Left < r1.Right && r2.Right > r1.Left && r2.Top < r1.Bottom && r2.Bottom > r1.Top;
	if (p)
	{
		if (!b)
			qm_rectf_rst(p);
		else
			*p = qm_rectf(
				QN_MAX(r1.Left, r2.Left), QN_MAX(r1.Top, r2.Top),
				QN_MIN(r1.Right, r2.Right), QN_MIN(r1.Bottom, r2.Bottom));
	}
	return b;
}


// color

#define qm_color				qm_vec4
#define qm_color_set			qm_vec4_set
#define qm_color_rst			qm_vec4_rst
#define qm_color_add			qm_vec4_add
#define qm_color_sub			qm_vec4_sub
#define qm_color_mag			qm_vec4_mag
#define qm_color_mul			qm_vec4_mul
#define qm_color_div			qm_vec4_div
#define qm_color_min			qm_vec4_min
#define qm_color_max			qm_vec4_max
#define qm_color_eq				qm_vec4_eq
#define qm_color_interpolete	qm_vec4_interpolate
#define qm_color_lerp			qm_vec4_lerp

/// @brief 32비트 RGBA 정수로 색깔을 설정한다
/// @param value 32비트 RGBA 정수
QN_INLINE QmColor qm_coloru(uint value)
{
	const float f = 1.0f / 255.0f;
	QmColor r;
	r.B = (float)(value & 255) * f; value >>= 8;
	r.G = (float)(value & 255) * f; value >>= 8;
	r.R = (float)(value & 255) * f; value >>= 8;
	r.A = (float)(value & 255) * f;
	return r;
}

/// @brief 정수형 색깔로 색깔을 설정한다
/// @param cu 정수형 색깔
QN_INLINE QmColor qm_colork(const QmKolor cu)
{
	const float f = 1.0f / 255.0f;
	QmColor r;
	r.A = (float)cu.A * f;
	r.R = (float)cu.R * f;
	r.G = (float)cu.G * f;
	r.B = (float)cu.B * f;
	return r;
}

/// @brief 32비트 RGBA 정수로 색깔을 설정한다
/// @param c 설정할 색깔
/// @param value 32비트 RGBA 정수
QN_INLINE void qm_color_setu(QmColor* c, uint value)
{
	const float f = 1.0f / 255.0f;
	c->B = (float)(value & 255) * f; value >>= 8;
	c->G = (float)(value & 255) * f; value >>= 8;
	c->R = (float)(value & 255) * f; value >>= 8;
	c->A = (float)(value & 255) * f;
}

/// @brief 정수형 색깔로 색깔을 설정한다
/// @param c 설정할 색깔
/// @param cu 정수형 색깔
QN_INLINE void qm_color_setk(QmColor*c, const QmKolor cu)
{
	const float f = 1.0f / 255.0f;
	c->A = (float)cu.A * f;
	c->R = (float)cu.R * f;
	c->G = (float)cu.G * f;
	c->B = (float)cu.B * f;
}

/// @brief 네거티브 색깔을 얻는다
/// @param c 원본 색깔
QN_INLINE QmColor qm_color_neg(const QmColor c)
{
	return qm_vec4(1.0f - c.R, 1.0f - c.G, 1.0f - c.B, c.A);
}

/// @brief 색깔을 32비트 정수로 만든다 (빠른 버전)
/// @param c 대상 색깔
/// @return 32비트 정수
QN_INLINE uint qm_color_to_uint_fast(const QmColor c)
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
QN_INLINE uint qm_color_to_uint(const QmColor c)
{
	const byte R = (c.R >= 1.0f) ? 0xff : (c.R <= 0.0f) ? 0x00 : (byte)(c.R * 255.0f + 0.5f);
	const byte G = (c.G >= 1.0f) ? 0xff : (c.G <= 0.0f) ? 0x00 : (byte)(c.G * 255.0f + 0.5f);
	const byte B = (c.B >= 1.0f) ? 0xff : (c.B <= 0.0f) ? 0x00 : (byte)(c.B * 255.0f + 0.5f);
	const byte A = (c.A >= 1.0f) ? 0xff : (c.A <= 0.0f) ? 0x00 : (byte)(c.A * 255.0f + 0.5f);
	return ((uint)A << 24) | ((uint)R << 16) | ((uint)G << 8) | (uint)B;
}

/// @brief 색깔의 콘트라스트를 조정한다
/// @param c 원본 색깔
/// @param scale 조정값
QN_INLINE QmColor qm_color_contrast(const QmColor c, const float scale)
{
	return qm_vec4(
		0.5f + scale * (c.R - 0.5f),
		0.5f + scale * (c.G - 0.5f),
		0.5f + scale * (c.B - 0.5f),
		c.A);
}

/// @brief 색깔의 새츄레이션을 조정한다
/// @param c 원본 색깔
/// @param scale 조정값
QN_INLINE QmColor qm_color_saturation(const QmColor c, const float scale)
{
	const float g = c.R * 0.2125f + c.G * 0.7154f + c.B * 0.0721f;
	return qm_vec4(
		g + scale * (c.R - g),
		g + scale * (c.G - g),
		g + scale * (c.B - g),
		c.A);
}


// point

#define qm_point				qm_veci2
#define qm_point_set			qm_veci2_set
#define qm_point_rst			qm_veci2_rst
#define qm_point_add			qm_veci2_add
#define qm_point_sub			qm_veci2_sub
#define qm_point_mag			qm_veci2_mag
#define qm_point_mul			qm_veci2_mul
#define qm_point_div			qm_veci2_div
#define qm_point_min			qm_veci2_min
#define qm_point_max			qm_veci2_max
#define qm_point_eq				qm_veci2_eq
#define qm_point_len			qm_veci2_len
#define qm_point_len_sq			qm_veci2_len_sq
#define qm_point_dist			qm_veci2_dist
#define qm_point_dist_sq		qm_veci2_dist_sq


// size

#define qm_size					qm_veci2
#define qm_size_set				qm_veci2_set
#define qm_size_rst				qm_veci2_rst
#define qm_size_add				qm_veci2_add
#define qm_size_sub				qm_veci2_sub
#define qm_size_mag				qm_veci2_mag
#define qm_size_mul				qm_veci2_mul
#define qm_size_div				qm_veci2_div
#define qm_size_min				qm_veci2_min
#define qm_size_max				qm_veci2_max
#define qm_size_eq				qm_veci2_eq
#define qm_size_len				qm_veci2_len
#define qm_size_len_sq			qm_veci2_len_sq
#define qm_size_dist			qm_veci2_dist
#define qm_size_dist_sq			qm_veci2_dist_sq

/// @brief 사각형으로 크기를 설정한다
/// @param rt 대상 사각형
QN_INLINE QmSize qm_size_set_rect(const QmRect rt)
{
	return qm_veci2(rt.Right - rt.Left, rt.Bottom - rt.Top);
}

/// @brief 종횡비(너비 나누기 높이)를 계신한다
/// @param pt 계산할 크기
/// @return 종횡비값
QN_INLINE float qm_size_aspect(const QmSize pt)
{
	return (float)pt.Width / (float)pt.Height;
}

/// @brief 대각선 DPI를 구한다
/// @param pt 너비와 높이
/// @param horizontal 너비 DPI
/// @param vertical 높이 DPI
/// @return 대각선 DPI
QN_INLINE float qm_size_diag_dpi(const QmSize pt, const float horizontal, const float vertical)
{
	const float dsq = horizontal * horizontal + vertical + vertical;
	if (dsq <= 0.0f)
		return 0.0f;
	return qm_sqrtf((float)pt.Width * (float)pt.Width + (float)pt.Height * (float)pt.Height) / qm_sqrtf(dsq);
}


// 사각형

#define qm_rect					qm_veci4
#define qm_rect_set				qm_veci4_set
#define qm_rect_rst				qm_veci4_rst
#define qm_rect_add				qm_veci4_add
#define qm_rect_sub				qm_veci4_sub
#define qm_rect_mag				qm_veci4_mag
#define qm_rect_min				qm_veci4_min
#define qm_rect_max				qm_veci4_max
#define qm_rect_eq				qm_veci4_eq

/// @brief 사각형을 좌표와 크기로 설정한다
/// @param x,y 좌표
/// @param width,height 크기
QN_INLINE QmRect qm_rect_set_size(const int x, const int y, const int width, const int height)
{
	return qm_veci4(x, y, x + width, y + height);
}

/// @brief 사각형을 좌표와 크기 타입을 사용하여 설정한다
/// @param pos 좌표
/// @param size 크기
QN_INLINE QmRect qm_rect_set_pos_size(const QmPoint pos, const QmSize size)
{
	return qm_veci4(pos.X, pos.Y, pos.X + size.Width, pos.Y + size.Height);
}

/// @brief 사각형 크기를 키운다 (요소가 양수일 경우)
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른쪽
/// @param bottom 아래쪽
QN_INLINE QmRect qm_rect_inflate(const QmRect rt, const int left, const int top, const int right, const int bottom)
{
	return qm_veci4(rt.Left - left, rt.Top - top, rt.Right + right, rt.Bottom + bottom);
}

/// @brief 사각형 크기를 줄인다 (요소가 양수일 경우)
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른족
/// @param bottom 아래쪽
QN_INLINE QmRect qm_rect_deflate(const QmRect rt, const int left, const int top, const int right, const int bottom)
{
	return qm_veci4(rt.Left + left, rt.Top + top, rt.Right - right, rt.Bottom - bottom);
}

/// @brief 사각형을 움직인다
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른쪽
/// @param bottom 아래쪽
QN_INLINE QmRect qm_rect_offset(const QmRect rt, const int left, const int top, const int right, const int bottom)
{
	return qm_veci4(rt.Left + left, rt.Top + top, rt.Right + right, rt.Bottom + bottom);
}

/// @brief 사각형을 움직인다
/// @param rt 원본 사각형
/// @param left 새 왼쪽
/// @param top 새 오른쪽
/// @return
QN_INLINE QmRect qm_rect_move(const QmRect rt, const int left, const int top)
{
	const int dx = left - rt.Left;
	const int dy = top - rt.Top;
	return qm_veci4(rt.Left + dx, rt.Top + dy, rt.Right + dx, rt.Bottom + dy);
}

/// @brief 사각형의 크기를 재설정한다
/// @param rt 재설정할 사각형
/// @param width 너비
/// @param height 높이
QN_INLINE QmRect qm_rect_resize(const QmRect rt, const int width, const int height)
{
	return qm_veci4(rt.Left, rt.Top, rt.Left + width, rt.Top + height);
}

/// @brief 사각형의 너비를 얻는다
/// @param rt 대상 사각형
/// @return 사각형의 너비값
QN_INLINE int qm_rect_width(const QmRect rt)
{
	return rt.Right - rt.Left;
}

/// @brief 사각형의 높이를 얻는다
/// @param rt 대상 사각형
/// @return 사각형의 높이값
QN_INLINE int qm_rect_height(const QmRect rt)
{
	return rt.Bottom - rt.Top;
}

/// @brief 좌표가 사각형 안에 있는지 조사한다
/// @param rt 대상 사각형
/// @param x,y 좌표
/// @return 좌표가 사각형 안에 있으면 참
QN_INLINE bool qm_rect_in(const QmRect rt, const int x, const int y)
{
	return (x >= rt.Left && x <= rt.Right && y >= rt.Top && y <= rt.Bottom);
}

/// @brief 두 사각형이 충돌하는지 비교하고 충돌 사각형을 만든다
/// @param r1 왼쪽 사각형
/// @param r2 오른쪽 사각형
/// @param p 충돌 사각형이 담길 사각형 (NULL 가능)
/// @return 두 사각형이 충돌했다면 참
QN_INLINE bool qm_rect_intersect(const QmRect r1, const QmRect r2, QmRect* p)
{
	const bool b = r2.Left < r1.Right && r2.Right > r1.Left && r2.Top < r1.Bottom && r2.Bottom > r1.Top;
	if (p)
	{
		if (!b)
			qm_rect_rst(p);
		else
			*p = qm_rect(
				QN_MAX(r1.Left, r2.Left), QN_MAX(r1.Top, r2.Top),
				QN_MIN(r1.Right, r2.Right), QN_MIN(r1.Bottom, r2.Bottom));
	}
	return b;
}


// kolor

/// @brief 색깔을 설정한다
/// @param r 빨강
/// @param g 녹색
/// @param b 파랑
/// @param a 알파
QN_INLINE QmKolor qm_kolor(const byte r, const byte g, const byte b, const byte a)
{
	QmKolor k = { .R = r, .G = g, .B = b, .A = a };
	return k;
}

/// @brief 색깔을 설정한다
/// @param r 빨강
/// @param g 녹색
/// @param b 파랑
/// @param a 알파
QN_INLINE QmKolor qm_kolorf(const float r, const float g, const float b, const float a)
{
	QmKolor k =
	{
		.R = (byte)(r * 255.0f),
		.G = (byte)(g * 255.0f),
		.B = (byte)(b * 255.0f),
		.A = (byte)(a * 255.0f)
	};
	return k;
}

/// @brief 색깔을 설정한다
/// @param value 32비트 RGBA 정수
QN_INLINE QmKolor qm_koloru(const uint value)
{
	QmKolor k = { .U = value };
	return k;
}

/// @brief 색깔을 설정한다
/// @param cr 실수형 색깔
QN_INLINE QmKolor qm_kolorc(const QmColor cr)
{
	QmKolor k =
	{
		.R = (byte)(cr.R * 255.0f),
		.G = (byte)(cr.G * 255.0f),
		.B = (byte)(cr.B * 255.0f),
		.A = (byte)(cr.A * 255.0f),
	};
	return k;
}

/// @brief 색깔을 설정한다
/// @param k 반환 색깔
/// @param r 빨강
/// @param g 녹색
/// @param b 파랑
/// @param a 알파
QN_INLINE void qm_kolor_set(QmKolor* k, const byte r, const byte g, const byte b, const byte a)
{
	k->R = r, k->G = g, k->B = b, k->A = a;
}

/// @brief 색깔을 설정한다
/// @param k 반환 색깔
/// @param r 빨강
/// @param g 녹색
/// @param b 파랑
/// @param a 알파
QN_INLINE void qm_kolor_setf(QmKolor* k, const float r, const float g, const float b, const float a)
{
	k->R = (byte)(r * 255.0f);
	k->G = (byte)(g * 255.0f);
	k->B = (byte)(b * 255.0f);
	k->A = (byte)(a * 255.0f);
}

/// @brief 색깔을 설정한다
/// @param k 반환 색깔
/// @param value 32비트 RGBA 정수
QN_INLINE void qm_kolor_setu(QmKolor* k, const uint value)
{
	k->U = value;
}

/// @brief 색깔을 설정한다
/// @param k 반환 색깔
/// @param cr 실수형 색깔
QN_INLINE void qm_kolor_setc(QmKolor* k, const QmColor cr)
{
	k->R = (byte)(cr.R * 255.0f);
	k->G = (byte)(cr.G * 255.0f);
	k->B = (byte)(cr.B * 255.0f);
	k->A = (byte)(cr.A * 255.0f);
}

/// @brief 두 색깔의 덧셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
/// @return
QN_INLINE QmKolor qm_kolor_add(const QmKolor left, const QmKolor right)
{
	QmKolor k = { .R = left.R + right.R, .G = left.G + right.G, .B = left.B + right.B, .A = left.A + right.A };
	return k;
}

/// @brief 두 색깔의 뺄셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
/// @return
QN_INLINE QmKolor qm_kolor_sub(const QmKolor left, const QmKolor right)
{
	QmKolor k = { .R = left.R - right.R, .G = left.G - right.G, .B = left.B - right.B, .A = left.A - right.A };
	return k;
}

/// @brief 색깔을 확대한다 (밝기가 아니다)
/// @param left 왼쪽 색깔
/// @param scale 확대값
QN_INLINE QmKolor qm_kolor_mag(const QmKolor left, const float scale)
{
	QmKolor k =
	{
		.R = (byte)((float)left.R * scale),
		.G = (byte)((float)left.G * scale),
		.B = (byte)((float)left.B * scale),
		.A = (byte)((float)left.A * scale),
	};
	return k;
}

/// @brief 네거티브 색깔을 만든다
/// @param c 원본 색깔
QN_INLINE QmKolor qm_kolor_neg(const QmKolor c)
{
	QmKolor k = { .R = 255 - c.R, .G = 255 - c.G, .B = 255 - c.B, .A = c.A, };
	return k;
}

/// @brief 두 색깔의 곱셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
QN_INLINE QmKolor qm_kolor_mul(const QmKolor left, const QmKolor right)
{
	QmKolor k = { .R = left.R * right.R, .G = left.G * right.G, .B = left.B * right.B, .A = left.A * right.A };
	return k;
}

/// @brief 두 색깔의 나눗셈
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
QN_INLINE QmKolor qm_kolor_div(const QmKolor left, const QmKolor right)
{
	QmKolor k = { .R = left.R / right.R, .G = left.G / right.G, .B = left.B / right.B, .A = left.A / right.A };
	return k;
}

/// @brief 색깔을 보간한다
/// @param left 시작 색깔
/// @param right 끝 색깔
/// @param scale 보간값
QN_INLINE QmKolor qm_kolor_interpolate(const QmKolor left, const QmKolor right, const float scale)
{
	const float f = 1.0f - scale;
	QmKolor k =
	{
		.R = (byte)((float)left.R * f + (float)right.R * scale),
		.G = (byte)((float)left.G * f + (float)right.G * scale),
		.B = (byte)((float)left.B * f + (float)right.B * scale),
		.A = (byte)((float)left.A * f + (float)right.A * scale),
	};
	return k;
}

/// @brief 색깔을 선형 보간한다
/// @param left 시작 색깔
/// @param right 끝 색깔
/// @param scale 보간값
/// @return
QN_INLINE QmKolor qm_kolor_lerp(const QmKolor left, const QmKolor right, const float scale)
{
	QmKolor k =
	{
		.R = (byte)((float)left.R + scale * (float)(right.R - left.R)),
		.G = (byte)((float)left.G + scale * (float)(right.G - left.G)),
		.B = (byte)((float)left.B + scale * (float)(right.B - left.B)),
		.A = (byte)((float)left.A + scale * (float)(right.A - left.A)),
	};
	return k;
}

/// @brief 두 색깔의 최소값
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
QN_INLINE QmKolor qm_kolor_min(const QmKolor left, const QmKolor right)
{
	QmKolor k =
	{
		.R = (left.R < right.R) ? left.R : right.R,
		.G = (left.G < right.G) ? left.G : right.G,
		.B = (left.B < right.B) ? left.B : right.B,
		.A = (left.A < right.A) ? left.A : right.A,
	};
	return k;
}

/// @brief 두 색깔의 최대값
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
QN_INLINE QmKolor qm_kolor_max(const QmKolor left, const QmKolor right)
{
	QmKolor k =
	{
		.R = (left.R > right.R) ? left.R : right.R,
		.G = (left.G > right.G) ? left.G : right.G,
		.B = (left.B > right.B) ? left.B : right.B,
		.A = (left.A > right.A) ? left.A : right.A,
	};
	return k;
}

/// @brief 두 색깔이 같은지 비교한다
/// @param left 왼쪽 색깔
/// @param right 오른쪽 색깔
/// @return 두 색깔이 같으면 참
QN_INLINE bool qm_kolor_eq(const QmKolor left, const QmKolor right)
{
	return left.U == right.U;
}


//////////////////////////////////////////////////////////////////////////
// 제네릭

/// @brief (제네릭) 설정
#define qm_set(o,f,...)	_Generic((o),\
	QmVec2*: qm_vec2_set,\
	QmVec3*: qm_vec3_set,\
	QmVec4*: _Generic((f),\
		float: qm_vec4_set,\
		QmVec3: qm_vec4_setv),\
	QmVecI2*: qm_veci2_set,\
	QmVecI3*: qm_veci3_set,\
	QmVecI4*: _Generic((f),\
		int: qm_veci4_set,\
		QmVecI3: qm_veci4_setv),\
	QmQuat*: _Generic((f),\
		float: qm_quat_set,\
		QmVec3: qm_quat_setv3),\
	QmPlane*: _Generic((f),\
		float: qm_plane_set,\
		QmVec3: qm_plane_setv),\
	QmLine3*: _Generic((f),\
		float: qm_line3_set,\
		QmVec3: qm_line3_setvv),\
	QmKolor*: _Generic((f),\
		byte: qm_kolor_set,\
		float: qm_kolor_setf),\
	QmVecH2*: qm_vec2h_set,\
	QmVecH3*: qm_vec3h_set,\
	QmVecH4*: qm_vec4h_set)(o,f,__VA_ARGS__)
/// @brief (제네릭) 설정 항목1개
#define qm_set_1(o,v)	_Generic((o),\
	QmVec2*: qm_vec2_setv,\
	QmQuat*: qm_quat_setv4,\
	QmColor*: _Generic((v),\
		uint: qm_color_setu,\
		QmKolor: qm_color_setk),\
	QmKolor*: _Generic((v),\
		uint: qm_kolor_setu,\
		QmColor: qm_kolor_setc))(o,v)
/// @brief (제네릭) 리셋
#define qm_rst(o)		_Generic((o),\
	QmVec2*: qm_vec2_rst,\
	QmVec3*: qm_vec3_rst,\
	QmVec4*: qm_vec4_rst,\
	QmVecI2*: qm_veci2_rst,\
	QmVecI3*: qm_veci3_rst,\
	QmVecI4*: qm_veci4_rst,\
	QmQuat*: qm_quat_rst,\
	QmMat4*: qm_mat4_rst,\
	QmPlane*: qm_plane_rst)(o)
/// @brief (제네릭) 대각
#define qm_diag(o)		_Generic((o),\
	QmVec2*: qm_vec2_diag,\
	QmVec3*: qm_vec3_diag,\
	QmVec4*: qm_vec4_diag,\
	QmVecI2*: qm_veci2_diag,\
	QmVecI3*: qm_veci3_diag,\
	QmVecI4*: qm_veci4_diag,\
	QmQuat*: qm_quat_diag,\
	QmMat4*: qm_mat4_diag)(o)
/// @brief (제네릭) 부호 반전
#define qm_ivt(x)		_Generic((x),\
	QmVec2: qm_vec2_ivt,\
	QmVec3: qm_vec3_ivt,\
	QmVec4: qm_vec4_ivt,\
	QmVecI2: qm_veci2_ivt,\
	QmVecI3: qm_veci3_ivt,\
	QmVecI4: qm_veci4_ivt,\
	QmQuat: qm_quat_ivt)(x)
/// @brief (제네릭) 네거티브
#define qm_neg(x)		_Generic((x),\
	QmVec2: qm_vec2_neg,\
	QmVec3: qm_vec3_neg,\
	QmVec4: qm_vec4_neg,\
	QmQuat: qm_quat_neg,\
	QmColor: qm_color_neg,\
	QmKolor: qm_kolor_neg)(x)
/// @brief (제네릭) 덧셈
#define qm_add(l,r)		_Generic((l),\
	QmVec2: qm_vec2_add,\
	QmVec3: qm_vec3_add,\
	QmVec4: qm_vec4_add,\
	QmVecI2: qm_veci2_add,\
	QmVecI3: qm_veci3_add,\
	QmVecI4: qm_veci4_add,\
	QmQuat: qm_quat_add,\
	QmMat4: qm_mat4_add,\
	QmLine3: qm_line3_add,\
	QmKolor: qm_kolor_add)(l,r)
/// @brief (제네릭) 뺄셈
#define qm_sub(l,r)		_Generic((l),\
	QmVec2: qm_vec2_sub,\
	QmVec3: qm_vec3_sub,\
	QmVec4: qm_vec4_sub,\
	QmVecI2: qm_veci2_sub,\
	QmVecI3: qm_veci3_sub,\
	QmVecI4: qm_veci4_sub,\
	QmQuat: qm_quat_sub,\
	QmMat4: qm_mat4_sub,\
	QmLine3: qm_line3_sub,\
	QmKolor: qm_kolor_sub)(l,r)
/// @brief (제네릭) 확대
#define qm_mag(i,s)		_Generic((i),\
	QmVec2: qm_vec2_mag,\
	QmVec3: qm_vec3_mag,\
	QmVec4: qm_vec4_mag,\
	QmVecI2: qm_veci2_mag,\
	QmVecI3: qm_veci3_mag,\
	QmVecI4: qm_veci4_mag,\
	QmQuat: qm_quat_mag,\
	QmMat4: qm_mat4_mag,\
	QmPlane: qm_plane_mag,\
	QmKolor: qm_kolor_mag)(i,s)
/// @brief (제네릭) 곱셈
#define qm_mul(l,r)		_Generic((l),\
	QmVec2: qm_vec2_mul,\
	QmVec3: qm_vec3_mul,\
	QmVec4: qm_vec4_mul,\
	QmVecI2: qm_veci2_mul,\
	QmVecI3: qm_veci3_mul,\
	QmVecI4: qm_veci4_mul,\
	QmQuat: qm_quat_mul,\
	QmMat4: qm_mat4_mul,\
	QmKolor: qm_kolor_mul)(l,r)
/// @brief (제네릭) 나눗셈
#define qm_div(l,r)		_Generic((l),\
	QmVec2: qm_vec2_div,\
	QmVec3: qm_vec3_div,\
	QmVec4: qm_vec4_div,\
	QmVecI2: qm_veci2_div,\
	QmVecI3: qm_veci3_div,\
	QmVecI4: qm_veci4_div,\
	QmKolor: qm_kolor_div)(l,r)
/// @brief (제네릭) 최소값
#define qm_min(l,r)		_Generic((l),\
	float: qm_minf,\
	QmVec2: qm_vec2_min,\
	QmVec3: qm_vec3_min,\
	QmVec4: qm_vec4_min,\
	QmVecI2: qm_veci2_min,\
	QmVecI3: qm_veci3_min,\
	QmVecI4: qm_veci4_min,\
	QmQuat: qm_quat_min,\
	QmKolor: qm_kolor_min)(l,r)
/// @brief (제네릭) 최대값
#define qm_max(l,r)		_Generic((l),\
	float: qm_maxf,\
	QmVec2: qm_vec2_max,\
	QmVec3: qm_vec3_max,\
	QmVec4: qm_vec4_max,\
	QmVecI2: qm_veci2_max,\
	QmVecI3: qm_veci3_max,\
	QmVecI4: qm_veci4_max,\
	QmQuat: qm_quat_max,\
	QmKolor: qm_kolor_max)(l,r)
/// @brief (제네릭) 같나 비교
#define qm_eq(l,r)		_Generic((l),\
	float: qm_eqf,\
	QmVec2: qm_vec2_eq,\
	QmVec3: qm_vec3_eq,\
	QmVec4: qm_vec4_eq,\
	QmVecI2: qm_veci2_eq,\
	QmVecI3: qm_veci3_eq,\
	QmVecI4: qm_veci4_eq,\
	QmQuat: qm_quat_eq,\
	QmPlane: qm_plane_eq,\
	QmKolor: qm_kolor_eq)(l,r)
/// @brief (제네릭) 단위 요소인가 확인
#define qm_isi(x)		_Generic((x),\
	QmVec2: qm_vec2_isi,\
	QmVec3: qm_vec3_isi,\
	QmVec4: qm_vec4_isi,\
	QmVecI2: qm_veci2_isi,\
	QmVecI3: qm_veci3_isi,\
	QmVecI4: qm_veci4_isi,\
	QmQuat: qm_quat_isi,\
	QmMat4: qm_mat4_isi,\
	QmPlane: qm_plane_isi)(x)
/// @brief (제네릭) 내적
#define qm_dot(l,r)		_Generic((l),\
	QmVec2: qm_vec2_dot,\
	QmVec3: qm_vec3_dot,\
	QmVec4: qm_vec4_dot,\
	QmVecI2: qm_veci2_dot,\
	QmVecI3: qm_veci3_dot,\
	QmVecI4: qm_veci4_dot,\
	QmQuat: qm_quat_dot,\
	QmPlane: qm_plane_dot_coord)(l,r)
/// @brief (제네릭) 외적
#define qm_cross(l,r)		_Generic((l),\
	QmVec2: qm_vec2_cross,\
	QmVec3: qm_vec3_cross,\
	QmVec4: qm_vec4_cross,\
	QmVecI2: qm_veci2_cross,\
	QmVecI3: qm_veci3_cross,\
	QmVecI4: qm_veci4_cross)(l,r)
/// @brief (제네릭) 길의 제곱
#define qm_len_sq(x)	_Generic((x),\
	QmVec2: qm_vec2_len_sq,\
	QmVec3: qm_vec3_len_sq,\
	QmVec4: qm_vec4_len_sq,\
	QmVecI2: qm_veci2_len_sq,\
	QmVecI3: qm_veci3_len_sq,\
	QmVecI4: qm_veci4_len_sq,\
	QmQuat: qm_quat_len_sq,\
	QmLine3: qm_line3_len_sq)(x)
/// @brief (제네릭) 길이
#define qm_len(x)		_Generic((x),\
	QmVec2: qm_vec2_len,\
	QmVec3: qm_vec3_len,\
	QmVec4: qm_vec4_len,\
	QmVecI2: qm_veci2_len,\
	QmVecI3: qm_veci3_len,\
	QmVecI4: qm_veci4_len,\
	QmQuat: qm_quat_len,\
	QmLine3: qm_line3_len)(x)
/// @brief (제네릭) 거리의 제곱
#define qm_dist_sq(x,y)	_Generic((x),\
	QmVec2: qm_vec2_dist_sq,\
	QmVec3: qm_vec3_dist_sq,\
	QmVec4: qm_vec4_dist_sq,\
	QmVecI2: qm_veci2_dist_sq,\
	QmVecI3: qm_veci3_dist_sq,\
	QmVecI4: qm_veci4_dist_sq)(x,y)
/// @brief (제네릭) 거리
#define qm_dist(x,y)	_Generic((x),\
	QmVec2: qm_vec2_dist,\
	QmVec3: qm_vec3_dist,\
	QmVec4: qm_vec4_dist,\
	QmVecI2: qm_veci2_dist,\
	QmVecI3: qm_veci3_dist,\
	QmVecI4: qm_veci4_dist,\
	QmPlane: qm_plane_dist)(x,y)
/// @brief (제네릭) 정규화
#define qm_norm(x)		_Generic((x),\
	QmVec2: qm_vec2_norm,\
	QmVec3: qm_vec3_norm,\
	QmVec4: qm_vec4_norm,\
	QmQuat: qm_quat_norm,\
	QmPlane: qm_plane_norm)(o)
/// @brief (제네릭) 원본에서 대상으로 보간
#define qm_interpolate(l,r,s)	_Generic((l),\
	QmVec2: qm_vec2_interpolate,\
	QmVec3: qm_vec3_interpolate,\
	QmVec4: qm_vec4_interpolate,\
	QmQuat: qm_quat_interpolate,\
	QmKolor: qm_kolor_interpolate)(l,r,s)
/// @brief (제네릭) 선형 보간
#define qm_lerp(l,r,s)	_Generic((l),\
	float: qm_lerpf,\
	QmVec2: qm_vec2_lerp,\
	QmVec3: qm_vec3_lerp,\
	QmVec4: qm_vec4_lerp,\
	QmQuat: qm_quat_lerp,\
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

QN_EXTC_END

#ifdef __GNUC__
#pragma GCC diagnotics pop
#endif

// set rst diag ivt add sub  mag mul div min max eq isi
// dot cross lensq len distsq dist
