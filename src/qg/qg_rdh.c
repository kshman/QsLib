#include "pch.h"
#include "qs_qn.h"
#include "qs_math.h"
#include "qs_qg.h"
#include "qg/qg_stub.h"

//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

// 활성 렌더러
RdhBase* qg_instance_rdh = NULL;

//
bool qg_open_rdh(const char* driver, const char* title, int display, int width, int height, int flags)
{
	struct rdh_renderer
	{
		const char* name;
		const char* alias;
		RdhBase* (*allocator)(int);
		QgFlag renderer;
	};
	static struct rdh_renderer renderers[] =
	{
		{ "ES", "GLES", es_allocator, QGRENDERER_ES3 },
		{ NULL, NULL, es_allocator, QGRENDERER_ES3 },
	};
	struct rdh_renderer* renderer = &renderers[QN_COUNTOF(renderers) - 1];
	if (driver != NULL)
	{
		for (size_t i = 0; i < QN_COUNTOF(renderers) - 1; i++)
			if (qn_stricmp(renderers[i].name, driver) == 0 || qn_stricmp(renderers[i].alias, driver) == 0)
			{
				renderer = &renderer[i];
				break;
			}
	}

	bool open_stub = qg_open_stub(title, display, width, height, flags | renderer->renderer);
	qn_val_if_fail(qg_instance_stub, false);

	// 개별 디바이스
	RdhBase* self = renderer->allocator(flags);
	if (self == NULL)
	{
		if (open_stub)
			qg_close_stub();
		return false;
	}

	// 뒤에서 쓸지도 모르니 미리 설정
	qg_instance_rdh = self;
	self->info.open_stub = open_stub;

	// 공통 설정
	RenderTransform* tm = &self->tm;
	tm->depth.Near = 1.0f;
	tm->depth.Far = 100000.0f;

	RenderParam* param = &self->param;
	qm_set4(&param->bgc, 0.1f, 0.1f, 0.1f, 1.0f);
	for (size_t i = 0; i < QN_COUNTOF(param->v); i++)
		qm_rst(&param->v[i]);
	for (size_t i = 0; i < QN_COUNTOF(param->m); i++)
		qm_rst(&param->m[i]);

	//
	qv_cast(self, RdhBase)->reset(self);
	return true;
}

//
void qg_close_rdh(void)
{
	RdhBase* self = qg_instance_rdh;
	qs_unload(self);
}

//
void rdh_internal_dispose(void)
{
	RdhBase* self = qg_instance_rdh;
	bool open_stub = self->info.open_stub;

	qg_instance_rdh = NULL;
	qn_free(self);

	if (open_stub)
		qg_close_stub();
}

//
void rdh_internal_reset(void)
{
	RdhBase* self = qg_instance_rdh;
	const QmSize client_size = qg_instance_stub->client_size;

	// tm
	RenderTransform* tm = &self->tm;
	qm_set1(&tm->size, &client_size);
	const float aspect = tm->size.x / tm->size.y;
	qm_rst(&tm->world);
	qm_rst(&tm->view);
	qm_mat4_perspective_lh(&tm->project, QM_PI_H, aspect, &tm->depth);
	qm_mul(&tm->view_project, &tm->view, &tm->project);
	qm_rst(&tm->inv);
	qm_rst(&tm->ortho);
	qm_rst(&tm->frm);
	for (size_t i = 0; i < QN_COUNTOF(tm->tex); i++)
		qm_rst(&tm->tex[i]);
	qm_set4(&tm->scissor, 0, 0, client_size.width, client_size.height);

	// param
	RenderParam* param = &self->param;
	param->bone_ptr = NULL;
	param->bone_count = 0;
}

//
void rdh_internal_invoke_reset(void)
{
	RdhBase* self = qg_instance_rdh;
	RenderInvoke* ivk = &self->invokes;
	ivk->invokes = 0;
	ivk->begins = 0;
	ivk->ends = 0;
	ivk->renders = 0;
	ivk->params = 0;
	ivk->vars = 0;
	ivk->transforms = 0;
	ivk->draws = 0;
	ivk->primitives = 0;
	ivk->flush = false;
}

//
void rdh_internal_check_layout(void)
{
	RdhBase* self = qg_instance_rdh;
	const QmSize size = qg_instance_stub->client_size;
	const int width = (int)self->tm.size.x;
	const int height = (int)self->tm.size.y;
	if (size.width == width && size.height == height)
		return;
	qv_cast(self, RdhBase)->reset(self);
}

//
bool qg_rdh_begin(bool clear)
{
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	self->invokes.begins++;
	self->invokes.flush = false;
	return qv_cast(self, RdhBase)->begin(self, clear);
}

