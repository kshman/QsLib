﻿#include "pch.h"
#include "qn.h"
#if _M_IX86_FP || _M_AMD64  || __amd64__ || __x86_64__
#include <emmintrin.h>
#define USE_EMM_INTRIN		1
#endif
#include "qnmath.h"

#if USE_EMM_INTRIN
#ifndef QN_ALIGN
#if _MSC_VER
#define QN_ALIGN(x)			__declspec(align(x))
#else
#define QN_ALIGN(x)			__attribute__ ((aligned(x)))
#endif
#endif
#define _mm_ror_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((uint8_t)(i+3)%4,(uint8_t)(i+2)%4,(uint8_t)(i+1)%4,(uint8_t)(i+0)%4))) : (vec))
#define _mm_rol_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((uint8_t)(7-i)%4,(uint8_t)(6-i)%4,(uint8_t)(5-i)%4,(uint8_t)(4-i)%4))) : (vec))
#endif

/*!
 * @brief 선에 가까운 점의 위치
 * @param pv 반환되는 근처 점
 * @param line 선
 * @param loc 점
*/
void qn_vec3_closed_line(qnVec3* pv, const qnLine3* line, const qnVec3* loc)
{
	qnVec3 c, z;
	qn_vec3_sub(&c, loc, &line->begin);
	qn_vec3_sub(&z, &line->end, &line->begin);
	float d = qn_vec3_len(&z);
	qn_vec3_mag(&z, &z, 1.0f / d);
	float t = qn_vec3_dot(&z, &c);

	if (t < 0.0f)
		*pv = line->begin;
	else if (t > d)
		*pv = line->end;
	else
	{
		qn_vec3_mag(&z, &z, t);
		qn_vec3_add(pv, &line->begin, &z);
	}
}

/*!
 * @brief 단위 벡터로 만들어 크기를 조정하고 길이 만큼 혼합
 * @param pv 결과 벡터
 * @param left 시작 벡터
 * @param right 끝 벡터
 * @param scale 크기 변화량
 * @param len 길이
*/
void qn_vec3_lerp_len(qnVec3* pv, const qnVec3* left, const qnVec3* right, float scale, float len)
{
	qnVec3 v0, v1;
	qn_vec3_norm(&v0, left);
	qn_vec3_norm(&v1, right);
	pv->x = v0.x + (v1.x - v0.x) * scale;
	pv->y = v0.y + (v1.y - v0.y) * scale;
	pv->z = v0.z + (v1.z - v0.z) * scale;
	qn_vec3_mag(pv, pv, len);
}

/*!
 * @brief 세 벡터로 법선 벡터를 만든다
 * @param pv 얻은 법선 벡터
 * @param v0 벡터 1
 * @param v1 벡터 2
 * @param v2 벡터 3
*/
void qn_vec3_form_norm(qnVec3* pv, const qnVec3* v0, const qnVec3* v1, const qnVec3* v2)
{
	qnVec3 t0, t1;
	qn_vec3_sub(&t0, v1, v0);
	qn_vec3_sub(&t1, v2, v1);
	qn_vec3_cross(pv, &t0, &t1);
	qn_vec3_mag(pv, pv, qn_vec3_len(pv));
}

/*!
 * @brief 반사 벡터를 만든다
 * @param pv 반사된 벡터
 * @param in 입력 벡터
 * @param dir 법선 벡터
 * @return 반사 벡터가 만들어지면 참
*/
bool qn_vec3_reflect(qnVec3* pv, const qnVec3* in, const qnVec3* dir)
{
	qnVec3 t;
	float dot, len = qn_vec3_len(in);
	if (len == 0.0f)
		qn_vec3_rst(&t);
	else
	{
		dot = 1.0f / len;
		qn_vec3_mag(&t, in, dot);
	}
	dot = qn_vec3_dot(&t, dir);
	if (dot >= 0.0f)
		return false;
	else
	{
		qn_vec3_set(pv, -2.0f * dot * dir->x + t.x, -2.0f * dot * dir->y + t.y, -2.0f * dot * dir->z + t.z);
		qn_vec3_mag(pv, pv, len);
		return true;
	}
}

/*!
 * @brief 면과 선분 충돌 여부
 * @param pv 충돌 지점
 * @param plane 면
 * @param loc 선의 시작
 * @param dir 선의 방량
 * @return 충돌하면 참
*/
bool qn_vec3_intersect_line(qnVec3* pv, const qnPlane* plane, const qnVec3* loc, const qnVec3* dir)
{
	// v2->pl<-v1
	float dot = qn_vec3_dot((const qnVec3*)plane, dir);
	if (!dot)
	{
		qn_vec3_rst(pv);
		return false;
	}
	else
	{
		float tmp = (plane->d + qn_vec3_dot((const qnVec3*)plane, loc)) / dot;
		pv->x = loc->x - tmp * dir->x;
		pv->y = loc->y - tmp * dir->y;
		pv->z = loc->z - tmp * dir->z;
		return true;
	}
}

/*!
 * @brief 두 벡터 사이의 선분과 면의 충돌 여부
 * @param pv 충돌 지점
 * @param plane 면
 * @param v1 벡터1
 * @param v2 벡터2
 * @return 충돌하면 참
*/
bool qn_vec3_intersect_point(qnVec3* pv, const qnPlane* plane, const qnVec3* v1, const qnVec3* v2)
{
	qnVec3 dir;
	qn_vec3_sub(&dir, v2, v1);
	return qn_vec3_intersect_line(pv, plane, v1, &dir);
}

