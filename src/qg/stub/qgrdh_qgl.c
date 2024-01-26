//
// qgrdh_qgl.c - GL 공통 디바이스
// 2024-01-26 by kim
//

#include "pch.h"
#if defined USE_ES || defined USE_GL
#if (defined _QN_EMSCRIPTEN_ || defined _QN_ANDROID_) && !defined ES_STATIC_LINK
#define ES_STATIC_LINK
#endif
#ifdef ES_STATIC_LINK
#include <GLES3/gl3.h>
#else
#include "glad/gl.h"
#endif // ES_STATIC_LINK
#include "qgrdh_qgl.h"
#include <qs_supp.h>
#include <limits.h>

//////////////////////////////////////////////////////////////////////////
// OPENGL 공통 디바이스

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"QGLRDH"

// 얻고 싶은 정보
void qgl_wanted_config(QglConfig* config, int version, QgFlag flags)
{
	config->handle = NULL;
	config->red = 8;
	config->green = 8;
	config->blue = 8;
	config->alpha = 8;
	config->depth = qn_get_prop_int(QG_PROP_DEPTH_SIZE, 24, 4, 32);
	config->stencil = qn_get_prop_int(QG_PROP_STENCIL_SIZE, 8, 4, 16);
	config->samples = QN_TMASK(flags, QGFLAG_MSAA) ? qn_get_prop_int(QG_PROP_MSAA, 4, 0, 8) : 0;
	config->version = version;

	const char* size_prop = qn_get_prop(QG_PROP_RGBA_SIZE);
	if (size_prop != NULL && strlen(size_prop) >= 4)
	{
		config->red = size_prop[0] - '0';
		config->green = size_prop[1] - '0';
		config->blue = size_prop[2] - '0';
		config->alpha = size_prop[3] - '0';
	}
}

// 컨피그 얻기
const QglConfig* qgl_detect_config(const QglConfig* wanted, const QglConfig* configs, int count)
{
	uint least_missing = UINT_MAX;
	uint least_color = UINT_MAX;
	uint least_extra = UINT_MAX;
	const QglConfig* found = NULL;
	for (int i = 0; i < count; i++)
	{
		const QglConfig* c = &configs[i];

		uint missing = 0;
		if (wanted->alpha > 0 && c->alpha == 0) missing++;
		if (wanted->depth > 0 && c->depth == 0) missing++;
		if (wanted->stencil > 0 && c->stencil == 0) missing++;
		if (wanted->samples > 0 && c->samples == 0) missing++;

		uint color = 0;
		if (wanted->red > 0)
			color += (wanted->red - c->red) * (wanted->red - c->red);
		if (wanted->blue > 0)
			color += (wanted->blue - c->blue) * (wanted->blue - c->blue);
		if (wanted->green > 0)
			color += (wanted->green - c->green) * (wanted->green - c->green);

		uint extra = 0;
		if (wanted->alpha > 0)
			extra += (wanted->alpha - c->alpha) * (wanted->alpha - c->alpha);
		if (wanted->depth > 0)
			extra += (wanted->depth - c->depth) * (wanted->depth - c->depth);
		if (wanted->stencil > 0)
			extra += (wanted->stencil - c->stencil) * (wanted->stencil - c->stencil);
		if (wanted->samples > 0)
			extra += (wanted->samples - c->samples) * (wanted->samples - c->samples);
		if (wanted->version > 0)
			extra += (wanted->version - c->version) * (wanted->version - c->version);

		if (missing < least_missing)
			found = c;
		else if (missing == least_missing)
		{
			if (color < least_color || (color == least_color && extra < least_extra))
				found = c;
		}
		if (found == c)
		{
			least_missing = missing;
			least_color = color;
			least_extra = extra;
		}
	}
	return found;
}

