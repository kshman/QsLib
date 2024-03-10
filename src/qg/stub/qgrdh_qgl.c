//
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

#ifdef _DEBUG
#define GLDEBUG(x)				QN_STMT_BEGIN{ x; GLenum err = glGetError(); if (err != GL_NO_ERROR) qn_mesgfb(VAR_CHK_NAME, "GL Error: %d(%X) [%u]", err, err, __LINE__); }QN_STMT_END
#else
#define GLDEBUG(x)				x
#endif
#define GLCHECK(x,action,ret)	QN_STMT_BEGIN{ x; GLenum err = glGetError(); if (err != GL_NO_ERROR) { qn_mesgfb(VAR_CHK_NAME, "GL Error: %d(%X) [%u]", err, err, __LINE__); action; return ret; } }QN_STMT_END

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
		for (int i = 0; i < (int)QN_COUNTOF(opengles_versions); i++)
		{
			if (opengles_versions[i] == v)
				return i;
		}
		return -1;
	}
#ifdef _QN_MOBILE_
	return -1;
#else
	for (int i = 0; i < (int)QN_COUNTOF(openglcore_versions); i++)
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
		for (int i = 0; i < (int)QN_COUNTOF(opengles_versions); i++)
		{
			const int d = qm_absi(opengles_versions[i] - v);
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
	for (int i = 0; i < (int)QN_COUNTOF(openglcore_versions); i++)
	{
		const int d = qm_absi(openglcore_versions[i] - v);
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
	const bool core = QN_TMASK(flags, QGSPECIFIC_CORE);

#ifdef _QN_EMSCRIPTEN_
	config->version = 300;
#else
	const int ver_major = qn_get_prop_int(QG_PROP_DRIVER_MAJOR, -1, 2, 4);
	const int ver_minor = qn_get_prop_int(QG_PROP_DRIVER_MINOR, -1, 0, 9);
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
		const char* sep = "; ";
		char* brk = NULL, * p = qn_strdup(prop);
		for (const char* tok = qn_strtok(p, sep, &brk); tok != NULL; tok = qn_strtok(NULL, sep, &brk))
		{
			if (qn_stricmp(tok, "floatbuffer") == 0)
				config->float_buffer = 1;
			else if (qn_stricmp(tok, "noerror") == 0)
				config->no_error = 1;
			else if (qn_stricmp(tok, "noreset") == 0)
				config->robustness = 1;
			else if (qn_stricmp(tok, "lostcontext") == 0)
				config->robustness = 2;
			else if (qn_stricmp(tok, "stereo") == 0)
				config->stereo = 1;
			else if (qn_stricmp(tok, "srgb") == 0)
				config->srgb = 1;
			else if (qn_stricmp(tok, "transparent") == 0)
				config->transparent = 1;
		}
		qn_free(p);
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
	const float f = qn_strtof(p);
	return (int)(floorf(f) * 100.0f + roundf(qm_fractf(f) * 10.0f));
}

// 문자열 얻기
INLINE char* qgl_copy_string(char* buf, size_t size, GLenum name)
{
	const char* s = (const char*)glGetString(name); GLDEBUG((void)s);
	if (s == NULL)
		buf[0] = '\0';
	else
		qn_strncpy(buf, s, size - 1);
	return buf;
}

// 문자열 얻기 (할당)
INLINE char* qgl_alloc_string(GLenum name)
{
	const char* s = (const char*)glGetString(name); GLDEBUG((void)s);
	if (s == NULL)
		return NULL;
	return qn_strdup(s);
}

// 정수 얻기
INLINE GLint qgl_get_integer_v(GLenum name)
{
	GLint n;
	GLDEBUG(glGetIntegerv(name, &n));
	return n;
}

// irrcht 엔젠에서 가져온 텍스쳐 행렬
INLINE QmMat4 QM_VECTORCALL qgl_mat4_irrcht_texture(float radius, float cx, float cy, float tx, float ty, float sx, float sy)
{
	float c, s;
	qm_sincosf(radius, &s, &c);
	const QmMat4 m =
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

static void qgl_rdh_dispose(QnGam g);
static void qgl_rdh_layout(void);
static void qgl_rdh_reset(void);
static void qgl_rdh_flush(void);
static void qgl_rdh_clear(QgClear flags);
static bool qgl_rdh_begin(bool clear);
static void qgl_rdh_end(void);
static bool qgl_rdh_set_vertex(QgLayoutStage stage, void* buffer);
static bool qgl_rdh_set_index(void* buffer);
static bool qgl_rdh_set_render(void* render);
static bool qgl_rdh_set_texture(int stage, void* texture);
static bool qgl_rdh_draw(QgTopology tpg, int vertices);
static bool qgl_rdh_draw_indexed(QgTopology tpg, int indices);
static void qgl_rdh_draw_sprite(const QmRect* bound, void* texture, QmKolor color, const QMVEC* coord);
static void qgl_rdh_draw_sprite_ex(const QmRect* bound, float angle, void* texture, QmKolor color, const QMVEC* coord);
static void qgl_rdh_draw_glyph(const QmRect* bound, void* texture, QmKolor color, const QMVEC* coord);

static QgBuffer* qgl_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data);
static QgRenderState* qgl_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader);
static QgTexture* qgl_create_texture(const char* name, const QgImage* image, QgTexFlag flags);

static void* qgl_buffer_map(void* g, bool synchronized);
static bool qgl_buffer_unmap(void* g);
static bool qgl_buffer_data(void* g, int size, const void* data);

static void qgl_batch_stream_dispose(QglBatchStream* self);
static void qgl_batch_stream_clear(QglBatchStream* self);

static QglBatchStream* qgl_create_batch_ortho(void);
static void qgl_batch_ortho_flush(QglBatchStream* batch);
static void qg_batch_ortho_draw_rect(QglBatchStream* batch, OrthoVertex* v4, QglTexture* tex);
static void qg_batch_ortho_draw_glyph(QglBatchStream* batch, OrthoVertex* v4, QglTexture* tex);

QN_DECL_VTABLE(RDHBASE) vt_qgl_rdh =
{
	{
		/* name */			VAR_CHK_NAME,
		/* dispose */		qgl_rdh_dispose,
	},

	/* layout */			qgl_rdh_layout,
	/* reset */				qgl_rdh_reset,
	/* clear */				qgl_rdh_clear,

	/* begin */				qgl_rdh_begin,
	/* end */				qgl_rdh_end,
	/* flush */				qgl_rdh_flush,

	/* create_buffer */		qgl_create_buffer,
	/* create_render */		qgl_create_render,
	/* create_texture */	qgl_create_texture,

	/* set_vertex */		qgl_rdh_set_vertex,
	/* set_index */			qgl_rdh_set_index,
	/* set_render */		qgl_rdh_set_render,
	/* set_texture */		qgl_rdh_set_texture,

	/* draw */				qgl_rdh_draw,
	/* draw_indexed */		qgl_rdh_draw_indexed,
	/* draw_sprite */		qgl_rdh_draw_sprite,
	/* draw_sprite_ex */	qgl_rdh_draw_sprite_ex,
	/* draw_glyph */		qgl_rdh_draw_glyph,
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

	const EGLDisplay display = egl_initialize(wanted_config);
	if (display == EGL_NO_DISPLAY)
		return NULL;

	const EGLContext context = egl_create_context(display, wanted_config, config);
	if (context == EGL_NO_CONTEXT)
		return NULL;

	const EGLSurface surface = egl_create_surface(display, context, config, 0);
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
	qn_return_when_fail(QN_TMASK(features, QGRENDERER_OPENGL), NULL);

	QglRdh* self = qn_alloc_zero_1(QglRdh);

	const char* name;
	QglConfig wanted_config;
	qgl_default_config(&wanted_config, flags);

#if defined QGL_MAYBE_GL_CORE
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

	// 이거 없으면 CORE에서 그냥 버텍스 관련 기능이 안된다 (3.0부터 되며, 당연히 ES3에서 됨)
	glGenVertexArrays(1, &self->vao);
	glBindVertexArray(self->vao);

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
	info->max_tex_count = qgl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);	// GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
	info->max_off_count = qgl_get_integer_v(GL_MAX_DRAW_BUFFERS);

	info->clr_fmt = qg_rgba_to_clrfmt(self->cfg.red, self->cfg.green, self->cfg.blue, self->cfg.alpha, false);

	info->enabled_stencil = self->cfg.stencil > 0;

	if (self->cfg.core)
	{
		const int major = info->shader_version / 100;
		const int minor = info->shader_version % 100;
		qn_snprintf(self->res.hdr_vertex, QN_COUNTOF(self->res.hdr_vertex), "#version %d%d0\n", major, minor);
		qn_snprintf(self->res.hdr_fragment, QN_COUNTOF(self->res.hdr_fragment), "#version %d%d0\n", major, minor);
	}
	else
	{
		self->res.hdr_vertex[0] = '\0';
		qn_strcpy(self->res.hdr_fragment, "precision mediump float;");
	}

	//
	qn_mesgf(VAR_CHK_NAME, "%s %d/%d [%s by %s]",
		name, info->renderer_version, info->shader_version, info->renderer, info->vendor);
	qn_mesgf(VAR_CHK_NAME, "%s / %s", gl_version, gl_shader_version);

#if false && defined _QN_EMSCRIPTEN_
	GLint num_ext = qgl_get_integer_v(GL_NUM_EXTENSIONS);
	if (num_ext > 0)
	{
		qn_mesgf(VAR_CHK_NAME, "Extensions:");
		for (int i = 0; i < num_ext; i++)
		{
			const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
			qn_mesgf(VAR_CHK_NAME, "  %s", ext);
		}
	}
#endif

	//
	return qn_gam_init(self, vt_qgl_rdh);
}

