//
// glad_egl.c - EGL 대리자
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

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"EGL"


//////////////////////////////////////////////////////////////////////////
// EGL 확장

int GLAD_EGL_EXT_platform_base = 0;
int GLAD_EGL_EXT_platform_device = 0;
int GLAD_EGL_EXT_platform_wayland = 0;
int GLAD_EGL_EXT_platform_x11 = 0;
int GLAD_EGL_EXT_present_opaque = 0;
int GLAD_EGL_KHR_context_flush_control = 0;
int GLAD_EGL_KHR_create_context = 0;
int GLAD_EGL_KHR_create_context_no_error = 0;
int GLAD_EGL_KHR_get_all_proc_addresses = 0;
int GLAD_EGL_KHR_gl_colorspace = 0;

static int glad_egl_get_extensions(EGLDisplay display, const char **extensions) {
	*extensions = eglQueryString(display, EGL_EXTENSIONS);
	return extensions != NULL;
}

static int glad_egl_has_extension(const char *extensions, const char *ext) {
	return qn_strext(extensions, ext, ' ') != NULL;
}

static int glad_egl_find_extensions_egl(EGLDisplay display) {
	const char *extensions;
	if (!glad_egl_get_extensions(display, &extensions)) return 0;

	GLAD_EGL_EXT_platform_base = glad_egl_has_extension(extensions, "EGL_EXT_platform_base");
	GLAD_EGL_EXT_platform_device = glad_egl_has_extension(extensions, "EGL_EXT_platform_device");
	GLAD_EGL_EXT_platform_wayland = glad_egl_has_extension(extensions, "EGL_EXT_platform_wayland");
	GLAD_EGL_EXT_platform_x11 = glad_egl_has_extension(extensions, "EGL_EXT_platform_x11");
	GLAD_EGL_EXT_present_opaque = glad_egl_has_extension(extensions, "EGL_EXT_present_opaque");
	GLAD_EGL_KHR_context_flush_control = glad_egl_has_extension(extensions, "EGL_KHR_context_flush_control");
	GLAD_EGL_KHR_create_context = glad_egl_has_extension(extensions, "EGL_KHR_create_context");
	GLAD_EGL_KHR_create_context_no_error = glad_egl_has_extension(extensions, "EGL_KHR_create_context_no_error");
	GLAD_EGL_KHR_get_all_proc_addresses = glad_egl_has_extension(extensions, "EGL_KHR_get_all_proc_addresses");
	GLAD_EGL_KHR_gl_colorspace = glad_egl_has_extension(extensions, "EGL_KHR_gl_colorspace");

#ifdef _DEBUG
	char* brk = qn_strchr(extensions, ' ');
	while (brk != NULL)
	{
		char buf[64];
		size_t len = brk - extensions;
		if (len > 63) len = 63;
		qn_strncpy(buf, extensions, len);
		qn_debug_outputf(false, VAR_CHK_NAME, "EGL extension: %s", buf);
		extensions = brk + 1;
		brk = qn_strchr(extensions, ' ');
	}
#endif

	return 1;
}


//////////////////////////////////////////////////////////////////////////
// GLAD 로드

#ifndef ES_LINK_STATIC
int GLAD_EGL_VERSION_1_0 = 0;
int GLAD_EGL_VERSION_1_1 = 0;
int GLAD_EGL_VERSION_1_2 = 0;
int GLAD_EGL_VERSION_1_3 = 0;
int GLAD_EGL_VERSION_1_4 = 0;
int GLAD_EGL_VERSION_1_5 = 0;

