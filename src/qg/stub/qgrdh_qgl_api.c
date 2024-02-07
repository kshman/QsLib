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
// 공용

// 후보 컨피그 중에서 가장 적합한 것 찾기
static const QglConfig* qgl_detect_config(_In_ const QglConfig* wanted, _In_ const QglCtnConfig* configs)
{
	uint least_missing = UINT_MAX;
	uint least_color = UINT_MAX;
	uint least_extra = UINT_MAX;
	const QglConfig* found = NULL;
	size_t i;
	QN_CTNR_FOREACH(*configs, i)
	{
		const QglConfig* c = qgl_cfg_ctnr_nth_ptr(configs, i);
		if (c->handle == NULL)
			continue;

		uint missing = 0;
		if (wanted->alpha > 0 && c->alpha == 0) missing++;
		if (wanted->depth > 0 && c->depth == 0) missing++;
		if (wanted->stencil > 0 && c->stencil == 0) missing++;
		if (wanted->samples > 0 && c->samples == 0) missing++;
		if (wanted->transparent != c->transparent) missing++;

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

static int glad_find_extensions_egl(EGLDisplay display)
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
	static const char* egl_libs[] =
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
	for (size_t i = 0; egl_libs[i]; i++)
	{
		egl_module = qn_open_mod(egl_libs[i], 1);
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
static bool glad_load_egl_gl(void)
{
	static int version = 0;
	if (version != 0)
		return true;
	version = gladLoadGL(eglGetProcAddress);
	return version != 0;
}

//
static bool glad_load_egl_gl_es2(void)
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
#ifdef _QN_EMSCRIPTEN_
	return qn_p_unknown(error, true);
#else
#define ERROR_CASE(x)		case x: return #x
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
			return qn_p_unknown(error, true);
	}
#undef ERROR_CASE
#endif
}

// eglGetConfigAttrib 편하게 쓰자고 대리자
static EGLint egl_get_config_attrib(_In_ EGLDisplay display, _In_ EGLConfig config, _In_ EGLint name)
{
	EGLint v;
	eglGetConfigAttrib(display, config, name, &v);
	return v;
}

// EGL 설정 읽기
static void egl_get_config(_Out_ QglConfig* config, _In_ EGLDisplay display, _In_ EGLConfig ec, _In_ const QglConfig* wanted_config)
{
	config->version = wanted_config->version;
	config->red = (byte)egl_get_config_attrib(display, ec, EGL_RED_SIZE);
	config->green = (byte)egl_get_config_attrib(display, ec, EGL_GREEN_SIZE);
	config->blue = (byte)egl_get_config_attrib(display, ec, EGL_BLUE_SIZE);
	config->alpha = (byte)egl_get_config_attrib(display, ec, EGL_ALPHA_SIZE);
	config->depth = (byte)egl_get_config_attrib(display, ec, EGL_DEPTH_SIZE);
	config->stencil = (byte)egl_get_config_attrib(display, ec, EGL_STENCIL_SIZE);
	config->samples = (byte)egl_get_config_attrib(display, ec, EGL_SAMPLES);
	config->srgb = wanted_config->srgb;
	config->stereo = wanted_config->stereo;
	config->transparent = egl_get_config_attrib(display, ec, EGL_TRANSPARENT_TYPE) == EGL_TRANSPARENT_RGB;
	config->float_buffer = wanted_config->float_buffer;
	config->no_error = wanted_config->no_error;
	config->robustness = wanted_config->robustness;
	config->core = wanted_config->core;
	config->handle = ec;
}

// EGL 설정 얻기
static bool egl_choose_config(_In_ EGLDisplay display, _In_ const QglConfig* wanted_config, _Out_ QglConfig* found_config)
{
#define ATTR_ADD(e,v)	QN_STMT_BEGIN{ attrs[i++] = e; attrs[i++] = v; }QN_STMT_END
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
	qn_debug_verify((size_t)i < QN_COUNTOF(attrs));
#undef ATTR_ADD

	EGLint config_count = 0;
	EGLConfig configs[64];
	if (eglChooseConfig(display, attrs, configs, 64, &config_count) == EGL_FALSE || config_count == 0)
		VAR_CHK_RET(egl_error_string(eglGetError()), false);

	bool ret;
	if (config_count == 1)
	{
		ret = true;
		egl_get_config(found_config, display, configs[0], wanted_config);
	}
	else
	{
		QglCtnConfig qgl_configs;
		qgl_cfg_ctnr_init(&qgl_configs, config_count);
		for (i = 0; i < config_count; i++)
			egl_get_config(qgl_cfg_ctnr_nth_ptr(&qgl_configs, i), display, configs[i], wanted_config);

		const QglConfig* found = qgl_detect_config(wanted_config, &qgl_configs);
		if (found == NULL)
			ret = false;
		else
		{
			memcpy(found_config, found, sizeof(QglConfig));
			ret = true;
		}
		qgl_cfg_ctnr_dispose(&qgl_configs);
	}
	return ret;
}

// EGL 초기화
EGLDisplay egl_initialize(_In_ const QglConfig* wanted_config)
{
	qn_debug_verify(wanted_config != NULL);
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
#else
	// EMSCRIPTEN 이외에서는 필요가 없다
	QN_DUMMY(wanted_config);
#endif

	const EGLDisplay display = eglGetDisplay((EGLNativeDisplayType)stub_system_get_display());
	if (display == EGL_NO_DISPLAY)
		VAR_CHK_RET("failed to get display", EGL_NO_DISPLAY);

	EGLint major, minor;
	if (eglInitialize(display, &major, &minor) == false)
	{
		const char* error = egl_error_string(eglGetError());
		eglTerminate(display);
		VAR_CHK_RET(error, EGL_NO_DISPLAY);
	}

#ifndef QGL_EGL_NO_EXT
	glad_find_extensions_egl(display);
#endif

	return display;
}

// EGL 컨텍스트 만들기
EGLContext egl_create_context(_In_ EGLDisplay display, _In_ const QglConfig* wanted_config, _Out_ QglConfig* config)
{
	qn_debug_verify(display != EGL_NO_DISPLAY);
	qn_debug_verify(wanted_config != NULL);
	qn_debug_verify(config != NULL);

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
		EGLint mask = 0, flags = 0;
		ATTR_ADD(EGL_CONTEXT_MAJOR_VERSION, config->version / 100);
		ATTR_ADD(EGL_CONTEXT_MINOR_VERSION, config->version % 100);
#ifndef _QN_MOBILE_
		if (config->core)
		{
			// 일단 ANGLE은 안됨, LINUX/BSD는 됨
			profile = EGL_OPENGL_API;
			mask |= EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
		}
#endif
		if (config->no_error)
			ATTR_ADD(EGL_CONTEXT_OPENGL_NO_ERROR_KHR, EGL_TRUE);
		if (config->robustness != 0)
		{
			ATTR_ADD(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY,
				config->robustness == 1 ? EGL_NO_RESET_NOTIFICATION : EGL_LOSE_CONTEXT_ON_RESET);
			flags |= EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
		}
		if (mask != 0)
			ATTR_ADD(EGL_CONTEXT_OPENGL_PROFILE_MASK, mask);
		if (flags != 0)
			ATTR_ADD(EGL_CONTEXT_FLAGS_KHR, flags);
	}
	else
#endif
	{
#ifdef _QN_EMSCRIPTEN_
		ATTR_ADD(EGL_CONTEXT_CLIENT_VERSION, 3);
#else
		ATTR_ADD(EGL_CONTEXT_CLIENT_VERSION, config->version / 100);
#endif
	}
	ATTR_ADD(EGL_NONE, EGL_NONE);
#undef ATTR_ADD

	// 바인드 먼저
	if (eglBindAPI(profile) == false)
		VAR_CHK_GOTO(egl_error_string(eglGetError()), pos_error_exit);

	// 컨텍스트 만들고
	EGLContext context = eglCreateContext(display, config->handle, NULL, attrs);
	if (context == EGL_NO_CONTEXT)
	{
#ifdef _QN_EMSCRIPTEN_
		attrs[1] = 2;
		context = eglCreateContext(display, config->handle, NULL, attrs);
#else
		const int index = qgl_index_of_opengl_version(config->core, config->version);
		if (index >= 0)
		{
			for (int i = index + 1; ; i++)
			{
				const int version = qgl_get_opengl_version(config->core, i);
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
			VAR_CHK_GOTO(egl_error_string(eglGetError()), pos_error_exit);
	}

	return context;

pos_error_exit:
	eglTerminate(display);
	return EGL_NO_CONTEXT;
}

// 서피스 만들기
EGLSurface egl_create_surface(_In_ EGLDisplay display, _In_ EGLContext context, _In_ const QglConfig* config, _In_ int visual_id)
{
	QN_DUMMY(visual_id);
	qn_debug_verify(display != EGL_NO_DISPLAY);
	qn_debug_verify(context != EGL_NO_CONTEXT);
	qn_debug_verify(config != NULL);

	// 속성 설정
#ifdef QGL_EGL_NO_EXT
	EGLint attrs[] = { EGL_NONE, EGL_NONE };
#else
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[attr_count++] = e; attrs[attr_count++] = v; }QN_STMT_END
	EGLint attrs[10], attr_count = 0;
	if (GLAD_EGL_KHR_gl_colorspace && config->srgb)
		ATTR_ADD(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB);
	if (GLAD_EGL_EXT_present_opaque && config->transparent)
		ATTR_ADD(EGL_PRESENT_OPAQUE_EXT, EGL_TRUE);
	ATTR_ADD(EGL_NONE, EGL_NONE);
#undef ATTR_ADD
#endif

	EGLNativeWindowType native_window = (EGLNativeWindowType)stub_system_get_window();
#ifdef _QN_ANDROID_
	EGLint format;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(native_window, 0, 0, format);
#endif
	const EGLSurface surface = eglCreateWindowSurface(display, config->handle, native_window, attrs);
	if (surface == EGL_NO_SURFACE)
	{
		const char* error = egl_error_string(eglGetError());
		eglDestroyContext(display, context);
		eglTerminate(display);
		VAR_CHK_RET(error, EGL_NO_SURFACE);
	}
#ifdef _QN_ANDROID_
	int android_format = ANativeWindow_getFormat(native_window);
	Android_SetFormat(format, android_format);
#endif

	return surface;
}

// 커런트 만들기
bool egl_make_current(_In_ EGLDisplay display, _In_ EGLSurface surface, _In_ EGLContext context, _In_ const QglConfig* config)
{
	qn_debug_verify(display != EGL_NO_DISPLAY);
	qn_debug_verify(surface != EGL_NO_SURFACE);
	qn_debug_verify(context != EGL_NO_CONTEXT);

	if (eglMakeCurrent(display, surface, surface, context) == false)
		VAR_CHK_RET(egl_error_string(eglGetError()), false);

#ifndef QGL_LINK_STATIC
	if ((config->core ? glad_load_egl_gl() : glad_load_egl_gl_es2()) == false)
		VAR_CHK_RET("failed to load proc", false);
#endif // QGL_LINK_STATIC

	return true;
}

// 모든 EGL 자원 해제
void egl_dispose(_In_ EGLDisplay display, _In_ EGLSurface surface, _In_ EGLContext context)
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

int GLAD_WGL_ARB_create_context = 0;
int GLAD_WGL_ARB_create_context_robustness = 0;
int GLAD_WGL_ARB_extensions_string = 0;
int GLAD_WGL_ARB_multisample = 0;
int GLAD_WGL_ARB_pixel_format = 0;
int GLAD_WGL_ARB_pixel_format_float = 0;
int GLAD_WGL_EXT_colorspace = 0;
int GLAD_WGL_EXT_create_context_es2_profile = 0;
int GLAD_WGL_EXT_create_context_es_profile = 0;
int GLAD_WGL_EXT_depth_float = 0;
int GLAD_WGL_EXT_extensions_string = 0;
int GLAD_WGL_EXT_swap_control = 0;
int GLAD_WGL_NV_float_buffer = 0;
int GLAD_WGL_ARB_create_context_no_error = 0;

PFNWGLGETPROCADDRESSPROC glad_wglGetProcAddress = NULL;
PFNWGLCREATECONTEXTPROC glad_wglCreateContext = NULL;
PFNWGLDELETECONTEXTPROC glad_wglDeleteContext = NULL;
PFNWGLMAKECURRENTPROC glad_wglMakeCurrent = NULL;
PFNWGLGETCURRENTDCPROC glad_wglGetCurrentDC = NULL;
PFNWGLGETCURRENTCONTEXTPROC glad_wglGetCurrentContext = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC glad_wglChoosePixelFormatARB = NULL;
PFNWGLCREATECONTEXTATTRIBSARBPROC glad_wglCreateContextAttribsARB = NULL;
PFNWGLGETEXTENSIONSSTRINGARBPROC glad_wglGetExtensionsStringARB = NULL;
PFNWGLGETEXTENSIONSSTRINGEXTPROC glad_wglGetExtensionsStringEXT = NULL;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC glad_wglGetPixelFormatAttribfvARB = NULL;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC glad_wglGetPixelFormatAttribivARB = NULL;
PFNWGLGETSWAPINTERVALEXTPROC glad_wglGetSwapIntervalEXT = NULL;
PFNWGLSWAPINTERVALEXTPROC glad_wglSwapIntervalEXT = NULL;

static int glad_find_extensions_wgl(HDC hdc)
{
	if (wglGetExtensionsStringEXT == NULL && wglGetExtensionsStringARB == NULL)
		return 0;

	const char* extensions;
	if (wglGetExtensionsStringARB != NULL && hdc != INVALID_HANDLE_VALUE)
		extensions = wglGetExtensionsStringARB(hdc);
	else if (wglGetExtensionsStringEXT != NULL)
		extensions = wglGetExtensionsStringEXT();
	else
		return 0;
	if (extensions == NULL) return 0;

	GLAD_WGL_ARB_create_context = qn_strext(extensions, "WGL_ARB_create_context", ' ') != 0;
	GLAD_WGL_ARB_create_context_robustness = qn_strext(extensions, "WGL_ARB_create_context_robustness", ' ') != 0;
	GLAD_WGL_ARB_extensions_string = qn_strext(extensions, "WGL_ARB_extensions_string", ' ') != 0;
	GLAD_WGL_ARB_multisample = qn_strext(extensions, "WGL_ARB_multisample", ' ') != 0;
	GLAD_WGL_ARB_pixel_format = qn_strext(extensions, "WGL_ARB_pixel_format", ' ') != 0;
	GLAD_WGL_ARB_pixel_format_float = qn_strext(extensions, "WGL_ARB_pixel_format_float", ' ') != 0;
	GLAD_WGL_EXT_colorspace = qn_strext(extensions, "WGL_EXT_colorspace", ' ') != 0;
	GLAD_WGL_EXT_create_context_es2_profile = qn_strext(extensions, "WGL_EXT_create_context_es2_profile", ' ') != 0;
	GLAD_WGL_EXT_create_context_es_profile = qn_strext(extensions, "WGL_EXT_create_context_es_profile", ' ') != 0;
	GLAD_WGL_EXT_depth_float = qn_strext(extensions, "WGL_EXT_depth_float", ' ') != 0;
	GLAD_WGL_EXT_extensions_string = qn_strext(extensions, "WGL_EXT_extensions_string", ' ') != 0;
	GLAD_WGL_EXT_swap_control = qn_strext(extensions, "WGL_EXT_swap_control", ' ') != 0;
	GLAD_WGL_NV_float_buffer = qn_strext(extensions, "WGL_NV_float_buffer", ' ') != 0;
	GLAD_WGL_ARB_create_context_no_error = qn_strext(extensions, "WGL_ARB_create_context_no_error", ' ') != 0;
	return 1;
}

//
static HWND wgl_create_dummy_window(void)
{
	const HWND hwnd = CreateWindow(stub_system_get_class_name(), L"qgl_wgl", WS_POPUP | WS_DISABLED,
		0, 0, 1, 1, NULL, NULL, (HINSTANCE)stub_system_get_instance(), NULL);
	stub_system_poll();
	return hwnd;
}

//
static void wgl_destroy_dummy_window(HWND hwnd)
{
	DestroyWindow(hwnd);
	stub_system_poll();
}

//
static QnModule* wgl_module = NULL;

#ifdef _MSC_VER
#pragma warning(disable:4191)
#endif

//
static bool glad_load_wgl(void)
{
	static bool init = false;
	if (init)
		return true;
	wgl_module = qn_open_mod("opengl32", 1);
	VAR_CHK_IF_COND(wgl_module == NULL, "cannot load wgl library", false);

	glad_wglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)qn_mod_func(wgl_module, "wglGetProcAddress");
	glad_wglCreateContext = (PFNWGLCREATECONTEXTPROC)qn_mod_func(wgl_module, "wglCreateContext");
	glad_wglDeleteContext = (PFNWGLDELETECONTEXTPROC)qn_mod_func(wgl_module, "wglDeleteContext");
	glad_wglMakeCurrent = (PFNWGLMAKECURRENTPROC)qn_mod_func(wgl_module, "wglMakeCurrent");
	glad_wglGetCurrentDC = (PFNWGLGETCURRENTDCPROC)qn_mod_func(wgl_module, "wglGetCurrentDC");
	glad_wglGetCurrentContext = (PFNWGLGETCURRENTCONTEXTPROC)qn_mod_func(wgl_module, "wglGetCurrentContext");

	bool ret = false;

	const HWND hwnd = wgl_create_dummy_window();
	const HDC hdc = GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), };
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	if (!SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd))
		goto pos_error;

	const HGLRC hglrc = wglCreateContext(hdc);
	if (hglrc == NULL)
		goto pos_error;

	const HDC wdc = wglGetCurrentDC();
	const HGLRC wglrc = wglGetCurrentContext();
	if (wglMakeCurrent(hdc, hglrc) == FALSE)
	{
		wglMakeCurrent(wdc, wglrc);
		wglDeleteContext(hglrc);
		goto pos_error;
	}

	glad_wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	glad_wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	glad_wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	glad_wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
	glad_wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
	glad_wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
	glad_wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
	glad_wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

	glad_find_extensions_wgl(hdc);
	wglMakeCurrent(wdc, wglrc);
	wglDeleteContext(hglrc);
	ret = true;

