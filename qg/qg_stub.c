﻿#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

// 전역 스터브
qgStub* qg_stub_instance = NULL;


//////////////////////////////////////////////////////////////////////////
// 스터브

qgStub* qg_stub_new(const char* title, int width, int height, int flags)
{
	if (qg_stub_instance)
	{
		qn_debug_output(true, "STUB: '%s' is already instanced.\n", qvt_cast(qg_stub_instance, qnGam)->name);
		return qg_stub_instance;
	}

	qgStub* self = qm_cast(_stub_allocator(), qgStub);
	if (!self)
		return NULL;

	struct stubParam param = { title, width, height, flags };
	_stub_construct(self, &param);

	if (!qvt_cast(self, qgStub)->_construct(self, &param))
	{
		_stub_finalize(self);
		qn_free(self);
		return NULL;
	}

	qg_stub_instance = self;
	return self;
}

void _stub_dispose(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);

	qvt_cast(self, qgStub)->_finalize(self);

	_stub_finalize(self);
	qn_free(self);

	if (qg_stub_instance == self)
		qg_stub_instance = NULL;
}

void _stub_construct(pointer_t g, struct stubParam* param)
{
	qgStub* self = qm_cast(g, qgStub);

	self->flags = param->flags;
	self->sttis = QGSTTI_ACTIVE;
	self->delay = 10;

	self->timer = qn_timer_new();
	qn_timer_set_manual(self->timer, true);

	self->mouse.lim.move = /*제한 이동 거리(포인트)의 제곱*/5 * 5;
	self->mouse.lim.tick = /*제한 클릭 시간(밀리초)*/500;

	qn_list_init(qgListEvent, &self->events);
}

void _stub_finalize(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);

	qn_timer_delete(self->timer);

	qn_list_disp(qgListEvent, &self->events);
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

void qg_stub_close(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	QN_SET_MASK(&self->sttis, QGSTTI_EXIT, true);
}

//
bool qg_stub_loop(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);

	if (!QN_TEST_MASK(self->sttis, QGSTTI_VIRTUAL))
	{
		if (!qvt_cast(self, qgStub)->_poll(self) || QN_TEST_MASK(self->sttis, QGSTTI_EXIT))
			return false;
	}

	qn_timer_update(self->timer);
	float adv = (float)qn_timer_get_adv(self->timer);
	self->run = qn_timer_get_run(self->timer);
	self->fps = qn_timer_get_fps(self->timer);
	self->refadv = adv;

	if (!QN_TEST_MASK(self->sttis, QGSTTI_PAUSE))
		self->advance = adv;

	if (QN_TEST_MASK(self->flags, QGFLAG_IDLE))
		qn_sleep(!QN_TEST_MASK(self->sttis, QGSTTI_ACTIVE | QGSTTI_VIRTUAL) ? self->delay : 1);

	return true;
}

//
bool qg_stub_poll(pointer_t g, qgEvent* ev)
{
	qn_retval_if_fail(ev, false);

	qgStub* self = qm_cast(g, qgStub);

	if (qn_list_is_empty(&self->events))
		return false;

	*ev = qn_list_data_first(&self->events);
	qn_list_remove_first(qgListEvent, &self->events);
	return true;
}

//
const qgUimMouse* qg_stub_get_mouse(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return &self->mouse;
}

//
const qgUimKey* qg_stub_get_key(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return &self->key;
}

//
bool qg_stub_test_key(pointer_t g, qIkKey key)
{
	qgStub* self = qm_cast(g, qgStub);
	return (uint32_t)key < QIK_MAX_VALUE ? self->key.key[key] : false;
}

//
double qg_stub_get_runtime(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return self->run;
}

//
double qg_stub_get_fps(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return self->fps;
}

//
double qg_stub_get_ref_adv(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return self->refadv;
}

//
double qg_stub_get_def_adv(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return self->advance;
}

//
void qg_stub_set_delay(pointer_t g, int delay)
{
	qgStub* self = qm_cast(g, qgStub);
	self->delay = (uint32_t)QN_CLAMP(delay, 1, 1000);
}

//
int qg_stub_get_delay(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return (int)self->delay;
}

//
int qg_stub_left_events(pointer_t g)
{
	qgStub* self = qm_cast(g, qgStub);
	return (int)qn_list_count(&self->events);
}

//
int qg_stub_add_event(pointer_t g, const qgEvent* ev)
{
	qn_retval_if_fail(ev, -1);
	qgStub* self = qm_cast(g, qgStub);
	int cnt = (int)qn_list_count(&self->events);
	if (cnt >= QNEVENT_MAX_VALUE)
		return -1;
	qn_list_append(qgListEvent, &self->events, *ev);
	return cnt + 1;
}

//
int qg_stub_add_event_type(pointer_t g, qgEventType type)
{
	qgStub* self = qm_cast(g, qgStub);
	int cnt = (int)qn_list_count(&self->events);
	if (type != QGEV_EXIT && cnt >= QNEVENT_MAX_VALUE)
		return -1;
	qgEvent e = { .ev = type };
	qn_list_append(qgListEvent, &self->events, e);
	return cnt + 1;
}

//
bool qg_stub_pop_event(pointer_t g, qgEvent* ev)
{
	qn_retval_if_fail(ev, false);
	qgStub* self = qm_cast(g, qgStub);
	if (qn_list_is_empty(&self->events))
		return false;
	*ev = qn_list_data_first(&self->events);
	qn_list_remove_first(qgListEvent, &self->events);
	return true;
}
