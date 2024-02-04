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
#ifdef _QN_WINDOWS_
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qg/qg_stub.h"
#include <windowsx.h>
#include <Xinput.h>
#include <shellscalingapi.h>
#include <Dbt.h>

static_assert(sizeof(RECT) == sizeof(QmRect), "RECT size not equal to QmRect");

#ifdef _DEBUG
//#define DEBUG_WIN_DLL_TRACE
#endif

#define QGSTUB_WIN_EXSTYLE	(WS_EX_LEFT | WS_EX_APPWINDOW)

#pragma region DLL 처리
// DLL 정의
#define DEF_WIN_FUNC(ret,name,args)\
	ret (WINAPI* QN_CONCAT(Win32, name)) args;	// NOLINT
#include "qgstub_win_func.h"

// DLL 함수
static void* windows_dll_func(QnModule* module, const char* func_name, const char* dll_name)
{
	void* ret = qn_mod_func(module, func_name);
#ifdef DEBUG_WIN_DLL_TRACE
	static const char* current_dll = NULL;
	if (current_dll != dll_name)
	{
		current_dll = dll_name;
		qn_mesgf(false, "WINDOWS STUB", "DLL: %s", dll_name);
	}
	qn_mesgf(false, "WINDOWS STUB", "%s: %s",
		ret == NULL ? "load failed" : "loaded", func_name);
#else
	QN_DUMMY(dll_name);
#endif
	return ret;
}

// DLL 초기화
static bool windows_dll_init(void)
{
	static bool loaded = false;
	qn_return_on_ok(loaded, true);
	QnModule* module = NULL;
	const char* dll_name = NULL;
	static char xinput_dll[64] = "XINPUT1_9";
	for (int i = 4; i >= 1; i--)
	{
		xinput_dll[8] = (char)('0' + i);
		if ((module = qn_open_mod(xinput_dll, 1)) != NULL)
		{
			dll_name = xinput_dll;
			break;
		}
	}
	if (module == NULL)
	{
		// 이건 오바다
		qn_mesgf(true, "WINDOWS STUB", "no '%s' DLL found!", "XINPUT");
		return false;
	}
#define DEF_WIN_DLL_BEGIN(name)\
	module = qn_open_mod(dll_name = (name), 1); if (module == NULL)\
	{ qn_mesgf(true, "WINDOWS STUB", "DLL load filed: %s", dll_name); return false; } else {
#define DEF_WIN_DLL_END }
#define DEF_WIN_FUNC(ret,name,args)\
	QN_CONCAT(Win32, name) = (ret(WINAPI*)args)windows_dll_func(module, QN_STRING(name), dll_name);	// NOLINT
#include "qgstub_win_func.h"
	return loaded = true;
}

// 별명
#define SetProcessDPIAware					Win32SetProcessDPIAware
#define SetProcessDpiAwarenessContext		Win32SetProcessDpiAwarenessContext
//#define SetThreadDpiAwarenessContext		Win32SetThreadDpiAwarenessContext
//#define GetThreadDpiAwarenessContext		Win32GetThreadDpiAwarenessContext
//#define GetAwarenessFromDpiAwarenessContext	Win32GetAwarenessFromDpiAwarenessContext
//#define EnableNonClientDpiScaling			Win32EnableNonClientDpiScaling
#define AdjustWindowRectExForDpi			Win32AdjustWindowRectExForDpi
#define GetDpiForWindow						Win32GetDpiForWindow
//#define AreDpiAwarenessContextsEqual		Win32AreDpiAwarenessContextsEqual
//#define IsValidDpiAwarenessContext			Win32IsValidDpiAwarenessContext
#define EnableNonClientDpiScaling			Win32EnableNonClientDpiScaling
#define CallNextHookEx						Win32CallNextHookEx
//#define GetSystemMetricsForDpi				Win32GetSystemMetricsForDpi
//#define ChangeWindowMessageFilterEx			Win32ChangeWindowMessageFilterEx
#define UnhookWindowsHookEx					Win32UnhookWindowsHookEx
#define SetWindowsHookExW					Win32SetWindowsHookExW
#define GetKeyboardState					Win32GetKeyboardState
#define GetDisplayConfigBufferSizes			Win32GetDisplayConfigBufferSizes
#define QueryDisplayConfig					Win32QueryDisplayConfig
#define DisplayConfigGetDeviceInfo			Win32DisplayConfigGetDeviceInfo

//#define GetDpiForMonitor					Win32GetDpiForMonitor
#define SetProcessDpiAwareness				Win32SetProcessDpiAwareness

#define ImmAssociateContextEx				Win32ImmAssociateContextEx
#pragma endregion


//////////////////////////////////////////////////////////////////////////

#pragma region 스터브 선언

static const GUID GUID_DEVINTERFACE_HID = { 0x4D1E55B2, 0xF16F, 0x11CF,{ 0x88,0xCB,0x00,0x11,0x11,0x00,0x00,0x30 } };
static const GUID GUID_DEVINTERFACE_MONITOR = { 0xE6F07B5F, 0xEE97, 0x4a90,{ 0xB0,0x4E,0x82,0x42,0x33,0x28,0x25,0x96 } };

// 윈도우 모니터 > QgUdevMonitor
typedef struct WINDOWSMONITOR
{
	QgUdevMonitor		base;

	HMONITOR			handle;
	wchar				adapter[32];
	wchar				display[32];

	RECT				rcMonitor;
	RECT				rcWork;
} WindowsMonitor;

// 마우스 이벤트 소스 (https://learn.microsoft.com/ko-kr/windows/win32/tablet/system-events-and-mouse-messages)
#define MI_WP_SIGNATURE		0xFF515700
#define SIGNATURE_MASK		0xFFFFFF00
#define IsPenEvent(dw)		(((dw) & SIGNATURE_MASK) == MI_WP_SIGNATURE)

// 마우스 이벤트 소스
typedef enum WINDOWSMOUSESOURCE
{
	WINDOWS_MOUSE_SOURCE_MOUSE,
	WINDOWS_MOUSE_SOURCE_TOUCH,
	WINDOWS_MOUSE_SOURCE_PEN,
} WindowsMouseSource;

// 윈도우 스터브 > StubBase
typedef struct WINDOWSSTUB
{
	StubBase			base;

	HINSTANCE			instance;
	HWND				hwnd;
	HDC					hdc;

	wchar				class_name[64];
	DWORD				window_style;

	RECT				window_bound;

	STICKYKEYS			acs_sticky;
	TOGGLEKEYS			acs_toggle;
	FILTERKEYS			acs_filter;
	HHOOK				key_hook;
	BYTE				key_hook_state[256];

	HIMC				himc;
	int					imcs;
	int					high_surrogate;

	HDEVNOTIFY			hdn_controller;
	int					deadzone_min;
	int					deadzone_max;

	HCURSOR				mouse_cursor;
	WPARAM				mouse_wparam;
	LPARAM				mouse_lparam;
	QimMask				mouse_pending;

	bool				class_registered;
	bool				clear_background;
	bool				enter_movesize;
	bool				mouse_tracked;
} WindowsStub;

// 윈도우 스터브 여기 있다!
static WindowsStub wStub;

//
static void windows_rect_adjust(QmSize* size, int width, int height, UINT dpi);
static void windows_rect_aspect(RECT* rect, int edge);
static void windows_set_key_hook(HINSTANCE instance);
static void windows_set_dpi_awareness(void);
static bool windows_detect_displays(void);
static void windows_hold_mouse(bool hold);
static LRESULT CALLBACK windows_mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp);
#pragma endregion 스터브 선언

