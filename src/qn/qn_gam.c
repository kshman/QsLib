#include "pch.h"
#include "qs_qn.h"

// 만들었슴
QmGam* qm_init(QmGam* self, void* vt)
{
	qn_retval_if_fail(self, NULL);
	self->vt = vt;
	self->ref = 1;
	return self;
}

// 로드
QmGam* qm_load(QmGam* self)
{
	qn_retval_if_fail(self, NULL);
	self->ref++;
	return self;
}

// 언로드
QmGam* qm_unload(QmGam* self)
{
	qn_retval_if_fail(self, NULL);
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
size_t qm_get_ref(QmGam* self)
{
	qn_retval_if_fail(self, (size_t)-1);
	return self->ref;
}

//
nuint qm_get_desc(QmGam* self)
{
	qn_retval_if_fail(self, 0);
	return self->desc;
}

//
QSAPI nuint qm_set_desc(QmGam* self, nuint ptr)
{
	qn_retval_if_fail(self, 0);
	nuint prev = self->desc;
	self->desc = ptr;
	return prev;
}
