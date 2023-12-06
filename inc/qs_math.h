#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)		// L4, nonstandard extension used : nameless struct/union
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#include <math.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif

//////////////////////////////////////////////////////////////////////////
// constant
#define QN_EPSILON						0.0001
#define QN_E							2.7182818284590452353602874713526624977572470937000
#define QN_LOG2_E						1.44269504088896340736
#define QN_LOG10_E						0.434294481903251827651
#define QN_LOG2_B10						0.30102999566398119521
#define QN_LN2							0.6931471805599453094172321214581765680755001343603
#define QN_LN10							2.3025850929940456840179914546843642076011014886288
#define QN_PI							3.1415926535897932384626433832795028841971693993751
#define QN_PI2							6.2831853071795864769252867665590057683943387987502
#define QN_PI_H							1.5707963267948966192313216916397514420985846996876
#define QN_PI_Q							0.7853981633974483096156608458198757210492923498438
#define QN_SQRT2						1.4142135623730950488016887242096980785696718753769
#define QN_SQRTH						0.7071067811865475244008443621048490392848359376884


//////////////////////////////////////////////////////////////////////////
// macro & iline
#define QN_FRACT(f)						((f)-floorf(f))
#define QN_TORADIAN(degree)				((degree)*(180.0f/(float)QN_PI))
#define QN_TODEGREE(radian)				((radian)*((float)QN_PI/180.0f))
#define QN_EQEPS(a,b,epsilon)			(((a)+(epsilon)>(b)) && ((a)-(epsilon)<(b)))

QN_INLINE bool qn_eqf(const float a, const float b) { return QN_EQEPS(a, b, (float)QN_EPSILON); }
QN_INLINE float qn_maxf(const float a, const float b) { return QN_MAX(a, b); }
QN_INLINE float qn_minf(const float a, const float b) { return QN_MIN(a, b); }
QN_INLINE float qn_absf(const float v) { return QN_ABS(v); }
QN_INLINE float qn_clampf(const float v, const float min, const float max) { return QN_CLAMP(v, min, max); }
QN_INLINE float qn_cradf(const float v) { return v < (float)-QN_PI_H ? v + (float)QN_PI_H : v >(float)QN_PI_H ? v - (float)QN_PI_H : v; }
QN_INLINE float qn_lerpf(const float l, const float r, const float f) { return l + f * (r - l); }
QN_INLINE void qn_sincosf(const float f, float* s, float* c) { *s = sinf(f); *c = cosf(f); }


//////////////////////////////////////////////////////////////////////////
// types

typedef struct QnVec2					QnVec2;
typedef struct QnVec3					QnVec3;
typedef struct QnVec4					QnVec4;
typedef struct QnQuat					QnQuat;
typedef struct QnMat4					QnMat4;
typedef struct QnPoint					QnPoint;
typedef struct QnSize					QnSize;
typedef struct QnRect					QnRect;
typedef struct QnRectF					QnRectF;
typedef struct QnCoord					QnCoord;
typedef struct QnColor					QnColor;
typedef struct QnKolor					QnKolor;
typedef struct QnKolorU					QnKolorU;
typedef struct QnPlane					QnPlane;
typedef struct QnLine3					QnLine3;
typedef struct QnTrfm					QnTrfm;
typedef struct QnVecH2					QnVecH2;
typedef struct QnVecH3					QnVecH3;
typedef struct QnVecH4					QnVecH4;

// vector2
struct QnVec2
{
	float x, y;
};

// vector3
struct QnVec3
{
	float x, y, z;
};

// vector4
struct QnVec4
{
	float x, y, z, w;
};

// quaternion
struct QnQuat
{
	float x, y, z, w;
};

// matrix4
struct QnMat4
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

// point
struct QnPoint
{
	int x, y;
};

// size
struct QnSize
{
	int width, height;
};

// rect
struct QnRect
{
	int left, top, right, bottom;
};

//
struct QnCoord
{
	float u, v;
};

// color
struct QnColor
{
	float r, g, b, a;
};

// byte color
QN_ALIGN(4) struct QnKolor
{
	byte b, g, r, a;
};

// byte color uni
struct QnKolorU
{
	union
	{
		QnKolor k;
		uint u;
	};
};

// plane
struct QnPlane
{
	float a, b, c, d;
};

// line3
struct QnLine3
{
	QnVec3 begin, end;
};

// transform
struct QnTrfm
{
	QnVec3 loc;
	QnQuat rot;
	QnVec3 scl;
};

// half vector2
struct QnVecH2
{
	halfint x, y;
};

// half vector3
struct QnVecH3
{
	halfint x, y, z;
};

// hafl vector4
struct QnVecH4
{
	halfint x, y, z, w;
};


//////////////////////////////////////////////////////////////////////////
// functions

// external function
QN_EXTC_BEGIN
// vector3
QSAPI void qn_vec3_closed_line(QnVec3* pv, const QnLine3* line, const QnVec3* loc);
QSAPI void qn_vec3_lerp_len(QnVec3* pv, const QnVec3* left, const QnVec3* right, float scale, float len);
QSAPI void qn_vec3_form_norm(QnVec3* pv, const QnVec3* v0, const QnVec3* v1, const QnVec3* v2);
QSAPI bool qn_vec3_reflect(QnVec3* pv, const QnVec3* in, const QnVec3* dir);
QSAPI bool qn_vec3_intersect_line(QnVec3* pv, const QnPlane* plane, const QnVec3* loc, const QnVec3* dir);
QSAPI bool qn_vec3_intersect_point(QnVec3* pv, const QnPlane* plane, const QnVec3* v1, const QnVec3* v2);
QSAPI bool qn_vec3_intersect_between_point(QnVec3* pv, const QnPlane* plane, const QnVec3* v1, const QnVec3* v2);
QSAPI bool qn_vec3_intersect_planes(QnVec3* pv, const QnPlane* plane, const QnPlane* other1, const QnPlane* other2);
// quaternion
QSAPI void qn_quat_slerp(QnQuat* pq, const QnQuat* left, const QnQuat* right, float change);
QSAPI void qn_quat_mat4(QnQuat* pq, const QnMat4* rot);
QSAPI void qn_quat_vec(QnQuat* pq, const QnVec3* rot);
QSAPI void qn_quat_ln(QnQuat* pq, const QnQuat* q);
// matrix4
QSAPI void qn_mat4_tran(QnMat4* pm, const QnMat4* m);
QSAPI void qn_mat4_mul(QnMat4* pm, const QnMat4* left, const QnMat4* right);
QSAPI void qn_mat4_inv(QnMat4* pm, const QnMat4* m, float* /*NULLABLE*/determinant);
QSAPI void qn_mat4_tmul(QnMat4* pm, const QnMat4* left, const QnMat4* right);
QSAPI void qn_mat4_shadow(QnMat4* pm, const QnVec4* light, const QnPlane* plane);
QSAPI void qn_mat4_affine(QnMat4* pm, const QnVec3* scl, const QnVec3* rotcenter, const QnQuat* rot, const QnVec3* loc);
QSAPI void qn_mat4_trfm(QnMat4* m, const QnVec3* loc, const QnQuat* rot, const QnVec3* scl);
QSAPI void qn_mat4_trfm_vec(QnMat4* m, const QnVec3* loc, const QnVec3* rot, const QnVec3* scl);
QSAPI float qn_mat4_det(const QnMat4* m);
// plane
QSAPI void qn_plane_trfm(QnPlane* pp, const QnPlane* plane, const QnMat4* trfm);
QSAPI void qn_plane_points(QnPlane* pp, const QnVec3* v1, const QnVec3* v2, const QnVec3* v3);
QSAPI bool qn_plane_intersect(const QnPlane* p, QnVec3* loc, QnVec3* dir, const QnPlane* o);
// line3
QSAPI bool qn_line3_intersect_sphere(const QnLine3* p, const QnVec3* org, float rad, float* dist);
QN_EXTC_END

