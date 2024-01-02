#pragma once

#include "qs_qn.h"
#include "qs_math.h"
#include "qs_qg.h"
#include "../stub/qg_stub.h"

//
typedef struct QglRdh		QglRdh;
typedef struct QglBuf		QglBuf;

#ifndef GL_INVALID_HANDLE
#define GL_INVALID_HANDLE	(GLuint)-1
#endif

#define QGL_RDH_INSTANCE	((QglRdh*)qg_rdh_instance)

// 참조 핸들
typedef struct QglRefHandle
{
	volatile nint		ref;
	GLuint				handle;
} QglRefHandle;

// GL 캐파
typedef struct QglCaps
{
	int					tmp;
} QglCaps;

// 세션 데이터
typedef struct QglSession
{
	QgDepth				depth;
	QgStencil			stencil;
} QglSession;

// 펜딩 데이터
typedef struct QglPending
{
	QglBuf*				ib;
	QglBuf*				vb[QGLOS_MAX_VALUE];
} QglPending;

// GL 렌더 디바이스
struct QglRdh
{
	QgRdh				base;

	QglSession			ss;
	QglPending			pd;

	nint				disposed;
};

qv_name(QglRdh)
{
	qv_name(QgRdh);

	void (*swap_interval)(void*, int);
	void (*swap_buffers)(void*, void*);
};

// 버퍼
struct QglBuffer
{
	QgBuffer			base;

	GLenum				gl_type;
	GLenum				gl_usage;

	void*				lock_buf;
};

// 문자열 버전에서 숫자만 mnn 방식으로
QN_INLINE int qgl_get_version(GLenum name, const char* name1, const char* name2)
{
	const char* s = (const char*)glGetString(name);
	qn_val_if_fail(s, 0);
	const float f =
		qn_strnicmp(s, name1, strlen(name1)) == 0 ? strtof(s + strlen(name1), NULL) :
		qn_strnicmp(s, name2, strlen(name2)) == 0 ? strtof(s + strlen(name2), NULL) :
		(float)strtof(s, NULL);
	return (int)(floorf(f) * 100.0f + (QM_FRACT(f) * 10.0));
}

// 문자열 얻기
QN_INLINE char* qgl_copy_string(char* buf, size_t size, GLenum name)
{
	const char* s = (const char*)glGetString(name);
	if (s == NULL)
		buf[0] = '\0';
	else
		qn_strncpy(buf, size, s, size - 1);
	return buf;
}

// 정수 얻기
QN_INLINE GLint qgl_get_integer_v(GLenum name)
{
	GLint n;
	glGetIntegerv(name, &n);
	return n;
}

// 프로그램 상태값 얻기
QN_INLINE GLint qgl_get_program_iv(GLuint program, GLenum name)
{
	GLint n;
	glGetProgramiv(program, name, &n);
	return n;
}

// 세이더 상태값 얻기
QN_INLINE GLint qgl_get_shader_iv(GLuint handle, GLenum name)
{
	GLint n;
	glGetShaderiv(handle, name, &n);
	return n;
}

// irrcht 엔젠에서 가져온 텍스쳐 행렬
QN_INLINE void qgl_mat4_irrcht_texture(QmMat4* m, float radius, float cx, float cy, float tx, float ty, float sx, float sy)
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

// 참조 핸들 만들기
QN_INLINE QglRefHandle* qgl_ref_handle_new(GLuint handle)
{
	QglRefHandle* ptr = qn_alloc_1(QglRefHandle);
	ptr->ref = 1;
	ptr->handle = handle;
	return ptr;
}

// 참조 핸들을 해제한다 (세이더용)
QN_INLINE void qgl_ref_handle_unload_shader(QglRefHandle* ptr, GLuint handle)
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
