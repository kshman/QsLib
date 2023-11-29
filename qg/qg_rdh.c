#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스
qgRdh* qg_rdh_instance = NULL;

bool _rdh_on_init(pointer_t g, const char* title, int width, int height, int flags)
{
	qgStub* stub = qg_stub_new(NULL, title, width, height, flags);
	qn_retval_if_fail(stub, false);

	qgRdh* self = qm_cast(g, qgRdh);
	self->stub = stub;

	_rdh_reset(self);

	qg_rdh_instance = self;
	return true;
}

void _rdh_on_disp(pointer_t g)
{
	qgRdh* self = qm_cast(g, qgRdh);

	qm_unload(self->stub);

	if (qg_rdh_instance = self)
		qg_rdh_instance = NULL;
}

void _rdh_reset(pointer_t g)
{
	qgRdh* self = qm_cast(g, qgRdh);

	// tm
	float aspect = qn_point_aspect(&self->stub->size);
	qn_mat4_rst(&self->tm.world);
	qn_mat4_rst(&self->tm.view);
	qn_mat4_perspective_lh(&self->tm.proj, QN_PIH, aspect, 1.0f, 10000.0f);
	self->tm.vp = self->tm.proj;
	qn_mat4_rst(&self->tm.inv);
	qn_mat4_rst(&self->tm.ortho);
	qn_mat4_rst(&self->tm.frm);
	for (size_t i = 0; i < QN_COUNTOF(self->tm.tex); i++)
		qn_mat4_rst(&self->tm.tex[i]);

	// param
	self->param.bones = 0;
	self->param.bonptr = NULL;
	for (size_t i = 0; i < QN_COUNTOF(self->param.v); i++)
		qn_vec4_rst(&self->param.v[i]);
	for (size_t i = 0; i < QN_COUNTOF(self->param.m); i++)
		qn_mat4_rst(&self->param.m[i]);
	qn_color_set(&self->param.clear, 0.2f, 0.2f, 0.2f, 1.0f);
}

const qgDeviceInfo* qg_rdh_get_device_info(pointer_t g)
{
	qn_retval_if_fail(g, NULL);
	qgRdh* self = qm_cast(g, qgRdh);
	return &self->caps;
}

const qgRenderInfo* qg_rdh_get_render_info(pointer_t g)
{
	qn_retval_if_fail(g, NULL);
	qgRdh* self = qm_cast(g, qgRdh);
	return &self->info;
}

const qgRenderTm* qg_rdh_get_render_tm(pointer_t g)
{
	qn_retval_if_fail(g, NULL);
	qgRdh* self = qm_cast(g, qgRdh);
	return &self->tm;
}

const qgRenderParam* qg_rdh_get_render_param(pointer_t g)
{
	qn_retval_if_fail(g, NULL);
	qgRdh* self = qm_cast(g, qgRdh);
	return &self->param;
}

bool qg_rdh_poll(pointer_t g, qgEvent* ev)
{
	qn_retval_if_fail(g && ev, false);
	qgRdh* self = qm_cast(g, qgRdh);
	bool ret = qg_stub_poll(self->stub, ev);
	if (!ret)
		return false;

	if (ev->ev == QGEV_LAYOUT)
	{
		float aspect = qn_point_aspect(&self->stub->size);
		qn_mat4_rst(&self->tm.world);
		qn_mat4_rst(&self->tm.view);
		qn_mat4_perspective_lh(&self->tm.proj, QN_PIH, aspect, 1.0f, 10000.0f);
	}

	return ret;
}

bool qg_rdh_begin(pointer_t g)
{
	qn_retval_if_fail(g, false);
	qgRdh* self = qm_cast(g, qgRdh);
	if (!qg_stub_update(self->stub))
		return false;

	// 여기서 가상일 때 레이아웃 확인

	// 초기화
	qgRenderInfo* i = &self->info;
	i->invokes = 0;
	i->shaders = 0;
	i->transforms = 0;
	i->draws = 0;
	i->vertices = 0;

	i->begins++;
	i->flush = false;

	return _rdh_impl_begin(self);
}

