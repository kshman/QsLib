#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"

QgRdh* qg_rdh_instance = NULL;


//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

//
QgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags)
{
	qg_open_stub(title, width, height, flags);
	qn_retval_if_fail(qg_stub_instance, NULL);

	static struct
	{
		const char* name;
		const char* alias;
		QgRdh* (*func)(void*, int);
	} s_allocators[] =
	{
		{ "ES2", "GLES2", es2_allocator },
		{ NULL, NULL, es2_allocator },
	};

	QgRdh* (*allocator)(void*, int) = s_allocators[QN_COUNTOF(s_allocators) - 1].func;
	if (driver != NULL)
	{
		for (size_t i = 0; i < QN_COUNTOF(s_allocators); i++)
			if (strcmp(s_allocators[i].name, driver) == 0 || strcmp(s_allocators[i].alias, driver) == 0)
			{
				allocator = s_allocators[i].func;
				break;
			}
	}

	// 개별 디바이스
	QgRdh* self = allocator(qg_stub_instance->oshandle, flags);
	if (self == NULL)
		return NULL;

	// 뒤에서 쓸지도 모르니 미리 설정
	qg_rdh_instance = self;

	// 공통 설정
	self->caps.test_stage_valid = true;
	qn_vec2_set(&self->tm.z, 1.0f, 100000.0f);

	qvt_cast(self, QgRdh)->reset(self);

	return self;
}

//
void rdh_internal_dispose(QmGam* g)
{
	QgRdh* self = qm_cast(g, QgRdh);

	if (qg_rdh_instance == self)
		qg_rdh_instance = NULL;
	qn_free(self);

	qg_close_stub();
}

//
void rdh_internal_reset(QgRdh* self)
{
	// tm
	QgRenderTm* tm = &self->tm;
	tm->size = qg_stub_instance->size;
	const float aspect = qn_point_aspect(&tm->size);
	qn_mat4_rst(&tm->world);
	qn_mat4_rst(&tm->view);
	qn_mat4_perspective_lh(&tm->proj, QN_PI_H, aspect, tm->z._near, tm->z._far);
	tm->vipr = tm->proj;
	qn_mat4_rst(&tm->inv);
	qn_mat4_rst(&tm->ortho);
	qn_mat4_rst(&tm->frm);
	for (size_t i = 0; i < QN_COUNTOF(tm->tex); i++)
		qn_mat4_rst(&tm->tex[i]);

	// param
	QgRenderParam* param = &self->param;
	param->bones = 0;
	param->bonptr = NULL;
	for (size_t i = 0; i < QN_COUNTOF(param->v); i++)
		qn_vec4_rst(&param->v[i]);
	for (size_t i = 0; i < QN_COUNTOF(param->m); i++)
		qn_mat4_rst(&param->m[i]);
	qn_color_set(&param->bgc, 0.1f, 0.1f, 0.1f, 1.0f);
}

//
const QgDeviceInfo* qg_rdh_get_device_info(QgRdh* self)
{
	return &self->caps;
}

//
const QgRenderInvoke* qg_rdh_get_render_invokes(QgRdh* self)
{
	return &self->invokes;
}

//
const QgRenderTm* qg_rdh_get_render_tm(QgRdh* self)
{
	return &self->tm;
}

//
const QgRenderParam* qg_rdh_get_render_param(QgRdh* self)
{
	return &self->param;
}

//
bool qg_rdh_loop(QgRdh* self)
{
	if (!qg_loop())
		return false;

	// 인보크 리셋
	QgRenderInvoke* ivk = &self->invokes;
	ivk->invokes = 0;
	ivk->begins = 0;
	ivk->ends = 0;
	ivk->shaders = 0;
	ivk->params = 0;
	ivk->transforms = 0;
	ivk->draws = 0;
	ivk->primitives = 0;

	ivk->flush = false;

	return true;
}

//
bool qg_rdh_poll(QgRdh* self, QgEvent* ev)
{
	const bool ret = qg_poll(ev);
	if (!ret)
		return false;

	if (ev->ev == QGEV_LAYOUT && !qn_point_eq(&self->tm.size, &qg_stub_instance->size))
		qvt_cast(self, QgRdh)->reset(self);

	return ret;
}

//
void qg_rdh_exit_loop(QgRdh* self)
{
	qg_exit_loop();
}

//
bool qg_rdh_begin(QgRdh* self, bool clear)
{
	self->invokes.invokes++;
	self->invokes.begins++;
	self->invokes.flush = false;
	return qvt_cast(self, QgRdh)->begin(self, clear);
}

//
void qg_rdh_end(QgRdh* self)
{
	self->invokes.invokes++;
	self->invokes.ends++;
	self->invokes.flush = true;
	qvt_cast(self, QgRdh)->end(self);
}

//
void qg_rdh_flush(QgRdh* self)
{
	if (!self->invokes.flush)
	{
		qn_debug_outputs(true, "RDH", "use end before flush");
		qg_rdh_end(self);
	}
	qvt_cast(self, QgRdh)->flush(self);
	self->invokes.invokes++;
	self->invokes.frames++;
}

//
void qg_rdh_reset(QgRdh* self)
{
	self->invokes.invokes++;
	qvt_cast(self, QgRdh)->reset(self);
}

//
void qg_rdh_clear(QgRdh* self, QgClear clear, const QnColor* color, int stencil, float depth)
{
	self->invokes.invokes++;
	qvt_cast(self, QgRdh)->clear(self, clear, color, stencil, depth);
}

