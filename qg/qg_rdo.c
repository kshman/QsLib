#include "pch.h"
#include "qg.h"

//////////////////////////////////////////////////////////////////////////
// 레이아웃

//
uint32_t qg_vlo_get_stride(qgVlo* self, qgLoStage stage)
{
	qn_retval_if_fail((size_t)stage < QGLOS_MAX_VALUE, 0);
	return self->stride[stage];
}


//////////////////////////////////////////////////////////////////////////
// 세이더

const char* qg_shd_get_name(qgShd* self)
{
	return self->name;
}

void qg_shd_set_intr(qgShd* self, qgVarShaderFunc func, void* data)
{
	if (func == NULL)
	{
		self->intr.func = NULL;
		self->intr.data = NULL;
		return;
	}

	self->intr.func = (paramfunc_t)func;
	self->intr.data = data;
}

bool qg_shd_bind(qgShd* self, qgShdType type, const void* data, int size, int flags)
{
	return qvt_cast(self, qgShd)->bind(self, type, data, size, flags);
}

bool qg_shd_bind_shd(qgShd* self, qgShdType type, qgShd* shader)
{
	return qvt_cast(self, qgShd)->bind_shd(self, type, shader);
}

bool qg_shd_link(qgShd* self)
{
	return qvt_cast(self, qgShd)->link(self);
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

// 버퍼 타입
qgBufType qg_buf_get_type(qgBuf* self)
{
	return self->type;
}

// 폭
ushort qg_buf_get_stride(qgBuf* self)
{
	return self->stride;
}

// 버퍼 크기
int qg_buf_get_size(qgBuf* self)
{
	return self->size;
}

//
void* qg_buf_map(qgBuf* self)
{
	return qvt_cast(self, qgBuf)->map(self);
}

bool qg_buf_unmap(qgBuf* self)
{
	return qvt_cast(self, qgBuf)->unmap(self);
}

bool qg_buf_data(qgBuf* self, const void* data)
{
	qn_retval_if_fail(data, false);
	return qvt_cast(self, qgBuf)->data(self, data);
}

