//
// qn_gam.c - 감 잡는 QnGam
// 2023-12-27 by kim
//

#include "pch.h"

// 만들었슴
QnGam* qn_sc_init(QnGam* RESTRICT g, void* RESTRICT vt)
{
	g->vt = vt;
	g->ref = 1;
	return g;
}

// 로드
QnGam* qn_sc_load(QnGam* RESTRICT g)
{
	g->ref++;
	return g;
}

// 언로드
QnGam* qn_sc_unload(QnGam* RESTRICT g)
{
	const volatile int ref = (int)--g->ref;
	if (ref != 0)
	{
		qn_assert(ref > 0, "invalid reference value!");
		return g;
	}
	g->vt->dispose(g);
	return NULL;
}

//
nint qn_sc_get_ref(QnGam* RESTRICT g)
{
	return g->ref;
}

//
nuint qn_sc_get_desc(const QnGam* RESTRICT g)
{
	return g->desc;
}

//
nuint qn_sc_set_desc(QnGam* RESTRICT g, const nuint ptr)
{
	const nuint prev = g->desc;
	g->desc = ptr;
	return prev;
}