void qg_rdh_end(pointer_t g)
{
	qn_ret_if_fail(g);
	qgRdh* self = qm_cast(g, qgRdh);

	self->info.ends++;
	self->info.flush = true;

	_rdh_impl_end(self);
}

void qg_rdh_set_param_vec4(pointer_t g, int at, const qnVec4* v)
{
	qn_ret_if_fail(g && v);
	qgRdh* self = qm_cast(g, qgRdh);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.v));
	self->param.v[at] = *v;

	self->info.invokes++;
}

void qg_rdh_set_param_vec3(pointer_t g, int at, const qnVec3* v)
{
	qn_ret_if_fail(g && v);
	qgRdh* self = qm_cast(g, qgRdh);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.v));
	qn_vec4_set(&self->param.v[at], v->x, v->y, v->z, 0.0f);

	self->info.invokes++;
}

void qg_rdh_set_param_mat4(pointer_t g, int at, const qnMat4* m)
{
	qn_ret_if_fail(g && m);
	qgRdh* self = qm_cast(g, qgRdh);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.m));
	self->param.m[at] = *m;

	self->info.invokes++;
}

void qg_rdh_set_param_weight(pointer_t g, int count, qnMat4* weight)
{
	qn_ret_if_fail(g);
	qgRdh* self = qm_cast(g, qgRdh);
	self->param.bones = count;
	self->param.bonptr = weight;

	self->info.invokes++;
}

void qg_rdh_set_clear(pointer_t g, const qnColor* color)
{
	qn_ret_if_fail(g && color);
	qgRdh* self = qm_cast(g, qgRdh);
	self->param.clear = *color;

	self->info.invokes++;
}

void qg_rdh_set_proj(pointer_t g, const qnMat4* m)
{
	qn_ret_if_fail(g && m);
	qgRdh* self = qm_cast(g, qgRdh);
	self->tm.proj = *m;
	qn_mat4_mul(&self->tm.vp, &self->tm.view, m);

	self->info.invokes++;
	self->info.transforms++;
}

void qg_rdh_set_view(pointer_t g, const qnMat4* m)
{
	qn_ret_if_fail(g && m);
	qgRdh* self = qm_cast(g, qgRdh);
	self->tm.view = *m;
	qn_mat4_inv(&self->tm.inv, m, NULL);
	qn_mat4_mul(&self->tm.vp, m, &self->tm.proj);

	self->info.invokes++;
	self->info.transforms++;
}

void qg_rdh_set_world(pointer_t g, const qnMat4* m)
{
	qn_ret_if_fail(g && m);
	qgRdh* self = qm_cast(g, qgRdh);
	self->tm.world = *m;

	self->info.invokes++;
	self->info.transforms++;
}

void qg_rdh_draw_primitive(pointer_t g, qgTopology tpg, int count, int stride, cpointer_t data)
{
	qn_ret_if_fail(g);
	qgRdh* self = qm_cast(g, qgRdh);

	qn_ret_if_fail((size_t)count > 0 && (size_t)stride > 0 && data);
	pointer_t vert;
	_rdh_impl_primitive_begin(self, tpg, count, stride, &vert);
	memcpy(vert, data, count * stride);
	_rdh_impl_primitive_end(self);

	self->info.invokes++;
	self->info.draws++;
	self->info.vertices += count;
}

void qg_rdh_draw_indexed_primitive(pointer_t g, qgTopology tpg, int vcount, int vstride, cpointer_t vdata, int icount, int istride, cpointer_t idata)
{
	qn_ret_if_fail(g);
	qgRdh* self = qm_cast(g, qgRdh);
	qn_ret_if_fail((size_t)vcount > 0 && (size_t)vstride > 0 && vdata);
	qn_ret_if_fail((size_t)icount > 0 && (size_t)istride > 0 && idata);
	pointer_t vert, ind;
	_rdh_impl_indexed_primitive_begin(self, tpg, vcount, vstride, &vert, icount, istride, &ind);
	memcpy(vert, vdata, vcount * vstride);
	memcpy(ind, idata, icount * istride);
	_rdh_impl_indexed_primitive_end(self);

	self->info.invokes++;
	self->info.draws++;
	self->info.vertices += vcount;
}
