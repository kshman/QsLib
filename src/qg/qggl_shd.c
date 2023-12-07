﻿#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qg_glbase.h"
#include "qg_glsupp.h"

//////////////////////////////////////////////////////////////////////////
// 레이아웃

static void gl_vlo_dispose(QmGam* g);

qvt_name(QmGam) vt_es2_vlo =
{
	.name = "ES2Vlo",
	.dispose = gl_vlo_dispose,
};

//
GlVlo* gl_vlo_allocator(QgRdh* rdh, int count, const QgPropLayout* layouts)
{
	static GLenum s_format[QGLOT_MAX_VALUE] =
	{
		GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT,
		GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, // 둘다 원래 GL_HALF_FLOAT 인데 ES2에서 지원안하네
		GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE,
		GL_SHORT, GL_SHORT, GL_SHORT,
		GL_FLOAT, GL_UNSIGNED_INT,
	};
	static byte s_size[QGLOT_MAX_VALUE] =
	{
		1, 2, 3, 4,
		2, 4,
		2, 4, 4,
		2, 4, 2,
		4, 1,
	};
	static byte s_calc_size[QGLOT_MAX_VALUE] =
	{
		1 * sizeof(float), 2 * sizeof(float), 3 * sizeof(float), 4 * sizeof(float),
		2 * sizeof(halfint), 4 * sizeof(halfint),
		2 * sizeof(byte), 4 * sizeof(byte), 4 * sizeof(byte),
		2 * sizeof(short), 4 * sizeof(short), 2 * sizeof(short),
		4 * sizeof(float), 1 * sizeof(uint),
	};
	static GLboolean s_norm[QGLOT_MAX_VALUE] =
	{
		GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE,
		GL_TRUE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_TRUE,
	};
	static GLboolean s_conv[QGLOT_MAX_VALUE] =
	{
		GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE,
		GL_TRUE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_FALSE, GL_TRUE,
		GL_FALSE, GL_TRUE,
	};

	// 갯수 파악 및 스테이지 오류 검사
	byte accum[QGLOS_MAX_VALUE] = { 0, };
	for (int i = 0; i < count; i++)
	{
		const QgPropLayout* l = &layouts[i];
		if ((size_t)l->stage >= QGLOS_MAX_VALUE)
			return NULL;
		accum[l->stage]++;
	}

	GlVlo* self = qn_alloc_zero_1(GlVlo);
	qn_retval_if_fail(self, NULL);

	// 스테이지 할당
	GlLayoutElement* elms[QGLOS_MAX_VALUE] = { NULL, };
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (accum[i] == 0)
			continue;
		self->es_cnt[i] = (byte)accum[i];
		self->es_elm[i] = elms[i] = qn_alloc(accum[i], GlLayoutElement);
	}

	// 데이터 작성
	ushort offset[QGLOS_MAX_VALUE] = { 0, };
	for (int i = 0; i < count; i++)
	{
		const QgPropLayout* l = &layouts[i];
		GlLayoutElement* e = elms[l->stage]++;
		e->stage = l->stage;
		e->usage = l->usage;
		e->index = l->index;
		e->attr = (GLuint)i;
		e->size = s_size[l->type];
		e->format = s_format[l->type];
		e->offset = (GLuint)offset[l->stage];
		e->normalized = s_norm[l->type];
		e->conv = s_conv[l->type];
		offset[l->stage] += s_calc_size[l->type];
	}

	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
		self->base.stride[i] = (ushort)offset[i];

	return qm_init(self, GlVlo, &vt_es2_vlo);
}

//
static void gl_vlo_dispose(QmGam* g)
{
	GlVlo* self = qm_cast(g, GlVlo);
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
		if (self->es_elm[i])
			qn_free(self->es_elm[i]);
	qn_free(self);
}


//////////////////////////////////////////////////////////////////////////
// 세이더

static void gl_shd_match_auto_uniform(GlUniform* u);

