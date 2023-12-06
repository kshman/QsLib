#pragma once

#include "qs_ctn.h"

#define QGMAX_ES2_VERTEX_ATTRIBUTE		16

typedef struct Es2RefHandle				Es2RefHandle;
typedef struct Es2Session				Es2Session;
typedef struct Es2Pending				Es2Pending;
typedef struct Es2LayoutElement			Es2LayoutElement;
typedef struct Es2LayoutProperty		Es2LayoutProperty;
typedef struct Es2Uniform				Es2Uniform;
typedef struct Es2Attrib				Es2Attrib;
typedef struct Es2Rdh					Es2Rdh;
typedef struct Es2Vlo					Es2Vlo;
typedef struct Es2Shd					Es2Shd;
typedef struct Es2Buf					Es2Buf;
typedef void(*es2_shd_auto_func)(const Es2Rdh*, GLint, const QgVarShader*);

#if _QN_WINDOWS_
typedef struct Es2Func					Es2Func;
struct Es2Func
{
#define DEF_GL_FUNC(ret,func,params)	ret (APIENTRY *func) params;  // NOLINT(bugprone-macro-parentheses)
#include "qg_es2func.h"
#undef DEF_GL_FUNC
};
extern Es2Func es2_func;
#define GLFUNC(f)						es2_func.##f
#else
#define GLFUNC(f)						f
#endif

#define ES2_RDH_INSTANCE				((Es2Rdh*)qg_rdh_instance)


//////////////////////////////////////////////////////////////////////////
// 데이터 타입

// 참조 핸들
struct Es2RefHandle
{
	volatile nint		ref;
	GLuint				handle;
};

// 레이아웃 요소
struct Es2LayoutElement
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
struct Es2LayoutProperty
{
	nuint				pointer;
	GLuint				buffer;
	GLsizei				stride;
	GLint				size;
	GLenum				format;
	GLboolean			normalized;
};

// 세이더 유니폼(=세이더 변수) 속성
struct Es2Uniform
{
	QgVarShader			base;

	size_t				hash;
	es2_shd_auto_func	auto_func;
};

// 세이더 어트리뷰트 속성
struct Es2Attrib
{
	char				name[32];
	GLint				attrib : 8;
	GLint				size : 8;
	QgLoUsage			usage : 16;
	QgShdConst			cnst : 16;
	size_t				hash;
};

// 세션 데이터
struct Es2Session
{
	GLuint				program;
	uint				layout_mask;
	Es2LayoutProperty	layouts[QGMAX_ES2_VERTEX_ATTRIBUTE];

	GLuint				buf_array;
	GLuint				buf_element_array;
	GLuint				buf_pixel_unpack;

	bool				scissor;
	QnRect				scirect;
};

// 펜딩 데이터
struct Es2Pending
{
	Es2Shd*				shd;
	Es2Vlo*				vlo;

	Es2Buf*				ib;
	Es2Buf*				vb[QGLOS_MAX_VALUE];

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
struct Es2Rdh
{
	QgRdh				base;

	SDL_Renderer*		renderer;

	Es2Session			ss;
	Es2Pending			pd;
};
extern void es2_bind_buffer(Es2Rdh* self, GLenum type, GLuint id);
extern void es2_commit_layout(Es2Rdh* self);
extern void es2_commit_layout_ptr(Es2Rdh* self, const void* buffer, GLsizei stride);
extern void es2_commit_shader(Es2Rdh* self);

// 레이아웃
struct Es2Vlo
{
	QgVlo				base;

	int					es_cnt[QGLOS_MAX_VALUE];
	Es2LayoutElement*	es_elm[QGLOS_MAX_VALUE];
};
extern Es2Vlo* es2_vlo_allocator(Es2Rdh* rdh, int count, const QgPropLayout* layouts);

// 세이더 저장소
QN_DECL_CTNR(Es2CtnUniform, Es2Uniform);
QN_DECL_CTNR(Es2CtnAttrib, Es2Attrib);

// 세이더
struct Es2Shd
{
	QgShd				base;

	Es2RefHandle*		rfragment;
	Es2RefHandle*		rvertex;

	Es2CtnUniform		uniforms;
	Es2CtnAttrib		attrs;
	int					attr_mask;

	bool				linked;
};
extern Es2Shd* es2_shd_allocator(Es2Rdh* rdh, const char* name);
extern void es2_shd_init_auto_uniforms(void);

// 버퍼
struct Es2Buf
{
	QgBuf				base;

	GLenum				gl_type;
	GLenum				gl_usage;

	void*				lockbuf;
};
extern Es2Buf* es2_buf_allocator(Es2Rdh* rdh, QgBufType type, int count, int stride, const void* data);
