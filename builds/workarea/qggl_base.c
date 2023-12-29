#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qggl_base.h"

//////////////////////////////////////////////////////////////////////////
// GL 렌더 디바이스

/**
 * @brief GL 렌더러 초기화
 * @param self GL 렌더러
 * @param window 윈도우 핸들
 * @param renderer 렌더러 핸들
 * @param flags 렌더러 플래그
*/
void gl_initialize(GlRdh* self, SDL_Window* window, SDL_Renderer* renderer, const int flags)
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
	// caps->renderer_version 렌더러 버전 => 각자
	// caps->shader_version 세이더 버전 => 각자

	caps->max_layout_count = gl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);
	caps->max_tex_dim = gl_get_integer_v(GL_MAX_TEXTURE_SIZE);
	caps->max_tex_count = gl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	caps->max_off_count = gl_get_integer_v(GL_FRAMEBUFFER_BINDING);			// 이 값은 이상할 수 있다. 각자 렌더러가 다시 설정

	const SDL_Surface* surface = SDL_GetWindowSurface(window);
	caps->clr_fmt = surface->format->BitsPerPixel == 16 ?
		surface->format->Amask != 0 ? QGCF16_RGBA : QGCF16_RGB :
		surface->format->Amask != 0 ? QGCF32_RGBA : QGCF32_RGB;

	// 세이더 자동 유니폼
	//gl_shd_init_auto_uniforms();
}

/**
 * @brief GL 렌더러 정리
 * @param self GL 렌더러
*/
void gl_finalize(GlRdh* self)
{
	qn_ret_if_ok(self->disposed);

	// 펜딩
	qs_unload(self->pd.ib);
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qs_unload(self->pd.vb[i]);

	// 세션

	//
	self->disposed = true;
}