PFNEGLBINDAPIPROC glad_eglBindAPI = NULL;
PFNEGLBINDTEXIMAGEPROC glad_eglBindTexImage = NULL;
PFNEGLCHOOSECONFIGPROC glad_eglChooseConfig = NULL;
PFNEGLCLIENTWAITSYNCPROC glad_eglClientWaitSync = NULL;
PFNEGLCOPYBUFFERSPROC glad_eglCopyBuffers = NULL;
PFNEGLCREATECONTEXTPROC glad_eglCreateContext = NULL;
PFNEGLCREATEIMAGEPROC glad_eglCreateImage = NULL;
PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC glad_eglCreatePbufferFromClientBuffer = NULL;
PFNEGLCREATEPBUFFERSURFACEPROC glad_eglCreatePbufferSurface = NULL;
PFNEGLCREATEPIXMAPSURFACEPROC glad_eglCreatePixmapSurface = NULL;
PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC glad_eglCreatePlatformPixmapSurface = NULL;
PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC glad_eglCreatePlatformPixmapSurfaceEXT = NULL;
PFNEGLCREATEPLATFORMWINDOWSURFACEPROC glad_eglCreatePlatformWindowSurface = NULL;
PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC glad_eglCreatePlatformWindowSurfaceEXT = NULL;
PFNEGLCREATESYNCPROC glad_eglCreateSync = NULL;
PFNEGLCREATEWINDOWSURFACEPROC glad_eglCreateWindowSurface = NULL;
PFNEGLDESTROYCONTEXTPROC glad_eglDestroyContext = NULL;
PFNEGLDESTROYIMAGEPROC glad_eglDestroyImage = NULL;
PFNEGLDESTROYSURFACEPROC glad_eglDestroySurface = NULL;
PFNEGLDESTROYSYNCPROC glad_eglDestroySync = NULL;
PFNEGLGETCONFIGATTRIBPROC glad_eglGetConfigAttrib = NULL;
PFNEGLGETCONFIGSPROC glad_eglGetConfigs = NULL;
PFNEGLGETCURRENTCONTEXTPROC glad_eglGetCurrentContext = NULL;
PFNEGLGETCURRENTDISPLAYPROC glad_eglGetCurrentDisplay = NULL;
PFNEGLGETCURRENTSURFACEPROC glad_eglGetCurrentSurface = NULL;
PFNEGLGETDISPLAYPROC glad_eglGetDisplay = NULL;
PFNEGLGETERRORPROC glad_eglGetError = NULL;
PFNEGLGETPLATFORMDISPLAYPROC glad_eglGetPlatformDisplay = NULL;
PFNEGLGETPLATFORMDISPLAYEXTPROC glad_eglGetPlatformDisplayEXT = NULL;
PFNEGLGETPROCADDRESSPROC glad_eglGetProcAddress = NULL;
PFNEGLGETSYNCATTRIBPROC glad_eglGetSyncAttrib = NULL;
PFNEGLINITIALIZEPROC glad_eglInitialize = NULL;
PFNEGLMAKECURRENTPROC glad_eglMakeCurrent = NULL;
PFNEGLQUERYAPIPROC glad_eglQueryAPI = NULL;
PFNEGLQUERYCONTEXTPROC glad_eglQueryContext = NULL;
PFNEGLQUERYSTRINGPROC glad_eglQueryString = NULL;
PFNEGLQUERYSURFACEPROC glad_eglQuerySurface = NULL;
PFNEGLRELEASETEXIMAGEPROC glad_eglReleaseTexImage = NULL;
PFNEGLRELEASETHREADPROC glad_eglReleaseThread = NULL;
PFNEGLSURFACEATTRIBPROC glad_eglSurfaceAttrib = NULL;
PFNEGLSWAPBUFFERSPROC glad_eglSwapBuffers = NULL;
PFNEGLSWAPINTERVALPROC glad_eglSwapInterval = NULL;
PFNEGLTERMINATEPROC glad_eglTerminate = NULL;
PFNEGLWAITCLIENTPROC glad_eglWaitClient = NULL;
PFNEGLWAITGLPROC glad_eglWaitGL = NULL;
PFNEGLWAITNATIVEPROC glad_eglWaitNative = NULL;
PFNEGLWAITSYNCPROC glad_eglWaitSync = NULL;

#ifdef _MSC_VER
#pragma warning(disable: 4191)
#endif

