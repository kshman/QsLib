#pragma once

#include "qg/qg_stub.h"

//
typedef struct QGLRDH		QglRdh;
typedef struct QGLBUFFER	QglBuffer;
typedef struct QGLRENDER	QglRender;

#ifndef GL_INVALID_HANDLE
#define GL_INVALID_HANDLE	(GLuint)(-1)
#endif

#define QGL_RDH				((QglRdh*)qg_instance_rdh)
#define QGL_PENDING			(&QGL_RDH->pd)
#define QGL_SESSION			(&QGL_RDH->ss)
#define QGL_RESOURCE		(&QGL_RDH->res)

// 레이아웃 요소
typedef struct QGLLAYOUTINPUT
{
	QgLayoutStage		stage : 16;
	QgLayoutUsage		usage : 16;
	GLuint				offset;
	GLenum				format;
	GLuint				count : 16;
	GLboolean			normalized;
} QglLayoutInput;
QN_DECL_CTNR(QglCtnLayoutInput, QglLayoutInput);

// 레이아웃 프로퍼티
typedef struct QGLLAYOUTPROPERTY
{
	GLuint				offset;
	GLenum				format;
	GLsizei				stride;
	GLuint				count : 16;
	GLboolean			normalized;
} QglLayoutProperty;

// 세이더 유니폼
QN_DECL_CTNR(QglCtnUniform, QgVarShader);

// 세이더 어트리뷰트
typedef struct QGLVARATTR
{
#ifdef _QN_64_
	char				name[32 + 8];
#else
	char				name[32];
#endif
	size_t				hash;

	GLint				attrib;
	GLint				size;

	QgLayoutUsage		usage : 8;			// 최대 24(2024-01-20 시점)으로 괜춘
	QgScType			sctype : 8;			// 최대 22(2032-01-20 시점)으로 괜춘		
} QglVarAttr;
QN_DECL_CTNR(QglCtnAttr, QglVarAttr);

// 세션 데이터
typedef struct QGLSESSION
{
	struct QGLSESSION_SHADER
	{
		GLuint				program;
		uint				amask;						// 어트리뷰트 마스크
		QglLayoutProperty	lprops[QGLOU_MAX_SIZE];		// 레이아웃 상태 저장
	}					shader;
	struct QGLSESSION_BUFFER
	{
		GLuint				vertex;		// array
		GLuint				index;		// element array
		GLuint				uniform;
	}					buffer;

	QgDepth				depth;
	QgStencil			stencil;
} QglSession;

// 펜딩 데이터
typedef struct QGLPENDING
{
	struct QGLPENDING_RENDER
	{
		QglBuffer*			index_buffer;
		QglBuffer*			vertex_buffers[QGLOS_MAX_VALUE];
		QglRender*			render;
	}					render;
	struct QGLPENDING_DRAW
	{
		uint				topology;
	}					draw;
} QglPending;

// 리소스
typedef struct QGLRESOURCE
{
	QglRender*			ortho_render;
	QglRender*			glyph_render;
} QglResource;

// GL 렌더 디바이스
struct QGLRDH
{
	RdhBase				base;

	QglSession			ss;
	QglPending			pd;
	QglResource			res;

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

// 렌더 파이프라인
struct QGLRENDER
{
	QgRender			base;

	struct QGLRENDER_SHADER
	{
		GLuint				program;
		GLuint				vertex;
		GLuint				fragment;

		QglCtnUniform		uniforms;
		QglCtnAttr			attrs;
		byte				usages[QGLOU_MAX_SIZE];
	}					shader;

	struct QGLRENDER_LAYOUT
	{
		QglCtnLayoutInput	inputs;
		QglLayoutInput*		stages[QGLOS_MAX_VALUE];
		ushort				counts[QGLOS_MAX_VALUE];
		ushort				strides[QGLOS_MAX_VALUE];
	}					layout;
};

// 문자열 버전에서 숫자만 mnn 방식으로
INLINE int qgl_get_version(const char* s, const char* name1, const char* name2)
{
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

// 토폴로지 변환
INLINE GLenum qgl_topology_to_enum(QgTopology tpg)
{
	switch (tpg)
	{
		case QGTPG_POINT:		return GL_POINTS;
		case QGTPG_LINE:		return GL_LINES;
		case QGTPG_LINE_STRIP:	return GL_LINE_STRIP;
		case QGTPG_TRI:			return GL_TRIANGLES;
		case QGTPG_TRI_STRIP:	return GL_TRIANGLE_STRIP;
		case QGTPGEX_TRI_FAN:	return GL_TRIANGLE_FAN;
		default:				return GL_NONE;
	}
}

// GLenum을 상수로
INLINE QgScType qgl_enum_to_shader_const(GLenum gl_type)
{
	switch (gl_type)
	{
		case GL_FLOAT:				return QGSCT_FLOAT1;
		case GL_FLOAT_VEC2:			return QGSCT_FLOAT2;
		case GL_FLOAT_VEC3:			return QGSCT_FLOAT3;
		case GL_FLOAT_VEC4:			return QGSCT_FLOAT4;
		case GL_INT:				return QGSCT_INT1;
		case GL_INT_VEC2:			return QGSCT_INT2;
		case GL_INT_VEC3:			return QGSCT_INT3;
		case GL_INT_VEC4:			return QGSCT_INT4;
		case GL_UNSIGNED_INT:		return QGSCT_UINT1;
		case GL_UNSIGNED_INT_VEC2:	return QGSCT_UINT2;
		case GL_UNSIGNED_INT_VEC3:	return QGSCT_UINT3;
		case GL_UNSIGNED_INT_VEC4:	return QGSCT_UINT4;
		case GL_BOOL:				return QGSCT_BYTE1;
		case GL_BOOL_VEC2:			return QGSCT_BYTE2;
		case GL_BOOL_VEC3:			return QGSCT_BYTE3;
		case GL_BOOL_VEC4:			return QGSCT_BYTE4;
		case GL_FLOAT_MAT2:			return QGSCT_FLOAT4;
		case GL_FLOAT_MAT4:			return QGSCT_FLOAT16;
#ifdef GL_SAMPLER_1D
		case GL_SAMPLER_1D:			return QGSCT_SPLR_1D;
#endif
		case GL_SAMPLER_2D:			return QGSCT_SPLR_2D;
#ifdef GL_SAMPLER_3D
		case GL_SAMPLER_3D:			return QGSCT_SPLR_3D;
#endif
#ifdef GL_SAMPLER_CUBE
		case GL_SAMPLER_CUBE:		return QGSCT_SPLR_CUBE;
#endif
#if false
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_CUBE_SHADOW:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_3D:
		case GL_INT_SAMPLER_CUBE:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_CUBE:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
#endif
		default:					return QGSCT_UNKNOWN;
	}
}
