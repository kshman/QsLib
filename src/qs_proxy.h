#pragma once

#include "qs_qn.h"
#include "qs_math.h"
#ifdef _QN_UNIX_
#include <unistd.h>
#endif // _QN_UNIX_

#define malloc(x)		((void*)qn_alloc(x,byte))
#define calloc(x,y)		((void*)qn_alloc_zero((x)*(y),byte))
#define free(x)			qn_free(x)

#ifndef HAVE_VSNPRINTF
#define HAVE_VSNPRINTF	1
#endif // HAVE_VSNPRINTF
#define vsnprintf		qn_vsnprintf
#define snprintf		qn_snprintf

#define wcstombs(x,y,z)	qn_wcstombs(x, z, y, 0)

#ifdef GLAD_PLATFORM_H_
#define sscanf(a,b,c,d)		glad_version_sscanf(a,c,d)
#define sscanf_s(a,b,c,d)	glad_version_sscanf(a,c,d)

// GLAD_IMPL_UTIL_SSCANF(version, "%d.%d", &major, &minor);
QN_INLINE void glad_version_sscanf(const char* version, int* major, int* minor)
{
	const float f = qn_strtof(version);
	*major = (int)QM_FLOORF(f);
	*minor = (int)(QM_FRACT(f) * 10.0f);
}
#endif // GLAD_PLATFORM_H_

#ifdef _MSC_VER
#pragma warning(disable:4127)
#pragma warning(disable:4242)
#pragma warning(disable:4244)
#pragma warning(disable:4996)
#endif // _MSC_VER
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#endif // __GNUC__
