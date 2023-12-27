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

// DLL
#define DEF_WIN_FUNC(ret,name,args)		typedef ret(WINAPI* QN_CONCAT(PFNWin32, name)) args;
#define DEF_WIN_XIFUNC(ret,name,args)	typedef ret(WINAPI* QN_CONCAT(PFNWin32, name)) args;
#include "qgwin_func.h"
#define DEF_WIN_FUNC(ret,name,args)		extern QN_CONCAT(PFNWin32, name) QN_CONCAT(Win32, name);
#define DEF_WIN_XIFUNC(ret,name,args)	extern QN_CONCAT(PFNWin32, name) QN_CONCAT(Win32, name);
#include "qgwin_func.h"

// 윈도우 모니터 > QgUdevMonitor
typedef struct WindowsMonitor
{
	QgUdevMonitor		base;

	HMONITOR			handle;
	wchar				adapter[32];
	wchar				display[32];
} WindowsMonitor;

// 마우스 이벤트 소스 (https://learn.microsoft.com/ko-kr/windows/win32/tablet/system-events-and-mouse-messages)
#define MI_WP_SIGNATURE		0xFF515700
#define SIGNATURE_MASK		0xFFFFFF00
#define IsPenEvent(dw)		(((dw) & SIGNATURE_MASK) == MI_WP_SIGNATURE)

// 마우스 이벤트 소스
typedef enum WindowsMouseSource
{
	WINDOWS_MOUSE_SOURCE_MOUSE,
	WINDOWS_MOUSE_SOURCE_TOUCH,
	WINDOWS_MOUSE_SOURCE_PEN,
} WindowsMouseSource;

// 윈도우 스터브 > StubBase
typedef struct WindowsStub
{
	StubBase			base;

	HINSTANCE			instance;
	HWND				hwnd;

	wchar*				class_name;
	wchar*				window_title;
	DWORD				window_style;

	STICKYKEYS			acs_sticky;
	TOGGLEKEYS			acs_toggle;
	FILTERKEYS			acs_filter;
	HHOOK				key_hook;
	BYTE				key_hook_state[256];

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
	bool				bool_padding1;
	bool				bool_padding2;
} WindowsStub;

// 윈도우 스터브 선언
extern WindowsStub winStub;