//
static void qgl_rdh_dispose(QnGam g)
{
	QglRdh* self = qn_cast_type(g, QglRdh);
	qn_return_on_ok(self->disposed, /*void*/);
	self->disposed = true;

	// 펜딩
	const QglPending* pd = &self->pd;
	qn_unload(pd->render.index_buffer);
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
		qn_unload(pd->render.vertex_buffers[i]);
	qn_unload(pd->render.render_state);
	for (size_t i = 0; i < QN_COUNTOF(pd->render.textures); i++)
		qn_unload(pd->render.textures[i]);

	// 리소스
	rdh_internal_clean();

	const QglResource* res = &self->res;
	qn_unload(res->white_texture);
	qn_unload(res->index_stream_rect);
	qgl_batch_stream_dispose((QglBatchStream*)res->batch_ortho);

	//
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &self->vao);

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

	RendererTransform* tm = RDH_TRANSFORM;
	const RendererInfo* info = RDH_INFO;

	QMMAT ortho = qm_mat4_ortho_lh((float)tm->size.Width, (float)-tm->size.Height, -1.0f, 1.0f);
	ortho.r[3] = qm_vec4(-1.0f, 1.0f, 0.0f, 1.0f);
	tm->ortho.s = ortho;

	// 뷰포트
	GLDEBUG(glViewport(0, 0, (GLsizei)tm->size.Width, (GLsizei)tm->size.Height));
#ifdef QGL_MAYBE_GL_CORE
	if (QGL_CORE && info->renderer_version < 400)
		GLDEBUG(glDepthRange(0.0f, 1.0f));
	else
		GLDEBUG(glDepthRangef(0.0f, 1.0f));
#else
	GLDEBUG(glDepthRangef(0.0f, 1.0f));
#endif
}

//
static void qgl_rdh_reset(void)
{
	rdh_internal_reset();

	const RendererInfo* info = RDH_INFO;
	RendererTransform* tm = RDH_TRANSFORM;
	QglSession* ss = QGL_SESSION;
	uint i;

	// 펜딩

	// 세션
	const uint amask = ss->shader.attr_mask;
	ss->shader.program = 0;
	ss->shader.attr_mask = 0;
	qn_zero(ss->shader.layouts, QGLOU_MAX_SIZE, QglLayoutDesc);
	ss->buffer.vertex = 0;
	ss->buffer.index = 0;
	ss->buffer.uniform = 0;
	ss->state.depth = QGDEPTH_LE;
	ss->state.stencil = QGSTENCIL_OFF;
	ss->state.alpha_to_coverage = false;
	qn_zero(ss->state.blends, QGRVS_MAX_VALUE, QglBlendDesc);
	qn_zero_1(&ss->state.rasz);

	// 트랜스폼
	tm->frm = qgl_mat4_irrcht_texture(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	// 장치 설정
	GLDEBUG(glPixelStorei(GL_PACK_ALIGNMENT, 1));
	GLDEBUG(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

	// 뎁스 스텐실
	GLDEBUG(glEnable(GL_DEPTH_TEST));
	GLDEBUG(glDepthMask(GL_TRUE));
	GLDEBUG(glDepthFunc(GL_LEQUAL));

	if (info->enabled_stencil)
	{
		GLDEBUG(glEnable(GL_STENCIL_TEST));
		GLDEBUG(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
		GLDEBUG(glStencilFunc(GL_ALWAYS, 1, 0xFF));
		GLDEBUG(glStencilMask(0xFF));
	}
	GLDEBUG(glDisable(GL_STENCIL_TEST));

	// 블렌드
#ifdef QGL_MAYBE_GL_CORE
	if (QGL_CORE && info->renderer_version >= 400)
	{
		for (i = 0; i < info->max_off_count; i++)
		{
			GLDEBUG(glEnablei(GL_BLEND, i));
			GLDEBUG(glBlendEquationi(i, GL_FUNC_ADD));
			GLDEBUG(glBlendFunci(i, GL_ONE, GL_ZERO));

			GLDEBUG(glDisablei(GL_BLEND, i));
			GLDEBUG(glColorMaski(i, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		}
	}
	else
#endif
	{
		GLDEBUG(glEnable(GL_BLEND));
		GLDEBUG(glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD));
		GLDEBUG(glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO));

		GLDEBUG(glDisable(GL_BLEND));
		GLDEBUG(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	}

	// 래스터라이저
	GLDEBUG(glEnable(GL_CULL_FACE));
	GLDEBUG(glCullFace(GL_BACK));
	GLDEBUG(glDisable(GL_CULL_FACE));

	GLDEBUG(glDisable(GL_POLYGON_OFFSET_FILL));
#ifdef QGL_MAYBE_GL_CORE
	if (QGL_CORE)
	{
		GLDEBUG(glDisable(GL_POLYGON_OFFSET_LINE));
		GLDEBUG(glDisable(GL_POLYGON_OFFSET_POINT));
	}
#endif

	// 텍스쳐
	for (i = 0; i < info->max_tex_count; i++)
	{
		GLDEBUG(glActiveTexture(GL_TEXTURE0 + i));
		GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
	}

	// 세이더
	for (i = 0; i < info->max_layout_count; i++)
	{
		if (QN_TBIT(amask, i))
			GLDEBUG(glDisableVertexAttribArray((GLuint)i));
	}
	GLDEBUG(glUseProgram(0));

	// 가위질
	GLDEBUG(glDisable(GL_SCISSOR_TEST));
	GLDEBUG(glFrontFace(GL_CW));

#ifdef QGL_MAYBE_GL_CORE
	if (QGL_CORE)
	{
		GLDEBUG(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		GLDEBUG(glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST));
		GLDEBUG(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
		if (info->renderer_version >= 400)
			GLDEBUG(glPatchParameteri(GL_PATCH_VERTICES, 3));
	}
#endif

	// 리소스
	static char vs_ortho[] = \
		"uniform mat4 OrthoProj;" \
		"attribute vec3 aPosition;" \
		"attribute vec2 aCoord;" \
		"attribute vec4 aColor;" \
		"varying vec2 vCoord;" \
		"varying vec4 vColor;" \
		"void main()" \
		"{" \
		"	gl_Position = OrthoProj * vec4(aPosition.xyz, 1.0);" \
		"	vCoord = aCoord;"\
		"	vColor = aColor;" \
		"}";
	static char ps_ortho[] = \
		"uniform sampler2D Texture;" \
		"varying vec2 vCoord;" \
		"varying vec4 vColor;" \
		"void main()" \
		"{" \
		"	gl_FragColor = texture2D(Texture, vCoord) * vColor;\n" \
		"}";
	static char ps_glyph[] = \
		"uniform sampler2D Texture;" \
		"varying vec2 vCoord;" \
		"varying vec4 vColor;" \
		"void main()" \
		"{" \
		"	float a = texture2D(Texture, vCoord).a;"\
		"	gl_FragColor = vec4(vColor.bgr, vColor.a * a);" \
		"}";
	static QgLayoutInput inputs_ortho[] =
	{
		{ QGLOS_1, QGLOU_POSITION, QGLOT_FLOAT3, false },
		{ QGLOS_1, QGLOU_COORD1, QGLOT_FLOAT2, false },
		{ QGLOS_1, QGLOU_COLOR1, QGLOT_BYTE4, true },
	};
	static QgPropRender render_ortho = QGL_PROP_RENDER_BLEND;

	// 텍스쳐 => 리셋하면 다시 만들어야 함
	QglResource* res = QGL_RESOURCE;
	qn_unload(res->white_texture);
	res->white_texture = (QglTexture*)qg_create_texture("qg_white", qg_create_image_filled(2, 2, &QMCOLOR_WHITE.s), QGTEXF_DISCARD_IMAGE);

	// 평면용 렌더 스테이트 => 한번만 만들어도 됨
	if (res->ortho_render == NULL)
	{
		const QgPropShader shader_ortho = { { QN_COUNTOF(inputs_ortho), inputs_ortho }, { 0, vs_ortho }, { 0, ps_ortho } };
		res->ortho_render = (QglRenderState*)qg_create_render_state("qg_ortho", &render_ortho, &shader_ortho);
		qn_unload(res->ortho_render);
	}

	// 평면용 글자 스테이트 => 한번만 만들어도 됨
	if (res->ortho_glyph == NULL)
	{
		const QgPropShader shader_glyph = { { QN_COUNTOF(inputs_ortho), inputs_ortho }, { 0, vs_ortho }, { 0, ps_glyph } };
		res->ortho_glyph = (QglRenderState*)qg_create_render_state("qg_glyph", &render_ortho, &shader_glyph);
		qn_unload(res->ortho_glyph);
	}

	// 배치 스트림 => 리셋하면 다시 만들어야 함
	qgl_batch_stream_dispose((QglBatchStream*)res->batch_ortho);
	res->batch_ortho = qgl_create_batch_ortho();

	qn_unload(res->index_stream_rect);
	ushort* index16 = qn_alloc(6 * QGL_MAX_BATCH_ITEM, ushort), * keep_index16 = index16;
	for (i = 0; i < QGL_MAX_BATCH_ITEM; i++, index16 += 6)
	{
		const ushort base = (ushort)(i * 4);
		index16[0] = base + 0;
		index16[1] = base + 1;
		index16[2] = base + 2;
		index16[3] = base + 1;
		index16[4] = base + 3;
		index16[5] = base + 2;
	}
	res->index_stream_rect = (QglBuffer*)qgl_create_buffer(QGBUFFER_INDEX, QGL_MAX_BATCH_ITEM * 6, sizeof(ushort), keep_index16);
	qn_free(keep_index16);
}

// 지우기
static void qgl_rdh_clear(QgClear flags)
{
	// 도움: https://open.gl/depthstencils
	const RendererInfo* info = RDH_INFO;
	const RendererParam* param = RDH_PARAM;
	QglSession* ss = QGL_SESSION;
	GLbitfield cf = 0;

	if (QN_TMASK(flags, QGCLEAR_STENCIL) && info->enabled_stencil)
	{
		GLDEBUG(glClearStencil(0));
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_DEPTH))
	{
		if (ss->state.depth == QGDEPTH_OFF)
		{
			ss->state.depth = QGDEPTH_LE;
			GLDEBUG(glDepthMask(GL_TRUE));
			GLDEBUG(glDepthFunc(GL_LESS));
		}
#ifdef QGL_MAYBE_GL_CORE
		if (QGL_CORE && info->renderer_version < 400)
			GLDEBUG(glClearDepth(1.0f));
		else
			GLDEBUG(glClearDepthf(1.0f));
#else
		GLDEBUG(glClearDepthf(1.0f));
#endif
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_RENDER))
	{
		const QmVec4 c = param->bgc;
		GLDEBUG(glClearColor(c.X, c.Y, c.Z, c.W));
		cf |= GL_COLOR_BUFFER_BIT;
	}

	if (cf != 0)
		GLDEBUG(glClear(cf));
}

// 시작
static bool qgl_rdh_begin(bool clear)
{
	if (clear)
		qgl_rdh_clear((QgClear)(QGCLEAR_DEPTH | QGCLEAR_STENCIL | QGCLEAR_RENDER));

	const QglResource* res = QGL_RESOURCE;
	qgl_batch_stream_clear((QglBatchStream*)res->batch_ortho);

	return true;
}

// 끝
static void qgl_rdh_end(void)
{
	const QglResource* res = QGL_RESOURCE;
	qgl_batch_ortho_flush((QglBatchStream*)res->batch_ortho);
}

// 플러시
static void qgl_rdh_flush(void)
{
	glFlush();
	const QglRdh* self = QGL_RDH;
#ifdef QGL_MAYBE_GL_CORE
	if (gl_swap_buffers() == false)
#endif
	{
		if (self->egl.display && self->egl.surface)
			eglSwapBuffers(self->egl.display, self->egl.surface);
	}
}

// 정점 버퍼 설정, stage에 대한 오류 설정은 rdh에서 하고 왔을 거임
static bool qgl_rdh_set_vertex(QgLayoutStage stage, void* buffer)
{
	QglBuffer* buf = qn_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (pd->render.vertex_buffers[stage] != buf)
	{
		qn_unload(pd->render.vertex_buffers[stage]);
		pd->render.vertex_buffers[stage] = qn_loadc(buf, QglBuffer);
	}
	return true;
}

// 인덱스 버퍼 설정
static bool qgl_rdh_set_index(void* buffer)
{
	QglBuffer* buf = qn_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (pd->render.index_buffer != buf)
	{
		qn_unload(pd->render.index_buffer);
		pd->render.index_buffer = qn_loadc(buf, QglBuffer);
	}
	return true;
}

// 렌더 설정
static bool qgl_rdh_set_render(void* render)
{
	QglRenderState* rdr = qn_cast_type(render, QglRenderState);
	QglPending* pd = QGL_PENDING;
	if (pd->render.render_state != rdr)
	{
		qn_unload(pd->render.render_state);
		pd->render.render_state = qn_loadc(rdr, QglRenderState);
	}
	return true;
}

// 텍스쳐 설정
static bool qgl_rdh_set_texture(int stage, void* texture)
{
	QglTexture* tex = qn_cast_type(texture, QglTexture);
	QglPending* pd = QGL_PENDING;
	if (pd->render.textures[stage] != tex)
	{
		qn_unload(pd->render.textures[stage]);
		pd->render.textures[stage] = qn_loadc(tex, QglTexture);
	}
	return true;
}

// 정점 바인딩
INLINE void qgl_bind_vertex_buffer(const QglBuffer* buffer)
{
	qn_debug_assert(buffer->base.type == QGBUFFER_VERTEX, "try to bind non-vertex buffer");
	QglSession* ss = QGL_SESSION;
	const GLuint gl_id = buffer == NULL ? 0 : qn_get_gam_desc(buffer, GLuint);
	if (ss->buffer.vertex != gl_id)
	{
		GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, gl_id));
		ss->buffer.vertex = gl_id;
	}
}

// 인덱스 바인딩
INLINE void qgl_bind_index_buffer(const QglBuffer* buffer)
{
	qn_debug_assert(buffer->base.type == QGBUFFER_INDEX, "try to bind non-index buffer");
	QglSession* ss = QGL_SESSION;
	const GLuint gl_id = buffer == NULL ? 0 : qn_get_gam_desc(buffer, GLuint);
	if (ss->buffer.index != gl_id)
	{
		GLDEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_id));
		ss->buffer.index = gl_id;
	}
}