// 할당하는데서 쓰는 최초 정보 설정
void qgl_rdh_init_info(QglRdh* self, const QglConfig* config, const QglInitialInfo* initial)
{
	const char* gl_version = (const char*)glGetString(GL_VERSION);
	const char* gl_shader_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	const int max_layout_count = qgl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);

	RendererInfo* info = &self->base.info;	// 여기서 RDH_INFO를 쓰면 안된다 (인스턴스가 아직 널임)
	memcpy(&self->cfg, config, sizeof(QglConfig));

	qgl_copy_string(info->renderer, QN_COUNTOF(info->renderer), GL_RENDERER);
	qgl_copy_string(info->vendor, QN_COUNTOF(info->vendor), GL_VENDOR);
	info->renderer_version = qgl_get_version(gl_version, initial->version_string[0], initial->version_string[1]);
	info->shader_version = qgl_get_version(gl_shader_version, initial->shader_string[0], initial->shader_string[1]);

	info->max_layout_count = QN_MIN(max_layout_count, initial->max_layout_count);
	info->max_indices = qgl_get_integer_v(GL_MAX_ELEMENTS_INDICES);
	info->max_vertices = qgl_get_integer_v(GL_MAX_ELEMENTS_VERTICES);
	info->max_tex_dim = qgl_get_integer_v(GL_MAX_TEXTURE_SIZE);
	info->max_tex_count = qgl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	info->max_off_count = qgl_get_integer_v(GL_MAX_DRAW_BUFFERS);

	info->clr_fmt = qg_rgba_to_clrfmt(config->red, config->green, config->blue, config->alpha, false);

	info->enabled_stencil = config->stencil > 0;

	//
	qn_debug_outputf(false, VAR_CHK_NAME, "%s %d/%d [%s by %s]",
		initial->name,
		info->renderer_version, info->shader_version,
		info->renderer, info->vendor);
	qn_debug_outputs(false, VAR_CHK_NAME, gl_version);
	qn_debug_outputs(false, VAR_CHK_NAME, gl_shader_version);
}

// 제거할 때 호출
bool qgl_rdh_finalize(QglRdh* self)
{
	qn_val_if_ok(self->disposed, false);
	self->disposed = true;

	//----- 펜딩
	const QglPending* pd = &self->pd;
	qs_unload(pd->render.index_buffer);
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qs_unload(pd->render.vertex_buffers[i]);
	qs_unload(pd->render.render);

	//----- 리소스
	rdh_internal_clean();

	return true;
}

//
void qgl_rdh_layout(void)
{
	rdh_internal_layout();

	RenderTransform* tm = RDH_TRANSFORM;
	QmMat4 ortho = ortho = qm_mat4_ortho_lh(tm->size.Width, -tm->size.Height, -1.0f, 1.0f);
	tm->ortho = qm_mat4_mul(ortho, qm_mat4_loc(-1.0f, 1.0f, 0.0f));

	// 뷰포트
	glViewport(0, 0, (GLsizei)tm->size.Width, (GLsizei)tm->size.Height);
	qs_cast_vt(RDH, QGLRDH)->gl_depth_range(0.0f, 1.0f);
}

