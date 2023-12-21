/*
 * QsLib [MATH Layer]
 * Made by kim 2004-2024
 *
 * 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
 * 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
 */
 /**
  * @file qs_math.h
  *
  * [MATH Layer]는 [QG Layer]에서 사용할 수학 기능 구현을 목표로 합니다.
  */
#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)		// L4, nonstandard extension used : nameless struct/union
#pragma warning(disable:4514)		// L4, 'function' : unreferenced inline function has been removed
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#include <math.h>
#if defined _MSC_VER
#include <intrin.h>
#elif defined _M_IX86 || defined _M_X64 || defined __i386__  || defined __amd64__ || defined __x86_64__
#include <xmmintrin.h>
#endif

  //////////////////////////////////////////////////////////////////////////
  // constant
#define QN_EPSILON						0.0001														/** @brief 엡실론 */
#define QN_E							2.7182818284590452353602874713526624977572470937000			/** @brief 지수 */
#define QN_LOG2_E						1.44269504088896340736										/** @brief 로그2의 밑 지수 */
#define QN_LOG10_E						0.434294481903251827651										/** @brief 로그10의 밑 지수 */
#define QN_LOG2_B10						0.30102999566398119521										
#define QN_LN2							0.6931471805599453094172321214581765680755001343603			/** @brief 2로그 */
#define QN_LN10							2.3025850929940456840179914546843642076011014886288			/** @brief 10로그 */
#define QN_PI							3.1415926535897932384626433832795028841971693993751			/** @brief 원주율 */
#define QN_PI2							6.2831853071795864769252867665590057683943387987502			/** @brief 원주율 두배 */
#define QN_PI_H							1.5707963267948966192313216916397514420985846996876			/** @brief 원주울의 반 */
#define QN_PI_Q							0.7853981633974483096156608458198757210492923498438			/** @brief 원주율의 사분의 일 */
#define QN_SQRT2						1.4142135623730950488016887242096980785696718753769			/** @brief 2의 제곱근 */
#define QN_SQRTH						0.7071067811865475244008443621048490392848359376884			/** @brief 2의 제곱근의 반 */


//////////////////////////////////////////////////////////////////////////
// macro & inline

/** @brief 실수를 정수부만 뽑기 */
#define QN_FRACT(f)						((f)-floorf(f))
/** @brief 각도를 호도로 변환 */
#define QN_TORADIAN(degree)				((degree)*(180.0f/(float)QN_PI))
/** @brief 호도를 각도로 변환 */
#define QN_TODEGREE(radian)				((radian)*((float)QN_PI/180.0f))
/** @brief 실수의 앱실론 비교 */
#define QN_EQEPS(a,b,epsilon)			(((a)+(epsilon)>(b)) && ((a)-(epsilon)<(b)))

/** @brief 실수의 엡실론 비교 */
QN_INLINE bool qn_eqf(const float a, const float b) { return QN_EQEPS(a, b, (float)QN_EPSILON); }
/** @brief 두 실수의 최대값 */
QN_INLINE float qn_maxf(const float a, const float b) { return QN_MAX(a, b); }
/** @brief 두 실수의 최소값 */
QN_INLINE float qn_minf(const float a, const float b) { return QN_MIN(a, b); }
/** @brief 실수의 절대값 */
QN_INLINE float qn_absf(const float v) { return QN_ABS(v); }
/** @brief 실수를 범위 내로 자르기 */
QN_INLINE float qn_clampf(const float v, const float min, const float max) { return QN_CLAMP(v, min, max); }
/** @brief 각도를 -180 ~ +180 사이로 자르기 */
QN_INLINE float qn_cradf(const float v) { return v < (float)-QN_PI_H ? v + (float)QN_PI_H : v >(float)QN_PI_H ? v - (float)QN_PI_H : v; }
/** @brief 실수의 보간 */
QN_INLINE float qn_lerpf(const float l, const float r, const float f) { return l + f * (r - l); }
/** @brief 사인과 코사인을 동시에 계산 */
QN_INLINE void qn_sincosf(const float f, float* s, float* c) { *s = sinf(f); *c = cosf(f); }


//////////////////////////////////////////////////////////////////////////
// types

typedef struct QnVec2	QnVec2;								/** @brief 벡터2 */
typedef struct QnVec3	QnVec3;								/** @brief 벡터3 */
typedef struct QnVec4	QnVec4;								/** @brief 벡터4 */
typedef struct QnQuat	QnQuat;								/** @brief 사원수 */
typedef struct QnMat4	QnMat4;								/** @brief 행렬4x4 */
typedef struct QnPoint	QnPoint;							/** @brief 점 */
typedef struct QnSize	QnSize;								/** @brief 크기 */
typedef struct QnRect	QnRect;								/** @brief 사각형 */
typedef struct QnRectF	QnRectF;							/** @brief 실수형 사각형 */
typedef struct QnDepth	QnDepth;							/** @brief 뎁스 */
typedef struct QnCoord	QnCoord;							/** @brief 좌표 */
typedef struct QnColor	QnColor;							/** @brief 실수형 색깔 */
typedef struct QnKolor	QnKolor;							/** @brief 정수형 색깔 */
typedef struct QnKolorU	QnKolorU;							/** @brief 혼합형 정수형 색깔 */
typedef struct QnPlane	QnPlane;							/** @brief 면 */
typedef struct QnLine3	QnLine3;							/** @brief 선분 */
typedef struct QnTrfm	QnTrfm;								/** @brief 트랜스폼 */
typedef struct QnVecH2	QnVecH2;							/** @brief 하프 벡터2 */
typedef struct QnVecH3	QnVecH3;							/** @brief 하프 벡터3 */
typedef struct QnVecH4	QnVecH4;							/** @brief 하프 벡터3 */

/** @brief vector2 */
struct QnVec2
{
	float x, y;
};

/** @brief vector3 */
struct QnVec3
{
	float x, y, z;
};

/** @brief vector4 */
struct QN_STRUCT_ALIGN(16) QnVec4
{
	float x, y, z, w;
};

/** @brief quaternion */
struct QN_STRUCT_ALIGN(16) QnQuat
{
	float x, y, z, w;
};

/** @brief matrix4 */
struct QN_STRUCT_ALIGN(16) QnMat4
{
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float f[4][4];
		float f16[16];
#ifdef _INCLUDED_MM2
		__m128 m128[4];
#endif
	};
};

/** @brief point */
struct QnPoint
{
	int x, y;
};

/** @brief size */
struct QnSize
{
	int width, height;
};

/** @brief rect */
struct QnRect
{
	int left, top, right, bottom;
};

/** @brief depth */
struct QnDepth
{
	// unfortunately lower near & far are empty macros in Windows
	float Near, Far;
};

/** @brief coordinate */
struct QnCoord
{
	float u, v;
};

/** @brief color */
struct QN_STRUCT_ALIGN(16) QnColor
{
	float r, g, b, a;
};

/** @brief byte color */
struct QN_STRUCT_ALIGN(4) QnKolor
{
	byte b, g, r, a;
};

/** @brief unified byte color */
struct QnKolorU
{
	union
	{
		QnKolor k;
		uint u;
	};
};

/** @brief plane */
struct QnPlane
{
	float a, b, c, d;
};

/** @brief line3 */
struct QnLine3
{
	QnVec3 begin, end;
};

/** @brief transform */
struct QnTrfm
{
	QnQuat rot;
	QnVec3 loc;
	QnVec3 scl;
	float unused[2];
};

/** @brief half vector2 */
struct QnVecH2
{
	halfint x, y;
};

/** @brief half vector3 */
struct QnVecH3
{
	halfint x, y, z;
};

/** @brief half vector4 */
struct QnVecH4
{
	halfint x, y, z, w;
};


//////////////////////////////////////////////////////////////////////////
// functions

// external function
QN_EXTC_BEGIN
/**
 * @brief 선에 가까운 점의 위치
 * @param pv 반환되는 근처 점
 * @param line 선
 * @param loc 점
*/
QSAPI void qn_vec3_closed_line(QnVec3* pv, const QnLine3* line, const QnVec3* loc);
/**
 * @brief 단위 벡터로 만들어 크기를 조정하고 길이 만큼 혼합
 * @param pv 결과 벡터
 * @param left 시작 벡터
 * @param right 끝 벡터
 * @param scale 크기 변화량
 * @param len 길이
*/
QSAPI void qn_vec3_lerp_len(QnVec3* pv, const QnVec3* left, const QnVec3* right, float scale, float len);
/**
 * @brief 세 벡터로 법선 벡터를 만든다
 * @param pv 얻은 법선 벡터
 * @param v0 벡터 1
 * @param v1 벡터 2
 * @param v2 벡터 3
*/
QSAPI void qn_vec3_form_norm(QnVec3* pv, const QnVec3* v0, const QnVec3* v1, const QnVec3* v2);
/**
 * @brief 반사 벡터를 만든다
 * @param pv 반사된 벡터
 * @param in 입력 벡터
 * @param dir 법선 벡터
 * @return 반사 벡터가 만들어지면 참
*/
QSAPI bool qn_vec3_reflect(QnVec3* pv, const QnVec3* in, const QnVec3* dir);
/**
 * @brief 면과 선분 충돌 여부
 * @param pv 충돌 지점
 * @param plane 면
 * @param loc 선의 시작
 * @param dir 선의 방량
 * @return 충돌하면 참
*/
QSAPI bool qn_vec3_intersect_line(QnVec3* pv, const QnPlane* plane, const QnVec3* loc, const QnVec3* dir);
/**
 * @brief 두 벡터 사이의 선분과 면의 충돌 여부
 * @param pv 충돌 지점
 * @param plane 면
 * @param v1 벡터1
 * @param v2 벡터2
 * @return 충돌하면 참
*/
QSAPI bool qn_vec3_intersect_point(QnVec3* pv, const QnPlane* plane, const QnVec3* v1, const QnVec3* v2);
/**
 * @brief 이거 뭔지 기억이 안난다. 뭐에 쓰던거지. 기본적으로 qn_vec3_intersect_point 에다 방향 벡터와의 거리 계산 추가
 * @param pv 충돌 지점
 * @param plane 면
 * @param v1 벡터1
 * @param v2 벡터2
 * @return 충돌하면서 방향 벡터의 거리 안쪽(?)이면 참
*/
QSAPI bool qn_vec3_intersect_between_point(QnVec3* pv, const QnPlane* plane, const QnVec3* v1, const QnVec3* v2);
/**
 * @brief 세 면이 충돌 하면 참
 * @param pv 충돌 위치
 * @param plane 기준 면
 * @param other1 대상 면1
 * @param other2 대상 면2
 * @return 충돌하면 참
*/
QSAPI bool qn_vec3_intersect_planes(QnVec3* pv, const QnPlane* plane, const QnPlane* other1, const QnPlane* other2);
/**
 * @brief 사원수의 스플라인 lerp
 * @param pq 반환 사원수
 * @param left 기준 사원수
 * @param right 대상 사원수
 * @param change 변화량
*/
QSAPI void qn_quat_slerp(QnQuat* pq, const QnQuat* left, const QnQuat* right, float change);
/**
 * @brief 행렬로 사원수 회전
 * @param pq 반환 사원수
 * @param rot 회전할 행렬
*/
QSAPI void qn_quat_mat4(QnQuat* pq, const QnMat4* rot);
/**
 * @brief 벡터로 화전
 * @param pq 반환 사원수
 * @param rot 회전 행렬
*/
QSAPI void qn_quat_vec(QnQuat* pq, const QnVec3* rot);
/**
 * @brief 사원수 로그
 * @param pq 반환 사원수
 * @param q 입력 사원수
*/
QSAPI void qn_quat_ln(QnQuat* pq, const QnQuat* q);
/**
 * @brief 행렬 전치
 * @param pm 반환 행렬
 * @param m 전치할 행렬
*/
QSAPI void qn_mat4_tran(QnMat4* pm, const QnMat4* m);
/**
 * @brief 행렬 곱
 * @param pm 반환 행렬
 * @param left 좌측 행렬
 * @param right 우측 행렬
*/
QSAPI void qn_mat4_mul(QnMat4* pm, const QnMat4* left, const QnMat4* right);
/**
 * @brief 역행렬
 * @param pm 반환 행렬
 * @param m 입력 행렬
 * @param determinant 행렬식
*/
QSAPI void qn_mat4_inv(QnMat4* pm, const QnMat4* m, float* /*NULLABLE*/determinant);
/**
 * @brief 전치곱
 * @param pm 반환 행렬
 * @param left 왼쪽 행렬
 * @param right 오른쪽 행렬
*/
QSAPI void qn_mat4_tmul(QnMat4* pm, const QnMat4* left, const QnMat4* right);
/**
 * @brief 그림자 행렬을 만든다
 * @param pm 반환 행렬
 * @param light 빛의 방향
 * @param plane 투영될 면
*/
QSAPI void qn_mat4_shadow(QnMat4* pm, const QnVec4* light, const QnPlane* plane);
/**
 * @brief 아핀 변환 행렬
 * @param pm 반환 행렬
 * @param scl 스케일
 * @param rotcenter 회전축(원점일 경우 NULL)
 * @param rot 회전
 * @param loc 위치
*/
QSAPI void qn_mat4_affine(QnMat4* pm, const QnVec3* scl, const QnVec3* rotcenter, const QnQuat* rot, const QnVec3* loc);
/**
 * @brief 행렬 트랜스폼
 * @param m 반환 행렬
 * @param loc 위치
 * @param rot 회전
 * @param scl 스케일 (1일 경우 NULL)
*/
QSAPI void qn_mat4_trfm(QnMat4* m, const QnVec3* loc, const QnQuat* rot, const QnVec3* scl);
/**
 * @brief 행렬 트랜스폼. 단 벡터 회전
 * @param m 반환 행렬
 * @param loc 위치
 * @param rot 회전
 * @param scl 스케일 (1일 경우 NULL)
*/
QSAPI void qn_mat4_trfm_vec(QnMat4* m, const QnVec3* loc, const QnVec3* rot, const QnVec3* scl);
/**
 * @brief 행렬식
 * @param m 행렬
 * @return 행렬식
*/
QSAPI float qn_mat4_det(const QnMat4* m);
/**
 * @brief 면 트랜스폼
 * @param pp 반환 면
 * @param plane 대상 면
 * @param trfm 트랜스폼 행렬
*/
QSAPI void qn_plane_trfm(QnPlane* pp, const QnPlane* plane, const QnMat4* trfm);
/**
 * @brief 점 세개로 평면을 만든다
 * @param pp 반환 면
 * @param v1 점1
 * @param v2 점2
 * @param v3 점3
*/
QSAPI void qn_plane_points(QnPlane* pp, const QnVec3* v1, const QnVec3* v2, const QnVec3* v3);
/**
 * @brief 벡터와 면의 충돌 평면을 만든다
 * @param p 반환 면
 * @param loc 시작 벡터
 * @param dir 방향 벡터
 * @param o 대상 평면
 * @return 만들 수 있으면 TRUE
*/
QSAPI bool qn_plane_intersect(const QnPlane* p, QnVec3* loc, QnVec3* dir, const QnPlane* o);
/**
 * @brief 구와 충돌하는 선 판정
 * @param p 처리할 선
 * @param org 구의 중점
 * @param rad 구의 반지름
 * @param dist 충돌 거리
 * @return 충돌하면 true
*/
QSAPI bool qn_line3_intersect_sphere(const QnLine3* p, const QnVec3* org, float rad, float* dist);
QN_EXTC_END

