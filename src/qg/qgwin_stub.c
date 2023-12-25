//
// qgstub_windows.c - 윈도우 스터브
// 2023-12-13 by kim
//

#include "pch.h"
#include "qs_qn.h"
#if defined _QN_WINDOWS_
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qgwin_stub.h"

#ifdef _DEBUG
#define DEBUG_WINDLL_TRACE
//#define DEBUG_WINPROC_MESG
#endif

// 윈도우 스터브 여기 있다!
WindowsStub winStub;

// DLL 선언
#define DEF_WIN_FUNC(ret,name,args)		QN_CONCAT(PFNWin32, name) QN_CONCAT(Win32, name);
#define DEF_WIN_XIFUNC(ret,name,args)	QN_CONCAT(PFNWin32, name) QN_CONCAT(Win32, name);
#include "qgwin_func.h"

// DLL 함수
static void* windows_load_func(QnModule* module, const char* dllname, const char* fnname)
{
	void* ret = qn_mod_func(module, fnname);
#ifdef DEBUG_WINDLL_TRACE
	qn_debug_outputf(false, "WINDOWS STUB", "\t%s: '%s' in '%s'",
		ret == NULL ? "load failed" : "loaded", fnname, dllname);
#else
	QN_DUMMY(dllname);
#endif
	return ret;
}

// DLL 초기화
static bool windows_dll_init(void)
{
	static bool loaded = false;
	qn_val_if_ok(loaded, true);
	QnModule* module;
	const char* dllname = NULL;
	static char xinput_dll[64] = "XINPUT1_ ";
	for (int i = 4; i >= 1; i--)
	{
		xinput_dll[8] = (char)('0' + i);
		if ((module = qn_mod_load(xinput_dll, 1)) != NULL)
		{
			dllname = xinput_dll;
			break;
		}
	}
	if (module == NULL)
	{
		// 이건 오바다
		qn_debug_outputf(true, "WINDOWS STUB", "no %s DLL found!", "XINPUT");
		return false;
	}
#define DEF_WIN_DLL_BEGIN(name)\
	module = qn_mod_load(dllname = QN_STRING(name), 1); if (module == NULL)\
	{ qn_debug_outputf(true, "WINDOWS STUB", "no %s DLL found!", QN_STRING(name)); return false; } else {
#define DEF_WIN_DLL_END }
#define DEF_WIN_FUNC(ret,name,args)\
	QN_CONCAT(Win32, name) = (QN_CONCAT(PFNWin32, name))windows_load_func(module, dllname, QN_STRING(name));
#define DEF_WIN_XIFUNC(ret,name,args)\
	QN_CONCAT(Win32, name) = (QN_CONCAT(PFNWin32, name))windows_load_func(module, dllname, QN_STRING(name));
#include "qgwin_func.h"
	return loaded = true;
}

// 키 후킹 콜백
static LRESULT CALLBACK windows_key_hook_proc(int code, WPARAM wp, LPARAM lp)
{
	LPKBDLLHOOKSTRUCT kh = (LPKBDLLHOOKSTRUCT)lp;
	if (code == HC_ACTION &&
		(wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN || wp == WM_KEYUP || wp == WM_SYSKEYUP) &&
		(kh->vkCode == VK_LWIN || kh->vkCode == VK_RWIN || kh->vkCode == VK_MENU))
		return 1;
	return Win32CallNextHookEx(winStub.key_hook, code, wp, lp);
}

// 키 후킹 끔
static void windows_key_unhook(void)
{
	if (winStub.key_hook == NULL)
		return;
	Win32UnhookWindowsHookEx(winStub.key_hook);
	winStub.key_hook = NULL;
}

// 키 후킹 시작
static void windows_key_hook(HINSTANCE instance)
{
	windows_key_unhook();
	winStub.key_hook = Win32SetWindowsHookExW(WH_KEYBOARD_LL, windows_key_hook_proc, instance, 0);
}

// DPI 설정
static void windows_set_dpi_aware(void)
{
	if (Win32SetProcessDpiAwarenessContext != NULL &&
		(Win32SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) ||
			Win32SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)))
		return;
	if (Win32SetProcessDpiAwareness != NULL &&
		Win32SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) != E_INVALIDARG)
		return;
	Win32SetProcessDPIAware();
}