#pragma region 시스템 함수
//
bool stub_system_open(const char* title, int display, int width, int height, QgFlag flags, QgFeature features)
{
	QN_DUMMY(features);	// 다른데서는 쓸 가능성이 있다. (웨이랜드라든가 웨이이놈이라던가)
	qn_zero_1(&wStub);

	//
	if (windows_dll_init() == false)
	{
		qn_mesg(true, "WINDOWS STUB", "DLL load failed");
		return false;
	}

	wStub.instance = GetModuleHandle(NULL);
	if (wStub.instance == NULL)
	{
		qn_mesg(true, "WINDOWS STUB", "cannot retrieve module handle");
		return false;
	}

	//
	stub_initialize((StubBase*)&wStub, flags);

	windows_set_dpi_awareness();
	if (windows_detect_displays() == false)
	{
		qn_mesg(true, "WINDOWS STUB", "cannot detect any display");
		return false;
	}

	// 윈도우 클래스
	if (wStub.class_registered == false)
	{
		char class_name[64];
		qn_snprintf(class_name, QN_COUNTOF(class_name), "qs_stub_%llu", qn_now());
		qn_u8to16(wStub.class_name, QN_COUNTOF(wStub.class_name), class_name, 0);

		WNDCLASSEX wc =
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_BYTEALIGNCLIENT | CS_OWNDC,
			.lpfnWndProc = windows_mesg_proc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = wStub.instance,
			.hIcon = NULL,
			.hCursor = NULL,
			.hbrBackground = NULL,
			.lpszMenuName = NULL,
			.lpszClassName = wStub.class_name,
			.hIconSm = NULL,
		};

		const char* prop = qn_get_prop(QG_PROP_WINDOWS_ICON);
		if (prop && *prop)
		{
			wc.hIcon = LoadIcon(wStub.instance, MAKEINTRESOURCE(qn_strtoi(prop, 10)));
			prop = qn_get_prop(QG_PROP_WINDOWS_SMALLICON);
			if (prop && *prop)
				wc.hIconSm = LoadIcon(wStub.instance, MAKEINTRESOURCE(qn_strtoi(prop, 10)));
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
			qn_mesg(true, "WINDOWS STUB", "window class registration failed");
			return false;
		}

		wStub.class_registered = true;
	}

	// 사용할 모니터 (모니터 번호는 정렬되어 있어서 그냥 순번 ㅇㅋ)
	const WindowsMonitor* mon = (const WindowsMonitor*)qn_pctnr_nth(&wStub.base.monitors,
		(size_t)display < qn_pctnr_count(&wStub.base.monitors) ? display : 0);
	wStub.base.display = mon->base.no;

	// 크기와 위치 (윈도우 크기, 화면 크기)
	QmSize client_size;
	if (width > 128 && height > 128)
		client_size = qm_size(width, height);
	else
	{
		// 세로 ÷ 가로 = 0.5625
		if (mon->base.height > 800)
			client_size = qm_size(1280, 720);
		else if (mon->base.height > 600)
			client_size = qm_size(960, 540);
		else
			client_size = qm_size(720, 405);
	}

	// 여기서는 윈도우용 스타일만 만듦. 풀스크린은 아래서 값 다시 설정
	DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;
	if (QN_TMASK(flags, QGFLAG_MAXIMIZE))
		style |= WS_MAXIMIZE;
	if (QN_TMASK(wStub.base.flags, QGFLAG_NOTITLE))
	{
		style |= WS_POPUP;
		if (QN_TMASK(wStub.base.flags, QGFLAG_RESIZE))
			style |= WS_SIZEBOX;
		if (QN_TMASK(wStub.base.flags, QGFLAG_BORDERLESS) == false)
			style |= WS_BORDER;
	}
	else
	{
		style |= WS_SYSMENU | WS_TILED | WS_CAPTION | WS_MINIMIZEBOX;
		if (QN_TMASK(wStub.base.flags, QGFLAG_BORDERLESS))
			style |= WS_POPUP;
		if (QN_TMASK(wStub.base.flags, QGFLAG_RESIZE))
			style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
	}

	RECT rect;
	SetRect(&rect, 0, 0, client_size.Width, client_size.Height);
	AdjustWindowRectEx(&rect, style, FALSE, QGSTUB_WIN_EXSTYLE);

	QmSize size = qm_size(rect.right - rect.left, rect.bottom - rect.top);
	QmPoint pos = qm_point(
		(int)(mon->base.x + (mon->base.width - size.Width) / 2),
		(int)(mon->base.y + (mon->base.height - size.Height) / 2));

	// 값설정 (바운드는 stub_system_update_bound()에서 하므로 여기서 안해도 됨)
	wStub.base.client_size = client_size;
	wStub.mouse_cursor = LoadCursor(NULL, IDC_ARROW);
	wStub.deadzone_min = -CTRL_DEAD_ZONE;
	wStub.deadzone_max = +CTRL_DEAD_ZONE;

	// 풀스크린으로 변신!
	wStub.window_style = style;
	SetRect(&wStub.window_bound, pos.X, pos.Y, pos.X + size.Width, pos.Y + size.Height);

	if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
	{
		QN_SMASK(wStub.base.stats, QGSST_FULLSCREEN, true);
		style &= ~WS_OVERLAPPEDWINDOW;
		style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
		pos = qm_point(mon->rcMonitor.left, mon->rcMonitor.top);
		size = qm_size(mon->rcMonitor.right - mon->rcMonitor.left, mon->rcMonitor.bottom - mon->rcMonitor.top);
	}

	// 토글키 상태 저장 (윈도우가 만들어지고 나면 메시지로 키가 들어 올텐데 혼란하기 땜에 먼저 해둠)
	QgUimKey* uk = &wStub.base.key;
	if (GetKeyState(VK_CAPITAL) & 0x1)
		QN_SMASK(uk->mask, QIKM_CAPS, true);
	if (GetKeyState(VK_SCROLL) & 0x1)
		QN_SMASK(uk->mask, QIKM_SCRL, true);
	if (GetKeyState(VK_NUMLOCK) & 0x1)
		QN_SMASK(uk->mask, QIKM_NUM, true);

	//윈도우 만들기
	wchar* wtitle = qn_u8to16_dup(title ? title : "QS", 0);
	wStub.hwnd = CreateWindow(wStub.class_name, wtitle,
		style, pos.X, pos.Y, size.Width, size.Height,
		NULL, NULL, wStub.instance, NULL);
	qn_free(wtitle);
	if (wStub.hwnd == NULL)
	{
		qn_mesg(true, "WINDOWS STUB", "cannot create window");
		return false;
	}
	wStub.hdc = GetDC(wStub.hwnd);

	// 최종 스타일 변경
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN) == false)
	{
		SetRect(&rect, 0, 0, client_size.Width, client_size.Height);
		if (AdjustWindowRectExForDpi)
			AdjustWindowRectExForDpi(&rect, style, FALSE, QGSTUB_WIN_EXSTYLE, GetDpiForWindow(wStub.hwnd));
		else
			AdjustWindowRectEx(&rect, style, FALSE, QGSTUB_WIN_EXSTYLE);

		WINDOWPLACEMENT wp = { .length = sizeof(WINDOWPLACEMENT) };
		GetWindowPlacement(wStub.hwnd, &wp);
		OffsetRect(&rect, wp.rcNormalPosition.left - rect.left, wp.rcNormalPosition.top - rect.top);
		wp.rcNormalPosition = rect;
		wp.showCmd = SW_HIDE;
		SetWindowPlacement(wStub.hwnd, &wp);

		if (QN_TMASK(flags, QGFLAG_MAXIMIZE) && QN_TMASK(flags, QGFLAG_NOTITLE))
		{
			SetWindowPos(wStub.hwnd, HWND_TOP, mon->rcWork.left, mon->rcWork.top,
				mon->rcWork.right - mon->rcWork.left, mon->rcWork.bottom - mon->rcWork.top,
				SWP_NOACTIVATE | SWP_NOZORDER);
		}

		wStub.window_style = (DWORD)GetWindowLong(wStub.hwnd, GWL_STYLE);
	}

	// 컨트롤러 연결 알림
	DEV_BROADCAST_DEVICEINTERFACE filter =
	{
		.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE),
		.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE,
		.dbcc_classguid = GUID_DEVINTERFACE_HID,
	};
	wStub.hdn_controller = RegisterDeviceNotification(wStub.hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	// 렌더러가 없다면 표시하고, 있으면 렌더러 만들고 stub_system_actuate()를 호출할 것
	if (QN_TMASK(features, 0xFF000000) == false)
		stub_system_actuate();

	// ㅇㅋ
	stub_system_update_bound();

	return true;
}

