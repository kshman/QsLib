﻿//
// qgrdh_qgl.c - GL 공통 디바이스
// 2024-01-26 by kim
//

#include "pch.h"
#if defined USE_GL
#include "qgrdh_qgl.h"
#include <qs_supp.h>
#include <ctype.h>

//////////////////////////////////////////////////////////////////////////
// 오픈GL 컨피그 + 도움꾼

#ifndef _QN_EMSCRIPTEN_
#ifndef _QN_MOBILE_
static short openglcore_versions[] =
{
	/* 4.6 */ 406,
	/* 4.5 */ 405,
	/* 4.4 */ 404,
	/* 4.3 */ 403,
	/* 4.2 */ 402,
	/* 4.1 */ 401,
	/* 4.0 */ 400,
	/* 3.3 */ 303,
	/* 3.2 */ 302,
	/* 3.1 */ 301,
	/* 3.0 */ 300,
	/* 2.1 */ 201,
	/* 2.0 */ 200,
};
#endif // _QN_MOBILE_
static short opengles_versions[] =
{
	/* 3.2 */ 302,
	/* 3.1 */ 301,
	/* 3.0 */ 300,
	/* 2.0 */ 200,
};

// 오픈 GL 버전 얻기
int qgl_get_opengl_version(bool is_core, int i)
{
	if (is_core == false)
		return (size_t)i < QN_COUNTOF(opengles_versions) ? opengles_versions[i] : -1;
#ifdef _QN_MOBILE_
	return -1;
#else
	return (size_t)i < QN_COUNTOF(openglcore_versions) ? openglcore_versions[i] : -1;
#endif
}

// 오픈 GL 버전 인덱스
int qgl_index_of_opengl_version(bool is_core, int v)
{
	if (is_core == false)
	{
		for (int i = 0; i < QN_COUNTOF(opengles_versions); i++)
		{
			if (opengles_versions[i] == v)
				return i;
		}
		return -1;
	}
#ifdef _QN_MOBILE_
	return -1;
#else
	for (int i = 0; i < QN_COUNTOF(openglcore_versions); i++)
	{
		if (openglcore_versions[i] == v)
			return i;
	}
	return -1;
#endif
}

// 오픈 GL 버전 근사치 얻기
static int qgl_near_of_opengl_version(bool is_core, int v)
{
	if (is_core == false)
	{
		int index = -1, diff = INT_MAX;
		for (int i = 0; i < QN_COUNTOF(opengles_versions); i++)
		{
			int d = qm_absi(opengles_versions[i] - v);
			if (d < diff)
			{
				index = i;
				diff = d;
			}
		}
		if (index < 0)
			index = 0;
		return opengles_versions[index];
	}
#ifdef _QN_MOBILE_
	return opengles_versions[0];
#else
	int index = -1, diff = INT_MAX;
	for (int i = 0; i < QN_COUNTOF(openglcore_versions); i++)
	{
		int d = qm_absi(openglcore_versions[i] - v);
		if (d < diff)
		{
			index = i;
			diff = d;
		}
	}
	if (index < 0)
		index = 0;
	return openglcore_versions[index];
#endif
}
#endif

// 기본값 만들기
static void qgl_default_config(QglConfig* config, QgFlag flags)
{
	bool core = QN_TMASK(flags, QGSPECIFIC_CORE);

#ifdef _QN_EMSCRIPTEN_
	config->version = 300;
#else
	int ver_major = qn_get_prop_int(QG_PROP_DRIVER_MAJOR, -1, 2, 4);
	int ver_minor = qn_get_prop_int(QG_PROP_DRIVER_MINOR, -1, 0, 9);
	if (ver_major < 0 || ver_minor < 0)
		config->version = qgl_get_opengl_version(core, 0);
	else
		config->version = qgl_near_of_opengl_version(core, ver_major * 100 + ver_minor);
#endif
	const char* prop = qn_get_prop(QG_PROP_RGBA_SIZE);
	if (prop == NULL || strlen(prop) < 4)
		prop = "8888";
	config->red = (byte)prop[0] - '0';
	config->green = (byte)prop[1] - '0';
	config->blue = (byte)prop[2] - '0';
	config->alpha = (byte)prop[3] - '0';
	config->depth = (byte)qn_get_prop_int(QG_PROP_DEPTH_SIZE, 24, 4, 32);
	config->stencil = (byte)qn_get_prop_int(QG_PROP_STENCIL_SIZE, 8, 4, 16);
	config->samples = QN_TMASK(flags, QGFLAG_MSAA) ? (byte)qn_get_prop_int(QG_PROP_MSAA, 4, 0, 8) : 0;

#ifdef QGL_EGL_NO_EXT
	config->srgb = 0;
	config->stereo = 0;
	config->transparent = 0;
	config->float_buffer = 0;
	config->no_error = 0;
	config->robustness = 0;
#else
	prop = qn_get_prop(QG_PROP_CAPABILITY);
	if (prop == NULL)
	{
		config->srgb = 0;
		config->stereo = 0;
		config->transparent = 0;
		config->float_buffer = 0;
		config->no_error = 0;
		config->robustness = 0;
	}
	else
	{
		char* brk = qn_strchr(prop, ';');
		while (brk != NULL)
		{
			size_t len = brk - prop;
			if (len > 32)
				continue;
			if (qn_strnicmp(prop, "floatbuffer", len) == 0)
				config->float_buffer = 1;
			else if (qn_strnicmp(prop, "noerror", len) == 0)
				config->no_error = 1;
			else if (qn_strnicmp(prop, "noreset", len) == 0)
				config->robustness = 1;
			else if (qn_strnicmp(prop, "lostcontext", len) == 0)
				config->robustness = 2;
			else if (qn_strnicmp(prop, "stereo", len) == 0)
				config->stereo = 1;
			else if (qn_strnicmp(prop, "srgb", len) == 0)
				config->srgb = 1;
			else if (qn_strnicmp(prop, "transparent", len) == 0)
				config->transparent = 1;
			prop = qn_strchr(brk + 1, ';');
		}
	}
#endif

	config->core = core;
	config->handle = NULL;
}

// 문자열 버전에서 숫자만 mnn 방식으로
INLINE int qgl_get_version(const char* s)
{
	const char* p;
	for (p = s; *p; p++)
	{
		if (isdigit(*p))
			break;
	}
	if (*p == '\0')
		return 0;
	float f = qn_strtof(p);
	return (int)(floorf(f) * 100.0f + roundf(qm_fractf(f) * 10.0f));
}