//
void qg_rdh_end(void)
{
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	self->invokes.ends++;
	self->invokes.flush = true;
	qv_cast(self, RdhBase)->end(self);
}

//
void qg_rdh_flush(void)
{
	RdhBase* self = qg_instance_rdh;
	if (!self->invokes.flush)
	{
		qn_debug_outputs(true, "RDH", "use end before flush");
		qg_rdh_end();
	}
	qv_cast(self, RdhBase)->flush(self);
	self->invokes.invokes++;
	self->invokes.frames++;
}

//
void qg_rdh_reset(void)
{
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	qv_cast(self, RdhBase)->reset(self);
}

//
void qg_rdh_clear(QgClear clear, const QmColor* color, int stencil, float depth)
{
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	qv_cast(self, RdhBase)->clear(self, clear, color, stencil, depth);
}

//
void qg_rdh_set_param_vec3(int at, const QmVec3* v)
{
	RdhBase* self = qg_instance_rdh;
	qn_ret_if_fail(v && (size_t)at < QN_COUNTOF(self->param.v));
	qm_set4(&self->param.v[at], v->x, v->y, v->z, 0.0f);
	self->invokes.invokes++;
}

//
void qg_rdh_set_param_vec4(int at, const QmVec4* v)
{
	RdhBase* self = qg_instance_rdh;
	qn_ret_if_fail(v && (size_t)at < QN_COUNTOF(self->param.v));
	self->param.v[at] = *v;
	self->invokes.invokes++;
}

//
void qg_rdh_set_param_mat4(int at, const QmMat4* m)
{
	RdhBase* self = qg_instance_rdh;
	qn_ret_if_fail(m && (size_t)at < QN_COUNTOF(self->param.m));
	self->param.m[at] = *m;
	self->invokes.invokes++;
}

//
void qg_rdh_set_param_weight(int count, QmMat4* weight)
{
	qn_ret_if_fail(weight && count > 0);
	RdhBase* self = qg_instance_rdh;
	self->param.bone_count = count;
	self->param.bone_ptr = weight;
	self->invokes.invokes++;
}

//
void qg_rdh_set_background(const QmColor* color)
{
	RdhBase* self = qg_instance_rdh;
	if (color)
		self->param.bgc = *color;
	else
		qm_set4(&self->param.bgc, 0.0f, 0.0f, 0.0f, 1.0f);
	self->invokes.invokes++;
}

//
void qg_rdh_set_world(const QmMat4* world)
{
	qn_ret_if_fail(world);
	RdhBase* self = qg_instance_rdh;
	self->tm.world = *world;
	self->invokes.invokes++;
	self->invokes.transforms++;
}

