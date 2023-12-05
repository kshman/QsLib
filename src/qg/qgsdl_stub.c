#include "pch.h"
#if USE_SDL
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qg_stub.h"
#include "SDL2/SDL_syswm.h"
#include "SDL2/SDL_keyboard.h"

//////////////////////////////////////////////////////////////////////////
// SDL 스터브

//
struct SDLStub
{
	struct StubBase		base;

	SDL_Window*			window;
	SDL_SysWMinfo		wminfo;
};

//
struct StubBase* stub_system_open(struct StubParam* param)
{
	static struct SDLStub s_stub;

	// SDL_INIT_TIMER, SDL_INIT_AUDIO, SDL_INIT_HAPTIC, SDL_INIT_GAMECONTROLLER, SDL_INIT_SENSOR
	Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
	if (SDL_Init(sdl_flags) != 0)
		return NULL;
	if (param->title == NULL)
		param->title = "STUB";

	struct SDLStub* stub = &s_stub;
	qn_zero_1(stub);

	int wflags = SDL_WINDOW_OPENGL;
	if (QN_TEST_MASK(param->flags, QGFLAG_FULLSCREEN))
		wflags |= SDL_WINDOW_FULLSCREEN;
	if (QN_TEST_MASK(param->flags, QGFLAG_BORDERLESS))
		wflags |= SDL_WINDOW_BORDERLESS;
	if (QN_TEST_MASK(param->flags, QGFLAG_RESIZABLE))
		wflags |= SDL_WINDOW_RESIZABLE;
	if (QN_TEST_MASK(param->flags, QGFLAG_FOCUS))
		wflags |= SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;

	stub->window = SDL_CreateWindow(
		param->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		param->width, param->height, wflags);
	if (stub->window == NULL)
	{
		qn_debug_outputf(false, "SDL", "unable to initialize (%d)", SDL_GetError());
		return NULL;
	}

	SDL_GetWindowWMInfo(stub->window, &stub->wminfo);
	SDL_GetWindowSize(stub->window, &stub->base.size.x, &stub->base.size.y);
	SDL_GetWindowPosition(stub->window, &stub->base.bound.left, &stub->base.bound.top);
	stub->base.bound.right = stub->base.bound.left + stub->base.size.width;
	stub->base.bound.bottom = stub->base.bound.top + stub->base.size.height;

	stub->base.oshandle = stub->window;

	return (struct StubBase*)stub;
}

//
void stub_system_finalize(void)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;

	if (stub->window)
		SDL_DestroyWindow(stub->window);

	SDL_Quit();
}

//
static double _sdl_mesg_active(bool isactive)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	double now = qn_timer_get_abs(stub->base.timer);
	double adv = now - stub->base.active;
	stub->base.active = now;
	QN_SET_MASK(&stub->base.sttis, QGSTTI_ACTIVE, isactive);

	QgEvent e =
	{
		.active.ev = QGEV_ACTIVE,
		.active.active = isactive,
		.active.delta = adv,
	};
	qg_add_event(&e);

	return adv;
}

//
static void _sdl_mesg_layout(void)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	QnPoint prev = stub->base.size;

	SDL_GetWindowSize(stub->window, &stub->base.size.x, &stub->base.size.y);
	SDL_GetWindowPosition(stub->window, &stub->base.bound.left, &stub->base.bound.top);
	stub->base.bound.right = stub->base.bound.left + stub->base.size.width;
	stub->base.bound.bottom = stub->base.bound.top + stub->base.size.height;

	if (!qn_point_eq(&prev, &stub->base.size))
	{
		// 크기가 바꼈다 -> 트리거
		QgEvent e =
		{
			.layout.ev = QGEV_LAYOUT,
			.layout.bound = stub->base.bound,
		};
		qg_add_event(&e);
	}
}

//
static void _sdl_mesg_key(SDL_KeyboardEvent* se, bool isdown)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	QikMask mask = sdl_kmod_to_qikm(se->keysym.mod);
	stub->base.key.mask = mask;

	QikKey key = sdlk_to_qik(se->keysym.sym);
	qn_ret_if_fail(key != QIK_NONE);
	stub->base.key.key[key] = isdown;

	QgEvent e =
	{
		.key.ev = isdown ? QGEV_KEYDOWN : QGEV_KEYUP,
		.key.state = mask,
		.key.key = key,
		.key.pressed = se->state == SDL_PRESSED,
		.key.repeat = se->repeat,
	};
	qg_add_event(&e);
}

//
static void _sdl_set_mouse_point(int x, int y)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	QgUimMouse* m = &stub->base.mouse;
	m->last = m->pt;
	qn_point_set(&m->pt, x, y);
}

