#pragma once

#ifndef QSAPI
#	if defined(_LIB) || defined(_STATIC)
#		define QSAPI					extern
#	else
#		if defined(_WIN32) || defined(_WIN64)
#			define QSAPI				__declspec(dllexport)
#		else
#			define QSAPI				__attribute__((__visibility__("default")))
#		endif
#	endif
#endif

#ifdef __EMSCRIPTEN__
#ifndef USE_NO_LOCK
#define USE_NO_LOCK
#endif
#ifndef USE_NO_THREAD
#define USE_NO_THREAD
#endif
#endif

// 디버그 출력 버퍼 길이
#ifndef MAX_DEBUG_LENGTH
#define MAX_DEBUG_LENGTH	1024
#endif
static_assert(MAX_DEBUG_LENGTH >= 256 && MAX_DEBUG_LENGTH <= 4096, "256 <= MAX_DEBUG_LENGTH <= 4096");

// 컨트롤러 데드존
#ifndef CTRL_DEAD_ZONE
#define CTRL_DEAD_ZONE		(int)(0.24f*((float)INT16_MAX))
#endif

// Poll 당 메시지 처리 개수
#ifndef MAX_POLL_LENGTH
#define MAX_POLL_LENGTH		3
#endif
