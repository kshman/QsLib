//
// qgwlx_stub.h - Wayland 스터브
// 2023-12-23 by kim
//

#pragma once

#include "qg_stub.h"
#include <windowsx.h>
#include <Xinput.h>
#include <shellscalingapi.h>

// DLL
#define DEF_WLX_FUNC(ret,name,args)		typedef ret(*QN_CONCAT(PFNWlx, name)) args;
#include "qgwin_func.h"
#define DEF_WLX_FUNC(ret,name,args)		extern QN_CONCAT(PFNWlx, name) QN_CONCAT(Wlx, name);
#include "qgwin_func.h"

// 웨이랜드 모니터
typedef struct WaylandMonitor
{
	QgUdevMonitor		base;

	wchar				adapter[32];
	wchar				display[32];
} WaylandMonitor;

// 웨이랜드 스터브
typedef struct WaylandStub
{
	StubBase			base;

#if false
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
#endif
} WaylandStub;

// 웨이랜드 스터브 선언
extern WaylandStub wlxStub;
