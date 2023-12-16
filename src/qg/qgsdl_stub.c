//
// qgsdl_stub.c - SDL 스터브
// 2023-12-13 by kim
//

#include "pch.h"
#if USE_SDL2
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qg_stub.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4820)
#endif
#include "SDL2/SDL_syswm.h"
#include "SDL2/SDL_keyboard.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////
// SDL 스터브

/** @brief SDL 스터브 */
typedef struct SdlStub SdlStub;
struct SdlStub
{
	struct StubBase		base;

	SDL_Window*			window;
	SDL_SysWMinfo		wminfo;

	QN_PADDING_32(4, 0)
};

//
StubBase* stub_system_open(const char* title, int width, int height, int flags)
{
	static SdlStub s_stub;

	// SDL_INIT_TIMER SDL_INIT_AUDIO SDL_INIT_HAPTIC SDL_INIT_SENSOR
	const Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER;
	if (SDL_Init(sdl_flags) != 0)
		return NULL;

	SdlStub* stub = &s_stub;
	qn_zero_1(stub);

	const char* wtitle = title != NULL ? title : "STUB Window";
	int wflags = SDL_WINDOW_OPENGL;
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
		wflags |= SDL_WINDOW_FULLSCREEN;
	if (QN_TMASK(flags, QGFLAG_BORDERLESS))
		wflags |= SDL_WINDOW_BORDERLESS;
	if (QN_TMASK(flags, QGFLAG_RESIZABLE))
		wflags |= SDL_WINDOW_RESIZABLE;
	if (QN_TMASK(flags, QGFLAG_FOCUS))
		wflags |= SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;

	stub->window = SDL_CreateWindow(wtitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, wflags);
	if (stub->window == NULL)
	{
		qn_debug_outputf(false, "SDL", "unable to initialize (%d)", SDL_GetError());
		return NULL;
	}

	SDL_GetWindowWMInfo(stub->window, &stub->wminfo);
	SDL_GetWindowPosition(stub->window, &stub->base.pos.x, &stub->base.pos.y);
	SDL_GetWindowSize(stub->window, &stub->base.size.width, &stub->base.size.height);
	qn_rect_set_qn(&stub->base.bound, &stub->base.pos, &stub->base.size);

	stub->base.handle = stub->window;

	return &stub->base;
}

//
void stub_system_finalize(void)
{
	const SdlStub* stub = (SdlStub*)qg_stub_instance;

	if (stub->window)
		SDL_DestroyWindow(stub->window);

	SDL_Quit();
}

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
static void sdl_mesg_keyboard(const SDL_KeyboardEvent* se, bool down)
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
static void sdl_mesg_mouse_move(const SDL_MouseMotionEvent* se)
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
static void sdl_mesg_mouse_button(const SDL_MouseButtonEvent* se, bool down)
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
static void sdl_mesg_mouse_wheel(const SDL_MouseWheelEvent* se)
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
