#pragma once

#include "qs_ctn.h"
#include "qg_glfunc.h"

//
typedef struct GlRefHandle				GlRefHandle;
typedef struct GlUniformSession			GlUniformSession;
typedef struct GlUniformPending			GlUniformPending;
typedef struct GlLayoutElement			GlLayoutElement;
typedef struct GlLayoutProp				GlLayoutProp;
typedef struct GlUniform				GlUniform;
typedef struct GlAttrib					GlAttrib;
typedef struct GlDepthStencilProp		GlDepthStencilProp;
typedef struct GlRasterizeProp			GlRasterizeProp;
typedef struct GlSession				GlSession;
typedef struct GlPending				GlPending;
typedef void(*gl_shd_auto_func)(const QgRdh*, GLint, const QgVarShader*);

typedef struct GlRdhBase				GlRdhBase;
typedef struct GlVlo					GlVlo;
typedef struct GlShd					GlShd;
typedef struct GlBuf					GlBuf;
typedef struct GlDsm					GlDsm;
typedef struct GlRsz					GlRsz;

#ifndef GL_INVALID_HANDLE
#define GL_INVALID_HANDLE				(GLuint)-1
#endif

#define GLBASE_RDH_INSTANCE				((GlRdhBase*)qg_rdh_instance)


//////////////////////////////////////////////////////////////////////////
// 데이터 타입

// 참조 핸들
struct GlRefHandle
{
	volatile nint		ref;
	GLuint				handle;
};

// 레이아웃 요소
struct GlLayoutElement
{
	QgLoStage			stage : 8;
	QgLoUsage			usage : 8;
	int					index : 8;
	GLuint				attr : 8;
	GLint				size;
	GLenum				format;
	GLuint				offset;
	GLboolean			normalized;
	GLboolean			conv;
};

// 레이아웃 프로퍼티
struct GlLayoutProp
{
	nuint				pointer;
	GLuint				buffer;
	GLsizei				stride;
	GLint				size;
	GLenum				format;
	GLboolean			normalized;
};

// 세이더 유니폼(=세이더 변수) 속성
struct GlUniform
{
	QgVarShader			base;

	size_t				hash;
	gl_shd_auto_func	auto_func;
};

// 세이더 어트리뷰트 속성
struct GlAttrib
{
	char				name[32];
	GLint				attrib : 8;
	GLint				size : 8;
	QgLoUsage			usage : 16;
	QgShdConstType			cnst : 16;
	size_t				hash;
};

// 세이더 저장소
QN_DECL_CTNR(GlCtnUniform, GlUniform)
QN_DECL_CTNR(GlCtnAttrib, GlAttrib)

// 뎁스 스텐셀
struct GlDepthStencilProp
{
	struct GlDepth
	{
		GLboolean			enable;
		GLboolean			write;
		GLenum				func;
	}					depth;
	struct GlStencilInfo
	{
		GLboolean			enable;
		GLboolean			two_side;
		struct GlStencilValue
		{
			GLenum				func;
			GLenum				fail;
			GLenum				pass;
			GLenum				z_fail;
		}					front, back;
		struct GlStencilMask
		{
			uint				read : 8;
			uint				write : 8;
		}					mask;
	}					stencil;
};

// 래스터라이저
struct GlRasterizeProp
{
	GLenum				fill;
	GLenum				cull;
	float				depth_bias;
	float				slope_scale;
};

// 세션 데이터
struct GlSession
{
	GLuint				program;
	struct GlSessionLayout
	{
		uint				mask;
		uint				count;
		GlLayoutProp*		props;
	}					layout;
	struct GlSessionBuffer
	{
		GLuint				array;
		GLuint				element_array;
		GLuint				pixel_unpack;
	}					buf;

	GlDepthStencilProp	dsm_prop;
	GlRasterizeProp		rsz_prop;

	bool				scissor;
	QnRect				sci_rect;
};

// 펜딩 데이터
struct GlPending
{
	GlShd*				shd;
	GlVlo*				vlo;
	GlBuf*				ib;
	GlBuf*				vb[QGLOS_MAX_VALUE];

	GlDepthStencilProp	dsm_prop;
	GlRasterizeProp		rsz_prop;
};


