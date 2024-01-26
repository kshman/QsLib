//
// qgrdh_es.h - OPENGL ES 렌더 디바이스
// 2024-1-26 by kim
//

#pragma once

#ifndef __QS_QN__
#error include "qs_qn.h" first
#endif

#if defined _QN_EMSCRIPTEN_ || defined _QN_ANDROID_
#define ES_STATIC_LINK
#endif

#ifdef ES_STATIC_LINK
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#else
#ifdef _MSC_VER
#pragma warning(disable: 4191)
#endif
#include "glad/egl.h"
#include "glad/gles2.h"
#ifdef _MSC_VER
#pragma warning(default: 4191)
#endif
#endif
#include "qgrdh_qgl.h"

static_assert(sizeof(int) == sizeof(EGLint), "EGLint size is not equal to int");
static_assert(sizeof(void*) == sizeof(EGLConfig), "EGLConfig size is not equal to void*");

typedef struct ESRDH		EsRdh;

#define ES_RDH				((EsRdh*)qg_instance_rdh)

// ES 렌더 디바이스
struct ESRDH
{
	QglRdh				base;

	NativeWindowType	native_window;
	NativeDisplayType	native_display;

	EGLContext			context;
	EGLDisplay			display;
	EGLSurface			surface;
	EGLint				version;
};