// vector2

QN_INLINE void qn_vec2_set(QnVec2* pv, float x, float y)
{
	pv->x = x;
	pv->y = y;
}

QN_INLINE void qn_vec2_rst(QnVec2* pv) // identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
}

QN_INLINE void qn_vec2_diag(QnVec2* pv, float v)
{
	pv->x = v;
	pv->y = v;
}

QN_INLINE float qn_vec2_dot(const QnVec2* left, const QnVec2* right)
{
	return left->x * right->x + left->y * right->y;
}

QN_INLINE float qn_vec2_len_sq(const QnVec2* pv)
{
	return pv->x * pv->x + pv->y * pv->y;
}

QN_INLINE float qn_vec2_len(const QnVec2* pv)
{
	return sqrtf(qn_vec2_len_sq(pv));
}

QN_INLINE void qn_vec2_norm(QnVec2* pv, const QnVec2* v)
{
	const float f = 1.0f / qn_vec2_len(v);
	pv->x = v->x * f;
	pv->y = v->y * f;
}

QN_INLINE void qn_vec2_add(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
}

QN_INLINE void qn_vec2_sub(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
}

QN_INLINE void qn_vec2_mag(QnVec2* pv, const QnVec2* left, float right)
{
	pv->x = left->x * right;
	pv->y = left->y * right;
}

QN_INLINE float qn_vec2_accm(const QnVec2* pv, float s)
{
	return (pv->x + pv->y) * s;
}

QN_INLINE void qn_vec2_cross(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = left->y * right->x - left->x * right->y;
	pv->y = left->x * right->y - left->y * right->x;
}

QN_INLINE void qn_vec2_ivt(QnVec2* pv, const QnVec2* v)  // invert
{
	pv->x = -v->x;
	pv->y = -v->y;
}

QN_INLINE void qn_vec2_neg(QnVec2* pv, const QnVec2* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
}

QN_INLINE bool qn_vec2_eq(const QnVec2* left, const QnVec2* right)
{
	return qn_eqf(left->x, right->x) && qn_eqf(left->y, right->y);
}

QN_INLINE bool qn_vec2_isi(const QnVec2* pv)
{
	return qn_eqf(pv->x, 0.0f) && qn_eqf(pv->y, 0.0f);
}

QN_INLINE void qn_vec2_interpolate(QnVec2* pv, const QnVec2* left, const QnVec2* right, float s)
{
	const float f = 1.0f - s;
	pv->x = right->x * f + left->x * s;
	pv->y = right->y * f + left->y * s;
}

QN_INLINE void qn_vec2_lerp(QnVec2* pv, const QnVec2* left, const QnVec2* right, float s)
{
	pv->x = left->x + s * (right->x - left->x);
	pv->y = left->y + s * (right->y - left->y);
}

QN_INLINE void qn_vec2_min(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = (left->x < right->x) ? left->x : right->x;
	pv->y = (left->y < right->y) ? left->y : right->y;
}

QN_INLINE void qn_vec2_max(QnVec2* pv, const QnVec2* left, const QnVec2* right)
{
	pv->x = (left->x > right->x) ? left->x : right->x;
	pv->y = (left->y > right->y) ? left->y : right->y;
}

// vector3

QN_INLINE void qn_vec3_set(QnVec3* pv, float x, float y, float z)
{
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

QN_INLINE void qn_vec3_rst(QnVec3* pv)	// identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
	pv->z = 0.0f;
}

QN_INLINE void qn_vec3_diag(QnVec3* pv, float v)
{
	pv->x = v;
	pv->y = v;
	pv->z = v;
}

QN_INLINE float qn_vec3_len_sq(const QnVec3* pv)
{
	return pv->x * pv->x + pv->y * pv->y + pv->z * pv->z;
}

QN_INLINE float qn_vec3_len(const QnVec3* pv)
{
	return sqrtf(qn_vec3_len_sq(pv));
}

QN_INLINE void qn_vec3_norm(QnVec3* pv, const QnVec3* v)
{
	const float f = 1.0f / qn_vec3_len(v);
	pv->x = v->x * f;
	pv->y = v->y * f;
	pv->z = v->z * f;
}

QN_INLINE void qn_vec3_add(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
	pv->z = left->z + right->z;
}

QN_INLINE void qn_vec3_sub(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
	pv->z = left->z - right->z;
}

QN_INLINE void qn_vec3_mag(QnVec3* pv, const QnVec3* v, float scale)
{
	pv->x = v->x * scale;
	pv->y = v->y * scale;
	pv->z = v->z * scale;
}

QN_INLINE float qn_vec3_accm(const QnVec3* pv, float scale)
{
	return (pv->x + pv->y + pv->z) * scale;
}

QN_INLINE void qn_vec3_cross(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->y * right->z - left->z * right->y;
	pv->y = left->z * right->x - left->x * right->z;
	pv->z = left->x * right->y - left->y * right->x;
}

QN_INLINE void qn_vec3_ivt(QnVec3* pv, const QnVec3* v)  // invert
{
	pv->x = -v->x;
	pv->y = -v->y;
	pv->z = -v->z;
}

QN_INLINE void qn_vec3_neg(QnVec3* pv, const QnVec3* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
	pv->z = 1.0f - v->z;
}

QN_INLINE float qn_vec3_dot(const QnVec3* left, const QnVec3* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z;
}

QN_INLINE void qn_vec3_min(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->x < right->x ? left->x : right->x;
	pv->y = left->y < right->y ? left->y : right->y;
	pv->z = left->z < right->z ? left->z : right->z;
}