static void gl_shd_dispose(QmGam* g);
static bool gl_shd_bind(QgShd* g, QgShdType type, const void* data, int size, int flags);
static bool gl_shd_bind_shd(QgShd* g, QgShdType type, QgShd* shaderptr);

qvt_name(QgShd) vt_es2_shd =
{
	.base.name = "ES2Shd",
	.base.dispose = gl_shd_dispose,
	.bind = gl_shd_bind,
	.bind_shd = gl_shd_bind_shd,
	//.bind_name = NULL,
	//.add_condition = NULL,
	//.clear_condition = NULL,
	.link = gl_shd_link,
};

//
GlShd* gl_shd_allocator(QgRdh* rdh, const char* name)
{
	GlShd* self = qn_alloc_zero_1(GlShd);
	qn_retval_if_fail(self, NULL);
#ifdef _MSC_VER
	qm_set_desc(self, gl_func.glCreateProgram());
#else
	qm_set_desc(self, GL_FUNC(glCreateProgram)());	// 이렇게 하면 LINT에 걸려버렷
#endif
	return qm_init(self, GlShd, &vt_es2_shd);
}

//
static void gl_shd_dispose(QmGam* g)
{
	GlShd* self = qm_cast(g, GlShd);
	const GLuint handle = qm_get_desc(self, GLuint);

	gl_shd_handle_unload(self->rvertex, handle);
	gl_shd_handle_unload(self->rfragment, handle);

	GL_FUNC(glDeleteProgram)(handle);

	qn_ctnr_disp(es2CtnUniform, &self->uniforms);
	qn_ctnr_disp(es2CtnAttrib, &self->attrs);

	qn_free(self);
}

//
static GlRefHandle* gl_shd_compile(GlShd* self, GLenum gl_type, const char* src, GLint gl_len)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	const GLuint gl_shader = GL_FUNC(glCreateShader)(gl_type);
	qn_retval_if_fail(gl_shader != 0, NULL);

	GL_FUNC(glShaderSource)(gl_shader, 1, &src, gl_len == 0 ? NULL : &gl_len);
	GL_FUNC(glCompileShader)(gl_shader);

	if (gl_get_shader_iv(gl_shader, GL_COMPILE_STATUS) == GL_FALSE)
	{
		const char* progress = gl_type == GL_VERTEX_SHADER ? "vertex" : gl_type == GL_FRAGMENT_SHADER ? "fragment" : "(unknown)";
		GLsizei gl_log_len = gl_get_shader_iv(gl_shader, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
			qn_debug_outputf(true, "ES2Shd", "%s shader error", progress);
		else
		{
			GLchar* psz = qn_alloca(gl_log_len, GLchar);
			GL_FUNC(glGetShaderInfoLog)(gl_shader, gl_log_len, &gl_log_len, psz);
			qn_debug_outputf(true, "ES2Shd", "%s shader error in %s", psz, progress);
			qn_freea(psz);
		}
		return NULL;
	}

	return gl_shd_handle_new(gl_shader);
}

//
static bool gl_shd_bind(QgShd* g, QgShdType type, const void* data, int size, int flags)
{
	// 해야함: 매크로
	// 해야함: 인클루트
	// 해야함: 플래그
	// 해야함: 보관
	qn_retval_if_fail(data != NULL && size >= 0, false);
	GlShd* self = qm_cast(g, GlShd);
	const GLuint handle = qm_get_desc(self, GLuint);

	if (type == QGSHT_VS)
	{
		gl_shd_handle_unload(self->rvertex, handle);
		if ((self->rvertex = gl_shd_compile(self, GL_VERTEX_SHADER, (const char*)data, (GLint)size)) == NULL)
			return false;
		GL_FUNC(glAttachShader)(handle, self->rvertex->handle);
	}
	else if (type == QGSHT_PS)
	{
		gl_shd_handle_unload(self->rfragment, handle);
		if ((self->rfragment = gl_shd_compile(self, GL_FRAGMENT_SHADER, (const char*)data, (GLint)size)) == NULL)
			return false;
		GL_FUNC(glAttachShader)(handle, self->rfragment->handle);
	}
	else
	{
		// 엥 머여
		return false;
	}

	self->linked = false;
	return true;
}

