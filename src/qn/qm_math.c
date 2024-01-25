//
// qm_math.c - 수학 함수
// 2023-12-27 by kim
//

#include "pch.h"

//////////////////////////////////////////////////////////////////////////
// XOR SHIFT

// 내장 랜덤 상태
static QmRandom random_state = { 0, UINTPTR_MAX / 3, UINTPTR_MAX / 5 };

//
void qm_srand(QmRandom* r, nuint seed)
{
	if (r == NULL)
		r = &random_state;
	r->seed = seed != 0 ? seed : (nuint)qn_now();
	r->state1 = r->seed;
	r->state2 = r->seed % 7;
}

//
nuint qm_rand(QmRandom* r)
{
	if (r == NULL)
		r = &random_state;
	nuint state1 = r->state1;
	nuint state2 = r->state2;
	state1 ^= state1 << 13; state1 ^= state1 >> 17; state1 ^= state1 << 5;
	state2 ^= state2 << 17; state2 ^= state2 >> 13; state2 ^= state2 << 7;
	r->state1 = state1;
	r->state2 = state2;
	return state1 ^ state2;
}

//
float qm_randf(QmRandom* r)
{
	return (float)qm_rand(r) / (float)UINTPTR_MAX;
}

//
double qm_randd(QmRandom* r)
{
	return (double)qm_rand(r) / (double)UINTPTR_MAX;
}


//////////////////////////////////////////////////////////////////////////
// 수학 함수

/// @brief 행렬식
QSAPI float QM_VECTORCALL qm_mat4_det(QmMat4 m)
{
	QmVec4 c01 = qm_vec3_cross(m.r[0], m.r[1]);
	QmVec4 c23 = qm_vec3_cross(m.r[2], m.r[3]);
	QmVec4 s10 = qm_vec_sub(qm_vec_mag(m.r[0], m._24), qm_vec_mag(m.r[1], m._14));
	QmVec4 s32 = qm_vec_sub(qm_vec_mag(m.r[2], m._44), qm_vec_mag(m.r[3], m._34));
	return qm_vec3_dot(c01, s32) + qm_vec3_dot(c23, s10);
}



//////////////////////////////////////////////////////////////////////////
// 지운거 남겨 둠
#if false
// line3

/// @brief 선분을 만든다
/// @param bx,by,bz 시작 점
/// @param ex,ey,ez 끝 점
INLINE QmLine3 qm_line3(float bx, float by, float bz, float ex, float ey, float ez)
{
	QmLine3 r = { .Begin = qm_vec3(bx, by, bz),.End = qm_vec3(ex, ey, ez), };
	return r;
}

/// @brief 벡터3 두개로 선분을 만든다
/// @param begin 시작 벡터3
/// @param end 끝 벡터3
INLINE QmLine3 qm_line3vv(QmVec3 begin, QmVec3 end)
{
	QmLine3 r = { .Begin = begin, .End = end };
	return r;
}

/// @brief 선분을 만든다
///	@param l 반환 선분
/// @param bx,by,bz 시작 점
/// @param ex,ey,ez 끝 점
INLINE void qm_line3_set(QmLine3 * l, float bx, float by, float bz, float ex, float ey, float ez)
{
	l->Begin = qm_vec3(bx, by, bz);
	l->End = qm_vec3(ex, ey, ez);
}

/// @brief 벡터3 두개로 선분을 만든다
///	@param l 반환 선분
/// @param begin 시작 벡터3
/// @param end 끝 벡터3
INLINE void qm_line3_setvv(QmLine3 * l, QmVec3 begin, QmVec3 end)
{
	l->Begin = begin;
	l->End = end;
}

/// @brief 선분을 이동한다 (덧셈)
/// @param l 원본 선분
/// @param v 이동 거리 벡터3
INLINE QmLine3 qm_line3_add(QmLine3 l, QmVec3 v)
{
	return qm_line3vv(qm_vec3_add(l.Begin, v), qm_vec3_add(l.End, v));
}

/// @brief 선분을 이동한다 (뺄셈)
/// @param l 원본 선분
/// @param v 이동 거리 벡터3
INLINE QmLine3 qm_line3_sub(QmLine3 l, QmVec3 v)
{
	return qm_line3vv(qm_vec3_sub(l.Begin, v), qm_vec3_sub(l.End, v));
}

/// @brief 선분 길이의 제곱을 얻는다
/// @param l 대상 선분
/// @return 선분 길이의 제곱
INLINE float qm_line3_len_sq(QmLine3 l)
{
	return qm_vec3_dist_sq(l.Begin, l.End);
}

/// @brief 선분 길이를 얻는다
/// @param l 대상 선분
/// @return 선분 길이
INLINE float qm_line3_len(QmLine3 l)
{
	return qm_vec3_dist(l.Begin, l.End);
}

/// @brief 선분의 중심을 얻는다
/// @param l 대상 선분
INLINE QmVec3 qm_line3_center(QmLine3 l)
{
	return qm_vec3_mag(qm_vec3_add(l.Begin, l.End), 0.5f);
}

/// @brief 선분의 벡터를 얻는다
/// @param l 대상 선분
INLINE QmVec3 qm_line3_vec(QmLine3 l)
{
	return qm_vec3_sub(l.End, l.Begin);
}

/// @brief 점(벡터3)이 선분 위에 있는지 판단한다
/// @param l 대상 선분
/// @param v 대상 점
/// @return 점이 선분 위에 있으면 참
INLINE bool qm_line3_in(QmLine3 l, QmVec3 v)
{
	return qm_vec3_between(v, l.Begin, l.End);
}

/// @brief 구와 충돌하는 선 판정
/// @param l 처리할 선
/// @param org 구의 중점
/// @param rad 구의 반지름
/// @param dist 충돌 거리
/// @return 충돌하면 true
INLINE bool qm_line3_intersect_sphere(QmLine3 l, QmVec3 org, float rad, float* dist)
{
	QmVec3 v = qm_vec3_norm(qm_line3_vec(l));
	QmVec3 t = qm_vec3_sub(org, l.Begin);
	float c = qm_vec3_len(t);
	float z = qm_vec3_dot(t, v);
	float d = rad * rad - (c * c - z * z);
	if (d < 0.0f)
		return false;
	if (dist)
		*dist = z - qm_sqrtf(d);
	return true;
}
#endif

