#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qg_es2.h"
#include "qg_glsupp.h"

static bool es2shd_link(QgShd* g);
static void es2shd_process_uniforms(es2Shd* self);

//
void es2_bind_buffer(es2Rdh* self, GLenum gl_type, GLuint gl_id)
{
	switch (gl_type)
	{
		case GL_ARRAY_BUFFER:
			if (self->ss.buf_array != gl_id)
			{
				GLFUNC(glBindBuffer)(GL_ARRAY_BUFFER, gl_id);
				self->ss.buf_array = gl_id;
			}
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			if (self->ss.buf_element_array != gl_id)
			{
				GLFUNC(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, gl_id);
				self->ss.buf_element_array = gl_id;
			}
			break;
		case GL_PIXEL_UNPACK_BUFFER_NV:
			if (self->ss.buf_pixel_unpack != gl_id)
			{
				GLFUNC(glBindBuffer)(GL_PIXEL_UNPACK_BUFFER_NV, gl_id);
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

	const int max_attrs = rdh_caps(self)->max_vertex_attrs;
	const es2CtnAttrib* shd_attrs = &shd->attrs;

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
			GLuint gl_buf = qm_get_desc(buf, GLuint);
			es2_bind_buffer(self, GL_ARRAY_BUFFER, gl_buf);

			// 해당 스테이지의 레이아웃
			for (int i = 0; i < stcnt; i++)
			{
				if (index >= max_attrs)
					break;

				const es2LayoutElement* le = &stelm[i];
				if (rdh_caps(self)->test_stage_valid)
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
					GLFUNC(glEnableVertexAttribArray)(gl_attr);
				}
				if (lp->pointer != pointer ||
					lp->buffer != gl_buf ||
					lp->stride != gl_stride ||
					lp->size != le->size ||
					lp->format != le->format ||
					lp->normalized != le->normalized)
				{
					GLFUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, pointer);
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

				if (rdh_caps(self)->test_stage_valid)
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
					GLFUNC(glDisableVertexAttribArray)(gl_attr);
				}
				GLfloat tmp[4] = { 0.0f, };
				GLFUNC(glVertexAttrib4fv)(gl_attr, tmp);

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
			GLFUNC(glDisableVertexAttribArray)(i);
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

	const int max_attrs = rdh_caps(self)->max_vertex_attrs;
	const es2CtnAttrib* shd_attrs = &shd->attrs;

	es2LayoutElement* stelm = vlo->es_elm[0];	// 스테이지 0밖에 안씀. 버퍼가 하나니깐
	int stcnt = vlo->es_cnt[0];

	es2_bind_buffer(self, GL_ARRAY_BUFFER, 0);	// 버퍼 초기화 안하면 안된다.

	uint ok = 0;
	QN_STATIC_ASSERT(ES2_MAX_VERTEX_ATTRIBUTES <= sizeof(ok) * 8, "Not enough for access ES2_MAX_VERTEX_ATTRIBUTES");

	for (int s = 0, index = 0; s < stcnt; s++)
	{
		const es2LayoutElement* le = &stelm[s];
		if (rdh_caps(self)->test_stage_valid)
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
			GLFUNC(glEnableVertexAttribArray)(gl_attr);
		}
		if (lp->pointer != pointer ||
			lp->buffer != 0 ||
			lp->stride != gl_stride ||
			lp->size != le->size ||
			lp->format != le->format ||
			lp->normalized != le->normalized)
		{
			GLFUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, pointer);
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
			GLFUNC(glDisableVertexAttribArray)(i);
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
			GLFUNC(glUseProgram)(0);
		}
		return;
	}

	es2shd_link((QgShd*)shd);
	GLuint gl_program = qm_get_desc(shd, GLuint);
	if (self->ss.program != gl_program)
	{
		self->ss.program = gl_program;
		GLFUNC(glUseProgram)(gl_program);
	}

	// 세이더 값 여기서 넣는다!!!
	es2shd_process_uniforms(shd);
}


//////////////////////////////////////////////////////////////////////////
// 레이아웃

static void _es2vlo_dispose(QmGam* g);

qvt_name(QmGam) _vt_es2vlo =
{
	.name = "ES2Vlo",
	.dispose = _es2vlo_dispose,
};