static void glad_egl_load_EGL_VERSION_1_0(GLADuserptrloadfunc load, void* userptr) {
	if (!GLAD_EGL_VERSION_1_0) return;
	glad_eglChooseConfig = (PFNEGLCHOOSECONFIGPROC)load(userptr, "eglChooseConfig");
	glad_eglCopyBuffers = (PFNEGLCOPYBUFFERSPROC)load(userptr, "eglCopyBuffers");
	glad_eglCreateContext = (PFNEGLCREATECONTEXTPROC)load(userptr, "eglCreateContext");
	glad_eglCreatePbufferSurface = (PFNEGLCREATEPBUFFERSURFACEPROC)load(userptr, "eglCreatePbufferSurface");
	glad_eglCreatePixmapSurface = (PFNEGLCREATEPIXMAPSURFACEPROC)load(userptr, "eglCreatePixmapSurface");
	glad_eglCreateWindowSurface = (PFNEGLCREATEWINDOWSURFACEPROC)load(userptr, "eglCreateWindowSurface");
	glad_eglDestroyContext = (PFNEGLDESTROYCONTEXTPROC)load(userptr, "eglDestroyContext");
	glad_eglDestroySurface = (PFNEGLDESTROYSURFACEPROC)load(userptr, "eglDestroySurface");
	glad_eglGetConfigAttrib = (PFNEGLGETCONFIGATTRIBPROC)load(userptr, "eglGetConfigAttrib");
	glad_eglGetConfigs = (PFNEGLGETCONFIGSPROC)load(userptr, "eglGetConfigs");
	glad_eglGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC)load(userptr, "eglGetCurrentDisplay");
	glad_eglGetCurrentSurface = (PFNEGLGETCURRENTSURFACEPROC)load(userptr, "eglGetCurrentSurface");
	glad_eglGetDisplay = (PFNEGLGETDISPLAYPROC)load(userptr, "eglGetDisplay");
	glad_eglGetError = (PFNEGLGETERRORPROC)load(userptr, "eglGetError");
	glad_eglGetProcAddress = (PFNEGLGETPROCADDRESSPROC)load(userptr, "eglGetProcAddress");
	glad_eglInitialize = (PFNEGLINITIALIZEPROC)load(userptr, "eglInitialize");
	glad_eglMakeCurrent = (PFNEGLMAKECURRENTPROC)load(userptr, "eglMakeCurrent");
	glad_eglQueryContext = (PFNEGLQUERYCONTEXTPROC)load(userptr, "eglQueryContext");
	glad_eglQueryString = (PFNEGLQUERYSTRINGPROC)load(userptr, "eglQueryString");
	glad_eglQuerySurface = (PFNEGLQUERYSURFACEPROC)load(userptr, "eglQuerySurface");
	glad_eglSwapBuffers = (PFNEGLSWAPBUFFERSPROC)load(userptr, "eglSwapBuffers");
	glad_eglTerminate = (PFNEGLTERMINATEPROC)load(userptr, "eglTerminate");
	glad_eglWaitGL = (PFNEGLWAITGLPROC)load(userptr, "eglWaitGL");
	glad_eglWaitNative = (PFNEGLWAITNATIVEPROC)load(userptr, "eglWaitNative");
}
static void glad_egl_load_EGL_VERSION_1_1(GLADuserptrloadfunc load, void* userptr) {
	if (!GLAD_EGL_VERSION_1_1) return;
	glad_eglBindTexImage = (PFNEGLBINDTEXIMAGEPROC)load(userptr, "eglBindTexImage");
	glad_eglReleaseTexImage = (PFNEGLRELEASETEXIMAGEPROC)load(userptr, "eglReleaseTexImage");
	glad_eglSurfaceAttrib = (PFNEGLSURFACEATTRIBPROC)load(userptr, "eglSurfaceAttrib");
	glad_eglSwapInterval = (PFNEGLSWAPINTERVALPROC)load(userptr, "eglSwapInterval");
}
static void glad_egl_load_EGL_VERSION_1_2(GLADuserptrloadfunc load, void* userptr) {
	if (!GLAD_EGL_VERSION_1_2) return;
	glad_eglBindAPI = (PFNEGLBINDAPIPROC)load(userptr, "eglBindAPI");
	glad_eglCreatePbufferFromClientBuffer = (PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC)load(userptr, "eglCreatePbufferFromClientBuffer");
	glad_eglQueryAPI = (PFNEGLQUERYAPIPROC)load(userptr, "eglQueryAPI");
	glad_eglReleaseThread = (PFNEGLRELEASETHREADPROC)load(userptr, "eglReleaseThread");
	glad_eglWaitClient = (PFNEGLWAITCLIENTPROC)load(userptr, "eglWaitClient");
}
static void glad_egl_load_EGL_VERSION_1_4(GLADuserptrloadfunc load, void* userptr) {
	if (!GLAD_EGL_VERSION_1_4) return;
	glad_eglGetCurrentContext = (PFNEGLGETCURRENTCONTEXTPROC)load(userptr, "eglGetCurrentContext");
}
static void glad_egl_load_EGL_VERSION_1_5(GLADuserptrloadfunc load, void* userptr) {
	if (!GLAD_EGL_VERSION_1_5) return;
	glad_eglClientWaitSync = (PFNEGLCLIENTWAITSYNCPROC)load(userptr, "eglClientWaitSync");
	glad_eglCreateImage = (PFNEGLCREATEIMAGEPROC)load(userptr, "eglCreateImage");
	glad_eglCreatePlatformPixmapSurface = (PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC)load(userptr, "eglCreatePlatformPixmapSurface");
	glad_eglCreatePlatformWindowSurface = (PFNEGLCREATEPLATFORMWINDOWSURFACEPROC)load(userptr, "eglCreatePlatformWindowSurface");
	glad_eglCreateSync = (PFNEGLCREATESYNCPROC)load(userptr, "eglCreateSync");
	glad_eglDestroyImage = (PFNEGLDESTROYIMAGEPROC)load(userptr, "eglDestroyImage");
	glad_eglDestroySync = (PFNEGLDESTROYSYNCPROC)load(userptr, "eglDestroySync");
	glad_eglGetPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYPROC)load(userptr, "eglGetPlatformDisplay");
	glad_eglGetSyncAttrib = (PFNEGLGETSYNCATTRIBPROC)load(userptr, "eglGetSyncAttrib");
	glad_eglWaitSync = (PFNEGLWAITSYNCPROC)load(userptr, "eglWaitSync");
}
static void glad_egl_load_EGL_EXT_platform_base(GLADuserptrloadfunc load, void* userptr) {
	if (!GLAD_EGL_EXT_platform_base) return;
	glad_eglCreatePlatformPixmapSurfaceEXT = (PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC)load(userptr, "eglCreatePlatformPixmapSurfaceEXT");
	glad_eglCreatePlatformWindowSurfaceEXT = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)load(userptr, "eglCreatePlatformWindowSurfaceEXT");
	glad_eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)load(userptr, "eglGetPlatformDisplayEXT");
}

