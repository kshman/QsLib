#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

//////////////////////////////////////////////////////////////////////////
// ES2 SDL 렌더 디바이스

bool _es2_construct(pointer_t g, int flags);
void _es2_finalize(pointer_t g);

qvt_name(qgRdh) _vt_es2 =
{
	.base.name = "GLES2Device",
	.base.dispose = _rdh_dispose,

	._construct = _es2_construct,
	._finalize = _es2_finalize,

	.begin = NULL,
	.end = NULL,
	.primitive_begin = NULL,
	.primitive_end = NULL,
	.indexed_primitive_begin = NULL,
	.indexed_primitive_end = NULL,
};

typedef struct es2Rdh
{
	qgRdh				base;
} es2Rdh;

pointer_t _es2_allocator()
{
	es2Rdh* self = qn_alloc_zero_1(es2Rdh);
	qn_retval_if_fail(self, NULL);
	return qm_init(self, &_vt_es2);
}

bool _es2_construct(pointer_t g, int flags)
{
	return true;
}

void _es2_finalize(pointer_t g)
{
}
