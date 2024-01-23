//
// qm_math.c - 수학 함수
// 2023-12-27 by kim
//

#include "pch.h"

//////////////////////////////////////////////////////////////////////////
// XOR SHIFT

static nuint qm_rand_state1 = UINTPTR_MAX / 3;
static nuint qm_rand_state2 = UINTPTR_MAX / 5;

//
void qm_srand(nuint seed)
{
	if (seed == 0)
		seed = (nuint)qn_now();
	qm_rand_state1 = seed;
	qm_rand_state2 = seed % 7;
}

//
nuint qm_rand(void)
{
	qm_rand_state1 ^= qm_rand_state1 << 13;
	qm_rand_state1 ^= qm_rand_state1 >> 17;
	qm_rand_state1 ^= qm_rand_state1 << 5;
	qm_rand_state2 ^= qm_rand_state2 << 17;
	qm_rand_state2 ^= qm_rand_state2 >> 13;
	qm_rand_state2 ^= qm_rand_state2 << 7;
	return qm_rand_state1 ^ qm_rand_state2;
}

//
float qm_randf(void)
{
	return (float)qm_rand() / (float)UINTPTR_MAX;
}

//
double qm_randd(void)
{
	return (double)qm_rand() / (double)UINTPTR_MAX;
}


//////////////////////////////////////////////////////////////////////////
// 수학 함수

#if defined QM_USE_AVX
#define _mm_ror_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((byte)((i)+3)%4,(byte)((i)+2)%4,(byte)((i)+1)%4,(byte)((i)+0)%4))) : (vec))
#define _mm_rol_ps(vec,i)   (((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((byte)(7-(i))%4,(byte)(6-(i))%4,(byte)(5-(i))%4,(byte)(4-(i))%4))) : (vec))		// NOLINT

// SSE 행렬식
float QM_VECTORCALL qm_sse_mat4_det(const QmMat4 m)
{
	QMVECTOR va, vb, vc;
	QMVECTOR r1, r2, r3;
	QMVECTOR t1, t2, sum;
	QMVECTOR det;

	t1 = _mm_loadu_ps(&m._41);
	t2 = _mm_loadu_ps(&m._31);
	t2 = _mm_ror_ps(t2, 1);
	vc = _mm_mul_ps(t2, _mm_ror_ps(t1, 0));
	va = _mm_mul_ps(t2, _mm_ror_ps(t1, 2));
	vb = _mm_mul_ps(t2, _mm_ror_ps(t1, 3));

	r1 = _mm_sub_ps(_mm_ror_ps(va, 1), _mm_ror_ps(vc, 2));
	r2 = _mm_sub_ps(_mm_ror_ps(vb, 2), _mm_ror_ps(vb, 0));
	r3 = _mm_sub_ps(_mm_ror_ps(va, 0), _mm_ror_ps(vc, 1));

	va = _mm_loadu_ps(&m._21);
	va = _mm_ror_ps(va, 1);	sum = _mm_mul_ps(va, r1);
	vb = _mm_ror_ps(va, 1);	sum = _mm_add_ps(sum, _mm_mul_ps(vb, r2));
	vc = _mm_ror_ps(vb, 1);	sum = _mm_add_ps(sum, _mm_mul_ps(vc, r3));

	det = _mm_mul_ps(sum, _mm_loadu_ps(&m._11));
	det = _mm_add_ps(det, _mm_movehl_ps(det, det));
	det = _mm_sub_ss(det, _mm_shuffle_ps(det, det, 1));

	return *(((float*)&det) + 0);
}

