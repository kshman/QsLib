//
// qgrdh_es.c - OPENGL ES 렌더 디바이스
// 2024-01-10 by kim
//

#include "pch.h"
#ifdef USE_ES
#ifndef _QN_EMSCRIPTEN_
#define GLAD_EGL_IMPLEMENTATION		1
#define GLAD_GLES2_IMPLEMENTATION	1
#else
#include <emscripten/html5_webgl.h>
#endif // _QN_EMSCRIPTEN_
#include "qgrdh_es.h"
#include <limits.h>

#define ES_MAX_LAYOUT_COUNT		16
#define ES_VERSION_2			200
#define ES_VERSION_3			300


//////////////////////////////////////////////////////////////////////////
// OPENGL ES 렌더 디바이스

static void es_dispose(QsGam* g);
static void es_reset(void);
static void es_clear(int flags, const QmColor* color, int stencil, float depth);
static bool es_begin(bool clear);
static void es_end(void);
static void es_flush(void);
static QgBuffer* es_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data);
static bool es_set_index(QgBuffer* buffer);
static bool es_set_vertex(QgLayoutStage stage, QgBuffer* buffer);
static QgShader* es_create_shader(const char* name, size_t count, const QgLayoutInput* inputs);
static QgRender* es_create_render(const QgPropRender* prop, QgShader* shader);
static bool es_set_render(QgRender* render);
static bool es_draw(QgTopology tpg, int vertices);
static bool es_draw_indexed(QgTopology tpg, int indices);

static QglBuffer* esbuffer_new(QgBufferType type, uint count, uint stride, const void* initial_data);
static QglShader* esshader_new(const char* name, size_t count, const QgLayoutInput* inputs);
static bool esshader_link(QglShader* g);
static QglRender* esrender_new(const QgPropRender* prop, QgShader* shader);

qs_name_vt(RDHBASE) vt_es_rdh =
{
	.base.name = "ESRDH",
	.base.dispose = es_dispose,

	.reset = es_reset,
	.clear = es_clear,

	.begin = es_begin,
	.end = es_end,
	.flush = es_flush,

	.create_buffer = es_create_buffer,
	.create_shader = es_create_shader,
	.create_render = es_create_render,

	.set_index = es_set_index,
	.set_vertex = es_set_vertex,
	.set_render = es_set_render,

	.draw = es_draw,
	.draw_indexed = es_draw_indexed,
};

#ifndef ES_STATIC_LINK
//
static QnModule* es_egl_module = NULL;

//
static GLADapiproc es_load_egl(const char* name)
{
	return (GLADapiproc)qn_mod_func(es_egl_module, name);
}

//
static bool es_init_egl(void)
{
	static int version = 0;
	if (version != 0)
		return true;
	static const char* egllibs[] =
	{
#if defined _QN_WINDOWS_
		"libEGL",
#elif defined _QN_FREEBSD_ || defined _QN_LINUX_
		"libEGL.so",
#else
#error unknown EGL platform!
#endif
		NULL,
	};
	for (size_t i = 0; egllibs[i]; i++)
	{
		es_egl_module = qn_mod_load(egllibs[i], 1);
		if (es_egl_module != NULL)
			break;
	}
	if (es_egl_module == NULL)
	{
		qn_debug_outputs(true, "ESRDH", "cannot load egl library");
		return false;
	}
	version = gladLoadEGL(EGL_NO_DISPLAY, es_load_egl);
	if (version == 0)
	{
		qn_mod_unload(es_egl_module);
		return false;
	}
	return true;
}

//
static void es_reload_egl(EGLDisplay display)
{
	gladLoadEGL(display, es_load_egl);
}

//
static QnModule* es_api_module = NULL;

//
static GLADapiproc es_load_api(const char* name)
{
	return (GLADapiproc)qn_mod_func(es_api_module, name);
}

//
static bool es_init_api(int major)
{
	static int version = 0;
	if (version != 0)
		return true;
	static const char* es1libs[] =
	{
#if defined _QN_WINDOWS_
		"libGLESv1_CM",
#elif defined _QN_FREEBSD_ || defined _QN_LINUX_
		"libGLESv1_CM.so",
		"libGLES_CM.so",
#else
#error unknown EGL platform!
#endif
		NULL,
	};
	static const char* es2libs[] =
	{
#if defined _QN_WINDOWS_
		"libGLESv2",
#elif defined _QN_FREEBSD_ || defined _QN_LINUX_
		"libGLESv2.so",
#else
#error unknown EGL platform!
#endif
		NULL,
	};
	const char** apilibs = major == 1 ? es1libs : es2libs;
	for (size_t i = 0; apilibs[i]; i++)
	{
		es_api_module = qn_mod_load(apilibs[i], 1);
		if (es_api_module != NULL)
			break;
	}
	if (es_api_module == NULL)
	{
		qn_debug_outputs(true, "ESRDH", "cannot load es client library");
		return false;
	}
	version = gladLoadGLES2(es_load_api);
	if (version == 0)
	{
		qn_mod_unload(es_api_module);
		return false;
	}
	return true;
}
#endif // ES_STATIC_LINK

//
static const char* es_egl_error_string(EGLint error)
{
#define ERROR_CASE(x)		case x: return #x
	static char unknown[32];
	switch (error)
	{
		ERROR_CASE(EGL_SUCCESS);
		ERROR_CASE(EGL_NOT_INITIALIZED);
		ERROR_CASE(EGL_BAD_ACCESS);
		ERROR_CASE(EGL_BAD_ALLOC);
		ERROR_CASE(EGL_BAD_ATTRIBUTE);
		ERROR_CASE(EGL_BAD_CONTEXT);
		ERROR_CASE(EGL_BAD_CONFIG);
		ERROR_CASE(EGL_BAD_CURRENT_SURFACE);
		ERROR_CASE(EGL_BAD_DISPLAY);
		ERROR_CASE(EGL_BAD_SURFACE);
		ERROR_CASE(EGL_BAD_MATCH);
		ERROR_CASE(EGL_BAD_PARAMETER);
		ERROR_CASE(EGL_BAD_NATIVE_PIXMAP);
		ERROR_CASE(EGL_BAD_NATIVE_WINDOW);
		ERROR_CASE(EGL_CONTEXT_LOST);
		default:
			return qg_unknown_str(error);
	}
#undef ERROR_CASE
}