// vector2

/**
 * @brief 벡터2 설정
 * @param pv 벡터2
 * @param x,y 좌표
*/
QN_INLINE void qn_vec2_set(QnVec2* pv, float x, float y)
{
	pv->x = x;
	pv->y = y;
}

/**
 * @brief 사이즈로 벡터2 설정
 * @param pv 벡터2
 * @param size 사이즈
*/
QN_INLINE void qn_vec2_set_size(QnVec2* pv, const QnSize* size)
{
	pv->x = (float)size->width;
	pv->y = (float)size->height;
}

/**
 * @brief 벡터2 정규화
 * @param pv 벡터2
*/
QN_INLINE void qn_vec2_rst(QnVec2* pv)		// identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
}

/**
 * @brief 벡터2 대각 (같은값 설정)
 * @param pv 벡터2
 * @param v 대각 값
*/
QN_INLINE void qn_vec2_diag(QnVec2* pv, float v)
{
	pv->x = v;
	pv->y = v;
}

/**
 * @brief 벡터2 내적
 * @param left 왼쪽 벡터2
 * @param right 오른쪽 벡터2
 * @return 내적 값
*/
QN_INLINE float qn_vec2_dot(const QnVec2* left, const QnVec2* right)
{
	return left->x * right->x + left->y * right->y;
}

/**
 * @brief 벡터2 길이의 제곱
 * @param pv 벡터2
 * @return 길이의 제곱
*/
QN_INLINE float qn_vec2_len_sq(const QnVec2* pv)
{
	return pv->x * pv->x + pv->y * pv->y;
}

/**
 * @brief 벡터2 길이
 * @param pv 벡터2
 * @return 길이
*/
QN_INLINE float qn_vec2_len(const QnVec2* pv)
{
	return sqrtf(qn_vec2_len_sq(pv));
}

/**
 * @brief 벡터2 정규화
 * @param pv 정규 벡터2를 담을 곳
 * @param v 벡터2
*/
QN_INLINE void qn_vec2_norm(QnVec2* pv, const QnVec2* v)
{
	const float f = 1.0f / qn_vec2_len(v);
	pv->x = v->x * f;
	pv->y = v->y * f;
}

/**
 * @brief 벡터2 덧셈
 * @param pv 덧셈 결과를 담을 곳
 * @param left 왼쪽 벡터2
 * @param right 오른쪽 벡터2
*/
QN_INLINE void qn_vec2_add(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
}

/**
 * @brief 벡터2 뺄셈
 * @param pv 뺄셈 결과를 담을 곳
 * @param left 왼쪽 벡터2
 * @param right 오른쪽 벡터2
*/
QN_INLINE void qn_vec2_sub(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
}

/**
 * @brief 벡터2 확대
 * @param pv 확대 결과를 담을 곳
 * @param left 왼쪽 벡터2
 * @param right 오른쪽 확대값
*/
QN_INLINE void qn_vec2_mag(QnVec2* pv, const QnVec2* left, float right)
{
	pv->x = left->x * right;
	pv->y = left->y * right;
}

/**
 * @brief 벡터2의 누적의 배수
 * @param pv 벡터2
 * @param s 배수
 * @return 누적의 배수 (모든 요소의 합에 배수를 곱한값)
*/
QN_INLINE float qn_vec2_accm(const QnVec2* pv, float s)
{
	return (pv->x + pv->y) * s;
}

/**
 * @brief 벡터2의 외적
 * @param pv 외적을 담을 벡터2
 * @param left 왼쪽 벡터2
 * @param right 오른쪽 벡터2
*/
QN_INLINE void qn_vec2_cross(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = left->y * right->x - left->x * right->y;
	pv->y = left->x * right->y - left->y * right->x;
}

/**
 * @brief 벡터2 반전
 * @param pv 반전을 담을 벡터2
 * @param v 벡터2
*/
QN_INLINE void qn_vec2_ivt(QnVec2* pv, const QnVec2* v)  // invert
{
	pv->x = -v->x;
	pv->y = -v->y;
}

/**
 * @brief 벡터2 네거티브 (1 - 벡터2)
 * @param pv 네거티브를 담을 벡터2
 * @param v 벡터2
*/
QN_INLINE void qn_vec2_neg(QnVec2* pv, const QnVec2* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
}

/**
 * @brief 벡터2의 비교
 * @param left 왼쪽 벡터
 * @param right 오른쪽 벡트
 * @return 같으면 참
*/
QN_INLINE bool qn_vec2_eq(const QnVec2* left, const QnVec2* right)
{
	return qn_eqf(left->x, right->x) && qn_eqf(left->y, right->y);
}

/**
 * @brief 벡터2가 0인가 비교
 * @param pv 비교할 벡터2
 * @return 0이면 참
*/
QN_INLINE bool qn_vec2_isi(const QnVec2* pv)
{
	return qn_eqf(pv->x, 0.0f) && qn_eqf(pv->y, 0.0f);
}

/**
 * @brief 벡터2 보간 (원본에서 대상으로 보간)
 * @param pv 결과를 담을 벡터2
 * @param left 원본 벡터2
 * @param right 대상 벡터2
 * @param s 보간값
*/
QN_INLINE void qn_vec2_interpolate(QnVec2* pv, const QnVec2* left, const QnVec2* right, float s)
{
	const float f = 1.0f - s;
	pv->x = right->x * f + left->x * s;
	pv->y = right->y * f + left->y * s;
}

/**
 * @brief 벡터2 선형 보간
 * @param pv 결과를 담을 벡터2
 * @param left 원본 벡터2
 * @param right 대상 벡터2
 * @param s 보간값
*/
QN_INLINE void qn_vec2_lerp(QnVec2* pv, const QnVec2* left, const QnVec2* right, float s)
{
	pv->x = left->x + s * (right->x - left->x);
	pv->y = left->y + s * (right->y - left->y);
}

/**
 * @brief 벡터2의 최소값
 * @param pv 최소값을 담을 벡터2
 * @param left 왼쪽 벡터2
 * @param right 오른쪽 벡터2
*/
QN_INLINE void qn_vec2_min(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = (left->x < right->x) ? left->x : right->x;
	pv->y = (left->y < right->y) ? left->y : right->y;
}

/**
 * @brief 벡터2의 최대값
 * @param pv 최대값을 담을 벡터2
 * @param left 왼쪽 벡터2
 * @param right 오른쪽 벡터2
*/
QN_INLINE void qn_vec2_max(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = (left->x > right->x) ? left->x : right->x;
	pv->y = (left->y > right->y) ? left->y : right->y;
}

// vector3

/**
 * @brief 벡터3 설정
 * @param pv 값을 담을 벡터3
 * @param x,y,z 좌표
*/
QN_INLINE void qn_vec3_set(QnVec3* pv, float x, float y, float z)
{
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

/**
 * @brief 벡터3 초기화
 * @param pv 초기화할 벡터3
*/
QN_INLINE void qn_vec3_rst(QnVec3* pv)		// identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
	pv->z = 0.0f;
}

/**
 * @brief 벡터3 대각값 설정 (모두 같은값으로 설정)
 * @param pv 값을 넣을 벡터3
 * @param v 대각값
*/
QN_INLINE void qn_vec3_diag(QnVec3* pv, float v)
{
	pv->x = v;
	pv->y = v;
	pv->z = v;
}

/**
 * @brief 벡터3의 길이의 제곱
 * @param pv 벡터3
 * @return 길이의 제곱
*/
QN_INLINE float qn_vec3_len_sq(const QnVec3* pv)
{
	return pv->x * pv->x + pv->y * pv->y + pv->z * pv->z;
}

/**
 * @brief 벡터3의 길이
 * @param pv 벡터3
 * @return 길이
*/
QN_INLINE float qn_vec3_len(const QnVec3* pv)
{
	return sqrtf(qn_vec3_len_sq(pv));
}

/**
 * @brief 벡터3 정규화
 * @param pv 정규화된 벡터3
 * @param v 원본 벡터3
*/
QN_INLINE void qn_vec3_norm(QnVec3* pv, const QnVec3* v)
{
	const float f = 1.0f / qn_vec3_len(v);
	pv->x = v->x * f;
	pv->y = v->y * f;
	pv->z = v->z * f;
}

/**
 * @brief 벡터3 덧셈
 * @param pv 덧셈 결과를 담을 벡터3
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
*/
QN_INLINE void qn_vec3_add(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
	pv->z = left->z + right->z;
}

/**
 * @brief 벡터3 뺄셈
 * @param pv 뺄셈 결과를 담을 벡터3
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
*/
QN_INLINE void qn_vec3_sub(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
	pv->z = left->z - right->z;
}

/**
 * @brief 벡터3 확대
 * @param pv 확대 결과를 담을 벡터3
 * @param v 원본 벡터3
 * @param scale 확대값
*/
QN_INLINE void qn_vec3_mag(QnVec3* pv, const QnVec3* v, float scale)
{
	pv->x = v->x * scale;
	pv->y = v->y * scale;
	pv->z = v->z * scale;
}

/**
 * @brief 벡터3 누적의 확대 (모든 요소를 더하고 확대한다)
 * @param pv 원본 벡터3
 * @param scale 확대값
 * @return 누적의 확대 결과값
*/
QN_INLINE float qn_vec3_accm(const QnVec3* pv, float scale)
{
	return (pv->x + pv->y + pv->z) * scale;
}

/**
 * @brief 벡터3의 외적
 * @param pv 외적의 결과를 담을 벡터3
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
*/
QN_INLINE void qn_vec3_cross(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->y * right->z - left->z * right->y;
	pv->y = left->z * right->x - left->x * right->z;
	pv->z = left->x * right->y - left->y * right->x;
}

/**
 * @brief 벡터3 반전
 * @param pv 반전 결과를 담을 벡터3
 * @param v 원본 벡터3
*/
QN_INLINE void qn_vec3_ivt(QnVec3* pv, const QnVec3* v)  // invert
{
	pv->x = -v->x;
	pv->y = -v->y;
	pv->z = -v->z;
}

/**
 * @brief 벡터3 네거티브 (1 - 값)
 * @param pv 네거티브 결과를 담을 벡터3
 * @param v 원본 벡터3
*/
QN_INLINE void qn_vec3_neg(QnVec3* pv, const QnVec3* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
	pv->z = 1.0f - v->z;
}

/**
 * @brief 벡터3의 내적
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
 * @return 내적값
*/
QN_INLINE float qn_vec3_dot(const QnVec3* left, const QnVec3* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z;
}

/**
 * @brief 벡터3의 최소값
 * @param pv 최소값을 담을 벡터3
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
*/
QN_INLINE void qn_vec3_min(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->x < right->x ? left->x : right->x;
	pv->y = left->y < right->y ? left->y : right->y;
	pv->z = left->z < right->z ? left->z : right->z;
}

/**
 * @brief 벡터3의 최대값
 * @param pv 최대값을 담을 벡터3
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
*/
QN_INLINE void qn_vec3_max(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->x > right->x ? left->x : right->x;
	pv->y = left->y > right->y ? left->y : right->y;
	pv->z = left->z > right->z ? left->z : right->z;
}

/**
 * @brief 두 벡터3이 같은지 판단
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
 * @return 두 벡터3이 같으면 참
*/
QN_INLINE bool qn_vec3_eq(const QnVec3* left, const QnVec3* right)
{
	return qn_eqf(left->x, right->x) && qn_eqf(left->y, right->y) && qn_eqf(left->z, right->z);
}

/**
 * @brief 벡터3이 0인지 판단
 * @param pv 벡터3
 * @return 벡터3이 0이면 참
*/
QN_INLINE bool qn_vec3_isi(const QnVec3* pv)
{
	return (pv->x == 0.0f && pv->y == 0.0f && pv->z == 0.0f);
}

/**
 * @brief 두 벡터3 거리의 제곱
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
 * @return 두 벡터3 거리의 제곱값
*/
QN_INLINE float qn_vec3_dist_sq(const QnVec3* left, const QnVec3* right)
{
	QnVec3 t;
	qn_vec3_sub(&t, left, right);
	return qn_vec3_len_sq(&t);
}

/**
 * @brief 두 벡터3의 거리
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
 * @return 두 벡터3의 거리값
*/
QN_INLINE float qn_vec3_dist(const QnVec3* left, const QnVec3* right)
{
	return sqrtf(qn_vec3_dist_sq(left, right));
}