pos_error:
	wgl_destroy_dummy_window(hwnd);
	return ret;
}

//
static bool glad_load_wgl_gl(void)
{
	static int version = 0;
	if (version != 0)
		return true;
	version = gladLoadGL((GLADloadfunc)wglGetProcAddress);
	return version != 0;
}

//
static bool glad_load_wgl_gles2(void)
{
	static int version = 0;
	if (version != 0)
		return true;
	version = gladLoadGLES2((GLADloadfunc)wglGetProcAddress);
	return version != 0;
}

#ifdef _MSC_VER
#pragma warning(default:4191)
#endif

// eglGetConfigAttrib 같은 함수가 없다...
static int wgl_find_format_attr(int count, const int* attrs, const int* values, int attr)
{
	for (int i = 0; i < count; i++)
	{
		if (attrs[i] == attr)
			return values[i];
	}
	return 0;
}

// 사양 확인
_Success_(return) static bool wgl_get_config(_Out_ QglConfig * config,
	_In_ int attr_count, _In_ const int* attrs, _In_ const int* values,
	_In_ int pixel_format, _In_ const QglConfig * wanted_config)
{
#define ATTR_FIND(e)		wgl_find_format_attr(attr_count, attrs, values, e)
	if (ATTR_FIND(WGL_SUPPORT_OPENGL_ARB) == false ||
		ATTR_FIND(WGL_DRAW_TO_WINDOW_ARB) == false)
		return false;
	if (ATTR_FIND(WGL_PIXEL_TYPE_ARB) != WGL_TYPE_RGBA_ARB)
		return false;
	if (ATTR_FIND(WGL_DOUBLE_BUFFER_ARB) == false)
		return false;
	config->handle = (void*)(nuint)pixel_format;
	config->version = wanted_config->version;
	config->red = (byte)ATTR_FIND(WGL_RED_BITS_ARB);
	config->green = (byte)ATTR_FIND(WGL_GREEN_BITS_ARB);
	config->blue = (byte)ATTR_FIND(WGL_BLUE_BITS_ARB);
	config->alpha = (byte)ATTR_FIND(WGL_ALPHA_BITS_ARB);
	config->depth = (byte)ATTR_FIND(WGL_DEPTH_BITS_ARB);
	config->stencil = (byte)ATTR_FIND(WGL_STENCIL_BITS_ARB);
	config->samples = (byte)ATTR_FIND(WGL_SAMPLES_ARB);
	if (GLAD_WGL_EXT_colorspace)
	{
		config->srgb = ATTR_FIND(WGL_COLORSPACE_EXT) == WGL_COLORSPACE_SRGB_EXT;
		config->transparent = ATTR_FIND(WGL_TRANSPARENT_ARB) != 0;
	}
	else
	{
		config->srgb = 0;
		config->transparent = 0;
	}
	config->stereo = ATTR_FIND(WGL_STEREO_ARB) != 0;
	config->float_buffer = GLAD_WGL_NV_float_buffer ? (byte)ATTR_FIND(WGL_FLOAT_COMPONENTS_NV) : 0;
	config->no_error = GLAD_WGL_ARB_create_context_no_error ? wanted_config->no_error : 0;
	config->robustness = GLAD_WGL_ARB_create_context_robustness ? wanted_config->robustness : 0;
	config->core = wanted_config->core;
	config->handle = NULL;
#undef ATTR_FIND
	return true;
}