/**
 * @brief 리셋
 * @param rdh GL 렌더러
*/
void gl_reset(QgRdh* rdh)
{
	rdh_internal_reset(rdh);

	GlRdh* self = qs_cast(rdh, GlRdh);
	QgDeviceInfo* caps = &rdh_caps(self);

	//----- 펜딩
	GlPending* pd = &self->pd;

	//----- 세션
	GlSession* ss = &self->ss;
	ss->depth = QGDEPTH_LE;
	ss->stencil = QGSTENCIL_OFF;

	//----- TM
	QgRenderTm* tm = &rdh->tm;
	qm_mat4_ortho_lh(&tm->ortho, tm->size.x, tm->size.y, -1.0f, 1.0f);
	qm_mat4_loc(&tm->ortho, -1.0f, 1.0f, 0.0f, false);
	gl_mat4_irrcht_texture(&tm->frm, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//----- 설정
	GL_FUNC(glPixelStorei)(GL_PACK_ALIGNMENT, 1);
	GL_FUNC(glPixelStorei)(GL_UNPACK_ALIGNMENT, 1);

	// 뎁스 스텐실
	GL_FUNC(glEnable)(GL_DEPTH_TEST);
	GL_FUNC(glDepthMask)(GL_TRUE);
	GL_FUNC(glDepthFunc)(GL_LEQUAL);
	GL_FUNC(glDisable)(GL_STENCIL_TEST);

	// 래스터라이즈
	GL_FUNC(glEnable)(GL_CULL_FACE);
	GL_FUNC(glCullFace)(GL_BACK);

	GL_FUNC(glDisable)(GL_POLYGON_OFFSET_FILL);

	// 블렌드 상태 리셋은 각자 알아서
	// 텍스쳐 상태도 각자 알아서

	// 가위질
	GL_FUNC(glDisable)(GL_SCISSOR_TEST);
	GL_FUNC(glFrontFace)(GL_CW);
}

// 시작
bool gl_begin(QgRdh* rdh, bool clear)
{
	//GlRdh* self = qm_cast(rdh, GlRdh);
	if (clear)
		gl_clear(rdh, QGCLEAR_DEPTH | QGCLEAR_STENCIL | QGCLEAR_RENDER, &rdh->param.bgc, 0, 1.0f);
	return true;
}

// 끝
void gl_end(QgRdh* rdh)
{
}

// 플러시
void gl_flush(QgRdh* rdh)
{
	const GlRdh* self = qs_cast(rdh, GlRdh);
	GL_FUNC(glFlush)();
	SDL_GL_SwapWindow(self->window);
}

// 지우기
void gl_clear(QgRdh* rdh, int flag, const QmColor* color, int stencil, float depth)
{
	// 도움: https://open.gl/depthstencils
	GLbitfield cf = 0;

	if (QN_TMASK(flag, QGCLEAR_STENCIL))
	{
		GL_FUNC(glStencilMaskSeparate)(GL_FRONT_AND_BACK, stencil);
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TMASK(flag, QGCLEAR_DEPTH))
	{
		GL_FUNC(glDepthMask)(GL_TRUE);
		GL_FUNC(glClearDepthf)(depth);
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TMASK(flag, QGCLEAR_RENDER))
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
	GlRdh* self = qs_cast(rdh, GlRdh);
	GlShd* shd = qs_cast(shader, GlShd);
	GlVlo* vlo = qs_cast(layout, GlVlo);

	if (shd == NULL)
	{
		if (self->pd.shd != NULL)
		{
			qs_unload(self->pd.shd);
			self->pd.shd = NULL;
		}
	}
	else
	{
		if (self->pd.shd != shd)
		{
			qs_unload(self->pd.shd);
			self->pd.shd = shd;
			qs_load(shd);
		}
	}

	if (vlo == NULL)
	{
		if (self->pd.vlo != NULL)
		{
			qs_unload(self->pd.vlo);
			self->pd.vlo = NULL;
		}
	}
	else
	{
		if (self->pd.vlo != vlo)
		{
			qs_unload(self->pd.vlo);
			self->pd.vlo = vlo;
			qs_load(vlo);
		}
	}
}

// 인덱스 설정
bool gl_set_index(QgRdh* rdh, QgBuf* buffer)
{
	GlRdh* self = qs_cast(rdh, GlRdh);
	GlBuf* buf = qs_cast(buffer, GlBuf);

	if (buf == NULL)
	{
		if (self->pd.ib != NULL)
		{
			qs_unload(self->pd.ib);
			self->pd.ib = NULL;
		}
	}
	else
	{
		qn_val_if_fail(buf->base.type == QGBUFFER_INDEX, false);

		if (self->pd.ib != buf)
		{
			qs_unload(self->pd.ib);
			self->pd.ib = buf;
			qs_load(buf);
		}
	}

	return true;
}

// 정점 설정
bool gl_set_vertex(QgRdh* rdh, QgLoStage stage, QgBuf* buffer)
{
	GlRdh* self = qs_cast(rdh, GlRdh);
	GlBuf* buf = qs_cast(buffer, GlBuf);

	if (buf == NULL)
	{
		if (self->pd.vb[stage] != NULL)
		{
			qs_unload(self->pd.vb[stage]);
			self->pd.vb[stage] = NULL;
		}
	}
	else
	{
		qn_val_if_fail(buf->base.type == QGBUFFER_VERTEX, false);

		if (self->pd.vb[stage] != buf)
		{
			qs_unload(self->pd.vb[stage]);
			self->pd.vb[stage] = buf;
			qs_load(buf);
		}
	}

	return true;
}

// 뎁스 스텐셀 설정
bool gl_set_depth_stencil(QgRdh* rdh, QgDsm* depth_stencil)
{
	GlRdh* self = qs_cast(rdh, GlRdh);
	GlDsm* dsm = qs_cast(depth_stencil, GlDsm);

	if (dsm != NULL)
	{
		self->pd.dsm_prop = dsm->prop;
		return true;
	}

	// 뎁스 스텐실의 기본값
	static GlDepthStencilProp s_depth_stencil =
	{
		GL_TRUE, GL_TRUE, GL_LEQUAL,
		false, false,
		GL_ALWAYS, GL_KEEP, GL_KEEP, GL_KEEP,
		GL_ALWAYS, GL_KEEP, GL_KEEP, GL_KEEP,
		0xFF, 0xFF,
	};
	self->pd.dsm_prop = s_depth_stencil;
	return false;
}

// 래스터라이저 설정
bool gl_set_rasterizer(QgRdh* rdh, QgRsz* rasterizer)
{
	GlRdh* self = qs_cast(rdh, GlRdh);
	GlRsz* rsz = qs_cast(rasterizer, GlRsz);

	if (rsz != NULL)
	{
		self->pd.rsz_prop = rsz->prop;
		return true;
	}

	// 래스터라이저의 기본값
	static GlRasterizeProp s_rasterizer =
	{
		0,
		GL_BACK,
		0.0f,
		0.0f,
	};
	self->pd.rsz_prop = s_rasterizer;
	return false;
}

// 레이아웃 만들기
QgVlo* gl_create_layout(QgRdh* rdh, int count, const QgLayoutInput* layouts)
{
	//GlRdh* self = qm_cast(rdh, GlRdh);
	GlVlo* vlo = gl_vlo_allocator(rdh, count, layouts);
	// 해야함: 관리
	return qs_cast(vlo, QgVlo);
}

// 세이더 만들기
QgShd* gl_create_shader(QgRdh* rdh, const char* name)
{
	//GlRdh* self = qm_cast(rdh, GlRdh);
	GlShd* shd = gl_shd_allocator(rdh, name);
	// 해야함: 관리
	return qs_cast(shd, QgShd);
}

// 버퍼 만들기
QgBuf* gl_create_buffer(QgRdh* rdh, QgBufType type, int count, int stride, const void* data)
{
	//GlRdh* self = qm_cast(rdh, GlRdh);
	GlBuf* buf = gl_buf_allocator(rdh, type, count, stride, data);
	// 해야함: 관리
	return qs_cast(buf, QgBuf);
}

// 뎁스 스텐실 만들기
QgDsm* gl_create_depth_stencil(QgRdh* rdh, const QgDepthStencilProp* prop)
{
	//GlRdh* self = qm_cast(rdh, GlRdh);
	GlDsm* dsm = gl_dsm_allocator(rdh, prop);
	// 해야함: 관리
	return qs_cast(dsm, QgDsm);
}

// 래스터라이저 만들기
QgRsz* gl_create_rasterizer(QgRdh* rdh, const QgRasterizerProp* prop)
{
	//GlRdh* self = qm_cast(rdh, GlRdh);
	GlRsz* rsz = gl_rsz_allocator(rdh, prop);
	// 해야함: 관리
	return qs_cast(rsz, QgRsz);
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
	GlRdh* self = qs_cast(rdh, GlRdh);
	const GlPending* pd = &self->pd;
	qn_val_if_fail(pd->vlo && pd->shd, false);

	gl_commit_shader(self);
	gl_commit_layout(self);
	gl_commit_depth_stencil(self);

	const GLenum gl_tpg = gl_conv_topology(tpg);
	GL_FUNC(glDrawArrays)(gl_tpg, 0, (GLsizei)vcount);
	return true;
}

// 그리기 인덱스
bool gl_draw_indexed(QgRdh* rdh, QgTopology tpg, int icount)
{
	GlRdh* self = qs_cast(rdh, GlRdh);
	const GlPending* pd = &self->pd;
	qn_val_if_fail(pd->vlo && pd->shd && pd->ib, false);

	const GLenum gl_index =
		pd->ib->base.stride == sizeof(ushort) ? GL_UNSIGNED_SHORT :
		pd->ib->base.stride == sizeof(uint) ? GL_UNSIGNED_INT : 0;
	qn_val_if_fail(gl_index != 0, false);
	gl_bind_buffer(self, GL_ELEMENT_ARRAY_BUFFER, qs_get_desc(pd->ib, GLuint));

	gl_commit_shader(self);
	gl_commit_layout(self);
	gl_commit_depth_stencil(self);

	const GLenum gl_tpg = gl_conv_topology(tpg);
	GL_FUNC(glDrawElements)(gl_tpg, (GLint)icount, gl_index, NULL);
	return true;
}

// 포인터 데이터로 그리기
bool gl_ptr_draw(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata)
{
	GlRdh* self = qs_cast(rdh, GlRdh);
	const GlPending* pd = &self->pd;
	qn_val_if_fail(pd->vlo && pd->shd, false);

	gl_commit_shader(self);
	gl_commit_layout_ptr(self, vdata, vstride);
	gl_commit_depth_stencil(self);

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

	GlRdh* self = qs_cast(rdh, GlRdh);
	const GlPending* pd = &self->pd;
	qn_val_if_fail(pd->vlo && pd->shd, false);

	gl_commit_shader(self);
	gl_commit_layout_ptr(self, vdata, vstride);
	gl_commit_depth_stencil(self);

	const GLenum gl_tpg = gl_conv_topology(tpg);
	GL_FUNC(glDrawElements)(gl_tpg, (GLint)icount, gl_index, idata);
	return true;
}

//
void gl_bind_buffer(GlRdh* self, GLenum gl_type, GLuint gl_id)
{
	switch (gl_type)
	{
		case GL_ARRAY_BUFFER:
			if (self->ss.buf.array != gl_id)
			{
				self->ss.buf.array = gl_id;
				GL_FUNC(glBindBuffer)(GL_ARRAY_BUFFER, gl_id);
			}
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			if (self->ss.buf.element_array != gl_id)
			{
				self->ss.buf.element_array = gl_id;
				GL_FUNC(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, gl_id);
			}
			break;
		case GL_PIXEL_UNPACK_BUFFER_NV:
			if (self->ss.buf.pixel_unpack != gl_id)
			{
				self->ss.buf.pixel_unpack = gl_id;
				GL_FUNC(glBindBuffer)(GL_PIXEL_UNPACK_BUFFER_NV, gl_id);
			}
			break;
		default:
			break;
	}
}

//
void gl_commit_layout(GlRdh* self)
{
	const GlShd* shd = self->pd.shd;
	const GlVlo* vlo = self->pd.vlo;

	const int max_attrs = rdh_caps(self).max_layout_count;
	const GlCtnAttrib* shd_attrs = &shd->attrs;

	uint ok = 0;
	//static_assert(QGMAX_ES2_VERTEX_ATTRIBUTE <= sizeof(ok) * 8, "Not enough for access QGMAX_ES2_VERTEX_ATTRIBUTE");

	for (int s = 0, index = 0; s < QGLOS_MAX_VALUE; s++)
	{
		GlBuf* buf = self->pd.vb[s];
		const int stcnt = vlo->es_cnt[s];

		if (buf != NULL)
		{
			// 정점 버퍼가 있다
			const GlLayoutElement* stelm = vlo->es_elm[s];

			const GLsizei gl_stride = (GLsizei)vlo->base.stride[s];
			const GLuint gl_buf = qs_get_desc(buf, GLuint);
			gl_bind_buffer(self, GL_ARRAY_BUFFER, gl_buf);

			// 해당 스테이지의 레이아웃
			for (int i = 0; i < stcnt; i++)
			{
				if (index >= max_attrs)
					break;

				const GlLayoutElement* le = &stelm[i];
				if (rdh_caps(self).test_stage_valid)
				{
					if (QN_TBIT(shd->attr_mask, index) == false)	// 세이더에 해당 데이터가 없으면 패스
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
				GlLayoutProp* lp = &self->ss.layout.props[gl_attr];
				const nuint pointer = (nuint)le->offset;
				if (QN_TBIT(self->ss.layout.mask, gl_attr) == false)
				{
					QN_SBIT(&self->ss.layout.mask, gl_attr, true);
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
					if (QN_TBIT(shd->attr_mask, index) == false)	// 세이더에 해당 데이터가 없으면 패스
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
				if (QN_TBIT(self->ss.layout.mask, gl_attr))
				{
					QN_SBIT(&self->ss.layout.mask, gl_attr, false);
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
		if (QN_TBIT(aftermask, 0))
		{
			QN_SBIT(&self->ss.layout.mask, i, false);
			GL_FUNC(glDisableVertexAttribArray)(i);
		}
		aftermask >>= 1;
	}
}

// 사용자 포인터 그리기
void gl_commit_layout_ptr(GlRdh* self, const void* buffer, GLsizei gl_stride)
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

	const int max_attrs = rdh_caps(self).max_layout_count;
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
			if (QN_TBIT(shd->attr_mask, index) == false)	// 세이더에 해당 데이터가 없으면 패스
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
		GlLayoutProp* lp = &self->ss.layout.props[gl_attr];
		const nuint pointer = (nuint)((const byte*)buffer + le->offset);
		if (QN_TBIT(self->ss.layout.mask, gl_attr) == false)
		{
			QN_SBIT(&self->ss.layout.mask, gl_attr, true);
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
		if (QN_TBIT(aftermask, 0))
		{
			QN_SBIT(&self->ss.layout.mask, i, false);
			GL_FUNC(glDisableVertexAttribArray)(i);
		}
		aftermask >>= 1;
	}
}

//
void gl_commit_shader(GlRdh* self)
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

	gl_shd_link(qs_cast(shd, QgShd));
	const GLuint gl_program = qs_get_desc(shd, GLuint);
	if (self->ss.program != gl_program)
	{
		self->ss.program = gl_program;
		GL_FUNC(glUseProgram)(gl_program);
	}

	// 세이더 값 여기서 넣는다!!!
	gl_shd_process_uniforms(shd);
}

//
void gl_commit_depth_stencil(GlRdh* self)
{
	const GlDepthStencilProp* pd = &self->pd.dsm_prop;
	GlDepthStencilProp* ss = &self->ss.dsm_prop;

	// 뎁스
	if (ss->depth.enable != pd->depth.enable)
	{
		if (pd->depth.enable)
		{
			ss->depth.enable = true;
			GL_FUNC(glEnable)(GL_DEPTH_TEST);

			if (ss->depth.func != pd->depth.func)
			{
				ss->depth.func = pd->depth.func;
				GL_FUNC(glDepthFunc)(pd->depth.func);
			}
		}
		else
		{
			ss->depth.enable = false;
			GL_FUNC(glDisable)(GL_DEPTH_TEST);
		}
	}
	if (ss->depth.write != pd->depth.write)
	{
		ss->depth.write = pd->depth.write;
		GL_FUNC(glDepthMask)(pd->depth.write);
	}

	// 스텐실
	if (ss->stencil.enable != pd->stencil.enable)
	{
		ss->stencil.enable = pd->stencil.enable;
		if (ss->stencil.two_side != pd->stencil.two_side)
		{
			ss->stencil.two_side = pd->stencil.two_side;
			ss->stencil.front.func = GL_INVALID_ENUM;
			ss->stencil.front.fail = GL_INVALID_ENUM;
			ss->stencil.front.pass = GL_INVALID_ENUM;
			ss->stencil.front.z_fail = GL_INVALID_ENUM;
			ss->stencil.back.func = GL_INVALID_ENUM;
			ss->stencil.back.fail = GL_INVALID_ENUM;
			ss->stencil.back.pass = GL_INVALID_ENUM;
			ss->stencil.back.z_fail = GL_INVALID_ENUM;
		}

		if (pd->stencil.enable == false)
			GL_FUNC(glDisable)(GL_STENCIL_TEST);
		else
		{
			if (pd->stencil.two_side) // 양면
			{
				// 앞변
				const struct GlStencilValue* pf = &pd->stencil.front;
				struct GlStencilValue* sf = &ss->stencil.front;
				if (sf->func != pf->func || ss->stencil.mask.read != pd->stencil.mask.read)
				{
					ss->stencil.front.func = pd->stencil.front.func;
					GL_FUNC(glStencilFuncSeparate)(GL_BACK, pf->func, 0, pd->stencil.mask.read);
				}
				if (sf->pass != pf->pass || sf->fail != pf->fail || sf->z_fail != pf->z_fail)
				{
					sf->pass = pf->pass;
					sf->fail = pf->fail;
					sf->z_fail = pf->z_fail;
					GL_FUNC(glStencilOpSeparate)(GL_BACK, pf->fail, pf->z_fail, pf->pass);
				}

				// 뒷면
				const struct GlStencilValue* pb = &pd->stencil.back;
				struct GlStencilValue* sb = &ss->stencil.back;
				if (sb->func != pb->func || ss->stencil.mask.read != pd->stencil.mask.read)
				{
					ss->stencil.front.func = pd->stencil.front.func;
					GL_FUNC(glStencilFuncSeparate)(GL_FRONT, pb->func, 0, pd->stencil.mask.read);
				}
				if (sb->pass != pb->pass || sb->fail != pb->fail || sb->z_fail != pb->z_fail)
				{
					sb->pass = pb->pass;
					sb->fail = pb->fail;
					sb->z_fail = pb->z_fail;
					GL_FUNC(glStencilOpSeparate)(GL_FRONT, pb->fail, pb->z_fail, pb->pass);
				}
			}
			else // 단면
			{
				const struct GlStencilValue* pf = &pd->stencil.front;
				struct GlStencilValue* sf = &ss->stencil.front;
				if (sf->func != pf->func || ss->stencil.mask.read != pd->stencil.mask.read)
				{
					ss->stencil.front.func = pd->stencil.front.func;
					GL_FUNC(glStencilFunc)(pf->func, 0, pd->stencil.mask.read);
				}
				if (sf->pass != pf->pass || sf->fail != pf->fail || sf->z_fail != pf->z_fail)
				{
					sf->pass = pf->pass;
					sf->fail = pf->fail;
					sf->z_fail = pf->z_fail;
					GL_FUNC(glStencilOp)(pf->fail, pf->z_fail, pf->pass);
				}
			}
			if (ss->stencil.mask.write != pd->stencil.mask.write)
			{
				ss->stencil.mask.write = pd->stencil.mask.write;
				GL_FUNC(glStencilMask)(pd->stencil.mask.write);
			}

			ss->stencil.mask.read = pd->stencil.mask.read;
		}
	}
}

