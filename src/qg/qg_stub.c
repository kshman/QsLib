//
// qg_stub.c - 스터브
// 2023-12-13 by kim
//

#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"

struct StubBase* qg_stub_instance = NULL;
bool qg_stub_atexit = false;


//////////////////////////////////////////////////////////////////////////
// 스터브

/** @brief atexit에 등록할 스터브 정리 함수 */
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
	stub->stats = QGSTTI_ACTIVE;
	stub->delay = 10;

	stub->timer = qn_timer_new();
	qn_timer_set_manual(stub->timer, true);					// 유닉스 계열에서는 매뉴얼 밖에 안된다. 첨부터 매뉴얼로 만들껄 그랬나
	stub->active = qn_timer_get_abs(stub->timer);

	stub->mouse.lim.move = 5 * 5;							// 제한 이동 거리(포인트)의 제곱
	stub->mouse.lim.tick = 500;								// 제한 클릭 시간(밀리초)

	qn_list_init(qgListEvent, &stub->events);

	if (qg_stub_atexit == false)
	{
		qg_stub_atexit = true;
		qn_atexitp(_stub_atexit, NULL);
	}

	qg_stub_instance = stub;
	return true;
}

//
static void _stub_atexit(void* dummy)
{
	qg_close_stub();
}

//
void qg_close_stub(void)
{
	qn_ret_if_fail(qg_stub_instance);

	stub_system_finalize();

	qn_timer_delete(qg_stub_instance->timer);
	qn_list_disp(qgListEvent, &qg_stub_instance->events);

	qg_stub_instance = NULL;
}

//
bool stub_internal_mouse_clicks(QimButton button, QimTrack track)
{
	QgUimMouse* m = &qg_stub_instance->mouse;

	if (track == QIMT_MOVE)
	{
		if (m->clk.tick > 0)
		{
			const int dx = m->clk.loc.x - m->pt.x;
			const int dy = m->clk.loc.y - m->pt.y;
			const int d = dx * dx + dy * dy;
			if (d > m->lim.move)
			{
				// 마우스가 move 만큼 움직이면 두번 누르기 취소
				m->clk.tick = 0;
			}
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
				const uint d = (uint)qn_tick() - m->clk.tick;
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

//
void qg_exit_loop(void)
{
	qn_ret_if_fail(qg_stub_instance);
	QN_SMASK(&qg_stub_instance->stats, QGSTTI_EXIT, true);
}

//
bool qg_loop(void)
{
	StubBase* stub = qg_stub_instance;
	qn_val_if_fail(stub, false);

	if (QN_TMASK(stub->stats, QGSTTI_VIRTUAL) == false)
	{
		if (!stub_system_poll() || QN_TMASK(stub->stats, QGSTTI_EXIT))
			return false;
	}

	qn_timer_update(stub->timer);
	const float adv = (float)qn_timer_get_adv(stub->timer);
	stub->run = qn_timer_get_run(stub->timer);
	stub->fps = (float)qn_timer_get_fps(stub->timer);
	stub->reference = adv;
	stub->advance = QN_TMASK(stub->stats, QGSTTI_PAUSE) == false ? adv : 0.0f;

	if (QN_TMASK(stub->flags, QGFLAG_IDLE))
		qn_sleep(QN_TMASK(stub->stats, QGSTTI_ACTIVE | QGSTTI_VIRTUAL) == false ? stub->delay : 1);

	return true;
}

//
bool qg_poll(QgEvent* ev)
{
	qn_val_if_fail(ev, false);

	StubBase* stub = qg_stub_instance;
	qn_val_if_fail(stub, false);

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
	qn_val_if_fail((size_t)key < QIK_MAX_VALUE && key != QIK_NONE, false);
	byte nth = (byte)key >> 3;
	byte mask = (byte)key & ~(nth << 3);
	return QN_TMASK(qg_stub_instance->key.key[nth], mask);
}

//
void qg_set_key(QikKey key, bool down)
{
	qn_ret_if_fail((size_t)key < QIK_MAX_VALUE && key != QIK_NONE);
	byte nth = (byte)key >> 3;
	byte mask = (byte)key & ~(nth << 3);
	QN_SMASK(&qg_stub_instance->key.key[nth], mask, down);
}

//
float qg_get_fps(void)
{
	return qg_stub_instance->fps;
}

//
double qg_get_run(void)
{
	return qg_stub_instance->run;
}

//
double qg_get_reference(void)
{
	return qg_stub_instance->reference;
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
	qn_val_if_fail(ev, -1);
	const int cnt = (int)qn_list_count(&qg_stub_instance->events);
	if (cnt >= QGMAX_EVENTS)
		return -1;
	qn_list_append(qgListEvent, &qg_stub_instance->events, *ev);
	return cnt + 1;
}

//
int qg_add_event_type(QgEventType type)
{
	const int cnt = (int)qn_list_count(&qg_stub_instance->events);
	if (type != QGEV_EXIT && cnt >= QGMAX_EVENTS)
		return -1;
	const QgEvent e = { .ev = type };
	qn_list_append(qgListEvent, &qg_stub_instance->events, e);
	return cnt + 1;
}

//
bool qg_pop_event(QgEvent* ev)
{
	qn_val_if_fail(ev, false);
	if (qn_list_is_empty(&qg_stub_instance->events))
		return false;
	*ev = qn_list_data_first(&qg_stub_instance->events);
	qn_list_remove_first(qgListEvent, &qg_stub_instance->events);
	return true;
}
