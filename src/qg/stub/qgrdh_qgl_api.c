//
// qgrdh_qgl_api.c - API 대리자 (EGL/WGL)
// 2024-1-27 by kim
//

#include "pch.h"
#ifdef USE_GL
#include "qgrdh_qgl.h"
#include <qs_supp.h>
#ifdef _QN_EMSCRIPTEN_
#include <emscripten/html5_webgl.h>
#endif
#ifdef _QN_ANDROID_
#include <android/native_window.h>
#endif

#ifndef QGL_LINK_STATIC
extern int gladLoadGL(GLADloadfunc load);
extern int gladLoadGLES2(GLADloadfunc load);
#endif


//////////////////////////////////////////////////////////////////////////
// EGL 확장

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"EGL"

#ifndef QGL_EGL_NO_EXT
int GLAD_EGL_EXT_present_opaque = 0;
int GLAD_EGL_KHR_context_flush_control = 0;
int GLAD_EGL_KHR_create_context = 0;
int GLAD_EGL_KHR_create_context_no_error = 0;
int GLAD_EGL_KHR_get_all_proc_addresses = 0;
int GLAD_EGL_KHR_gl_colorspace = 0;

static int glad_egl_find_extensions_egl(EGLDisplay display)
{
	const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
	if (extensions == NULL) return 0;

	GLAD_EGL_EXT_present_opaque = qn_strext(extensions, "EGL_EXT_present_opaque", ' ') != 0;
	GLAD_EGL_KHR_context_flush_control = qn_strext(extensions, "EGL_KHR_context_flush_control", ' ') != 0;
	GLAD_EGL_KHR_create_context = qn_strext(extensions, "EGL_KHR_create_context", ' ') != 0;
	GLAD_EGL_KHR_create_context_no_error = qn_strext(extensions, "EGL_KHR_create_context_no_error", ' ') != 0;
	GLAD_EGL_KHR_get_all_proc_addresses = qn_strext(extensions, "EGL_KHR_get_all_proc_addresses", ' ') != 0;
	GLAD_EGL_KHR_gl_colorspace = qn_strext(extensions, "EGL_KHR_gl_colorspace", ' ') != 0;
	return 1;
}
#endif // QGL_EGL_NO_EXT


//////////////////////////////////////////////////////////////////////////
// GLAD 로드

#ifndef QGL_LINK_STATIC
PFNEGLBINDAPIPROC glad_eglBindAPI = NULL;
PFNEGLCHOOSECONFIGPROC glad_eglChooseConfig = NULL;
PFNEGLCREATECONTEXTPROC glad_eglCreateContext = NULL;
PFNEGLCREATEWINDOWSURFACEPROC glad_eglCreateWindowSurface = NULL;
PFNEGLDESTROYCONTEXTPROC glad_eglDestroyContext = NULL;
PFNEGLDESTROYSURFACEPROC glad_eglDestroySurface = NULL;
PFNEGLGETCONFIGATTRIBPROC glad_eglGetConfigAttrib = NULL;
PFNEGLGETDISPLAYPROC glad_eglGetDisplay = NULL;
PFNEGLGETERRORPROC glad_eglGetError = NULL;
PFNEGLGETPROCADDRESSPROC glad_eglGetProcAddress = NULL;
PFNEGLINITIALIZEPROC glad_eglInitialize = NULL;
PFNEGLMAKECURRENTPROC glad_eglMakeCurrent = NULL;
PFNEGLQUERYSTRINGPROC glad_eglQueryString = NULL;
PFNEGLSWAPBUFFERSPROC glad_eglSwapBuffers = NULL;
PFNEGLSWAPINTERVALPROC glad_eglSwapInterval = NULL;
PFNEGLTERMINATEPROC glad_eglTerminate = NULL;

//
static QnModule* egl_module = NULL;

//
static bool glad_load_egl(void)
{
	static bool init = false;
	if (init)
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
		egl_module = qn_mod_load(egllibs[i], 1);
		if (egl_module != NULL)
			break;
	}
	VAR_CHK_IF_COND(egl_module == NULL, "cannot load egl library", false);

