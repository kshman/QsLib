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
// 벡터 3


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