/*!
 * @brief 이거 뭔지 기억이 안난다. 뭐에 쓰던거지. 기본적으로 qn_vec3_intersect_point 에다 방향 벡터와의 거리 계산 추가
 * @param pv 충돌 지점
 * @param plane 면
 * @param v1 벡터1
 * @param v2 벡터2
 * @return 충돌하면서 방향 벡터의 거리 안쪽(?)이면 참
*/
bool qn_vec3_intersect_between_point(qnVec3* pv, const qnPlane* plane, const qnVec3* v1, const qnVec3* v2)
{
	qnVec3 dir;
	qn_vec3_sub(&dir, v2, v1);
	if (!qn_vec3_intersect_line(pv, plane, v1, &dir))
		return false;
	else
	{
		float f = qn_vec3_len_sq(&dir);
		return qn_vec3_dist_sq(pv, v1) <= f && qn_vec3_dist_sq(pv, v2) <= f;
	}
}

/*!
 * @brief 세 면이 충돌 하면 참
 * @param pv 충돌 위치
 * @param plane 기준 면
 * @param other1 대상 면1
 * @param other2 대상 면2
 * @return 충돌하면 참
*/
bool qn_vec3_intersect_planes(qnVec3* pv, const qnPlane* plane, const qnPlane* other1, const qnPlane* other2)
{
	qnVec3 dir, loc;
	return (qn_plane_intersect(plane, &loc, &dir, other1)) ? qn_vec3_intersect_line(pv, other2, &loc, &dir) : false;
}

/*!
 * @brief 사원수의 스플라인 lerp
 * @param pq 반환 사원수
 * @param left 기준 사원수
 * @param right 대상 사원수
 * @param change 변화량
*/
void qn_quat_slerp(qnQuat* pq, const qnQuat* left, const qnQuat* right, float change)
{
	float fc = qn_quat_dot(left, right);

	qnQuat q1, q2;
	if (fc < 0.0f)
	{
		qn_quat_ivt(&q1, left);
		q2 = *right;
		fc = -fc;
	}
	else
	{
		q1 = *left;
		q2 = *right;
	}

	float f1, f2;
	if ((fc + 1.0f) > 0.05f)
	{
		if ((1.0f - fc) < 0.05f)
		{
			f1 = 1.0f - change;
			f2 = change;
		}
		else
		{
			float fl = cosf(fc);
			float fr = sinf(fl);

			f1 = sinf(fl * (1.0f - change)) / fr;
			f2 = sinf(fl * change) / fr;
		}
	}
	else
	{
		qn_quat_set(&q2, -q1.y, q1.x, -q1.w, q1.z);
		f1 = sinf((float)QN_PI * (0.5f - change));
		f2 = sinf((float)QN_PI * change);
	}

	pq->x = f1 * q1.x + f2 * q2.x;
	pq->y = f1 * q1.y + f2 * q2.y;
	pq->z = f1 * q1.z + f2 * q2.z;
	pq->w = f1 * q1.w + f2 * q2.w;
}

/*!
 * @brief 행렬로 사원수 회전
 * @param pq 반환 사원수
 * @param rot 회전할 행렬
*/
void qn_quat_mat4(qnQuat* pq, const qnMat4* rot)
{
	float diag = rot->_11 + rot->_22 + rot->_33 + 1.0f;
	float scale = 0.0f;
	float iscl = 0.0f;
	float norm;
	qnQuat q;

	if (diag > 0.0f)
	{
		// 진단값에서 안전치
		scale = sqrtf(diag) * 2.0f;
		iscl = 1.0f / scale;

		q.x = (rot->_23 - rot->_32) * iscl;
		q.y = (rot->_31 - rot->_13) * iscl;
		q.z = (rot->_12 - rot->_21) * iscl;
		q.w = 0.25f * scale;
	}
	else
	{
		// 필요한 스케일 만들기
		if (rot->_11 > rot->_22 && rot->_11 > rot->_33)
		{
			scale = sqrtf(1.0f + rot->_11 - rot->_22 - rot->_33) * 2.0f;
			iscl = 1.0f / scale;

			q.x = 0.25f * scale;
			q.y = (rot->_12 + rot->_21) * iscl;
			q.z = (rot->_31 + rot->_13) * iscl;
			q.w = (rot->_23 - rot->_32) * iscl;
		}
		else if (rot->_22 > rot->_33)
		{
			scale = sqrtf(1.0f - rot->_11 + rot->_22 - rot->_33) * 2.0f;
			iscl = 1.0f / scale;

			q.x = (rot->_12 + rot->_21) * iscl;
			q.y = 0.25f * scale;
			q.z = (rot->_23 + rot->_32) * iscl;
			q.w = (rot->_31 - rot->_13) * iscl;
		}
		else
		{
			scale = sqrtf(1.0f - rot->_11 - rot->_22 + rot->_33) * 2.0f;
			iscl = 1.0f / scale;

			q.x = (rot->_31 + rot->_13) * iscl;
			q.y = (rot->_23 + rot->_32) * iscl;
			q.z = 0.25f * scale;
			q.w = (rot->_12 - rot->_21) * iscl;
		}
	}

	norm = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (norm == 1.0f)
		*pq = q;
	else
	{
		norm = 1.0f / sqrtf(norm);
		pq->x = q.x * norm;
		pq->y = q.y * norm;
		pq->z = q.z * norm;
		pq->w = q.w * norm;
	}
}