#ifdef _MSC_VER
#pragma warning(disable:4191)
#endif
	/* 1.0 */glad_eglChooseConfig = (PFNEGLCHOOSECONFIGPROC)qn_mod_func(egl_module, "eglChooseConfig");
	/* 1.0 */glad_eglCreateContext = (PFNEGLCREATECONTEXTPROC)qn_mod_func(egl_module, "eglCreateContext");
	/* 1.0 */glad_eglCreateWindowSurface = (PFNEGLCREATEWINDOWSURFACEPROC)qn_mod_func(egl_module, "eglCreateWindowSurface");
	/* 1.0 */glad_eglDestroyContext = (PFNEGLDESTROYCONTEXTPROC)qn_mod_func(egl_module, "eglDestroyContext");
	/* 1.0 */glad_eglDestroySurface = (PFNEGLDESTROYSURFACEPROC)qn_mod_func(egl_module, "eglDestroySurface");
	/* 1.0 */glad_eglGetConfigAttrib = (PFNEGLGETCONFIGATTRIBPROC)qn_mod_func(egl_module, "eglGetConfigAttrib");
	/* 1.0 */glad_eglGetDisplay = (PFNEGLGETDISPLAYPROC)qn_mod_func(egl_module, "eglGetDisplay");
	/* 1.0 */glad_eglGetError = (PFNEGLGETERRORPROC)qn_mod_func(egl_module, "eglGetError");
	/* 1.0 */glad_eglGetProcAddress = (PFNEGLGETPROCADDRESSPROC)qn_mod_func(egl_module, "eglGetProcAddress");
	/* 1.0 */glad_eglInitialize = (PFNEGLINITIALIZEPROC)qn_mod_func(egl_module, "eglInitialize");
	/* 1.0 */glad_eglMakeCurrent = (PFNEGLMAKECURRENTPROC)qn_mod_func(egl_module, "eglMakeCurrent");
	/* 1.0 */glad_eglQueryString = (PFNEGLQUERYSTRINGPROC)qn_mod_func(egl_module, "eglQueryString");
	/* 1.0 */glad_eglSwapBuffers = (PFNEGLSWAPBUFFERSPROC)qn_mod_func(egl_module, "eglSwapBuffers");
	/* 1.0 */glad_eglTerminate = (PFNEGLTERMINATEPROC)qn_mod_func(egl_module, "eglTerminate");
	/* 1.1 */glad_eglSwapInterval = (PFNEGLSWAPINTERVALPROC)qn_mod_func(egl_module, "eglSwapInterval");
	/* 1.2 */glad_eglBindAPI = (PFNEGLBINDAPIPROC)qn_mod_func(egl_module, "eglBindAPI");
#ifdef _MSC_VER
#pragma warning(default:4191)
#endif

	return true;
}

//
static bool glad_load_gl(void)
{
	static int version = 0;
	if (version != 0)
		return true;
	version = gladLoadGL(eglGetProcAddress);
	return version != 0;
}

//
static bool glad_load_gles2(void)
{
	static int version = 0;
	if (version != 0)
		return true;
	version = gladLoadGLES2(eglGetProcAddress);
	return version != 0;
}
#endif // !QGL_LINK_STATIC


//////////////////////////////////////////////////////////////////////////
// 본격 EGL

// EGL 오류 메시지 얻기
static const char* egl_error_string(EGLint error)
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

// eglGetConfigAttrib 편하게 쓰자고 대리자
static EGLint egl_get_config_attrib(EGLDisplay display, EGLConfig config, EGLenum name)
{
	EGLint v;
	eglGetConfigAttrib(display, config, name, &v);
	return v;
}

// EGL 설정 읽기
static void egl_get_config(EGLDisplay display, EGLConfig ec, QglConfig* config, const QglConfig* wanted_config)
{
	config->handle = ec;
	config->version = wanted_config->version;
	config->red = egl_get_config_attrib(display, ec, EGL_RED_SIZE);
	config->green = egl_get_config_attrib(display, ec, EGL_GREEN_SIZE);
	config->blue = egl_get_config_attrib(display, ec, EGL_BLUE_SIZE);
	config->alpha = egl_get_config_attrib(display, ec, EGL_ALPHA_SIZE);
	config->depth = egl_get_config_attrib(display, ec, EGL_DEPTH_SIZE);
	config->stencil = egl_get_config_attrib(display, ec, EGL_STENCIL_SIZE);
	config->samples = egl_get_config_attrib(display, ec, EGL_SAMPLES);
	config->float_buffer = 0;
	config->no_error = 0;
	config->robustness = 0;
}

