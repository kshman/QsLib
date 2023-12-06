#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qg_es2.h"
#include "qg_glsupp.h"

static bool es2_shd_link(QgShd* g);
static void es2_shd_process_uniforms(Es2Shd* self);

//
void es2_bind_buffer(Es2Rdh* self, GLenum gl_type, GLuint gl_id)
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
		default:
			break;
	}
}

//
void es2_commit_layout(Es2Rdh* self)
{
	const Es2Shd* shd = self->pd.shd;
	const Es2Vlo* vlo = self->pd.vlo;

	const int max_attrs = rdh_caps(self).max_vertex_attrs;
	const Es2CtnAttrib* shd_attrs = &shd->attrs;

	uint ok = 0;
	QN_STATIC_ASSERT(QGMAX_ES2_VERTEX_ATTRIBUTE <= sizeof(ok) * 8, "Not enough for access QGMAX_ES2_VERTEX_ATTRIBUTE");

	for (int s = 0, index = 0; s < QGLOS_MAX_VALUE; s++)
	{
		Es2Buf* buf = self->pd.vb[s];
		const int stcnt = vlo->es_cnt[s];

		if (buf != NULL)
		{
			// 정점 버퍼가 있다
			const Es2LayoutElement* stelm = vlo->es_elm[s];

			const GLsizei gl_stride = (GLsizei)vlo->base.stride[s];
			const GLuint gl_buf = qm_get_desc(buf, GLuint);
			es2_bind_buffer(self, GL_ARRAY_BUFFER, gl_buf);

			// 해당 스테이지의 레이아웃
			for (int i = 0; i < stcnt; i++)
			{
				if (index >= max_attrs)
					break;

				const Es2LayoutElement* le = &stelm[i];
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
				Es2LayoutProperty* lp = &self->ss.layouts[gl_attr];
				const nuint pointer = (nuint)le->offset;
				if (QN_TEST_BIT(self->ss.layout_mask, gl_attr) == false)
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
#ifdef _QN_WINDOWS_
					GLFUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, pointer);
#else
					GLFUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, (const void*)pointer);
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
				if (QN_TEST_BIT(self->ss.layout_mask, gl_attr))
				{
					QN_SET_BIT(&self->ss.layout_mask, gl_attr, false);
					GLFUNC(glDisableVertexAttribArray)(gl_attr);
				}
				const GLfloat tmp[4] = { 0.0f, };
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
void es2_commit_layout_ptr(Es2Rdh* self, const void* buffer, GLsizei gl_stride)
{
	const Es2Shd* shd = self->pd.shd;
	const Es2Vlo* vlo = self->pd.vlo;

	if (gl_stride == 0)
		gl_stride = (GLsizei)vlo->base.stride[0];
	else if (gl_stride != (GLsizei)vlo->base.stride[0])
	{
		qn_debug_outputf(true, "ES2Rdh", "stride is not same %d != %d", gl_stride, vlo->base.stride[0]);
		return;
	}

	const int max_attrs = rdh_caps(self).max_vertex_attrs;
	const Es2CtnAttrib* shd_attrs = &shd->attrs;

	const Es2LayoutElement* stelm = vlo->es_elm[0];	// 스테이지 0밖에 안씀. 버퍼가 하나니깐
	const int stcnt = vlo->es_cnt[0];

	es2_bind_buffer(self, GL_ARRAY_BUFFER, 0);	// 버퍼 초기화 안하면 안된다.

	uint ok = 0;
	QN_STATIC_ASSERT(QGMAX_ES2_VERTEX_ATTRIBUTE <= sizeof(ok) * 8, "Not enough for access QGMAX_ES2_VERTEX_ATTRIBUTE");

	for (int s = 0, index = 0; s < stcnt; s++)
	{
		const Es2LayoutElement* le = &stelm[s];
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
		Es2LayoutProperty* lp = &self->ss.layouts[gl_attr];
		const nuint pointer = (nuint)((const byte*)buffer + le->offset);
		if (QN_TEST_BIT(self->ss.layout_mask, gl_attr) == false)
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
#ifdef _QN_WINDOWS_
			GLFUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, pointer);
#else
			GLFUNC(glVertexAttribPointer)(gl_attr, le->size, le->format, le->normalized, gl_stride, (const void*)pointer);
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
void es2_commit_shader(Es2Rdh* self)
{
	Es2Shd* shd = self->pd.shd;
	if (shd == NULL)
	{
		if (self->ss.program != 0)
		{
			self->ss.program = 0;
			GLFUNC(glUseProgram)(0);
		}
		return;
	}

	es2_shd_link((QgShd*)shd);
	const GLuint gl_program = qm_get_desc(shd, GLuint);
	if (self->ss.program != gl_program)
	{
		self->ss.program = gl_program;
		GLFUNC(glUseProgram)(gl_program);
	}

	// 세이더 값 여기서 넣는다!!!
	es2_shd_process_uniforms(shd);
}


//////////////////////////////////////////////////////////////////////////
// 레이아웃

static void es2_vlo_dispose(QmGam* g);

qvt_name(QmGam) vt_es2_vlo =
{
	.name = "ES2Vlo",
	.dispose = es2_vlo_dispose,
};

//
Es2Vlo* es2_vlo_allocator(Es2Rdh* rdh, int count, const QgPropLayout* layouts)
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

	Es2Vlo* self = qn_alloc_zero_1(Es2Vlo);
	qn_retval_if_fail(self, NULL);

	// 스테이지 할당
	Es2LayoutElement* elms[QGLOS_MAX_VALUE] = { NULL, };
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (accum[i] == 0)
			continue;
		self->es_cnt[i] = (byte)accum[i];
		self->es_elm[i] = elms[i] = qn_alloc(accum[i], Es2LayoutElement);
	}

	// 데이터 작성
	ushort offset[QGLOS_MAX_VALUE] = { 0, };
	for (int i = 0; i < count; i++)
	{
		const QgPropLayout* l = &layouts[i];
		Es2LayoutElement* e = elms[l->stage]++;
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

	return qm_init(self, Es2Vlo, &vt_es2_vlo);
}

//
static void es2_vlo_dispose(QmGam* g)
{
	Es2Vlo* self = qm_cast(g, Es2Vlo);
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
		if (self->es_elm[i])
			qn_free(self->es_elm[i]);
	qn_free(self);
}


//////////////////////////////////////////////////////////////////////////
// 세이더

static void es2_shd_match_auto_uniform(Es2Uniform* u);

static void es2_shd_dispose(QmGam* g);
static bool es2_shd_bind(QgShd* g, QgShdType type, const void* data, int size, int flags);
static bool es2_shd_bind_shd(QgShd* g, QgShdType type, QgShd* shaderptr);

qvt_name(QgShd) vt_es2_shd =
{
	.base.name = "ES2Shd",
	.base.dispose = es2_shd_dispose,
	.bind = es2_shd_bind,
	.bind_shd = es2_shd_bind_shd,
	//.bind_name = NULL,
	//.add_condition = NULL,
	//.clear_condition = NULL,
	.link = es2_shd_link,
};

//
static Es2RefHandle* es2_shd_handle_new(GLuint gl_program)
{
	Es2RefHandle* ptr = qn_alloc_1(Es2RefHandle);
	ptr->ref = 1;
	ptr->handle = gl_program;
	return ptr;
}

//
static void es2_shd_handle_unload(Es2RefHandle* ptr, GLuint gl_program)
{
	if (ptr)
	{
		if (gl_program > 0)
			GLFUNC(glDetachShader)(gl_program, ptr->handle);

		const nint ref = --ptr->ref;
		if (ref == 0)
		{
			GLFUNC(glDeleteShader)(ptr->handle);
			qn_free(ptr);
		}
	}
}

//
Es2Shd* es2_shd_allocator(Es2Rdh* rdh, const char* name)
{
	Es2Shd* self = qn_alloc_zero_1(Es2Shd);
	qn_retval_if_fail(self, NULL);
#ifdef _QN_WINDOWS_
	qm_set_desc(self, es2_func.glCreateProgram());
#else
	qm_set_desc(self, GLFUNC(glCreateProgram)());	// 이렇게 하면 LINT에 걸려버렷
#endif
	return qm_init(self, Es2Shd, &vt_es2_shd);
}

//
static void es2_shd_dispose(QmGam* g)
{
	Es2Shd* self = qm_cast(g, Es2Shd);
	const GLuint handle = qm_get_desc(self, GLuint);

	es2_shd_handle_unload(self->rvertex, handle);
	es2_shd_handle_unload(self->rfragment, handle);

	GLFUNC(glDeleteProgram)(handle);

	qn_ctnr_disp(es2CtnUniform, &self->uniforms);
	qn_ctnr_disp(es2CtnAttrib, &self->attrs);

	qn_free(self);
}

//
static Es2RefHandle* es2_shd_compile(Es2Shd* self, GLenum gl_type, const char* src, GLint gl_len)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	const GLuint gl_shader = GLFUNC(glCreateShader)(gl_type);
	qn_retval_if_fail(gl_shader != 0, NULL);

	GLFUNC(glShaderSource)(gl_shader, 1, &src, gl_len == 0 ? NULL : &gl_len);
	GLFUNC(glCompileShader)(gl_shader);

	if (gl_get_shader_iv(gl_shader, GL_COMPILE_STATUS) == GL_FALSE)
	{
		const char* progress = gl_type == GL_VERTEX_SHADER ? "vertex" : gl_type == GL_FRAGMENT_SHADER ? "fragment" : "(unknown)";
		GLsizei gl_log_len = gl_get_shader_iv(gl_shader, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
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

	return es2_shd_handle_new(gl_shader);
}

//
static bool es2_shd_bind(QgShd* g, QgShdType type, const void* data, int size, int flags)
{
	// 해야함: 매크로
	// 해야함: 인클루트
	// 해야함: 플래그
	// 해야함: 보관
	qn_retval_if_fail(data != NULL && size >= 0, false);
	Es2Shd* self = qm_cast(g, Es2Shd);
	const GLuint handle = qm_get_desc(self, GLuint);

	if (type == QGSHT_VS)
	{
		es2_shd_handle_unload(self->rvertex, handle);
		if ((self->rvertex = es2_shd_compile(self, GL_VERTEX_SHADER, (const char*)data, (GLint)size)) == NULL)
			return false;
		GLFUNC(glAttachShader)(handle, self->rvertex->handle);
	}
	else if (type == QGSHT_PS)
	{
		es2_shd_handle_unload(self->rfragment, handle);
		if ((self->rfragment = es2_shd_compile(self, GL_FRAGMENT_SHADER, (const char*)data, (GLint)size)) == NULL)
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
static bool es2_shd_bind_shd(QgShd* g, QgShdType type, QgShd* shaderptr)
{
	qn_retval_if_fail(shaderptr, false);
	Es2Shd* self = qm_cast(g, Es2Shd);
	const Es2Shd* shader = qm_cast(shaderptr, Es2Shd);
	const GLuint handle = qm_get_desc(self, GLuint);
	bool ok = false;

	if (QN_TEST_MASK(type, QGSHT_VS))
	{
		qn_retval_if_fail(shader->rvertex, false);
		es2_shd_handle_unload(self->rvertex, handle);

		self->rvertex->ref++;
		GLFUNC(glAttachShader)(handle, self->rvertex->handle);
		ok = true;
	}

	if (QN_TEST_MASK(type, QGSHT_PS))
	{
		qn_retval_if_fail(shader->rfragment, false);
		es2_shd_handle_unload(self->rfragment, handle);

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
static QgShdConst es2_shd_enum_to_const(GLenum gl_type)
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
QgLoUsage es2_shd_attrib_to_usage(const char* name)
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
static bool es2_shd_link(QgShd* g)
{
	Es2Shd* self = qm_cast(g, Es2Shd);
	qn_retval_if_ok(self->linked, true);
	qn_retval_if_fail(self->rvertex && self->rfragment, false);

	const GLuint gl_program = qm_get_desc(self, GLuint);
	GLFUNC(glLinkProgram)(gl_program);

	if (gl_get_program_iv(gl_program, GL_LINK_STATUS) == GL_FALSE)
	{
		GLsizei gl_log_len = gl_get_program_iv(gl_program, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
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
	GLchar sz[128];

	// 전역 변수 (유니폼)
	qn_ctnr_disp(es2CtnUniform, &self->uniforms);
	const GLint gl_uniforms = gl_get_program_iv(gl_program, GL_ACTIVE_UNIFORMS);
	if (gl_uniforms > 0)
	{
		qn_ctnr_init(Es2CtnUniform, &self->uniforms, gl_uniforms);

		for (GLint i = 0, index = 0; i < gl_uniforms; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			GLFUNC(glGetActiveUniform)(gl_program, i, QN_COUNTOF(sz) - 1, NULL, &gl_size, &gl_type, sz);

			const QgShdConst cnst = es2_shd_enum_to_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)
			{
				// 사용할 수 없는 유니폼
				qn_debug_outputf(true, "ES2Shd", "not supported uniform type '%X' in '%s'", gl_type, sz);
				continue;
			}

			Es2Uniform* u = &qn_ctnr_nth(&self->uniforms, index);
			qn_strncpy(u->base.name, QN_COUNTOF(u->base.name), sz, QN_COUNTOF(u->base.name) - 1);
			u->base.cnst = cnst;
			u->base.size = (ushort)gl_size;
			u->base.offset = (uint)GLFUNC(glGetUniformLocation)(gl_program, sz);
			u->hash = qn_strihash(u->base.name);
			es2_shd_match_auto_uniform(u);

			index++;
		}
	}

	// 인수 (어트리뷰트)
	self->attr_mask = 0;
	qn_ctnr_disp(es2CtnAttrib, &self->attrs);
	const GLint gl_attrs = gl_get_program_iv(gl_program, GL_ACTIVE_ATTRIBUTES);
	if (gl_attrs > 0)
	{
		qn_ctnr_init(Es2CtnAttrib, &self->attrs, gl_attrs);

		for (GLint i = 0, index = 0; i < gl_attrs; i++)
		{
			GLint gl_size;
			GLenum gl_type;
			GLsizei gl_len;
			GLFUNC(glGetActiveAttrib)(gl_program, i, QN_COUNTOF(sz) - 1, &gl_len, &gl_size, &gl_type, sz);

			const QgShdConst cnst = es2_shd_enum_to_const(gl_type);
			if (cnst == QGSHC_UNKNOWN)
			{
				// 사용할 수 없는 어트리뷰트
				qn_debug_outputf(true, "ES2Shd", "not supported attribute type '%X' in '%s'", gl_type, sz);
				continue;
			}

			const GLint gl_attrib = GLFUNC(glGetAttribLocation)(gl_program, sz);
			if ((size_t)gl_attrib >= QGMAX_ES2_VERTEX_ATTRIBUTE)
			{
				// 이 어트리뷰트는 못쓴다
				qn_debug_outputf(true, "ES2Shd", "invalid attribute id '%d' in '%s'", gl_attrib, sz);
				continue;
			}

			Es2Attrib* a = &qn_ctnr_nth(&self->attrs, index);
			qn_strncpy(a->name, QN_COUNTOF(a->name), sz, QN_COUNTOF(a->name) - 1);
			a->attrib = gl_attrib;
			a->size = gl_size;
			a->usage = es2_shd_attrib_to_usage(sz);
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
static void es2_shd_process_uniforms(Es2Shd* self)
{
	for (size_t i = 0; i < qn_ctnr_count(&self->uniforms); i++)
	{
		Es2Uniform* u = &qn_ctnr_nth(&self->uniforms, i);
		if (u->base.role == QGSHR_AUTO)
			u->auto_func((Es2Rdh*)qg_rdh_instance, (GLint)u->base.offset, &u->base);
		else if (u->base.role == QGSHR_MANUAL && self->base.intr_func)
			(self->base.intr_func)(self->base.intr_data, &u->base, qm_cast(self, QgShd));
	}
}

// 자동 변수들

// QGSHA_ORTHO_PROJ
static void es2_shd_auto_otho_proj(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.ortho);
}

// QGSHA_WORLD
static void es2_shd_auto_world(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.world);
}

// QGSHA_VIEW
static void es2_shd_auto_view(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.view);
}

// QGSHA_PROJ
static void es2_shd_auto_proj(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.proj);
}

// QGSHA_VIEW_PROJ
static void es2_shd_auto_view_proj(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.vipr);
}

// QGSHA_INV_VIEW
static void es2_shd_auto_inv_view(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.tm.inv);
}

// QGSHA_WORLD_VIEW_PROJ
static void es2_shd_auto_world_view_proj(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16); qn_verify(v->size == 1);
	QnMat4 m;
	qn_mat4_mul(&m, &rdh->base.tm.world, &rdh->base.tm.vipr);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&m);
}

// QGSHA_TEX0
static void es2_shd_auto_tex_0(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 0);
}

// QGSHA_TEX1
static void es2_shd_auto_tex_1(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 1);
}

// QGSHA_TEX2
static void es2_shd_auto_tex_2(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 2);
}