//
void qgl_rdh_reset(void)
{
	rdh_internal_reset();

	const RendererInfo* info = RDH_INFO;
	RenderTransform* tm = RDH_TRANSFORM;
	QglSession* ss = QGL_SESSION;

	//----- 펜딩

	//----- 세션
	uint amask = ss->shader.amask;
	ss->shader.program = 0;
	ss->shader.amask = 0;
	qn_zero(ss->shader.lprops, QGLOU_MAX_SIZE, QglLayoutProperty);
	ss->buffer.vertex = GL_INVALID_HANDLE;
	ss->buffer.index = GL_INVALID_HANDLE;
	ss->buffer.uniform = GL_INVALID_HANDLE;
	ss->depth = QGDEPTH_LE;
	ss->stencil = QGSTENCIL_OFF;

	//----- 트랜스폼
	tm->frm = qgl_mat4_irrcht_texture(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//----- 장치 설정
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 뎁스 스텐실
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	if (info->enabled_stencil)
	{
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}
	glDisable(GL_STENCIL_TEST);

	// 래스터라이즈
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDisable(GL_POLYGON_OFFSET_FILL);

	// 텍스쳐
	for (size_t i = 0; i < info->max_tex_count; i++)
	{
		glActiveTexture((GLenum)(GL_TEXTURE0 + i));
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// 세이더
	for (size_t i = 0; i < info->max_layout_count; i++)
	{
		if (QN_TBIT(amask, i))
			glDisableVertexAttribArray((GLuint)i);
	}
	glUseProgram(0);

	// 가위질
	glDisable(GL_SCISSOR_TEST);
	glFrontFace(GL_CW);
}

// 지우기
void qgl_rdh_clear(QgClear flags)
{
	// 도움: https://open.gl/depthstencils
	GLbitfield cf = 0;

	if (QN_TMASK(flags, QGCLEAR_STENCIL) && RDH_INFO->enabled_stencil)
	{
		glClearStencil(0.0f);
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_DEPTH))
	{
		if (QGL_SESSION->depth == QGDEPTH_OFF)
		{
			QGL_SESSION->depth = QGDEPTH_LE;
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
		}
		qs_cast_vt(RDH, QGLRDH)->gl_clear_depth(1.0f);
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_RENDER))
	{
		const QmVec4 c = RDH_PARAM->bgc;
		glClearColor(c.X, c.Y, c.Z, c.W);
		cf |= GL_COLOR_BUFFER_BIT;
	}

	if (cf != 0)
		glClear(cf);
}

// 시작
bool qgl_rdh_begin(bool clear)
{
	if (clear)
		qgl_rdh_clear(QGCLEAR_DEPTH | QGCLEAR_STENCIL | QGCLEAR_RENDER);
	return true;
}

// 끝
void qgl_rdh_end(void)
{
}

// 플러시
void qgl_rdh_flush(void)
{
	glFlush();
}

// 정점 버퍼 설정, stage에 대한 오류 설정은 rdh에서 하고 왔을 거임
bool qgl_rdh_set_vertex(QgLayoutStage stage, QgBuffer* buffer)
{
	QglBuffer* buf = qs_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (pd->render.vertex_buffers[stage] != buf)
	{
		qs_unload(pd->render.vertex_buffers[stage]);
		pd->render.vertex_buffers[stage] = qs_loadc(buf, QglBuffer);
	}
	return true;
}

// 인덱스 버퍼 설정
bool qgl_rdh_set_index(QgBuffer* buffer)
{
	QglBuffer* buf = qs_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (pd->render.index_buffer != buf)
	{
		qs_unload(pd->render.index_buffer);
		pd->render.index_buffer = qs_loadc(buf, QglBuffer);
	}
	return true;
}

// 렌더 설정
bool qgl_rdh_set_render(QgRender* render)
{
	QglRender* rdr = qs_cast_type(render, QglRender);
	QglPending* pd = QGL_PENDING;
	if (pd->render.render != rdr)
	{
		qs_unload(pd->render.render);
		pd->render.render = qs_loadc(rdr, QglRender);
	}
	return true;
}

// 버퍼 바인딩
void qgl_bind_buffer(const QglBuffer* buffer)
{
	switch (buffer->base.type)
	{
		case QGBUFFER_VERTEX:
			qgl_bind_vertex_buffer(buffer);
			break;
		case QGBUFFER_INDEX:
			qgl_bind_index_buffer(buffer);
			break;
		case QGBUFFER_CONSTANT:
			qgl_bind_uniform_buffer(buffer);
			break;
		default:
			qn_debug_outputs(true, VAR_CHK_NAME, "invalid buffer type");
			break;
	}
}

