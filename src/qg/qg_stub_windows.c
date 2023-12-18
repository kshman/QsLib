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
#include <Xinput.h>

#ifdef _MSC_VER
#pragma warning(disable:4152)
#endif


//////////////////////////////////////////////////////////////////////////
// 윈도우 스터브

static const wchar* s_windows_class_name = L"__qs_qg_windows_stub_23_12_13__";

/** @brief 컨트롤러 */
#define DEFAULT_DEAD_ZONE	(int)(0.24f*((float)INT16_MAX))

typedef void (WINAPI* stubXInputEnable)(_In_ BOOL enable);
typedef DWORD(WINAPI* stubXInputGetState)(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState);
typedef DWORD(WINAPI* stubXInputSetState)(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);
typedef DWORD(WINAPI* stubXInputGetCapabilities)(_In_ DWORD dwUserIndex, _In_ DWORD dwFlags, _Out_ XINPUT_CAPABILITIES* pCapabilities);
typedef DWORD(WINAPI* stubXInputGetBatteryInformation)(_In_ DWORD dwUserIndex, _In_ BYTE devType, _Out_ XINPUT_BATTERY_INFORMATION* pBatteryInformation);
static struct WindowsXInputFunc
{
	stubXInputEnable funcXInputEnable;
	stubXInputGetState funcXInputGetState;
	stubXInputSetState funcXInputSetState;
	stubXInputGetCapabilities funcXInputGetCapabilities;
	stubXInputGetBatteryInformation funcXInputGetBatteryInformation;
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
}

/** @brief 키 후킹 */
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
	wchar*				title;

	RECT				window_rect;
	QnRect				bound;

	DEVMODE				dev_mode;
	STICKYKEYS			acs_sticky;
	TOGGLEKEYS			acs_toggle;
	FILTERKEYS			acs_filter;

	HIMC				himc;
	int					imcs;

	int					deadzone_min;
	int					deadzone_max;

	bool				focus;
	bool				hold_mouse;
	bool				class_registed;
};

//
static void windows_stub_hold_mouse(WindowsStub* stub, bool hold);
static LRESULT CALLBACK windows_stub_window_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp);

//
StubBase* stub_system_open(const char* title, int width, int height, int flags)
{
	static WindowsStub s_stub;
	WindowsStub* stub = &s_stub;

	windows_internal_controller_init();
	if (s_xinput_fn.funcXInputEnable != NULL)
		s_xinput_fn.funcXInputEnable(TRUE);

	stub->instance = GetModuleHandle(NULL);
	qn_val_if_fail(stub->instance != NULL, NULL);

	// 윈도우 클래스
	stub->dev_mode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &stub->dev_mode);

	wchar module_name[260];
	GetModuleFileName(NULL, module_name, QN_COUNTOF(module_name) - 1);

	// 윈도우 클래스
	if (stub->class_registed == false)
	{
		WNDCLASSEX wc =
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_BYTEALIGNCLIENT | CS_OWNDC,
			.lpfnWndProc = windows_stub_window_proc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = stub->instance,
			.hIcon = NULL,
			.hCursor = NULL,
			.hbrBackground = NULL,
			.lpszMenuName = NULL,
			.lpszClassName = s_windows_class_name,
			.hIconSm = NULL,
		};

		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.hIcon = ExtractIcon(stub->instance, module_name, 0);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpfnWndProc = windows_stub_window_proc;
		if (!RegisterClass(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		{
			qn_debug_outputs(true, "WINDOWS STUB", "register window class failed");
			return NULL;
		}

		stub->class_registed = true;
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

	QnSize scrsize = { GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };
	RECT rect;
	if (width > 128 && height > 128)
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

	QnPoint loc = { 0, 0 };
	QnSize size = { rect.right - rect.left, rect.bottom - rect.top };
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN) == false)
		qn_point_set(&loc, (scrsize.width - size.width) / 2, (scrsize.height - size.height) / 2);

	// 윈도우 만들기
	stub->title = qn_u8to16_dup(title, 0);
	stub->hwnd = CreateWindowEx(0, s_windows_class_name, stub->title, style,
		loc.x, loc.y, size.width, size.height,
		NULL, NULL, stub->instance, NULL);
	if (stub->hwnd == NULL)
	{
		qn_debug_outputs(true, "WINDOWS STUB", "cannot create window");
		qn_free(stub->title);
		return NULL;
	}

	ShowWindow(stub->hwnd, SW_SHOWNORMAL);
	UpdateWindow(stub->hwnd);

	// 최종 스타일 변경
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN | QGFLAG_NOTITLE))
		SetWindowLong(stub->hwnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE);
	else
	{
		style = GetWindowLong(stub->hwnd, GWL_STYLE);
		SetWindowLong(stub->hwnd, GWL_STYLE, style);
	}
	SetProcessDPIAware();

	// 토글키 상태 저장
	if (GetKeyState(VK_CAPITAL) & 0x1)
		QN_SMASK(&stub->base.key.mask, QIKM_CAPS, true);
	if (GetKeyState(VK_SCROLL) & 0x1)
		QN_SMASK(&stub->base.key.mask, QIKM_SCRL, true);
	if (GetKeyState(VK_NUMLOCK) & 0x1)
		QN_SMASK(&stub->base.key.mask, QIKM_NUM, true);

	// ACS
	if (QN_TMASK(flags, QGFLAG_NOACS))
	{
		QN_SMASK(&stub->base.stats, QGSTTI_NOACS, true);

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

	// 값설정
	GetWindowRect(stub->hwnd, &stub->window_rect);
	GetClientRect(stub->hwnd, &rect);
	qn_rect_set(&stub->bound, rect.left, rect.top, rect.right, rect.bottom);

	stub->base.stats = QGSTTI_CURSOR | QGSTTI_SCRSAVE;
	stub->deadzone_min = -DEFAULT_DEAD_ZONE;
	stub->deadzone_max = +DEFAULT_DEAD_ZONE;

	return (StubBase*)stub;
}