//
static bool gl_shd_bind_shd(QgShd* g, QgShdType type, QgShd* shaderptr)
{
	qn_retval_if_fail(shaderptr, false);
	GlShd* self = qm_cast(g, GlShd);
	const GlShd* shader = qm_cast(shaderptr, GlShd);
	const GLuint handle = qm_get_desc(self, GLuint);
	bool ok = false;

	if (QN_TEST_MASK(type, QGSHT_VS))
	{
		qn_retval_if_fail(shader->rvertex, false);
		gl_shd_handle_unload(self->rvertex, handle);

		self->rvertex->ref++;
		GL_FUNC(glAttachShader)(handle, self->rvertex->handle);
		ok = true;
	}

	if (QN_TEST_MASK(type, QGSHT_PS))
	{
		qn_retval_if_fail(shader->rfragment, false);
		gl_shd_handle_unload(self->rfragment, handle);

		self->rfragment->ref++;
		GL_FUNC(glAttachShader)(handle, self->rfragment->handle);
		ok = true;
	}

	if (!ok)
		return false;

	self->linked = false;
	return true;
}

//
static QgShdConst gl_shd_enum_to_const(GLenum gl_type)
{
	switch (gl_type)
	{
		case GL_FLOAT:				return QGSHC_FLOAT1;
		case GL_FLOAT_VEC2:			return QGSHC_FLOAT2;
		case GL_FLOAT_VEC3:			return QGSHC_FLOAT3;
		case GL_FLOAT_VEC4:			return QGSHC_FLOAT4;
		case GL_FLOAT_MAT4:			return QGSHC_FLOAT16;
#ifdef GL_SAMPLER_1D
		case GL_SAMPLER_1D:
#endif
#ifdef GL_SAMPLER_1D_SHADOW
		case GL_SAMPLER_1D_SHADOW:
#endif
		case GL_SAMPLER_2D:
#ifdef GL_SAMPLER_2D_SHADOW
		case GL_SAMPLER_2D_SHADOW:
#endif
		case GL_SAMPLER_CUBE:
#ifdef GL_SAMPLER_3D
		case GL_SAMPLER_3D:
#endif
		case GL_INT:				return QGSHC_INT1;
		case GL_INT_VEC2:			return QGSHC_INT2;
		case GL_INT_VEC3:			return QGSHC_INT3;
		case GL_INT_VEC4:			return QGSHC_INT4;
		case GL_BOOL:				return QGSHC_BYTE1;
		case GL_BOOL_VEC2:			return QGSHC_BYTE2;
		case GL_BOOL_VEC3:			return QGSHC_BYTE3;
		case GL_BOOL_VEC4:			return QGSHC_BYTE4;
		default:					return QGSHC_UNKNOWN;
	}
}

//
QgLoUsage gl_shd_attrib_to_usage(const char* name)
{
	// 각 속성 이름에 "+숫자" 일 경우가 있기 때문에 길이 비교를 한다
	// 예를 들면 색깔은 acolor 또는 acolor0, 색깔2(스페큘러)는 acolor1 이런 식
	if (qn_strnicmp(name, "aposition", 9) == 0)		return QGLOU_POSITION;
	if (qn_strnicmp(name, "acolor", 6) == 0)		return QGLOU_COLOR;
	if (qn_strnicmp(name, "aweight", 7) == 0)		return QGLOU_WEIGHT;
	if (qn_strnicmp(name, "aindex", 6) == 0)		return QGLOU_INDEX;
	if (qn_strnicmp(name, "anormal", 7) == 0)		return QGLOU_NORMAL;
	if (qn_strnicmp(name, "atexcoord", 9) == 0 ||
		qn_strnicmp(name, "acoord", 6) == 0)		return QGLOU_TEXTURE;
	if (qn_strnicmp(name, "atangent", 8) == 0)		return QGLOU_TANGENT;
	if (qn_strnicmp(name, "abinormal", 9) == 0)		return QGLOU_BINORMAL;
	return QGLOU_MAX_VALUE;
}

