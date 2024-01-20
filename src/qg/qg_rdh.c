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

// 인수 검사 매크로
#define RDH_ARG_CHK(cond,mesg)				QN_STMT_BEGIN{if (cond) { qn_debug_outputs(true, "RDH", mesg); return; }}QN_STMT_END
#define RDH_ARG_CHK_NULL(cond,mesg)			QN_STMT_BEGIN{if (cond) { qn_debug_outputs(true, "RDH", mesg); return NULL; }}QN_STMT_END
#define RDH_ARG_CHK_FALSE(cond,mesg)		QN_STMT_BEGIN{if (cond) { qn_debug_outputs(true, "RDH", mesg); return false; }}QN_STMT_END
#define RDH_ARG_CHKV_NULL(cond,mesg,...)	QN_STMT_BEGIN{if (cond) { qn_debug_outputf(true, "RDH", mesg, __VA_ARGS__); return NULL; }}QN_STMT_END
#define RDH_ARG_CHKV_FALSE(cond,mesg,...)	QN_STMT_BEGIN{if (cond) { qn_debug_outputf(true, "RDH", mesg, __VA_ARGS__); return false; }}QN_STMT_END

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
	RDH_ARG_CHK(v != NULL, "parameter is null");
	RDH_ARG_CHK((size_t)at >= QN_COUNTOF(rdh->param.v), "invalid parameter index");
	rdh->param.v[at] = qm_vec4v(*v, 0.0f);
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_vec4(int at, const QmVec4* v)
{
	RdhBase* rdh = qg_instance_rdh;
	RDH_ARG_CHK(v != NULL, "parameter is null");
	RDH_ARG_CHK((size_t)at >= QN_COUNTOF(rdh->param.v), "invalid parameter index");
	rdh->param.v[at] = *v;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_mat4(int at, const QmMat4* m)
{
	RdhBase* rdh = qg_instance_rdh;
	RDH_ARG_CHK(m != NULL, "parameter is null");
	RDH_ARG_CHK((size_t)at >= QN_COUNTOF(rdh->param.m), "invalid parameter index");
	rdh->param.m[at] = *m;
	rdh->invokes.invokes++;
}

//
void qg_rdh_set_param_weight(int count, QmMat4* weight)
{
	RDH_ARG_CHK(count > 0, "invalid bone count");
	RDH_ARG_CHK(weight != NULL, "bone weight is null");
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
	RDH_ARG_CHK(world != NULL, "world matrix is null");
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.world = *world;
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_view(const QmMat4* view)
{
	RDH_ARG_CHK(view != NULL, "view matrix is null");
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
	RDH_ARG_CHK(proj != NULL, "project matrix is null");
	RdhBase* rdh = qg_instance_rdh;
	rdh->tm.proj = *proj;
	rdh->tm.view_proj = qm_mul(rdh->tm.view, *proj);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_rdh_set_view_project(const QmMat4* proj, const QmMat4* view)
{
	RDH_ARG_CHK(proj == NULL, "project matrix is null");
	RDH_ARG_CHK(view == NULL, "view matrix is null");
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
	RDH_ARG_CHKV_NULL(count == 0, "invalid buffer count: %d", count);
	RDH_ARG_CHKV_NULL(type == QGBUFFER_INDEX && stride != 2 && stride != 4, "invalid index buffer stride: %d, require 2 or 4", stride);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->create_buffer(type, count, stride, initial_data);
}

//
QgRender* qg_rdh_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader)
{
	RDH_ARG_CHK_NULL(prop == NULL, "pipeline property is null");
	RDH_ARG_CHK_NULL(shader == NULL, "pipeline shader is null");

#define RDH_CHK_NULL(sec,item)				if (prop->sec.item == NULL)\
		{ qn_debug_outputf(true, "RDH", "'%s.%s' has no data", #sec, #item); return NULL; } (void)NULL
#define RDH_CHK_RANGE_MAX1(item,vmax)		if ((size_t)prop->item >= (vmax))\
		{ qn_debug_outputf(true, "RDH", "invalid '%s' value: %d", #item, prop->item); return NULL; } (void)NULL
#define RDH_CHK_RANGE_MAX2(sec,item,vmax)	if ((size_t)prop->sec.item >= (vmax))\
		{ qn_debug_outputf(true, "RDH", "invalid '%s.%s' value: %d", #sec, #item, prop->sec.item); return NULL; } (void)NULL
#define RDH_CHK_RANGE_MIN(sec,item,value)	if ((size_t)prop->sec.item <= (value))\
		{ qn_debug_outputf(true, "RDH", "invalid '%s.%s' value: %d", #sec, #item, prop->sec.item); return NULL; } (void)NULL

	// 래스터라이저
	RDH_CHK_RANGE_MAX2(rasterizer, fill, QGFILL_MAX_VALUE);
	RDH_CHK_RANGE_MAX2(rasterizer, cull, QGCULL_MAX_VALUE);
	// 뎁스
	RDH_CHK_RANGE_MAX1(depth, QGDEPTH_MAX_VALUE);
	// 스텐실
	RDH_CHK_RANGE_MAX1(stencil, QGSTENCIL_MAX_VALUE);
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
	return qs_cast_vt(rdh, RDHBASE)->create_render(name, prop, shader);
}

//
bool qg_rdh_set_index(QgBuffer * buffer)
{
	RDH_ARG_CHK_FALSE(buffer == NULL, "cannot set null index buffer");
	RDH_ARG_CHK_FALSE(buffer->type != QGBUFFER_INDEX, "cannot set non-index buffer as index buffer");
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_index(buffer);
}

//
bool qg_rdh_set_vertex(QgLayoutStage stage, QgBuffer * buffer)
{
	RDH_ARG_CHK_FALSE(buffer == NULL, "cannot set null vertex buffer");
	RDH_ARG_CHK_FALSE(buffer->type != QGBUFFER_VERTEX, "cannot set non-vertex buffer as vertex buffer");
	RDH_ARG_CHKV_FALSE((size_t)stage >= QGLOS_MAX_VALUE, "invalid vertex layout stage: %d (max: %d)", stage, QGLOS_MAX_VALUE - 1);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_vertex(stage, buffer);
}

//
bool qg_rdh_set_render(QgRender * render)
{
	RDH_ARG_CHK_FALSE(render == NULL, "cannot set null render");
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	return qs_cast_vt(rdh, RDHBASE)->set_render(render);
}

//
bool qg_rdh_draw(QgTopology tpg, int vertices)
{
	RDH_ARG_CHKV_FALSE((size_t)tpg >= QGTPG_MAX_VALUE, "invalid topology: %d", tpg);
	RDH_ARG_CHKV_FALSE(vertices <= 0, "invalid vertices: %d", vertices);
	RdhBase* rdh = qg_instance_rdh;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qs_cast_vt(rdh, RDHBASE)->draw(tpg, vertices);
}

//
bool qg_rdh_draw_indexed(QgTopology tpg, int indices)
{
	RDH_ARG_CHKV_FALSE((size_t)tpg >= QGTPG_MAX_VALUE, "invalid topology: %d", tpg);
	RDH_ARG_CHKV_FALSE(indices <= 0, "invalid indices: %d", indices);
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

//
bool qg_shader_bind(QgShader * self, QgShader * shader)
{
	qn_val_if_fail(shader != NULL, false);
	return qs_cast_vt(self, QGSHADER)->bind_shader(self, shader);
}

//
bool qg_shader_bind_buffer(QgShader * self, QgShaderType type, const QgCodeData* code, QgScFlag flags)
{
	qn_val_if_fail(QN_TMASK(type, QGSHADER_ALL), false);
	qn_val_if_fail(code != NULL && code->code != NULL && code->size > 0, false);
	return qs_cast_vt(self, QGSHADER)->bind_buffer(self, type, code->code, code->size, flags);
}

//
bool qg_shader_bind_file(QgShader * self, QgShaderType type, const char* filename, int flags)
{
	qn_val_if_fail(QN_TMASK(type, QGSHADER_ALL), false);
	qn_val_if_fail(filename != NULL, false);

	int size;
	void* data = qn_file_alloc(filename, &size);
	qn_val_if_fail(data != NULL, false);
	bool ret = qs_cast_vt(self, QGSHADER)->bind_buffer(self, type, data, (uint)size, flags);

	qn_free(data);
	return ret;
}