// QGSHA_TEX3
static void es2_shd_auto_tex_3(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 3);
}

// QGSHA_TEX4
static void es2_shd_auto_tex_4(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 4);
}

// QGSHA_TEX5
static void es2_shd_auto_tex_5(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 5);
}

// QGSHA_TEX6
static void es2_shd_auto_tex_6(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 6);
}

// QGSHA_TEX7
static void es2_shd_auto_tex_7(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_INT1);
	GLFUNC(glUniform1i)(handle, 7);
}

// QGSHA_PROP_VEC0
static void es2_shd_auto_prop_vec_0(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GLFUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[0]);
}

// QGSHA_PROP_VEC1
static void es2_shd_auto_prop_vec_1(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GLFUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[1]);
}

// QGSHA_PROP_VEC2
static void es2_shd_auto_prop_vec_2(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GLFUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[2]);
}

// QGSHA_PROP_VEC3
static void es2_shd_auto_prop_vec_3(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT4);
	GLFUNC(glUniform4fv)(handle, 1, (const GLfloat*)&rdh->base.param.v[3]);
}

// QGSHA_PROP_MAT0
static void es2_shd_auto_prop_mat_0(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[0]);
}

// QGSHA_PROP_MAT1
static void es2_shd_auto_prop_mat_1(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[1]);
}