// SSE 역행렬
QmMat4 QM_VECTORCALL qm_sse_mat4_inv(const QmMat4 m)
{
	static const ALIGNOF(16) uint PNNP[] = { 0x00000000, 0x80000000, 0x80000000, 0x00000000 };		// NOLINT

	QMVECTOR a, b, c, d;
	QMVECTOR ia, ib, ic, id;
	QMVECTOR dc, ab;
	QMVECTOR ma, mb, mc, md;
	QMVECTOR dt, d0, d1, d2;
	QMVECTOR rd;
	QMVECTOR b1, b2, b3, b4;

	b1 = _mm_loadu_ps(&m._11);
	b2 = _mm_loadu_ps(&m._21);
	b3 = _mm_loadu_ps(&m._31);
	b4 = _mm_loadu_ps(&m._41);

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
	rd = _mm_xor_ps(rd, *(QMVECTOR*)(&PNNP));  // NOLINT

	ib = _mm_sub_ps(_mm_mul_ps(c, _mm_shuffle_ps(mb, mb, 0)), ib);
	ic = _mm_sub_ps(_mm_mul_ps(b, _mm_shuffle_ps(mc, mc, 0)), ic);

	ia = _mm_div_ps(ia, rd);
	ib = _mm_div_ps(ib, rd);
	ic = _mm_div_ps(ic, rd);
	id = _mm_div_ps(id, rd);

	QmMat4 res;
	_mm_storeu_ps(&res._11, _mm_shuffle_ps(ia, ib, 0x77));
	_mm_storeu_ps(&res._21, _mm_shuffle_ps(ia, ib, 0x22));
	_mm_storeu_ps(&res._31, _mm_shuffle_ps(ic, id, 0x77));
	_mm_storeu_ps(&res._41, _mm_shuffle_ps(ic, id, 0x22));
	return res;

	// 행렬식: *(float*)&dt
}
#endif // QM_USE_AVX

#if defined QM_USE_NEON
// NEON 행렬식
float QM_VECTORCALL qm_neon_mat4_det(const QmMat4 m)
{
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

	return vgetq_lane_f32(det, 0);
}

// NEON 역행렬
QmMat4 QM_VECTORCALL qm_neon_mat4_inv(const QmMat4 m)
{
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

	QmMat4 res;
	res.v[0] = vmulq_f32(det, minor0);
	res.v[1] = vmulq_f32(det, minor1);
	res.v[2] = vmulq_f32(det, minor2);
	res.v[3] = vmulq_f32(det, minor3);
	return res;
}
#endif // QM_USE_NEON


//////////////////////////////////////////////////////////////////////////
// 지운거 남겨 둠
#if false
// 벡터 3

/// @brief 벡터3 값 설정
/// @param x,y,z 좌표
INLINE QmVec3 qm_vec3(float x, float y, float z)
{
	return (QmVec3) { {x, y, z} };
}

/// @brief 벡터3 값 설정
///	@param v 반환 벡터
/// @param x,y,z 좌표
INLINE void qm_vec3_set(QmVec3 * v, float x, float y, float z)
{
	v->X = x; v->Y = y; v->Z = z;
}

/// @brief 벡터3 초기화
INLINE QmVec3 qm_vec3_unit(void)		// identify
{
	return QMVEC3_ZERO;
}

/// @brief 벡터3 대각값 설정 (모두 같은값으로 설정)
/// @param diag 대각값
INLINE QmVec3 qm_vec3_diag(float diag)
{
	return (QmVec3) { {diag, diag, diag} };
}

/// @brief 벡터3 반전
/// @param v 원본 벡터3
INLINE QmVec3 qm_vec3_neg(QmVec3 v)  // invert
{
	return (QmVec3) { {-v.X, -v.Y, -v.Z} };
}

/// @brief 벡터3 네거티브 (1 - 값)
/// @param v 원본 벡터3
INLINE QmVec3 qm_vec3_neg(QmVec3 v)
{
	return (QmVec3) { {1.0f - v.X, 1.0f - v.Y, 1.0f - v.Z} };
}

/// @brief 벡터3 덧셈
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
INLINE QmVec3 qm_vec3_add(QmVec3 left, QmVec3 right)
{
	return (QmVec3) { {left.X + right.X, left.Y + right.Y, left.Z + right.Z} };
}

/// @brief 벡터3 뺄셈
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
INLINE QmVec3 qm_vec3_sub(QmVec3 left, QmVec3 right)
{
	return (QmVec3) { {left.X - right.X, left.Y - right.Y, left.Z - right.Z} };
}

/// @brief 벡터3 확대
/// @param left 원본 벡터3
/// @param right 확대값
INLINE QmVec3 qm_vec3_mag(QmVec3 left, float right)
{
	return (QmVec3) { {left.X * right, left.Y * right, left.Z * right} };
}

/// @brief 벡터3 줄이기
/// @param left 원본 벡터3
/// @param right 줄일값
INLINE QmVec3 qm_vec3_abr(QmVec3 left, float right)
{
	return (QmVec3) { {left.X / right, left.Y / right, left.Z / right} };
}

/// @brief 벡터3 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmVec3 qm_vec3_mul(QmVec3 left, QmVec3 right)
{
	return (QmVec3) { {left.X * right.X, left.Y * right.Y, left.Z * right.Z} };
}