// 오토 세이더 변수 (오토가 아니면 RDH의 사용자 함수로 떠넘긴다)
static void qgl_process_shader_variable(const QgVarShader* var)
{
	switch (var->scauto)
	{
		case QGSCA_ORTHO_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->ortho._11);
			break;
		case QGSCA_WORLD:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->world._11);
			break;
		case QGSCA_VIEW:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->view._11);
			break;
		case QGSCA_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->proj._11);
			break;
		case QGSCA_VIEW_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->view_proj._11);
			break;
		case QGSCA_INV_VIEW:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->invv._11);
			break;
		case QGSCA_WORLD_VIEW_PROJ:
		{
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			QmMat4 m = qm_mat4_mul(RDH_TRANSFORM->world, RDH_TRANSFORM->view_proj);
			glUniformMatrix4fv(var->offset, 1, false, &m._11);
		} break;
		case QGSCA_TEX1:
		case QGSCA_TEX2:
		case QGSCA_TEX3:
		case QGSCA_TEX4:
		case QGSCA_TEX5:
		case QGSCA_TEX6:
		case QGSCA_TEX7:
		case QGSCA_TEX8:
			switch (var->sctype)
			{
				case QGSCT_SAMPLER1D:
				case QGSCT_SAMPLER2D:
				case QGSCT_SAMPLER3D:
				case QGSCT_SAMPLERCUBE:
					glUniform1i(var->offset, var->scauto - QGSCA_TEX1);
					break;
				default:
					qn_debug_outputs(true, VAR_CHK_NAME, "invalid auto shader texture");
					break;
			}
			break;
		case QGSCA_PROP_VEC1:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[0].f);
			break;
		case QGSCA_PROP_VEC2:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[1].f);
			break;
		case QGSCA_PROP_VEC3:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[2].f);
			break;
		case QGSCA_PROP_VEC4:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[3].f);
			break;
		case QGSCA_PROP_MAT1:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[0]._11);
			break;
		case QGSCA_PROP_MAT2:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[1]._11);
			break;
		case QGSCA_PROP_MAT3:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[2]._11);
			break;
		case QGSCA_PROP_MAT4:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[3]._11);
			break;
		case QGSCA_MAT_PALETTE:
			qn_verify(var->sctype == QGSCT_FLOAT16);
			glUniformMatrix4fv(var->offset, RDH_PARAM->bone_count, false, (const GLfloat*)RDH_PARAM->bone_ptr);
			break;
		default:
			if (RDH_PARAM->callback_func != NULL)
				RDH_PARAM->callback_func(RDH_PARAM->callback_data, (nint)var->hash, var);
			break;
	}
}