//
bool gl_shd_link(QgShd* g)
{
	GlShd* self = qm_cast(g, GlShd);
	qn_retval_if_ok(self->linked, true);
	qn_retval_if_fail(self->rvertex && self->rfragment, false);

	GLchar sz[1024];

	// 링크
	const GLuint gl_program = qm_get_desc(self, GLuint);
	GL_FUNC(glLinkProgram)(gl_program);

	if (gl_get_program_iv(gl_program, GL_LINK_STATUS) == GL_FALSE)
	{
		GLsizei gl_log_len = gl_get_program_iv(gl_program, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
			qn_debug_outputs(true, "ES2Shd", "program link error");
		else
		{
			GL_FUNC(glGetProgramInfoLog)(gl_program, QN_COUNTOF(sz) - 1, &gl_log_len, sz);
			qn_debug_outputf(true, "ES2Shd" "program link error '%s'", sz);
		}
		return false;
	}

	// 전역 변수 (유니폼)
	qn_ctnr_disp(es2CtnUniform, &self->uniforms);
	const GLint gl_uniforms = gl_get_program_iv(gl_program, GL_ACTIVE_UNIFORMS);
	if (gl_uniforms > 0)
	{
		qn_ctnr_init(GlCtnUniform, &self->uniforms, gl_uniforms);

		for (GLint i = 0, index = 0; i < gl_uniforms; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			GL_FUNC(glGetActiveUniform)(gl_program, i, QN_COUNTOF(sz) - 1, NULL, &gl_size, &gl_type, sz);

			const QgShdConst cnst = gl_shd_enum_to_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)
			{
				// 사용할 수 없는 유니폼
				qn_debug_outputf(true, "ES2Shd", "not supported uniform type '%X' in '%s'", gl_type, sz);
				continue;
			}

			GlUniform* u = &qn_ctnr_nth(&self->uniforms, index);
			qn_strncpy(u->base.name, QN_COUNTOF(u->base.name), sz, QN_COUNTOF(u->base.name) - 1);
			u->base.cnst = cnst;
			u->base.size = (ushort)gl_size;
			u->base.offset = (uint)GL_FUNC(glGetUniformLocation)(gl_program, sz);
			u->hash = qn_strihash(u->base.name);
			gl_shd_match_auto_uniform(u);

			index++;
		}
	}

	// 인수 (어트리뷰트)
	self->attr_mask = 0;
	qn_ctnr_disp(es2CtnAttrib, &self->attrs);
	const GLint gl_attrs = gl_get_program_iv(gl_program, GL_ACTIVE_ATTRIBUTES);
	if (gl_attrs > 0)
	{
		const size_t max_vertex_attrs = rdh_caps(qg_rdh_instance).max_vertex_attrs;
		qn_ctnr_init(GlCtnAttrib, &self->attrs, gl_attrs);

		for (GLint i = 0, index = 0; i < gl_attrs; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			GLsizei gl_len;
			GL_FUNC(glGetActiveAttrib)(gl_program, i, QN_COUNTOF(sz) - 1, &gl_len, &gl_size, &gl_type, sz);

			const QgShdConst cnst = gl_shd_enum_to_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)
			{
				// 사용할 수 없는 어트리뷰트
				qn_debug_outputf(true, "ES2Shd", "not supported attribute type '%X' in '%s'", gl_type, sz);
				continue;
			}

			const GLint gl_attrib = GL_FUNC(glGetAttribLocation)(gl_program, sz);
			if ((size_t)gl_attrib >= max_vertex_attrs)
			{
				// 이 어트리뷰트는 못쓴다
				qn_debug_outputf(true, "ES2Shd", "invalid attribute id '%d' in '%s'", gl_attrib, sz);
				continue;
			}

			GlAttrib* a = &qn_ctnr_nth(&self->attrs, index);
			qn_strncpy(a->name, QN_COUNTOF(a->name), sz, QN_COUNTOF(a->name) - 1);
			a->attrib = gl_attrib;
			a->size = gl_size;
			a->usage = gl_shd_attrib_to_usage(sz);
			a->cnst = cnst;
			a->hash = qn_strihash(a->name);

			self->attr_mask |= QN_BIT(index);
			index++;
		}
	}

	self->linked = true;
	return true;
}

