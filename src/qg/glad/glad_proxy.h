#pragma once

#include "qs_qn.h"

#define malloc(x)		((void*)qn_alloc(x,byte))
#define free(x)			qn_free(x)

QN_INLINE void glad_scan_version(const char* version, int* major, int* minor)
{
	const float f = strtof(version, NULL);
	*major = (int)floorf(f);
	*minor = (int)(f - floorf(f));
}

#define sscanf_s(v,s,a,i)	glad_scan_version(v,a,i)
#define sscanf(v,s,a,i)		glad_scan_version(v,a,i)

#ifdef _MSC_VER
#pragma warning(disable:4191)
#endif

// 도움
// GL
//	https://registry.khronos.org/OpenGL-Refpages/es2.0/
//	https://registry.khronos.org/OpenGL-Refpages/es3.0/
// GLAD
//	https://stackoverflow.com/questions/76638441/how-to-init-glad-without-the-glfw-loader-using-windows-headers
//	https://github.com/Dav1dde/glad/blob/glad2/example/c/egl_gles2_glfw_emscripten.c
//	https://github.com/Dav1dde/glad/blob/glad2/example/c/gles2_glfw_emscripten.c
