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
	RendererTransform* tm = &rdh->tm;
	tm->world = qm_mat4_unit();
	tm->view = qm_mat4_unit();
	tm->invv = qm_mat4_unit();
	tm->frm = qm_mat4_unit();
	for (size_t i = 0; i < QN_COUNTOF(tm->tex); i++)		// 텍스쳐 행렬
		tm->tex[i] = qm_mat4_unit();
	tm->Near = 1.0f;										// Z깊이
	tm->Far = 100000.0f;

	RendererParam* param = &rdh->param;
	for (size_t i = 0; i < QN_COUNTOF(param->v); i++)		// 벡터 인수
		param->v[i] = qm_vec_zero();
	for (size_t i = 0; i < QN_COUNTOF(param->m); i++)		// 행렬 인수
		param->m[i] = qm_mat4_unit();
	param->bgc = qm_color(0.0f, 0.0f, 0.0f, 1.0f);			// 배경색

	// 묶음
	for (size_t i = 0; i < QN_COUNTOF(rdh->mukums); i++)
		qn_node_mukum_init_fast(&rdh->mukums[i]);

	// 
	qn_cast_vtable(rdh, RDHBASE)->layout();						// 레이아웃 재설정
	qn_cast_vtable(rdh, RDHBASE)->reset();						// 장치 리셋
	qn_timer_reset(STUB->timer);							// 타이머도 리셋해둔다
	return true;
}

//
void qg_close_rdh(void)
{
	RDH = qn_unloadc(RDH);
}

//
void rdh_internal_clean(void)
{
	RdhBase* rdh = RDH;
	for (size_t i = 0; i < QN_COUNTOF(rdh->mukums); i++)
		qn_node_mukum_safe_dispose(&rdh->mukums[i]);
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
	RendererTransform* tm = RDH_TRANSFORM;
	tm->size = client_size;
	tm->proj = qm_mat4_perspective_lh(QM_PI_H, aspect, tm->Near, tm->Far);
	tm->view_proj = qm_mat4_mul(tm->view, tm->proj);
	tm->scissor = qm_rect_size(0, 0, client_size.Width, client_size.Height);
}

//
void rdh_internal_reset(void)
{
	// param
	RendererParam* param = RDH_PARAM;
	param->bone_ptr = NULL;									// 뼈대 포인터는 메모리 안전을 위해 초기화
	param->bone_count = 0;									// 뼈대 가중치 개수도 초기화
}

//
void rdh_internal_invoke_reset(void)
{
	RdhBase* rdh = RDH;
	RendererInvoke* ivk = &rdh->invokes;
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
	qn_cast_vtable(rdh, RDHBASE)->layout();
}

//
void rdh_internal_add_node(RenderNodeShed shed, void* node)
{
	VAR_CHK_IF_MAX(shed, RDHNODE_MAX_VALUE, );
	RdhBase* rdh = RDH;
	qn_node_mukum_set(&rdh->mukums[shed], qn_loadu(node, QnGamNode));
}

//
void rdh_internal_unlink_node(RenderNodeShed shed, void* node)
{
	VAR_CHK_IF_MAX(shed, RDHNODE_MAX_VALUE, );
	RdhBase* rdh = RDH;
	qn_node_mukum_unlink(&rdh->mukums[shed], qn_cast_type(node, QnGamNode));
}

//
void qg_set_shader_var_callback(QgVarShaderFunc func, void* data)
{
	RendererParam* param = RDH_PARAM;
	param->callback_func = func;
	param->callback_data = data;
}

//
bool qg_begin_render(bool clear)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.begins++;
	rdh->invokes.flush = false;
	return qn_cast_vtable(rdh, RDHBASE)->begin(clear);
}

//
void qg_end_render(bool flush)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.ends++;
	rdh->invokes.flush = true;
	qn_cast_vtable(rdh, RDHBASE)->end();
	if (flush)
		qn_cast_vtable(rdh, RDHBASE)->flush();
}

//
void qg_flush(void)
{
	RdhBase* rdh = RDH;
	if (!rdh->invokes.flush)
	{
		qn_mesg(true, "RDH", "call end() before flush");
		qg_end_render(false);
	}
	qn_cast_vtable(rdh, RDHBASE)->flush();
	rdh->invokes.invokes++;
	rdh->invokes.frames++;
}

//
void qg_rdh_reset(void)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	qn_cast_vtable(rdh, RDHBASE)->reset();
}

//
void qg_clear_render(QgClear clear)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	qn_cast_vtable(rdh, RDHBASE)->clear(clear);
}

//
void qg_set_param_vec4(int at, const QmVec4* v)
{
	RdhBase* rdh = RDH;
	VAR_CHK_IF_NULL(v, );
	VAR_CHK_IF_MAX(at, QN_COUNTOF(rdh->param.v), );
	rdh->param.v[at] = *v;
	rdh->invokes.invokes++;
}

//
void qg_set_param_mat4(int at, const QmMat4* m)
{
	RdhBase* rdh = RDH;
	VAR_CHK_IF_NULL(m, );
	VAR_CHK_IF_MAX(at, QN_COUNTOF(rdh->param.m), );
	rdh->param.m[at] = *m;
	rdh->invokes.invokes++;
}

//
void qg_set_param_weight(int count, QmMat4* weight)
{
	VAR_CHK_IF_MIN(count, 0, );
	VAR_CHK_IF_NULL(weight, );
	RdhBase* rdh = RDH;
	rdh->param.bone_count = count;
	rdh->param.bone_ptr = weight;
	rdh->invokes.invokes++;
}