//
es2Vlo* es2vlo_allocator(es2Rdh* rdh, int count, const QgPropLayout* layouts)
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

	es2Vlo* self = qn_alloc_zero_1(es2Vlo);
	qn_retval_if_fail(self, NULL);

	// 스테이지 할당
	es2LayoutElement* elms[QGLOS_MAX_VALUE] = { NULL, };
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (accum[i] == 0)
			continue;
		self->es_cnt[i] = (byte)accum[i];
		self->es_elm[i] = elms[i] = qn_alloc(accum[i], es2LayoutElement);
	}

	// 데이터 작성
	ushort offset[QGLOS_MAX_VALUE] = { 0, };
	for (int i = 0; i < count; i++)
	{
		const QgPropLayout* l = &layouts[i];
		es2LayoutElement* e = elms[l->stage]++;
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

	return qm_init(self, es2Vlo, &_vt_es2vlo);
}

//
static void _es2vlo_dispose(QmGam* g)
{
	es2Vlo* self = qm_cast(g, es2Vlo);
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
		if (self->es_elm[i])
			qn_free(self->es_elm[i]);
	qn_free(self);
}


//////////////////////////////////////////////////////////////////////////
// 세이더

static void es2shd_match_auto_uniform(es2Uniform* v);

static void _es2shd_dispose(QmGam* g);
static bool _es2shd_bind(QgShd* g, QgShdType type, const void* data, int size, int flags);
static bool _es2shd_bind_shd(QgShd* g, QgShdType type, QgShd* shaderptr);

qvt_name(QgShd) _vt_es2shd =
{
	.base.name = "ES2Shd",
	.base.dispose = (paramfunc_t)_es2shd_dispose,
	.bind = _es2shd_bind,
	.bind_shd = _es2shd_bind_shd,
	//.bind_name = NULL,
	//.add_condition = NULL,
	//.clear_condition = NULL,
	.link = es2shd_link,
};

//
static es2RefHandle* es2shd_handle_new(GLuint gl_program)
{
	es2RefHandle* ptr = qn_alloc_1(es2RefHandle);
	ptr->ref = 1;
	ptr->handle = gl_program;
	return ptr;
}

//
static void es2shd_handle_unload(es2RefHandle* ptr, GLuint gl_program)
{
	if (ptr)
	{
		if (gl_program > 0)
			GLFUNC(glDetachShader)(gl_program, ptr->handle);

		intptr_t ref = --ptr->ref;
		if (ref == 0)
		{
			GLFUNC(glDeleteShader)(ptr->handle);
			qn_free(ptr);
		}
	}
}

//
es2Shd* es2shd_allocator(es2Rdh* rdh, const char* name)
{
	es2Shd* self = qn_alloc_zero_1(es2Shd);
	qn_retval_if_fail(self, NULL);
	qm_set_desc(self, GLFUNC(glCreateProgram)());
	return qm_init(self, es2Shd, &_vt_es2shd);
}

//
static void _es2shd_dispose(QmGam* g)
{
	es2Shd* self = qm_cast(g, es2Shd);
	GLuint handle = qm_get_desc(self, GLuint);

	es2shd_handle_unload(self->rvertex, handle);
	es2shd_handle_unload(self->rfragment, handle);

	GLFUNC(glDeleteProgram)(handle);

	qn_ctnr_disp(es2CtnUniform, &self->uniforms);
	qn_ctnr_disp(es2CtnAttrib, &self->attrs);

	qn_free(self);
}

//
static es2RefHandle* es2shd_compile(es2Shd* self, GLenum gl_type, const char* src, GLint gl_len)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	GLuint gl_shader = GLFUNC(glCreateShader)(gl_type);
	qn_retval_if_fail(gl_shader != 0, NULL);

	GLFUNC(glShaderSource)(gl_shader, 1, &src, gl_len == 0 ? NULL : &gl_len);
	GLFUNC(glCompileShader)(gl_shader);

	if (gl_get_shader_iv(gl_shader, GL_COMPILE_STATUS) == GL_FALSE)
	{
		const char* progress = gl_type == GL_VERTEX_SHADER ? "vertex" : gl_type == GL_FRAGMENT_SHADER ? "fragment" : "(unknown)";
		GLsizei gl_log_len = gl_get_shader_iv(gl_shader, GL_INFO_LOG_LENGTH);
		if (gl_log_len<=0)
			qn_debug_outputf(true, "ES2Shd", "%s shader error", progress);
		else
		{
			GLchar* psz = qn_alloca(gl_log_len, GLchar);
			GLFUNC(glGetShaderInfoLog)(gl_shader, gl_log_len, &gl_log_len, psz);
			qn_debug_outputf(true, "ES2Shd", "%s shader error in %s", psz, progress);
			qn_freea(psz);
		}
		return NULL;
	}

	return es2shd_handle_new(gl_shader);
}

