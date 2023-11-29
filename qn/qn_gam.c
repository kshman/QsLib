#include "pch.h"
#include "qn.h"

// 만들었슴
pointer_t qm_init(pointer_t g, pointer_t vt)
{
	qn_retval_if_fail(g, NULL);
	qnGam* self = qm_cast(g, qnGam);
	self->vt = vt;
	self->ref = 1;
	return self;
}

// 로드
pointer_t qm_load(pointer_t g)
{
	qn_retval_if_fail(g, NULL);
	qnGam* self = qm_cast(g, qnGam);
	self->ref++;
	return self;
}

// 언로드
pointer_t qm_unload(pointer_t g)
{
	qn_retval_if_fail(g, NULL);
	qnGam* self = qm_cast(g, qnGam);
	int ref = --self->ref;
	if (ref != 0)
	{
		qn_assert(ref > 0, "invalid reference count!");
		return self;
	}
	self->vt->dispose(self);
	return NULL;
}

//
pointer_t qm_get_ptr(pointer_t g)
{
	qn_retval_if_fail(g, NULL);
	qnGam* self = qm_cast(g, qnGam);
	return self->ptr;
}

//
pointer_t qm_set_ptr(pointer_t g, pointer_t ptr)
{
	qn_retval_if_fail(g, NULL);
	qnGam* self = qm_cast(g, qnGam);
	pointer_t prev = self->ptr;
	self->ptr = ptr;
	return prev;
}

//
int qm_get_ref(pointer_t g)
{
	qn_retval_if_fail(g, -1);
	qnGam* self = qm_cast(g, qnGam);
	return self->ref;
}
