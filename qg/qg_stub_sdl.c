#include "pch.h"
#include "qg.h"
#include "qg_kmc.h"
#include "qg_stub.h"
#include "SDL/SDL_syswm.h"
#include "SDL/SDL_keyboard.h"

//////////////////////////////////////////////////////////////////////////
// SDL 스터브
qvt_name(qgStub) _vt_sdl_stub =
{
	.base.name = "SDLRenderStub",
	.base.dispose = qg_stub_dispose,
};

typedef struct sdlStub
{
	qgStub				base;

	SDL_Window*			window;
	SDL_SysWMinfo		wminfo;

	double				active;
} sdlStub;

qgStub* qg_stub_new(const char* driver, const char* title, int width, int height, int flags)
{
	if (driver != NULL && qn_stricmp(driver, "sdl") == 0)
		return NULL;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		return NULL;

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

	sdlStub* self = qn_alloc_zero_1(sdlStub);
	qn_retval_if_fail(self, NULL);

	if (!_stub_on_init(self))
	{
		qn_free(self);
		return NULL;
	}

	if (!title)
		title = "QG stub";

	int wflags = SDL_WINDOW_OPENGL;
	if (QN_TEST_MASK(flags, QGFLAG_FULLSCREEN))
		wflags |= SDL_WINDOW_FULLSCREEN;
	if (QN_TEST_MASK(flags, QGFLAG_BORDERLESS))
		wflags |= SDL_WINDOW_BORDERLESS;
	if (QN_TEST_MASK(flags, QGFLAG_RESIZABLE))
		wflags |= SDL_WINDOW_RESIZABLE;
	if (QN_TEST_MASK(flags, QGFLAG_FOCUS))
		wflags |= SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;

	self->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, wflags);
	if (self->window == NULL)
	{
		qn_debug_output(false, "SDL: unable to initialize (%d)\n", SDL_GetError());
		qg_stub_dispose(self);
		return NULL;
	}

	SDL_GetWindowWMInfo(self->window, &self->wminfo);
#if _QN_WINDOWS_
	self->base.handle = self->wminfo.info.win.window;
#elif _QN_ANDROID_
	self->base.handle = self->wminfo.info.android.window;
#elif _QN_UNIX_
	self->base.handle = self->wminfo.info.x11.window;
#endif

	SDL_GetWindowSize(self->window, &self->base.size.width, &self->base.size.height);
	SDL_GetWindowPosition(self->window, &self->base.bound.left, &self->base.bound.top);
	self->base.bound.right = self->base.bound.left + self->base.size.width;
	self->base.bound.bottom = self->base.bound.top + self->base.size.height;

	SDL_Surface* surface = SDL_GetWindowSurface(self->window);

	self->base.flags = flags;
	self->base.stats = QGSTI_ACTIVE;

	self->active = qn_timer_get_abs(self->base.timer);

	return qm_init(self, &_vt_sdl_stub);
}

void qg_stub_dispose(pointer_t g)
{
	sdlStub* self = qm_cast(g, sdlStub);

	if (self->window)
		SDL_DestroyWindow(self->window);

	_stub_on_disp(self);
	qn_free(self);

	SDL_Quit();
}

bool qg_stub_close(pointer_t g)
{
	sdlStub* self = qm_cast(g, sdlStub);

	QN_SET_MASK(&self->base.stats, QGSTI_EXIT, true);

	return true;
}

static double _stub_update_active(sdlStub* self, bool isactive)
{
	double now = qn_timer_get_abs(self->base.timer);
	double adv = now - self->active;
	self->active = now;
	QN_SET_MASK(&self->base.stats, QGSTI_ACTIVE, isactive);

	qgEvent e =
	{
		.active.ev = QGEV_ACTIVE,
		.active.active = isactive,
		.active.delta = adv,
	};
	qg_stub_add_event(self, &e);

	return adv;
}

static void _stub_layout(sdlStub* self)
{
	qnPoint prev = self->base.size;

	SDL_GetWindowSize(self->window, &self->base.size.width, &self->base.size.height);
	SDL_GetWindowPosition(self->window, &self->base.bound.left, &self->base.bound.top);
	self->base.bound.right = self->base.bound.left + self->base.size.width;
	self->base.bound.bottom = self->base.bound.top + self->base.size.height;

	if (!qn_point_eq(&prev, &self->base.size))
	{
		// 크기가 바꼈다 -> 트리거
		qgEvent e =
		{
			.layout.ev = QGEV_LAYOUT,
			.layout.bound = self->base.bound,
		};
		qg_stub_add_event(self, &e);
	}
}

static void _stub_keyboard(sdlStub* self, SDL_KeyboardEvent* se, bool isdown)
{
	qIkMask mask = _sdl_kmod_to_qikm(se->keysym.mod);
	self->base.key.mask = mask;

	qIkKey key = _sdlk_to_qik(se->keysym.sym);
	qn_ret_if_fail(key != QIK_NONE);
	self->base.key.key[key] = isdown;

	qgEvent e =
	{
		.key.ev = isdown ? QGEV_KEYDOWN : QGEV_KEYUP,
		.key.state = mask,
		.key.key = key,
		.key.pressed = se->state == SDL_PRESSED,
		.key.repeat = se->repeat,
	};
	qg_stub_add_event(self, &e);
}