/// @brief 벡터3 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmVec3 qm_vec3_div(QmVec3 left, QmVec3 right)
{
	return (QmVec3) { {left.X / right.X, left.Y / right.Y, left.Z / right.Z} };
}

/// @brief 벡터3의 최소값
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
INLINE QmVec3 qm_vec3_min(QmVec3 left, QmVec3 right)
{
	return (QmVec3) { {QN_MIN(left.X, right.X), QN_MIN(left.Y, right.Y), QN_MIN(left.Z, right.Z)} };
}

/// @brief 벡터3의 최대값
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
INLINE QmVec3 qm_vec3_max(QmVec3 left, QmVec3 right)
{
	return (QmVec3) { {QN_MAX(left.X, right.X), QN_MAX(left.Y, right.Y), QN_MAX(left.Z, right.Z)} };
}

/// @brief 두 벡터3이 같은지 판단
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3이 같으면 참
INLINE bool qm_vec3_eq(QmVec3 left, QmVec3 right)
{
	return qm_eqf(left.X, right.X) && qm_eqf(left.Y, right.Y) && qm_eqf(left.Z, right.Z);
}

/// @brief 벡터3이 0인지 판단
/// @param pv 벡터3
/// @return 벡터3이 0이면 참
INLINE bool qm_vec3_isu(QmVec3 pv)
{
	return pv.X == 0.0f && pv.Y == 0.0f && pv.Z == 0.0f;
}

/// @brief 벡터3의 내적
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 내적값
INLINE float qm_vec3_dot(QmVec3 left, QmVec3 right)
{
	return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
}

/// @brief 벡터3의 외적
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
INLINE QmVec3 qm_vec3_cross(QmVec3 left, QmVec3 right)
{
	return qm_vec3(
		left.Y * right.Z - left.Z * right.Y,
		left.Z * right.X - left.X * right.Z,
		left.X * right.Y - left.Y * right.X);
}

/// @brief 벡터3의 길이의 제곱
/// @param pv 벡터3
/// @return 길이의 제곱
INLINE float qm_vec3_len_sq(QmVec3 pv)
{
	return qm_vec3_dot(pv, pv);
}

/// @brief 벡터3의 길이
/// @param pv 벡터3
/// @return 길이
INLINE float qm_vec3_len(QmVec3 pv)
{
	return qm_sqrtf(qm_vec3_len_sq(pv));
}

/// @brief 벡터3 정규화
/// @param v 벡터3
INLINE QmVec3 qm_vec3_norm(QmVec3 v)
{
	float l = qm_vec3_len(v);
	if (l > 0.0f)
		l = 1.0f / l;
	return qm_vec3_mag(v, l);
}

/// @brief 두 벡터3 거리의 제곱
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3 거리의 제곱값
INLINE float qm_vec3_dist_sq(QmVec3 left, QmVec3 right)
{
	return qm_vec3_len_sq(qm_vec3_sub(left, right));
}

/// @brief 두 벡터3의 거리
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
/// @return 두 벡터3의 거리값
INLINE float qm_vec3_dist(QmVec3 left, QmVec3 right)
{
	return qm_sqrtf(qm_vec3_dist_sq(left, right));
}

/// @brief 벡터3 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmVec3 qm_vec3_interpolate(QmVec3 left, QmVec3 right, float scale)
{
	return qm_vec3_add(qm_vec3_mag(left, 1.0f - scale), qm_vec3_mag(right, scale));
}

/// @brief 벡터3 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmVec3 qm_vec3_lerp(QmVec3 left, QmVec3 right, float scale)
{
	return qm_vec3_add(left, qm_vec3_mag(qm_vec3_sub(right, left), scale));		// NOLINT
}

/// @brief 단위 벡터로 만들어 크기를 조정하고 길이 만큼 혼합
/// @param left 시작 벡터
/// @param right 끝 벡터
/// @param scale 크기 변화량
/// @param len 길이
INLINE QmVec3 qm_vec3_lerp_len(QmVec3 left, QmVec3 right, float scale, float len)
{
	QmVec3 r = qm_vec3_lerp(qm_vec3_norm(left), qm_vec3_norm(right), scale);
	return qm_vec3_mag(r, len);
}

/// @brief 벡터3의 방향
/// @param left 왼쪽 벡터3
/// @param right 오른쪽 벡터3
INLINE QmVec3 qm_vec3_dir(QmVec3 left, QmVec3 right)
{
	return qm_vec3_norm(qm_vec3_sub(left, right));
}

