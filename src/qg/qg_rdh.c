#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

// 전역 디바이스
QgRdh* qg_rdh_instance = NULL;


//////////////////////////////////////////////////////////////////////////
// 도움꾼


//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

static void _rdh_construct(QgRdh* self, qgStub* stub);
static void _rdh_finalize(QgRdh* self);
static void _rdh_reset(QgRdh* self);

QgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags)
{
	qgStub* stub = qg_stub_new(title, width, height, flags);
	qn_retval_if_fail(stub, NULL);

	struct
	{
		const char* name;
		const char* alias;
		QgRdh* (*func)();
	} allocators[] =
	{
		{ "ES2", "GLES2", _es2_allocator },
		{ NULL, NULL, _es2_allocator },
	};

	QgRdh* (*allocator)() = allocators[QN_COUNTOF(allocators) - 1].func;
	if (driver != NULL)
	{
		for (size_t i = 0; i < QN_COUNTOF(allocators); i++)
			if (strcmp(allocators[i].name, driver) == 0 || strcmp(allocators[i].alias, driver) == 0)
			{
				allocator = allocators[i].func;
				break;
			}
	}

	QgRdh* self = allocator();

	_rdh_construct(self, stub);
	if (!qvt_cast(self, QgRdh)->_construct(self, flags))
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

static void _rdh_construct(QgRdh* self, qgStub* stub)
{
	self->stub = stub;

	self->caps.test_stage_valid = true;
}

void _rdh_dispose(QgRdh* self)
{
	qvt_cast(self, QgRdh)->_finalize(self);
	_rdh_finalize(self);

	qn_free(self);

	if (qg_rdh_instance == self)
		qg_rdh_instance = NULL;
}

static void _rdh_finalize(QgRdh* self)
{
	qm_unload(self->stub);
}

static void _rdh_reset(QgRdh* self)
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
	qvt_cast(self, QgRdh)->_reset(self);
}

const QgDeviceInfo* qg_rdh_get_device_info(QgRdh* self)
{
	return &self->caps;
}

const QgRenderInvoke* qg_rdh_get_render_info(QgRdh* self)
{
	return &self->invokes;
}

const QgRenderTm* qg_rdh_get_render_tm(QgRdh* self)
{
	return &self->tm;
}

const QgRenderParam* qg_rdh_get_render_param(QgRdh* self)
{
	return &self->param;
}

//
bool qg_rdh_loop(QgRdh* self)
{
	if (!qg_stub_loop(self->stub))
		return false;

	// 여기서 가상일 때 레이아웃 확인

	// 초기화
	QgRenderInvoke* i = &self->invokes;
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
bool qg_rdh_poll(QgRdh* self, QgEvent* ev)
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

bool qg_rdh_begin(QgRdh* self)
{
	self->invokes.flush = false;
	return qvt_cast(self, QgRdh)->begin(self);
}

void qg_rdh_end(QgRdh* self)
{
	self->invokes.ends++;
	self->invokes.flush = true;
	qvt_cast(self, QgRdh)->end(self);
}

void qg_rdh_flush(QgRdh* self)
{
	if (!self->invokes.flush)
	{
		qn_debug_outputs(true, "RDH", "call end before flush");
		qg_rdh_end(self);
	}
	qvt_cast(self, QgRdh)->flush(self);
	self->invokes.frames++;
}

void qg_rdh_set_param_vec4(QgRdh* self, int at, const QnVec4* v)
{
	qn_ret_if_fail(v);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.v));
	self->param.v[at] = *v;
	self->invokes.invokes++;
}

void qg_rdh_set_param_vec3(QgRdh* self, int at, const QnVec3* v)
{
	qn_ret_if_fail(v);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.v));
	qn_vec4_set(&self->param.v[at], v->x, v->y, v->z, 0.0f);
	self->invokes.invokes++;
}

void qg_rdh_set_param_mat4(QgRdh* self, int at, const QnMat4* m)
{
	qn_ret_if_fail(m);
	qn_ret_if_fail((size_t)at < QN_COUNTOF(self->param.m));
	self->param.m[at] = *m;
	self->invokes.invokes++;
}

void qg_rdh_set_param_weight(QgRdh* self, int count, QnMat4* weight)
{
	self->param.bones = count;
	self->param.bonptr = weight;
	self->invokes.invokes++;
}

void qg_rdh_set_clear(QgRdh* self, const QnColor* color)
{
	if (color)
		self->param.clear = *color;
	else
		qn_color_set(&self->param.clear, 0.0f, 0.0f, 0.0f, 1.0f);
	self->invokes.invokes++;
}

void qg_rdh_set_proj(QgRdh* self, const QnMat4* m)
{
	qn_ret_if_fail(m);
	self->tm.proj = *m;
	qn_mat4_mul(&self->tm.vp, &self->tm.view, m);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

void qg_rdh_set_view(QgRdh* self, const QnMat4* m)
{
	qn_ret_if_fail(m);
	self->tm.view = *m;
	qn_mat4_inv(&self->tm.inv, m, NULL);
	qn_mat4_mul(&self->tm.vp, m, &self->tm.proj);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

void qg_rdh_set_world(QgRdh* self, const QnMat4* m)
{
	qn_ret_if_fail(m);
	self->tm.world = *m;
	self->invokes.invokes++;
	self->invokes.transforms++;
}

QgVlo* qg_rdh_create_layout(QgRdh* self, int count, const QgPropLayout* layouts)
{
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->create_layout(self, count, layouts);
}

QgShd* qg_rdh_create_shader(QgRdh* self, const char* name)
{
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->create_shader(self, name);
}

QgBuf* qg_rdh_create_buffer(QgRdh* self, QgBufType type, int count, int stride, const void* data)
{
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->create_buffer(self, type, count, stride, data);
}

void qg_rdh_set_shader(QgRdh* self, QgShd* shader, QgVlo* layout)
{
	self->invokes.invokes++;
	self->invokes.shaders++;
	qvt_cast(self, QgRdh)->set_shader(self, shader, layout);
}

bool qg_rdh_set_index(QgRdh* self, void* buffer)
{
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->set_index(self, buffer);
}

bool qg_rdh_set_vertex(QgRdh* self, QgLoStage stage, void* buffer)
{
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->set_vertex(self, stage, buffer);
}

bool qg_rdh_draw(QgRdh* self, QgTopology tpg, int vcount)
{
	qn_retval_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_retval_if_fail(vcount > 0, false);

	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, QgRdh)->draw(self, tpg, vcount);
}

bool qg_rdh_draw_indexed(QgRdh* self, QgTopology tpg, int icount)
{
	qn_retval_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_retval_if_fail(icount > 0, false);

	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, QgRdh)->draw_indexed(self, tpg, icount);
}

bool qg_rdh_ptr_draw(QgRdh* self, QgTopology tpg, int vcount, int vstride, const void* vdata)
{
	qn_retval_if_fail(vcount > 0 && vstride >= 0 && vdata, false);

	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, QgRdh)->ptr_draw(self, tpg, vcount, vstride, vdata);

}

bool qg_rdh_ptr_draw_indexed(QgRdh* self, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata)
{
	qn_retval_if_fail(vcount > 0 && vstride >= 0 && vdata, false);
	qn_retval_if_fail(icount > 0 && idata, false);

	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, QgRdh)->ptr_draw_indexed(self, tpg, vcount, vstride, vdata, icount, istride, idata);
}

