//
// qg_dpct.c - 노드 & 카메라
// 2024-2-20 by kim
//

#include "pch.h"
#include "qg_dpct.h"
#include "qg_stub.h"
#include "qs_supp.h"

//////////////////////////////////////////////////////////////////////////
// 데픽트

void _dpct_init(QgDpct* self, const char* name, const QMMAT* defm)
{
	if (name != NULL)
	{
		qn_strncpy(self->name, name, QN_COUNTOF(self->name) - 1);
		self->hash = qn_strihash(self->name);
	}

	self->trfm.mdef.s = defm != NULL ? *defm : qm_mat4_unit();
	self->trfm.mcalc.s = qm_mat4_unit();
	self->trfm.mlocal.s = qm_mat4_unit();
	self->trfm.vloc.s = qm_vec_zero();
	self->trfm.qrot.s = qm_quat_unit();
	self->trfm.vscl.s = qm_vec_one();
}

// 업데이트
bool _dpct_update(QnGam g, float advance)
{
	QgDpct* self = qn_cast_type(g, QgDpct);
	QN_DUMMY(advance);
	qg_dpct_update_tm(self);
	return true;
}

// 위치
void _dpct_set_loc(QnGam g, const QMVEC* loc)
{
	QgDpct* self = qn_cast_type(g, QgDpct);
	self->trfm.vloc.s = *loc;
}

// 회전
void _dpct_set_rot(QnGam g, const QMVEC* rot)
{
	QgDpct* self = qn_cast_type(g, QgDpct);
	self->trfm.qrot.s = *rot;
}

// 크기
void _dpct_set_scl(QnGam g, const QMVEC* scl)
{
	QgDpct* self = qn_cast_type(g, QgDpct);
	self->trfm.vscl.s = *scl;
}

//
bool qg_dpct_update(QNGAM dpct, float advance)
{
	return qn_cast_vtable(dpct, QGDPCT)->update(dpct, advance);
}

//
void qg_dpct_draw(QNGAM dpct)
{
	qn_cast_vtable(dpct, QGDPCT)->draw(dpct);
}

//
void qg_dpct_set_loc(QNGAM dpct, const QMVEC* loc)
{
	qn_cast_vtable(dpct, QGDPCT)->set_loc(dpct, loc);
}

//
void qg_dpct_set_rot(QNGAM dpct, const QMVEC* rot)
{
	qn_cast_vtable(dpct, QGDPCT)->set_rot(dpct, rot);
}

//
void qg_dpct_set_scl(QNGAM dpct, const QMVEC* scl)
{
	qn_cast_vtable(dpct, QGDPCT)->set_scl(dpct, scl);
}

//
void qg_dpct_set_loc_param(QNGAM dpct, float x, float y, float z)
{
	QMVEC v = qm_vec(x, y, z, 1.0f);
	qg_dpct_set_loc(dpct, &v);
}

//
void qg_dpct_set_rot_param(QNGAM dpct, float x, float y, float z, float w)
{
	QMVEC v = qm_vec(x, y, z, w);
	qg_dpct_set_rot(dpct, &v);
}

//
void qg_dpct_set_scl_param(QNGAM dpct, float x, float y, float z)
{
	QMVEC v = qm_vec(x, y, z, 1.0f);
	qg_dpct_set_scl(dpct, &v);
}

//
void qg_dpct_set_name(QNGAM dpct, const char* name)
{
	QgDpct* self = qn_cast_type(dpct, QgDpct);
	qn_strncpy(self->name, name, QN_COUNTOF(self->name) - 1);
}

//
void qg_dpct_update_tm(QNGAM dpct)
{
	QgDpct* self = qn_cast_type(dpct, QgDpct);
	self->trfm.mlocal.s = qm_mat4_trfm(self->trfm.vloc.s, self->trfm.qrot.s, self->trfm.vscl.s);
	if (self->parent != NULL)
		self->trfm.mcalc.s = qm_mat4_mul(self->trfm.mlocal.s, self->parent->trfm.mcalc.s);
	else
		self->trfm.mcalc.s = self->trfm.mlocal.s;
}


//////////////////////////////////////////////////////////////////////////
// 레이 캐스트

void qg_ray_set_point(QgRay* self, float x, float y)
{
	qn_return_when_fail(self->camera != NULL, );
	const QmSize size = RDH_TRANSFORM->size;
	const QmMat4* proj = &self->camera->param.proj;
	const QmMat4* invv = &self->camera->param.invv;

	QMVEC v = qm_vec(
		((((x - size.Width) * 2.0f / size.Width) - 1.0f) - proj->_31) / proj->_11,
		-((((y - size.Height) * 2.0f / size.Height) - 1.0f) - proj->_32) / proj->_22,
		1.0f, 0.0f);

	self->location.s = qm_vec(x, y, 0.0f, 0.0f);
	self->direction.s = qm_vec3_trfm_norm(v, invv->s);
	self->origin.s = qm_vec(invv->_41, invv->_42, invv->_43, 0.0f);
}