// 세이더랑 레이아웃
bool qgl_commit_shader_layout(const QglRender* rdr)
{
	const size_t max_attrs = RDH_INFO->max_layout_count;

	// 프로그램 설정
	if (QGL_SESSION->shader.program != rdr->shader.program)
	{
		QGL_SESSION->shader.program = rdr->shader.program;
		glUseProgram(rdr->shader.program);
	}

	// 유니폼 값 넣고
	size_t s, i;
	qn_ctnr_foreach(&rdr->shader.uniforms, i)
	{
		const QgVarShader* var = &qn_ctnr_nth(&rdr->shader.uniforms, i);
		qgl_process_shader_variable(var);
	}

	// 어트리뷰트 + 레이아웃
	uint aok = 0, amask = QGL_SESSION->shader.amask;
	for (s = 0; s < QGLOS_MAX_VALUE; s++)
	{
		QglBuffer* buf = QGL_PENDING->render.vertex_buffers[s];
		if (buf == NULL)
			continue;
		qgl_bind_vertex_buffer(buf);

		const QglLayoutInput * stages = rdr->layout.stages[s];
		const GLsizei gl_stride = (GLsizei)rdr->layout.strides[s];
		const size_t count = rdr->layout.counts[s];
		for (i = 0; i < count; i++)
		{
			const QglLayoutInput* input = &stages[i];
			const GLint gl_attr = rdr->shader.usages[input->usage];
			if (gl_attr == 0xFF)
			{
				const char* name = qg_layout_usage_to_str(input->usage);
				qn_debug_outputf(true, VAR_CHK_NAME, "shader attribute not found: %s", name);
				continue;
			}

			aok |= QN_BIT(gl_attr);
			if (QN_TBIT(amask, gl_attr) == false)
			{
				QN_SBIT(&amask, gl_attr, true);
				glEnableVertexAttribArray(gl_attr);
			}

			const GLuint gl_offset = input->offset;
			QglLayoutProperty* lp = &QGL_SESSION->shader.lprops[input->usage];
			if (lp->offset != gl_offset ||
				lp->format != input->format ||
				lp->stride != gl_stride ||
				lp->count != input->count ||
				lp->normalized != input->normalized)
			{
				glVertexAttribPointer(gl_attr, input->count, input->format, input->normalized, gl_stride, (GLvoid*)(size_t)gl_offset);
				lp->offset = gl_offset;
				lp->format = input->format;
				lp->stride = gl_stride;
				lp->count = input->count;
				lp->normalized = input->normalized;
			}
		}
	}

	// 레이아웃 정리
	uint aftermask = amask & ~aok;
	for (i = 0; i < max_attrs && aftermask; i++)
	{
		if (QN_TBIT(aftermask, 0))
		{
			QN_SBIT(&amask, i, false);
			glDisableVertexAttribArray((GLuint)i);
		}
		aftermask >>= 1;
	}
	QGL_SESSION->shader.amask = amask;

	return true;
}

// 뎁스 스텐실
void qgl_commit_depth_stencil(const QglRender* rdr)
{
	QglSession* ss = QGL_SESSION;

	// 뎁스
	if (ss->depth != rdr->depth)
	{
		if (rdr->depth == QGDEPTH_OFF)
		{
			if (ss->depth != QGDEPTH_OFF)
				glDisable(GL_DEPTH_TEST);
		}
		else
		{
			if (ss->depth == QGDEPTH_OFF)
				glEnable(GL_DEPTH_TEST);
			GLenum gl_func = qgl_depth_to_enum(rdr->depth);
			glDepthFunc(gl_func);
		}
		ss->depth = rdr->depth;
	}

	// 스텐실
	if (RDH_INFO->enabled_stencil && ss->stencil != rdr->stencil)
	{
		if (rdr->stencil == QGSTENCIL_OFF)
		{
			glDisable(GL_STENCIL_TEST);
		}
		else
		{
			if (ss->stencil == QGSTENCIL_OFF)
				glEnable(GL_STENCIL_TEST);
			if (rdr->stencil == QGSTENCIL_WRITE)
			{
				// 보통으로 그릴 때 스텐실을 1로 채움
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilMask(0xFF);
			}
			else if (rdr->stencil == QGSTENCIL_EVADE)
			{
				// 스텐실 1로 채워진 부분은 그리지 않음 -> 외곽선 같은거 그릴때 (https://heinleinsgame.tistory.com/25)
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glStencilMask(0);
			}
			else if (rdr->stencil == QGSTENCIL_OVER)
			{
				// 스텐실 1로 채워진 부분만 그림 -> 그림자/반사 같은거 그릴때 (https://open.gl/depthstencils)
				glStencilFunc(GL_EQUAL, 1, 0xFF);
				glStencilMask(0);
			}
		}
		ss->stencil = rdr->stencil;
	}
}


//////////////////////////////////////////////////////////////////////////
// 렌더러

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "QGLRENDER"

