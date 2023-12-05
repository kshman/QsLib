#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qg_es2.h"
#include "qg_glsupp.h"

#if _MSC_VER
#pragma warning(disable: 4152)	//비표준 확장입니다. 식에서 함수/데이터 포인터 변환이 있습니다.
#endif

// GL 도움꾼
#if _QN_WINDOWS_
es2Func _es2func;
#define GLFUNC(f)		_es2func.##f
#else
#define GLFUNC(f)		f
#endif
#include "qg_glsupp.h"


//////////////////////////////////////////////////////////////////////////
// ES2 SDL 렌더 디바이스

static void _es2_dispose(QmGam* g);
static void _es2_reset(QgRdh* rdh);
static void _es2_clear(QgRdh* rdh, int flag, const QnColor* color, int stencil, float depth);

static bool _es2_begin(QgRdh* rdh, bool clear);
static void _es2_end(QgRdh* rdh);
static void _es2_flush(QgRdh* rdh);

static QgVlo* _es2_create_layout(QgRdh* rdh, int count, const QgPropLayout* layouts);
static QgShd* _es2_create_shader(QgRdh* rdh, const char* name);
static QgBuf* _es2_create_buffer(QgRdh* rdh, QgBufType type, int count, int stride, const void* data);

static void _es2_set_shader(QgRdh* rdh, QgShd* shader, QgVlo* layout);
static bool _es2_set_index(QgRdh* rdh, QgBuf* buffer);
static bool _es2_set_vertex(QgRdh* rdh, QgLoStage stage, QgBuf* buffer);

static bool _es2_draw(QgRdh* rdh, QgTopology tpg, int vcount);
static bool _es2_draw_indexed(QgRdh* rdh, QgTopology tpg, int icount);
static bool _es2_ptr_draw(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata);
static bool _es2_ptr_draw_indexed(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata);

qvt_name(QgRdh) _vt_es2 =
{
	.base.name = "ES2Device",
	.base.dispose = (paramfunc_t)_es2_dispose,

	.reset = _es2_reset,
	.clear = _es2_clear,

	.begin = _es2_begin,
	.end = _es2_end,
	.flush = _es2_flush,

	.create_layout = _es2_create_layout,
	.create_shader = _es2_create_shader,
	.create_buffer = _es2_create_buffer,

	.set_shader = _es2_set_shader,
	.set_index = _es2_set_index,
	.set_vertex = _es2_set_vertex,

	.draw = _es2_draw,
	.draw_indexed = _es2_draw_indexed,
	.ptr_draw = _es2_ptr_draw,
	.ptr_draw_indexed = _es2_ptr_draw_indexed,
};

#if USE_SDL
static SDL_Renderer* _es2_create_renderer(SDL_Window* window, int flags)
{
	// 렌더러 초기화
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

	//
	if (QN_TEST_MASK(flags, QGFLAG_DITHER))
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

	if (QN_TEST_MASK(flags, QGFLAG_MSAA))
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	}

	Uint32 sdl_flag = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
	if (QN_TEST_MASK(flags, QGFLAG_VSYNC))
		sdl_flag |= SDL_RENDERER_PRESENTVSYNC;

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, sdl_flag);
	if (renderer == NULL || SDL_GL_LoadLibrary(NULL) != 0)
	{
		qn_debug_outputf(true, "ES2Device", "failed to create renderer (%s)", SDL_GetError());
		if (renderer)
			SDL_DestroyRenderer(renderer);
		return NULL;
	}

#if _QN_WINDOWS_
#define DEF_GL_FUNC(ret, func, params)\
	_es2func.func = SDL_GL_GetProcAddress(#func);\
	if (!_es2func.func) goto pos_gl_func_error;
#include "qg_es2func.h"
#undef DEF_GL_FUNC
#endif

	return renderer;

pos_gl_func_error:
	SDL_DestroyRenderer(renderer);
	return NULL;
}
#endif

