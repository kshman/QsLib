#pragma once

#include "qs_qg.h"

//////////////////////////////////////////////////////////////////////////
// 데픽트

extern void _dpct_init(QgDpct* self, const char* name, const QMMAT* defm);
extern bool _dpct_update(QnGam g, float advance);
extern void _dpct_set_loc(QnGam g, const QMVEC* loc);
extern void _dpct_set_rot(QnGam g, const QMVEC* rot);
extern void _dpct_set_scl(QnGam g, const QMVEC* scl);