// 후보 컨피그 중에서 가장 적합한 것 찾기
static const QglConfig* egl_detect_config(const QglConfig* wanted, const QglCtnConfig* configs)
{
	uint least_missing = UINT_MAX;
	uint least_color = UINT_MAX;
	uint least_extra = UINT_MAX;
	const QglConfig* found = NULL;
	size_t i;
	qn_ctnr_foreach(configs, i)
	{
		const QglConfig* c = &qn_ctnr_nth(configs, i);

		uint missing = 0;
		if (wanted->alpha > 0 && c->alpha == 0) missing++;
		if (wanted->depth > 0 && c->depth == 0) missing++;
		if (wanted->stencil > 0 && c->stencil == 0) missing++;
		if (wanted->samples > 0 && c->samples == 0) missing++;
		if (wanted->float_buffer > 0 && c->float_buffer == 0) missing++;
		if (wanted->no_error > 0 && c->no_error == 0) missing++;

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


// EGL 설정 얻기
static bool egl_choose_config(EGLDisplay display, const QglConfig* wanted_config, QglConfig* found_config)
{
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[i++] = e; attrs[i++] = v; }QN_STMT_END
	EGLint attrs[32], i = 0;
	ATTR_ADD(EGL_RENDERABLE_TYPE, wanted_config->version >= 300 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT);
	ATTR_ADD(EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER);
	ATTR_ADD(EGL_SURFACE_TYPE, EGL_WINDOW_BIT);
	ATTR_ADD(EGL_RED_SIZE, wanted_config->red);
	ATTR_ADD(EGL_GREEN_SIZE, wanted_config->green);
	ATTR_ADD(EGL_BLUE_SIZE, wanted_config->blue);
	if (wanted_config->alpha > 0)
		ATTR_ADD(EGL_ALPHA_SIZE, wanted_config->alpha);
	if (wanted_config->depth > 0)
		ATTR_ADD(EGL_DEPTH_SIZE, wanted_config->depth);
	if (wanted_config->stencil > 0)
		ATTR_ADD(EGL_STENCIL_SIZE, wanted_config->stencil);
	if (wanted_config->samples > 0)
		ATTR_ADD(EGL_SAMPLES, wanted_config->samples);
	ATTR_ADD(EGL_NONE, EGL_NONE);
	qn_verify(i < QN_COUNTOF(attrs));
#undef ATTR_ADD

	EGLint config_count = 0;
	EGLConfig configs[64];
	if (eglChooseConfig(display, attrs, configs, 64, &config_count) == EGL_FALSE || config_count == 0)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to choose config: %s", egl_error_string(eglGetError()));
		qn_free(configs);
		return false;
	}

	bool ret;
	if (config_count == 1)
	{
		ret = true;
		egl_get_config(display, configs[0], found_config, wanted_config);
	}
	else
	{
		QglCtnConfig qgl_configs;
		qn_ctnr_init(QglCtnConfig, &qgl_configs, config_count);
		for (i = 0; i < config_count; i++)
			egl_get_config(display, configs[i], &qn_ctnr_nth(&qgl_configs, i), wanted_config);

		const QglConfig* found = egl_detect_config(wanted_config, &qgl_configs);
		if (found == NULL)
			ret = false;
		else
		{
			memcpy(found_config, found, sizeof(QglConfig));
			ret = true;
		}
		qn_ctnr_disp(&qgl_configs);
	}
	return ret;
}

