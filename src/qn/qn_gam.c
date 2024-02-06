//
// qn_gam.c - 감 잡는 QnGamBase
// 2023-12-27 by kim
//

#include "pch.h"

// 만들었슴
QnGam qn_sc_init(QnGam g, const void* RESTRICT vt)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	base->vt = vt;
	base->ref = 1;
	return g;
}

// 로드
QnGam qn_sc_load(QnGam g)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	base->ref++;
	return g;
}

// 언로드
QnGam qn_sc_unload(QnGam g)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	const volatile int ref = (int)--base->ref;
	if (ref != 0)
	{
		qn_debug_assert(ref > 0, "invalid reference value!");
		return g;
	}
	base->vt->dispose(g);
	return NULL;
}

//
nint qn_sc_get_ref(const QnGam g)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	return base->ref;
}

//
nuint qn_sc_get_desc(const QnGam g)
{
	const QnGamBase* base = qn_cast_type(g, QnGamBase);
	return base->desc;
}

//
nuint qn_sc_set_desc(QnGam g, const nuint ptr)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	const nuint prev = base->desc;
	base->desc = ptr;
	return prev;
}
