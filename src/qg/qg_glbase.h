#pragma once

#include "qs_ctn.h"
#include "qg_glfunc.h"

//
typedef struct GlRefHandle				GlRefHandle;
typedef struct GlUniformSession			GlUniformSession;
typedef struct GlUniformPending			GlUniformPending;
typedef struct GlLayoutElement			GlLayoutElement;
typedef struct GlLayoutProperty			GlLayoutProperty;
typedef struct GlUniform				GlUniform;
typedef struct GlAttrib					GlAttrib;
typedef struct GlSession				GlSession;
typedef struct GlPending				GlPending;
typedef void(*gl_shd_auto_func)(const QgRdh*, GLint, const QgVarShader*);

typedef struct GlRdhBase				GlRdhBase;
typedef struct GlVlo					GlVlo;
typedef struct GlShd					GlShd;
typedef struct GlBuf					GlBuf;

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
struct GlLayoutProperty
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
	QgShdConst			cnst : 16;
	size_t				hash;
};

// 세이더 저장소
QN_DECL_CTNR(GlCtnUniform, GlUniform)
QN_DECL_CTNR(GlCtnAttrib, GlAttrib)

// 세션 데이터
struct GlSession
{
	GLuint				program;

	struct LayoutSession
	{
		uint				mask;
		uint				count;
		GlLayoutProperty*	props;
	}					layout;

	GLuint				buf_array;
	GLuint				buf_element_array;
	GLuint				buf_pixel_unpack;

	bool				scissor;
	QnRect				scirect;
};

// 펜딩 데이터
struct GlPending
{
	GlShd*				shd;
	GlVlo*				vlo;

	GlBuf*				ib;
	GlBuf*				vb[QGLOS_MAX_VALUE];

	int					tpg;
	int					vcount;
	int					vstride;
	int					vsize;
	int					istride;
	int					isize;
	void*				vdata;
	void*				idata;
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

extern QgVlo* gl_create_layout(QgRdh* rdh, int count, const QgPropLayout* layouts);
extern QgShd* gl_create_shader(QgRdh* rdh, const char* name);
extern QgBuf* gl_create_buffer(QgRdh* rdh, QgBufType type, int count, int stride, const void* data);

extern void gl_set_shader(QgRdh* rdh, QgShd* shader, QgVlo* layout);
extern bool gl_set_index(QgRdh* rdh, QgBuf* buffer);
extern bool gl_set_vertex(QgRdh* rdh, QgLoStage stage, QgBuf* buffer);

extern bool gl_draw(QgRdh* rdh, QgTopology tpg, int vcount);
extern bool gl_draw_indexed(QgRdh* rdh, QgTopology tpg, int icount);
extern bool gl_ptr_draw(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata);
extern bool gl_ptr_draw_indexed(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata);

extern void gl_bind_buffer(GlRdhBase* self, GLenum type, GLuint id);
extern void gl_commit_layout(GlRdhBase* self);
extern void gl_commit_layout_ptr(GlRdhBase* self, const void* buffer, GLsizei stride);
extern void gl_commit_shader(GlRdhBase* self);


//////////////////////////////////////////////////////////////////////////
// RDO

// 레이아웃
struct GlVlo
{
	QgVlo				base;

	int					es_cnt[QGLOS_MAX_VALUE];
	GlLayoutElement*	es_elm[QGLOS_MAX_VALUE];
};
extern GlVlo* gl_vlo_allocator(QgRdh* rdh, int count, const QgPropLayout* layouts);

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

	void*				lockbuf;
};
extern GlBuf* gl_buf_allocator(QgRdh* rdh, QgBufType type, int count, int stride, const void* data);

