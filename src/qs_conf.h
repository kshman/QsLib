#pragma once

#if !defined __STDC_VERSION__ || __STDC_VERSION__ < 201112L
#error unknown c version! need at least c11
#endif

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

// 디버그 출력 버퍼 길이
#ifndef MAX_DEBUG_LENGTH
#define MAX_DEBUG_LENGTH	1024
#endif
static_assert(MAX_DEBUG_LENGTH >= 256 && MAX_DEBUG_LENGTH <= 4096, "256 <= MAX_DEBUG_LENGTH <= 4096");

// 스레드 별 TLS 갯수
#ifndef MAX_TLS
#define MAX_TLS	64
#endif

// 컨트롤러 데드존
#ifndef CTRL_DEAD_ZONE
#define CTRL_DEAD_ZONE		(int)(0.24f*((float)INT16_MAX))
#endif

// Poll 당 메시지 처리 개수
#ifndef MAX_POLL_LENGTH
#define MAX_POLL_LENGTH		3
#endif

// 컴파일러 전용
#ifndef PRAGMA
#define PRAGMA(x)			_Pragma(#x)
#endif
