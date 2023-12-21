#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qg_es2.h"
#include "qg_glsupp.h"

#define QGMAX_ES2_VERTEX_ATTRIBUTE		16


//////////////////////////////////////////////////////////////////////////
// ES2 렌더 디바이스

static void es2_dispose(QsGam* g);
static void es2_reset(QgRdh* rdh);

qv_name(QgRdh) vt_es2_rdh =
{
	.base.name = "ES2Device",
	.base.dispose = es2_dispose,

	.reset = es2_reset,
	.clear = gl_clear,

	.begin = gl_begin,
	.end = gl_end,
	.flush = gl_flush,

	.create_layout = gl_create_layout,
	.create_shader = gl_create_shader,
	.create_buffer = gl_create_buffer,
	.create_depth_stencil = gl_create_depth_stencil,
	.create_rasterizer = gl_create_rasterizer,

	.set_shader = gl_set_shader,
	.set_index = gl_set_index,
	.set_vertex = gl_set_vertex,
	.set_depth_stencil = gl_set_depth_stencil,
	.set_rasterizer = gl_set_rasterizer,

	.draw = gl_draw,
	.draw_indexed = gl_draw_indexed,
	.ptr_draw = gl_ptr_draw,
	.ptr_draw_indexed = gl_ptr_draw_indexed,
};

static SDL_Renderer* es2_create_renderer(SDL_Window* window, const int flags)
{
	// 렌더러 초기화
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

	//
	if (QN_TMASK(flags, QGFLAG_DITHER))
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 4);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 4);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 4);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	if (QN_TMASK(flags, QGFLAG_MSAA))
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	}

	Uint32 sdl_flag = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
	if (QN_TMASK(flags, QGFLAG_VSYNC))
		sdl_flag |= SDL_RENDERER_PRESENTVSYNC;

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, sdl_flag);
	if (renderer == NULL || SDL_GL_LoadLibrary(NULL) != 0)
	{
		qn_debug_outputf(true, "ES2Device", "failed to create renderer (%s)", SDL_GetError());
		if (renderer)
			SDL_DestroyRenderer(renderer);
		return NULL;
	}

	// GL 함수 초기화
	if (!gl_init_func())
	{
		SDL_DestroyRenderer(renderer);
		return NULL;
	}

	return renderer;
}

// 할당
QgRdh* es2_allocator(void* oshandle, const int flags)
{
	Es2Rdh* self = qn_alloc_zero_1(Es2Rdh);
	qn_val_if_fail(self, NULL);

	SDL_Window* window = (SDL_Window*)oshandle;
	SDL_Renderer* renderer = es2_create_renderer(window, flags);
	if (renderer == NULL)
	{
		qn_free(self);
		return NULL;
	}

	gl_initialize(qs_cast(self, GlRdhBase), window, renderer, flags);

	// capa
	QgDeviceInfo* caps = &rdh_caps(self);
	caps->renderer_version = gl_get_version(GL_VERSION, "OPENGLES", "OPENGL ES");
	caps->shader_version = gl_get_version(GL_SHADING_LANGUAGE_VERSION, "OPENGL ES GLSL ES ", "OPENGL ES GLSL ");
	caps->max_off_count = 1;

	if (caps->max_vertex_attrs > QGMAX_ES2_VERTEX_ATTRIBUTE)
		caps->max_vertex_attrs = QGMAX_ES2_VERTEX_ATTRIBUTE;

	// session
	GlSession* ss = &self->base.ss;
	ss->layout.count = caps->max_vertex_attrs;
	ss->layout.props = qn_alloc_zero(caps->max_vertex_attrs, GlLayoutProp);

	es2_reset(qs_cast(self, QgRdh));
	return qs_init(self, QgRdh, &vt_es2_rdh);
}

// 끝장내
static void es2_dispose(QsGam* g)
{
	const Es2Rdh* self = qs_cast(g, Es2Rdh);

	gl_finalize(qs_cast(self, GlRdhBase));

	// SDL 렌더러
	if (self->base.renderer)
		SDL_DestroyRenderer(self->base.renderer);

	rdh_internal_dispose(g);
}

// 리셋
static void es2_reset(QgRdh* rdh)
{
	gl_reset(rdh);

	const QgDeviceInfo* caps = &rdh->caps;

	//for (int i = 0; i < caps->max_off_count; i++)	// 오프가 1개뿐이다. 어짜피 관련 함수에 인덱스 넣는 곳도 없음
	{
		GL_FUNC(glEnable)(GL_BLEND);
		GL_FUNC(glBlendEquationSeparate)(GL_FUNC_ADD, GL_FUNC_ADD);
		GL_FUNC(glBlendFuncSeparate)(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

		GL_FUNC(glDisable)(GL_BLEND);
		GL_FUNC(glColorMask)(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	for (int i = 0; i < caps->max_tex_count; i++)
	{
		GL_FUNC(glActiveTexture)(GL_TEXTURE0 + i);
		GL_FUNC(glBindTexture)(GL_TEXTURE_2D, 0);
	}
}
