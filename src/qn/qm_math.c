//
// qm_math.c - 수학 함수
// 2023-12-27 by kim
//

#include "pch.h"
#include "qs_qn.h"
#include "qs_math.h"
#if defined __INTRIN_H_ || defined _INCLUDED_MM2 || defined __XMMINTRIN_H || defined _XMMINTRIN_H_INCLUDED
#define USE_EMM_INTRIN		1
#endif

#if defined QM_USE_SSE
#define _mm_ror_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((byte)((i)+3)%4,(byte)((i)+2)%4,(byte)((i)+1)%4,(byte)((i)+0)%4))) : (vec))
#define _mm_rol_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((byte)(7-(i))%4,(byte)(6-(i))%4,(byte)(5-(i))%4,(byte)(4-(i))%4))) : (vec))		// NOLINT

// SSE 행렬곱
void qm_sse_mat4_mul(QmMat4* pm, const QmMat4* restrict left, const QmMat4* restrict right)
{
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
}

// SSE 행렬식
float qm_sse_mat4_det(const QmMat4* m)
{
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
}

// SSE 역행렬
void qm_sse_mat4_inv(QmMat4* restrict pm, const QmMat4* restrict m)
{
	static const QN_ALIGN(16) uint PNNP[] = { 0x00000000, 0x80000000, 0x80000000, 0x00000000 };		// NOLINT

	__m128 a, b, c, d;
	__m128 ia, ib, ic, id;
	__m128 dc, ab;
	__m128 ma, mb, mc, md;
	__m128 dt, d0, d1, d2;
	__m128 rd;
	__m128 b1, b2, b3, b4;

	b1 = _mm_loadu_ps(&m->_11);
	b2 = _mm_loadu_ps(&m->_21);
	b3 = _mm_loadu_ps(&m->_31);
	b4 = _mm_loadu_ps(&m->_41);

	a = _mm_movelh_ps(b1, b2);
	b = _mm_movehl_ps(b2, b1);
	c = _mm_movelh_ps(b3, b4);
	d = _mm_movehl_ps(b4, b3);

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
	rd = _mm_xor_ps(rd, *(__m128*)(&PNNP));  // NOLINT

	ib = _mm_sub_ps(_mm_mul_ps(c, _mm_shuffle_ps(mb, mb, 0)), ib);
	ic = _mm_sub_ps(_mm_mul_ps(b, _mm_shuffle_ps(mc, mc, 0)), ic);

	ia = _mm_div_ps(ia, rd);
	ib = _mm_div_ps(ib, rd);
	ic = _mm_div_ps(ic, rd);
	id = _mm_div_ps(id, rd);

	_mm_storeu_ps(&pm->_11, _mm_shuffle_ps(ia, ib, 0x77));
	_mm_storeu_ps(&pm->_21, _mm_shuffle_ps(ia, ib, 0x22));
	_mm_storeu_ps(&pm->_31, _mm_shuffle_ps(ic, id, 0x77));
	_mm_storeu_ps(&pm->_41, _mm_shuffle_ps(ic, id, 0x22));

	// 행렬식: *(float*)&dt
}

#endif // QM_USE_SSE

//
void qm_vec3_closed_line(QmVec3* restrict pv,
	const QmLine3* restrict line,
	const QmVec3* restrict loc)
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
void qm_vec3_lerp_len(QmVec3* restrict pv,
	const QmVec3* restrict left,
	const QmVec3* restrict right,
	const float scale, const float len)
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
void qm_vec3_form_norm(QmVec3* restrict pv,
	const QmVec3* restrict v0,
	const QmVec3* restrict v1,
	const QmVec3* restrict v2)
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
bool qm_vec3_intersect_line(QmVec3* restrict pv,
	const QmPlane* restrict plane,
	const QmVec3* restrict loc,
	const QmVec3* restrict dir)
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
bool qm_vec3_intersect_point(QmVec3* restrict pv,
	const QmPlane* restrict plane,
	const QmVec3* restrict v1,
	const QmVec3* restrict v2)
{
	QmVec3 dir;
	qm_sub(&dir, v2, v1);
	return qm_vec3_intersect_line(pv, plane, v1, &dir);
}

//
bool qm_vec3_intersect_between_point(QmVec3* restrict pv,
	const QmPlane* restrict plane,
	const QmVec3* restrict v1,
	const QmVec3* restrict v2)
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
bool qm_vec3_intersect_planes(QmVec3* restrict pv,
	const QmPlane* restrict plane,
	const QmPlane* restrict other1,
	const QmPlane* restrict other2)
{
	QmVec3 dir, loc;
	return (qm_plane_intersect(plane, &loc, &dir, other1)) ? qm_vec3_intersect_line(pv, other2, &loc, &dir) : false;
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
		qn_assert(false && "법선이 1보다 작은데? 어째서???");
		pq->x = 0.0f;
		pq->y = 0.0f;
		pq->z = 0.0f;
		pq->w = 0.0f;
	}
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
void qm_mat4_affine(QmMat4* restrict pm,
	const QmVec3* restrict scl,
	const QmVec3* restrict rotcenter,
	const QmQuat* restrict rot,
	const QmVec3* restrict loc)
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
void qm_mat4_trfm(QmMat4* restrict m,
	const QmVec3* restrict loc,
	const QmQuat* restrict rot,
	const QmVec3* restrict scl)
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
void qm_mat4_trfm_vec(QmMat4* restrict m,
	const QmVec3* restrict loc,
	const QmVec3* restrict rot,
	const QmVec3* restrict scl)
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
void qm_plane_points(QmPlane* restrict pp,
	const QmVec3* restrict v1,
	const QmVec3* restrict v2,
	const QmVec3* restrict v3)
{
	QmVec3 t0, t1, t2;
	qm_sub(&t0, v2, v1);
	qm_sub(&t1, v3, v2);
	qm_vec3_cross(&t2, &t0, &t1);
	qm_norm(&t2, &t2);
	qm_set4(pp, t2.x, t2.y, t2.z, -qm_vec3_dot(v1, &t2));
}

//
bool qm_plane_intersect(const QmPlane* restrict p,
	QmVec3* restrict loc,
	QmVec3* restrict dir,
	const QmPlane* restrict o)
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
bool qm_line3_intersect_sphere(const QmLine3* restrict p,
	const QmVec3* restrict org,
	const float rad, float* dist)
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
