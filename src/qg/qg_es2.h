#pragma once

#include "qs_ctn.h"

#define ES2_MAX_VERTEX_ATTRIBUTES		16

typedef struct _es2RefHandle			es2RefHandle;
typedef struct _es2Session				es2Session;
typedef struct _es2Pending				es2Pending;
typedef struct _es2LayoutElement		es2LayoutElement;
typedef struct _es2LayoutProperty		es2LayoutProperty;
typedef struct _es2Uniform		es2Uniform;
typedef struct _es2Attrib			es2Attrib;
typedef struct _es2Rdh					es2Rdh;
typedef struct _es2Vlo					es2Vlo;
typedef struct _es2Shd					es2Shd;
typedef struct _es2Buf					es2Buf;
typedef void(*es2shd_auto_func)(es2Rdh*, GLint, const QgVarShader*);

#if _QN_WINDOWS_
typedef struct _es2Func					es2Func;
struct _es2Func
{
#define DEF_GL_FUNC(ret,func,params)	ret (APIENTRY *func) params;
#include "qg_es2func.h"
#undef DEF_GL_FUNC
};
extern es2Func _es2func;
#define GLFUNC(f)						_es2func.##f
#else
#define GLFUNC(f)						f
#endif

#define ES2RDH_INSTANCE					((es2Rdh*)qg_rdh_instance)


//////////////////////////////////////////////////////////////////////////
// 데이터 타입

// 참조 핸들
struct _es2RefHandle
{
	volatile intptr_t	ref;
	GLuint				handle;
};

// 레이아웃 요소
struct _es2LayoutElement
{
	QgLoStage			stage : 8;
	QgLoUsage			usage : 8;
	int					index : 8;
	GLuint				attr : 8;
	GLuint				size;
	GLenum				format;
	GLuint				offset;
	GLboolean			normalized;
	GLboolean			conv;
};

// 레이아웃 프로퍼티
struct _es2LayoutProperty
{
	const void*			pointer;
	GLuint				buffer;
	GLsizei				stride;
	GLuint				size;
	GLenum				format;
	GLboolean			normalized;
};

// 세이더 유니폼(=세이더 변수) 속성
struct _es2Uniform
{
	QgVarShader			base;

	size_t				hash;
	es2shd_auto_func	auto_func;
};

// 세이더 어트리뷰트 속성
struct _es2Attrib
{
	char				name[32];
	GLint				attrib : 8;
	GLint				size : 8;
	QgLoUsage			usage : 16;
	QgShdConst			cnst : 16;
	size_t				hash;
};

// 세션 데이터
struct _es2Session
{
	GLuint				program;
	uint				layout_mask;
	es2LayoutProperty	layouts[ES2_MAX_VERTEX_ATTRIBUTES];

	GLuint				buf_array;
	GLuint				buf_element_array;
	GLuint				buf_pixel_unpack;

	bool				scissor;
	QnRect				scirect;
};

// 펜딩 데이터
struct _es2Pending
{
	es2Shd*				shd;
	es2Vlo*				vlo;

	es2Buf*				ib;
	es2Buf*				vb[QGLOS_MAX_VALUE];

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

// ES2 렌더 디바이스
struct _es2Rdh
{
	QgRdh				base;

	SDL_Renderer*		renderer;

	es2Session			ss;
	es2Pending			pd;
};
extern void es2_bind_buffer(es2Rdh* self, GLenum type, GLuint id);
extern void es2_commit_layout(es2Rdh* self);
extern void es2_commit_layout_ptr(es2Rdh* self, const void* buffer, GLsizei stride);
extern void es2_commit_shader(es2Rdh* self);

// 레이아웃
struct _es2Vlo
{
	QgVlo				base;

	int					es_cnt[QGLOS_MAX_VALUE];
	es2LayoutElement*	es_elm[QGLOS_MAX_VALUE];
};
extern es2Vlo* es2vlo_allocator(es2Rdh* rdh, int count, const QgPropLayout* layouts);

// 세이더 저장소
QN_DECL_CTNR(es2CtnUniform, es2Uniform);
QN_DECL_CTNR(es2CtnAttrib, es2Attrib);

// 세이더
struct _es2Shd
{
	QgShd				base;

	es2RefHandle*		rfragment;
	es2RefHandle*		rvertex;

	es2CtnUniform		uniforms;
	es2CtnAttrib		attrs;
	int					attr_mask;

	bool				linked;
};
extern es2Shd* es2shd_allocator(es2Rdh* rdh, const char* name);
extern void es2shd_init_auto_uniforms(void);

// 버퍼
struct _es2Buf
{
	QgBuf				base;

	GLenum				gl_type;
	GLenum				gl_usage;

	void*				lockbuf;
};
extern es2Buf* es2buf_allocator(es2Rdh* rdh, QgBufType type, int count, int stride, const void* data);
