#include "pch.h"
#include "qg.h"
#include "qg_internal.h"
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
	if (QN_TEST_MASK(flags, QGSTUB_FULLSCREEN))
		wflags |= SDL_WINDOW_FULLSCREEN;
	if (QN_TEST_MASK(flags, QGSTUB_BORDERLESS))
		wflags |= SDL_WINDOW_BORDERLESS;
	if (QN_TEST_MASK(flags, QGSTUB_RESIZABLE))
		wflags |= SDL_WINDOW_RESIZABLE;
	if (QN_TEST_MASK(flags, QGSTUB_FOCUS))
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

	// 트리거

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
	}
}

static void _stub_mouse_point(sdlStub* self, int x, int y)
{
	qgUimMouse* m = &self->base.mouse;
	m->last = m->pt;
	qn_point_set(&m->pt, x, y);
}

static qImButton _stub_mouse_button(sdlStub* self, int button, bool isdown)
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
				{
					_stub_update_active(self, true);
					// 액티브 트리거
				}
				else if (ev.window.event == SDL_WINDOWEVENT_FOCUS_LOST)		// 포커스 얻음
				{
					_stub_update_active(self, false);
					// 액티브 트리거

					qgUimKey* k = &self->base.key;
					for (int i = 0; i < QN_COUNTOF(k->key); i++)
					{
						if (!k->key[i])
							continue;
						k->key[i] = false;
						// 키 놓임 트리거
					}
				}
				else if (ev.window.event == SDL_WINDOWEVENT_RESIZED)		// 크기 변경
				{
					_stub_layout(self);
					// 레이아웃 트리거
				}
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
				self->base.key.mask = _sdl_kmod_to_qikm(ev.key.keysym.mod);
				QN_STMT_BEGIN{
					qIkKey key = _sdlk_to_qik(ev.key.keysym.sym);
					self->base.key.key[key] = true;
					// 키 눌림 트리거
				}QN_STMT_END;
				break;

			case SDL_KEYUP:
				self->base.key.mask = _sdl_kmod_to_qikm(ev.key.keysym.mod);
				QN_STMT_BEGIN{
					qIkKey key = _sdlk_to_qik(ev.key.keysym.sym);
					self->base.key.key[key] = false;
					// 키 놓임 트리거
				}QN_STMT_END;
				break;

			case SDL_MOUSEMOTION:
				_stub_mouse_point(self, ev.motion.x, ev.motion.y);
				_stub_mouse_clicks(self, QIM_NONE, QIMT_MOVE);
				// 마우스 이동 트리거
				break;

			case SDL_MOUSEBUTTONDOWN:
				_stub_mouse_point(self, ev.button.x, ev.button.y);
				QN_STMT_BEGIN{
					qImButton btn = _stub_mouse_button(self, ev.button.button, true);
					bool doubleclick = _stub_mouse_clicks(self, btn, QIMT_DOWN);
					// 더블클릭이 참이면 더블 클릭 트리거
					// 마우스 눌림 트리거
				}QN_STMT_END;
				break;

			case SDL_MOUSEBUTTONUP:
				_stub_mouse_point(self, ev.button.x, ev.button.y);
				QN_STMT_BEGIN{
					qImButton btn = _stub_mouse_button(self, ev.button.button, false);
					bool doubleclick = _stub_mouse_clicks(self, btn, QIMT_UP);
					// 마우스 놓임 트리거
				}QN_STMT_END;
				break;

			case SDL_MOUSEWHEEL:
				// 해야함
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
				return false;
		}
	}

	if (QN_TEST_MASK(self->base.stats, QGSTI_EXIT))
		return false;

	self->base.polls++;

	return true;
}