//
void stub_system_finalize(void)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	if (stub->hwnd != NULL)
	{
		stub->base.stats |= QGSTTI_EXIT;
		windows_stub_hold_mouse(stub, false);
		if (stub->himc != NULL)
			ImmAssociateContextEx(stub->hwnd, stub->himc, IACE_DEFAULT);
		if (QN_TMASK(stub->base.stats, QGSTTI_VIRTUAL) == false)
			SendMessage(stub->hwnd, WM_CLOSE, 0, 0);
	}

	if (QN_TMASK(stub->base.stats, QGSTTI_NOACS))
	{
		windows_internal_unhook_key();

		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &stub->acs_sticky, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &stub->acs_toggle, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &stub->acs_filter, 0);
	}

	if (s_xinput_fn.funcXInputEnable != NULL)
		s_xinput_fn.funcXInputEnable(FALSE);
}

//
static void windows_stub_mesg_loop(WindowsStub* stub)
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
	}
}

//
static void windows_stub_hold_mouse(WindowsStub* stub, bool hold)
{
	if (hold)
	{
		if (stub->hold_mouse == false)
		{
			stub->hold_mouse = true;
			SetCapture(stub->hwnd);
		}
	}
	else
	{
		if (stub->hold_mouse)
		{
			stub->hold_mouse = false;
			ReleaseCapture();
		}
	}
}

//
static LRESULT CALLBACK windows_stub_window_proc(HWND hwnd, UINT mesg, WPARAM wp, LPARAM lp)
{
	WindowsStub* stub = (WindowsStub*)qg_stub_instance;

	return DefWindowProc(hwnd, mesg, wp, lp);
}

#endif

#ifdef USE_SDL2
// 메시지: 액티브
static double sdl_mesg_active(bool isactive)
{
	SdlStub* stub = (SdlStub*)qg_stub_instance;
	const double now = qn_timer_get_abs(stub->base.timer);
	const double adv = now - stub->base.active;
	stub->base.active = now;
	QN_SMASK(&stub->base.stats, QGSTTI_ACTIVE, isactive);

	const QgEvent e =
	{
		.active.ev = QGEV_ACTIVE,
		.active.active = isactive,
		.active.delta = adv,
	};
	qg_add_event(&e);

	return adv;
}