/**
 * @brief 벡터3의 방향
 * @param pv 방향을 담을 벡터3
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
*/
QN_INLINE void qn_vec3_dir(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	qn_vec3_sub(pv, left, right);
	qn_vec3_norm(pv, pv);
}

/**
 * @brief 두 벡터3의 반지름의 제곱
 * @param left 왼쪽(가운데) 벡터3
 * @param right 오른쪽(바깥쪽) 벡터3
 * @return 두 벡터3의 반지름의 제곱값
*/
QN_INLINE float qn_vec3_rad_sq(const QnVec3* left, const QnVec3* right)
{
	QnVec3 t;
	qn_vec3_add(&t, left, right);
	qn_vec3_mag(&t, &t, 0.5f);
	qn_vec3_sub(&t, &t, left);
	return qn_vec3_len_sq(&t);
}

/**
 * @brief 두 벡터3의 반지름
 * @param left 왼쪽(가운데) 벡터3
 * @param right 오른쪽(바깥쪽) 벡터3
 * @return 두 벡터3의 반지름
*/
QN_INLINE float qn_vec3_rad(const QnVec3* left, const QnVec3* right)
{
	return sqrtf(qn_vec3_rad_sq(left, right));
}

/**
 * @brief 벡터3 트랜스폼
 * @param pv 트랜스폼 결과를 담을 벡터3
 * @param v 벡터3
 * @param trfm 변환 행렬
*/
QN_INLINE void qn_vec3_trfm(QnVec3* pv, const QnVec3* v, const QnMat4* trfm)
{
	const float x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31 + trfm->_41;
	const float y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32 + trfm->_42;
	const float z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33 + trfm->_43;
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

/**
 * @brief 벡터3 정규화 트랜스폼
 * @param pv 트랜스폼 결과를 담을 벡터3
 * @param v 벡터3
 * @param trfm 정규화된 변환 행렬
*/
QN_INLINE void qn_vec3_trfm_norm(QnVec3* pv, const QnVec3* v, const QnMat4* trfm)
{
	const float x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31;
	const float y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32;
	const float z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33;
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

/**
 * @brief 벡터3 사원수 회전
 * @param pv 회전 값을 담을 벡터3
 * @param rot 사원수
*/
QN_INLINE void qn_vec3_quat(QnVec3* pv, const QnQuat* rot)
{
	const float x = rot->x * rot->x;
	const float y = rot->y * rot->y;
	const float z = rot->z * rot->z;
	const float w = rot->w * rot->w;
	pv->x = atan2f(2.0f * (rot->y * rot->z + rot->x * rot->w), -x - y + z + w);
	pv->y = asinf(qn_clampf(-2.0f * (rot->x * rot->z + rot->y * rot->w), -1.0f, 1.0f));
	pv->z = atan2f(2.0f * (rot->x * rot->y + rot->z * rot->w), x - y - z + w);
}

/**
 * @brief 벡터3 행렬 회전
 * @param pv 회전 값을 담을 벡터3
 * @param rot 행렬
*/
QN_INLINE void qn_vec3_mat4(QnVec3* pv, const QnMat4* rot)
{
	if (rot->_31 == 0.0f && rot->_33 == 0.0f)
	{
		pv->x = rot->_32 > 0.0f ? (float)(QN_PI_H + QN_PI) : (float)QN_PI_H;
		pv->z = atan2f(rot->_21, rot->_23);
		pv->y = 0.0f;
	}
	else
	{
		pv->y = -atan2f(rot->_31, rot->_33);
		pv->x = -atan2f(rot->_32, sqrtf(rot->_31 * rot->_31 + rot->_33 * rot->_33));
		pv->z = atan2f(rot->_12, sqrtf(rot->_11 * rot->_11 + rot->_13 * rot->_13));
	}
}

/**
 * @brief 사원수와 벡터3 각도로 회전
 * @param pv 회전값을 담을 벡터3
 * @param rot 사원수
 * @param angle 각도 벡터3
*/
QN_INLINE void qn_vec3_quat_vec3(QnVec3* pv, const QnQuat* rot, const QnVec3* angle)
{
	QnQuat q1 =
	{
		.x = angle->x * rot->w,
		.y = angle->y * rot->w,
		.z = angle->z * rot->w,
		.w = -qn_vec3_dot((const QnVec3*)rot, angle),
	};

	QnVec3 t;
	qn_vec3_cross(&t, (const QnVec3*)rot, angle);
	q1.x += t.x;
	q1.y += t.y;
	q1.z += t.z;

	const QnQuat q2 =
	{
		.x = -rot->x,
		.y = -rot->y,
		.z = -rot->z,
		.w = rot->w,
	};

	pv->x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
	pv->y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
	pv->z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
}

/**
 * @brief 벡터3 보간
 * @param pv 보간 결과를 담을 벡터3
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
 * @param scale 보간 수치
*/
QN_INLINE void qn_vec3_interpolate(QnVec3* pv, const QnVec3* left, const QnVec3* right, float scale)
{
	const float f = 1.0f - scale;
	pv->x = right->x * f + left->x * scale;
	pv->y = right->y * f + left->y * scale;
	pv->z = right->z * f + left->z * scale;
}

/**
 * @brief 벡터3 선형 보간
 * @param pv 보간 결과를 담을 벡터3
 * @param left 왼쪽 벡터3
 * @param right 오른쪽 벡터3
 * @param scale 보간 수치
*/
QN_INLINE void qn_vec3_lerp(QnVec3* pv, const QnVec3* left, const QnVec3* right, float scale)
{
	pv->x = left->x + scale * (right->x - left->x);
	pv->y = left->y + scale * (right->y - left->y);
	pv->z = left->z + scale * (right->z - left->z);
}

/**
 * @brief 축 회전(yaw)
 * @param pv 벡터3
 * @return 축 회전 각도
*/
QN_INLINE float qn_vec3_yaw(const QnVec3* pv)
{
	return -atanf(pv->z / pv->x) + ((pv->x > 0.0f) ? (float)-QN_PI_H : (float)QN_PI_H);
}

/**
 * @brief 두 벡터3 사이에 벡터3이 있는지 조사
 * @param p 조사할 벡터3
 * @param begin 선분의 시작점 벡터3
 * @param end 선분이 끝점 벡터3
 * @return 벡터 사이에 있었다면 참
*/
QN_INLINE bool qn_vec3_between(const QnVec3* p, const QnVec3* begin, const QnVec3* end)
{
	QnVec3 t;
	qn_vec3_sub(&t, end, begin);
	const float f = qn_vec3_len_sq(&t);
	return qn_vec3_dist_sq(p, begin) <= f && qn_vec3_dist_sq(p, end) <= f;
}

// vector4

/**
 * @brief 벡터4 값 설정
 * @param pv 결과를 담을 벡터4
 * @param x,y,z,w 벡터4 요소
*/
QN_INLINE void qn_vec4_set(QnVec4* pv, float x, float y, float z, float w)
{
	pv->x = x;
	pv->y = y;
	pv->z = z;
	pv->w = w;
}

/**
 * @brief 벡터4 초기화 (0으로 만든ㄷ)
 * @param pv 초기화할 벡터4
*/
QN_INLINE void qn_vec4_rst(QnVec4* pv)		// identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
	pv->z = 0.0f;
	pv->w = 0.0f;
}

/**
 * @brief 벡터4 대각값 설정 (모든 요소를 같은 값을)
 * @param pv 대상 벡터4
 * @param v 대각값
*/
QN_INLINE void qn_vec4_diag(QnVec4* pv, float v)
{
	pv->x = v;
	pv->y = v;
	pv->z = v;
	pv->w = v;
}

/**
 * @brief 벡터4 거리의 제곱
 * @param pv 대상 벡터4
 * @return 벡터4 거리의 제곱값
*/
QN_INLINE float qn_vec4_len_sq(const QnVec4* pv)
{
	return pv->x * pv->x + pv->y * pv->y + pv->z * pv->z + pv->w * pv->w;
}

/**
 * @brief 벡터4 거리
 * @param pv 대상 벡터4
 * @return 벡터4 거리값
*/
QN_INLINE float qn_vec4_len(const QnVec4* pv)
{
	return sqrtf(qn_vec4_len_sq(pv));
}

/**
 * @brief 벡터4 정규화
 * @param pv 정규화 결과를 담을 벡터4
 * @param v 원본 벡터4
*/
QN_INLINE void qn_vec4_norm(QnVec4* pv, const QnVec4* v)
{
	const float f = 1.0f / qn_vec4_len(v);
	pv->x = v->x * f;
	pv->y = v->y * f;
	pv->z = v->z * f;
	pv->w = v->w * f;
}

/**
 * @brief 벡터4 덧셈
 * @param pv 덧셈 결과를 담을 벡터4
 * @param left 왼쪽 벡터4
 * @param right 오른쪽 벡터4
*/
QN_INLINE void qn_vec4_add(QnVec4* pv, const QnVec4* left, const QnVec4* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
	pv->z = left->z + right->z;
	pv->w = left->w + right->w;
}

/**
 * @brief 벡터4 뺄셈
 * @param pv 뺄셈 결과를 담을 벡터4
 * @param left 왼쪽 벡터4
 * @param right 오른쪽 벡터4
*/
QN_INLINE void qn_vec4_sub(QnVec4* pv, const QnVec4* left, const QnVec4* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
	pv->z = left->z - right->z;
	pv->w = left->w - right->w;
}

/**
 * @brief 벡터4 확대
 * @param pv 확대 결과를 담을 벡터4
 * @param v 원본 벡터4
 * @param scale 확대값
*/
QN_INLINE void qn_vec4_mag(QnVec4* pv, const QnVec4* v, float scale)
{
	pv->x = v->x * scale;
	pv->y = v->y * scale;
	pv->z = v->z * scale;
	pv->w = v->w * scale;
}

/**
 * @brief 벡터4 누적곱 (모든 요소를 더하고 확대)
 * @param pv 대상 벡터4
 * @param scale 확대값
 * @return 벡터4 누적곱값
*/
QN_INLINE float qn_vec4_accm(const QnVec4* pv, float scale)
{
	return (pv->x + pv->y + pv->z * pv->w) * scale;
}

/**
 * @brief 벡터4 반전
 * @param pv 반전 결과를 담을 벡터4
 * @param v 원본 벡터4
*/
QN_INLINE void qn_vec4_ivt(QnVec4* pv, const QnVec4* v)
{
	pv->x = -v->x;
	pv->y = -v->y;
	pv->z = -v->z;
	pv->w = -v->w;
}

/**
 * @brief 벡터4 네거티브
 * @param pv 네거티브 결과를 담을 벡터4
 * @param v 원본 벡터4
*/
QN_INLINE void qn_vec4_neg(QnVec4* pv, const QnVec4* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
	pv->z = 1.0f - v->z;
	pv->w = 1.0f - v->w;
}

/**
 * @brief 벡터4 내적
 * @param left 왼쪽 벡터4
 * @param right 오른쪽 벡터4
 * @return 내적값
*/
QN_INLINE float qn_vec4_dot(const QnVec4* left, const QnVec4* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z + left->w * right->w;
}

/**
 * @brief 벡터4 외적
 * @param pv 외적 결과를 담을 벡터4
 * @param v1 첫번째 벡터4
 * @param v2 두번째 벡터4
 * @param v3 세번째 벡터4
*/
QN_INLINE void qn_vec4_cross(QnVec4* pv, const QnVec4* v1, const QnVec4* v2, const QnVec4* v3)
{
	pv->x = v1->y * (v2->z * v3->w - v3->z * v2->w) - v1->z * (v2->y * v3->w - v3->y * v2->w) + v1->w * (v2->y * v3->z - v2->z * v3->y);
	pv->y = -(v1->x * (v2->z * v3->w - v3->z * v2->w) - v1->z * (v2->x * v3->w - v3->x * v2->w) + v1->w * (v2->x * v3->z - v3->x * v2->z));
	pv->z = v1->x * (v2->y * v3->w - v3->y * v2->w) - v1->y * (v2->x * v3->w - v3->x * v2->w) + v1->w * (v2->x * v3->y - v3->x * v2->y);
	pv->w = -(v1->x * (v2->y * v3->z - v3->y * v2->z) - v1->y * (v2->x * v3->z - v3->x * v2->z) + v1->z * (v2->x * v3->y - v3->x * v2->y));
}

/**
 * @brief 벡터4의 최소값
 * @param pv 최소값을 담을 벡터4
 * @param left 왼쪽 벡터4
 * @param right 오른쪽 벡터4
*/
QN_INLINE void qn_vec4_min(QnVec4* pv, const QnVec4* left, const QnVec4* right)
{
	pv->x = (left->x < right->x) ? left->x : right->x;
	pv->y = (left->y < right->y) ? left->y : right->y;
	pv->z = (left->z < right->z) ? left->z : right->z;
	pv->w = (left->w < right->w) ? left->w : right->w;
}

/**
 * @brief 벡터4의 최대값
 * @param pv 최대값을 담을 벡터4
 * @param left 왼쪽 벡터4
 * @param right 오른쪽 벡터4
*/
QN_INLINE void qn_vec4_max(QnVec4* pv, const QnVec4* left, const QnVec4* right)
{
	pv->x = (left->x > right->x) ? left->x : right->x;
	pv->y = (left->y > right->y) ? left->y : right->y;
	pv->z = (left->z > right->z) ? left->z : right->z;
	pv->w = (left->w > right->w) ? left->w : right->w;
}

/**
 * @brief 두 벡터4를 비교
 * @param left 왼쪽 벡터4
 * @param right 오른쪽 벡터4
 * @return 두 벡터4가 같으면 참
*/
QN_INLINE bool qn_vec4_eq(const QnVec4* left, const QnVec4* right)
{
	return
		qn_eqf(left->x, right->x) &&
		qn_eqf(left->y, right->y) &&
		qn_eqf(left->z, right->z) &&
		qn_eqf(left->w, right->w);
}

/**
 * @brief 벡터가 0인지 비교
 * @param pv 비교할 벡터4
 * @return 벡터4가 0이면 참
*/
QN_INLINE bool qn_vec4_isi(const QnVec4* pv)
{
	return (pv->x == 0.0f && pv->y == 0.0f && pv->z == 0.0f && pv->w == 0.0f);
}

/**
 * @brief 벡터4 트랜스폼
 * @param pv 변환 결과를 담을 벡터4
 * @param v 원본 벡터4
 * @param trfm 변환 행렬
*/
QN_INLINE void qn_vec4_trfm(QnVec4* pv, const QnVec4* v, const QnMat4* trfm)
{
	pv->x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31 + v->w * trfm->_41;
	pv->y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32 + v->w * trfm->_42;
	pv->z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33 + v->w * trfm->_43;
	pv->w = v->x * trfm->_14 + v->y * trfm->_24 + v->z * trfm->_34 + v->w * trfm->_44;
}

/**
 * @brief 벡터4 보간
 * @param pv 보간 결과를 담을 벡터4
 * @param left 왼쪽 벡터4
 * @param right 오른쪽 벡터4
 * @param scale 보간값
*/
QN_INLINE void qn_vec4_interpolate(QnVec4* pv, const QnVec4* left, const QnVec4* right, float scale)
{
	const float f = 1.0f - scale;
	pv->x = right->x * f + left->x * scale;
	pv->y = right->y * f + left->y * scale;
	pv->z = right->z * f + left->z * scale;
	pv->w = right->w * f + left->w * scale;
}

/**
 * @brief 벡터4 선형 보간
 * @param pv 보간 결과를 담을 벡터4
 * @param left 왼쪽 벡터4
 * @param right 오른쪽 벡터4
 * @param scale 보간값
*/
QN_INLINE void qn_vec4_lerp(QnVec4* pv, const QnVec4* left, const QnVec4* right, float scale)
{
	pv->x = left->x + scale * (right->x - left->x);
	pv->y = left->y + scale * (right->y - left->y);
	pv->z = left->z + scale * (right->z - left->z);
	pv->w = left->w + scale * (right->w - left->w);
}

// quaternion

/**
 * @brief 사원수 설정
 * @param pq 설정할 사원수
 * @param x,y,z,w 사원수 요소
*/
QN_INLINE void qn_quat_set(QnQuat* pq, float x, float y, float z, float w)
{
	pq->x = x;
	pq->y = y;
	pq->z = z;
	pq->w = w;
}

/**
 * @brief 사원수를 0으로 초기화
 * @param pq 초기화할 사원수
*/
QN_INLINE void qn_quat_zero(QnQuat* pq)
{
	pq->x = 0.0f;
	pq->y = 0.0f;
	pq->z = 0.0f;
	pq->w = 0.0f;
}

/**
 * @brief 사원수를 리셋 (단위 사원수)
 * @param pq 리셋할 사원수
*/
QN_INLINE void qn_quat_rst(QnQuat* pq)		// identify
{
	pq->x = 0.0f;
	pq->y = 0.0f;
	pq->z = 0.0f;
	pq->w = 1.0f;
}

/**
 * @brief 사원수 길이의 제곱
 * @param pq 대상 사원수
 * @return 사원수 길이의 제곱값
*/
QN_INLINE float qn_quat_len_sq(const QnQuat* pq)
{
	return pq->x * pq->x + pq->y * pq->y + pq->z * pq->z + pq->w * pq->w;
}

/**
 * @brief 사원수 길이
 * @param pq 대상 사원수
 * @return 사원수 길이
*/
QN_INLINE float qn_quat_len(const QnQuat* pq)
{
	return sqrtf(qn_quat_len_sq(pq));
}

/**
 * @brief 사원수 정규화
 * @param pq 정규화 결과를 담을 사원수
 * @param q 대상 사원수
*/
QN_INLINE void qn_quat_norm(QnQuat* pq, const QnQuat* q)
{
	const float f = 1.0f / qn_quat_len(q);
	pq->x = q->x * f;
	pq->y = q->y * f;
	pq->z = q->z * f;
	pq->w = q->w * f;
}

/**
 * @brief 두 사원수의 덧셈
 * @param pq 덧셈 결과를 담을 사원수
 * @param left 왼쪽 사원수
 * @param right 오른쪽 사원수
*/
QN_INLINE void qn_quat_add(QnQuat* pq, const QnQuat* left, const QnQuat* right)
{
	pq->x = left->x + right->x;
	pq->y = left->y + right->y;
	pq->z = left->z + right->z;
	pq->w = left->w + right->w;
}

/**
 * @brief 두 사원수의 뺄셈
 * @param pq 뺄셈 결과를 담을 사원수
 * @param left 왼쪽 사원수
 * @param right 오른쪽 사원수
 * @return
*/
QN_INLINE void qn_quat_sub(QnQuat* pq, const QnQuat* left, const QnQuat* right)
{
	pq->x = left->x - right->x;
	pq->y = left->y - right->y;
	pq->z = left->z - right->z;
	pq->w = left->w - right->w;
}

/**
 * @brief 사원수 확대
 * @param pq 확대 결과를 담을 사원수
 * @param left 원본 사원수
 * @param right 확대값
*/
QN_INLINE void qn_quat_mag(QnQuat* pq, const QnQuat* left, float right)
{
	pq->x = left->x * right;
	pq->y = left->y * right;
	pq->z = left->z * right;
	pq->w = left->w * right;
}

/**
 * @brief 사원수 반전
 * @param pq 반전 결과를 담을 사원수
 * @param v 원본 사원수
*/
QN_INLINE void qn_quat_ivt(QnQuat* pq, const QnQuat* v)
{
	pq->x = -v->x;
	pq->y = -v->y;
	pq->z = -v->z;
	pq->w = -v->w;
}

/**
 * @brief 사원수 네거티브
 * @param pq 네거티브 결과를 담을 사원수
 * @param v 원본 사원수
*/
QN_INLINE void qn_quat_neg(QnQuat* pq, const QnQuat* v)
{
	pq->x = 1.0f - v->x;
	pq->y = 1.0f - v->y;
	pq->z = 1.0f - v->z;
	pq->w = 1.0f - v->w;
}

/**
 * @brief 켤레 사원수
 * @param pq 켤레 사원수를 담을 사원수
 * @param q 원본 사원수
*/
QN_INLINE void qn_quat_cjg(QnQuat* pq, const QnQuat* q)	// conjugate
{
	pq->x = -q->x;
	pq->y = -q->y;
	pq->z = -q->z;
	pq->w = q->w;
}

/**
 * @brief 사원수의 내적
 * @param left 왼쪽 사원수
 * @param right 오른쪽 사원수
 * @return
*/
QN_INLINE float qn_quat_dot(const QnQuat* left, const QnQuat* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z + left->w * right->w;
}

/**
 * @brief 사원수의 외적
 * @param pq 외적 결과를 담을 사원수
 * @param q1 첫번째 사원수
 * @param q2 두번째 사원수
 * @param q3 세번째 사원수
*/
QN_INLINE void qn_quat_cross(QnQuat* pq, const QnQuat* q1, const QnQuat* q2, const QnQuat* q3)
{
	pq->x = q1->y * (q2->z * q3->w - q3->z * q2->w) - q1->z * (q2->y * q3->w - q3->y * q2->w) + q1->w * (q2->y * q3->z - q2->z * q3->y);
	pq->y = -(q1->x * (q2->z * q3->w - q3->z * q2->w) - q1->z * (q2->x * q3->w - q3->x * q2->w) + q1->w * (q2->x * q3->z - q3->x * q2->z));
	pq->z = q1->x * (q2->y * q3->w - q3->y * q2->w) - q1->y * (q2->x * q3->w - q3->x * q2->w) + q1->w * (q2->x * q3->y - q3->x * q2->y);
	pq->w = -(q1->x * (q2->y * q3->z - q3->y * q2->z) - q1->y * (q2->x * q3->z - q3->x * q2->z) + q1->z * (q2->x * q3->y - q3->x * q2->y));
}

/**
 * @brief 두 사원수의 비교
 * @param left 왼쪽 사원수
 * @param right 오른쪽 사원수
 * @return 두 사원수가 같으면 참
*/
QN_INLINE bool qn_quat_eq(const QnQuat* left, const QnQuat* right)
{
	return
		qn_eqf(left->x, right->x) &&
		qn_eqf(left->y, right->y) &&
		qn_eqf(left->z, right->z) &&
		qn_eqf(left->w, right->w);
}

/**
 * @brief 사원수가 단위 사원수 인지 비교
 * @param pq 대상 사원수
 * @return 단위 사원수라면 참
*/
QN_INLINE bool qn_quat_isi(const QnQuat* pq)
{
	return pq->x == 0.0f && pq->y == 0.0f && pq->z == 0.0f && pq->w == 1.0f;
}

/**
 * @brief 두 사원수의 곱셈
 * @param pq 곱셈 결과를 담을 사원수
 * @param left 왼쪽 사원수
 * @param right 오른쪽 사원수
*/
QN_INLINE void qn_quat_mul(QnQuat* pq, const QnQuat* left, const QnQuat* right)
{
	pq->x = left->x * right->w + left->y * right->z - left->z * right->y + left->w * right->x;
	pq->y = -left->x * right->z + left->y * right->w + left->z * right->x + left->w * right->y;
	pq->z = left->x * right->y - left->y * right->x + left->z * right->w + left->w * right->z;
	pq->w = -left->x * right->x - left->y * right->y - left->z * right->z + left->w * right->w;
}

/**
 * @brief 사원수를 X축 회전시킨다
 * @param pq 결과를 담을 사원수
 * @param rot_x X축 회전값
*/
QN_INLINE void qn_quat_x(QnQuat* pq, float rot_x)
{
	pq->y = pq->z = 0.0f;
	qn_sincosf(rot_x * 0.5f, &pq->x, &pq->w);
}

/**
 * @brief 사원수를 Y축 회전시킨다
 * @param pq 결과를 담을 사원수
 * @param rot_y Y축 회전값
*/
QN_INLINE void qn_quat_y(QnQuat* pq, float rot_y)
{
	pq->x = pq->z = 0.0f;
	qn_sincosf(rot_y * 0.5f, &pq->y, &pq->w);
}

/**
 * @brief 사원수를 Z축 회전시킨다
 * @param pq 결과를 담을 사원수
 * @param rot_z Z축 회전값
*/
QN_INLINE void qn_quat_z(QnQuat* pq, float rot_z)
{
	pq->x = pq->y = 0.0f;
	qn_sincosf(rot_z * 0.5f, &pq->z, &pq->w);
}

/**
 * @brief 사원수를 벡터3 축으로 회전시킨다
 * @param pq 결과를 담을 사원수
 * @param v 벡터3 회전축
 * @param angle 회전값
*/
QN_INLINE void qn_quat_axis_vec(QnQuat* pq, const QnVec3* v, float angle)
{
	float s, c;
	qn_sincosf(angle * 0.5f, &s, &c);
	qn_quat_set(pq, v->x * s, v->y * s, v->z * s, c);
}

/**
 * @brief 지수 사원수 값을 얻는다
 * @param pq 자수 사원수 결과를 담을 사원수
 * @param q 원본 사원수
*/
QN_INLINE void qn_quat_exp(QnQuat* pq, const QnQuat* q)
{
	float n = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z);
	if (n != 0.0)
	{
		float sn, cn;
		qn_sincosf(n, &sn, &cn);
		n = 1.0f / n;
		pq->x = sn * q->x * n;
		pq->y = sn * q->y * n;
		pq->z = sn * q->z * n;
		pq->w = cn;
	}
	else
	{
		pq->x = 0.0f;
		pq->y = 0.0f;
		pq->z = 0.0f;
		pq->w = 1.0f;
	}
}