static GLADapiproc glad_egl_get_proc_from_userptr(void *userptr, const char *name) {
	return (GLAD_GNUC_EXTENSION(GLADapiproc(*)(const char *name)) userptr)(name);
}

bool gladLoadEGL_Basic(GLADuserptrloadfunc load, void* userptr)
{
	eglGetDisplay = (PFNEGLGETDISPLAYPROC)load(userptr, "eglGetDisplay");
	eglGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC)load(userptr, "eglGetCurrentDisplay");
	eglQueryString = (PFNEGLQUERYSTRINGPROC)load(userptr, "eglQueryString");
	eglGetError = (PFNEGLGETERRORPROC)load(userptr, "eglGetError");
	if (eglGetDisplay == NULL || eglGetCurrentDisplay == NULL || eglQueryString == NULL || eglGetError == NULL)
		return false;
	GLAD_EGL_VERSION_1_0 = 1;
	glad_egl_load_EGL_VERSION_1_0(load, userptr);
	return true;
}

bool gladLoadEGL_Extension(EGLint major, EGLint minor, GLADuserptrloadfunc load, void* userptr)
{
	GLAD_EGL_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
	GLAD_EGL_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
	GLAD_EGL_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;
	GLAD_EGL_VERSION_1_4 = (major == 1 && minor >= 4) || major > 1;
	GLAD_EGL_VERSION_1_5 = (major == 1 && minor >= 5) || major > 1;
	int version = GLAD_MAKE_VERSION(major, minor);
	if (!version) return false;
	glad_egl_load_EGL_VERSION_1_1(load, userptr);
	glad_egl_load_EGL_VERSION_1_2(load, userptr);
	glad_egl_load_EGL_VERSION_1_4(load, userptr);
	glad_egl_load_EGL_VERSION_1_5(load, userptr);
	glad_egl_load_EGL_EXT_platform_base(load, userptr);
	return true;
}

//
static QnModule* egl_module = NULL;

//
static GLADapiproc egl_load_proc(const char* name)
{
	return (GLADapiproc)qn_mod_func(egl_module, name);
}
#ifdef _MSC_VER
#pragma warning(default: 4191)
#endif

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
	if (gladLoadEGL_Basic(glad_egl_get_proc_from_userptr, GLAD_GNUC_EXTENSION(void*)egl_load_proc) == false)
	{
		qn_mod_unload(egl_module);
		return false;
	}
	return true;
}

//
static void glad_load_extensions(EGLint major, EGLint minor)
{
	gladLoadEGL_Extension(major, minor, glad_egl_get_proc_from_userptr, GLAD_GNUC_EXTENSION(void*)egl_load_proc);
}