// 문자열 얻기
INLINE char* qgl_copy_string(char* buf, size_t size, GLenum name)
{
	const char* s = (const char*)glGetString(name);
	if (s == NULL)
		buf[0] = '\0';
	else
		qn_strncpy(buf, s, size - 1);
	return buf;
}

// 정수 얻기
INLINE GLint qgl_get_integer_v(GLenum name)
{
	GLint n;
	glGetIntegerv(name, &n);
	return n;
}

// irrcht 엔젠에서 가져온 텍스쳐 행렬
INLINE QmMat4 QM_VECTORCALL qgl_mat4_irrcht_texture(float radius, float cx, float cy, float tx, float ty, float sx, float sy)
{
	float c, s;
	qm_sincosf(radius, &s, &c);
	QmMat4 m =
	{
		._11 = c * sx,
		._12 = s * sy,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = -s * sx,
		._22 = c * sy,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = c * sx * cx + -s * cy + tx,
		._32 = s * sy * cx + c * cy + ty,
		._33 = 1.0f,
		._34 = 0.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = 0.0f,
		._44 = 1.0f,
	};
	return m;
}


//////////////////////////////////////////////////////////////////////////
// OPENGL 렌더 디바이스

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"QGLRDH"

static void qgl_rdh_dispose(QsGam* g);
static void qgl_rdh_layout(void);
static void qgl_rdh_reset(void);
static void qgl_rdh_flush(void);
static void qgl_rdh_clear(QgClear flags);
static bool qgl_rdh_begin(bool clear);
static void qgl_rdh_end(void);
static bool qgl_rdh_set_vertex(QgLayoutStage stage, QgBuffer* buffer);
static bool qgl_rdh_set_index(QgBuffer* buffer);
static bool qgl_rdh_set_render(QgRender* render);
static bool qgl_rdh_draw(QgTopology tpg, int vertices);
static bool qgl_rdh_draw_indexed(QgTopology tpg, int indices);

static QgBuffer* qgl_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data);
static QgRender* qgl_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader);

qs_name_vt(RDHBASE) vt_qgl_rdh =
{
	{
		/* name */		VAR_CHK_NAME,
		/* dispose */	qgl_rdh_dispose,
	},

	/* layout */		qgl_rdh_layout,
	/* reset */			qgl_rdh_reset,
	/* clear */			qgl_rdh_clear,

	/* begin */			qgl_rdh_begin,
	/* end */			qgl_rdh_end,
	/* flush */			qgl_rdh_flush,

	/* create_buffer */	qgl_create_buffer,
	/* create_render */	qgl_create_render,

	/* set_vertex */	qgl_rdh_set_vertex,
	/* set_index */		qgl_rdh_set_index,
	/* set_render */	qgl_rdh_set_render,

	/* draw */			qgl_rdh_draw,
	/* draw_indexed */	qgl_rdh_draw_indexed,
};

#ifdef QGL_MAYBE_GL_CORE
//
static const char* qgl_initialize(QglRdh* self, const QglConfig* wanted_config, QglConfig* config, bool vsync)
{
	static const char* name = "OpenGL";

	if (gl_initialize() == false)
		return NULL;

	void* context = gl_create_context(wanted_config, config);
	if (context == NULL)
		return NULL;

	stub_system_actuate();
	if (gl_make_current(context, config) == false)
		return NULL;

	gl_swap_interval(vsync ? 1 : 0);

	self->context = context;

	return name;
}
#endif

//
static const char* qgl_initialize_egl(QglRdh* self, const QglConfig* wanted_config, QglConfig* config, bool vsync)
{
	static const char* name = "OpenGL ES";

	EGLDisplay display = egl_initialize(wanted_config);
	if (display == EGL_NO_DISPLAY)
		return NULL;

	EGLContext context = egl_create_context(display, wanted_config, config);
	if (context == EGL_NO_CONTEXT)
		return NULL;

	EGLSurface surface = egl_create_surface(display, context, config, 0);
	if (surface == EGL_NO_SURFACE)
		return NULL;

	stub_system_actuate();
	if (egl_make_current(display, surface, context, config) == false)
		return NULL;

	eglSwapInterval(display, vsync ? 1 : 0);

	self->egl.display = display;
	self->egl.context = context;
	self->egl.surface = surface;

	return name;
}

//
RdhBase* qgl_allocator(QgFlag flags, QgFeature features)
{
	qn_val_if_fail(QN_TMASK(features, QGRENDERER_OPENGL), NULL);

	QglRdh* self = qn_alloc_zero_1(QglRdh);

	const char* name;
	QglConfig wanted_config;
	qgl_default_config(&wanted_config, flags);

#ifdef QGL_MAYBE_GL_CORE
	name = qgl_initialize(self, &wanted_config, &self->cfg, QN_TMASK(flags, QGFLAG_VSYNC));
	if (name == NULL)
#endif
	{
		name = qgl_initialize_egl(self, &wanted_config, &self->cfg, QN_TMASK(flags, QGFLAG_VSYNC));
		if (name == NULL)
		{
			qn_free(self);
			return NULL;
		}
	}

	// 정보
	const char* gl_version = (const char*)glGetString(GL_VERSION);
	const char* gl_shader_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	const int max_layout_count = qgl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);

	RendererInfo* info = &self->base.info;	// 여기서 RDH_INFO를 쓰면 안된다 (인스턴스가 아직 널임)
	qgl_copy_string(info->renderer, QN_COUNTOF(info->renderer), GL_RENDERER);
	qgl_copy_string(info->vendor, QN_COUNTOF(info->vendor), GL_VENDOR);

	info->renderer_version = qgl_get_version(gl_version);
	info->shader_version = qgl_get_version(gl_shader_version);
	info->max_layout_count = QN_MIN(max_layout_count, QGLOU_MAX_SIZE);
	info->max_indices = qgl_get_integer_v(GL_MAX_ELEMENTS_INDICES);
	info->max_vertices = qgl_get_integer_v(GL_MAX_ELEMENTS_VERTICES);
	info->max_tex_dim = qgl_get_integer_v(GL_MAX_TEXTURE_SIZE);
	info->max_tex_count = qgl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	info->max_off_count = qgl_get_integer_v(GL_MAX_DRAW_BUFFERS);

	info->clr_fmt = qg_rgba_to_clrfmt(self->cfg.red, self->cfg.green, self->cfg.blue, self->cfg.alpha, false);

	info->enabled_stencil = self->cfg.stencil > 0;

	if (self->cfg.core)
	{
		int major = info->shader_version / 100, minor = info->shader_version % 100;
		qn_snprintf(self->res.hdr_vertex, QN_COUNTOF(self->res.hdr_vertex), "#version %d%d0 core\n", major, minor);
		qn_snprintf(self->res.hdr_fragment, QN_COUNTOF(self->res.hdr_fragment), "#version %d%d0 core\n", major, minor);
	}
	else
	{
		self->res.hdr_vertex[0] = '\0';
		qn_strcpy(self->res.hdr_fragment, "precision mediump float;");
	}

	//
	qn_debug_outputf(false, VAR_CHK_NAME, "%s %d/%d [%s by %s]",
		name, info->renderer_version, info->shader_version, info->renderer, info->vendor);
	qn_debug_outputf(false, VAR_CHK_NAME, "%s / %s", gl_version, gl_shader_version);

	//
	return qs_init(self, RdhBase, &vt_qgl_rdh);
}