/*!
 * @brief 벡터로 화전
 * @param pq 반환 사원수
 * @param rot 회전 행렬
*/
void qn_quat_vec(qnQuat* pq, const qnVec3* rot)
{
	float angle;
	float rs, rc;
	float ys, yc;
	float ps, pc;
	float pcyc, psyc, pcys, psys;

	angle = rot->x * 0.5f;
	qn_sincosf(angle, &rs, &rc);

	angle = rot->y * 0.5f;
	qn_sincosf(angle, &ps, &pc);

	angle = rot->z * 0.5f;
	qn_sincosf(angle, &ys, &yc);

	pcyc = pc * yc;
	psyc = ps * yc;
	pcys = pc * ys;
	psys = ps * ys;

	pq->x = rs * pcyc - rc * psys;
	pq->y = rc * psyc + rs * pcys;
	pq->z = rc * pcys + rs * psyc;
	pq->w = rc * pcyc + rs * psys;
}

/*!
 * @brief 사원수 로그
 * @param pq 반환 사원수
 * @param q 입력 사원수
*/
void qn_quat_ln(qnQuat* pq, const qnQuat* q)
{
	float n = qn_quat_len_sq(q);
	if (n > 1.0001f)
	{
		pq->x = q->x;
		pq->y = q->y;
		pq->z = q->z;
		pq->w = 0.0f;
	}
	else if (n > 0.99999f)
	{
		float nv = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z);
		float t = atan2f(nv, q->w) / nv;
		pq->x = t * q->x;
		pq->y = t * q->y;
		pq->z = t * q->z;
		pq->w = 0.0f;
	}
	else
	{
		// 법선이 1보다 작다. 이런일은 생기지 않는다!!!!
		qn_assert(false, "qn_quat_ln: normal is below 1. what???");
		pq->x = 0.0f;
		pq->y = 0.0f;
		pq->z = 0.0f;
		pq->w = 0.0f;
	}
}

/*!
 * @brief 행렬 전치
 * @param pm 반환 행렬
 * @param m 전치할 행렬
*/
void qn_mat4_tran(qnMat4* pm, const qnMat4* m)
{
#if USE_EMM_INTRIN
#if 1
	__m128 mm0 = _mm_unpacklo_ps(_mm_loadu_ps(&m->_11), _mm_loadu_ps(&m->_21));
	__m128 mm1 = _mm_unpacklo_ps(_mm_loadu_ps(&m->_31), _mm_loadu_ps(&m->_41));
	__m128 mm2 = _mm_unpackhi_ps(_mm_loadu_ps(&m->_11), _mm_loadu_ps(&m->_21));
	__m128 mm3 = _mm_unpackhi_ps(_mm_loadu_ps(&m->_31), _mm_loadu_ps(&m->_41));

	_mm_storeu_ps(&pm->_11, _mm_movelh_ps(mm0, mm1));
	_mm_storeu_ps(&pm->_21, _mm_movehl_ps(mm1, mm0));
	_mm_storeu_ps(&pm->_31, _mm_movelh_ps(mm2, mm3));
	_mm_storeu_ps(&pm->_41, _mm_movehl_ps(mm3, mm2));
#else
	__m128 mm0 = _mm_load_ps(&m->_11);
	__m128 mm1 = _mm_load_ps(&m->_21);
	__m128 mm2 = _mm_load_ps(&m->_31);
	__m128 mm3 = _mm_load_ps(&m->_41);

	__m128 sm0 = _mm_shuffle_ps(mm0, mm1, 0x44);
	__m128 sm1 = _mm_shuffle_ps(mm0, mm1, 0xEE);
	__m128 sm2 = _mm_shuffle_ps(mm2, mm3, 0x44);
	__m128 sm3 = _mm_shuffle_ps(mm2, mm3, 0xEE);

	_mm_store_ps(&pm->_11, _mm_shuffle_ps(sm0, sm1, 0x88));
	_mm_store_ps(&pm->_12, _mm_shuffle_ps(sm0, sm1, 0xDD));
	_mm_store_ps(&pm->_13, _mm_shuffle_ps(sm2, sm3, 0x88));
	_mm_store_ps(&pm->_14, _mm_shuffle_ps(sm2, sm3, 0xDD));
#endif
#else
	qnMat4 t =
	{
		._11 = m->_11,
		._12 = m->_21,
		._13 = m->_31,
		._14 = m->_41,

		._21 = m->_12,
		._22 = m->_22,
		._23 = m->_32,
		._24 = m->_42,

		._31 = m->_13,
		._32 = m->_23,
		._33 = m->_33,
		._34 = m->_43,

		._41 = m->_14,
		._42 = m->_24,
		._43 = m->_34,
		._44 = m->_44,
	};
	*pm = t;
#endif
}