//
static bool egl_init_gl_api(void)
{
	static int version = 0;
	if (version != 0)
		return true;
	version = gladLoadGLES2(eglGetProcAddress);
	return version != 0;
}
#endif // !ES_LINK_STATIC


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
	config->ver_major = wanted_config->ver_major;
	config->ver_minor = wanted_config->ver_minor;
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

// EGL 설정 얻기
static bool egl_choose_config(EGLDisplay display, const QglConfig* wanted_config, QglConfig* found_config)
{
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[i++] = e; attrs[i++] = v; }QN_STMT_END
	EGLint attrs[32], i = 0;
	ATTR_ADD(EGL_RENDERABLE_TYPE, wanted_config->ver_major == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT);
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

		const QglConfig* found = qgl_detect_config(wanted_config, &qgl_configs);
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
#ifndef ES_LINK_STATIC
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
#ifndef ES_LINK_STATIC
	// eglInitialize에서 인수로 버전을 알 수 있다.
	glad_load_extensions(major, minor);
#endif

	return display;
}

// EGL 컨텍스트 만들기
EGLContext egl_create_context(EGLDisplay display, const QglConfig* wanted_config, QglConfig* final_config, QgFlag flags)
{
	qn_verify(display != EGL_NO_DISPLAY);
	qn_verify(wanted_config != NULL);
	qn_verify(final_config != NULL);

	// 컨피그 얻고
	if (egl_choose_config(display, wanted_config, final_config) == false)
		goto pos_error_exit;

	// 속성 설정
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[attr_count++] = e; attrs[attr_count++] = v; }QN_STMT_END
	EGLenum profile = EGL_OPENGL_ES_API;
	EGLint attrs[24], attr_count = 0;
	if (GLAD_EGL_KHR_create_context)
	{
		EGLint emask = 0, eflags = 0;
		ATTR_ADD(EGL_CONTEXT_MAJOR_VERSION, wanted_config->ver_major);
		ATTR_ADD(EGL_CONTEXT_MINOR_VERSION, wanted_config->ver_minor);
#if !defined _QN_EMSCRIPTEN_ && !defined _QN_MOBILE_
		const char* driver_profile = qn_get_prop(QG_PROP_DRIVER_PROFILE);
		if (driver_profile != NULL)
		{
			if (strstr(driver_profile, "core") != NULL)
			{
				profile = EGL_OPENGL_API;
				emask |= EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
			}
			if (strstr(driver_profile, "forward") != NULL)
			{
				profile = EGL_OPENGL_API;
				eflags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;
				ATTR_ADD(EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE);
			}
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
	{
		ATTR_ADD(EGL_CONTEXT_CLIENT_VERSION, wanted_config->ver_major);
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
	EGLContext context = eglCreateContext(display, final_config->handle, NULL, attrs);
	if (context == EGL_NO_CONTEXT)
	{
#ifdef _QN_EMSCRIPTEN_
		attrs[1] = final_config->ver_major == 2 ? 3 : 2;
		context = eglCreateContext(display, final_config->handle, NULL, attrs);
#else
		if (final_config->ver_minor == 0)
		{
			attrs[1] = final_config->ver_major == 2 ? 3 : 2;
			context = eglCreateContext(display, final_config->handle, NULL, attrs);
		}
		else
		{
			for (EGLint i = final_config->ver_minor - 1; i >= 0; i--)
			{
				attrs[3] = i;
				context = eglCreateContext(display, final_config->handle, NULL, attrs);
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
	if (QN_TMASK(flags, QGFLAG_SRGB) && GLAD_EGL_KHR_gl_colorspace)
		ATTR_ADD(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB);
	if (QN_TMASK(flags, QGFLAG_TRANSPARENT) && GLAD_EGL_EXT_present_opaque)
		ATTR_ADD(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR);
	ATTR_ADD(EGL_NONE, EGL_NONE);
#undef ATTR_ADD

	EGLNativeWindowType native_window = stub_system_get_window();
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
bool egl_make_current(EGLDisplay display, EGLSurface surface, EGLContext context)
{
	qn_verify(display != EGL_NO_DISPLAY);
	qn_verify(surface != EGL_NO_SURFACE);
	qn_verify(context != EGL_NO_CONTEXT);

	if (eglMakeCurrent(display, surface, surface, context) == false
#ifndef ES_LINK_STATIC
		|| egl_init_gl_api() == false
#endif // ES_LINK_STATIC
		)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to make current: %s", egl_error_string(eglGetError()));
		return false;
	}

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

#endif // USE_GL