// 유니폼 바인딩
INLINE void qgl_bind_uniform_buffer(const QglBuffer* buffer)
{
	qn_debug_assert(buffer->base.type == QGBUFFER_CONSTANT, "try to bind non-uniform buffer");
	QglSession* ss = QGL_SESSION;
	const GLuint gl_id = buffer == NULL ? 0 : qn_get_gam_desc(buffer, GLuint);
	if (ss->buffer.uniform != gl_id)
	{
		GLDEBUG(glBindBuffer(GL_UNIFORM_BUFFER, gl_id));
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
			return;
		case QGBUFFER_INDEX:
			qgl_bind_index_buffer(buffer);
			return;
		case QGBUFFER_CONSTANT:
			qgl_bind_uniform_buffer(buffer);
			return;
	}
	qn_mesgb(VAR_CHK_NAME, "invalid buffer type");
}

// 오토 세이더 변수 (오토가 아니면 RDH의 사용자 함수로 떠넘긴다)
static void qgl_process_shader_variable(const QgVarShader* var)
{
	const RendererTransform* tm = RDH_TRANSFORM;
	const RendererParam* param = RDH_PARAM;
	switch ((size_t)var->scauto)
	{
		case QGSCA_ORTHO_PROJ:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, tm->ortho.f));
			break;
		case QGSCA_WORLD:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, tm->world.f));
			break;
		case QGSCA_VIEW:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, tm->view.f));
			break;
		case QGSCA_PROJ:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, tm->proj.f));
			break;
		case QGSCA_VIEW_PROJ:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, tm->vwpr.f));
			break;
		case QGSCA_PROJ_VIEW:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, tm->prvw.f));
			break;
		case QGSCA_INV_VIEW:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, tm->invv.f));
			break;
		case QGSCA_MVP:
		{
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			//const QMMAT m = qm_mat4_tmul(tm->world.s, tm->vwpr.s);	// 이건 DirectX 방식
			const QMMAT m = qm_mat4_mul(tm->prvw.s, tm->world.s);		// OpenGL 방식
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, ((const QmMat4*)&m)->f));
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
					GLDEBUG(glUniform1i(var->offset, var->scauto - QGSCA_TEX1));
					break;
				default:
					qn_mesgb(VAR_CHK_NAME, "invalid auto shader texture");
					break;
			}
			break;
		case QGSCA_PROP_VEC1:
			qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			GLDEBUG(glUniform4fv(var->offset, 1, param->v[0].f));
			break;
		case QGSCA_PROP_VEC2:
			qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			GLDEBUG(glUniform4fv(var->offset, 1, param->v[1].f));
			break;
		case QGSCA_PROP_VEC3:
			qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			GLDEBUG(glUniform4fv(var->offset, 1, param->v[2].f));
			break;
		case QGSCA_PROP_VEC4:
			qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			GLDEBUG(glUniform4fv(var->offset, 1, param->v[3].f));
			break;
		case QGSCA_PROP_MAT1:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, param->m[0].f));
			break;
		case QGSCA_PROP_MAT2:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, param->m[1].f));
			break;
		case QGSCA_PROP_MAT3:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, param->m[2].f));
			break;
		case QGSCA_PROP_MAT4:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			GLDEBUG(glUniformMatrix4fv(var->offset, 1, false, param->m[3].f));
			break;
		case QGSCA_MAT_PALETTE:
			qn_debug_verify(var->sctype == QGSCT_FLOAT16);
			GLDEBUG(glUniformMatrix4fv(var->offset, param->bone_count, false, (const GLfloat*)param->bone_ptr));
			break;
		case QGSCA_DIFFUSE:
			qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			GLDEBUG(glUniform4fv(var->offset, 1, param->diffuse.f));
			break;
		case QGSCA_SPECULAR:
			qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			GLDEBUG(glUniform4fv(var->offset, 1, param->specular.f));
			break;
		case QGSCA_AMBIENT:
			qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			GLDEBUG(glUniform4fv(var->offset, 1, param->ambient.f));
			break;
		case QGSCA_EMISSIVE:
			qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			GLDEBUG(glUniform4fv(var->offset, 1, param->emissive.f));
			break;
			/*
			case QGSCA_LIGHT_POS:
				qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
				GLDEBUG(glUniform4fv(var->offset, 1, param->light_pos.f));
				break;
			case QGSCA_LIGHT_DIR:
				qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
				GLDEBUG(glUniform4fv(var->offset, 1, param->light_dir.f));
				break;
			case QGSCA_LIGHT_COLOR:
				qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
				GLDEBUG(glUniform4fv(var->offset, 1, param->light_color.f));
				break;
			case QGSCA_LIGHT_ATTEN:
				qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
				GLDEBUG(glUniform4fv(var->offset, 1, param->light_atten.f));
				break;
			case QGSCA_LIGHT_SPOT:
				qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
				GLDEBUG(glUniform4fv(var->offset, 1, param->light_spot.f));
				break;
			case QGSCA_LIGHT_RANGE:
				qn_debug_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
				GLDEBUG(glUniform4fv(var->offset, 1, param->light_range.f));
				break;
			case QGSCA_LIGHT_COUNT:
				qn_debug_verify(var->sctype == QGSCT_INT1 && var->size == 1);
				GLDEBUG(glUniform1i(var->offset, param->light_count));
				break;
			*/
		default:
			if (param->callback_func != NULL)
				param->callback_func(param->callback_data, (nint)var->hash, var);
			break;
	}
}

