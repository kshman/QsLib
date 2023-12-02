#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

// 전역 디바이스
qgRdh* qg_rdh_instance = NULL;


//////////////////////////////////////////////////////////////////////////
// 도움꾼


//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

static void _rdh_construct(qgRdh* self, qgStub* stub);
static void _rdh_finalize(qgRdh* self);
static void _rdh_reset(qgRdh* self);

qgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags)
{
	qgStub* stub = qg_stub_new(title, width, height, flags);
	qn_retval_if_fail(stub, NULL);

	struct
	{
		const char* name;
		const char* alias;
		qgRdh* (*func)();
	} allocators[] =
	{
		{ "ES2", "GLES2", _es2_allocator },
		{ NULL, NULL, _es2_allocator },
	};

	qgRdh* (*allocator)() = allocators[QN_COUNTOF(allocators) - 1].func;
	if (driver != NULL)
	{
		for (size_t i = 0; i < QN_COUNTOF(allocators); i++)
			if (strcmp(allocators[i].name, driver) == 0 || strcmp(allocators[i].alias, driver) == 0)
			{
				allocator = allocators[i].func;
				break;
			}
	}

	qgRdh* self = allocator();

	_rdh_construct(self, stub);
	if (!qvt_cast(self, qgRdh)->_construct(self, flags))
	{
		_rdh_finalize(self);
		qn_free(self);
		return NULL;
	}

	_rdh_reset(self);

	if (qg_rdh_instance == NULL)
		qg_rdh_instance = self;
	return self;
}

static void _rdh_construct(qgRdh* self, qgStub* stub)
{
	self->stub = stub;

	self->caps.test_stage_valid = true;
}

void _rdh_dispose(qgRdh* self)
{
	qvt_cast(self, qgRdh)->_finalize(self);
	_rdh_finalize(self);

	qn_free(self);

	if (qg_rdh_instance == self)
		qg_rdh_instance = NULL;
}

static void _rdh_finalize(qgRdh* self)
{
	qm_unload(self->stub);
}

static void _rdh_reset(qgRdh* self)
{
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
	qn_color_set(&self->param.clear, 0.1f, 0.1f, 0.1f, 1.0f);

	//
	qvt_cast(self, qgRdh)->_reset(self);
}

const qgDeviceInfo* qg_rdh_get_device_info(qgRdh* self)
{
	return &self->caps;
}

const qgRenderInvoke* qg_rdh_get_render_info(qgRdh* self)
{
	return &self->invokes;
}

const qgRenderTm* qg_rdh_get_render_tm(qgRdh* self)
{
	return &self->tm;
}

const qgRenderParam* qg_rdh_get_render_param(qgRdh* self)
{
	return &self->param;
}

//
bool qg_rdh_loop(qgRdh* self)
{
	if (!qg_stub_loop(self->stub))
		return false;

	// 여기서 가상일 때 레이아웃 확인

	// 초기화
	qgRenderInvoke* i = &self->invokes;
	i->invokes = 0;
	i->shaders = 0;
	i->transforms = 0;
	i->draws = 0;
	i->primitives = 0;

	i->begins++;
	i->flush = false;

	return true;
}

//
bool qg_rdh_poll(qgRdh* self, qgEvent* ev)
{
	qn_retval_if_fail(ev, false);
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

bool qg_rdh_begin(qgRdh* self)
{
	self->invokes.flush = false;
	return qvt_cast(self, qgRdh)->begin(self);
}

void qg_rdh_end(qgRdh* self)
{
	self->invokes.ends++;
	self->invokes.flush = true;
	qvt_cast(self, qgRdh)->end(self);
}

void qg_rdh_flush(qgRdh* self)
{
	if (!self->invokes.flush)
	{
		qn_debug_outputs(true, "RDH", "call end before flush");
		qg_rdh_end(self);
	}
	qvt_cast(self, qgRdh)->flush(self);
	self->invokes.frames++;
}

void qg_rdh_set_param_vec4(qgRdh* self, int at, const qnVec4* v)
{
	qn_ret_if_fail(v);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.v));
	self->param.v[at] = *v;
	self->invokes.invokes++;
}

void qg_rdh_set_param_vec3(qgRdh* self, int at, const qnVec3* v)
{
	qn_ret_if_fail(v);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.v));
	qn_vec4_set(&self->param.v[at], v->x, v->y, v->z, 0.0f);
	self->invokes.invokes++;
}

