﻿//
// qgrdh_es.c - OPENGL ES 렌더 디바이스
// 2024-01-10 by kim
//

#include "pch.h"
#ifdef USE_ES
#ifndef _QN_EMSCRIPTEN_
#ifndef USE_SDL2
#define GLAD_EGL_IMPLEMENTATION		1
#endif
#define GLAD_GLES2_IMPLEMENTATION	1
#endif
#include "qgrdh_es.h"
#include <limits.h>

#define ES_MAX_LAYOUT_COUNT		16

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
static bool es_set_vertex(QgLoStage stage, QgBuffer* buffer);
static QgShader* es_create_shader(const char* name);
static QgRender* es_create_render(const QgPropRender* prop, QgShader* shader);
static bool es_set_render(QgRender* render);

static QglBuffer* esbuffer_new(QgBufferType type, uint count, uint stride, const void* initial_data);
static QglShader* esshader_new(const char* name);
static bool esshader_link(QgShader* g);
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
};

#if !defined STATIC_ES_LIBRARY && !defined USE_SDL2
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
#if false
	return (GLADapiproc)eglGetProcAddress(name);
#else
	return (GLADapiproc)qn_mod_func(es_api_module, name);
#endif
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
#endif

#ifdef USE_SDL2
//
static GLADapiproc es_load_sdl_api(const char* name)
{
	return (GLADapiproc)SDL_GL_GetProcAddress(name);
}

//
static bool esl_init_sdl_api(void)
{
	return gladLoadGLES2(es_load_sdl_api) != 0;
}
#else
//
static const char* es_egl_error_string(EGLint error)
{
#define ERROR_CASE(x)		case EGL_##x: return "EGL_" QN_STRING(x)
	static char unknown[32];
	switch (error)
	{
		ERROR_CASE(SUCCESS);
		ERROR_CASE(NOT_INITIALIZED);
		ERROR_CASE(BAD_ACCESS);
		ERROR_CASE(BAD_ALLOC);
		ERROR_CASE(BAD_ATTRIBUTE);
		ERROR_CASE(BAD_CONTEXT);
		ERROR_CASE(BAD_CONFIG);
		ERROR_CASE(BAD_CURRENT_SURFACE);
		ERROR_CASE(BAD_DISPLAY);
		ERROR_CASE(BAD_SURFACE);
		ERROR_CASE(BAD_MATCH);
		ERROR_CASE(BAD_PARAMETER);
		ERROR_CASE(BAD_NATIVE_PIXMAP);
		ERROR_CASE(BAD_NATIVE_WINDOW);
		ERROR_CASE(CONTEXT_LOST);
		default:
			qn_snprintf(unknown, QN_COUNTOF(unknown), "UNKNOWN(%X)", error);
			return unknown;
	}
#undef ERROR_CASE
}

//
static EGLint es_get_config_attr(const EsRdh* self, EGLConfig config, EGLenum name)
{
	EGLint v;
	eglGetConfigAttrib(self->display, config, name, &v);
	return v;
}

