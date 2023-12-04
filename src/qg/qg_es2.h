#pragma once

#define ES2_MAX_VERTEX_ATTRIBUTES	16

#if _QN_WINDOWS_
typedef struct es2Func
{
#define DEF_ES2_FUNC(ret,func,params) ret (APIENTRY *func) params;
#include "qg_es2func.h"
#undef DEF_ES2_FUNC
} es2Func;

extern es2Func _es2func;
#define ES2FUNC(f)		_es2func.##f
#else
#define ES2FUNC(f)		f
#endif

#define ES2RDH_INSTANCE			((es2Rdh*)qg_rdh_instance)

typedef struct es2RefHandle es2RefHandle;
typedef struct es2Session es2Session;
typedef struct es2Pending es2Pending;
typedef struct es2LayoutElement es2LayoutElement;
typedef struct es2LayoutProperty es2LayoutProperty;
typedef struct es2ShaderUniform es2ShaderUniform;
typedef struct es2ShaderAttrib es2ShaderAttrib;
typedef struct es2Rdh es2Rdh;
typedef struct es2Vlo es2Vlo;
typedef struct es2Shd es2Shd;
typedef struct es2Buf es2Buf;

typedef void(*es2shd_auto_func)(es2Rdh*, GLint, const QgVarShader*);


//////////////////////////////////////////////////////////////////////////
// 데이터 타입

// 참조 핸들
struct es2RefHandle
{
	volatile intptr_t	ref;
	GLuint				handle;
};

// 레이아웃 요소
struct es2LayoutElement
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
struct es2LayoutProperty
{
	const void*			pointer;
	GLuint				buffer;
	GLsizei				stride;
	GLuint				size;
	GLenum				format;
	GLboolean			normalized;
};

// 세이더 유니폼(=세이더 변수) 속성
struct es2ShaderUniform
{
	QgVarShader			base;

	size_t				hash;
	es2shd_auto_func	auto_func;
};

// 세이더 어트리뷰트 속성
struct es2ShaderAttrib
{
	char				name[32];
	GLint				attrib : 8;
	GLint				size : 8;
	QgLoUsage			usage : 16;
	QgShdConst			cnst : 16;
	size_t				hash;
};

// 세션 데이터
struct es2Session
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
struct es2Pending
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
struct es2Rdh
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
struct es2Vlo
{
	QgVlo				base;

	int					es_cnt[QGLOS_MAX_VALUE];
	es2LayoutElement*	es_elm[QGLOS_MAX_VALUE];
};
extern void* _es2vlo_allocator();

// 세이더 저장소
QN_CTNR_DECL(es2CtnShaderUniform, es2ShaderUniform);
QN_CTNR_DECL(es2CtnShaderAttrib, es2ShaderAttrib);

// 세이더
struct es2Shd
{
	QgShd				base;

	es2RefHandle*		rfragment;
	es2RefHandle*		rvertex;

	es2CtnShaderUniform	uniforms;
	es2CtnShaderAttrib	attrs;
	int					attr_mask;

	bool				linked;
};
extern es2Shd* _es2shd_allocator(const char* name);
extern void es2shd_init_auto_uniforms(void);

// 버퍼
struct es2Buf
{
	QgBuf				base;

	GLenum				gl_type;
	GLenum				gl_usage;

	void*				lockbuf;
};
extern es2Buf* _es2buf_allocator(GLuint gl_id, GLenum gl_type, GLenum gl_usage, int stride, int size, QgBufType type);