//
void stub_system_finalize(void)
{
	if (QN_TMASK(wStub.base.features, QGFEATURE_RELATIVE_MOUSE))
		stub_system_relative_mouse(false);
	if (QN_TMASK(wStub.base.features, QGFEATURE_DISABLE_SCRSAVE))
		stub_system_disable_scr_save(false);
	if (QN_TMASK(wStub.base.features, QGFEATURE_DISABLE_ACS))
		stub_system_disable_acs(false);

	if (wStub.hdn_controller != NULL)
		UnregisterDeviceNotification(wStub.hdn_controller);

	if (wStub.hwnd != NULL)
	{
		wStub.base.stats |= QGSST_EXIT;
		windows_hold_mouse(false);

		if (wStub.himc != NULL && ImmAssociateContextEx != NULL)
			ImmAssociateContextEx(wStub.hwnd, wStub.himc, IACE_DEFAULT);
		if (QN_TMASK(wStub.base.flags, QGSPECIFIC_VIRTUAL) == false)
			SendMessage(wStub.hwnd, WM_CLOSE, 0, 0);
	}

	if (wStub.mouse_cursor != NULL)
		DestroyCursor(wStub.mouse_cursor);

	if (wStub.class_registered)
	{
		wStub.class_registered = false;
		UnregisterClass(wStub.class_name, wStub.instance);
	}
}

//
void stub_system_actuate(void)
{
	ShowWindow(wStub.hwnd, SW_SHOWNORMAL);
	stub_system_focus();

	if (QN_TMASK(wStub.base.flags, QGFLAG_FULLSCREEN) && QN_TMASK(wStub.base.stats, QGSST_FULLSCREEN) == false)
	{
		QN_SMASK(wStub.base.stats, QGSST_FULLSCREEN, true);
		stub_system_fullscreen(true);
	}
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
			if (++count > MAX_POLL_LENGTH)
				break;
		}
	}

#if false
	// 쉬프트는 프로시저 키보드 부분에서 처리 못한다. 마찬가지로 WIN키도 그런데 그건 후킹서 처리
	// 처리되는데? 응?
	if (qg_get_key_state(QIK_LSHIFT) && (GetKeyState(VK_LSHIFT) & 0x8000) == 0)
		stub_event_on_keyboard(QIK_LSHIFT, false);
	if (qg_get_key_state(QIK_RSHIFT) && (GetKeyState(VK_RSHIFT) & 0x8000) == 0)
		stub_event_on_keyboard(QIK_RSHIFT, false);
#endif

	return QN_TMASK(wStub.base.stats, QGSST_EXIT) == false;
}

//
bool stub_system_disable_acs(const bool enable)
{
	if (enable)
	{
		wStub.acs_sticky.cbSize = sizeof(STICKYKEYS);
		wStub.acs_toggle.cbSize = sizeof(TOGGLEKEYS);
		wStub.acs_filter.cbSize = sizeof(FILTERKEYS);
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &wStub.acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &wStub.acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &wStub.acs_filter, 0);

		STICKYKEYS sticky = wStub.acs_sticky;
		TOGGLEKEYS toggle = wStub.acs_toggle;
		FILTERKEYS filter = wStub.acs_filter;
		sticky.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		toggle.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		filter.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &filter, 0);

		windows_set_key_hook(wStub.instance);
	}
	else
	{
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &wStub.acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &wStub.acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &wStub.acs_filter, 0);

		windows_set_key_hook(NULL);
	}
	return true;
}