//
static void qgl_rdh_dispose(QsGam* g)
{
	QglRdh* self = qs_cast_type(g, QglRdh);
	qn_ret_if_ok(self->disposed);
	self->disposed = true;

	// 펜딩
	const QglPending* pd = &self->pd;
	qs_unload(pd->render.index_buffer);
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qs_unload(pd->render.vertex_buffers[i]);
	qs_unload(pd->render.render);

	// 리소스
	rdh_internal_clean();

	//
	egl_dispose(self->egl.display, self->egl.surface, self->egl.context);
#ifdef QGL_MAYBE_GL_CORE
	gl_dispose(self->context);
#endif
	rdh_internal_dispose();
}

//
static void qgl_rdh_layout(void)
{
	rdh_internal_layout();

	RenderTransform* tm = RDH_TRANSFORM;
	QmMat4 ortho = ortho = qm_mat4_ortho_lh((float)tm->size.Width, (float)-tm->size.Height, -1.0f, 1.0f);
	tm->ortho = qm_mat4_mul(ortho, qm_mat4_loc(-1.0f, 1.0f, 0.0f));

	// 뷰포트
	glViewport(0, 0, (GLsizei)tm->size.Width, (GLsizei)tm->size.Height);
#ifdef QGL_MAYBE_GL_CORE
	if (QGL_CORE && RDH_INFO->renderer_version < 400)
		glDepthRange(0.0f, 1.0f);
	else
		glDepthRangef(0.0f, 1.0f);
#else
	glDepthRangef(0.0f, 1.0f);
#endif
}

//
static void qgl_rdh_reset(void)
{
	rdh_internal_reset();

	const RendererInfo* info = RDH_INFO;
	RenderTransform* tm = RDH_TRANSFORM;
	QglSession* ss = QGL_SESSION;
	uint i;

	//----- 펜딩

	//----- 세션
	uint amask = ss->shader.amask;
	ss->shader.program = 0;
	ss->shader.amask = 0;
	qn_zero(ss->shader.lprops, QGLOU_MAX_SIZE, QglLayoutProperty);
	ss->buffer.vertex = GL_INVALID_HANDLE;
	ss->buffer.index = GL_INVALID_HANDLE;
	ss->buffer.uniform = GL_INVALID_HANDLE;
	ss->depth = QGDEPTH_LE;
	ss->stencil = QGSTENCIL_OFF;

	//----- 트랜스폼
	tm->frm = qgl_mat4_irrcht_texture(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//----- 장치 설정
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 뎁스 스텐실
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	if (info->enabled_stencil)
	{
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}
	glDisable(GL_STENCIL_TEST);

	// 래스터라이즈
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDisable(GL_POLYGON_OFFSET_FILL);

	// 브랜드
#ifdef QGL_MAYBE_GL_CORE
	if (QGL_CORE && info->renderer_version >= 400)
	{
		for (i = 0; i < info->max_tex_count; i++)
		{
			glEnablei(GL_BLEND, i);
			glBlendEquationi(i, GL_FUNC_ADD);
			glBlendFunci(i, GL_ONE, GL_ZERO);

			glDisablei(GL_BLEND, i);
			glColorMaski(i, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
	}
	else
#endif
	{
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

		glDisable(GL_BLEND);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	// 텍스쳐
	for (i = 0; i < info->max_tex_count; i++)
	{
		glActiveTexture((GLenum)(GL_TEXTURE0 + i));
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// 세이더
	for (i = 0; i < info->max_layout_count; i++)
	{
		if (QN_TBIT(amask, i))
			glDisableVertexAttribArray((GLuint)i);
	}
	glUseProgram(0);

	// 가위질
	glDisable(GL_SCISSOR_TEST);
	glFrontFace(GL_CW);

#ifdef QGL_MAYBE_GL_CORE
	if (QGL_CORE)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		if (info->renderer_version >= 400)
			glPatchParameteri(GL_PATCH_VERTICES, 3);
	}
#endif

	// 리소스
	// TODO: 옮겨야함
}

// 지우기
static void qgl_rdh_clear(QgClear flags)
{
	// 도움: https://open.gl/depthstencils
	GLbitfield cf = 0;

	if (QN_TMASK(flags, QGCLEAR_STENCIL) && RDH_INFO->enabled_stencil)
	{
		glClearStencil(0);
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_DEPTH))
	{
		if (QGL_SESSION->depth == QGDEPTH_OFF)
		{
			QGL_SESSION->depth = QGDEPTH_LE;
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
		}
#ifdef QGL_MAYBE_GL_CORE
		if (QGL_CORE && RDH_INFO->renderer_version < 400)
			glClearDepth(1.0f);
		else
			glClearDepthf(1.0f);
#else
		glClearDepthf(1.0f);
#endif
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_RENDER))
	{
		const QmVec4 c = RDH_PARAM->bgc;
		glClearColor(c.X, c.Y, c.Z, c.W);
		cf |= GL_COLOR_BUFFER_BIT;
	}

	if (cf != 0)
		glClear(cf);
	}

// 시작
static bool qgl_rdh_begin(bool clear)
{
	if (clear)
		qgl_rdh_clear(QGCLEAR_DEPTH | QGCLEAR_STENCIL | QGCLEAR_RENDER);
	return true;
}

// 끝
static void qgl_rdh_end(void)
{
}

// 플러시
static void qgl_rdh_flush(void)
{
	glFlush();
	QglRdh* self = QGL_RDH;
#ifdef QGL_MAYBE_GL_CORE
	if (gl_swap_buffers() == false)
#endif
	{
		if (self->egl.display && self->egl.surface)
			eglSwapBuffers(self->egl.display, self->egl.surface);
	}
}

// 정점 버퍼 설정, stage에 대한 오류 설정은 rdh에서 하고 왔을 거임
static bool qgl_rdh_set_vertex(QgLayoutStage stage, QgBuffer* buffer)
{
	QglBuffer* buf = qs_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (pd->render.vertex_buffers[stage] != buf)
	{
		qs_unload(pd->render.vertex_buffers[stage]);
		pd->render.vertex_buffers[stage] = qs_loadc(buf, QglBuffer);
	}
	return true;
}

// 인덱스 버퍼 설정
static bool qgl_rdh_set_index(QgBuffer* buffer)
{
	QglBuffer* buf = qs_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (pd->render.index_buffer != buf)
	{
		qs_unload(pd->render.index_buffer);
		pd->render.index_buffer = qs_loadc(buf, QglBuffer);
	}
	return true;
}

// 렌더 설정
static bool qgl_rdh_set_render(QgRender* render)
{
	QglRender* rdr = qs_cast_type(render, QglRender);
	QglPending* pd = QGL_PENDING;
	if (pd->render.render != rdr)
	{
		qs_unload(pd->render.render);
		pd->render.render = qs_loadc(rdr, QglRender);
	}
	return true;
}

// 정점 바인딩
INLINE void qgl_bind_vertex_buffer(const QglBuffer* buffer)
{
	qn_assert(buffer->base.type == QGBUFFER_VERTEX, "try to bind non-vertex buffer");
	QglSession* ss = QGL_SESSION;
	GLuint gl_id = qs_get_desc(buffer, GLuint);
	if (ss->buffer.vertex != gl_id)
	{
		glBindBuffer(GL_ARRAY_BUFFER, gl_id);
		ss->buffer.vertex = gl_id;
	}
}

// 인덱스 바인딩
INLINE void qgl_bind_index_buffer(const QglBuffer* buffer)
{
	qn_assert(buffer->base.type == QGBUFFER_INDEX, "try to bind non-index buffer");
	QglSession* ss = QGL_SESSION;
	GLuint gl_id = qs_get_desc(buffer, GLuint);
	if (ss->buffer.index != gl_id)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_id);
		ss->buffer.index = gl_id;
	}
}

