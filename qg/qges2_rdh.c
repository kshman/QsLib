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

static bool _es2_construct(qgRdh* g, int flags);
static void _es2_finalize(qgRdh* g);
static void _es2_reset(qgRdh* g);
static void _es2_clear(qgRdh* g, int flag, const qnColor* color, int stencil, float depth);

static bool _es2_begin(qgRdh* g);
static void _es2_end(qgRdh* g);
static void _es2_flush(qgRdh* g);

static qgVlo* _es2_create_layout(qgRdh* g, int count, const qgVarLayout* vars);
static qgShd* _es2_create_shader(qgRdh* g, const char* name);
static qgBuf* _es2_create_buffer(qgRdh* g, qgBufType type, int count, int stride, cpointer_t data);

static void _es2_set_shader(qgRdh* g, qgShd* shader, qgVlo* layout);
static bool _es2_set_index(qgRdh* g, qgBuf* buffer);
static bool _es2_set_vertex(qgRdh* g, int stage, qgBuf* buffer);

static bool _es2_primitive_begin(qgRdh* g, qgTopology tpg, int count, int stride, pointer_t* pptr);
static void _es2_primitive_end(qgRdh* g);
static bool _es2_indexed_primitive_begin(qgRdh* g, qgTopology tpg, int vcount, int vstride, pointer_t* pvptr, int icount, int istride, pointer_t* piptr);
static void _es2_indexed_primitive_end(qgRdh* g);
static bool _es2_draw(qgRdh* g, qgTopology tpg, int vcount);
static bool _es2_draw_indexed(qgRdh* g, qgTopology tpg, int icount);

qvt_name(qgRdh) _vt_es2 =
{
	.base.name = "GLES2Device",
	.base.dispose = _rdh_dispose,

	._construct = _es2_construct,
	._finalize = _es2_finalize,
	._reset = _es2_reset,
	._clear = _es2_clear,

	.begin = _es2_begin,
	.end = _es2_end,
	.flush = _es2_flush,

	.create_layout = _es2_create_layout,
	.create_shader = _es2_create_shader,
	.create_buffer = _es2_create_buffer,

	.set_shader = _es2_set_shader,
	.set_index = _es2_set_index,
	.set_vertex = _es2_set_vertex,

	.primitive_begin = _es2_primitive_begin,
	.primitive_end = _es2_primitive_end,
	.indexed_primitive_begin = _es2_indexed_primitive_begin,
	.indexed_primitive_end = _es2_indexed_primitive_end,
	.draw = _es2_draw,
	.draw_indexed = _es2_draw_indexed,
};

// 할당
qgRdh* _es2_allocator()
{
	es2Rdh* self = qn_alloc_zero_1(es2Rdh);
	qn_retval_if_fail(self, NULL);
	return qm_init(self, &_vt_es2);
}

// 버전 문자열에서 숫자만
static int es2_gl_get_version(GLenum name, const char* name1, const char* name2)
{
	const char* s = (const char*)ES2FUNC(glGetString)(name);
	float f =
		qn_strnicmp(s, name1, strlen(name1)) == 0 ? (float)atof(s + strlen(name1)) :
		qn_strnicmp(s, name2, strlen(name2)) == 0 ? (float)atof(s + strlen(name2)) :
		(float)atof(s);
	return (int)(floor(f) * 100.0f + (QN_FRACT(f) * 10.0));
}

// 값
static int es2_gl_get_integer(GLenum name)
{
	GLint n;
	ES2FUNC(glGetIntegerv)(name, &n);
	return n;
}