// 세이더랑 레이아웃
static bool qgl_commit_shader_layout(const QglRenderState* rdr)
{
	const RendererInfo* info = RDH_INFO;
	const QglPending* pd = QGL_PENDING;
	QglSession* ss = QGL_SESSION;
	const size_t max_attrs = info->max_layout_count;

	// 프로그램 설정
	if (ss->shader.program != rdr->shader.program)
	{
		ss->shader.program = rdr->shader.program;
		GLDEBUG(glUseProgram(rdr->shader.program));
	}

	// 유니폼 값 넣고
	size_t s, i;
	QN_CTNR_FOREACH(rdr->shader.uniforms, 0, i)
	{
		const QgVarShader* var = qgl_uni_ctnr_nth_ptr(&rdr->shader.uniforms, i);
		qgl_process_shader_variable(var);
	}

	// 어트리뷰트 + 레이아웃
	uint aok = 0, amask = ss->shader.attr_mask;
	for (s = 0; s < QGLOS_MAX_VALUE; s++)
	{
		QglBuffer* buf = pd->render.vertex_buffers[s];
		if (buf == NULL)
			continue;
		qgl_bind_vertex_buffer(buf);

		const QglVarLayout* stages = rdr->layout.stages[s];
		const GLsizei gl_stride = (GLsizei)rdr->layout.strides[s];
		const size_t count = rdr->layout.counts[s];
		for (i = 0; i < count; i++)
		{
			const QglVarLayout* input = &stages[i];
			const GLint gl_attr = rdr->shader.usages[input->usage];
			if (gl_attr == 0xFF)
			{
				const char* name = qg_layout_usage_to_str(input->usage);
				qn_mesgfb(VAR_CHK_NAME, "shader attribute not found: %s", name);
				continue;
			}

			aok |= QN_BIT(gl_attr);
			if (QN_TBIT(amask, gl_attr) == false)
			{
				QN_SBIT(amask, gl_attr, true);
				GLDEBUG(glEnableVertexAttribArray(gl_attr));
			}

			const GLuint gl_offset = input->offset;
			QglLayoutDesc* desc = &ss->shader.layouts[input->usage];
			if (desc->buffer != buf ||
				desc->offset != gl_offset ||
				desc->format != input->format ||
				desc->stride != gl_stride ||
				desc->count != input->count ||
				desc->normalized != input->normalized)
			{
				GLDEBUG(glVertexAttribPointer(gl_attr, input->count, input->format, input->normalized, gl_stride, (GLvoid*)(size_t)gl_offset));
				desc->buffer = buf;
				desc->offset = gl_offset;
				desc->format = input->format;
				desc->stride = gl_stride;
				desc->count = input->count;
				desc->normalized = input->normalized;
			}
		}
	}

	// 레이아웃 정리
	uint after_mask = amask & ~aok;
	for (i = 0; i < max_attrs && after_mask; i++)
	{
		if (QN_TBIT(after_mask, 0))
		{
			QN_SBIT(amask, i, false);
			GLDEBUG(glDisableVertexAttribArray((GLuint)i));
		}
		after_mask >>= 1;
	}
	ss->shader.attr_mask = amask;

	return true;
}

// depth 함수 변환
static INLINE GLenum qgl_depth_to_enum(QgDepth func)
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
static void qgl_commit_depth_stencil(const QglRenderState* rdr)
{
	const RendererInfo* info = RDH_INFO;
	QglSession* ss = QGL_SESSION;

	// 뎁스
	if (ss->state.depth != rdr->depth)
	{
		if (rdr->depth == QGDEPTH_OFF)
		{
			if (ss->state.depth != QGDEPTH_OFF)
				GLDEBUG(glDisable(GL_DEPTH_TEST));
		}
		else
		{
			if (ss->state.depth == QGDEPTH_OFF)
				GLDEBUG(glEnable(GL_DEPTH_TEST));
			const GLenum gl_func = qgl_depth_to_enum(rdr->depth);
			GLDEBUG(glDepthFunc(gl_func));
		}
		ss->state.depth = rdr->depth;
	}

	// 스텐실
	if (info->enabled_stencil && ss->state.stencil != rdr->stencil)
	{
		if (rdr->stencil == QGSTENCIL_OFF)
		{
			GLDEBUG(glDisable(GL_STENCIL_TEST));
		}
		else
		{
			if (ss->state.stencil == QGSTENCIL_OFF)
				GLDEBUG(glEnable(GL_STENCIL_TEST));
			if (rdr->stencil == QGSTENCIL_WRITE)
			{
				// 보통으로 그릴 때 스텐실을 1로 채움
				GLDEBUG(glStencilFunc(GL_ALWAYS, 1, 0xFF));
				GLDEBUG(glStencilMask(0xFF));
			}
			else if (rdr->stencil == QGSTENCIL_EVADE)
			{
				// 스텐실 1로 채워진 부분은 그리지 않음 -> 외곽선 같은거 그릴때 (https://heinleinsgame.tistory.com/25)
				GLDEBUG(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
				GLDEBUG(glStencilMask(0));
			}
			else if (rdr->stencil == QGSTENCIL_OVER)
			{
				// 스텐실 1로 채워진 부분만 그림 -> 그림자/반사 같은거 그릴때 (https://open.gl/depthstencils)
				GLDEBUG(glStencilFunc(GL_EQUAL, 1, 0xFF));
				GLDEBUG(glStencilMask(0));
			}
		}
		ss->state.stencil = rdr->stencil;
	}
}

// 블랜드 & 래스터라이저 커밋
static void qgl_commit_blend_rasz(const QglRenderState* rdr)
{
	static QglBlendDesc gl_blend_values[] =
	{
		[QGBLEND_OFF] = { QGBLEND_OFF, GL_ONE, GL_ZERO, GL_FUNC_ADD, GL_ONE, GL_ZERO, GL_FUNC_ADD },
		[QGBLEND_BLEND] = { QGBLEND_BLEND, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD },
		[QGBLEND_ADD] = { QGBLEND_ADD, GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD, GL_ZERO, GL_ONE, GL_FUNC_ADD },
		[QGBLEND_SUB] = { QGBLEND_SUB, GL_SRC_ALPHA, GL_ONE, GL_FUNC_SUBTRACT, GL_ZERO, GL_ONE, GL_FUNC_SUBTRACT },
		[QGBLEND_REV_SUB] = { QGBLEND_REV_SUB, GL_SRC_ALPHA, GL_ONE, GL_FUNC_REVERSE_SUBTRACT, GL_ZERO, GL_ONE, GL_FUNC_REVERSE_SUBTRACT },
		[QGBLEND_MOD] = { QGBLEND_MOD, GL_ZERO, GL_SRC_COLOR, GL_FUNC_ADD, GL_ZERO, GL_ONE, GL_FUNC_ADD },
		[QGBLEND_MUL] = { QGBLEND_MUL, GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_ZERO, GL_ONE, GL_FUNC_ADD },
	};

	const RendererInfo* info = RDH_INFO;
	const QglPending* pd = QGL_PENDING;
	QglSession* ss = QGL_SESSION;
	const size_t max_off_count = QN_MIN(QGRVS_MAX_VALUE, info->max_off_count);

	// 블랜드
	const QgPropBlend* blend = &rdr->blend;
#ifdef QGL_MAYBE_GL_CORE
#ifdef GL_SAMPLE_ALPHA_TO_COVERAGE
	if (QGL_CORE)
	{
		if (blend->coverage != ss->state.alpha_to_coverage)
		{
			if (blend->coverage)
				GLDEBUG(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE));
			else
				GLDEBUG(glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE));
			ss->state.alpha_to_coverage = blend->coverage;
		}
	}
#endif
	if (QGL_CORE && info->renderer_version >= 300 && blend->separate)
	{
		for (GLuint i = 0; i < max_off_count; i++)
		{
			QglBlendDesc* ssbnd = &ss->state.blends[i];

			if (i > 0 && pd->off.buffer[i] == NULL)
			{
				if (ssbnd->state != QGBLEND_OFF)
				{
					GLDEBUG(glDisablei(GL_BLEND, i));
					ssbnd->state = QGBLEND_OFF;
				}
				continue;
			}

			const QgBlend state = blend->rb[i];
			if (state != ssbnd->state)
			{
				if (state == QGBLEND_OFF)
				{
					if (ssbnd->state != QGBLEND_OFF)
						GLDEBUG(glDisablei(GL_BLEND, i));
				}
				else
				{
					if (ssbnd->state == QGBLEND_OFF)
						GLDEBUG(glEnablei(GL_BLEND, i));

					const QglBlendDesc* value = &gl_blend_values[state];
					if (value->colorOp != ssbnd->colorOp || value->alphaOp != ssbnd->alphaOp)
						GLDEBUG(glBlendEquationSeparatei(i, value->colorOp, value->alphaOp));
					if (value->srcColor != ssbnd->srcColor || value->dstColor != ssbnd->dstColor ||
						value->srcAlpha != ssbnd->srcAlpha || value->dstAlpha != ssbnd->dstAlpha)
						GLDEBUG(glBlendFuncSeparatei(i, value->srcColor, value->dstColor, value->srcAlpha, value->dstAlpha));

					*ssbnd = *value;
				}

				ssbnd->state = QGBLEND_OFF;
			}
		}
	}
	else
#endif
	{
		QglBlendDesc* ssbnd = &ss->state.blends[0];
		const QgBlend state = blend->rb[0];
		if (state != ssbnd->state)
		{
			if (state == QGBLEND_OFF)
			{
				if (ssbnd->state != QGBLEND_OFF)
					GLDEBUG(glDisable(GL_BLEND));
			}
			else
			{
				if (ssbnd->state == QGBLEND_OFF)
					GLDEBUG(glEnable(GL_BLEND));

				const QglBlendDesc* value = &gl_blend_values[state];
				if (value->colorOp != ssbnd->colorOp || value->alphaOp != ssbnd->alphaOp)
					GLDEBUG(glBlendEquationSeparate(value->colorOp, value->alphaOp));
				if (value->srcColor != ssbnd->srcColor || value->dstColor != ssbnd->dstColor ||
					value->srcAlpha != ssbnd->srcAlpha || value->dstAlpha != ssbnd->dstAlpha)
					GLDEBUG(glBlendFuncSeparate(value->srcColor, value->dstColor, value->srcAlpha, value->dstAlpha));

				*ssbnd = *value;
			}

			ssbnd->state = QGBLEND_OFF;
		}
	}

	// 래스터라이저
	const QglRasterizerDesc* rasz = &rdr->rasz;
	QglRasterizerDesc* ssrz = &ss->state.rasz;
#ifdef QGL_MAYBE_GL_CORE
	if (QGL_CORE && ssrz->fill != rasz->fill)
	{
		ssrz->fill = rasz->fill;
		glPolygonMode(GL_FRONT_AND_BACK, rasz->fill);
	}
