#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"

QSAPI struct StubBase* qg_stub_instance;
struct StubBase* qg_stub_instance = NULL;
bool qg_stub_atexit = false;


//////////////////////////////////////////////////////////////////////////
// 스터브

static void _stub_atexit(void* dummy);

//
bool qg_open_stub(const char* title, int width, int height, int flags)
{
	qn_runtime(NULL);

	if (qg_stub_instance)
	{
		qn_debug_outputf(true, "STUB", "already opened.");
		return false;
	}

	struct StubParam param = { title, width, height, flags };
	struct StubBase* stub = stub_system_open(&param);

	stub->flags = flags;
	stub->sttis = QGSTTI_ACTIVE;
	stub->delay = 10;

	stub->timer = qn_timer_new();
	qn_timer_set_manual(stub->timer, true);
	stub->active = qn_timer_get_abs(stub->timer);

	stub->mouse.lim.move = /*제한 이동 거리(포인트)의 제곱*/5 * 5;
	stub->mouse.lim.tick = /*제한 클릭 시간(밀리초)*/500;

	qn_list_init(qgListEvent, &stub->events);

	if (!qg_stub_atexit)
	{
		qg_stub_atexit = true;
		qn_atexitp(_stub_atexit, NULL);
	}

	qg_stub_instance = stub;
	return true;
}

static void _stub_atexit(void* dummy)
{
	qg_close_stub();
}

void qg_close_stub(void)
{
	qn_ret_if_fail(qg_stub_instance);

	stub_system_finalize();

	qn_timer_delete(qg_stub_instance->timer);
	qn_list_disp(qgListEvent, &qg_stub_instance->events);

	qg_stub_instance = NULL;
}

bool stub_internal_mouse_clicks(QimButton button, QimTrack track)
{
	QgUimMouse* m = &qg_stub_instance->mouse;

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
			m->clk.tick = (uint)qn_tick();
			m->clk.loc = m->pt;
			m->clk.btn = button;
		}
		else
		{
			if (m->clk.btn == button)
			{
				uint d = (uint)qn_tick() - m->clk.tick;
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
	else if (track == QIMT_UP)
	{
		// 버튼 업은 검사 안하지
	}

	return false;
}

void qg_exit_loop(void)
{
	qn_ret_if_fail(qg_stub_instance);
	QN_SET_MASK(&qg_stub_instance->sttis, QGSTTI_EXIT, true);
}

//
bool qg_loop(void)
{
	struct StubBase* stub = qg_stub_instance;
	qn_retval_if_fail(stub, false);

	if (!QN_TEST_MASK(stub->sttis, QGSTTI_VIRTUAL))
	{
		if (!stub_system_poll() || QN_TEST_MASK(stub->sttis, QGSTTI_EXIT))
			return false;
	}

	qn_timer_update(stub->timer);
	float adv = (float)qn_timer_get_adv(stub->timer);
	stub->run = qn_timer_get_run(stub->timer);
	stub->fps = qn_timer_get_fps(stub->timer);
	stub->refadv = adv;

	if (!QN_TEST_MASK(stub->sttis, QGSTTI_PAUSE))
		stub->advance = adv;

	if (QN_TEST_MASK(stub->flags, QGFLAG_IDLE))
		qn_sleep(!QN_TEST_MASK(stub->sttis, QGSTTI_ACTIVE | QGSTTI_VIRTUAL) ? stub->delay : 1);

	return true;
}

//
bool qg_poll(QgEvent* ev)
{
	qn_retval_if_fail(ev, false);
	struct StubBase* stub = qg_stub_instance;
	qn_retval_if_fail(stub, false);

	if (qn_list_is_empty(&stub->events))
		return false;

	*ev = qn_list_data_first(&stub->events);
	qn_list_remove_first(qgListEvent, &stub->events);
	return true;
}

//
const QgUimKey* qg_get_key_info(void)
{
	return &qg_stub_instance->key;
}

//
const QgUimMouse* qg_get_mouse_info(void)
{
	return &qg_stub_instance->mouse;
}

//
bool qg_test_key(QikKey key)
{
	return (uint)key < QIK_MAX_VALUE ? qg_stub_instance->key.key[key] : false;
}

//
double qg_get_run_time(void)
{
	return qg_stub_instance->run;
}

//
double qg_get_fps(void)
{
	return qg_stub_instance->fps;
}

//
double qg_get_ref_adv(void)
{
	return qg_stub_instance->refadv;
}

//
double qg_get_advance(void)
{
	return qg_stub_instance->advance;
}

//
int qg_get_delay(void)
{
	return (int)qg_stub_instance->delay;
}

//
void qg_set_delay(int delay)
{
	qg_stub_instance->delay = (uint)QN_CLAMP(delay, 1, 1000);
}

//
int qg_left_events(void)
{
	return (int)qn_list_count(&qg_stub_instance->events);
}

//
int qg_add_event(const QgEvent* ev)
{
	qn_retval_if_fail(ev, -1);
	int cnt = (int)qn_list_count(&qg_stub_instance->events);
	if (cnt >= QNEVENT_MAX_VALUE)
		return -1;
	qn_list_append(qgListEvent, &qg_stub_instance->events, *ev);
	return cnt + 1;
}

//
int qg_add_event_type(QgEventType type)
{
	int cnt = (int)qn_list_count(&qg_stub_instance->events);
	if (type != QGEV_EXIT && cnt >= QNEVENT_MAX_VALUE)
		return -1;
	QgEvent e = { .ev = type };
	qn_list_append(qgListEvent, &qg_stub_instance->events, e);
	return cnt + 1;
}

//
bool qg_pop_event(QgEvent* ev)
{
	qn_retval_if_fail(ev, false);
	if (qn_list_is_empty(&qg_stub_instance->events))
		return false;
	*ev = qn_list_data_first(&qg_stub_instance->events);
	qn_list_remove_first(qgListEvent, &qg_stub_instance->events);
	return true;
}