//
void qg_rdh_set_param_vec4(QgRdh* self, int at, const QnVec4* v)
{
	qn_ret_if_fail(v && (size_t)at < QN_COUNTOF(self->param.v));
	self->param.v[at] = *v;
	self->invokes.invokes++;
}

//
void qg_rdh_set_param_vec3(QgRdh* self, int at, const QnVec3* v)
{
	qn_ret_if_fail(v && (size_t)at < QN_COUNTOF(self->param.v));
	qn_vec4_set(&self->param.v[at], v->x, v->y, v->z, 0.0f);
	self->invokes.invokes++;
}

//
void qg_rdh_set_param_mat4(QgRdh* self, int at, const QnMat4* m)
{
	qn_ret_if_fail(m && (size_t)at < QN_COUNTOF(self->param.m));
	self->param.m[at] = *m;
	self->invokes.invokes++;
}

//
void qg_rdh_set_param_weight(QgRdh* self, int count, QnMat4* weight)
{
	qn_ret_if_fail(weight && count > 0);
	self->param.bones = count;
	self->param.bonptr = weight;
	self->invokes.invokes++;
}

//
void qg_rdh_set_background(QgRdh* self, const QnColor* color)
{
	if (color)
		self->param.bgc = *color;
	else
		qn_color_set(&self->param.bgc, 0.0f, 0.0f, 0.0f, 1.0f);
	self->invokes.invokes++;
}

//
void qg_rdh_set_view_proj(QgRdh* self, const QnMat4* proj, const QnMat4* view)
{
	qn_ret_if_fail(proj && view);
	self->tm.proj = *proj;
	self->tm.view = *view;
	qn_mat4_inv(&self->tm.inv, view, NULL);
	qn_mat4_mul(&self->tm.vipr, proj, view);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

//
void qg_rdh_set_proj(QgRdh* self, const QnMat4* proj)
{
	qn_ret_if_fail(proj);
	self->tm.proj = *proj;
	qn_mat4_mul(&self->tm.vipr, &self->tm.view, proj);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

//
void qg_rdh_set_view(QgRdh* self, const QnMat4* view)
{
	qn_ret_if_fail(view);
	self->tm.view = *view;
	qn_mat4_inv(&self->tm.inv, view, NULL);
	qn_mat4_mul(&self->tm.vipr, view, &self->tm.proj);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

//
void qg_rdh_set_world(QgRdh* self, const QnMat4* world)
{
	qn_ret_if_fail(world);
	self->tm.world = *world;
	self->invokes.invokes++;
	self->invokes.transforms++;
}

//
QgVlo* qg_rdh_create_layout(QgRdh* self, int count, const QgPropLayout* layouts)
{
	qn_retval_if_fail(layouts && count > 0, false);
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->create_layout(self, count, layouts);
}

//
QgShd* qg_rdh_create_shader(QgRdh* self, const char* name)
{
	if (name == NULL)
	{
		char tmpname[64];
		qn_snprintf(tmpname, QN_COUNTOF(tmpname), "Shader#%zu", qn_number());
		name = tmpname;
	}
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->create_shader(self, name);
}

//
QgBuf* qg_rdh_create_buffer(QgRdh* self, QgBufType type, int count, int stride, const void* data)
{
	qn_retval_if_fail(count > 0 && stride > 0, NULL);
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->create_buffer(self, type, count, stride, data);
}

//
void qg_rdh_set_shader(QgRdh* self, QgShd* shader, QgVlo* layout)
{
	self->invokes.invokes++;
	self->invokes.shaders++;
	qvt_cast(self, QgRdh)->set_shader(self, shader, layout);
}

//
bool qg_rdh_set_index(QgRdh* self, QgBuf* buffer)
{
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->set_index(self, buffer);
}

//
bool qg_rdh_set_vertex(QgRdh* self, QgLoStage stage, QgBuf* buffer)
{
	qn_retval_if_fail((size_t)stage < QGLOS_MAX_VALUE, false);
	self->invokes.invokes++;
	return qvt_cast(self, QgRdh)->set_vertex(self, stage, buffer);
}

//
bool qg_rdh_draw(QgRdh* self, QgTopology tpg, int vcount)
{
	qn_retval_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_retval_if_fail(vcount > 0, false);
	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, QgRdh)->draw(self, tpg, vcount);
}

//
bool qg_rdh_draw_indexed(QgRdh* self, QgTopology tpg, int icount)
{
	qn_retval_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_retval_if_fail(icount > 0, false);
	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, QgRdh)->draw_indexed(self, tpg, icount);
}

//
bool qg_rdh_ptr_draw(QgRdh* self, QgTopology tpg, int vcount, int vstride, const void* vdata)
{
	qn_retval_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_retval_if_fail(vcount > 0 && vstride >= 0 && vdata, false);

	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, QgRdh)->ptr_draw(self, tpg, vcount, vstride, vdata);

}

//
bool qg_rdh_ptr_draw_indexed(QgRdh* self, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata)
{
	qn_retval_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_retval_if_fail(vcount > 0 && vstride >= 0 && vdata, false);
	qn_retval_if_fail(icount > 0 && idata, false);

	self->invokes.invokes++;
	self->invokes.draws++;
	return qvt_cast(self, QgRdh)->ptr_draw_indexed(self, tpg, vcount, vstride, vdata, icount, istride, idata);
}

