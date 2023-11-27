#pragma once

#include "qn.h"

#define HAVE_VSNPRINTF
#define vsnprintf		qn_vsnprintf
#define snprintf		qn_snprintf

#define malloc(x)		((void*)qn_alloc(x,uint8_t))
#define calloc(x,y)		((void*)qn_alloc_zero((x)*(y),uint8_t))
#define free(x)			qn_free(x)

#if _MSC_VER
#pragma warning(disable:4131)
#pragma warning(disable:4244)
#pragma warning(disable:4996)
#endif