//
static const EsConfig* es_find_config(const EsConfig* wanted, const EsConfig* configs, int count)
{
	uint least_missing = UINT_MAX;
	uint least_color = UINT_MAX;
	uint least_extra = UINT_MAX;
	const EsConfig* found = NULL;
	for (int i = 0; i < count; i++)
	{
		const EsConfig* c = &configs[i];
		if (c->version < wanted->version)
			continue;

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
#endif

RdhBase* es_allocator(QgFlag flags, QgFeature features)
{
	if (QN_TMASK(features, QGRENDERER_ES3) == false)
		return NULL;

#if !defined STATIC_ES_LIBRARY && !defined USE_SDL2
	if (es_init_egl() == false)
		return NULL;
#endif

#ifdef _QN_EMSCRIPTEN_
	flags &= ~(QGFLAG_MSAA | QGFLAG_DITHER | QGFLAG_DITHER_ALPHA_STENCIL);
#endif

	// 프로퍼티에 있으면 가져온다
	int size_red = 8, size_green = 8, size_blue = 8, size_alpha = 8;
	const char* size_prop = qn_get_prop(QG_PROP_RGBA_SIZE);
	if (size_prop != NULL && strlen(size_prop) >= 4)
	{
		size_red = size_prop[0] - '0';
		size_green = size_prop[1] - '0';
		size_blue = size_prop[2] - '0';
		size_alpha = size_prop[3] - '0';
	}
	int size_depth = qn_get_prop_int(QG_PROP_DEPTH_SIZE, 24, 4, 32);
	int size_stencil = qn_get_prop_int(QG_PROP_STENCIL_SIZE, 8, 4, 16);
	int msaa_value = qn_get_prop_int(QG_PROP_MSAA, 4, 0, 8);

	if (QN_TMASK(flags, QGFLAG_DITHER | QGFLAG_DITHER_ALPHA_STENCIL))	// 디더 강제로 줄여
	{
		size_red = 5;
		size_blue = 5;
		if (QN_TMASK(flags, QGFLAG_DITHER_ALPHA_STENCIL))
		{
			size_green = 5;
			size_alpha = 1;
			size_stencil = 8;
		}
		else
		{
			size_green = 6;
			size_alpha = 0;
			size_stencil = 0;
		}
		size_depth = 16;
	}

	//
	EsRdh* self = qn_alloc_zero_1(EsRdh);

#ifdef USE_SDL2
	//----- SDL 초기화
	self->window = (SDL_Window*)stub_system_get_window();
	self->context = SDL_GL_CreateContext(self->window);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, size_red);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, size_green);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, size_blue);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, size_alpha);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, size_depth);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, size_stencil);
	if (QN_TMASK(flags, QGFLAG_MSAA))
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa_value);
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	Uint32 sdl_flag = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
	if (QN_TMASK(flags, QGFLAG_VSYNC))
		sdl_flag |= SDL_RENDERER_PRESENTVSYNC;

	self->renderer = SDL_CreateRenderer(self->window, -1, sdl_flag);
	if (self->renderer == NULL || SDL_GL_LoadLibrary(NULL) != 0)
	{
		qn_debug_outputf(true, "ESRDH", "failed to create renderer (%s)", SDL_GetError());
		goto pos_fail_exit;
	}

	// GL 함수 초기화
	if (esl_init_sdl_api() == false)
	{
		qn_debug_outputs(true, "ESRDH", "cannot initialize es client");
		goto pos_fail_exit;
	}

	if (QN_TMASK(flags, QGFLAG_VSYNC) == false)
		SDL_GL_SetSwapInterval(0);
	else
	{
		if (SDL_GL_SetSwapInterval(-1) < 0)
			SDL_GL_SetSwapInterval(1);
	}
