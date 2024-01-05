#pragma once

#include "qs_qn.h"
#include "qs_math.h"

#define malloc(x)			qn_alloc(x,byte)
#define free(x)				qn_free(x)
#define sscanf(a,b,c,d)		glad_version_sscanf(a,c,d)
#define sscanf_s(a,b,c,d)	glad_version_sscanf(a,c,d)

// GLAD_IMPL_UTIL_SSCANF(version, "%d.%d", &major, &minor);
QN_INLINE void glad_version_sscanf(const char* version, int* major, int* minor)
{
	const float f = (float)strtof(version, NULL);
	*major = (int)floorf(f);
	*minor = (int)(QM_FRACT(f) * 10.0f);
}
