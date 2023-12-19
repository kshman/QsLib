#include "pch.h"
#include "qs_qn.h"

// 만들었슴
QmGam* qm_stc_init(QmGam* self, void* vt)
{
	self->vt = vt;
	self->ref = 1;
	return self;
}

// 로드
QmGam* qm_stc_load(QmGam* self)
{
	self->ref++;
	return self;
}

// 언로드
QmGam* qm_stc_unload(QmGam* self)
{
	const volatile int ref = (int)--self->ref;
	if (ref != 0)
	{
		qn_assert(ref > 0, "잘못된 참조값!");
		return self;
	}
	self->vt->dispose(self);
	return NULL;
}

//
size_t qm_stc_get_ref(QmGam* self)
{
	return self->ref;
}

//
nuint qm_stc_get_desc(const QmGam* self)
{
	return self->desc;
}

//
QSAPI nuint qm_stc_set_desc(QmGam* self, nuint ptr)
{
	const nuint prev = self->desc;
	self->desc = ptr;
	return prev;
}
