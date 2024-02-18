//
// qgrdh_qgl.h - OPENGL 디바이스
// 2024-1-2 by kim
//

#pragma once

#include "qg/qg_stub.h"
#include "qg/qg_primtype.h"
#ifdef _QN_MOBILE_
#define QGL_LINK_STATIC		1
#endif
#ifdef _QN_EMSCRIPTEN_
#define QGL_EGL_NO_EXT		1
#endif
#if defined _QN_WINDOWS_ || (defined _QN_UNIX_ && !defined _QN_MOBILE_ && !defined USE_WAYLAND)
#define QGL_MAYBE_GL_CORE	1
#endif

#ifdef QGL_LINK_STATIC
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#else
#include "glad/egl.h"
#include "glad/gl.h"
#endif

//
typedef struct QGLRDH			QglRdh;
typedef struct QGLBUFFER		QglBuffer;
typedef struct QGLRENDERSTATE	QglRenderState;
typedef struct QGLTEXTURE		QglTexture;

#ifndef GL_INVALID_HANDLE
#define GL_INVALID_HANDLE	(GLuint)(-1)
#endif

#define QGL_RDH				qn_cast_type(qg_instance_rdh, QglRdh)
#define QGL_CORE			(QGL_RDH->cfg.core)
#define QGL_PENDING			(&QGL_RDH->pd)
#define QGL_SESSION			(&QGL_RDH->ss)
#define QGL_RESOURCE		(&QGL_RDH->res)

#ifndef GLAPIENTRY
#ifdef APIENTRY
#define GLAPIENTRY			APIENTRY
#elif defined(_WIN32)
#define GLAPIENTRY			__stdcall
#else
#define GLAPIENTRY
#endif
#endif

// 컨피그
typedef struct QGLCONFIG
{
	int					version;
	byte				red, green, blue, alpha;
	byte				depth, stencil, samples;
	byte				stereo, srgb, transparent;
	byte				float_buffer, no_error, robustness;
	bool				core;
	void*				handle;
} QglConfig;

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

// 레이아웃 프로퍼티
typedef struct QGLLAYOUTPROPERTY
{
	QglBuffer*			buffer;
	GLuint				offset;
	GLenum				format;
	GLsizei				stride;
	GLuint				count : 16;
	GLboolean			normalized;
} QglLayoutProperty;

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

// 컬러 포맷을 텍스쳐 포맷으로 구조체
typedef struct QGLTEXFORMAT
{
	GLenum				ifmt;
	GLenum				format;
	GLenum				type;
} QglTexFormat;

// 블렌드를 GLenum으로 구조체
typedef struct QGLBLENDVALUE
{
	GLenum				srcColor, dstColor, colorOp;
	GLenum				srcAlpha, dstAlpha, alphaOp;
} QglBlendValue;

// 블렌드
typedef struct QGLBLEND
{
	QgBlend				state;
	QglBlendValue		value;
} QglBlend;

// 래스터라이저
typedef struct QGLRASTERIZER
{
	GLenum				fill;
	GLenum				cull;
	float				depth_bias;
	float				slope_scale;
	bool				scissor;
	bool				bias;
} QglRasterizer;

// 2D 배치
typedef struct QGLORTHOBATCH
{
	QgBatchCmd			cmd;	
	GLint				offset;
	GLsizei				count;
	GLuint				gl_tex;
	bool				glyph;
} QglOrthoBatch;

// 컨터이너
QN_DECLIMPL_CTNR(QglCtnConfig, QglConfig, qgl_cfg_ctnr);			// 컨피그
QN_DECLIMPL_CTNR(QglCtnLayoutInput, QglLayoutInput, qgl_li_ctnr);	// 레이아웃 입력
QN_DECLIMPL_CTNR(QglCtnUniform, QgVarShader, qgl_uni_ctnr);			// 세이더 유니폼
QN_DECLIMPL_CTNR(QglCtnAttr, QglVarAttr, qgl_attr_ctnr);			// 세이더 어트리뷰트
QN_DECLIMPL_ARRAY(QglArrayOrthoBatch, QglOrthoBatch, qgl_ortho_batch_array);

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
	struct QGLSESSION_TEXTURE
	{
		int					active;
		GLuint				handle[8];
		GLenum				target[8];
	}					texture;

	QgDepth				depth;
	QgStencil			stencil;

	cham				alpha_to_coverage;
	QglBlend			blend[QGRVS_MAX_VALUE];
	QglRasterizer		rasz;
} QglSession;

