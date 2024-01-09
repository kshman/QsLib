#include "pch.h"
#ifdef USE_ES
#include "qs_qn.h"
#ifndef USE_SDL2
#define GLAD_EGL_IMPLEMENTATION		1
#endif
#define GLAD_GLES2_IMPLEMENTATION	1
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

qv_name(RdhBase) vt_es_rdh =
{
	.base.name = "ESRDH",
	.base.dispose = es_dispose,

	.reset = es_reset,
	.clear = es_clear,

	.begin = es_begin,
	.end = es_end,
	.flush = es_flush,
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
static EGLint es_get_config_attr(EsRdh* self, EGLConfig config, EGLenum name)
{
	EGLint v;
	eglGetConfigAttrib(self->display, config, name, &v);
	return v;
}

//
static const EsConfig* es_find_config(const EsConfig* wanted, EsConfig* configs, int count)
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

	EsConfig want_config =
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
	EGLint surface_attrs[] =
	{
		EGL_NONE, EGL_NONE,
	};
	self->surface = eglCreateWindowSurface(self->display, self->config.handle, self->native_window, surface_attrs);
	if (self->surface == EGL_NO_SURFACE)
	{
		qn_debug_outputf(true, "ESRDH", "failed to create surface: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}

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

	int max_layout_count = qgl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);
	infos->max_layout_count = QN_MIN(max_layout_count, ES_MAX_LAYOUT_COUNT);
	infos->max_tex_dim = qgl_get_integer_v(GL_MAX_TEXTURE_SIZE);
	infos->max_tex_count = qgl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	infos->max_off_count = 1; //qgl_get_integer_v(GL_FRAMEBUFFER_BINDING);
	infos->clr_fmt = qg_clrfmt_from_size(size_red, size_green, size_blue, size_alpha, false);
	infos->max_off_count = 1;

	// 여기서 세이더 유니폼 함수 등록

	//
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
	EsRdh* self = qs_cast(g, EsRdh);
	qn_ret_if_ok(self->base.disposed);

	//----- 펜딩
	QglPending* pd = &self->base.pd;
	qs_unload(pd->draw.index_buffer);
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qs_unload(pd->draw.vertex_buffers[i]);

	//----- 세션


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
	const RendererInfo* info = &rdh_info();
	RenderTransform* tm = &rdh_transform();

	//----- 펜딩

	//----- 세션
	QglSession* ss = &self->base.ss;
	ss->depth = QGDEPTH_LE;
	ss->stencil = QGSTENCIL_OFF;

	//----- 트랜스폼
	tm->ortho = qm_mat4_ortho_lh(tm->size.X, tm->size.Y, -1.0f, 1.0f);
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
			color = &rdh_param().bgc;
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
#ifdef _QN_EMSCRIPTEN_
	if (emscripten_has_asyncify())
		emscripten_sleep(0);
#endif
}

#endif // USE_ES
