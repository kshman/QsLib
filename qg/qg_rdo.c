#include "pch.h"
#include "qg.h"

//////////////////////////////////////////////////////////////////////////
// 레이아웃

//
uint32_t qg_vlo_get_stride(pointer_t g)
{
	qgVlo* self = qm_cast(g, qgVlo);
	return self->stride;
}

//
uint32_t qg_vlo_get_stage(pointer_t g, int stage)
{
	qn_retval_if_fail((size_t)stage < QGLOS_MAX_VALUE, 0);
	qgVlo* self = qm_cast(g, qgVlo);
	return self->stage[stage];
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

// 버퍼 타입
qgBufType qg_buf_get_type(pointer_t g)
{
	qgBuf* self = qm_cast(g, qgBuf);
	return self->type;
}

// 폭
uint32_t qg_buf_get_stride(pointer_t g)
{
	qgBuf* self = qm_cast(g, qgBuf);
	return self->stride;
}

// 버퍼 크기
uint32_t qg_buf_get_size(pointer_t g)
{
	qgBuf* self = qm_cast(g, qgBuf);
	return self->size;
}

//
bool qg_buf_mapped_data(pointer_t g, cpointer_t data, int size)
{
	qn_retval_if_fail(data, false);

	qgBuf* self = qm_cast(g, qgBuf);
	size_t sz = size == 0 ? self->size : (size_t)size;

	pointer_t lock = qvt_cast(self, qgBuf)->map(self);
	qn_retval_if_fail(lock, false);
	memcpy(lock, data, sz);
	return qvt_cast(self, qgBuf)->unmap(self);
}
