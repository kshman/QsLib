#include "pch.h"
#include "qg.h"
#include "qg_stub.h"
#include "qg_es2.h"

static bool _es2shd_link(qgShd* g);
static void es2shd_process_uniforms(es2Shd* self);

//
void es2_bind_buffer(es2Rdh* self, GLenum gl_type, GLuint gl_id)
{
	switch (gl_type)
	{
		case GL_ARRAY_BUFFER:
			if (self->ss.buf_array != gl_id)
			{
				ES2FUNC(glBindBuffer)(GL_ARRAY_BUFFER, gl_id);
				self->ss.buf_array = gl_id;
			}
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			if (self->ss.buf_element_array != gl_id)
			{
				ES2FUNC(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, gl_id);
				self->ss.buf_element_array = gl_id;
			}
			break;
		case GL_PIXEL_UNPACK_BUFFER_NV:
			if (self->ss.buf_pixel_unpack != gl_id)
			{
				ES2FUNC(glBindBuffer)(GL_PIXEL_UNPACK_BUFFER_NV, gl_id);
				self->ss.buf_pixel_unpack = gl_id;
			}
			break;
	}
}

//
void es2_commit_layout(es2Rdh* self)
{
	es2Shd* shd = self->pd.shd;
	es2Vlo* vlo = self->pd.vlo;

	const int max_attrs = qg_rdh_caps(self)->max_vertex_attrs;
	const es2CtnShaderAttrib* shd_attrs = &shd->attrs;

	uint ok = 0;
	QN_STATIC_ASSERT(ES2_MAX_VERTEX_ATTRIBUTES <= sizeof(ok) * 8, "Not enough for access ES2_MAX_VERTEX_ATTRIBUTES");

	for (int s = 0, index = 0; s < QGLOS_MAX_VALUE; s++)
	{
		es2Buf* buf = self->pd.vb[s];
		const int stcnt = vlo->es_cnt[s];

		if (buf != NULL)
		{
			// 정점 버퍼가 있다
			const es2LayoutElement* stelm = vlo->es_elm[s];

			GLsizei gl_stride = (GLsizei)vlo->base.stride[s];
			GLuint gl_buf = (GLuint)qm_get_desc(buf);
			es2_bind_buffer(self, GL_ARRAY_BUFFER, gl_buf);

			// 해당 스테이지의 레이아웃
			for (int i = 0; i < stcnt; i++)
			{
				if (index >= max_attrs)
					break;

				const es2LayoutElement* le = &stelm[i];
				if (qg_rdh_caps(self)->test_stage_valid)
				{
					if (!QN_TEST_BIT(shd->attr_mask, index))	// 세이더에 해당 데이터가 없으면 패스
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

				GLint gl_attr = qn_ctnr_nth(shd_attrs, index).attrib;
				ok |= QN_BIT(gl_attr);
				es2LayoutProperty* lp = &self->ss.layouts[gl_attr];
				void* pointer = (void*)(nuint)le->offset;
				if (!QN_TEST_BIT(self->ss.layout_mask, gl_attr))
				{
					QN_SET_BIT(&self->ss.layout_mask, gl_attr, true);
					ES2FUNC(glEnableVertexAttribArray)(gl_attr);
				}
				if (lp->pointer != pointer ||
					lp->buffer != gl_buf ||
					lp->stride != gl_stride ||
					lp->size != le->size ||
					lp->format != le->format ||
					lp->normalized != le->normalized)
				{
					ES2FUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, pointer);
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

				if (qg_rdh_caps(self)->test_stage_valid)
				{
					if (!QN_TEST_BIT(shd->attr_mask, index))	// 세이더에 해당 데이터가 없으면 패스
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

				GLint gl_attr = qn_ctnr_nth(shd_attrs, index).attrib;
				if (QN_TEST_BIT(self->ss.layout_mask, gl_attr))
				{
					QN_SET_BIT(&self->ss.layout_mask, gl_attr, false);
					ES2FUNC(glDisableVertexAttribArray)(gl_attr);
				}
				GLfloat tmp[4] = { 0.0f, };
				ES2FUNC(glVertexAttrib4fv)(gl_attr, tmp);

				index++;
			}
		}
	}

	// 정리
	uint aftermask = self->ss.layout_mask & ~ok;
	for (int i = 0; i < max_attrs && aftermask; i++)
	{
		if (QN_TEST_BIT(aftermask, 0))
		{
			QN_SET_BIT(&self->ss.layout_mask, i, false);
			ES2FUNC(glDisableVertexAttribArray)(i);
		}
		aftermask >>= 1;
	}
}

// 사용자 포인터 그리기
void es2_commit_layout_ptr(es2Rdh* self, const void* buffer, GLsizei gl_stride)
{
	es2Shd* shd = self->pd.shd;
	es2Vlo* vlo = self->pd.vlo;

	if (gl_stride == 0)
		gl_stride = (GLsizei)vlo->base.stride[0];
	else if (gl_stride != (GLsizei)vlo->base.stride[0])
	{
		qn_debug_outputf(true, "ES2Rdh", "stride is not same %d != %d", gl_stride, vlo->base.stride[0]);
		return;
	}

	const int max_attrs = qg_rdh_caps(self)->max_vertex_attrs;
	const es2CtnShaderAttrib* shd_attrs = &shd->attrs;

	es2LayoutElement* stelm = vlo->es_elm[0];	// 스테이지 0밖에 안씀. 버퍼가 하나니깐
	int stcnt = vlo->es_cnt[0];

	es2_bind_buffer(self, GL_ARRAY_BUFFER, 0);	// 버퍼 초기화 안하면 안된다.

	uint ok = 0;
	QN_STATIC_ASSERT(ES2_MAX_VERTEX_ATTRIBUTES <= sizeof(ok) * 8, "Not enough for access ES2_MAX_VERTEX_ATTRIBUTES");

	for (int s = 0, index = 0; s < stcnt; s++)
	{
		const es2LayoutElement* le = &stelm[s];
		if (qg_rdh_caps(self)->test_stage_valid)
		{
			if (!QN_TEST_BIT(shd->attr_mask, index))	// 세이더에 해당 데이터가 없으면 패스
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

		GLint gl_attr = qn_ctnr_nth(shd_attrs, index).attrib;
		ok |= QN_BIT(gl_attr);
		es2LayoutProperty* lp = &self->ss.layouts[gl_attr];
		const void* pointer = ((const byte*)buffer + le->offset);
		if (!QN_TEST_BIT(self->ss.layout_mask, gl_attr))
		{
			QN_SET_BIT(&self->ss.layout_mask, gl_attr, true);
			ES2FUNC(glEnableVertexAttribArray)(gl_attr);
		}
		if (lp->pointer != pointer ||
			lp->buffer != 0 ||
			lp->stride != gl_stride ||
			lp->size != le->size ||
			lp->format != le->format ||
			lp->normalized != le->normalized)
		{
			ES2FUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, pointer);
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
	uint aftermask = self->ss.layout_mask & ~ok;
	for (int i = 0; i < max_attrs && aftermask; i++)
	{
		if (QN_TEST_BIT(aftermask, 0))
		{
			QN_SET_BIT(&self->ss.layout_mask, i, false);
			ES2FUNC(glDisableVertexAttribArray)(i);
		}
		aftermask >>= 1;
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
			self->ss.program = 0;
			ES2FUNC(glUseProgram)(0);
		}
		return;
	}

	_es2shd_link((qgShd*)shd);
	GLuint gl_program = (GLuint)qm_get_desc(shd);
	if (self->ss.program != gl_program)
	{
		self->ss.program = gl_program;
		ES2FUNC(glUseProgram)(gl_program);
	}

	// 세이더 값 여기서 넣는다!!!
	es2shd_process_uniforms(shd);
}


//////////////////////////////////////////////////////////////////////////
// 레이아웃

static void _es2vlo_dispose(qgVlo* g);

qvt_name(qnGam) _vt_es2vlo =
{
	.name = "ES2Vlo",
	.dispose = (paramfunc_t)_es2vlo_dispose,
};

void* _es2vlo_allocator()
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
	qn_free(self);
}


//////////////////////////////////////////////////////////////////////////
// 세이더

static void es2shd_match_auto_uniform(es2ShaderUniform* v);

static void _es2shd_dispose(qgShd* g);
static bool _es2shd_bind(qgShd* g, qgShdType type, const void* data, int size, int flags);
static bool _es2shd_bind_shd(qgShd* g, qgShdType type, qgShd* shaderptr);

qvt_name(qgShd) _vt_es2shd =
{
	.base.name = "ES2Shd",
	.base.dispose = (paramfunc_t)_es2shd_dispose,
	.bind = _es2shd_bind,
	.bind_shd = _es2shd_bind_shd,
	//.bind_name = NULL,
	//.add_condition = NULL,
	//.clear_condition = NULL,
	.link = _es2shd_link,
};

static es2RefHandle* es2shd_handle_new(GLuint gl_program)
{
	es2RefHandle* ptr = qn_alloc_1(es2RefHandle);
	ptr->ref = 1;
	ptr->handle = gl_program;
	return ptr;
}

static void es2shd_handle_unload(es2RefHandle* ptr, GLuint gl_program)
{
	if (ptr)
	{
		if (gl_program > 0)
			ES2FUNC(glDetachShader)(gl_program, ptr->handle);

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
	es2Shd* self = qn_alloc_zero_1(es2Shd);
	qn_retval_if_fail(self, NULL);
	qm_set_desc(self, ES2FUNC(glCreateProgram)());
	if (name != NULL)
		qn_strncpy(self->base.name, QN_COUNTOF(self->base.name), name, QN_COUNTOF(self->base.name) - 1);
	else
		qn_snprintf(self->base.name, QN_COUNTOF(self->base.name), "%s#%Lu", "ES2Shd", qg_number());
	return qm_init(self, &_vt_es2shd);
}

static void _es2shd_dispose(qgShd* g)
{
	es2Shd* self = qm_cast(g, es2Shd);
	GLuint handle = (GLuint)qm_get_desc(self);

	es2shd_handle_unload(self->rvertex, handle);
	es2shd_handle_unload(self->rfragment, handle);

	ES2FUNC(glDeleteProgram)(handle);

	qn_ctnr_disp(es2CtnShaderUniform, &self->uniforms);
	qn_ctnr_disp(es2CtnShaderAttrib, &self->attrs);

	qn_free(self);
}

//
static void es2shd_error_program(GLuint handle, const char* progress)
{
	GLint gl_len = 0;
	ES2FUNC(glGetProgramiv)(handle, GL_INFO_LOG_LENGTH, &gl_len);
	if (gl_len <= 0)
		qn_debug_outputf(true, "ES2Shd", "program error in %s", progress);
	else
	{
		GLchar* psz = qn_alloca(gl_len, GLchar);
		ES2FUNC(glGetProgramInfoLog)(handle, gl_len, &gl_len, psz);
		qn_debug_outputf(true, "ES2Shd" "program error '%s'", psz);
		qn_freea(psz);
	}
}

//
static void es2shd_error_shader(GLuint handle, const char* progress)
{
	GLint gl_len = 0;
	ES2FUNC(glGetShaderiv)(handle, GL_INFO_LOG_LENGTH, &gl_len);
	if (gl_len <= 0)
		qn_debug_outputf(true, "ES2Shd", "shader error in %s", progress);
	else
	{
		GLchar* psz = qn_alloca(gl_len, GLchar);
		ES2FUNC(glGetShaderInfoLog)(handle, gl_len, &gl_len, psz);
		qn_debug_outputf(true, "ES2Shd", "shader error '%s' in %s", psz, progress);
		qn_freea(psz);
	}
}

//
static es2RefHandle* es2shd_compile(es2Shd* self, GLenum gl_type, const char* src, GLint gl_len)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	GLuint gl_shader = ES2FUNC(glCreateShader)(gl_type);
	qn_retval_if_fail(gl_shader != 0, NULL);

	ES2FUNC(glShaderSource)(gl_shader, 1, &src, gl_len == 0 ? NULL : &gl_len);
	ES2FUNC(glCompileShader)(gl_shader);

	GLint gl_status = 0;
	ES2FUNC(glGetShaderiv)(gl_shader, GL_COMPILE_STATUS, &gl_status);
	if (gl_status == GL_FALSE)
	{
		const char* s = gl_type == GL_VERTEX_SHADER ? "Vertex" : gl_type == GL_FRAGMENT_SHADER ? "Fragment" : "(unknown)";
		es2shd_error_shader(gl_shader, s);
		return NULL;
	}

	return es2shd_handle_new(gl_shader);
}

//
static bool _es2shd_bind(qgShd* g, qgShdType type, const void* data, int size, int flags)
{
	// 해야함: 매크로
	// 해야함: 인클루트
	// 해야함: 플래그
	// 해야함: 보관
	qn_retval_if_fail(data != NULL && size >= 0, false);
	es2Shd* self = qm_cast(g, es2Shd);
	GLuint handle = (GLuint)qm_get_desc(self);

	if (type == QGSHT_VS)
	{
		es2shd_handle_unload(self->rvertex, handle);
		if ((self->rvertex = es2shd_compile(self, GL_VERTEX_SHADER, (const char*)data, (GLint)size)) == NULL)
			return false;
		ES2FUNC(glAttachShader)(handle, self->rvertex->handle);
	}
	else if (type == QGSHT_PS)
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
	bool ok = false;

	if (QN_TEST_MASK(type, QGSHT_VS))
	{
		qn_retval_if_fail(shader->rvertex, false);
		es2shd_handle_unload(self->rvertex, handle);

		self->rvertex->ref++;
		ES2FUNC(glAttachShader)(handle, self->rvertex->handle);
		ok = true;
	}

	if (QN_TEST_MASK(type, QGSHT_PS))
	{
		qn_retval_if_fail(shader->rfragment, false);
		es2shd_handle_unload(self->rfragment, handle);

		self->rfragment->ref++;
		ES2FUNC(glAttachShader)(handle, self->rfragment->handle);
		ok = true;
	}

	if (!ok)
		return false;

	self->linked = false;
	return true;
}

//
static qgShdConst es2shd_enum_to_const(GLenum gl_type)
{
	switch (gl_type)
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

	GLuint gl_program = (GLuint)qm_get_desc(self);
	ES2FUNC(glLinkProgram)(gl_program);

	GLint gl_status = 0;
	ES2FUNC(glGetProgramiv)(gl_program, GL_LINK_STATUS, &gl_status);
	if (gl_status == GL_FALSE)
	{
		es2shd_error_program(gl_program, "Link");
		return false;
	}

	// 분석
	GLint gl_count;
	char sz[128];

	// 전역 변수 (유니폼)
	qn_ctnr_disp(es2CtnShaderUniform, &self->uniforms);

	ES2FUNC(glGetProgramiv)(gl_program, GL_ACTIVE_UNIFORMS, &gl_count);
	if (gl_count > 0)
	{
		qn_ctnr_init(es2CtnShaderUniform, &self->uniforms, gl_count);

		for (GLint i = 0, index = 0; i < gl_count; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			ES2FUNC(glGetActiveUniform)(gl_program, i, QN_COUNTOF(sz) - 1, NULL, &gl_size, &gl_type, sz);

			qgShdConst cnst = es2shd_enum_to_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)
			{
				// 사용할 수 없는 유니폼
				qn_debug_outputf(true, "ES2Shd", "not supported uniform type '%X' in '%s'", gl_type, sz);
				continue;
			}

			es2ShaderUniform* u = &qn_ctnr_nth(&self->uniforms, index);
			qn_strncpy(u->base.name, QN_COUNTOF(u->base.name), sz, QN_COUNTOF(u->base.name) - 1);
			u->base.cnst = cnst;
			u->base.size = (ushort)gl_size;
			u->base.offset = (uint)ES2FUNC(glGetUniformLocation)(gl_program, sz);
			u->hash = qn_strihash(u->base.name);
			es2shd_match_auto_uniform(u);

			index++;
		}
	}

	// 인수 (어트리뷰트)
	self->attr_mask = 0;
	qn_ctnr_disp(es2CtnShaderAttrib, &self->attrs);

	ES2FUNC(glGetProgramiv)(gl_program, GL_ACTIVE_ATTRIBUTES, &gl_count);
	if (gl_count > 0)
	{
		qn_ctnr_init(es2CtnShaderAttrib, &self->attrs, gl_count);

		for (GLint i = 0, index = 0; i < gl_count; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			GLsizei gl_len;
			ES2FUNC(glGetActiveAttrib)(gl_program, i, QN_COUNTOF(sz) - 1, &gl_len, &gl_size, &gl_type, sz);

			qgShdConst cnst = es2shd_enum_to_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)
			{
				// 사용할 수 없는 어트리뷰트
				qn_debug_outputf(true, "ES2Shd", "not supported attribute type '%X' in '%s'", gl_type, sz);
				continue;
			}

			GLint gl_attrib = ES2FUNC(glGetAttribLocation)(gl_program, sz);
			if (gl_attrib < 0 || gl_attrib >= ES2_MAX_VERTEX_ATTRIBUTES)
			{
				// 이 어트리뷰트는 못쓴다
				qn_debug_outputf(true, "ES2Shd", "invalid attribute id '%d' in '%s'", gl_attrib, sz);
				continue;
			}

			es2ShaderAttrib* a = &qn_ctnr_nth(&self->attrs, index);
			qn_strncpy(a->name, QN_COUNTOF(a->name), sz, QN_COUNTOF(a->name) - 1);
			a->attrib = gl_attrib;
			a->size = gl_size;
			a->usage = es2shd_attrib_to_usage(sz);
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
static void es2shd_process_uniforms(es2Shd* self)
{
	for (size_t i = 0; i < qn_ctnr_count(&self->uniforms); i++)
	{
		es2ShaderUniform* u = &qn_ctnr_nth(&self->uniforms, i);
		if (u->base.role == QGSHR_AUTO)
			u->auto_func((es2Rdh*)qg_rdh_instance, (GLint)u->base.offset, &u->base);
		else if (u->base.role == QGSHR_MANUAL && self->base.intr.func)
			((qgVarShaderFunc)self->base.intr.func)(self->base.intr.data, &u->base, qm_cast(self, qgShd));
	}
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
	ES2FUNC(glUniform1i)(handle, 0);
}

// QGSHA_TEX1
static void es2shd_auto_tex_1(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	ES2FUNC(glUniform1i)(handle, 1);
}

// QGSHA_TEX2
static void es2shd_auto_tex_2(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	ES2FUNC(glUniform1i)(handle, 2);
}

// QGSHA_TEX3
static void es2shd_auto_tex_3(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	ES2FUNC(glUniform1i)(handle, 3);
}

// QGSHA_TEX4
static void es2shd_auto_tex_4(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	ES2FUNC(glUniform1i)(handle, 4);
}

// QGSHA_TEX5
static void es2shd_auto_tex_5(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	ES2FUNC(glUniform1i)(handle, 5);
}

// QGSHA_TEX6
static void es2shd_auto_tex_6(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	ES2FUNC(glUniform1i)(handle, 6);
}

// QGSHA_TEX7
static void es2shd_auto_tex_7(es2Rdh* rdh, GLint handle, const qgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	ES2FUNC(glUniform1i)(handle, 7);
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
static bool _es2shd_auto_uniform_inited = false;

static struct es2ShaderAutoInfo
{
	const char*			name;
	qgShdAuto			type;
	void(*func)(es2Rdh*, GLint, const qgVarShader*);
	size_t				hash;
} _es2shd_auto_uniforms[QGSHA_MAX_VALUE] =
{
#define ES2SAI(name, type, func) { "s" QN_STRING(name), type, func, 0 }
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
void es2shd_init_auto_uniforms(void)
{
	qn_ret_if_ok(_es2shd_auto_uniform_inited);
	_es2shd_auto_uniform_inited = true;

	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
		_es2shd_auto_uniforms[i].hash = qn_strihash(_es2shd_auto_uniforms[i].name);
}

// 자동 함수
static void es2shd_match_auto_uniform(es2ShaderUniform* u)
{
	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
	{
		if (u->hash == _es2shd_auto_uniforms[i].hash && qn_stricmp(u->base.name, _es2shd_auto_uniforms[i].name) == 0)
		{
			u->base.role = QGSHR_AUTO;
			u->auto_func = _es2shd_auto_uniforms[i].func;
			return;
		}
	}

	u->base.role = QGSHR_MANUAL;
	u->auto_func = NULL;
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

static void _es2buf_dispose(qgBuf* g);
static void* _es2buf_map(qgBuf* g);
static bool _es2buf_unmap(qgBuf* g);
static bool _es2buf_data(qgBuf* g, const void* data);

qvt_name(qgBuf) _vt_es2buf =
{
	.base.name = "ES2Buf",
	.base.dispose = (paramfunc_t)_es2buf_dispose,
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
	self->base.stride = (ushort)stride;
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

	GLuint handle = (GLuint)qm_get_desc(self);
	ES2FUNC(glDeleteBuffers)(1, &handle);

	qn_free(self);
}

static void* _es2buf_map(qgBuf* g)
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

static bool _es2buf_data(qgBuf* g, const void* data)
{
	qn_retval_if_fail(data, false);
	es2Buf* self = qm_cast(g, es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);
	qn_retval_if_fail(self->gl_usage == GL_DYNAMIC_DRAW, false);

	es2_bind_buffer(ES2RDH_INSTANCE, self->gl_type, (GLuint)qm_get_desc(self));
	ES2FUNC(glBufferData)(self->gl_type, self->base.size, data, self->gl_usage);

	return true;
}

