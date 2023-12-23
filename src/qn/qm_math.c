﻿#include "pch.h"
#include "qs_qn.h"
#include "qs_math.h"
#if defined __INTRIN_H_ || defined _INCLUDED_MM2 || defined __XMMINTRIN_H || defined _XMMINTRIN_H_INCLUDED
#define USE_EMM_INTRIN		1
#endif

#ifdef USE_EMM_INTRIN
#define _mm_ror_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((byte)((i)+3)%4,(byte)((i)+2)%4,(byte)((i)+1)%4,(byte)((i)+0)%4))) : (vec))  // NOLINT
#define _mm_rol_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((byte)(7-(i))%4,(byte)(6-(i))%4,(byte)(5-(i))%4,(byte)(4-(i))%4))) : (vec))  // NOLINT
#endif

//
void qm_vec3_closed_line(QmVec3* restrict pv, const QmLine3* restrict line, const QmVec3* restrict loc)
{
	QmVec3 c, z;
	qm_sub(&c, loc, &line->begin);
	qm_sub(&z, &line->end, &line->begin);
	const float d = qm_vec3_len(&z);
	qm_mag(&z, &z, 1.0f / d);
	const float t = qm_vec3_dot(&z, &c);

	if (t < 0.0f)
		*pv = line->begin;
	else if (t > d)
		*pv = line->end;
	else
	{
		qm_mag(&z, &z, t);
		qm_add(pv, &line->begin, &z);
	}
}

//
void qm_vec3_lerp_len(QmVec3* restrict pv, const QmVec3* restrict left, const QmVec3* restrict right, float scale, float len)
{
	QmVec3 v0, v1;
	qm_norm(&v0, left);
	qm_norm(&v1, right);
	pv->x = v0.x + (v1.x - v0.x) * scale;
	pv->y = v0.y + (v1.y - v0.y) * scale;
	pv->z = v0.z + (v1.z - v0.z) * scale;
	qm_mag(pv, pv, len);
}

//
void qm_vec3_form_norm(QmVec3* restrict pv, const QmVec3* restrict v0, const QmVec3* restrict v1, const QmVec3* restrict v2)
{
	QmVec3 t0, t1;
	qm_sub(&t0, v1, v0);
	qm_sub(&t1, v2, v1);
	qm_vec3_cross(pv, &t0, &t1);
	qm_mag(pv, pv, qm_vec3_len(pv));
}

//
bool qm_vec3_reflect(QmVec3* restrict pv, const QmVec3* restrict in, const QmVec3* restrict dir)
{
	const float len = qm_vec3_len(in);
	QmVec3 t;
	float dot;
	if (qm_eq(len, 0.0f))
		qm_rst(&t);
	else
	{
		dot = 1.0f / len;
		qm_mag(&t, in, dot);
	}
	dot = qm_dot(&t, dir);
	if (dot + QM_EPSILON > 0.0f)
		return false;
	else
	{
		qm_set3(pv, -2.0f * dot * dir->x + t.x, -2.0f * dot * dir->y + t.y, -2.0f * dot * dir->z + t.z);
		qm_mag(pv, pv, len);
		return true;
	}
}

//
bool qm_vec3_intersect_line(QmVec3* restrict pv, const QmPlane* restrict plane, const QmVec3* restrict loc, const QmVec3* restrict dir)
{
	// v2->pl<-v1
	const QmVec3* plvec= (const QmVec3*)plane;
	const float dot = qm_dot(plvec, dir);
	if (qm_eq(dot, 0.0f))
	{
		qm_rst(pv);
		return false;
	}
	else
	{
		const float tmp = (plane->d + qm_dot(plvec, loc)) / dot;
		pv->x = loc->x - tmp * dir->x;
		pv->y = loc->y - tmp * dir->y;
		pv->z = loc->z - tmp * dir->z;
		return true;
	}
}

//
bool qm_vec3_intersect_point(QmVec3* restrict pv, const QmPlane* restrict plane, const QmVec3* restrict v1, const QmVec3* restrict v2)
{
	QmVec3 dir;
	qm_sub(&dir, v2, v1);
	return qm_vec3_intersect_line(pv, plane, v1, &dir);
}