#else
	//----- EGL 초기화
	self->native_window = (NativeWindowType)stub_system_get_window();
	self->native_display = (NativeDisplayType)stub_system_get_display();

	self->display = eglGetDisplay(self->native_display);
	if (self->display == EGL_NO_DISPLAY)
	{
		qn_debug_outputs(true, "ESRDH", "failed to get display");
		goto pos_fail_exit;
	}

	if (eglInitialize(self->display, &self->egl_major, &self->egl_minor) == false)
	{
		qn_debug_outputf(true, "ESRDH", "failed to initialize: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}
#if !defined STATIC_ES_LIBRARY
	// eglInitialize 이후에나 버전을 알 수 있다. glad egl은 버전이 필요함
	es_reload_egl(self->display);
#endif

	EGLint egl_config_count;
	eglGetConfigs(self->display, NULL, 0, &egl_config_count);
	if (egl_config_count == 0)
	{
		qn_debug_outputs(true, "ESRDH", "no config found");
		goto pos_fail_exit;
	}

	bool support3 = false;
	int config_entry_count = 0;
	EsConfig* config_entries = qn_alloc_zero(egl_config_count, EsConfig);
	EGLConfig* egl_configs = qn_alloc_zero(egl_config_count, EGLConfig);
	eglGetConfigs(self->display, egl_configs, egl_config_count, &egl_config_count);

	for (EGLint i = 0; i < egl_config_count; i++)
	{
		const EGLConfig c = egl_configs[i];
		EsConfig* u = &config_entries[config_entry_count];

		if (es_get_config_attr(self, c, EGL_COLOR_BUFFER_TYPE) != EGL_RGB_BUFFER)
			continue;
		if (QN_TMASK(es_get_config_attr(self, c, EGL_SURFACE_TYPE), EGL_WINDOW_BIT) == false)
			continue;
		const EGLint vmask = es_get_config_attr(self, c, EGL_RENDERABLE_TYPE);
		if (QN_TMASK(vmask, EGL_OPENGL_ES3_BIT | EGL_OPENGL_ES2_BIT) == false)
			continue;

		u->handle = c;
		u->red = es_get_config_attr(self, c, EGL_RED_SIZE);
		u->green = es_get_config_attr(self, c, EGL_GREEN_SIZE);
		u->blue = es_get_config_attr(self, c, EGL_BLUE_SIZE);
		u->alpha = es_get_config_attr(self, c, EGL_ALPHA_SIZE);
		u->depth = es_get_config_attr(self, c, EGL_DEPTH_SIZE);
		u->stencil = es_get_config_attr(self, c, EGL_STENCIL_SIZE);
		u->samples = es_get_config_attr(self, c, EGL_SAMPLES);
		if (QN_TMASK(vmask, EGL_OPENGL_ES3_BIT) == false)
			u->version = 2;
		else
		{
			u->version = 3;
			support3 = true;
		}
		config_entry_count++;
	}

	const EsConfig want_config =
	{
		NULL,
		size_red, size_green, size_blue, size_alpha,
		size_depth, size_stencil, QN_TMASK(flags, QGFLAG_MSAA) ? msaa_value : 0,
		support3 ? 3 : 2
	};
	const EsConfig* found_entry = es_find_config(&want_config, config_entries, config_entry_count);
	self->config = *found_entry;
	qn_free(egl_configs);
	qn_free(config_entries);

	if (eglBindAPI(EGL_OPENGL_ES_API) == false)
	{
		qn_debug_outputf(true, "ESRDH", "failed to initialize: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}

	EGLint context_attrs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, support3 ? 3 : 2,
		EGL_NONE, EGL_NONE,
	};
	self->context = eglCreateContext(self->display, self->config.handle, NULL, context_attrs);
	if (self->context == EGL_NO_CONTEXT)
	{
		if (support3)
		{
			context_attrs[1] = 2;
			self->context = eglCreateContext(self->display, self->config.handle, NULL, context_attrs);
			if (self->context != EGL_NO_CONTEXT)
				goto pos_context_ok;
		}

		qn_debug_outputf(true, "ESRDH", "failed to create context: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}
pos_context_ok:
	eglQueryContext(self->display, self->context, EGL_CONTEXT_CLIENT_VERSION, &self->version_major);

#ifdef _QN_ANDROID_
	EGLint android_visual_id;
	eglGetConfigAttrib(self->display, self->config, EGL_NATIVE_VISUAL_ID, &android_visual_id);
	ANativeWindow_setBuffersGeometry(self->native_window, 0, 0, android_visual_id);
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

	// 서피스를 만들었으니 여기서 윈도우 표시
	stub_system_actuate();

	// 커런트 만들고
	if (eglMakeCurrent(self->display, self->surface, self->surface, self->context) == false)
	{
		qn_debug_outputf(true, "ESRDH", "failed to make current: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}

#ifndef STATIC_ES_LIBRARY
	// eglMakeCurrent 하고 나야 GL 드라이버가 로드된다
	if (es_init_api(self->version_major) == false)
		goto pos_fail_exit;
#endif

	eglSwapInterval(self->display, QN_TMASK(flags, QGFLAG_VSYNC) ? 1 : 0);
#endif

	//----- 정보 설정
	RendererInfo* infos = &self->base.base.info;	// 여기서 rdh_info()는 쓸 수 없다

	qgl_copy_string(infos->renderer, QN_COUNTOF(infos->renderer), GL_RENDERER);
	qgl_copy_string(infos->vendor, QN_COUNTOF(infos->vendor), GL_VENDOR);
	infos->renderer_version = qgl_get_version(GL_VERSION, "OPENGLES", "OPENGL ES");
	infos->shader_version = qgl_get_version(GL_SHADING_LANGUAGE_VERSION, "OPENGL ES GLSL ES ", "OPENGL ES GLSL ");

	const int max_layout_count = qgl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);
	infos->max_layout_count = QN_MIN(max_layout_count, ES_MAX_LAYOUT_COUNT);
	infos->max_tex_dim = qgl_get_integer_v(GL_MAX_TEXTURE_SIZE);
	infos->max_tex_count = qgl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	infos->max_off_count = 1; //qgl_get_integer_v(GL_FRAMEBUFFER_BINDING);
	infos->clr_fmt = qg_rgba_to_clrfmt(size_red, size_green, size_blue, size_alpha, false);
	infos->max_off_count = 1;

	// 여기서 세이더 유니폼 함수 등록

	//
	qn_debug_outputf(false, "ESRDH", "OPENGLES %d/%d [%s by %s]",
		infos->renderer_version, infos->shader_version,
		infos->renderer, infos->vendor);
	return qs_init(self, RdhBase, &vt_es_rdh);

pos_fail_exit:
#ifdef USE_SDL2
	if (self->renderer)
		SDL_DestroyRenderer(self->renderer);
#else
	if (self->context != EGL_NO_CONTEXT)
		eglDestroyContext(self->display, self->context);
	if (self->surface != EGL_NO_SURFACE)
		eglDestroySurface(self->display, self->surface);
	if (self->display != EGL_NO_DISPLAY)
		eglTerminate(self->display);
#endif
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
#ifdef USE_SDL2
	if (self->renderer)
		SDL_DestroyRenderer(self->renderer);
#else
	if (self->context != EGL_NO_CONTEXT)
		eglDestroyContext(self->display, self->context);
	if (self->surface != EGL_NO_SURFACE)
		eglDestroySurface(self->display, self->surface);
	if (self->display != EGL_NO_DISPLAY)
		eglTerminate(self->display);
#endif

	self->base.disposed = true;
	rdh_internal_dispose();
}

//
static void es_reset(void)
{
	rdh_internal_reset();

	EsRdh* self = ES_RDH_INSTANCE;
	const RendererInfo* info = rdh_info();
	RenderTransform* tm = rdh_transform();

	//----- 펜딩

	//----- 세션
	QglSession* ss = &self->base.ss;
	uint amask = ss->shader.amask;
	ss->shader.program = 0;
	ss->shader.amask = 0;
	qn_zero(ss->shader.lprops, QGLOU_MAX_SIZE, QglLayoutProperty);
	ss->buffer.array = GL_INVALID_HANDLE;
	ss->buffer.element_array = GL_INVALID_HANDLE;
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
	for (int i = 0; i < info->max_tex_count; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// 세이더
	for (size_t i = 0; i < info->max_layout_count; i++)
	{
		if (QN_TBIT(amask, i))
			glDisableVertexAttribArray(i);
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
	EsRdh* self = ES_RDH_INSTANCE;

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
			if (ss->buffer.array != gl_id)
			{
				glBindBuffer(GL_ARRAY_BUFFER, gl_id);
				ss->buffer.array = gl_id;
			}
			break;
		case QGBUFFER_INDEX:				// 인덱스 버퍼
			if (ss->buffer.element_array != gl_id)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_id);
				ss->buffer.element_array = gl_id;
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
	if (buf == NULL)
	{
		if (pd->render.index_buffer != NULL)
		{
			qs_unload(pd->render.index_buffer);
			pd->render.index_buffer = NULL;
		}
	}
	else
	{
		qn_val_if_fail(buf->base.type == QGBUFFER_INDEX, false);

		if (pd->render.index_buffer != buf)
		{
			qs_unload(pd->render.index_buffer);
			pd->render.index_buffer = qs_loadc(buf, QglBuffer);
		}
	}
	return true;
}

// 정점 버퍼 설정, stage에 대한 오류 설정은 rdh에서 하고 왔을 거임
static bool es_set_vertex(QgLoStage stage, QgBuffer* buffer)
{
	QglBuffer* buf = qs_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (buf == NULL)
	{
		if (pd->render.vertex_buffers[stage] != NULL)
		{
			qs_unload(pd->render.vertex_buffers[stage]);
			pd->render.vertex_buffers[stage] = NULL;
		}
	}
	else
	{
		qn_val_if_fail(buf->base.type == QGBUFFER_VERTEX, false);

		if (pd->render.vertex_buffers[stage] != buf)
		{
			qs_unload(pd->render.vertex_buffers[stage]);
			pd->render.vertex_buffers[stage] = qs_loadc(buf, QglBuffer);
		}
	}
	return true;
}

// 세이더 만들기
static QgShader* es_create_shader(const char* name)
{
	QglShader* shader = esshader_new(name);
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
	if (rdr == NULL)
	{
		if (pd->render.render != NULL)
		{
			qs_unload(pd->render.render);
			pd->render.render = NULL;
		}
	}
	else
	{
		if (pd->render.render != rdr)
		{
			qs_unload(pd->render.render);
			pd->render.render = qs_loadc(rdr, QglBuffer);
		}
	}
	return true;
}

// 세이더랑 레이아웃
static bool es_commit_shader_layout(const QglRender* rdr)
{
	const size_t max_attrs = RDH_INFO->max_layout_count;
	const QglShader* shd = rdr->shader;

	// 세이더
	if (shd == NULL)
	{
		if (QGL_SESSION->shader.program != 0)
		{
			QGL_SESSION->shader.program = 0;
			glUseProgram(0);
		}
		return false;
	}

	esshader_link(shd);
	GLuint gl_program = qs_get_desc(shd, GLuint);
	if (QGL_SESSION->shader.program != gl_program)
	{
		QGL_SESSION->shader.program = gl_program;
		glUseProgram(gl_program);
	}

	// 세이더값 여기서 넣어야 한다!

	// 레이아웃
	uint ok = 0, amask = QGL_SESSION->shader.amask;
	for (size_t s = 0; s < QGLOS_MAX_VALUE; s++)
	{
		QglBuffer* buf = QGL_PENDING->render.vertex_buffers[s];
		if (buf == NULL)
			continue;

		const int count = rdr->layout.count[s];
		const QglLayoutElement* elms = rdr->layout.elms[s];
		const GLsizei gl_stride = (GLsizei)rdr->layout.stride[s];
		const GLuint gl_buf = qs_get_desc(buf, GLuint);
		es_bind_buffer(QGBUFFER_VERTEX, gl_buf);

		for (int i = 0; i < count; i++, i++)
		{
			const QglLayoutElement* le = &elms[i];
			const GLint gl_attr = shd->attr_index[le->usage];
			if (gl_attr == 0)
			{
				qn_debug_outputf(true, "ESRDH", "shader attribute not found: %d", le->usage);
				continue;
			}

			ok |= QN_BIT(gl_attr);
			if (!QN_TBIT(amask, gl_attr))
			{
				QN_SBIT(&amask, gl_attr, true);
				glEnableVertexAttribArray(gl_attr);
			}

			const GLvoid* gl_ptr = (GLvoid*)(size_t)le->offset;
			QglLayoutProperty* lp = &QGL_SESSION->shader.lprops[le->usage];
			if (lp->pointer != gl_ptr ||
				lp->buffer != gl_buf ||
				lp->stride != gl_stride ||
				lp->count != le->count ||
				lp->format != le->format ||
				lp->normalized != le->normalized)
			{
				glVertexAttribPointer(gl_attr, le->count, le->format, le->normalized, gl_stride, gl_ptr);
				lp->pointer = gl_ptr;
				lp->buffer = gl_buf;
				lp->stride = gl_stride;
				lp->count = le->count;
				lp->format = le->format;
				lp->normalized = le->normalized;
			}
		}
	}

	// 레이아웃 정리
	uint aftermask = amask & ~ok;
	for (size_t i = 0; i < max_attrs && aftermask; i++)
	{
		if (QN_TBIT(aftermask, 0))
		{
			QN_SBIT(&amask, i, false);
			glDisableVertexAttribArray(i);
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
	qn_ret_if_fail(rdr != NULL);

	if (es_commit_shader_layout(rdr) == false)
		return false;

	return true;
}

// 그리기
static bool es_draw(QgTopology tpg, int vertices)
{
	GLenum gl_tpg = qgl_topology_to_enum(tpg);
	if (gl_tpg == GL_NONE)
	{
		qn_debug_outputf(true, "ESRDH", "invalid topology: %d", tpg);
		return false;
	}

	if (vertices == 0)
	{
		if (tpg == QGTPG_LINE)
			vertices = vertices / 2;
		else if (tpg == QGTPG_TRI)
			vertices = vertices / 3;
		else
		{
			qn_debug_outputs(true, "ESRDH", "invalid vertices");
			return false;
		}
	}

	if (es_commit_render() == false)
	{
		if (QGL_PENDING->render.render == NULL)
			qn_debug_outputs(true, "ESRDH", "render is null");
		else if (QGL_PENDING->render.render->shader == NULL)
			qn_debug_outputs(true, "ESRDH", "shader is null");
		return false;
	}

	glDrawArrays(gl_tpg, 0, (GLsizei)vertices);

	qn_val_if_fail(gl_tpg != GL_NONE, false);


	const QglRender* rdr = QGL_PENDING->render.render;
	qn_ret_if_fail(rdr != NULL);

	const GLenum gl_tpg = qgl_topology_to_gl(tpg);
	const GLenum gl_type = rdr->index != NULL ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;
	const GLsizei gl_count = rdr->index != NULL ? rdr->index->base.count : vertices;

	if (rdr->index != NULL)
	{
		const GLuint gl_buf = qs_get_desc(rdr->index, GLuint);
		es_bind_buffer(QGBUFFER_INDEX, gl_buf);
	}

	glDrawElements(gl_tpg, gl_count, gl_type, NULL);
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

	if (RDH_INFO->renderer_version < 300)
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

	if (RDH_INFO->renderer_version >= 300)
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
	if (type == QGBUFFER_INDEX)
		gl_type = GL_ELEMENT_ARRAY_BUFFER;
	else if (type == QGBUFFER_VERTEX)
		gl_type = GL_ARRAY_BUFFER;
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

	self->linked = false;
	return true;
}

//
static bool esshader_bind_buffer(QgShader* g, QgShaderType type, const void* data, uint size, int flags)
{
	QN_DUMMY(flags);
	// TODO: 매크로
	// TODO: 포함
	// TODO: 플래그
	// TODO: 보관 (이건 바인드가 아니고 전체)
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

	self->linked = false;
	return true;
}

//
static bool esshader_link(QgShader* g)
{
	QglShader* self = qs_cast_type(g, QglShader);
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
				continue;
			}

			QglVarAttr* a = &qn_ctnr_nth(&self->attrs, index);
			qn_strcpy(a->name, sz);
			a->hash = qn_strihash(a->name);
			a->attrib = gl_attr;
			a->size = gl_size;
			a->usage = qg_str_to_layout_usage(a->hash, a->name);
			a->sctype = sct;
			self->attr_index[a->usage] = gl_attr;
			index++;
		}
	}
}

//
static QglShader* esshader_new(const char* name)
{
	QglShader* self = qn_alloc_zero_1(QglShader);
	qs_set_desc(self, glCreateProgram());

	if (name != NULL)
		qn_strncpy(self->base.name, name, QN_COUNTOF(self->base.name) - 1);
	else
	{
		size_t n = qn_p_index();
		qn_snprintf(self->base.name, QN_COUNTOF(self->base.name), "SHADER%zu", n);
	}

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
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
		qs_unload(self->layout.elms[i]);
	qn_free(self);
}

//
static QglRender* esrender_new(const QgPropRender* prop, QgShader* shader)
{
	static byte lo_count[QGLOT_MAX_VALUE] =
	{
		1, 2, 3, 4,
		2, 4,
		2, 4, 4,
		2, 4, 2,
		4, 1,
	};
	static GLenum lo_format[QGLOT_MAX_VALUE] =
	{
		GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT,
		GL_HALF_FLOAT, GL_HALF_FLOAT,
		GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE,
		GL_SHORT, GL_SHORT, GL_SHORT,
		GL_FLOAT, GL_UNSIGNED_INT,
	};
	static byte lo_elemental_size[QGLOT_MAX_VALUE] =
	{
		1 * sizeof(float), 2 * sizeof(float), 3 * sizeof(float), 4 * sizeof(float),
		2 * sizeof(halfint), 4 * sizeof(halfint),
		2 * sizeof(byte), 4 * sizeof(byte), 4 * sizeof(byte),
		2 * sizeof(short), 4 * sizeof(short), 2 * sizeof(short),
		4 * sizeof(float), 1 * sizeof(uint),
	};
	static GLboolean lo_normalized[QGLOT_MAX_VALUE] =
	{
		GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE,
		GL_TRUE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_TRUE,
	};
	static GLboolean s_converted[QGLOT_MAX_VALUE] =
	{
		GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE,
		GL_TRUE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_TRUE,
	};

	// 검사
	byte loac[QGLOS_MAX_VALUE] = { 0, };
	for (size_t i = 0; i < prop->layout.count; i++)
	{
		QgLayoutInput* l = &prop->layout.elements[i];
		if ((size_t)l->stage >= QGLOS_MAX_VALUE)
			return NULL;
		loac[l->stage]++;
	}

	QglRender* self = qn_alloc_zero_1(QglRender);

	// 입력 레이아웃
	QglLayoutElement* elms[QGLOS_MAX_VALUE] = { NULL, };
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (loac[i] == 0)
			continue;
		self->layout.count[i] = (byte)loac[i];
		self->layout.elms[i] = elms[i] = qn_alloc(loac[i], QglLayoutElement);
	}

	for (size_t i = 0; i < prop->layout.count; i++)
	{
		const QgLayoutInput* l = &prop->layout.elements[i];
		QglLayoutElement* e = elms[l->stage]++;
		e->stage = l->stage;
		e->usage = l->usage;
		e->count = lo_count[l->type];
		e->format = lo_format[l->type];
		e->offset = self->layout.stride[l->stage];
		e->normalized = lo_normalized[l->type];
		e->converted = s_converted[l->type];
		self->layout.stride[l->stage] += lo_elemental_size[l->type];
	}

	// 세이더
	self->shader = qs_loadc(shader, QglShader);
	esshader_link(shader);

	//
	static qs_name_vt(QSGAM) vt_es_render =
	{
		.name = "ESRENDER",
		.dispose = esrender_dispose,
	};
	return qs_init(self, QglRender, &vt_es_render);
}

#endif // USE_ES