// 할당
QgRdh* es2_allocator(void* oshandle, int flags)
{
	es2Rdh* self = qn_alloc_zero_1(es2Rdh);
	qn_retval_if_fail(self, NULL);

#if USE_SDL
	SDL_Window* window = (SDL_Window*)oshandle;
	self->renderer = _es2_create_renderer(window, flags);

	// capa
	QgDeviceInfo* caps = &self->base.caps;

	SDL_RendererInfo info;
	SDL_GetRendererInfo(self->renderer, &info);

	qn_strncpy(caps->name, QN_COUNTOF(caps->name), info.name, QN_COUNTOF(caps->name)-1);
	gl_copy_string(caps->renderer, QN_COUNTOF(caps->renderer), GL_RENDERER);
	gl_copy_string(caps->vendor, QN_COUNTOF(caps->vendor), GL_VENDOR);
	caps->renderer_version = gl_get_version(GL_VERSION, "OPENGLES", "OPENGL ES");
	caps->shader_version = gl_get_version(GL_SHADING_LANGUAGE_VERSION, "OPENGL ES GLSL ES ", "OPENGL ES GLSL ");

	caps->max_vertex_attrs = gl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);
	if (caps->max_vertex_attrs > ES2_MAX_VERTEX_ATTRIBUTES)
		caps->max_vertex_attrs = ES2_MAX_VERTEX_ATTRIBUTES;
	caps->max_tex_dim = info.max_texture_width;
	caps->max_tex_count = gl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	caps->max_off_count = /*임시*/1;
	caps->tex_image_flag = /*임시*/0;

	SDL_Surface* surface = SDL_GetWindowSurface(window);
	caps->clrfmt = surface->format->BitsPerPixel == 16 ?
		surface->format->Amask != 0 ? QGCF16_RGBA : QGCF16_RGB :
		surface->format->Amask != 0 ? QGCF32_RGBA : QGCF32_RGB;
#endif	// USE_SDL

	self->ss.scissor = false;

	// 세이더 자동 유니폼
	es2shd_init_auto_uniforms();

	return qm_init(self, QgRdh, &_vt_es2);
}

// 끝장내
static void _es2_dispose(QmGam* g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);

	// 펜딩
	qm_unload(self->pd.shd);
	qm_unload(self->pd.vlo);

	qm_unload(self->pd.ib);
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qm_unload(self->pd.vb[i]);

	// SDL 렌더러
	if (self->renderer)
		SDL_DestroyRenderer(self->renderer);

	rdh_internal_dispose(g);
}

