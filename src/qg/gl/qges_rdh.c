#include "pch.h"
#if !defined STATIC_ES_LIBRARY && !defined USE_SDL2
#define GLAD_EGL_IMPLEMENTATION		1
#define GLAD_GLES2_IMPLEMENTATION	1
#endif
#include "qges_rdh.h"
#include "qggl_code.h"

#define ES_MAX_LAYOUT_COUNT		16

//////////////////////////////////////////////////////////////////////////
// OPENGL ES 렌더 디바이스

static void es_dispose(QsGam* g);
static void es_reset(QgRdh* rdh);
static void es_flush(QgRdh* rdh);

qv_name(QgRdh) vt_es_rdh =
{
	.base.name = "ESDevice",
	.base.dispose = es_dispose,

	.reset = es_reset,
	.clear = qgl_clear,

	.begin = qgl_begin,
	.end = qgl_end,
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

#if !defined USE_SDL2
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
static const EsConfig* es_find_config(EsConfig* configs, int count, bool support3)
{
	int least_missing = INT_MAX;
	int least_extra = INT_MAX;
	const EsConfig* found = NULL;
	for (int i = 0; i < count; i++)
	{
		const EsConfig* c = &configs[i];
		if (support3 && c->version == 2)
			continue;

		int missing = 0;
		if (c->alpha == 0) missing++;
		if (c->depth == 0) missing++;
		if (c->stencil == 0) missing++;
		if (c->samples == 0) missing++;

		int extra = 0;
		extra += (8 - c->alpha) * (8 - c->alpha);
		extra += (8 - c->depth) * (8 - c->depth);
		extra += (8 - c->stencil) * (8 - c->stencil);

		if (missing < least_missing)
			found = c;
		else if (missing == least_missing && extra < least_extra)
			found = c;
		if (found == c)
		{
			least_missing = missing;
			least_extra = extra;
		}
	}
	return found;
}
#endif

QgRdh* es_allocator(int flags)
{
#if !defined STATIC_ES_LIBRARY && !defined USE_SDL2
	if (es_init_egl() == false)
		return NULL;
#endif

	EsRdh* self = qn_alloc_zero_1(EsRdh);

#if !defined USE_SDL2
	self->native_window = stub_system_get_window();
	self->native_display = stub_system_get_display();

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
	// eglInitialize 이후에나 버전을 알 수 있다
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

	const EsConfig* found_entry = es_find_config(config_entries, config_entry_count, support3);
	self->config = found_entry->handle;
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
	self->context = eglCreateContext(self->display, self->config, NULL, context_attrs);
	if (self->context == EGL_NO_CONTEXT)
	{
		if (support3)
		{
			context_attrs[1] = 2;
			self->context = eglCreateContext(self->display, self->config, NULL, context_attrs);
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
	self->surface = eglCreateWindowSurface(self->display, self->config, self->native_window, surface_attrs);
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

	eglSwapInterval(self->display, 0);
#endif

	int renderer_version = qgl_get_version(GL_VERSION, "OPENGLES", "OPENGL ES");
	int shader_version = qgl_get_version(GL_SHADING_LANGUAGE_VERSION, "OPENGL ES GLSL ES ", "OPENGL ES GLSL ");
	qgl_initialize(&self->base, flags, renderer_version, shader_version);

	QgDeviceInfo* caps = &rdh_caps(self);
	caps->max_off_count = 1;
	caps->max_layout_count = QN_MIN(caps->max_layout_count, ES_MAX_LAYOUT_COUNT);

	return qs_init(self, QgRdh, &vt_es_rdh);

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
	EsRdh* self = qs_cast(g, EsRdh);

	qgl_finalize(qs_cast(self, QglRdh));

	if (self->context != EGL_NO_CONTEXT)
		eglDestroyContext(self->display, self->context);
	if (self->surface != EGL_NO_SURFACE)
		eglDestroySurface(self->display, self->surface);
	if (self->display != EGL_NO_DISPLAY)
		eglTerminate(self->display);

	rdh_internal_dispose(g);
}

//
static void es_reset(QgRdh* rdh)
{
	qgl_reset(rdh);
}

//
static void es_flush(QgRdh* rdh)
{
	EsRdh* self = qs_cast(rdh, EsRdh);

	qgl_flush(rdh);
	eglSwapBuffers(self->display, self->surface);
}

