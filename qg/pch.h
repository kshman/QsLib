#ifndef PCH_H
#define PCH_H

#if defined(_WIN32)
// 윈도우
#	define STRICT
#	define WIN32_LEAN_AND_MEAN
#	include <sdkddkver.h>
#	include <windows.h>
#	include <excpt.h>
#	include <mmsystem.h>
#	include <commctrl.h>
#	include <shellapi.h>
#endif

// GCC
#if __GNUC__
#	define _GNU_SOURCE					1		// GNU 라이브러리
#	define __STDC_WANT_LIB_EXT1__		1
#	include <unistd.h>
#endif

// QN
#include <qn.h>
#include <qnctn.h>
#include <qnmath.h>

// API
#ifdef QNAPI
#	undef QNAPI
#endif

#ifdef _STATIC
#	define QNAPI						extern 
#else
#	if defined(_WIN32)
#		define QNAPI					__declspec(dllexport)
#	else
#		define QNAPI					__attribute__((__visibility__("default")))
#	endif
#endif

// BUILD CONFIG
#if _QN_WINDOWS_
#	define _QG_USE_GL					1
#endif
#if _QN_UNIX_
#	define _QG_USE_GL					1
#endif

// ANDROID
#if _QN_ANDROID_
#	include <android/configuration.h>
#	include <android/looper.h>
#	include <android/native_activity.h>
#endif

// SDL
#if _MSC_VER
#include "SDL/SDL.h"
#include "SDL/SDL_system.h"
#include "SDL/SDL_opengles2.h"
#include "SDL/SDL_keycode.h"
#include "SDL/SDL_mouse.h"
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_system.h>
#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#endif

//
#if _MSC_VER
#pragma warning(disable:4100)		// 참조되지 않은 정식 매개 변수입니다.
#pragma warning(disable:4127)		// 조건식이 상수입니다.
#endif
#if __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#endif //PCH_H