//
bool stub_system_disable_scr_save(const bool enable)
{
	if (enable)
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
	else
		SetThreadExecutionState(ES_CONTINUOUS);
	return true;
}

//
bool stub_system_enable_drop(const bool enable)
{
	if (enable)
		DragAcceptFiles(wStub.hwnd, TRUE);
	else
		DragAcceptFiles(wStub.hwnd, FALSE);
	return true;
}

//
bool stub_system_relative_mouse(bool enable)
{
	if (enable)
	{
		RECT rt;
		GetClientRect(wStub.hwnd, &rt);
		ClientToScreen(wStub.hwnd, ((LPPOINT)&rt) + 0);
		ClientToScreen(wStub.hwnd, ((LPPOINT)&rt) + 1);
		ClipCursor(&rt);
	}
	else
	{
		ClipCursor(NULL);
	}
	return true;
}

//
void stub_system_set_title(const char* title)
{
	qn_return_when_fail(title && *title, /*void*/);
	wchar* wtitle = qn_u8to16_dup(title, 0);
	SetWindowText(wStub.hwnd, wtitle);
	qn_free(wtitle);
}

//
void stub_system_update_bound(void)
{
	RECT rect;

	GetWindowRect(wStub.hwnd, &rect);
	wStub.base.bound = qm_rect_RECT(rect);

	GetClientRect(wStub.hwnd, &rect);
	wStub.base.client_size = qm_size(rect.right - rect.left, rect.bottom - rect.top);
}

//
void stub_system_focus(void)
{
	BringWindowToTop(wStub.hwnd);
	SetForegroundWindow(wStub.hwnd);
	SetFocus(wStub.hwnd);
}

//
void stub_system_aspect(void)
{
	RECT rect;
	GetWindowRect(wStub.hwnd, &rect);
	windows_rect_aspect(&rect, WMSZ_BOTTOMRIGHT);
	MoveWindow(wStub.hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

//
void stub_system_fullscreen(bool fullscreen)
{
	DWORD style = GetWindowLong(wStub.hwnd, GWL_STYLE);

	if (fullscreen)
	{
		wStub.window_bound = qm_rect_to_RECT(wStub.base.bound);

		style &= ~WS_OVERLAPPEDWINDOW;
		style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
		SetWindowLong(wStub.hwnd, GWL_STYLE, (LONG)style);

		UINT uflags = SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS;
		if (QN_TMASK(wStub.base.flags, QGFLAG_NOTITLE) == false)
			uflags |= SWP_FRAMECHANGED;

		const WindowsMonitor* mon = (const WindowsMonitor*)qg_get_current_monitor();
		SetWindowPos(wStub.hwnd, HWND_TOP, mon->rcMonitor.left, mon->rcMonitor.top,
			mon->rcMonitor.right - mon->rcMonitor.left, mon->rcMonitor.bottom - mon->rcMonitor.top, uflags);
	}
	else
	{
		style &= ~WS_POPUP;
		style |= wStub.window_style;
		SetWindowLong(wStub.hwnd, GWL_STYLE, (LONG)style);

		UINT uflags = SWP_NOACTIVATE | SWP_NOCOPYBITS;
		if (QN_TMASK(wStub.base.features, QGFLAG_NOTITLE) == false)
			uflags |= SWP_FRAMECHANGED;

		const RECT rect = wStub.window_bound;
		SetWindowPos(wStub.hwnd, HWND_NOTOPMOST, rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top, uflags);
	}
}

//
void* stub_system_get_window(void)
{
	return (void*)wStub.hwnd;
}

//
void* stub_system_get_display(void)
{
	return (void*)wStub.hdc;
}

//
void* stub_system_get_instance(void)
{
	return (void*)wStub.instance;
}

//
const wchar* stub_system_get_class_name(void)
{
	return wStub.class_name;
}
#pragma endregion 시스템 함수


//////////////////////////////////////////////////////////////////////////

#pragma region 내부 정적 함수
// 크기 조정
static void windows_rect_adjust(QmSize* size, int width, int height, UINT dpi)
{
	RECT rect;
	SetRect(&rect, 0, 0, width, height);	// NOLINT
	if (AdjustWindowRectExForDpi)
		AdjustWindowRectExForDpi(&rect, wStub.window_style, FALSE, QGSTUB_WIN_EXSTYLE, dpi);
	else
		AdjustWindowRectEx(&rect, wStub.window_style, FALSE, QGSTUB_WIN_EXSTYLE);
	size->Width = rect.right - rect.left;
	size->Height = rect.bottom - rect.top;
}

// 종횡비 조정
static void windows_rect_aspect(RECT* rect, int edge)
{
	QmSize offset;
	windows_rect_adjust(&offset, 0, 0,
		GetDpiForWindow ? GetDpiForWindow(wStub.hwnd) : USER_DEFAULT_SCREEN_DPI);
	if (edge == WMSZ_LEFT || edge == WMSZ_BOTTOMLEFT || edge == WMSZ_RIGHT || edge == WMSZ_BOTTOMRIGHT)
		rect->bottom = rect->top + offset.Height + (int)((float)(rect->right - rect->left - offset.Width) * wStub.base.aspect);
	else if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
		rect->top = rect->bottom - offset.Height - (int)((float)(rect->right - rect->left - offset.Width) * wStub.base.aspect);
	else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM)
		rect->right = rect->left + offset.Height + (int)((float)(rect->bottom - rect->top - offset.Width) / wStub.base.aspect);
}

// 키 후킹 콜백
static LRESULT CALLBACK windows_key_hook_callback(int code, WPARAM wp, LPARAM lp)
{
	if (code < 0 || code != HC_ACTION || QN_TMASK(wStub.base.stats, QGSST_ACTIVE) == false)
		return CallNextHookEx(wStub.key_hook, code, wp, lp);

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
			return CallNextHookEx(wStub.key_hook, code, wp, lp);
	}

	if (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN)
		stub_event_on_keyboard((QikKey)kh->vkCode, true);
	else
	{
		stub_event_on_keyboard((QikKey)kh->vkCode, false);
		if (kh->vkCode < 0xFF && wStub.key_hook_state[kh->vkCode])
		{
			wStub.key_hook_state[kh->vkCode] = 0;
			return CallNextHookEx(wStub.key_hook, code, wp, lp);
		}
	}
	return 1;
}

