#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"

#ifdef _MSC_VER
#pragma warning(disable:6011)
#endif

//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

// 활성 렌더러
RdhBase* qg_instance_rdh = NULL;

//
bool qg_open_rdh(const char* driver, const char* title, int display, int width, int height, int flags, int features)
{
	qg_init_enum_convs();

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
		{ "ES", "GLES", es_allocator, QGRENDERER_ES },
#endif
#if defined USE_ES
		{ NULL, NULL, es_allocator, QGRENDERER_ES },
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

	if (QN_TMASK(flags, QGFLAG_DITHER | QGFLAG_DITHER_ALPHA_STENCIL))	// 디더 강제로 줄여
	{
		if (QN_TMASK(flags, QGFLAG_DITHER_ALPHA_STENCIL))
		{
			if (qn_get_prop(QG_PROP_RGBA_SIZE) == NULL)
				qn_set_prop(QG_PROP_RGBA_SIZE, "4444");
			if (qn_get_prop(QG_PROP_STENCIL_SIZE) == NULL)
				qn_set_prop(QG_PROP_STENCIL_SIZE, "8");
		}
		else
		{
			if (qn_get_prop(QG_PROP_RGBA_SIZE) == NULL)
				qn_set_prop(QG_PROP_RGBA_SIZE, "5650");
			if (qn_get_prop(QG_PROP_STENCIL_SIZE) == NULL)
				qn_set_prop(QG_PROP_STENCIL_SIZE, "0");
		}
		if (qn_get_prop(QG_PROP_DEPTH_SIZE) == NULL)
			qn_set_prop(QG_PROP_DEPTH_SIZE, "16");
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
		qn_val_if_fail(qg_instance_stub != NULL, false);
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

	// 묶음
	for (size_t i = 0; i < QN_COUNTOF(rdh->mukums); i++)
		qg_node_mukum_init_fast(&rdh->mukums[i]);

	// 
	qs_cast_vt(rdh, RDHBASE)->layout();			// 레이아웃 재설정
	qs_cast_vt(rdh, RDHBASE)->reset();			// 장치 리셋
	qn_timer_reset(qg_instance_stub->timer);	// 타이머도 리셋해둔다
	return true;
}

//
void qg_close_rdh(void)
{
	qg_instance_rdh = qs_unloadc(qg_instance_rdh, RdhBase);
}

//
void rdh_internal_clean(void)
{
	RdhBase* rdh = qg_instance_rdh;
	for (size_t i = 0; i < QN_COUNTOF(rdh->mukums); i++)
		qg_node_mukum_disp_safe(&rdh->mukums[i]);
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
void rdh_internal_layout(void)
{
	RdhBase* rdh = qg_instance_rdh;
	const QmSize client_size = qg_instance_stub->client_size;
	const float aspect = qm_size_aspect(client_size);

	// tm
	RenderTransform* tm = &rdh->tm;
	tm->size = qm_sizef_size(client_size);
	tm->world = qm_mat4_identity();
	tm->view = qm_mat4_identity();
	tm->proj = qm_mat4_perspective_lh(QM_PI_H, aspect, tm->depth.Near, tm->depth.Far);
	tm->view_proj = tm->proj;
	tm->invv = qm_mat4_identity();
	tm->scissor = qm_rect_size(0, 0, client_size.Width, client_size.Height);
}

//
void rdh_internal_reset(void)
{
	RdhBase* rdh = qg_instance_rdh;

	// tm
	RenderTransform* tm = &rdh->tm;
	qm_rst(&tm->frm);
	for (size_t i = 0; i < QN_COUNTOF(tm->tex); i++)
		qm_rst(&tm->tex[i]);

	// param
	RenderParam* param = &rdh->param;
	param->bone_ptr = NULL;
	param->bone_count = 0;
	for (size_t i = 0; i < QN_COUNTOF(param->v); i++)
		qm_rst(&param->v[i]);
	for (size_t i = 0; i < QN_COUNTOF(param->m); i++)
		qm_rst(&param->m[i]);
	param->bgc = qm_color(0.0f, 0.0f, 0.0f, 1.0f);
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
	qs_cast_vt(rdh, RDHBASE)->layout();
}

//
void rdh_internal_add_node(RenderNodeShed shed, void* node)
{
	qn_assert((size_t)shed < RDHNODE_MAX_VALUE, "invalid rdh node");
	RdhBase* rdh = qg_instance_rdh;
	qg_node_mukum_set(&rdh->mukums[shed], node);
}

//
void rdh_internal_unlink_node(RenderNodeShed shed, void* node)
{
	qn_assert((size_t)shed < RDHNODE_MAX_VALUE, "invalid rdh node");
	RdhBase* rdh = qg_instance_rdh;
	qg_node_mukum_unlink(&rdh->mukums[shed], node);
}

//
void qg_rdh_set_shader_var_callback(QgVarShaderFunc func, void* data)
{
	RenderParam* param = RDH_PARAM;
	param->callback_func = func;
	param->callback_data = data;
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
void qg_rdh_end(bool flush)
{
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.ends++;
	rdh->invokes.flush = true;
	qs_cast_vt(rdh, RDHBASE)->end();
	if (flush)
		qs_cast_vt(rdh, RDHBASE)->flush();
}

//
void qg_rdh_flush(void)
{
	RdhBase* rdh = qg_instance_rdh;
	if (!rdh->invokes.flush)
	{
		qn_debug_outputs(true, "RDH", "call end() before flush");
		qg_rdh_end(false);
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
void qg_rdh_clear(QgClear clear)
{
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	qs_cast_vt(rdh, RDHBASE)->clear(clear);
}

//
void qg_rdh_set_param_vec3(int at, const QmVec3* v)
{
	RdhBase* rdh = qg_instance_rdh;
	RDH_ACK_NULL(v, );
	RDH_ACK_MAX(at, QN_COUNTOF(rdh->param.v), );
	rdh->param.v[at] = qm_vec4v(*v, 0.0f);
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_vec4(int at, const QmVec4* v)
{
	RdhBase* rdh = qg_instance_rdh;
	RDH_ACK_NULL(v, );
	RDH_ACK_MAX(at, QN_COUNTOF(rdh->param.v), );
	rdh->param.v[at] = *v;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_mat4(int at, const QmMat4* m)
{
	RdhBase* rdh = qg_instance_rdh;
	RDH_ACK_NULL(m, );
	RDH_ACK_MAX(at, QN_COUNTOF(rdh->param.m), );
	rdh->param.m[at] = *m;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_weight(int count, QmMat4* weight)
{
	RDH_ACK_MIN(count, 0, );
	RDH_ACK_NULL(weight, );
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
	RDH_ACK_NULL(world, );
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.world = *world;
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_view(const QmMat4* view)
{
	RDH_ACK_NULL(view, );
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.view = *view;
	rdh->tm.invv = qm_inv(*view);
	rdh->tm.view_proj = qm_mul(*view, rdh->tm.proj);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_project(const QmMat4* proj)
{
	RDH_ACK_NULL(proj, );
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.proj = *proj;
	rdh->tm.view_proj = qm_mul(rdh->tm.view, *proj);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_view_project(const QmMat4* proj, const QmMat4* view)
{
	RDH_ACK_NULL(proj, );
	RDH_ACK_NULL(view, );
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.proj = *proj;
	rdh->tm.view = *view;
	rdh->tm.invv = qm_inv(*view);
	rdh->tm.view_proj = qm_mul(*proj, *view);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
QgBuffer* qg_rdh_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	RDH_ACK_ZERO(count, NULL);
	RDH_ACK_COND(type == QGBUFFER_INDEX && stride != 2 && stride != 4, "invalid index buffer stride. require 2 or 4", NULL);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->create_buffer(type, count, stride, initial_data);
}

//
QgRender* qg_rdh_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader)
{
	RDH_ACK_NULL(prop, NULL);
	RDH_ACK_MAX3(prop, rasterizer, fill, QGFILL_MAX_VALUE, NULL);
	RDH_ACK_MAX3(prop, rasterizer, cull, QGCULL_MAX_VALUE, NULL);
	RDH_ACK_MAX2(prop, depth, QGDEPTH_MAX_VALUE, NULL);
	RDH_ACK_MAX2(prop, stencil, QGSTENCIL_MAX_VALUE, NULL);
	RDH_ACK_ZERO3(prop, format, count, NULL);
	RDH_ACK_MAX3(prop, format, count, QGRVS_MAX_VALUE, NULL);
	RDH_ACK_MAX2(prop, topology, QGTPG_MAX_VALUE, NULL);

	RDH_ACK_NULL(shader, NULL);
	RDH_ACK_NULL3(shader, layout, inputs, NULL);
	RDH_ACK_ZERO3(shader, layout, count, NULL);
	RDH_ACK_NULL3(shader, vertex, code, NULL);
	RDH_ACK_NULL3(shader, pixel, code, NULL);

	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->create_render(name, prop, shader);
}

//
bool qg_rdh_set_index(QgBuffer * buffer)
{
	RDH_ACK_NULL(buffer, false);
	RDH_ACK_COND(buffer->type != QGBUFFER_INDEX, "cannot set non-index buffer as index buffer", false);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_index(buffer);
}

//
bool qg_rdh_set_vertex(QgLayoutStage stage, QgBuffer * buffer)
{
	RDH_ACK_NULL(buffer, false);
	RDH_ACK_COND(buffer->type != QGBUFFER_INDEX, "cannot set non-vertex buffer as vertex buffer", false);
	RDH_ACK_MAX(stage, QGLOS_MAX_VALUE, false);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_vertex(stage, buffer);
}

//
bool qg_rdh_set_render(QgRender * render)
{
	RDH_ACK_NULL(render, false);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_render(render);
}

//
bool qg_rdh_draw(QgTopology tpg, int vertices)
{
	RDH_ACK_MAX(tpg, QGTPG_MAX_VALUE, false);
	RDH_ACK_MIN(vertices, 0, false);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->draw(tpg, vertices);
}

//
bool qg_rdh_draw_indexed(QgTopology tpg, int indices)
{
	RDH_ACK_MAX(tpg, QGTPG_MAX_VALUE, false);
	RDH_ACK_MIN(indices, 0, false);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->draw_indexed(tpg, indices);
}


//////////////////////////////////////////////////////////////////////////
// 오브젝트

//
void qg_node_set_name(QgNode * self, const char* name)
{
	if (name)
		qn_strncpy(self->NAME, name, QN_COUNTOF(self->NAME) - 1);
	else
	{
		size_t i = qn_p_index();
		qn_snprintf(self->NAME, QN_COUNTOF(self->NAME), "node_%zu", i);
	}
	self->HASH = qn_strihash(self->NAME);
}

//
void* qg_buffer_map(QgBuffer * self)
{
	qn_val_if_fail(self->mapped == false, NULL);
	return qs_cast_vt(self, QGBUFFER)->map(self);
}

//
bool qg_buffer_unmap(QgBuffer * self)
{
	qn_val_if_fail(self->mapped != false, false);
	return qs_cast_vt(self, QGBUFFER)->unmap(self);
}

//
bool qg_buffer_data(QgBuffer * self, const void* data)
{
	qn_val_if_fail(self->mapped == false, false);
	qn_val_if_fail(data, false);
	return qs_cast_vt(self, QGBUFFER)->data(self, data);
}
