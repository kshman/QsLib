#pragma once

#include <math.h>
#if _MSC_VER
#include <intrin.h>
#endif

//////////////////////////////////////////////////////////////////////////
// constant
#define QN_EPSILON						0.0001
#define QN_E							2.7182818284590452353602874713526624977572470937000
#define QN_LOG2E						1.44269504088896340736
#define QN_LOG10E						0.434294481903251827651
#define QN_LOG2B10						0.30102999566398119521
#define QN_LN2							0.6931471805599453094172321214581765680755001343603
#define QN_LN10							2.3025850929940456840179914546843642076011014886288
#define QN_PI							3.1415926535897932384626433832795028841971693993751
#define QN_PI2							6.2831853071795864769252867665590057683943387987502
#define QN_PIH							1.5707963267948966192313216916397514420985846996876
#define QN_PIQ							0.7853981633974483096156608458198757210492923498438
#define QN_SQRT2						1.4142135623730950488016887242096980785696718753769
#define QN_SQRTH						0.7071067811865475244008443621048490392848359376884


//////////////////////////////////////////////////////////////////////////
// macro & iline
#define QN_FRACT(f)						((f)-floorf(f))
#define QN_TORADIAN(degree)				((degree)*(180.0f/(float)QN_PI))
#define QN_TODEGREE(radian)				((radian)*((float)QN_PI/180.0f))
#define QN_EQEPS(a,b,epsilon)			(((a)+(epsilon)>(b)) && ((a)-(epsilon)<(b)))
#define QN_MAX(a,b)						(((a)>(b))?(a):(b))
#define QN_MIN(a,b)						(((a)<(b))?(a):(b))
#define QN_ABS(v)						(((v)<0)?-(v):(v))
#define QN_CLAMP(v,min,max)				((v)<(min)?(min):(v)>(max)?(max):(v))

QN_INLINE bool qn_eqf(float a, float b) { return QN_EQEPS(a, b, (float)QN_EPSILON); }
QN_INLINE float qn_maxf(float a, float b) { return QN_MAX(a, b); }
QN_INLINE float qn_minf(float a, float b) { return QN_MIN(a, b); }
QN_INLINE float qn_absf(float v) { return QN_ABS(v); }
QN_INLINE float qn_clampf(float v, float min, float max) { return QN_CLAMP(v, min, max); }
QN_INLINE float qn_cradf(float v) { return v < (float)-QN_PIH ? v + (float)QN_PIH : v >(float)QN_PIH ? v - (float)QN_PIH : v; }
QN_INLINE float qn_lerpf(float l, float r, float f) { return l + f * (r - l); }
QN_INLINE void qn_sincosf(float f, float* s, float* c) { *s = sinf(f); *c = cosf(f); }


//////////////////////////////////////////////////////////////////////////
// types

// vector2
typedef struct qnVec2
{
	union
	{
		struct { float x, y; };
		struct { float u, v; };
		struct { float min, max; };
		struct { float width, height; };
	};
} qnVec2;

// vector3
typedef struct qnVec3
{
	union
	{
		struct { float x, y, z; };
		struct { float a, b, c; };
	};
} qnVec3;

// vector4
typedef struct qnVec4
{
	union
	{
		struct { float x, y, z, w; };
		struct { float a, b, c, d; };
	};
} qnVec4;

// quaternion
typedef struct qnQuat
{
	float x, y, z, w;
} qnQuat;

// matrix4
typedef struct qnMat4
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
		float m[4][4];
		float _m[16];
#if defined(_INCLUDED_MM2) && defined(SB_MM2)
		__m128 m128[4];
#endif
	};
} qnMat4;

// point
typedef struct qnPoint
{
	union
	{
		struct { int32_t x, y; };
		struct { int32_t min, max; };
		struct { int32_t left, right; };
		struct { uint32_t ux, uy; };
		struct { uint32_t umin, umax; };
		struct { uint32_t uleft, uright; };
		struct { uint32_t width, height; };
	};
} qnPoint;

// rect
typedef struct qnRect
{
	int32_t left, top, right, bottom;
} qnRect;

// quad
typedef struct qnRectF
{
	float left, top, right, bottom;
} qnRectF;

// color
typedef struct qnColor
{
	float r, g, b, a;
} qnColor;

// byte color
typedef struct qnKolor
{
	union
	{
		struct
		{
			uint8_t b, g, r, a;
		};
		uint32_t u;
	};
} qnKolor;

// plane
typedef struct qnPlane
{
	float a, b, c, d;
} qnPlane;

// line3
typedef struct qnLine3
{
	qnVec3 begin, end;
} qnLine3;

// transform
typedef struct qnTrfm
{
	qnVec3 loc;
	qnQuat rot;
	qnVec3 scl;
} qnTrfm;

// half vector2
typedef struct qnVecH2
{
	half_t x, y;
} qnVecH2;

// half vector3
typedef struct qnVecH3
{
	half_t x, y, z;
} qnVecH3;

// hafl vector4
typedef struct qnVecH4
{
	half_t x, y, z, w;
} qnVecH4;

// declaration
QN_INLINE bool qn_plane_intersect(const qnPlane* p, qnVec3* loc, qnVec3* dir, const qnPlane* o);


//////////////////////////////////////////////////////////////////////////
// vector2

QN_INLINE void qn_vec2_set(qnVec2* pv, float x, float y)
{
	pv->x = x;
	pv->y = y;
}

QN_INLINE void qn_vec2_rst(qnVec2* pv) // identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
}

QN_INLINE void qn_vec2_diag(qnVec2* pv, float v)
{
	pv->x = v;
	pv->y = v;
}

QN_INLINE float qn_vec2_dot(const qnVec2* left, const qnVec2* right)
{
	return left->x * right->x + left->y * right->y;
}

QN_INLINE float qn_vec2_len_sq(const qnVec2* pv)
{
	return pv->x * pv->x + pv->y * pv->y;
}

QN_INLINE float qn_vec2_len(const qnVec2* pv)
{
	return sqrtf(qn_vec2_len_sq(pv));
}

QN_INLINE void qn_vec2_norm(qnVec2* pv, const qnVec2* v)
{
	float f = qn_vec2_len(v);
	if (f > 0.0f) f = 1.0f / f;
	pv->x = v->x * f;
	pv->y = v->y * f;
}

QN_INLINE void qn_vec2_add(qnVec2* pv, const qnVec2* left, const qnVec2* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
}

QN_INLINE void qn_vec2_sub(qnVec2* pv, const qnVec2* left, const qnVec2* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
}

QN_INLINE void qn_vec2_mag(qnVec2* pv, const qnVec2* left, float right)
{
	pv->x = left->x * right;
	pv->y = left->y * right;
}

QN_INLINE float qn_vec2_accm(const qnVec2* pv, float s)
{
	return (pv->x + pv->y) * s;
}

QN_INLINE void qn_vec2_cross(qnVec2* pv, const qnVec2* left, const qnVec2* right)
{
	pv->x = left->y * right->x - left->x * right->y;
	pv->y = left->x * right->y - left->y * right->x;
}

QN_INLINE void qn_vec2_ivt(qnVec2* pv, const qnVec2* v)  // invert
{
	pv->x = -v->x;
	pv->y = -v->y;
}

QN_INLINE void qn_vec2_neg(qnVec2* pv, const qnVec2* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
}

QN_INLINE bool qn_vec2_eq(const qnVec2* left, const qnVec2* right)
{
	return qn_eqf(left->x, right->x) && qn_eqf(left->y, right->y);
}

QN_INLINE bool qn_vec2_isi(const qnVec2* pv)
{
	return qn_eqf(pv->x, 0.0f) && qn_eqf(pv->y, 0.0f);
}

QN_INLINE void qn_vec2_interpolate(qnVec2* pv, const qnVec2* left, const qnVec2* right, float s)
{
	float f = 1.0f - s;
	pv->x = right->x * f + left->x * s;
	pv->y = right->y * f + left->y * s;
}

QN_INLINE void qn_vec2_lerp(qnVec2* pv, const qnVec2* left, const qnVec2* right, float s)
{
	pv->x = left->x + s * (right->x - left->x);
	pv->y = left->y + s * (right->y - left->y);
}

QN_INLINE void qn_vec2_min(qnVec2* pv, const qnVec2* left, const qnVec2* right)
{
	pv->x = (left->x < right->x) ? left->x : right->x;
	pv->y = (left->y < right->y) ? left->y : right->y;
}

