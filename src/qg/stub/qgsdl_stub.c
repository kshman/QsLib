//
// qgsdl_stub.c - SDL 스터브
// 2023-11-23 by kim
//

#include "pch.h"
#include "qs_qn.h"
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qg_stub.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#pragma region 정적 함수 미리 선언
// SDLK를 QIK로
extern QikKey sdlk_to_qik(uint32_t sdlk);
// SDL 키보드 상태 변환
extern QikMask kmod_to_qikm(int modifier);
// SDL 마우스 버튼을 QIM으로
extern QimButton sdlm_to_qim(byte button);

static bool _sdl_mesg_proc(void);
#pragma endregion


//////////////////////////////////////////////////////////////////////////

#pragma region 스터브 선언
// SDL 스터브
typedef struct SDLStub
{
	StubBase			base;

	SDL_Window*			window;
	SDL_SysWMinfo		syswm;
} SDLStub;

// SDL 스터브 여기 있다!
SDLStub sdlStub;
#pragma endregion 스터브 선언

#pragma region 시스템 함수
//
bool stub_system_open(const char* title, const int display, const int width, const int height, QgFlag flags)
{
	qn_zero_1(&sdlStub);

	// SDL_INIT_TIMER SDL_INIT_AUDIO SDL_INIT_GAMECONTROLLER SDL_INIT_EVENTS SDL_INIT_SENSOR
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		qn_debug_outputs(true, "SDL STUB", "SDL_Init() failed");
		return false;
	}

	int monitors = SDL_GetNumVideoDisplays();
	if (monitors < 1)
	{
		qn_debug_outputs(true, "SDL STUB", "no display found!");
		return false;
	}

	//
	stub_initialize((StubBase*)&sdlStub, flags);

	//
	for (int i = 0; i < monitors; i++)
	{
		SDL_Rect rect;
		SDL_DisplayMode mode;
		float ddpi, hdpi, vdpi;
		const char *name = SDL_GetDisplayName(i);
		SDL_GetDisplayBounds(i, &rect);
		SDL_GetDesktopDisplayMode(i, &mode);
		SDL_GetDisplayDPI(i, &ddpi, &hdpi, &vdpi);

		QgUdevMonitor* mon = qn_alloc_1(QgUdevMonitor);
		qn_strncpy(mon->name, QN_COUNTOF(mon->name), name, QN_COUNTOF(mon->name) - 1);
		mon->no = i;
		mon->x = (uint)rect.x;
		mon->y = (uint)rect.y;
		mon->width = (uint)rect.w;
		mon->height = (uint)rect.h;
		mon->refresh = (uint)mode.refresh_rate;
		mon->mmwidth = (uint)((rect.w * 25.0f) / hdpi);
		mon->mmheight = (uint)((rect.h * 25.0f) / vdpi);
		mon->oshandle = NULL;

		stub_event_on_monitor(mon, true, false);
	}

	//
	const QgUdevMonitor* monitor = qn_pctnr_nth(&sdlStub.base.monitors,
		(size_t)display < qn_pctnr_count(&sdlStub.base.monitors) ? display : 0);
	sdlStub.base.display = monitor->no;

	int window_flags = SDL_WINDOW_OPENGL;
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	if (QN_TMASK(flags, QGFLAG_BORDERLESS))
		window_flags |= SDL_WINDOW_BORDERLESS;
	if (QN_TMASK(flags, QGFLAG_RESIZABLE))
		window_flags |= SDL_WINDOW_RESIZABLE;

	QmSize clientsize;
	if (width > 256 && height > 256)
		qm_set2(&clientsize, width, height);
	else
	{
		if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
			qm_set2(&clientsize, 256, 256);	// SDL은 풀스크린일 때 너비/높이 무시함
		else
		{
			if (monitor->height > 800)
				qm_set2(&clientsize, 1280, 720);
			else
				qm_set2(&clientsize, 720, 450);
		}
	}

	sdlStub.window = SDL_CreateWindow(title && *title ? title : "QS",
		SDL_WINDOWPOS_CENTERED_DISPLAY(sdlStub.base.display),
		SDL_WINDOWPOS_CENTERED_DISPLAY(sdlStub.base.display),
		clientsize.width, clientsize.height, window_flags);
	if (sdlStub.window == NULL)
	{
		qn_debug_outputs(true, "SDL STUB", "SDL_CreateWindow() failed");
		return false;
	}

	SDL_GetWindowWMInfo(sdlStub.window, &sdlStub.syswm);

	// 오케
	stub_system_update_bound();

	return true;
}