// 컨스트럭터
static bool _es2_construct(qgRdh* g, int flags)
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
	SDL_Window* window = (SDL_Window*)qm_get_desc(self->base.stub);
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
	caps->renderer_version = es2_gl_get_version(GL_VERSION, "OPENGLES", "OPENGL ES");
	caps->shader_version = es2_gl_get_version(GL_SHADING_LANGUAGE_VERSION, "OPENGL ES GLSL ES ", "OPENGL ES GLSL ");
	caps->max_vertex_attrs = es2_gl_get_integer(GL_MAX_VERTEX_ATTRIBS);
	if (caps->max_vertex_attrs > ES2_MAX_VERTEXT_ATTRS)
		caps->max_vertex_attrs = ES2_MAX_VERTEXT_ATTRS;
	caps->max_tex_dim = info.max_texture_width;
	caps->max_tex_count = es2_gl_get_integer(GL_MAX_TEXTURE_IMAGE_UNITS);
	caps->max_off_count = /*임시*/1;
	caps->tex_image_flag = /*임시*/0;

	SDL_Surface* surface = SDL_GetWindowSurface(window);
	caps->clrfmt = surface->format->BitsPerPixel == 16 ?
		surface->format->Amask != 0 ? QGCF16_RGBA : QGCF16_RGB :
		surface->format->Amask != 0 ? QGCF32_RGBA : QGCF32_RGB;

	// 초기화
	self->ss.program = GL_INVALID_VALUE;
	self->ss.buf_array = GL_INVALID_VALUE;
	self->ss.buf_element_array = GL_INVALID_VALUE;
	self->ss.buf_pixel_unpack = GL_INVALID_VALUE;

	self->ss.scissor = false;

	return true;
}

// 파이날라이즈
static void _es2_finalize(qgRdh* g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
	size_t i;

	// 펜딩
	qm_unload(self->pd.shd);
	qm_unload(self->pd.vlo);

	qm_unload(self->pd.ib);
	for (i = 0; i < self->base.caps.max_vertex_attrs; i++)
		qm_unload(self->pd.vb[i]);

	// SDL 렌더러
	if (self->renderer)
		SDL_DestroyRenderer(self->renderer);
}

//
static void es2_mat4_irrcht_texture(qnMat4* m, float radius, float cx, float cy, float tx, float ty, float sx, float sy)
{
	float c, s;
	qn_sincosf(radius, &s, &c);

	m->_11 = c * sx;
	m->_12 = s * sy;
	m->_13 = 0.0f;
	m->_14 = 0.0f;

	m->_21 = -s * sx;
	m->_22 = c * sy;
	m->_23 = 0.0f;
	m->_24 = 0.0f;

	m->_31 = c * sx * cx + -s * cy + tx;
	m->_32 = s * sy * cx + c * cy + ty;
	m->_33 = 1.0f;
	m->_34 = 0.0f;

	m->_41 = 0.0f;
	m->_42 = 0.0f;
	m->_43 = 0.0f;
	m->_44 = 1.0f;
}