/**
 * @brief 역사원수를 얻는다
 * @param pq 역사원수를 얻을 사원수
 * @param q 원본 사원수
*/
QN_INLINE void qn_quat_inv(QnQuat* pq, const QnQuat* q)
{
	float n = qn_quat_len_sq(q);
	if (n == 0.0f)
	{
		pq->x = 0.0f;
		pq->y = 0.0f;
		pq->z = 0.0f;
		pq->w = 0.0f;
	}
	else
	{
		QnQuat t;
		qn_quat_cjg(&t, q);
		n = 1.0f / n;
		pq->x = t.x * n;
		pq->y = t.y * n;
		pq->z = t.z * n;
		pq->w = t.w * n;
	}
}

/**
 * @brief 두 사원수 보간한다
 * @param pq 보간 값을 담을 사원수
 * @param left 왼쪽 사원수
 * @param right 오른쪽 사원수
 * @param scale 보간값
*/
QN_INLINE void qn_quat_interpolate(QnQuat* pq, const QnQuat* left, const QnQuat* right, float scale)
{
	const float f = 1.0f - scale;
	pq->x = right->x * f + left->x * scale;
	pq->y = right->y * f + left->y * scale;
	pq->z = right->z * f + left->z * scale;
	pq->w = right->w * f + left->w * scale;
}

/**
 * @brief 두 사원수를 선형 보간한다
 * @param pq 보간 값을 담을 사원수
 * @param left 왼쪽 사원수
 * @param right 오른쪽 사원수
 * @param scale 보간값
 * @see qn_quat_slerp
*/
QN_INLINE void qn_quat_lerp(QnQuat* pq, const QnQuat* left, const QnQuat* right, float scale)
{
	pq->x = left->x + scale * (right->x - left->x);
	pq->y = left->y + scale * (right->y - left->y);
	pq->z = left->z + scale * (right->z - left->z);
	pq->w = left->w + scale * (right->w - left->w);
}

// matrix4

/**
 * @brief 행렬을 0으로 초기화 한다
 * @param pm 초기화할 대상 행렬
*/
QN_INLINE void qn_mat4_zero(QnMat4* pm)
{
	pm->_11 = pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_22 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_33 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = pm->_44 = 0.0f;
}