/// @brief 두 벡터3의 반지름의 제곱
/// @param left 왼쪽(가운데) 벡터3
/// @param right 오른쪽(바깥쪽) 벡터3
/// @return 두 벡터3의 반지름의 제곱값
INLINE float qm_vec3_rad_sq(QmVec3 left, QmVec3 right)
{
	return qm_vec3_len_sq(qm_vec3_sub(qm_vec3_mag(qm_vec3_add(left, right), 0.5f), left));
}

/// @brief 두 벡터3의 반지름
/// @param left 왼쪽(가운데) 벡터3
/// @param right 오른쪽(바깥쪽) 벡터3
/// @return 두 벡터3의 반지름
INLINE float qm_vec3_rad(QmVec3 left, QmVec3 right)
{
	return qm_sqrtf(qm_vec3_rad_sq(left, right));
}

/// @brief 벡터3 트랜스폼
/// @param v 벡터3
/// @param trfm 변환 행렬
INLINE QmVec3 qm_vec3_trfm(QmVec3 v, QmMat4 trfm)
{
	return qm_vec3(
		v.X * trfm._11 + v.Y * trfm._21 + v.Z * trfm._31 + trfm._41,
		v.X * trfm._12 + v.Y * trfm._22 + v.Z * trfm._32 + trfm._42,
		v.X * trfm._13 + v.Y * trfm._23 + v.Z * trfm._33 + trfm._43);
}

/// @brief 벡터3 정규화 트랜스폼
/// @param v 벡터3
/// @param trfm 정규화된 변환 행렬
INLINE QmVec3 qm_vec3_trfm_norm(QmVec3 v, QmMat4 trfm)
{
	return qm_vec3(
		v.X * trfm._11 + v.Y * trfm._21 + v.Z * trfm._31,
		v.X * trfm._12 + v.Y * trfm._22 + v.Z * trfm._32,
		v.X * trfm._13 + v.Y * trfm._23 + v.Z * trfm._33);
}

/// @brief 벡터3 사원수 회전
/// @param rot 사원수
INLINE QmVec3 qm_vec3_quat(QmQuat rot)
{
	QmQuat q = { .v = QMVECTOR_DOT4(rot.v, rot.v) };
	return qm_vec3(
		atan2f(2.0f * (rot.Y * rot.Z + rot.X * rot.W), -q.X - q.Y + q.Z + q.W),
		asinf(qm_clampf(-2.0f * (rot.X * rot.Z + rot.Y * rot.W), -1.0f, 1.0f)),
		atan2f(2.0f * (rot.X * rot.Y + rot.Z * rot.W), q.X - q.Y - q.Z + q.W));
}

/// @brief 벡터3 행렬 회전
/// @param rot 행렬
INLINE QmVec3 qm_vec3_mat4(QmMat4 rot)
{
	if (rot._31 == 0.0f && rot._33 == 0.0f)
		return qm_vec3(
			rot._32 > 0.0f ? (float)(QM_PI_H + QM_PI) : (float)QM_PI_H,
			0.0f,
			atan2f(rot._21, rot._23));
	return qm_vec3(
		-atan2f(rot._32, qm_sqrtf(rot._31 * rot._31 + rot._33 * rot._33)),
		-atan2f(rot._31, rot._33),
		atan2f(rot._12, qm_sqrtf(rot._11 * rot._11 + rot._13 * rot._13)));
}

/// @brief 사원수와 벡터3 각도로 회전
/// @param rot 사원수
/// @param angle 각도 벡터3
INLINE QmVec3 qm_vec3_quat_vec3(QmQuat rot, QmVec3 angle)
{
	QmQuat q1 = { { angle.X * rot.W, angle.Y * rot.W, angle.Z * rot.W, -qm_vec3_dot(rot.XYZ, angle) } };
	QmVec3 t = qm_vec3_cross(rot.XYZ, angle);
	q1.X += t.X;
	q1.Y += t.Y;
	q1.Z += t.Z;
	QmQuat q2 = { .v = QMVECTOR_CJG(rot.v) };
	return qm_vec3(
		q1.X * q2.W + q1.Y * q2.X - q1.Z * q2.Y + q1.W * q2.X,
		-q1.X * q2.Z + q1.Y * q2.W + q1.Z * q2.X + q1.W * q2.Y,
		q1.X * q2.Y - q1.Y * q2.X + q1.Z * q2.W + q1.W * q2.Z);
}

