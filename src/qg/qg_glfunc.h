#pragma once

#if _QN_WINDOWS_
typedef struct GlFunc					GlFunc;
struct GlFunc
{
#define DEF_GL_FUNC(ret,func,params)	ret (APIENTRY *func) params;  // NOLINT
#include "qg_glfunc_list.h"
#undef DEF_GL_FUNC
};
extern GlFunc gl_func;
#define GL_FUNC(f)						gl_func.##f
#else
#define GL_FUNC(f)						f
#endif

extern bool gl_init_func(void);