/*!
 * @brief 행렬 곱
 * @param pm 반환 행렬
 * @param left 좌측 행렬
 * @param right 우측 행렬
*/
void qn_mat4_mul(qnMat4* pm, const qnMat4* left, const qnMat4* right)
{
#if USE_EMM_INTRIN
	register __m128 r1, r2;
	register __m128 b1, b2, b3, b4;

	b1 = _mm_loadu_ps(&right->_11);
	b2 = _mm_loadu_ps(&right->_21);
	b3 = _mm_loadu_ps(&right->_31);
	b4 = _mm_loadu_ps(&right->_41);

	r1 = _mm_mul_ps(_mm_set_ps1(left->_11), b1);
	r2 = _mm_mul_ps(_mm_set_ps1(left->_21), b1);
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left->_12), b2));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left->_22), b2));
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left->_13), b3));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left->_23), b3));
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left->_14), b4));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left->_24), b4));
	_mm_storeu_ps(&pm->_11, r1);
	_mm_storeu_ps(&pm->_21, r2);

	r1 = _mm_mul_ps(_mm_set_ps1(left->_31), b1);
	r2 = _mm_mul_ps(_mm_set_ps1(left->_41), b1);
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left->_32), b2));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left->_42), b2));
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left->_33), b3));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left->_43), b3));
	r1 = _mm_add_ps(r1, _mm_mul_ps(_mm_set_ps1(left->_34), b4));
	r2 = _mm_add_ps(r2, _mm_mul_ps(_mm_set_ps1(left->_44), b4));
	_mm_storeu_ps(&pm->_31, r1);
	_mm_storeu_ps(&pm->_41, r2);
#else
	qnMat4 m =
	{
		._11 = left->_11 * right->_11 + left->_12 * right->_21 + left->_13 * right->_31 + left->_14 * right->_41,
		._12 = left->_11 * right->_12 + left->_12 * right->_22 + left->_13 * right->_32 + left->_14 * right->_42,
		._13 = left->_11 * right->_13 + left->_12 * right->_23 + left->_13 * right->_33 + left->_14 * right->_43,
		._14 = left->_11 * right->_14 + left->_12 * right->_24 + left->_13 * right->_34 + left->_14 * right->_44,

		._21 = left->_21 * right->_11 + left->_22 * right->_21 + left->_23 * right->_31 + left->_24 * right->_41,
		._22 = left->_21 * right->_12 + left->_22 * right->_22 + left->_23 * right->_32 + left->_24 * right->_42,
		._23 = left->_21 * right->_13 + left->_22 * right->_23 + left->_23 * right->_33 + left->_24 * right->_43,
		._24 = left->_21 * right->_14 + left->_22 * right->_24 + left->_23 * right->_34 + left->_24 * right->_44,

		._31 = left->_31 * right->_11 + left->_32 * right->_21 + left->_33 * right->_31 + left->_34 * right->_41,
		._32 = left->_31 * right->_12 + left->_32 * right->_22 + left->_33 * right->_32 + left->_34 * right->_42,
		._33 = left->_31 * right->_13 + left->_32 * right->_23 + left->_33 * right->_33 + left->_34 * right->_43,
		._34 = left->_31 * right->_14 + left->_32 * right->_24 + left->_33 * right->_34 + left->_34 * right->_44,

		._41 = left->_41 * right->_11 + left->_42 * right->_21 + left->_43 * right->_31 + left->_44 * right->_41,
		._42 = left->_41 * right->_12 + left->_42 * right->_22 + left->_43 * right->_32 + left->_44 * right->_42,
		._43 = left->_41 * right->_13 + left->_42 * right->_23 + left->_43 * right->_33 + left->_44 * right->_43,
		._44 = left->_41 * right->_14 + left->_42 * right->_24 + left->_43 * right->_34 + left->_44 * right->_44,
	};
	*pm = m;
#endif
}