void qg_ray_set_bound_point(QgRay* self, const QmRect* bound, float x, float y)
{
	qn_return_when_fail(self->camera != NULL, );
	const QmMat4* proj = &self->camera->param.proj;
	const QmMat4* invv = &self->camera->param.invv;
	const int width = qm_rect_get_width(*bound);
	const int height = qm_rect_get_height(*bound);

	QMVEC v = qm_vec(
		((((x - bound->Left) * 2.0f / width) - 1.0f) - proj->_31) / proj->_11,
		-((((y - bound->Top) * 2.0f / height) - 1.0f) - proj->_32) / proj->_22,
		1.0f, 0.0f);

	self->location.s = qm_vec(x, y, 0.0f, 0.0f);
	self->direction.s = qm_vec3_trfm_norm(v, invv->s);
	self->origin.s = qm_vec(invv->_41, invv->_42, invv->_43, 0.0f);
}

QMVEC qg_ray_get_loc(const QgRay* self, float dist)
{
	return qm_vec_add(qm_vec_mag(self->direction.s, dist), self->origin.s);
}

bool qg_ray_intersect_tri(const QgRay* self, const QMVEC* v1, const QMVEC* v2, const QMVEC* v3, float* distance)
{
	const QMVEC e1 = qm_vec_sub(*v2, *v1);
	const QMVEC e2 = qm_vec_sub(*v3, *v1);

	const QMVEC h = qm_vec3_cross(self->direction.s, e2);
	const float a = qm_vec3_dot(e1, h);
	if (a < QM_EPSILON)
		return false;

	const float f = 1.0f / a;
	const QMVEC s = qm_vec_sub(self->origin.s, *v1);
	const float u = f * qm_vec3_dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return false;

	const QMVEC q = qm_vec3_cross(s, e1);
	const float v = f * qm_vec3_dot(self->direction.s, q);
	if (v < 0.0f || u + v > 1.0f)
		return false;

	const float t = f * qm_vec3_dot(e2, q);
	if (t > QM_EPSILON)
	{
		*distance = t;
		return true;
	}

	return false;
}

bool qg_ray_intersect_plane(const QgRay* self, const QMVEC* plane, const QMVEC* normal, float* distance)
{
	const float d = qm_vec3_dot(*normal, *plane);
	const float t = (d - qm_vec3_dot(*normal, self->origin.s)) / qm_vec3_dot(*normal, self->direction.s);
	if (t > 0.0f)
	{
		*distance = t;
		return true;
	}

	return false;
}

static void qg_ray_dispose(QnGam g)
{
	QgRay* self = qn_cast_type(g, QgRay);
	qn_unload(self->camera);
	qn_free(self);
}

QgRay* qg_create_ray(QgCamera* camera)
{
	QgRay* self = qn_alloc_zero_1(QgRay);
	self->camera = qn_loadc(camera, QgCamera);
	static const QN_DECL_VTABLE(QNGAMBASE) vt_qg_ray =
	{
		"Ray",
		qg_ray_dispose,
	};
	return qn_gam_init(self, vt_qg_ray);
}


//////////////////////////////////////////////////////////////////////////
// 카메라

//
void qg_camera_set_proj(QgCamera* self, float fov, float znear, float zfar)
{
	self->proj.aspect = qg_get_aspect();
	self->proj.fov = fov;
	self->proj.znear = znear;
	self->proj.zfar = zfar;
}

//
void qg_camera_set_proj_aspect(QgCamera* self, float ascpect, float fov, float znear, float zfar)
{
	self->proj.aspect = ascpect;
	self->proj.fov = fov;
	self->proj.znear = znear;
	self->proj.zfar = zfar;
}

//
bool qg_camera_set_view(QgCamera* self, const QMVEC* eye, const QMVEC* at, const QMVEC* ahead)
{
	qn_return_when_fail(self->param.use_maya, false);
	if (eye)
		self->view.eye.s = *eye;
	if (at)
		self->view.at.s = *at;
	if (ahead)
		self->view.ahead.s = *ahead;
	return true;
}

//
void qg_camera_set_rot(QgCamera* self, const QMVEC* rot)
{
	self->param.rot.s = *rot;
}

//
void qg_camera_set_rot_speed(QgCamera* self, float spd)
{
	self->param.spd_rot = spd;
}

//
void qg_camera_set_move_speed(QgCamera* self, float spd)
{
	self->param.spd_move = spd;
}

//
float qg_camera_get_distsq(const QgCamera* self, const QMVEC* pos)
{
	const QMVEC v = qm_vec_sub(*pos, self->view.eye.s);
	return qm_vec3_dot(v, v);
}

