#pragma once

#ifdef __EMSCRIPTEN__
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include "gles2.h"
#endif
#include "qggl_base.h"

typedef struct EsRdh					EsRdh;

#define ES_RDH_INSTANCE					((EsRdh*)qg_rdh_instance)


//////////////////////////////////////////////////////////////////////////
// 디바이스

// ES3 렌더 디바이스
struct EsRdh
{
	QglRdh				base;
};