//
void gl_shd_process_uniforms(GlShd* self)
{
	for (size_t i = 0; i < qn_ctnr_count(&self->uniforms); i++)
	{
		GlUniform* u = &qn_ctnr_nth(&self->uniforms, i);
		if (u->base.role == QGSHR_AUTO)
			u->auto_func(qg_rdh_instance, (GLint)u->base.offset, &u->base);
		else if (u->base.role == QGSHR_MANUAL && self->base.intr_func)
			(self->base.intr_func)(self->base.intr_data, &u->base, qm_cast(self, QgShd));
	}
}

// 자동 변수들

// QGSHA_ORTHO_PROJ
static void gl_shd_auto_otho_proj(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->tm.ortho);
}

// QGSHA_WORLD
static void gl_shd_auto_world(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->tm.world);
}

// QGSHA_VIEW
static void gl_shd_auto_view(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->tm.view);
}

// QGSHA_PROJ
static void gl_shd_auto_proj(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->tm.proj);
}

// QGSHA_VIEW_PROJ
static void gl_shd_auto_view_proj(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->tm.vipr);
}

// QGSHA_INV_VIEW
static void gl_shd_auto_inv_view(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->tm.inv);
}

// QGSHA_WORLD_VIEW_PROJ
static void gl_shd_auto_world_view_proj(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	QnMat4 m;
	qn_mat4_mul(&m, &rdh->tm.world, &rdh->tm.vipr);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&m);
}

// QGSHA_TEX0
static void gl_shd_auto_tex_0(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GL_FUNC(glUniform1i)(handle, 0);
}

// QGSHA_TEX1
static void gl_shd_auto_tex_1(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GL_FUNC(glUniform1i)(handle, 1);
}

// QGSHA_TEX2
static void gl_shd_auto_tex_2(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GL_FUNC(glUniform1i)(handle, 2);
}

// QGSHA_TEX3
static void gl_shd_auto_tex_3(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GL_FUNC(glUniform1i)(handle, 3);
}

// QGSHA_TEX4
static void gl_shd_auto_tex_4(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GL_FUNC(glUniform1i)(handle, 4);
}

// QGSHA_TEX5
static void gl_shd_auto_tex_5(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GL_FUNC(glUniform1i)(handle, 5);
}

// QGSHA_TEX6
static void gl_shd_auto_tex_6(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GL_FUNC(glUniform1i)(handle, 6);
}

// QGSHA_TEX7
static void gl_shd_auto_tex_7(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GL_FUNC(glUniform1i)(handle, 7);
}

// QGSHA_PROP_VEC0
static void gl_shd_auto_prop_vec_0(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GL_FUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->param.v[0]);
}

// QGSHA_PROP_VEC1
static void gl_shd_auto_prop_vec_1(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GL_FUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->param.v[1]);
}

// QGSHA_PROP_VEC2
static void gl_shd_auto_prop_vec_2(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GL_FUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->param.v[2]);
}

// QGSHA_PROP_VEC3
static void gl_shd_auto_prop_vec_3(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GL_FUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->param.v[3]);
}

// QGSHA_PROP_MAT0
static void gl_shd_auto_prop_mat_0(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->param.m[0]);
}

// QGSHA_PROP_MAT1
static void gl_shd_auto_prop_mat_1(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->param.m[1]);
}

// QGSHA_PROP_MAT2
static void gl_shd_auto_prop_mat_2(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->param.m[2]);
}

// QGSHA_PROP_MAT3
static void gl_shd_auto_prop_mat_3(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GL_FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->param.m[3]);
}