// 펜딩 데이터
typedef struct QGLPENDING
{
	struct QGLPENDING_RENDER
	{
		QglBuffer*			index_buffer;
		QglBuffer*			vertex_buffers[QGLOS_MAX_VALUE];
		QglRenderState*		render_state;
		QglTexture*			textures[8];
	}					render;
	struct QGLPENDING_DRAW
	{
		uint				topology;
	}					draw;
	struct QGLPENDING_OFF
	{
		void*				buffer[QGRVS_MAX_VALUE];
	}					off;
} QglPending;

// 리소스
typedef struct QGLRESOURCE
{
	char				hdr_vertex[256];
	char				hdr_fragment[256];

	QglTexture*			white_texture;

	struct QGLRESOURCE_ORTHO
	{
		QglRenderState*		render;
		QglRenderState*		glyph;
		QglBuffer*			vertex;
		OrthoVertex*		data;
		size_t				data_count;
		QglArrayOrthoBatch	batches;
	}					ortho;
} QglResource;

// GL 렌더 디바이스
struct QGLRDH
{
	RdhBase				base;

	QglConfig			cfg;
	QglSession			ss;
	QglPending			pd;
	QglResource			res;

	struct
	{
		EGLDisplay			display;
		EGLContext			context;
		EGLSurface			surface;
	}					egl;
#ifdef _QN_WINDOWS_
	HGLRC				context;
#endif
	GLuint				vao;

	bool				disposed;
};

// 버퍼
struct QGLBUFFER
{
	QgBuffer			base;

	GLenum				gl_type;
	GLenum				gl_usage;

	void*				lock_pointer;
};

// 렌더 파이프라인 상태
struct QGLRENDERSTATE
{
	QgRenderState			base;

	struct QGLRENDERSTATE_SHADER
	{
		GLuint				program;
		GLuint				vertex;
		GLuint				fragment;

		QglCtnUniform		uniforms;
		QglCtnAttr			attrs;
		byte				usages[QGLOU_MAX_SIZE];
	}					shader;

	struct QGLRENDERSTATE_LAYOUT
	{
		QglCtnLayoutInput	inputs;
		QglLayoutInput*		stages[QGLOS_MAX_VALUE];
		ushort				counts[QGLOS_MAX_VALUE];
		ushort				strides[QGLOS_MAX_VALUE];
	}					layout;

	QgDepth				depth;
	QgStencil			stencil;

	QgPropBlend			blend;
	QglRasterizer		rasz;
};

// 텍스쳐
struct QGLTEXTURE
{
	QgTexture			base;

	GLenum				gl_target;
	QglTexFormat		gl_enum;	
};

// 버전 찾기
#ifndef _QN_EMSCRIPTEN_
extern int qgl_get_opengl_version(bool is_core, int i);
extern int qgl_index_of_opengl_version(bool is_core, int v);
#endif

// EGL
extern EGLDisplay egl_initialize(_In_ const QglConfig* wanted_config);
extern EGLContext egl_create_context(_In_ EGLDisplay display, _In_ const QglConfig* wanted_config, _Out_ QglConfig* config);
extern EGLSurface egl_create_surface(_In_ EGLDisplay display, _In_ EGLContext context, _In_ const QglConfig* config, _In_ int visual_id);
extern bool egl_make_current(_In_ EGLDisplay display, _In_ EGLSurface surface, _In_ EGLContext context, _In_ const QglConfig* config);
extern void egl_dispose(_In_ EGLDisplay display, _In_ EGLSurface surface, _In_ EGLContext context);

// WGL, GLX?
extern bool gl_initialize(void);
extern void* gl_create_context(_In_ const QglConfig* wanted_config, _Out_ QglConfig* config);
extern bool gl_make_current(_In_ void* context, _In_ const QglConfig* config);
extern bool gl_swap_buffers(void);
extern bool gl_swap_interval(_In_ int interval);
extern void gl_dispose(_In_ void* context);
