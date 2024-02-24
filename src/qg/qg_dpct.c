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

void _dpct_init(QgDpct* self, const char* name, const QmMat* defm)
{
	if (name != NULL)
	{
		qn_strncpy(self->name, name, QN_COUNTOF(self->name) - 1);
		self->hash = qn_strihash(self->name);
	}

	self->trfm.mdef = defm != NULL ? *defm : qm_mat4_unit();
	self->trfm.mcalc = qm_mat4_unit();
	self->trfm.mlocal = qm_mat4_unit();
	self->trfm.vloc = qm_vec_zero();
	self->trfm.qrot = qm_quat_unit();
	self->trfm.vscl = qm_vec_one();
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
void _dpct_set_loc(QnGam g, const QmVec* loc)
{
	QgDpct* self = qn_cast_type(g, QgDpct);
	self->trfm.vloc = *loc;
}

// 회전
void _dpct_set_rot(QnGam g, const QmVec* rot)
{
	QgDpct* self = qn_cast_type(g, QgDpct);
	self->trfm.qrot = *rot;
}

// 크기
void _dpct_set_scl(QnGam g, const QmVec* scl)
{
	QgDpct* self = qn_cast_type(g, QgDpct);
	self->trfm.vscl = *scl;
}

//
bool qg_dpct_update(QgDpct* self, float advance)
{
	return qn_cast_vtable(self, QGDPCT)->update(self, advance);
}

//
void qg_dpct_draw(QgDpct* self)
{
	qn_cast_vtable(self, QGDPCT)->draw(self);
}

//
void qg_dpct_set_loc(QgDpct* self, const QmVec* loc)
{
	qn_cast_vtable(self, QGDPCT)->set_loc(self, loc);
}

//
void qg_dpct_set_rot(QgDpct* self, const QmVec* rot)
{
	qn_cast_vtable(self, QGDPCT)->set_rot(self, rot);
}

//
void qg_dpct_set_scl(QgDpct* self, const QmVec* scl)
{
	qn_cast_vtable(self, QGDPCT)->set_scl(self, scl);
}

//
void qg_dpct_set_name(QgDpct* self, const char* name)
{
	qn_strncpy(self->name, name, QN_COUNTOF(self->name) - 1);
}

//
void qg_dpct_update_tm(QgDpct* self)
{
	self->trfm.mlocal = qm_mat4_trfm(self->trfm.vloc, self->trfm.qrot, &self->trfm.vscl);
	if (self->parent != NULL)
		self->trfm.mcalc = qm_mat4_mul(self->trfm.mlocal, self->parent->trfm.mcalc);
	else
		self->trfm.mcalc = self->trfm.mlocal;
}


//////////////////////////////////////////////////////////////////////////
// 레이 캐스트

void qg_ray_set_point(QgRay* self, float x, float y)
{
	qn_return_when_fail(self->camera != NULL, );
	const QmSize size = RDH_TRANSFORM->size;
	const QmMat4* proj = &self->camera->param.proj;
	const QmMat4* invv = &self->camera->param.invv;

	QmVec v = qm_vec(
		((((x - size.Width) * 2.0f / size.Width) - 1.0f) - proj->_31) / proj->_11,
		-((((y - size.Height) * 2.0f / size.Height) - 1.0f) - proj->_32) / proj->_22,
		1.0f, 0.0f);

	self->location = qm_vec(x, y, 0.0f, 0.0f);
	self->direction = qm_vec(
		v.X * invv->_11 + v.Y * invv->_21 + v.Z * invv->_31,
		v.X * invv->_12 + v.Y * invv->_22 + v.Z * invv->_32,
		v.X * invv->_13 + v.Y * invv->_23 + v.Z * invv->_33,
		0.0f);
	self->origin = qm_vec(invv->_41, invv->_42, invv->_43, 0.0f);
}

void qg_ray_set_bound_point(QgRay* self, const QmRect* bound, float x, float y)
{
	qn_return_when_fail(self->camera != NULL, );
	const QmMat4* proj = &self->camera->param.proj;
	const QmMat4* invv = &self->camera->param.invv;
	const int width = qm_rect_get_width(*bound);
	const int height = qm_rect_get_height(*bound);

	QmVec v = qm_vec(
		((((x - bound->Left) * 2.0f / width) - 1.0f) - proj->_31) / proj->_11,
		-((((y - bound->Top) * 2.0f / height) - 1.0f) - proj->_32) / proj->_22,
		1.0f, 0.0f);

	self->location = qm_vec(x, y, 0.0f, 0.0f);
	self->direction = qm_vec(
		v.X * invv->_11 + v.Y * invv->_21 + v.Z * invv->_31,
		v.X * invv->_12 + v.Y * invv->_22 + v.Z * invv->_32,
		v.X * invv->_13 + v.Y * invv->_23 + v.Z * invv->_33,
		0.0f);
	self->origin = qm_vec(invv->_41, invv->_42, invv->_43, 0.0f);
}

const QmVec QM_VECTORCALL qg_ray_get_loc(const QgRay* self, float dist)
{
	return qm_vec_add(qm_vec_mag(self->direction, dist), self->origin);
}

bool qg_ray_intersect_tri(const QgRay* self, const QmVec* v1, const QmVec* v2, const QmVec* v3, float* distance)
{
	const QmVec e1 = qm_vec_sub(*v2, *v1);
	const QmVec e2 = qm_vec_sub(*v3, *v1);

	const QmVec h = qm_vec3_cross(self->direction, e2);
	const float a = qm_vec3_dot(e1, h);
	if (a < QM_EPSILON)
		return false;

	const float f = 1.0f / a;
	const QmVec s = qm_vec_sub(self->origin, *v1);
	const float u = f * qm_vec3_dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return false;

	const QmVec q = qm_vec3_cross(s, e1);
	const float v = f * qm_vec3_dot(self->direction, q);
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

bool qg_ray_intersect_plane(const QgRay* self, const QmVec* plane, const QmVec* normal, float* distance)
{
	const float d = qm_vec3_dot(*normal, *plane);
	const float t = (d - qm_vec3_dot(*normal, self->origin)) / qm_vec3_dot(*normal, self->direction);
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
bool qg_camera_set_view(QgCamera* self, const QmVec* eye, const QmVec* at, const QmVec* ahead)
{
	qn_return_when_fail(self->param.use_maya, false);
	if (eye)
		self->view.eye = *eye;
	if (at)
		self->view.at = *at;
	if (ahead)
		self->view.ahead = *ahead;
	return true;
}

//
void qg_camera_set_rot(QgCamera* self, const QmVec* rot)
{
	self->param.rot = *rot;
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
float qg_camera_get_distsq(const QgCamera* self, const QmVec* pos)
{
	const QmVec v = qm_vec_sub(*pos, self->view.eye);
	return qm_vec3_dot(v, v);
}

//
float qg_camera_get_dist(const QgCamera* self, const QmVec* pos)
{
	return qm_sqrtf(qg_camera_get_distsq(self, pos));
}

//
QmVec QM_VECTORCALL qg_camera_project(const QgCamera* self, const QmVec* v, const QmMat4* world)
{
	const QmSize size = RDH_TRANSFORM->size;
	QmMat4 m = qm_mat4_mul(world != NULL ? *world : RDH_TRANSFORM->world, self->param.vipr);
	QmVec4 t = qm_vec3_trfm(*v, m);
	return qm_vec(
		(t.X + 1.0f) * 0.5f * size.Width,
		(1.0f - t.Y) * 0.5f * size.Height,
		RDH_TRANSFORM->Near + t.Z * (RDH_TRANSFORM->Far - RDH_TRANSFORM->Near), 0.0f);
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
		self->param.proj = qm_mat4_perspective_lh(self->proj.fov, self->proj.aspect, self->proj.znear, self->proj.zfar);
	}

	if (self->param.use_maya)
	{
		QmMat r = qm_mat4_rot_quat(self->param.rot);
		self->view.up = qm_vec3_trfm(qm_vec(0.0f, 1.0f, 0.0f, 0.0f), r);
		self->view.ahead = qm_vec3_trfm(qm_vec(0.0f, 0.0f, 1.0f, 0.0f), r);

		QmVec ahead = qm_vec_mag(self->view.ahead, self->param.dist);
		self->view.eye = qm_vec_sub(self->view.at, ahead);
		QmVec at = qm_vec_add(self->view.eye, self->view.ahead);
		self->param.view = qm_mat4_lookat_lh(self->view.eye, at, self->view.up);
	}
	else
	{
		self->param.view = qm_mat4_lookat_lh(self->view.eye, self->view.at, self->view.up);
	}

	self->param.invv = qm_mat4_inv(self->param.view);
	self->param.vipr = qm_mat4_mul(self->param.proj, self->param.view);

	qg_set_camera(self);
}

//
static void _camera_init(QgCamera* self)
{
	self->proj.aspect = qg_get_aspect();
	self->proj.fov = QM_PI_Q;
	self->proj.znear = 1.0f;
	self->proj.zfar = 10000.0f;

	self->view.eye = qm_vec(0.0f, 0.0f, 0.0f, 0.0f);
	self->view.at = qm_vec(0.0f, 0.0f, 1.0f, 0.0f);
	self->view.up = qm_vec(0.0f, 1.0f, 0.0f, 0.0f);
	self->view.ahead = qm_vec(0.0f, 0.0f, 1.0f, 0.0f);

	self->param.proj = qm_mat4_perspective_lh(self->proj.fov, self->proj.aspect, self->proj.znear, self->proj.zfar);
	self->param.view = qm_mat4_lookat_lh(self->view.eye, self->view.at, self->view.up);
	self->param.invv = qm_mat4_inv(self->param.view);
	self->param.vipr = qm_mat4_mul(self->param.proj, self->param.view);
	self->param.rot = qm_quat_unit();
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
