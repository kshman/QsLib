#pragma once

// Windows
#if defined(_WIN32)
#	define STRICT
#	define WIN32_LEAN_AND_MEAN
#	include <excpt.h>
#	include <sdkddkver.h>
#	include <windows.h>
#	include <mmsystem.h>
#	include <commctrl.h>
#	include <shellapi.h>
#if _MSC_VER
#	include <intrin.h>
#	include <crtdbg.h>
#endif
#endif

// comon
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <wchar.h>
#include <wctype.h>
#include <math.h>
#if __unix__
#include <sys/time.h>
#endif

// ANDROID
#if __android__
#	include <android/configuration.h>
#	include <android/looper.h>
#	include <android/native_activity.h>
#endif

// SDL
#if _MSC_VER
#	include "qg/SDL/SDL.h"
#else
#	include <SDL2/SDL.h>
#endif

//
#include "qs_conf.h"

//
#if _MSC_VER
#pragma warning(disable:4100)		// 참조되지 않은 정식 매개 변수입니다.
#pragma warning(disable:4127)		// 조건식이 상수입니다.
#endif
#if __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