//
void stub_system_finalize(void)
{
	if (sdlStub.window != NULL)
		SDL_DestroyWindow(sdlStub.window);

	SDL_Quit();
}

//
bool stub_system_poll(void)
{
	if (_sdl_mesg_proc() == false)
		return false;
	return QN_TMASK(sdlStub.base.stats, QGSSTT_EXIT) == false;
}

//
void stub_system_disable_acs(const bool enable)
{
	// 지원 안하능가
	QN_DUMMY(enable);
}

//
void stub_system_disable_scr_save(const bool enable)
{
	if (enable)
	{
		if (QN_TMASK(sdlStub.base.flags, QGFEATURE_DISABLE_SCRSAVE))
			return;
		QN_SMASK(&sdlStub.base.flags, QGFEATURE_DISABLE_SCRSAVE, true);
		SDL_DisableScreenSaver();
	}
	else
	{
		if (QN_TMASK(sdlStub.base.flags, QGFEATURE_DISABLE_SCRSAVE) == false)
			return;
		QN_SMASK(&sdlStub.base.flags, QGFEATURE_DISABLE_SCRSAVE, false);
		SDL_EnableScreenSaver();
	}
}

//
void stub_system_enable_drop(const bool enable)
{
	if (QN_TMASK(sdlStub.base.flags, QGFEATURE_ENABLE_DROP) == enable)
		return;
	QN_SMASK(&sdlStub.base.flags, QGFEATURE_ENABLE_DROP, enable);
}

//
void stub_system_set_title(const char* title)
{
	SDL_SetWindowTitle(sdlStub.window, title);
}

//
void stub_system_update_bound(void)
{
	SDL_GetWindowSize(sdlStub.window, &sdlStub.base.client_size.width, &sdlStub.base.client_size.height);
	SDL_GetWindowPosition(sdlStub.window, &sdlStub.base.window_bound.left, &sdlStub.base.window_bound.top);
	qm_rect_resize(&sdlStub.base.window_bound, sdlStub.base.client_size.width, sdlStub.base.client_size.height);
}

//
void stub_system_focus(void)
{
	SDL_RaiseWindow(sdlStub.window);
}
#pragma endregion 시스템 함수


//////////////////////////////////////////////////////////////////////////

#pragma region SDL 폴링
// 액티브 메시지
static void _sdl_mesg_active(const bool focus)
{
	const double now = sdlStub.base.timer->abstime;
	const double delta = now - sdlStub.base.active;
	sdlStub.base.active = now;

	if (focus)
	{
		QN_SMASK(&sdlStub.base.stats, QGSSTT_ACTIVE, true);

		SDL_Keymod mod = SDL_GetModState();
		sdlStub.base.key.mask = kmod_to_qikm(mod);

		stub_event_on_window_event(QGWEV_FOCUS, 0, 0);
		stub_event_on_active(true, delta);
	}
	else
	{
		QN_SMASK(&sdlStub.base.stats, QGSSTT_ACTIVE, false);

		stub_event_on_reset_keys();
		stub_event_on_window_event(QGWEV_LOSTFOCUS, 0, 0);
		stub_event_on_active(false, delta);
	}
}

// 키보드 메시지
static void _sdl_mesg_keyboard(SDL_KeyCode sdl_key, const bool down)
{
	const QikKey key = sdlk_to_qik(sdl_key);

#ifdef _DEBUG
	if (down && key == QIK_F12)
	{
		SDL_Quit();
		return;
	}
#endif

	stub_event_on_keyboard(key, down);
}

// 마우스 이동 포인터
static void _sdl_set_mouse_point(int x, int y)
{
	QgUimMouse* um = &sdlStub.base.mouse;
	um->last = um->pt;
	qm_set2(&um->pt, x, y);
}