/// @brief 축 회전(yaw)
/// @param pv 벡터3
/// @return 축 회전 각도
INLINE float qm_vec3_yaw(QmVec3 pv)
{
	return -atanf(pv.X / pv.Y) + ((pv.X > 0.0f) ? (float)-QM_PI_H : (float)QM_PI_H);
}

/// @brief 세 벡터로 법선 벡터를 만든다
/// @param v1 벡터 1
/// @param v2 벡터 2
/// @param v3 벡터 3
INLINE QmVec3 qm_vec3_form_norm(QmVec3 v1, QmVec3 v2, QmVec3 v3)
{
	QmVec3 c = qm_vec3_cross(qm_vec3_sub(v2, v1), qm_vec3_sub(v3, v1));
	return qm_vec3_mag(c, qm_vec3_len(c));
}

/// @brief 반사 벡터를 만든다
/// @param in 입력 벡터
/// @param dir 법선 벡터
INLINE QmVec3 qm_vec3_reflect(QmVec3 in, QmVec3 dir)
{
	float len = qm_vec3_len(in);
	QmVec3 t = qm_eqf(len, 0.0f) ? qm_vec3_unit() : qm_vec3_mag(in, 1.0f / len);
	float dot = qm_vec3_dot(t, dir);
	return qm_vec3_mag(qm_vec3(-2.0f * dot * dir.X + t.X, -2.0f * dot * dir.Y + t.Y, -2.0f * dot * dir.Z + t.Z), len);
}

// 실수형 크기

/// @brief 사이즈 값 설정
/// @param width,height 너비와 높이
INLINE QmSizeF qm_sizef(float width, float height)
{
	QmSizeF s = { .Width = width, .Height = height };
	return s;
}

/// @brief 사이즈 설정
/// @param sz 좌표
INLINE QmSizeF qm_sizef_size(QmSize sz)
{
	return qm_sizef((float)sz.Width, (float)sz.Height);
}

/// @brief 사이즈 값 설정
///	@param s 반환 사이즈
/// @param width,height 너비와 높이
INLINE void qm_sizef_set(QmSizeF * s, float width, float height)
{
	s->Width = width;
	s->Height = height;
}

/// @brief 사이즈 초기화
INLINE void qm_sizef_unit(QmSizeF * s)		// identify
{
	qm_sizef_set(s, 0.0f, 0.0f);
}

/// @brief 사이즈 대각값 설정 (모두 같은값으로 설정)
///	@param s 반환 사이즈
/// @param diag 대각 값
INLINE void qm_sizef_diag(QmSizeF * s, float diag)
{
	s->Width = diag;
	s->Height = diag;
}

/// @brief 사이즈 덧셈
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
INLINE QmSizeF qm_sizef_add(QmSizeF left, QmSizeF right)
{
	return qm_sizef(left.Width + right.Width, left.Height + right.Height);
}

/// @brief 사이즈 뺄셈
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
INLINE QmSizeF qm_sizef_sub(QmSizeF left, QmSizeF right)
{
	return qm_sizef(left.Width - right.Width, left.Height - right.Height);
}

/// @brief 사이즈 확대
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 확대값
INLINE QmSizeF qm_sizef_mag(QmSizeF left, float right)
{
	return qm_sizef(left.Width * right, left.Height * right);
}

/// @brief 사이즈 항목 곱셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmSizeF qm_sizef_mul(QmSizeF left, QmSizeF right)
{
	return qm_sizef(left.Width * right.Width, left.Height * right.Height);
}

/// @brief 사이즈 항목 나눗셈
/// @param left 왼쪽 벡터
/// @param right 오른쪽 벡터
INLINE QmSizeF qm_sizef_div(QmSizeF left, QmSizeF right)
{
	return qm_sizef(left.Width / right.Width, left.Height / right.Height);
}

/// @brief 사이즈의 최소값
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
INLINE QmSizeF qm_sizef_min(QmSizeF left, QmSizeF right)
{
	return qm_sizef((left.Width < right.Width) ? left.Width : right.Width, (left.Height < right.Height) ? left.Height : right.Height);
}

/// @brief 사이즈의 최대값
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
INLINE QmSizeF qm_sizef_max(QmSizeF left, QmSizeF right)
{
	return qm_sizef((left.Width > right.Width) ? left.Width : right.Width, (left.Height > right.Height) ? left.Height : right.Height);
}

/// @brief 사이즈의 비교
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
/// @return 같으면 참
INLINE bool qm_sizef_eq(QmSizeF left, QmSizeF right)
{
	return qm_eqf(left.Width, right.Width) && qm_eqf(left.Height, right.Height);
}

