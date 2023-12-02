#include "pch.h"
#include "qn.h"

// 만들었슴
void* qm_init(void* g, void* vt)
{
	qn_retval_if_fail(g, NULL);
	qnGam* self = qm_cast(g, qnGam);
	self->vt = vt;
	self->ref = 1;
	return self;
}

// 로드
void* qm_load(void* g)
{
	qn_retval_if_fail(g, NULL);
	qnGam* self = qm_cast(g, qnGam);
	self->ref++;
	return self;
}

// 언로드
void* qm_unload(void* g)
{
	qn_retval_if_fail(g, NULL);
	qnGam* self = qm_cast(g, qnGam);
	volatile int ref = (int)--self->ref;
	if (ref != 0)
	{
		qn_assert(ref > 0, "invalid reference count!");
		return self;
	}
	self->vt->dispose(self);
	return NULL;
}

//
int qm_get_ref(void* g)
{
	qn_retval_if_fail(g, -1);
	qnGam* self = qm_cast(g, qnGam);
	return (int)self->ref;
}

//
nuint qm_get_desc(void* g)
{
	qn_retval_if_fail(g, 0);
	qnGam* self = qm_cast(g, qnGam);
	return self->desc;
}

//
QNAPI nuint qm_set_desc(void* g, nuint ptr)
{
	qn_retval_if_fail(g, 0);
	qnGam* self = qm_cast(g, qnGam);
	nuint prev = self->desc;
	self->desc = ptr;
	return prev;
}