/**
 * @brief 단위 행렬을 만든다
 * @param pm 단위 행렬로 만들 행렬
*/
QN_INLINE void qn_mat4_rst(QnMat4* pm)		// identify
{
	pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = 0.0f;
	pm->_11 = pm->_22 = pm->_33 = pm->_44 = 1.0f;
}

/**
 * @brief 대각 행렬을 만든다
 * @param pm 대각 행렬로 만들 행렬
 * @param v 대각값
*/
QN_INLINE void qn_mat4_diag(QnMat4* pm, float v)
{
	pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = 0.0f;
	pm->_11 = pm->_22 = pm->_33 = pm->_44 = v;
}

/**
 * @brief 두 행렬의 덧셈
 * @param pm 덧셈 결과를 담을 행렬
 * @param left 왼쪽 행렬
 * @param right 오른쪽 행렬
*/
QN_INLINE void qn_mat4_add(QnMat4* pm, const QnMat4* left, const QnMat4* right)
{
	pm->_11 = left->_11 + right->_11;
	pm->_12 = left->_12 + right->_12;
	pm->_13 = left->_13 + right->_13;
	pm->_14 = left->_14 + right->_14;
	pm->_21 = left->_21 + right->_21;
	pm->_22 = left->_22 + right->_22;
	pm->_23 = left->_23 + right->_23;
	pm->_24 = left->_24 + right->_24;
	pm->_31 = left->_31 + right->_31;
	pm->_32 = left->_32 + right->_32;
	pm->_33 = left->_33 + right->_33;
	pm->_34 = left->_34 + right->_34;
	pm->_41 = left->_41 + right->_41;
	pm->_42 = left->_42 + right->_42;
	pm->_43 = left->_43 + right->_43;
	pm->_44 = left->_44 + right->_44;
}

/**
 * @brief 두 행렬의 뺄셈
 * @param pm 뺄셈 결과를 담을 행렬
 * @param left 왼쪽 행렬
 * @param right 오른쪽 행렬
*/
QN_INLINE void qn_mat4_sub(QnMat4* pm, const QnMat4* left, const QnMat4* right)
{
	pm->_11 = left->_11 - right->_11;
	pm->_12 = left->_12 - right->_12;
	pm->_13 = left->_13 - right->_13;
	pm->_14 = left->_14 - right->_14;
	pm->_21 = left->_21 - right->_21;
	pm->_22 = left->_22 - right->_22;
	pm->_23 = left->_23 - right->_23;
	pm->_24 = left->_24 - right->_24;
	pm->_31 = left->_31 - right->_31;
	pm->_32 = left->_32 - right->_32;
	pm->_33 = left->_33 - right->_33;
	pm->_34 = left->_34 - right->_34;
	pm->_41 = left->_41 - right->_41;
	pm->_42 = left->_42 - right->_42;
	pm->_43 = left->_43 - right->_43;
	pm->_44 = left->_44 - right->_44;
}

/**
 * @brief 행렬의 확대
 * @param pm 확대 결과를 담을 행렬
 * @param left 대상 행렬
 * @param right 확대값
*/
QN_INLINE void qn_mat4_mag(QnMat4* pm, const QnMat4* left, float right)
{
	pm->_11 = left->_11 * right;
	pm->_12 = left->_12 * right;
	pm->_13 = left->_13 * right;
	pm->_14 = left->_14 * right;
	pm->_21 = left->_21 * right;
	pm->_22 = left->_22 * right;
	pm->_23 = left->_23 * right;
	pm->_24 = left->_24 * right;
	pm->_31 = left->_31 * right;
	pm->_32 = left->_32 * right;
	pm->_33 = left->_33 * right;
	pm->_34 = left->_34 * right;
	pm->_41 = left->_41 * right;
	pm->_42 = left->_42 * right;
	pm->_43 = left->_43 * right;
	pm->_44 = left->_44 * right;
}

/**
 * @brief 위치 행렬을 만든다
 * @param pm 위치 행렬을 담을 행렬
 * @param x,y,z 좌표
 * @param reset 이 값이 참이면 회전과 스케일을 단위 행렬로 만든다
 * @note 행렬에서 좌표만 변경하려면 reset 값을 거짓으로 보낸다
*/
QN_INLINE void qn_mat4_loc(QnMat4* pm, float x, float y, float z, bool reset)
{
	if (reset)
	{
		pm->_12 = pm->_13 = pm->_14 = 0.0f;
		pm->_21 = pm->_23 = pm->_24 = 0.0f;
		pm->_31 = pm->_32 = pm->_34 = 0.0f;
		pm->_11 = pm->_22 = pm->_33 = pm->_44 = 1.0f;
	}
	pm->_41 = x;
	pm->_42 = y;
	pm->_43 = z;
}

/**
 * @brief 스케일 행렬을 만든다
 * @param pm 스케일 행렬을 담을 행렬
 * @param x,y,z 각 축 별 스케일 값
 * @param reset 이 값이 참이면 회전과 좌표를 단위 행렬로 만든다
 * @note reset 을 거짓으로 해서 간단히 계산할 수 있지만 회전값이 있는 행렬의 경우 제대로 연산되지 않는다
*/
QN_INLINE void qn_mat4_scl(QnMat4* pm, float x, float y, float z, bool reset)
{
	if (reset)
	{
		pm->_12 = pm->_13 = pm->_14 = 0.0f;
		pm->_21 = pm->_23 = pm->_24 = 0.0f;
		pm->_31 = pm->_32 = pm->_34 = 0.0f;
		pm->_41 = pm->_42 = pm->_43 = 0.0f;
	}
	pm->_11 = x;
	pm->_22 = y;
	pm->_33 = z;
	pm->_44 = 1.0f;
}

/**
 * @brief 행렬의 좌표를 주어진 수치만큼 이동한다 (수치는 덧셈으로 좌표가 아니다)
 * @param pm 이동할 행렬
 * @param x,y,z 이동할 수치
*/
QN_INLINE void qn_mat4_move(QnMat4* pm, float x, float y, float z)
{
	pm->_41 += x;
	pm->_42 += y;
	pm->_43 += z;
}

/**
 * @brief 두 행렬이 같은지 비교
 * @param left 왼쪽 행렬
 * @param right 오른쪽 행렬
 * @return 두 행렬이 같으면 참. 다만 실수 비교이므로 틀릴 수 있다
*/
QN_INLINE bool qn_mat4_eq(QnMat4 const* left, QnMat4 const* right)
{
	return memcmp((void const*)left, (void const*)right, sizeof(QnMat4)) == 0;
}

/**
 * @brief 단위 행렬인지 비교
 * @param pm 비교할 행렬
 * @return 단위 행렬이면 참을 반환
*/
QN_INLINE bool qn_mat4_isi(const QnMat4* pm)
{
	return
		pm->_11 == 1.0f && pm->_12 == 0.0f && pm->_13 == 0.0f && pm->_14 == 0.0f &&
		pm->_21 == 0.0f && pm->_22 == 1.0f && pm->_23 == 0.0f && pm->_24 == 0.0f &&
		pm->_31 == 0.0f && pm->_32 == 0.0f && pm->_33 == 1.0f && pm->_34 == 0.0f &&
		pm->_41 == 0.0f && pm->_42 == 0.0f && pm->_43 == 0.0f && pm->_44 == 1.0f;
}

/**
 * @brief 변환 행렬을 만든다
 * @param pm 변환 행렬을 담을 행렬
 * @param rot 벡터3 회전값
 * @param loc 위치값. NULL 값으로 무시할 수 있다
*/
QN_INLINE void qn_mat4_vec(QnMat4* pm, const QnVec3* rot, const QnVec3* /*NULLABLE*/loc)
{
	float sr, sp, sy;
	float cr, cp, cy;

	qn_sincosf(rot->x, &sr, &cr);
	qn_sincosf(rot->y, &sp, &cp);
	qn_sincosf(rot->z, &sy, &cy);
	const float srsp = sr * sp;
	const float crsp = cr * sp;

	pm->_11 = cp * cy;
	pm->_12 = cp * sy;
	pm->_13 = -sp;
	pm->_14 = 0.0f;

	pm->_21 = srsp * cy - cr * sy;
	pm->_22 = srsp * sy + cr * cy;
	pm->_23 = sr * cp;
	pm->_24 = 0.0f;

	pm->_31 = crsp * cy + sr * sy;
	pm->_32 = crsp * sy - sr * cy;
	pm->_33 = cr * cp;
	pm->_34 = 0.0f;

	if (loc)
	{
		pm->_41 = loc->x;
		pm->_42 = loc->y;
		pm->_43 = loc->z;
	}
	else
	{
		pm->_41 = 0.0f;
		pm->_42 = 0.0f;
		pm->_43 = 0.0f;
	}
	pm->_44 = 1.0f;
}

/**
 * @brief 사원수로 변환 행렬을 만든다
 * @param pm 변환 행렬을 담을 행렬
 * @param rot 사원수 회전값
 * @param loc 위치값. NULL 값으로 무시할 수 있다
 * @return
*/
QN_INLINE void qn_mat4_quat(QnMat4* pm, const QnQuat* rot, const QnVec3* /*NULLABLE*/loc)
{
	pm->_11 = 1.0f - 2.0f * rot->y * rot->y - 2.0f * rot->z * rot->z;
	pm->_12 = 2.0f * rot->x * rot->y + 2.0f * rot->z * rot->w;
	pm->_13 = 2.0f * rot->x * rot->z - 2.0f * rot->y * rot->w;
	pm->_14 = 0.0f;

	pm->_21 = 2.0f * rot->x * rot->y - 2.0f * rot->z * rot->w;
	pm->_22 = 1.0f - 2.0f * rot->x * rot->x - 2.0f * rot->z * rot->z;
	pm->_23 = 2.0f * rot->z * rot->y + 2.0f * rot->x * rot->w;
	pm->_24 = 0.0f;

	pm->_31 = 2.0f * rot->x * rot->z + 2.0f * rot->y * rot->w;
	pm->_32 = 2.0f * rot->z * rot->y - 2.0f * rot->x * rot->w;
	pm->_33 = 1.0f - 2.0f * rot->x * rot->x - 2.0f * rot->y * rot->y;
	pm->_34 = 0.0f;

	if (loc)
	{
		pm->_41 = loc->x;
		pm->_42 = loc->y;
		pm->_43 = loc->z;
	}
	else
	{
		pm->_41 = 0.0f;
		pm->_42 = 0.0f;
		pm->_43 = 0.0f;
	}
	pm->_44 = 1.0f;
}

/**
 * @brief X축 회전 행렬을 만든다
 * @param pm 회전 행렬을 담을 행렬
 * @param rot X측 회전값
*/
QN_INLINE void qn_mat4_x(QnMat4* pm, float rot)
{
	float vsin, vcos;
	qn_sincosf(rot, &vsin, &vcos);
	pm->_11 = 1.0f; pm->_12 = 0.0f; pm->_13 = 0.0f; pm->_14 = 0.0f;
	pm->_21 = 0.0f; pm->_22 = vcos; pm->_23 = vsin; pm->_24 = 0.0f;
	pm->_31 = 0.0f; pm->_32 = -vsin; pm->_33 = vcos; pm->_34 = 0.0f;
	pm->_41 = 0.0f; pm->_42 = 0.0f; pm->_43 = 0.0f; pm->_44 = 1.0f;
}

/**
 * @brief Y축 회전 행렬을 만든다
 * @param pm 회전 행렬을 담을 행렬
 * @param rot Y측 회전값
*/
QN_INLINE void qn_mat4_y(QnMat4* pm, float rot)
{
	float vsin, vcos;
	qn_sincosf(rot, &vsin, &vcos);
	pm->_11 = vcos; pm->_12 = 0.0f; pm->_13 = -vsin; pm->_14 = 0.0f;
	pm->_21 = 0.0f; pm->_22 = 1.0f; pm->_23 = 0.0f; pm->_24 = 0.0f;
	pm->_31 = vsin; pm->_32 = 0.0f; pm->_33 = vcos; pm->_34 = 0.0f;
	pm->_41 = 0.0f; pm->_42 = 0.0f; pm->_43 = 0.0f; pm->_44 = 1.0f;
}

/**
 * @brief Z축 회전 행렬을 만든다
 * @param pm 회전 행렬을 담을 행렬
 * @param rot Z측 회전값
*/
QN_INLINE void qn_mat4_z(QnMat4* pm, float rot)
{
	float vsin, vcos;
	qn_sincosf(rot, &vsin, &vcos);
	pm->_11 = vcos; pm->_12 = vsin; pm->_13 = 0.0f; pm->_14 = 0.0f;
	pm->_21 = -vsin; pm->_22 = vcos; pm->_23 = 0.0f; pm->_24 = 0.0f;
	pm->_31 = 0.0f; pm->_32 = 0.0f; pm->_33 = 1.0f; pm->_34 = 0.0f;
	pm->_41 = 0.0f; pm->_42 = 0.0f; pm->_43 = 0.0f; pm->_44 = 1.0f;
}

/**
 * @brief 보기 행렬을 만든다 (왼손 기준)
 * @param pm 보기 행렬을 담을 행렬
 * @param eye 시선의 위치
 * @param at 바라보는 방향
 * @param up 시선의 윗쪽 방향
*/
QN_INLINE void qn_mat4_lookat_lh(QnMat4* pm, const QnVec3* eye, const QnVec3* at, const QnVec3* up)
{
	QnVec3 vx, vy, vz;
	qn_vec3_sub(&vz, at, eye);
	qn_vec3_norm(&vz, &vz);
	qn_vec3_cross(&vx, up, &vz);
	qn_vec3_norm(&vx, &vx);
	qn_vec3_cross(&vy, &vz, &vx);

	pm->_11 = vx.x; pm->_12 = vy.x; pm->_13 = vz.x; pm->_14 = 0.0f;
	pm->_21 = vx.y; pm->_22 = vy.y; pm->_23 = vz.y; pm->_24 = 0.0f;
	pm->_31 = vx.z; pm->_32 = vy.z; pm->_33 = vz.z; pm->_34 = 0.0f;

	pm->_41 = -qn_vec3_dot(&vx, eye);
	pm->_42 = -qn_vec3_dot(&vy, eye);
	pm->_43 = -qn_vec3_dot(&vz, eye);
	pm->_44 = 1.0f;
}

