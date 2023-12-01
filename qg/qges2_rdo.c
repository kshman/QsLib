﻿#include "pch.h"
#include "qg.h"
#include "qg_stub.h"
#include "qg_es2.h"

static bool _es2shd_link(qgShd* g);
es2ShaderAttrib* es2shd_find_attr(es2Shd* self, qgLoUsage usage, int index);

//
void es2_bind_buffer(es2Rdh* self, GLenum type, GLuint id)
{
	switch (type)
	{
		case GL_ARRAY_BUFFER:
			if (self->ss.buf_array != id)
			{
				ES2FUNC(glBindBuffer)(GL_ARRAY_BUFFER, id);
				self->ss.buf_array = id;
			}
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			if (self->ss.buf_element_array != id)
			{
				ES2FUNC(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, id);
				self->ss.buf_element_array = id;
			}
			break;
		case GL_PIXEL_UNPACK_BUFFER_NV:
			if (self->ss.buf_pixel_unpack != id)
			{
				ES2FUNC(glBindBuffer)(GL_PIXEL_UNPACK_BUFFER_NV, id);
				self->ss.buf_pixel_unpack = id;
			}
			break;
	}
}

//
void es2_commit_layout(es2Rdh* self)
{
	size_t max_attrs = self->base.caps.max_vertex_attrs;
	es2Shd* shd = self->pd.shd;
	es2Vlo* vlo = self->pd.vlo;

	bool ok[QGLOS_MAX_VALUE * 3] = { false, };
	for (size_t s = 0; s < max_attrs; s++)
	{
		es2Buf* buf = self->pd.vb[s];
		es2LayoutElement* stelm = vlo->es_elm[s];
		int stcnt = vlo->es_cnt[s];

		if (buf != NULL)
		{
			// 버퍼 셋
			int stride = vlo->base.stage[s];
			GLuint gl_buf = (GLuint)qm_get_desc(buf);
			es2_bind_buffer(self, GL_ARRAY_BUFFER, gl_buf);

			// 레이아웃
			for (int i = 0; i < stcnt; i++)
			{
				const es2LayoutElement* le = &stelm[i];
				es2ShaderAttrib* pav = es2shd_find_attr(shd, le->usage, le->index);
				if (pav != NULL && pav->attrib < max_attrs)
				{
					GLuint gl_attr = pav->attrib;
					es2LayoutProperty* lp = &self->ss.layouts[gl_attr];
					pointer_t pointer = ((uint8_t*)NULL + le->offset);
					ok[gl_attr] = true;
					if (!lp->enable)
					{
						ES2FUNC(glEnableVertexAttribArray)(gl_attr);
						lp->enable = true;
					}
					if (lp->pointer != pointer ||
						lp->buffer != gl_buf ||
						lp->stride != le->size ||
						lp->format != le->format ||
						lp->normalized != le->normalized)
					{
						ES2FUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, stride, pointer);
						lp->pointer = pointer;
						lp->buffer = gl_buf;
						lp->stride = stride;
						lp->size = le->size;
						lp->format = le->format;
						lp->normalized = le->normalized;
					}
				}
			}
		}
		else
		{
			// 버퍼 엄네
			for (int i = 0; i < stcnt; i++)
			{
				const es2LayoutElement* le = &stelm[i];
				es2ShaderAttrib* pav = es2shd_find_attr(shd, le->usage, le->index);
				if (pav != NULL && pav->attrib < max_attrs)
				{
					GLuint gl_attr = pav->attrib;
					es2LayoutProperty* lp = &self->ss.layouts[gl_attr];
					if (lp->enable)
					{
						ES2FUNC(glDisableVertexAttribArray)(gl_attr);
						lp->enable = true;
					}
					GLfloat tmp[4] = { 0.0f, };
					ES2FUNC(glVertexAttrib4fv)(gl_attr, tmp);
				}
			}
		}
	}

	// 정리
	for (size_t s = 0; s < max_attrs; s++)
	{
		if (!ok[s] && self->ss.layouts[s].enable)
		{
			ES2FUNC(glDisableVertexAttribArray)((GLint)s);
			self->ss.layouts[s].enable = false;
		}
	}
}