// 키 후킹 컴/끔 (instance가 널이면 끈다)
static void windows_set_key_hook(const HINSTANCE instance)
{
	if (wStub.key_hook != NULL)
	{
		UnhookWindowsHookEx(wStub.key_hook);
		wStub.key_hook = NULL;
	}
	if (instance == NULL)
		return;
	if (GetKeyboardState(wStub.key_hook_state) == FALSE)
		return;
	wStub.key_hook = SetWindowsHookExW(WH_KEYBOARD_LL, windows_key_hook_callback, instance, 0);
}

// DPI 설정
static void windows_set_dpi_awareness(void)
{
	if (SetProcessDpiAwarenessContext != NULL &&
		(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) ||
			SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)))
		return;
	if (SetProcessDpiAwareness != NULL &&
		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) != E_INVALIDARG)
		return;
	if (SetProcessDPIAware != NULL)
		SetProcessDPIAware();
}

// 친숙한 모니터 이름 
typedef struct WindowsFriendlyMonitor
{
	WCHAR				deviceName[32];
	char				friendlyName[64];
} WindowsFriendlyMonitor;

// 모니터 이름 얻기
static UINT32 windows_friendly_monitors(WindowsFriendlyMonitor** monitors)
{
	// https://learn.microsoft.com/ko-kr/windows/win32/api/winuser/nf-winuser-querydisplayconfig
	// 윈도우 7 이상만. 요즘 이게 안되면 이상하지
	if (GetDisplayConfigBufferSizes == NULL ||
		QueryDisplayConfig == NULL ||
		DisplayConfigGetDeviceInfo == NULL)
	{
		*monitors = NULL;
		return 0;
	}

	WindowsFriendlyMonitor* fms = NULL;
	LONG result, tries = 0;
	UINT32 path_count, mode_count;
	DISPLAYCONFIG_PATH_INFO* path_infos = NULL;
	DISPLAYCONFIG_MODE_INFO* mode_infos = NULL;

	do
	{
		if (tries++ > 5)
			goto pos_error;
		result = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &path_count, &mode_count);
		if (result != ERROR_SUCCESS)
			goto pos_error;
		path_infos = qn_realloc(path_infos, path_count, DISPLAYCONFIG_PATH_INFO);
		mode_infos = qn_realloc(mode_infos, mode_count, DISPLAYCONFIG_MODE_INFO);
		result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &path_count, path_infos, &mode_count, mode_infos, NULL);
	} while (result == ERROR_INSUFFICIENT_BUFFER);

	if (result != ERROR_SUCCESS)
		goto pos_error;

	fms = qn_alloc_zero(path_count, WindowsFriendlyMonitor);

	for (UINT32 i = 0; i < path_count; i++)
	{
		DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName =
		{
			.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME,
			.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME),
			.header.adapterId = path_infos[i].targetInfo.adapterId,
			.header.id = path_infos[i].sourceInfo.id,
		};
		result = DisplayConfigGetDeviceInfo(&sourceName.header);
		if (result != ERROR_SUCCESS)
			continue;

		DISPLAYCONFIG_TARGET_DEVICE_NAME targetName =
		{
			.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME,
			.header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME),
			.header.adapterId = path_infos[i].targetInfo.adapterId,
			.header.id = path_infos[i].targetInfo.id,
		};
		result = DisplayConfigGetDeviceInfo(&targetName.header);
		if (result != ERROR_SUCCESS)
			continue;

		wcscpy_s(fms[i].deviceName, QN_COUNTOF(fms[i].deviceName), sourceName.viewGdiDeviceName);
		qn_u16to8(fms[i].friendlyName, QN_COUNTOF(fms[i].friendlyName) - 1, targetName.monitorFriendlyDeviceName, 0);
	}

	*monitors = fms;
	goto pos_success;

pos_error:
	qn_free(fms);
	*monitors = NULL;
	path_count = 0;

pos_success:
	qn_free(path_infos);
	qn_free(mode_infos);
	return path_count;
}

// 모니터 핸들 얻기 콜백
static BOOL CALLBACK windows_enum_display_callback(HMONITOR monitor, HDC dc, RECT* rect, LPARAM lp)
{
	QN_DUMMY(dc);
	QN_DUMMY(rect);
	MONITORINFOEX mi;
	qn_zero_1(&mi);
	((MONITORINFO*)&mi)->cbSize = sizeof(MONITORINFOEX);
	if (GetMonitorInfo(monitor, (LPMONITORINFO)&mi))
	{
		WindowsMonitor* wm = (WindowsMonitor*)lp;
		if (wcscmp(mi.szDevice, wm->adapter) == 0)
		{
			wm->handle = monitor;
			wm->rcMonitor = ((MONITORINFO*)&mi)->rcMonitor;
			wm->rcWork = ((MONITORINFO*)&mi)->rcWork;
		}
	}
	return TRUE;
}

// 모니터 검출
static WindowsMonitor* windows_get_monitor_info(DISPLAY_DEVICE* adapter_device, DISPLAY_DEVICE* display_device,
	WindowsFriendlyMonitor* friendly_monitors, UINT32 friendly_count)
{
	WindowsMonitor* mon = qn_alloc_zero_1(WindowsMonitor);
	wcsncpy_s(mon->adapter, QN_COUNTOF(mon->adapter), adapter_device->DeviceName, QN_COUNTOF(mon->adapter) - 1);

	for (UINT i = 0; i < friendly_count; i++)
	{
		const WindowsFriendlyMonitor* name = &friendly_monitors[i];
		if (wcscmp(name->deviceName, adapter_device->DeviceName) != 0)
			continue;
		if (*name->friendlyName)
			qn_strcpy(mon->base.name, name->friendlyName);
		break;
	}
	if (*mon->base.name == '\0')
	{
		wchar* name;
		if (display_device == NULL)
			name = adapter_device->DeviceString;
		else
		{
			name = display_device->DeviceString;
			wcsncpy_s(mon->display, QN_COUNTOF(mon->display), display_device->DeviceName, QN_COUNTOF(mon->display) - 1);
		}
		qn_u16to8(mon->base.name, QN_COUNTOF(mon->base.name), name, 0);
	}

	DEVMODE dm = { .dmSize = sizeof(DEVMODE) };
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
	EnumDisplayMonitors(NULL, &rect, windows_enum_display_callback, (LPARAM)mon);
	return mon;
}