//
static bool _es2shd_bind(QgShd* g, QgShdType type, const void* data, int size, int flags)
{
	// 해야함: 매크로
	// 해야함: 인클루트
	// 해야함: 플래그
	// 해야함: 보관
	qn_retval_if_fail(data != NULL && size >= 0, false);
	es2Shd* self = qm_cast(g, es2Shd);
	GLuint handle = qm_get_desc(self, GLuint);

	if (type == QGSHT_VS)
	{
		es2shd_handle_unload(self->rvertex, handle);
		if ((self->rvertex = es2shd_compile(self, GL_VERTEX_SHADER, (const char*)data, (GLint)size)) == NULL)
			return false;
		GLFUNC(glAttachShader)(handle, self->rvertex->handle);
}
	else if (type == QGSHT_PS)
	{
		es2shd_handle_unload(self->rfragment, handle);
		if ((self->rfragment = es2shd_compile(self, GL_FRAGMENT_SHADER, (const char*)data, (GLint)size)) == NULL)
			return false;
		GLFUNC(glAttachShader)(handle, self->rfragment->handle);
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
static bool _es2shd_bind_shd(QgShd* g, QgShdType type, QgShd* shaderptr)
{
	qn_retval_if_fail(shaderptr, false);
	es2Shd* self = qm_cast(g, es2Shd);
	es2Shd* shader = qm_cast(shaderptr, es2Shd);
	GLuint handle = qm_get_desc(self, GLuint);
	bool ok = false;

	if (QN_TEST_MASK(type, QGSHT_VS))
	{
		qn_retval_if_fail(shader->rvertex, false);
		es2shd_handle_unload(self->rvertex, handle);

		self->rvertex->ref++;
		GLFUNC(glAttachShader)(handle, self->rvertex->handle);
		ok = true;
	}

	if (QN_TEST_MASK(type, QGSHT_PS))
	{
		qn_retval_if_fail(shader->rfragment, false);
		es2shd_handle_unload(self->rfragment, handle);

		self->rfragment->ref++;
		GLFUNC(glAttachShader)(handle, self->rfragment->handle);
		ok = true;
	}

	if (!ok)
		return false;

	self->linked = false;
	return true;
}

//
static QgShdConst es2shd_enum_to_const(GLenum gl_type)
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
QgLoUsage es2shd_attrib_to_usage(const char* name)
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
static bool es2shd_link(QgShd* g)
{
	es2Shd* self = qm_cast(g, es2Shd);
	qn_retval_if_ok(self->linked, true);
	qn_retval_if_fail(self->rvertex && self->rfragment, false);

	GLuint gl_program = qm_get_desc(self, GLuint);
	GLFUNC(glLinkProgram)(gl_program);

	if (gl_get_program_iv(gl_program, GL_LINK_STATUS) == GL_FALSE)
	{
		GLsizei gl_log_len = gl_get_program_iv(gl_program, GL_INFO_LOG_LENGTH);
		if (gl_log_len<=0)
			qn_debug_outputs(true, "ES2Shd", "program link error");
		else
		{
			GLchar* psz = qn_alloca(gl_log_len, GLchar);
			GLFUNC(glGetProgramInfoLog)(gl_program, gl_log_len, &gl_log_len, psz);
			qn_debug_outputf(true, "ES2Shd" "program link error '%s'", psz);
			qn_freea(psz);
		}
		return false;
	}

	// 분석
	GLint gl_count;
	GLchar sz[128];

	// 전역 변수 (유니폼)
	qn_ctnr_disp(es2CtnUniform, &self->uniforms);
	gl_count = gl_get_program_iv(gl_program, GL_ACTIVE_UNIFORMS);
	if (gl_count > 0)
	{
		qn_ctnr_init(es2CtnUniform, &self->uniforms, gl_count);

		for (GLint i = 0, index = 0; i < gl_count; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			GLFUNC(glGetActiveUniform)(gl_program, i, QN_COUNTOF(sz) - 1, NULL, &gl_size, &gl_type, sz);

			QgShdConst cnst = es2shd_enum_to_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)
			{
				// 사용할 수 없는 유니폼
				qn_debug_outputf(true, "ES2Shd", "not supported uniform type '%X' in '%s'", gl_type, sz);
				continue;
			}

			es2Uniform* u = &qn_ctnr_nth(&self->uniforms, index);
			qn_strncpy(u->base.name, QN_COUNTOF(u->base.name), sz, QN_COUNTOF(u->base.name) - 1);
			u->base.cnst = cnst;
			u->base.size = (ushort)gl_size;
			u->base.offset = (uint)GLFUNC(glGetUniformLocation)(gl_program, sz);
			u->hash = qn_strihash(u->base.name);
			es2shd_match_auto_uniform(u);

			index++;
		}
	}

	// 인수 (어트리뷰트)
	self->attr_mask = 0;
	qn_ctnr_disp(es2CtnAttrib, &self->attrs);
	gl_count = gl_get_program_iv(gl_program, GL_ACTIVE_ATTRIBUTES);
	if (gl_count > 0)
	{
		qn_ctnr_init(es2CtnAttrib, &self->attrs, gl_count);

		for (GLint i = 0, index = 0; i < gl_count; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			GLsizei gl_len;
			GLFUNC(glGetActiveAttrib)(gl_program, i, QN_COUNTOF(sz) - 1, &gl_len, &gl_size, &gl_type, sz);

			QgShdConst cnst = es2shd_enum_to_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)
			{
				// 사용할 수 없는 어트리뷰트
				qn_debug_outputf(true, "ES2Shd", "not supported attribute type '%X' in '%s'", gl_type, sz);
				continue;
			}

			GLint gl_attrib = GLFUNC(glGetAttribLocation)(gl_program, sz);
			if ((size_t)gl_attrib >= ES2_MAX_VERTEX_ATTRIBUTES)
			{
				// 이 어트리뷰트는 못쓴다
				qn_debug_outputf(true, "ES2Shd", "invalid attribute id '%d' in '%s'", gl_attrib, sz);
				continue;
			}

			es2Attrib* a = &qn_ctnr_nth(&self->attrs, index);
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
		es2Uniform* u = &qn_ctnr_nth(&self->uniforms, i);
		if (u->base.role == QGSHR_AUTO)
			u->auto_func((es2Rdh*)qg_rdh_instance, (GLint)u->base.offset, &u->base);
		else if (u->base.role == QGSHR_MANUAL && self->base.intr.func)
			((QgVarShaderFunc)self->base.intr.func)(self->base.intr.data, &u->base, qm_cast(self, QgShd));
	}
}

// 자동 변수들

// QGSHA_ORTHO_PROJ
static void es2shd_auto_otho_proj(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.ortho);
}

