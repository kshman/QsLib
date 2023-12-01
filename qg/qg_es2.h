#pragma once

#define ES2_MAX_LAYOUT_ATTRIB	12

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

typedef struct es2Session es2Session;
typedef struct es2Pending es2Pending;
typedef struct es2Rdh es2Rdh;
typedef struct es2VloElement es2VloElement;
typedef struct es2Vlo es2Vlo;
typedef struct es2Buf es2Buf;

//
struct es2Session
{
	GLuint				program;
	GLuint				buf_array;
	GLuint				buf_element_array;
	GLuint				buf_pixel_unpack;

	bool				scissor;
	qnRect				scirect;
};

//
struct es2Pending
{
	es2Buf*				ib;
	es2Buf*				vb[ES2_MAX_LAYOUT_ATTRIB];

	int					tpg;
	int					vcount;
	int					vstride;
	int					vsize;
	int					istride;
	int					isize;
	pointer_t			vdata;
	pointer_t			idata;
};

//
struct es2Rdh
{
	qgRdh				base;

	SDL_Renderer*		renderer;

	es2Session			ss;
	es2Pending			pd;
};

void _es2_bind_buffer(es2Rdh* self, GLenum type, GLuint id);

// 레이아웃 요소
struct es2VloElement
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

// 레이아웃
struct es2Vlo
{
	qgVlo				base;

	int					es_cnt[QGLOS_MAX_VALUE];
	es2VloElement*		es_elm[QGLOS_MAX_VALUE];
};
extern pointer_t _es2vlo_allocator();

// 버퍼
struct es2Buf
{
	qgBuf				base;

	GLenum				gl_type;
	GLenum				gl_usage;

	pointer_t			lockbuf;
};
extern pointer_t _es2buf_allocator(GLuint gl_id, GLenum gl_type, GLenum gl_usage, int stride, int size, qgBufType type);

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
