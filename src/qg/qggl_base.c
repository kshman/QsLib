#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qg_glbase.h"
#include "qg_glsupp.h"


//////////////////////////////////////////////////////////////////////////
// GL 렌더 디바이스


// 할당
void gl_initialize(GlRdhBase* self, SDL_Window* window, SDL_Renderer* renderer, const int flags)
{
	self->window = window;
	self->renderer = renderer;

	// capa
	QgDeviceInfo* caps = &self->base.caps;

	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);

	qn_strncpy(caps->name, QN_COUNTOF(caps->name), info.name, QN_COUNTOF(caps->name) - 1);
	gl_copy_string(caps->renderer, QN_COUNTOF(caps->renderer), GL_RENDERER);
	gl_copy_string(caps->vendor, QN_COUNTOF(caps->vendor), GL_VENDOR);
	// 버텍스 세이더 버전
	// 프래그먼트 세이더 버전

	caps->max_vertex_attrs = gl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);
	caps->max_tex_dim = info.max_texture_width;
	caps->max_tex_count = gl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	caps->max_off_count = gl_get_integer_v(GL_MAX_RENDERBUFFER_SIZE);
	caps->tex_image_flag = /*임시*/0;

	const SDL_Surface* surface = SDL_GetWindowSurface(window);
	caps->clrfmt = surface->format->BitsPerPixel == 16 ?
		surface->format->Amask != 0 ? QGCF16_RGBA : QGCF16_RGB :
		surface->format->Amask != 0 ? QGCF32_RGBA : QGCF32_RGB;

	self->ss.scissor = false;

	// 세이더 자동 유니폼
	gl_shd_init_auto_uniforms();
}

// 끝장내
void gl_finalize(GlRdhBase* self)
{
	// 펜딩
	qm_unload(self->pd.shd);
	qm_unload(self->pd.vlo);

	qm_unload(self->pd.ib);
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qm_unload(self->pd.vb[i]);

	// 세션
	qn_free(self->ss.layout.props);

	//
	self->disposed = true;
}