// QGSHA_WORLD
static void es2shd_auto_world(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.world);
}

// QGSHA_VIEW
static void es2shd_auto_view(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.view);
}

// QGSHA_PROJ
static void es2shd_auto_proj(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.proj);
}

// QGSHA_VIEW_PROJ
static void es2shd_auto_view_proj(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.vipr);
}

// QGSHA_INV_VIEW
static void es2shd_auto_inv_view(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.inv);
}

// QGSHA_WORLD_VIEW_PROJ
static void es2shd_auto_world_view_proj(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	QnMat4 m;
	qn_mat4_mul(&m, &rdh->base.tm.world, &rdh->base.tm.vipr);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&m);
}

// QGSHA_TEX0
static void es2shd_auto_tex_0(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 0);
}

// QGSHA_TEX1
static void es2shd_auto_tex_1(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 1);
}

// QGSHA_TEX2
static void es2shd_auto_tex_2(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 2);
}

// QGSHA_TEX3
static void es2shd_auto_tex_3(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 3);
}

// QGSHA_TEX4
static void es2shd_auto_tex_4(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 4);
}

// QGSHA_TEX5
static void es2shd_auto_tex_5(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 5);
}

// QGSHA_TEX6
static void es2shd_auto_tex_6(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 6);
}

// QGSHA_TEX7
static void es2shd_auto_tex_7(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 7);
}

// QGSHA_PROP_VEC0
static void es2shd_auto_prop_vec_0(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GLFUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[0]);
}

// QGSHA_PROP_VEC1
static void es2shd_auto_prop_vec_1(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GLFUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[1]);
}

// QGSHA_PROP_VEC2
static void es2shd_auto_prop_vec_2(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GLFUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[2]);
}

// QGSHA_PROP_VEC3
static void es2shd_auto_prop_vec_3(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GLFUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[3]);
}

// QGSHA_PROP_MAT0
static void es2shd_auto_prop_mat_0(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[0]);
}

// QGSHA_PROP_MAT1
static void es2shd_auto_prop_mat_1(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[1]);
}

// QGSHA_PROP_MAT2
static void es2shd_auto_prop_mat_2(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[2]);
}

// QGSHA_PROP_MAT3
static void es2shd_auto_prop_mat_3(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[3]);
}

