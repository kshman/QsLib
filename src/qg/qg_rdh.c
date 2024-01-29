#include "pch.h"
#include "qg_stub.h"
#include <qs_supp.h>

#ifdef _MSC_VER
#pragma warning(disable:6011)
#endif

//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

// 활성 렌더러
RdhBase* RDH = NULL;

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"RDH"

//
bool qg_open_rdh(const char* driver, const char* title, int display, int width, int height, int flags, int features)
{
	qg_init_converters();

	struct rdh_renderer
	{
		const char* name;
		const char* alias;
		RdhBase* (*allocator)(QgFlag, QgFeature);
		QgFlag flag;
		QgFeature feature;
	};
	static struct rdh_renderer renderers[] =
	{
#if defined USE_GL
		{ "GL", "OPENGL", qgl_allocator, QGSPECIFIC_CORE, QGRENDERER_OPENGL },
		{ "ES", "OPENGLES", qgl_allocator, QGFLAG_NONE, QGRENDERER_OPENGL },
#endif
#if defined USE_GL
		{ NULL, NULL, qgl_allocator, QGFLAG_NONE, QGRENDERER_OPENGL },
#else
		{ NULL, NULL, NULL, 0 },
#endif
	};
	struct rdh_renderer* renderer = &renderers[QN_COUNTOF(renderers) - 1];
	if (driver != NULL && *driver != '\0')
	{
		for (size_t i = 0; i < QN_COUNTOF(renderers) - 1; i++)
			if (qn_strieqv(renderers[i].name, driver) || qn_strieqv(renderers[i].alias, driver))
			{
				renderer = &renderers[i];
				break;
			}
	}

	VAR_CHK_IF_COND(renderer->allocator == NULL, "no valid renderer found", false);
	flags |= renderer->flag;
	features |= renderer->feature;

	bool stub_created;
	if (STUB != NULL)
		stub_created = false;
	else
	{
		if (qg_open_stub(title, display, width, height, flags | QGSPECIFIC_RDHSTUB, features) == false)
			return false;
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
	RDH = rdh;

	// 한번 설정하면 바뀔일이 없거나 신경 안써도 되는것
	RenderTransform* tm = &rdh->tm;
	tm->world = qm_mat4_unit();
	tm->view = qm_mat4_unit();
	tm->invv = qm_mat4_unit();
	tm->frm = qm_mat4_unit();
	for (size_t i = 0; i < QN_COUNTOF(tm->tex); i++)		// 텍스쳐 행렬
		tm->tex[i] = qm_mat4_unit();
	tm->Near = 1.0f;										// Z깊이
	tm->Far = 100000.0f;

	RenderParam* param = &rdh->param;
	for (size_t i = 0; i < QN_COUNTOF(param->v); i++)		// 벡터 인수
		param->v[i] = qm_vec_zero();
	for (size_t i = 0; i < QN_COUNTOF(param->m); i++)		// 행렬 인수
		param->m[i] = qm_mat4_unit();
	param->bgc = qm_color(0.0f, 0.0f, 0.0f, 1.0f);			// 배경색

	// 묶음
	for (size_t i = 0; i < QN_COUNTOF(rdh->mukums); i++)
		qg_node_mukum_init_fast(&rdh->mukums[i]);

	// 
	qs_cast_vt(rdh, RDHBASE)->layout();						// 레이아웃 재설정
	qs_cast_vt(rdh, RDHBASE)->reset();						// 장치 리셋
	qn_timer_reset(STUB->timer);							// 타이머도 리셋해둔다
	return true;
}

//
void qg_close_rdh(void)
{
	RDH = qs_unloadc(RDH, RdhBase);
}

//
void rdh_internal_clean(void)
{
	RdhBase* rdh = RDH;
	for (size_t i = 0; i < QN_COUNTOF(rdh->mukums); i++)
		qg_node_mukum_disp_safe(&rdh->mukums[i]);
}

//
void rdh_internal_dispose(void)
{
	VAR_CHK_IF_COND(STUB == NULL, "스터브가 없어요", );
	VAR_CHK_IF_COND(RDH == NULL, "렌더러가 없어요", );

	RdhBase* rdh = RDH;
	qn_free(rdh);

	if (QN_TMASK(STUB->flags, QGSPECIFIC_RDHSTUB))
		qg_close_stub();
}

//
void rdh_internal_layout(void)
{
	const QmSize client_size = STUB->client_size;
	const float aspect = qm_size_get_aspect(client_size);

	// tm
	RenderTransform* tm = RDH_TRANSFORM;
	tm->size = client_size;
	tm->proj = qm_mat4_perspective_lh(QM_PI_H, aspect, tm->Near, tm->Far);
	tm->view_proj = qm_mat4_mul(tm->view, tm->proj);
	tm->scissor = qm_rect_size(0, 0, client_size.Width, client_size.Height);
}

//
void rdh_internal_reset(void)
{
	// param
	RenderParam* param = RDH_PARAM;
	param->bone_ptr = NULL;									// 뼈대 포인터는 메모리 안전을 위해 초기화
	param->bone_count = 0;									// 뼈대 가중치 개수도 초기화
}

//
void rdh_internal_invoke_reset(void)
{
	RdhBase* rdh = RDH;
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
	RdhBase* rdh = RDH;
	const QmSize size = STUB->client_size;
	const int width = (int)rdh->tm.size.Width;
	const int height = (int)rdh->tm.size.Height;
	if (size.Width == width && size.Height == height)
		return;
	qs_cast_vt(rdh, RDHBASE)->layout();
}

//
void rdh_internal_add_node(RenderNodeShed shed, void* node)
{
	VAR_CHK_IF_MAX(shed, RDHNODE_MAX_VALUE, );
	RdhBase* rdh = RDH;
	qg_node_mukum_set(&rdh->mukums[shed], qs_loadu(node, QgNode));
}

//
void rdh_internal_unlink_node(RenderNodeShed shed, void* node)
{
	VAR_CHK_IF_MAX(shed, RDHNODE_MAX_VALUE, );
	RdhBase* rdh = RDH;
	qg_node_mukum_unlink(&rdh->mukums[shed], qs_cast_type(node, QgNode));
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
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.begins++;
	rdh->invokes.flush = false;
	return qs_cast_vt(rdh, RDHBASE)->begin(clear);
}

//
void qg_rdh_end(bool flush)
{
	RdhBase* rdh = RDH;
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
	RdhBase* rdh = RDH;
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
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	qs_cast_vt(rdh, RDHBASE)->reset();
}

//
void qg_rdh_clear(QgClear clear)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	qs_cast_vt(rdh, RDHBASE)->clear(clear);
}

