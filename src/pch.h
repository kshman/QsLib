//
// pch.h - 미리 컴파일한 헤더
// 2023-11-14 by kim
//

#pragma once
#define __PCH__

#ifdef _MSC_VER
#define no_init_all deprecated		// VS2022 알 수 없는 특성
#endif
#ifdef __GNUC__
#ifndef _GNU_SOURCE					// gnu extension 쓰기 위해서
#define _GNU_SOURCE
#endif
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <salieri.h>

// Windows
#ifdef _WIN32
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <crtdbg.h>
#include <excpt.h>
#include <sdkddkver.h>
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <shellapi.h>
#include <lmcons.h>
#include <intrin.h>
#endif

// ANDROID
#ifdef __android__
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#endif

// EMSCRIPTEN
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

//
#ifdef _MSC_VER
#pragma warning(disable:4061)		// 열거형 '열거형' 스위치의 열거자 'identifier'는 사례 레이블에 의해 명시적으로 처리되지 않습니다.
#pragma warning(disable:4062)		// 열거형 '열거형' 스위치의 열거자 'identifier'가 처리되지 않음
#pragma warning(disable:4100)		// 'identifier': 함수 매개 변수가 참조되지 않았습니다.
#pragma warning(disable:4505)		// 'function' 함수가 인라인되지 않았습니다.
#pragma warning(disable:4710)		// 'function': 함수가 인라인되지 않음
#pragma warning(disable:4711)		// 인라인 확장을 위해 'function' 함수가 선택되었습니다.
#pragma warning(disable:4820)		// 'bytes'바이트 채움 문자가 construct 'member_name' 뒤에 추가되었습니다. (패딩)
#pragma warning(disable:5045)		// 컴파일러는 /Qspectre 스위치가 지정된 경우 메모리 로드를 위해 스펙터 완화를 삽입합니다.
#endif
#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

//
#include "qs_conf.h"
#include "qs_qn.h"
#include "qs_math.h"