//
void es2_commit_shader(es2Rdh* self)
{
	es2Shd* shd = self->pd.shd;
	if (shd == NULL)
	{
		if (self->ss.program != 0)
		{
			ES2FUNC(glUseProgram)(0);
			self->ss.program = 0;
		}
		return;
	}

	_es2shd_link((qgShd*)shd);
	GLuint program = (GLuint)qm_get_desc(shd);
	if (self->ss.program != program)
	{
		ES2FUNC(glUseProgram)(program);
		self->ss.program = program;
	}
}


//////////////////////////////////////////////////////////////////////////
// 레이아웃

static void _es2vlo_dispose(qgVlo* g);

qvt_name(qnGam) _vt_es2vlo =
{
	.name = "ES2Vlo",
	.dispose = _es2vlo_dispose,
};

pointer_t _es2vlo_allocator()
{
	es2Vlo* self = qn_alloc_zero_1(es2Vlo);
	qn_retval_if_fail(self, NULL);
	return qm_init(self, &_vt_es2vlo);
}

static void _es2vlo_dispose(qgVlo* g)
{
	es2Vlo* self = qm_cast(g, es2Vlo);
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
		if (self->es_elm[i])
			qn_free(self->es_elm[i]);
}


//////////////////////////////////////////////////////////////////////////
// 세이더

typedef void(*es2shd_auto_type)(es2Rdh*, GLint, const qgVarShader*);

static void es2shd_init_auto_info(void);
static void es2shd_match_auto_info(qgVarShader* v);

static void _es2shd_dispose(qgShd* g);
static bool _es2shd_bind(qgShd* g, qgShdType type, cpointer_t data, int size, int flags);
static bool _es2shd_bind_shd(qgShd* g, qgShdType type, qgShd* shaderptr);

qvt_name(qgShd) _vt_es2shd =
{
	.base.name = "ES2Shd",
	.base.dispose = _es2shd_dispose,
	.bind = _es2shd_bind,
	.bind_shd = _es2shd_bind_shd,
	//.bind_name = NULL,
	//.add_condition = NULL,
	//.clear_condition = NULL,
	.link = _es2shd_link,
};

static es2RefHandle* es2shd_handle_new(GLuint handle)
{
	es2RefHandle* ptr = qn_alloc_1(es2RefHandle);
	ptr->ref = 1;
	ptr->handle = handle;
	return ptr;
}

static void es2shd_handle_unload(es2RefHandle* ptr, GLuint program)
{
	if (ptr)
	{
		if (program > 0)
			ES2FUNC(glDetachShader)(program, ptr->handle);

		intptr_t ref = --ptr->ref;
		if (ref == 0)
		{
			ES2FUNC(glDeleteShader)(ptr->handle);
			qn_free(ptr);
		}
	}
}

es2Shd* _es2shd_allocator(const char* name)
{
	es2shd_init_auto_info();
	es2Shd* self = qn_alloc_zero_1(es2Shd);
	qn_retval_if_fail(self, NULL);
	qm_set_desc(self, ES2FUNC(glCreateProgram)());
	if (name != NULL)
		qn_strncpy(self->base.name, 64, name, 63);
	else
		qn_snprintf(self->base.name, 64, "ES2Shd#%Lu", qg_number());
	return qm_init(self, &_vt_es2shd);
}

static void _es2shd_dispose(qgShd* g)
{
	es2Shd* self = qm_cast(g, es2Shd);

	es2shd_handle_unload(self->rvertex, 0);
	es2shd_handle_unload(self->rfragment, 0);

	ES2FUNC(glDeleteProgram)((GLuint)qm_get_desc(self));

	qn_ctnr_disp(es2CtnVarShader, &self->vars);
	qn_ctnr_disp(es2CtnShaderAttrib, &self->attrs);

	qn_free(self);
}

