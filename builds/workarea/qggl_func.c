#include "pch.h"
#include "qs_qn.h"
#include "qg_glfunc.h"

#ifdef _MSC_VER
#pragma warning(disable: 4152)	//비표준 확장입니다. 식에서 함수/데이터 포인터 변환이 있습니다.
#endif

GlFunc gl_func;

bool gl_init_func(void)
{
#ifdef _QN_WINDOWS_
	const char* name;
#define DEF_GL_FUNC(ret, func, params)\
	gl_func.func = SDL_GL_GetProcAddress(#func);\
	if (!gl_func.func)\
	{\
		name = #func;\
		goto pos_gl_func_error;\
	}
#include "qg_glfunc_list.h"
#undef DEF_GL_FUNC
#endif
	return true;

#ifdef _QN_WINDOWS_
pos_gl_func_error:
	qn_debug_outputf(true, "GL", "invalid GL function: '%s'", name);
	return false;
#endif
}
