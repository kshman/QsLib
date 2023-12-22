//
// qg_stub_windows.c - 윈도우 스터브
// 2023-12-13 by kim
//

#include "pch.h"
#include "qs_qn.h"
#if defined _QN_WINDOWS_ && !defined USE_SDL2
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qg_stub.h"
#include <windowsx.h>
#include <Xinput.h>
#include <shellscalingapi.h>

#ifdef _MSC_VER
#pragma warning(disable:4152)
#endif

#ifdef _DEBUG
//#define DEBUG_WINPROC_MESG
#endif

static_assert(sizeof(RECT) == sizeof(QmRect), "RECT size not equal to QmRect");

extern const char* windows_message_str(UINT mesg);


//////////////////////////////////////////////////////////////////////////
// 윈도우 스터브

// 컨트롤러
#define DEFAULT_DEAD_ZONE	(int)(0.24f*((float)INT16_MAX))

#ifndef _QN_XBOX_
static struct WindowsXInputFunc
{
	void (WINAPI* funcXInputEnable)(_In_ BOOL enable);
	DWORD(WINAPI* funcXInputGetState)(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState);
	DWORD(WINAPI* funcXInputSetState)(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);
	DWORD(WINAPI* funcXInputGetCapabilities)(_In_ DWORD dwUserIndex, _In_ DWORD dwFlags, _Out_ XINPUT_CAPABILITIES* pCapabilities);
	DWORD(WINAPI* funcXInputGetBatteryInformation)(_In_ DWORD dwUserIndex, _In_ BYTE devType, _Out_ XINPUT_BATTERY_INFORMATION* pBatteryInformation);
} s_xinput_fn;

static void windows_internal_controller_init(void)
{
	QnModule* mod = NULL;
	static char dll[64] = "xinput1_ ";
	for (int i = 4; i >= 1; i--)
	{
		dll[8] = (char)('0' + i);
		if ((mod = qn_mod_open(dll, 0)) != NULL)
			break;
	}
	if (mod == NULL)
		return;
#define DEF_CTRL_FN(x)		s_xinput_fn.func##x = qn_mod_func(mod, #x)
	DEF_CTRL_FN(XInputEnable);
	DEF_CTRL_FN(XInputGetState);
	DEF_CTRL_FN(XInputSetState);
	DEF_CTRL_FN(XInputGetCapabilities);
	DEF_CTRL_FN(XInputGetBatteryInformation);
#undef DEF_CTRL_FN

	if (s_xinput_fn.funcXInputEnable != NULL)
		s_xinput_fn.funcXInputEnable(TRUE);
}