//
static QimButton _sdl_set_mouse_mask(int button, bool isdown)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	switch (button)
	{
		case SDL_BUTTON_LEFT:
			QN_SET_MASK(&stub->base.mouse.mask, QIMM_LEFT, isdown);
			return QIM_LEFT;

		case SDL_BUTTON_RIGHT:
			QN_SET_MASK(&stub->base.mouse.mask, QIMM_RIGHT, isdown);
			return QIM_RIGHT;

		case SDL_BUTTON_MIDDLE:
			QN_SET_MASK(&stub->base.mouse.mask, QIMM_MIDDLE, isdown);
			return QIM_MIDDLE;

		case SDL_BUTTON_X1:
			QN_SET_MASK(&stub->base.mouse.mask, QIMM_X1, isdown);
			return QIM_X1;

		case SDL_BUTTON_X2:
			QN_SET_MASK(&stub->base.mouse.mask, QIMM_X2, isdown);
			return QIM_X2;
	}
	return QIM_NONE;
}

//
static void _sdl_mesg_mouse_move(const SDL_MouseMotionEvent* se)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	_sdl_set_mouse_point(se->x, se->y);
	stub_internal_mouse_clicks(QIM_NONE, QIMT_MOVE);

	QgEvent e =
	{
		.mmove.ev = QGEV_MOUSEMOVE,
		.mmove.state = stub->base.mouse.mask,
		.mmove.x = se->x,
		.mmove.y = se->y,
		.mmove.dx = se->x - stub->base.mouse.last.x,
		.mmove.dy = se->y - stub->base.mouse.last.y,
	};
	qg_add_event(&e);
}

//
static void _sdl_mesg_mouse_button(const SDL_MouseButtonEvent* se, bool isdown)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	_sdl_set_mouse_point(se->x, se->y);

	QgEvent e =
	{
		.mbutton.button = _sdl_set_mouse_mask(se->button, isdown),
		.mbutton.state = stub->base.mouse.mask,
		.mbutton.x = se->x,
		.mbutton.y = se->y,
	};

	if (isdown)
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

//
static void _sdl_mesg_mouse_wheel(const SDL_MouseWheelEvent* se)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	stub->base.mouse.wheel = se->direction == SDL_MOUSEWHEEL_NORMAL ? 1 : -1;
	QgEvent e =
	{
		.mwheel.ev = QGEV_MOUSEWHEEL,
		.mwheel.dir = stub->base.mouse.wheel,
		.mwheel.x = se->x,
		.mwheel.y = se->y,
	};
	qg_add_event(&e);
}

//
bool stub_system_poll(void)
{
	struct SDLStub* stub = (struct SDLStub*)qg_stub_instance;
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)		// 포커스 잃음
					_sdl_mesg_active(true);
				else if (ev.window.event == SDL_WINDOWEVENT_FOCUS_LOST)		// 포커스 얻음
				{
					_sdl_mesg_active(false);

					QgEvent e =
					{
						.key.ev = QGEV_KEYUP,
						.key.state = stub->base.key.mask,
						.key.pressed = 0,
						.key.repeat = 0,
					};
					QgUimKey* k = &stub->base.key;
					for (int i = 0; i < (int)QN_COUNTOF(k->key); i++)
					{
						if (!k->key[i])
							continue;
						k->key[i] = false;
						e.key.key = (QikKey)i;
						qg_add_event(&e);
					}
				}
				else if (ev.window.event == SDL_WINDOWEVENT_RESIZED)		// 크기 변경
					_sdl_mesg_layout();
				break;

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

			case SDL_KEYDOWN:
				_sdl_mesg_key(&ev.key, true);
				break;

			case SDL_KEYUP:
				_sdl_mesg_key(&ev.key, false);
				break;

			case SDL_MOUSEMOTION:
				_sdl_mesg_mouse_move(&ev.motion);
				break;

			case SDL_MOUSEBUTTONDOWN:
				_sdl_mesg_mouse_button(&ev.button, true);
				break;

			case SDL_MOUSEBUTTONUP:
				_sdl_mesg_mouse_button(&ev.button, false);
				break;

			case SDL_MOUSEWHEEL:
				_sdl_mesg_mouse_wheel(&ev.wheel);
				break;

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

			case SDL_QUIT:
				QN_SET_MASK(&stub->base.sttis, QGSTTI_EXIT, true);
				qg_add_event_type(QGEV_EXIT);
				return false;
		}
	}
	return !QN_TEST_MASK(stub->base.sttis, QGSTTI_EXIT);
}

#endif	// USE_SDL
