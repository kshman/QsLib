#include "pch.h"
#include "qg.h"
#include "qg_stub.h"
#include "qg_es2.h"

// 함수
#if _QN_WINDOWS_
es2Func _es2func;
#define ES2FUNC(f)		_es2func.##f
#else
#define ES2FUNC(f)		f
#endif

//////////////////////////////////////////////////////////////////////////
// ES2 SDL 렌더 디바이스

bool _es2_construct(pointer_t g, int flags);
void _es2_finalize(pointer_t g);
void _es2_reset(pointer_t g);

bool _es2_begin(pointer_t g);
void _es2_end(pointer_t g);
void _es2_flush(pointer_t g);

bool _es2_primitive_begin(pointer_t g, qgTopology tpg, int count, int stride, pointer_t* pptr);
void _es2_primitive_end(pointer_t g);
bool _es2_indexed_primitive_begin(pointer_t g, qgTopology tpg, int vcount, int vstride, pointer_t* pvptr, int icount, int istride, pointer_t* piptr);
void _es2_indexed_primitive_end(pointer_t g);

qvt_name(qgRdh) _vt_es2 =
{
	.base.name = "GLES2Device",
	.base.dispose = _rdh_dispose,

	._construct = _es2_construct,
	._finalize = _es2_finalize,
	._reset = _es2_reset,

	.begin = _es2_begin,
	.end = _es2_end,
	.flush = _es2_flush,

	.primitive_begin = _es2_primitive_begin,
	.primitive_end = _es2_primitive_end,
	.indexed_primitive_begin = _es2_indexed_primitive_begin,
	.indexed_primitive_end = _es2_indexed_primitive_end,
};

pointer_t _es2_allocator()
{
	es2Rdh* self = qn_alloc_zero_1(es2Rdh);
	qn_retval_if_fail(self, NULL);
	return qm_init(self, &_vt_es2);
}

static int _es2_get_version(GLenum name, const char* name1, const char* name2)
{
	const char* s = (const char*)ES2FUNC(glGetString)(name);
	float f =
		qn_strnicmp(s, name1, strlen(name1)) == 0 ? (float)atof(s + strlen(name1)) :
		qn_strnicmp(s, name2, strlen(name2)) == 0 ? (float)atof(s + strlen(name2)) :
		(float)atof(s);
	return (int)(floor(f) * 100.0f + (QN_FRACT(f) * 10.0));
}

