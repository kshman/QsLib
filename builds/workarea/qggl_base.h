#pragma once

#include <stdbool.h>
#include "qs_qn.h"
#include "qs_math.h"
#include "qs_qg.h"
#ifdef __EMSCRIPTEN__
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include "glad/glad.h"
#endif

//
typedef struct GlRdh		GlRdh;
typedef struct GlBuf		GlBuf;

#ifndef GL_INVALID_HANDLE
#define GL_INVALID_HANDLE	(GLuint)-1
#endif

#define GL_RDH_INSTANCE		((GlRdh*)qg_rdh_instance)


//////////////////////////////////////////////////////////////////////////
// 데이터 타입

/// @brief GL 캐파
typedef struct GlCaps
{
} GlCaps;

/** @brief 세션 데이터 */
typedef struct GlSession
{
	QgDepth				depth;
	QgStencil			stencil;
} GlSession;

/** @brief 펜딩 데이터 */
typedef struct GlPending
{
	GlBuf*				ib;
	GlBuf*				vb[QGLOS_MAX_VALUE];
} GlPending;


//////////////////////////////////////////////////////////////////////////
// 디바이스

/** @brief GL 렌더 디바이스 */
struct GlRdh
{
	QgRdh				base;

	SDL_Window*			window;
	SDL_Renderer*		renderer;

	GlSession			ss;
	GlPending			pd;

	nint				disposed;
};

qv_name(GlRdh)
{
	qv_name(QgRdh);

	void (*swap_interval)(void*, int);
	void (*swap_buffers)(void*, void*);
}

extern void gl_initialize(GlRdh* self, SDL_Window* window, SDL_Renderer* renderer, const int flags);
extern void gl_finalize(GlRdh* self);
extern void gl_reset(QgRdh* rdh);

extern void gl_clear(QgRdh* rdh, int flag, const QmColor* color, int stencil, float depth);
extern bool gl_begin(QgRdh* rdh, bool clear);
extern void gl_end(QgRdh* rdh);
extern void gl_flush(QgRdh* rdh);

extern QgBuffer* gl_create_buffer(QgRdh* rdh, QgBufType type, int count, int stride, const void* data);
extern QgRender* gl_create_render(QgRdh* self, const QgPropRender* prop, bool compile_shader);

extern bool gl_set_index(QgRdh* rdh, QgBuffer* buffer);
extern bool gl_set_vertex(QgRdh* rdh, QgLoStage stage, QgBuffer* buffer);
extern void gl_set_render(QgRdh* self, QgRender* render);

extern bool gl_draw(QgRdh* rdh, QgTopology tpg, int vcount);
extern bool gl_draw_indexed(QgRdh* rdh, QgTopology tpg, int icount);
extern bool gl_ptr_draw(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata);
extern bool gl_ptr_draw_indexed(QgRdh* rdh, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata);

//extern void gl_bind_buffer(GlRdhBase* self, GLenum type, GLuint id);
//extern void gl_commit_layout(GlRdhBase* self);
//extern void gl_commit_layout_ptr(GlRdhBase* self, const void* buffer, GLsizei stride);
//extern void gl_commit_shader(GlRdhBase* self);
//extern void gl_commit_depth_stencil(GlRdhBase* self);


//////////////////////////////////////////////////////////////////////////
// RDO

// 버퍼
struct GlBuffer
{
	QgBuffer			base;

	GLenum				gl_type;
	GLenum				gl_usage;

	void*				lock_buf;
};
extern GlBuf* gl_buf_allocator(QgRdh* rdh, QgBufType type, int count, int stride, const void* data);
