﻿//
// qs_gam.c - 감 잡는 QsGam
// 2023-12-27 by kim
//

#include "pch.h"

// 만들었슴
QsGam* qs_sc_init(QsGam* RESTRICT g, void* RESTRICT vt)
{
	g->vt = vt;
	g->ref = 1;
	return g;
}

// 로드
QsGam* qs_sc_load(QsGam* RESTRICT g)
{
	g->ref++;
	return g;
}

// 언로드
QsGam* qs_sc_unload(QsGam* RESTRICT g)
{
	const volatile int ref = (int)--g->ref;
	if (ref != 0)
	{
		qn_assert(ref > 0, "잘못된 참조값!");
		return g;
	}
	g->vt->dispose(g);
	return NULL;
}

//
nint qs_sc_get_ref(QsGam* RESTRICT g)
{
	return g->ref;
}

//
nuint qs_sc_get_desc(const QsGam* RESTRICT g)
{
	return g->desc;
}

//
QSAPI nuint qs_sc_set_desc(QsGam* RESTRICT g, const nuint ptr)
{
	const nuint prev = g->desc;
	g->desc = ptr;
	return prev;
}
