#pragma once

#include "qs_qn.h"
#include "qs_math.h"
#ifdef __EMSCRIPTEN__
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include "glad/glad.h"
#endif


//////////////////////////////////////////////////////////////////////////
// 함수

/**
 * @brief GL 버전 문자열에서 숫자만
 * @param name GL 항목 이름
 * @param name1 이름1
 * @param name2 이름2
 * @return 버전 숫자
*/
QN_INLINE int gl_get_version(GLenum name, const char* name1, const char* name2)
{
	const char* s = (const char*)glGetString(name);
	qn_val_if_fail(s, 0);
	const float f =
		qn_strnicmp(s, name1, strlen(name1)) == 0 ? strtof(s + strlen(name1), NULL) :
		qn_strnicmp(s, name2, strlen(name2)) == 0 ? strtof(s + strlen(name2), NULL) :
		(float)strtof(s, NULL);
	return (int)(floorf(f) * 100.0f + (QM_FRACT(f) * 10.0));
}

/**
 * @brief GL 문자열 얻기
 * @param buf 문자열을 넣을 버퍼
 * @param bufsize 버퍼의 크기
 * @param name GL 항목 이름
 * @return buf 그대로 반환
*/
QN_INLINE char* gl_copy_string(char* buf, size_t bufsize, GLenum name)
{
	const char* s = (const char*)glGetString(name);
	if (s == NULL)
		buf[0] = '\0';
	else
		qn_strncpy(buf, bufsize, s, bufsize - 1);
	return buf;
}

/**
 * @brief GL 정수값 얻기
 * @param name GL 항목 이름
 * @return 얻은 정수값
*/
QN_INLINE GLint gl_get_integer_v(GLenum name)
{
	GLint n;
	glGetIntegerv(name, &n);
	return n;
}

/**
 * @brief GL 프로그램 상태값 얻기
 * @param program 프로그램 핸들
 * @param name GL 항목 이름
 * @return 얻은 상태값
*/
QN_INLINE GLint gl_get_program_iv(GLuint program, GLenum name)
{
	GLint n;
	glGetProgramiv(program, name, &n);
	return n;
}

/**
 * @brief GL 세이더 상태값 얻기
 * @param handle 세이더 핸들
 * @param name GL 항목 이름
 * @return 얻은 상태값
*/
QN_INLINE GLint gl_get_shader_iv(GLuint handle, GLenum name)
{
	GLint n;
	glGetShaderiv(handle, name, &n);
	return n;
}

/**
 * @brief irrcht 엔진에서 가져온 텍스쳐 행렬 생성
 * @param m 대상 행렬
 * @param radius 반지름
 * @param cx,cy 가운데
 * @param tx,ty 트랜스폼
 * @param sx,sy 스케일
*/
QN_INLINE void gl_mat4_irrcht_texture(QmMat4* m, float radius, float cx, float cy, float tx, float ty, float sx, float sy)
{
	float c, s;
	qm_sincosf(radius, &s, &c);

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

//////////////////////////////////////////////////////////////////////////
// GL 참조 핸들

// 참조 핸들
typedef struct GlRefHandle	GlRefHandle;
struct GlRefHandle
{
	volatile nint		ref;
	GLuint				handle;
};

/**
 * @brief GL 참조 핸들을 만든다
 * @param handle 핸들
 * @return 만들어진 GL 참조 핸들
*/
QN_INLINE GlRefHandle* gl_ref_handle_new(GLuint handle)
{
	GlRefHandle* ptr = qn_alloc_1(GlRefHandle);
	ptr->ref = 1;
	ptr->handle = handle;
	return ptr;
}

/**
 * @brief GL 참조 핸들의 참조를 해제한다 (세이더용)
 * @param ptr GL 참조 핸들
 * @param handle Detach 할 프로그램 핸들
*/
QN_INLINE void gl_ref_handle_unload_shader(GlRefHandle* ptr, GLuint handle)
{
	qn_ret_if_fail(ptr);
	if (handle > 0)
		glDetachShader(handle, ptr->handle);
	const nint ref = --ptr->ref;
	if (ref == 0)
	{
		glDeleteShader(ptr->handle);
		qn_free(ptr);
	}
}
