#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"

//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

// 활성 렌더러
RdhBase* qg_instance_rdh = NULL;

//
bool qg_open_rdh(const char* driver, const char* title, int display, int width, int height, int flags, int features)
{
	struct rdh_renderer
	{
		const char* name;
		const char* alias;
		RdhBase* (*allocator)(QgFlag, QgFeature);
		QgFeature feature;
	};
	static struct rdh_renderer renderers[] =
	{
#if defined USE_ES
		{ "ES", "GLES", es_allocator, QGRENDERER_ES3 },
#endif
#if defined USE_ES
		{ NULL, NULL, es_allocator, QGRENDERER_ES3 },
#else
		{ NULL, NULL, NULL, 0 },
#endif
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

	features |= renderer->feature;
	if (renderer->allocator == NULL)
	{
		qn_debug_outputs(true, "RDH", "cannot found valid renderer");
		return false;
	}

	bool stub_created;
	if (qg_instance_stub != NULL)
		stub_created = false;
	else
	{
		qg_open_stub(title, display, width, height, flags | QGSPECIFIC_RDHSTUB, features);
		qn_val_if_fail(qg_instance_stub, false);
		stub_created = true;
	}

	// 개별 디바이스
	RdhBase* rdh = renderer->allocator(flags, features);
	if (rdh == NULL)
	{
		if (stub_created)
			qg_close_stub();
		return false;
	}
	qg_instance_rdh = rdh;

	// 공통 설정
	RenderTransform* tm = &rdh->tm;
	tm->depth.Near = 1.0f;
	tm->depth.Far = 100000.0f;

	RenderParam* param = &rdh->param;
	param->bgc = qm_color(0.1f, 0.1f, 0.1f, 1.0f);
	for (size_t i = 0; i < QN_COUNTOF(param->v); i++)
		qm_rst(&param->v[i]);
	for (size_t i = 0; i < QN_COUNTOF(param->m); i++)
		qm_rst(&param->m[i]);

	// 
	qs_cast_vt(rdh, RDHBASE)->reset();			// 장치 리셋
	qn_timer_reset(qg_instance_stub->timer);	// 타이머도 리셋해둔다
	return true;
}

//
void qg_close_rdh(void)
{
	RdhBase* rdh = qg_instance_rdh;
	qs_unload(rdh);
}

//
void rdh_internal_dispose(void)
{
	qn_assert(qg_instance_stub != NULL, "스터브가 없어요");
	qn_assert(qg_instance_rdh != NULL, "렌더러가 없어요");

	RdhBase* rdh = qg_instance_rdh;
	const bool stub_created = QN_TMASK(qg_instance_stub->flags, QGSPECIFIC_RDHSTUB);

	qg_instance_rdh = NULL;
	qn_free(rdh);

	if (stub_created)
		qg_close_stub();
}

//
void rdh_internal_reset(void)
{
	RdhBase* rdh = qg_instance_rdh;
	const QmSize client_size = qg_instance_stub->client_size;
	const float aspect = qm_size_aspect(client_size);

	// tm
	RenderTransform* tm = &rdh->tm;
	tm->size = qm_sizef_size(client_size);
	qm_rst(&tm->world);
	qm_rst(&tm->view);
	tm->project = qm_mat4_perspective_lh(QM_PI_H, aspect, tm->depth.Near, tm->depth.Far);
	tm->view_project = qm_mul(tm->view, tm->project);
	qm_rst(&tm->inv);
	qm_rst(&tm->ortho);
	qm_rst(&tm->frm);
	for (size_t i = 0; i < QN_COUNTOF(tm->tex); i++)
		qm_rst(&tm->tex[i]);
	tm->scissor = qm_rect_size(0, 0, client_size.Width, client_size.Height);

	// param
	RenderParam* param = &rdh->param;
	param->bone_ptr = NULL;
	param->bone_count = 0;
}

//
void rdh_internal_invoke_reset(void)
{
	RdhBase* rdh = qg_instance_rdh;
	RenderInvoke* ivk = &rdh->invokes;
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
	RdhBase* rdh = qg_instance_rdh;
	const QmSize size = qg_instance_stub->client_size;
	const int width = (int)rdh->tm.size.Width;
	const int height = (int)rdh->tm.size.Height;
	if (size.Width == width && size.Height == height)
		return;
	qs_cast_vt(rdh, RDHBASE)->reset();
}

//
bool qg_rdh_begin(bool clear)
{
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.begins++;
	rdh->invokes.flush = false;
	return qs_cast_vt(rdh, RDHBASE)->begin(clear);
}

//
void qg_rdh_end(void)
{
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.ends++;
	rdh->invokes.flush = true;
	qs_cast_vt(rdh, RDHBASE)->end();
}

//
void qg_rdh_flush(void)
{
	RdhBase* rdh = qg_instance_rdh;
	if (!rdh->invokes.flush)
	{
		qn_debug_outputs(true, "RDH", "use end before flush");
		qg_rdh_end();
	}
	qs_cast_vt(rdh, RDHBASE)->flush();
	rdh->invokes.invokes++;
	rdh->invokes.frames++;
}

//
void qg_rdh_reset(void)
{
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	qs_cast_vt(rdh, RDHBASE)->reset();
}

//
void qg_rdh_clear(QgClear clear, const QmColor* color, int stencil, float depth)
{
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	qs_cast_vt(rdh, RDHBASE)->clear(clear, color, stencil, depth);
}

//
void qg_rdh_set_param_vec3(int at, const QmVec3* v)
{
	RdhBase* rdh = qg_instance_rdh;
	qn_ret_if_fail(v && (size_t)at < QN_COUNTOF(rdh->param.v));
	rdh->param.v[at] = qm_vec4v(*v, 0.0f);
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_vec4(int at, const QmVec4* v)
{
	RdhBase* rdh = qg_instance_rdh;
	qn_ret_if_fail(v && (size_t)at < QN_COUNTOF(rdh->param.v));
	rdh->param.v[at] = *v;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_mat4(int at, const QmMat4* m)
{
	RdhBase* rdh = qg_instance_rdh;
	qn_ret_if_fail(m && (size_t)at < QN_COUNTOF(rdh->param.m));
	rdh->param.m[at] = *m;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_weight(int count, QmMat4* weight)
{
	qn_ret_if_fail(weight && count > 0);
	RdhBase* rdh = qg_instance_rdh;
	rdh->param.bone_count = count;
	rdh->param.bone_ptr = weight;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_background(const QmColor* color)
{
	RdhBase* rdh = qg_instance_rdh;
	if (color)
		rdh->param.bgc = *color;
	else
		rdh->param.bgc = qm_color(0.0f, 0.0f, 0.0f, 1.0f);
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_world(const QmMat4* world)
{
	qn_ret_if_fail(world);
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.world = *world;
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_view(const QmMat4* view)
{
	qn_ret_if_fail(view);
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.view = *view;
	rdh->tm.inv = qm_inv(*view);
	rdh->tm.view_project = qm_mul(*view, rdh->tm.project);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_project(const QmMat4* proj)
{
	qn_ret_if_fail(proj);
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.project = *proj;
	rdh->tm.view_project = qm_mul(rdh->tm.view, *proj);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_view_project(const QmMat4* proj, const QmMat4* view)
{
	qn_ret_if_fail(proj && view);
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.project = *proj;
	rdh->tm.view = *view;
	rdh->tm.inv = qm_inv(*view);
	rdh->tm.view_project = qm_mul(*proj, *view);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
QgBuffer* qg_rdh_create_buffer(QgBufType type, int count, int stride, const void* data)
{
	qn_val_if_fail(count > 0 && stride > 0, NULL);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->create_buffer(type, count, stride, data);
}

//
QgRender* qg_rdh_create_render(const QgPropRender* prop, bool compile_shader)
{
	qn_val_if_fail(prop != NULL, NULL);

#define RDH_CHK_NULL(sec,item)				if (prop->sec.item == NULL)\
		{ qn_debug_outputf(true, "RDH", "'%s.%s' has no data", #sec, #item); return NULL; } (void)1
#define RDH_CHK_RANGE_MAX1(item,vmax)		if ((size_t)prop->item < (vmax))\
		{ qn_debug_outputf(true, "RDH", "invalid '%s' value: %d", #item, prop->item); return NULL; } (void)1
#define RDH_CHK_RANGE_MAX2(sec,item,vmax)	if ((size_t)prop->sec.item < (vmax))\
		{ qn_debug_outputf(true, "RDH", "invalid '%s.%s' value: %d", #sec, #item, prop->sec.item); return NULL; } (void)1
#define RDH_CHK_RANGE_MIN(sec,item,value)	if ((size_t)prop->sec.item > (value))\
		{ qn_debug_outputf(true, "RDH", "invalid '%s.%s' value: %d", #sec, #item, prop->sec.item); return NULL; } (void)1

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

	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->create_render(prop, compile_shader);
}

//
bool qg_rdh_set_index(QgBuffer* buffer)
{
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_index(buffer);
}

//
bool qg_rdh_set_vertex(QgLoStage stage, QgBuffer* buffer)
{
	qn_val_if_fail((size_t)stage < QGLOS_MAX_VALUE, false);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_vertex(stage, buffer);
}

//
void qg_rdh_set_render(QgRender* render)
{
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	qs_cast_vt(rdh, RDHBASE)->set_render(render);
}

//
bool qg_rdh_draw(QgTopology tpg, int vertices)
{
	qn_val_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_val_if_fail(vertices > 0, false);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->draw(tpg, vertices);
}

//
bool qg_rdh_draw_indexed(QgTopology tpg, int indices)
{
	qn_val_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_val_if_fail(indices > 0, false);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->draw_indexed(tpg, indices);
}

//
bool qg_rdh_ptr_draw(QgTopology tpg, int vertices, int stride, const void* vertex_data)
{
	qn_val_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_val_if_fail(vertices > 0 && stride >= 0 && vertex_data, false);

	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->ptr_draw(tpg, vertices, stride, vertex_data);

}

//
bool qg_rdh_ptr_draw_indexed(QgTopology tpg,
	int vertices, int vertex_stride, const void* vertex_data,
	int indices, int index_stride, const void* index_data)
{
	qn_val_if_fail((size_t)tpg < QGTPG_MAX_VALUE, false);
	qn_val_if_fail(vertices > 0 && vertex_stride >= 0 && vertex_data, false);
	qn_val_if_fail(indices > 0 && index_data, false);

	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->ptr_draw_indexed(tpg, vertices, vertex_stride, vertex_data, indices, index_stride, index_data);
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

//
void* qg_buf_map(QgBuffer* self)
{
	qn_val_if_fail(self->mapped == false, NULL);
	return qs_cast_vt(self, QGBUFFER)->map(self);
}

//
bool qg_buf_unmap(QgBuffer* self)
{
	qn_val_if_fail(self->mapped != false, false);
	return qs_cast_vt(self, QGBUFFER)->unmap(self);
}

//
bool qg_buf_data(QgBuffer* self, const void* data)
{
	qn_val_if_fail(self->mapped == false, false);
	qn_val_if_fail(data, false);
	return qs_cast_vt(self, QGBUFFER)->data(self, data);
}