/**
 * @brief 보기 행렬을 만든다 (오른손 기준)
 * @param pm 보기 행렬을 담을 행렬
 * @param eye 시선의 위치
 * @param at 바라보는 방향
 * @param up 시선의 윗쪽 방향
 * @return
*/
QN_INLINE void qn_mat4_lookat_rh(QnMat4* pm, const QnVec3* eye, const QnVec3* at, const QnVec3* up)
{
	QnVec3 vx, vy, vz;
	qn_vec3_sub(&vz, eye, at);
	qn_vec3_norm(&vz, &vz);
	qn_vec3_cross(&vx, up, &vz);
	qn_vec3_norm(&vx, &vx);
	qn_vec3_cross(&vy, &vz, &vx);

	pm->_11 = vx.x; pm->_12 = vy.x; pm->_13 = vz.x; pm->_14 = 0.0f;
	pm->_21 = vx.y; pm->_22 = vy.y; pm->_23 = vz.y; pm->_24 = 0.0f;
	pm->_31 = vx.z; pm->_32 = vy.z; pm->_33 = vz.z; pm->_34 = 0.0f;

	pm->_41 = -qn_vec3_dot(&vx, eye);
	pm->_42 = -qn_vec3_dot(&vy, eye);
	pm->_43 = -qn_vec3_dot(&vz, eye);
	pm->_44 = 1.0f;
}

/**
 * @brief 투영 행렬을 만든다 (왼손 기준)
 * @param pm 투영 행렬을 담을 행렬
 * @param fov 포브(Field Of View)값
 * @param aspect 화면 종횡비(가로 나누기 세로)
 * @param depth 뎁스 너비
*/
QN_INLINE void qn_mat4_perspective_lh(QnMat4* pm, float fov, float aspect, const QnDepth* depth)
{
	const float f = 1.0f / tanf(fov * 0.5f);
	const float q = depth->Far / (depth->Far - depth->Near);

	pm->_11 = f / aspect;
	pm->_12 = 0.0f;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = f;
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = 0.0f;
	pm->_33 = q;
	pm->_34 = 1.0f;

	pm->_41 = 0.0f;
	pm->_42 = 0.0f;
	pm->_43 = -depth->Far * q;
	pm->_44 = 0.0f;
}

/**
 * @brief 투영 행렬을 만든다 (오른손 기준)
 * @param pm 투영 행렬을 담을 행렬
 * @param fov 포브(Field Of View)값
 * @param aspect 화면 종횡비(가로 나누기 세로)
 * @param depth 뎁스 너비
*/
QN_INLINE void qn_mat4_perspective_rh(QnMat4* pm, float fov, float aspect, const QnDepth* depth)
{
	const float f = 1.0f / tanf(fov * 0.5f);
	const float q = depth->Far / (depth->Near - depth->Far);

	pm->_11 = f / aspect;
	pm->_12 = 0.0f;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = f;
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = 0.0f;
	pm->_33 = q;
	pm->_34 = -1.0f;

	pm->_41 = 0.0f;
	pm->_42 = 0.0f;
	pm->_43 = depth->Near * q;
	pm->_44 = 0.0f;
}

/**
 * @brief 정규 행렬를 만든다 (왼손 기준)
 * @param pm 정규 행렬을 담을 행렬
 * @param width 너비
 * @param height 높이
 * @param zn 깊이 가까운곳
 * @param zf 깊이 먼곳
*/
QN_INLINE void qn_mat4_ortho_lh(QnMat4* pm, float width, float height, float zn, float zf)
{
	pm->_11 = 2.0f / width;
	pm->_12 = 0.0f;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = 2.0f / height;
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = 0.0f;
	pm->_33 = 1.0f / (zf - zn);
	pm->_34 = 0.0f;

	pm->_41 = 0.0f;
	pm->_42 = 0.0f;
	pm->_43 = zn / (zn - zf);
	pm->_44 = 1.0f;
}
/**
 * @brief 정규 행렬을 만든다 (오른손 기준)
 * @param pm 정규 행렬을 담을 행렬
 * @param width 너비
 * @param height 높이
 * @param zn 깊이 가까운곳
 * @param zf 깊이 먼곳
*/
QN_INLINE void qn_mat4_ortho_rh(QnMat4* pm, float width, float height, float zn, float zf)
{
	pm->_11 = 2.0f / width;
	pm->_12 = 0.0f;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = 2.0f / height;
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = 0.0f;
	pm->_33 = 1.0f / (zn - zf);
	pm->_34 = 0.0f;

	pm->_41 = 0.0f;
	pm->_42 = 0.0f;
	pm->_43 = zn / (zn - zf);
	pm->_44 = 1.0f;
}

/**
 * @brief 사각형을 준으로 정규 행렬을 만든다 (왼손 기준)
 * @param pm 정규 행렬을 담을 행렬
 * @param l 사각형의 왼쪽
 * @param t 사각형의 윗쪽
 * @param r 사각형의 오른쪽
 * @param b 사각형의 아래쪽
 * @param zn 깊이 가까운곳
 * @param zf 깊이 먼곳
*/
QN_INLINE void qn_mat4_ortho_offcenter_lh(QnMat4* pm, float l, float t, float r, float b, float zn, float zf)
{
	pm->_11 = 2.0f / (r - l);
	pm->_12 = 0.0f;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = 2.0f / (t - b);
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = 0.0f;
	pm->_33 = 1.0f / (zf - zn);
	pm->_34 = 0.0f;

	pm->_41 = (l + r) / (l - r);
	pm->_42 = (t + b) / (b - t);
	pm->_43 = zn / (zn - zf);
	pm->_44 = 1.0f;
}

/**
 * @brief 사각형을 준으로 정규 행렬을 만든다 (오른손 기준)
 * @param pm 정규 행렬을 담을 행렬
 * @param l 사각형의 왼쪽
 * @param t 사각형의 윗쪽
 * @param r 사각형의 오른쪽
 * @param b 사각형의 아래쪽
 * @param zn 깊이 가까운곳
 * @param zf 깊이 먼곳
*/
QN_INLINE void qn_mat4_ortho_offcenter_rh(QnMat4* pm, float l, float t, float r, float b, float zn, float zf)
{
	pm->_11 = 2.0f / (r - l);
	pm->_12 = 0.0f;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = 2.0f / (t - b);
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = 0.0f;
	pm->_33 = 1.0f / (zn - zf);
	pm->_34 = 0.0f;

	pm->_41 = (l + r) / (l - r);
	pm->_42 = (t + b) / (b - t);
	pm->_43 = zn / (zn - zf);
	pm->_44 = 1.0f;
}

/**
 * @brief 뷰포트 행렬을 만든다
 * @param pm 뷰포트 행렬을 담을 행렬
 * @param x,y 좌표
 * @param width,height 너비와 높이
*/
QN_INLINE void qn_mat4_viewport(QnMat4* pm, float x, float y, float width, float height)
{
	pm->_11 = width * 0.5f;
	pm->_12 = 0.0f;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = height * -0.5f;
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = 0.0f;
	pm->_33 = 1.0f;
	pm->_34 = 0.0f;

	pm->_41 = x + pm->_11;
	pm->_42 = y - pm->_22;
	pm->_43 = 0.0f;
	pm->_44 = 1.0f;
}

/**
 * @brief 두 행렬을 보간한다
 * @param pm 보간값을 담을 행렬
 * @param left 왼쪽 행렬
 * @param right 오른쪽 행렬
 * @param f 보간값
*/
QN_INLINE void qn_mat4_interpolate(QnMat4* pm, const QnMat4* left, const QnMat4* right, float f)
{
	const float d = 1.0f - f;

	pm->_11 = left->_11 * d + right->_11 * f;
	pm->_12 = left->_12 * d + right->_12 * f;
	pm->_13 = left->_13 * d + right->_13 * f;
	pm->_14 = left->_14 * d + right->_14 * f;

	pm->_21 = left->_21 * d + right->_21 * f;
	pm->_22 = left->_22 * d + right->_22 * f;
	pm->_23 = left->_23 * d + right->_23 * f;
	pm->_24 = left->_24 * d + right->_24 * f;

	pm->_31 = left->_31 * d + right->_31 * f;
	pm->_32 = left->_32 * d + right->_32 * f;
	pm->_33 = left->_33 * d + right->_33 * f;
	pm->_34 = left->_34 * d + right->_34 * f;

	pm->_41 = left->_41 * d + right->_41 * f;
	pm->_42 = left->_42 * d + right->_42 * f;
	pm->_43 = left->_43 * d + right->_43 * f;
	pm->_44 = left->_44 * d + right->_44 * f;
}

// point

/**
 * @brief 점을 설정한다
 * @param pt 설정할 점
 * @param x,y 좌표
*/
QN_INLINE void qn_point_set(QnPoint* pt, int x, int y)
{
	pt->x = x;
	pt->y = y;
}

/**
 * @brief 두 점이 같은가 비교한다
 * @param left 왼쪽 점
 * @param right 오른쪽 점
 * @return 두 점이 같으면 참
*/
QN_INLINE bool qn_point_eq(const QnPoint* left, const QnPoint* right)
{
	return left->x == right->x && left->y == right->y;
}

// size

/**
 * @brief 크기를 설정한다
 * @param pt 설정할 크기
 * @param width 너비
 * @param height 높이
*/
QN_INLINE void qn_size_set(QnSize* pt, int width, int height)
{
	pt->width = width;
	pt->height = height;
}

/**
 * @brief 사각형으로 크기를 설정한다
 * @param pt 설정할 크기
 * @param rt 대상 사각형
*/
QN_INLINE void qn_size_set_rect(QnSize* pt, const QnRect* rt)
{
	pt->width = rt->right - rt->left;
	pt->height = rt->bottom - rt->top;
}

/**
 * @brief 종횡비(너비 나누기 높이)를 계신한다
 * @param pt 계산할 크기
 * @return 종횡비값
*/
QN_INLINE float qn_size_aspect(const QnSize* pt)
{
	return (float)pt->width / (float)pt->height;
}

/**
 * @brief 두 크기가 같은지 비교한다
 * @param left 왼쪽 크기
 * @param right 오른쪽 크기
 * @return 두 크기가 같으면 참을 반환한다
*/
QN_INLINE bool qn_size_eq(const QnSize* left, const QnSize* right)
{
	return left->width == right->width && left->height == right->height;
}

// rect

/**
 * @brief 사각형을 설정한다
 * @param prt 설정할 사각형
 * @param left 왼쪽
 * @param top 위
 * @param right 오른쪽
 * @param bottom 아래
*/
QN_INLINE void qn_rect_set(QnRect* prt, int left, int top, int right, int bottom)
{
	prt->left = left;
	prt->top = top;
	prt->right = right;
	prt->bottom = bottom;
}

/**
 * @brief 사각형을 좌표와 크기로 설정한다
 * @param p 설정할 사각형
 * @param x,y 좌표
 * @param width,height 크기
*/
QN_INLINE void qn_rect_set_size(QnRect* p, int x, int y, int width, int height)
{
	p->left = x;
	p->top = y;
	p->right = x + width;
	p->bottom = y + height;
}

/**
 * @brief 사각형을 좌표와 크기 타입을 사용하여 설정한다
 * @param p 설정할 사각형
 * @param pos 좌표
 * @param size 크기
*/
QN_INLINE void qn_rect_set_qn(QnRect* p, const QnPoint* pos, const QnSize* size)
{
	p->left = pos->x;
	p->top = pos->y;
	p->right = pos->x + size->width;
	p->bottom = pos->y + size->height;
}

/**
 * @brief 사각형을 0으로 설정한다
 * @param prt 설정할 사각형
*/
QN_INLINE void qn_rect_zero(QnRect* prt)
{
	prt->left = 0;
	prt->top = 0;
	prt->right = 0;
	prt->bottom = 0;
}

/**
 * @brief 사각형의 크기를 키운다
 * @param p 결과를 담을 사각형
 * @param rt 원본 사각형
 * @param w 가로 크기
 * @param h 사각형 세로 크기
*/
QN_INLINE void qn_rect_mag(QnRect* p, const QnRect* rt, int w, int h)
{
	p->left = rt->left - w;
	p->top = rt->top - h;
	p->right = rt->right + w;
	p->bottom = rt->bottom + h;
}

/**
 * @brief 사각형 크기를 키운다 (요소가 양수일 경우)
 * @param p 결과를 담을 사각형
 * @param rt 원본 사각형
 * @param l 왼쪽
 * @param t 윗쪽
 * @param r 오른쪽
 * @param b 아래쪽
*/
QN_INLINE void qn_rect_inflate(QnRect* p, const QnRect* rt, int l, int t, int r, int b)
{
	p->left = rt->left - l;
	p->top = rt->top - t;
	p->right = rt->right + r;
	p->bottom = rt->bottom + b;
}

/**
 * @brief 사각형 크기를 줄인다 (요소가 양수일 경우)
 * @param p 결과를 담을 사각형
 * @param rt 원본 사각형
 * @param l 왼쪽
 * @param t 윗쪽
 * @param r 오른족
 * @param b 아래쪽
*/
QN_INLINE void qn_rect_deflate(QnRect* p, const QnRect* rt, int l, int t, int r, int b)
{
	p->left = rt->left + l;
	p->top = rt->top + t;
	p->right = rt->right - r;
	p->bottom = rt->bottom - b;
}

