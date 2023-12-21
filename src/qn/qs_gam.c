#include "pch.h"
#include "qs_qn.h"

// 만들었슴
QsGam* qs_sc_init(QsGam* restrict self, void* restrict vt)
{
	self->vt = vt;
	self->ref = 1;
	return self;
}

// 로드
QsGam* qs_sc_load(QsGam* restrict self)
{
	self->ref++;
	return self;
}

// 언로드
QsGam* qs_sc_unload(QsGam* restrict self)
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
size_t qs_sc_get_ref(QsGam* restrict self)
{
	return self->ref;
}

//
nuint qs_sc_get_desc(const QsGam* restrict self)
{
	return self->desc;
}

//
QSAPI nuint qs_sc_set_desc(QsGam* restrict self, nuint ptr)
{
	const nuint prev = self->desc;
	self->desc = ptr;
	return prev;
}