/*!
 * @brief 역행렬
 * @param pm 반환 행렬
 * @param m 입력 행렬
 * @param determinant 행렬식
*/
void qn_mat4_inv(qnMat4* pm, const qnMat4* m, float* /*NULLABLE*/determinant)
{
#if USE_EMM_INTRIN
	static const QN_ALIGN(16) uint32_t s_PNNP[4] = { 0x00000000, 0x80000000, 0x80000000, 0x00000000 };

	__m128 A, B, C, D;
	__m128 iA, iB, iC, iD;
	__m128 DC, AB;
	__m128 dA, dB, dC, dD;
	__m128 det, d, d1, d2;
	__m128 rd;
	__m128 b1, b2, b3, b4;

	b1 = _mm_loadu_ps(&m->_11);
	b2 = _mm_loadu_ps(&m->_21);
	b3 = _mm_loadu_ps(&m->_31);
	b4 = _mm_loadu_ps(&m->_41);

	A = _mm_movelh_ps(b1, b2);
	B = _mm_movehl_ps(b2, b1);
	C = _mm_movelh_ps(b3, b4);
	D = _mm_movehl_ps(b4, b3);

	AB = _mm_mul_ps(_mm_shuffle_ps(A, A, 0x0F), B);
	AB = _mm_sub_ps(AB, _mm_mul_ps(_mm_shuffle_ps(A, A, 0xA5), _mm_shuffle_ps(B, B, 0x4E)));
	DC = _mm_mul_ps(_mm_shuffle_ps(D, D, 0x0F), C);
	DC = _mm_sub_ps(DC, _mm_mul_ps(_mm_shuffle_ps(D, D, 0xA5), _mm_shuffle_ps(C, C, 0x4E)));

	dA = _mm_mul_ps(_mm_shuffle_ps(A, A, 0x5F), A);
	dA = _mm_sub_ss(dA, _mm_movehl_ps(dA, dA));
	dB = _mm_mul_ps(_mm_shuffle_ps(B, B, 0x5F), B);
	dB = _mm_sub_ss(dB, _mm_movehl_ps(dB, dB));

	dC = _mm_mul_ps(_mm_shuffle_ps(C, C, 0x5F), C);
	dC = _mm_sub_ss(dC, _mm_movehl_ps(dC, dC));
	dD = _mm_mul_ps(_mm_shuffle_ps(D, D, 0x5F), D);
	dD = _mm_sub_ss(dD, _mm_movehl_ps(dD, dD));

	d = _mm_mul_ps(_mm_shuffle_ps(DC, DC, 0xD8), AB);

	iD = _mm_mul_ps(_mm_shuffle_ps(C, C, 0xA0), _mm_movelh_ps(AB, AB));
	iD = _mm_add_ps(iD, _mm_mul_ps(_mm_shuffle_ps(C, C, 0xF5), _mm_movehl_ps(AB, AB)));
	iA = _mm_mul_ps(_mm_shuffle_ps(B, B, 0xA0), _mm_movelh_ps(DC, DC));
	iA = _mm_add_ps(iA, _mm_mul_ps(_mm_shuffle_ps(B, B, 0xF5), _mm_movehl_ps(DC, DC)));

	d = _mm_add_ps(d, _mm_movehl_ps(d, d));
	d = _mm_add_ss(d, _mm_shuffle_ps(d, d, 1));
	d1 = _mm_mul_ps(dA, dD);
	d2 = _mm_mul_ps(dB, dC);

	iD = _mm_sub_ps(_mm_mul_ps(D, _mm_shuffle_ps(dA, dA, 0)), iD);
	iA = _mm_sub_ps(_mm_mul_ps(A, _mm_shuffle_ps(dD, dD, 0)), iA);

	det = _mm_sub_ss(_mm_add_ss(d1, d2), d);
	rd = _mm_div_ss(_mm_set_ss(1.0f), det);
	// ZERO_SINGULAR
	//rd=_mm_and_ps(_mm_cmpneq_ss(det,_mm_setzero_ps()),rd);

	iB = _mm_mul_ps(D, _mm_shuffle_ps(AB, AB, 0x33));
	iB = _mm_sub_ps(iB, _mm_mul_ps(_mm_shuffle_ps(D, D, 0xB1), _mm_shuffle_ps(AB, AB, 0x66)));
	iC = _mm_mul_ps(A, _mm_shuffle_ps(DC, DC, 0x33));
	iC = _mm_sub_ps(iC, _mm_mul_ps(_mm_shuffle_ps(A, A, 0xB1), _mm_shuffle_ps(DC, DC, 0x66)));

	rd = _mm_shuffle_ps(rd, rd, 0);
	rd = _mm_xor_ps(rd, *(__m128*) & s_PNNP);

	iB = _mm_sub_ps(_mm_mul_ps(C, _mm_shuffle_ps(dB, dB, 0)), iB);
	iC = _mm_sub_ps(_mm_mul_ps(B, _mm_shuffle_ps(dC, dC, 0)), iC);

	iA = _mm_div_ps(iA, rd);
	iB = _mm_div_ps(iB, rd);
	iC = _mm_div_ps(iC, rd);
	iD = _mm_div_ps(iD, rd);

	_mm_storeu_ps(&pm->_11, _mm_shuffle_ps(iA, iB, 0x77));
	_mm_storeu_ps(&pm->_21, _mm_shuffle_ps(iA, iB, 0x22));
	_mm_storeu_ps(&pm->_31, _mm_shuffle_ps(iC, iD, 0x77));
	_mm_storeu_ps(&pm->_41, _mm_shuffle_ps(iC, iD, 0x22));

	if (determinant)
		*determinant = *(float*)&det;
#else
	float f =
		(m->_11 * m->_22 - m->_21 * m->_12) * (m->_33 * m->_44 - m->_43 * m->_34) - (m->_11 * m->_32 - m->_31 * m->_12) * (m->_23 * m->_44 - m->_43 * m->_24) +
		(m->_11 * m->_42 - m->_41 * m->_12) * (m->_23 * m->_34 - m->_33 * m->_24) + (m->_21 * m->_32 - m->_31 * m->_22) * (m->_13 * m->_44 - m->_43 * m->_14) -
		(m->_21 * m->_42 - m->_41 * m->_22) * (m->_13 * m->_34 - m->_33 * m->_14) + (m->_31 * m->_42 - m->_41 * m->_32) * (m->_13 * m->_24 - m->_23 * m->_14);
	if (f == 0.0f)
	{
		if (determinant)
			*determinant = 0.0f;
		if (pm != m)
			*pm = *m;
		return;
	}

	f = 1.0f / f;
	qnMat4 t =
	{
		._11 = f * (m->_22 * (m->_33 * m->_44 - m->_43 * m->_34) + m->_32 * (m->_43 * m->_24 - m->_23 * m->_44) + m->_42 * (m->_23 * m->_34 - m->_33 * m->_24)),
		._12 = f * (m->_32 * (m->_13 * m->_44 - m->_43 * m->_14) + m->_42 * (m->_33 * m->_14 - m->_13 * m->_34) + m->_12 * (m->_43 * m->_34 - m->_33 * m->_44)),
		._13 = f * (m->_42 * (m->_13 * m->_24 - m->_23 * m->_14) + m->_12 * (m->_23 * m->_44 - m->_43 * m->_24) + m->_22 * (m->_43 * m->_14 - m->_13 * m->_44)),
		._14 = f * (m->_12 * (m->_33 * m->_24 - m->_23 * m->_34) + m->_22 * (m->_13 * m->_34 - m->_33 * m->_14) + m->_32 * (m->_23 * m->_14 - m->_13 * m->_24)),

		._21 = f * (m->_23 * (m->_31 * m->_44 - m->_41 * m->_34) + m->_33 * (m->_41 * m->_24 - m->_21 * m->_44) + m->_43 * (m->_21 * m->_34 - m->_31 * m->_24)),
		._22 = f * (m->_33 * (m->_11 * m->_44 - m->_41 * m->_14) + m->_43 * (m->_31 * m->_14 - m->_11 * m->_34) + m->_13 * (m->_41 * m->_34 - m->_31 * m->_44)),
		._23 = f * (m->_43 * (m->_11 * m->_24 - m->_21 * m->_14) + m->_13 * (m->_21 * m->_44 - m->_41 * m->_24) + m->_23 * (m->_41 * m->_14 - m->_11 * m->_44)),
		._24 = f * (m->_13 * (m->_31 * m->_24 - m->_21 * m->_34) + m->_23 * (m->_11 * m->_34 - m->_31 * m->_14) + m->_33 * (m->_21 * m->_14 - m->_11 * m->_24)),

		._31 = f * (m->_24 * (m->_31 * m->_42 - m->_41 * m->_32) + m->_34 * (m->_41 * m->_22 - m->_21 * m->_42) + m->_44 * (m->_21 * m->_32 - m->_31 * m->_22)),
		._32 = f * (m->_34 * (m->_11 * m->_42 - m->_41 * m->_12) + m->_44 * (m->_31 * m->_12 - m->_11 * m->_32) + m->_14 * (m->_41 * m->_32 - m->_31 * m->_42)),
		._33 = f * (m->_44 * (m->_11 * m->_22 - m->_21 * m->_12) + m->_14 * (m->_21 * m->_42 - m->_41 * m->_22) + m->_24 * (m->_41 * m->_12 - m->_11 * m->_42)),
		._34 = f * (m->_14 * (m->_31 * m->_22 - m->_21 * m->_32) + m->_24 * (m->_11 * m->_32 - m->_31 * m->_12) + m->_34 * (m->_21 * m->_12 - m->_11 * m->_22)),

		._41 = f * (m->_21 * (m->_42 * m->_33 - m->_32 * m->_43) + m->_31 * (m->_22 * m->_43 - m->_42 * m->_23) + m->_41 * (m->_32 * m->_23 - m->_22 * m->_33)),
		._42 = f * (m->_31 * (m->_42 * m->_13 - m->_12 * m->_43) + m->_41 * (m->_12 * m->_33 - m->_32 * m->_13) + m->_11 * (m->_32 * m->_43 - m->_42 * m->_33)),
		._43 = f * (m->_41 * (m->_22 * m->_13 - m->_12 * m->_23) + m->_11 * (m->_42 * m->_23 - m->_22 * m->_43) + m->_21 * (m->_12 * m->_43 - m->_42 * m->_13)),
		._44 = f * (m->_11 * (m->_22 * m->_33 - m->_32 * m->_23) + m->_21 * (m->_32 * m->_13 - m->_12 * m->_33) + m->_31 * (m->_12 * m->_23 - m->_22 * m->_13)),
	};
	*pm = t;

	if (determinant)
		*determinant = f;
#endif
}
/*!
 * @brief 전치곱
 * @param pm 반환 행렬
 * @param left 왼쪽 행렬
 * @param right 오른쪽 행렬
*/
void qn_mat4_tmul(qnMat4* pm, const qnMat4* left, const qnMat4* right)
{
	qnMat4 m;
	qn_mat4_mul(&m, left, right);
	qn_mat4_tran(pm, &m);
}