// EGL 초기화
EGLDisplay egl_initialize(const QglConfig* wanted_config)
{
	qn_verify(wanted_config != NULL);
#ifndef QGL_LINK_STATIC
	if (glad_load_egl() == false)
		return EGL_NO_DISPLAY;
#endif
#ifdef _QN_EMSCRIPTEN_
	EmscriptenWebGLContextAttributes emattrs =
	{
		.alpha = wanted_config->alpha > 0 ? EM_TRUE : EM_FALSE,
		.depth = wanted_config->depth > 0 ? EM_TRUE : EM_FALSE,
		.stencil = wanted_config->stencil > 0 ? EM_TRUE : EM_FALSE,
		.antialias = wanted_config->samples > 0 ? EM_TRUE : EM_FALSE,
		.premultipliedAlpha = 0,
		.preserveDrawingBuffer = 0,
		.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT,
		.failIfMajorPerformanceCaveat = 0,
		.majorVersion = 3,
		.minorVersion = 2,
		.enableExtensionsByDefault = 0,
		.explicitSwapControl = 0,
	};
	emscripten_webgl_init_context_attributes(&emattrs);
	emattrs.majorVersion = 3;
	const char* canvas = stub_system_get_canvas();
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context = emscripten_webgl_create_context(canvas, &emattrs);
	emscripten_webgl_make_context_current(webgl_context);
#endif

	EGLDisplay display = eglGetDisplay((EGLNativeDisplayType)stub_system_get_display());
	if (display == EGL_NO_DISPLAY)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to get display");
		return EGL_NO_DISPLAY;
	}

	EGLint major, minor;
	if (eglInitialize(display, &major, &minor) == false)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to initialize: %s", egl_error_string(eglGetError()));
		eglTerminate(display);
		return EGL_NO_DISPLAY;
	}

	glad_egl_find_extensions_egl(display);

	return display;
}

// EGL 컨텍스트 만들기
EGLContext egl_create_context(_In_ EGLDisplay display, _In_ const QglConfig* wanted_config, _Out_ QglConfig* config, _In_ QgFlag flags, _In_ bool isCore)
{
	qn_verify(display != EGL_NO_DISPLAY);
	qn_verify(wanted_config != NULL);
	qn_verify(config != NULL);

	// 컨피그 얻고
	if (egl_choose_config(display, wanted_config, config) == false)
		goto pos_error_exit;

	// 속성 설정
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[attr_count++] = e; attrs[attr_count++] = v; }QN_STMT_END
	EGLenum profile = EGL_OPENGL_ES_API;
	EGLint attrs[24], attr_count = 0;
#ifndef QGL_EGL_NO_EXT
	if (GLAD_EGL_KHR_create_context)
	{
		EGLint emask = 0, eflags = 0;
		// EGL_CONTEXT_MAJOR_VERSION == EGL_CONTEXT_CLIENT_VERSION
		ATTR_ADD(EGL_CONTEXT_MAJOR_VERSION, wanted_config->version / 100);
		ATTR_ADD(EGL_CONTEXT_MINOR_VERSION, wanted_config->version % 100);
#ifndef _QN_MOBILE_
		if (isCore)
		{
			// 일단 ANGLE은 안됨
			profile = EGL_OPENGL_API;
			emask |= EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
		}
#endif
		if (QN_TMASK(flags, QGFLAG_DEBUG))
		{
			ATTR_ADD(EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE);
			eflags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
		}
		if (wanted_config->no_error)
			ATTR_ADD(EGL_CONTEXT_OPENGL_NO_ERROR_KHR, EGL_TRUE);
		if (wanted_config->robustness != 0)
		{
			ATTR_ADD(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY,
				wanted_config->robustness == 1 ? EGL_NO_RESET_NOTIFICATION : EGL_LOSE_CONTEXT_ON_RESET);
			eflags |= EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
		}
		if (emask != 0)
			ATTR_ADD(EGL_CONTEXT_OPENGL_PROFILE_MASK, emask);
		if (eflags != 0)
			ATTR_ADD(EGL_CONTEXT_FLAGS_KHR, eflags);
	}
	else
#endif
	{
		// EGL_CONTEXT_MAJOR_VERSION == EGL_CONTEXT_CLIENT_VERSION
#ifdef _QN_EMSCRIPTEN_
		ATTR_ADD(EGL_CONTEXT_CLIENT_VERSION, 3);
#else
		ATTR_ADD(EGL_CONTEXT_CLIENT_VERSION, wanted_config->version / 100);
#endif
}
	ATTR_ADD(EGL_NONE, EGL_NONE);
