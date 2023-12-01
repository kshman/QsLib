#pragma once

#define ES2_MAX_VERTEXT_ATTRS	32

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
typedef struct es2ShaderAttrib es2ShaderAttrib;
typedef struct es2Rdh es2Rdh;
typedef struct es2Vlo es2Vlo;
typedef struct es2Shd es2Shd;
typedef struct es2Buf es2Buf;

//
struct es2RefHandle
{
	volatile intptr_t	ref;
	GLuint				handle;
};

// 레이아웃 요소
struct es2LayoutElement
{
	qgLoUsage			usage;
	int					index;
	GLenum				format;
	GLuint				offset;
	GLuint				attrib;
	GLuint				size;
	GLboolean			normalized;
	GLboolean			conv;
};

// 레이아웃 프로퍼티
struct es2LayoutProperty
{
	bool				enable;
	pointer_t			pointer;
	GLuint				buffer;
	GLsizei				stride;
	GLuint				size;
	GLenum				format;
	GLboolean			normalized;
};

// 세이더 속성
struct es2ShaderAttrib
{
	GLint				attrib;
	qgShdConst			cnst : 16;
	uint16_t			size;
	qgLoUsage			usage : 16;
	uint16_t			index;
	size_t				hash;
	char				name[32];
	es2ShaderAttrib*	next;
};

//
struct es2Session
{
	GLuint				program;
	es2LayoutProperty	layouts[ES2_MAX_VERTEXT_ATTRS];

	GLuint				buf_array;
	GLuint				buf_element_array;
	GLuint				buf_pixel_unpack;

	bool				scissor;
	qnRect				scirect;
};

//
struct es2Pending
{
	es2Shd*				shd;
	es2Vlo*				vlo;

	es2Buf*				ib;
	es2Buf*				vb[ES2_MAX_VERTEXT_ATTRS];

	int					tpg;
	int					vcount;
	int					vstride;
	int					vsize;
	int					istride;
	int					isize;
	pointer_t			vdata;
	pointer_t			idata;
};

// ES2 렌더 디바이스
struct es2Rdh
{
	qgRdh				base;

	SDL_Renderer*		renderer;

	es2Session			ss;
	es2Pending			pd;
};
extern void es2_bind_buffer(es2Rdh* self, GLenum type, GLuint id);

// 레이아웃
struct es2Vlo
{
	qgVlo				base;

	int					es_cnt[QGLOS_MAX_VALUE];
	es2LayoutElement*	es_elm[QGLOS_MAX_VALUE];
};
extern pointer_t _es2vlo_allocator();

// 세이더 저장소
QN_CTNR_DECL(es2CtnVarShader, qgVarShader);
QN_CTNR_DECL(es2CtnShaderAttrib, es2ShaderAttrib);

// 세이더
struct es2Shd
{
	qgShd				base;

	es2RefHandle*		rfragment;
	es2RefHandle*		rvertex;

	es2CtnVarShader		vars;
	es2CtnShaderAttrib	attrs;

	int					amask;
	int					acount[QGLOU_MAX_VALUE];
	es2ShaderAttrib*	alink[QGLOU_MAX_VALUE];

	bool				linked;
};
extern es2Shd* _es2shd_allocator(const char* name);

// 버퍼
struct es2Buf
{
	qgBuf				base;

	GLenum				gl_type;
	GLenum				gl_usage;

	pointer_t			lockbuf;
};
extern es2Buf* _es2buf_allocator(GLuint gl_id, GLenum gl_type, GLenum gl_usage, int stride, int size, qgBufType type);

//
QN_INLINE void _es2_mat4_tex_form(qnMat4* m, float radius, float cx, float cy, float tx, float ty, float sx, float sy)
{
	float c, s;
	qn_sincosf(radius, &s, &c);

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