// 모니터 검사
static bool windows_detect_displays(void)
{
	StubMonitorCtnr	keep;
	qn_pctnr_init_copy(&keep, &wStub.base.monitors);

	WindowsFriendlyMonitor* friendly_monitors;
	const UINT32 friendly_count = windows_friendly_monitors(&friendly_monitors);

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

			qn_pctnr_foreach(&keep, i)
			{
				const WindowsMonitor * mon = (WindowsMonitor*)qn_pctnr_nth(&keep, i);
				if (mon == NULL || wcscmp(mon->display, display_device.DeviceName) != 0)
					continue;
				qn_pctnr_set(&keep, i, NULL);
				EnumDisplayMonitors(NULL, NULL, windows_enum_display_callback, (LPARAM)&qn_pctnr_nth(&wStub.base.monitors, i));
				break;
			}
			if (i < qn_pctnr_count(&keep))
				continue;

			WindowsMonitor* mon = windows_get_monitor_info(&adapter_device, &display_device, friendly_monitors, friendly_count);
			stub_event_on_monitor((QgUdevMonitor*)mon, true,
				QN_TMASK(adapter_device.StateFlags, DISPLAY_DEVICE_PRIMARY_DEVICE), false);
		}

		if (display == 0)
		{
			qn_pctnr_foreach(&keep, i)
			{
				const WindowsMonitor* mon = (const WindowsMonitor*)qn_pctnr_nth(&keep, i);
				if (mon == NULL || wcscmp(mon->adapter, adapter_device.DeviceName) != 0)
					continue;
				qn_pctnr_set(&keep, i, NULL);
				break;
			}
			if (i < qn_pctnr_count(&keep))
				continue;

			WindowsMonitor* mon = windows_get_monitor_info(&adapter_device, NULL, friendly_monitors, friendly_count);
			stub_event_on_monitor((QgUdevMonitor*)mon, true,
				QN_TMASK(adapter_device.StateFlags, DISPLAY_DEVICE_PRIMARY_DEVICE), false);
		}
	}

	qn_pctnr_foreach(&keep, i)
	{
		QgUdevMonitor* mon = qn_pctnr_nth(&keep, i);
		if (mon != NULL)
			stub_event_on_monitor(mon, false, false, false);
	}
	qn_pctnr_disp(&keep);

	qn_free(friendly_monitors);

	return qn_pctnr_is_have(&wStub.base.monitors);
}

//
static void windows_hold_mouse(const bool hold)
{
	if (hold)
	{
		if (QN_TMASK(wStub.base.stats, QGSST_HOLD))
			return;
		QN_SMASK(wStub.base.stats, QGSST_HOLD, true);
		SetCapture(wStub.hwnd);
	}
	else
	{
		if (QN_TMASK(wStub.base.stats, QGSST_HOLD) == false)
			return;
		QN_SMASK(wStub.base.stats, QGSST_HOLD, false);
		ReleaseCapture();
	}
}

// 마우스 이벤트 소스
static WindowsMouseSource windows_get_mouse_source(void)
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

// 지정한 마우스 버튼 처리
static void windows_check_mouse_button(const bool pressed, const QimMask mask, const QimButton button)
{
	if (QN_TBIT(wStub.mouse_pending, button))
	{
		if (pressed == false)
		{
			wStub.mouse_pending &= ~QN_BIT(button);
			// 클리핑 업데이트
		}
	}

	if (pressed && QN_TBIT(mask, button) == false)
		stub_event_on_mouse_button(button, true);
	else if (pressed == false && QN_TBIT(mask, button))
		stub_event_on_mouse_button(button, false);
}

// 마우스 눌림 해제 (한번에 처리)
static void windows_check_mouse_release(void)
{
	const QimMask mask = wStub.base.mouse.mask;
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
	wStub.mouse_wparam = 0;
}
#pragma endregion 내부 정적 함수


//////////////////////////////////////////////////////////////////////////