//////////////////////////////////////////////////////////////////////////
// 디바이스

// GL 렌더 디바이스 베이스
struct GlRdhBase
{
	QgRdh				base;

	SDL_Window*			window;
	SDL_Renderer*		renderer;

	GlSession			ss;
	GlPending			pd;

	nint				disposed;
};
extern void gl_initialize(GlRdhBase* self, SDL_Window* window, SDL_Renderer* renderer, const int flags);
extern void gl_finalize(GlRdhBase* self);
extern void gl_reset(QgRdh* rdh);

extern void gl_clear(QgRdh* rdh, int flag, const QnColor* color, int stencil, float depth);
extern bool gl_begin(QgRdh* rdh, bool clear);
extern void gl_end(QgRdh* rdh);
extern void gl_flush(QgRdh* rdh);

extern QgVlo* gl_create_layout(QgRdh* rdh, int count, const QgLayoutInput* layouts);
extern QgShd* gl_create_shader(QgRdh* rdh, const char* name);
extern QgBuf* gl_create_buffer(QgRdh* rdh, QgBufType type, int count, int stride, const void* data);
extern QgDsm* gl_create_depth_stencil(QgRdh* rdh, const QgDepthStencilProp* prop);
extern QgRsz* gl_create_rasterizer(QgRdh* rdh, const QgRasterizerProp* prop);

extern void gl_set_shader(QgRdh* rdh, QgShd* shader, QgVlo* layout);
extern bool gl_set_index(QgRdh* rdh, QgBuf* buffer);
extern bool gl_set_vertex(QgRdh* rdh, QgLoStage stage, QgBuf* buffer);
extern bool gl_set_depth_stencil(QgRdh* rdh, QgDsm* depth_stencil);
extern bool gl_set_rasterizer(QgRdh* rdh, QgRsz* rasterizer);

extern bool gl_draw(QgRdh* rdh, QgTopology tpg, int vcount);
extern bool gl_draw_indexed(QgRdh* rdh, QgTopology tpg, int icount);
extern bool gl_ptr_draw(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata);
extern bool gl_ptr_draw_indexed(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata);

extern void gl_bind_buffer(GlRdhBase* self, GLenum type, GLuint id);
extern void gl_commit_layout(GlRdhBase* self);
extern void gl_commit_layout_ptr(GlRdhBase* self, const void* buffer, GLsizei stride);
extern void gl_commit_shader(GlRdhBase* self);
extern void gl_commit_depth_stencil(GlRdhBase* self);


//////////////////////////////////////////////////////////////////////////
// RDO

// 레이아웃
struct GlVlo
{
	QgVlo				base;

	int					es_cnt[QGLOS_MAX_VALUE];
	GlLayoutElement*	es_elm[QGLOS_MAX_VALUE];
};
extern GlVlo* gl_vlo_allocator(QgRdh* rdh, int count, const QgLayoutInput* layouts);

// 세이더
struct GlShd
{
	QgShd				base;

	GlRefHandle*		rfragment;
	GlRefHandle*		rvertex;

	GlCtnUniform		uniforms;
	GlCtnAttrib			attrs;
	int					attr_mask;

	bool				linked;
};
extern GlShd* gl_shd_allocator(QgRdh* rdh, const char* name);
extern void gl_shd_init_auto_uniforms(void);
extern void gl_shd_process_uniforms(GlShd* self);
extern bool gl_shd_link(QgShd* g);

// 버퍼
struct GlBuf
{
	QgBuf				base;

	GLenum				gl_type;
	GLenum				gl_usage;

	void*				lock_buf;
};
extern GlBuf* gl_buf_allocator(QgRdh* rdh, QgBufType type, int count, int stride, const void* data);

// 뎁스 스텐실
struct GlDsm
{
	QgDsm				base;

	GlDepthStencilProp	prop;
};
extern GlDsm* gl_dsm_allocator(QgRdh* rdh, const QgDepthStencilProp* prop);

// 래스터라이즈
struct GlRsz
{
	QgRsz				base;

	GlRasterizeProp		prop;
};
extern GlRsz* gl_rsz_allocator(QgRdh* rdh, const QgRasterizerProp* prop);

