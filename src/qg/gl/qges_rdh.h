#pragma once

#if defined __EMSCRIPTEN__ || defined _QN_ANDROID_
#define STATIC_ES_LIBRARY
#endif

#ifdef STATIC_ES_LIBRARY
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4191)
#endif
#include "glad_supp.h"
#include "egl.h"
#include "gles2.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif
#include "qggl_base.h"

typedef struct EsRdh					EsRdh;

#define ES_RDH_INSTANCE					((EsRdh*)qg_rdh_instance)

// ES 컨피그
typedef struct EsConfig
{
	void*				handle;
	EGLint				red, green, blue, alpha;
	EGLint				depth, stencil, samples;
	int					version;
} EsConfig;

// ES 렌더 디바이스
struct EsRdh
{
	QglRdh				base;

#if !defined USE_SDL2
	NativeWindowType	native_window;
	NativeDisplayType	native_display;
	EGLint				egl_major;
	EGLint				egl_minor;
	EGLint				version_major;

	EGLConfig			config;
	EGLContext			context;
	EGLDisplay			display;
	EGLSurface			surface;
#endif
};