// 리셋
static void _es2_reset(qgRdh* g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
	qgDeviceInfo* caps = &self->base.caps;
	qgRenderTm* tm = &self->base.tm;

	//
	qn_mat4_ortho_lh(&tm->ortho, (float)self->base.stub->size.width, (float)self->base.stub->size.height, -1.0f, 1.0f);
	tm->ortho._41 = -1.0f; tm->ortho._42 = 1.0f; tm->ortho._43 = 0.0f;
	es2_mat4_irrcht_texture(&tm->frm, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//
	ES2FUNC(glPixelStorei)(GL_PACK_ALIGNMENT, 1);
	ES2FUNC(glPixelStorei)(GL_UNPACK_ALIGNMENT, 1);

	ES2FUNC(glEnable)(GL_DEPTH_TEST);
	ES2FUNC(glDepthMask)(GL_TRUE);
	ES2FUNC(glDepthFunc)(GL_LEQUAL);
	ES2FUNC(glDisable)(GL_DEPTH_TEST);

	ES2FUNC(glEnable)(GL_CULL_FACE);
	ES2FUNC(glCullFace)(GL_BACK);

	ES2FUNC(glDisable)(GL_POLYGON_OFFSET_FILL);

	//for (int i = 0; i < caps->max_off_count; i++)	// 오프가 1개뿐이다. 어짜피 관련 함수에 인덱스 넣는 곳도 없음
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
}

// 시작
static bool _es2_begin(qgRdh* g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
	_es2_clear(qm_cast(self, qgRdh), QGCLR_DEPTH | QGCLR_STENCIL | QGCLR_RENDER, &self->base.param.clear, 0, 1.0f);
	ES2FUNC(glFrontFace)(GL_CW);
	return true;
}

// 끝
static void _es2_end(qgRdh* g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
}

// 플러시
static void _es2_flush(qgRdh* g)
{
	es2Rdh* self = qm_cast(g, es2Rdh);

	ES2FUNC(glFlush)();
	SDL_GL_SwapWindow((SDL_Window*)qm_get_desc(self->base.stub));
}

// 지우기
static void _es2_clear(qgRdh* g, int flag, const qnColor* color, int stencil, float depth)
{
	// 도움: https://open.gl/depthstencils
	es2Rdh* self = qm_cast(g, es2Rdh);

	GLbitfield cf = 0;

	if (QN_TEST_MASK(flag, QGCLR_STENCIL))
	{
		ES2FUNC(glStencilMaskSeparate)(GL_FRONT_AND_BACK, stencil);
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TEST_MASK(flag, QGCLR_DEPTH))
	{
		ES2FUNC(glDepthMask)(GL_TRUE);
		ES2FUNC(glClearDepthf)(depth);
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TEST_MASK(flag, QGCLR_RENDER) && color)
	{
		ES2FUNC(glClearColor)(color->r, color->g, color->b, color->a);
		cf |= GL_COLOR_BUFFER_BIT;
	}

	if (cf != 0)
		ES2FUNC(glClear)(cf);
}

// 세이더 설정
static void _es2_set_shader(qgRdh* g, qgShd* shader, qgVlo* layout)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
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
static bool _es2_set_index(qgRdh* g, qgBuf* buffer)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
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
		qn_retval_if_fail(buf->base.type != QGBUF_INDEX, false);

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
static bool _es2_set_vertex(qgRdh* g, int stage, qgBuf* buffer)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
	qn_retval_if_fail((size_t)stage < self->base.caps.max_vertex_attrs, false);
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
		qn_retval_if_fail(buf->base.type != QGBUF_VERTEX, false);

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
static qgVlo* _es2_create_layout(qgRdh* g, int count, const qgVarLayout* vars)
{
	static GLenum s_format[QGLOT_MAX_VALUE] =
	{
		GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT,
		GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, // 둘다 원래 GL_HALF_FLOAT 인데 ES2에서 지원안하네
		GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE,
		GL_SHORT, GL_SHORT, GL_SHORT,
		GL_UNSIGNED_BYTE
	};
	static uint8_t s_size[QGLOT_MAX_VALUE] =
	{
		1, 2, 3, 4,
		2, 4,
		2, 4, 4,
		2, 4, 2,
		4
	};
	static uint8_t s_calc_size[QGLOT_MAX_VALUE] =
	{
		1 * sizeof(float), 2 * sizeof(float), 3 * sizeof(float), 4 * sizeof(float),
		2 * sizeof(half_t), 4 * sizeof(half_t),
		2 * sizeof(uint8_t), 4 * sizeof(uint8_t), 4 * sizeof(uint8_t),
		2 * sizeof(uint16_t), 4 * sizeof(uint16_t), 2 * sizeof(uint16_t),
		4 * sizeof(uint8_t),
	};
	static GLboolean s_norm[QGLOT_MAX_VALUE] =
	{
		GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE,
		GL_TRUE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_TRUE
	};
	static GLboolean s_conv[QGLOT_MAX_VALUE] =
	{
		GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE,
		GL_TRUE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_TRUE
	};

	qn_retval_if_fail(count > 0 && vars != NULL, NULL);
	es2Rdh* self = qm_cast(g, es2Rdh);

	es2Vlo* vlo = qm_cast(_es2vlo_allocator(), es2Vlo);
	qn_retval_if_fail(vlo, NULL);

	size_t offset[QGLOS_MAX_VALUE] = { 0, };
	size_t accum[QGLOS_MAX_VALUE] = { 0, };
	for (size_t i = 0; i < count; i++)
	{
		const qgVarLayout* v = &vars[i];
		accum[v->slot]++;
	}

	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (accum[i] == 0)
			continue;
		vlo->es_cnt[i] = (int)accum[i];
		vlo->es_elm[i] = qn_alloc(accum[i], es2LayoutElement);
	}

	for (size_t i = 0; i < count; i++)
	{
		const qgVarLayout* v = &vars[i];
		es2LayoutElement* e = vlo->es_elm[v->slot]++;
		e->usage = v->usage;
		e->index = v->index;
		e->format = s_format[v->type];
		e->attrib = (GLuint)i;
		e->offset = (GLuint)offset[v->slot];
		e->size = s_size[v->type];
		e->normalized = s_norm[v->type];
		e->conv = s_conv[v->type];
		offset[v->slot] += s_calc_size[v->type];
	}

	vlo->base.stride = 0;
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		vlo->base.stride += (int)offset[i];
		vlo->base.stage[i] += (uint16_t)offset[i];
	}

	return qm_cast(vlo, qgVlo);
}