QN_INLINE void qn_vec3_max(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	pv->x = left->x > right->x ? left->x : right->x;
	pv->y = left->y > right->y ? left->y : right->y;
	pv->z = left->z > right->z ? left->z : right->z;
}

QN_INLINE bool qn_vec3_eq(const QnVec3* left, const QnVec3* right)
{
	return qn_eqf(left->x, right->x) && qn_eqf(left->y, right->y) && qn_eqf(left->z, right->z);
}

QN_INLINE bool qn_vec3_isi(const QnVec3* pv)
{
	return (pv->x == 0.0f && pv->y == 0.0f && pv->z == 0.0f);
}

QN_INLINE float qn_vec3_dist_sq(const QnVec3* left, const QnVec3* right)
{
	QnVec3 t;
	qn_vec3_sub(&t, left, right);
	return qn_vec3_len_sq(&t);
}

QN_INLINE float qn_vec3_dist(const QnVec3* left, const QnVec3* right)
{
	return sqrtf(qn_vec3_dist_sq(left, right));
}

QN_INLINE void qn_vec3_dir(QnVec3* pv, const QnVec3* left, const QnVec3* right)
{
	qn_vec3_sub(pv, left, right);
	qn_vec3_norm(pv, pv);
}

QN_INLINE float qn_vec3_rad_sq(const QnVec3* left, const QnVec3* right)
{
	QnVec3 t;
	qn_vec3_add(&t, left, right);
	qn_vec3_mag(&t, &t, 0.5f);
	qn_vec3_sub(&t, &t, left);
	return qn_vec3_len_sq(&t);
}

QN_INLINE float qn_vec3_rad(const QnVec3* left, const QnVec3* right)
{
	return sqrtf(qn_vec3_rad_sq(left, right));
}