/// @brief 사이즈가 0인가 비교
/// @param s 비교할 사이즈
/// @return 0이면 참
INLINE bool qm_sizef_isu(QmSizeF s)
{
	return s.Width == 0.0f && s.Height == 0.0f;
}

/// @brief 사이즈 길이의 제곱
/// @param s 사이즈
/// @return 길이의 제곱
INLINE float qm_sizef_len_sq(QmSizeF s)
{
	return s.Width * s.Width + s.Height * s.Height;
}

/// @brief 사이즈 길이
/// @param v 사이즈
/// @return 길이
INLINE float qm_sizef_len(QmSizeF v)
{
	return qm_sqrtf(qm_sizef_len_sq(v));
}

/// @brief 두 사이즈 거리의 제곱
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
/// @return 두 사이즈 거리의 제곱값
INLINE float qm_sizef_dist_sq(QmSizeF left, QmSizeF right)
{
	return qm_sizef_len_sq(qm_sizef_sub(left, right));
}

/// @brief 두 사이즈의 거리
/// @param left 왼쪽 사이즈
/// @param right 오른쪽 사이즈
/// @return 두 사이즈의 거리값
INLINE float qm_sizef_dist(QmSizeF left, QmSizeF right)
{
	return qm_sqrtf(qm_sizef_dist_sq(left, right));
}

/// @brief 사이즈 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmSizeF qm_sizef_interpolate(QmSizeF left, QmSizeF right, float scale)
{
	return qm_sizef_add(qm_sizef_mag(left, 1.0f - scale), qm_sizef_mag(right, scale));
}

/// @brief 사이즈 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 벡터
/// @param right 대상 벡터
/// @param scale 보간값
INLINE QmSizeF qm_sizef_lerp(QmSizeF left, QmSizeF right, float scale)
{
	return qm_sizef_add(left, qm_sizef_mag(qm_sizef_sub(right, left), scale));		// NOLINT
}

/// @brief 사각형으로 크기를 설정한다
/// @param rt 대상 사각형
INLINE QmSizeF qm_sizef_rect(QmRectF rt)
{
	return qm_sizef(rt.Right - rt.Left, rt.Bottom - rt.Top);
}

/// @brief 종횡비(너비 나누기 높이)를 계신한다
/// @param s 계산할 크기
/// @return 종횡비값
INLINE float qm_sizef_aspect(QmSizeF s)
{
	return s.Width / s.Height;
}

/// @brief 대각선 DPI를 구한다
/// @param pt 너비와 높이
/// @param horizontal 너비 DPI
/// @param vertical 높이 DPI
/// @return 대각선 DPI
INLINE float qm_sizef_diag_dpi(QmSizeF pt, float horizontal, float vertical)
{
	float dsq = horizontal * horizontal + vertical + vertical;
	if (dsq <= 0.0f)
		return 0.0f;
	return qm_sqrtf(pt.Width * pt.Width + pt.Height * pt.Height) / qm_sqrtf(dsq);
}

// 깊이

/// @brief 깊이 값 설정
/// @param Near,Far 깊이
INLINE QmDepth qm_depth(float Near, float Far)
{
	QmDepth s = { .Near = Near, .Far = Far };
	return s;
}

/// @brief 깊이 값 설정
///	@param d 반환 깊이
/// @param Near,Far 깊이
INLINE void qm_depth_set(QmDepth * d, float Near, float Far)
{
	d->Near = Near;
	d->Far = Far;
}


// 실수형 사각형

/// @brief 사각형 값 설정
/// @param left,top,right,bottom 사각형 요소
/// @return 만든 사각형
INLINE QmRectF qm_rectf(float left, float top, float right, float bottom)
{
	QmRectF v =
#if defined QM_USE_AVX
	{ .v = _mm_setr_ps(left, top, right, bottom) };
#elif defined QM_USE_NEON
	{.v = { left, top, right, bottom } };
#else
	{.Left = left, .Top = top, .Right = right, .Bottom = bottom };
#endif
	return v;
}

/// @brief 사각형을 좌표와 크기로 설정한다
/// @param x,y 좌표
/// @param width,height 크기
INLINE QmRectF qm_rectf_size(float x, float y, float width, float height)
{
	return qm_rectf(x, y, x + width, y + height);
}

/// @brief 사각형을 좌표와 크기 타입을 사용하여 설정한다
/// @param pos 좌표
/// @param size 크기
INLINE QmRectF qm_rectf_pos_size(QmPointF pos, QmSizeF size)
{
	return qm_rectf(pos.X, pos.Y, pos.X + size.Width, pos.Y + size.Height);
}