// QGSHA_PROP_MAT2
static void es2_shd_auto_prop_mat_2(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[2]);
}

// QGSHA_PROP_MAT3
static void es2_shd_auto_prop_mat_3(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
{
	qn_verify(v->cnst == QGSHC_FLOAT16);
	GLFUNC(glUniformMatrix4fv)(handle, 1, false, (const GLfloat*)&rdh->base.param.m[3]);
}

// QGSHA_MAT_PALETTE
static void es2_shd_auto_mat_palette(const Es2Rdh* rdh, const GLint handle, const QgVarShader* v)
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
		void(*func)(const Es2Rdh*, GLint, const QgVarShader*);
		size_t				hash;
	}					autos[QGSHA_MAX_VALUE];
} _es2_shd_auto_uniforms =
{
	false,
	{
#define ES2SAI(name, type, func) { "s" QN_STRING(name), type, func, 0 }
		ES2SAI(OrthoProj, QGSHA_ORTHO_PROJ, es2_shd_auto_otho_proj),
		ES2SAI(World, QGSHA_WORLD, es2_shd_auto_world),
		ES2SAI(View, QGSHA_VIEW, es2_shd_auto_view),
		ES2SAI(Proj, QGSHA_PROJ, es2_shd_auto_proj),
		ES2SAI(ViewProj, QGSHA_VIEW_PROJ, es2_shd_auto_view_proj),
		ES2SAI(InvView, QGSHA_INV_VIEW, es2_shd_auto_inv_view),
		ES2SAI(WorldViewProj, QGSHA_WORLD_VIEW_PROJ, es2_shd_auto_world_view_proj),
		ES2SAI(Tex0, QGSHA_TEX0, es2_shd_auto_tex_0),
		ES2SAI(Tex1, QGSHA_TEX1, es2_shd_auto_tex_1),
		ES2SAI(Tex2, QGSHA_TEX2, es2_shd_auto_tex_2),
		ES2SAI(Tex3, QGSHA_TEX3, es2_shd_auto_tex_3),
		ES2SAI(Tex4, QGSHA_TEX4, es2_shd_auto_tex_4),
		ES2SAI(Tex5, QGSHA_TEX5, es2_shd_auto_tex_5),
		ES2SAI(Tex6, QGSHA_TEX6, es2_shd_auto_tex_6),
		ES2SAI(Tex7, QGSHA_TEX7, es2_shd_auto_tex_7),
		ES2SAI(PropVec0, QGSHA_PROP_VEC0, es2_shd_auto_prop_vec_0),
		ES2SAI(PropVec1, QGSHA_PROP_VEC1, es2_shd_auto_prop_vec_1),
		ES2SAI(PropVec2, QGSHA_PROP_VEC2, es2_shd_auto_prop_vec_2),
		ES2SAI(PropVec3, QGSHA_PROP_VEC3, es2_shd_auto_prop_vec_3),
		ES2SAI(PropMat0, QGSHA_PROP_MAT0, es2_shd_auto_prop_mat_0),
		ES2SAI(PropMat1, QGSHA_PROP_MAT1, es2_shd_auto_prop_mat_1),
		ES2SAI(PropMat2, QGSHA_PROP_MAT2, es2_shd_auto_prop_mat_2),
		ES2SAI(PropMat3, QGSHA_PROP_MAT3, es2_shd_auto_prop_mat_3),
		ES2SAI(MatPalette, QGSHA_MAT_PALETTE, es2_shd_auto_mat_palette),
#undef ES2SAI
	}
};