//
static void qgl_render_delete_shader(QglRender* self, bool check_rdh)
{
	if (self->shader.program == 0)
		return;
	if (check_rdh && QGL_RDH->disposed == false && QGL_SESSION->shader.program == self->shader.program)
	{
		QGL_SESSION->shader.program = 0;
		glUseProgram(0);
	}
	if (self->shader.vertex != 0)
	{
		glDetachShader(self->shader.program, self->shader.vertex);
		glDeleteShader(self->shader.vertex);
	}
	if (self->shader.fragment != 0)
	{
		glDetachShader(self->shader.program, self->shader.fragment);
		glDeleteShader(self->shader.fragment);
	}
	glDeleteProgram(self->shader.program);
}

//
static void qgl_render_dispose(QsGam* g)
{
	QglRender* self = qs_cast_type(g, QglRender);

	qgl_render_delete_shader(self, true);
	qn_ctnr_disp(&self->shader.uniforms);
	qn_ctnr_disp(&self->shader.attrs);
	qn_ctnr_disp(&self->layout.inputs);

	rdh_internal_unlink_node(RDHNODE_RENDER, self);
	qn_free(self);
}

// 세이더 컴파일
static GLuint qgl_render_compile_shader(GLenum gl_type, const char* code)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	GLuint gl_shader = glCreateShader(gl_type);
	glShaderSource(gl_shader, 1, &code, NULL);
	glCompileShader(gl_shader);

	if (qgl_get_shader_iv(gl_shader, GL_COMPILE_STATUS) != GL_FALSE)
		return gl_shader;

	const char* type = gl_type == GL_VERTEX_SHADER ? "vertex" : gl_type == GL_FRAGMENT_SHADER ? "fragment" : "unknown";
	GLint gl_log_len = qgl_get_shader_iv(gl_shader, GL_INFO_LOG_LENGTH);
	if (gl_log_len <= 0)
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to %s shader compile", type);
	else
	{
		GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
		glGetShaderInfoLog(gl_shader, gl_log_len, &gl_log_len, gl_log);
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to %s shader compile: %s", type, gl_log);
		qn_free(gl_log);
	}

	glDeleteShader(gl_shader);
	return 0;
}