// Windows DLL
static struct WindowsDll
{
	// user32
	BOOL(WINAPI *funcSetProcessDPIAware)(void);
	BOOL(WINAPI *funcSetProcessDpiAwarenessContext)(DPI_AWARENESS_CONTEXT);
	DPI_AWARENESS_CONTEXT(WINAPI *funcSetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT);
	DPI_AWARENESS_CONTEXT(WINAPI *funcGetThreadDpiAwarenessContext)(void);
	DPI_AWARENESS(WINAPI *funcGetAwarenessFromDpiAwarenessContext)(DPI_AWARENESS_CONTEXT);
	BOOL(WINAPI *funcEnableNonClientDpiScaling)(HWND);
	BOOL(WINAPI *funcAdjustWindowRectExForDpi)(LPRECT, DWORD, BOOL, DWORD, UINT);
	UINT(WINAPI *funcGetDpiForWindow)(HWND);
	BOOL(WINAPI *funcAreDpiAwarenessContextsEqual)(DPI_AWARENESS_CONTEXT, DPI_AWARENESS_CONTEXT);
	BOOL(WINAPI *funcIsValidDpiAwarenessContext)(DPI_AWARENESS_CONTEXT);
	// SHCORE
	HRESULT(WINAPI *funcGetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);
	HRESULT(WINAPI *funcSetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
	// IMM
	BOOL(WINAPI *funcImmAssociateContextEx)(HWND, HIMC, DWORD);
} s_dlls;

static void windows_internal_user_dll_init(void)
{
#define DEF_DLL_FN(name)	s_dlls.func##name = qn_mod_func(mod, #name)
	QnModule* mod;

	// user32
	mod = qn_mod_open("USER32", 0);
	if (mod != NULL)
	{
		DEF_DLL_FN(SetProcessDPIAware);
		DEF_DLL_FN(SetProcessDpiAwarenessContext);
		DEF_DLL_FN(SetThreadDpiAwarenessContext);
		DEF_DLL_FN(GetThreadDpiAwarenessContext);
		DEF_DLL_FN(GetAwarenessFromDpiAwarenessContext);
		DEF_DLL_FN(EnableNonClientDpiScaling);
		DEF_DLL_FN(AdjustWindowRectExForDpi);
		DEF_DLL_FN(GetDpiForWindow);
		DEF_DLL_FN(AreDpiAwarenessContextsEqual);
		DEF_DLL_FN(IsValidDpiAwarenessContext);
	}

	// shcore
	mod = qn_mod_open("SHCORE", 0);
	if (mod != NULL)
	{
		DEF_DLL_FN(GetDpiForMonitor);
		DEF_DLL_FN(SetProcessDpiAwareness);
	}

	// imm
	mod = qn_mod_open("IMM32", 0);
	if (mod != NULL)
	{
		DEF_DLL_FN(ImmAssociateContextEx);
	}
#undef DEF_DLL_FN
}
#endif

// 키 후킹
static HHOOK s_key_hook;

static LRESULT CALLBACK windows_internal_hook_key_proc(int code, WPARAM wp, LPARAM lp)
{
	if (code < 0 || code != HC_ACTION)
		return CallNextHookEx(s_key_hook, code, wp, lp);
	LPKBDLLHOOKSTRUCT kh = (LPKBDLLHOOKSTRUCT)lp;
	bool hook = false;
	if (wp == WM_KEYDOWN || wp == WM_KEYUP)
		hook = kh->vkCode == VK_LWIN || kh->vkCode == VK_RWIN || kh->vkCode == VK_MENU;
	return hook ? 1 : CallNextHookEx(s_key_hook, code, wp, lp);
}

static void windows_internal_unhook_key(void)
{
	if (s_key_hook != NULL)
	{
		UnhookWindowsHookEx(s_key_hook);
		s_key_hook = NULL;
	}
}

static void windows_internal_hook_key(HINSTANCE instance)
{
	windows_internal_unhook_key();
	s_key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, windows_internal_hook_key_proc, instance, 0);
}

/** @brief 윈도우 스터브 */
typedef struct WindowsStub WindowsStub;
struct WindowsStub
{
	StubBase			base;

	HWND				hwnd;
	HINSTANCE			instance;

	wchar*				class_name;
	wchar*				window_title;

	RECT				window_rect;
	DWORD				window_style;

	DEVMODE				dev_mode;
	STICKYKEYS			acs_sticky;
	TOGGLEKEYS			acs_toggle;
	FILTERKEYS			acs_filter;

	HIMC				himc;
	int					imcs;
	int					high_surrogate;

	int					deadzone_min;
	int					deadzone_max;

	HCURSOR				mouse_cursor;
	llong				mouse_warp_time;
	WPARAM				mouse_wparam;
	LPARAM				mouse_lparam;
	QimMask				mouse_pending;

	bool				class_registered;
	bool				diable_acs;
	bool				disable_scrsave;
	bool				enable_drop;

	bool				clear_background;
};

// 정적 함수 미리 정의
static void win_dpi_awareness(WindowsStub* stub);
static LRESULT CALLBACK win_mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp);

//
StubBase* stub_system_open(const char* title, int width, int height, int flags)
{
	static WindowsStub s_stub;
	WindowsStub* stub = &s_stub;

	stub->instance = GetModuleHandle(NULL);
	if (stub->instance == NULL)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "cannot retrieve module handle");
		return NULL;
	}

#ifndef _QN_XBOX_
	windows_internal_controller_init();
	windows_internal_user_dll_init();
#endif

	// 디스플레이 정보
	stub->dev_mode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &stub->dev_mode);

	// 윈도우 클래스
	if (stub->class_registered == false)
	{
		QnDateTime dt;
		qn_now(&dt);
		stub->class_name = qn_apswprintf(L"qs_stub_class_%llu", dt.stamp);

		WNDCLASSEX wc =
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_BYTEALIGNCLIENT | CS_OWNDC,
			.lpfnWndProc = win_mesg_proc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = stub->instance,
			.hIcon = NULL,
			.hCursor = NULL,
			.hbrBackground = NULL,
			.lpszMenuName = NULL,
			.lpszClassName = stub->class_name,
			.hIconSm = NULL,
		};