// 리셋
void gl_reset(QgRdh* rdh)
{
	rdh_internal_reset(rdh);

	QgRenderTm* tm = &rdh->tm;

	//
	qn_mat4_ortho_lh(&tm->ortho, (float)tm->size.width, (float)tm->size.height, -1.0f, 1.0f);
	qn_mat4_loc(&tm->ortho, -1.0f, 1.0f, 0.0f, false);
	gl_mat4_irrcht_texture(&tm->frm, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//
	GL_FUNC(glPixelStorei)(GL_PACK_ALIGNMENT, 1);
	GL_FUNC(glPixelStorei)(GL_UNPACK_ALIGNMENT, 1);

	GL_FUNC(glEnable)(GL_DEPTH_TEST);
	GL_FUNC(glDepthMask)(GL_TRUE);
	GL_FUNC(glDepthFunc)(GL_LEQUAL);
	GL_FUNC(glDisable)(GL_DEPTH_TEST);

	GL_FUNC(glEnable)(GL_CULL_FACE);
	GL_FUNC(glCullFace)(GL_BACK);

	GL_FUNC(glDisable)(GL_POLYGON_OFFSET_FILL);

	// 블렌드 상태 리셋은 각자 알아서
	// 텍스쳐 상태도 각자 알아서

	GL_FUNC(glDisable)(GL_SCISSOR_TEST);
	GL_FUNC(glFrontFace)(GL_CW);
}

// 시작
bool gl_begin(QgRdh* rdh, bool clear)
{
	//Es2Rdh* self = qm_cast(rdh, Es2Rdh);
	if (clear)
		gl_clear(rdh, QGCLR_DEPTH | QGCLR_STENCIL | QGCLR_RENDER, &rdh->param.bgc, 0, 1.0f);
	return true;
}

// 끝
void gl_end(QgRdh* rdh)
{
}

// 플러시
void gl_flush(QgRdh* rdh)
{
	const GlRdhBase* self = qm_cast(rdh, GlRdhBase);
	GL_FUNC(glFlush)();
	SDL_GL_SwapWindow(self->window);
}

// 지우기
void gl_clear(QgRdh* rdh, int flag, const QnColor* color, int stencil, float depth)
{
	// 도움: https://open.gl/depthstencils
	GLbitfield cf = 0;

	if (QN_TEST_MASK(flag, QGCLR_STENCIL))
	{
		GL_FUNC(glStencilMaskSeparate)(GL_FRONT_AND_BACK, stencil);
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TEST_MASK(flag, QGCLR_DEPTH))
	{
		GL_FUNC(glDepthMask)(GL_TRUE);
		GL_FUNC(glClearDepthf)(depth);
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TEST_MASK(flag, QGCLR_RENDER))
	{
		if (color == NULL)
			color = &rdh->param.bgc;
		GL_FUNC(glClearColor)(color->r, color->g, color->b, color->a);
		cf |= GL_COLOR_BUFFER_BIT;
	}

	if (cf != 0)
		GL_FUNC(glClear)(cf);
}

// 세이더 설정
void gl_set_shader(QgRdh* rdh, QgShd* shader, QgVlo* layout)
{
	GlRdhBase* self = qm_cast(rdh, GlRdhBase);
	GlShd* shd = qm_cast(shader, GlShd);
	GlVlo* vlo = qm_cast(layout, GlVlo);

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
bool gl_set_index(QgRdh* rdh, QgBuf* buffer)
{
	GlRdhBase* self = qm_cast(rdh, GlRdhBase);
	GlBuf* buf = qm_cast(buffer, GlBuf);

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
bool gl_set_vertex(QgRdh* rdh, QgLoStage stage, QgBuf* buffer)
{
	GlRdhBase* self = qm_cast(rdh, GlRdhBase);
	GlBuf* buf = qm_cast(buffer, GlBuf);

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
QgVlo* gl_create_layout(QgRdh* rdh, int count, const QgPropLayout* layouts)
{
	//Es2Rdh* self = qm_cast(rdh, Es2Rdh);
	GlVlo* vlo = gl_vlo_allocator(rdh, count, layouts);
	// 해야함: 관리
	return qm_cast(vlo, QgVlo);
}

// 세이더 만들기
QgShd* gl_create_shader(QgRdh* rdh, const char* name)
{
	//Es2Rdh* self = qm_cast(rdh, Es2Rdh);
	GlShd* shd = gl_shd_allocator(rdh, name);
	// 해야함: 관리
	return qm_cast(shd, QgShd);
}

// 버퍼 만들기
QgBuf* gl_create_buffer(QgRdh* rdh, QgBufType type, int count, int stride, const void* data)
{
	//Es2Rdh* self = qm_cast(rdh, Es2Rdh);
	GlBuf* buf = gl_buf_allocator(rdh, type, count, stride, data);
	// 해야함: 관리
	return qm_cast(buf, QgBuf);
}

// 토폴로지 변환
static GLenum gl_conv_topology(QgTopology tpg)
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
bool gl_draw(QgRdh* rdh, QgTopology tpg, int vcount)
{
	GlRdhBase* self = qm_cast(rdh, GlRdhBase);
	const GlPending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd, false);

	gl_commit_shader(self);
	gl_commit_layout(self);

	const GLenum gl_tpg = gl_conv_topology(tpg);
	GL_FUNC(glDrawArrays)(gl_tpg, 0, (GLsizei)vcount);
	return true;
}

// 그리기 인덱스
bool gl_draw_indexed(QgRdh* rdh, QgTopology tpg, int icount)
{
	GlRdhBase* self = qm_cast(rdh, GlRdhBase);
	const GlPending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd && pd->ib, false);

	const GLenum gl_index =
		pd->ib->base.stride == sizeof(ushort) ? GL_UNSIGNED_SHORT :
		pd->ib->base.stride == sizeof(uint) ? GL_UNSIGNED_INT : 0;
	qn_retval_if_fail(gl_index != 0, false);
	gl_bind_buffer(self, GL_ELEMENT_ARRAY_BUFFER, qm_get_desc(pd->ib, GLuint));

	gl_commit_shader(self);
	gl_commit_layout(self);

	const GLenum gl_tpg = gl_conv_topology(tpg);
	GL_FUNC(glDrawElements)(gl_tpg, (GLint)icount, gl_index, NULL);
	return true;
}

// 포인터 데이터로 그리기
bool gl_ptr_draw(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata)
{
	GlRdhBase* self = qm_cast(rdh, GlRdhBase);
	const GlPending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd, false);

	gl_commit_shader(self);
	gl_commit_layout_ptr(self, vdata, vstride);

	const GLenum gl_tpg = gl_conv_topology(tpg);
	GL_FUNC(glDrawArrays)(gl_tpg, 0, (GLsizei)vcount);
	return true;
}

// 포인터 데이터로 그리기 인덱스
bool gl_ptr_draw_indexed(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata)
{
	GLenum gl_index;
	if (istride == sizeof(ushort)) gl_index = GL_UNSIGNED_SHORT;
	else if (istride == sizeof(uint)) gl_index = GL_UNSIGNED_INT;
	else return false;

	GlRdhBase* self = qm_cast(rdh, GlRdhBase);
	const GlPending* pd = &self->pd;
	qn_retval_if_fail(pd->vlo && pd->shd, false);

	gl_commit_shader(self);
	gl_commit_layout_ptr(self, vdata, vstride);

	const GLenum gl_tpg = gl_conv_topology(tpg);
	GL_FUNC(glDrawElements)(gl_tpg, (GLint)icount, gl_index, idata);
	return true;
}

//
void gl_bind_buffer(GlRdhBase* self, GLenum gl_type, GLuint gl_id)
{
	switch (gl_type)
	{
		case GL_ARRAY_BUFFER:
			if (self->ss.buf_array != gl_id)
			{
				GL_FUNC(glBindBuffer)(GL_ARRAY_BUFFER, gl_id);
				self->ss.buf_array = gl_id;
			}
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			if (self->ss.buf_element_array != gl_id)
			{
				GL_FUNC(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, gl_id);
				self->ss.buf_element_array = gl_id;
			}
			break;
		case GL_PIXEL_UNPACK_BUFFER_NV:
			if (self->ss.buf_pixel_unpack != gl_id)
			{
				GL_FUNC(glBindBuffer)(GL_PIXEL_UNPACK_BUFFER_NV, gl_id);
				self->ss.buf_pixel_unpack = gl_id;
			}
			break;
		default:
			break;
	}
}

//
void gl_commit_layout(GlRdhBase* self)
{
	const GlShd* shd = self->pd.shd;
	const GlVlo* vlo = self->pd.vlo;

	const int max_attrs = rdh_caps(self).max_vertex_attrs;
	const GlCtnAttrib* shd_attrs = &shd->attrs;

	uint ok = 0;
	//QN_STATIC_ASSERT(QGMAX_ES2_VERTEX_ATTRIBUTE <= sizeof(ok) * 8, "Not enough for access QGMAX_ES2_VERTEX_ATTRIBUTE");

	for (int s = 0, index = 0; s < QGLOS_MAX_VALUE; s++)
	{
		GlBuf* buf = self->pd.vb[s];
		const int stcnt = vlo->es_cnt[s];

		if (buf != NULL)
		{
			// 정점 버퍼가 있다
			const GlLayoutElement* stelm = vlo->es_elm[s];

			const GLsizei gl_stride = (GLsizei)vlo->base.stride[s];
			const GLuint gl_buf = qm_get_desc(buf, GLuint);
			gl_bind_buffer(self, GL_ARRAY_BUFFER, gl_buf);

			// 해당 스테이지의 레이아웃
			for (int i = 0; i < stcnt; i++)
			{
				if (index >= max_attrs)
					break;

				const GlLayoutElement* le = &stelm[i];
				if (rdh_caps(self).test_stage_valid)
				{
					if (QN_TEST_BIT(shd->attr_mask, index) == false)	// 세이더에 해당 데이터가 없으면 패스
					{
						qn_debug_outputf(true, "ES2Rdh", "unmatched vertex attribute index %d", index);
						continue;
					}
					if ((size_t)index >= qn_ctnr_count(shd_attrs))		// 인덱스가 세이더 어트리뷰트 갯수 보다 크다
					{
						qn_debug_outputf(true, "ES2Rdh", "vertex attribute index overflow: %d (max: %d)", index, qn_ctnr_count(shd_attrs));
						continue;
					}
				}

				const GLint gl_attr = qn_ctnr_nth(shd_attrs, index).attrib;
				ok |= QN_BIT(gl_attr);
				GlLayoutProperty* lp = &self->ss.layout.props[gl_attr];
				const nuint pointer = (nuint)le->offset;
				if (QN_TEST_BIT(self->ss.layout.mask, gl_attr) == false)
				{
					QN_SET_BIT(&self->ss.layout.mask, gl_attr, true);
					GL_FUNC(glEnableVertexAttribArray)(gl_attr);
				}
				if (lp->pointer != pointer ||
					lp->buffer != gl_buf ||
					lp->stride != gl_stride ||
					lp->size != le->size ||
					lp->format != le->format ||
					lp->normalized != le->normalized)
				{
#ifdef _QN_WINDOWS_
					GL_FUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, pointer);
#else
					GL_FUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, (const void*)pointer);
#endif
					lp->pointer = pointer;
					lp->buffer = gl_buf;
					lp->stride = gl_stride;
					lp->size = le->size;
					lp->format = le->format;
					lp->normalized = le->normalized;
			}

				index++;
		}
	}
		else
		{
			// 버퍼 엄네
			for (int i = 0; i < stcnt; i++)
			{
				if (index >= max_attrs)
					break;

				if (rdh_caps(self).test_stage_valid)
				{
					if (QN_TEST_BIT(shd->attr_mask, index) == false)	// 세이더에 해당 데이터가 없으면 패스
					{
						qn_debug_outputf(true, "ES2Rdh", "unmatched vertex attribute index %d", index);
						continue;
					}
					if ((size_t)index >= qn_ctnr_count(shd_attrs))		// 인덱스가 세이더 어트리뷰트 갯수 보다 크다
					{
						qn_debug_outputf(true, "ES2Rdh", "vertex attribute index overflow: %d (max: %d)", index, qn_ctnr_count(shd_attrs));
						continue;
					}
				}

				const GLint gl_attr = qn_ctnr_nth(shd_attrs, index).attrib;
				if (QN_TEST_BIT(self->ss.layout.mask, gl_attr))
				{
					QN_SET_BIT(&self->ss.layout.mask, gl_attr, false);
					GL_FUNC(glDisableVertexAttribArray)(gl_attr);
				}
				const GLfloat tmp[4] = { 0.0f, };
				GL_FUNC(glVertexAttrib4fv)(gl_attr, tmp);

				index++;
			}
		}
}

	// 정리
	uint aftermask = self->ss.layout.mask & ~ok;
	for (int i = 0; i < max_attrs && aftermask; i++)
	{
		if (QN_TEST_BIT(aftermask, 0))
		{
			QN_SET_BIT(&self->ss.layout.mask, i, false);
			GL_FUNC(glDisableVertexAttribArray)(i);
		}
		aftermask >>= 1;
	}
}