// ARB 픽셀 포맷을 고른다
_Success_(return) static bool wgl_choose_arb_pixel_format(_In_ HDC hdc, _In_ const QglConfig * wanted_config, _Out_ QglConfig * found_config)
{
#define ATTR_ADD(e,v)	QN_STMT_BEGIN{ attrs[attr_count++]=e; attrs[attr_count++]=v; }QN_STMT_END
	int attrs[40], attr_count = 0;
	ATTR_ADD(WGL_SUPPORT_OPENGL_ARB, GL_TRUE);
	ATTR_ADD(WGL_DRAW_TO_WINDOW_ARB, GL_TRUE);
	ATTR_ADD(WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
	ATTR_ADD(WGL_DOUBLE_BUFFER_ARB, GL_TRUE);
	ATTR_ADD(WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB);
	ATTR_ADD(WGL_RED_BITS_ARB, wanted_config->red);
	ATTR_ADD(WGL_GREEN_BITS_ARB, wanted_config->green);
	ATTR_ADD(WGL_BLUE_BITS_ARB, wanted_config->blue);
	if (wanted_config->alpha > 0)
		ATTR_ADD(WGL_ALPHA_BITS_ARB, wanted_config->alpha);
	if (wanted_config->depth > 0)
		ATTR_ADD(WGL_DEPTH_BITS_ARB, wanted_config->depth);
	if (wanted_config->stencil > 0)
		ATTR_ADD(WGL_STENCIL_BITS_ARB, wanted_config->stencil);
	if (GLAD_WGL_ARB_multisample && wanted_config->samples > 0)
	{
		ATTR_ADD(WGL_SAMPLE_BUFFERS_ARB, GL_TRUE);
		ATTR_ADD(WGL_SAMPLES_ARB, wanted_config->samples);
	}
	if (GLAD_WGL_EXT_colorspace)
	{
		if (wanted_config->srgb)
			ATTR_ADD(WGL_COLORSPACE_EXT, WGL_COLORSPACE_SRGB_EXT);
		if (wanted_config->transparent)
			ATTR_ADD(WGL_TRANSPARENT_ARB, GL_TRUE);
	}
	if (wanted_config->stereo)
		ATTR_ADD(WGL_STEREO_ARB, GL_TRUE);
	if (GLAD_WGL_NV_float_buffer && wanted_config->float_buffer > 0)
		ATTR_ADD(WGL_FLOAT_COMPONENTS_NV, GL_TRUE);
	ATTR_ADD(0, 0);
#undef ATTR_ADD

	UINT config_count;
	int pixel_formats[64];
	if (wglChoosePixelFormatARB(hdc, attrs, NULL, 64, pixel_formats, &config_count) == FALSE || config_count == 0)
		return false;
	qn_debug_verify(config_count < QN_COUNTOF(pixel_formats));

#define ATTR_ADD(e)		QN_STMT_BEGIN{ attrs[attr_count++]=e; }QN_STMT_END
	attr_count = 0;
	ATTR_ADD(WGL_SUPPORT_OPENGL_ARB);
	ATTR_ADD(WGL_DRAW_TO_WINDOW_ARB);
	ATTR_ADD(WGL_PIXEL_TYPE_ARB);
	ATTR_ADD(WGL_ACCELERATION_ARB);
	ATTR_ADD(WGL_DOUBLE_BUFFER_ARB);
	ATTR_ADD(WGL_RED_BITS_ARB);
	ATTR_ADD(WGL_GREEN_BITS_ARB);
	ATTR_ADD(WGL_BLUE_BITS_ARB);
	ATTR_ADD(WGL_ALPHA_BITS_ARB);
	ATTR_ADD(WGL_DEPTH_BITS_ARB);
	ATTR_ADD(WGL_STENCIL_BITS_ARB);
	if (GLAD_WGL_ARB_multisample)
		ATTR_ADD(WGL_SAMPLES_ARB);
	ATTR_ADD(WGL_STEREO_ARB);
	if (GLAD_WGL_EXT_colorspace)
	{
		ATTR_ADD(WGL_COLORSPACE_EXT);
		ATTR_ADD(WGL_TRANSPARENT_ARB);
	}
	if (GLAD_WGL_NV_float_buffer)
		ATTR_ADD(WGL_FLOAT_COMPONENTS_NV);
#undef ATTR_ADD

	bool ret;
	int values[20] = { 0, };
	if (config_count == 1)
	{
		if (wglGetPixelFormatAttribivARB(hdc, pixel_formats[0], 0, attr_count, attrs, values) == FALSE)
			return false;
		ret = true;
		wgl_get_config(found_config, attr_count, attrs, values, pixel_formats[0], wanted_config);
	}
	else
	{
		QglCtnConfig qgl_configs;
		qgl_cfg_ctnr_init(&qgl_configs, config_count);
		for (UINT i = 0; i < config_count; i++)
		{
			QglConfig* c = qgl_cfg_ctnr_nth_ptr(&qgl_configs, i);
			if (wglGetPixelFormatAttribivARB(hdc, pixel_formats[i], 0, attr_count, attrs, values) == FALSE)
			{
				c->handle = NULL;
				continue;
			}
			wgl_get_config(c, attr_count, attrs, values, pixel_formats[i], wanted_config);
		}

		const QglConfig* found = qgl_detect_config(wanted_config, &qgl_configs);
		if (found == NULL)
			ret = false;
		else
		{
			memcpy(found_config, found, sizeof(QglConfig));
			ret = true;
		}
		qgl_cfg_ctnr_dispose(&qgl_configs);
	}
	return ret;
}

//
_Success_(return) static bool wgl_choose_desc_pixel_format(_In_ HDC hdc, _In_ const QglConfig * wanted_config, _Out_ QglConfig * found_config)
{
	// https://learn.microsoft.com/ko-kr/windows/win32/api/wingdi/ns-wingdi-pixelformatdescriptor
	PIXELFORMATDESCRIPTOR target = { sizeof(PIXELFORMATDESCRIPTOR), 1, };
	target.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	if (wanted_config->stereo)
		target.dwFlags |= PFD_STEREO;
	target.iPixelType = PFD_TYPE_RGBA;
	target.cColorBits = wanted_config->red + wanted_config->green + wanted_config->blue;
	target.cRedBits = wanted_config->red;
	target.cGreenBits = wanted_config->green;
	target.cBlueBits = wanted_config->blue;
	target.cAlphaBits = wanted_config->alpha;
	target.cDepthBits = wanted_config->depth;
	target.cStencilBits = wanted_config->stencil;

	const int desc_count = DescribePixelFormat(hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), NULL);
	int best_format = -1;
	uint best_dist = UINT_MAX;
	for (int i = 1; i < desc_count; i++)
	{
		PIXELFORMATDESCRIPTOR pfd;
		if (DescribePixelFormat(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
			continue;
		if ((pfd.dwFlags & target.dwFlags) != target.dwFlags)
			continue;
		if (pfd.iPixelType != target.iPixelType)
			continue;
		if (pfd.iLayerType != target.iLayerType)
			continue;
		if (pfd.cColorBits < target.cColorBits)
			continue;
		if (pfd.cRedBits < target.cRedBits)
			continue;
		if (pfd.cGreenBits < target.cGreenBits)
			continue;
		if (pfd.cBlueBits < target.cBlueBits)
			continue;
		if (pfd.cAlphaBits < target.cAlphaBits)
			continue;
		if (pfd.cDepthBits < target.cDepthBits)
			continue;
		if (pfd.cStencilBits < target.cStencilBits)
			continue;
		uint dist = 0;
		dist += pfd.cColorBits - target.cColorBits;
		dist += pfd.cRedBits - target.cRedBits;
		dist += pfd.cGreenBits - target.cGreenBits;
		dist += pfd.cBlueBits - target.cBlueBits;
		dist += pfd.cAlphaBits - target.cAlphaBits;
		dist += pfd.cDepthBits - target.cDepthBits;
		dist += pfd.cStencilBits - target.cStencilBits;
		if (dist < best_dist)
		{
			best_format = i;
			best_dist = dist;
		}
	}

	if (best_format < 0)
		return false;

	DescribePixelFormat(hdc, best_format, sizeof(PIXELFORMATDESCRIPTOR), &target);
	found_config->version = wanted_config->version;
	found_config->red = target.cRedBits;
	found_config->green = target.cGreenBits;
	found_config->blue = target.cBlueBits;
	found_config->alpha = target.cAlphaBits;
	found_config->depth = target.cDepthBits;
	found_config->stencil = target.cStencilBits;
	found_config->samples = 0;
	found_config->srgb = 0;
	found_config->stereo = wanted_config->stereo;
	found_config->transparent = 0;
	found_config->float_buffer = 0;
	found_config->no_error = 0;
	found_config->robustness = 0;
	found_config->core = wanted_config->core;
	found_config->handle = (void*)(nuint)best_format;
	return true;
}

//
static bool wgl_choose_pixel_format(_In_ const QglConfig * wanted_config, _Out_ QglConfig * found_config)
{
	qn_debug_verify(wanted_config != NULL);
	qn_debug_verify(found_config != NULL);

	const HWND hwnd = wgl_create_dummy_window();
	const HDC hdc = GetDC(hwnd);

	bool isok = false;
	if (GLAD_WGL_ARB_pixel_format)
		isok = wgl_choose_arb_pixel_format(hdc, wanted_config, found_config);
	if (isok == false)
		isok = wgl_choose_desc_pixel_format(hdc, wanted_config, found_config);

	ReleaseDC(hwnd, hdc);
	wgl_destroy_dummy_window(hwnd);
	return isok;
}

//
static HGLRC wgl_create_context_arb(_In_ HDC hdc, _In_ int* attrs, _In_ int version, _In_ bool core)
{
	const int index = qgl_index_of_opengl_version(core, version);
	if (index < 0)
		return NULL;

	for (int i = index; ; i++)
	{
		version = qgl_get_opengl_version(core, i);
		if (version < 0)
			break;

		attrs[1] = version / 100;
		attrs[3] = version % 100;
		const HGLRC context = wglCreateContextAttribsARB(hdc, NULL, attrs);
		if (context != NULL)
			return context;

		const DWORD dw = GetLastError();
		if (dw == ERROR_INVALID_VERSION_ARB)
		{
			qn_mesgf(false, VAR_CHK_NAME, "driver not support version %d.%d", attrs[1], attrs[3]);
			continue;
		}
		if (dw == ERROR_INVALID_PROFILE_ARB)
			qn_mesg(true, VAR_CHK_NAME, "driver not support profile");
		else if (dw == ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB)
			qn_mesg(true, VAR_CHK_NAME, "driver not support context");
		else if (dw == ERROR_INVALID_PARAMETER)
			qn_mesg(true, VAR_CHK_NAME, "invalid parameter");
		else if (dw == ERROR_INVALID_PIXEL_FORMAT)
			qn_mesg(true, VAR_CHK_NAME, "invalid pixel format");
		else if (dw == ERROR_NO_SYSTEM_RESOURCES)
			qn_mesg(true, VAR_CHK_NAME, "no system resources");
		else
			qn_mesg(true, VAR_CHK_NAME, qn_p_unknown((int)dw, true));
		break;
	}
	return NULL;
}

//
bool gl_initialize(void)
{
	if (glad_load_wgl() == false)
		return false;
	return true;
}

//
void* gl_create_context(_In_ const QglConfig * wanted_config, _Out_ QglConfig * config)
{
	qn_debug_verify(wanted_config != NULL);
	qn_debug_verify(config != NULL);

	if (wanted_config->core == false && GLAD_WGL_EXT_create_context_es2_profile == 0)
	{
		config->version = 0;
		VAR_CHK_RET("driver not support opengl es", NULL);
	}

	if (wgl_choose_pixel_format(wanted_config, config) == false)
		return NULL;

	const HDC hdc = (HDC)stub_system_get_display();
	PIXELFORMATDESCRIPTOR pfd;
	if (DescribePixelFormat(hdc, (int)(nuint)config->handle, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
		goto pos_error;
	if (SetPixelFormat(hdc, (int)(nuint)config->handle, &pfd) == FALSE)
		goto pos_error;

	HGLRC context = NULL;
	if (GLAD_WGL_ARB_create_context)
	{
#define ATTR_ADD(e, v)		QN_STMT_BEGIN{ attrs[attr_count++] = e; attrs[attr_count++] = v; }QN_STMT_END
		int attrs[20], attr_count = 0, flags = 0;
#ifdef _DEBUG
		flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif
		ATTR_ADD(WGL_CONTEXT_MAJOR_VERSION_ARB, config->version / 100);
		ATTR_ADD(WGL_CONTEXT_MINOR_VERSION_ARB, config->version % 100);
		// WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB / WGL_CONTEXT_CORE_PROFILE_BIT_ARB / WGL_CONTEXT_ES2_PROFILE_BIT_EXT
		ATTR_ADD(WGL_CONTEXT_PROFILE_MASK_ARB, config->core ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_ES2_PROFILE_BIT_EXT);
		if (GLAD_WGL_ARB_create_context_no_error && config->no_error != 0)
			ATTR_ADD(WGL_CONTEXT_OPENGL_NO_ERROR_ARB, GL_TRUE);
		if (GLAD_WGL_ARB_create_context_robustness && config->robustness != 0)
		{
			ATTR_ADD(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
				config->robustness == 1 ? WGL_NO_RESET_NOTIFICATION_ARB : WGL_LOSE_CONTEXT_ON_RESET_ARB);
			flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
		}
		if (flags)
			ATTR_ADD(WGL_CONTEXT_FLAGS_ARB, flags);
		ATTR_ADD(0, 0);
#undef ATTR_ADD

		context = wgl_create_context_arb(hdc, attrs, config->version, config->core);
	}
	if (context == NULL)
	{
		context = wglCreateContext(hdc);
		if (context == NULL)
			goto pos_error;
	}

	return context;

pos_error:
	return NULL;
}

//
bool gl_make_current(_In_ void* context, _In_ const QglConfig * config)
{
	qn_debug_verify(context != NULL);

	const HDC hdc = (HDC)stub_system_get_display();
	if (wglMakeCurrent(hdc, (HGLRC)context) == FALSE)
		goto pos_error_exit;

	if ((config->core ? glad_load_wgl_gl() : glad_load_wgl_gles2()) == false)
		VAR_CHK_GOTO("failed to load proc", pos_error_exit);

	return true;

pos_error_exit:
	wglMakeCurrent(hdc, NULL);
	wglDeleteContext((HGLRC)context);
	return false;
}

//
bool gl_swap_buffers(void)
{
	const HDC hdc = (HDC)stub_system_get_display();
	SwapBuffers(hdc);
	return true;
}

//
bool gl_swap_interval(_In_ int interval)
{
	if (GLAD_WGL_EXT_swap_control == 0)
		return false;
	return (bool)wglSwapIntervalEXT(interval);
}

//
void gl_dispose(_In_ void* context)
{
	qn_return_when_fail(context != NULL, /*void*/);

	const HDC hdc = (HDC)stub_system_get_display();
	wglMakeCurrent(hdc, NULL);
	wglDeleteContext((HGLRC)context);
}

#else // _QN_WINDOWS_
//////////////////////////////////////////////////////////////////////////
// GL 확장이 없네!

#endif // _QN_WINDOWS_

#endif // USE_GL