#endif
	if (ssrz->cull != rasz->cull)
	{
		if (rasz->cull == GL_NONE)
		{
			if (ssrz->cull != GL_NONE)
				GLDEBUG(glDisable(GL_CULL_FACE));
		}
		else
		{
			if (ssrz->cull == GL_NONE)
				GLDEBUG(glEnable(GL_CULL_FACE));
			GLDEBUG(glCullFace(rasz->cull));
		}
		ssrz->cull = rasz->cull;
	}
	if (ssrz->bias != rasz->bias)
	{
		if (rasz->bias == false)
		{
			if (ssrz->bias)
			{
				GLDEBUG(glDisable(GL_POLYGON_OFFSET_FILL));
#ifdef QGL_MAYBE_GL_CORE
				if (QGL_CORE)
				{
					GLDEBUG(glDisable(GL_POLYGON_OFFSET_LINE));
					GLDEBUG(glDisable(GL_POLYGON_OFFSET_POINT));
				}
#endif
			}
		}
		else if (qm_eqf(ssrz->depth_bias, rasz->depth_bias) == false || qm_eqf(ssrz->slope_scale, rasz->slope_scale) == false)
		{
			if (ssrz->bias == false)
			{
				GLDEBUG(glEnable(GL_POLYGON_OFFSET_FILL));
#ifdef QGL_MAYBE_GL_CORE
				if (QGL_CORE)
				{
					GLDEBUG(glEnable(GL_POLYGON_OFFSET_LINE));
					GLDEBUG(glEnable(GL_POLYGON_OFFSET_POINT));
				}
#endif
			}
			const float bias = rasz->depth_bias * (float)((1 << 24) - 1);
			glPolygonOffset(rasz->slope_scale, bias);

			ssrz->depth_bias = rasz->depth_bias;
			ssrz->slope_scale = rasz->slope_scale;
		}

		ssrz->bias = rasz->bias;
	}
}

// 렌더 커밋
static bool qgl_rdh_commit_render(void)
{
	const QglPending* pd = QGL_PENDING;
	const QglRenderState* rdr = pd->render.render_state;
	VAR_CHK_IF_NULL(rdr, false);

	if (qgl_commit_shader_layout(rdr) == false)
		return false;

	qgl_commit_depth_stencil(rdr);
	qgl_commit_blend_rasz(rdr);

	return true;
}

// 텍스쳐 바인딩
static void qgl_rdh_bind_texture(int stage)
{
	const QglPending* pd = QGL_PENDING;
	const QglTexture* tex = pd->render.textures[stage];
	QglSession* ss = QGL_SESSION;

	if (tex == NULL)
	{
		if (ss->texture.handle[stage] == GL_NONE)
			return;
		GLDEBUG(glActiveTexture((GLenum)(GL_TEXTURE0 + stage)));
		GLDEBUG(glBindTexture(ss->texture.target[stage], 0));

		ss->texture.handle[stage] = GL_NONE;
		ss->texture.target[stage] = GL_NONE;
		return;
	}

	const GLuint gl_id = qn_get_gam_desc(tex, GLuint);
	const GLuint gl_target = tex->gl_target;
	if (ss->texture.handle[stage] == gl_id && ss->texture.target[stage] == gl_target)
		return;

	GLDEBUG(glActiveTexture(GL_TEXTURE0 + stage));
	GLDEBUG(glBindTexture(tex->gl_target, gl_id));

	ss->texture.handle[stage] = gl_id;
	ss->texture.target[stage] = tex->gl_target;
}

// 텍스쳐 커밋
static void qgl_rdh_commit_texture(void)
{
	for (int i = 0; i < (int)QN_COUNTOF(QGL_PENDING->render.textures); i++)
		qgl_rdh_bind_texture(i);
}

// 토폴로지 변환
INLINE GLenum qgl_topology_to_enum(QgTopology tpg)
{
	static GLenum gl_topologies[] =
	{
		[QGTPG_POINT] = GL_POINTS,
		[QGTPG_LINE] = GL_LINES,
		[QGTPG_LINE_STRIP] = GL_LINE_STRIP,
		[QGTPG_TRI] = GL_TRIANGLES,
		[QGTPG_TRI_STRIP] = GL_TRIANGLE_STRIP,
		[QGTPGEX_TRI_FAN] = GL_TRIANGLE_FAN,
	};
	return gl_topologies[tpg];
}

// 그리기
static bool qgl_rdh_draw(QgTopology tpg, int vertices)
{
	qgl_rdh_commit_texture();
	if (qgl_rdh_commit_render() == false)
		return false;

	const GLenum gl_tpg = qgl_topology_to_enum(tpg);
	GLDEBUG(glDrawArrays(gl_tpg, 0, (GLsizei)vertices));
	return true;
}

// 그리기 인덱스
static bool qgl_rdh_draw_indexed(QgTopology tpg, int indices)
{
	const QglPending* pd = QGL_PENDING;
	const QglBuffer* index = pd->render.index_buffer;
	VAR_CHK_IF_NULL(index, false);

	qgl_rdh_commit_texture();
	if (qgl_rdh_commit_render() == false)
		return false;
	qgl_bind_index_buffer(index);

	const GLenum gl_tpg = qgl_topology_to_enum(tpg);
	const GLenum gl_stride = index->base.stride == sizeof(uint) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	GLDEBUG(glDrawElements(gl_tpg, (GLsizei)indices, gl_stride, NULL));
	return true;
}

// 스프라이트 그리기 -> 배치에 사각형으로 넣기
static void qgl_rdh_draw_sprite(const QmRect* bound, void* texture, const QmKolor color, const QMVEC* coord)
{
	const QglResource* res = QGL_RESOURCE;
	QglTexture* tex = texture != NULL ? texture : res->white_texture;
	const QmVec4* uv = (const QmVec4*)coord;

	OrthoVertex v[4];
	ortho_vertex_set_param(&v[0], (float)bound->Left, (float)bound->Top, 0.0f, uv->X, uv->Y, color);
	ortho_vertex_set_param(&v[1], (float)bound->Right, (float)bound->Top, 0.0f, uv->Z, uv->Y, color);
	ortho_vertex_set_param(&v[2], (float)bound->Left, (float)bound->Bottom, 0.0f, uv->X, uv->W, color);
	ortho_vertex_set_param(&v[3], (float)bound->Right, (float)bound->Bottom, 0.0f, uv->Z, uv->W, color);
	qg_batch_ortho_draw_rect(res->batch_ortho, v, tex);
}

// 회전 스프라이트 그리기 -> 배치에 사각형으로 넣기
static void qgl_rdh_draw_sprite_ex(const QmRect* bound, float angle, void* texture, const QmKolor color, const QMVEC* coord)
{
	const QglResource* res = QGL_RESOURCE;
	QglTexture* tex = texture != NULL ? texture : res->white_texture;
	const QmVec4* uv = (const QmVec4*)coord;

	QmVec2 tl, tr, bl, br;
	qm_rect_rotate(*bound, angle, &tl, &tr, &bl, &br);

	OrthoVertex v[4];
	ortho_vertex_set_param(&v[0], tl.X, tl.Y, 0.0f, uv->X, uv->Y, color);
	ortho_vertex_set_param(&v[1], tr.X, tr.Y, 0.0f, uv->Z, uv->Y, color);
	ortho_vertex_set_param(&v[2], bl.X, bl.Y, 0.0f, uv->X, uv->W, color);
	ortho_vertex_set_param(&v[3], br.X, br.Y, 0.0f, uv->Z, uv->W, color);
	qg_batch_ortho_draw_rect(res->batch_ortho, v, tex);
}

// 글자 그리기
static void qgl_rdh_draw_glyph(const QmRect* bound, void* texture, const QmKolor color, const QMVEC* coord)
{
	const QglResource* res = QGL_RESOURCE;
	QglTexture* tex = texture != NULL ? texture : res->white_texture;
	const QmVec4* uv = (const QmVec4*)coord;

	OrthoVertex v[4];
	ortho_vertex_set_param(&v[0], (float)bound->Left, (float)bound->Top, 0.0f, uv->X, uv->Y, color);
	ortho_vertex_set_param(&v[1], (float)bound->Right, (float)bound->Top, 0.0f, uv->Z, uv->Y, color);
	ortho_vertex_set_param(&v[2], (float)bound->Left, (float)bound->Bottom, 0.0f, uv->X, uv->W, color);
	ortho_vertex_set_param(&v[3], (float)bound->Right, (float)bound->Bottom, 0.0f, uv->Z, uv->W, color);
	qg_batch_ortho_draw_glyph(res->batch_ortho, v, tex);
}


//////////////////////////////////////////////////////////////////////////
// 배치

// 배치 제거
static void qgl_batch_stream_dispose(QglBatchStream* self)
{
	qn_return_when_fail(self != NULL, );
	for (size_t i = 0; i < QN_COUNTOF(self->vbuffer); i++)
		qn_unload(self->vbuffer[i]);
	qn_free(self);
}

// 아이템 깨끗이
static void qgl_batch_stream_clear(QglBatchStream* self)
{
	self->index = 0;
}

// 텍스쳐 소트
static int qgl_batch_stream_sort_texture(const void* a, const void* b)
{
	const QglBatchItem* ia = *(const QglBatchItem* const*)a;
	const QglBatchItem* ib = *(const QglBatchItem* const*)b;
#if false
	return ia->gl_tex == ib->gl_tex ?
		ia->index - ib->index :
		(nint)ia->gl_tex - (nint)ib->gl_tex;
#else
	return (int)ia->gl_tex - (int)ib->gl_tex;
#endif
}

// 평면 배치 만들기
static QglBatchStream* qgl_create_batch_ortho(void)
{
	QglBatchOrtho* self = qn_alloc_zero_1(QglBatchOrtho);
	self->base.cmd = QGBTC_NONE;
	for (size_t i = 0; i < QN_COUNTOF(self->base.vbuffer); i++)
		self->base.vbuffer[i] = (QglBuffer*)qgl_create_buffer(QGBUFFER_VERTEX, QGL_MAX_BATCH_ITEM * 4, sizeof(OrthoVertex), NULL);
	return (QglBatchStream*)self;
}

