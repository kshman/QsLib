//
// qm_math.c - 수학 함수
// 2023-12-27 by kim
//

#include "pch.h"
#include "qs_qn.h"
#include "qs_math.h"

#if defined QM_USE_SSE
#define _mm_ror_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((byte)((i)+3)%4,(byte)((i)+2)%4,(byte)((i)+1)%4,(byte)((i)+0)%4))) : (vec))
#define _mm_rol_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((byte)(7-(i))%4,(byte)(6-(i))%4,(byte)(5-(i))%4,(byte)(4-(i))%4))) : (vec))		// NOLINT

// SEE 벡터4 외적
void qm_sse_vec4_cross(QmVec4* pv, const QmVec4* v1, const QmVec4* v2, const QmVec4* v3)
{
	__m128 a_yzx = _mm_shuffle_ps(v1->m128, v1->m128, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 b_zxy = _mm_shuffle_ps(v2->m128, v2->m128, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 a_zxy = _mm_shuffle_ps(v1->m128, v1->m128, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 b_yzx = _mm_shuffle_ps(v2->m128, v2->m128, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 a_yzx_b_zxy = _mm_mul_ps(a_yzx, b_zxy);
	__m128 a_zxy_b_yzx = _mm_mul_ps(a_zxy, b_yzx);
	__m128 ab = _mm_sub_ps(a_yzx_b_zxy, a_zxy_b_yzx);
	__m128 c_yzx = _mm_shuffle_ps(v3->m128, v3->m128, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 ab_zxy = _mm_shuffle_ps(ab, ab, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 ab_xzy = _mm_shuffle_ps(ab, ab, _MM_SHUFFLE(3, 2, 0, 1));
	__m128 c_zxy = _mm_shuffle_ps(v3->m128, v3->m128, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 c_xzy = _mm_shuffle_ps(v3->m128, v3->m128, _MM_SHUFFLE(3, 2, 0, 1));
	__m128 a_yzx_b_zxy_c_zxy = _mm_mul_ps(a_yzx_b_zxy, c_zxy);
	__m128 a_zxy_b_yzx_c_yzx = _mm_mul_ps(a_zxy_b_yzx, c_yzx);
	__m128 ab_yzx_c_zxy = _mm_mul_ps(ab, c_zxy);
	__m128 ab_zxy_c_yzx = _mm_mul_ps(ab_zxy, c_yzx);
	__m128 ab_xzy_c_xzy = _mm_mul_ps(ab_xzy, c_xzy);
	__m128 abc = _mm_add_ps(a_yzx_b_zxy_c_zxy, a_zxy_b_yzx_c_yzx);
	pv->m128 = _mm_sub_ps(abc, _mm_add_ps(ab_yzx_c_zxy, _mm_add_ps(ab_zxy_c_yzx, ab_xzy_c_xzy)));
}

// SSE 행렬곱
void qm_sse_mat4_mul(QmMat4* pm, const QmMat4* left, const QmMat4* right)
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
void qm_sse_mat4_inv(QmMat4* pm, const QmMat4* m)
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

#if defined QM_USE_NEON
// NEON 벡터4 외적
void qm_neon_vec4_cross(QmVec4* pv, const QmVec4* v1, const QmVec4* v2, const QmVec4* v3)
{
	float32x4_t a_yzx = vextq_f32(v1->neon, v1->neon, 1);
	float32x4_t b_zxy = vextq_f32(v2->neon, v2->neon, 2);
	float32x4_t a_zxy = vextq_f32(v1->neon, v1->neon, 2);
	float32x4_t b_yzx = vextq_f32(v2->neon, v2->neon, 1);
	float32x4_t a_yzx_b_zxy = vmulq_f32(a_yzx, b_zxy);
	float32x4_t a_zxy_b_yzx = vmulq_f32(a_zxy, b_yzx);
	float32x4_t ab = vsubq_f32(a_yzx_b_zxy, a_zxy_b_yzx);
	float32x4_t c_yzx = vextq_f32(v3->neon, v3->neon, 1);
	float32x4_t ab_zxy = vextq_f32(ab, ab, 2);
	float32x4_t ab_xzy = vextq_f32(ab, ab, 1);
	float32x4_t c_zxy = vextq_f32(v3->neon, v3->neon, 2);
	float32x4_t c_xzy = vextq_f32(v3->neon, v3->neon, 1);
	float32x4_t ab_yzx_c_zxy = vmulq_f32(ab, c_zxy);
	float32x4_t ab_zxy_c_yzx = vmulq_f32(ab_zxy, c_yzx);
	float32x4_t ab_xzy_c_xzy = vmulq_f32(ab_xzy, c_xzy);
	float32x4_t abc = vaddq_f32(ab_yzx_c_zxy, ab_zxy_c_yzx);
	pv->neon = vsubq_f32(abc, ab_xzy_c_xzy);
}

// NEON 행렬곱
void qm_neon_mat4_mul(QmMat4* pm, const QmMat4* left, const QmMat4* right)
{
	float32x4_t l0 = left->neon[0];
	float32x4_t l1 = left->neon[1];
	float32x4_t l2 = left->neon[2];
	float32x4_t l3 = left->neon[3];
	for (int i = 0; i < 4; i++)
	{
		float32x4_t v, r = right->neon[i];
		v = vmulq_n_f32(l0, vgetq_lane_f32(r, 0));
		v = vmlaq_n_f32(v, l1, vgetq_lane_f32(r, 1));
		v = vmlaq_n_f32(v, l2, vgetq_lane_f32(r, 2));
		v = vmlaq_n_f32(v, l3, vgetq_lane_f32(r, 3));
		pm->neon[i] = v;
	}
}

// NEON 행렬식
float qm_neon_mat4_det(const QmMat4* m)
{
	float32x4_t row1 = m->neon[0];
	float32x4_t row2 = m->neon[1];
	float32x4_t row3 = m->neon[2];
	float32x4_t row4 = m->neon[3];

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

	return vgetq_lane_f32(det, 0);
}

// NEON 역행렬
void qm_neon_mat4_inv(QmMat4* pm, const QmMat4* m)
{
	float32x4_t row1 = m->neon[0];
	float32x4_t row2 = m->neon[1];
	float32x4_t row3 = m->neon[2];
	float32x4_t row4 = m->neon[3];

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

	pm->neon[0] = vmulq_f32(det, minor0);
	pm->neon[1] = vmulq_f32(det, minor1);
	pm->neon[2] = vmulq_f32(det, minor2);
	pm->neon[3] = vmulq_f32(det, minor3);
}
#endif // QM_USE_NEON

/// @brief 행렬로 사원수 회전
/// @param pq 반환 사원수
/// @param rot 회전할 행렬
void qm_quat_mat_extend(QmQuat* pq, const QmMat4* rot)
{
	const float diag = rot->_11 + rot->_22 + rot->_33 + 1.0f;
	QmQuat q;

	if (diag > 0.0f)
	{
		// 진단값에서 안전치
		const float scale = sqrtf(diag) * 2.0f;
		const float iscl = 1.0f / scale;

		q.X = (rot->_23 - rot->_32) * iscl;
		q.Y = (rot->_31 - rot->_13) * iscl;
		q.Z = (rot->_12 - rot->_21) * iscl;
		q.W = 0.25f * scale;
	}
	else
	{
		// 필요한 스케일 만들기
		const float scale = sqrtf(1.0f + rot->_11 - rot->_22 - rot->_33) * 2.0f;
		const float iscl = 1.0f / scale;

		if (rot->_11 > rot->_22 && rot->_11 > rot->_33)
		{
			q.X = 0.25f * scale;
			q.Y = (rot->_12 + rot->_21) * iscl;
			q.Z = (rot->_31 + rot->_13) * iscl;
			q.W = (rot->_23 - rot->_32) * iscl;
		}
		else if (rot->_22 > rot->_33)
		{
			q.X = (rot->_12 + rot->_21) * iscl;
			q.Y = 0.25f * scale;
			q.Z = (rot->_23 + rot->_32) * iscl;
			q.W = (rot->_31 - rot->_13) * iscl;
		}
		else
		{
			q.X = (rot->_31 + rot->_13) * iscl;
			q.Y = (rot->_23 + rot->_32) * iscl;
			q.Z = 0.25f * scale;
			q.W = (rot->_12 - rot->_21) * iscl;
		}
	}

	float norm = q.X * q.X + q.Y * q.Y + q.Z * q.Z + q.W * q.W;
	if (norm == 1.0f)
		*pq = q;
	else
	{
		norm = 1.0f / sqrtf(norm);
		pq->X = q.X * norm;
		pq->Y = q.Y * norm;
		pq->Z = q.Z * norm;
		pq->W = q.W * norm;
	}
}