void qg_rdh_set_param_mat4(qgRdh* self, int at, const qnMat4* m)
{
	qn_ret_if_fail(m);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.m));
	self->param.m[at] = *m;
	self->invokes.invokes++;
}

void qg_rdh_set_param_weight(qgRdh* self, int count, qnMat4* weight)
{
	self->param.bones = count;
	self->param.bonptr = weight;
	self->invokes.invokes++;
}

void qg_rdh_set_clear(qgRdh* self, const qnColor* color)
{
	if (color)
		self->param.clear = *color;
	else
		qn_color_set(&self->param.clear, 0.0f, 0.0f, 0.0f, 1.0f);
	self->invokes.invokes++;
}

void qg_rdh_set_proj(qgRdh* self, const qnMat4* m)
{
	qn_ret_if_fail(m);
	self->tm.proj = *m;
	qn_mat4_mul(&self->tm.vp, &self->tm.view, m);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

void qg_rdh_set_view(qgRdh* self, const qnMat4* m)
{
	qn_ret_if_fail(m);
	self->tm.view = *m;
	qn_mat4_inv(&self->tm.inv, m, NULL);
	qn_mat4_mul(&self->tm.vp, m, &self->tm.proj);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

void qg_rdh_set_world(qgRdh* self, const qnMat4* m)
{
	qn_ret_if_fail(m);
	self->tm.world = *m;
	self->invokes.invokes++;
	self->invokes.transforms++;
}

qgVlo* qg_rdh_create_layout(qgRdh* self, int count, const qgPropLayout* layouts)
{
	self->invokes.invokes++;
	return qvt_cast(self, qgRdh)->create_layout(self, count, layouts);
}

qgShd* qg_rdh_create_shader(qgRdh* self, const char* name)
{
	self->invokes.invokes++;
	return qvt_cast(self, qgRdh)->create_shader(self, name);
}

qgBuf* qg_rdh_create_buffer(qgRdh* self, qgBufType type, int count, int stride, const void* data)
{
	self->invokes.invokes++;
	return qvt_cast(self, qgRdh)->create_buffer(self, type, count, stride, data);
}

void qg_rdh_set_shader(qgRdh* self, qgShd* shader, qgVlo* layout)
{
	self->invokes.invokes++;
	self->invokes.shaders++;
	qvt_cast(self, qgRdh)->set_shader(self, shader, layout);
}

bool qg_rdh_set_index(qgRdh* self, void* buffer)
{
	self->invokes.invokes++;
	return qvt_cast(self, qgRdh)->set_index(self, buffer);
}

bool qg_rdh_set_vertex(qgRdh* self, qgLoStage stage, void* buffer)
{
	self->invokes.invokes++;
	return qvt_cast(self, qgRdh)->set_vertex(self, stage, buffer);
}

void qg_rdh_primitive_draw(qgRdh* self, qgTopology tpg, int count, int stride, const void* data)
{
	qn_ret_if_fail(count > 0 && stride > 0 && data);
	void* vert;
	if (!qvt_cast(self, qgRdh)->primitive_begin(self, tpg, count, stride, &vert))
		return;
	memcpy(vert, data, (size_t)(count * stride));
	qvt_cast(self, qgRdh)->primitive_end(self);

	self->invokes.invokes++;
	self->invokes.draws++;
	self->invokes.primitives += count;
}

void qg_rdh_primitive_draw_indexed(qgRdh* self, qgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata)
{
	qn_ret_if_fail(vcount > 0 && vstride > 0 && vdata);
	qn_ret_if_fail(icount > 0 && istride > 0 && idata);
	void* vert;
	void* ind;
	if (!qvt_cast(self, qgRdh)->indexed_primitive_begin(self, tpg, vcount, vstride, &vert, icount, istride, &ind))
		return;
	memcpy(vert, vdata, (size_t)(vcount * vstride));
	memcpy(ind, idata, (size_t)(icount * istride));
	qvt_cast(self, qgRdh)->indexed_primitive_end(self);

	self->invokes.invokes++;
	self->invokes.draws++;
	self->invokes.primitives += vcount;
}

bool qg_rdh_draw(qgRdh* self, qgTopology tpg, int vcount)
{
	qn_retval_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_retval_if_fail(vcount > 0, false);

	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, qgRdh)->draw(self, tpg, vcount);
}

bool qg_rdh_draw_indexed(qgRdh* self, qgTopology tpg, int icount)
{
	qn_retval_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_retval_if_fail(icount > 0, false);

	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, qgRdh)->draw_indexed(self, tpg, icount);
}
