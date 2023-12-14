#include "pch.h"
#include "qs_qg.h"

//////////////////////////////////////////////////////////////////////////
// 레이아웃

//
uint qg_vlo_get_stride(QgVlo* self, QgLoStage stage)
{
	qn_val_if_fail((size_t)stage < QGLOS_MAX_VALUE, 0);
	return self->stride[stage];
}


//////////////////////////////////////////////////////////////////////////
// 세이더

const char* qg_shd_get_name(QgShd* self)
{
	return self->name;
}

void qg_shd_set_intr(QgShd* self, QgVarShaderFunc func, void* data)
{
	if (func == NULL)
	{
		self->intr_func = NULL;
		self->intr_data = NULL;
		return;
	}

	self->intr_func = func;
	self->intr_data = data;
}

bool qg_shd_bind(QgShd* self, QgShdType type, const void* data, int size, int flags)
{
	return qv_cast(self, QgShd)->bind(self, type, data, size, flags);
}

bool qg_shd_bind_shd(QgShd* self, QgShdType type, QgShd* shader)
{
	return qv_cast(self, QgShd)->bind_shd(self, type, shader);
}

bool qg_shd_bind_file(QgShd* self, QgShdType type, const char* filename, int flags)
{
	int size;
	void* data = qn_file_alloc(filename, &size);
	qn_val_if_fail(data, false);
	const bool ret = qv_cast(self, QgShd)->bind(self, type, data, size, flags);
	qn_free(data);
	return ret;
}

bool qg_shd_link(QgShd* self)
{
	return qv_cast(self, QgShd)->link(self);
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

// 버퍼 타입
QgBufType qg_buf_get_type(QgBuf* self)
{
	return self->type;
}

// 폭
ushort qg_buf_get_stride(QgBuf* self)
{
	return self->stride;
}

// 버퍼 크기
int qg_buf_get_size(QgBuf* self)
{
	return self->size;
}

//
void* qg_buf_map(QgBuf* self)
{
	return qv_cast(self, QgBuf)->map(self);
}

bool qg_buf_unmap(QgBuf* self)
{
	return qv_cast(self, QgBuf)->unmap(self);
}

bool qg_buf_data(QgBuf* self, const void* data)
{
	qn_val_if_fail(data, false);
	return qv_cast(self, QgBuf)->data(self, data);
}