// 자동 초기화
void es2_shd_init_auto_uniforms(void)
{
	qn_ret_if_ok(_es2_shd_auto_uniforms.inited);
	_es2_shd_auto_uniforms.inited = true;

	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
		_es2_shd_auto_uniforms.autos[i].hash = qn_strihash(_es2_shd_auto_uniforms.autos[i].name);
}

// 자동 함수
static void es2_shd_match_auto_uniform(Es2Uniform* u)
{
	for (size_t i = 0; i < QGSHA_MAX_VALUE; i++)
	{
		if (u->hash == _es2_shd_auto_uniforms.autos[i].hash && qn_stricmp(u->base.name, _es2_shd_auto_uniforms.autos[i].name) == 0)
		{
			u->base.role = QGSHR_AUTO;
			u->auto_func = _es2_shd_auto_uniforms.autos[i].func;
			return;
		}
	}

	u->base.role = QGSHR_MANUAL;
	u->auto_func = NULL;
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

static void es2_buf_dispose(QmGam* g);
static void* es2_buf_map(QgBuf* g);
static bool es2_buf_unmap(QgBuf* g);
static bool es2_buf_data(QgBuf* g, const void* data);

qvt_name(QgBuf) vt_es2_buf =
{
	.base.name = "ES2Buf",
	.base.dispose = es2_buf_dispose,
	.map = es2_buf_map,
	.unmap = es2_buf_unmap,
	.data = es2_buf_data,
};

Es2Buf* es2_buf_allocator(Es2Rdh* rdh, QgBufType type, int count, int stride, const void* data)
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

	const GLsizeiptr size = (GLsizeiptr)count * stride;
	GLFUNC(glBufferData)(gl_type, size, ptr, gl_usage);

	// 진짜 만듦
	Es2Buf* self = qn_alloc_zero_1(Es2Buf);
	if (self == NULL)
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

	return qm_init(self, Es2Buf, &vt_es2_buf);
}