/// @brief 사각형 값 설정
///	@param r 반환 사각형
/// @param left,top,right,bottom 사각형 요소
INLINE void qm_rectf_set(QmRectF * r, float left, float top, float right, float bottom)
{
#if defined QM_USE_AVX
	r->m128 = _mm_setr_ps(left, top, right, bottom);
#elif defined QM_USE_NEON
	float32x4_t neon = { left, top, right, bottom };
	r->neon = neon;
#else
	r->Left = left, r->Top = top, r->Right = right, r->Bottom = bottom;
#endif
}

/// @brief 사각형 초기화
INLINE void qm_rectf_unit(QmRectF * v)		// identify
{
	qm_rectf_set(v, 0.0f, 0.0f, 0.0f, 0.0f);
}

/// @brief 사각형 대각값 설정 (모든 요소를 같은 값을)
///	@param v 반환 사각형
/// @param diag 대각값
INLINE void qm_rectf_diag(QmRectF * v, float diag)
{
	qm_rectf_set(v, diag, diag, diag, diag);
}

/// @brief 사각형 덧셈
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
INLINE QmRectF qm_rectf_add(QmRectF left, QmRectF right)
{
	QmRectF v =
#if defined QM_USE_AVX
	{ .v = _mm_add_ps(left.v, right.v) };
#elif defined QM_USE_NEON
	{.v = vaddq_f32(left.v, right.v) };
#else
	{.Left = left.Left + right.Left, .Top = left.Top + right.Top, .Right = left.Right + right.Right, .Bottom = left.Bottom + right.Bottom, };
#endif
	return v;
}

/// @brief 사각형 뺄셈
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
INLINE QmRectF qm_rectf_sub(QmRectF left, QmRectF right)
{
	QmRectF v =
#if defined QM_USE_AVX
	{ .v = _mm_sub_ps(left.v, right.v) };
#elif defined QM_USE_NEON
	{.v = vsubq_f32(left.v, right.v) };
#else
	{.Left = left.Left - right.Left, .Top = left.Top - right.Top, .Right = left.Right - right.Right, .Bottom = left.Bottom - right.Bottom, };
#endif
	return v;
}

/// @brief 사각형 확대
/// @param left 원본 사각형
/// @param right 확대값
INLINE QmRectF qm_rectf_mag(QmRectF left, float right)
{
#if defined QM_USE_AVX
	const __m128 m = _mm_set1_ps(right);
	QmRectF v = { .v = _mm_mul_ps(left.v, m) };
#elif defined QM_USE_NEON
	QmRectF v = { .v = vmulq_n_f32(left.v, right) };
#else
	QmRectF v = { .Left = left.Left * right, .Top = left.Top * right, .Right = left.Right * right, .Bottom = left.Bottom * right };
#endif
	return v;
}

/// @brief 사각형의 최소값
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
INLINE QmRectF qm_rectf_min(QmRectF left, QmRectF right)
{
	return qm_rectf(
		(left.Left < right.Left) ? left.Left : right.Left,
		(left.Top < right.Top) ? left.Top : right.Top,
		(left.Right < right.Right) ? left.Right : right.Right,
		(left.Bottom < right.Bottom) ? left.Bottom : right.Bottom);
}

/// @brief 사각형의 최대값
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
INLINE QmRectF qm_rectf_max(QmRectF left, QmRectF right)
{
	return qm_rectf(
		(left.Left > right.Left) ? left.Left : right.Left,
		(left.Top > right.Top) ? left.Top : right.Top,
		(left.Right > right.Right) ? left.Right : right.Right,
		(left.Bottom > right.Bottom) ? left.Bottom : right.Bottom);
}

/// @brief 두 사각형를 비교
/// @param left 왼쪽 사각형
/// @param right 오른쪽 사각형
/// @return 두 사각형가 같으면 참
INLINE bool qm_rectf_eq(QmRectF left, QmRectF right)
{
	return
		qm_eqf(left.Left, right.Left) && qm_eqf(left.Top, right.Top) &&
		qm_eqf(left.Right, right.Right) && qm_eqf(left.Bottom, right.Bottom);
}

/// @brief 사각형가 0인지 비교
/// @param pv 비교할 사각형
/// @return 사각형가 0이면 참
INLINE bool qm_rectf_isu(QmRectF pv)
{
	return pv.Left == 0.0f && pv.Top == 0.0f && pv.Right == 0.0f && pv.Bottom == 0.0f;
}