/*!
 * @brief 행렬식
 * @param m 행렬
 * @return 행렬식
*/
float qn_mat4_det(const qnMat4* m)
{
#if USE_EMM_INTRIN
	__m128 va, vb, vc;
	__m128 r1, r2, r3;
	__m128 t1, t2, sum;
	__m128 det;

	t1 = _mm_loadu_ps(&m->_41);
	t2 = _mm_loadu_ps(&m->_31);
	t2 = _mm_ror_ps(t2, 1);
	vc = _mm_mul_ps(t2, _mm_ror_ps(t1, 0));
	va = _mm_mul_ps(t2, _mm_ror_ps(t1, 2));
	vb = _mm_mul_ps(t2, _mm_ror_ps(t1, 3));

	r1 = _mm_sub_ps(_mm_ror_ps(va, 1), _mm_ror_ps(vc, 2));
	r2 = _mm_sub_ps(_mm_ror_ps(vb, 2), _mm_ror_ps(vb, 0));
	r3 = _mm_sub_ps(_mm_ror_ps(va, 0), _mm_ror_ps(vc, 1));

	va = _mm_loadu_ps(&m->_21);
	va = _mm_ror_ps(va, 1);	sum = _mm_mul_ps(va, r1);
	vb = _mm_ror_ps(va, 1);	sum = _mm_add_ps(sum, _mm_mul_ps(vb, r2));
	vc = _mm_ror_ps(vb, 1);	sum = _mm_add_ps(sum, _mm_mul_ps(vc, r3));

	det = _mm_mul_ps(sum, _mm_loadu_ps(&m->_11));
	det = _mm_add_ps(det, _mm_movehl_ps(det, det));
	det = _mm_sub_ss(det, _mm_shuffle_ps(det, det, 1));

	return *(((float*)&det) + 0);
#else
	float f =
		(m->_11 * m->_22 - m->_21 * m->_12) * (m->_33 * m->_44 - m->_43 * m->_34) - (m->_11 * m->_32 - m->_31 * m->_12) * (m->_23 * m->_44 - m->_43 * m->_24) +
		(m->_11 * m->_42 - m->_41 * m->_12) * (m->_23 * m->_34 - m->_33 * m->_24) + (m->_21 * m->_32 - m->_31 * m->_22) * (m->_13 * m->_44 - m->_43 * m->_14) -
		(m->_21 * m->_42 - m->_41 * m->_22) * (m->_13 * m->_34 - m->_33 * m->_14) + (m->_31 * m->_42 - m->_41 * m->_32) * (m->_13 * m->_24 - m->_23 * m->_14);
	return f == 0.0f ? 0.0f : 1.0f / f;
#endif
}

