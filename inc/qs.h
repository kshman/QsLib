//
// QsLib
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: BSD-2-Clause
//
#pragma once

#define __QS__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)		// 비표준 확장이 사용됨: 구조체/공용 구조체의 이름이 없습니다.
#pragma warning(disable:4820)		// 'bytes'바이트 채움 문자가 construct 'member_name' 뒤에 추가되었습니다. (패딩)
#endif

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
#ifdef __cplusplus
extern "C" {
#endif
#include <salieri.h>
#include <qs_qn.h>
#include <qs_ctn.h>
#include <qs_math.h>
#include <qs_kmc.h>
#include <qs_qg.h>
#ifdef __cplusplus
}
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#pragma comment(lib, "QsLib")
#endif


//////////////////////////////////////////////////////////////////////////
// appendix

// create로 시작하는 함수는 참조할 때 qn_load, 제거할 때 qn_unload를 사용할 수 있다
// open으로 시작하는 함수는 제거할 때 close를 사용한다
// new로 시작하는 함수는 제거할 때 delete를 사용한다
//
// 추가적으로 load로 시작하는 함수는 create와 동일하다
// 예외적인 기능이 있다면 별도로 추가한다