#ifndef _QN_XBOX_
		const char* prop = qn_get_prop(QG_PROP_WINDOWS_ICON);
		if (prop && *prop)
		{
			wc.hIcon = LoadIcon(stub->instance, MAKEINTRESOURCE(qn_strtoi(prop, 10)));
			prop = qn_get_prop(QG_PROP_WINDOWS_SMALLICON);
			if (prop && *prop)
				wc.hIconSm = LoadIcon(stub->instance, MAKEINTRESOURCE(qn_strtoi(prop, 10)));
		}
		else
		{
			wchar module_name[260];
			GetModuleFileName(NULL, module_name, QN_COUNTOF(module_name) - 1);
			ExtractIconEx(module_name, 0, &wc.hIcon, &wc.hIconSm, 1);
		}
#endif
		if (!RegisterClassEx(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		{
#ifndef _QN_XBOX_
			if (wc.hIcon)
				DestroyIcon(wc.hIcon);
			if (wc.hIconSm)
				DestroyIcon(wc.hIconSm);
#endif
			qn_free(stub->class_name);
			qn_debug_outputs(true, "WINDOWS STUB", "window class registration failed");
			return NULL;
		}

		stub->class_registered = true;
	}

	// 크기 및 윈도우 위치
	DWORD style;
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
		style = WS_POPUP;
	else
	{
		if (QN_TMASK(flags, QGFLAG_BORDERLESS))
		{
			style = WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN;
			if (QN_TMASK(flags, QGFLAG_RESIZABLE))
				style |= WS_THICKFRAME | WS_BORDER;
		}
		else
		{
			style = WS_SYSMENU | WS_TILED | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
			if (QN_TMASK(flags, QGFLAG_RESIZABLE))
				style |= WS_MAXIMIZEBOX | WS_THICKFRAME | WS_BORDER;
		}
	}

	QmSize scrsize = { GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };
	RECT rect;
	if (width > 256 && height > 256)
		SetRect(&rect, 0, 0, width, height);
	else
	{
		if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
			SetRect(&rect, 0, 0, scrsize.width, scrsize.height);
		else
		{
			if (scrsize.height > 800)
				SetRect(&rect, 0, 0, 1280, 720);
			else
				SetRect(&rect, 0, 0, 720, 450);
		}
	}
	AdjustWindowRect(&rect, style, FALSE);

	QmPoint pos = { 0, 0 };
	QmSize size = { rect.right - rect.left, rect.bottom - rect.top };
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN) == false)
		qm_point_set(&pos, (scrsize.width - size.width) / 2, (scrsize.height - size.height) / 2);

	SetRect(&stub->window_rect, pos.x, pos.y, pos.x + size.width, pos.y + size.height);
	memcpy(&stub->base.window_bound, &stub->window_rect, sizeof(RECT));
	stub->window_style = style;

	// 토글키 상태 저장
	QgUimKey* uk = &stub->base.key;
	if (GetKeyState(VK_CAPITAL) & 0x1)
		QN_SMASK(&uk->mask, QIKM_CAPS, true);
	if (GetKeyState(VK_SCROLL) & 0x1)
		QN_SMASK(&uk->mask, QIKM_SCRL, true);
	if (GetKeyState(VK_NUMLOCK) & 0x1)
		QN_SMASK(&uk->mask, QIKM_NUM, true);

	// 커서
	stub->mouse_cursor = LoadCursor(NULL, IDC_ARROW);

	// 값설정
	stub->window_title = qn_u8to16_dup(title ? title : "QS", 0);

	stub->deadzone_min = -DEFAULT_DEAD_ZONE;
	stub->deadzone_max = +DEFAULT_DEAD_ZONE;

	return (StubBase*)stub;
}

//
bool stub_system_create_window(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	stub->hwnd = CreateWindowEx(0, stub->class_name, stub->window_title, stub->window_style,
		stub->window_rect.left, stub->window_rect.top,
		stub->window_rect.right - stub->window_rect.left,
		stub->window_rect.bottom - stub->window_rect.top,
		NULL, NULL, stub->instance, NULL);
	if (stub->hwnd == NULL)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "cannot create window");
		return false;
	}

	ShowWindow(stub->hwnd, SW_SHOWNORMAL);
	UpdateWindow(stub->hwnd);

	// 최종 스타일 변경
	if (QN_TMASK(stub->base.flags, QGFLAG_FULLSCREEN | QGFLAG_NOTITLE))
		SetWindowLong(stub->hwnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE);
	else
	{
		stub->window_style = GetWindowLong(stub->hwnd, GWL_STYLE);
		SetWindowLong(stub->hwnd, GWL_STYLE, stub->window_style);
	}

	stub_system_calc_layout();
	win_dpi_awareness(stub);

	return true;
}

//
void stub_system_finalize(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

#ifndef _QN_XBOX_
	if (s_xinput_fn.funcXInputEnable != NULL)
		s_xinput_fn.funcXInputEnable(FALSE);
#endif

	if (stub->hwnd != NULL)
	{
		stub->base.stats |= QGSSTT_EXIT;
		stub_system_hold_mouse(false);

		if (stub->himc != NULL && s_dlls.funcImmAssociateContextEx != NULL)
			s_dlls.funcImmAssociateContextEx(stub->hwnd, stub->himc, IACE_DEFAULT);
		if (QN_TMASK(stub->base.flags, QGSPECIFIC_VIRTUAL) == false)
			SendMessage(stub->hwnd, WM_CLOSE, 0, 0);
	}

	if (stub->mouse_cursor != NULL)
		DestroyCursor(stub->mouse_cursor);

	stub_system_disable_acs(false);

	if (stub->class_registered)
	{
		stub->class_registered = false;
#ifndef _QN_XBOX_
		WNDCLASSEX wc;
		if (GetClassInfoEx(stub->instance, stub->class_name, &wc))
			UnregisterClass(stub->class_name, stub->instance);
#endif		
	}

	qn_free(stub->window_title);
	qn_free(stub->class_name);
}

//
bool stub_system_poll(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
	MSG msg;
	ULONGLONG tick = GetTickCount64() + 1;
	int count = 0;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;

		if (msg.message == WM_MOUSEMOVE && stub->mouse_warp_time)
		{
			if ((int)(stub->mouse_warp_time + 1 - msg.time) > 0)
				continue;
			stub->mouse_warp_time = 0;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if ((int)(tick - msg.time) <= 0)
		{
			count++;
			if (count > 3)
				break;
		}
	}
#ifndef _QN_XBOX_
	if (qg_test_key(QIK_LSHIFT) && (GetKeyState(VK_LSHIFT) & 0x8000) == 0)
		stub_internal_on_keyboard(QIK_LSHIFT, false);
	if (qg_test_key(QIK_RSHIFT) && (GetKeyState(VK_RSHIFT) & 0x8000) == 0)
		stub_internal_on_keyboard(QIK_RSHIFT, false);
#endif

	return QN_TMASK(stub->base.stats, QGSSTT_EXIT) == false;
}

//
void stub_system_disable_acs(bool enable)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	if (enable)
	{
		if (stub->diable_acs)
			return;
		stub->diable_acs = true;
		QN_SMASK(&stub->base.flags, QGFEATURE_DISABLE_ACS, true);

		stub->acs_sticky.cbSize = sizeof(STICKYKEYS);
		stub->acs_toggle.cbSize = sizeof(TOGGLEKEYS);
		stub->acs_filter.cbSize = sizeof(FILTERKEYS);
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &stub->acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &stub->acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &stub->acs_filter, 0);

		STICKYKEYS sticky = stub->acs_sticky;
		TOGGLEKEYS toggle = stub->acs_toggle;
		FILTERKEYS filter = stub->acs_filter;
		sticky.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		toggle.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		filter.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &filter, 0);

		windows_internal_hook_key(stub->instance);
	}
	else
	{
		if (stub->diable_acs == false)
			return;
		stub->diable_acs = false;
		QN_SMASK(&stub->base.flags, QGFEATURE_DISABLE_ACS, false);

		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &stub->acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &stub->acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &stub->acs_filter, 0);

		windows_internal_unhook_key();
	}
}

//
void stub_system_diable_scrsave(bool enable)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	if (enable)
	{
		if (stub->disable_scrsave)
			return;
		stub->disable_scrsave = true;
		QN_SMASK(&stub->base.flags, QGFEATURE_DISABLE_SCRSAVE, true);
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
	}
	else
	{
		if (stub->disable_scrsave == false)
			return;
		stub->disable_scrsave = true;
		QN_SMASK(&stub->base.flags, QGFEATURE_DISABLE_SCRSAVE, false);
		SetThreadExecutionState(ES_CONTINUOUS);
	}
}

//
void stub_system_enable_drop(bool enable)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
	if (enable)
	{
		if (stub->enable_drop)
			return;
		stub->enable_drop = true;
		QN_SMASK(&stub->base.flags, QGFEATURE_ENABLE_DROP, true);
		DragAcceptFiles(stub->hwnd, TRUE);
	}
	else
	{
		if (stub->enable_drop == false)
			return;
		stub->enable_drop = false;
		QN_SMASK(&stub->base.flags, QGFEATURE_ENABLE_DROP, false);
		DragAcceptFiles(stub->hwnd, FALSE);
	}
}

//
void stub_system_set_title(const char* u8text)
{
	qn_ret_if_fail(u8text && *u8text);
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
	qn_free(stub->window_title);
	stub->window_title = qn_u8to16_dup(u8text, 0);
	SetWindowText(stub->hwnd, stub->window_title);
}

//
void stub_system_hold_mouse(bool hold)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	if (hold)
	{
		if (QN_TMASK(stub->base.stats, QGSSTT_MOUSEHOLD))
			return;
		QN_SMASK(&stub->base.stats, QGSSTT_MOUSEHOLD, true);
		SetCapture(stub->hwnd);
	}
	else
	{
		if (QN_TMASK(stub->base.stats, QGSSTT_MOUSEHOLD) == false)
			return;
		QN_SMASK(&stub->base.stats, QGSSTT_MOUSEHOLD, false);
		ReleaseCapture();
	}
}

//
void stub_system_calc_layout(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
	RECT rect;

	GetWindowRect(stub->hwnd, &rect);
	stub->window_rect = rect;
	memcpy(&stub->base.window_bound, &rect, sizeof(RECT));

	GetClientRect(stub->hwnd, &rect);
	qm_rect_set(&stub->base.bound, rect.left, rect.top, rect.right, rect.bottom);
	qm_size_set_rect(&stub->base.size, &stub->base.bound);
}

//
static void win_dpi_awareness(WindowsStub* stub)
{
	// 해야한다!
}

#ifndef _QN_XBOX_
// https://learn.microsoft.com/ko-kr/windows/win32/tablet/system-events-and-mouse-messages
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

// 마우스 이벤트 소스
static WindowsMouseSource win_get_mouse_source(void)
{
	LPARAM info = GetMessageExtraInfo();
	if (IsPenEvent(info))
	{
		if (QN_TMASK(info, 0x80))
			return WINDOWS_MOUSE_SOURCE_TOUCH;
		return WINDOWS_MOUSE_SOURCE_PEN;
	}
	return WINDOWS_MOUSE_SOURCE_MOUSE;
}

//
static void win_set_mouse_point(WindowsStub* stub, LPARAM lp, bool save)
{
	if (stub->mouse_lparam == lp)
		return;

	QgUimMouse* mouse = &stub->base.mouse;
	POINT pt = { .x = GET_X_LPARAM(lp), .y = GET_Y_LPARAM(lp) };
	if (save)
		mouse->last = mouse->pt;
	qm_point_set(&mouse->pt, pt.x, pt.y);
}

// 마우스 버튼 처리
static void win_check_mouse_button(WindowsStub* stub, bool pressed, QimMask mask, QimButton button)
{
	if (QN_TBIT(stub->mouse_pending, button))
	{
		if (pressed == false)
		{
			stub->mouse_pending &= ~QN_BIT(button);
			// 클리핑 업데이트
		}
		// 포커스 클릭 무시
	}

	if (pressed && QN_TBIT(mask, button) == false)
		stub_internal_on_mouse_button(button, true);
	else if (pressed == false && QN_TBIT(mask, button))
		stub_internal_on_mouse_button(button, false);
}

// 마우스 눌림 해제
static void win_check_mouse_release(WindowsStub* stub)
{
	QimMask mask = stub->base.mouse.mask;
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
		win_check_mouse_button(stub, FALSE, mask, QIM_LEFT);
	if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
		win_check_mouse_button(stub, FALSE, mask, QIM_RIGHT);
	if ((GetAsyncKeyState(VK_MBUTTON) & 0x8000) == 0)
		win_check_mouse_button(stub, FALSE, mask, QIM_MIDDLE);
	if ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) == 0)
		win_check_mouse_button(stub, FALSE, mask, QIM_X1);
	if ((GetAsyncKeyState(VK_XBUTTON2) & 0x8000) == 0)
		win_check_mouse_button(stub, FALSE, mask, QIM_X2);
	stub->mouse_wparam = 0;
}
#endif

// 키보드 눌림
static bool win_mesg_keyboard(WindowsStub* stub, WPARAM wp, LPARAM lp, bool down)
{
	byte key = (byte)(wp & 0xFF);

#ifdef _DEBUG
	if (down && key == VK_F12)
	{
		SendMessage(stub->hwnd, WM_CLOSE, 0, 0);
		return true;
	}
#endif

	return stub_internal_on_keyboard((QikKey)key, down) == 0 ? false : key != VK_MENU;
}

// 액티브
static void win_mesg_active(WindowsStub* stub, bool focus)
{
	double now = stub->base.timer->abstime;
	double delta = now - stub->base.active;
	stub->base.active = now;

	bool has_focus = GetForegroundWindow() == stub->hwnd;
	if (has_focus != focus)
		return;

	if (has_focus)
	{
		QN_SMASK(&stub->base.stats, QGSSTT_ACTIVE, true);

		if (GetAsyncKeyState(VK_LBUTTON))
			stub->mouse_pending |= QIMM_LEFT;
		if (GetAsyncKeyState(VK_RBUTTON))
			stub->mouse_pending |= QIMM_RIGHT;
		if (GetAsyncKeyState(VK_MBUTTON))
			stub->mouse_pending |= QIMM_MIDDLE;
		if (GetAsyncKeyState(VK_XBUTTON1))
			stub->mouse_pending |= QIMM_X1;
		if (GetAsyncKeyState(VK_XBUTTON2))
			stub->mouse_pending |= QIMM_X2;

		win_check_mouse_release(stub);

		stub_internal_toggle_key(QIKM_CAPS, (GetKeyState(VK_CAPITAL) & 0x1) != 0);
		stub_internal_toggle_key(QIKM_SCRL, (GetKeyState(VK_SCROLL) & 0x1) != 0);
		stub_internal_toggle_key(QIKM_NUM, (GetKeyState(VK_NUMLOCK) & 0x1) != 0);

		stub_internal_on_window_event(QGWEV_GOTFOCUS, 0, 0);
		stub_internal_on_active(true, delta);
	}
	else
	{
		QN_SMASK(&stub->base.stats, QGSSTT_ACTIVE, false);

		stub_internal_on_reset_keys();
		stub_internal_on_window_event(QGWEV_LOSTFOCUS, 0, 0);
		stub_internal_on_active(false, delta);
	}
}

//
static LRESULT CALLBACK win_mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
	LRESULT result = -1;

	if (stub == NULL)
	{
#ifdef DEBUG_WINPROC_MESG
		qn_debug_outputf(false, "WINDOW STUB", "처리안된 메시지: %s(%X)", windows_message_str(mesg), mesg);
#endif
		return CallWindowProc(DefWindowProc, hwnd, mesg, wp, lp);
	}

	if (QN_TMASK(stub->base.flags, QGFEATURE_ENABLE_SYSWM))
	{
		QgEvent e =
		{
			.syswm.ev = QGEV_SYSWM,
			.syswm.hwnd = hwnd,
			.syswm.mesg = mesg,
			.syswm.wparam = wp,
			.syswm.lparam = lp,
		};
		qg_add_event(&e);
	}

#ifdef DEBUG_WINPROC_MESG
	qn_debug_outputf(false, "WINDOW STUB", "윈도우 메시지: %s(%X)", windows_message_str(mesg), mesg);
#endif

#ifndef _QN_XBOX_
	// 마우스
	while (true)
	{
		if (mesg == WM_MOUSEMOVE)
		{
			win_set_mouse_point(stub, lp, true);
			stub_internal_mouse_clicks(QIM_NONE, QIMT_MOVE);
			stub_internal_on_mouse_move();
		}
		else if ((mesg >= WM_LBUTTONDOWN && mesg <= WM_MBUTTONDBLCLK) || (mesg >= WM_XBUTTONDOWN && mesg <= WM_XBUTTONDBLCLK))
		{
			if (win_get_mouse_source() != WINDOWS_MOUSE_SOURCE_TOUCH)
			{
				win_set_mouse_point(stub, lp, false);
				if (stub->mouse_wparam != wp)
				{
					stub->mouse_wparam = wp;
					QimMask mask = stub->base.mouse.mask;
					win_check_mouse_button(stub, QN_TMASK(wp, MK_LBUTTON), mask, QIM_LEFT);
					win_check_mouse_button(stub, QN_TMASK(wp, MK_RBUTTON), mask, QIM_RIGHT);
					win_check_mouse_button(stub, QN_TMASK(wp, MK_MBUTTON), mask, QIM_MIDDLE);
					win_check_mouse_button(stub, QN_TMASK(wp, MK_XBUTTON1), mask, QIM_X1);
					win_check_mouse_button(stub, QN_TMASK(wp, MK_XBUTTON2), mask, QIM_X2);
				}
			}
		}
		else if (mesg == WM_MOUSEWHEEL || mesg == WM_MOUSEHWHEEL)
		{
			int wheel = GET_WHEEL_DELTA_WPARAM(wp);
			float delta = (float)wheel / WHEEL_DELTA;
			if (mesg == WM_MOUSEWHEEL)
				stub_internal_on_mouse_wheel(0.0f, delta, false);
			else
				stub_internal_on_mouse_wheel(delta, 0.0f, false);
		}
		else break;
		goto pos_mesg_proc_exit;
	}
#endif

	// 키보드
	while (true)
	{
		if (mesg == WM_KEYDOWN || mesg == WM_SYSKEYDOWN)
		{
			if (!win_mesg_keyboard(stub, wp, lp, true))
				result = 0;
		}
		else if (mesg == WM_KEYUP || mesg == WM_SYSKEYUP)
		{
			if (!win_mesg_keyboard(stub, wp, lp, false))
				result = 0;
		}
		else break;
		goto pos_mesg_proc_exit;
	}

	// 기타 메시지
	switch (mesg)
	{
#ifndef _QN_XBOX_
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_SIZE:
			switch (wp)
			{
				case SIZE_MAXIMIZED:
					stub_internal_on_window_event(QGWEV_MAXIMIZED, 0, 0);
					break;
				case SIZE_MINIMIZED:
					stub_internal_on_window_event(QGWEV_MINIMIZED, 0, 0);
					break;
				case SIZE_RESTORED:
					stub_internal_on_window_event(QGWEV_RESTORED, 0, 0);
					break;
				default:
					break;
			}
			break;

		case WM_ACTIVATE:
			win_mesg_active(stub, !!(LOWORD(wp)));
			break;

		case WM_SETFOCUS:
			win_mesg_active(stub, true);
			break;

		case WM_KILLFOCUS:
		case WM_ENTERIDLE:
			win_mesg_active(stub, false);
			break;

		case WM_PAINT:
		{
			RECT rt;
			if (GetUpdateRect(hwnd, &rt, FALSE))
			{
				ValidateRect(hwnd, NULL);
				stub_internal_on_window_event(QGWEV_PAINTED, 0, 0);
			}
			return 0;
		} break;

		case WM_CLOSE:
			stub_internal_on_window_event(QGWEV_CLOSE, 0, 0);
			QN_SMASK(&stub->base.stats, QGSSTT_EXIT, true);
			DestroyWindow(hwnd);
			result = 0;
			break;

		case WM_ERASEBKGND:
			if (stub->clear_background == false)
			{
				stub->clear_background = true;
				RECT rect;
				GetClientRect(hwnd, &rect);
				HBRUSH brush = CreateSolidBrush(0);
				FillRect(GetDC(hwnd), &rect, brush);
				DeleteObject(brush);
			}
			return 1;

		case WM_SHOWWINDOW:
			stub_internal_on_window_event(wp ? QGWEV_SHOW : QGWEV_HIDE, 0, 0);
			break;

		case WM_SETTINGCHANGE:
			if (wp == SPI_SETMOUSE || wp == SPI_SETMOUSESPEED)
			{
				// 웁스
			}
			break;

		case WM_SETCURSOR:
		{
			WORD hittest = LOWORD(lp);
			if (hittest == HTCLIENT && QN_TMASK(stub->base.stats, QGSSTT_CURSOR))
			{
				SetCursor(stub->mouse_cursor);
				result = 1;
			}
			else if (stub->mouse_cursor == NULL)
			{
				SetCursor(NULL);
				result = 1;
			}
		}break;

		case WM_GETMINMAXINFO:
			// 창의 크기 또는 위치가 변경될 때
			// 여기서 기본 기본 최대 크기와 위치, 기본 최소 또는 최대 추적 크기 재정의
			// result = 0;
			break;

		case WM_WINDOWPOSCHANGING:
			if (QN_TMASK(stub->base.stats, QGSSTT_LAYOUT))
				result = 0;
			break;

		case WM_WINDOWPOSCHANGED:
		{
			if (IsIconic(hwnd))
				break;

			RECT rect;
			if (!GetWindowRect(hwnd, &rect) || rect.right <= rect.left || rect.bottom <= rect.top)
				break;

			//ClientToScreen(hwnd, (LPPOINT)&rect);		// GetClientRect일땐 이거쓰는데 지금 GetWindowRect씀
			//ClientToScreen(hwnd, (LPPOINT)&rect + 1);
			stub_internal_on_window_event(QGWEV_MOVED, rect.left, rect.top);
			stub_internal_on_window_event(QGWEV_SIZED, rect.right - rect.left, rect.bottom - rect.top);

			InvalidateRect(hwnd, NULL, FALSE);
		}break;

		case WM_INPUTLANGCHANGE:
			//result = 1;
			break;

		case WM_NCCALCSIZE:
			break;

		case WM_NCACTIVATE:
			win_mesg_active(stub, !!wp);
			break;

		case WM_NCHITTEST:
			if (QN_TMASK(stub->base.flags, QGFLAG_FULLSCREEN))
				return HTCLIENT;
			break;

		case WM_NCLBUTTONUP:
			// 타이틀 눌림
			break;

		case WM_DISPLAYCHANGE:
			break;

		case WM_INPUT:
			break;

		case WM_CHAR:
			if (IS_HIGH_SURROGATE(wp))
				stub->high_surrogate = (WCHAR)wp;
			else if (IS_SURROGATE_PAIR(stub->high_surrogate, wp))
			{
				// 여기서 stub->high_surrogate 와 wp 를 조합해서 키보드 데이터를 처리 
				// https://learn.microsoft.com/ko-kr/windows/win32/inputdev/wm-char
				char u8[7];
				if (qn_u16ucb((uchar2)stub->high_surrogate, (uchar2)wp, u8))
					stub_internal_on_text(u8);
				stub->high_surrogate = 0;
			}
			else
			{
				// 윈도우10 UTF32
				char u8[7];
				if (qn_u32ucb((uchar4)wp, u8))
					stub_internal_on_text(u8);
			}
			result = 0;
			break;

		case WM_UNICHAR:
			if (wp == UNICODE_NOCHAR)
				result = 1;
			else
			{
				// 여기서 키보드 데이터를 처리
				// https://learn.microsoft.com/ko-kr/windows/win32/inputdev/wm-unichar
				char u8[7];
				if (qn_u32ucb((uchar4)wp, u8))
					stub_internal_on_text(u8);
				result = 0;
			}
			break;

		case WM_SYSCOMMAND:
		{
			ushort cmd = (ushort)(wp & 0xFFFF0);
			if (cmd == SC_KEYMENU)
				result = 0;
			if (stub->disable_scrsave && (cmd == SC_SCREENSAVE || cmd == SC_MONITORPOWER))
				result = 0;
		} break;

		case WM_MENUCHAR:
			result = MAKELRESULT(0, MNC_CLOSE);
			break;

		case WM_CAPTURECHANGED:
			// 타이틀 안 눌림
			break;

		case WM_ENTERSIZEMOVE:
			stub_internal_on_event_layout(true);
			break;

		case WM_EXITSIZEMOVE:
			stub_internal_on_event_layout(false);
			break;

		case WM_DROPFILES:
			if (QN_TMASK(stub->base.flags, QGFEATURE_ENABLE_DROP))
			{
				DragAcceptFiles(hwnd, false);
				HDROP handle = (HDROP)wp;
				UINT cnt = DragQueryFile(handle, 0xFFFFFFFF, NULL, 0);
				for (UINT i = 0; i < cnt; i++)
				{
					UINT len = DragQueryFile(handle, i, NULL, 0) + 1;
					LPWSTR buf = qn_alloc(len, WCHAR);
					if (DragQueryFile(handle, i, buf, len))
					{
						char *filename = qn_u16to8_dup(buf, 0);
						stub_internal_on_drop(filename, 0, false);
					}
					qn_free(buf);
				}
				stub_internal_on_drop(NULL, 0, true);
				DragAcceptFiles(hwnd, true);
				result = 0;
			}
			break;

		case WM_TOUCH:
			break;

		case WM_POINTERUPDATE:
			stub->mouse_lparam = lp;
			break;

		case WM_MOUSELEAVE:
			result = 0;
			break;

		case WM_DPICHANGED:
			break;

		case WM_GETDPISCALEDSIZE:
			break;
#endif
	}

pos_mesg_proc_exit:
	if (result >= 0)
		return result;
	return CallWindowProc(DefWindowProc, hwnd, mesg, wp, lp);
}
#endif