//
static void es2shd_error(GLuint handle, const char* progress, bool shader, bool program)
{
	qn_debug_output(false, "ES2Shd: compile error in '%s'\n", progress);

	if (shader)
	{
		GLint len = 0;
		ES2FUNC(glGetShaderiv)(handle, GL_INFO_LOG_LENGTH, &len);
		if (len > 0)
		{
			GLchar* psz = qn_alloca(len, GLchar);
			ES2FUNC(glGetShaderInfoLog)(handle, len, &len, psz);
			qn_debug_output(false, "ES2Shd: shader error '%s'\n", psz);
			qn_freea(psz);
		}
	}

	if (program)
	{
		GLint len = 0;
		ES2FUNC(glGetProgramiv)(handle, GL_INFO_LOG_LENGTH, &len);
		if (len > 0)
		{
			GLchar* psz = qn_alloca(len, GLchar);
			ES2FUNC(glGetProgramInfoLog)(handle, len, &len, psz);
			qn_debug_output(false, "ES2Shd: program error '%s'\n", psz);
			qn_freea(psz);
		}
	}

#if _QN_WINDOWS_
	if (IsDebuggerPresent())
		DebugBreak();
#else
	raise(SIGTRAP);
#endif
}

//
static es2RefHandle* es2shd_compile(es2Shd* self, GLenum type, const char* src, GLint len)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	GLuint shader = ES2FUNC(glCreateShader)(type);
	qn_retval_if_fail(shader != 0, NULL);

	ES2FUNC(glShaderSource)(shader, 1, &src, &len);
	ES2FUNC(glCompileShader)(shader);

	GLint status = 0;
	ES2FUNC(glGetShaderiv)(shader, GL_COMPILE_STATUS, &status);
	if (status != 0)
	{
		const char* s = type == GL_VERTEX_SHADER ? "Vertex" : type == GL_FRAGMENT_SHADER ? "Fragment" : "(unknown)";
		es2shd_error(shader, s, true, false);
		return NULL;
	}

	return es2shd_handle_new(shader);
}