//
void qg_rdh_set_view(const QmMat4* view)
{
	qn_ret_if_fail(view);
	RdhBase* self = qg_instance_rdh;
	self->tm.view = *view;
	qm_inv(&self->tm.inv, view);
	qm_mul(&self->tm.view_project, view, &self->tm.project);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

//
void qg_rdh_set_project(const QmMat4* proj)
{
	qn_ret_if_fail(proj);
	RdhBase* self = qg_instance_rdh;
	self->tm.project = *proj;
	qm_mul(&self->tm.view_project, &self->tm.view, proj);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

//
void qg_rdh_set_view_project(const QmMat4* proj, const QmMat4* view)
{
	qn_ret_if_fail(proj && view);
	RdhBase* self = qg_instance_rdh;
	self->tm.project = *proj;
	self->tm.view = *view;
	qm_inv(&self->tm.inv, view);
	qm_mul(&self->tm.view_project, proj, view);
	self->invokes.invokes++;
	self->invokes.transforms++;
}

//
QgBuffer* qg_rdh_create_buffer(QgBufType type, int count, int stride, const void* data)
{
	qn_val_if_fail(count > 0 && stride > 0, NULL);
	RdhBase* self = qg_instance_rdh;
	self->invokes.creations++;
	self->invokes.invokes++;
	return qv_cast(self, RdhBase)->create_buffer(self, type, count, stride, data);
}

//
QgRender* qg_rdh_create_render(const QgPropRender* prop, bool compile_shader)
{
	qn_val_if_fail(prop != NULL, NULL);

#define RDH_CHK_NULL(sec,item)				if (prop->sec.item == NULL)\
		{ qn_debug_outputf(true, "RDH", "'%s.%s' has no data", #sec, #item); return NULL; }
#define RDH_CHK_RANGE_MAX1(item,vmax)		if ((size_t)prop->item < vmax)\
		{ qn_debug_outputf(true, "RDH", "invalid '%s' value: %d", #item, prop->item); return NULL; }
#define RDH_CHK_RANGE_MAX2(sec,item,vmax)	if ((size_t)prop->sec.item < vmax)\
		{ qn_debug_outputf(true, "RDH", "invalid '%s.%s' value: %d", #sec, #item, prop->sec.item); return NULL; }
#define RDH_CHK_RANGE_MIN(sec,item,value)	if ((size_t)prop->sec.item > value)\
		{ qn_debug_outputf(true, "RDH", "invalid '%s.%s' value: %d", #sec, #item, prop->sec.item); return NULL; }

	// 세이더
	RDH_CHK_NULL(vs, code);
	RDH_CHK_NULL(ps, code);
	// 래스터라이저
	RDH_CHK_RANGE_MAX2(rasterizer, fill, QGFILL_MAX_VALUE);
	RDH_CHK_RANGE_MAX2(rasterizer, cull, QGCULL_MAX_VALUE);
	// 뎁스
	RDH_CHK_RANGE_MAX1(depth, QGDEPTH_MAX_VALUE);
	// 스텐실
	RDH_CHK_RANGE_MAX1(stencil, QGSTENCIL_MAX_VALUE);
	// 레이아웃
	RDH_CHK_RANGE_MIN(layout, count, 0);
	RDH_CHK_NULL(layout, elements);
	// 렌더 뎁스 포맷
	RDH_CHK_RANGE_MIN(format, count, 0);
	RDH_CHK_RANGE_MAX2(format, count, QGRVS_MAX_VALUE);
	// 토폴로지
	RDH_CHK_RANGE_MAX1(topology, QGTPG_MAX_VALUE);

#undef RDH_CHK_RANGE_MIN
#undef RDH_CHK_RANGE_MAX2
#undef RDH_CHK_RANGE_MAX1
#undef RDH_CHK_NULL

	RdhBase* self = qg_instance_rdh;
	self->invokes.creations++;
	self->invokes.invokes++;
	return qv_cast(self, RdhBase)->create_render(self, prop, compile_shader);
}

//
bool qg_rdh_set_index(QgBuffer* buffer)
{
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	return qv_cast(self, RdhBase)->set_index(self, buffer);
}

//
bool qg_rdh_set_vertex(QgLoStage stage, QgBuffer* buffer)
{
	qn_val_if_fail((size_t)stage < QGLOS_MAX_VALUE, false);
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	return qv_cast(self, RdhBase)->set_vertex(self, stage, buffer);
}

//
void qg_rdh_set_render(QgRender* render)
{
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	qv_cast(self, RdhBase)->set_render(self, render);
}

//
bool qg_rdh_draw(QgTopology tpg, int vertices)
{
	qn_val_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_val_if_fail(vertices > 0, false);
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	self->invokes.draws++;
	return qv_cast(self, RdhBase)->draw(self, tpg, vertices);
}

//
bool qg_rdh_draw_indexed(QgTopology tpg, int indices)
{
	qn_val_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_val_if_fail(indices > 0, false);
	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	self->invokes.draws++;
	return qv_cast(self, RdhBase)->draw_indexed(self, tpg, indices);
}

//
bool qg_rdh_ptr_draw(QgTopology tpg, int vertices, int stride, const void* vertex_data)
{
	qn_val_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_val_if_fail(vertices > 0 && stride >= 0 && vertex_data, false);

	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	self->invokes.draws++;
	return qv_cast(self, RdhBase)->ptr_draw(self, tpg, vertices, stride, vertex_data);

}

//
bool qg_rdh_ptr_draw_indexed(QgTopology tpg,
	int vertices, int vertex_stride, const void* vertex_data,
	int indices, int index_stride, const void* index_data)
{
	qn_val_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_val_if_fail(vertices > 0 && vertex_stride >= 0 && vertex_data, false);
	qn_val_if_fail(indices > 0 && index_data, false);

	RdhBase* self = qg_instance_rdh;
	self->invokes.invokes++;
	self->invokes.draws++;
	return qv_cast(self, RdhBase)->ptr_draw_indexed(self, tpg, vertices, vertex_stride, vertex_data, indices, index_stride, index_data);
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

//
void* qg_buf_map(QgBuffer* self)
{
	qn_val_if_fail(self->mapped == false, NULL);
	return qv_cast(self, QgBuffer)->map(self);
}

bool qg_buf_unmap(QgBuffer* self)
{
	qn_val_if_fail(self->mapped != false, false);
	return qv_cast(self, QgBuffer)->unmap(self);
}

bool qg_buf_data(QgBuffer* self, const void* data)
{
	qn_val_if_fail(self->mapped == false, false);
	qn_val_if_fail(data, false);
	return qv_cast(self, QgBuffer)->data(self, data);
}