//
void qg_set_background(const QmColor* color)
{
	RdhBase* rdh = RDH;
	if (color)
		rdh->param.bgc = *color;
	else
		rdh->param.bgc = qm_color(0.0f, 0.0f, 0.0f, 1.0f);
	rdh->invokes.invokes++;
}

//
void qg_set_world(const QmMat4* world)
{
	VAR_CHK_IF_NULL(world, );
	RdhBase* rdh = RDH;
	rdh->tm.world = *world;
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_set_view(const QmMat4* view)
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
void qg_set_project(const QmMat4* proj)
{
	VAR_CHK_IF_NULL(proj, );
	RdhBase* rdh = RDH;
	rdh->tm.proj = *proj;
	rdh->tm.view_proj = qm_mat4_mul(rdh->tm.view, *proj);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_set_view_project(const QmMat4* proj, const QmMat4* view)
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
QgBuffer* qg_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	VAR_CHK_IF_ZERO(count, NULL);
	VAR_CHK_IF_MIN(stride, 1, NULL);
	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDHBASE)->create_buffer(type, count, stride, initial_data);
}

//
QgRenderState* qg_create_render_state(const char* name, const QgPropRender* render, const QgPropShader* shader)
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
	return qn_cast_vtable(rdh, RDHBASE)->create_render(name, render, shader);
}

//
QgTexture* qg_create_texture(const char* name, const QgImage* image, QgTexFlag flags)
{
	VAR_CHK_IF_NULL(image, NULL);
	VAR_CHK_IF_NULL2(image, data, NULL);

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDHBASE)->create_texture(name, image, flags);
}

//
QgTexture* qg_load_texture(int mount, const char* filename, QgTexFlag flags)
{
	VAR_CHK_IF_NULL(filename, NULL);

	QgImage* image = qg_load_image(mount, filename);
	VAR_CHK_IF_NULL(image, NULL);
	VAR_CHK_IF_NULL2(image, data, NULL);

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDHBASE)->create_texture(filename, image, flags | QGTEXF_DISCARD_IMAGE);
}

//
bool qg_set_index(QgBuffer* buffer)
{
	VAR_CHK_IF_NULL(buffer, false);
	VAR_CHK_IF_COND(buffer->type != QGBUFFER_INDEX, "cannot set non-index buffer as index buffer", false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDHBASE)->set_index(buffer);
}

//
bool qg_set_vertex(QgLayoutStage stage, QgBuffer* buffer)
{
	VAR_CHK_IF_NULL(buffer, false);
	VAR_CHK_IF_COND(buffer->type != QGBUFFER_VERTEX, "cannot set non-vertex buffer as vertex buffer", false);
	VAR_CHK_IF_MAX(stage, QGLOS_MAX_VALUE, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDHBASE)->set_vertex(stage, buffer);
}

//
bool qg_set_render_state(QgRenderState * render)
{
	VAR_CHK_IF_NULL(render, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDHBASE)->set_render(render);
}

//
bool qg_set_render_named(const char* name)
{
	VAR_CHK_IF_NULL(name, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	QgRenderState* rdr = qn_node_mukum_get(&rdh->mukums[RDHNODE_RENDER], name);
	return rdr == NULL ? false : qn_cast_vtable(rdh, RDHBASE)->set_render(rdr);
}

//
bool qg_set_texture(int stage, QgTexture* texture)
{
	VAR_CHK_IF_MAX(stage, 8/*RDH_INFO->max_tex_count*/, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDHBASE)->set_texture(stage, texture);
}

//
bool qg_draw(QgTopology tpg, int vertices)
{
	VAR_CHK_IF_MAX(tpg, QGTPG_MAX_VALUE, false);
	VAR_CHK_IF_MIN(vertices, 0, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qn_cast_vtable(rdh, RDHBASE)->draw(tpg, vertices);
}

//
bool qg_draw_indexed(QgTopology tpg, int indices)
{
	VAR_CHK_IF_MAX(tpg, QGTPG_MAX_VALUE, false);
	VAR_CHK_IF_MIN(indices, 0, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qn_cast_vtable(rdh, RDHBASE)->draw_indexed(tpg, indices);
}

//
void qg_draw_sprite(const QmRect* bound, QgTexture* texture, const QmColor* color, const QmVec* coord)
{
	VAR_CHK_IF_NULL(bound, );
	if (color == NULL)
		color = &QMCOLOR_WHITE;
	if (coord == NULL)
		coord = &QMCONST_00ZW;
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	qn_cast_vtable(rdh, RDHBASE)->draw_sprite(bound, texture, color, coord);
}

//
void qg_draw_sprite_ex(const QmRect* bound, float angle, QgTexture* texture, const QmColor* color, const QmVec* coord)
{
	VAR_CHK_IF_NULL(bound, );
	if (color == NULL)
		color = &QMCOLOR_WHITE;
	if (coord == NULL)
		coord = &QMCONST_00ZW;
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	qn_cast_vtable(rdh, RDHBASE)->draw_sprite_ex(bound, angle, texture, color, coord);
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"BUFFER"

//
void* qg_buffer_map(QgBuffer * self)
{
	VAR_CHK_IF_COND(self->mapped != false, "buffer already mapped", NULL);
	return qn_cast_vtable(self, QGBUFFER)->map(self);
}

//
bool qg_buffer_unmap(QgBuffer * self)
{
	VAR_CHK_IF_COND(self->mapped == false, "buffer not mapped", false);
	return qn_cast_vtable(self, QGBUFFER)->unmap(self);
}

//
bool qg_buffer_data(QgBuffer * self, int size, const void* data)
{
	VAR_CHK_IF_COND(self->mapped != false, "buffer already mapped", false);
	VAR_CHK_IF_NULL(data, false);
	return qn_cast_vtable(self, QGBUFFER)->data(self, size, data);
}