//
static bool _es2shd_bind(qgShd* g, qgShdType type, cpointer_t data, int size, int flags)
{
	// 해야함: 매크로
	// 해야함: 인클루트
	// 해야함: 플래그
	// 해야함: 보관
	qn_retval_if_fail(data != NULL && size > 0, false);
	es2Shd* self = qm_cast(g, es2Shd);
	GLuint handle = (GLuint)qm_get_desc(self);

	if (type == QGSHT_VS)
	{
		es2shd_handle_unload(self->rvertex, handle);
		if ((self->rvertex = es2shd_compile(self, GL_VERTEX_SHADER, (const char*)data, (GLint)size)) == NULL)
			return false;
		ES2FUNC(glAttachShader)(handle, self->rvertex->handle);
	}
	else if (type = QGSHT_PS)
	{
		es2shd_handle_unload(self->rfragment, handle);
		if ((self->rfragment = es2shd_compile(self, GL_FRAGMENT_SHADER, (const char*)data, (GLint)size)) == NULL)
			return false;
		ES2FUNC(glAttachShader)(handle, self->rfragment->handle);
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
static bool _es2shd_bind_shd(qgShd* g, qgShdType type, qgShd* shaderptr)
{
	qn_retval_if_fail(shaderptr, false);
	es2Shd* self = qm_cast(g, es2Shd);
	es2Shd* shader = qm_cast(shaderptr, es2Shd);
	GLuint handle = (GLuint)qm_get_desc(self);

	if (type == QGSHT_VS)
	{
		qn_retval_if_fail(shader->rvertex, false);
		es2shd_handle_unload(self->rvertex, handle);

		self->rvertex->ref++;
		ES2FUNC(glAttachShader)(handle, self->rvertex->handle);
	}
	else if (type == QGSHT_PS)
	{
		qn_retval_if_fail(shader->rfragment, false);
		es2shd_handle_unload(self->rfragment, handle);

		self->rfragment->ref++;
		ES2FUNC(glAttachShader)(handle, self->rfragment->handle);
	}
	else
	{
		// 멍미!
		return false;
	}

	self->linked = false;
	return true;
}

//
qgShdConst es2shd_to_shader_const(GLenum type)
{
	switch (type)
	{
		case GL_FLOAT:				return QGSHC_FLOAT1;
		case GL_FLOAT_VEC2:			return QGSHC_FLOAT2;
		case GL_FLOAT_VEC3:			return QGSHC_FLOAT3;
		case GL_FLOAT_VEC4:			return QGSHC_FLOAT4;
		case GL_FLOAT_MAT4:			return QGSHC_FLOAT16;
		case GL_SAMPLER_2D:			return QGSHC_INT1;
		case GL_SAMPLER_CUBE:		return QGSHC_INT1;
		case GL_INT:				return QGSHC_INT1;
		case GL_INT_VEC2:			return QGSHC_INT2;
		case GL_INT_VEC3:			return QGSHC_INT3;
		case GL_INT_VEC4:			return QGSHC_INT4;
		case GL_BOOL:				return QGSHC_BYTE1;
		case GL_BOOL_VEC2:			return QGSHC_BYTE2;
		case GL_BOOL_VEC3:			return QGSHC_BYTE3;
		case GL_BOOL_VEC4:			return QGSHC_BYTE4;
#ifdef GL_SAMPLER_1D
		case GL_SAMPLER_1D:			return QGSHC_INT1;
#endif
#ifdef GL_SAMPLER_3D
		case GL_SAMPLER_3D:			return QGSHC_INT1;
#endif
#ifdef GL_SAMPLER_1D_SHADOW
		case GL_SAMPLER_1D_SHADOW:	return QGSHC_INT1;
#endif
#ifdef GL_SAMPLER_2D_SHADOW
		case GL_SAMPLER_2D_SHADOW:	return QGSHC_INT1;
#endif
	}
	return QGSHC_UNKNOWN;
}

//
qgLoUsage es2shd_attrib_to_usage(const char* name)
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
static bool _es2shd_link(qgShd* g)
{
	es2Shd* self = qm_cast(g, es2Shd);
	qn_retval_if_ok(self->linked, true);
	qn_retval_if_fail(self->rvertex && self->rfragment, false);

	GLuint handle = (GLuint)qm_get_desc(self);
	ES2FUNC(glLinkProgram)(handle);

	GLint status = 0;
	ES2FUNC(glGetProgramiv)(handle, GL_LINK_STATUS, &status);
	if (status != 0)
	{
		es2shd_error(handle, "Link", false, true);
		return false;
	}

	// 분석
	GLint count = 0, maxlen = 0;
	char sz[64];

	// 전역 변수 (유니폼)
	ES2FUNC(glGetProgramiv)(handle, GL_ACTIVE_UNIFORMS, &count);
	if (count > 0)
	{
		ES2FUNC(glGetProgramiv)(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxlen);
		if (maxlen == 0 || maxlen > 31)	// 오류 처리는 안하는데 브레이크는 검
			qn_debug_output(true, "ES2Shd: invalid maximum length of uniforms.\n");

		qn_ctnr_disp(es2CtnVarShader, &self->vars);
		qn_ctnr_init(es2CtnVarShader, &self->vars, count);

		for (GLint i = 0; i < count; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			ES2FUNC(glGetActiveUniform)(handle, i, 32 - 1, NULL, &gl_size, &gl_type, sz);

			qgShdConst cnst = es2shd_to_shader_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)	// 여기도 오류 처리는 안하고 브레이크만
				qn_debug_output(true, "ES2Shd: not supported uniform data type. (id: %X, name: '%s')\n", gl_type, self->base.name);

			qgVarShader* v = &qn_ctnr_nth(&self->vars, i);
			qn_strncpy(v->name, 32, sz, 32 - 1);
			v->hash = qn_strihash(v->name);
			v->offset = ES2FUNC(glGetUniformLocation)(handle, sz);
			v->size = (uint16_t)gl_size;
			v->cnst = cnst;
			es2shd_match_auto_info(v);
			v->xptr = NULL;
		}
	}

	// 인수 (어트리뷰트)
	ES2FUNC(glGetProgramiv)(handle, GL_ACTIVE_ATTRIBUTES, &count);
	if (count > 0)
	{
		ES2FUNC(glGetProgramiv)(handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxlen);
		/*
		if (maxlen == 0)
			// 이 경우는 드라이버가 gl_로 시작하는 애들을 인수로 처리했을 경우. 한편 사용자 인수가 없다는 말.
			// 고정 기능과 호환되는 기능을 사용할 경우이므로 인수로 처리할 필요는 없지만. 지금 우리가 쓰는건 GL이 아니고 ES2
			qn_debug_output(true, "ES2Shd: invalid maximum length of attribute.\n");
		*/

		qn_ctnr_disp(es2CtnShaderAttrib, &self->attrs);
		qn_ctnr_init(es2CtnShaderAttrib, &self->attrs, count);
		self->amask = 0;
		qn_zero(self->acount, QGLOU_MAX_VALUE, int);
		qn_zero(self->alink, QGLOU_MAX_VALUE, es2ShaderAttrib*);

		for (GLint i = 0; i < count; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			ES2FUNC(glGetActiveUniform)(handle, i, 32 - 1, NULL, &gl_size, &gl_type, sz);

			qgShdConst cnst = es2shd_to_shader_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)	// 여기도 오류 처리는 안하고 브레이크만
				qn_debug_output(true, "ES2Shd: not supported attribute data type. (id: %X, name: '%s')\n", gl_type, self->base.name);

			es2ShaderAttrib* a = &qn_ctnr_nth(&self->attrs, i);
			qn_strncpy(a->name, 32, sz, 32 - 1);
			a->hash = qn_strihash(a->name);
			a->cnst = cnst;
			a->size = gl_size;
			a->attrib = ES2FUNC(glGetAttribLocation)(handle, sz);
			if (a->attrib < 0)
			{
				a->usage = QGLOU_MAX_VALUE;
				a->index = UINT16_MAX;
				a->next = NULL;
			}
			else
			{
				a->usage = es2shd_attrib_to_usage(sz);
				if (a->usage == QGLOU_MAX_VALUE)
				{
					a->index = 0;
					a->next = NULL;
				}
				else
				{
					a->index = self->acount[a->usage]++;
					// 인덱스 검사용으로 저장. 단, 순서는 꺼꾸로 라는거
					a->next = self->alink[a->usage];
					self->alink[a->usage] = a;
				}
				self->amask |= QN_BIT(a->usage);
			}
		}
	}

	self->linked = true;
	return true;
}

