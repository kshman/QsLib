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
void qg_dcpt_set_local(QNGAM dcpt, const QMMAT* m)
{
	QgDpct* self = qn_cast_type(dcpt, QgDpct);
	self->trfm.mlocal.s = m != NULL ? *m : qm_mat4_unit();
}

//
void qg_dcpt_set_calc(QNGAM dcpt, const QMMAT* m)
{
	QgDpct* self = qn_cast_type(dcpt, QgDpct);
	self->trfm.mcalc.s = m != NULL ? *m : qm_mat4_unit();
}

//
void qg_dpct_set_loc_param(QNGAM dpct, float x, float y, float z)
{
	QMVEC v = qm_vec(x, y, z, 0.0f);
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

//
void qg_ray_set_point(QgRay* self, float x, float y)
{
	qn_return_when_fail(self->camera != NULL, );
	const QmSize size = RDH_TRANSFORM->size;
	const QmMat4* proj = (const QmMat4*)&self->camera->mat.proj;
	const QmMat4* invv = (const QmMat4*)&self->camera->mat.invv;

	QMVEC v = qm_vec(
		((((x - size.Width) * 2.0f / size.Width) - 1.0f) - proj->_31) / proj->_11,
		-((((y - size.Height) * 2.0f / size.Height) - 1.0f) - proj->_32) / proj->_22,
		1.0f, 0.0f);

	self->location.s = qm_vec(x, y, 0.0f, 0.0f);
	self->direction.s = qm_vec3_trfm_norm(v, invv->s);
	self->origin.s = qm_vec_set_w(invv->r[3], 0.0f);
}

//
void qg_ray_set_bound_point(QgRay* self, const QmRect* bound, float x, float y)
{
	qn_return_when_fail(self->camera != NULL, );
	const QmMat4* proj = (const QmMat4*)&self->camera->mat.proj;
	const QmMat4* invv = (const QmMat4*)&self->camera->mat.invv;
	const int width = qm_rect_get_width(*bound);
	const int height = qm_rect_get_height(*bound);

	QMVEC v = qm_vec(
		((((x - bound->Left) * 2.0f / width) - 1.0f) - proj->_31) / proj->_11,
		-((((y - bound->Top) * 2.0f / height) - 1.0f) - proj->_32) / proj->_22,
		1.0f, 0.0f);

	self->location.s = qm_vec(x, y, 0.0f, 0.0f);
	self->direction.s = qm_vec3_trfm_norm(v, invv->s);
	self->origin.s = qm_vec_set_w(invv->r[3], 0.0f);
}

//
QMVEC qg_ray_get_loc(const QgRay* self, float dist)
{
	return qm_vec_add(qm_vec_mag(self->direction.s, dist), self->origin.s);
}

//
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

//
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

//
static void qg_ray_dispose(QnGam g)
{
	QgRay* self = qn_cast_type(g, QgRay);
	qn_unload(self->camera);
	qn_free(self);
}

//
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
void qg_camera_set_proj_param(QgCamera* self, float ascpect, float fov, float znear, float zfar)
{
	self->param.aspect = ascpect;
	self->param.fov = fov;
	self->param.znear = znear;
	self->param.zfar = zfar;
}

//
void qg_camera_set_position(QgCamera* self, const QMVEC* pos)
{
	if (QN_TMASK(self->flags, QGCAMF_MAYA))
		self->param.at.s = *pos;
	else
		self->param.eye.s = *pos;
}

//
void qg_camera_set_angle(QgCamera* self, const QMVEC* angle)
{
	self->param.angle.s = *angle;
}

//
void qg_camera_set_move_speed(QgCamera* self, const QMVEC* s)
{
	self->param.smove.s = *s;
}

//
void qg_camera_set_rot_speed(QgCamera* self, const QMVEC* s)
{
	self->param.srot.s = *s;
}

//
void qg_camera_set_position_param(QgCamera* self, float x, float y, float z)
{
	if (QN_TMASK(self->flags, QGCAMF_MAYA))
		self->param.at.s = qm_vec3(x, y, z);
	else
		self->param.eye.s = qm_vec3(x, y, z);
}

//
void qg_camera_set_angle_param(QgCamera* self, float x, float y, float z)
{
	self->param.angle.s = qm_vec3(x, y, z);
}

//
void qg_camera_set_move_speed_param(QgCamera* self, float sx, float sy, float sz)
{
	self->param.smove.s = qm_vec3(sx, sy, sz);
}

//
void qg_camera_set_rot_speed_param(QgCamera* self, float sx, float sy, float sz)
{
	self->param.srot.s = qm_vec3(sx, sy, sz);
}

//
float qg_camera_get_dist(const QgCamera* self, const QMVEC* pos)
{
	return qm_vec3_dist(self->param.eye.s, *pos);
}

//
QMVEC qg_camera_project(const QgCamera* self, const QMVEC v, const QMMAT* world)
{
	const QmSize size = RDH_TRANSFORM->size;
	QMMAT vp = qm_mat4_mul(self->mat.view, self->mat.proj);
	QMMAT m = qm_mat4_mul(world != NULL ? *world : RDH_TRANSFORM->world.s, vp);
	QMVEC t = qm_vec3_trfm(v, m);
	QmFloat3A a;
	qm_vec_to_float3a(t, &a);
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
void qg_camera_control(QgCamera* self, const QgCamCtrl* ctrl, float advance)
{
	qn_cast_vtable(self, QGCAMERA)->control(self, ctrl, advance);
}

//
static void _camera_internal_update(QnGam g)
{
	QgCamera* self = qn_cast_type(g, QgCamera);

	if (QN_TMASK(self->flags, QGCAMF_LAYOUT))
		self->param.aspect = qg_get_aspect();
	self->mat.proj = qm_mat4_perspective_lh(self->param.fov, self->param.aspect, self->param.znear, self->param.zfar);

	self->param.angle.s = qm_vec_crad(qm_vec_add(self->param.angle.s, self->param.drot.s));
	QMMAT r = qm_mat4_rot_vec3(self->param.angle.s);
	QMVEC up = qm_vec3_trfm_norm(QMCONST_UNIT_R1.s, r);
	QMVEC ah = qm_vec3_trfm_norm(QMCONST_UNIT_R2.s, r);

	if (QN_TMASK(self->flags, QGCAMF_MAYA))
	{
		// 마야 카메라
		QMVEC move = qm_vec3_trfm_norm(self->param.dmove.s, r);
		self->param.at.s = qm_vec_add(self->param.at.s, move);
		QMVEC dist = qm_vec_mag(ah, self->param.dist);
		self->param.eye.s = qm_vec_sub(self->param.at.s, dist);
		QMVEC at = qm_vec_add(self->param.eye.s, ah);
		self->mat.view = qm_mat4_lookat_lh(self->param.eye.s, at, up);
	}
	else
	{
		// FPS 카메라
		QMVEC move = qm_vec3_trfm_norm(self->param.dmove.s, r);
		self->param.eye.s = qm_vec_add(self->param.eye.s, move);
		self->param.at.s = qm_vec_add(self->param.eye.s, ah);
		self->mat.view = qm_mat4_lookat_lh(self->param.eye.s, self->param.at.s, up);
	}

	self->mat.invv = qm_mat4_inv(self->mat.view);

	self->param.dmove.s = qm_vec_zero();
	self->param.drot.s = qm_vec_zero();
	qg_set_camera(self);
}

//
static void _camera_internal_control(QnGam g, const QgCamCtrl* ctrl, float advance)
{
	static const QgCamCtrl def_ctrl =
	{
		QIK_W, QIK_S, QIK_A, QIK_D, QIK_R, QIK_F,
		QIK_LEFT, QIK_RIGHT, QIK_UP, QIK_DOWN,
		QIM_RIGHT,
	};
	QgCamera* self = qn_cast_type(g, QgCamera);
	const QgCamCtrl* c = ctrl != NULL ? ctrl : &def_ctrl;
	QmVec4 move = { .s = qm_vec_zero() };
	QmVec4 rot = { .s = qm_vec_zero() };

	if (qg_get_key_state(c->move_front))
		move.Z += 1.0f;
	if (qg_get_key_state(c->move_back))
		move.Z -= 1.0f;
	if (qg_get_key_state(c->move_left))
		move.X -= 1.0f;
	if (qg_get_key_state(c->move_right))
		move.X += 1.0f;
	if (qg_get_key_state(c->move_up))
		move.Y += 1.0f;
	if (qg_get_key_state(c->move_down))
		move.Y -= 1.0f;
	if (qg_get_key_state(c->rot_yaw_left))
		rot.Y -= 1.0f;
	if (qg_get_key_state(c->rot_yaw_right))
		rot.Y += 1.0f;
	if (qg_get_key_state(c->rot_pitch_up))
		rot.X -= 1.0f;
	if (qg_get_key_state(c->rot_pitch_down))
		rot.X += 1.0f;

	if (c->rot_button == QIM_NONE || qg_get_mouse_button_state(c->rot_button))
	{
		QmPoint d;
		qg_get_mouse_get_delta(&d);
		rot.Y += (float)d.X * -0.1f;
		rot.X += (float)d.Y * -0.1f;
	}

	self->param.dmove.s = qm_vec_mul(move.s, qm_vec_mag(self->param.smove.s, advance));
	self->param.drot.s = qm_vec_mul(rot.s, qm_vec_mag(self->param.srot.s, advance));
}

//
static void _camera_init(QgCamera* self)
{
	RendererTransform* tm = RDH_TRANSFORM;

	self->param.aspect = qg_get_aspect();
	self->param.fov = QM_PI_Q;
	self->param.znear = tm->Near;
	self->param.zfar = tm->Far;

	self->param.dist = 10.0f;
	self->param.smove.s = qm_vec3(10.0f, 10.0f, 10.0f);
	self->param.srot.s = qm_vec3(QM_TAU_H, QM_TAU_H, QM_TAU_H);

	self->param.angle.s = qm_vec_zero();
	// eye와 at은 카메라 초기화에서 설정

	self->flags = QGCAMF_LAYOUT | QGCAMF_PAUSE;
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
	self->param.eye.s = qm_vec_zero();
	self->param.at.s = QMCONST_UNIT_R2.s;
	QN_SMASK(self->flags, QGCAMF_FPS, true);
	_camera_internal_update(self);

	static const QN_DECL_VTABLE(QGCAMERA) vt_qg_camera =
	{
		{
			"Camera",
			qg_camera_dispose,
		},
		_camera_internal_update,
		_camera_internal_control,
	};
	return qn_gam_init(self, vt_qg_camera);
}

//
QgCamera* qg_create_maya_camera(void)
{
	QgCamera* self = qn_alloc_zero_1(QgCamera);
	_camera_init(self);
	self->param.eye.s = qm_vec3(0.0f, 0.0f, -self->param.dist);
	self->param.at.s = qm_vec_zero();
	QN_SMASK(self->flags, QGCAMF_MAYA, true);
	_camera_internal_update(self);

	static const QN_DECL_VTABLE(QGCAMERA) vt_qg_camera =
	{
		{
			"MayaCamera",
			qg_camera_dispose,
		},
		_camera_internal_update,
		_camera_internal_control,
	};
	return qn_gam_init(self, vt_qg_camera);
}