// 세이더 만들기
static qgShd* _es2_create_shader(qgRdh* g, const char* name)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
	es2Shd* shd = _es2shd_allocator(name);
	return qm_cast(shd, qgShd);
}

// 버퍼 만들기
static qgBuf* _es2_create_buffer(qgRdh* g, qgBufType type, int count, int stride, cpointer_t data)
{
	qn_retval_if_fail(count > 0 && stride > 0, NULL);
	es2Rdh* self = qm_cast(g, es2Rdh);

	// 타입
	GLenum gl_type;
	if (type == QGBUF_INDEX)
		gl_type = GL_ELEMENT_ARRAY_BUFFER;
	else if (type == QGBUF_VERTEX)
		gl_type = GL_ARRAY_BUFFER;
	else
		return NULL;

	// 만들고 바인드
	GLuint gl_id;
	ES2FUNC(glGenBuffers)(1, &gl_id);
	qn_retval_if_fail(gl_id != 0, NULL);

	es2_bind_buffer(self, gl_type, gl_id);

	// 데이터
	GLenum gl_usage;
	const GLvoid* ptr;
	if (data != NULL)
	{
		gl_usage = GL_STATIC_DRAW;
		ptr = data;
	}
	else
	{
		gl_usage = GL_DYNAMIC_DRAW;
		ptr = NULL;
	}

	int size = count * stride;
	ES2FUNC(glBufferData)(gl_type, (GLsizeiptr)size, ptr, gl_usage);

	// 준비는 끝났다
	es2Buf* buf = _es2buf_allocator(gl_id, gl_type, gl_usage, stride, size, type);
	if (buf == NULL)
	{
		ES2FUNC(glDeleteBuffers)(1, &gl_id);
		return NULL;
	}
	return qm_cast(buf, qgBuf);
}

// 프리미티브 시작
static bool _es2_primitive_begin(qgRdh* g, qgTopology tpg, int count, int stride, pointer_t* pptr)
{
	return false;
}

// 프리미티브 끝
static void _es2_primitive_end(qgRdh* g)
{
}

// 인덱스 프리미티브 시작
static bool _es2_indexed_primitive_begin(qgRdh* g, qgTopology tpg, int vcount, int vstride, pointer_t* pvptr, int icount, int istride, pointer_t* piptr)
{
	return false;
}

// 인덱스 프리미티브 끝
static void _es2_indexed_primitive_end(qgRdh* g)
{
}

// 토폴로지 변환
static GLenum es2_conv_topology(qgTopology tpg)
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
static bool _es2_draw(qgRdh* g, qgTopology tpg, int vcount)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
	es2Pending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd && pd->vb[0], false);

	es2_commit_shader(self);
	es2_commit_layout(self);

	GLenum gl_tpg = es2_conv_topology(tpg);
	ES2FUNC(glDrawArrays)(tpg, 0, (GLsizei)vcount);
	return true;
}

// 그리기 인덱스
static bool _es2_draw_indexed(qgRdh* g, qgTopology tpg, int icount)
{
	es2Rdh* self = qm_cast(g, es2Rdh);
	es2Pending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd && pd->vb[0] && pd->ib, false);

	GLenum gl_index =
		pd->ib->base.stride == sizeof(uint16_t) ? GL_UNSIGNED_SHORT :
		pd->ib->base.stride == sizeof(uint32_t) ? GL_UNSIGNED_INT : 0;
	qn_retval_if_fail(gl_index != 0, false);
	es2_bind_buffer(self, GL_ELEMENT_ARRAY_BUFFER, (GLuint)qm_get_desc(pd->ib));

	es2_commit_shader(self);
	es2_commit_layout(self);

	GLenum gl_tpg = es2_conv_topology(tpg);
	ES2FUNC(glDrawElements)(tpg, (GLint)icount, gl_index, NULL);
	return true;
}

