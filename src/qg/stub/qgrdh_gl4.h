//
// qgrdh_gl4.h - OPENGL 렌더 디바이스
// 2024-1-26 by kim
//

#pragma once

#ifndef __QS_QN__
#error include "qs_qn.h" first
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4191)
#endif
#include "glad/gl.h"
#ifdef _MSC_VER
#pragma warning(default: 4191)
#endif
#include "qgrdh_qgl.h"

//static_assert(sizeof(int) == sizeof(EGLint), "EGLint size is not equal to int");
//static_assert(sizeof(void*) == sizeof(EGLConfig), "EGLConfig size is not equal to void*");

typedef struct GLRDH		GlRdh;

#define GL_RDH				((GlRdh*)qg_instance_rdh)	

// ES 렌더 디바이스
struct GLRDH
{
	QglRdh				base;

#ifdef _QN_WINDOWS_
	HGLRC				context;
#endif

	struct
	{
		bool			ARB_multisample;
		bool			ARB_create_context;
		bool			ARB_create_context_robustness;
		bool			EXT_swap_control;
		bool			EXT_colorspace;
#ifdef _QN_WINDOWS_
		bool			ARB_pixel_format;
#endif
	}					ext;
};