// QGSHA_MAT_PALETTE
static void gl_shd_auto_mat_palette(const QgRdh* rdh, const GLint handle, const QgVarShader* v)
{
	GL_FUNC(glUniformMatrix4fv)(handle, rdh->param.bones, false, (const GLfloat*)rdh->param.bonptr);
}

// 자동 변수
static struct GlShaderAutoUniforms
{
	bool				inited;
	struct AutoInfo
	{
		const char*			name;
		QgShdAuto			type;
		void(*func)(const QgRdh*, GLint, const QgVarShader*);
		size_t				hash;
	}					autos[QGSHA_MAX_VALUE];
} gl_shd_auto_uniforms =
{
	false,
	{
#define ES2_AUTO_FUNC(name, type, func) { "s" QN_STRING(name), type, func, 0 }
		ES2_AUTO_FUNC(OrthoProj, QGSHA_ORTHO_PROJ, gl_shd_auto_otho_proj),
		ES2_AUTO_FUNC(World, QGSHA_WORLD, gl_shd_auto_world),
		ES2_AUTO_FUNC(View, QGSHA_VIEW, gl_shd_auto_view),
		ES2_AUTO_FUNC(Proj, QGSHA_PROJ, gl_shd_auto_proj),
		ES2_AUTO_FUNC(ViewProj, QGSHA_VIEW_PROJ, gl_shd_auto_view_proj),
		ES2_AUTO_FUNC(InvView, QGSHA_INV_VIEW, gl_shd_auto_inv_view),
		ES2_AUTO_FUNC(WorldViewProj, QGSHA_WORLD_VIEW_PROJ, gl_shd_auto_world_view_proj),
		ES2_AUTO_FUNC(Tex0, QGSHA_TEX0, gl_shd_auto_tex_0),
		ES2_AUTO_FUNC(Tex1, QGSHA_TEX1, gl_shd_auto_tex_1),
		ES2_AUTO_FUNC(Tex2, QGSHA_TEX2, gl_shd_auto_tex_2),
		ES2_AUTO_FUNC(Tex3, QGSHA_TEX3, gl_shd_auto_tex_3),
		ES2_AUTO_FUNC(Tex4, QGSHA_TEX4, gl_shd_auto_tex_4),
		ES2_AUTO_FUNC(Tex5, QGSHA_TEX5, gl_shd_auto_tex_5),
		ES2_AUTO_FUNC(Tex6, QGSHA_TEX6, gl_shd_auto_tex_6),
		ES2_AUTO_FUNC(Tex7, QGSHA_TEX7, gl_shd_auto_tex_7),
		ES2_AUTO_FUNC(PropVec0, QGSHA_PROP_VEC0, gl_shd_auto_prop_vec_0),
		ES2_AUTO_FUNC(PropVec1, QGSHA_PROP_VEC1, gl_shd_auto_prop_vec_1),
		ES2_AUTO_FUNC(PropVec2, QGSHA_PROP_VEC2, gl_shd_auto_prop_vec_2),
		ES2_AUTO_FUNC(PropVec3, QGSHA_PROP_VEC3, gl_shd_auto_prop_vec_3),
		ES2_AUTO_FUNC(PropMat0, QGSHA_PROP_MAT0, gl_shd_auto_prop_mat_0),
		ES2_AUTO_FUNC(PropMat1, QGSHA_PROP_MAT1, gl_shd_auto_prop_mat_1),
		ES2_AUTO_FUNC(PropMat2, QGSHA_PROP_MAT2, gl_shd_auto_prop_mat_2),
		ES2_AUTO_FUNC(PropMat3, QGSHA_PROP_MAT3, gl_shd_auto_prop_mat_3),
		ES2_AUTO_FUNC(MatPalette, QGSHA_MAT_PALETTE, gl_shd_auto_mat_palette),
#undef ES2_AUTO_FUNC
	}
};