/**
 * @brief 사각형을 움직인다
 * @param p 결과를 담을 사각형
 * @param rt 원본 사각형
 * @param l 왼쪽
 * @param t 윗쪽
 * @param r 오른쪽
 * @param b 아래쪽
*/
QN_INLINE void qn_rect_offset(QnRect* p, const QnRect* rt, int l, int t, int r, int b)
{
	p->left = rt->left + l;
	p->top = rt->top + t;
	p->right = rt->right + r;
	p->bottom = rt->bottom + b;
}

/**
 * @brief 사각형을 움직인다
 * @param p 움직일 사각형
 * @param left 새 왼쪽
 * @param top 새 오른쪽
 * @return
*/
QN_INLINE void qn_rect_move(QnRect* p, int left, int top)
{
	int dx = left - p->left;
	int dy = top - p->top;
	p->left += dx;
	p->top += dy;
	p->right += dx;
	p->bottom += dy;
}

/**
 * @brief 사각형의 크기를 재설정한다
 * @param p 재설정할 사각형
 * @param width 너비
 * @param height 높이
*/
QN_INLINE void qn_rect_size(QnRect* p, int width, int height)
{
	p->right = p->left + width;
	p->bottom = p->top + height;
}

/**
 * @brief 사각형의 너비를 얻는다
 * @param prt 대상 사각형
 * @return 사각형의 너비값
*/
QN_INLINE int qn_rect_width(const QnRect* prt)
{
	return prt->right - prt->left;
}

/**
 * @brief 사각형의 높이를 얻는다
 * @param prt 대상 사각형
 * @return 사각형의 높이값
*/
QN_INLINE int qn_rect_height(const QnRect* prt)
{
	return prt->bottom - prt->top;
}

/**
 * @brief 좌표가 사각형 안에 있는지 조사한다
 * @param prt 대상 사각형
 * @param x,y 좌표
 * @return 좌표가 사각형 안에 있으면 참
*/
QN_INLINE bool qn_rect_in(const QnRect* prt, int x, int y)
{
	return (x >= prt->left && x <= prt->right && y >= prt->top && y <= prt->bottom);
}

/**
 * @brief 두 사각형이 같은지 조시한다
 * @param r1 왼쪽 사각형
 * @param r2 오른쪽 사각형
 * @return 두 사각형이 같으면 참
*/
QN_INLINE bool qn_rect_eq(const QnRect* r1, const QnRect* r2)
{
	return r1->left == r2->left && r1->right == r2->right && r1->top == r2->top && r1->bottom == r2->bottom;
}

/**
 * @brief 두 사각형이 충돌하는지 비교하고 충돌 사각형을 만든다
 * @param p 충돌 사각형이 담길 사각형 (NULL 가능)
 * @param r1 왼쪽 사각형
 * @param r2 오른쪽 사각형
 * @return 두 사각형이 충돌했다면 참
*/
QN_INLINE bool qn_rect_intersect(QnRect* p, const QnRect* r1, const QnRect* r2)
{
	const bool b = r2->left < r1->right && r2->right > r1->left && r2->top < r1->bottom && r2->bottom > r1->top;
	if (p)
	{
		if (!b)
			qn_rect_set(p, 0, 0, 0, 0);
		else
		{
			qn_rect_set(p,
				QN_MAX(r1->left, r2->left), QN_MAX(r1->top, r2->top),
				QN_MIN(r1->right, r2->right), QN_MIN(r1->bottom, r2->bottom));
		}
	}
	return b;
}

// color

/**
 * @brief 색깔을 설정한다
 * @param pc 설정할 색깔
 * @param r 빨강
 * @param g 녹색
 * @param b 파랑
 * @param a 알파
*/
QN_INLINE void qn_color_set(QnColor* pc, float r, float g, float b, float a)
{
	pc->r = r;
	pc->g = g;
	pc->b = b;
	pc->a = a;
}

/**
 * @brief 32비트 RGBA 정수로 색깔을 설정한다
 * @param pc 설정할 색깔
 * @param value 32비트 RGBA 정수
*/
QN_INLINE void qn_color_set_uint(QnColor* pc, uint value)
{
	const float f = 1.0f / 255.0f;
	pc->b = (float)(value & 255) * f; value >>= 8;
	pc->g = (float)(value & 255) * f; value >>= 8;
	pc->r = (float)(value & 255) * f; value >>= 8;
	pc->a = (float)(value & 255) * f;
}

/**
 * @brief 정수형 색깔로 색깔을 설정한다
 * @param pc 설정할 색깔
 * @param cu 정수형 색깔
*/
QN_INLINE void qn_color_set_kolor(QnColor* pc, const QnKolor* cu)
{
	const float f = 1.0f / 255.0f;
	pc->a = (float)cu->a * f;
	pc->r = (float)cu->r * f;
	pc->g = (float)cu->g * f;
	pc->b = (float)cu->b * f;
}

/**
 * @brief 두 색깔의 덧셈
 * @param pc 덧셈 결과를 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
*/
QN_INLINE void qn_color_add(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = left->r + right->r;
	pc->g = left->g + right->b;
	pc->b = left->b + right->b;
	pc->a = left->a + right->a;
}

/**
 * @brief 두 색깔의 뺄셈
 * @param pc 뺄셈 결과를 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
*/
QN_INLINE void qn_color_sub(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = left->r - right->r;
	pc->g = left->g - right->b;
	pc->b = left->b - right->b;
	pc->a = left->a - right->a;
}

/**
 * @brief 색깔의 크기를 변경한다 (밝기 변경이 아니다)
 * @param pc 크기를 변경한 결과를 담을 색깔
 * @param left 원본 색깔
 * @param scale 변경할 크기
*/
QN_INLINE void qn_color_mag(QnColor* pc, const QnColor* left, float scale)
{
	pc->r = left->r * scale;
	pc->g = left->g * scale;
	pc->b = left->b * scale;
	pc->a = left->a * scale;
}

/**
 * @brief 네거티브 색깔을 얻는다
 * @param pc 네거티브 결과를 담을 색깔
 * @param c 원본 색깔
*/
QN_INLINE void qn_color_neg(QnColor* pc, const QnColor* c)
{
	pc->r = 1.0f - c->r;
	pc->g = 1.0f - c->g;
	pc->b = 1.0f - c->b;
	pc->a = c->a;
}

/**
 * @brief 색깔을 보간한다
 * @param pc 보간 결과를 담을 색깔
 * @param left 시작 색깔
 * @param right 끝 색깔
 * @param s 보간값
*/
QN_INLINE void qn_color_interpolate(QnColor* pc, const QnColor* left, const QnColor* right, float s)
{
	const float f = 1.0f - s;
	pc->r = left->r * f + right->r * s;
	pc->g = left->g * f + right->g * s;
	pc->b = left->b * f + right->b * s;
	pc->a = left->a * f + right->a * s;
}

/**
 * @brief 색깔을 선형 보간한다
 * @param pc 보간 결과를 담을 색깔
 * @param left 시작 색깔
 * @param right 끝 색깔
 * @param s 보간값
 * @return
*/
QN_INLINE void qn_color_lerp(QnColor* pc, const QnColor* left, const QnColor* right, float s)
{
	pc->r = left->r + s * (right->r - left->r);
	pc->g = left->g + s * (right->g - left->g);
	pc->b = left->b + s * (right->b - left->b);
	pc->a = left->a + s * (right->a - left->a);
}

/**
 * @brief 두 색깔의 곱셈
 * @param pc 곱셈 결과를 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
*/
QN_INLINE void qn_color_mod(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = left->r * right->r;
	pc->g = left->g * right->g;
	pc->b = left->b * right->b;
	pc->a = left->a * right->a;
}

/**
 * @brief 두 색깔의 최소값
 * @param pc 최소값을 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
*/
QN_INLINE void qn_color_min(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = (left->r < right->r) ? left->r : right->r;
	pc->g = (left->g < right->g) ? left->g : right->g;
	pc->b = (left->b < right->b) ? left->b : right->b;
	pc->a = (left->a < right->a) ? left->a : right->a;
}

/**
 * @brief 두 색깔의 최대값
 * @param pc 최대값을 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
*/
QN_INLINE void qn_color_max(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = (left->r > right->r) ? left->r : right->r;
	pc->g = (left->g > right->g) ? left->g : right->g;
	pc->b = (left->b > right->b) ? left->b : right->b;
	pc->a = (left->a > right->a) ? left->a : right->a;
}

/**
 * @brief 색깔을 실수형 배열로
 * @param c 대상 색깔
 * @return 실수형 배열 포인터
*/
QN_INLINE float* qn_color_float(QnColor* c)
{
	return &c->r;
}

/**
 * @brief 색깔을 32비트 정수로 만든다 (빠른 버전)
 * @param pc 대상 색깔
 * @return 32비트 정수
*/
QN_INLINE uint qn_color_uint_fast(const QnColor* pc)
{
	const byte R = (byte)(pc->r * 255.0f + 0.5f);
	const byte G = (byte)(pc->g * 255.0f + 0.5f);
	const byte B = (byte)(pc->b * 255.0f + 0.5f);
	const byte A = (byte)(pc->a * 255.0f + 0.5f);
	return ((uint)A << 24) | ((uint)R << 16) | ((uint)G << 8) | (uint)B;
}

/**
 * @brief 색깔을 32비트 정수로 만든다
 * @param pc 대상 색깔
 * @return 32비트 정수
*/
QN_INLINE uint qn_color_uint(const QnColor* pc)
{
	const uint R = (pc->r >= 1.0f) ? 0xff : (pc->r <= 0.0f) ? 0x00 : (uint)(pc->r * 255.0f + 0.5f);
	const uint G = (pc->g >= 1.0f) ? 0xff : (pc->g <= 0.0f) ? 0x00 : (uint)(pc->g * 255.0f + 0.5f);
	const uint B = (pc->b >= 1.0f) ? 0xff : (pc->b <= 0.0f) ? 0x00 : (uint)(pc->b * 255.0f + 0.5f);
	const uint A = (pc->a >= 1.0f) ? 0xff : (pc->a <= 0.0f) ? 0x00 : (uint)(pc->a * 255.0f + 0.5f);
	return (A << 24) | (R << 16) | (G << 8) | B;
}

/**
 * @brief 두 색깔이 같은지 비교한다
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
 * @return 두 색깔이 같으면 참
*/
QN_INLINE bool qn_color_eq(const QnColor* left, const QnColor* right)
{
	return
		qn_eqf(left->r, right->r) &&
		qn_eqf(left->g, right->g) &&
		qn_eqf(left->b, right->b) &&
		qn_eqf(left->a, right->a);
}

/**
 * @brief 색깔의 콘트라스트를 조정한다
 * @param pc 조정 결과를 담을 색깔
 * @param c 원본 색깔
 * @param s 조정값
 * @return
 */
QN_INLINE void qn_color_contrast(QnColor* pc, const QnColor* c, float s)
{
	pc->r = 0.5f + s * (c->r - 0.5f);
	pc->g = 0.5f + s * (c->g - 0.5f);
	pc->b = 0.5f + s * (c->b - 0.5f);
	pc->a = c->a;
}

/**
 * @brief 색깔의 새츄레이션을 조정한다
 * @param pc 조정 결과를 담을 색깔
 * @param c 원본 색깔
 * @param s 조정값
*/
QN_INLINE void qn_color_saturation(QnColor* pc, const QnColor* c, float s)
{
	const float g = c->r * 0.2125f + c->g * 0.7154f + c->b * 0.0721f;
	pc->r = g + s * (c->r - g);
	pc->g = g + s * (c->g - g);
	pc->b = g + s * (c->b - g);
	pc->a = c->a;
}

// byte color

/**
 * @brief 색깔을 설정한다
 * @param pc 설정한 색깔
 * @param r 빨강
 * @param g 녹색
 * @param b 파랑
 * @param a 알파
*/
QN_INLINE void qn_kolor_set(QnKolor* pc, byte r, byte g, byte b, byte a)
{
	pc->r = r;
	pc->g = g;
	pc->b = b;
	pc->a = a;
}

/**
 * @brief 색깔을 설정한다
 * @param pc 설정한 색깔
 * @param r 빨강
 * @param g 녹색
 * @param b 파랑
 * @param a 알파
*/
QN_INLINE void qn_kolor_set_float(QnKolor* pc, float r, float g, float b, float a)
{
	pc->r = (byte)lrintf(r * 255.0f);
	pc->g = (byte)lrintf(g * 255.0f);
	pc->b = (byte)lrintf(b * 255.0f);
	pc->a = (byte)lrintf(a * 255.0f);
}

/**
 * @brief 색깔을 설정한다
 * @param pc 설정한 색깔
 * @param value 32비트 RGBA 정수
*/
QN_INLINE void qn_kolor_set_uint(QnKolor* pc, uint value)
{
	((QnKolorU*)pc)->u = value;
}

/**
 * @brief 색깔을 설정한다
 * @param pc 설정한 색깔
 * @param cr 실수형 색깔
*/
QN_INLINE void qn_kolor_set_color(QnKolor* pc, const QnColor* cr)
{
	pc->r = (byte)lrintf(cr->r * 255.0f);
	pc->g = (byte)lrintf(cr->g * 255.0f);
	pc->b = (byte)lrintf(cr->b * 255.0f);
	pc->a = (byte)lrintf(cr->a * 255.0f);
}

/**
 * @brief 두 색깔의 덧셈
 * @param pc 덧셈 결과를 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
 * @return
*/
QN_INLINE void qn_kolor_add(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	((QnKolorU*)pc)->u = ((const QnKolorU*)left)->u + ((const QnKolorU*)right)->u;
}

/**
 * @brief 두 색깔의 뺄셈
 * @param pc 뺄셈 결과를 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
 * @return
*/
QN_INLINE void qn_kolor_sub(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	((QnKolorU*)pc)->u = ((const QnKolorU*)left)->u - ((const QnKolorU*)right)->u;
}

/**
 * @brief 색깔을 확대한다 (밝기가 아니다)
 * @param pc 확대한 색깔을 담을 색깔
 * @param left 왼쪽 색깔
 * @param scale 확대값
 * @return */