//
static LRESULT CALLBACK windows_mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp);

//
bool stub_system_open(const char* title, int display, int width, int height, QgFlag flags)
{
	qn_zero_1(&winStub);
	QN_DUMMY(display);	// 모니터 번호는 나중에

	//
	if (windows_dll_init() == false)
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
	windows_set_dpi_aware();
	stub_initialize((StubBase*)&winStub, display, flags);
	stub_system_check_display();

	// 윈도우 클래스
	if (winStub.class_registered == false)
	{
		winStub.class_name = qn_apswprintf(L"qs_stub_class_%llu", qn_now());

		WNDCLASSEX wc =
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_BYTEALIGNCLIENT | CS_OWNDC,
			.lpfnWndProc = windows_mesg_proc,
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
		qm_set2(&pos, (scrsize.width - size.width) / 2, (scrsize.height - size.height) / 2);

	// 값설정
	qm_rect_set_pos_size(&winStub.base.window_bound, &pos, &size);
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
	winStub.hwnd = CreateWindowEx(0, winStub.class_name, winStub.window_title,
		style, pos.x, pos.y, size.width, size.height,
		NULL, NULL, winStub.instance, NULL);
	if (winStub.hwnd == NULL)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "cannot create window");
		return false;
	}

	ShowWindow(winStub.hwnd, SW_SHOWNORMAL);
	UpdateWindow(winStub.hwnd);

	// 최종 스타일 변경
	if (QN_TMASK(winStub.base.flags, QGFLAG_FULLSCREEN | QGFLAG_NOTITLE))
		SetWindowLong(winStub.hwnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE);
	else
	{
		style = GetWindowLong(winStub.hwnd, GWL_STYLE);
		SetWindowLong(winStub.hwnd, GWL_STYLE, style);
	}

	stub_system_calc_layout();

	return true;
}

//
void stub_system_finalize(void)
{
	if (winStub.hwnd != NULL)
	{
		winStub.base.stats |= QGSSTT_EXIT;
		stub_system_hold_mouse(false);

		if (winStub.himc != NULL && Win32ImmAssociateContextEx != NULL)
			Win32ImmAssociateContextEx(winStub.hwnd, winStub.himc, IACE_DEFAULT);
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
	_Pragma("warning(suppress:28159)")
		DWORD tick = GetTickCount() + 1;
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

	if (qg_test_key(QIK_LSHIFT) && (GetKeyState(VK_LSHIFT) & 0x8000) == 0)
		stub_event_on_keyboard(QIK_LSHIFT, false);
	if (qg_test_key(QIK_RSHIFT) && (GetKeyState(VK_RSHIFT) & 0x8000) == 0)
		stub_event_on_keyboard(QIK_RSHIFT, false);

	return QN_TMASK(winStub.base.stats, QGSSTT_EXIT) == false;
}

//
void stub_system_disable_acs(bool enable)
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

		windows_key_hook(winStub.instance);
	}
	else
	{
		if (QN_TMASK(winStub.base.flags, QGFEATURE_DISABLE_ACS) == false)
			return;
		QN_SMASK(&winStub.base.flags, QGFEATURE_DISABLE_ACS, false);

		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &winStub.acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &winStub.acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &winStub.acs_filter, 0);

		windows_key_unhook();
	}
}

//
void stub_system_diable_scrsave(bool enable)
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
void stub_system_enable_drop(bool enable)
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
void stub_system_set_title(const char* u8text)
{
	qn_ret_if_fail(u8text && *u8text);
	qn_free(winStub.window_title);
	winStub.window_title = qn_u8to16_dup(u8text, 0);
	SetWindowText(winStub.hwnd, winStub.window_title);
}

//
void stub_system_hold_mouse(bool hold)
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

