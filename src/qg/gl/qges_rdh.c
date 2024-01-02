#include "pch.h"
#include "qges_rdh.h"
#include "qggl_code.h"

//////////////////////////////////////////////////////////////////////////
// ES3 렌더 디바이스

static void es_dispose(QsGam* g);
static void es_reset(QgRdh* rdh);


qv_name(QgRdh) vt_es_rdh =
{
.base.name = "ESDevice",
.base.dispose = es_dispose,

.reset = es_reset,
};

QgRdh* es_allocator(int flags)
{
	EsRdh* self = qn_alloc_zero_1(EsRdh);

	qgl_initialize(&self->base, flags,
		"OPENGLES", "OPENGL ES",
		"OPENGL ES GLSL ES ", "OPENGL ES GLSL ");

	QgDeviceInfo* caps = &rdh_caps(self);
	caps->max_off_count;
	caps->max_layout_count = QN_MIN(caps->max_layout_count)
}