//
static EGLint es_get_config_attr(EGLDisplay display, EGLConfig config, EGLenum name)
{
	EGLint v;
	eglGetConfigAttrib(display, config, name, &v);
	return v;
}

//
static void es_get_config(EGLDisplay display, EGLConfig ec, EsConfig* config)
{
	config->handle = ec;
	config->red = es_get_config_attr(display, ec, EGL_RED_SIZE);
	config->green = es_get_config_attr(display, ec, EGL_GREEN_SIZE);
	config->blue = es_get_config_attr(display, ec, EGL_BLUE_SIZE);
	config->alpha = es_get_config_attr(display, ec, EGL_ALPHA_SIZE);
	config->depth = es_get_config_attr(display, ec, EGL_DEPTH_SIZE);
	config->stencil = es_get_config_attr(display, ec, EGL_STENCIL_SIZE);
	config->samples = es_get_config_attr(display, ec, EGL_SAMPLES);
	config->version = (es_get_config_attr(display, ec, EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES3_BIT) != 0 ? 3 : 2;
}

//
static const EsConfig* es_detect_config(const EsConfig* wanted, const EsConfig* configs, EGLint count)
{
	uint least_missing = UINT_MAX;
	uint least_color = UINT_MAX;
	uint least_extra = UINT_MAX;
	const EsConfig* found = NULL;
	for (EGLint i = 0; i < count; i++)
	{
		const EsConfig* c = &configs[i];

		uint missing = 0;
		if (wanted->alpha > 0 && c->alpha == 0) missing++;
		if (wanted->depth > 0 && c->depth == 0) missing++;
		if (wanted->stencil > 0 && c->stencil == 0) missing++;
		if (wanted->samples > 0 && c->samples == 0) missing++;

		uint color = 0;
		if (wanted->red > 0)
			color += (wanted->red - c->red) * (wanted->red - c->red);
		if (wanted->blue > 0)
			color += (wanted->blue - c->blue) * (wanted->blue - c->blue);
		if (wanted->green > 0)
			color += (wanted->green - c->green) * (wanted->green - c->green);

		uint extra = 0;
		if (wanted->alpha > 0)
			extra += (wanted->alpha - c->alpha) * (wanted->alpha - c->alpha);
		if (wanted->depth > 0)
			extra += (wanted->depth - c->depth) * (wanted->depth - c->depth);
		if (wanted->stencil > 0)
			extra += (wanted->stencil - c->stencil) * (wanted->stencil - c->stencil);
		if (wanted->samples > 0)
			extra += (wanted->samples - c->samples) * (wanted->samples - c->samples);
		if (wanted->version > 0)
			extra += (wanted->version - c->version) * (wanted->version - c->version);

		if (missing < least_missing)
			found = c;
		else if (missing == least_missing)
		{
			if (color < least_color || (color == least_color && extra < least_extra))
				found = c;
		}
		if (found == c)
		{
			least_missing = missing;
			least_color = color;
			least_extra = extra;
		}
	}
	return found;
}

//
static bool es_choose_config(EsRdh* self, const EsConfig* config, bool msaa)
{
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[i++] = e; attrs[i++] = v; }QN_STMT_END
	EGLint attrs[32], i = 0;
	ATTR_ADD(EGL_RED_SIZE, config->red);
	ATTR_ADD(EGL_GREEN_SIZE, config->green);
	ATTR_ADD(EGL_BLUE_SIZE, config->blue);
	if (config->alpha > 0)
		ATTR_ADD(EGL_ALPHA_SIZE, config->alpha);
	if (config->depth > 0)
		ATTR_ADD(EGL_DEPTH_SIZE, config->depth);
	if (config->stencil > 0)
		ATTR_ADD(EGL_STENCIL_SIZE, config->stencil);
	if (msaa && config->samples > 0)
		ATTR_ADD(EGL_SAMPLES, config->samples);
	ATTR_ADD(EGL_RENDERABLE_TYPE, config->version == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT);
	ATTR_ADD(EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER);
	ATTR_ADD(EGL_SURFACE_TYPE, EGL_WINDOW_BIT);
	ATTR_ADD(EGL_NONE, EGL_NONE);
	qn_verify(i < QN_COUNTOF(attrs));
#undef ATTR_ADD

	EGLint config_count = 0;
	EGLConfig* configs = qn_alloc(64, EGLConfig);
	if (eglChooseConfig(self->display, attrs, configs, 64, &config_count) == EGL_FALSE || config_count == 0)
	{
		qn_debug_outputf(true, "ESRDH", "failed to choose config: %s", es_egl_error_string(eglGetError()));
		qn_free(configs);
		return false;
	}

	bool ret;
	if (config_count == 1)
	{
		ret = true;
		es_get_config(self->display, configs[0], &self->config);
	}
	else
	{
		EsConfig* esconfigs = qn_alloc(config_count, EsConfig);
		for (i = 0; i < config_count; i++)
			es_get_config(self->display, configs[i], &esconfigs[i]);

		const EsConfig* found = es_detect_config(config, esconfigs, config_count);
		if (found == NULL)
			ret = false;
		else
		{
			self->config = *found;
			ret = true;
		}
		qn_free(esconfigs);
	}

	qn_free(configs);
	return ret;
}