/// @brief 사각형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 사각형
/// @param right 대상 사각형
/// @param scale 보간값
INLINE QmRectF qm_rectf_interpolate(QmRectF left, QmRectF right, float scale)
{
	return qm_rectf_add(qm_rectf_mag(left, 1.0f - scale), qm_rectf_mag(right, scale));
}

/// @brief 사각형 선형 보간 (왼쪽에서 오른쪽으로 보간)
/// @param left 원본 사각형
/// @param right 대상 사각형
/// @param scale 보간값
INLINE QmRectF qm_rectf_lerp(QmRectF left, QmRectF right, float scale)
{
	return qm_rectf_add(left, qm_rectf_mag(qm_rectf_sub(right, left), scale));		// NOLINT
}

/// @brief 사각형 크기를 키운다 (요소가 양수일 경우)
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른쪽
/// @param bottom 아래쪽
INLINE QmRectF qm_rectf_inflate(QmRectF rt, float left, float top, float right, float bottom)
{
	return qm_rectf(rt.Left - left, rt.Top - top, rt.Right + right, rt.Bottom + bottom);
}

/// @brief 사각형 크기를 줄인다 (요소가 양수일 경우)
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른족
/// @param bottom 아래쪽
INLINE QmRectF qm_rectf_deflate(QmRectF rt, float left, float top, float right, float bottom)
{
	return qm_rectf(rt.Left + left, rt.Top + top, rt.Right - right, rt.Bottom - bottom);
}

/// @brief 사각형을 움직인다
/// @param rt 원본 사각형
/// @param left 왼쪽
/// @param top 윗쪽
/// @param right 오른쪽
/// @param bottom 아래쪽
INLINE QmRectF qm_rectf_offset(QmRectF rt, float left, float top, float right, float bottom)
{
	return qm_rectf(rt.Left + left, rt.Top + top, rt.Right + right, rt.Bottom + bottom);
}

/// @brief 사각형을 움직인다
/// @param rt 원본 사각형
/// @param left 새 왼쪽
/// @param top 새 오른쪽
/// @return
INLINE QmRectF qm_rectf_move(QmRectF rt, float left, float top)
{
	float dx = left - rt.Left;
	float dy = top - rt.Top;
	return qm_rectf(rt.Left + dx, rt.Top + dy, rt.Right + dx, rt.Bottom + dy);
}

/// @brief 사각형의 크기를 재설정한다
/// @param rt 원본 사각형
/// @param width 너비
/// @param height 높이
INLINE QmRectF qm_rectf_resize(QmRectF rt, float width, float height)
{
	return qm_rectf(rt.Left, rt.Top, rt.Left + width, rt.Top + height);
}

/// @brief 사각형의 너비를 얻는다
/// @param rt 대상 사각형
/// @return 사각형의 너비값
INLINE float qm_rectf_width(QmRectF rt)
{
	return rt.Right - rt.Left;
}

/// @brief 사각형의 높이를 얻는다
/// @param rt 대상 사각형
/// @return 사각형의 높이값
INLINE float qm_rectf_height(QmRectF rt)
{
	return rt.Bottom - rt.Top;
}

/// @brief 좌표가 사각형 안에 있는지 조사한다
/// @param rt 대상 사각형
/// @param x,y 좌표
/// @return 좌표가 사각형 안에 있으면 참
INLINE bool qm_rectf_in(QmRectF rt, float x, float y)
{
	return (x >= rt.Left && x <= rt.Right && y >= rt.Top && y <= rt.Bottom);
}

/// @brief 대상 사각형이 원본 사각형안에 있는지 조사한다
/// @param dest 원본 사각형
/// @param target 대상 사각형
/// @return 대상이 원본 안에 있으면 참
INLINE bool qm_rectf_include(QmRectF dest, QmRectF target)
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
INLINE bool qm_rectf_intersect(QmRectF r1, QmRectF r2, QmRectF * p)
{
	const bool b = r2.Left < r1.Right && r2.Right > r1.Left && r2.Top < r1.Bottom && r2.Bottom > r1.Top;
	if (p)
	{
		if (!b)
			qm_rectf_unit(p);
		else
			*p = qm_rectf(
				QN_MAX(r1.Left, r2.Left), QN_MAX(r1.Top, r2.Top),
				QN_MIN(r1.Right, r2.Right), QN_MIN(r1.Bottom, r2.Bottom));
	}
	return b;
}




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