// 리셋
static void _es2_reset(QgRdh* rdh)
{
	rdh_internal_reset(rdh);

	//es2Rdh* self = qm_cast(rdh, es2Rdh);
	const QgDeviceInfo* caps = &rdh->caps;
	QgRenderTm* tm = &rdh->tm;

	//
	qn_mat4_ortho_lh(&tm->ortho, (float)tm->size.width, (float)tm->size.height, -1.0f, 1.0f);
	qn_mat4_loc(&tm->ortho, -1.0f, 1.0f, 0.0f, false);
	gl_mat4_irrcht_texture(&tm->frm, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//
	GLFUNC(glPixelStorei)(GL_PACK_ALIGNMENT, 1);
	GLFUNC(glPixelStorei)(GL_UNPACK_ALIGNMENT, 1);

	GLFUNC(glEnable)(GL_DEPTH_TEST);
	GLFUNC(glDepthMask)(GL_TRUE);
	GLFUNC(glDepthFunc)(GL_LEQUAL);
	GLFUNC(glDisable)(GL_DEPTH_TEST);

	GLFUNC(glEnable)(GL_CULL_FACE);
	GLFUNC(glCullFace)(GL_BACK);

	GLFUNC(glDisable)(GL_POLYGON_OFFSET_FILL);

	//for (int i = 0; i < caps->max_off_count; i++)	// 오프가 1개뿐이다. 어짜피 관련 함수에 인덱스 넣는 곳도 없음
	{
		GLFUNC(glEnable)(GL_BLEND);
		GLFUNC(glBlendEquationSeparate)(GL_FUNC_ADD, GL_FUNC_ADD);
		GLFUNC(glBlendFuncSeparate)(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

		GLFUNC(glDisable)(GL_BLEND);
		GLFUNC(glColorMask)(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	for (int i = 0; i < caps->max_tex_count; i++)
	{
		GLFUNC(glActiveTexture)(GL_TEXTURE0 + i);
		GLFUNC(glBindTexture)(GL_TEXTURE_2D, 0);
	}

	GLFUNC(glDisable)(GL_SCISSOR_TEST);
	GLFUNC(glFrontFace)(GL_CW);
}

// 시작
static bool _es2_begin(QgRdh* rdh, bool clear)
{
	//es2Rdh* self = qm_cast(rdh, es2Rdh);
	if (clear)
		_es2_clear(rdh, QGCLR_DEPTH | QGCLR_STENCIL | QGCLR_RENDER, &rdh->param.bgc, 0, 1.0f);
	return true;
}

// 끝
static void _es2_end(QgRdh* rdh)
{
}

// 플러시
static void _es2_flush(QgRdh* rdh)
{
	GLFUNC(glFlush)();
	SDL_GL_SwapWindow((SDL_Window*)qg_stub_instance->oshandle);
}

// 지우기
static void _es2_clear(QgRdh* rdh, int flag, const QnColor* color, int stencil, float depth)
{
	// 도움: https://open.gl/depthstencils
	GLbitfield cf = 0;

	if (QN_TEST_MASK(flag, QGCLR_STENCIL))
	{
		GLFUNC(glStencilMaskSeparate)(GL_FRONT_AND_BACK, stencil);
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TEST_MASK(flag, QGCLR_DEPTH))
	{
		GLFUNC(glDepthMask)(GL_TRUE);
		GLFUNC(glClearDepthf)(depth);
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TEST_MASK(flag, QGCLR_RENDER))
	{
		if (color == NULL)
			color = &rdh->param.bgc;
		GLFUNC(glClearColor)(color->r, color->g, color->b, color->a);
		cf |= GL_COLOR_BUFFER_BIT;
	}

	if (cf != 0)
		GLFUNC(glClear)(cf);
}

// 세이더 설정
static void _es2_set_shader(QgRdh* rdh, QgShd* shader, QgVlo* layout)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Shd* shd = qm_cast(shader, es2Shd);
	es2Vlo* vlo = qm_cast(layout, es2Vlo);

	if (shd == NULL)
	{
		if (self->pd.shd != NULL)
		{
			qm_unload(self->pd.shd);
			self->pd.shd = NULL;
		}
	}
	else
	{
		if (self->pd.shd != shd)
		{
			qm_unload(self->pd.shd);
			self->pd.shd = shd;
			qm_load(shd);
		}
	}

	if (vlo == NULL)
	{
		if (self->pd.vlo != NULL)
		{
			qm_unload(self->pd.vlo);
			self->pd.vlo = NULL;
		}
	}
	else
	{
		if (self->pd.vlo != vlo)
		{
			qm_unload(self->pd.vlo);
			self->pd.vlo = vlo;
			qm_load(vlo);
		}
	}
}

// 인덱스 설정
static bool _es2_set_index(QgRdh* rdh, QgBuf* buffer)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Buf* buf = qm_cast(buffer, es2Buf);

	if (buf == NULL)
	{
		if (self->pd.ib != NULL)
		{
			qm_unload(self->pd.ib);
			self->pd.ib = NULL;
		}
	}
	else
	{
		qn_retval_if_fail(buf->base.type == QGBUF_INDEX, false);

		if (self->pd.ib != buf)
		{
			qm_unload(self->pd.ib);
			self->pd.ib = buf;
			qm_load(buf);
		}
	}

	return true;
}

// 정점 설정
static bool _es2_set_vertex(QgRdh* rdh, QgLoStage stage, QgBuf* buffer)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Buf* buf = qm_cast(buffer, es2Buf);

	if (buf == NULL)
	{
		if (self->pd.vb[stage] != NULL)
		{
			qm_unload(self->pd.vb[stage]);
			self->pd.vb[stage] = NULL;
		}
	}
	else
	{
		qn_retval_if_fail(buf->base.type == QGBUF_VERTEX, false);

		if (self->pd.vb[stage] != buf)
		{
			qm_unload(self->pd.vb[stage]);
			self->pd.vb[stage] = buf;
			qm_load(buf);
		}
	}

	return true;
}

// 레이아웃 만들기
static QgVlo* _es2_create_layout(QgRdh* rdh, int count, const QgPropLayout* layouts)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Vlo* vlo = es2vlo_allocator(self, count, layouts);
	// 해야함: 관리
	return qm_cast(vlo, QgVlo);
}

