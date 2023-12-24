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

//
static void windows_awareness_dpi(WindowsStub* stub);
static LRESULT CALLBACK windows_mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp);

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
	static bool dll_inited = false;
	qn_val_if_ok(dll_inited, true);
	QnModule* module;
	const char* dllname = NULL;
	static char xinput_dll[64] = "xinput1_ ";
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
	return dll_inited = true;
}

// 키 후킹
static HHOOK windows_key_hook_handle;

// 키 후킹 콜백
static LRESULT CALLBACK windows_key_hook_proc(int code, WPARAM wp, LPARAM lp)
{
	LPKBDLLHOOKSTRUCT kh = (LPKBDLLHOOKSTRUCT)lp;
	if (code == HC_ACTION &&
		(wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN || wp == WM_KEYUP || wp == WM_SYSKEYUP) &&
		(kh->vkCode == VK_LWIN || kh->vkCode == VK_RWIN || kh->vkCode == VK_MENU))
		return 1;
	return Win32CallNextHookEx(windows_key_hook_handle, code, wp, lp);
}

// 키 후킹 끔
static void windows_key_unhook(void)
{
	if (windows_key_hook_handle == NULL)
		return;
	Win32UnhookWindowsHookEx(windows_key_hook_handle);
	windows_key_hook_handle = NULL;
}

// 키 후킹 시작
static void windows_key_hook(HINSTANCE instance)
{
	windows_key_unhook();
	windows_key_hook_handle = Win32SetWindowsHookExW(WH_KEYBOARD_LL, windows_key_hook_proc, instance, 0);
}

//
StubBase* stub_system_open(const char* title, int display, int width, int height, QgFlag flags)
{
	(void)display; // 모니터 번호는 나중에

	static WindowsStub s_stub;
	WindowsStub* stub = &s_stub;

	stub->instance = GetModuleHandle(NULL);
	if (stub->instance == NULL)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "cannot retrieve module handle");
		return NULL;
	}

	if (windows_dll_init() == false)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "DLL load failed");
		return NULL;
	}
	if (Win32XInputEnable != NULL)
		Win32XInputEnable(TRUE);

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
			.lpfnWndProc = windows_mesg_proc,
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

		if (!RegisterClassEx(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		{
			if (wc.hIcon)
				DestroyIcon(wc.hIcon);
			if (wc.hIconSm)
				DestroyIcon(wc.hIconSm);
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
		qm_set2(&pos, (scrsize.width - size.width) / 2, (scrsize.height - size.height) / 2);

	stub->window_style = style;
	qm_rect_set_pos_size(&stub->base.window_bound, &pos, &size);

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

	stub->deadzone_min = -XINPUT_DEAD_ZONE;
	stub->deadzone_max = +XINPUT_DEAD_ZONE;

	return (StubBase*)stub;
}

//
bool stub_system_create_window(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
	QmRect* rect = &stub->base.window_bound;

	stub->hwnd = CreateWindowEx(0, stub->class_name, stub->window_title, stub->window_style,
		rect->left, rect->top, qm_rect_width(rect), qm_rect_height(rect),
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
	windows_awareness_dpi(stub);

	return true;
}

//
void stub_system_finalize(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	if (Win32XInputEnable != NULL)
		Win32XInputEnable(FALSE);

	if (stub->hwnd != NULL)
	{
		stub->base.stats |= QGSSTT_EXIT;
		stub_system_hold_mouse(false);

		if (stub->himc != NULL && Win32ImmAssociateContextEx != NULL)
			Win32ImmAssociateContextEx(stub->hwnd, stub->himc, IACE_DEFAULT);
		if (QN_TMASK(stub->base.flags, QGSPECIFIC_VIRTUAL) == false)
			SendMessage(stub->hwnd, WM_CLOSE, 0, 0);
	}

	if (stub->mouse_cursor != NULL)
		DestroyCursor(stub->mouse_cursor);

	stub_system_disable_acs(false);

	if (stub->class_registered)
	{
		stub->class_registered = false;
		WNDCLASSEX wc;
		if (GetClassInfoEx(stub->instance, stub->class_name, &wc))
			UnregisterClass(stub->class_name, stub->instance);
	}

	qn_free(stub->window_title);
	qn_free(stub->class_name);
}

//
bool stub_system_poll(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
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

	return QN_TMASK(stub->base.stats, QGSSTT_EXIT) == false;
}

//
void stub_system_disable_acs(bool enable)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	if (enable)
	{
		if (QN_TMASK(stub->base.flags, QGFEATURE_DISABLE_ACS))
			return;
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

		windows_key_hook(stub->instance);
	}
	else
	{
		if (QN_TMASK(stub->base.flags, QGFEATURE_DISABLE_ACS) == false)
			return;
		QN_SMASK(&stub->base.flags, QGFEATURE_DISABLE_ACS, false);

		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &stub->acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &stub->acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &stub->acs_filter, 0);

		windows_key_unhook();
	}
}

//
void stub_system_diable_scrsave(bool enable)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	if (enable)
	{
		if (QN_TMASK(stub->base.flags, QGFEATURE_DISABLE_SCRSAVE))
			return;
		QN_SMASK(&stub->base.flags, QGFEATURE_DISABLE_SCRSAVE, true);
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
	}
	else
	{
		if (QN_TMASK(stub->base.flags, QGFEATURE_DISABLE_SCRSAVE) == false)
			return;
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
		if (QN_TMASK(stub->base.flags, QGFEATURE_ENABLE_DROP))
			return;
		QN_SMASK(&stub->base.flags, QGFEATURE_ENABLE_DROP, true);
		DragAcceptFiles(stub->hwnd, TRUE);
	}
	else
	{
		if (QN_TMASK(stub->base.flags, QGFEATURE_ENABLE_DROP) == false)
			return;
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
		if (QN_TMASK(stub->base.stats, QGSSTT_HOLD))
			return;
		QN_SMASK(&stub->base.stats, QGSSTT_HOLD, true);
		SetCapture(stub->hwnd);
	}
	else
	{
		if (QN_TMASK(stub->base.stats, QGSSTT_HOLD) == false)
			return;
		QN_SMASK(&stub->base.stats, QGSSTT_HOLD, false);
		ReleaseCapture();
	}
}