//
void qg_rdh_set_param_vec4(int at, const QmVec4* v)
{
	RdhBase* rdh = RDH;
	VAR_CHK_IF_NULL(v, );
	VAR_CHK_IF_MAX(at, QN_COUNTOF(rdh->param.v), );
	rdh->param.v[at] = *v;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_mat4(int at, const QmMat4* m)
{
	RdhBase* rdh = RDH;
	VAR_CHK_IF_NULL(m, );
	VAR_CHK_IF_MAX(at, QN_COUNTOF(rdh->param.m), );
	rdh->param.m[at] = *m;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_weight(int count, QmMat4* weight)
{
	VAR_CHK_IF_MIN(count, 0, );
	VAR_CHK_IF_NULL(weight, );
	RdhBase* rdh = RDH;
	rdh->param.bone_count = count;
	rdh->param.bone_ptr = weight;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_background(const QmVec4* color)
{
	RdhBase* rdh = RDH;
	if (color)
		rdh->param.bgc = *color;
	else
		rdh->param.bgc = qm_color(0.0f, 0.0f, 0.0f, 1.0f);
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_world(const QmMat4* world)
{
	VAR_CHK_IF_NULL(world, );
	RdhBase* rdh = RDH;
	rdh->tm.world = *world;
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_view(const QmMat4* view)
{
	VAR_CHK_IF_NULL(view, );
	RdhBase* rdh = RDH;
	rdh->tm.view = *view;
	rdh->tm.invv = qm_mat4_inv(*view);
	rdh->tm.view_proj = qm_mat4_mul(*view, rdh->tm.proj);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_project(const QmMat4* proj)
{
	VAR_CHK_IF_NULL(proj, );
	RdhBase* rdh = RDH;
	rdh->tm.proj = *proj;
	rdh->tm.view_proj = qm_mat4_mul(rdh->tm.view, *proj);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_view_project(const QmMat4* proj, const QmMat4* view)
{
	VAR_CHK_IF_NULL(proj, );
	VAR_CHK_IF_NULL(view, );
	RdhBase* rdh = RDH;
	rdh->tm.proj = *proj;
	rdh->tm.view = *view;
	rdh->tm.invv = qm_mat4_inv(*view);
	rdh->tm.view_proj = qm_mat4_mul(*proj, *view);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
QgBuffer* qg_rdh_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	VAR_CHK_IF_ZERO(count, NULL);
	VAR_CHK_IF_MIN(stride, 1, NULL);
	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->create_buffer(type, count, stride, initial_data);
}

//
QgRender* qg_rdh_create_render(const char* name, const QgPropRender* render, const QgPropShader* shader)
{
	VAR_CHK_IF_NULL(render, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, fill, QGFILL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, cull, QGCULL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, depth, QGDEPTH_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, stencil, QGSTENCIL_MAX_VALUE, NULL);
	VAR_CHK_IF_ZERO3(render, format, count, NULL);
	VAR_CHK_IF_MAX3(render, format, count, QGRVS_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, topology, QGTPG_MAX_VALUE, NULL);

	VAR_CHK_IF_NULL(shader, NULL);
	VAR_CHK_IF_NULL3(shader, layout, inputs, NULL);
	VAR_CHK_IF_ZERO3(shader, layout, count, NULL);
	VAR_CHK_IF_NULL3(shader, vertex, code, NULL);
	VAR_CHK_IF_NULL3(shader, pixel, code, NULL);

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->create_render(name, render, shader);
}

//
bool qg_rdh_set_index(QgBuffer* buffer)
{
	VAR_CHK_IF_NULL(buffer, false);
	VAR_CHK_IF_COND(buffer->type != QGBUFFER_INDEX, "cannot set non-index buffer as index buffer", false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_index(buffer);
}

//
bool qg_rdh_set_vertex(QgLayoutStage stage, QgBuffer* buffer)
{
	VAR_CHK_IF_NULL(buffer, false);
	VAR_CHK_IF_COND(buffer->type != QGBUFFER_VERTEX, "cannot set non-vertex buffer as vertex buffer", false);
	VAR_CHK_IF_MAX(stage, QGLOS_MAX_VALUE, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_vertex(stage, buffer);
}

//
bool qg_rdh_set_render(QgRender * render)
{
	VAR_CHK_IF_NULL(render, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_render(render);
}

//
bool qg_rdh_set_render_named(const char* name)
{
	VAR_CHK_IF_NULL(name, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	QgRender* rdr = qg_node_mukum_get(&rdh->mukums[RDHNODE_RENDER], name);
	return rdr == NULL ? false : qs_cast_vt(rdh, RDHBASE)->set_render(rdr);
}

//
bool qg_rdh_draw(QgTopology tpg, int vertices)
{
	VAR_CHK_IF_MAX(tpg, QGTPG_MAX_VALUE, false);
	VAR_CHK_IF_MIN(vertices, 0, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->draw(tpg, vertices);
}

//
bool qg_rdh_draw_indexed(QgTopology tpg, int indices)
{
	VAR_CHK_IF_MAX(tpg, QGTPG_MAX_VALUE, false);
	VAR_CHK_IF_MIN(indices, 0, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->draw_indexed(tpg, indices);
}


//////////////////////////////////////////////////////////////////////////
// 노드

//
void qg_node_set_name(QgNode * self, const char* name)
{
	if (name)
	{
		qn_strncpy(self->NAME, name, QN_COUNTOF(self->NAME) - 1);
		self->HASH = qn_strhash(self->NAME);
	}
	else
	{
		size_t i = qn_p_index();
		qn_snprintf(self->NAME, QN_COUNTOF(self->NAME), "node_%zu", i);
		// 이름 없는 노드는 관리하지 않으므로 해시가 없다
		self->HASH = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"BUFFER"

//
void* qg_buffer_map(QgBuffer * self)
{
	VAR_CHK_IF_COND(self->mapped != false, "buffer already mapped", NULL);
	return qs_cast_vt(self, QGBUFFER)->map(self);
}

//
bool qg_buffer_unmap(QgBuffer * self)
{
	VAR_CHK_IF_COND(self->mapped == false, "buffer not mapped", false);
	return qs_cast_vt(self, QGBUFFER)->unmap(self);
}

//
bool qg_buffer_data(QgBuffer * self, const void* data)
{
	VAR_CHK_IF_COND(self->mapped != false, "buffer already mapped", false);
	VAR_CHK_IF_NULL(data, false);
	return qs_cast_vt(self, QGBUFFER)->data(self, data);
}