//
void stub_system_calc_layout(void)
{
	RECT rect;

	GetWindowRect(winStub.hwnd, &rect);
	memcpy(&winStub.base.window_bound, &rect, sizeof(RECT));

	GetClientRect(winStub.hwnd, &rect);
	qm_set2(&winStub.base.client_size, rect.right - rect.left, rect.bottom - rect.top);
}

// 모니터 핸들 얻기 콜백
static BOOL CALLBACK windows_monitor_callback(HMONITOR monitor, HDC dc, RECT* rect, LPARAM lp)
{
	QN_DUMMY(dc);
	QN_DUMMY(rect);
	MONITORINFOEX mi = { sizeof(MONITORINFOEX), };
	if (GetMonitorInfo(monitor, (LPMONITORINFO)&mi))
	{
		WindowsMonitor* wm = (WindowsMonitor*)lp;
		if (qn_wcseqv(mi.szDevice, wm->adapter))
			wm->base.oshandle = monitor;
	}
	return TRUE;
}

// 모니터 만들기
static WindowsMonitor* windows_create_monitor(DISPLAY_DEVICE* adev, DISPLAY_DEVICE* ddev)
{
	WindowsMonitor* mon = qn_alloc_zero_1(WindowsMonitor);
	qn_wcscpy(mon->adapter, QN_COUNTOF(mon->adapter), adev->DeviceName);
	wchar* name;
	if (ddev == NULL)
		name = adev->DeviceString;
	else
	{
		name = ddev->DeviceString;
		qn_wcscpy(mon->display, QN_COUNTOF(mon->display), ddev->DeviceName);
	}
	qn_u16to8(mon->base.name, QN_COUNTOF(mon->base.name), name, 0);

	DEVMODE dm;
	ZeroMemory(&dm, sizeof(DEVMODE));
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(adev->DeviceName, ENUM_CURRENT_SETTINGS, &dm);

	HDC hdc = CreateDC(L"DISPLAY", adev->DeviceName, NULL, NULL);
	mon->base.mmwidth = GetDeviceCaps(hdc, HORZSIZE);	// (int)(dm.dmPelsWidth * 25.4f / GetDeviceCaps(hdc, LOGPIXELSX))
	mon->base.mmheight = GetDeviceCaps(hdc, VERTSIZE);	// (int)(dm.dmPelsHeight * 25.4f / GetDeviceCaps(hdc, LOGPIXELSY))
	DeleteDC(hdc);

	mon->base.x = dm.dmPosition.x;
	mon->base.y = dm.dmPosition.y;
	mon->base.width = dm.dmPelsWidth;
	mon->base.height = dm.dmPelsHeight;

	RECT rect;
	SetRect(&rect, dm.dmPosition.x, dm.dmPosition.y, dm.dmPosition.x + dm.dmPelsWidth, dm.dmPosition.y + dm.dmPelsHeight);
	EnumDisplayMonitors(NULL, &rect, windows_monitor_callback, (LPARAM)mon);
	return mon;
}