// 유니폼 바인딩
INLINE void qgl_bind_uniform_buffer(const QglBuffer* buffer)
{
	qn_assert(buffer->base.type == QGBUFFER_CONSTANT, "try to bind non-uniform buffer");
	QglSession* ss = QGL_SESSION;
	GLuint gl_id = qs_get_desc(buffer, GLuint);
	if (ss->buffer.uniform != gl_id)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, gl_id);
		ss->buffer.uniform = gl_id;
	}
}

// 버퍼 바인딩
static void qgl_bind_buffer(const QglBuffer* buffer)
{
	switch (buffer->base.type)
	{
		case QGBUFFER_VERTEX:
			qgl_bind_vertex_buffer(buffer);
			break;
		case QGBUFFER_INDEX:
			qgl_bind_index_buffer(buffer);
			break;
		case QGBUFFER_CONSTANT:
			qgl_bind_uniform_buffer(buffer);
			break;
		default:
			qn_debug_outputs(true, VAR_CHK_NAME, "invalid buffer type");
			break;
	}
}

// 오토 세이더 변수 (오토가 아니면 RDH의 사용자 함수로 떠넘긴다)
static void qgl_process_shader_variable(const QgVarShader* var)
{
	switch (var->scauto)
	{
		case QGSCA_ORTHO_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->ortho._11);
			break;
		case QGSCA_WORLD:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->world._11);
			break;
		case QGSCA_VIEW:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->view._11);
			break;
		case QGSCA_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->proj._11);
			break;
		case QGSCA_VIEW_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->view_proj._11);
			break;
		case QGSCA_INV_VIEW:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->invv._11);
			break;
		case QGSCA_WORLD_VIEW_PROJ:
		{
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			QmMat4 m = qm_mat4_mul(RDH_TRANSFORM->world, RDH_TRANSFORM->view_proj);
			glUniformMatrix4fv(var->offset, 1, false, &m._11);
		} break;
		case QGSCA_TEX1:
		case QGSCA_TEX2:
		case QGSCA_TEX3:
		case QGSCA_TEX4:
		case QGSCA_TEX5:
		case QGSCA_TEX6:
		case QGSCA_TEX7:
		case QGSCA_TEX8:
			switch (var->sctype)
			{
				case QGSCT_SAMPLER1D:
				case QGSCT_SAMPLER2D:
				case QGSCT_SAMPLER3D:
				case QGSCT_SAMPLERCUBE:
					glUniform1i(var->offset, var->scauto - QGSCA_TEX1);
					break;
				default:
					qn_debug_outputs(true, VAR_CHK_NAME, "invalid auto shader texture");
					break;
			}
			break;
		case QGSCA_PROP_VEC1:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[0].f);
			break;
		case QGSCA_PROP_VEC2:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[1].f);
			break;
		case QGSCA_PROP_VEC3:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[2].f);
			break;
		case QGSCA_PROP_VEC4:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[3].f);
			break;
		case QGSCA_PROP_MAT1:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[0]._11);
			break;
		case QGSCA_PROP_MAT2:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[1]._11);
			break;
		case QGSCA_PROP_MAT3:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[2]._11);
			break;
		case QGSCA_PROP_MAT4:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[3]._11);
			break;
		case QGSCA_MAT_PALETTE:
			qn_verify(var->sctype == QGSCT_FLOAT16);
			glUniformMatrix4fv(var->offset, RDH_PARAM->bone_count, false, (const GLfloat*)RDH_PARAM->bone_ptr);
			break;
		default:
			if (RDH_PARAM->callback_func != NULL)
				RDH_PARAM->callback_func(RDH_PARAM->callback_data, (nint)var->hash, var);
			break;
	}
}

