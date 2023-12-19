#pragma once

// 버전 문자열에서 숫자만
QN_INLINE int gl_get_version(GLenum name, const char* name1, const char* name2)
{
	const char* s = (const char*)GL_FUNC(glGetString)(name);
	qn_val_if_fail(s, 0);
	const float f =
		qn_strnicmp(s, name1, strlen(name1)) == 0 ? strtof(s + strlen(name1), NULL) :
		qn_strnicmp(s, name2, strlen(name2)) == 0 ? strtof(s + strlen(name2), NULL) :
		(float)strtof(s, NULL);
	return (int)(floorf(f) * 100.0f + (QN_FRACT(f) * 10.0));
}

// GetString
QN_INLINE const char* gl_copy_string(char* buf, size_t bufsize, GLenum name)
{
	const char* s = (const char*)GL_FUNC(glGetString)(name);
	if (s == NULL)
		buf[0] = '\0';
	else
		qn_strncpy(buf, bufsize, s, bufsize - 1);
	return buf;
}

// GetIntegerv
QN_INLINE GLint gl_get_integer_v(GLenum name)
{
	GLint n;
	GL_FUNC(glGetIntegerv)(name, &n);
	return n;
}

// GetShaderiv
QN_INLINE GLint gl_get_shader_iv(GLuint handle, GLenum name)
{
	GLint n;
	GL_FUNC(glGetShaderiv)(handle, name, &n);
	return n;
}

// GetProgramiv
QN_INLINE GLint gl_get_program_iv(GLuint program, GLenum name)
{
	GLint n;
	GL_FUNC(glGetProgramiv)(program, name, &n);
	return n;
}

// irrcht 텍스쳐 매트릭스
QN_INLINE void gl_mat4_irrcht_texture(QnMat4* m, float radius, float cx, float cy, float tx, float ty, float sx, float sy)
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

//
QN_INLINE GlRefHandle* gl_shd_handle_new(GLuint gl_program)
{
	GlRefHandle* ptr = qn_alloc_1(GlRefHandle);
	ptr->ref = 1;
	ptr->handle = gl_program;
	return ptr;
}

//
QN_INLINE void gl_shd_handle_unload(GlRefHandle* ptr, GLuint gl_program)
{
	qn_ret_if_fail(ptr);
	if (gl_program > 0)
		GL_FUNC(glDetachShader)(gl_program, ptr->handle);
	const nint ref = --ptr->ref;
	if (ref == 0)
	{
		GL_FUNC(glDeleteShader)(ptr->handle);
		qn_free(ptr);
	}
}