// 자동 초기화
void gl_shd_init_auto_uniforms(void)
{
	qn_ret_if_ok(gl_shd_auto_uniforms.inited);
	gl_shd_auto_uniforms.inited = true;

	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
		gl_shd_auto_uniforms.autos[i].hash = qn_strihash(gl_shd_auto_uniforms.autos[i].name);
}

// 자동 함수
static void gl_shd_match_auto_uniform(GlUniform* u)
{
	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
	{
		if (u->hash == gl_shd_auto_uniforms.autos[i].hash && qn_stricmp(u->base.name, gl_shd_auto_uniforms.autos[i].name) == 0)
		{
			u->base.role = QGSHR_AUTO;
			u->auto_func = gl_shd_auto_uniforms.autos[i].func;
			return;
		}
	}

	u->base.role = QGSHR_MANUAL;
	u->auto_func = NULL;
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

static void gl_buf_dispose(QmGam* g);
static void* gl_buf_map(QgBuf* g);
static bool gl_buf_unmap(QgBuf* g);
static bool gl_buf_data(QgBuf* g, const void* data);

qvt_name(QgBuf) vt_es2_buf =
{
	.base.name = "ES2Buf",
	.base.dispose = gl_buf_dispose,
	.map = gl_buf_map,
	.unmap = gl_buf_unmap,
	.data = gl_buf_data,
};

GlBuf* gl_buf_allocator(QgRdh* rdh, QgBufType type, int count, int stride, const void* data)
{
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
	GL_FUNC(glGenBuffers)(1, &gl_id);
	qn_retval_if_fail(gl_id != 0, NULL);
	gl_bind_buffer(GLBASE_RDH_INSTANCE, gl_type, gl_id);

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

	const GLsizeiptr size = (GLsizeiptr)count * stride;
	GL_FUNC(glBufferData)(gl_type, size, ptr, gl_usage);

	// 진짜 만듦
	GlBuf* self = qn_alloc_zero_1(GlBuf);
	if (self == NULL)
	{
		GL_FUNC(glDeleteBuffers)(1, &gl_id);
		return NULL;
	}

	qm_set_desc(self, gl_id);
	self->base.type = type;
	self->base.stride = (ushort)stride;
	self->base.size = (int)size;
	self->gl_type = gl_type;
	self->gl_usage = gl_usage;

	return qm_init(self, GlBuf, &vt_es2_buf);
}

static void gl_buf_dispose(QmGam* g)
{
	GlBuf* self = qm_cast(g, GlBuf);

	if (self->lockbuf)
		qn_free(self->lockbuf);

	const GLuint gl_handle = qm_get_desc(self, GLuint);
	GL_FUNC(glDeleteBuffers)(1, &gl_handle);

	qn_free(self);
}

static void* gl_buf_map(QgBuf* g)
{
	GlBuf* self = qm_cast(g, GlBuf);

	if (self->lockbuf != NULL)
		return NULL;
	if (self->gl_usage != GL_DYNAMIC_DRAW)
		return NULL;

	return self->lockbuf = qn_alloc(self->base.size, byte);
}

static bool gl_buf_unmap(QgBuf* g)
{
	GlBuf* self = qm_cast(g, GlBuf);
	qn_retval_if_fail(self->lockbuf == NULL, false);

	gl_bind_buffer(GLBASE_RDH_INSTANCE, self->gl_type, qm_get_desc(self, GLuint));
	GL_FUNC(glBufferData)(self->gl_type, self->base.size, self->lockbuf, self->gl_usage);

	qn_free_ptr(&self->lockbuf);

	return true;
}

static bool gl_buf_data(QgBuf* g, const void* data)
{
	qn_retval_if_fail(data, false);
	GlBuf* self = qm_cast(g, GlBuf);
	qn_retval_if_fail(self->lockbuf == NULL, false);
	qn_retval_if_fail(self->gl_usage == GL_DYNAMIC_DRAW, false);

	gl_bind_buffer(GLBASE_RDH_INSTANCE, self->gl_type, qm_get_desc(self, GLuint));
	GL_FUNC(glBufferData)(self->gl_type, self->base.size, data, self->gl_usage);

	return true;
}