// 세이더랑 레이아웃
static bool qgl_commit_shader_layout(const QglRender* rdr)
{
	const size_t max_attrs = RDH_INFO->max_layout_count;

	// 프로그램 설정
	if (QGL_SESSION->shader.program != rdr->shader.program)
	{
		QGL_SESSION->shader.program = rdr->shader.program;
		glUseProgram(rdr->shader.program);
	}

	// 유니폼 값 넣고
	size_t s, i;
	qn_ctnr_foreach(&rdr->shader.uniforms, i)
	{
		const QgVarShader* var = &qn_ctnr_nth(&rdr->shader.uniforms, i);
		qgl_process_shader_variable(var);
	}

	// 어트리뷰트 + 레이아웃
	uint aok = 0, amask = QGL_SESSION->shader.amask;
	for (s = 0; s < QGLOS_MAX_VALUE; s++)
	{
		QglBuffer* buf = QGL_PENDING->render.vertex_buffers[s];
		if (buf == NULL)
			continue;
		qgl_bind_vertex_buffer(buf);

		const QglLayoutInput * stages = rdr->layout.stages[s];
		const GLsizei gl_stride = (GLsizei)rdr->layout.strides[s];
		const size_t count = rdr->layout.counts[s];
		for (i = 0; i < count; i++)
		{
			const QglLayoutInput* input = &stages[i];
			const GLint gl_attr = rdr->shader.usages[input->usage];
			if (gl_attr == 0xFF)
			{
				const char* name = qg_layout_usage_to_str(input->usage);
				qn_debug_outputf(true, VAR_CHK_NAME, "shader attribute not found: %s", name);
				continue;
			}

			aok |= QN_BIT(gl_attr);
			if (QN_TBIT(amask, gl_attr) == false)
			{
				QN_SBIT(&amask, gl_attr, true);
				glEnableVertexAttribArray(gl_attr);
			}

			const GLuint gl_offset = input->offset;
			QglLayoutProperty* lp = &QGL_SESSION->shader.lprops[input->usage];
			if (lp->offset != gl_offset ||
				lp->format != input->format ||
				lp->stride != gl_stride ||
				lp->count != input->count ||
				lp->normalized != input->normalized)
			{
				glVertexAttribPointer(gl_attr, input->count, input->format, input->normalized, gl_stride, (GLvoid*)(size_t)gl_offset);
				lp->offset = gl_offset;
				lp->format = input->format;
				lp->stride = gl_stride;
				lp->count = input->count;
				lp->normalized = input->normalized;
			}
		}
	}

	// 레이아웃 정리
	uint aftermask = amask & ~aok;
	for (i = 0; i < max_attrs && aftermask; i++)
	{
		if (QN_TBIT(aftermask, 0))
		{
			QN_SBIT(&amask, i, false);
			glDisableVertexAttribArray((GLuint)i);
		}
		aftermask >>= 1;
	}
	QGL_SESSION->shader.amask = amask;

	return true;
}

// depth 함수 변환
INLINE GLenum qgl_depth_to_enum(QgDepth func)
{
	switch (func)
	{
		case QGDEPTH_OFF:		return GL_NEVER;
		case QGDEPTH_LE:		return GL_LESS;
		case QGDEPTH_LEQ:		return GL_LEQUAL;
		case QGDEPTH_GR:		return GL_GREATER;
		case QGDEPTH_GEQ:		return GL_GEQUAL;
		case QGDEPTH_EQ:		return GL_EQUAL;
		case QGDEPTH_NEQ:		return GL_NOTEQUAL;
		case QGDEPTH_ALWAYS:	return GL_ALWAYS;
		default:				return GL_NONE;
	}
}

// 뎁스 스텐실
static void qgl_commit_depth_stencil(const QglRender* rdr)
{
	QglSession* ss = QGL_SESSION;

	// 뎁스
	if (ss->depth != rdr->depth)
	{
		if (rdr->depth == QGDEPTH_OFF)
		{
			if (ss->depth != QGDEPTH_OFF)
				glDisable(GL_DEPTH_TEST);
		}
		else
		{
			if (ss->depth == QGDEPTH_OFF)
				glEnable(GL_DEPTH_TEST);
			GLenum gl_func = qgl_depth_to_enum(rdr->depth);
			glDepthFunc(gl_func);
		}
		ss->depth = rdr->depth;
	}

	// 스텐실
	if (RDH_INFO->enabled_stencil && ss->stencil != rdr->stencil)
	{
		if (rdr->stencil == QGSTENCIL_OFF)
		{
			glDisable(GL_STENCIL_TEST);
		}
		else
		{
			if (ss->stencil == QGSTENCIL_OFF)
				glEnable(GL_STENCIL_TEST);
			if (rdr->stencil == QGSTENCIL_WRITE)
			{
				// 보통으로 그릴 때 스텐실을 1로 채움
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilMask(0xFF);
			}
			else if (rdr->stencil == QGSTENCIL_EVADE)
			{
				// 스텐실 1로 채워진 부분은 그리지 않음 -> 외곽선 같은거 그릴때 (https://heinleinsgame.tistory.com/25)
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glStencilMask(0);
			}
			else if (rdr->stencil == QGSTENCIL_OVER)
			{
				// 스텐실 1로 채워진 부분만 그림 -> 그림자/반사 같은거 그릴때 (https://open.gl/depthstencils)
				glStencilFunc(GL_EQUAL, 1, 0xFF);
				glStencilMask(0);
			}
		}
		ss->stencil = rdr->stencil;
	}
}

// 렌더 커밋
static bool qgl_rdh_commit_render(void)
{
	const QglRender* rdr = QGL_PENDING->render.render;
	VAR_CHK_IF_NULL(rdr, false);

	if (qgl_commit_shader_layout(rdr) == false)
		return false;

	qgl_commit_depth_stencil(rdr);

	return true;
}

// 토폴로지 변환
INLINE GLenum qgl_topology_to_enum(QgTopology tpg)
{
	static GLenum gl_tpgs[] =
	{
		[QGTPG_POINT] = GL_POINTS,
		[QGTPG_LINE] = GL_LINES,
		[QGTPG_LINE_STRIP] = GL_LINE_STRIP,
		[QGTPG_TRI] = GL_TRIANGLES,
		[QGTPG_TRI_STRIP] = GL_TRIANGLE_STRIP,
		[QGTPGEX_TRI_FAN] = GL_TRIANGLE_FAN,
	};
	return gl_tpgs[tpg];
}

// 그리기
static bool qgl_rdh_draw(QgTopology tpg, int vertices)
{
	if (qgl_rdh_commit_render() == false)
		return false;

	GLenum gl_tpg = qgl_topology_to_enum(tpg);
	glDrawArrays(gl_tpg, 0, (GLsizei)vertices);
	return true;
}