#undef ATTR_ADD

	// 바인드 먼저
	if (eglBindAPI(profile) == false)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to initialize: %s", egl_error_string(eglGetError()));
		goto pos_error_exit;
	}

	// 컨텍스트 만들고
	EGLContext context = eglCreateContext(display, config->handle, NULL, attrs);
	if (context == EGL_NO_CONTEXT)
	{
#ifdef _QN_EMSCRIPTEN_
		attrs[1] = 2;
		context = eglCreateContext(display, config->handle, NULL, attrs);
#else
		int index = qgl_index_of_opengl_version(isCore, wanted_config->version);
		if (index >= 0)
		{
			for (int i = index + 1; ; i++)
			{
				int version = qgl_get_opengl_version(isCore, i);
				if (version < 0)
					break;
				attrs[1] = version / 100;
				if (attrs[2] == EGL_CONTEXT_MINOR_VERSION)
					attrs[3] = version % 100;
				context = eglCreateContext(display, config->handle, NULL, attrs);
				if (context != EGL_NO_CONTEXT)
					break;
			}
		}
#endif
		if (context == EGL_NO_CONTEXT)
		{
			qn_debug_outputf(true, VAR_CHK_NAME, "failed to create context: %s", egl_error_string(eglGetError()));
			goto pos_error_exit;
		}
	}

	return context;

pos_error_exit:
	eglTerminate(display);
	return EGL_NO_CONTEXT;
}

// 서피스 만들기
EGLSurface egl_create_surface(EGLDisplay display, EGLContext context, EGLConfig config, int visual_id, QgFlag flags)
{
	QN_DUMMY(visual_id);
	qn_verify(display != EGL_NO_DISPLAY);
	qn_verify(context != EGL_NO_CONTEXT);
	qn_verify(config != NULL);

	// 속성 설정
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[attr_count++] = e; attrs[attr_count++] = v; }QN_STMT_END
	EGLint attrs[10], attr_count = 0;
#ifndef QGL_EGL_NO_EXT
	if (QN_TMASK(flags, QGFLAG_SRGB) && GLAD_EGL_KHR_gl_colorspace)
		ATTR_ADD(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB);
	if (QN_TMASK(flags, QGFLAG_TRANSPARENT) && GLAD_EGL_EXT_present_opaque)
		ATTR_ADD(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR);
#endif
	ATTR_ADD(EGL_NONE, EGL_NONE);
#undef ATTR_ADD

	EGLNativeWindowType native_window = (EGLNativeWindowType)stub_system_get_window();
#ifdef _QN_ANDROID_
	EGLint format;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(native_window, 0, 0, format);
#endif
	EGLSurface surface = eglCreateWindowSurface(display, config, native_window, attrs);
	if (surface == EGL_NO_SURFACE)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to create surface: %s", egl_error_string(eglGetError()));
		eglDestroyContext(display, context);
		eglTerminate(display);
		return EGL_NO_SURFACE;
	}
#ifdef _QN_ANDROID_
	int android_format = ANativeWindow_getFormat(native_window);
	Android_SetFormat(format, android_format);
#endif

	return surface;
	}

// 커런트 만들기
bool egl_make_current(EGLDisplay display, EGLSurface surface, EGLContext context, bool isCore)
{
	qn_verify(display != EGL_NO_DISPLAY);
	qn_verify(surface != EGL_NO_SURFACE);
	qn_verify(context != EGL_NO_CONTEXT);

	if (eglMakeCurrent(display, surface, surface, context) == false)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to make current: %s", egl_error_string(eglGetError()));
		return false;
	}

#ifndef QGL_LINK_STATIC
	if ((isCore ? glad_load_gl() : glad_load_gles2()) == false)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to make current: %s", egl_error_string(eglGetError()));
		return false;
	}
#endif // QGL_LINK_STATIC

	return true;
}

// 모든 EGL 자원 해제
void egl_dispose(EGLDisplay display, EGLSurface surface, EGLContext context)
{
	if (display != EGL_NO_DISPLAY)
	{
		if (surface != EGL_NO_SURFACE)
			eglDestroySurface(display, surface);
		if (context != EGL_NO_CONTEXT)
			eglDestroyContext(display, context);
		eglTerminate(display);
	}
}


#ifdef _QN_WINDOWS_
//////////////////////////////////////////////////////////////////////////
// WGL 확장
#include "glad/wgl.h"

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"WGL"




#endif // _QN_WINDOWS_

#endif // USE_GL