//
float qg_camera_get_dist(const QgCamera* self, const QMVEC* pos)
{
	return qm_sqrtf(qg_camera_get_distsq(self, pos));
}

//
QMVEC qg_camera_project(const QgCamera* self, const QMVEC v, const QMMAT* world)
{
	const QmSize size = RDH_TRANSFORM->size;
	QMMAT m = qm_mat4_mul(world != NULL ? *world : RDH_TRANSFORM->world.s, self->param.vipr.s);
	QMVEC t = qm_vec3_trfm(v, m);
	QmFloat3A a;
	qm_vec_st_float3a(&a, t);
	return qm_vec(
		(a.X + 1.0f) * 0.5f * size.Width,
		(1.0f - a.Y) * 0.5f * size.Height,
		RDH_TRANSFORM->Near + a.Z * (RDH_TRANSFORM->Far - RDH_TRANSFORM->Near), 0.0f);
}

//
void qg_camera_update(QgCamera* self)
{
	qn_cast_vtable(self, QGCAMERA)->update(self);
}

//
static void _camera_internal_update(QnGam g)
{
	QgCamera* self = qn_cast_type(g, QgCamera);

	if (self->param.use_layout)
	{
		self->proj.aspect = qg_get_aspect();
		self->param.proj.s = qm_mat4_perspective_lh(self->proj.fov, self->proj.aspect, self->proj.znear, self->proj.zfar);
	}

	if (self->param.use_maya)
	{
		QMMAT r = qm_mat4_rot_quat(self->param.rot.s);
		self->view.up.s = qm_vec3_trfm(qm_vec(0.0f, 1.0f, 0.0f, 0.0f), r);
		self->view.ahead.s = qm_vec3_trfm(qm_vec(0.0f, 0.0f, 1.0f, 0.0f), r);

		QMVEC ahead = qm_vec_mag(self->view.ahead.s, self->param.dist);
		self->view.eye.s = qm_vec_sub(self->view.at.s, ahead);
		QMVEC at = qm_vec_add(self->view.eye.s, self->view.ahead.s);
		self->param.view.s = qm_mat4_lookat_lh(self->view.eye.s, at, self->view.up.s);
	}
	else
	{
		self->param.view.s = qm_mat4_lookat_lh(self->view.eye.s, self->view.at.s, self->view.up.s);
	}

	self->param.invv.s = qm_mat4_inv(self->param.view.s);
	self->param.vipr.s = qm_mat4_mul(self->param.proj.s, self->param.view.s);

	qg_set_camera(self);
}

//
static void _camera_init(QgCamera* self)
{
	self->proj.aspect = qg_get_aspect();
	self->proj.fov = QM_PI_Q;
	self->proj.znear = 1.0f;
	self->proj.zfar = 10000.0f;

	self->view.eye.s = qm_vec(0.0f, 0.0f, 0.0f, 0.0f);
	self->view.at.s = qm_vec(0.0f, 0.0f, 1.0f, 0.0f);
	self->view.up.s = qm_vec(0.0f, 1.0f, 0.0f, 0.0f);
	self->view.ahead.s = qm_vec(0.0f, 0.0f, 1.0f, 0.0f);

	self->param.proj.s = qm_mat4_perspective_lh(self->proj.fov, self->proj.aspect, self->proj.znear, self->proj.zfar);
	self->param.view.s = qm_mat4_lookat_lh(self->view.eye.s, self->view.at.s, self->view.up.s);
	self->param.invv.s = qm_mat4_inv(self->param.view.s);
	self->param.vipr.s = qm_mat4_mul(self->param.proj.s, self->param.view.s);
	self->param.rot.s = qm_quat_unit();
	self->param.dist = 10.0f;
	self->param.spd_move = 1.0f;
	self->param.spd_rot = 1.0f;

	self->param.use_layout = true;
	self->param.use_pause = true;
}

//
static void qg_camera_dispose(QnGam g)
{
	QgCamera* self = qn_cast_type(g, QgCamera);
	qn_free(self);
}

//
QgCamera* qg_create_camera(void)
{
	QgCamera* self = qn_alloc_zero_1(QgCamera);
	_camera_init(self);
	static const QN_DECL_VTABLE(QGCAMERA) vt_qg_camera =
	{
		{
			"Camera",
			qg_camera_dispose,
		},
		_camera_internal_update,
	};
	return qn_gam_init(self, vt_qg_camera);
}

//
QgCamera* qg_create_maya_camera(void)
{
	QgCamera* self = qn_alloc_zero_1(QgCamera);
	_camera_init(self);
	self->param.use_maya = true;
	static const QN_DECL_VTABLE(QGCAMERA) vt_qg_camera =
	{
		{
			"MayaCamera",
			qg_camera_dispose,
		},
		_camera_internal_update,
	};
	return qn_gam_init(self, vt_qg_camera);
}
