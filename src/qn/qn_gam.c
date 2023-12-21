#include "pch.h"
#include "qs_qn.h"

// 만들었슴
QxGam* qx_stc_init(QxGam* restrict self, void* restrict vt)
{
	self->vt = vt;
	self->ref = 1;
	return self;
}

// 로드
QxGam* qx_stc_load(QxGam* restrict self)
{
	self->ref++;
	return self;
}

// 언로드
QxGam* qx_stc_unload(QxGam* restrict self)
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
size_t qx_stc_get_ref(QxGam* restrict self)
{
	return self->ref;
}

//
nuint qx_stc_get_desc(const QxGam* restrict self)
{
	return self->desc;
}

//
QSAPI nuint qx_stc_set_desc(QxGam* restrict self, nuint ptr)
{
	const nuint prev = self->desc;
	self->desc = ptr;
	return prev;
}