//
void stub_system_calc_layout(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
	RECT rect;

	GetWindowRect(stub->hwnd, &rect);
	memcpy(&stub->base.window_bound, &rect, sizeof(RECT));

	GetClientRect(stub->hwnd, &rect);
	qm_set2(&stub->base.client_size, rect.right - rect.left, rect.bottom - rect.top);
}

//
static void windows_awareness_dpi(WindowsStub* stub)
{
	// 해야한다!
	(void)stub;
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
static void windows_set_mouse_point(WindowsStub* stub, LPARAM lp, bool save)
{
	if (stub->mouse_lparam == lp)
		return;

	QgUimMouse* mouse = &stub->base.mouse;
	POINT pt = { .x = GET_X_LPARAM(lp), .y = GET_Y_LPARAM(lp) };
	if (save)
		mouse->last = mouse->pt;
	qm_point_set(&mouse->pt, pt.x, pt.y);
}

// 지정한 마우스 버튼 처리
static void windows_check_mouse_button(WindowsStub* stub, bool pressed, QimMask mask, QimButton button)
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
		stub_event_on_mouse_button(button, true);
	else if (pressed == false && QN_TBIT(mask, button))
		stub_event_on_mouse_button(button, false);
}

// 마우스 눌림 해제 (한번에 처리)
static void windows_check_mouse_release(WindowsStub* stub)
{
	QimMask mask = stub->base.mouse.mask;
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
		windows_check_mouse_button(stub, FALSE, mask, QIM_LEFT);
	if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
		windows_check_mouse_button(stub, FALSE, mask, QIM_RIGHT);
	if ((GetAsyncKeyState(VK_MBUTTON) & 0x8000) == 0)
		windows_check_mouse_button(stub, FALSE, mask, QIM_MIDDLE);
	if ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) == 0)
		windows_check_mouse_button(stub, FALSE, mask, QIM_X1);
	if ((GetAsyncKeyState(VK_XBUTTON2) & 0x8000) == 0)
		windows_check_mouse_button(stub, FALSE, mask, QIM_X2);
	stub->mouse_wparam = 0;
}
#endif

