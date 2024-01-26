﻿#pragma once

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