QN_INLINE void qn_vec2_max(qnVec2* pv, const qnVec2* left, const qnVec2* right)
{
	pv->x = (left->x > right->x) ? left->x : right->x;
	pv->y = (left->y > right->y) ? left->y : right->y;
}


//////////////////////////////////////////////////////////////////////////
// vector3

QN_INLINE void qn_vec3_set(qnVec3* pv, float x, float y, float z)
{
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

QN_INLINE void qn_vec3_rst(qnVec3* pv)	// identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
	pv->z = 0.0f;
}

QN_INLINE void qn_vec3_diag(qnVec3* pv, float v)
{
	pv->x = v;
	pv->y = v;
	pv->z = v;
}

QN_INLINE float qn_vec3_len_sq(const qnVec3* pv)
{
	return pv->x * pv->x + pv->y * pv->y + pv->z * pv->z;
}

QN_INLINE float qn_vec3_len(const qnVec3* pv)
{
	return sqrtf(qn_vec3_len_sq(pv));
}

QN_INLINE void qn_vec3_norm(qnVec3* pv, const qnVec3* v)
{
	float f = qn_vec3_len(v);
	if (f > 0.0f) f = 1.0f / f;
	pv->x = v->x * f;
	pv->y = v->y * f;
	pv->z = v->z * f;
}

QN_INLINE void qn_vec3_add(qnVec3* pv, const qnVec3* left, const qnVec3* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
	pv->z = left->z + right->z;
}

QN_INLINE void qn_vec3_sub(qnVec3* pv, const qnVec3* left, const qnVec3* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
	pv->z = left->z - right->z;
}

QN_INLINE void qn_vec3_mag(qnVec3* pv, const qnVec3* v, float scale)
{
	pv->x = v->x * scale;
	pv->y = v->y * scale;
	pv->z = v->z * scale;
}

QN_INLINE float qn_vec3_accm(const qnVec3* pv, float scale)
{
	return (pv->x + pv->y + pv->z) * scale;
}

QN_INLINE void qn_vec3_cross(qnVec3* pv, const qnVec3* left, const qnVec3* right)
{
	pv->x = left->y * right->z - left->z * right->y;
	pv->y = left->z * right->x - left->x * right->z;
	pv->z = left->x * right->y - left->y * right->x;
}

QN_INLINE void qn_vec3_ivt(qnVec3* pv, const qnVec3* v)  // invert
{
	pv->x = -v->x;
	pv->y = -v->y;
	pv->z = -v->z;
}

QN_INLINE void qn_vec3_neg(qnVec3* pv, const qnVec3* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
	pv->z = 1.0f - v->z;
}

QN_INLINE float qn_vec3_dot(const qnVec3* left, const qnVec3* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z;
}

QN_INLINE void qn_vec3_min(qnVec3* pv, const qnVec3* left, const qnVec3* right)
{
	pv->x = left->x < right->x ? left->x : right->x;
	pv->y = left->y < right->y ? left->y : right->y;
	pv->z = left->z < right->z ? left->z : right->z;
}

QN_INLINE void qn_vec3_max(qnVec3* pv, const qnVec3* left, const qnVec3* right)
{
	pv->x = left->x > right->x ? left->x : right->x;
	pv->y = left->y > right->y ? left->y : right->y;
	pv->z = left->z > right->z ? left->z : right->z;
}

QN_INLINE bool qn_vec3_eq(const qnVec3* left, const qnVec3* right)
{
	return qn_eqf(left->x, right->x) && qn_eqf(left->y, right->y) && qn_eqf(left->z, right->z);
}

QN_INLINE bool qn_vec3_isi(const qnVec3* pv)
{
	return (pv->x == 0.0f && pv->y == 0.0f && pv->z == 0.0f);
}

QN_INLINE float qn_vec3_dist_sq(const qnVec3* left, const qnVec3* right)
{
	qnVec3 t;
	qn_vec3_sub(&t, left, right);
	return qn_vec3_len_sq(&t);
}

QN_INLINE float qn_vec3_dist(const qnVec3* left, const qnVec3* right)
{
	return sqrtf(qn_vec3_dist_sq(left, right));
}

QN_INLINE void qn_vec3_dir(qnVec3* pv, const qnVec3* left, const qnVec3* right)
{
	qn_vec3_sub(pv, left, right);
	qn_vec3_norm(pv, pv);
}

QN_INLINE float qn_vec3_rad_sq(const qnVec3* left, const qnVec3* right)
{
	qnVec3 t;
	qn_vec3_add(&t, left, right);
	qn_vec3_mag(&t, &t, 0.5f);
	qn_vec3_sub(&t, &t, left);
	return qn_vec3_len_sq(&t);
}

QN_INLINE float qn_vec3_rad(const qnVec3* left, const qnVec3* right)
{
	return sqrtf(qn_vec3_rad_sq(left, right));
}

QN_INLINE void qn_vec3_trfm(qnVec4* pv, const qnVec3* v, const qnMat4* trfm)
{
	float x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31 + trfm->_41;
	float y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32 + trfm->_42;
	float z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33 + trfm->_43;
	float w = v->x * trfm->_14 + v->y * trfm->_24 + v->z * trfm->_34 + trfm->_44;
	pv->x = x;
	pv->y = y;
	pv->z = z;
	pv->w = w;
}

