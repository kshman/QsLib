#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

//////////////////////////////////////////////////////////////////////////
// 스터브
qgStub* qg_stub_instance = NULL;

bool _stub_on_init(pointer_t g)
{
	if (qg_stub_instance)
	{
		qn_debug_output(true, "'%s' is already instanced.\n", qvt_cast(qg_stub_instance, qnGam)->name);
		return false;
	}

	qgStub* self = qm_cast(g, qgStub);

	self->timer = qn_timer_new();
	qn_timer_set_manual(self->timer, true);

	self->delay = 10;

	self->mouse.lim.move = /*제한 이동 거리(포인트)의 제곱*/5 * 5;
	self->mouse.lim.tick = /*제한 클릭 시간(밀리초)*/500;

	qg_stub_instance = self;
	return true;
}

void _stub_on_disp(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);

	qn_timer_delete(self->timer);

	if (qg_stub_instance == self)
		qg_stub_instance = NULL;

}

bool _stub_mouse_clicks(pointer_t g, qImButton button, qimTrack track)
{
	qgStub* self = qm_cast(g, qgStub);
	qgUimMouse* m = &self->mouse;

	if (track == QIMT_MOVE)
	{
		if (m->clk.tick > 0)
		{
			int dx = m->clk.loc.x - m->pt.x;
			int dy = m->clk.loc.y - m->pt.y;
			int d = dx * dx + dy * dy;
			if (d > m->lim.move)	// 마우스가 lim_move 움직이면 두번 누르기 취소
				m->clk.tick = 0;
		}
	}
	else if (track == QIMT_DOWN)
	{
		if (m->clk.tick == 0)
		{
			m->clk.tick = (uint32_t)qn_tick();
			m->clk.loc = m->pt;
			m->clk.btn = button;
		}
		else
		{
			if (m->clk.btn == button)
			{
				uint32_t d = (uint32_t)qn_tick() - m->clk.tick;
				if (d < m->lim.tick)
				{
					// 더블 클릭으로 인정
					m->clk.tick = 0;
					return true;
				}
			}
			// 취소
			m->clk.tick = 0;
		}
	}
	else if (track = QIMT_UP)
	{
		// 버튼 업은 검사 안하지
	}

	return false;
}

//
bool qg_stub_poll(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);

	if (!QN_TEST_MASK(self->stats, QGSTI_VIRTUAL))
	{
		if (!_stub_poll(self) || QN_TEST_MASK(self->stats, QGSTI_EXIT))
			return false;
	}

	qn_timer_update(self->timer);
	float adv = (float)qn_timer_get_adv(self->timer);
	self->run = qn_timer_get_run(self->timer);
	self->fps = qn_timer_get_fps(self->timer);
	self->refadv = adv;

	if (!QN_TEST_MASK(self->stats, QGSTI_PAUSE))
		self->advance = adv;

	if (QN_TEST_MASK(self->flags, QGSTUB_IDLE))
		qn_sleep(!QN_TEST_MASK(self->stats, QGSTI_ACTIVE | QGSTI_VIRTUAL) ? self->delay : 1);

	return true;
}

const qgUimMouse* qg_stub_get_mouse(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return &self->mouse;
}

const qgUimKey* qg_stub_get_key(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return &self->key;
}

bool qg_stub_test_key(pointer_t g, qIkKey key)
{
	qgStub* self = qm_cast(g, qgStub);
	return (uint32_t)key < QIK_MAX_VALUE ? self->key.key[key] : false;
}

double qg_stub_get_runtime(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return self->run;
}

double qg_stub_get_fps(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return self->fps;
}

double qg_stub_get_ref_adv(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return self->refadv;
}

double qg_stub_get_def_adv(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return self->advance;
}

void qg_stub_set_delay(pointer_t g, int delay)
{
	qgStub* self = qm_cast(g, qgStub);
	self->delay = (uint32_t)QN_CLAMP(delay, 1, 1000);
}

int qn_stub_get_delay(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return (int)self->delay;
}