// 평면 그리기
static void qgl_batch_ortho_flush(QglBatchStream* batch)
{
	QglBatchOrtho* self = (QglBatchOrtho*)batch;
	qn_return_on_ok(self->base.index == 0, );

	const QglResource* res = QGL_RESOURCE;
	size_t i;
	QglBatchItemOrtho* item;

	// 텍스쳐 소트 및 그리기 설정
	const size_t count = self->base.index;
	qn_qsort(self->ptrs, count, sizeof(QglBatchItemOrtho*), qgl_batch_stream_sort_texture);

	QglBuffer* vbuffer = self->base.vbuffer[self->base.vindex++];
	if (self->base.vindex == QN_COUNTOF(self->base.vbuffer))
		self->base.vindex = 0;

	const size_t cmd_stride = self->base.stride;
	const size_t vertex_stride = sizeof(OrthoVertex) * cmd_stride;
	OrthoVertex* vertices = qgl_buffer_map(vbuffer, true);
	for (i = 0; i < count; i++, vertices += cmd_stride)
	{
		item = self->ptrs[i];
		memcpy(vertices, item->vertices, vertex_stride);
	}
	qgl_buffer_unmap(vbuffer);

	qgl_rdh_set_vertex(QGLOS_1, vbuffer);
	qgl_rdh_set_render(self->base.cmd == QGBTC_GLYPH ? res->ortho_glyph : res->ortho_render);
	qgl_rdh_commit_render();
	GLDEBUG(glActiveTexture(GL_TEXTURE0));

	// 명령 별로 그리기
	item = self->ptrs[0];
	GLsizei gl_checkpoint = 0, gl_draws;
	GLuint gl_tex = item->base.gl_tex;
	GLDEBUG(glBindTexture(GL_TEXTURE_2D, gl_tex));

	switch ((size_t)self->base.cmd)
	{
		case QGBTC_RECT:
		case QGBTC_GLYPH:
			qgl_rdh_set_index(res->index_stream_rect);
			for (i = 0; i < count; i++)
			{
				item = self->ptrs[i];
				if (gl_tex != item->base.gl_tex)
				{
					gl_draws = (GLsizei)(i - gl_checkpoint);
					GLDEBUG(glDrawElements(GL_TRIANGLES, gl_draws * 6, GL_UNSIGNED_SHORT, (GLvoid*)(gl_checkpoint * 6 * sizeof(ushort))));
					gl_checkpoint = (GLsizei)i;

					gl_tex = item->base.gl_tex;
					GLDEBUG(glBindTexture(GL_TEXTURE_2D, gl_tex));
				}
			}
			if ((size_t)gl_checkpoint < count)
			{
				gl_draws = (GLsizei)(count - gl_checkpoint);
				GLDEBUG(glDrawElements(GL_TRIANGLES, gl_draws * 6, GL_UNSIGNED_SHORT, (GLvoid*)(gl_checkpoint * 6 * sizeof(ushort))));
			}
			break;
		default:
			break;
	}

	// 정리
	QglSession* ss = QGL_SESSION;
	ss->texture.target[0] = GL_NONE;
	ss->texture.handle[0] = GL_NONE;

	qgl_batch_stream_clear(batch);
}

// 평면 아이템 얻기
static QglBatchItemOrtho* qgl_batch_ortho_item(QglBatchStream* batch)
{
	QglBatchOrtho* self = (QglBatchOrtho*)batch;
	if (self->base.index == QGL_MAX_BATCH_ITEM)
	{
		qgl_batch_ortho_flush(batch);
		self->base.index = 0;
	}

	const size_t index = self->base.index++;
	QglBatchItemOrtho* item = self->ptrs[index] = &self->items[index];
	item->base.index = (GLsizei)index;
	return item;
}

// 평면 사각형 그리기
static void qg_batch_ortho_draw_rect(QglBatchStream* batch, OrthoVertex* v4, QglTexture* tex)
{
	QglBatchOrtho* self = (QglBatchOrtho*)batch;
	if (self->base.cmd != QGBTC_RECT)
	{
		qgl_batch_ortho_flush(batch);
		self->base.cmd = QGBTC_RECT;
		self->base.stride = 4;
	}

	QglBatchItemOrtho* item = qgl_batch_ortho_item(batch);
	item->base.gl_tex = qn_get_gam_desc(tex, GLuint);
	memcpy(item->vertices, v4, sizeof(OrthoVertex) * 4);
}

// 평면 문자 그리기
static void qg_batch_ortho_draw_glyph(QglBatchStream* batch, OrthoVertex* v4, QglTexture* tex)
{
	QglBatchOrtho* self = (QglBatchOrtho*)batch;
	if (self->base.cmd != QGBTC_GLYPH)
	{
		qgl_batch_ortho_flush(batch);
		self->base.cmd = QGBTC_GLYPH;
		self->base.stride = 4;
	}

	QglBatchItemOrtho* item = qgl_batch_ortho_item(batch);
	item->base.gl_tex = qn_get_gam_desc(tex, GLuint);
	memcpy(item->vertices, v4, sizeof(OrthoVertex) * 4);
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "QGLBUFFER"

// 버퍼 제거
static void qgl_buffer_dispose(QnGam g)
{
	QglBuffer* self = qn_cast_type(g, QglBuffer);

	if (self->base.mapped)
	{
		if (QGL_CORE == false && RDH_VERSION == 200)
			qn_free(self->map_ptr);
		else
		{
			qgl_bind_buffer(self);
			GLDEBUG(glUnmapBuffer(self->gl_type));
		}
	}

	const GLuint gl_handle = qn_get_gam_desc(self, GLuint);
	GLDEBUG(glDeleteBuffers(1, &gl_handle));

	qn_free(self);
}

// 버퍼 잠그기
static void* qgl_buffer_map(void* g, bool synchronized)
{
	QglBuffer* self = qn_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEQ3(self, base, dynamic, false, NULL);

	QN_LOCK(self->lock);
	qgl_bind_buffer(self);
	if (QGL_CORE == false && RDH_VERSION == 200) // ES2 전용
	{
		if (self->map_ptr == NULL)
			self->map_ptr = qn_alloc(self->base.size, byte);
	}
	else
	{
		GLbitfield gl_bf = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
		if (synchronized == false)
			gl_bf |= GL_MAP_UNSYNCHRONIZED_BIT;
		GLDEBUG(self->map_ptr = glMapBufferRange(self->gl_type, 0, self->base.size, gl_bf));
	}

	self->base.mapped = true;
	return self->map_ptr;
}

// 버퍼 잠금 풀기
static bool qgl_buffer_unmap(void* g)
{
	QglBuffer* self = qn_cast_type(g, QglBuffer);

	qgl_bind_buffer(self);
	if (QGL_CORE == false && RDH_VERSION == 200)	// ES2 전용
		GLDEBUG(glBufferSubData(self->gl_type, 0, self->base.size, self->map_ptr));
	else
		GLDEBUG(glUnmapBuffer(self->gl_type));
	QN_UNLOCK(self->lock);

	self->base.mapped = false;
	return true;
}

// 버퍼에 데이터 한방에 설정
static bool qgl_buffer_data(void* g, int size, const void* data)
{
	const QglBuffer* self = qn_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEQ3(self, base, dynamic, false, false);

	qgl_bind_buffer(self);
	GLDEBUG(glBufferSubData(self->gl_type, 0, size <= 0 ? self->base.size : size, data));

	return true;
}

// 버퍼 만들기
static QgBuffer* qgl_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	// 우선 만들자
	QglSession* ss = QGL_SESSION;
	GLsizeiptr gl_size = (GLsizeiptr)count * (GLsizeiptr)stride;
	GLenum gl_type;
	if (type == QGBUFFER_VERTEX)
	{
		gl_type = GL_ARRAY_BUFFER;
		ss->buffer.vertex = 0;
	}
	else if (type == QGBUFFER_INDEX)
	{
		VAR_CHK_IF_COND(stride != 2 && stride != 4, "invalid index buffer stride. require 2 or 4", NULL);
		gl_type = GL_ELEMENT_ARRAY_BUFFER;
		ss->buffer.index = 0;
	}
	else if (type == QGBUFFER_CONSTANT)
	{
		gl_type = GL_UNIFORM_BUFFER;
		// 256 바이트 정렬
		gl_size = (gl_size + 255) & ~255;
		ss->buffer.uniform = 0;
	}
	else
	{
		qn_mesgb(VAR_CHK_NAME, "invalid buffer type");
		return NULL;
	}

	GLuint gl_id;
	GLDEBUG(glGenBuffers(1, &gl_id));
	GLDEBUG(glBindBuffer(gl_type, gl_id));

	const GLenum gl_usage = initial_data != NULL ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	GLDEBUG(glBufferData(gl_type, gl_size, initial_data, gl_usage));		// 여기까지 초기 메모리 설정 또는 데이터 넣기(STATIC)

	// 진짜 만듦
	QglBuffer* self = qn_alloc_zero_1(QglBuffer);
	qn_set_gam_desc(self, gl_id);
	self->base.type = type;
	self->base.size = (uint)gl_size;
	self->base.count = count;
	self->base.stride = (ushort)stride;
	self->base.dynamic = gl_usage == GL_DYNAMIC_DRAW;
	self->gl_type = gl_type;

	// VT 여기서 설정
	static QN_DECL_VTABLE(QGBUFFER) vt_qgl_buffer =
	{
		.base.name = VAR_CHK_NAME,
		.base.dispose = qgl_buffer_dispose,

		.map = qgl_buffer_map,
		.unmap = qgl_buffer_unmap,
		.data = qgl_buffer_data,
	};
	return qn_gam_init(self, vt_qgl_buffer);
}


//////////////////////////////////////////////////////////////////////////
// 렌더러

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "QGLRENDERSTATE"

//
static void qgl_render_delete_shader(const QglRenderState* self, bool check_rdh)
{
	QglSession* ss = QGL_SESSION;
	if (self->shader.program == 0)
		return;
	if (check_rdh && QGL_RDH->disposed == false && ss->shader.program == self->shader.program)
	{
		ss->shader.program = 0;
		GLDEBUG(glUseProgram(0));
	}
	if (self->shader.vertex != 0)
	{
		GLDEBUG(glDetachShader(self->shader.program, self->shader.vertex));
		GLDEBUG(glDeleteShader(self->shader.vertex));
	}
	if (self->shader.fragment != 0)
	{
		GLDEBUG(glDetachShader(self->shader.program, self->shader.fragment));
		GLDEBUG(glDeleteShader(self->shader.fragment));
	}
	GLDEBUG(glDeleteProgram(self->shader.program));
}

//
static void qgl_render_dispose(QnGam g)
{
	QglRenderState* self = qn_cast_type(g, QglRenderState);

	qgl_render_delete_shader(self, true);
	qgl_uni_ctnr_dispose(&self->shader.uniforms);
	qgl_attr_ctnr_dispose(&self->shader.attrs);
	qgl_layout_ctnr_dispose(&self->layout.inputs);

	rdh_internal_unlink_node(RDHNODE_RENDER, self);
	qn_free(self);
}

// 프로그램 상태값 얻기
INLINE GLint qgl_get_program_iv(GLuint program, GLenum name)
{
	GLint n;
	GLDEBUG(glGetProgramiv(program, name, &n));
	return n;
}

