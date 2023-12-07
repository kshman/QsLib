#pragma once

#include "qg_glbase.h"

typedef struct Es2Rdh					Es2Rdh;

#define ES2_RDH_INSTANCE				((Es2Rdh*)qg_rdh_instance)


//////////////////////////////////////////////////////////////////////////
// 디바이스

// ES2 렌더 디바이스
struct Es2Rdh
{
	GlRdhBase			base;
};