//
RdhBase* es_allocator(QgFlag flags, QgFeature features)
{
	if (QN_TMASK(features, QGRENDERER_ES) == false)
		return NULL;

#ifndef ES_STATIC_LINK
	if (es_init_egl() == false)
		return NULL;
#endif // ES_STATIC_LINK

	//----- 설정 초기화. 프로퍼티에 있으면 가져온다
	bool msaa = QN_TMASK(flags, QGFLAG_MSAA);
	EsConfig config =
	{
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = qn_get_prop_int(QG_PROP_DEPTH_SIZE, 24, 4, 32),
		.stencil = qn_get_prop_int(QG_PROP_STENCIL_SIZE, 8, 4, 16),
		.samples = msaa ? qn_get_prop_int(QG_PROP_MSAA, 4, 0, 8) : 0,
		.version = 3,
	};
	const char* size_prop = qn_get_prop(QG_PROP_RGBA_SIZE);
	if (size_prop != NULL && strlen(size_prop) >= 4)
	{
		config.red = size_prop[0] - '0';
		config.green = size_prop[1] - '0';
		config.blue = size_prop[2] - '0';
		config.alpha = size_prop[3] - '0';
	}

	//----- 여기에 RDH가!
	EsRdh* self = qn_alloc_zero_1(EsRdh);
	self->native_window = (NativeWindowType)stub_system_get_window();
	self->native_display = (NativeDisplayType)stub_system_get_display();

#ifdef _QN_EMSCRIPTEN_
	// EMSCRIPTEN ES3 초기화 WEBGL 이용하는 듯
	EmscriptenWebGLContextAttributes emattrs =
	{
		.alpha = config.alpha > 0 ? EM_TRUE : EM_FALSE,
		.depth = config.depth > 0 ? EM_TRUE : EM_FALSE,
		.stencil = config.stencil > 0 ? EM_TRUE : EM_FALSE,
		.antialias = msaa ? EM_TRUE : EM_FALSE,
		//.premultipliedAlpha = 0,
		//.preserveDrawingBuffer = 0,
		.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT,
		//.failIfMajorPerformanceCaveat = 0,
		.majorVersion = 3,
		.minorVersion = 2,
		//.enableExtensionsByDefault = 0,
		//.explicitSwapControl = 0,	
	};
	emscripten_webgl_init_context_attributes(&emattrs);
	emattrs.majorVersion = 3;
	const char* canvas = stub_system_get_canvas();
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context = emscripten_webgl_create_context(canvas, &emattrs);
	emscripten_webgl_make_context_current(webgl_context);
#endif // _QN_EMSCRIPTEN_

	//----- EGL 초기화
	self->display = eglGetDisplay(self->native_display);
	if (self->display == EGL_NO_DISPLAY)
	{
		qn_debug_outputs(true, "ESRDH", "failed to get display");
		goto pos_fail_exit;
	}

	if (eglInitialize(self->display, NULL, NULL) == false)
	{
		qn_debug_outputf(true, "ESRDH", "failed to initialize: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}
#if !defined ES_STATIC_LINK
	es_reload_egl(self->display);	// eglInitialize 이후에나 버전을 알 수 있다. glad egl은 버전이 필요함
#endif

	// API 바인드
	if (eglBindAPI(EGL_OPENGL_ES_API) == false)
	{
		qn_debug_outputf(true, "ESRDH", "failed to initialize: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}

	// 컨피그 찾기
	if (es_choose_config(self, &config, msaa) == false)
		goto pos_fail_exit;

	// 컨텍스트 만들기
	EGLint context_attrs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, self->config.version,
		EGL_NONE, EGL_NONE,
	};
	self->context = eglCreateContext(self->display, self->config.handle, NULL, context_attrs);
	if (self->context == EGL_NO_CONTEXT)
	{
		context_attrs[1] = self->config.version == 2 ? 3 : 2;
		self->context = eglCreateContext(self->display, self->config.handle, NULL, context_attrs);
		if (self->context != EGL_NO_CONTEXT)
			goto pos_context_ok;

		qn_debug_outputf(true, "ESRDH", "failed to create context: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}
pos_context_ok:
	eglQueryContext(self->display, self->context, EGL_CONTEXT_CLIENT_VERSION, &self->version);

	// 서피스 만들기
#ifdef _QN_ANDROID_
	ANativeWindow_setBuffersGeometry(self->native_window, 0, 0, self->base.base.info.visual_id);
#endif
	const EGLint surface_attrs[] =
	{
		EGL_NONE, EGL_NONE,
	};
	self->surface = eglCreateWindowSurface(self->display, self->config.handle, self->native_window, surface_attrs);
	if (self->surface == EGL_NO_SURFACE)
	{
		qn_debug_outputf(true, "ESRDH", "failed to create surface: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}
#ifdef _QN_ANDROID_
	int android_format = ANativeWindow_getFormat(self->native_window);
	Android_SetFormat(self->base.base.info.visual_id, android_format);
#endif

	// 서피스를 만들었으니 여기서 윈도우 표시
	stub_system_actuate();

	// 커런트 만들고
	if (eglMakeCurrent(self->display, self->surface, self->surface, self->context) == false)
	{
		qn_debug_outputf(true, "ESRDH", "failed to make current: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}

#ifndef ES_STATIC_LINK
	if (es_init_api(self->version) == false)	// eglMakeCurrent 하고 나야 GL 드라이버가 로드된다
		goto pos_fail_exit;
#endif // ES_STATIC_LINK

	eglSwapInterval(self->display, QN_TMASK(flags, QGFLAG_VSYNC) ? 1 : 0);

	//----- 정보 설정
	RendererInfo* infos = &self->base.base.info;	// 여기서 RDH_INFO는 쓸 수 없다

	qgl_copy_string(infos->renderer, QN_COUNTOF(infos->renderer), GL_RENDERER);
	qgl_copy_string(infos->vendor, QN_COUNTOF(infos->vendor), GL_VENDOR);
	const char* gl_version = (const char*)glGetString(GL_VERSION);
	const char* gl_shader_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	infos->renderer_version = qgl_get_version(gl_version, "OPENGLES", "OPENGL ES");
	infos->shader_version = qgl_get_version(gl_shader_version, "OPENGL ES GLSL ES ", "OPENGL ES GLSL ");

	const int max_layout_count = qgl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);
	infos->max_layout_count = QN_MIN(max_layout_count, ES_MAX_LAYOUT_COUNT);
	infos->max_tex_dim = qgl_get_integer_v(GL_MAX_TEXTURE_SIZE);
	infos->max_tex_count = qgl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	infos->max_off_count = 1; //qgl_get_integer_v(GL_FRAMEBUFFER_BINDING);
	infos->clr_fmt = qg_rgba_to_clrfmt(config.red, config.green, config.blue, config.alpha, false);
	infos->max_off_count = 1;

	//
	qn_debug_outputf(false, "ESRDH", "OPENGLES %d/%d [%s by %s]",
		infos->renderer_version, infos->shader_version,
		infos->renderer, infos->vendor);
	qn_debug_outputf(false, "ESRDH", "%s", gl_version);
	qn_debug_outputf(false, "ESRDH", "%s", gl_shader_version);
	return qs_init(self, RdhBase, &vt_es_rdh);

pos_fail_exit:
	if (self->context != EGL_NO_CONTEXT)
		eglDestroyContext(self->display, self->context);
	if (self->surface != EGL_NO_SURFACE)
		eglDestroySurface(self->display, self->surface);
	if (self->display != EGL_NO_DISPLAY)
		eglTerminate(self->display);
	qn_free(self);
	return NULL;
}

//
static void es_dispose(QsGam* g)
{
	EsRdh* self = qs_cast_type(g, EsRdh);
	qn_ret_if_ok(self->base.disposed);

	//----- 펜딩
	const QglPending* pd = &self->base.pd;
	qs_unload(pd->render.index_buffer);
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qs_unload(pd->render.vertex_buffers[i]);
	qs_unload(pd->render.render);

	//----- 장치 제거
	if (self->context != EGL_NO_CONTEXT)
		eglDestroyContext(self->display, self->context);
	if (self->surface != EGL_NO_SURFACE)
		eglDestroySurface(self->display, self->surface);
	if (self->display != EGL_NO_DISPLAY)
		eglTerminate(self->display);

	self->base.disposed = true;
	rdh_internal_dispose();
}

//
static void es_reset(void)
{
	rdh_internal_reset();

	EsRdh* self = ES_RDH;
	const RendererInfo* info = RDH_INFO;
	RenderTransform* tm = RDH_TRANSFORM;

	//----- 펜딩

	//----- 세션
	QglSession* ss = &self->base.ss;
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
	tm->ortho = qm_mat4_ortho_lh(tm->size.Width, tm->size.Height, -1.0f, 1.0f);
	//qm_mat4_loc(&tm->ortho, -1.0f, 1.0f, 0.0f, false);
	tm->frm = qgl_mat4_irrcht_texture(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//----- 장치 설정
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 뎁스 스텐실
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_STENCIL_TEST);

	// 래스터라이즈
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDisable(GL_POLYGON_OFFSET_FILL);

	// 블렌드
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

	glDisable(GL_BLEND);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// 텍스쳐
	for (size_t i = 0; i < info->max_tex_count; i++)
	{
		glActiveTexture((GLenum)(GL_TEXTURE0 + i));
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// 세이더
	for (size_t i = 0; i < info->max_layout_count; i++)
	{
		if (QN_TBIT(amask, i))
			glDisableVertexAttribArray((GLuint)i);
	}
	glUseProgram(0);

	// 가위질
	glDisable(GL_SCISSOR_TEST);
	glFrontFace(GL_CW);
}

// 지우기
static void es_clear(int flags, const QmColor* color, int stencil, float depth)
{
	// 도움: https://open.gl/depthstencils
	GLbitfield cf = 0;

	if (QN_TMASK(flags, QGCLEAR_STENCIL))
	{
		glStencilMaskSeparate(GL_FRONT_AND_BACK, stencil);
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_DEPTH))
	{
		glDepthMask(GL_TRUE);
		glClearDepthf(depth);
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_RENDER))
	{
		if (color == NULL)
			color = &(RDH_PARAM->bgc);
		glClearColor(color->R, color->G, color->B, color->A);
		cf |= GL_COLOR_BUFFER_BIT;
	}

	if (cf != 0)
		glClear(cf);
}

// 시작
static bool es_begin(bool clear)
{
	if (clear)
		es_clear(QGCLEAR_DEPTH | QGCLEAR_STENCIL | QGCLEAR_RENDER, NULL, 0, 1.0f);
	return true;
}

// 끝
static void es_end(void)
{
}

// 플러시
static void es_flush(void)
{
	EsRdh* self = ES_RDH;

	glFlush();
#ifdef USE_SDL2
	SDL_GL_SwapWindow(self->window);
#else
	eglSwapBuffers(self->display, self->surface);
#endif
}

// 버퍼 만들기
static QgBuffer* es_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	QglBuffer* buf = esbuffer_new(type, count, stride, initial_data);
	// 관리할게 있다면 하고
	return qs_cast_type(buf, QgBuffer);
}

// 버퍼 바인딩
static void es_bind_buffer(QgBufferType type, GLuint gl_id)
{
	QglSession* ss = QGL_SESSION;
	switch (type)
	{
		case QGBUFFER_VERTEX:				// 정점 버퍼
			if (ss->buffer.vertex != gl_id)
			{
				glBindBuffer(GL_ARRAY_BUFFER, gl_id);
				ss->buffer.vertex = gl_id;
			}
			break;
		case QGBUFFER_INDEX:				// 인덱스 버퍼
			if (ss->buffer.index != gl_id)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_id);
				ss->buffer.index = gl_id;
			}
			break;
		case QGBUFFER_CONSTANT:				// 유니폼(상수) 버퍼
			if (ss->buffer.uniform != gl_id)
			{
				glBindBuffer(GL_UNIFORM_BUFFER, gl_id);
				ss->buffer.uniform = gl_id;
			}
		default:
			break;
	}
}

// 인덱스 버퍼 설정
static bool es_set_index(QgBuffer* buffer)
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

// 정점 버퍼 설정, stage에 대한 오류 설정은 rdh에서 하고 왔을 거임
static bool es_set_vertex(QgLayoutStage stage, QgBuffer* buffer)
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

// 세이더 만들기
static QgShader* es_create_shader(const char* name, size_t count, const QgLayoutInput* inputs)
{
	QglShader* shader = esshader_new(name, count, inputs);
	// 관리할게 있다면 하고
	return qs_cast_type(shader, QgShader);
}

// 렌더 만들기
static QgRender* es_create_render(const QgPropRender* prop, QgShader* shader)
{
	QglRender* render = esrender_new(prop, shader);
	// 관리할게 있다면 하고
	return qs_cast_type(render, QgRender);
}

// 렌더 설정
static bool es_set_render(QgRender* render)
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

// 오토 세이더 변수 (오토가 아니면 RDH의 사용자 함수로 떠넘긴다)
static void es_process_shader_variable(QglShader* shader, const QgVarShader* var)
{
	switch (var->scauto)
	{
		case QGSCA_ORTHO_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_TRANSFORM->ortho.l);
			break;
		case QGSCA_WORLD:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_TRANSFORM->world.l);
			break;
		case QGSCA_VIEW:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_TRANSFORM->view.l);
			break;
		case QGSCA_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_TRANSFORM->proj.l);
			break;
		case QGSCA_VIEW_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_TRANSFORM->view_proj.l);
			break;
		case QGSCA_INV_VIEW:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_TRANSFORM->invv.l);
			break;
		case QGSCA_WORLD_VIEW_PROJ:
		{
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			QmMat4 m = qm_mul(RDH_TRANSFORM->world, RDH_TRANSFORM->view_proj);
			glUniformMatrix4fv(var->offset, 1, false, m.l);
		} break;
		case QGSCA_TEX1:
			qn_debug_outputf(true, "ESRDH", "auto shader variable not supported: %d", var->scauto);
			break;
		case QGSCA_TEX2:
			qn_debug_outputf(true, "ESRDH", "auto shader variable not supported: %d", var->scauto);
			break;
		case QGSCA_TEX3:
			qn_debug_outputf(true, "ESRDH", "auto shader variable not supported: %d", var->scauto);
			break;
		case QGSCA_TEX4:
			qn_debug_outputf(true, "ESRDH", "auto shader variable not supported: %d", var->scauto);
			break;
		case QGSCA_TEX5:
			qn_debug_outputf(true, "ESRDH", "auto shader variable not supported: %d", var->scauto);
			break;
		case QGSCA_TEX6:
			qn_debug_outputf(true, "ESRDH", "auto shader variable not supported: %d", var->scauto);
			break;
		case QGSCA_TEX7:
			qn_debug_outputf(true, "ESRDH", "auto shader variable not supported: %d", var->scauto);
			break;
		case QGSCA_TEX8:
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
			glUniformMatrix4fv(var->offset, 1, false, RDH_PARAM->m[0].l);
			break;
		case QGSCA_PROP_MAT2:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_PARAM->m[1].l);
			break;
		case QGSCA_PROP_MAT3:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_PARAM->m[2].l);
			break;
		case QGSCA_PROP_MAT4:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, RDH_PARAM->m[3].l);
			break;
		case QGSCA_MAT_PALETTE:
			qn_verify(var->sctype == QGSCT_FLOAT16);
			glUniformMatrix4fv(var->offset, RDH_PARAM->bone_count, false, (const GLfloat*)RDH_PARAM->bone_ptr);
			break;
		default:
			if (RDH_PARAM->callback_func != NULL)
			{
				QgShader* param_shader = qs_cast_type(shader, QgShader);
				RDH_PARAM->callback_func(RDH_PARAM->callback_data, param_shader, var->offset, var);
			}
			break;
	}
}

