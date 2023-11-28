#include "pch.h"
#include "qg.h"
#include "SDL/SDL_syswm.h"
#include "SDL/SDL_keyboard.h"

//////////////////////////////////////////////////////////////////////////
// SDL 스터브
struct _qgvt_stub _vt_sdl_stub =
{
	.base.name = "SDLRenderStub",
	.base.dispose = qg_stub_dispose,
};

typedef struct sdlStub
{
	qgStub			base;

	SDL_Window*		window;
	SDL_SysWMinfo	wminfo;
} sdlStub;

qgStub* qg_stub_new(const char* driver, const char* title, int width, int height, int flags)
{
	if (driver != NULL && qn_stricmp(driver, "sdl") == 0)
		return NULL;

	sdlStub* self = qn_alloc_zero_1(sdlStub);
	qn_retval_if_fail(self, NULL);

	if (!qg_stub_on_init(self))
	{
		qn_free(self);
		return NULL;
	}

	if (!title)
		title = "QG stub";

	int wflags = 0;
	if (QN_BIT_TEST(flags, QGSTUB_FULLSCREEN))
		wflags |= SDL_WINDOW_FULLSCREEN;
	if (QN_BIT_TEST(flags, QGSTUB_BORDERLESS))
		wflags |= SDL_WINDOW_BORDERLESS;
	if (QN_BIT_TEST(flags, QGSTUB_RESIZABLE))
		wflags |= SDL_WINDOW_RESIZABLE;
	if (QN_BIT_TEST(flags, QGSTUB_FOCUS))
		wflags |= SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;

	self->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, wflags);
	if (self->window == NULL)
	{
		qn_debug_output(false, "SDL: unable to initialize (%d)\n", SDL_GetError());
		qg_stub_dispose(self);
	}

	SDL_GetWindowWMInfo(self->window, &self->wminfo);
#if _QN_WINDOWS_
	self->base.handle = self->wminfo.info.win.window;
#elif _QN_ANDROID_
	self->base.handle = self->wminfo.info.android.window;
#elif _QN_UNIX_
	self->base.handle = self->wminfo.info.x11.window;
#endif

	qn_set_vt(self, &_vt_sdl_stub);
	return qn_gam(self, qgStub);
}

void qg_stub_dispose(pointer_t g)
{
	sdlStub* self = qn_gam(g, sdlStub);

	if (self->window)
		SDL_DestroyWindow(self->window);

	qg_stub_on_disp(self);
	qn_free(self);
}