// 모니터 검사
void stub_system_check_display(void)
{
	size_t prev_count = qn_pctnr_count(&winStub.base.monitors);
	WindowsMonitor** prev_mons =
		prev_count == 0 ? NULL : qn_memdup(qn_pctnr_data(&winStub.base.monitors), prev_count * sizeof(WindowsMonitor*));

	size_t i;
	for (DWORD adapter = 0; ; adapter++)
	{
		DISPLAY_DEVICE adev = { sizeof(DISPLAY_DEVICE), };
		if (EnumDisplayDevices(NULL, adapter, &adev, 0) == FALSE)
			break;
		if (QN_TMASK(adev.StateFlags, DISPLAY_DEVICE_ACTIVE) == false)
			continue;

		DWORD display;
		for (display = 0; ; display++)
		{
			DISPLAY_DEVICE ddev = { sizeof(DISPLAY_DEVICE), };
			if (EnumDisplayDevices(adev.DeviceName, display, &ddev, 0) == FALSE)
				break;
			if (QN_TMASK(ddev.StateFlags, DISPLAY_DEVICE_ACTIVE) == false)
				break;

			for (i = 0; i < prev_count; i++)
			{
				if (prev_mons[i] == NULL || qn_wcseqv(prev_mons[i]->display, ddev.DeviceName) == false)
					continue;
				prev_mons[i] = NULL;
				EnumDisplayMonitors(NULL, NULL, windows_monitor_callback, (LPARAM)&qn_pctnr_nth(&winStub.base.monitors, i));
				break;
			}
			if (i < prev_count)
				continue;

			WindowsMonitor* mon = windows_create_monitor(&adev, &ddev);
			stub_event_on_monitor((QgUdevMonitor*)mon, true, QN_TMASK(adev.StateFlags, DISPLAY_DEVICE_PRIMARY_DEVICE));
		}

		if (display == 0)
		{
			for (i = 0; i < prev_count; i++)
			{
				if (prev_mons[i] == NULL || qn_wcseqv(prev_mons[i]->adapter, adev.DeviceName) == false)
					continue;
				prev_mons[i] = NULL;
				break;
			}
			if (i < prev_count)
				continue;

			WindowsMonitor* mon = windows_create_monitor(&adev, NULL);
			stub_event_on_monitor((QgUdevMonitor*)mon, true, QN_TMASK(adev.StateFlags, DISPLAY_DEVICE_PRIMARY_DEVICE));
		}
	}

	if (prev_mons != NULL)
	{
		for (i = 0; i < prev_count; i++)
		{
			if (prev_mons[i] != NULL)
				stub_event_on_monitor((QgUdevMonitor*)prev_mons[i], false, false);
		}
		qn_free(prev_mons);
	}
}

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

// 마우스 이벤트 소스
static WindowsMouseSource windows_get_mouse_source(void)
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

// 마으스 위치 저장
static void windows_set_mouse_point(LPARAM lp, bool save)
{
	if (winStub.mouse_lparam == lp)
		return;

	QgUimMouse* mouse = &winStub.base.mouse;
	POINT pt = { .x = GET_X_LPARAM(lp), .y = GET_Y_LPARAM(lp) };
	if (save)
		mouse->last = mouse->pt;
	qm_point_set(&mouse->pt, pt.x, pt.y);
}

// 지정한 마우스 버튼 처리
static void windows_check_mouse_button(bool pressed, QimMask mask, QimButton button)
{
	if (QN_TBIT(winStub.mouse_pending, button))
	{
		if (pressed == false)
		{
			winStub.mouse_pending &= ~QN_BIT(button);
			// 클리핑 업데이트
		}
		// 포커스 클릭 무시
	}

	if (pressed && QN_TBIT(mask, button) == false)
		stub_event_on_mouse_button(button, true);
	else if (pressed == false && QN_TBIT(mask, button))
		stub_event_on_mouse_button(button, false);
}

// 마우스 눌림 해제 (한번에 처리)
static void windows_check_mouse_release(void)
{
	QimMask mask = winStub.base.mouse.mask;
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
		windows_check_mouse_button(FALSE, mask, QIM_LEFT);
	if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
		windows_check_mouse_button(FALSE, mask, QIM_RIGHT);
	if ((GetAsyncKeyState(VK_MBUTTON) & 0x8000) == 0)
		windows_check_mouse_button(FALSE, mask, QIM_MIDDLE);
	if ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) == 0)
		windows_check_mouse_button(FALSE, mask, QIM_X1);
	if ((GetAsyncKeyState(VK_XBUTTON2) & 0x8000) == 0)
		windows_check_mouse_button(FALSE, mask, QIM_X2);
	winStub.mouse_wparam = 0;
}

// 키보드 메시지
static bool windows_mesg_keyboard(WPARAM wp, bool down)
{
	byte key = (byte)(wp & 0xFF);

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
static void windows_mesg_active(bool focus)
{
	double now = winStub.base.timer->abstime;
	double delta = now - winStub.base.active;
	winStub.base.active = now;

	bool has_focus = GetForegroundWindow() == winStub.hwnd;
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

		windows_check_mouse_release();

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
static LRESULT CALLBACK windows_mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp)
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
		qg_add_event(&e);
	}