//
es2ShaderAttrib* es2shd_find_attr(es2Shd* self, qgLoUsage usage, int index)
{
	es2ShaderAttrib* a = self->alink[usage];
	for (int i = 0; i < index; i++)
	{
		if (!a) break;
		a = a->next;
	}
	return a;
}

//
void es2shd_process_var(es2Shd* self)
{
	for (size_t i = 0; i < qn_ctnr_count(&self->vars); i++)
	{
		qgVarShader* v = &qn_ctnr_nth(&self->vars, i);
		if (v->role == QGSHR_AUTO)
			((es2shd_auto_type)v->aptr)((es2Rdh*)qg_rdh_instance, (GLint)v->offset, v);
	}

	if (self->base.intr.func)
		((qgVarShaderFunc)self->base.intr.func)(self->base.intr.data, (int)self->vars.count, self->vars.data, (qgShd*)self);
}

// 자동 변수들

// QGSHA_ORTHO_PROJ
static void es2shd_auto_otho_proj(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.ortho);
}

// QGSHA_WORLD
static void es2shd_auto_world(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.world);
}

// QGSHA_VIEW
static void es2shd_auto_view(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.view);
}

// QGSHA_PROJ
static void es2shd_auto_proj(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.proj);
}

// QGSHA_VIEW_PROJ
static void es2shd_auto_view_proj(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.vp);
}

// QGSHA_INV_VIEW
static void es2shd_auto_inv_view(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.inv);
}

// QGSHA_WORLD_VIEW_PROJ
static void es2shd_auto_world_view_proj(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	qnMat4 m;
	qn_mat4_mul(&m, &rdh->base.tm.world, &rdh->base.tm.vp);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&m);
}

