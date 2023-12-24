#pragma once

#ifndef QSAPI
#	if defined(_LIB) || defined(_STATIC)
#		define QSAPI					extern
#	else
#		if defined(_WIN32) || defined(_WIN64)
#			define QSAPI				__declspec(dllimport)
#		else
#			define QSAPI				__attribute__((__visibility__("default")))
#		endif
#	endif
#endif

#ifdef __EMSCRIPTEN__
#ifndef NO_LOCK
#define NO_LOCK
#endif
#ifndef NO_THREAD
#define NO_THREAD
#endif
#endif