#ifdef DEBUG_WINPROC_MESG
	qn_debug_outputf(false, "WINDOW STUB", "윈도우 메시지: %s(%X)", windows_message_string(mesg), mesg);
#endif

	// 마우스
	while (true)
	{
		if (mesg == WM_MOUSEMOVE)
		{
			windows_set_mouse_point(lp, true);
			stub_track_mouse_click(QIM_NONE, QIMT_MOVE);
			stub_event_on_mouse_move();
		}
		else if ((mesg >= WM_LBUTTONDOWN && mesg <= WM_MBUTTONDBLCLK) || (mesg >= WM_XBUTTONDOWN && mesg <= WM_XBUTTONDBLCLK))
		{
			if (windows_get_mouse_source() != WINDOWS_MOUSE_SOURCE_TOUCH)
			{
				windows_set_mouse_point(lp, false);
				if (winStub.mouse_wparam != wp)
				{
					winStub.mouse_wparam = wp;
					QimMask mask = winStub.base.mouse.mask;
					windows_check_mouse_button(QN_TMASK(wp, MK_LBUTTON), mask, QIM_LEFT);
					windows_check_mouse_button(QN_TMASK(wp, MK_RBUTTON), mask, QIM_RIGHT);
					windows_check_mouse_button(QN_TMASK(wp, MK_MBUTTON), mask, QIM_MIDDLE);
					windows_check_mouse_button(QN_TMASK(wp, MK_XBUTTON1), mask, QIM_X1);
					windows_check_mouse_button(QN_TMASK(wp, MK_XBUTTON2), mask, QIM_X2);
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
			if (!windows_mesg_keyboard(wp, true))
				result = 0;
		}
		else if (mesg == WM_KEYUP || mesg == WM_SYSKEYUP)
		{
			if (!windows_mesg_keyboard(wp, false))
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
			switch (wp)
			{
				case SIZE_MAXIMIZED:
					stub_event_on_window_event(QGWEV_MAXIMIZED, 0, 0);
					break;
				case SIZE_MINIMIZED:
					stub_event_on_window_event(QGWEV_MINIMIZED, 0, 0);
					break;
				case SIZE_RESTORED:
					stub_event_on_window_event(QGWEV_RESTORED, 0, 0);
					break;
				default:
					break;
			}
			break;

		case WM_ACTIVATE:
			windows_mesg_active(!!(LOWORD(wp)));
			break;

		case WM_SETFOCUS:
			windows_mesg_active(true);
			break;

		case WM_KILLFOCUS:
		case WM_ENTERIDLE:
			windows_mesg_active(false);
			break;

		case WM_PAINT:
		{
			RECT rt;
			if (GetUpdateRect(hwnd, &rt, FALSE))
			{
				ValidateRect(hwnd, NULL);
				stub_event_on_window_event(QGWEV_PAINTED, 0, 0);
			}
			return 0;
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

			//ClientToScreen(hwnd, (LPPOINT)&rect);		// GetClientRect일땐 이거쓰는데 지금 GetWindowRect씀
			//ClientToScreen(hwnd, (LPPOINT)&rect + 1);
			stub_event_on_window_event(QGWEV_MOVED, rect.left, rect.top);
			stub_event_on_window_event(QGWEV_SIZED, rect.right - rect.left, rect.bottom - rect.top);

			InvalidateRect(hwnd, NULL, FALSE);
		}break;

		case WM_INPUTLANGCHANGE:
			//result = 1;
			break;

		case WM_NCCALCSIZE:
			break;

		case WM_NCACTIVATE:
			windows_mesg_active(!!wp);
			break;

		case WM_NCHITTEST:
			if (QN_TMASK(winStub.base.flags, QGFLAG_FULLSCREEN))
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
			break;

		case WM_GETDPISCALEDSIZE:
			break;
	}

pos_mesg_proc_exit:
	if (result >= 0)
		return result;
	return CallWindowProc(DefWindowProc, hwnd, mesg, wp, lp);
}

#endif