// QGSHA_TEX0
static void es2shd_auto_tex_0(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	//glUniform1i(handle, 0);
}

// QGSHA_TEX1
static void es2shd_auto_tex_1(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	//glUniform1i(handle, 1);
}

// QGSHA_TEX2
static void es2shd_auto_tex_2(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	//glUniform1i(handle, 2);
}

// QGSHA_TEX3
static void es2shd_auto_tex_3(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
}

// QGSHA_TEX4
static void es2shd_auto_tex_4(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
}

// QGSHA_TEX5
static void es2shd_auto_tex_5(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
}

// QGSHA_TEX6
static void es2shd_auto_tex_6(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
}

// QGSHA_TEX7
static void es2shd_auto_tex_7(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
}

// QGSHA_PROP_VEC0
static void es2shd_auto_prop_vec_0(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	ES2FUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[0]);
}

// QGSHA_PROP_VEC1
static void es2shd_auto_prop_vec_1(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	ES2FUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[1]);
}

// QGSHA_PROP_VEC2
static void es2shd_auto_prop_vec_2(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	ES2FUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[2]);
}

// QGSHA_PROP_VEC3
static void es2shd_auto_prop_vec_3(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	ES2FUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[3]);
}

// QGSHA_PROP_MAT0
static void es2shd_auto_prop_mat_0(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[0]);
}

// QGSHA_PROP_MAT1
static void es2shd_auto_prop_mat_1(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[1]);
}

// QGSHA_PROP_MAT2
static void es2shd_auto_prop_mat_2(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[2]);
}

// QGSHA_PROP_MAT3
static void es2shd_auto_prop_mat_3(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	ES2FUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[3]);
}

// QGSHA_MAT_PALETTE
static void es2shd_auto_mat_palette(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	ES2FUNC(glUniformMatrix4fv)(handle, rdh->base.param.bones, false, (const GLfloat*)rdh->base.param.bonptr);
}

// 자동 변수
static bool _es2sai_inited = false;

static struct es2ShaderAutoInfo
{
	size_t				hash;
	const char*			name;
	qgShdAuto			type;
	void(*func)(es2Rdh*, GLint, const qgVarShader*);
} _es2sai[QGSHA_MAX_VALUE] =
{
#define ES2SAI(name, type, func) { 0, "s" QN_STRING(name), type, func }
	ES2SAI(OrthoProj, QGSHA_ORTHO_PROJ, es2shd_auto_otho_proj),
	ES2SAI(World, QGSHA_WORLD, es2shd_auto_world),
	ES2SAI(View, QGSHA_VIEW, es2shd_auto_view),
	ES2SAI(Proj, QGSHA_PROJ, es2shd_auto_proj),
	ES2SAI(ViewProj, QGSHA_VIEW_PROJ, es2shd_auto_view_proj),
	ES2SAI(InvView, QGSHA_INV_VIEW, es2shd_auto_inv_view),
	ES2SAI(WorldViewProj, QGSHA_WORLD_VIEW_PROJ, es2shd_auto_world_view_proj),
	ES2SAI(Tex0, QGSHA_TEX0, es2shd_auto_tex_0),
	ES2SAI(Tex1, QGSHA_TEX1, es2shd_auto_tex_1),
	ES2SAI(Tex2, QGSHA_TEX2, es2shd_auto_tex_2),
	ES2SAI(Tex3, QGSHA_TEX3, es2shd_auto_tex_3),
	ES2SAI(Tex4, QGSHA_TEX4, es2shd_auto_tex_4),
	ES2SAI(Tex5, QGSHA_TEX5, es2shd_auto_tex_5),
	ES2SAI(Tex6, QGSHA_TEX6, es2shd_auto_tex_6),
	ES2SAI(Tex7, QGSHA_TEX7, es2shd_auto_tex_7),
	ES2SAI(PropVec0, QGSHA_PROP_VEC0, es2shd_auto_prop_vec_0),
	ES2SAI(PropVec1, QGSHA_PROP_VEC1, es2shd_auto_prop_vec_1),
	ES2SAI(PropVec2, QGSHA_PROP_VEC2, es2shd_auto_prop_vec_2),
	ES2SAI(PropVec3, QGSHA_PROP_VEC3, es2shd_auto_prop_vec_3),
	ES2SAI(PropMat0, QGSHA_PROP_MAT0, es2shd_auto_prop_mat_0),
	ES2SAI(PropMat1, QGSHA_PROP_MAT1, es2shd_auto_prop_mat_1),
	ES2SAI(PropMat2, QGSHA_PROP_MAT2, es2shd_auto_prop_mat_2),
	ES2SAI(PropMat3, QGSHA_PROP_MAT3, es2shd_auto_prop_mat_3),
	ES2SAI(MatPalette, QGSHA_MAT_PALETTE, es2shd_auto_mat_palette),
#undef ES2SAI
};