// 세이더 상태값 얻기
INLINE GLint qgl_get_shader_iv(GLuint handle, GLenum name)
{
	GLint n;
	GLDEBUG(glGetShaderiv(handle, name, &n));
	return n;
}

// 세이더 컴파일
static GLuint qgl_render_compile_shader(GLenum gl_type, const char* header, const char* code)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	const GLuint gl_shader = glCreateShader(gl_type); GLDEBUG((void)0);
	const char* codes[2] = { header, code };
	GLDEBUG(glShaderSource(gl_shader, 2, codes, NULL));
	GLDEBUG(glCompileShader(gl_shader));

	if (qgl_get_shader_iv(gl_shader, GL_COMPILE_STATUS) != GL_FALSE)
		return gl_shader;

	const char* type = gl_type == GL_VERTEX_SHADER ? "vertex" : gl_type == GL_FRAGMENT_SHADER ? "fragment" : "unknown";
	GLint gl_log_len = qgl_get_shader_iv(gl_shader, GL_INFO_LOG_LENGTH);
	if (gl_log_len <= 0)
		qn_mesgfb(VAR_CHK_NAME, "failed to %s shader compile", type);
	else
	{
		GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
		GLDEBUG(glGetShaderInfoLog(gl_shader, gl_log_len, &gl_log_len, gl_log));
		qn_mesgfb(VAR_CHK_NAME, "failed to %s shader compile: %s", type, gl_log);
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
static bool qgl_render_bind_shader(QglRenderState* self, const QgCodeData* vertex, const QgCodeData* fragment)
{
	// 프로그램이랑 세이더 만들고
	const QglResource* res = QGL_RESOURCE;
	const GLuint gl_vertex_shader = qgl_render_compile_shader(GL_VERTEX_SHADER, res->hdr_vertex, vertex->code);
	const GLuint gl_fragment_shader = qgl_render_compile_shader(GL_FRAGMENT_SHADER, res->hdr_fragment, fragment->code);
	if (gl_vertex_shader == 0 || gl_fragment_shader == 0)
	{
		if (gl_vertex_shader != 0)
			GLDEBUG(glDeleteShader(gl_vertex_shader));
		if (gl_fragment_shader != 0)
			GLDEBUG(glDeleteShader(gl_fragment_shader));
		return false;
	}

	self->shader.vertex = gl_vertex_shader;
	self->shader.fragment = gl_fragment_shader;
	self->shader.program = glCreateProgram(); GLDEBUG((void)0);
	GLDEBUG(glAttachShader(self->shader.program, gl_vertex_shader));
	GLDEBUG(glAttachShader(self->shader.program, gl_fragment_shader));

	// 링크
	GLDEBUG(glLinkProgram(self->shader.program));
	if (qgl_get_program_iv(self->shader.program, GL_LINK_STATUS) == GL_FALSE)
	{
		GLsizei gl_log_len = qgl_get_program_iv(self->shader.program, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
			qn_mesgb(VAR_CHK_NAME, "failed to link shader");
		else
		{
			GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
			GLDEBUG(glGetProgramInfoLog(self->shader.program, gl_log_len, &gl_log_len, gl_log));
			qn_mesgfb(VAR_CHK_NAME, "failed to link shader: %s", gl_log);
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
		qgl_uni_ctnr_init(&self->shader.uniforms, gl_count);
		for (index = i = 0; i < gl_count; i++)
		{
			GLDEBUG(glGetActiveUniform(self->shader.program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz));
			gl_index = glGetUniformLocation(self->shader.program, sz); GLDEBUG((void)0);
			if (gl_index < 0)
			{
				qn_mesgfb(VAR_CHK_NAME, "failed to get uniform location: %s", sz);
				continue;
			}

			const QgScType sctype = qgl_enum_to_shader_const(gl_type);
			if (sctype == QGSCT_UNKNOWN)
				qn_mesgfb(VAR_CHK_NAME, "unsupported uniform type: %s (type: %X)", sz, gl_type);

			QgVarShader* var = qgl_uni_ctnr_nth_ptr(&self->shader.uniforms, index++);
			qn_strcpy(var->name, sz);
			var->hash = qn_strihash(sz);
			var->offset = (ushort)glGetUniformLocation(self->shader.program, sz); GLDEBUG((void)0);
			var->size = (ushort)gl_size;
			var->sctype = sctype;
			var->scauto = qg_str_to_shader_const_auto(var->hash, sz);
			if (var->scauto == QGSCA_UNKNOWN)
				var->hash = (size_t)qn_sym(sz);
		}
	}

	// 어트리뷰트
	memset(self->shader.usages, 0xFF, QN_COUNTOF(self->shader.usages));
	gl_count = qgl_get_program_iv(self->shader.program, GL_ACTIVE_ATTRIBUTES);
	if (gl_count > 0)
	{
		qgl_attr_ctnr_init(&self->shader.attrs, gl_count);
		for (index = i = 0; i < gl_count; i++)
		{
			GLDEBUG(glGetActiveAttrib(self->shader.program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz));
			gl_index = glGetAttribLocation(self->shader.program, sz); GLDEBUG((void)0);
			if (gl_index < 0)
			{
				qn_mesgfb(VAR_CHK_NAME, "failed to get attribute location: %s", sz);
				continue;
			}

			const QgScType sctype = qgl_enum_to_shader_const(gl_type);
			if (sctype == QGSCT_UNKNOWN)
				qn_mesgfb(VAR_CHK_NAME, "unsupported attribute type: %s (type: %X)", sz, gl_type);

			QglVarAttr* var = qgl_attr_ctnr_nth_ptr(&self->shader.attrs, index++);
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
static bool qgl_render_bind_layout_input(QglRenderState* self, const QgLayoutData* layout)
{
	static byte lo_count[QGLOT_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4, 3, 2, 1,
		/* INT     */ 4, 3, 2, 1,
		/* HALF-F  */ 4, 2, 1, 1,
		/* HALF-I  */ 4, 2, 1, 1,
		/* BYTE    */ 4, 3, 2, 1, 1, 1, 2,
		/* USHORT  */ 1, 1, 1,
	};
	static GLenum lo_format[QGLOT_MAX_VALUE] =
	{
		/* UNKNOWN */ GL_NONE,
		/* FLOAT   */ GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT,
		/* INT     */ GL_INT, GL_INT, GL_INT, GL_INT,
		/* HALF-F  */ GL_HALF_FLOAT, GL_HALF_FLOAT, GL_HALF_FLOAT, GL_UNSIGNED_INT_10F_11F_11F_REV,
		/* HALF-I  */ GL_SHORT, GL_SHORT, GL_SHORT, GL_UNSIGNED_INT_2_10_10_10_REV,
		/* BYTE    */ GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT,
		/* USHORT  */ GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_4_4_4_4,
	};
	static byte lo_size[QGLOT_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4 * sizeof(float), 3 * sizeof(float), 2 * sizeof(float), 1 * sizeof(float),
		/* INT     */ 4 * sizeof(int), 3 * sizeof(int), 2 * sizeof(int), 1 * sizeof(int),
		/* HALF-F  */ 4 * sizeof(half_t), 2 * sizeof(half_t), 1 * sizeof(half_t), 1 * sizeof(int),
		/* HALF-I  */ 4 * sizeof(short), 2 * sizeof(short), 1 * sizeof(short), 1 * sizeof(int),
		/* BYTE    */ 4 * sizeof(byte), 3 * sizeof(byte), 2 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte), 2 * sizeof(ushort),
		/* USHORT  */ 2 * sizeof(ushort), 2 * sizeof(ushort), 2 * sizeof(ushort),
	};

	size_t i;

	// 갯수 및 내용 검사
	ushort loac[QGLOS_MAX_VALUE] = { 0, };
	for (i = 0; i < layout->count; i++)
	{
		const QgLayoutInput* input = &layout->inputs[i];
		if ((size_t)input->stage >= QGLOS_MAX_VALUE)
		{
			qn_mesgfb(VAR_CHK_NAME, "invalid layout stage: %d", input->stage);
			return false;
		}
		if ((size_t)input->format >= QN_COUNTOF(lo_count))
		{
			qn_mesgfb(VAR_CHK_NAME, "invalid layout format: %d", input->format);
			return false;
		}
		loac[input->stage]++;
	}

	// 레이아웃
	qgl_layout_ctnr_init(&self->layout.inputs, layout->count);
	QglVarLayout* pstage = qgl_layout_ctnr_data(&self->layout.inputs);
	QglVarLayout* stages[QGLOS_MAX_VALUE];
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
			qn_mesgfb(VAR_CHK_NAME, "cannot use or unknown layout format: %s", name);
			return false;
		}

		QglVarLayout* stage = stages[input->stage]++;
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

// 래스터라이저 채우기
static void qgl_render_set_rasterizer(QglRasterizerDesc* rasz, const QgPropRasterizer* source)
{
#if defined GL_FILL && defined GL_LINE && defined GL_LINE
	static const GLenum gl_fill_modes[] =
	{
		[QGFILL_SOLID] = GL_FILL,
		[QGFILL_WIRE] = GL_LINE,
		[QGFILL_POINT] = GL_POINT,
	};
#endif
	static const GLenum gl_cull_modes[] =
	{
		[QGCULL_NONE] = GL_NONE,
		[QGCULL_FRONT] = GL_FRONT,
		[QGCULL_BACK] = GL_BACK,
		[QGCULL_BOTH] = GL_FRONT_AND_BACK,
	};
#if defined GL_FILL && defined GL_LINE && defined GL_LINE
	rasz->fill = gl_fill_modes[source->fill];
#else
	rasz->fill = GL_NONE;
#endif
	rasz->cull = gl_cull_modes[source->cull];
	rasz->depth_bias = source->depth_bias;
	rasz->slope_scale = source->slope_scale;
	rasz->scissor = source->scissor;
	rasz->bias = qm_eqf(source->depth_bias, 0.0f) == false || qm_eqf(source->slope_scale, 0.0f) == false;
}

// 렌더 만들기. 오류 처리는 다하고 왔을 것이다
QgRenderState* qgl_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader)
{
	QglRenderState* self = qn_alloc_zero_1(QglRenderState);
	qn_node_set_name(qn_cast_type(self, QnGamNode), name);

	// 세이더
	if (qgl_render_bind_shader(self, &shader->vertex, &shader->pixel) == false)
		goto pos_error;

	// 레이아웃
	if (qgl_render_bind_layout_input(self, &shader->layout) == false)
		goto pos_error;

	// 속성
	self->depth = prop->depth;
	self->stencil = prop->stencil;

	self->blend = prop->blend;
	qgl_render_set_rasterizer(&self->rasz, &prop->rasterizer);

	//
	static QN_DECL_VTABLE(QNGAMBASE) vt_es_render =
	{
		.name = VAR_CHK_NAME,
		.dispose = qgl_render_dispose,
	};
	qn_gam_init(self, vt_es_render);
	if (name)
		rdh_internal_add_node(RDHNODE_RENDER, self);
	return qn_cast_type(self, QgRenderState);

pos_error:
	qgl_render_delete_shader(self, false);
	qgl_uni_ctnr_dispose(&self->shader.uniforms);
	qgl_attr_ctnr_dispose(&self->shader.attrs);
	qgl_layout_ctnr_dispose(&self->layout.inputs);
	qn_free(self);
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// 텍스쳐

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "QGLTEXTURE"

// 텍스쳐 제거
static void qgl_texture_dispose(QnGam g)
{
	QglTexture* self = qn_cast_type(g, QglTexture);
	const GLuint gl_handle = qn_get_gam_desc(self, GLuint);
	GLDEBUG(glDeleteTextures(1, &gl_handle));
	qn_free(self);
}

// 컬러 포맷을 텍스쳐 포맷으로
INLINE QglTexFormatDesc qgl_clrfmt_to_tex_enum(QgClrFmt fmt)
{
	// s3tc => dxt1, dxt3, dxt5
	// rgtc => bc4
	// bptc => bc7
	static QglTexFormatDesc gl_enums[QGCF_MAX_VALUE] =
	{
		[QGCF_UNKNOWN] = { GL_NONE, GL_NONE, GL_NONE },

		[QGCF_R32G32B32A32F] = { GL_RGBA32F, GL_RGBA, GL_FLOAT },
		[QGCF_R32G32B32F] = { GL_RGB32F, GL_RGB, GL_FLOAT },
		[QGCF_R32G32F] = { GL_RG32F, GL_RG, GL_FLOAT },
		[QGCF_R32F] = { GL_R32F, GL_RED, GL_FLOAT },

		[QGCF_R32G32B32A32] = { GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT },
		[QGCF_R32G32B32] = { GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT },
		[QGCF_R32G32] = { GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT },
		[QGCF_R32] = { GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT },

		[QGCF_R16G16B16A16F] = { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT },
		[QGCF_R16G16F] = { GL_RG16F, GL_RG, GL_HALF_FLOAT },
		[QGCF_R16F] = { GL_R16F, GL_RED, GL_HALF_FLOAT },
		[QGCF_R11G11B10F] = { GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV },

		[QGCF_R16G16B16A16] = { GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT },
		[QGCF_R16G16] = { GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT },
		[QGCF_R16] = { GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT },
		[QGCF_R10G10B10A2] = { GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV },

		[QGCF_R8G8B8A8] = { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE },
		[QGCF_R8G8B8] = { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE },
		[QGCF_R8G8] = { GL_RG8, GL_RG, GL_UNSIGNED_BYTE },
		[QGCF_R8] = { GL_R8, GL_RED, GL_UNSIGNED_BYTE },
		[QGCF_A8] = { GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE },
#if false
		[QGCF_L8] = { GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE },
		[QGCF_A8L8] = { GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE },
#else
		[QGCF_L8] = { GL_R8, GL_RED, GL_UNSIGNED_BYTE },
		[QGCF_A8L8] = { GL_RG8, GL_RG, GL_UNSIGNED_BYTE },
#endif

		[QGCF_R5G6B5] = { GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5 },
		[QGCF_R5G5B5A1] = { GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 },
		[QGCF_R4G4B4A4] = { GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 },

		[QGCF_D32F] = { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT },
		[QGCF_D24S8] = { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 },

		[QGCF_DXT1] = { GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_NONE, GL_NONE },
		[QGCF_DXT3] = { GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_NONE, GL_NONE },
		[QGCF_DXT5] = { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_NONE, GL_NONE },
#ifdef GL_COMPRESSED_RGB8_ETC1
		[QGCF_EXT1] = { GL_COMPRESSED_RGB8_ETC1, GL_NONE, GL_NONE },
#else
		[QGCF_EXT1] = { GL_NONE, GL_NONE, GL_NONE },
#endif
#ifdef GL_COMPRESSED_RGB8_ETC2
		[QGCF_EXT2] = { GL_COMPRESSED_RGB8_ETC2, GL_NONE, GL_NONE },
#else
		[QGCF_EXT2] = { GL_NONE, GL_NONE, GL_NONE },
#endif
#ifdef GL_COMPRESSED_RGBA8_ETC2_EAC
		[QGCF_EXT2_EAC] = { GL_COMPRESSED_RGBA8_ETC2_EAC, GL_NONE, GL_NONE },
#else
		[QGCF_EXT2_EAC] = { GL_NONE, GL_NONE, GL_NONE },
#endif
#ifdef GL_COMPRESSED_RGBA_ASTC_4x4
		[QGCF_ASTC4] = { GL_COMPRESSED_RGBA_ASTC_4x4, GL_NONE, GL_NONE },
#else
		[QGCF_ASTC4] = { GL_NONE, GL_NONE, GL_NONE },
#endif
#ifdef GL_COMPRESSED_RGBA_ASTC_8x8
		[QGCF_ASTC8] = { GL_COMPRESSED_RGBA_ASTC_8x8, GL_NONE, GL_NONE },
#else
		[QGCF_ASTC8] = { GL_NONE, GL_NONE, GL_NONE },
#endif
	};
	return gl_enums[fmt];
}

// 2D 텍스쳐
static QgTexture* qgl_create_texture(const char* name, const QgImage* image, QgTexFlag flags)
{
	QN_DUMMY(name);	// 어케할지 못정했음

	QglSession* ss = QGL_SESSION;
	const QgClrFmt fmt = image->prop.format;
	const QglTexFormatDesc gl_enum = qgl_clrfmt_to_tex_enum(fmt);
	if (gl_enum.ifmt == GL_NONE)
	{
		qn_mesgfb(VAR_CHK_NAME, "unsupported texture format: %s", qg_clrfmt_to_str(fmt));
		if (QN_TMASK(flags, QGTEXF_DISCARD_IMAGE))
			qn_unload(image);
		return NULL;
	}

	GLuint gl_id;
	GLDEBUG(glActiveTexture(GL_TEXTURE0));
	GLDEBUG(glGenTextures(1, &gl_id));
	GLDEBUG(glBindTexture(GL_TEXTURE_2D, gl_id));
	ss->texture.handle[0] = 0;
	ss->texture.target[0] = GL_TEXTURE_2D;

	int mip_count;
	byte* buffer = qn_get_gam_pointer(image);
	if (gl_enum.format != GL_NONE)
	{
		// 그냥 이미지. 밉맵은 없을 것이다
		qn_debug_verify(image->mipmaps == 1);
		mip_count = 1;
		GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, gl_enum.ifmt, image->width, image->height, 0, gl_enum.format, gl_enum.type, buffer),
			glDeleteTextures(1, &gl_id), NULL);
	}
	else
	{
		// 압축 이미지
		int mip_width = image->width, mip_height = image->height;
		mip_count = image->mipmaps;
		for (int i = 0; i < mip_count; i++)
		{
			const size_t mip_size = qg_calc_image_block_size(&image->prop, mip_width, mip_height);
			GLCHECK(glCompressedTexImage2D(GL_TEXTURE_2D, i, gl_enum.ifmt, mip_width, mip_height, 0, (GLsizei)mip_size, buffer),
				glDeleteTextures(1, &gl_id), NULL);
			buffer += mip_size;
			mip_width = qm_maxi(mip_width >> 1, 1);
			mip_height = qm_maxi(mip_height >> 1, 1);
		}
		flags |= QGTEXFS_COMPRESS;	// 압축 포맷이면 압축 플래그 추가 (압축 포맷이 아니면 무시됨)
	}

#ifdef GL_CLAMP_TO_BORDER
	const GLenum gl_wrap = QN_TMASK(flags, QGTEXF_CLAMP) ? GL_CLAMP_TO_EDGE : QN_TMASK(flags, QGTEXF_BORDER) ? GL_CLAMP_TO_BORDER : GL_REPEAT;
#else
	const GLenum gl_wrap = QN_TMASK(flags, QGTEXF_CLAMP) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
#endif
	GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_wrap));
	GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wrap));

	if (mip_count <= 1 && QN_TMASK(flags, QGTEXF_MIPMAP) && qm_pot(image->width) && qm_pot(image->height))
	{
		if (QN_TMASK(flags, QGTEXF_LINEAR | QGTEXF_ANISO))
		{
			if (QGL_CORE == false)
				GLDEBUG(glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST));
			GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
			GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		}
		else
		{
			if (QGL_CORE == false)
				GLDEBUG(glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST));
			GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
			GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		}
		glGenerateMipmap(GL_TEXTURE_2D);
		mip_count = 1 + (int)floor(log2(QN_MAX(image->width, image->height)));	// NOLINT
	}
	else
	{
		if (QN_TMASK(flags, QGTEXF_LINEAR | QGTEXF_ANISO))
		{
			GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mip_count > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
			GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		}
		else
		{
			GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mip_count > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST));
			GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		}
		QN_SMASK(flags, QGTEXF_MIPMAP, mip_count > 1);
	}

	QglTexture* self = qn_alloc_zero_1(QglTexture);
	qn_set_gam_desc(self, gl_id);
	self->base.prop = image->prop;
	self->base.width = image->width;
	self->base.height = image->height;
	self->base.mipmaps = mip_count;
	self->base.flags = flags | QGTEXSPEC_2D;
	self->gl_target = GL_TEXTURE_2D;
	self->gl_enum = gl_enum;

	if (QN_TMASK(flags, QGTEXF_DISCARD_IMAGE))
	{
		qn_unload(image);
		QN_SMASK(self->base.flags, QGTEXF_DISCARD_IMAGE, false);
	}

	static QN_DECL_VTABLE(QGTEXTURE) vt_qgl_texture =
	{
		.base.name = VAR_CHK_NAME,
		.base.dispose = qgl_texture_dispose,
	};
	return qn_gam_init(self, vt_qgl_texture);
}

#endif // USE_GL