bool _es2_construct(pointer_t g, int flags)
{
	es2Rdh* self = qm_cast(g, es2Rdh);

	// 렌더러 초기화
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

	//
	int bpp = QN_TEST_MASK(flags, QGFLAG_DITHER) ? 16 : 32;
	if (bpp == 16)
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

	// SDL_RENDERER_PRESENTVSYNC -> SDL_GL_SetSwapInterval(1)
	SDL_Window* window = (SDL_Window*)self->base.stub->handle;
	self->renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if (self->renderer == NULL)
	{
		qn_debug_output(true, "ES2: failed to create renderer (%s)\n", SDL_GetError());
		return false;
	}

	// 프로시져
	if (SDL_GL_LoadLibrary(NULL) != 0)
	{
		qn_debug_output(true, "ES2: failed to load library (%s)\n", SDL_GetError());
		return false;
	}

#if _QN_WINDOWS_
#define DEF_ES2_FUNC(ret, func, params)\
	QN_STMT_BEGIN{\
		_es2func.func = SDL_GL_GetProcAddress(#func);\
		if (!_es2func.func) {\
			qn_debug_output(true, "ES2: function load failed '%s'\n", #func);\
			return false;\
		}\
	}QN_STMT_END;
#include "qg_es2func.h"
#undef DEF_ES2_FUNC
#endif

	// capa
	qgDeviceInfo* caps = &self->base.caps;
	SDL_RendererInfo info;
	SDL_GetRendererInfo(self->renderer, &info);
	qn_strncpy(caps->name, 64, info.name, 63);
	qn_strncpy(caps->renderer, 64, (const char*)ES2FUNC(glGetString)(GL_RENDERER), 63);
	qn_strncpy(caps->vendor, 64, (const char*)ES2FUNC(glGetString)(GL_VENDOR), 63);
	caps->renderer_version = _es2_get_version(GL_VERSION, "OPENGLES", "OPENGL ES");
	caps->shader_version = _es2_get_version(GL_SHADING_LANGUAGE_VERSION, "OPENGL ES GLSL ES ", "OPENGL ES GLSL ");
	caps->max_tex_dim = info.max_texture_width;
	caps->max_tex_count = /*임시*/1;
	caps->max_off_count = /*임시*/1;
	caps->tex_image_flag = /*임시*/0;

	// 초기화
	self->ss.program = GL_INVALID_VALUE;
	self->ss.buf_array = GL_INVALID_VALUE;
	self->ss.buf_element_array = GL_INVALID_VALUE;
	self->ss.buf_pixel_unpack = GL_INVALID_VALUE;

	self->ss.scissor = false;

	return true;
}

void _es2_finalize(pointer_t g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);

	if (self->renderer)
		SDL_DestroyRenderer(self->renderer);
}

void _es2_reset(pointer_t g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
	qgDeviceInfo* caps = &self->base.caps;
	qgRenderTm* tm = &self->base.tm;

	//
	qn_mat4_ortho_lh(&tm->ortho, (float)self->base.stub->size.width, (float)self->base.stub->size.height, -1.0f, 1.0f);
	tm->ortho._41 = -1.0f; tm->ortho._42 = 1.0f; tm->ortho._43 = 0.0f;
	_es2_mat4_tex_form(&tm->frm, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//
	ES2FUNC(glPixelStorei)(GL_PACK_ALIGNMENT, 1);

	ES2FUNC(glEnable)(GL_DEPTH_TEST);
	ES2FUNC(glDepthMask)(GL_TRUE);
	ES2FUNC(glDepthFunc)(GL_LEQUAL);
	ES2FUNC(glDisable)(GL_DEPTH_TEST);

	ES2FUNC(glEnable)(GL_CULL_FACE);
	ES2FUNC(glCullFace)(GL_BACK);

	ES2FUNC(glDisable)(GL_POLYGON_OFFSET_FILL);

	//for (int i = 0; i < caps->max_off_count; i++)	// 오프가 1개뿐이다
	{
		ES2FUNC(glEnable)(GL_BLEND);
		ES2FUNC(glBlendEquationSeparate)(GL_FUNC_ADD, GL_FUNC_ADD);
		ES2FUNC(glBlendFuncSeparate)(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

		ES2FUNC(glDisable)(GL_BLEND);
		ES2FUNC(glColorMask)(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	for (int i = 0; i < caps->max_tex_count; i++)
	{
		ES2FUNC(glActiveTexture)(GL_TEXTURE0 + i);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		ES2FUNC(glBindTexture)(GL_TEXTURE_2D, 0);
	}

	ES2FUNC(glDisable)(GL_SCISSOR_TEST);
	ES2FUNC(glFrontFace)(GL_CW);
	//glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
}

bool _es2_begin(pointer_t g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);

	/* 이부분은 clear로 빼야함 */
	GLbitfield cf = 0;

	ES2FUNC(glStencilMaskSeparate)(GL_FRONT_AND_BACK, 0xFFFFFFFF);
	cf |= GL_STENCIL_BUFFER_BIT;

	ES2FUNC(glDepthMask)(GL_TRUE);
	ES2FUNC(glClearDepthf)(1.0f);
	cf |= GL_DEPTH_BUFFER_BIT;

	qnColor* cc = &self->base.param.clear;
	ES2FUNC(glClearColor)(cc->r, cc->g, cc->b, cc->a);
	cf |= GL_COLOR_BUFFER_BIT;

	if (cf != 0)
		ES2FUNC(glClear)(cf);
	/* 여기까지 */

	ES2FUNC(glFrontFace)(GL_CW);

	return true;
}

void _es2_end(pointer_t g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
}

void _es2_flush(pointer_t g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);

	ES2FUNC(glFlush)();
	SDL_GL_SwapWindow((SDL_Window*)self->base.stub->handle);
}

bool _es2_primitive_begin(pointer_t g, qgTopology tpg, int count, int stride, pointer_t* pptr)
{
	return false;
}

void _es2_primitive_end(pointer_t g)
{
}

bool _es2_indexed_primitive_begin(pointer_t g, qgTopology tpg, int vcount, int vstride, pointer_t* pvptr, int icount, int istride, pointer_t* piptr)
{
	return false;
}

void _es2_indexed_primitive_end(pointer_t g)
{
}