/*!
 * @brief 그림자 행렬을 만든다
 * @param pm 반환 행렬
 * @param light 빛의 방향
 * @param plane 투영될 면
*/
void qn_mat4_shadow(qnMat4* pm, const qnVec4* light, const qnPlane* plane)
{
	float d = plane->a * light->x + plane->b * light->y + plane->c * light->z + plane->d;

	if (light->w == 0.0f)
	{
		pm->_11 = -plane->a * light->x + d;
		pm->_12 = -plane->a * light->y;
		pm->_13 = -plane->a * light->z;
		pm->_14 = 0.0f;

		pm->_21 = -plane->b * light->x;
		pm->_22 = -plane->b * light->y + d;
		pm->_23 = -plane->b * light->z;
		pm->_24 = 0.0f;

		pm->_31 = -plane->c * light->x;
		pm->_32 = -plane->c * light->y;
		pm->_33 = -plane->c * light->z + d;
		pm->_34 = 0.0f;

		pm->_41 = -plane->d * light->x;
		pm->_42 = -plane->d * light->y;
		pm->_43 = -plane->d * light->z;
		pm->_44 = d;
	}
	else
	{
		pm->_11 = -plane->a * light->x + d;
		pm->_12 = -plane->a * light->y;
		pm->_13 = -plane->a * light->z;
		pm->_14 = -plane->a * light->w;

		pm->_21 = -plane->b * light->x;
		pm->_22 = -plane->b * light->y + d;
		pm->_23 = -plane->b * light->z;
		pm->_24 = -plane->b * light->w;

		pm->_31 = -plane->c * light->x;
		pm->_32 = -plane->c * light->y;
		pm->_33 = -plane->c * light->z + d;
		pm->_34 = -plane->c * light->w;

		pm->_41 = -plane->d * light->x;
		pm->_42 = -plane->d * light->y;
		pm->_43 = -plane->d * light->z;
		pm->_44 = -plane->d * light->w + d;
	}
}

/*!
 * @brief 아핀 변환 행렬
 * @param pm 반환 행렬
 * @param scl 스케일
 * @param rotcenter 회전축(원점일 경우 NULL)
 * @param rot 회전
 * @param loc 위치
*/
void qn_mat4_affine(qnMat4* pm, const qnVec3* scl, const qnVec3* rotcenter, const qnQuat* rot, const qnVec3* loc)
{
	qnMat4 m1, m2, m3, m4, m5, p1, p2, p3;

	if (scl)
		qn_mat4_scl(&m1, scl->x, scl->y, scl->z, true);
	else
		qn_mat4_rst(&m1);

	if (rotcenter)
	{
		qn_mat4_loc(&m2, -rotcenter->x, -rotcenter->y, -rotcenter->z, true);
		qn_mat4_loc(&m4, rotcenter->x, rotcenter->y, rotcenter->z, true);
	}
	else
	{
		qn_mat4_rst(&m2);
		qn_mat4_rst(&m4);
	}

	if (rot)
		qn_mat4_quat(&m3, rot, NULL);
	else
		qn_mat4_rst(&m3);

	if (loc)
		qn_mat4_loc(&m5, loc->x, loc->y, loc->z, true);
	else
		qn_mat4_rst(&m5);

	qn_mat4_mul(&p1, &m1, &m2);
	qn_mat4_mul(&p2, &p1, &m3);
	qn_mat4_mul(&p3, &p2, &m4);
	qn_mat4_mul(pm, &p3, &m5);
}

/*!
 * @brief 행렬 트랜스폼
 * @param m 반환 행렬
 * @param loc 위치
 * @param rot 회전
 * @param scl 스케일 (1일 경우 NULL)
*/
void qn_mat4_trfm(qnMat4* m, const qnVec3* loc, const qnQuat* rot, const qnVec3* scl)
{
	float* f = m->_m;

	qn_mat4_quat(m, rot, NULL);

	f[0] += loc->x * f[3];
	f[1] += loc->y * f[3];
	f[2] += loc->z * f[3];
	f[4] += loc->x * f[7];
	f[5] += loc->y * f[7];
	f[6] += loc->z * f[7];
	f[8] += loc->x * f[11];
	f[9] += loc->y * f[11];
	f[10] += loc->z * f[11];
	f[12] += loc->x * f[15];
	f[13] += loc->y * f[15];
	f[14] += loc->z * f[15];

	if (scl)
	{
		f[0] *= scl->x;
		f[1] *= scl->x;
		f[2] *= scl->x;
		f[3] *= scl->x;
		f[4] *= scl->y;
		f[5] *= scl->y;
		f[6] *= scl->y;
		f[7] *= scl->y;
		f[8] *= scl->z;
		f[9] *= scl->z;
		f[10] *= scl->z;
		f[11] *= scl->z;
	}
}