QN_INLINE void qn_kolor_mag(QnKolor* pc, const QnKolor* left, float scale)
{
	pc->r = (byte)lrintf((float)left->r * scale);
	pc->g = (byte)lrintf((float)left->g * scale);
	pc->b = (byte)lrintf((float)left->b * scale);
	pc->a = (byte)lrintf((float)left->a * scale);
}

/**
 * @brief 네거티브 색깔을 만든다
 * @param pc 네거티브 색깔을 담을 색깔
 * @param c 원본 색깔
*/
QN_INLINE void qn_kolor_neg(QnKolor* pc, const QnKolor* c)
{
	pc->r = 255 - c->r;
	pc->g = 255 - c->g;
	pc->b = 255 - c->b;
	pc->a = c->a;
}

/**
 * @brief 색깔을 보간한다
 * @param pc 보간 결과를 담을 색깔
 * @param left 시작 색깔
 * @param right 끝 색깔
 * @param s 보간값
*/
QN_INLINE void qn_kolor_interpolate(QnKolor* pc, const QnKolor* left, const QnKolor* right, float s)
{
	const float f = 1.0f - s;
	pc->r = (byte)lrintf((float)left->r * f + (float)right->r * s);
	pc->g = (byte)lrintf((float)left->g * f + (float)right->g * s);
	pc->b = (byte)lrintf((float)left->b * f + (float)right->b * s);
	pc->a = (byte)lrintf((float)left->a * f + (float)right->a * s);
}

/**
 * @brief 색깔을 선형 보간한다
 * @param pc 보간 결과를 담을 색깔
 * @param left 시작 색깔
 * @param right 끝 색깔
 * @param s 보간값
 * @return
*/
QN_INLINE void qn_kolor_lerp(QnKolor* pc, const QnKolor* left, const QnKolor* right, float s)
{
	pc->r = (byte)lrintf((float)left->r + s * (float)(right->r - left->r));
	pc->g = (byte)lrintf((float)left->g + s * (float)(right->g - left->g));
	pc->b = (byte)lrintf((float)left->b + s * (float)(right->b - left->b));
	pc->a = (byte)lrintf((float)left->a + s * (float)(right->a - left->a));
}

/**
 * @brief 두 색깔의 곱셈
 * @param pc 곱셈 결과를 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
*/
QN_INLINE void qn_kolor_mod(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	pc->r = left->r * right->r;
	pc->g = left->g * right->g;
	pc->b = left->b * right->b;
	pc->a = left->a * right->a;
}

/**
 * @brief 두 색깔의 최소값
 * @param pc 최소값을 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
*/
QN_INLINE void qn_kolor_min(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	pc->r = (left->r < right->r) ? left->r : right->r;
	pc->g = (left->g < right->g) ? left->g : right->g;
	pc->b = (left->b < right->b) ? left->b : right->b;
	pc->a = (left->a < right->a) ? left->a : right->a;
}

/**
 * @brief 두 색깔의 최대값
 * @param pc 최대값을 담을 색깔
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
*/
QN_INLINE void qn_kolor_max(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	pc->r = (left->r > right->r) ? left->r : right->r;
	pc->g = (left->g > right->g) ? left->g : right->g;
	pc->b = (left->b > right->b) ? left->b : right->b;
	pc->a = (left->a > right->a) ? left->a : right->a;
}

/**
 * @brief 두 색깔이 같은지 비교한다
 * @param left 왼쪽 색깔
 * @param right 오른쪽 색깔
 * @return 두 색깔이 같으면 참
*/
QN_INLINE bool qn_kolor_eq(const QnKolor* left, const QnKolor* right)
{
	return ((const QnKolorU*)left)->u == ((const QnKolorU*)right)->u;
}

/**
 * @brief 색깔의 콘트라스트를 조정한다
 * @param pc 조정 결과를 담을 색깔
 * @param c 원본 색깔
 * @param s 조정값
 * @return
 */
QN_INLINE void qn_kolor_contrast(QnKolor* pc, const QnKolor* c, float s)
{
	QnColor cr, cc;
	qn_color_set_kolor(&cr, pc);
	qn_color_set_kolor(&cc, c);
	qn_color_contrast(&cr, &cc, s);
	qn_kolor_set_color(pc, &cr);
}

/**
 * @brief 색깔의 새츄레이션을 조정한다
 * @param pc 조정 결과를 담을 색깔
 * @param c 원본 색깔
 * @param s 조정값
*/
QN_INLINE void qn_kolor_saturation(QnKolor* pc, const QnKolor* c, float s)
{
	QnColor cr, cc;
	qn_color_set_kolor(&cr, pc);
	qn_color_set_kolor(&cc, c);
	qn_color_saturation(&cr, &cc, s);
	qn_kolor_set_color(pc, &cr);
}

// plane

/**
 * @brief 면을 초기화한다
 * @param pp 초기화할 면
*/
QN_INLINE void qn_plane_rst(QnPlane* pp)
{
	pp->a = pp->b = pp->c = 0.0f;
	pp->d = 1.0f;
}

/**
 * @brief 면을 만든다
 * @param pp 설정할 면
 * @param a,b,c,d 면의 설정값
*/
QN_INLINE void qn_plane_set(QnPlane* pp, float a, float b, float c, float d)
{
	pp->a = a;
	pp->b = b;
	pp->c = c;
	pp->d = d;
}

/**
 * @brief 면과 점(벡터3)의 내적
 * @param pp 대상 면
 * @param v 대상 벡터3
 * @return 면과 점의 내적
*/
QN_INLINE float qn_plane_dot_coord(const QnPlane* pp, const QnVec3* v)
{
	return pp->a * v->x + pp->b * v->y + pp->c * v->z + pp->d;
}

/**
 * @brief 점과 점의 법선으로 면을 만든다
 * @param pp 만들어진 면을 담을 면
 * @param pv 점
 * @param pn 점의 법선
*/
QN_INLINE void qn_plane_from_point_norm(QnPlane* pp, const QnVec3* pv, const QnVec3* pn)
{
	pp->a = pn->x;
	pp->b = pn->y;
	pp->c = pn->z;
	pp->d = -qn_vec3_dot(pv, pn);
}

/**
 * @brief 점과 면의 관계를 얻는다
 * @param p 대상 면
 * @param v 관계를 얻을 점
 * @return 관계 값을 얻는다
 * @retval 0 점이 면 위에 있다
 * @retval 1 점이 면 앞에 있다
 * @retval -1 점이 면 뒤에 있다
*/
QN_INLINE int qn_plane_face_point(const QnPlane* p, const QnVec3* v)
{
	const float f = qn_vec3_dot((const QnVec3*)p, v) + p->d;
	if (f < -QN_EPSILON)
		return -1;  // back
	if (f > QN_EPSILON)
		return 1;   // front
	return 0;       // on
}

/**
 * @brief 면과 점의 거리를 얻는다
 * @param p 대상 면
 * @param v 대상 점
 * @return 면과 점의 거리
*/
QN_INLINE float qn_plane_dist(const QnPlane* p, const QnVec3* v)
{
	return qn_vec3_dot(v, (const QnVec3*)p) + p->d;
}

/**
 * @brief 면을 정규화 한다
 * @param pp 정규화한 면을 담을 면
 * @param p 대상 면
*/
QN_INLINE void qn_plane_norm(QnPlane* pp, const QnPlane* p)
{
	const float f = 1.0f / sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
	pp->a = p->a * f;
	pp->b = p->b * f;
	pp->c = p->c * f;
	pp->d = p->d * f;
}

/**
 * @brief 면을 뒤집어서 정규화 한다
 * @param pp 정규화한 면을 담을 면
 * @param p 대상 면
*/
QN_INLINE void qn_plane_rnorm(QnPlane* pp, const QnPlane* p)
{
	const float n = -1.0f / sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
	pp->a = p->a * n;
	pp->b = p->b * n;
	pp->c = p->c * n;
	pp->d = p->d * n;
}

/**
 * @brief 면과 선의 충돌을 조사한다
 * @param p 대상 면
 * @param v1 대상 선의 시작
 * @param v2 대상 선의 끝
 * @return 면과 선의 거리
*/
QN_INLINE float qn_plane_intersect_line(const QnPlane* p, const QnVec3* v1, const QnVec3* v2)
{
	QnVec3 t;
	qn_vec3_sub(&t, v2, v1);
	const float f = 1.0f / qn_vec3_dot((const QnVec3*)p, &t);
	return -(qn_vec3_dot((const QnVec3*)p, v1) + p->d) * f;
}

/**
 * @brief 두 면이 충돌하는지 조시한다
 * @param p 대상 면
 * @param o1 첫번째 충돌 검사 면
 * @param o2 두번째 충돌 검사 면
 * @return 두 면과 충돌했으면 참
*/
QN_INLINE bool qn_plane_intersect_plane(const QnPlane* p, const QnPlane* o1, const QnPlane* o2)
{
	QnVec3 v, dir, loc;
	return (qn_plane_intersect(p, &loc, &dir, o1)) ? qn_vec3_intersect_line(&v, o2, &loc, &dir) : false;
}

// line3

/**
 * @brief 선분을 만든다
 * @param p 만들어진 선분
 * @param bx,by,bz 시작 점
 * @param ex,ey,ez 끝 점
*/
QN_INLINE void qn_line3_set(QnLine3* p, float bx, float by, float bz, float ex, float ey, float ez)
{
	qn_vec3_set(&p->begin, bx, by, bz);
	qn_vec3_set(&p->end, ex, ey, ez);
}

/**
 * @brief 벡터3 두개로 선분을 만든다
 * @param p 만들어진 선분
 * @param begin 시작 벡터3
 * @param end 끝 벡터3
*/
QN_INLINE void qn_line3_set_vec(QnLine3* p, const QnVec3* begin, const QnVec3* end)
{
	p->begin = *begin;
	p->end = *end;
}

/**
 * @brief 선분을 이동한다 (덧셈)
 * @param p 이동한 선분을 담을 선분
 * @param l 원본 선분
 * @param v 이동 거리 벡터3
*/
QN_INLINE void qn_line3_add(QnLine3* p, const QnLine3* l, const QnVec3* v)
{
	qn_vec3_add(&p->begin, &l->begin, v);
	qn_vec3_add(&p->end, &l->end, v);
}

/**
 * @brief 선분을 이동한다 (뺄셈)
 * @param p 이동한 선분을 담을 선분
 * @param l 원본 선분
 * @param v 이동 거리 벡터3
*/
QN_INLINE void qn_line3_sub(QnLine3* p, const QnLine3* l, const QnVec3* v)
{
	qn_vec3_sub(&p->begin, &l->begin, v);
	qn_vec3_sub(&p->end, &l->end, v);
}

/**
 * @brief 선분 길이의 제곱을 얻는다
 * @param p 대상 선분
 * @return 선분 길이의 제곱
*/
QN_INLINE float qn_line3_len_sq(const QnLine3* p)
{
	return qn_vec3_dist_sq(&p->begin, &p->end);
}

/**
 * @brief 선분 길이를 얻는다
 * @param p 대상 선분
 * @return 선분 길이
*/
QN_INLINE float qn_line3_len(const QnLine3* p)
{
	return qn_vec3_dist(&p->begin, &p->end);
}

/**
 * @brief 선분의 중심을 얻는다
 * @param p 대상 선분
 * @param v 얻은 중심을 담은 벡터3
*/
QN_INLINE void qn_line3_center(const QnLine3* p, QnVec3* v)
{
	qn_vec3_add(v, &p->begin, &p->end);
	qn_vec3_mag(v, v, 0.5f);
}

/**
 * @brief 선분의 벡터를 얻는다
 * @param p 대상 선분
 * @param v 선분의 벡터를 담은 벡터3
*/
QN_INLINE void qn_line3_vec(const QnLine3* p, QnVec3* v)
{
	qn_vec3_sub(v, &p->end, &p->begin);
}

/**
 * @brief 점(벡터3)이 선분 위에 있는지 판단한다
 * @param p 대상 선분
 * @param v 대상 점
 * @return 점이 선분 위에 있으면 참
*/
QN_INLINE bool qn_line3_in(const QnLine3* p, const QnVec3* v)
{
	return qn_vec3_between(v, &p->begin, &p->end);
}

// half

/**
 * @brief 32비트 실수를 16비트 실수로 변환한다
 * @param v 32비트 실수
 * @return 변환한 16비트 실수
*/
QN_INLINE halfint qn_f2hf(const float v)
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

/**
 * @brief 16비트 실수를 32비트 실수로 변환한다
 * @param v 변환할 16비트 실수
 * @return 변환한 32비트 실수
*/
QN_INLINE float qn_hf2f(const halfint v)
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

	uint r = ((v & 0x8000) << 16) | ((e + 112) << 23) | (m << 13);
	return *(float*)&r;
}

/**
 * @brief 하프 벡터2를 설정한다
 * @param p 대상 하프 벡터
 * @param x,y 좌표
*/
QN_INLINE void qn_vec2h_set(QnVecH2* p, float x, float y)
{
	p->x = qn_f2hf(x);
	p->y = qn_f2hf(y);
}

/**
 * @brief 하프 벡터3를 설정한다
 * @param p 대상 하프 벡터
 * @param x,y,z 좌표
*/
QN_INLINE void qn_vec3h_set(QnVecH3* p, float x, float y, float z)
{
	p->x = qn_f2hf(x);
	p->y = qn_f2hf(y);
	p->z = qn_f2hf(z);
}

/**
 * @brief 하프 벡터4를 설정한다
 * @param p 대상 하프 벡터
 * @param x,y,z,w 좌표
*/
QN_INLINE void qn_vec4h_set(QnVecH4* p, float x, float y, float z, float w)
{
	p->x = qn_f2hf(x);
	p->y = qn_f2hf(y);
	p->z = qn_f2hf(z);
	p->w = qn_f2hf(w);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
