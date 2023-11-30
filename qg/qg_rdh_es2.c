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

typedef struct es2Rdh
{
	qgRdh				base;
} es2Rdh;

qgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags)
{
	es2Rdh* self = qn_alloc_zero_1(es2Rdh);
	qn_retval_if_fail(self, NULL);

	if (!_rdh_on_init(self, title, width, height, flags))
	{
		qn_free(self);
		return NULL;
	}
}

