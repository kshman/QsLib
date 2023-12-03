#pragma once

// Windows
#if defined(_WIN32)
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <excpt.h>
#include <sdkddkver.h>
#include <windows.h>
#include <mmsystem.h>
#if _MSC_VER
#include <intrin.h>
#include <crtdbg.h>
#endif
#endif

// GCC
#if __GNUC__
#define _GNU_SOURCE				1		// GNU 라이브러리
#define __STDC_WANT_LIB_EXT1__	1
#include <unistd.h>
#endif

// comon
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <math.h>

//
#if _MSC_VER
#pragma warning(disable:4100)		// 참조되지 않은 정식 매개 변수입니다.
#pragma warning(disable:4127)		// 조건식이 상수입니다.
#endif
#if __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