#pragma region 윈도우 메시지
// 키보드 메시지
static bool windows_mesg_keyboard(const WPARAM wp, const LPARAM lp, const bool down)
{
	byte key = (byte)(wp & 0xFF);

#ifdef _DEBUG
	if (down && key == VK_F12)
	{
		SendMessage(wStub.hwnd, WM_CLOSE, 0, 0);
		return true;
	}
#endif

	switch (key)
	{
		case VK_SHIFT:
			key = (byte)MapVirtualKey((lp & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
			break;
		case VK_CONTROL:
			key = (HIWORD(lp) & KF_EXTENDED) ? VK_RCONTROL : VK_LCONTROL;
			break;
		case VK_MENU:
			key = (HIWORD(lp) & KF_EXTENDED) ? VK_RMENU : VK_LMENU;
			break;
		default:
			break;
	}

	return stub_event_on_keyboard((QikKey)key, down) == 0 ? false : key != VK_MENU;
}

// 액티브 메시지
static void windows_mesg_active(const bool focus)
{
	const double now = wStub.base.timer->abstime;
	const double delta = now - wStub.base.active;
	wStub.base.active = now;

	const bool has_focus = GetForegroundWindow() == wStub.hwnd;
	if (has_focus != focus)
		return;

	if (has_focus)
	{
		QN_SMASK(wStub.base.stats, QGSST_ACTIVE, true);

		if (GetAsyncKeyState(VK_LBUTTON))
			wStub.mouse_pending |= QIMM_LEFT;
		if (GetAsyncKeyState(VK_RBUTTON))
			wStub.mouse_pending |= QIMM_RIGHT;
		if (GetAsyncKeyState(VK_MBUTTON))
			wStub.mouse_pending |= QIMM_MIDDLE;
		if (GetAsyncKeyState(VK_XBUTTON1))
			wStub.mouse_pending |= QIMM_X1;
		if (GetAsyncKeyState(VK_XBUTTON2))
			wStub.mouse_pending |= QIMM_X2;

		windows_check_mouse_release();

		stub_toggle_keys(QIKM_CAPS, (GetKeyState(VK_CAPITAL) & 0x1) != 0);
		stub_toggle_keys(QIKM_SCRL, (GetKeyState(VK_SCROLL) & 0x1) != 0);
		stub_toggle_keys(QIKM_NUM, (GetKeyState(VK_NUMLOCK) & 0x1) != 0);

		stub_event_on_window_event(QGWEV_FOCUS, 0, 0);
		stub_event_on_active(true, delta);
	}
	else
	{
		QN_SMASK(wStub.base.stats, QGSST_ACTIVE, false);

		stub_event_on_reset_keys();
		stub_event_on_window_event(QGWEV_LOSTFOCUS, 0, 0);
		stub_event_on_active(false, delta);
	}
}

// 윈도우 메시지 프로시저
static LRESULT CALLBACK windows_mesg_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp)
{
	LRESULT result = -1;

	if (QN_TMASK(wStub.base.features, QGFEATURE_ENABLE_SYSWM))
	{
		const QgEvent e =
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
			if (wStub.mouse_tracked == false)
			{
				TRACKMOUSEEVENT tme =
				{
					.cbSize = sizeof(TRACKMOUSEEVENT),
					.dwFlags = TME_LEAVE,
					.hwndTrack = hwnd
				};
				if (TrackMouseEvent(&tme))
					wStub.mouse_tracked = true;

				if (QN_TMASK(wStub.base.stats, QGSST_FOCUS) == false)
					stub_event_on_focus(true);
			}

			const QmPoint pt = qm_point(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
			stub_event_on_mouse_move(pt.X, pt.Y);
			stub_track_mouse_click(QIM_NONE, QIMT_MOVE);
		}
		else if ((mesg >= WM_LBUTTONDOWN && mesg <= WM_MBUTTONDBLCLK) || (mesg >= WM_XBUTTONDOWN && mesg <= WM_XBUTTONDBLCLK))
		{
			if (windows_get_mouse_source() != WINDOWS_MOUSE_SOURCE_TOUCH)
			{
				if (wStub.mouse_wparam != wp)
				{
					wStub.mouse_wparam = wp;
					const QimMask mask = wStub.base.mouse.mask;
					windows_check_mouse_button(QN_TMASK(wp, MK_LBUTTON), mask, QIM_LEFT);
					windows_check_mouse_button(QN_TMASK(wp, MK_RBUTTON), mask, QIM_RIGHT);
					windows_check_mouse_button(QN_TMASK(wp, MK_MBUTTON), mask, QIM_MIDDLE);
					windows_check_mouse_button(QN_TMASK(wp, MK_XBUTTON1), mask, QIM_X1);
					windows_check_mouse_button(QN_TMASK(wp, MK_XBUTTON2), mask, QIM_X2);

					windows_hold_mouse(wStub.base.mouse.mask != 0);
				}
			}
		}
		else if (mesg == WM_MOUSEWHEEL || mesg == WM_MOUSEHWHEEL)
		{
			const int wheel = GET_WHEEL_DELTA_WPARAM(wp);
			const float delta = (float)wheel / WHEEL_DELTA;
			if (mesg == WM_MOUSEWHEEL)
				stub_event_on_mouse_wheel(0.0f, delta, false);
			else
				stub_event_on_mouse_wheel(delta, 0.0f, false);
		}
		else break;
		goto pos_windows_mesg_proc_exit;
	}

	// 키보드
	while (true)
	{
		if (mesg == WM_KEYDOWN || mesg == WM_SYSKEYDOWN)
		{
			if (!windows_mesg_keyboard(wp, lp, true))
				result = 0;
		}
		else if (mesg == WM_KEYUP || mesg == WM_SYSKEYUP)
		{
			if (!windows_mesg_keyboard(wp, lp, false))
				result = 0;
		}
		else break;
		goto pos_windows_mesg_proc_exit;
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
			if (QN_TMASK(wStub.base.stats, QGSST_FULLSCREEN) && wp != SIZE_MINIMIZED)
			{
				const WindowsMonitor* mon = (const WindowsMonitor*)qg_get_current_monitor();
				SetWindowPos(hwnd, HWND_TOP, mon->rcMonitor.left, mon->rcMonitor.top,
					mon->rcMonitor.right - mon->rcMonitor.left, mon->rcMonitor.bottom - mon->rcMonitor.top,
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
			}
		} break;

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
			result = 0;
		} break;

		case WM_CLOSE:
			stub_event_on_window_event(QGWEV_CLOSE, 0, 0);
			QN_SMASK(wStub.base.stats, QGSST_EXIT, true);
			DestroyWindow(hwnd);
			result = 0;
			break;

		case WM_ERASEBKGND:
			if (wStub.clear_background == false)
			{
				wStub.clear_background = true;
				RECT rect;
				GetClientRect(hwnd, &rect);
				const HBRUSH brush = CreateSolidBrush(0);
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
			const WORD hittest = LOWORD(lp);
			if (hittest == HTCLIENT && QN_TMASK(wStub.base.stats, QGSST_CURSOR))
			{
				SetCursor(wStub.mouse_cursor);
				result = 1;
			}
			else if (wStub.mouse_cursor == NULL)
			{
				SetCursor(NULL);
				result = 1;
			}
		}break;

		case WM_GETMINMAXINFO:
			if (wStub.window_style != 0)
			{
				// 창의 크기 또는 위치가 변경될 때, 최소/최대 크기와 위치 및 최소/최대 추적 크기 재정의
				MINMAXINFO* mmi = (MINMAXINFO*)lp;
				QmSize offset;
				windows_rect_adjust(&offset, 0, 0, GetDpiForWindow ? GetDpiForWindow(hwnd) : USER_DEFAULT_SCREEN_DPI);
				mmi->ptMinTrackSize.x = 128 + offset.Width;
				mmi->ptMinTrackSize.y = 128 + offset.Height;
				if (QN_TMASK(wStub.base.flags, QGFLAG_NOTITLE))
				{
					const HMONITOR mh = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
					MONITORINFO mi = { .cbSize = sizeof(MONITORINFO) };
					GetMonitorInfo(mh, &mi);
					mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
					mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
					mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
					mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
				}
				result = 0;
			}
			break;

		case WM_WINDOWPOSCHANGING:
			if (QN_TMASK(wStub.base.stats, QGSST_LAYOUT))
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
			if (wStub.enter_movesize == false)
				stub_event_on_layout(false);

			InvalidateRect(hwnd, NULL, FALSE);
		}break;

#if false
		case WM_INPUTLANGCHANGE:
			//result = 1;
			break;
#endif

		case WM_DISPLAYCHANGE:
			windows_detect_displays();
			break;

		case WM_NCCALCSIZE:
			break;

		case WM_NCACTIVATE:
			windows_mesg_active(!!wp);
			break;

		case WM_NCHITTEST:
			if (QN_TMASK(wStub.base.stats, QGSST_FULLSCREEN))
				return HTCLIENT;
			break;

		case WM_NCCREATE:
			if (EnableNonClientDpiScaling != NULL && QN_TMASK(wStub.base.flags, QGFLAG_DPISCALE))
				EnableNonClientDpiScaling(hwnd);
			break;

		case WM_INPUT:
			break;

		case WM_CHAR:
			if (IS_HIGH_SURROGATE(wp))
				wStub.high_surrogate = (WCHAR)wp;
			else if (IS_SURROGATE_PAIR(wStub.high_surrogate, wp))
			{
				// 여기서 wStub.high_surrogate 와 wp 를 조합해서 키보드 데이터를 처리
				// https://learn.microsoft.com/ko-kr/windows/win32/inputdev/wm-char
				char u8[7];
				if (qn_u16ucb((uchar2)wStub.high_surrogate, (uchar2)wp, u8))
					stub_event_on_text(u8);
				wStub.high_surrogate = 0;
			}
			else
			{
				// 윈도우10 UTF32
				char u8[7];
				if (qn_u32ucb((uchar4)wp, u8) > 0)
					stub_event_on_text(u8);
			}
			result = 0;
			break;

		case WM_UNICHAR:
			if (wp == UNICODE_NOCHAR)
				result = 1;
			else
			{
				// 여기서 키보드 데이터를 처리, 유니코드가 아닌 윈도우에서 유니코드를 처리할 때 사용
				// https://learn.microsoft.com/ko-kr/windows/win32/inputdev/wm-unichar
				char u8[7];
				if (qn_u32ucb((uchar4)wp, u8))
					stub_event_on_text(u8);
				result = 0;
			}
			break;

		case WM_SYSCOMMAND:
		{
			const ushort cmd = (ushort)(wp & 0xFFFF0);
			if (cmd == SC_KEYMENU)
				result = 0;
			if (QN_TMASK(wStub.base.features, QGFEATURE_DISABLE_SCRSAVE) && (cmd == SC_SCREENSAVE || cmd == SC_MONITORPOWER))
				result = 0;
		} break;

		case WM_MENUCHAR:
			result = MAKELRESULT(0, MNC_CLOSE);
			break;

		case WM_SIZING:
			if (wStub.window_style != 0 && QN_TMASK(wStub.base.features, QGFEATURE_ENABLE_ASPECT))
				windows_rect_aspect((RECT*)lp, (int)wp);
			result = 1;
			break;

		case WM_CAPTURECHANGED:
			// 타이틀 안 눌림
			break;

		case WM_DEVICECHANGE:
		{
			if (wp == DBT_DEVICEARRIVAL)
			{
				const DEV_BROADCAST_HDR* hdr = (const DEV_BROADCAST_HDR*)lp;
				if (hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
				{
					const DEV_BROADCAST_DEVICEINTERFACE* di = (const DEV_BROADCAST_DEVICEINTERFACE*)lp;

					if (memcmp(&di->dbcc_classguid, &GUID_DEVINTERFACE_HID, sizeof(GUID)) == 0)
					{
						// 컨트롤러
					}
					/*else if (memcmp(&di->dbcc_classguid, &GUID_DEVINTERFACE_MONITOR, sizeof(GUID)) == 0)
						windows_detect_displays();*/
				}
			}
			else if (wp == DBT_DEVICEREMOVECOMPLETE)
			{
				const DEV_BROADCAST_HDR* hdr = (const DEV_BROADCAST_HDR*)lp;
				if (hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
				{
					const DEV_BROADCAST_DEVICEINTERFACE* di = (const DEV_BROADCAST_DEVICEINTERFACE*)lp;
					if (memcmp(&di->dbcc_classguid, &GUID_DEVINTERFACE_HID, sizeof(GUID)) == 0)
					{
						// 컨트롤러
					}
					/*else if (memcmp(&di->dbcc_classguid, &GUID_DEVINTERFACE_MONITOR, sizeof(GUID)) == 0)
						windows_detect_displays();*/
				}
			}
		} break;

		case WM_ENTERSIZEMOVE:
			wStub.enter_movesize = true;
			stub_event_on_layout(true);
			break;

		case WM_EXITSIZEMOVE:
			wStub.enter_movesize = false;
			stub_event_on_layout(false);
			break;

		case WM_DROPFILES:
			if (QN_TMASK(wStub.base.features, QGFEATURE_ENABLE_DROP))
			{
				DragAcceptFiles(hwnd, false);
				const HDROP handle = (HDROP)wp;
				const UINT cnt = DragQueryFile(handle, 0xFFFFFFFF, NULL, 0);
				for (UINT i = 0; i < cnt; i++)
				{
					const UINT len = DragQueryFile(handle, i, NULL, 0) + 1;
					const LPWSTR buf = qn_alloc(len, WCHAR);
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
			wStub.mouse_lparam = lp;
			break;

		case WM_MOUSELEAVE:
			if (QN_TMASK(wStub.base.stats, QGSST_HOLD) == false)
			{
				POINT pos;
				GetCursorPos(&pos);
				ScreenToClient(hwnd, &pos);
				stub_event_on_mouse_move(pos.x, pos.y);
				stub_event_on_focus(false);
			}
			wStub.mouse_tracked = false;
			result = 0;
			break;

		case WM_DPICHANGED:
			if (wStub.window_style != 0)
			{
				// https://learn.microsoft.com/ko-kr/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
				if (QN_TMASK(wStub.base.flags, QGFLAG_DPISCALE))
				{
					const RECT* const suggested = (RECT*)lp;
					SetWindowPos(hwnd, HWND_TOP, suggested->left, suggested->top,
						suggested->right - suggested->left, suggested->bottom - suggested->top,
						SWP_NOACTIVATE | SWP_NOZORDER);
				}
#if false
				const float xdpi = HIWORD(wp) / 96.0f;
				const float ydpi = LOWORD(wp) / 96.0f;
				// TODO: 여기에 DPI 변경 알림 이벤트 날리면 좋겠네
#endif
			}
			break;

		case WM_GETDPISCALEDSIZE:
			if (wStub.window_style != 0)
			{
				if (AdjustWindowRectExForDpi == NULL || QN_TMASK(wStub.base.flags, QGFLAG_DPISCALE) == false)
					break;
				RECT src = { 0, 0, 0, 0 };
				RECT dst = { 0, 0, 0, 0 };
				AdjustWindowRectExForDpi(&src, wStub.window_style, FALSE, QGSTUB_WIN_EXSTYLE, GetDpiForWindow(hwnd));
				AdjustWindowRectExForDpi(&dst, wStub.window_style, FALSE, QGSTUB_WIN_EXSTYLE, LOWORD(wp));
				SIZE* size = (SIZE*)lp;
				size->cx += (dst.right - dst.left) - (src.right - src.left);
				size->cy += (dst.bottom - dst.top) - (src.bottom - src.top);
				result = 1;
			}
			break;

		default:
			break;
	}

pos_windows_mesg_proc_exit:
	if (result >= 0)
		return result;
	return CallWindowProc(DefWindowProc, hwnd, mesg, wp, lp);
}
#pragma endregion 윈도우 메시지

#endif // _QN_WINDOWS_
