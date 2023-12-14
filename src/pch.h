//
// pch.h - 미리 컴파일한 헤더
// 2023-11-14 by kim
//

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4820)
#endif

#ifndef USE_SDL2
#define USE_SDL2	1
#endif

#ifndef USE_ES2
#define USE_ES2		1
#endif

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
#if defined __unix__
#include <sys/time.h>
#endif

// Windows
#if defined _WIN32
#	define STRICT
#	define WIN32_LEAN_AND_MEAN
#	include <excpt.h>
#	include <sdkddkver.h>
#	include <windows.h>
#	include <mmsystem.h>
#	include <commctrl.h>
#	include <shellapi.h>
#if defined _MSC_VER
#	include <intrin.h>
#	include <crtdbg.h>
#endif
#endif

// ANDROID
#if defined __android__
#	include <android/configuration.h>
#	include <android/looper.h>
#	include <android/native_activity.h>
#endif

// SDL
#ifdef USE_SDL2
#	include <SDL2/SDL.h>
#if defined USE_ES2
#	include <SDL2/SDL_opengles2.h>
#endif
#endif

//
#include "qs_conf.h"

//
#if defined _MSC_VER
#pragma warning(pop)
#pragma warning(disable:4100)		// 참조되지 않은 정식 매개 변수입니다.
#pragma warning(disable:4127)		// 조건식이 상수입니다.
#pragma warning(disable:5045)		// 컴파일러는 /Qspectre 스위치가 지정된 경우 메모리 로드를 위해 스펙터 완화를 삽입합니다.
#endif
#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