// 메시지: 레이아웃
static void sdl_mesg_layout(void)
{
	SdlStub* stub = (SdlStub*)qg_stub_instance;
	const QnSize prev = stub->base.size;

	SDL_GetWindowPosition(stub->window, &stub->base.pos.x, &stub->base.pos.y);
	SDL_GetWindowSize(stub->window, &stub->base.size.width, &stub->base.size.height);
	qn_rect_set_qn(&stub->base.bound, &stub->base.pos, &stub->base.size);

	if (!qn_size_eq(&prev, &stub->base.size))
	{
		// 크기가 바꼈다 -> 트리거
		const QgEvent e =
		{
			.layout.ev = QGEV_LAYOUT,
			.layout.bound = stub->base.bound,
		};
		qg_add_event(&e);
	}
}

// 메시지: 키보드
static void sdl_mesg_keyboard(const SDL_KeyboardEvent * se, bool down)
{
	StubBase* stub = qg_stub_instance;
	const QikMask mask = kmod_to_qikm(se->keysym.mod);
	stub->key.mask = mask;

	const QikKey key = sdlk_to_qik(se->keysym.sym);
	qn_ret_if_fail(key != QIK_NONE);
	qg_set_key(key, down);

	const QgEvent e =
	{
		.key.ev = down ? QGEV_KEYDOWN : QGEV_KEYUP,
		.key.state = mask,
		.key.key = key,
		.key.pressed = se->state == SDL_PRESSED,
		.key.repeat = se->repeat,
	};
	qg_add_event(&e);
}

// 도움: 마우스 위치
static void sdl_set_mouse_point(int x, int y)
{
	StubBase* stub = qg_stub_instance;
	QgUimMouse* m = &stub->mouse;
	m->last = m->pt;
	qn_point_set(&m->pt, x, y);
}

// 도움: 마우스 버튼 마스크
static QimButton sdl_set_mouse_button(int button, bool down)
{
	StubBase* stub = qg_stub_instance;
	switch (button)
	{
		case SDL_BUTTON_LEFT:
			QN_SMASK(&stub->mouse.mask, QIMM_LEFT, down);
			return QIM_LEFT;
		case SDL_BUTTON_RIGHT:
			QN_SMASK(&stub->mouse.mask, QIMM_RIGHT, down);
			return QIM_RIGHT;
		case SDL_BUTTON_MIDDLE:
			QN_SMASK(&stub->mouse.mask, QIMM_MIDDLE, down);
			return QIM_MIDDLE;
		case SDL_BUTTON_X1:
			QN_SMASK(&stub->mouse.mask, QIMM_X1, down);
			return QIM_X1;
		case SDL_BUTTON_X2:
			QN_SMASK(&stub->mouse.mask, QIMM_X2, down);
			return QIM_X2;
		default:
			return QIM_NONE;
	}
}

// 메시지: 마우스 이동
static void sdl_mesg_mouse_move(const SDL_MouseMotionEvent * se)
{
	StubBase* stub = qg_stub_instance;
	sdl_set_mouse_point(se->x, se->y);
	stub_internal_mouse_clicks(QIM_NONE, QIMT_MOVE);

	const QgEvent e =
	{
		.mmove.ev = QGEV_MOUSEMOVE,
		.mmove.state = stub->mouse.mask,
		.mmove.pt.x = se->x,
		.mmove.pt.y = se->y,
		.mmove.delta.x = stub->mouse.last.x - se->x,
		.mmove.delta.y = stub->mouse.last.y - se->y,
	};
	qg_add_event(&e);
}

// 메시지: 마우스 버튼
static void sdl_mesg_mouse_button(const SDL_MouseButtonEvent * se, bool down)
{
	StubBase* stub = qg_stub_instance;
	sdl_set_mouse_point(se->x, se->y);

	QgEvent e =
	{
		.mbutton.button = sdl_set_mouse_button(se->button, down),
		.mbutton.state = stub->mouse.mask,
		.mbutton.pt.x = se->x,
		.mbutton.pt.y = se->y,
	};

	if (down)
	{
		bool doubleclick = stub_internal_mouse_clicks(e.mbutton.button, QIMT_DOWN);
		if (doubleclick)
		{
			e.mbutton.ev = QGEV_MOUSEDOUBLE;
			qg_add_event(&e);
		}
		e.mbutton.ev = QGEV_MOUSEDOWN;
		qg_add_event(&e);
	}
	else
	{
		e.mbutton.ev = QGEV_MOUSEUP;
		qg_add_event(&e);
	}
}

