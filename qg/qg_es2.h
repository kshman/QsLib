#pragma once

#ifndef APIENTRY
#define APIENTRY
#endif

#if _QN_WINDOWS_
typedef struct es2Func
{
#define DEF_ES2_FUNC(ret,func,params) ret (APIENTRY *func) params;
#include "qg_es2func.h"
#undef DEF_ES2_FUNC
} es2Func;

extern es2Func _es2func;
#endif

typedef struct es2Session
{
	GLuint				program;
	GLuint				buf_array;
	GLuint				buf_element_array;
	GLuint				buf_pixel_unpack;

	bool				scissor;
	qnRect				scirect;
} es2Session;

typedef struct es2Pending
{
	int					tpg;
	int					vcount;
	int					vstride;
	int					vsize;
	int					istride;
	int					isize;
	pointer_t			vdata;
	pointer_t			idata;
} es2Pending;

typedef struct es2Rdh
{
	qgRdh				base;

	SDL_Renderer*		renderer;

	es2Session			ss;
	es2Pending			pd;
} es2Rdh;

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
