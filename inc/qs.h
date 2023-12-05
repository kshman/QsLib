#pragma once

//////////////////////////////////////////////////////////////////////////
// api
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


//////////////////////////////////////////////////////////////////////////
// version
#define _QS_VERSION_					202312L


//////////////////////////////////////////////////////////////////////////
// libraries

// include
#include <qs_qn.h>
#include <qs_ctn.h>
#include <qs_math.h>
#include <qs_kmc.h>
#include <qs_qg.h>

// MSVC library
#if _MSC_VER
#pragma comment(lib, "qs")
#if _QN_64_
#pragma comment(lib, "SDL2x64")
#else
#pragma comment(lib, "SDL2")
#endif
#endif
