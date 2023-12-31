﻿#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4191)
#endif

#ifndef __QS_QN__
#error include "qs_qn.h" first
#endif

#if defined _QN_EMSCRIPTEN_ || defined _QN_ANDROID_
#define STATIC_ES_LIBRARY
#endif

#ifdef STATIC_ES_LIBRARY
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#elif defined USE_SDL2
#include <SDL2/SDL.h>
#include "gl/glad_supp.h"
#include "gl/gles2.h"
#else
#include "gl/glad_supp.h"
#include "gl/egl.h"
#include "gl/gles2.h"
#endif
#include "qgrdh_glbase.h"

typedef struct EsRdh					EsRdh;

#define ES_RDH_INSTANCE					((EsRdh*)qg_instance_rdh)

#ifndef USE_SDL2
// ES 컨피그
typedef struct EsConfig
{
	void*				handle;
	EGLint				red, green, blue, alpha;
	EGLint				depth, stencil, samples;
	int					version;
} EsConfig;
#endif

// ES 렌더 디바이스
struct EsRdh
{
	QglRdh				base;

#ifdef USE_SDL2
	SDL_Window*			window;
	SDL_GLContext*		context;
	SDL_Renderer*		renderer;
#else
	NativeWindowType	native_window;
	NativeDisplayType	native_display;
	EGLint				egl_major;
	EGLint				egl_minor;
	EGLint				version_major;

	EGLContext			context;
	EGLDisplay			display;
	EGLSurface			surface;

	EsConfig			config;
#endif
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif
