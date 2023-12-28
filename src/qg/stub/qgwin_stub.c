//
// qgwin_stub.c - 윈도우 스터브
// 2023-12-13 by kim
//

// TODO: 마우스 감추고 보이기 + 커서 처리
// TODO: 화면 종횡비 맞추기
// TODO: DPI 추정

// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppParameterMayBeConstPtrOrRef

#include "pch.h"
#include "qs_qn.h"
#ifdef _QN_WINDOWS_
#include "qs_qg.h"
#include "qs_kmc.h"
#include <windowsx.h>
#include <Xinput.h>
#include <shellscalingapi.h>

#ifdef _DEBUG
#define DEBUG_WIN_DLL_TRACE
#endif

#pragma region 정적 함수 미리 선언
static void _set_key_hook(HINSTANCE instance);
static void _set_dpi_awareness(void);
static bool _detect_displays(void);
static void _hold_mouse(bool hold);
static LRESULT CALLBACK _mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp);
#pragma endregion

#pragma region DLL 처리
// DLL 정의
#define DEF_WIN_TYPEDEF
#define DEF_WIN_FUNC(ret,name,args)\
	typedef ret(WINAPI* QN_CONCAT(PFNWin32, name)) args;\
	QN_CONCAT(PFNWin32, name) QN_CONCAT(Win32, name);
#include "qgwin_stub_func.h"

// DLL 함수
static void* _load_dll_func(QnModule* module, const char* dll_name, const char* func_name)
{
	void* ret = qn_mod_func(module, func_name);
#ifdef DEBUG_WIN_DLL_TRACE
	qn_debug_outputf(false, "WINDOWS STUB", "\t%s: '%s' in '%s'",
		ret == NULL ? "load failed" : "loaded", func_name, dll_name);
#else
	QN_DUMMY(dll_name);
#endif
	return ret;
}

// DLL 초기화
static bool _dll_init(void)
{
	static bool loaded = false;
	qn_val_if_ok(loaded, true);
	QnModule* module = NULL;
	const char* dll_name = NULL;
	static char xinput_dll[64] = "XINPUT1_9";
	for (int i = 4; i >= 1; i--)
	{
		xinput_dll[8] = (char)('0' + i);
		if ((module = qn_mod_load(xinput_dll, 1)) != NULL)
		{
			dll_name = xinput_dll;
			break;
		}
	}
	if (module == NULL)
	{
		// 이건 오바다
		qn_debug_outputf(true, "WINDOWS STUB", "no '%s' DLL found!", "XINPUT");
		return false;
	}
#define DEF_WIN_DLL_BEGIN(name)\
	module = qn_mod_load(dll_name = name, 1); if (module == NULL)\
	{ qn_debug_outputf(true, "WINDOWS STUB", "no '%s' DLL found!", dll_name); return false; } else {
#define DEF_WIN_DLL_END }
#define DEF_WIN_FUNC(ret,name,args)\
	QN_CONCAT(Win32, name) = (QN_CONCAT(PFNWin32, name))_load_dll_func(module, dll_name, QN_STRING(name));
#include "qgwin_stub_func.h"
	return loaded = true;
}
#pragma endregion


//////////////////////////////////////////////////////////////////////////

#pragma region 스터브 선언
// 본편 시작
#include "qgwin_stub.h"

// 윈도우 스터브 여기 있다!
WindowsStub winStub;
#pragma endregion 스터브 선언

#pragma region 시스템 함수
//
bool stub_system_open(const char* title, const int display, const int width, const int height, QgFlag flags)
{
	qn_zero_1(&winStub);

	//
	if (_dll_init() == false)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "DLL load failed");
		return false;
	}

	winStub.instance = GetModuleHandle(NULL);
	if (winStub.instance == NULL)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "cannot retrieve module handle");
		return false;
	}

	//
	stub_initialize((StubBase*)&winStub, flags);

	_set_dpi_awareness();
	if (_detect_displays() == false)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "cannot detect any display");
		return false;
	}

	// 윈도우 클래스
	if (winStub.class_registered == false)
	{
		winStub.class_name = qn_apswprintf(L"qs_stub_class_%llu", qn_now());

		WNDCLASSEX wc =
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_BYTEALIGNCLIENT | CS_OWNDC,
			.lpfnWndProc = _mesg_proc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = winStub.instance,
			.hIcon = NULL,
			.hCursor = NULL,
			.hbrBackground = NULL,
			.lpszMenuName = NULL,
			.lpszClassName = winStub.class_name,
			.hIconSm = NULL,
		};

		const char* prop = qn_get_prop(QG_PROP_WINDOWS_ICON);
		if (prop && *prop)
		{
			wc.hIcon = LoadIcon(winStub.instance, MAKEINTRESOURCE(qn_strtoi(prop, 10)));
			prop = qn_get_prop(QG_PROP_WINDOWS_SMALLICON);
			if (prop && *prop)
				wc.hIconSm = LoadIcon(winStub.instance, MAKEINTRESOURCE(qn_strtoi(prop, 10)));
		}
		else
		{
			wchar module_name[260];
			GetModuleFileName(NULL, module_name, QN_COUNTOF(module_name) - 1);
			ExtractIconEx(module_name, 0, &wc.hIcon, &wc.hIconSm, 1);
		}

		if (!RegisterClassEx(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		{
			if (wc.hIcon)
				DestroyIcon(wc.hIcon);
			if (wc.hIconSm)
				DestroyIcon(wc.hIconSm);
			qn_debug_outputs(true, "WINDOWS STUB", "window class registration failed");
			return false;
		}

		winStub.class_registered = true;
	}

	// 크기 및 윈도우 위치
	const WindowsMonitor* monitor = (const WindowsMonitor*)qn_pctnr_nth(&winStub.base.monitors,
		(size_t)display < qn_pctnr_count(&winStub.base.monitors) ? display : 0);
	winStub.base.display = monitor->base.no;

	DWORD style;
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
		style = WS_POPUP;
	else
	{
		if (QN_TMASK(flags, QGFLAG_BORDERLESS))
		{
			style = WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN;
#if FALSE	// 모서리 없는데 리사이즈는 뭔가 그렇다
			if (QN_TMASK(flags, QGFLAG_RESIZABLE))
				style |= WS_THICKFRAME | WS_BORDER;
#endif
		}
		else
		{
			style = WS_SYSMENU | WS_TILED | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
			if (QN_TMASK(flags, QGFLAG_RESIZABLE))
				style |= WS_MAXIMIZEBOX | WS_THICKFRAME | WS_BORDER;
		}
	}

	const QmSize scrsize = { (int)monitor->base.width, (int)monitor->base.height };
	QmSize clientsize;
	if (width > 256 && height > 256)
		qm_set2(&clientsize, width, height);
	else
	{
		if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
			qm_set2(&clientsize, scrsize.width, scrsize.height);
		else
		{
			if (scrsize.height > 800)
				qm_set2(&clientsize, 1280, 720);
			else
				qm_set2(&clientsize, 720, 450);
		}
	}

	RECT rect;
	SetRect(&rect, 0, 0, clientsize.width, clientsize.height);
	AdjustWindowRect(&rect, style, FALSE);

	const QmSize size = { rect.right - rect.left, rect.bottom - rect.top };
	QmPoint pos = { (int)monitor->base.x, (int)monitor->base.y };
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN) == false)
	{
		pos.x += (scrsize.width - size.width) / 2;
		pos.y += (scrsize.height - size.height) / 2;
	}

	// 값설정
	qm_rect_set_pos_size(&winStub.base.window_bound, &pos, &size);
	winStub.base.client_size = clientsize;

	winStub.mouse_cursor = LoadCursor(NULL, IDC_ARROW);
	winStub.window_title = qn_u8to16_dup(title ? title : "QS", 0);
	winStub.deadzone_min = -CTRL_DEAD_ZONE;
	winStub.deadzone_max = +CTRL_DEAD_ZONE;

	// 토글키 상태 저장 (윈도우가 만들어지고 나면 메시지로 키가 들어 올텐데 혼란하기 땜에 먼저 해둠)
	QgUimKey* uk = &winStub.base.key;
	if (GetKeyState(VK_CAPITAL) & 0x1)
		QN_SMASK(&uk->mask, QIKM_CAPS, true);
	if (GetKeyState(VK_SCROLL) & 0x1)
		QN_SMASK(&uk->mask, QIKM_SCRL, true);
	if (GetKeyState(VK_NUMLOCK) & 0x1)
		QN_SMASK(&uk->mask, QIKM_NUM, true);

	//윈도우 만들기
	winStub.hwnd = CreateWindowEx(WS_EX_APPWINDOW, winStub.class_name, winStub.window_title,
		style, pos.x, pos.y, size.width, size.height,
		NULL, NULL, winStub.instance, NULL);
	if (winStub.hwnd == NULL)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "cannot create window");
		return false;
	}

	// TODO: DPI 설정

	// 표시하고
	ShowWindow(winStub.hwnd, SW_SHOWNORMAL);
	UpdateWindow(winStub.hwnd);

	// 최종 스타일 변경
	if (QN_TMASK(winStub.base.flags, QGFLAG_FULLSCREEN | QGFLAG_NOTITLE))
	{
		SetWindowLong(winStub.hwnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE);
		winStub.window_style = WS_POPUP | WS_SYSMENU | WS_VISIBLE;
	}
	else
	{
		winStub.window_style = (DWORD)GetWindowLong(winStub.hwnd, GWL_STYLE);
		SetWindowLong(winStub.hwnd, GWL_STYLE, (LONG)winStub.window_style);
	}
	stub_system_update_bound();

	return true;
}

//
void stub_system_finalize(void)
{
	if (winStub.hwnd != NULL)
	{
		winStub.base.stats |= QGSSTT_EXIT;
		_hold_mouse(false);

		if (winStub.himc != NULL && ImmAssociateContextEx != NULL)
			ImmAssociateContextEx(winStub.hwnd, winStub.himc, IACE_DEFAULT);
		if (QN_TMASK(winStub.base.flags, QGSPECIFIC_VIRTUAL) == false)
			SendMessage(winStub.hwnd, WM_CLOSE, 0, 0);
	}

	if (winStub.mouse_cursor != NULL)
		DestroyCursor(winStub.mouse_cursor);

	stub_system_disable_acs(false);

	if (winStub.class_registered)
	{
		winStub.class_registered = false;
		WNDCLASSEX wc;
		if (GetClassInfoEx(winStub.instance, winStub.class_name, &wc))
			UnregisterClass(winStub.class_name, winStub.instance);
	}

	qn_free(winStub.window_title);
	qn_free(winStub.class_name);
}

//
bool stub_system_poll(void)
{
	_Pragma("warning(suppress:28159)") const DWORD tick = GetTickCount() + 1;
	int count = 0;
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if ((int)(msg.time - tick) <= 0)
		{
			if (++count > MAX_POLL_MESGS)
				break;
		}
	}

	// 쉬프트는 프로시저 키보드 부분에서 처리 못한다. 마찬가지로 WIN키도 그런데 그건 후킹서 처리
	if (qg_test_key(QIK_LSHIFT) && (GetKeyState(VK_LSHIFT) & 0x8000) == 0)
		stub_event_on_keyboard(QIK_LSHIFT, false);
	if (qg_test_key(QIK_RSHIFT) && (GetKeyState(VK_RSHIFT) & 0x8000) == 0)
		stub_event_on_keyboard(QIK_RSHIFT, false);

	return QN_TMASK(winStub.base.stats, QGSSTT_EXIT) == false;
}

//
void stub_system_disable_acs(const bool enable)
{
	if (enable)
	{
		if (QN_TMASK(winStub.base.flags, QGFEATURE_DISABLE_ACS))
			return;
		QN_SMASK(&winStub.base.flags, QGFEATURE_DISABLE_ACS, true);

		winStub.acs_sticky.cbSize = sizeof(STICKYKEYS);
		winStub.acs_toggle.cbSize = sizeof(TOGGLEKEYS);
		winStub.acs_filter.cbSize = sizeof(FILTERKEYS);
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &winStub.acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &winStub.acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &winStub.acs_filter, 0);

		STICKYKEYS sticky = winStub.acs_sticky;
		TOGGLEKEYS toggle = winStub.acs_toggle;
		FILTERKEYS filter = winStub.acs_filter;
		sticky.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		toggle.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		filter.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &filter, 0);

		_set_key_hook(winStub.instance);
	}
	else
	{
		if (QN_TMASK(winStub.base.flags, QGFEATURE_DISABLE_ACS) == false)
			return;
		QN_SMASK(&winStub.base.flags, QGFEATURE_DISABLE_ACS, false);

		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &winStub.acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &winStub.acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &winStub.acs_filter, 0);

		_set_key_hook(NULL);
	}
}

//
void stub_system_disable_scr_save(const bool enable)
{
	if (enable)
	{
		if (QN_TMASK(winStub.base.flags, QGFEATURE_DISABLE_SCRSAVE))
			return;
		QN_SMASK(&winStub.base.flags, QGFEATURE_DISABLE_SCRSAVE, true);
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
	}
	else
	{
		if (QN_TMASK(winStub.base.flags, QGFEATURE_DISABLE_SCRSAVE) == false)
			return;
		QN_SMASK(&winStub.base.flags, QGFEATURE_DISABLE_SCRSAVE, false);
		SetThreadExecutionState(ES_CONTINUOUS);
	}
}

//
void stub_system_enable_drop(const bool enable)
{
	if (enable)
	{
		if (QN_TMASK(winStub.base.flags, QGFEATURE_ENABLE_DROP))
			return;
		QN_SMASK(&winStub.base.flags, QGFEATURE_ENABLE_DROP, true);
		DragAcceptFiles(winStub.hwnd, TRUE);
	}
	else
	{
		if (QN_TMASK(winStub.base.flags, QGFEATURE_ENABLE_DROP) == false)
			return;
		QN_SMASK(&winStub.base.flags, QGFEATURE_ENABLE_DROP, false);
		DragAcceptFiles(winStub.hwnd, FALSE);
	}
}

//
void stub_system_set_title(const char* title)
{
	qn_ret_if_fail(title && *title);
	qn_free(winStub.window_title);
	winStub.window_title = qn_u8to16_dup(title, 0);
	SetWindowText(winStub.hwnd, winStub.window_title);
}

//
void stub_system_update_bound(void)
{
	RECT rect;

	GetWindowRect(winStub.hwnd, &rect);
	memcpy(&winStub.base.window_bound, &rect, sizeof(RECT));

	GetClientRect(winStub.hwnd, &rect);
	qm_set2(&winStub.base.client_size, rect.right - rect.left, rect.bottom - rect.top);

}

//
void stub_system_focus(void)
{
	BringWindowToTop(winStub.hwnd);
	SetForegroundWindow(winStub.hwnd);
	SetFocus(winStub.hwnd);
}
#pragma endregion 시스템 함수


//////////////////////////////////////////////////////////////////////////

#pragma region 내부 정적 함수
// 키 후킹 콜백
static LRESULT CALLBACK _key_hook_callback(int code, WPARAM wp, LPARAM lp)
{
	if (code < 0 || code != HC_ACTION || QN_TMASK(winStub.base.stats, QGSSTT_ACTIVE) == false)
		return CallNextHookEx(winStub.key_hook, code, wp, lp);

	const LPKBDLLHOOKSTRUCT kh = (LPKBDLLHOOKSTRUCT)lp;
	switch (kh->vkCode)
	{
		case VK_LWIN: case VK_RWIN:
		case VK_RCONTROL: case VK_LCONTROL:
#if FALSE	// ALT+TAB, ALT+ESC 막으려면 해제
		case VK_LMENU: case VK_RMENU:
		case VK_TAB: case VK_ESCAPE:
#endif
			break;
		default:
			return CallNextHookEx(winStub.key_hook, code, wp, lp);
	}

	if (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN)
		stub_event_on_keyboard((QikKey)kh->vkCode, true);
	else
	{
		stub_event_on_keyboard((QikKey)kh->vkCode, false);
		if (kh->vkCode < 0xFF && winStub.key_hook_state[kh->vkCode])
		{
			winStub.key_hook_state[kh->vkCode] = 0;
			return CallNextHookEx(winStub.key_hook, code, wp, lp);
		}
	}
	return 1;
}

// 키 후킹 컴/끔 (instance가 널이면 끈다)
static void _set_key_hook(const HINSTANCE instance)
{
	if (winStub.key_hook != NULL)
	{
		UnhookWindowsHookEx(winStub.key_hook);
		winStub.key_hook = NULL;
	}
	if (instance == NULL)
		return;
	if (GetKeyboardState(winStub.key_hook_state) == FALSE)
		return;
	winStub.key_hook = SetWindowsHookExW(WH_KEYBOARD_LL, _key_hook_callback, instance, 0);
}

// DPI 설정
static void _set_dpi_awareness(void)
{
	if (SetProcessDpiAwarenessContext != NULL &&
		(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) ||
			SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)))
		return;
	if (SetProcessDpiAwareness != NULL &&
		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) != E_INVALIDARG)
		return;
	SetProcessDPIAware();
}

// 모니터 핸들 얻기 콜백
static BOOL CALLBACK _enum_display_callback(HMONITOR monitor, HDC dc, RECT* rect, LPARAM lp)
{
	QN_DUMMY(dc);
	QN_DUMMY(rect);
	MONITORINFOEX mi = { .cbSize = sizeof(mi) };	// NOLINT
	if (GetMonitorInfo(monitor, (LPMONITORINFO)&mi))
	{
		WindowsMonitor* wm = (WindowsMonitor*)lp;
		if (qn_wcseqv(mi.szDevice, wm->adapter))
			wm->base.oshandle = monitor;
	}
	return TRUE;
}

// 모니터 검출
static WindowsMonitor* _get_monitor_info(DISPLAY_DEVICE* adapter_device, DISPLAY_DEVICE* display_device)
{
	WindowsMonitor* mon = qn_alloc_zero_1(WindowsMonitor);
	qn_wcscpy(mon->adapter, QN_COUNTOF(mon->adapter), adapter_device->DeviceName);
	wchar* name;
	if (display_device == NULL)
		name = adapter_device->DeviceString;
	else
	{
		name = display_device->DeviceString;
		qn_wcscpy(mon->display, QN_COUNTOF(mon->display), display_device->DeviceName);
	}
	qn_u16to8(mon->base.name, QN_COUNTOF(mon->base.name), name, 0);

	DEVMODE dm;
	ZeroMemory(&dm, sizeof(DEVMODE));
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(adapter_device->DeviceName, ENUM_CURRENT_SETTINGS, &dm);

	const HDC hdc = CreateDC(L"DISPLAY", adapter_device->DeviceName, NULL, NULL);
	mon->base.mmwidth = (uint)GetDeviceCaps(hdc, HORZSIZE);
	mon->base.mmheight = (uint)GetDeviceCaps(hdc, VERTSIZE);
	DeleteDC(hdc);

	mon->base.x = dm.dmPosition.x;
	mon->base.y = dm.dmPosition.y;
	mon->base.width = (uint)dm.dmPelsWidth;
	mon->base.height = (uint)dm.dmPelsHeight;

	RECT rect;
	SetRect(&rect, dm.dmPosition.x, dm.dmPosition.y,
		(int)dm.dmPosition.x + (int)dm.dmPelsWidth, (int)dm.dmPosition.y + (int)dm.dmPelsHeight);
	EnumDisplayMonitors(NULL, &rect, _enum_display_callback, (LPARAM)mon);
	return mon;
}

// 모니터 검사
static bool _detect_displays(void)
{
	StubMonitorCtnr	keep;
	qn_pctnr_init_copy(&keep, &winStub.base.monitors);

	size_t i;
	for (DWORD adapter = 0; ; adapter++)
	{
		DISPLAY_DEVICE adapter_device = { .cb = sizeof(DISPLAY_DEVICE), };
		if (EnumDisplayDevices(NULL, adapter, &adapter_device, 0) == FALSE)
			break;
		if (QN_TMASK(adapter_device.StateFlags, DISPLAY_DEVICE_ACTIVE) == false)
			continue;

		DWORD display;
		for (display = 0; ; display++)
		{
			DISPLAY_DEVICE display_device = { .cb = sizeof(DISPLAY_DEVICE), };
			if (EnumDisplayDevices(adapter_device.DeviceName, display, &display_device, 0) == FALSE)
				break;
			if (QN_TMASK(display_device.StateFlags, DISPLAY_DEVICE_ACTIVE) == false)
				break;

			qn_pctnr_each_index(&keep, i,
				{
					const WindowsMonitor * mon = (WindowsMonitor*)qn_pctnr_nth(&keep, i);
					if (mon == NULL || qn_wcseqv(mon->display, display_device.DeviceName) == false)
						continue;
					qn_pctnr_set(&keep, i, NULL);
					EnumDisplayMonitors(NULL, NULL, _enum_display_callback, (LPARAM)&qn_pctnr_nth(&winStub.base.monitors, i));
					break;
				});
			if (i < qn_pctnr_count(&keep))
				continue;

			WindowsMonitor* mon = _get_monitor_info(&adapter_device, &display_device);
			stub_event_on_monitor((QgUdevMonitor*)mon, true, QN_TMASK(adapter_device.StateFlags, DISPLAY_DEVICE_PRIMARY_DEVICE));
		}

		if (display == 0)
		{
			qn_pctnr_each_index(&keep, i,
				{
					const WindowsMonitor * mon = (const WindowsMonitor*)qn_pctnr_nth(&keep, i);
					if (mon == NULL || qn_wcseqv(mon->adapter, adapter_device.DeviceName) == false)
						continue;
					qn_pctnr_set(&keep, i, NULL);
					break;
				});
			if (i < qn_pctnr_count(&keep))
				continue;

			WindowsMonitor* mon = _get_monitor_info(&adapter_device, NULL);
			stub_event_on_monitor((QgUdevMonitor*)mon, true, QN_TMASK(adapter_device.StateFlags, DISPLAY_DEVICE_PRIMARY_DEVICE));
		}
	}

	QgUdevMonitor* mon;
	qn_pctnr_each_item(&keep, mon,
		{
			if (mon != NULL)
				stub_event_on_monitor(mon, false, false);
		});
	qn_pctnr_disp(&keep);

	return qn_pctnr_is_have(&winStub.base.monitors);
}

//
static void _hold_mouse(const bool hold)
{
	if (hold)
	{
		if (QN_TMASK(winStub.base.stats, QGSSTT_HOLD))
			return;
		QN_SMASK(&winStub.base.stats, QGSSTT_HOLD, true);
		SetCapture(winStub.hwnd);
	}
	else
	{
		if (QN_TMASK(winStub.base.stats, QGSSTT_HOLD) == false)
			return;
		QN_SMASK(&winStub.base.stats, QGSSTT_HOLD, false);
		ReleaseCapture();
	}
}

// 마우스 이벤트 소스
static WindowsMouseSource _get_mouse_source(void)
{
	const LPARAM info = GetMessageExtraInfo();
	if (IsPenEvent(info))
	{
		if (QN_TMASK(info, 0x80))
			return WINDOWS_MOUSE_SOURCE_TOUCH;
		return WINDOWS_MOUSE_SOURCE_PEN;
	}
	return WINDOWS_MOUSE_SOURCE_MOUSE;
}

// 마으스 위치 저장
static void _set_mouse_point(const LPARAM lp, const bool save)
{
	if (winStub.mouse_lparam == lp)
		return;

	QgUimMouse* mouse = &winStub.base.mouse;
	const POINT pt = { .x = GET_X_LPARAM(lp), .y = GET_Y_LPARAM(lp) };

	// TODO: 클리핑 영역 처리

	if (save)
		mouse->last = mouse->pt;
	qm_point_set(&mouse->pt, pt.x, pt.y);
}

// 지정한 마우스 버튼 처리
static void _check_mouse_button(const bool pressed, const QimMask mask, const QimButton button)
{
	if (QN_TBIT(winStub.mouse_pending, button))
	{
		if (pressed == false)
		{
			winStub.mouse_pending &= ~QN_BIT(button);
			// 클리핑 업데이트
		}
	}

	if (pressed && QN_TBIT(mask, button) == false)
		stub_event_on_mouse_button(button, true);
	else if (pressed == false && QN_TBIT(mask, button))
		stub_event_on_mouse_button(button, false);
}

// 마우스 눌림 해제 (한번에 처리)
static void _check_mouse_release(void)
{
	const QimMask mask = winStub.base.mouse.mask;
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
		_check_mouse_button(FALSE, mask, QIM_LEFT);
	if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
		_check_mouse_button(FALSE, mask, QIM_RIGHT);
	if ((GetAsyncKeyState(VK_MBUTTON) & 0x8000) == 0)
		_check_mouse_button(FALSE, mask, QIM_MIDDLE);
	if ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) == 0)
		_check_mouse_button(FALSE, mask, QIM_X1);
	if ((GetAsyncKeyState(VK_XBUTTON2) & 0x8000) == 0)
		_check_mouse_button(FALSE, mask, QIM_X2);
	winStub.mouse_wparam = 0;
}
#pragma endregion 내부 정적 함수


//////////////////////////////////////////////////////////////////////////

#pragma region 윈도우 메시지
// 키보드 메시지
static bool _mesg_keyboard(const WPARAM wp, const bool down)
{
	const byte key = (byte)(wp & 0xFF);

#ifdef _DEBUG
	if (down && key == VK_F12)
	{
		SendMessage(winStub.hwnd, WM_CLOSE, 0, 0);
		return true;
	}
#endif

	return stub_event_on_keyboard((QikKey)key, down) == 0 ? false : key != VK_MENU;
}

// 액티브 메시지
static void _mesg_active(const bool focus)
{
	const double now = winStub.base.timer->abstime;
	const double delta = now - winStub.base.active;
	winStub.base.active = now;

	const bool has_focus = GetForegroundWindow() == winStub.hwnd;
	if (has_focus != focus)
		return;

	if (has_focus)
	{
		QN_SMASK(&winStub.base.stats, QGSSTT_ACTIVE, true);

		if (GetAsyncKeyState(VK_LBUTTON))
			winStub.mouse_pending |= QIMM_LEFT;
		if (GetAsyncKeyState(VK_RBUTTON))
			winStub.mouse_pending |= QIMM_RIGHT;
		if (GetAsyncKeyState(VK_MBUTTON))
			winStub.mouse_pending |= QIMM_MIDDLE;
		if (GetAsyncKeyState(VK_XBUTTON1))
			winStub.mouse_pending |= QIMM_X1;
		if (GetAsyncKeyState(VK_XBUTTON2))
			winStub.mouse_pending |= QIMM_X2;

		_check_mouse_release();

		stub_toggle_keys(QIKM_CAPS, (GetKeyState(VK_CAPITAL) & 0x1) != 0);
		stub_toggle_keys(QIKM_SCRL, (GetKeyState(VK_SCROLL) & 0x1) != 0);
		stub_toggle_keys(QIKM_NUM, (GetKeyState(VK_NUMLOCK) & 0x1) != 0);

		stub_event_on_window_event(QGWEV_FOCUS, 0, 0);
		stub_event_on_active(true, delta);
	}
	else
	{
		QN_SMASK(&winStub.base.stats, QGSSTT_ACTIVE, false);

		stub_event_on_reset_keys();
		stub_event_on_window_event(QGWEV_LOSTFOCUS, 0, 0);
		stub_event_on_active(false, delta);
	}
}

// 윈도우 메시지 프로시저
static LRESULT CALLBACK _mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp)
{
	LRESULT result = -1;

	if (QN_TMASK(winStub.base.flags, QGFEATURE_ENABLE_SYSWM))
	{
		QgEvent e =
		{
			.windows.ev = QGEV_SYSWM,
			.windows.hwnd = hwnd,
			.windows.mesg = mesg,
			.windows.wparam = wp,
			.windows.lparam = lp,
		};
		qg_add_event(&e, false);
	}

	// 마우스
	while (true)
	{
		if (mesg == WM_MOUSEMOVE)
		{
			_set_mouse_point(lp, true);
			stub_track_mouse_click(QIM_NONE, QIMT_MOVE);
			stub_event_on_mouse_move();
		}
		else if ((mesg >= WM_LBUTTONDOWN && mesg <= WM_MBUTTONDBLCLK) || (mesg >= WM_XBUTTONDOWN && mesg <= WM_XBUTTONDBLCLK))
		{
			if (_get_mouse_source() != WINDOWS_MOUSE_SOURCE_TOUCH)
			{
				_set_mouse_point(lp, false);
				if (winStub.mouse_wparam != wp)
				{
					winStub.mouse_wparam = wp;
					QimMask mask = winStub.base.mouse.mask;
					_check_mouse_button(QN_TMASK(wp, MK_LBUTTON), mask, QIM_LEFT);
					_check_mouse_button(QN_TMASK(wp, MK_RBUTTON), mask, QIM_RIGHT);
					_check_mouse_button(QN_TMASK(wp, MK_MBUTTON), mask, QIM_MIDDLE);
					_check_mouse_button(QN_TMASK(wp, MK_XBUTTON1), mask, QIM_X1);
					_check_mouse_button(QN_TMASK(wp, MK_XBUTTON2), mask, QIM_X2);

					_hold_mouse(winStub.base.mouse.mask != 0);
				}
			}
		}
		else if (mesg == WM_MOUSEWHEEL || mesg == WM_MOUSEHWHEEL)
		{
			int wheel = GET_WHEEL_DELTA_WPARAM(wp);
			float delta = (float)wheel / WHEEL_DELTA;
			if (mesg == WM_MOUSEWHEEL)
				stub_event_on_mouse_wheel(0.0f, delta, false);
			else
				stub_event_on_mouse_wheel(delta, 0.0f, false);
		}
		else break;
		goto pos_mesg_proc_exit;
	}

	// 키보드
	while (true)
	{
		if (mesg == WM_KEYDOWN || mesg == WM_SYSKEYDOWN)
		{
			if (!_mesg_keyboard(wp, true))
				result = 0;
		}
		else if (mesg == WM_KEYUP || mesg == WM_SYSKEYUP)
		{
			if (!_mesg_keyboard(wp, false))
				result = 0;
		}
		else break;
		goto pos_mesg_proc_exit;
	}

	// 기타 메시지
	switch (mesg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_SIZE:
		{
			const int width = LOWORD(lp);
			const int height = HIWORD(lp);
			switch (wp)
			{
				case SIZE_MAXIMIZED:
					stub_event_on_window_event(QGWEV_MAXIMIZED, width, height);
					break;
				case SIZE_MINIMIZED:
					stub_event_on_window_event(QGWEV_MINIMIZED, width, height);
					break;
				case SIZE_RESTORED:
					stub_event_on_window_event(QGWEV_RESTORED, width, height);
					break;
				default:
					break;
			}
		} break;

		case WM_ACTIVATE:
			_mesg_active(!!(LOWORD(wp)));
			break;

		case WM_SETFOCUS:
			_mesg_active(true);
			break;

		case WM_KILLFOCUS:
		case WM_ENTERIDLE:
			_mesg_active(false);
			break;

		case WM_PAINT:
		{
			RECT rt;
			if (GetUpdateRect(hwnd, &rt, FALSE))
			{
				ValidateRect(hwnd, NULL);
				stub_event_on_window_event(QGWEV_PAINTED, 0, 0);
			}
			result = 0;
		} break;

		case WM_CLOSE:
			stub_event_on_window_event(QGWEV_CLOSE, 0, 0);
			QN_SMASK(&winStub.base.stats, QGSSTT_EXIT, true);
			DestroyWindow(hwnd);
			result = 0;
			break;

		case WM_ERASEBKGND:
			if (winStub.clear_background == false)
			{
				winStub.clear_background = true;
				RECT rect;
				GetClientRect(hwnd, &rect);
				HBRUSH brush = CreateSolidBrush(0);
				FillRect(GetDC(hwnd), &rect, brush);
				DeleteObject(brush);
			}
			return 1;

		case WM_SHOWWINDOW:
			stub_event_on_window_event(wp ? QGWEV_SHOW : QGWEV_HIDE, 0, 0);
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
			if (hittest == HTCLIENT && QN_TMASK(winStub.base.stats, QGSSTT_CURSOR))
			{
				SetCursor(winStub.mouse_cursor);
				result = 1;
			}
			else if (winStub.mouse_cursor == NULL)
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
			if (QN_TMASK(winStub.base.stats, QGSSTT_LAYOUT))
				result = 0;
			break;

		case WM_WINDOWPOSCHANGED:
		{
			if (IsIconic(hwnd))
				break;

			RECT rect;
			if (!GetWindowRect(hwnd, &rect) || rect.right <= rect.left || rect.bottom <= rect.top)
				break;

			stub_event_on_window_event(QGWEV_MOVED, rect.left, rect.top);
			stub_event_on_window_event(QGWEV_SIZED, rect.right - rect.left, rect.bottom - rect.top);

			InvalidateRect(hwnd, NULL, FALSE);
		}break;

#if false
		case WM_INPUTLANGCHANGE:
			//result = 1;
			break;
#endif

		case WM_NCCALCSIZE:
			break;

		case WM_NCACTIVATE:
			_mesg_active(!!wp);
			break;

		case WM_NCHITTEST:
			if (QN_TMASK(winStub.base.flags, QGFLAG_FULLSCREEN))
				return HTCLIENT;
			break;

		case WM_DISPLAYCHANGE:
			break;

		case WM_NCCREATE:
			if (EnableNonClientDpiScaling != NULL && QN_TMASK(winStub.base.flags, QGFLAG_DPISCALE))
				EnableNonClientDpiScaling(hwnd);
			break;

		case WM_INPUT:
			break;

		case WM_CHAR:
			if (IS_HIGH_SURROGATE(wp))
				winStub.high_surrogate = (WCHAR)wp;
			else if (IS_SURROGATE_PAIR(winStub.high_surrogate, wp))
			{
				// 여기서 winStub.high_surrogate 와 wp 를 조합해서 키보드 데이터를 처리
				// https://learn.microsoft.com/ko-kr/windows/win32/inputdev/wm-char
				char u8[7];
				if (qn_u16ucb((uchar2)winStub.high_surrogate, (uchar2)wp, u8))
					stub_event_on_text(u8);
				winStub.high_surrogate = 0;
			}
			else
			{
				// 윈도우10 UTF32
				char u8[7];
				if (qn_u32ucb((uchar4)wp, u8))
					stub_event_on_text(u8);
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
					stub_event_on_text(u8);
				result = 0;
			}
			break;

		case WM_SYSCOMMAND:
		{
			ushort cmd = (ushort)(wp & 0xFFFF0);
			if (cmd == SC_KEYMENU)
				result = 0;
			if (QN_TMASK(winStub.base.flags, QGFEATURE_DISABLE_SCRSAVE) && (cmd == SC_SCREENSAVE || cmd == SC_MONITORPOWER))
				result = 0;
		} break;

		case WM_MENUCHAR:
			result = MAKELRESULT(0, MNC_CLOSE);
			break;

		case WM_CAPTURECHANGED:
			// 타이틀 안 눌림
			break;

		case WM_ENTERSIZEMOVE:
			stub_event_on_layout(true);
			break;

		case WM_EXITSIZEMOVE:
			stub_event_on_layout(false);
			break;

		case WM_DROPFILES:
			if (QN_TMASK(winStub.base.flags, QGFEATURE_ENABLE_DROP))
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
						stub_event_on_drop(filename, 0, false);
					}
					qn_free(buf);
				}
				stub_event_on_drop(NULL, 0, true);
				DragAcceptFiles(hwnd, true);
				result = 0;
			}
			break;

		case WM_TOUCH:
			break;

		case WM_POINTERUPDATE:
			winStub.mouse_lparam = lp;
			break;

		case WM_MOUSELEAVE:
			result = 0;
			break;

		case WM_DPICHANGED:
		{
			if (QN_TMASK(winStub.base.flags, QGFLAG_DPISCALE) ||
				AdjustWindowRectExForDpi != NULL)
			{
				RECT* const suggested = (RECT*)lp;
				SetWindowPos(hwnd, HWND_TOP, suggested->left, suggested->top,
					suggested->right - suggested->left, suggested->bottom - suggested->top,
					SWP_NOACTIVATE | SWP_NOZORDER);
			}
#if false
			const float xdpi = HIWORD(wp) / 96.0f;
			const float ydpi = LOWORD(wp) / 96.0f;
			// TODO: 여기에 DPI 변경 알림 이벤트 날리면 좋겠네
#endif
		} break;

		case WM_GETDPISCALEDSIZE:
		{
			if (QN_TMASK(winStub.base.flags, QGFLAG_DPISCALE) ||
				AdjustWindowRectExForDpi == NULL)
				break;
			RECT src = { 0, 0, 0, 0 };
			RECT dst = { 0, 0, 0, 0 };
			AdjustWindowRectExForDpi(&src, winStub.window_style, FALSE, WS_EX_APPWINDOW, GetDpiForWindow(hwnd));
			AdjustWindowRectExForDpi(&dst, winStub.window_style, FALSE, WS_EX_APPWINDOW, LOWORD(wp));
			SIZE* size = (SIZE*)lp;
			size->cx += (dst.right - dst.left) - (src.right - src.left);
			size->cy += (dst.bottom - dst.top) - (src.bottom - src.top);
			result = 1;
		} break;

		default:
			break;
	}

pos_mesg_proc_exit:
	if (result >= 0)
		return result;
	return CallWindowProc(DefWindowProc, hwnd, mesg, wp, lp);
}
#pragma endregion 윈도우 메시지

#endif