// 그리기 인덱스
static bool qgl_rdh_draw_indexed(QgTopology tpg, int indices)
{
	const QglBuffer* index = QGL_PENDING->render.index_buffer;
	VAR_CHK_IF_NULL(index, false);
	qgl_bind_index_buffer(index);

	if (qgl_rdh_commit_render() == false)
		return false;

	const GLenum gl_tpg = qgl_topology_to_enum(tpg);
	const GLenum gl_stride = index->base.stride == sizeof(uint) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	glDrawElements(gl_tpg, (GLsizei)indices, gl_stride, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "QGLBUFFER"

//
static void qgl_buffer_dispose(QsGam* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	if (self->base.mapped)
	{
		if (QGL_CORE == false && RDH_INFO->renderer_version < 300)
			qn_free(self->lock_pointer);
		else
			glUnmapBuffer(self->gl_type);
	}

	GLuint gl_handle = qs_get_desc(self, GLuint);
	glDeleteBuffers(1, &gl_handle);

	qn_free(self);
}

//
static void* qgl_buffer_map(QgBuffer* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEED2(self, gl_usage, GL_DYNAMIC_DRAW, NULL);
	qn_assert(self->lock_pointer == NULL, "버퍼가 잠겨있는데요!");

	if (QGL_CORE == false && RDH_INFO->renderer_version < 300)
	{
		// ES2 전용
		self->lock_pointer = qn_alloc(self->base.size, byte);
	}
	else
	{
		qgl_bind_buffer(self);
		self->lock_pointer = glMapBufferRange(self->gl_type, 0, self->base.size,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	}

	self->base.mapped = true;
	return self->lock_pointer;
}

//
static bool qgl_buffer_unmap(QgBuffer* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	qn_assert(self->lock_pointer != NULL, "버퍼가 안 잠겼는데요!");

	qgl_bind_buffer(self);

	if (QGL_CORE == false && RDH_INFO->renderer_version < 300)
	{
		// ES2 전용
		glBufferSubData(self->gl_type, 0, self->base.size, self->lock_pointer);
		qn_free(self->lock_pointer);
	}
	else
	{
		glUnmapBuffer(self->gl_type);
	}

	self->lock_pointer = NULL;
	self->base.mapped = false;
	return true;
}

//
static bool qgl_buffer_data(QgBuffer* g, const void* data)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEED2(self, gl_usage, GL_DYNAMIC_DRAW, false);

	qgl_bind_buffer(self);
	glBufferSubData(self->gl_type, 0, self->base.size, data);

	return true;
}

//
static QgBuffer* qgl_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	// 우선 만들자
	GLsizeiptr gl_size = (GLsizeiptr)count * stride;
	GLenum gl_type;
	if (type == QGBUFFER_VERTEX)
	{
		gl_type = GL_ARRAY_BUFFER;
		QGL_SESSION->buffer.vertex = GL_INVALID_HANDLE;
	}
	else if (type == QGBUFFER_INDEX)
	{
		VAR_CHK_IF_COND(stride != 2 && stride != 4, "invalid index buffer stride. require 2 or 4", NULL);
		gl_type = GL_ELEMENT_ARRAY_BUFFER;
		QGL_SESSION->buffer.index = GL_INVALID_HANDLE;
	}
	else if (type == QGBUFFER_CONSTANT)
	{
		gl_type = GL_UNIFORM_BUFFER;
		// 256 바이트 정렬
		gl_size = (gl_size + 255) & ~255;
		QGL_SESSION->buffer.uniform = GL_INVALID_HANDLE;
	}
	else
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "invalid buffer type");
		return NULL;
	}

	GLuint gl_id;
	glGenBuffers(1, &gl_id);
	glBindBuffer(gl_type, gl_id);

	GLenum gl_usage = initial_data != NULL ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	glBufferData(gl_type, gl_size, initial_data, gl_usage);		// 여기까지 초기 메모리 설정 또는 데이터 넣기(STATIC)

	// 진짜 만듦
	QglBuffer* self = qn_alloc_zero_1(QglBuffer);
	qs_set_desc(self, gl_id);
	self->base.type = type;
	self->base.size = (uint)gl_size;
	self->base.count = count;
	self->base.stride = (ushort)stride;
	self->gl_type = gl_type;
	self->gl_usage = gl_usage;

	// VT 여기서 설정
	static qs_name_vt(QGBUFFER) vt_qgl_buffer =
	{
		.base.name = VAR_CHK_NAME,
		.base.dispose = qgl_buffer_dispose,

		.map = qgl_buffer_map,
		.unmap = qgl_buffer_unmap,
		.data = qgl_buffer_data,
	};
	return qs_init(self, QgBuffer, &vt_qgl_buffer);
}


//////////////////////////////////////////////////////////////////////////
// 렌더러

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "QGLRENDER"

//
static void qgl_render_delete_shader(QglRender* self, bool check_rdh)
{
	if (self->shader.program == 0)
		return;
	if (check_rdh && QGL_RDH->disposed == false && QGL_SESSION->shader.program == self->shader.program)
	{
		QGL_SESSION->shader.program = 0;
		glUseProgram(0);
	}
	if (self->shader.vertex != 0)
	{
		glDetachShader(self->shader.program, self->shader.vertex);
		glDeleteShader(self->shader.vertex);
	}
	if (self->shader.fragment != 0)
	{
		glDetachShader(self->shader.program, self->shader.fragment);
		glDeleteShader(self->shader.fragment);
	}
	glDeleteProgram(self->shader.program);
}

//
static void qgl_render_dispose(QsGam* g)
{
	QglRender* self = qs_cast_type(g, QglRender);

	qgl_render_delete_shader(self, true);
	qn_ctnr_disp(&self->shader.uniforms);
	qn_ctnr_disp(&self->shader.attrs);
	qn_ctnr_disp(&self->layout.inputs);

	rdh_internal_unlink_node(RDHNODE_RENDER, self);
	qn_free(self);
}

// 프로그램 상태값 얻기
INLINE GLint qgl_get_program_iv(GLuint program, GLenum name)
{
	GLint n;
	glGetProgramiv(program, name, &n);
	return n;
}

// 세이더 상태값 얻기
INLINE GLint qgl_get_shader_iv(GLuint handle, GLenum name)
{
	GLint n;
	glGetShaderiv(handle, name, &n);
	return n;
}

// 세이더 컴파일
static GLuint qgl_render_compile_shader(GLenum gl_type, const char* header, const char* code)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	GLuint gl_shader = glCreateShader(gl_type);
	const char* codes[2] = { header, code };
	glShaderSource(gl_shader, 2, codes, NULL);
	glCompileShader(gl_shader);

	if (qgl_get_shader_iv(gl_shader, GL_COMPILE_STATUS) != GL_FALSE)
		return gl_shader;

	const char* type = gl_type == GL_VERTEX_SHADER ? "vertex" : gl_type == GL_FRAGMENT_SHADER ? "fragment" : "unknown";
	GLint gl_log_len = qgl_get_shader_iv(gl_shader, GL_INFO_LOG_LENGTH);
	if (gl_log_len <= 0)
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to %s shader compile", type);
	else
	{
		GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
		glGetShaderInfoLog(gl_shader, gl_log_len, &gl_log_len, gl_log);
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to %s shader compile: %s", type, gl_log);
		qn_free(gl_log);
	}

	glDeleteShader(gl_shader);
	return 0;
}