// 세이더 만들기
static QgShd* _es2_create_shader(QgRdh* rdh, const char* name)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Shd* shd = es2shd_allocator(self, name);
	// 해야함: 관리
	return qm_cast(shd, QgShd);
}

// 버퍼 만들기
static QgBuf* _es2_create_buffer(QgRdh* rdh, QgBufType type, int count, int stride, const void* data)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Buf* buf = es2buf_allocator(self, type, count, stride, data);
	// 해야함: 관리
	return qm_cast(buf, QgBuf);
}

// 토폴로지 변환
static GLenum es2_conv_topology(QgTopology tpg)
{
	static GLenum s_gl_tpg[] =
	{
		GL_POINTS,
		GL_LINES,
		GL_LINE_STRIP,
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_LINE_LOOP,
		GL_TRIANGLE_FAN,
	};
	return s_gl_tpg[tpg];
}

// 그리기
static bool _es2_draw(QgRdh* rdh, QgTopology tpg, int vcount)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Pending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd, false);

	es2_commit_shader(self);
	es2_commit_layout(self);

	GLenum gl_tpg = es2_conv_topology(tpg);
	GLFUNC(glDrawArrays)(gl_tpg, 0, (GLsizei)vcount);
	return true;
}

// 그리기 인덱스
static bool _es2_draw_indexed(QgRdh* rdh, QgTopology tpg, int icount)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Pending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd && pd->ib, false);

	GLenum gl_index =
		pd->ib->base.stride == sizeof(ushort) ? GL_UNSIGNED_SHORT :
		pd->ib->base.stride == sizeof(uint) ? GL_UNSIGNED_INT : 0;
	qn_retval_if_fail(gl_index != 0, false);
	es2_bind_buffer(self, GL_ELEMENT_ARRAY_BUFFER, qm_get_desc(pd->ib, GLuint));

	es2_commit_shader(self);
	es2_commit_layout(self);

	GLenum gl_tpg = es2_conv_topology(tpg);
	GLFUNC(glDrawElements)(gl_tpg, (GLint)icount, gl_index, NULL);
	return true;
}

// 포인터 데이터로 그리기
static bool _es2_ptr_draw(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata)
{
	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Pending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd, false);

	es2_commit_shader(self);
	es2_commit_layout_ptr(self, vdata, vstride);

	GLenum gl_tpg = es2_conv_topology(tpg);
	GLFUNC(glDrawArrays)(gl_tpg, 0, (GLsizei)vcount);
	return true;
}

// 포인터 데이터로 그리기 인덱스
static bool _es2_ptr_draw_indexed(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata)
{
	GLenum gl_index;
	if (istride == sizeof(ushort)) gl_index = GL_UNSIGNED_SHORT;
	else if (istride == sizeof(uint)) gl_index = GL_UNSIGNED_INT;
	else return false;

	es2Rdh* self = qm_cast(rdh, es2Rdh);
	es2Pending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd, false);

	es2_commit_shader(self);
	es2_commit_layout_ptr(self, vdata, vstride);

	GLenum gl_tpg = es2_conv_topology(tpg);
	GLFUNC(glDrawElements)(gl_tpg, (GLint)icount, gl_index, idata);
	return true;
}