QN_INLINE void qn_vec3_trfm_coord(qnVec3* pv, const qnVec3* v, const qnMat4* trfm)
{
	float x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31 + trfm->_41;
	float y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32 + trfm->_42;
	float z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33 + trfm->_43;
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

QN_INLINE void qn_vec3_trfm_norm(qnVec3* pv, const qnVec3* v, const qnMat4* trfm)
{
	float x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31;
	float y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32;
	float z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33;
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

QN_INLINE void qn_vec3_quat(qnVec3* pv, const qnQuat* rot)
{
	float x = rot->x * rot->x;
	float y = rot->y * rot->y;
	float z = rot->z * rot->z;
	float w = rot->w * rot->w;
	pv->x = atan2f(2.0f * (rot->y * rot->z + rot->x * rot->w), -x - y + z + w);
	pv->y = asinf(qn_clampf(-2.0f * (rot->x * rot->z + rot->y * rot->w), -1.0f, 1.0f));
	pv->z = atan2f(2.0f * (rot->x * rot->y + rot->z * rot->w), x - y - z + w);
}

QN_INLINE void qn_vec3_mat4(qnVec3* pv, const qnMat4* rot)
{
	if (rot->_31 == 0.0f && rot->_33 == 0.0f)
	{
		pv->x = rot->_32 > 0.0f ? (float)(QN_PIH + QN_PI) : (float)QN_PIH;
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

QN_INLINE void qn_vec3_quat_vec3(qnVec3* pv, const qnQuat* rot, const qnVec3* angle)
{
	qnQuat q1 =
	{
		.x = angle->x * rot->w,
		.y = angle->y * rot->w,
		.z = angle->z * rot->w,
		.w = -qn_vec3_dot((qnVec3*)rot, angle),
	};

	qnVec3 t;
	qn_vec3_cross(&t, (qnVec3*)rot, angle);
	q1.x += t.x;
	q1.y += t.y;
	q1.z += t.z;

	qnQuat q2 =
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

QN_INLINE void qn_vec3_closed_line(qnVec3* pv, const qnLine3* line, const qnVec3* loc)
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

QN_INLINE void qn_vec3_interpolate(qnVec3* pv, const qnVec3* left, const qnVec3* right, float scale)
{
	float f = 1.0f - scale;
	pv->x = right->x * f + left->x * scale;
	pv->y = right->y * f + left->y * scale;
	pv->z = right->z * f + left->z * scale;
}

QN_INLINE void qn_vec3_lerp(qnVec3* pv, const qnVec3* left, const qnVec3* right, float scale)
{
	pv->x = left->x + scale * (right->x - left->x);
	pv->y = left->y + scale * (right->y - left->y);
	pv->z = left->z + scale * (right->z - left->z);
}

QN_INLINE void qn_vec3_lerp_len(qnVec3* pv, const qnVec3* left, const qnVec3* right, float scale, float len)
{
	qnVec3 v0, v1;
	qn_vec3_norm(&v0, left);
	qn_vec3_norm(&v1, right);
	pv->x = v0.x + (v1.x - v0.x) * scale;
	pv->y = v0.y + (v1.y - v0.y) * scale;
	pv->z = v0.z + (v1.z - v0.z) * scale;
	qn_vec3_mag(pv, pv, len);
}

QN_INLINE void qn_vec3_form_norm(qnVec3* pv, const qnVec3* v0, const qnVec3* v1, const qnVec3* v2)
{
	qnVec3 t0, t1;
	qn_vec3_sub(&t0, v1, v0);
	qn_vec3_sub(&t1, v2, v1);
	qn_vec3_cross(pv, &t0, &t1);
	qn_vec3_mag(pv, pv, qn_vec3_len(pv));
}

QN_INLINE bool qn_vec3_reflect(qnVec3* pv, const qnVec3* in, const qnVec3* dir)
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

QN_INLINE float qn_vec3_yaw(const qnVec3* pv)
{
	return -atanf(pv->z / pv->x) + ((pv->x > 0.0f) ? (float)-QN_PIH : (float)QN_PIH);
}

QN_INLINE bool qn_vec3_between(const qnVec3* p, const qnVec3* begin, const qnVec3* end)
{
	qnVec3 t;
	qn_vec3_sub(&t, end, begin);
	float f = qn_vec3_len_sq(&t);
	return qn_vec3_dist_sq(p, begin) <= f && qn_vec3_dist_sq(p, end) <= f;
}

QN_INLINE bool qn_vec3_intersect_line(qnVec3* pv, const qnPlane* plane, const qnVec3* loc, const qnVec3* dir)
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

QN_INLINE bool qn_vec3_intersect_point(qnVec3* pv, const qnPlane* plane, const qnVec3* v1, const qnVec3* v2)
{
	qnVec3 dir;
	qn_vec3_sub(&dir, v2, v1);
	return qn_vec3_intersect_line(pv, plane, v1, &dir);
}

QN_INLINE bool qn_vec3_intersect_between_point(qnVec3* pv, const qnPlane* plane, const qnVec3* v1, const qnVec3* v2)
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

QN_INLINE bool qn_vec3_intersect_planes(qnVec3* pv, const qnPlane* plane, const qnPlane* other1, const qnPlane* other2)
{
	qnVec3 dir, loc;
	return (qn_plane_intersect(plane, &loc, &dir, other1)) ? qn_vec3_intersect_line(pv, other2, &loc, &dir) : false;
}


//////////////////////////////////////////////////////////////////////////
// vector4

QN_INLINE void qn_vec4_set(qnVec4* pv, float x, float y, float z, float w)
{
	pv->x = x;
	pv->y = y;
	pv->z = z;
	pv->w = w;
}

QN_INLINE void qn_vec4_rst(qnVec4* pv)	// identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
	pv->z = 0.0f;
	pv->w = 0.0f;
}

QN_INLINE void qn_vec4_diag(qnVec4* pv, float v)
{
	pv->x = v;
	pv->y = v;
	pv->z = v;
	pv->w = v;
}

QN_INLINE float qn_vec4_len_sq(const qnVec4* pv)
{
	return pv->x * pv->x + pv->y * pv->y + pv->z * pv->z + pv->w * pv->w;
}

QN_INLINE float qn_vec4_len(const qnVec4* pv)
{
	return sqrtf(qn_vec4_len_sq(pv));
}

QN_INLINE void qn_vec4_norm(qnVec4* pv, const qnVec4* v)
{
	float f = qn_vec4_len(v);
	if (f > 0.0f) f = 1.0f / f;
	pv->x = v->x * f;
	pv->y = v->y * f;
	pv->z = v->z * f;
	pv->w = v->w * f;
}

QN_INLINE void qn_vec4_add(qnVec4* pv, const qnVec4* left, const qnVec4* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
	pv->z = left->z + right->z;
	pv->w = left->w + right->w;
}

QN_INLINE void qn_vec4_sub(qnVec4* pv, const qnVec4* left, const qnVec4* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
	pv->z = left->z - right->z;
	pv->w = left->w - right->w;
}

QN_INLINE void qn_vec4_mag(qnVec4* pv, const qnVec4* v, float scale)
{
	pv->x = v->x * scale;
	pv->y = v->y * scale;
	pv->z = v->z * scale;
	pv->w = v->w * scale;
}

QN_INLINE float qn_vec4_accm(const qnVec4* pv, float scale)
{
	return (pv->x + pv->y + pv->z * pv->w) * scale;
}

QN_INLINE void qn_vec4_ivt(qnVec4* pv, const qnVec4* v)
{
	pv->x = -v->x;
	pv->y = -v->y;
	pv->z = -v->z;
	pv->w = -v->w;
}

QN_INLINE void qn_vec4_neg(qnVec4* pv, const qnVec4* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
	pv->z = 1.0f - v->z;
	pv->w = 1.0f - v->w;
}

QN_INLINE float qn_vec4_dot(const qnVec4* left, const qnVec4* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z + left->w * right->w;
}

QN_INLINE void qn_vec4_cross(qnVec4* pv, const qnVec4* v1, const qnVec4* v2, const qnVec4* v3)
{
	pv->x = v1->y * (v2->z * v3->w - v3->z * v2->w) - v1->z * (v2->y * v3->w - v3->y * v2->w) + v1->w * (v2->y * v3->z - v2->z * v3->y);
	pv->y = -(v1->x * (v2->z * v3->w - v3->z * v2->w) - v1->z * (v2->x * v3->w - v3->x * v2->w) + v1->w * (v2->x * v3->z - v3->x * v2->z));
	pv->z = v1->x * (v2->y * v3->w - v3->y * v2->w) - v1->y * (v2->x * v3->w - v3->x * v2->w) + v1->w * (v2->x * v3->y - v3->x * v2->y);
	pv->w = -(v1->x * (v2->y * v3->z - v3->y * v2->z) - v1->y * (v2->x * v3->z - v3->x * v2->z) + v1->z * (v2->x * v3->y - v3->x * v2->y));
}

QN_INLINE void qn_vec4_min(qnVec4* pv, const qnVec4* left, const qnVec4* right)
{
	pv->x = (left->x < right->x) ? left->x : right->x;
	pv->y = (left->y < right->y) ? left->y : right->y;
	pv->z = (left->z < right->z) ? left->z : right->z;
	pv->w = (left->w < right->w) ? left->w : right->w;
}

QN_INLINE void qn_vec4_max(qnVec4* pv, const qnVec4* left, const qnVec4* right)
{
	pv->x = (left->x > right->x) ? left->x : right->x;
	pv->y = (left->y > right->y) ? left->y : right->y;
	pv->z = (left->z > right->z) ? left->z : right->z;
	pv->w = (left->w > right->w) ? left->w : right->w;
}

QN_INLINE bool qn_vec4_eq(const qnVec4* left, const qnVec4* right)
{
	return
		qn_eqf(left->x, right->x) &&
		qn_eqf(left->y, right->y) &&
		qn_eqf(left->z, right->z) &&
		qn_eqf(left->w, right->w);
}

QN_INLINE bool qn_vec4_isi(const qnVec4* pv)
{
	return (pv->x == 0.0f && pv->y == 0.0f && pv->z == 0.0f && pv->w == 0.0f);
}

QN_INLINE void qn_vec4_trfm(qnVec4* pv, const qnVec4* v, const qnMat4* trfm)
{
	pv->x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31 + v->w * trfm->_41;
	pv->y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32 + v->w * trfm->_42;
	pv->z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33 + v->w * trfm->_43;
	pv->w = v->x * trfm->_14 + v->y * trfm->_24 + v->z * trfm->_34 + v->w * trfm->_44;
}

QN_INLINE void qn_vec4_interpolate(qnVec4* pv, const qnVec4* left, const qnVec4* right, float scale)
{
	float f = 1.0f - scale;
	pv->x = right->x * f + left->x * scale;
	pv->y = right->y * f + left->y * scale;
	pv->z = right->z * f + left->z * scale;
	pv->w = right->w * f + left->w * scale;
}

QN_INLINE void qn_vec4_lerp(qnVec4* pv, const qnVec4* left, const qnVec4* right, float scale)
{
	pv->x = left->x + scale * (right->x - left->x);
	pv->y = left->y + scale * (right->y - left->y);
	pv->z = left->z + scale * (right->z - left->z);
	pv->w = left->w + scale * (right->w - left->w);
}


//////////////////////////////////////////////////////////////////////////
// quaternion

QN_INLINE void qn_quat_set(qnQuat* pq, float x, float y, float z, float w)
{
	pq->x = x;
	pq->y = y;
	pq->z = z;
	pq->w = w;
}

QN_INLINE void qn_quat_zero(qnQuat* pq)
{
	pq->x = 0.0f;
	pq->y = 0.0f;
	pq->z = 0.0f;
	pq->w = 0.0f;
}

QN_INLINE void qn_quat_rst(qnQuat* pq)	// identify
{
	pq->x = 0.0f;
	pq->y = 0.0f;
	pq->z = 0.0f;
	pq->w = 1.0f;
}

QN_INLINE float qn_quat_len_sq(const qnQuat* pq)
{
	return pq->x * pq->x + pq->y * pq->y + pq->z * pq->z + pq->w * pq->w;
}

QN_INLINE float qn_quat_len(const qnQuat* pq)
{
	return sqrtf(qn_quat_len_sq(pq));
}

QN_INLINE void qn_quat_norm(qnQuat* pq, const qnQuat* q)
{
	float f = qn_quat_len(q);
	if (f > 0.0f) f = 1.0f / f;
	pq->x = q->x * f;
	pq->y = q->y * f;
	pq->z = q->z * f;
	pq->w = q->w * f;
}

QN_INLINE void qn_quat_add(qnQuat* pq, const qnQuat* left, const qnQuat* right)
{
	pq->x = left->x + right->x;
	pq->y = left->y + right->y;
	pq->z = left->z + right->z;
	pq->w = left->w + right->w;
}

QN_INLINE void qn_quat_sub(qnQuat* pq, const qnQuat* left, const qnQuat* right)
{
	pq->x = left->x - right->x;
	pq->y = left->y - right->y;
	pq->z = left->z - right->z;
	pq->w = left->w - right->w;
}

QN_INLINE void qn_quat_mag(qnQuat* pq, const qnQuat* left, float right)
{
	pq->x = left->x * right;
	pq->y = left->y * right;
	pq->z = left->z * right;
	pq->w = left->w * right;
}

QN_INLINE void qn_quat_ivt(qnQuat* pq, const qnQuat* v)
{
	pq->x = -v->x;
	pq->y = -v->y;
	pq->z = -v->z;
	pq->w = -v->w;
}

QN_INLINE void qn_quat_neg(qnQuat* pq, const qnQuat* v)
{
	pq->x = 1.0f - v->x;
	pq->y = 1.0f - v->y;
	pq->z = 1.0f - v->z;
	pq->w = 1.0f - v->w;
}

QN_INLINE void qn_quat_cjg(qnQuat* pq, const qnQuat* q)	// conjugate
{
	pq->x = -q->x;
	pq->y = -q->y;
	pq->z = -q->z;
	pq->w = q->w;
}

QN_INLINE float qn_quat_dot(const qnQuat* left, const qnQuat* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z + left->w * right->w;
}

QN_INLINE void qn_quat_cross(qnQuat* pq, const qnQuat* q1, const qnQuat* q2, const qnQuat* q3)
{
	pq->x = q1->y * (q2->z * q3->w - q3->z * q2->w) - q1->z * (q2->y * q3->w - q3->y * q2->w) + q1->w * (q2->y * q3->z - q2->z * q3->y);
	pq->y = -(q1->x * (q2->z * q3->w - q3->z * q2->w) - q1->z * (q2->x * q3->w - q3->x * q2->w) + q1->w * (q2->x * q3->z - q3->x * q2->z));
	pq->z = q1->x * (q2->y * q3->w - q3->y * q2->w) - q1->y * (q2->x * q3->w - q3->x * q2->w) + q1->w * (q2->x * q3->y - q3->x * q2->y);
	pq->w = -(q1->x * (q2->y * q3->z - q3->y * q2->z) - q1->y * (q2->x * q3->z - q3->x * q2->z) + q1->z * (q2->x * q3->y - q3->x * q2->y));
}

QN_INLINE bool qn_quat_eq(const qnQuat* left, const qnQuat* right)
{
	return
		qn_eqf(left->x, right->x) &&
		qn_eqf(left->y, right->y) &&
		qn_eqf(left->z, right->z) &&
		qn_eqf(left->w, right->w);
}

QN_INLINE bool qn_quat_isi(const qnQuat* pq)
{
	return pq->x == 0.0f && pq->y == 0.0f && pq->z == 0.0f && pq->w == 1.0f;
}

QN_INLINE void qn_quat_mul(qnQuat* pq, const qnQuat* left, const qnQuat* right)
{
	pq->x = left->x * right->w + left->y * right->z - left->z * right->y + left->w * right->x;
	pq->y = -left->x * right->z + left->y * right->w + left->z * right->x + left->w * right->y;
	pq->z = left->x * right->y - left->y * right->x + left->z * right->w + left->w * right->z;
	pq->w = -left->x * right->x - left->y * right->y - left->z * right->z + left->w * right->w;
}

QN_INLINE void qn_quat_slerp(qnQuat* pq, const qnQuat* left, const qnQuat* right, float change)
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

QN_INLINE void qn_quat_x(qnQuat* pq, float x)
{
	pq->y = pq->z = 0.0f;
	qn_sincosf(x * 0.5f, &pq->x, &pq->w);
}

QN_INLINE void qn_quat_y(qnQuat* pq, float y)
{
	pq->x = pq->z = 0.0f;
	qn_sincosf(y * 0.5f, &pq->y, &pq->w);
}

QN_INLINE void qn_quat_z(qnQuat* pq, float z)
{
	pq->x = pq->y = 0.0f;
	qn_sincosf(z * 0.5f, &pq->z, &pq->w);
}

QN_INLINE void qn_quat_mat4(qnQuat* pq, const qnMat4* rot)
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

QN_INLINE void qn_quat_vec(qnQuat* pq, const qnVec3* rot)
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

QN_INLINE void qn_quat_axis_vec(qnQuat* pq, const qnVec3* v, float angle)
{
	float s, c;
	qn_sincosf(angle * 0.5f, &s, &c);
	qn_quat_set(pq, v->x * s, v->y * s, v->z * s, c);
}

QN_INLINE void qn_quat_exp(qnQuat* pq, const qnQuat* q)
{
	float n = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z);
	if (n)
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

QN_INLINE void qn_quat_ln(qnQuat* pq, const qnQuat* q)
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

QN_INLINE void qn_quat_inv(qnQuat* pq, const qnQuat* q)
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
		qnQuat t;
		qn_quat_cjg(&t, q);
		n = 1.0f / n;
		pq->x = t.x * n;
		pq->y = t.y * n;
		pq->z = t.z * n;
		pq->w = t.w * n;
	}
}

QN_INLINE void qn_quat_interpolate(qnQuat* pq, const qnQuat* left, const qnQuat* right, float scale)
{
	float f = 1.0f - scale;
	pq->x = right->x * f + left->x * scale;
	pq->y = right->y * f + left->y * scale;
	pq->z = right->z * f + left->z * scale;
	pq->w = right->w * f + left->w * scale;
}

QN_INLINE void qn_quat_lerp(qnQuat* pq, const qnQuat* left, const qnQuat* right, float scale)
{
	pq->x = left->x + scale * (right->x - left->x);
	pq->y = left->y + scale * (right->y - left->y);
	pq->z = left->z + scale * (right->z - left->z);
	pq->w = left->w + scale * (right->w - left->w);
}


//////////////////////////////////////////////////////////////////////////
// matrix4

QN_INLINE void qn_mat4_zero(qnMat4* pm)
{
	pm->_11 = pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_22 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_33 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = pm->_44 = 0.0f;
}

QN_INLINE void qn_mat4_rst(qnMat4* pm)	// identify
{
	pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = 0.0f;
	pm->_11 = pm->_22 = pm->_33 = pm->_44 = 1.0f;
}

QN_INLINE void qn_mat4_diag(qnMat4* pm, float v)
{
	pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = 0.0f;
	pm->_11 = pm->_22 = pm->_33 = pm->_44 = v;
}

QN_INLINE void qn_mat4_add(qnMat4* pm, const qnMat4* left, const qnMat4* right)
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

QN_INLINE void qn_mat4_sub(qnMat4* pm, const qnMat4* left, const qnMat4* right)
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

QN_INLINE void qn_mat4_mag(qnMat4* pm, const qnMat4* left, float right)
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

QN_INLINE void qn_mat4_loc(qnMat4* pm, float x, float y, float z, bool reset)
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

QN_INLINE void qn_mat4_scl(qnMat4* pm, float x, float y, float z, bool reset)
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

QN_INLINE void qn_mat4_move(qnMat4* pm, float x, float y, float z)
{
	pm->_41 += x;
	pm->_42 += y;
	pm->_43 += z;
}

QN_INLINE bool qn_mat4_eq(const qnMat4* left, const qnMat4* right)
{
	return memcmp(left, right, sizeof(qnMat4)) == 0;
}

QN_INLINE bool qn_mat4_isi(const qnMat4* pm)
{
	return
		pm->_11 == 1.0f && pm->_12 == 0.0f && pm->_13 == 0.0f && pm->_14 == 0.0f &&
		pm->_21 == 0.0f && pm->_22 == 1.0f && pm->_23 == 0.0f && pm->_24 == 0.0f &&
		pm->_31 == 0.0f && pm->_32 == 0.0f && pm->_33 == 1.0f && pm->_34 == 0.0f &&
		pm->_41 == 0.0f && pm->_42 == 0.0f && pm->_43 == 0.0f && pm->_44 == 1.0f;
}

QN_INLINE void qn_mat4_mul(qnMat4* pm, const qnMat4* left, const qnMat4* right)
{
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
}

QN_INLINE void qn_mat4_tran(qnMat4* pm, const qnMat4* m)
{
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
}

QN_INLINE void qn_mat4_inv(qnMat4* pm, const qnMat4* m, float* /*NULLABLE*/determinant)
{

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
}

QN_INLINE void qn_mat4_vec(qnMat4* pm, const qnVec3* rot, const qnVec3* /*NULLABLE*/loc)
{
	float sr, sp, sy;
	float cr, cp, cy;
	float srsp, crsp;

	qn_sincosf(rot->x, &sr, &cr);
	qn_sincosf(rot->y, &sp, &cp);
	qn_sincosf(rot->z, &sy, &cy);
	srsp = sr * sp;
	crsp = cr * sp;

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

QN_INLINE void qn_mat4_quat(qnMat4* pm, const qnQuat* rot, const qnVec3* /*NULLABLE*/loc)
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

QN_INLINE void qn_mat4_x(qnMat4* pm, float rot)
{
	float fs, fc;
	qn_sincosf(rot, &fs, &fc);

	pm->_11 = 1.0f;
	pm->_12 = 0.0f;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = fc;
	pm->_23 = fs;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = -fs;
	pm->_33 = fc;
	pm->_34 = 0.0f;

	pm->_41 = 0.0f;
	pm->_42 = 0.0f;
	pm->_43 = 0.0f;
	pm->_44 = 1.0f;
}

QN_INLINE void qn_mat4_y(qnMat4* pm, float rot)
{
	float fs, fc;
	qn_sincosf(rot, &fs, &fc);

	pm->_11 = fc;
	pm->_12 = 0.0f;
	pm->_13 = -fs;
	pm->_14 = 0.0f;

	pm->_21 = 0.0f;
	pm->_22 = 1.0f;
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = fs;
	pm->_32 = 0.0f;
	pm->_33 = fc;
	pm->_34 = 0.0f;

	pm->_41 = 0.0f;
	pm->_42 = 0.0f;
	pm->_43 = 0.0f;
	pm->_44 = 1.0f;
}

QN_INLINE void qn_mat4_z(qnMat4* pm, float rot)
{
	float fs, fc;
	qn_sincosf(rot, &fs, &fc);

	pm->_11 = fc;
	pm->_12 = fs;
	pm->_13 = 0.0f;
	pm->_14 = 0.0f;

	pm->_21 = -fs;
	pm->_22 = fc;
	pm->_23 = 0.0f;
	pm->_24 = 0.0f;

	pm->_31 = 0.0f;
	pm->_32 = 0.0f;
	pm->_33 = 1.0f;
	pm->_34 = 0.0f;

	pm->_41 = 0.0f;
	pm->_42 = 0.0f;
	pm->_43 = 0.0f;
	pm->_44 = 1.0f;
}

QN_INLINE float qn_mat4_det(const qnMat4* m)
{
	float f =
		(m->_11 * m->_22 - m->_21 * m->_12) * (m->_33 * m->_44 - m->_43 * m->_34) - (m->_11 * m->_32 - m->_31 * m->_12) * (m->_23 * m->_44 - m->_43 * m->_24) +
		(m->_11 * m->_42 - m->_41 * m->_12) * (m->_23 * m->_34 - m->_33 * m->_24) + (m->_21 * m->_32 - m->_31 * m->_22) * (m->_13 * m->_44 - m->_43 * m->_14) -
		(m->_21 * m->_42 - m->_41 * m->_22) * (m->_13 * m->_34 - m->_33 * m->_14) + (m->_31 * m->_42 - m->_41 * m->_32) * (m->_13 * m->_24 - m->_23 * m->_14);
	return f == 0.0f ? 0.0f : 1.0f / f;
}

QN_INLINE void qn_mat4_lookat_lh(qnMat4* pm, const qnVec3* eye, const qnVec3* at, const qnVec3* up)
{
	qnVec3 vx, vy, vz;

	qn_vec3_sub(&vz, at, eye);
	qn_vec3_norm(&vz, &vz);

	qn_vec3_cross(&vx, up, &vz);
	qn_vec3_norm(&vx, &vx);

	qn_vec3_cross(&vy, &vz, &vx);

	pm->_11 = vx.x;
	pm->_12 = vy.x;
	pm->_13 = vz.x;
	pm->_14 = 0.0f;

	pm->_21 = vx.y;
	pm->_22 = vy.y;
	pm->_23 = vz.y;
	pm->_24 = 0.0f;

	pm->_31 = vx.z;
	pm->_32 = vy.z;
	pm->_33 = vz.z;
	pm->_34 = 0.0f;

	pm->_41 = -qn_vec3_dot(&vx, eye);
	pm->_42 = -qn_vec3_dot(&vy, eye);
	pm->_43 = -qn_vec3_dot(&vz, eye);
	pm->_44 = 1.0f;
}

QN_INLINE void qn_mat4_lookat_rh(qnMat4* pm, const qnVec3* eye, const qnVec3* at, const qnVec3* up)
{
	qnVec3 vx, vy, vz;

	qn_vec3_sub(&vz, eye, at);
	qn_vec3_norm(&vz, &vz);

	qn_vec3_cross(&vx, up, &vz);
	qn_vec3_norm(&vx, &vx);

	qn_vec3_cross(&vy, &vz, &vx);

	pm->_11 = vx.x;
	pm->_12 = vy.x;
	pm->_13 = vz.x;
	pm->_14 = 0.0f;

	pm->_21 = vx.y;
	pm->_22 = vy.y;
	pm->_23 = vz.y;
	pm->_24 = 0.0f;

	pm->_31 = vx.z;
	pm->_32 = vy.z;
	pm->_33 = vz.z;
	pm->_34 = 0.0f;

	pm->_41 = -qn_vec3_dot(&vx, eye);
	pm->_42 = -qn_vec3_dot(&vy, eye);
	pm->_43 = -qn_vec3_dot(&vz, eye);
	pm->_44 = 1.0f;
}

QN_INLINE void qn_mat4_perspective_lh(qnMat4* pm, float fov, float aspect, float zn, float zf)
{
	float f = 1.0f / tanf(fov * 0.5f);
	float q = zf / (zf - zn);

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
	pm->_43 = -zn * q;
	pm->_44 = 0.0f;

}

QN_INLINE void qn_mat4_perspective_rh(qnMat4* pm, float fov, float aspect, float zn, float zf)
{
	float f = 1.0f / tanf(fov * 0.5f);
	float q = zf / (zn - zf);

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
	pm->_43 = zn * q;
	pm->_44 = 0.0f;

}

QN_INLINE void qn_mat4_ortho_lh(qnMat4* pm, float width, float height, float zn, float zf)
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

QN_INLINE void qn_mat4_ortho_rh(qnMat4* pm, float width, float height, float zn, float zf)
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

QN_INLINE void qn_mat4_ortho_offcenter_lh(qnMat4* pm, float l, float r, float b, float t, float zn, float zf)
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

QN_INLINE void qn_mat4_ortho_offcenter_rh(qnMat4* pm, float l, float r, float b, float t, float zn, float zf)
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

QN_INLINE void qn_mat4_viewport(qnMat4* pm, float x, float y, float width, float height)
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

QN_INLINE void qn_mat4_shadow(qnMat4* pm, const qnVec4* light, const qnPlane* plane)
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

QN_INLINE void qn_mat4_affine(qnMat4* pm, const qnVec3* scl, const qnVec3* rotcenter, const qnQuat* rot, const qnVec3* loc)
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

QN_INLINE void qn_mat4_interpolate(qnMat4* pm, const qnMat4* left, const qnMat4* right, float f)
{
	float d = 1.0f - f;

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

QN_INLINE void qn_mat4_trfm(qnMat4* m, const qnVec3* loc, const qnQuat* rot, const qnVec3* scl)
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

QN_INLINE void qn_mat4_trfm_vec(qnMat4* m, const qnVec3* loc, const qnVec3* rot, const qnVec3* scl)
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


//////////////////////////////////////////////////////////////////////////
// point

QN_INLINE void qn_point_set(qnPoint* pt, int32_t x_or_min, int32_t y_or_max)
{
	pt->x = x_or_min;
	pt->y = y_or_max;
}

QN_INLINE void qn_point_set_size(qnPoint* pt, const qnRect* rt)
{
	pt->x = rt->right - rt->left;
	pt->y = rt->bottom - rt->top;
}

QN_INLINE float qn_point_aspect(const qnPoint* pt)
{
	return (float)pt->x / (float)pt->y;
}

QN_INLINE bool qn_point_between(const qnPoint* pt, int32_t v)
{
	return v > pt->min && v < pt->max;
}



//////////////////////////////////////////////////////////////////////////
// rect

QN_INLINE void qn_rect_set(qnRect* prt, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
	prt->left = left;
	prt->top = top;
	prt->right = right;
	prt->bottom = bottom;
}

QN_INLINE void qn_rect_set_size(qnRect* p, int32_t x, int32_t y, int32_t w, int32_t h)
{
	p->left = x;
	p->top = y;
	p->right = x + w;
	p->bottom = y + h;
}

QN_INLINE void qn_rect_zero(qnRect* prt)
{
	prt->left = 0;
	prt->top = 0;
	prt->right = 0;
	prt->bottom = 0;
}

QN_INLINE void qn_rect_mag(qnRect* p, const qnRect* rt, int32_t w, int32_t h)
{
	p->left = rt->left - w;
	p->top = rt->top - h;
	p->right = rt->right + w;
	p->bottom = rt->bottom + h;
}

QN_INLINE void qn_rect_inflate(qnRect* p, const qnRect* rt, int32_t l, int32_t t, int32_t r, int32_t b)
{
	p->left = rt->left - l;
	p->top = rt->top - t;
	p->right = rt->right + r;
	p->bottom = rt->bottom + b;
}

QN_INLINE void qn_rect_deflate(qnRect* p, const qnRect* rt, int32_t l, int32_t t, int32_t r, int32_t b)
{
	p->left = rt->left + l;
	p->top = rt->top + t;
	p->right = rt->right - r;
	p->bottom = rt->bottom - b;
}

QN_INLINE void qn_rect_offset(qnRect* p, const qnRect* rt, int32_t l, int32_t t, int32_t r, int32_t b)
{
	p->left = rt->left + l;
	p->top = rt->top + t;
	p->right = rt->right + r;
	p->bottom = rt->bottom + b;
}

QN_INLINE int32_t qn_rect_width(const qnRect* prt)
{
	return prt->right - prt->left;
}

QN_INLINE int32_t qn_rect_height(const qnRect* prt)
{
	return prt->bottom - prt->top;
}

QN_INLINE bool qn_rect_in(const qnRect* prt, int32_t x, int32_t y)
{
	return (x >= prt->left && x <= prt->right && y >= prt->top && y <= prt->bottom);
}

QN_INLINE bool qn_rect_eq(const qnRect* r1, const qnRect* r2)
{
	return r1->left == r2->left && r1->right == r2->right && r1->top == r2->top && r1->bottom == r2->bottom;
}

QN_INLINE bool qn_rect_intersect(qnRect* p, const qnRect* r1, const qnRect* r2)
{
	bool b = r2->left < r1->right && r2->right > r1->left && r2->top < r1->bottom && r2->bottom > r1->top;
	if (!b)
		qn_rect_set(p, 0, 0, 0, 0);
	else
	{
		qn_rect_set(p,
			QN_MAX(r1->left, r2->left), QN_MAX(r1->top, r2->top),
			QN_MIN(r1->right, r2->right), QN_MIN(r1->bottom, r2->bottom));
	}
	return b;
}



//////////////////////////////////////////////////////////////////////////
// quad

QN_INLINE void qn_rectf_set(qnRectF* pq, float left, float top, float right, float bottom)
{
	pq->left = left;
	pq->top = top;
	pq->right = right;
	pq->bottom = bottom;
}

QN_INLINE void qn_rectf_zero(qnRectF* pq)
{
	pq->left = 0.0f;
	pq->top = 0.0f;
	pq->right = 0.0f;
	pq->bottom = 0.0f;
}

QN_INLINE void qn_rectf_mag(qnRectF* p, const qnRectF* rt, float w, float h)
{
	p->left = rt->left - w;
	p->top = rt->top - h;
	p->right = rt->right + w;
	p->bottom = rt->bottom + h;
}

QN_INLINE void qn_rectf_inflate(qnRectF* p, const qnRectF* rt, float l, float t, float r, float b)
{
	p->left = rt->left - l;
	p->top = rt->top - t;
	p->right = rt->right + r;
	p->bottom = rt->bottom + b;
}

QN_INLINE void qn_rectf_deflate(qnRectF* p, const qnRectF* rt, float l, float t, float r, float b)
{
	p->left = rt->left + l;
	p->top = rt->top + t;
	p->right = rt->right - r;
	p->bottom = rt->bottom - b;
}

QN_INLINE void qn_rectf_offset(qnRectF* p, const qnRectF* rt, float l, float t, float r, float b)
{
	p->left = rt->left + l;
	p->top = rt->top + t;
	p->right = rt->right + r;
	p->bottom = rt->bottom + b;
}

QN_INLINE float qn_rectf_width(const qnRectF* pq)
{
	return pq->right - pq->left;
}

QN_INLINE float qn_rectf_height(const qnRectF* pq)
{
	return pq->bottom - pq->top;
}

QN_INLINE bool qn_rectf_in(const qnRectF* pq, float x, float y)
{
	return (x >= pq->left && x <= pq->right && y >= pq->top && y <= pq->bottom);
}

QN_INLINE bool qn_rectf_intersect(qnRectF* p, const qnRectF* q1, const qnRectF* q2)
{
	bool b = q2->left < q1->right && q2->right > q1->left && q2->top < q1->bottom && q2->bottom > q1->top;
	if (!b)
		qn_rectf_set(p, 0.0f, 0.0f, 0.0f, 0.0f);
	else
	{
		qn_rectf_set(p,
			QN_MAX(q1->left, q2->left), QN_MAX(q1->top, q2->top),
			QN_MIN(q1->right, q2->right), QN_MIN(q1->bottom, q2->bottom));
	}
	return b;
}


//////////////////////////////////////////////////////////////////////////
// color

QN_INLINE void qn_color_set(qnColor* pc, float r, float g, float b, float a)
{
	pc->r = r;
	pc->g = g;
	pc->b = b;
	pc->a = a;
}

QN_INLINE void qn_color_set_uint(qnColor* pc, uint32_t value)
{
	float f = 1.0f / 255.0f;
	pc->b = (float)(value & 255) * f; value >>= 8;
	pc->g = (float)(value & 255) * f; value >>= 8;
	pc->r = (float)(value & 255) * f; value >>= 8;
	pc->a = (float)(value & 255) * f; //value >>= 8;
}

QN_INLINE void qn_color_set_kolor(qnColor* pc, const qnKolor* cu)
{
	float f = 1.0f / 255.0f;
	pc->a = (float)cu->a * f;
	pc->r = (float)cu->r * f;
	pc->g = (float)cu->g * f;
	pc->b = (float)cu->b * f;
}

QN_INLINE void qn_color_add(qnColor* pc, const qnColor* left, const qnColor* right)
{
	pc->r = left->r + right->r;
	pc->g = left->g + right->b;
	pc->b = left->b + right->b;
	pc->a = left->a + right->a;
}

QN_INLINE void qn_color_sub(qnColor* pc, const qnColor* left, const qnColor* right)
{
	pc->r = left->r - right->r;
	pc->g = left->g - right->b;
	pc->b = left->b - right->b;
	pc->a = left->a - right->a;
}

QN_INLINE void qn_color_mag(qnColor* pc, const qnColor* left, float scale)
{
	pc->r = left->r * scale;
	pc->g = left->g * scale;
	pc->b = left->b * scale;
	pc->a = left->a * scale;
}

QN_INLINE void qn_color_neg(qnColor* pc, const qnColor* c)
{
	pc->r = 1.0f - c->r;
	pc->g = 1.0f - c->g;
	pc->b = 1.0f - c->b;
	pc->a = c->a;
}

QN_INLINE void qn_color_interpolate(qnColor* pc, const qnColor* left, const qnColor* right, float s)
{
	// left=tocolor, right=fromcolor
	float f = 1.0f - s;
	pc->r = right->r * f + left->r * s;
	pc->g = right->g * f + left->g * s;
	pc->b = right->b * f + left->b * s;
	pc->a = right->a * f + left->a * s;
}

QN_INLINE void qn_color_lerp(qnColor* pc, const qnColor* left, const qnColor* right, float s)
{
	pc->r = left->r + s * (right->r - left->r);
	pc->g = left->g + s * (right->g - left->g);
	pc->b = left->b + s * (right->b - left->b);
	pc->a = left->a + s * (right->a - left->a);
}

QN_INLINE void qn_color_mod(qnColor* pc, const qnColor* left, const qnColor* right)
{
	pc->r = left->r * right->r;
	pc->g = left->g * right->g;
	pc->b = left->b * right->b;
	pc->a = left->a * right->a;
}

QN_INLINE void qn_color_min(qnColor* pc, const qnColor* left, const qnColor* right)
{
	pc->r = (left->r < right->r) ? left->r : right->r;
	pc->g = (left->g < right->g) ? left->g : right->g;
	pc->b = (left->b < right->b) ? left->b : right->b;
	pc->a = (left->a < right->a) ? left->a : right->a;
}

QN_INLINE void qn_color_max(qnColor* pc, const qnColor* left, const qnColor* right)
{
	pc->r = (left->r > right->r) ? left->r : right->r;
	pc->g = (left->g > right->g) ? left->g : right->g;
	pc->b = (left->b > right->b) ? left->b : right->b;
	pc->a = (left->a > right->a) ? left->a : right->a;
}

QN_INLINE float* qn_color_float(qnColor* c)
{
	return &c->r;
}

QN_INLINE uint32_t qn_color_uint(const qnColor* pc)
{
	uint8_t R = (uint8_t)(pc->r * 255.0f + 0.5f);
	uint8_t G = (uint8_t)(pc->g * 255.0f + 0.5f);
	uint8_t B = (uint8_t)(pc->b * 255.0f + 0.5f);
	uint8_t A = (uint8_t)(pc->a * 255.0f + 0.5f);
	return ((uint32_t)A << 24) | ((uint32_t)R << 16) | ((uint32_t)G << 8) | (uint32_t)B;
}

QN_INLINE uint32_t qn_color_uint_exact(const qnColor* pc)
{
	uint32_t R = (pc->r >= 1.0f) ? 0xff : (pc->r <= 0.0f) ? 0x00 : (uint32_t)(pc->r * 255.0f + 0.5f);
	uint32_t G = (pc->g >= 1.0f) ? 0xff : (pc->g <= 0.0f) ? 0x00 : (uint32_t)(pc->g * 255.0f + 0.5f);
	uint32_t B = (pc->b >= 1.0f) ? 0xff : (pc->b <= 0.0f) ? 0x00 : (uint32_t)(pc->b * 255.0f + 0.5f);
	uint32_t A = (pc->a >= 1.0f) ? 0xff : (pc->a <= 0.0f) ? 0x00 : (uint32_t)(pc->a * 255.0f + 0.5f);
	return (A << 24) | (R << 16) | (G << 8) | B;
}

QN_INLINE bool qn_color_eq(const qnColor* left, const qnColor* right)
{
	return
		qn_eqf(left->r, right->r) &&
		qn_eqf(left->g, right->g) &&
		qn_eqf(left->b, right->b) &&
		qn_eqf(left->a, right->a);
}

QN_INLINE void qn_color_contrast(qnColor* pc, const qnColor* c, float s)
{
	pc->r = 0.5f + s * (c->r - 0.5f);
	pc->g = 0.5f + s * (c->g - 0.5f);
	pc->b = 0.5f + s * (c->b - 0.5f);
	pc->a = c->a;
}

QN_INLINE void qn_color_saturation(qnColor* pc, const qnColor* c, float s)
{
	float g = c->r * 0.2125f + c->g * 0.7154f + c->b * 0.0721f;
	pc->r = g + s * (c->r - g);
	pc->g = g + s * (c->g - g);
	pc->b = g + s * (c->b - g);
	pc->a = c->a;
}


//////////////////////////////////////////////////////////////////////////
// byte color

QN_INLINE void qn_kolor_set(qnKolor* pc, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	pc->r = r;
	pc->g = g;
	pc->b = b;
	pc->a = a;
}

QN_INLINE void qn_kolor_set_float(qnKolor* pc, float r, float g, float b, float a)
{
	pc->r = (uint8_t)rintf(r * 255.0f);
	pc->g = (uint8_t)rintf(g * 255.0f);
	pc->b = (uint8_t)rintf(b * 255.0f);
	pc->a = (uint8_t)rintf(a * 255.0f);
}

QN_INLINE void qn_kolor_set_uint(qnKolor* pc, uint32_t value)
{
	pc->u = value;
}

QN_INLINE void qn_kolor_set_color(qnKolor* pc, const qnColor* cr)
{
	pc->r = (uint8_t)rintf(cr->r * 255.0f);
	pc->g = (uint8_t)rintf(cr->g * 255.0f);
	pc->b = (uint8_t)rintf(cr->b * 255.0f);
	pc->a = (uint8_t)rintf(cr->a * 255.0f);
}

QN_INLINE void qn_kolor_add(qnKolor* pc, const qnKolor* left, const qnKolor* right)
{
	pc->u = left->u + right->u;
}

QN_INLINE void qn_kolor_sub(qnKolor* pc, const qnKolor* left, const qnKolor* right)
{
	pc->u = left->u - right->u;
}

QN_INLINE void qn_kolor_mag(qnKolor* pc, const qnKolor* left, float scale)
{
	pc->r = (uint8_t)rintf(left->r * scale);
	pc->g = (uint8_t)rintf(left->g * scale);
	pc->b = (uint8_t)rintf(left->b * scale);
	pc->a = (uint8_t)rintf(left->a * scale);
}

QN_INLINE void qn_kolor_neg(qnKolor* pc, const qnKolor* c)
{
	pc->r = 255 - c->r;
	pc->g = 255 - c->g;
	pc->b = 255 - c->b;
	pc->a = c->a;
}

QN_INLINE void qn_kolor_interpolate(qnKolor* pc, const qnKolor* left, const qnKolor* right, float s)
{
	// left=tocolor, right=fromcolor
	float f = 1.0f - s;
	pc->r = (uint8_t)rintf(right->r * f + left->r * s);
	pc->g = (uint8_t)rintf(right->g * f + left->g * s);
	pc->b = (uint8_t)rintf(right->b * f + left->b * s);
	pc->a = (uint8_t)rintf(right->a * f + left->a * s);
}

QN_INLINE void qn_kolor_lerp(qnKolor* pc, const qnKolor* left, const qnKolor* right, float s)
{
	pc->r = (uint8_t)rintf(left->r + s * (right->r - left->r));
	pc->g = (uint8_t)rintf(left->g + s * (right->g - left->g));
	pc->b = (uint8_t)rintf(left->b + s * (right->b - left->b));
	pc->a = (uint8_t)rintf(left->a + s * (right->a - left->a));
}

QN_INLINE void qn_kolor_mod(qnKolor* pc, const qnKolor* left, const qnKolor* right)
{
	pc->r = left->r * right->r;
	pc->g = left->g * right->g;
	pc->b = left->b * right->b;
	pc->a = left->a * right->a;
}

QN_INLINE void qn_kolor_min(qnKolor* pc, const qnKolor* left, const qnKolor* right)
{
	pc->r = (left->r < right->r) ? left->r : right->r;
	pc->g = (left->g < right->g) ? left->g : right->g;
	pc->b = (left->b < right->b) ? left->b : right->b;
	pc->a = (left->a < right->a) ? left->a : right->a;
}

QN_INLINE void qn_kolor_max(qnKolor* pc, const qnKolor* left, const qnKolor* right)
{
	pc->r = (left->r > right->r) ? left->r : right->r;
	pc->g = (left->g > right->g) ? left->g : right->g;
	pc->b = (left->b > right->b) ? left->b : right->b;
	pc->a = (left->a > right->a) ? left->a : right->a;
}

QN_INLINE bool qn_kolor_eq(const qnKolor* left, const qnKolor* right)
{
	return left->u == right->u;
}

QN_INLINE void qn_kolor_contrast(qnKolor* pc, const qnKolor* c, float s)
{
	pc->r = (uint8_t)rintf(s * (c->r - 0.5f));
	pc->g = (uint8_t)rintf(s * (c->g - 0.5f));
	pc->b = (uint8_t)rintf(s * (c->b - 0.5f));
	pc->a = c->a;
}

QN_INLINE void qn_kolor_saturation(qnKolor* pc, const qnKolor* c, float s)
{
	float g = c->r * 0.2125f + c->g * 0.7154f + c->b * 0.0721f;
	pc->r = (uint8_t)rintf(g + s * (c->r - g));
	pc->g = (uint8_t)rintf(g + s * (c->g - g));
	pc->b = (uint8_t)rintf(g + s * (c->b - g));
	pc->a = c->a;
}


//////////////////////////////////////////////////////////////////////////
// plane

QN_INLINE void qn_plane_rst(qnPlane* pp)
{
	pp->a = pp->b = pp->c = 0.0f;
	pp->d = 1.0f;
}

QN_INLINE void qn_plane_set(qnPlane* pp, float a, float b, float c, float d)
{
	pp->a = a;
	pp->b = b;
	pp->c = c;
	pp->d = d;
}

QN_INLINE float qn_plane_dot_coord(const qnPlane* pp, const qnVec3* v)
{
	return pp->a * v->x + pp->b * v->y + pp->c * v->z + pp->d;
}

QN_INLINE void qn_plane_from_point_norm(qnPlane* pp, const qnVec3* pv, const qnVec3* pn)
{
	pp->a = pn->x;
	pp->b = pn->y;
	pp->c = pn->z;
	pp->d = -qn_vec3_dot(pv, pn);
}

QN_INLINE int32_t qn_plane_face_point(const qnPlane* p, const qnVec3* v)
{
	float f = qn_vec3_dot((const qnVec3*)p, v) + p->d;
	if (f < -QN_EPSILON)
		return -1;  // back
	if (f > QN_EPSILON)
		return 1;   // front
	return 0;       // on
}

QN_INLINE float qn_plane_dist(const qnPlane* p, const qnVec3* v)
{
	return qn_vec3_dot(v, (const qnVec3*)p) + p->d;
}

QN_INLINE void qn_plane_norm(qnPlane* pp, const qnPlane* p)
{
	float f = sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
	if (f > 0.0f) f = 1.0f / f;
	pp->a = p->a * f;
	pp->b = p->b * f;
	pp->c = p->c * f;
	pp->d = p->d * f;
}

QN_INLINE void qn_plane_rnorm(qnPlane* pp, const qnPlane* p)
{
	float n = -1.0f / sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
	pp->a = p->a * n;
	pp->b = p->b * n;
	pp->c = p->c * n;
	pp->d = p->d * n;
}

QN_INLINE void qn_plane_trfm(qnPlane* pp, const qnPlane* plane, const qnMat4* trfm)
{
	qnVec3 v, n, s;

	qn_vec3_mag(&v, (const qnVec3*)plane, -plane->d);
	qn_vec3_trfm_coord(&v, &v, trfm);
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

QN_INLINE void qn_plane_points(qnPlane* pp, const qnVec3* v1, const qnVec3* v2, const qnVec3* v3)
{
	qnVec3 t0, t1, t2;
	qn_vec3_sub(&t0, v2, v1);
	qn_vec3_sub(&t1, v3, v2);
	qn_vec3_cross(&t2, &t0, &t1);
	qn_vec3_norm(&t2, &t2);
	qn_plane_set(pp, t2.x, t2.y, t2.z, -qn_vec3_dot(v1, &t2));
}

QN_INLINE bool qn_plane_intersect(const qnPlane* p, qnVec3* loc, qnVec3* dir, const qnPlane* o)
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

QN_INLINE float qn_plane_intersect_line(const qnPlane* p, const qnVec3* v1, const qnVec3* v2)
{
	qnVec3 t;
	qn_vec3_sub(&t, v2, v1);
	float f = 1.0f / qn_vec3_dot((const qnVec3*)p, &t);
	return -(qn_vec3_dot((const qnVec3*)p, v1) + p->d) * f;
}

QN_INLINE bool qn_plane_intersect_plane(const qnPlane* p, const qnPlane* o1, const qnPlane* o2)
{
	qnVec3 v, dir, loc;
	return (qn_plane_intersect(p, &loc, &dir, o1)) ? qn_vec3_intersect_line(&v, o2, &loc, &dir) : false;
}


//////////////////////////////////////////////////////////////////////////
// line3

QN_INLINE void qn_line3_set(qnLine3* p, float bx, float by, float bz, float ex, float ey, float ez)
{
	qn_vec3_set(&p->begin, bx, by, bz);
	qn_vec3_set(&p->end, ex, ey, ez);
}

QN_INLINE void qn_line3_set_vec(qnLine3* p, const qnVec3* begin, const qnVec3* end)
{
	p->begin = *begin;
	p->end = *end;
}

QN_INLINE void qn_line3_add(qnLine3* p, const qnLine3* l, const qnVec3* v)
{
	qn_vec3_add(&p->begin, &l->begin, v);
	qn_vec3_add(&p->end, &l->end, v);
}

QN_INLINE void qn_line3_sub(qnLine3* p, const qnLine3* l, const qnVec3* v)
{
	qn_vec3_sub(&p->begin, &l->begin, v);
	qn_vec3_sub(&p->end, &l->end, v);
}

QN_INLINE float qn_line3_len_sq(const qnLine3* p)
{
	return qn_vec3_dist_sq(&p->begin, &p->end);
}

QN_INLINE float qn_line3_len(const qnLine3* p)
{
	return qn_vec3_dist(&p->begin, &p->end);
}

QN_INLINE void qn_line3_center(const qnLine3* p, qnVec3* v)
{
	qn_vec3_add(v, &p->begin, &p->end);
	qn_vec3_mag(v, v, 0.5f);
}

QN_INLINE void qn_line3_vec(const qnLine3* p, qnVec3* v)
{
	qn_vec3_sub(v, &p->end, &p->begin);
}

QN_INLINE bool qn_line3_in(const qnLine3* p, const qnVec3* v)
{
	return qn_vec3_between(v, &p->begin, &p->end);
}

QN_INLINE bool qn_line3_intersect_sphere(const qnLine3* p, const qnVec3* org, float rad, float* dist)
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


//////////////////////////////////////////////////////////////////////////
// half

// half type
QN_INLINE half_t qn_float2half(const float v)
{
#if _MSC_VER
	_ReadWriteBarrier();
#elif __GNUC__
#if __X86__ || _M_X86 || __AMD64__ || __x86_64__
	__asm__ __volatile__("" : : : "memory");
#else
	__sync_synchronize();
#endif
#endif

	uint32_t u = *(const uint32_t*)&v;
	uint32_t s = (u & 0x80000000U) >> 16U;
	u = u & 0x7FFFFFFFU;

	uint32_t r;
	if (u > 0x47FFEFFFU)
		r = 0x7FFFU;
	else
	{
		if (u >= 0x38800000U)
			u += 0xC8000000U;
		else
		{
			uint32_t t = 113U - (u >> 23U);
			u = (0x800000U | (u & 0x7FFFFFU)) >> t;
		}

		r = ((u + 0x0FFFU + ((u >> 13U) & 1U)) >> 13U) & 0x7FFFU;
	}

	return (half_t)(r | s);
}

QN_INLINE float qn_half2float(const half_t v)
{
#if _MSC_VER
	_ReadWriteBarrier();
#elif __GNUC__
#if __X86__ || _M_X86 || __AMD64__ || __x86_64__
	__asm__ __volatile__("" : : : "memory");
#else
	__sync_synchronize();
#endif
#endif

	uint32_t e, m = (uint32_t)(v & 0x03FF);

	if ((v & 0x7C00) != 0)
		e = (uint32_t)((v >> 10) & 0x1F);
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
		e = (uint32_t)-112;

	uint32_t r = ((v & 0x8000) << 16) | ((e + 112) << 23) | (m << 13);
	return *(float*)&r;
}

// vec half
QN_INLINE void qn_vec2h_set(qnVecH2* p, float x, float y)
{
	p->x = qn_float2half(x);
	p->y = qn_float2half(y);
}

QN_INLINE void qn_vec3h_set(qnVecH3* p, float x, float y, float z)
{
	p->x = qn_float2half(x);
	p->y = qn_float2half(y);
	p->z = qn_float2half(z);
}

QN_INLINE void qn_vec4h_set(qnVecH4* p, float x, float y, float z, float w)
{
	p->x = qn_float2half(x);
	p->y = qn_float2half(y);
	p->z = qn_float2half(z);
	p->w = qn_float2half(w);
}