// GLenum을 상수로
INLINE QgScType qgl_enum_to_shader_const(GLenum gl_type)
{
	switch (gl_type)
	{
		case GL_FLOAT:						return QGSCT_FLOAT1;
		case GL_FLOAT_VEC2:					return QGSCT_FLOAT2;
		case GL_FLOAT_VEC3:					return QGSCT_FLOAT3;
		case GL_FLOAT_VEC4:					return QGSCT_FLOAT4;
		case GL_INT:						return QGSCT_INT1;
		case GL_INT_VEC2:					return QGSCT_INT2;
		case GL_INT_VEC3:					return QGSCT_INT3;
		case GL_INT_VEC4:					return QGSCT_INT4;
		case GL_UNSIGNED_INT:				return QGSCT_UINT1;
#ifdef GL_UNSIGNED_INT_VEC2
		case GL_UNSIGNED_INT_VEC2:			return QGSCT_UINT2;
#endif
#ifdef GL_UNSIGNED_INT_VEC3
		case GL_UNSIGNED_INT_VEC3:			return QGSCT_UINT3;
#endif
#ifdef GL_UNSIGNED_INT_VEC4
		case GL_UNSIGNED_INT_VEC4:			return QGSCT_UINT4;
#endif
		case GL_BOOL:						return QGSCT_BYTE1;
		case GL_BOOL_VEC2:					return QGSCT_BYTE2;
		case GL_BOOL_VEC3:					return QGSCT_BYTE3;
		case GL_BOOL_VEC4:					return QGSCT_BYTE4;
		case GL_FLOAT_MAT2:					return QGSCT_FLOAT4;
		case GL_FLOAT_MAT4:					return QGSCT_FLOAT16;
#ifdef GL_SAMPLER_1D
		case GL_SAMPLER_1D:					return QGSCT_SAMPLER1D;
#endif
		case GL_SAMPLER_2D:					return QGSCT_SAMPLER2D;
#ifdef GL_SAMPLER_3D
		case GL_SAMPLER_3D:					return QGSCT_SAMPLER3D;
#endif
		case GL_SAMPLER_CUBE:				return QGSCT_SAMPLERCUBE;
#if false
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_CUBE_SHADOW:
#endif
#ifdef GL_INT_SAMPLER_2D
		case GL_INT_SAMPLER_2D:				return QGSCT_SAMPLER2D;
#endif
#ifdef GL_INT_SAMPLER_3D
		case GL_INT_SAMPLER_3D:				return QGSCT_SAMPLER3D;
#endif
#ifdef GL_INT_SAMPLER_CUBE
		case GL_INT_SAMPLER_CUBE:			return QGSCT_SAMPLERCUBE;
#endif
#if false
		case GL_INT_SAMPLER_2D_ARRAY:
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_2D
		case GL_UNSIGNED_INT_SAMPLER_2D:	return QGSCT_SAMPLER2D;
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_3D
		case GL_UNSIGNED_INT_SAMPLER_3D:	return QGSCT_SAMPLER3D;
#endif
#ifdef GL_UNSIGNED_INT_SAMPLER_CUBE
		case GL_UNSIGNED_INT_SAMPLER_CUBE:	return QGSCT_SAMPLERCUBE;
#endif
#if false
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
#endif
		default:							return QGSCT_UNKNOWN;
	}
}