//
bool qm_vec3_intersect_between_point(QmVec3* restrict pv, const QmPlane* restrict plane, const QmVec3* restrict v1, const QmVec3* restrict v2)
{
	QmVec3 dir;
	qm_sub(&dir, v2, v1);
	if (!qm_vec3_intersect_line(pv, plane, v1, &dir))
		return false;
	else
	{
		const float f = qm_len_sq(&dir);
		return qm_dist_sq(pv, v1) <= f && qm_dist_sq(pv, v2) <= f;
	}
}

//
bool qm_vec3_intersect_planes(QmVec3* restrict pv, const QmPlane* restrict plane, const QmPlane* restrict other1, const QmPlane* restrict other2)
{
	QmVec3 dir, loc;
	return (qm_plane_intersect(plane, &loc, &dir, other1)) ? qm_vec3_intersect_line(pv, other2, &loc, &dir) : false;
}

//
void qm_quat_slerp(QmQuat* restrict pq, const QmQuat* restrict left, const QmQuat* restrict right, float change)
{
	float dot = qm_dot(left, right);

	QmQuat q1, q2;
	if (dot < 0.0f)
	{
		qm_ivt(&q1, left);
		q2 = *right;
		dot = -dot;
	}
	else
	{
		q1 = *left;
		q2 = *right;
	}

	float f1, f2;
	if ((dot + 1.0f) > 0.05f)
	{
		if ((1.0f - dot) < 0.05f)
		{
			f1 = 1.0f - change;
			f2 = change;
		}
		else
		{
			float fs, fc;
			qm_sincosf(dot, &fs, &fc);	// fs 는 sinf(fc) 일 수도 있다

			f1 = sinf(fc * (1.0f - change)) / fs;
			f2 = sinf(fc * change) / fs;
		}
	}
	else
	{
		qm_set4(&q2, -q1.y, q1.x, -q1.w, q1.z);
		f1 = sinf((float)QM_PI * (0.5f - change));
		f2 = sinf((float)QM_PI * change);
	}

	pq->x = f1 * q1.x + f2 * q2.x;
	pq->y = f1 * q1.y + f2 * q2.y;
	pq->z = f1 * q1.z + f2 * q2.z;
	pq->w = f1 * q1.w + f2 * q2.w;
}

//
void qm_quat_mat4(QmQuat* restrict pq, const QmMat4* restrict rot)
{
	const float diag = rot->_11 + rot->_22 + rot->_33 + 1.0f;
	QmQuat q;

	if (diag > 0.0f)
	{
		// 진단값에서 안전치
		const float scale = sqrtf(diag) * 2.0f;
		const float iscl = 1.0f / scale;

		q.x = (rot->_23 - rot->_32) * iscl;
		q.y = (rot->_31 - rot->_13) * iscl;
		q.z = (rot->_12 - rot->_21) * iscl;
		q.w = 0.25f * scale;
	}
	else
	{
		// 필요한 스케일 만들기
		const float scale = sqrtf(1.0f + rot->_11 - rot->_22 - rot->_33) * 2.0f;
		const float iscl = 1.0f / scale;

		if (rot->_11 > rot->_22 && rot->_11 > rot->_33)
		{
			q.x = 0.25f * scale;
			q.y = (rot->_12 + rot->_21) * iscl;
			q.z = (rot->_31 + rot->_13) * iscl;
			q.w = (rot->_23 - rot->_32) * iscl;
		}
		else if (rot->_22 > rot->_33)
		{
			q.x = (rot->_12 + rot->_21) * iscl;
			q.y = 0.25f * scale;
			q.z = (rot->_23 + rot->_32) * iscl;
			q.w = (rot->_31 - rot->_13) * iscl;
		}
		else
		{
			q.x = (rot->_31 + rot->_13) * iscl;
			q.y = (rot->_23 + rot->_32) * iscl;
			q.z = 0.25f * scale;
			q.w = (rot->_12 - rot->_21) * iscl;
		}
	}

	float norm = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
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

//
void qm_quat_vec(QmQuat* restrict pq, const QmVec3* restrict rot)
{
	float rs, rc;
	float angle = rot->x * 0.5f;
	qm_sincosf(angle, &rs, &rc);

	float ps, pc;
	angle = rot->y * 0.5f;
	qm_sincosf(angle, &ps, &pc);

	float ys, yc;
	angle = rot->z * 0.5f;
	qm_sincosf(angle, &ys, &yc);

	const float pcyc = pc * yc;
	const float psyc = ps * yc;
	const float pcys = pc * ys;
	const float psys = ps * ys;

	pq->x = rs * pcyc - rc * psys;
	pq->y = rc * psyc + rs * pcys;
	pq->z = rc * pcys + rs * psyc;
	pq->w = rc * pcyc + rs * psys;
}

//
void qm_quat_ln(QmQuat* restrict pq, const QmQuat* restrict q)
{
	const float n = qm_len_sq(q);
	if (n > 1.0001f)
	{
		pq->x = q->x;
		pq->y = q->y;
		pq->z = q->z;
		pq->w = 0.0f;
	}
	else if (n > 0.99999f)
	{
		const float nv = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z);
		const float t = atan2f(nv, q->w) / nv;
		pq->x = t * q->x;
		pq->y = t * q->y;
		pq->z = t * q->z;
		pq->w = 0.0f;
	}
	else
	{
		// 법선이 1보다 작다. 이런일은 생기지 않는다!!!!
		qn_assert(false, "법선이 1보다 작은데? 어째서???");
		pq->x = 0.0f;
		pq->y = 0.0f;
		pq->z = 0.0f;
		pq->w = 0.0f;
	}
}

