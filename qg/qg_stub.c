#include "pch.h"
#include "qg.h"

//////////////////////////////////////////////////////////////////////////
// 스터브
qgStub* qg_stub_instance = NULL;

bool qg_stub_on_init(pointer_t g)
{
	if (qg_stub_instance)
	{
		qn_debug_output(true, "'%s' is already instanced.\n", qn_get_vt(qg_stub_instance, _qnvt_gam)->name);
		return false;
	}

	qgStub* self = qn_gam(g, qgStub);

	self->timer = qn_timer_new();
	qn_timer_set_manual(self->timer, true);

	self->delay = 10;
	self->active = qn_timer_get_abs(self->timer);

	qg_stub_instance = self;
	return true;
}

bool qg_stub_on_disp(pointer_t g)
{
	qgStub* self = qn_gam(g, qgStub);

	qn_timer_delete(self->timer);

	if (qg_stub_instance == self)
		qg_stub_instance = NULL;
}