// 세이더 만들기
static bool qgl_render_bind_shader(QglRender* self, const QgCodeData* vertex, const QgCodeData* fragment)
{
	// 프로그램이랑 세이더 만들고
	GLuint gl_vertex_shader = qgl_render_compile_shader(GL_VERTEX_SHADER, QGL_RESOURCE->hdr_vertex, vertex->code);
	GLuint gl_fragment_shader = qgl_render_compile_shader(GL_FRAGMENT_SHADER, QGL_RESOURCE->hdr_fragment, fragment->code);
	if (gl_vertex_shader == 0 || gl_fragment_shader == 0)
	{
		if (gl_vertex_shader != 0)
			glDeleteShader(gl_vertex_shader);
		if (gl_fragment_shader != 0)
			glDeleteShader(gl_fragment_shader);
		return false;
	}

	self->shader.vertex = gl_vertex_shader;
	self->shader.fragment = gl_fragment_shader;
	self->shader.program = glCreateProgram();
	glAttachShader(self->shader.program, gl_vertex_shader);
	glAttachShader(self->shader.program, gl_fragment_shader);

	// 링크
	glLinkProgram(self->shader.program);
	if (qgl_get_program_iv(self->shader.program, GL_LINK_STATUS) == GL_FALSE)
	{
		GLsizei gl_log_len = qgl_get_program_iv(self->shader.program, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
			qn_debug_outputf(true, VAR_CHK_NAME, "failed to link shader");
		else
		{
			GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
			glGetProgramInfoLog(self->shader.program, gl_log_len, &gl_log_len, gl_log);
			qn_debug_outputf(true, VAR_CHK_NAME, "failed to link shader: %s", gl_log);
			qn_free(gl_log);
		}
		return false;
	}

	// 분석
	GLint i, index, gl_count, gl_size, gl_index;
	GLenum gl_type;
	GLchar sz[64];

	// 유니폼
	gl_count = qgl_get_program_iv(self->shader.program, GL_ACTIVE_UNIFORMS);
	if (gl_count > 0)
	{
		qn_ctnr_init(QglCtnUniform, &self->shader.uniforms, gl_count);
		for (index = i = 0; i < gl_count; i++)
		{
			glGetActiveUniform(self->shader.program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz);
			gl_index = glGetUniformLocation(self->shader.program, sz);
			if (gl_index < 0)
			{
				qn_debug_outputf(true, VAR_CHK_NAME, "failed to get uniform location: %s", sz);
				continue;
			}

			QgScType sctype = qgl_enum_to_shader_const(gl_type);
			if (sctype == QGSCT_UNKNOWN)
				qn_debug_outputf(true, VAR_CHK_NAME, "unsupported uniform type: %s (type: %X)", sz, gl_type);

			QgVarShader* var = &qn_ctnr_nth(&self->shader.uniforms, index++);
			qn_strcpy(var->name, sz);
			var->hash = qn_strihash(sz);
			var->offset = (ushort)glGetUniformLocation(self->shader.program, sz);
			var->size = (ushort)gl_size;
			var->sctype = sctype;
			var->scauto = qg_str_to_shader_const_auto(var->hash, sz);
			if (var->scauto == QGSCA_UNKNOWN)
				var->hash = (size_t)qn_get_key(sz);
		}
	}

	// 어트리뷰트
	memset(self->shader.usages, 0xFF, QN_COUNTOF(self->shader.usages));
	gl_count = qgl_get_program_iv(self->shader.program, GL_ACTIVE_ATTRIBUTES);
	if (gl_count > 0)
	{
		qn_ctnr_init(QglCtnAttr, &self->shader.attrs, gl_count);
		for (index = i = 0; i < gl_count; i++)
		{
			glGetActiveAttrib(self->shader.program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz);
			gl_index = glGetAttribLocation(self->shader.program, sz);
			if (gl_index < 0)
			{
				qn_debug_outputf(true, VAR_CHK_NAME, "failed to get attribute location: %s", sz);
				continue;
			}

			QgScType sctype = qgl_enum_to_shader_const(gl_type);
			if (sctype == QGSCT_UNKNOWN)
				qn_debug_outputf(true, VAR_CHK_NAME, "unsupported attribute type: %s (type: %X)", sz, gl_type);

			QglVarAttr* var = &qn_ctnr_nth(&self->shader.attrs, index++);
			qn_strcpy(var->name, sz);
			var->hash = qn_strihash(sz);
			var->attrib = gl_index;
			var->size = gl_size;
			var->usage = qg_str_to_layout_usage(var->hash, sz);
			var->sctype = sctype;
			self->shader.usages[var->usage] = (byte)gl_index;
		}
	}

	return true;
}

// 버텍스 레이아웃 만들기
static bool qgl_render_bind_layout_input(QglRender* self, const QgLayoutData* layout)
{
	static byte lo_count[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4, 3, 2, 1,
		/* INT     */ 4, 3, 2, 1,
		/* HALF-F  */ 4, 2, 1, 1,
		/* HALF-I  */ 4, 2, 1, 1,
		/* BYTE    */ 4, 3, 2, 1, 1, 1,
		/* USHORT  */ 1, 1, 1,
		/* NONE    */ 0, 0,
	};
	static GLenum lo_format[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ GL_NONE,
		/* FLOAT   */ GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT,
		/* INT     */ GL_INT, GL_INT, GL_INT, GL_INT,
		/* HALF-F  */ GL_HALF_FLOAT, GL_HALF_FLOAT, GL_HALF_FLOAT, GL_UNSIGNED_INT_10F_11F_11F_REV,
		/* HALF-I  */ GL_SHORT, GL_SHORT, GL_SHORT, GL_UNSIGNED_INT_2_10_10_10_REV,
		/* BYTE    */ GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE,
		/* USHORT  */ GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_4_4_4_4,
		/* NONE    */ GL_NONE, GL_NONE,
	};
	static byte lo_size[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4 * sizeof(float), 3 * sizeof(float), 2 * sizeof(float), 1 * sizeof(float),
		/* INT     */ 4 * sizeof(int), 3 * sizeof(int), 2 * sizeof(int), 1 * sizeof(int),
		/* HALF-F  */ 4 * sizeof(halffloat), 2 * sizeof(halffloat), 1 * sizeof(halffloat), 1 * sizeof(int),
		/* HALF-I  */ 4 * sizeof(short), 2 * sizeof(short), 1 * sizeof(short), 1 * sizeof(int),
		/* BYTE    */ 4 * sizeof(byte), 3 * sizeof(byte), 2 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte),
		/* USHORT  */ 2 * sizeof(ushort), 2 * sizeof(ushort), 2 * sizeof(ushort),
		/* NONE    */ 0, 0,
	};

	size_t i;

	// 갯수 검사
	ushort loac[QGLOS_MAX_VALUE] = { 0, };
	for (i = 0; i < layout->count; i++)
	{
		const QgLayoutInput* input = &layout->inputs[i];
		if ((size_t)input->stage >= QGLOS_MAX_VALUE)
		{
			qn_debug_outputf(true, VAR_CHK_NAME, "invalid layout stage: %d", input->stage);
			return false;
		}
		loac[input->stage]++;
	}

	// 레이아웃
	qn_ctnr_init(QglCtnLayoutInput, &self->layout.inputs, layout->count);
	QglLayoutInput* pstage = qn_ctnr_data(&self->layout.inputs);
	QglLayoutInput* stages[QGLOS_MAX_VALUE];
	for (i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (loac[i] == 0)
			stages[i] = NULL;
		else
		{
			self->layout.counts[i] = loac[i];
			self->layout.stages[i] = stages[i] = pstage;
			pstage += loac[i];
		}
	}

	// 항목별로 만들어서 스테이지로 구분
	for (i = 0; i < layout->count; i++)
	{
		const QgLayoutInput* input = &layout->inputs[i];
		const GLenum gl_format = lo_format[input->format];
		if (gl_format == GL_NONE)
		{
			const char* name = qg_clrfmt_to_str(input->format);
			qn_debug_outputf(true, VAR_CHK_NAME, "cannot use or unknown layout format: %s", name);
			return false;
		}

		QglLayoutInput* stage = stages[input->stage]++;
		stage->stage = input->stage;
		stage->usage = input->usage;
		stage->offset = self->layout.strides[input->stage];
		stage->format = gl_format;
		stage->count = lo_count[input->format];
		stage->normalized = (GLboolean)input->normalized;
		self->layout.strides[input->stage] += lo_size[input->format];
	}

	return true;
}

// 렌더 만들기. 오류 처리는 다하고 왔을 것이다
QgRender* qgl_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader)
{
	QglRender* self = qn_alloc_zero_1(QglRender);
	qg_node_set_name(qs_cast_type(self, QgNode), name);

	// 세이더
	if (qgl_render_bind_shader(self, &shader->vertex, &shader->pixel) == false)
		goto pos_error;

	// 레이아웃
	if (qgl_render_bind_layout_input(self, &shader->layout) == false)
		goto pos_error;

	// 속성
	self->depth = prop->depth;
	self->stencil = prop->stencil;

	//
	static qs_name_vt(QSGAM) vt_es_render =
	{
		.name = VAR_CHK_NAME,
		.dispose = qgl_render_dispose,
	};
	qs_init(self, QgRender, &vt_es_render);
	if (name)
		rdh_internal_add_node(RDHNODE_RENDER, self);
	return qs_cast_type(self, QgRender);

pos_error:
	qgl_render_delete_shader(self, false);
	qn_ctnr_disp(&self->shader.uniforms);
	qn_ctnr_disp(&self->shader.attrs);
	qn_ctnr_disp(&self->layout.inputs);
	qn_free(self);
	return NULL;
}

#endif // USE_GL