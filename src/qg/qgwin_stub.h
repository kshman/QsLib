//
// qgwin_stub.h - WINDOWS 스터브
// 2023-12-23 by kim
//

#pragma once

#include "qg_stub.h"
#include <windowsx.h>
#include <Xinput.h>
#include <shellscalingapi.h>

static_assert(sizeof(RECT) == sizeof(QmRect), "RECT size not equal to QmRect");

#ifndef XINPUT_DEAD_ZONE
/// @brief 컨트롤러 데드존
#define XINPUT_DEAD_ZONE	(int)(0.24f*((float)INT16_MAX))
#endif


// DLL
#define DEF_WIN_FUNC(ret,name,args)		typedef ret(WINAPI* QN_CONCAT(PFNWin32, name)) args;
#define DEF_WIN_XIFUNC(ret,name,args)	typedef ret(WINAPI* QN_CONCAT(PFNWin32, name)) args;
#include "qgwin_func.h"
#define DEF_WIN_FUNC(ret,name,args)		extern QN_CONCAT(PFNWin32, name) QN_CONCAT(Win32, name);
#define DEF_WIN_XIFUNC(ret,name,args)	extern QN_CONCAT(PFNWin32, name) QN_CONCAT(Win32, name);
#include "qgwin_func.h"

/// @brief 윈도우 스터브
typedef struct WindowsStub WindowsStub;
struct WindowsStub
{
	StubBase			base;

	HWND				hwnd;
	HINSTANCE			instance;

	wchar*				class_name;
	wchar*				window_title;
	DWORD				window_style;

	STICKYKEYS			acs_sticky;
	TOGGLEKEYS			acs_toggle;
	FILTERKEYS			acs_filter;

	HIMC				himc;
	int					imcs;
	int					high_surrogate;

	int					deadzone_min;
	int					deadzone_max;

	HCURSOR				mouse_cursor;
	WPARAM				mouse_wparam;
	LPARAM				mouse_lparam;
	QimMask				mouse_pending;

	bool				class_registered;
	bool				clear_background;
};

/// @brief 윈도우 메시지 => 문자열
extern const char* windows_message_string(UINT mesg);
