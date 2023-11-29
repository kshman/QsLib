#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

//////////////////////////////////////////////////////////////////////////
// ES2 SDL 렌더 디바이스
qvt_name(qgRdh) _vt_es2_rdh =
{
	.base.name = "GLES2Device",
	.base.dispose = qg_rdh_dispose,
};