QN_INLINE void qn_vec3_trfm(QnVec3* pv, const QnVec3* v, const QnMat4* trfm)
{
	const float x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31 + trfm->_41;
	const float y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32 + trfm->_42;
	const float z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33 + trfm->_43;
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

QN_INLINE void qn_vec3_trfm_norm(QnVec3* pv, const QnVec3* v, const QnMat4* trfm)
{
	const float x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31;
	const float y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32;
	const float z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33;
	pv->x = x;
	pv->y = y;
	pv->z = z;
}

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

QN_INLINE void qn_vec3_interpolate(QnVec3* pv, const QnVec3* left, const QnVec3* right, float scale)
{
	const float f = 1.0f - scale;
	pv->x = right->x * f + left->x * scale;
	pv->y = right->y * f + left->y * scale;
	pv->z = right->z * f + left->z * scale;
}

QN_INLINE void qn_vec3_lerp(QnVec3* pv, const QnVec3* left, const QnVec3* right, float scale)
{
	pv->x = left->x + scale * (right->x - left->x);
	pv->y = left->y + scale * (right->y - left->y);
	pv->z = left->z + scale * (right->z - left->z);
}

QN_INLINE float qn_vec3_yaw(const QnVec3* pv)
{
	return -atanf(pv->z / pv->x) + ((pv->x > 0.0f) ? (float)-QN_PI_H : (float)QN_PI_H);
}

QN_INLINE bool qn_vec3_between(const QnVec3* p, const QnVec3* begin, const QnVec3* end)
{
	QnVec3 t;
	qn_vec3_sub(&t, end, begin);
	const float f = qn_vec3_len_sq(&t);
	return qn_vec3_dist_sq(p, begin) <= f && qn_vec3_dist_sq(p, end) <= f;
}

// vector4

QN_INLINE void qn_vec4_set(QnVec4* pv, float x, float y, float z, float w)
{
	pv->x = x;
	pv->y = y;
	pv->z = z;
	pv->w = w;
}

QN_INLINE void qn_vec4_rst(QnVec4* pv)	// identify
{
	pv->x = 0.0f;
	pv->y = 0.0f;
	pv->z = 0.0f;
	pv->w = 0.0f;
}

QN_INLINE void qn_vec4_diag(QnVec4* pv, float v)
{
	pv->x = v;
	pv->y = v;
	pv->z = v;
	pv->w = v;
}

QN_INLINE float qn_vec4_len_sq(const QnVec4* pv)
{
	return pv->x * pv->x + pv->y * pv->y + pv->z * pv->z + pv->w * pv->w;
}

QN_INLINE float qn_vec4_len(const QnVec4* pv)
{
	return sqrtf(qn_vec4_len_sq(pv));
}

QN_INLINE void qn_vec4_norm(QnVec4* pv, const QnVec4* v)
{
	const float f = 1.0f / qn_vec4_len(v);
	pv->x = v->x * f;
	pv->y = v->y * f;
	pv->z = v->z * f;
	pv->w = v->w * f;
}

QN_INLINE void qn_vec4_add(QnVec4* pv, const QnVec4* left, const QnVec4* right)
{
	pv->x = left->x + right->x;
	pv->y = left->y + right->y;
	pv->z = left->z + right->z;
	pv->w = left->w + right->w;
}

QN_INLINE void qn_vec4_sub(QnVec4* pv, const QnVec4* left, const QnVec4* right)
{
	pv->x = left->x - right->x;
	pv->y = left->y - right->y;
	pv->z = left->z - right->z;
	pv->w = left->w - right->w;
}

QN_INLINE void qn_vec4_mag(QnVec4* pv, const QnVec4* v, float scale)
{
	pv->x = v->x * scale;
	pv->y = v->y * scale;
	pv->z = v->z * scale;
	pv->w = v->w * scale;
}

QN_INLINE float qn_vec4_accm(const QnVec4* pv, float scale)
{
	return (pv->x + pv->y + pv->z * pv->w) * scale;
}

QN_INLINE void qn_vec4_ivt(QnVec4* pv, const QnVec4* v)
{
	pv->x = -v->x;
	pv->y = -v->y;
	pv->z = -v->z;
	pv->w = -v->w;
}

QN_INLINE void qn_vec4_neg(QnVec4* pv, const QnVec4* v)
{
	pv->x = 1.0f - v->x;
	pv->y = 1.0f - v->y;
	pv->z = 1.0f - v->z;
	pv->w = 1.0f - v->w;
}

QN_INLINE float qn_vec4_dot(const QnVec4* left, const QnVec4* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z + left->w * right->w;
}

QN_INLINE void qn_vec4_cross(QnVec4* pv, const QnVec4* v1, const QnVec4* v2, const QnVec4* v3)
{
	pv->x = v1->y * (v2->z * v3->w - v3->z * v2->w) - v1->z * (v2->y * v3->w - v3->y * v2->w) + v1->w * (v2->y * v3->z - v2->z * v3->y);
	pv->y = -(v1->x * (v2->z * v3->w - v3->z * v2->w) - v1->z * (v2->x * v3->w - v3->x * v2->w) + v1->w * (v2->x * v3->z - v3->x * v2->z));
	pv->z = v1->x * (v2->y * v3->w - v3->y * v2->w) - v1->y * (v2->x * v3->w - v3->x * v2->w) + v1->w * (v2->x * v3->y - v3->x * v2->y);
	pv->w = -(v1->x * (v2->y * v3->z - v3->y * v2->z) - v1->y * (v2->x * v3->z - v3->x * v2->z) + v1->z * (v2->x * v3->y - v3->x * v2->y));
}

QN_INLINE void qn_vec4_min(QnVec4* pv, const QnVec4* left, const QnVec4* right)
{
	pv->x = (left->x < right->x) ? left->x : right->x;
	pv->y = (left->y < right->y) ? left->y : right->y;
	pv->z = (left->z < right->z) ? left->z : right->z;
	pv->w = (left->w < right->w) ? left->w : right->w;
}

QN_INLINE void qn_vec4_max(QnVec4* pv, const QnVec4* left, const QnVec4* right)
{
	pv->x = (left->x > right->x) ? left->x : right->x;
	pv->y = (left->y > right->y) ? left->y : right->y;
	pv->z = (left->z > right->z) ? left->z : right->z;
	pv->w = (left->w > right->w) ? left->w : right->w;
}

QN_INLINE bool qn_vec4_eq(const QnVec4* left, const QnVec4* right)
{
	return
		qn_eqf(left->x, right->x) &&
		qn_eqf(left->y, right->y) &&
		qn_eqf(left->z, right->z) &&
		qn_eqf(left->w, right->w);
}

QN_INLINE bool qn_vec4_isi(const QnVec4* pv)
{
	return (pv->x == 0.0f && pv->y == 0.0f && pv->z == 0.0f && pv->w == 0.0f);
}

QN_INLINE void qn_vec4_trfm(QnVec4* pv, const QnVec4* v, const QnMat4* trfm)
{
	pv->x = v->x * trfm->_11 + v->y * trfm->_21 + v->z * trfm->_31 + v->w * trfm->_41;
	pv->y = v->x * trfm->_12 + v->y * trfm->_22 + v->z * trfm->_32 + v->w * trfm->_42;
	pv->z = v->x * trfm->_13 + v->y * trfm->_23 + v->z * trfm->_33 + v->w * trfm->_43;
	pv->w = v->x * trfm->_14 + v->y * trfm->_24 + v->z * trfm->_34 + v->w * trfm->_44;
}

QN_INLINE void qn_vec4_interpolate(QnVec4* pv, const QnVec4* left, const QnVec4* right, float scale)
{
	const float f = 1.0f - scale;
	pv->x = right->x * f + left->x * scale;
	pv->y = right->y * f + left->y * scale;
	pv->z = right->z * f + left->z * scale;
	pv->w = right->w * f + left->w * scale;
}

QN_INLINE void qn_vec4_lerp(QnVec4* pv, const QnVec4* left, const QnVec4* right, float scale)
{
	pv->x = left->x + scale * (right->x - left->x);
	pv->y = left->y + scale * (right->y - left->y);
	pv->z = left->z + scale * (right->z - left->z);
	pv->w = left->w + scale * (right->w - left->w);
}

// quaternion

QN_INLINE void qn_quat_set(QnQuat* pq, float x, float y, float z, float w)
{
	pq->x = x;
	pq->y = y;
	pq->z = z;
	pq->w = w;
}

QN_INLINE void qn_quat_zero(QnQuat* pq)
{
	pq->x = 0.0f;
	pq->y = 0.0f;
	pq->z = 0.0f;
	pq->w = 0.0f;
}

QN_INLINE void qn_quat_rst(QnQuat* pq)	// identify
{
	pq->x = 0.0f;
	pq->y = 0.0f;
	pq->z = 0.0f;
	pq->w = 1.0f;
}

QN_INLINE float qn_quat_len_sq(const QnQuat* pq)
{
	return pq->x * pq->x + pq->y * pq->y + pq->z * pq->z + pq->w * pq->w;
}

QN_INLINE float qn_quat_len(const QnQuat* pq)
{
	return sqrtf(qn_quat_len_sq(pq));
}

QN_INLINE void qn_quat_norm(QnQuat* pq, const QnQuat* q)
{
	const float f = 1.0f / qn_quat_len(q);
	pq->x = q->x * f;
	pq->y = q->y * f;
	pq->z = q->z * f;
	pq->w = q->w * f;
}

QN_INLINE void qn_quat_add(QnQuat* pq, const QnQuat* left, const QnQuat* right)
{
	pq->x = left->x + right->x;
	pq->y = left->y + right->y;
	pq->z = left->z + right->z;
	pq->w = left->w + right->w;
}

QN_INLINE void qn_quat_sub(QnQuat* pq, const QnQuat* left, const QnQuat* right)
{
	pq->x = left->x - right->x;
	pq->y = left->y - right->y;
	pq->z = left->z - right->z;
	pq->w = left->w - right->w;
}

QN_INLINE void qn_quat_mag(QnQuat* pq, const QnQuat* left, float right)
{
	pq->x = left->x * right;
	pq->y = left->y * right;
	pq->z = left->z * right;
	pq->w = left->w * right;
}

QN_INLINE void qn_quat_ivt(QnQuat* pq, const QnQuat* v)
{
	pq->x = -v->x;
	pq->y = -v->y;
	pq->z = -v->z;
	pq->w = -v->w;
}

QN_INLINE void qn_quat_neg(QnQuat* pq, const QnQuat* v)
{
	pq->x = 1.0f - v->x;
	pq->y = 1.0f - v->y;
	pq->z = 1.0f - v->z;
	pq->w = 1.0f - v->w;
}

QN_INLINE void qn_quat_cjg(QnQuat* pq, const QnQuat* q)	// conjugate
{
	pq->x = -q->x;
	pq->y = -q->y;
	pq->z = -q->z;
	pq->w = q->w;
}

QN_INLINE float qn_quat_dot(const QnQuat* left, const QnQuat* right)
{
	return left->x * right->x + left->y * right->y + left->z * right->z + left->w * right->w;
}

QN_INLINE void qn_quat_cross(QnQuat* pq, const QnQuat* q1, const QnQuat* q2, const QnQuat* q3)
{
	pq->x = q1->y * (q2->z * q3->w - q3->z * q2->w) - q1->z * (q2->y * q3->w - q3->y * q2->w) + q1->w * (q2->y * q3->z - q2->z * q3->y);
	pq->y = -(q1->x * (q2->z * q3->w - q3->z * q2->w) - q1->z * (q2->x * q3->w - q3->x * q2->w) + q1->w * (q2->x * q3->z - q3->x * q2->z));
	pq->z = q1->x * (q2->y * q3->w - q3->y * q2->w) - q1->y * (q2->x * q3->w - q3->x * q2->w) + q1->w * (q2->x * q3->y - q3->x * q2->y);
	pq->w = -(q1->x * (q2->y * q3->z - q3->y * q2->z) - q1->y * (q2->x * q3->z - q3->x * q2->z) + q1->z * (q2->x * q3->y - q3->x * q2->y));
}

QN_INLINE bool qn_quat_eq(const QnQuat* left, const QnQuat* right)
{
	return
		qn_eqf(left->x, right->x) &&
		qn_eqf(left->y, right->y) &&
		qn_eqf(left->z, right->z) &&
		qn_eqf(left->w, right->w);
}

QN_INLINE bool qn_quat_isi(const QnQuat* pq)
{
	return pq->x == 0.0f && pq->y == 0.0f && pq->z == 0.0f && pq->w == 1.0f;
}

QN_INLINE void qn_quat_mul(QnQuat* pq, const QnQuat* left, const QnQuat* right)
{
	pq->x = left->x * right->w + left->y * right->z - left->z * right->y + left->w * right->x;
	pq->y = -left->x * right->z + left->y * right->w + left->z * right->x + left->w * right->y;
	pq->z = left->x * right->y - left->y * right->x + left->z * right->w + left->w * right->z;
	pq->w = -left->x * right->x - left->y * right->y - left->z * right->z + left->w * right->w;
}

QN_INLINE void qn_quat_x(QnQuat* pq, float rot_x)
{
	pq->y = pq->z = 0.0f;
	qn_sincosf(rot_x * 0.5f, &pq->x, &pq->w);
}

QN_INLINE void qn_quat_y(QnQuat* pq, float rot_y)
{
	pq->x = pq->z = 0.0f;
	qn_sincosf(rot_y * 0.5f, &pq->y, &pq->w);
}

QN_INLINE void qn_quat_z(QnQuat* pq, float rot_z)
{
	pq->x = pq->y = 0.0f;
	qn_sincosf(rot_z * 0.5f, &pq->z, &pq->w);
}

QN_INLINE void qn_quat_axis_vec(QnQuat* pq, const QnVec3* v, float angle)
{
	float s, c;
	qn_sincosf(angle * 0.5f, &s, &c);
	qn_quat_set(pq, v->x * s, v->y * s, v->z * s, c);
}

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

QN_INLINE void qn_quat_interpolate(QnQuat* pq, const QnQuat* left, const QnQuat* right, float scale)
{
	const float f = 1.0f - scale;
	pq->x = right->x * f + left->x * scale;
	pq->y = right->y * f + left->y * scale;
	pq->z = right->z * f + left->z * scale;
	pq->w = right->w * f + left->w * scale;
}

QN_INLINE void qn_quat_lerp(QnQuat* pq, const QnQuat* left, const QnQuat* right, float scale)
{
	pq->x = left->x + scale * (right->x - left->x);
	pq->y = left->y + scale * (right->y - left->y);
	pq->z = left->z + scale * (right->z - left->z);
	pq->w = left->w + scale * (right->w - left->w);
}

// matrix4

QN_INLINE void qn_mat4_zero(QnMat4* pm)
{
	pm->_11 = pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_22 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_33 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = pm->_44 = 0.0f;
}

QN_INLINE void qn_mat4_rst(QnMat4* pm)	// identify
{
	pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = 0.0f;
	pm->_11 = pm->_22 = pm->_33 = pm->_44 = 1.0f;
}

QN_INLINE void qn_mat4_diag(QnMat4* pm, float v)
{
	pm->_12 = pm->_13 = pm->_14 = 0.0f;
	pm->_21 = pm->_23 = pm->_24 = 0.0f;
	pm->_31 = pm->_32 = pm->_34 = 0.0f;
	pm->_41 = pm->_42 = pm->_43 = 0.0f;
	pm->_11 = pm->_22 = pm->_33 = pm->_44 = v;
}

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

QN_INLINE void qn_mat4_move(QnMat4* pm, float x, float y, float z)
{
	pm->_41 += x;
	pm->_42 += y;
	pm->_43 += z;
}

QN_INLINE bool qn_mat4_eq(QnMat4 const* left, QnMat4 const* right)
{
	return memcmp((void const*)left, (void const*)right, sizeof(QnMat4)) == 0;
}

QN_INLINE bool qn_mat4_isi(const QnMat4* pm)
{
	return
		pm->_11 == 1.0f && pm->_12 == 0.0f && pm->_13 == 0.0f && pm->_14 == 0.0f &&
		pm->_21 == 0.0f && pm->_22 == 1.0f && pm->_23 == 0.0f && pm->_24 == 0.0f &&
		pm->_31 == 0.0f && pm->_32 == 0.0f && pm->_33 == 1.0f && pm->_34 == 0.0f &&
		pm->_41 == 0.0f && pm->_42 == 0.0f && pm->_43 == 0.0f && pm->_44 == 1.0f;
}

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

QN_INLINE void qn_mat4_x(QnMat4* pm, float rot)
{
	float vsin, vcos;
	qn_sincosf(rot, &vsin, &vcos);
	pm->_11 = 1.0f; pm->_12 = 0.0f; pm->_13 = 0.0f; pm->_14 = 0.0f;
	pm->_21 = 0.0f; pm->_22 = vcos; pm->_23 = vsin; pm->_24 = 0.0f;
	pm->_31 = 0.0f; pm->_32 = -vsin; pm->_33 = vcos; pm->_34 = 0.0f;
	pm->_41 = 0.0f; pm->_42 = 0.0f; pm->_43 = 0.0f; pm->_44 = 1.0f;
}

QN_INLINE void qn_mat4_y(QnMat4* pm, float rot)
{
	float vsin, vcos;
	qn_sincosf(rot, &vsin, &vcos);
	pm->_11 = vcos; pm->_12 = 0.0f; pm->_13 = -vsin; pm->_14 = 0.0f;
	pm->_21 = 0.0f; pm->_22 = 1.0f; pm->_23 = 0.0f; pm->_24 = 0.0f;
	pm->_31 = vsin; pm->_32 = 0.0f; pm->_33 = vcos; pm->_34 = 0.0f;
	pm->_41 = 0.0f; pm->_42 = 0.0f; pm->_43 = 0.0f; pm->_44 = 1.0f;
}

QN_INLINE void qn_mat4_z(QnMat4* pm, float rot)
{
	float vsin, vcos;
	qn_sincosf(rot, &vsin, &vcos);
	pm->_11 = vcos; pm->_12 = vsin; pm->_13 = 0.0f; pm->_14 = 0.0f;
	pm->_21 = -vsin; pm->_22 = vcos; pm->_23 = 0.0f; pm->_24 = 0.0f;
	pm->_31 = 0.0f; pm->_32 = 0.0f; pm->_33 = 1.0f; pm->_34 = 0.0f;
	pm->_41 = 0.0f; pm->_42 = 0.0f; pm->_43 = 0.0f; pm->_44 = 1.0f;
}

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

QN_INLINE void qn_mat4_perspective_lh(QnMat4* pm, float fov, float aspect, float zn, float zf)
{
	const float f = 1.0f / tanf(fov * 0.5f);
	const float q = zf / (zf - zn);

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

QN_INLINE void qn_mat4_perspective_rh(QnMat4* pm, float fov, float aspect, float zn, float zf)
{
	const float f = 1.0f / tanf(fov * 0.5f);
	const float q = zf / (zn - zf);

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

QN_INLINE void qn_mat4_ortho_offcenter_lh(QnMat4* pm, float l, float r, float b, float t, float zn, float zf)
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

QN_INLINE void qn_mat4_ortho_offcenter_rh(QnMat4* pm, float l, float r, float b, float t, float zn, float zf)
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

QN_INLINE void qn_point_set(QnPoint* pt, int x, int y)
{
	pt->x = x;
	pt->y = y;
}

QN_INLINE bool qn_point_eq(const QnPoint* left, const QnPoint* right)
{
	return left->x == right->x && left->y == right->y;
}

// size

QN_INLINE void qn_size_set(QnSize* pt, int width, int height)
{
	pt->width = width;
	pt->height = height;
}

QN_INLINE void qn_size_set_rect(QnSize* pt, const QnRect* rt)
{
	pt->width = rt->right - rt->left;
	pt->height = rt->bottom - rt->top;
}

QN_INLINE float qn_size_aspect(const QnSize* pt)
{
	return (float)pt->width / (float)pt->height;
}

QN_INLINE bool qn_size_eq(const QnSize* left, const QnSize* right)
{
	return left->width == right->width && left->height == right->height;
}

// rect

QN_INLINE void qn_rect_set(QnRect* prt, int left, int top, int right, int bottom)
{
	prt->left = left;
	prt->top = top;
	prt->right = right;
	prt->bottom = bottom;
}

QN_INLINE void qn_rect_set_size(QnRect* p, int x, int y, int w, int h)
{
	p->left = x;
	p->top = y;
	p->right = x + w;
	p->bottom = y + h;
}

QN_INLINE void qn_rect_zero(QnRect* prt)
{
	prt->left = 0;
	prt->top = 0;
	prt->right = 0;
	prt->bottom = 0;
}

QN_INLINE void qn_rect_mag(QnRect* p, const QnRect* rt, int w, int h)
{
	p->left = rt->left - w;
	p->top = rt->top - h;
	p->right = rt->right + w;
	p->bottom = rt->bottom + h;
}

QN_INLINE void qn_rect_inflate(QnRect* p, const QnRect* rt, int l, int t, int r, int b)
{
	p->left = rt->left - l;
	p->top = rt->top - t;
	p->right = rt->right + r;
	p->bottom = rt->bottom + b;
}

QN_INLINE void qn_rect_deflate(QnRect* p, const QnRect* rt, int l, int t, int r, int b)
{
	p->left = rt->left + l;
	p->top = rt->top + t;
	p->right = rt->right - r;
	p->bottom = rt->bottom - b;
}

QN_INLINE void qn_rect_offset(QnRect* p, const QnRect* rt, int l, int t, int r, int b)
{
	p->left = rt->left + l;
	p->top = rt->top + t;
	p->right = rt->right + r;
	p->bottom = rt->bottom + b;
}

QN_INLINE int qn_rect_width(const QnRect* prt)
{
	return prt->right - prt->left;
}

QN_INLINE int qn_rect_height(const QnRect* prt)
{
	return prt->bottom - prt->top;
}

QN_INLINE bool qn_rect_in(const QnRect* prt, int x, int y)
{
	return (x >= prt->left && x <= prt->right && y >= prt->top && y <= prt->bottom);
}

QN_INLINE bool qn_rect_eq(const QnRect* r1, const QnRect* r2)
{
	return r1->left == r2->left && r1->right == r2->right && r1->top == r2->top && r1->bottom == r2->bottom;
}

QN_INLINE bool qn_rect_intersect(QnRect* p, const QnRect* r1, const QnRect* r2)
{
	const bool b = r2->left < r1->right && r2->right > r1->left && r2->top < r1->bottom && r2->bottom > r1->top;
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

// color

QN_INLINE void qn_color_set(QnColor* pc, float r, float g, float b, float a)
{
	pc->r = r;
	pc->g = g;
	pc->b = b;
	pc->a = a;
}

QN_INLINE void qn_color_set_uint(QnColor* pc, uint value)
{
	const float f = 1.0f / 255.0f;
	pc->b = (float)(value & 255) * f; value >>= 8;
	pc->g = (float)(value & 255) * f; value >>= 8;
	pc->r = (float)(value & 255) * f; value >>= 8;
	pc->a = (float)(value & 255) * f; //value >>= 8;
}

QN_INLINE void qn_color_set_kolor(QnColor* pc, const QnKolor* cu)
{
	const float f = 1.0f / 255.0f;
	pc->a = (float)cu->a * f;
	pc->r = (float)cu->r * f;
	pc->g = (float)cu->g * f;
	pc->b = (float)cu->b * f;
}

QN_INLINE void qn_color_add(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = left->r + right->r;
	pc->g = left->g + right->b;
	pc->b = left->b + right->b;
	pc->a = left->a + right->a;
}

QN_INLINE void qn_color_sub(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = left->r - right->r;
	pc->g = left->g - right->b;
	pc->b = left->b - right->b;
	pc->a = left->a - right->a;
}

QN_INLINE void qn_color_mag(QnColor* pc, const QnColor* left, float scale)
{
	pc->r = left->r * scale;
	pc->g = left->g * scale;
	pc->b = left->b * scale;
	pc->a = left->a * scale;
}

QN_INLINE void qn_color_neg(QnColor* pc, const QnColor* c)
{
	pc->r = 1.0f - c->r;
	pc->g = 1.0f - c->g;
	pc->b = 1.0f - c->b;
	pc->a = c->a;
}

QN_INLINE void qn_color_interpolate(QnColor* pc, const QnColor* left, const QnColor* right, float s)
{
	// left=tocolor, right=fromcolor
	const float f = 1.0f - s;
	pc->r = right->r * f + left->r * s;
	pc->g = right->g * f + left->g * s;
	pc->b = right->b * f + left->b * s;
	pc->a = right->a * f + left->a * s;
}

QN_INLINE void qn_color_lerp(QnColor* pc, const QnColor* left, const QnColor* right, float s)
{
	pc->r = left->r + s * (right->r - left->r);
	pc->g = left->g + s * (right->g - left->g);
	pc->b = left->b + s * (right->b - left->b);
	pc->a = left->a + s * (right->a - left->a);
}

QN_INLINE void qn_color_mod(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = left->r * right->r;
	pc->g = left->g * right->g;
	pc->b = left->b * right->b;
	pc->a = left->a * right->a;
}

QN_INLINE void qn_color_min(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = (left->r < right->r) ? left->r : right->r;
	pc->g = (left->g < right->g) ? left->g : right->g;
	pc->b = (left->b < right->b) ? left->b : right->b;
	pc->a = (left->a < right->a) ? left->a : right->a;
}

QN_INLINE void qn_color_max(QnColor* pc, const QnColor* left, const QnColor* right)
{
	pc->r = (left->r > right->r) ? left->r : right->r;
	pc->g = (left->g > right->g) ? left->g : right->g;
	pc->b = (left->b > right->b) ? left->b : right->b;
	pc->a = (left->a > right->a) ? left->a : right->a;
}

QN_INLINE float* qn_color_float(QnColor* c)
{
	return &c->r;
}

QN_INLINE uint qn_color_uint(const QnColor* pc)
{
	const byte R = (byte)(pc->r * 255.0f + 0.5f);
	const byte G = (byte)(pc->g * 255.0f + 0.5f);
	const byte B = (byte)(pc->b * 255.0f + 0.5f);
	const byte A = (byte)(pc->a * 255.0f + 0.5f);
	return ((uint)A << 24) | ((uint)R << 16) | ((uint)G << 8) | (uint)B;
}

QN_INLINE uint qn_color_uint_exact(const QnColor* pc)
{
	const uint R = (pc->r >= 1.0f) ? 0xff : (pc->r <= 0.0f) ? 0x00 : (uint)(pc->r * 255.0f + 0.5f);
	const uint G = (pc->g >= 1.0f) ? 0xff : (pc->g <= 0.0f) ? 0x00 : (uint)(pc->g * 255.0f + 0.5f);
	const uint B = (pc->b >= 1.0f) ? 0xff : (pc->b <= 0.0f) ? 0x00 : (uint)(pc->b * 255.0f + 0.5f);
	const uint A = (pc->a >= 1.0f) ? 0xff : (pc->a <= 0.0f) ? 0x00 : (uint)(pc->a * 255.0f + 0.5f);
	return (A << 24) | (R << 16) | (G << 8) | B;
}

QN_INLINE bool qn_color_eq(const QnColor* left, const QnColor* right)
{
	return
		qn_eqf(left->r, right->r) &&
		qn_eqf(left->g, right->g) &&
		qn_eqf(left->b, right->b) &&
		qn_eqf(left->a, right->a);
}

QN_INLINE void qn_color_contrast(QnColor* pc, const QnColor* c, float s)
{
	pc->r = 0.5f + s * (c->r - 0.5f);
	pc->g = 0.5f + s * (c->g - 0.5f);
	pc->b = 0.5f + s * (c->b - 0.5f);
	pc->a = c->a;
}

QN_INLINE void qn_color_saturation(QnColor* pc, const QnColor* c, float s)
{
	const float g = c->r * 0.2125f + c->g * 0.7154f + c->b * 0.0721f;
	pc->r = g + s * (c->r - g);
	pc->g = g + s * (c->g - g);
	pc->b = g + s * (c->b - g);
	pc->a = c->a;
}

// byte color

QN_INLINE void qn_kolor_set(QnKolor* pc, byte r, byte g, byte b, byte a)
{
	pc->r = r;
	pc->g = g;
	pc->b = b;
	pc->a = a;
}

QN_INLINE void qn_kolor_set_float(QnKolor* pc, float r, float g, float b, float a)
{
	pc->r = (byte)lrintf(r * 255.0f);
	pc->g = (byte)lrintf(g * 255.0f);
	pc->b = (byte)lrintf(b * 255.0f);
	pc->a = (byte)lrintf(a * 255.0f);
}

QN_INLINE void qn_kolor_set_uint(QnKolor* pc, uint value)
{
	((QnKolorU*)pc)->u = value;
}

QN_INLINE void qn_kolor_set_color(QnKolor* pc, const QnColor* cr)
{
	pc->r = (byte)lrintf(cr->r * 255.0f);
	pc->g = (byte)lrintf(cr->g * 255.0f);
	pc->b = (byte)lrintf(cr->b * 255.0f);
	pc->a = (byte)lrintf(cr->a * 255.0f);
}

QN_INLINE void qn_kolor_add(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	((QnKolorU*)pc)->u = ((const QnKolorU*)left)->u + ((const QnKolorU*)right)->u;
}

QN_INLINE void qn_kolor_sub(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	((QnKolorU*)pc)->u = ((const QnKolorU*)left)->u - ((const QnKolorU*)right)->u;
}

QN_INLINE void qn_kolor_mag(QnKolor* pc, const QnKolor* left, float scale)
{
	pc->r = (byte)lrintf((float)left->r * scale);
	pc->g = (byte)lrintf((float)left->g * scale);
	pc->b = (byte)lrintf((float)left->b * scale);
	pc->a = (byte)lrintf((float)left->a * scale);
}

QN_INLINE void qn_kolor_neg(QnKolor* pc, const QnKolor* c)
{
	pc->r = 255 - c->r;
	pc->g = 255 - c->g;
	pc->b = 255 - c->b;
	pc->a = c->a;
}

QN_INLINE void qn_kolor_interpolate(QnKolor* pc, const QnKolor* left, const QnKolor* right, float s)
{
	// left=tocolor, right=fromcolor
	const float f = 1.0f - s;
	pc->r = (byte)lrintf((float)right->r * f + (float)left->r * s);
	pc->g = (byte)lrintf((float)right->g * f + (float)left->g * s);
	pc->b = (byte)lrintf((float)right->b * f + (float)left->b * s);
	pc->a = (byte)lrintf((float)right->a * f + (float)left->a * s);
}

QN_INLINE void qn_kolor_lerp(QnKolor* pc, const QnKolor* left, const QnKolor* right, float s)
{
	pc->r = (byte)lrintf((float)left->r + s * (float)(right->r - left->r));
	pc->g = (byte)lrintf((float)left->g + s * (float)(right->g - left->g));
	pc->b = (byte)lrintf((float)left->b + s * (float)(right->b - left->b));
	pc->a = (byte)lrintf((float)left->a + s * (float)(right->a - left->a));
}

QN_INLINE void qn_kolor_mod(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	pc->r = left->r * right->r;
	pc->g = left->g * right->g;
	pc->b = left->b * right->b;
	pc->a = left->a * right->a;
}

QN_INLINE void qn_kolor_min(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	pc->r = (left->r < right->r) ? left->r : right->r;
	pc->g = (left->g < right->g) ? left->g : right->g;
	pc->b = (left->b < right->b) ? left->b : right->b;
	pc->a = (left->a < right->a) ? left->a : right->a;
}

QN_INLINE void qn_kolor_max(QnKolor* pc, const QnKolor* left, const QnKolor* right)
{
	pc->r = (left->r > right->r) ? left->r : right->r;
	pc->g = (left->g > right->g) ? left->g : right->g;
	pc->b = (left->b > right->b) ? left->b : right->b;
	pc->a = (left->a > right->a) ? left->a : right->a;
}

QN_INLINE bool qn_kolor_eq(const QnKolor* left, const QnKolor* right)
{
	return ((const QnKolorU*)left)->u == ((const QnKolorU*)right)->u;
}

QN_INLINE void qn_kolor_contrast(QnKolor* pc, const QnKolor* c, float s)
{
	QnColor cr;
	qn_color_set_kolor(&cr, pc);
	qn_color_contrast(&cr, &cr, s);
	qn_kolor_set_color(pc, &cr);
}

QN_INLINE void qn_kolor_saturation(QnKolor* pc, const QnKolor* c, float s)
{
	QnColor cr;
	qn_color_set_kolor(&cr, pc);
	qn_color_saturation(&cr, &cr, s);
	qn_kolor_set_color(pc, &cr);
}

// plane

QN_INLINE void qn_plane_rst(QnPlane* pp)
{
	pp->a = pp->b = pp->c = 0.0f;
	pp->d = 1.0f;
}

QN_INLINE void qn_plane_set(QnPlane* pp, float a, float b, float c, float d)
{
	pp->a = a;
	pp->b = b;
	pp->c = c;
	pp->d = d;
}

QN_INLINE float qn_plane_dot_coord(const QnPlane* pp, const QnVec3* v)
{
	return pp->a * v->x + pp->b * v->y + pp->c * v->z + pp->d;
}

QN_INLINE void qn_plane_from_point_norm(QnPlane* pp, const QnVec3* pv, const QnVec3* pn)
{
	pp->a = pn->x;
	pp->b = pn->y;
	pp->c = pn->z;
	pp->d = -qn_vec3_dot(pv, pn);
}

QN_INLINE int qn_plane_face_point(const QnPlane* p, const QnVec3* v)
{
	const float f = qn_vec3_dot((const QnVec3*)p, v) + p->d;
	if (f < -QN_EPSILON)
		return -1;  // back
	if (f > QN_EPSILON)
		return 1;   // front
	return 0;       // on
}

QN_INLINE float qn_plane_dist(const QnPlane* p, const QnVec3* v)
{
	return qn_vec3_dot(v, (const QnVec3*)p) + p->d;
}

QN_INLINE void qn_plane_norm(QnPlane* pp, const QnPlane* p)
{
	const float f = 1.0f / sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
	pp->a = p->a * f;
	pp->b = p->b * f;
	pp->c = p->c * f;
	pp->d = p->d * f;
}

QN_INLINE void qn_plane_rnorm(QnPlane* pp, const QnPlane* p)
{
	const float n = -1.0f / sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
	pp->a = p->a * n;
	pp->b = p->b * n;
	pp->c = p->c * n;
	pp->d = p->d * n;
}

QN_INLINE float qn_plane_intersect_line(const QnPlane* p, const QnVec3* v1, const QnVec3* v2)
{
	QnVec3 t;
	qn_vec3_sub(&t, v2, v1);
	const float f = 1.0f / qn_vec3_dot((const QnVec3*)p, &t);
	return -(qn_vec3_dot((const QnVec3*)p, v1) + p->d) * f;
}

QN_INLINE bool qn_plane_intersect_plane(const QnPlane* p, const QnPlane* o1, const QnPlane* o2)
{
	QnVec3 v, dir, loc;
	return (qn_plane_intersect(p, &loc, &dir, o1)) ? qn_vec3_intersect_line(&v, o2, &loc, &dir) : false;
}

// line3

QN_INLINE void qn_line3_set(QnLine3* p, float bx, float by, float bz, float ex, float ey, float ez)
{
	qn_vec3_set(&p->begin, bx, by, bz);
	qn_vec3_set(&p->end, ex, ey, ez);
}

QN_INLINE void qn_line3_set_vec(QnLine3* p, const QnVec3* begin, const QnVec3* end)
{
	p->begin = *begin;
	p->end = *end;
}

QN_INLINE void qn_line3_add(QnLine3* p, const QnLine3* l, const QnVec3* v)
{
	qn_vec3_add(&p->begin, &l->begin, v);
	qn_vec3_add(&p->end, &l->end, v);
}

QN_INLINE void qn_line3_sub(QnLine3* p, const QnLine3* l, const QnVec3* v)
{
	qn_vec3_sub(&p->begin, &l->begin, v);
	qn_vec3_sub(&p->end, &l->end, v);
}

QN_INLINE float qn_line3_len_sq(const QnLine3* p)
{
	return qn_vec3_dist_sq(&p->begin, &p->end);
}

QN_INLINE float qn_line3_len(const QnLine3* p)
{
	return qn_vec3_dist(&p->begin, &p->end);
}

QN_INLINE void qn_line3_center(const QnLine3* p, QnVec3* v)
{
	qn_vec3_add(v, &p->begin, &p->end);
	qn_vec3_mag(v, v, 0.5f);
}

QN_INLINE void qn_line3_vec(const QnLine3* p, QnVec3* v)
{
	qn_vec3_sub(v, &p->end, &p->begin);
}

QN_INLINE bool qn_line3_in(const QnLine3* p, const QnVec3* v)
{
	return qn_vec3_between(v, &p->begin, &p->end);
}

// half

// half type
QN_INLINE halfint qn_f2hf(const float v)
{
	QN_MEM_BARRIER();
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

QN_INLINE float qn_hf2f(const halfint v)
{
	QN_MEM_BARRIER();
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

// vec half
QN_INLINE void qn_vec2h_set(QnVecH2* p, float x, float y)
{
	p->x = qn_f2hf(x);
	p->y = qn_f2hf(y);
}

QN_INLINE void qn_vec3h_set(QnVecH3* p, float x, float y, float z)
{
	p->x = qn_f2hf(x);
	p->y = qn_f2hf(y);
	p->z = qn_f2hf(z);
}

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