// 세이더랑 레이아웃
static bool es_commit_shader_layout(QglShader* shd)
{
	const size_t max_attrs = RDH_INFO->max_layout_count;

	// 세이더
	if (shd == NULL)
	{
		if (QGL_SESSION->shader.program != 0)
		{
			QGL_SESSION->shader.program = 0;
			glUseProgram(0);
		}
		qn_debug_outputs(true, "ESRDH", "shader is empty");
		return false;
	}

	esshader_link(shd);
	GLuint gl_program = qs_get_desc(shd, GLuint);
	if (QGL_SESSION->shader.program != gl_program)
	{
		QGL_SESSION->shader.program = gl_program;
		glUseProgram(gl_program);
	}

	// 유니폼
	size_t s, i;
	qn_ctnr_foreach(&shd->uniforms, i)
	{
		const QgVarShader* var = &qn_ctnr_nth(&shd->uniforms, i);
		es_process_shader_variable(shd, var);
	}

	// 어트리뷰트 + 레이아웃
	uint aok = 0, amask = QGL_SESSION->shader.amask;
	for (s = 0; s < QGLOS_MAX_VALUE; s++)
	{
		QglBuffer* buf = QGL_PENDING->render.vertex_buffers[s];
		if (buf == NULL)
			continue;

		const size_t count = shd->base.layout.counts[s];
		const QglLayoutInput * stages = shd->stages[s];
		const GLsizei gl_stride = (GLsizei)shd->base.layout.strides[s];
		const GLuint gl_buf = qs_get_desc(buf, GLuint);
		es_bind_buffer(QGBUFFER_VERTEX, gl_buf);

		for (i = 0; i < count; i++)
		{
			const QglLayoutInput* input = &stages[i];
			const GLint gl_attr = shd->attr_index[input->usage];
			if (gl_attr == 0xFF)
			{
				const char* name = qg_layout_usage_to_str(input->usage);
				qn_debug_outputf(true, "ESRDH", "shader attribute not found: %s", name);
				continue;
			}

			aok |= QN_BIT(gl_attr);
			if (!QN_TBIT(amask, gl_attr))
			{
				QN_SBIT(&amask, gl_attr, true);
				glEnableVertexAttribArray(gl_attr);
			}

			const GLuint gl_offset = input->offset;
			QglLayoutProperty* lp = &QGL_SESSION->shader.lprops[input->usage];
			if (lp->buffer != gl_buf ||
				lp->offset != gl_offset ||
				lp->format != input->format ||
				lp->stride != gl_stride ||
				lp->count != input->count ||
				lp->normalized != input->normalized)
			{
				glVertexAttribPointer(gl_attr, input->count, input->format, input->normalized, gl_stride, (GLvoid*)(size_t)gl_offset);
				lp->buffer = gl_buf;
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

// 렌더 커밋
static bool es_commit_render(void)
{
	const QglRender* rdr = QGL_PENDING->render.render;
	if (rdr == NULL)
	{
		qn_debug_outputs(true, "ESRDH", "render is empty");
		return false;
	}

	if (es_commit_shader_layout(rdr->shader) == false)
		return false;

	return true;
}

// 그리기
static bool es_draw(QgTopology tpg, int vertices)
{
	if (es_commit_render() == false)
		return false;

	GLenum gl_tpg = qgl_topology_to_enum(tpg);
	glDrawArrays(gl_tpg, 0, (GLsizei)vertices);
	return true;
}

// 그리기 인덱스
static bool es_draw_indexed(QgTopology tpg, int indices)
{
	const QglBuffer* index = QGL_PENDING->render.index_buffer;
	if (index == NULL)
	{
		qn_debug_outputs(true, "ESRDH", "index buffer is empty");
		return false;
	}
	const GLenum gl_stride = index->base.stride == sizeof(uint) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	const GLuint gl_index = qs_get_desc(index, GLuint);
	es_bind_buffer(QGBUFFER_INDEX, gl_index);

	if (es_commit_render() == false)
		return false;

	GLenum gl_tpg = qgl_topology_to_enum(tpg);
	glDrawElements(gl_tpg, (GLsizei)indices, gl_stride, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
// ES 버퍼

//
static void esbuffer_dispose(QsGam* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	if (self->base.mapped)
	{
		if (RDH_INFO->renderer_version >= 300)
			glUnmapBuffer(self->gl_type);
		else
			qn_free(self->lock_pointer);
	}

	GLuint gl_handle = qs_get_desc(self, GLuint);
	glDeleteBuffers(1, &gl_handle);

	qn_free(self);
}

//
static void* esbuffer_map(QgBuffer* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	qn_val_if_fail(self->gl_usage == GL_DYNAMIC_DRAW, NULL);
	qn_assert(self->lock_pointer == NULL, "버퍼가 잠겨있는데요!");

	if (RDH_INFO->renderer_version < ES_VERSION_3)
	{
		// ES2
		self->lock_pointer = qn_alloc(self->base.size, byte);
	}
	else
	{
		// ES3
		es_bind_buffer(self->base.type, qs_get_desc(self, GLuint));
		self->lock_pointer = glMapBufferRange(self->gl_type, 0, self->base.size,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		return self->lock_pointer;
	}

	self->base.mapped = true;
	return self->lock_pointer;
}

//
static bool esbuffer_unmap(QgBuffer* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	qn_assert(self->lock_pointer != NULL, "버퍼가 안 잠겼는데요!");

	es_bind_buffer(self->base.type, qs_get_desc(self, GLuint));

	if (RDH_INFO->renderer_version >= ES_VERSION_3)
		glUnmapBuffer(self->gl_type);
	else
	{
		glBufferSubData(self->gl_type, 0, self->base.size, self->lock_pointer);
		qn_free(self->lock_pointer);
	}

	self->lock_pointer = NULL;
	self->base.mapped = false;
	return true;
}

static bool esbuffer_data(QgBuffer* g, const void* data)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	qn_val_if_fail(self->gl_usage == GL_DYNAMIC_DRAW, false);

	es_bind_buffer(self->base.type, qs_get_desc(self, GLuint));
	glBufferSubData(self->gl_type, 0, self->base.size, data);

	return true;
}

static QglBuffer* esbuffer_new(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	// 우선 만들자
	GLenum gl_type;
	if (type == QGBUFFER_VERTEX)
		gl_type = GL_ARRAY_BUFFER;
	else if (type == QGBUFFER_INDEX)
		gl_type = GL_ELEMENT_ARRAY_BUFFER;
	else if (type == QGBUFFER_CONSTANT)
		gl_type = GL_UNIFORM_BUFFER;
	else
		return NULL;

	GLuint gl_id;
	glGenBuffers(1, &gl_id);
	qn_val_if_fail(gl_id != 0, NULL);
	es_bind_buffer(type, gl_id);		// 여기까지 만들고 바인드

	GLsizeiptr gl_size = (GLsizeiptr)count * stride;
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
	static qs_name_vt(QGBUFFER) vt_es_buffer =
	{
		.base.name = "ESBUFFER",
		.base.dispose = esbuffer_dispose,

		.map = esbuffer_map,
		.unmap = esbuffer_unmap,
		.data = esbuffer_data,
	};
	return qs_init(self, QglBuffer, &vt_es_buffer);
}


//////////////////////////////////////////////////////////////////////////
// 세이더

//
static void esshader_dispose(QsGam* g)
{
	QglShader* self = qs_cast_type(g, QglShader);
	GLuint handle = qs_get_desc(self, GLuint);

	qgl_ref_handle_unload_shader(self->vertex, handle);
	qgl_ref_handle_unload_shader(self->fragment, handle);
	glDeleteProgram(handle);

	qn_ctnr_disp(&self->uniforms);
	qn_ctnr_disp(&self->attrs);
	qn_ctnr_disp(&self->inputs);

	qn_free(self);
}

//
static QglRefHandle* esshader_compile(GLenum gl_type, const char* src, GLint gl_len)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	GLuint gl_shader = glCreateShader(gl_type);
	qn_val_if_fail(gl_shader != 0, NULL);

	glShaderSource(gl_shader, 1, &src, gl_len == 0 ? NULL : &gl_len);
	glCompileShader(gl_shader);

	if (qgl_get_shader_iv(gl_shader, GL_COMPILE_STATUS) == GL_FALSE)
	{
		const char* type = gl_type == GL_VERTEX_SHADER ? "vertex" : gl_type == GL_FRAGMENT_SHADER ? "fragment" : "unknown";
		GLint gl_log_len = qgl_get_shader_iv(gl_shader, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
			qn_debug_outputf(true, "ESSHADER", "failed to compile %s shader", type);
		else
		{
			GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
			glGetShaderInfoLog(gl_shader, gl_log_len, NULL, gl_log);
			qn_debug_outputf(true, "ESSHADER", "failed to compile %s shader: %s", type, gl_log);
			qn_free(gl_log);
		}
		return NULL;
	}

	return qgl_ref_handle_new(gl_shader);
}

//
static bool esshader_bind_shader(QgShader* g, QgShader* shaderptr)
{
	QglShader* shader = qs_cast_type(shaderptr, QglShader);
	qn_val_if_fail(shader->vertex != NULL && shader->fragment != NULL, false);

	QglShader* self = qs_cast_type(g, QglShader);
	GLuint handle = qs_get_desc(self, GLuint);

	qgl_ref_handle_unload_shader(self->vertex, handle);
	self->vertex = qgl_ref_handle_load_shader(shader->vertex, handle);

	qgl_ref_handle_unload_shader(self->fragment, handle);
	self->fragment = qgl_ref_handle_load_shader(shader->fragment, handle);

	self->base.type = shader->base.type;
	self->linked = false;
	return true;
}

//
static bool esshader_bind_buffer(QgShader* g, QgShaderType type, const void* data, size_t size, QgScFlag flags)
{
	// TODO: 매크로
	// TODO: 포함
	// TODO: 보관 (이건 바인드가 아니고 전체)
	if (flags != QGSCF_TEXT)
	{
		if (flags == QGSCF_BINARY)
			qn_debug_outputs(true, "ESSHADER", "binary shader is not supported. use text compile");
		else
			return false;
	}

	QglShader* self = qs_cast_type(g, QglShader);
	GLuint handle = qs_get_desc(self, GLuint);

	if (type == QGSHADER_VS)
	{
		qgl_ref_handle_unload_shader(self->vertex, handle);
		self->vertex = esshader_compile(GL_VERTEX_SHADER, (const char*)data, (GLint)size);
		if (self->vertex == NULL)
			return false;
		glAttachShader(handle, self->vertex->handle);
	}
	else if (type == QGSHADER_PS)
	{
		qgl_ref_handle_unload_shader(self->fragment, handle);
		self->fragment = esshader_compile(GL_FRAGMENT_SHADER, (const char*)data, (GLint)size);
		if (self->fragment == NULL)
			return false;
		glAttachShader(handle, self->fragment->handle);
	}
	else
	{
		// 뭥미
		return false;
	}

	self->base.type |= type;
	self->linked = false;
	return true;
}

//
static bool esshader_link(QglShader* self)
{
	qn_val_if_ok(self->linked, true);
	qn_val_if_fail(self->vertex != NULL && self->fragment != NULL, false);

	// 링크
	GLuint gl_program = qs_get_desc(self, GLuint);
	glLinkProgram(gl_program);

	if (qgl_get_program_iv(gl_program, GL_LINK_STATUS) == GL_FALSE)
	{
		GLsizei gl_log_len = qgl_get_program_iv(gl_program, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
			qn_debug_outputs(true, "ESSHADER", "failed to link shader");
		else
		{
			GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
			glGetProgramInfoLog(gl_program, gl_log_len, NULL, gl_log);
			qn_debug_outputf(true, "ESSHADER", "failed to link shader: %s", gl_log);
			qn_free(gl_log);
		}
		return false;
	}

	// 분석
	GLint gl_count;
	GLchar sz[64];

	// 유니폼
	qn_ctnr_disp(&self->uniforms);
	gl_count = qgl_get_program_iv(gl_program, GL_ACTIVE_UNIFORMS);
	if (gl_count > 0)
	{
		qn_ctnr_init(QglCtnUniform, &self->uniforms, gl_count);
		for (GLint i = 0, index = 0; i < gl_count; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			glGetActiveUniform(gl_program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz);

			QgScType sct = qgl_enum_to_shader_const(gl_type);
			if (sct == QGSCT_UNKNOWN)
			{
				qn_debug_outputf(true, "ESSHADER", "unknown uniform type: %s (type: %X)", sz, gl_type);
				continue;
			}

			QgVarShader* u = &qn_ctnr_nth(&self->uniforms, index);
			qn_strcpy(u->name, sz);
			u->hash = qn_strihash(u->name);
			u->offset = (ushort)glGetUniformLocation(gl_program, sz);
			u->size = (ushort)gl_size;
			u->sctype = sct;
			u->scauto = qg_str_to_shader_const_auto(u->hash, u->name);
			index++;
		}
	}

	// 어트리뷰트
	memset(self->attr_index, 0xFF, QN_COUNTOF(self->attr_index) * sizeof(byte));
	qn_ctnr_disp(&self->attrs);
	gl_count = qgl_get_program_iv(gl_program, GL_ACTIVE_ATTRIBUTES);
	if (gl_count > 0)
	{
		qn_ctnr_init(QglCtnAttr, &self->attrs, gl_count);
		for (GLint i = 0, index = 0; i < gl_count; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			glGetActiveAttrib(gl_program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz);

			QgScType sct = qgl_enum_to_shader_const(gl_type);
			if (sct == QGSCT_UNKNOWN)
			{
				qn_debug_outputf(true, "ESSHADER", "not supported attribute type: %s (type: %X)", sz, gl_type);
				continue;
			}

			GLint gl_attr = glGetAttribLocation(gl_program, sz);
			if ((size_t)gl_attr >= ES_MAX_LAYOUT_COUNT)
			{
				qn_debug_outputf(true, "ESSHADER", "attributes overflow: %s (type: %X)", sz, gl_type);
				break;
			}

			QglVarAttr* a = &qn_ctnr_nth(&self->attrs, index);
			qn_strcpy(a->name, sz);
			a->hash = qn_strihash(a->name);
			a->attrib = gl_attr;
			a->size = gl_size;
			a->usage = qg_str_to_layout_usage(a->hash, a->name);
			a->sctype = sct;
			self->attr_index[a->usage] = (byte)gl_attr;
			index++;
		}
	}

	self->linked = true;
	return true;
}

//
static QglShader* esshader_new(const char* name, size_t count, const QgLayoutInput* inputs)
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

	// 입력 레이아웃 검사
	ushort loac[QGLOS_MAX_VALUE] = { 0, };
	for (i = 0; i < count; i++)
	{
		const QgLayoutInput* l = &inputs[i];
		if ((size_t)l->stage >= QGLOS_MAX_VALUE)
			return NULL;
		loac[l->stage]++;
	}

	// 세이더 만들고
	QglShader* self = qn_alloc_zero_1(QglShader);
	qs_set_desc(self, glCreateProgram());	// 오류 검사 안해도 되겠지?
	qn_ctnr_init(QglCtnLayoutInput, &self->inputs, count);

	// 입력 레이아웃
	QglLayoutInput* pstage = qn_ctnr_data(&self->inputs);
	QglLayoutInput* stages[QGLOS_MAX_VALUE];
	for (i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (loac[i] == 0)
		{
			stages[i] = NULL;
			continue;
		}
		self->base.layout.counts[i] = (ushort)loac[i];
		stages[i] = self->stages[i] = pstage;
		pstage += loac[i];
	}

	ushort strides[QGLOS_MAX_VALUE] = { 0, };
	for (i = 0; i < count; i++)
	{
		const QgLayoutInput* input = &inputs[i];
		const GLenum gl_format = lo_format[input->format];
		if (gl_format == GL_NONE)
		{
			const char* fmt_name = qg_clrfmt_to_str(input->format);
			qn_debug_outputf(true, "ESSHADER", "cannot use or unknown layout format: %s", fmt_name);
			continue;
		}

		QglLayoutInput* stage = stages[input->stage]++;
		stage->stage = input->stage;
		stage->usage = input->usage;
		stage->offset = strides[input->stage];
		stage->format = gl_format;
		stage->count = lo_count[input->format];
		stage->normalized = (GLboolean)input->normalize;
		strides[input->stage] += lo_size[input->format];
	}
	memcpy(self->base.layout.strides, strides, sizeof(strides));

	// 
	if (name != NULL)
		qn_strncpy(self->base.name, name, QN_COUNTOF(self->base.name) - 1);
	else
	{
		size_t n = qn_p_index();
		qn_snprintf(self->base.name, QN_COUNTOF(self->base.name), "SHADER%zu", n);
	}

	// 반환
	static qs_name_vt(QGSHADER) vt_es_shader =
	{
		.base.name = "ESSHADER",
		.base.dispose = esshader_dispose,

		.bind_shader = esshader_bind_shader,
		.bind_buffer = esshader_bind_buffer,
	};
	return qs_init(self, QglShader, &vt_es_shader);
}


//////////////////////////////////////////////////////////////////////////
// 렌더러

//
static void esrender_dispose(QsGam* g)
{
	QglRender* self = qs_cast_type(g, QglRender);
	qs_unload(self->shader);
	qn_free(self);
}

//
static QglRender* esrender_new(const QgPropRender* prop, QgShader* shader)
{
	QglRender* self = qn_alloc_zero_1(QglRender);

	// 세이더
	self->shader = qs_loadc(shader, QglShader);
	esshader_link(qs_cast_type(shader, QglShader));

	//
	QN_DUMMY(prop);

	//
	static qs_name_vt(QSGAM) vt_es_render =
	{
		.name = "ESRENDER",
		.dispose = esrender_dispose,
	};
	return qs_init(self, QglRender, &vt_es_render);
}

#endif // USE_ES