// 메시지: 마우스 휠
static void sdl_mesg_mouse_wheel(const SDL_MouseWheelEvent * se)
{
	StubBase* stub = qg_stub_instance;
	stub->mouse.wheel = se->direction == SDL_MOUSEWHEEL_NORMAL ? 1 : -1;
	const QgEvent e =
	{
		.mwheel.ev = QGEV_MOUSEWHEEL,
		.mwheel.dir = stub->mouse.wheel,
		.mwheel.pt.x = se->x,
		.mwheel.pt.y = se->y,
	};
	qg_add_event(&e);
}

//
bool stub_system_poll(void)
{
	StubBase* stub = qg_stub_instance;
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)		// 포커스 잃음
					sdl_mesg_active(true);
				else if (ev.window.event == SDL_WINDOWEVENT_FOCUS_LOST)		// 포커스 얻음
				{
					QgEvent e =
					{
						.key.ev = QGEV_KEYUP,
						.key.state = stub->key.mask,
						.key.pressed = 0,
						.key.repeat = 0,
					};
					QgUimKey* k = &stub->key;
					for (int i = 0; i < QIK_MAX_VALUE / 8 + 1; i++)
					{
						if (k->key[i] == 0)
							continue;

						for (int u = 0; u < 8; u++)
						{
							if (QN_TBIT(k->key[i], u) == false)
								continue;
							QN_SBIT(&k->key[i], u, false);

							e.key.key = (QikKey)(i * 8 + u);
							qg_add_event(&e);
						}
					}
					// 액티브 메시지는 키보드 처리 다음에
					sdl_mesg_active(false);
				}
				else if (ev.window.event == SDL_WINDOWEVENT_RESIZED)		// 크기 변경
					sdl_mesg_layout();
				break;

#if 0
			case SDL_CONTROLLERAXISMOTION:
				// 해야함
				break;

			case SDL_CONTROLLERBUTTONDOWN:
				// 해야함
				break;

			case SDL_CONTROLLERBUTTONUP:
				// 해야함
				break;

			case SDL_CONTROLLERDEVICEADDED:
				// 해야함
				break;

			case SDL_CONTROLLERDEVICEREMOVED:
				// 해야함
				break;
#endif

			case SDL_KEYDOWN:
				sdl_mesg_keyboard(&ev.key, true);
				break;

			case SDL_KEYUP:
				sdl_mesg_keyboard(&ev.key, false);
				break;

			case SDL_MOUSEMOTION:
				sdl_mesg_mouse_move(&ev.motion);
				break;

			case SDL_MOUSEBUTTONDOWN:
				sdl_mesg_mouse_button(&ev.button, true);
				break;

			case SDL_MOUSEBUTTONUP:
				sdl_mesg_mouse_button(&ev.button, false);
				break;

			case SDL_MOUSEWHEEL:
				sdl_mesg_mouse_wheel(&ev.wheel);
				break;

#if 0
			case SDL_DROPFILE:
				// 해야함
				break;

			case SDL_DROPTEXT:
				// 해야함
				break;

			case SDL_DROPBEGIN:
				// 해야함
				break;

			case SDL_DROPCOMPLETE:
				// 해야함
				break;

			case SDL_RENDER_TARGETS_RESET:
				// 해야함
				break;

			case SDL_RENDER_DEVICE_RESET:
				// 해야함
				break;
#endif

			case SDL_QUIT:
				QN_SMASK(&stub->stats, QGSTTI_EXIT, true);
				qg_add_event_type(QGEV_EXIT);
				return false;

			default:
				break;
		}
	}
	return QN_TMASK(stub->stats, QGSTTI_EXIT) == false;
}

#endif
