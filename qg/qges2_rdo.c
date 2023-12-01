#include "pch.h"
#include "qg.h"
#include "qg_es2.h"

//
void _es2_bind_buffer(es2Rdh* self, GLenum type, GLuint id)
{
	switch (type)
	{
		case GL_ARRAY_BUFFER:
			if (self->ss.buf_array != id)
			{
				ES2FUNC(glBindBuffer)(GL_ARRAY_BUFFER, id);
				self->ss.buf_array = id;
			}
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			if (self->ss.buf_element_array != id)
			{
				ES2FUNC(glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, id);
				self->ss.buf_element_array = id;
			}
			break;
		case GL_PIXEL_UNPACK_BUFFER_NV:
			if (self->ss.buf_pixel_unpack != id)
			{
				ES2FUNC(glBindBuffer)(GL_PIXEL_UNPACK_BUFFER_NV, id);
				self->ss.buf_pixel_unpack = id;
			}
			break;
	}
}

//
void _es2_commit_layout(es2Rdh* self)
{

}


//////////////////////////////////////////////////////////////////////////
// 레이아웃

static void _es2vlo_dispose(pointer_t g);

qvt_name(qnGam) _vt_es2vlo =
{
	.name = "ES2Vlo",
	.dispose = _es2vlo_dispose,
};

pointer_t _es2vlo_allocator()
{
	es2Vlo* self = qn_alloc_zero_1(es2Vlo);
	qn_retval_if_fail(self, NULL);
	return qm_init(self, &_vt_es2vlo);
}

static void _es2vlo_dispose(pointer_t g)
{
	es2Vlo* self = qm_cast(g, es2Vlo);
	for (size_t i = 0; i < QGLOS_MAX_VALUE; i++)
		if (self->es_elm[i])
			qn_free(self->es_elm[i]);
}


//////////////////////////////////////////////////////////////////////////
// 세이더

qvt_name(qgShd) _vt_es2shd =
{
	.base.name = "ES2Shd",
	.base.dispose = NULL,
	.bind = NULL,
	.bind_shd = NULL,
	.bind_name = NULL,
	.add_condition = NULL,
	.clear_condition = NULL,
	.link = NULL,
};

static es2RefHandle* es2shd_handle_new(GLuint handle)
{
	es2RefHandle* ptr = qn_alloc_1(es2RefHandle*);
	ptr->ref = 1;
	ptr->handle = handle;
	return ptr;
}

static void es2shd_handle_unload(es2RefHandle* ptr, GLuint program)
{
	if (ptr)
	{
		if (program > 0)
			ES2FUNC(glDetachShader)(program, ptr->handle);

		intptr_t ref = --ptr->ref;
		if (ref == 0)
		{
			ES2FUNC(glDeleteShader)(ptr->handle);
			qn_free(ptr);
		}
	}
}

pointer_t _es2shd_allocator()
{
	es2Shd* self = qn_alloc_zero_1(es2Shd);
	qn_retval_if_fail(self, NULL);
	qm_set_desc(self, ES2FUNC(glCreateProgram)());
	return qm_init(self, &_vt_es2shd);
}

static void _es2shd_dispose(pointer_t g)
{
	es2Shd* self = qm_cast(g, es2Shd);

	es2shd_handle_unload(self->rvp, 0);
	es2shd_handle_unload(self->rfp, 0);

	ES2FUNC(glDeleteProgram)((GLuint)qm_get_desc(self));

	qn_free(self);
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

static void _es2buf_dispose(pointer_t g);
static pointer_t _es2buf_map(pointer_t g);
static bool _es2buf_unmap(pointer_t g);
static bool _es2buf_data(pointer_t g, pointer_t data);

qvt_name(qgBuf) _vt_es2buf =
{
	.base.name = "ES2Buf",
	.base.dispose = _es2buf_dispose,
	.map = _es2buf_map,
	.unmap = _es2buf_unmap,
	.data = _es2buf_data,
};

pointer_t _es2buf_allocator(GLuint gl_id, GLenum gl_type, GLenum gl_usage, int stride, int size, qgBufType type)
{
	es2Buf* self = qn_alloc_zero_1(es2Buf);
	qn_retval_if_fail(self, NULL);

	qm_set_desc(self, gl_id);
	self->base.type = type;
	self->base.stride = stride;
	self->base.size = size;
	self->gl_type = gl_type;
	self->gl_usage = gl_usage;

	return qm_init(self, &_vt_es2buf);
}

static void _es2buf_dispose(pointer_t g)
{
	es2Buf* self = qm_cast(g, es2Buf);

	if (self->lockbuf)
		qn_free(self->lockbuf);

	GLuint id = (GLuint)qm_get_desc(self);
	ES2FUNC(glDeleteBuffers)(1, &id);

	qn_free(self);
}

static pointer_t _es2buf_map(pointer_t g)
{
	es2Buf* self = qm_cast(g, es2Buf);

	if (self->lockbuf != NULL)
		return NULL;
	if (self->gl_usage != GL_DYNAMIC_DRAW)
		return NULL;

	return self->lockbuf = qn_alloc(self->base.size, uint8_t);
}

static bool _es2buf_unmap(pointer_t g)
{
	es2Buf* self = qm_cast(g, es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);

	_es2_bind_buffer(ES2RDH_INSTANCE, self->gl_type, (GLuint)qm_get_desc(self));
	ES2FUNC(glBufferData)(self->gl_type, self->base.size, self->lockbuf, self->gl_usage);

	qn_free_ptr(&self->lockbuf);

	return true;
}

static bool _es2buf_data(pointer_t g, pointer_t data)
{
	qn_retval_if_fail(data, false);
	es2Buf* self = qm_cast(g, es2Buf);
	qn_retval_if_fail(self->lockbuf == NULL, false);
	qn_retval_if_fail(self->gl_usage == GL_DYNAMIC_DRAW, false);

	_es2_bind_buffer(ES2RDH_INSTANCE, self->gl_type, (GLuint)qm_get_desc(self));
	ES2FUNC(glBufferData)(self->gl_type, self->base.size, data, self->gl_usage);

	return true;
}

