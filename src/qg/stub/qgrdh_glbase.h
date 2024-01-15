#pragma once

#include "qs_qg.h"
#include "qg/qg_stub.h"

//
typedef struct QGLRDH		QglRdh;
typedef struct QGLBUFFER	QglBuffer;
typedef struct QGLSHADER	QglShader;
typedef struct QGLRENDER	QglRender;

#ifndef GL_INVALID_HANDLE
#define GL_INVALID_HANDLE	(GLuint)(-1)
#endif

#define QGL_RDH_INSTANCE	((QglRdh*)qg_instance_rdh)
#define QGL_PENDING()		(&QGL_RDH_INSTANCE->pd)
#define QGL_SESSION()		(&QGL_RDH_INSTANCE->ss)

// 참조 핸들
typedef struct QGLREFHANDLE
{
	volatile nint		ref;
	GLuint				handle;
} QglRefHandle;

// 레이아웃 요소
typedef struct QGLLAYOUTELEMENT
{
	QgLoStage			stage : 16;
	QgLoUsage			usage : 16;
	GLuint				attr;
	GLuint				size;
	GLenum				format;
	GLuint				offset;
	GLboolean			normalized;
	GLboolean			converted;
} QglLayoutElement;

// 레이아웃 프로퍼티
typedef struct QGLLAYOUTPROPERTY
{
	const void*			pointer;
	GLuint				buffer;
	GLsizei				stride;
	GLuint				soze;
	GLenum				format;
	GLboolean			normalized;
} QglLayoutProperty;
QN_DECL_CTNR(QglCtnLayoutProp, QglLayoutProperty);

// 세이더 자동 함수
typedef void (*QglShadeAutoFunc)(QglRdh*, GLint, const QgVarShader*);

// 세이더 유니폼
typedef struct QGLUNIFORM
{
	QgVarShader			base;
	size_t				hash;
	QglShadeAutoFunc	func;
} QglUniform;
QN_DECL_CTNR(QglCtnUniform, QglUniform);

// 세이더 어트리뷰트
typedef struct QGLATTRIB
{
	char				name[32];
	GLint				attrib : 8;
	GLint				size : 8;
	QgLoUsage			usage : 8;
	QgShdConstType		const_type : 8;
	size_t				hash;
} QglAttrib;
QN_DECL_CTNR(QglCtnAttrib, QglAttrib);

// GL 캐파
typedef struct QGLCAPS
{
	int					tmp;
} QglCaps;

// 세션 데이터
typedef struct QGLSESSION
{
	struct
	{
		GLuint				program;
		uint				lmask;
		QglCtnLayoutProp	lprops;
	}					shader;
	struct
	{
		GLuint				array;
		GLuint				element_array;
		GLuint				uniform;
	}					draw;

	QgDepth				depth;
	QgStencil			stencil;
} QglSession;

// 펜딩 데이터
typedef struct QGLPENDING
{
	struct
	{
		QglRender*			pipeline;
	}					render;
	struct
	{
		QglBuffer*			index_buffer;
		QglBuffer*			vertex_buffers[QGLOS_MAX_VALUE];
		uint				topology;
		uint				index_stride;
		uint				index_size;
		uint				vertex_count;
		uint				vertex_stride;
		uint				vertext_size;
		void*				index_data;
		void*				vertex_data;
	}					draw;
} QglPending;

// GL 렌더 디바이스
struct QGLRDH
{
	RdhBase				base;

	QglSession			ss;
	QglPending			pd;

	nint				disposed;
};

// 버퍼
struct QGLBUFFER
{
	QgBuffer			base;

	GLenum				gl_type;
	GLenum				gl_usage;

	void*				lock_pointer;
};

// 세이더
struct QGLSHADER
{
	QgShader			base;

	QglRefHandle*		vertex;
	QglRefHandle*		fragment;

	QglCtnUniform		uniforms;
	QglCtnAttrib		attrs;
	uint				attr_mask;

	bool				linked;
};

// 렌더 파이프라인
struct QGLRENDER
{
	QgRender			base;

	struct
	{
		QglRefHandle*		rhvertex;
		QglRefHandle*		rhfragment;
		QglCtnUniform		uniforms;
		QglCtnAttrib		attribs;
		uint				attr_mask;
		bool				lined;
	}					shader;
};

// 문자열 버전에서 숫자만 mnn 방식으로
INLINE int qgl_get_version(GLenum name, const char* name1, const char* name2)
{
	const char* s = (const char*)glGetString(name);
	qn_val_if_fail(s, 0);
	const float f =
		qn_strnicmp(s, name1, strlen(name1)) == 0 ? strtof(s + strlen(name1), NULL) :
		qn_strnicmp(s, name2, strlen(name2)) == 0 ? strtof(s + strlen(name2), NULL) :
		qn_strtof(s);
	return (int)(QM_FLOORF(f) * 100.0f + (QM_FRACT(f) * 10.0));
}

// 문자열 얻기
INLINE char* qgl_copy_string(char* buf, size_t size, GLenum name)
{
	const char* s = (const char*)glGetString(name);
	if (s == NULL)
		buf[0] = '\0';
	else
		qn_strncpy(buf, s, size - 1);
	return buf;
}

// 정수 얻기
INLINE GLint qgl_get_integer_v(GLenum name)
{
	GLint n;
	glGetIntegerv(name, &n);
	return n;
}

// 프로그램 상태값 얻기
INLINE GLint qgl_get_program_iv(GLuint program, GLenum name)
{
	GLint n;
	glGetProgramiv(program, name, &n);
	return n;
}

// 세이더 상태값 얻기
INLINE GLint qgl_get_shader_iv(GLuint handle, GLenum name)
{
	GLint n;
	glGetShaderiv(handle, name, &n);
	return n;
}

// irrcht 엔젠에서 가져온 텍스쳐 행렬
INLINE QmMat4 qgl_mat4_irrcht_texture(float radius, float cx, float cy, float tx, float ty, float sx, float sy)
{
	float c, s;
	qm_sincosf(radius, &s, &c);
	QmMat4 m =
	{
		._11 = c * sx,
		._12 = s * sy,
		._13 = 0.0f,
		._14 = 0.0f,
		._21 = -s * sx,
		._22 = c * sy,
		._23 = 0.0f,
		._24 = 0.0f,
		._31 = c * sx * cx + -s * cy + tx,
		._32 = s * sy * cx + c * cy + ty,
		._33 = 1.0f,
		._34 = 0.0f,
		._41 = 0.0f,
		._42 = 0.0f,
		._43 = 0.0f,
		._44 = 1.0f,
	};
	return m;
}

// 참조 핸들 만들기
INLINE QglRefHandle* qgl_ref_handle_new(GLuint handle)
{
	QglRefHandle* ptr = qn_alloc_1(QglRefHandle);
	ptr->ref = 1;
	ptr->handle = handle;
	return ptr;
}

// 참조 핸들을 해제한다 (세이더용)
INLINE void qgl_ref_handle_unload_shader(QglRefHandle* ptr, GLuint handle)
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

// 참조 핸들을 복제한다 (세이더용)
INLINE QglRefHandle* qgl_ref_handle_load_shader(QglRefHandle* ptr, GLuint handle)
{
	glAttachShader(handle, ptr->handle);
	++ptr->ref;
	return ptr;
}
