/*
 * QsLib
 * Made by kim 2004-2024
 *
 * 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
 * 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
 */
/**
 * @file qs.h
 *
 * QS 내부에 딸린 모든 헤더를 포함합니다.
 */
#pragma once

//////////////////////////////////////////////////////////////////////////
// API
#ifndef QSAPI
#	if defined _LIB || defined _STATIC
#		define QSAPI					extern
#	else
#		if defined _WIN32 || defined _WIN64
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
#ifdef _MSC_VER
#pragma comment(lib, "qs")
#ifdef USE_SDL2
#ifdef _QN_64_
#pragma comment(lib, "SDL2x64")
#else
#pragma comment(lib, "SDL2")
#endif
#endif
#endif