// QGSHA_MAT_PALETTE
static void es2shd_auto_mat_palette(es2Rdh* rdh, GLint handle, const QgVarShader* v)
{
	GLFUNC(glUniformMatrix4fv)(handle, rdh->base.param.bones, false, (const GLfloat*)rdh->base.param.bonptr);
}

// 자동 변수
static struct Es2ShaderAutoUniforms
{
	bool				inited;
	struct AutoInfo
	{
		const char*			name;
		QgShdAuto			type;
		void(*func)(es2Rdh*, GLint, const QgVarShader*);
		size_t				hash;
	}					autos[QGSHA_MAX_VALUE];
} _es2shd_auto_uniforms =
{
	false,
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
	}
};

// 자동 초기화
void es2shd_init_auto_uniforms(void)
{
	qn_ret_if_ok(_es2shd_auto_uniforms.inited);
	_es2shd_auto_uniforms.inited = true;

	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
		_es2shd_auto_uniforms.autos[i].hash =qn_strihash(_es2shd_auto_uniforms.autos[i].name);
}

// 자동 함수
static void es2shd_match_auto_uniform(es2Uniform* u)
{
	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
	{
		if (u->hash == _es2shd_auto_uniforms.autos[i].hash && qn_stricmp(u->base.name, _es2shd_auto_uniforms.autos[i].name) == 0)
		{
			u->base.role = QGSHR_AUTO;
			u->auto_func = _es2shd_auto_uniforms.autos[i].func;
			return;
		}
	}

	u->base.role = QGSHR_MANUAL;
	u->auto_func = NULL;
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

static void _es2buf_dispose(QmGam* g);
static void* _es2buf_map(QgBuf* g);
static bool _es2buf_unmap(QgBuf* g);
static bool _es2buf_data(QgBuf* g, const void* data);

qvt_name(QgBuf) _vt_es2buf =
{
	.base.name = "ES2Buf",
	.base.dispose = _es2buf_dispose,
	.map = _es2buf_map,
	.unmap = _es2buf_unmap,
	.data = _es2buf_data,
};

es2Buf* es2buf_allocator(es2Rdh* rdh, QgBufType type, int count, int stride, const void* data)
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
	GLFUNC(glGenBuffers)(1, &gl_id);
	qn_retval_if_fail(gl_id != 0, NULL);
	es2_bind_buffer(rdh, gl_type, gl_id);

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

	GLsizeiptr size = count * stride;
	GLFUNC(glBufferData)(gl_type, size, ptr, gl_usage);

	// 진짜 만듦
	es2Buf* self = qn_alloc_zero_1(es2Buf);
	if (self==NULL)
	{
		GLFUNC(glDeleteBuffers)(1, &gl_id);
		return NULL;
	}

	qm_set_desc(self, gl_id);
	self->base.type = type;
	self->base.stride = (ushort)stride;
	self->base.size = (int)size;
	self->gl_type = gl_type;
	self->gl_usage = gl_usage;

	return qm_init(self, es2Buf, &_vt_es2buf);
}

static void _es2buf_dispose(QmGam* g)
{
	es2Buf* self = qm_cast(g, es2Buf);

	if (self->lockbuf)
		qn_free(self->lockbuf);

	GLuint gl_handle = qm_get_desc(self, GLuint);
	GLFUNC(glDeleteBuffers)(1, &gl_handle);

	qn_free(self);
}

static void* _es2buf_map(QgBuf* g)
{
	es2Buf* self = qm_cast(g, es2Buf);

	if (self->lockbuf != NULL)
		return NULL;
	if (self->gl_usage != GL_DYNAMIC_DRAW)
		return NULL;

	return self->lockbuf = qn_alloc(self->base.size, byte);
}

static bool _es2buf_unmap(QgBuf* g)
{
	es2Buf* self = qm_cast(g, es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);

	es2_bind_buffer(ES2RDH_INSTANCE, self->gl_type, qm_get_desc(self, GLuint));
	GLFUNC(glBufferData)(self->gl_type, self->base.size, self->lockbuf, self->gl_usage);

	qn_free_ptr(&self->lockbuf);

	return true;
}

static bool _es2buf_data(QgBuf* g, const void* data)
{
	qn_retval_if_fail(data, false);
	es2Buf* self = qm_cast(g, es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);
	qn_retval_if_fail(self->gl_usage == GL_DYNAMIC_DRAW, false);

	es2_bind_buffer(ES2RDH_INSTANCE, self->gl_type, qm_get_desc(self, GLuint));
	GLFUNC(glBufferData)(self->gl_type, self->base.size, data, self->gl_usage);

	return true;
}