//
void qm_mat4_tran(QmMat4* restrict pm, const QmMat4* restrict m)
{
#ifdef USE_EMM_INTRIN
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
	QmMat4 t =
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

//
void qm_mat4_mul(QmMat4* restrict pm, const QmMat4* restrict left, const QmMat4* restrict right)
{
#ifdef USE_EMM_INTRIN
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
	QmMat4 m =
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

//
void qm_mat4_inv_det(QmMat4* restrict pm, const QmMat4* restrict m, float* /*NULLABLE*/determinant)
{
#ifdef USE_EMM_INTRIN
	static const alignas(16) uint s_PNNP[] = { 0x00000000, 0x80000000, 0x80000000, 0x00000000 };

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
	rd = _mm_xor_ps(rd, *(__m128*)(&s_PNNP));  // NOLINT

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
	if (qm_eqf(f, 0.0f))
	{
		if (determinant)
			*determinant = 0.0f;
		if (pm != m)
			*pm = *m;
		return;
	}

	f = 1.0f / f;
	QmMat4 t =
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

//
void qm_mat4_inv(QmMat4* restrict pm, const QmMat4* restrict m)
{
	qm_mat4_inv_det(pm, m, NULL);
}

//
void qm_mat4_tmul(QmMat4* restrict pm, const QmMat4* restrict left, const QmMat4* restrict right)
{
	QmMat4 m;
	qm_mul(&m, left, right);
	qm_mat4_tran(pm, &m);
}

//
float qm_mat4_det(const QmMat4* m)
{
#ifdef USE_EMM_INTRIN
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
	return qm_eqf(f, 0.0f) ? 0.0f : 1.0f / f;
#endif
}

//
void qm_mat4_shadow(QmMat4* restrict pm, const QmVec4* restrict light, const QmPlane* restrict plane)
{
	const float d = plane->a * light->x + plane->b * light->y + plane->c * light->z + plane->d;

	if (qm_eq(light->w, 0.0f))
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

//
void qm_mat4_affine(QmMat4* restrict pm, const QmVec3* restrict scl, const QmVec3* restrict rotcenter, const QmQuat* restrict rot, const QmVec3* restrict loc)
{
	QmMat4 m1, m2, m3, m4, m5, p1, p2, p3;

	if (scl)
		qm_mat4_scl(&m1, scl->x, scl->y, scl->z, true);
	else
		qm_rst(&m1);

	if (rotcenter)
	{
		qm_mat4_loc(&m2, -rotcenter->x, -rotcenter->y, -rotcenter->z, true);
		qm_mat4_loc(&m4, rotcenter->x, rotcenter->y, rotcenter->z, true);
	}
	else
	{
		qm_rst(&m2);
		qm_rst(&m4);
	}

	if (rot)
		qm_mat4_quat(&m3, rot, NULL);
	else
		qm_rst(&m3);

	if (loc)
		qm_mat4_loc(&m5, loc->x, loc->y, loc->z, true);
	else
		qm_rst(&m5);

	qm_mul(&p1, &m1, &m2);
	qm_mul(&p2, &p1, &m3);
	qm_mul(&p3, &p2, &m4);
	qm_mul(pm, &p3, &m5);
}

//
void qm_mat4_trfm(QmMat4* restrict m, const QmVec3* restrict loc, const QmQuat* restrict rot, const QmVec3* restrict scl)
{
	float* f = m->f16;

	qm_mat4_quat(m, rot, NULL);

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

//
void qm_mat4_trfm_vec(QmMat4* restrict m, const QmVec3* restrict loc, const QmVec3* restrict rot, const QmVec3* restrict scl)
{
	float* f = m->f16;

	qm_mat4_vec(m, rot, NULL);

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

//
void qm_plane_trfm(QmPlane* restrict pp, const QmPlane* restrict plane, const QmMat4* restrict trfm)
{
	QmVec3 v, n, s;

	qm_mag(&v, (const QmVec3*)plane, -plane->d);
	qm_vec3_trfm(&v, &v, trfm);
	qm_norm(&n, (const QmVec3*)plane);
	qm_set3(&s, trfm->_11, trfm->_22, trfm->_33);

	if (!qm_eq(s.x, 0.0f) && !qm_eq(s.y, 0.0f) && !qm_eq(s.z, 0.0f) &&
		(qm_eq(s.x, 1.0f) || qm_eq(s.y, 1.0f) || qm_eq(s.z, 1.0f)))
	{
		n.x = n.x / (s.x * s.x);
		n.y = n.y / (s.y * s.y);
		n.z = n.z / (s.z * s.z);
	}

	qm_vec3_trfm_norm(&n, &n, trfm);
	qm_norm((QmVec3*)pp, &n);
	pp->d = -qm_dot(&v, (const QmVec3*)pp);
}

//
void qm_plane_points(QmPlane* restrict pp, const QmVec3* restrict v1, const QmVec3* restrict v2, const QmVec3* restrict v3)
{
	QmVec3 t0, t1, t2;
	qm_sub(&t0, v2, v1);
	qm_sub(&t1, v3, v2);
	qm_vec3_cross(&t2, &t0, &t1);
	qm_norm(&t2, &t2);
	qm_set4(pp, t2.x, t2.y, t2.z, -qm_vec3_dot(v1, &t2));
}

//
bool qm_plane_intersect(const QmPlane* restrict p, QmVec3* restrict loc, QmVec3* restrict dir, const QmPlane* restrict o)
{
	const float f0 = qm_len((const QmVec3*)p);
	const float f1 = qm_len((const QmVec3*)o);
	const float f2 = qm_dot((const QmVec3*)p, (const QmVec3*)o);
	const float det = f0 * f1 - f2 * f2;
	if (qm_abs(det) < QM_EPSILON)
		return false;

	const float inv = 1.0f / det;
	const float fa = (f1 * -p->d + f2 * o->d) * inv;
	const float fb = (f0 * -o->d + f2 * p->d) * inv;
	qm_vec3_cross(dir, (const QmVec3*)p, (const QmVec3*)o);
	qm_set3(loc, p->a * fa + o->a * fb, p->b * fa + o->b * fb, p->c * fa + o->c * fb);
	return true;
}

//
bool qm_line3_intersect_sphere(const QmLine3* restrict p, const QmVec3* restrict org, float rad, float* dist)
{
	QmVec3 t;
	qm_sub(&t, org, &p->begin);
	const float c = qm_len(&t);

	QmVec3 v;
	qm_line3_vec(p, &v);
	qm_norm(&v, &v);
	const float z = qm_dot(&t, &v);
	const float d = rad * rad - (c * c - z * z);

	if (d < 0.0f)
		return false;

	if (dist)
		*dist = z - sqrtf(d);
	return true;
}