// 마우스 버튼
static void _sdl_mesg_mouse_button(SDL_MouseButtonEvent* se, bool isdown)
{
	_sdl_set_mouse_point(se->x, se->y);

	QimButton button = sdlm_to_qim(se->button);
	QgUimMouse* um = &sdlStub.base.mouse;
	QN_SBIT(&um->mask, button, isdown);

	stub_event_on_mouse_button(button, isdown);
}

// SDL 폴링 메시지
static bool _sdl_mesg_proc(void)
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
			case SDL_WINDOWEVENT:
				switch (ev.window.event)
				{
					case SDL_WINDOWEVENT_SHOWN:
						stub_event_on_window_event(QGWEV_SHOW, 0, 0);
						break;
					case SDL_WINDOWEVENT_HIDDEN:
						stub_event_on_window_event(QGWEV_HIDE, 0, 0);
						break;
					case SDL_WINDOWEVENT_EXPOSED:
						stub_event_on_window_event(QGWEV_PAINTED, 0, 0);
						break;
					case SDL_WINDOWEVENT_MOVED:
						stub_event_on_window_event(QGWEV_MOVED, ev.window.data1, ev.window.data2);
						break;
					case SDL_WINDOWEVENT_RESIZED:
						stub_event_on_window_event(QGWEV_SIZED, ev.window.data1, ev.window.data2);
						stub_event_on_layout(false);
						break;
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						stub_event_on_layout(false);
						break;
					case SDL_WINDOWEVENT_MINIMIZED:
						stub_event_on_window_event(QGWEV_MINIMIZED, ev.window.data1, ev.window.data2);
						break;
					case SDL_WINDOWEVENT_MAXIMIZED:
						stub_event_on_window_event(QGWEV_MAXIMIZED, ev.window.data1, ev.window.data2);
						break;
					case SDL_WINDOWEVENT_RESTORED:
						stub_event_on_window_event(QGWEV_RESTORED, ev.window.data1, ev.window.data2);
						break;
					case SDL_WINDOWEVENT_ENTER:
						break;
					case SDL_WINDOWEVENT_LEAVE:
						break;
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						_sdl_mesg_active(true);
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						_sdl_mesg_active(false);
						break;
					case SDL_WINDOWEVENT_CLOSE:
						break;
					case SDL_WINDOWEVENT_TAKE_FOCUS:
						break;
					case SDL_WINDOWEVENT_HIT_TEST:
						break;
				}
				break;

			case SDL_KEYDOWN:
				_sdl_mesg_keyboard(ev.key.keysym.sym, true);
				break;

			case SDL_KEYUP:
				_sdl_mesg_keyboard(ev.key.keysym.sym, false);
				break;

			case SDL_MOUSEMOTION:
				_sdl_set_mouse_point(ev.motion.x, ev.motion.y);
				stub_track_mouse_click(QIM_NONE, QIMT_MOVE);
				stub_event_on_mouse_move();
				break;

			case SDL_MOUSEBUTTONDOWN:
				_sdl_mesg_mouse_button(&ev.button, true);
				break;

			case SDL_MOUSEBUTTONUP:
				_sdl_mesg_mouse_button(&ev.button, false);
				break;

			case SDL_MOUSEWHEEL:
				stub_event_on_mouse_wheel(ev.wheel.preciseX, ev.wheel.preciseY, false);
				break;

			case SDL_DROPBEGIN:
				break;

			case SDL_DROPFILE:
				if (QN_TMASK(sdlStub.base.flags, QGFEATURE_ENABLE_DROP))
				{
					char* filename = qn_strdup(ev.drop.file);
					stub_event_on_drop(filename, (int)strlen(filename), false);
				}
				break;

			case SDL_DROPCOMPLETE:
				if (QN_TMASK(sdlStub.base.flags, QGFEATURE_ENABLE_DROP))
					stub_event_on_drop(NULL, 0, true);
				break;

			case SDL_TEXTINPUT:
				stub_event_on_text(ev.text.text);
				break;

			case SDL_RENDER_TARGETS_RESET:
				break;

			case SDL_RENDER_DEVICE_RESET:
				break;

			case SDL_QUIT:
				stub_event_on_window_event(QGWEV_CLOSE, 0, 0);
				QN_SMASK(&sdlStub.base.stats, QGSSTT_EXIT, true);
				return false;
		}
	}
	return true;
}

#pragma endregion SDL 폴링