// 사용자 포인터 그리기
void gl_commit_layout_ptr(GlRdhBase* self, const void* buffer, GLsizei gl_stride)
{
	const GlShd* shd = self->pd.shd;
	const GlVlo* vlo = self->pd.vlo;

	if (gl_stride == 0)
		gl_stride = (GLsizei)vlo->base.stride[0];
	else if (gl_stride != (GLsizei)vlo->base.stride[0])
	{
		qn_debug_outputf(true, "ES2Rdh", "stride is not same %d != %d", gl_stride, vlo->base.stride[0]);
		return;
	}

	const int max_attrs = rdh_caps(self).max_vertex_attrs;
	const GlCtnAttrib* shd_attrs = &shd->attrs;

	const GlLayoutElement* stelm = vlo->es_elm[0];	// 스테이지 0밖에 안씀. 버퍼가 하나니깐
	const int stcnt = vlo->es_cnt[0];

	gl_bind_buffer(self, GL_ARRAY_BUFFER, 0);	// 버퍼 끄지 않으면 안된다.

	uint ok = 0;
	for (int s = 0, index = 0; s < stcnt; s++)
	{
		const GlLayoutElement* le = &stelm[s];
		if (rdh_caps(self).test_stage_valid)
		{
			if (QN_TEST_BIT(shd->attr_mask, index) == false)	// 세이더에 해당 데이터가 없으면 패스
			{
				qn_debug_outputf(true, "ES2Rdh", "unmatched vertex attribute index %d", index);
				continue;
			}
			if ((size_t)index >= qn_ctnr_count(shd_attrs))		// 인덱스가 세이더 어트리뷰트 갯수 보다 크다
			{
				qn_debug_outputf(true, "ES2Rdh", "vertex attribute index overflow: %d (max: %d)", index, qn_ctnr_count(shd_attrs));
				continue;
			}
		}

		const GLint gl_attr = qn_ctnr_nth(shd_attrs, index).attrib;
		ok |= QN_BIT(gl_attr);
		GlLayoutProperty* lp = &self->ss.layout.props[gl_attr];
		const nuint pointer = (nuint)((const byte*)buffer + le->offset);
		if (QN_TEST_BIT(self->ss.layout.mask, gl_attr) == false)
		{
			QN_SET_BIT(&self->ss.layout.mask, gl_attr, true);
			GL_FUNC(glEnableVertexAttribArray)(gl_attr);
		}
		if (lp->pointer != pointer ||
			lp->buffer != 0 ||
			lp->stride != gl_stride ||
			lp->size != le->size ||
			lp->format != le->format ||
			lp->normalized != le->normalized)
		{
#ifdef _QN_WINDOWS_
			GL_FUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, pointer);
#else
			GL_FUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, (const void*)pointer);
#endif
			lp->pointer = pointer;
			lp->buffer = 0;
			lp->stride = gl_stride;
			lp->size = le->size;
			lp->format = le->format;
			lp->normalized = le->normalized;
	}

		index++;
}

	// 정리
	uint aftermask = self->ss.layout.mask & ~ok;
	for (int i = 0; i < max_attrs && aftermask; i++)
	{
		if (QN_TEST_BIT(aftermask, 0))
		{
			QN_SET_BIT(&self->ss.layout.mask, i, false);
			GL_FUNC(glDisableVertexAttribArray)(i);
		}
		aftermask >>= 1;
	}
}

//
void gl_commit_shader(GlRdhBase* self)
{
	GlShd* shd = self->pd.shd;
	if (shd == NULL)
	{
		if (self->ss.program != 0)
		{
			self->ss.program = 0;
			GL_FUNC(glUseProgram)(0);
		}
		return;
	}

	gl_shd_link(qm_cast(shd, QgShd));
	const GLuint gl_program = qm_get_desc(shd, GLuint);
	if (self->ss.program != gl_program)
	{
		self->ss.program = gl_program;
		GL_FUNC(glUseProgram)(gl_program);
	}

	// 세이더 값 여기서 넣는다!!!
	gl_shd_process_uniforms(shd);
}
