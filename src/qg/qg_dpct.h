#pragma once

#include "qs_qg.h"

//////////////////////////////////////////////////////////////////////////
// 데픽트

extern void _dpct_init(QgDpct* self, const QmMat* defm);
extern bool _dpct_update(QgDpct* self, float advance);
extern void _dpct_set_loc(QgDpct* self, const QmVec* loc);
extern void _dpct_set_rot(QgDpct* self, const QmVec* rot);
extern void _dpct_set_scl(QgDpct* self, const QmVec* scl);
