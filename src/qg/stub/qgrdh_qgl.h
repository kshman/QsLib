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
typedef struct QGLVARLAYOUT
{
	QgLayoutStage		stage : 16;
	QgLayoutUsage		usage : 16;
	GLuint				offset;
	GLenum				format;
	GLuint				count : 16;
	GLboolean			normalized;
} QglVarLayout;

// 레이아웃 상태 구조체
typedef struct QGLLAYOUTDESC
{
	QglBuffer*			buffer;
	GLuint				offset;
	GLenum				format;
	GLsizei				stride;
	GLuint				count : 16;
	GLboolean			normalized;
} QglLayoutDesc;

// 세이더 어트리뷰트
typedef struct QGLVARATTR
{
	char				name[32 QN_ON64(+ 8)];
	size_t				hash;

	GLint				attrib;
	GLint				size;

	QgLayoutUsage		usage : 8;			// 최대 24(2024-01-20 시점)으로 괜춘
	QgScType			sctype : 8;			// 최대 22(2032-01-20 시점)으로 괜춘		
} QglVarAttr;

// 컬러 포맷을 텍스쳐 포맷으로 구조체
typedef struct QGLTEXFORMATDESC
{
	GLenum				ifmt;
	GLenum				format;
	GLenum				type;
} QglTexFormatDesc;

// 블렌드 상태 구조체
typedef struct QGLBLENDDESC
{
	QgBlend				state;
	GLenum				srcColor, dstColor, colorOp;
	GLenum				srcAlpha, dstAlpha, alphaOp;
} QglBlendDesc;

// 래스터라이저 상태 구조체
typedef struct QGLRASTERIZERDESC
{
	GLenum				fill;
	GLenum				cull;
	float				depth_bias;
	float				slope_scale;
	bool				scissor;
	bool				bias;
} QglRasterizerDesc;

// 배치 아이템
typedef struct QGLBATCHITEM
{
	GLsizei				index;
	GLuint				gl_tex;
} QglBatchItem;

// 평면 배치 아이템
typedef struct QGLBATCHITEMORTHO
{
	QglBatchItem		base;
	OrthoVertex			vertices[4];
} QglBatchItemOrtho;

// 공간 배치 아이템
typedef struct QGBATCHITEMFRST
{
	QglBatchItem		base;
	FrstVertex			vertices[4];
} QglBatchItemFrst;

// 최대 배치 아이템 수
#define QGL_MAX_BATCH_ITEM		MAX_BATCH_ITEM

// 배치 스트림
typedef struct QGLBATCHSTREAM
{
	QgBatchCmd			cmd;
	ushort				index;
	byte				stride;		// 커맨드 폭: 선분: 2, 삼각형: 3, 사각형: 4
	byte				vindex;
	QglBuffer*			vbuffer[2];
} QglBatchStream;

// 평면 배치 스트림
typedef struct QGLBATCHORTHO
{
	QglBatchStream		base;
	QglBatchItemOrtho*	ptrs[QGL_MAX_BATCH_ITEM];
	QglBatchItemOrtho	items[QGL_MAX_BATCH_ITEM];
} QglBatchOrtho;

// 컨터이너
QN_DECLIMPL_CTNR(QglCtnConfig, QglConfig, qgl_cfg_ctnr);			// 컨피그
QN_DECLIMPL_CTNR(QglCtnLayout, QglVarLayout, qgl_layout_ctnr);		// 레이아웃 입력
QN_DECLIMPL_CTNR(QglCtnUniform, QgVarShader, qgl_uni_ctnr);			// 세이더 유니폼
QN_DECLIMPL_CTNR(QglCtnAttr, QglVarAttr, qgl_attr_ctnr);			// 세이더 어트리뷰트

// 세션 데이터 => 주로 디바이스의 상태 값을 저장한다
typedef struct QGLSESSION
{
	struct // 세이더 상태
	{
		GLuint				program;
		uint				attr_mask;					// 어트리뷰트 마스크
		QglLayoutDesc		layouts[QGLOU_MAX_SIZE];	// 레이아웃 상태 저장
	}					shader;
	struct // 버퍼 상태
	{
		GLuint				vertex;		// array
		GLuint				index;		// element array
		GLuint				uniform;
	}					buffer;
	struct // 텍스쳐 상태
	{
		int					active;
		GLuint				handle[8];
		GLenum				target[8];
	}					texture;
	struct // 렌더 상태
	{
		QgDepth				depth;
		QgStencil			stencil;

		cham				alpha_to_coverage;
		QglBlendDesc		blends[QGRVS_MAX_VALUE];
		QglRasterizerDesc	rasz;
	}					state;
} QglSession;

// 펜딩 데이터 => 주로 라이브러리 개체 상태 값을 저장한다
typedef struct QGLPENDING
{
	struct // 렌더 데이터
	{
		QglBuffer*			index_buffer;
		QglBuffer*			vertex_buffers[QGLOS_MAX_VALUE];
		QglRenderState*		render_state;
		QglTexture*			textures[8];
	}					render;
	struct // 그리기 속성
	{
		uint				topology;
	}					draw;
	struct // 오프 버퍼
	{
		void*				buffer[QGRVS_MAX_VALUE];
	}					off;
} QglPending;

// 리소스 => 라이브러리 내부에서 사용하는 리소스를 가지고 있다
typedef struct QGLRESOURCE
{
	char				hdr_vertex[256];
	char				hdr_fragment[256];

	QglRenderState*		ortho_render;			// rdh가 관리하므로 지우지 않아도 된다
	QglRenderState*		ortho_glyph;			// rdh가 관리하므로 지우지 않아도 된다

	QglTexture*			white_texture;
	QglBuffer*			index_stream_rect;		// 사각형 리스트 전용 인덱스 버퍼 (선분 리스트와 삼각 리스트는 인덱스 필요 ㄴㄴ)
	QglBatchStream*		batch_ortho;
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
	QnSpinLock			lock;
	GLenum				gl_type;
	void*				map_ptr;
};

// 렌더 파이프라인 상태
struct QGLRENDERSTATE
{
	QgRenderState			base;

	struct // 세이더
	{
		GLuint				program;
		GLuint				vertex;
		GLuint				fragment;

		QglCtnUniform		uniforms;
		QglCtnAttr			attrs;
		byte				usages[QGLOU_MAX_SIZE];
	}					shader;

	struct // 레이아웃
	{
		QglCtnLayout		inputs;
		QglVarLayout*		stages[QGLOS_MAX_VALUE];
		ushort				counts[QGLOS_MAX_VALUE];
		ushort				strides[QGLOS_MAX_VALUE];
	}					layout;

	QgDepth				depth;
	QgStencil			stencil;

	QgPropBlend			blend;
	QglRasterizerDesc	rasz;
};

// 텍스쳐
struct QGLTEXTURE
{
	QgTexture			base;

	GLenum				gl_target;
	QglTexFormatDesc	gl_enum;
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

// 텍스쳐 포맷 추가 (WebGL, 안드르에서는 되는데 정의가 안돼있음)
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#endif
#ifndef GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F
#endif
#ifndef GL_COMPRESSED_RED_RGTC1
#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#endif
#ifndef GL_COMPRESSED_SIGNED_RED_RGTC1
#define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#endif
#ifndef GL_COMPRESSED_RG_RGTC2
#define GL_COMPRESSED_RG_RGTC2 0x8DBD
#endif
#ifndef GL_COMPRESSED_SIGNED_RG_RGTC2
#define GL_COMPRESSED_SIGNED_RG_RGTC2 0x8DBE
#endif