static void _stub_mouse_point(sdlStub* self, int x, int y)
{
	qgUimMouse* m = &self->base.mouse;
	m->last = m->pt;
	qn_point_set(&m->pt, x, y);
}

static qImButton _stub_mouse_mask(sdlStub* self, int button, bool isdown)
{
	switch (button)
	{
		case SDL_BUTTON_LEFT:
			QN_SET_MASK(&self->base.mouse.mask, QIMM_LEFT, isdown);
			return QIM_LEFT;

		case SDL_BUTTON_RIGHT:
			QN_SET_MASK(&self->base.mouse.mask, QIMM_RIGHT, isdown);
			return QIM_RIGHT;

		case SDL_BUTTON_MIDDLE:
			QN_SET_MASK(&self->base.mouse.mask, QIMM_MIDDLE, isdown);
			return QIM_MIDDLE;

		case SDL_BUTTON_X1:
			QN_SET_MASK(&self->base.mouse.mask, QIMM_X1, isdown);
			return QIM_X1;

		case SDL_BUTTON_X2:
			QN_SET_MASK(&self->base.mouse.mask, QIMM_X2, isdown);
			return QIM_X2;
	}
	return QIM_NONE;
}

static void _stub_mouse_move(sdlStub* self, const SDL_MouseMotionEvent* se)
{
	_stub_mouse_point(self, se->x, se->y);
	_stub_mouse_clicks(self, QIM_NONE, QIMT_MOVE);

	qgEvent e =
	{
		.mmove.ev = QGEV_MOUSEMOVE,
		.mmove.state = self->base.mouse.mask,
		.mmove.x = se->x,
		.mmove.y = se->y,
		.mmove.dx = se->x - self->base.mouse.last.x,
		.mmove.dy = se->y - self->base.mouse.last.y,
	};
	qg_stub_add_event(self, &e);
}

static void _stub_mouse_button(sdlStub* self, const SDL_MouseButtonEvent* se, bool isdown)
{
	_stub_mouse_point(self, se->x, se->y);

	qgEvent e =
	{
		.mbutton.button = _stub_mouse_mask(self, se->button, isdown),
		.mbutton.state = self->base.mouse.mask,
		.mbutton.x = se->x,
		.mbutton.y = se->y,
	};

	if (isdown)
	{
		bool doubleclick = _stub_mouse_clicks(self, e.mbutton.button, QIMT_DOWN);
		if (doubleclick)
		{
			e.mbutton.ev = QGEV_MOUSEDOUBLE;
			qg_stub_add_event(self, &e);
		}
		e.mbutton.ev = QGEV_MOUSEDOWN;
		qg_stub_add_event(self, &e);
	}
	else
	{
		e.mbutton.ev = QGEV_MOUSEUP;
		qg_stub_add_event(self, &e);
	}
}

static void _stub_mouse_wheel(sdlStub* self, const SDL_MouseWheelEvent* se)
{
	qgEvent e =
	{
		.mwheel.ev = QGEV_MOUSEWHEEL,
		.mwheel.dir = se->direction == SDL_MOUSEWHEEL_NORMAL ? 1 : -1,
		.mwheel.x = se->x,
		.mwheel.y = se->y,
	};
	qg_stub_add_event(self, &e);
}

bool _stub_poll(pointer_t g)
{
	sdlStub* self = qm_cast(g, sdlStub);

	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)		// 포커스 잃음
					_stub_update_active(self, true);
				else if (ev.window.event == SDL_WINDOWEVENT_FOCUS_LOST)		// 포커스 얻음
				{
					_stub_update_active(self, false);

					qgEvent e =
					{
						.key.ev = QGEV_KEYUP,
						.key.state = self->base.key.mask,
						.key.pressed = 0,
						.key.repeat = 0,
					};
					qgUimKey* k = &self->base.key;
					for (int i = 0; i < QN_COUNTOF(k->key); i++)
					{
						if (!k->key[i])
							continue;
						k->key[i] = false;
						e.key.key = (qIkKey)i;
						qg_stub_add_event(self, &e);
					}
				}
				else if (ev.window.event == SDL_WINDOWEVENT_RESIZED)		// 크기 변경
					_stub_layout(self);
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
				_stub_keyboard(self, &ev.key, true);
				break;

			case SDL_KEYUP:
				_stub_keyboard(self, &ev.key, false);
				break;

			case SDL_MOUSEMOTION:
				_stub_mouse_move(self, &ev.motion);
				break;

			case SDL_MOUSEBUTTONDOWN:
				_stub_mouse_button(self, &ev.button, true);
				break;

			case SDL_MOUSEBUTTONUP:
				_stub_mouse_button(self, &ev.button, false);
				break;

			case SDL_MOUSEWHEEL:
				_stub_mouse_wheel(self, &ev.wheel);
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
				QN_SET_MASK(&self->base.stats, QGSTI_EXIT, true);
				qg_stub_add_event_type(self, QGEV_EXIT);
				return false;
		}
	}

	if (QN_TEST_MASK(self->base.stats, QGSTI_EXIT))
		return false;

	self->base.polls++;

	return true;
}