// 자동 초기화
static void es2shd_init_auto_info(void)
{
	qn_ret_if_ok(_es2sai_inited);
	_es2sai_inited = true;

	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
		_es2sai[i].hash = qn_strihash(_es2sai[i].name);
}

// 자동 함수
static void es2shd_match_auto_info(qgVarShader* v)
{
	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
	{
		if (v->hash == _es2sai[i].hash && qn_stricmp(v->name, _es2sai[i].name) == 0)
		{
			v->role = QGSHR_AUTO;
			v->aptr = (pointer_t)_es2sai[i].func;
			return;
		}
	}

	v->role = QGSHR_MANUAL;
	v->aptr = NULL;
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

static void _es2buf_dispose(qgBuf* g);
static pointer_t _es2buf_map(qgBuf* g);
static bool _es2buf_unmap(qgBuf* g);
static bool _es2buf_data(qgBuf* g, cpointer_t data);

qvt_name(qgBuf) _vt_es2buf =
{
	.base.name = "ES2Buf",
	.base.dispose = _es2buf_dispose,
	.map = _es2buf_map,
	.unmap = _es2buf_unmap,
	.data = _es2buf_data,
};

es2Buf* _es2buf_allocator(GLuint gl_id, GLenum gl_type, GLenum gl_usage, int stride, int size, qgBufType type)
{
	es2Buf* self = qn_alloc_zero_1(es2Buf);
	qn_retval_if_fail(self, NULL);

	qm_set_desc(self, gl_id);
	self->base.type = type;
	self->base.stride = stride;
	self->base.size = size;
	self->gl_type = gl_type;
	self->gl_usage = gl_usage;

	return qm_init(self, &_vt_es2buf);
}

static void _es2buf_dispose(qgBuf* g)
{
	es2Buf* self = qm_cast(g, es2Buf);

	if (self->lockbuf)
		qn_free(self->lockbuf);

	GLuint id = (GLuint)qm_get_desc(self);
	ES2FUNC(glDeleteBuffers)(1, &id);

	qn_free(self);
}

static pointer_t _es2buf_map(qgBuf* g)
{
	es2Buf* self = qm_cast(g, es2Buf);

	if (self->lockbuf != NULL)
		return NULL;
	if (self->gl_usage != GL_DYNAMIC_DRAW)
		return NULL;

	return self->lockbuf = qn_alloc(self->base.size, uint8_t);
}

static bool _es2buf_unmap(qgBuf* g)
{
	es2Buf* self = qm_cast(g, es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);

	es2_bind_buffer(ES2RDH_INSTANCE, self->gl_type, (GLuint)qm_get_desc(self));
	ES2FUNC(glBufferData)(self->gl_type, self->base.size, self->lockbuf, self->gl_usage);

	qn_free_ptr(&self->lockbuf);

	return true;
}

static bool _es2buf_data(qgBuf* g, cpointer_t data)
{
	qn_retval_if_fail(data, false);
	es2Buf* self = qm_cast(g, es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);
	qn_retval_if_fail(self->gl_usage == GL_DYNAMIC_DRAW, false);

	es2_bind_buffer(ES2RDH_INSTANCE, self->gl_type, (GLuint)qm_get_desc(self));
	ES2FUNC(glBufferData)(self->gl_type, self->base.size, data, self->gl_usage);

	return true;
}

