#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

qgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags)
{
	qgStub* stub = qg_stub_new(title, width, height, flags);
	qn_retval_if_fail(stub, NULL);

	struct
	{
		const char* name;
		const char* alias;
		pointer_t(*func)();
	} allocators[] =
	{
		{ "ES2", "GLES2", _es2_allocator },
		{ NULL, NULL, _es2_allocator },
	};

	pointer_t(*allocator)() = allocators[QN_COUNTOF(allocators) - 1].func;
	if (driver != NULL)
	{
		for (size_t i = 0; i < QN_COUNTOF(allocators); i++)
			if (strcmp(allocators[i].name, driver) == 0 || strcmp(allocators[i].alias, driver) == 0)
			{
				allocator = allocators[i].func;
				break;
			}
	}

	qgRdh* self = qm_cast(allocator(), qgRdh);

	_rdh_construct(self, stub);
	if (!qvt_cast(self, qgRdh)->_construct(self, flags))
	{
		_rdh_finalize(self);
		qn_free(self);
		return NULL;
	}

	qg_stub_instance = qm_cast(self, qgStub);
	return self;
}

void _rdh_dispose(pointer_t g)
{
	qgRdh* self = qm_cast(g, qgRdh);

	qvt_cast(self, qgRdh)->_finalize(self);
	_rdh_finalize(self);

	qn_free(self);
}

void _rdh_construct(pointer_t g, qgStub* stub)
{
	qgRdh* self = qm_cast(g, qgRdh);
	self->stub = stub;
	_rdh_reset(self);
}

void _rdh_finalize(pointer_t g)
{
	qgRdh* self = qm_cast(g, qgRdh);
	qm_unload(self->stub);
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

//
bool qg_rdh_loop(pointer_t g)
{
	qn_retval_if_fail(g, false);
	qgRdh* self = qm_cast(g, qgRdh);

	if (!qg_stub_loop(self->stub))
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

	return true;
}

//
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
	return qvt_cast(self, qgRdh)->begin(self);
}

void qg_rdh_end(pointer_t g)
{
	qn_ret_if_fail(g);
	qgRdh* self = qm_cast(g, qgRdh);

	self->info.ends++;
	self->info.flush = true;

	qvt_cast(self, qgRdh)->end(self);
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
	if (!qvt_cast(self, qgRdh)->primitive_begin(self, tpg, count, stride, &vert))
		return;
	memcpy(vert, data, (size_t)(count * stride));
	qvt_cast(self, qgRdh)->primitive_end(self);

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
	if (!qvt_cast(self, qgRdh)->indexed_primitive_begin(self, tpg, vcount, vstride, &vert, icount, istride, &ind))
		return;
	memcpy(vert, vdata, (size_t)(vcount * vstride));
	memcpy(ind, idata, (size_t)(icount * istride));
	qvt_cast(self, qgRdh)->indexed_primitive_end(self);

	self->info.invokes++;
	self->info.draws++;
	self->info.vertices += vcount;
}