static void es2_buf_dispose(QmGam* g)
{
	Es2Buf* self = qm_cast(g, Es2Buf);

	if (self->lockbuf)
		qn_free(self->lockbuf);

	const GLuint gl_handle = qm_get_desc(self, GLuint);
	GLFUNC(glDeleteBuffers)(1, &gl_handle);

	qn_free(self);
}

static void* es2_buf_map(QgBuf* g)
{
	Es2Buf* self = qm_cast(g, Es2Buf);

	if (self->lockbuf != NULL)
		return NULL;
	if (self->gl_usage != GL_DYNAMIC_DRAW)
		return NULL;

	return self->lockbuf = qn_alloc(self->base.size, byte);
}

static bool es2_buf_unmap(QgBuf* g)
{
	Es2Buf* self = qm_cast(g, Es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);

	es2_bind_buffer(ES2_RDH_INSTANCE, self->gl_type, qm_get_desc(self, GLuint));
	GLFUNC(glBufferData)(self->gl_type, self->base.size, self->lockbuf, self->gl_usage);

	qn_free_ptr(&self->lockbuf);

	return true;
}

static bool es2_buf_data(QgBuf* g, const void* data)
{
	qn_retval_if_fail(data, false);
	Es2Buf* self = qm_cast(g, Es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);
	qn_retval_if_fail(self->gl_usage == GL_DYNAMIC_DRAW, false);

	es2_bind_buffer(ES2_RDH_INSTANCE, self->gl_type, qm_get_desc(self, GLuint));
	GLFUNC(glBufferData)(self->gl_type, self->base.size, data, self->gl_usage);

	return true;
}