// 키보드 메시지
static bool windows_mesg_keyboard(WindowsStub* stub, WPARAM wp, bool down)
{
	byte key = (byte)(wp & 0xFF);

#ifdef _DEBUG
	if (down && key == VK_F12)
	{
		SendMessage(stub->hwnd, WM_CLOSE, 0, 0);
		return true;
	}
#else
	(void)stub;
#endif

	return stub_event_on_keyboard((QikKey)key, down) == 0 ? false : key != VK_MENU;
}

// 액티브 메시지
static void windows_mesg_active(WindowsStub* stub, bool focus)
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

		windows_check_mouse_release(stub);

		stub_toggle_keys(QIKM_CAPS, (GetKeyState(VK_CAPITAL) & 0x1) != 0);
		stub_toggle_keys(QIKM_SCRL, (GetKeyState(VK_SCROLL) & 0x1) != 0);
		stub_toggle_keys(QIKM_NUM, (GetKeyState(VK_NUMLOCK) & 0x1) != 0);

		stub_event_on_window_event(QGWEV_FOCUS, 0, 0);
		stub_event_on_active(true, delta);
	}
	else
	{
		QN_SMASK(&stub->base.stats, QGSSTT_ACTIVE, false);

		stub_event_on_reset_keys();
		stub_event_on_window_event(QGWEV_LOSTFOCUS, 0, 0);
		stub_event_on_active(false, delta);
	}
}

// 윈도우 메시지 프로시저
static LRESULT CALLBACK windows_mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;
	LRESULT result = -1;

	if (stub == NULL)
	{
#ifdef DEBUG_WINPROC_MESG
		qn_debug_outputf(false, "WINDOW STUB", "처리안된 메시지: %s(%X)", windows_message_string(mesg), mesg);
#endif
		return CallWindowProc(DefWindowProc, hwnd, mesg, wp, lp);
	}

	if (QN_TMASK(stub->base.flags, QGFEATURE_ENABLE_SYSWM))
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
			windows_set_mouse_point(stub, lp, true);
			stub_track_mouse_click(QIM_NONE, QIMT_MOVE);
			stub_event_on_mouse_move();
		}
		else if ((mesg >= WM_LBUTTONDOWN && mesg <= WM_MBUTTONDBLCLK) || (mesg >= WM_XBUTTONDOWN && mesg <= WM_XBUTTONDBLCLK))
		{
			if (windows_get_mouse_source() != WINDOWS_MOUSE_SOURCE_TOUCH)
			{
				windows_set_mouse_point(stub, lp, false);
				if (stub->mouse_wparam != wp)
				{
					stub->mouse_wparam = wp;
					QimMask mask = stub->base.mouse.mask;
					windows_check_mouse_button(stub, QN_TMASK(wp, MK_LBUTTON), mask, QIM_LEFT);
					windows_check_mouse_button(stub, QN_TMASK(wp, MK_RBUTTON), mask, QIM_RIGHT);
					windows_check_mouse_button(stub, QN_TMASK(wp, MK_MBUTTON), mask, QIM_MIDDLE);
					windows_check_mouse_button(stub, QN_TMASK(wp, MK_XBUTTON1), mask, QIM_X1);
					windows_check_mouse_button(stub, QN_TMASK(wp, MK_XBUTTON2), mask, QIM_X2);
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
			if (!windows_mesg_keyboard(stub, wp, true))
				result = 0;
		}
		else if (mesg == WM_KEYUP || mesg == WM_SYSKEYUP)
		{
			if (!windows_mesg_keyboard(stub, wp, false))
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
			windows_mesg_active(stub, !!(LOWORD(wp)));
			break;

		case WM_SETFOCUS:
			windows_mesg_active(stub, true);
			break;

		case WM_KILLFOCUS:
		case WM_ENTERIDLE:
			windows_mesg_active(stub, false);
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
			windows_mesg_active(stub, !!wp);
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
					stub_event_on_text(u8);
				stub->high_surrogate = 0;
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
			if (QN_TMASK(stub->base.flags, QGFEATURE_DISABLE_SCRSAVE) && (cmd == SC_SCREENSAVE || cmd == SC_MONITORPOWER))
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
			stub->mouse_lparam = lp;
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