// 세이더 만들기
static bool qgl_render_bind_shader(QglRender* self, const QgCodeData* vertex, const QgCodeData* fragment)
{
	// 프로그램이랑 세이더 만들고
	GLuint gl_vertex_shader = qgl_render_compile_shader(GL_VERTEX_SHADER, vertex->code);
	GLuint gl_fragment_shader = qgl_render_compile_shader(GL_FRAGMENT_SHADER, fragment->code);
	if (gl_vertex_shader == 0 || gl_fragment_shader == 0)
	{
		if (gl_vertex_shader != 0)
			glDeleteShader(gl_vertex_shader);
		if (gl_fragment_shader != 0)
			glDeleteShader(gl_fragment_shader);
		return false;
	}

	self->shader.vertex = gl_vertex_shader;
	self->shader.fragment = gl_fragment_shader;
	self->shader.program = glCreateProgram();
	glAttachShader(self->shader.program, gl_vertex_shader);
	glAttachShader(self->shader.program, gl_fragment_shader);

	// 링크
	glLinkProgram(self->shader.program);
	if (qgl_get_program_iv(self->shader.program, GL_LINK_STATUS) == GL_FALSE)
	{
		GLsizei gl_log_len = qgl_get_program_iv(self->shader.program, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
			qn_debug_outputf(true, VAR_CHK_NAME, "failed to link shader");
		else
		{
			GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
			glGetProgramInfoLog(self->shader.program, gl_log_len, &gl_log_len, gl_log);
			qn_debug_outputf(true, VAR_CHK_NAME, "failed to link shader: %s", gl_log);
			qn_free(gl_log);
		}
		return false;
	}

	// 분석
	GLint i, index, gl_count, gl_size, gl_index;
	GLenum gl_type;
	GLchar sz[64];

	// 유니폼
	gl_count = qgl_get_program_iv(self->shader.program, GL_ACTIVE_UNIFORMS);
	if (gl_count > 0)
	{
		qn_ctnr_init(QglCtnUniform, &self->shader.uniforms, gl_count);
		for (index = i = 0; i < gl_count; i++)
		{
			glGetActiveUniform(self->shader.program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz);
			gl_index = glGetUniformLocation(self->shader.program, sz);
			if (gl_index < 0)
			{
				qn_debug_outputf(true, VAR_CHK_NAME, "failed to get uniform location: %s", sz);
				continue;
			}

			QgScType sctype = qgl_enum_to_shader_const(gl_type);
			if (sctype == QGSCT_UNKNOWN)
				qn_debug_outputf(true, VAR_CHK_NAME, "unsupported uniform type: %s (type: %X)", sz, gl_type);

			QgVarShader* var = &qn_ctnr_nth(&self->shader.uniforms, index++);
			qn_strcpy(var->name, sz);
			var->hash = qn_strihash(sz);
			var->offset = (ushort)glGetUniformLocation(self->shader.program, sz);
			var->size = (ushort)gl_size;
			var->sctype = sctype;
			var->scauto = qg_str_to_shader_const_auto(var->hash, sz);
			if (var->scauto == QGSCA_UNKNOWN)
				var->hash = (size_t)qn_get_key(sz);
		}
	}

	// 어트리뷰트
	memset(self->shader.usages, 0xFF, QN_COUNTOF(self->shader.usages));
	gl_count = qgl_get_program_iv(self->shader.program, GL_ACTIVE_ATTRIBUTES);
	if (gl_count > 0)
	{
		qn_ctnr_init(QglCtnAttr, &self->shader.attrs, gl_count);
		for (index = i = 0; i < gl_count; i++)
		{
			glGetActiveAttrib(self->shader.program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz);
			gl_index = glGetAttribLocation(self->shader.program, sz);
			if (gl_index < 0)
			{
				qn_debug_outputf(true, VAR_CHK_NAME, "failed to get attribute location: %s", sz);
				continue;
			}

			QgScType sctype = qgl_enum_to_shader_const(gl_type);
			if (sctype == QGSCT_UNKNOWN)
				qn_debug_outputf(true, VAR_CHK_NAME, "unsupported attribute type: %s (type: %X)", sz, gl_type);

			QglVarAttr* var = &qn_ctnr_nth(&self->shader.attrs, index++);
			qn_strcpy(var->name, sz);
			var->hash = qn_strihash(sz);
			var->attrib = gl_index;
			var->size = gl_size;
			var->usage = qg_str_to_layout_usage(var->hash, sz);
			var->sctype = sctype;
			self->shader.usages[var->usage] = (byte)gl_index;
		}
	}

	return true;
}

// 버텍스 레이아웃 만들기
static bool qgl_render_bind_layout_input(QglRender* self, const QgLayoutData* layout)
{
	static byte lo_count[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4, 3, 2, 1,
		/* INT     */ 4, 3, 2, 1,
		/* HALF-F  */ 4, 2, 1, 1,
		/* HALF-I  */ 4, 2, 1, 1,
		/* BYTE    */ 4, 3, 2, 1, 1, 1,
		/* USHORT  */ 1, 1, 1,
		/* NONE    */ 0, 0,
	};
	static GLenum lo_format[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ GL_NONE,
		/* FLOAT   */ GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT,
		/* INT     */ GL_INT, GL_INT, GL_INT, GL_INT,
		/* HALF-F  */ GL_HALF_FLOAT, GL_HALF_FLOAT, GL_HALF_FLOAT, GL_UNSIGNED_INT_10F_11F_11F_REV,
		/* HALF-I  */ GL_SHORT, GL_SHORT, GL_SHORT, GL_UNSIGNED_INT_2_10_10_10_REV,
		/* BYTE    */ GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE,
		/* USHORT  */ GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_4_4_4_4,
		/* NONE    */ GL_NONE, GL_NONE,
	};
	static byte lo_size[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4 * sizeof(float), 3 * sizeof(float), 2 * sizeof(float), 1 * sizeof(float),
		/* INT     */ 4 * sizeof(int), 3 * sizeof(int), 2 * sizeof(int), 1 * sizeof(int),
		/* HALF-F  */ 4 * sizeof(halffloat), 2 * sizeof(halffloat), 1 * sizeof(halffloat), 1 * sizeof(int),
		/* HALF-I  */ 4 * sizeof(short), 2 * sizeof(short), 1 * sizeof(short), 1 * sizeof(int),
		/* BYTE    */ 4 * sizeof(byte), 3 * sizeof(byte), 2 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte),
		/* USHORT  */ 2 * sizeof(ushort), 2 * sizeof(ushort), 2 * sizeof(ushort),
		/* NONE    */ 0, 0,
	};

	size_t i;

	// 갯수 검사
	ushort loac[QGLOS_MAX_VALUE] = { 0, };
	for (i = 0; i < layout->count; i++)
	{
		const QgLayoutInput* input = &layout->inputs[i];
		if ((size_t)input->stage >= QGLOS_MAX_VALUE)
		{
			qn_debug_outputf(true, VAR_CHK_NAME, "invalid layout stage: %d", input->stage);
			return false;
		}
		loac[input->stage]++;
	}

	// 레이아웃
	qn_ctnr_init(QglCtnLayoutInput, &self->layout.inputs, layout->count);
	QglLayoutInput* pstage = qn_ctnr_data(&self->layout.inputs);
	QglLayoutInput* stages[QGLOS_MAX_VALUE];
	for (i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (loac[i] == 0)
			stages[i] = NULL;
		else
		{
			self->layout.counts[i] = loac[i];
			self->layout.stages[i] = stages[i] = pstage;
			pstage += loac[i];
		}
	}

	// 항목별로 만들어서 스테이지로 구분
	for (i = 0; i < layout->count; i++)
	{
		const QgLayoutInput* input = &layout->inputs[i];
		const GLenum gl_format = lo_format[input->format];
		if (gl_format == GL_NONE)
		{
			const char* name = qg_clrfmt_to_str(input->format);
			qn_debug_outputf(true, VAR_CHK_NAME, "cannot use or unknown layout format: %s", name);
			return false;
		}

		QglLayoutInput* stage = stages[input->stage]++;
		stage->stage = input->stage;
		stage->usage = input->usage;
		stage->offset = self->layout.strides[input->stage];
		stage->format = gl_format;
		stage->count = lo_count[input->format];
		stage->normalized = input->normalized;
		self->layout.strides[input->stage] += lo_size[input->format];
	}

	return true;
}

// 렌더 만들기. 오류 처리는 다하고 왔을 것이다
QgRender* qgl_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader)
{
	QglRender* self = qn_alloc_zero_1(QglRender);
	qg_node_set_name(qs_cast_type(self, QgNode), name);

	// 세이더
	if (qgl_render_bind_shader(self, &shader->vertex, &shader->pixel) == false)
		goto pos_error;

	// 레이아웃
	if (qgl_render_bind_layout_input(self, &shader->layout) == false)
		goto pos_error;

	// 속성
	self->depth = prop->depth;
	self->stencil = prop->stencil;

	//
	static qs_name_vt(QSGAM) vt_es_render =
	{
		.name = VAR_CHK_NAME,
		.dispose = qgl_render_dispose,
	};
	qs_init(self, QgRender, &vt_es_render);
	if (name)
		rdh_internal_add_node(RDHNODE_RENDER, self);
	return qs_cast_type(self, QgRender);

pos_error:
	qgl_render_delete_shader(self, false);
	qn_ctnr_disp(&self->shader.uniforms);
	qn_ctnr_disp(&self->shader.attrs);
	qn_ctnr_disp(&self->layout.inputs);
	qn_free(self);
	return NULL;
}

#endif // USE_ES || USE_GL