/*!
 * @brief 행렬 트랜스폼. 단 벡터 회전
 * @param m 반환 행렬
 * @param loc 위치
 * @param rot 회전
 * @param scl 스케일 (1일 경우 NULL)
*/
void qn_mat4_trfm_vec(qnMat4* m, const qnVec3* loc, const qnVec3* rot, const qnVec3* scl)
{
	float* f = m->_m;

	qn_mat4_vec(m, rot, NULL);

	f[0] += loc->x * f[3];
	f[1] += loc->y * f[3];
	f[2] += loc->z * f[3];
	f[4] += loc->x * f[7];
	f[5] += loc->y * f[7];
	f[6] += loc->z * f[7];
	f[8] += loc->x * f[11];
	f[9] += loc->y * f[11];
	f[10] += loc->z * f[11];
	f[12] += loc->x * f[15];
	f[13] += loc->y * f[15];
	f[14] += loc->z * f[15];

	if (scl)
	{
		f[0] *= scl->x;
		f[1] *= scl->x;
		f[2] *= scl->x;
		f[3] *= scl->x;
		f[4] *= scl->y;
		f[5] *= scl->y;
		f[6] *= scl->y;
		f[7] *= scl->y;
		f[8] *= scl->z;
		f[9] *= scl->z;
		f[10] *= scl->z;
		f[11] *= scl->z;
	}
}

/*!
 * @brief 면 트랜스폼
 * @param pp 반환 면
 * @param plane 대상 면
 * @param trfm 트랜스폼 행렬
*/
void qn_plane_trfm(qnPlane* pp, const qnPlane* plane, const qnMat4* trfm)
{
	qnVec3 v, n, s;

	qn_vec3_mag(&v, (const qnVec3*)plane, -plane->d);
	qn_vec3_trfm(&v, &v, trfm);
	qn_vec3_norm(&n, (const qnVec3*)plane);
	qn_vec3_set(&s, trfm->_11, trfm->_22, trfm->_33);

	if (!qn_eqf(s.x, 0.0f) && !qn_eqf(s.y, 0.0f) && !qn_eqf(s.z, 0.0f) && (qn_eqf(s.x, 1.0f) || qn_eqf(s.y, 1.0f) || qn_eqf(s.z, 1.0f)))
	{
		n.x = n.x / (s.x * s.x);
		n.y = n.y / (s.y * s.y);
		n.z = n.z / (s.z * s.z);
	}

	qn_vec3_trfm_norm(&n, &n, trfm);
	qn_vec3_norm((qnVec3*)pp, &n);
	pp->d = -qn_vec3_dot(&v, (const qnVec3*)pp);
}

/*!
 * @brief 점 세개로 평면을 만든다
 * @param pp 반환 면
 * @param v1 점1
 * @param v2 점2
 * @param v3 점3
*/
void qn_plane_points(qnPlane* pp, const qnVec3* v1, const qnVec3* v2, const qnVec3* v3)
{
	qnVec3 t0, t1, t2;
	qn_vec3_sub(&t0, v2, v1);
	qn_vec3_sub(&t1, v3, v2);
	qn_vec3_cross(&t2, &t0, &t1);
	qn_vec3_norm(&t2, &t2);
	qn_plane_set(pp, t2.x, t2.y, t2.z, -qn_vec3_dot(v1, &t2));
}

/*!
 * @brief 벡터와 면의 충돌 평면을 만든다
 * @param p 반환 면
 * @param loc 시작 벡터
 * @param dir 방향 벡터
 * @param o 대상 평면
 * @return 만들 수 있으면 TRUE
*/
bool qn_plane_intersect(const qnPlane* p, qnVec3* loc, qnVec3* dir, const qnPlane* o)
{
	float f0 = qn_vec3_len((const qnVec3*)p);
	float f1 = qn_vec3_len((const qnVec3*)o);
	float f2 = qn_vec3_dot((const qnVec3*)p, (const qnVec3*)o);
	float det = f0 * f1 - f2 * f2;
	if (qn_absf(det) < QN_EPSILON)
		return false;

	float inv = 1.0f / det;
	float fa = (f1 * -p->d + f2 * o->d) * inv;
	float fb = (f0 * -o->d + f2 * p->d) * inv;
	qn_vec3_cross(dir, (const qnVec3*)p, (const qnVec3*)o);
	qn_vec3_set(loc, p->a * fa + o->a * fb, p->b * fa + o->b * fb, p->c * fa + o->c * fb);
	return true;
}

/*!
 * @brief 구와 충돌하는 선 판정
 * @param p 처리할 선
 * @param org 구의 중점
 * @param rad 구의 반지름
 * @param dist 충돌 거리
 * @return 충돌하면 true
*/
bool qn_line3_intersect_sphere(const qnLine3* p, const qnVec3* org, float rad, float* dist)
{
	qnVec3 t;
	qn_vec3_sub(&t, org, &p->begin);
	float c = qn_vec3_len(&t);

	qnVec3 v;
	qn_line3_vec(p, &v);
	qn_vec3_norm(&v, &v);
	float z = qn_vec3_dot(&t, &v);
	float d = rad * rad - (c * c - z * z);

	if (d < 0.0f)
		return false;

	if (dist)
		*dist = z - sqrtf(d);
	return true;
}
