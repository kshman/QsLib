//
// qg_stub.c - 스터브
// 2023-12-13 by kim
//

#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"

#ifdef _MSC_VER
//#pragma warning(default:4820)		// 패딩 확인용
#endif

// 인스턴스 포인터
struct StubBase* qg_stub_instance = NULL;
// atexit 지시자
bool qg_stub_atexit = false;

//////////////////////////////////////////////////////////////////////////
// 이벤트 창고

// 이벤트 노드
typedef struct EventNode	EventNode;
struct EventNode
{
	EventNode*			prev;
	EventNode*			next;
	QgEvent				event;
	size_t				key;
	/* 32비트에서 4바이트 패딩 */
};
QN_DECL_NODELIST(EventNodeList, EventNode);

// 이벤트 창고
struct ShedEvent
{
	QnMutex*			mutex;
	struct {
		bool32				reset;				// loop()에서 리셋
		ushort				count;				// loop() 횟수
		ushort				poll;				// poll() 횟수
	}					loop;
	size_t				max_queue;				// 큐 갯수의 최대값
	EventNodeList		queue;					// 현재 메시지 큐
	EventNodeList		prior;					// 현재 우선 순위 메시지 큐
	EventNodeList		cache;					// 캐시
	QnPtrArr			reserved_mems;			// 할당자가 있는 메시지의 데이터 저장소
} shed_event = { NULL, };

// 초기화
static void shed_event_init(void)
{
	shed_event.mutex = qn_mutex_new();
	// 루프
	shed_event.loop.reset = false;
	shed_event.loop.count = 0;
	shed_event.loop.poll = 0;
	// 큐
	shed_event.max_queue = 0;
	qn_nodelist_init(&shed_event.queue);
	// 우선 순위 큐
	qn_nodelist_init(&shed_event.prior);
	// 캐시
	qn_nodelist_init(&shed_event.cache);
	// 예약 메모리
	qn_parr_init(&shed_event.reserved_mems, 0);
}

// 모두 제거
static void shed_event_dispose(void)
{
	qn_ret_if_fail(shed_event.mutex != NULL);

	// 예약 메모리
	qn_parr_each(&shed_event.reserved_mems, qn_memfre);
	qn_parr_disp(&shed_event.reserved_mems);
	// 우선 순위 큐
	qn_nodelist_disp_cb(EventNodeList, &shed_event.prior, qn_memfre);
	// 큐
	qn_nodelist_disp_cb(EventNodeList, &shed_event.queue, qn_memfre);
	// 캐시
	qn_nodelist_disp_cb(EventNodeList, &shed_event.cache, qn_memfre);

	qn_mutex_delete(shed_event.mutex);
}

// 하나 만들거나 캐시에서 꺼내서 큐에 넣기
static bool shed_event_queue(const QgEvent* e)
{
	qn_val_if_fail(shed_event.mutex, false);
	qn_mutex_enter(shed_event.mutex);

	bool ret = false;
	if (qn_nodelist_count(&shed_event.queue) < QGMAX_EVENTS)
	{
		EventNode* node = qn_nodelist_first(&shed_event.cache);
		if (node == NULL)
			node = qn_alloc_1(EventNode);
		else
			qn_nodelist_remove_first(EventNodeList, &shed_event.cache);

		qn_nodelist_append(EventNodeList, &shed_event.queue, node);
		if (shed_event.max_queue < qn_nodelist_count(&shed_event.queue))
			shed_event.max_queue = qn_nodelist_count(&shed_event.queue);

		node->key = 0;
		node->event = *e;
		ret = true;
	}

	qn_mutex_leave(shed_event.mutex);
	return ret;
}

// 우선 큐에서 찾기 콜백
static bool shed_event_prior_queue_key_callback(size_t key, EventNode* node)
{
	return key == node->key;
}

// 우선 큐에 키가 이미 있으면 내용을 갱신하거나, 캐시에서 꺼내던가 새로 할당해서 우선 큐에 넣기
static void shed_event_prior_queue(size_t key, const QgEvent* e)
{
	qn_ret_if_fail(shed_event.mutex);
	qn_mutex_enter(shed_event.mutex);

	EventNode* node;
	qn_nodelist_find(EventNodeList, &shed_event.prior, shed_event_prior_queue_key_callback, key, &node);

	if (node == NULL)
	{
		node = qn_nodelist_first(&shed_event.cache);
		if (node == NULL)
			node = qn_alloc_1(EventNode);
		else
			qn_nodelist_remove_first(EventNodeList, &shed_event.cache);

		qn_nodelist_append(EventNodeList, &shed_event.prior, node);
		node->key = key;
	}

	node->event = *e;

	qn_mutex_leave(shed_event.mutex);
}

// 큐에 있던거 빼내기
static bool shed_event_transition(QgEvent* e)
{
	bool ret = false;
	qn_mutex_enter(shed_event.mutex);

	if (qn_nodelist_is_have(&shed_event.prior))
	{
		EventNode* node = qn_nodelist_first(&shed_event.prior);
		qn_nodelist_remove_first(EventNodeList, &shed_event.prior);
		qn_nodelist_append(EventNodeList, &shed_event.cache, node);

		*e = node->event;
		ret = true;
	}
	else if (qn_nodelist_is_have(&shed_event.queue))
	{
		EventNode* node = qn_nodelist_first(&shed_event.queue);
		qn_nodelist_remove_first(EventNodeList, &shed_event.queue);
		qn_nodelist_append(EventNodeList, &shed_event.cache, node);

		*e = node->event;
		ret = true;
	}

	qn_mutex_leave(shed_event.mutex);
	return ret;
}

// 큐 비울 때 캐시로 옮기기
static void shed_event_flush_callback(EventNode* node)
{
	qn_nodelist_append(EventNodeList, &shed_event.cache, node);
}

// 큐 비우기
static void shed_event_flush(void)
{
	qn_mutex_enter(shed_event.mutex);

	qn_nodelist_each(EventNodeList, &shed_event.prior, shed_event_flush_callback);
	qn_nodelist_each(EventNodeList, &shed_event.queue, shed_event_flush_callback);

	qn_mutex_leave(shed_event.mutex);
}

// 보관 메모리에 넣기
static void shed_event_reserved_mem(void* ptr)
{
	qn_mutex_enter(shed_event.mutex);
	qn_parr_add(&shed_event.reserved_mems, ptr);
	qn_mutex_leave(shed_event.mutex);
}

// 보관 메모리 지우기
static void shed_event_clear_reserved_mem(void)
{
	qn_mutex_enter(shed_event.mutex);
	if (qn_parr_is_have(&shed_event.reserved_mems))
	{
		qn_parr_each(&shed_event.reserved_mems, qn_free);
		qn_parr_clear(&shed_event.reserved_mems);
	}
	qn_mutex_leave(shed_event.mutex);
}


//////////////////////////////////////////////////////////////////////////
// 스터브

// atexit 에 등록할 스터브 정리 함수
static void stub_atexit_callback(void* dummy)
{
	QN_DUMMY(dummy);
	qg_close_stub();
}

//
bool qg_open_stub(const char* title, int display, int width, int height, int flags)
{
	qn_runtime();

	if (qg_stub_instance)
	{
		qn_debug_outputs(true, "STUB", "already opened.");
		return false;
	}

	shed_event_init();

	// 진짜 만들기. 이 안에서 stub_initialize() 함수를 호출해서 기본 초기화
	if (stub_system_open(title, display, width, height, flags) == false)
	{
		qg_close_stub();
		return false;
	}

	// 사양 일괄 처리
	qg_feature(flags, true);

	//
	if (qg_stub_atexit == false)
	{
		qg_stub_atexit = true;
		qn_p_atexit(stub_atexit_callback, NULL);
	}

	return true;
}

//
void stub_initialize(StubBase* stub, int flags)
{
	qg_stub_instance = stub;

	//
	stub->flags = flags & 0x00FFFFFF;						// 확장(24~31) 부분만 빼고 저장
	stub->stats = 0;

	stub->stats = QGSSTT_ACTIVE | QGSSTT_CURSOR;			// 기본으로 활성 상태랑 커서는 켠다
	stub->delay = 10;

	stub->timer = qn_timer_new();
	stub->run = stub->timer->runtime;
	stub->active = stub->timer->abstime;

	stub->mouse.lim.move = 5 * 5;							// 제한 이동 거리(포인트)의 제곱
	stub->mouse.lim.tick = 500;								// 제한 클릭 시간(밀리초)

	qn_pctnr_init(&stub->monitors, 0);
}

//
void qg_close_stub(void)
{
	qn_ret_if_fail(qg_stub_instance);

	stub_system_finalize();

	qn_timer_delete(qg_stub_instance->timer);

	qn_pctnr_each(&qg_stub_instance->monitors, qn_memfre);
	qn_pctnr_disp(&qg_stub_instance->monitors);

	shed_event_dispose();

	qg_stub_instance = NULL;
}

//
int qg_feature(int feature, bool enable)
{
	int count = 0;

	if (QN_TMASK(feature, QGFEATURE_DISABLE_ACS))
	{
		count++;
		stub_system_disable_acs(enable);
	}
	if (QN_TMASK(feature, QGFEATURE_DISABLE_SCRSAVE))
	{
		count++;
		stub_system_diable_scrsave(enable);
	}
	if (QN_TMASK(feature, QGFEATURE_ENABLE_DROP))
	{
		count++;
		stub_system_enable_drop(enable);
	}
	if (QN_TMASK(feature, QGFEATURE_ENABLE_SYSWM))
	{
		QN_SMASK(&qg_stub_instance->flags, QGFEATURE_ENABLE_SYSWM, enable);
		count++;
	}
	if (QN_TMASK(feature, QGFEATURE_ENABLE_IDLE))
	{
		QN_SMASK(&qg_stub_instance->flags, QGFEATURE_ENABLE_IDLE, enable);
		count++;
	}

	return count;
}

//
void qg_set_title(const char* title)
{
	stub_system_set_title(title);
}

//
bool stub_track_mouse_click(QimButton button, QimTrack track)
{
	QgUimMouse* m = &qg_stub_instance->mouse;

	if (track == QIMT_MOVE)
	{
		if (m->clk.tick > 0)
		{
			const int dx = m->clk.loc.x - m->pt.x;
			const int dy = m->clk.loc.y - m->pt.y;
			const uint d = (uint)(dx * dx) + (uint)(dy * dy);
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
					m->clk.ltick = m->clk.tick;
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
	QN_SMASK(&qg_stub_instance->stats, QGSSTT_EXIT, true);
}

//
bool qg_loop(void)
{
	StubBase* stub = qg_stub_instance;
	qn_val_if_fail(stub, false);

	shed_event.loop.reset = true;
	shed_event.loop.count++;
	shed_event_clear_reserved_mem();

	if (QN_TMASK(stub->flags, QGSPECIFIC_VIRTUAL) == false)
	{
		if (!stub_system_poll() || QN_TMASK(stub->stats, QGSSTT_EXIT))
			return false;
	}

	qn_timer_update(stub->timer, true);
	const float adv = (float)stub->timer->advance;
	stub->run = stub->timer->runtime;
	stub->fps = (float)stub->timer->fps;
	stub->reference = adv;
	stub->advance = QN_TMASK(stub->stats, QGSSTT_PAUSE) == false ? adv : 0.0f;

	if (QN_TMASK(stub->flags, QGFEATURE_ENABLE_IDLE))
		qn_sleep(QN_TMASK(stub->stats, QGSSTT_ACTIVE) == false ? stub->delay : 1);

	return true;
}

//
bool qg_poll(QgEvent* ev)
{
	StubBase* stub = qg_stub_instance;
	qn_val_if_fail(stub, false);

	if (shed_event.loop.reset)
	{
		shed_event.loop.reset = false;
		shed_event.loop.poll++;
	}
	if (shed_event.loop.poll != shed_event.loop.count)
	{
		qn_debug_outputf(true, "STUB", "call qg_loop() before qg_poll()! [poll(%d) != loop(%d)]",
			shed_event.loop.poll, shed_event.loop.count);
		shed_event.loop.poll = shed_event.loop.count;
	}

	return qg_pop_event(ev);
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
bool qg_set_prop_double_click(uint density, uint interval)
{
	qn_val_if_fail((size_t)density < 50, false);
	qn_val_if_fail((size_t)interval <= 5000, false);

	QgUimMouse* m = &qg_stub_instance->mouse;
	m->lim.move = density * density;
	m->lim.tick = interval;
	return true;
}

//
bool qg_test_key(QikKey key)
{
	qn_val_if_fail((size_t)key < QIK_MAX_VALUE && key != QIK_NONE, false);
	byte nth = (byte)key / 8;
	byte bit = (byte)key % 8;
	return QN_TBIT(qg_stub_instance->key.key[nth], bit);
}

//
void qg_set_key(QikKey key, bool down)
{
	qn_ret_if_fail((size_t)key < QIK_MAX_VALUE && key != QIK_NONE);
	byte nth = (byte)key / 8;
	byte bit = (byte)key % 8;
	QN_SBIT(&qg_stub_instance->key.key[nth], bit, down);
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
	return (int)(qn_nodelist_count(&shed_event.queue) + qn_nodelist_count(&shed_event.prior));
}

//
void qg_flush_event(void)
{
	shed_event_flush();
}

//
bool qg_pop_event(QgEvent* ev)
{
	qn_val_if_fail(ev, false);
	return shed_event_transition(ev);
}

//
int qg_add_event(const QgEvent* ev)
{
	qn_val_if_fail(ev, -1);

	if (ev->ev == QGEV_WINDOW)
	{
		// 중복 이벤트는 해시로 저장
		size_t key = (QGEV_WINDOW << 16) | (ushort)(ev->wevent.mesg & 0xFFFF);
		shed_event_prior_queue(key, ev);
	}
	else if (ev->ev == QGEV_ACTIVE)
	{
		// 액티브 이벤트도 해시로 저장
		shed_event_prior_queue(QGEV_ACTIVE, ev);
	}
	else
	{
		// 기본 이벤트는 리스트로 저장
		if (shed_event_queue(ev) == false)
			return -2;
	}

	return (int)qn_nodelist_count(&shed_event.queue);
}

//
int qg_add_event_type(QgEventType type)
{
	const QgEvent e = { .ev = type };
	return qg_add_event(&e);
}

//
bool stub_event_on_monitor(QgUdevMonitor* monitor, bool connected, bool primary)
{
	StubBase* stub = qg_stub_instance;
	QgEvent e = { .monitor.ev = QGEV_MONITOR, .monitor.connectd = connected, };
	if (connected)
	{
		if (primary)
			qn_pctnr_insert(&stub->monitors, 0, monitor);
		else
			qn_pctnr_add(&stub->monitors, monitor);
	}
	else
	{
		int nth;
		qn_pctnr_contains(&stub->monitors, monitor, &nth);
		if (nth >= 0)
			qn_pctnr_remove_nth(&stub->monitors, nth);
	}
	size_t i;
	qn_pctnr_each_index(&stub->monitors, i, { qn_pctnr_nth(&stub->monitors, i)->no = (int)i; });

	QgUdevMonitor* another = qn_memdup(monitor, sizeof(QgUdevMonitor));
	e.monitor.monitor = another;

	shed_event_reserved_mem(another);
	bool ret = qg_add_event(&e) > 0;

	if (connected == false)
		qn_free(monitor);
	return ret;
}

//
bool stub_event_on_layout(bool enter)
{
	StubBase* stub = qg_stub_instance;

	if (enter != false)
	{
		// 레이아웃 시작 메시지 안보냄
		QN_SMASK(&stub->stats, QGSSTT_LAYOUT, true);
		return false;
	}
	QN_SMASK(&stub->stats, QGSSTT_LAYOUT, false);

	QmSize prev = stub->client_size;
	stub_system_calc_layout();
	if (qm_eq(&prev, &stub->client_size) == false)
	{
		// 크기가 변하면 레이아웃
		const QgEvent e =
		{
			.layout.ev = QGEV_LAYOUT,
			.layout.bound = stub->window_bound,
			.layout.size = stub->client_size,
		};
		return qg_add_event(&e) > 0;
	}

	return false;
}

//
bool stub_event_on_window_event(QgWindowEventType type, int param1, int param2)
{
	StubBase* stub = qg_stub_instance;

	switch (type)
	{
		case QGWEV_SHOW:
			QN_SBIT(&stub->window_stats, QGWEV_SHOW, true);
			break;
		case QGWEV_HIDE:
			QN_SBIT(&stub->window_stats, QGWEV_SHOW, false);
			break;
		case QGWEV_PAINTED:
			break;
		case QGWEV_RESTORED:	// 갑자기 커졌거나, 크기 복귀 하면 사이즈 메시지가 없어서 레이아웃 처리함
			QN_SBIT(&stub->window_stats, QGWEV_SHOW, true);
			if (QN_TMASK(stub->stats, QGSSTT_LAYOUT) == false)
				stub_system_calc_layout();
			break;
		case QGWEV_MAXIMIZED:	// 갑자기 커졌거나, 크기 복귀 하면 사이즈 메시지가 없어서 레이아웃 처리함
			QN_SBIT(&stub->window_stats, QGWEV_SHOW, true);
			stub_system_calc_layout();
			break;
		case QGWEV_MINIMIZED:
			QN_SBIT(&stub->window_stats, QGWEV_SHOW, false);
			break;
		case QGWEV_MOVED:
			if (stub->window_bound.left == param1 && stub->window_bound.top == param2)
				return 0;
			qm_rect_move(&stub->window_bound, param1, param2);
			break;
		case QGWEV_SIZED:
			if (qm_rect_width(&stub->window_bound) == param1 && qm_rect_height(&stub->window_bound) == param2)
				return 0;
			qm_rect_resize(&stub->window_bound, param1, param2);
			break;
		case QGWEV_FOCUS:
			QN_SBIT(&stub->window_stats, QGWEV_FOCUS, true);
			break;
		case QGWEV_LOSTFOCUS:
			QN_SBIT(&stub->window_stats, QGWEV_FOCUS, false);
			break;
		case QGWEV_CLOSE:
			break;
	}

	const QgEvent e =
	{
		.wevent.ev = QGEV_WINDOW,
		.wevent.mesg = type,
		.wevent.param1 = param1,
		.wevent.param2 = param2,
	};
	return qg_add_event(&e) > 0;
}

//
bool stub_event_on_text(const char* text)
{
	StubBase* stub = qg_stub_instance;
	qn_val_if_fail(QN_TMASK(stub->flags, QGFLAG_TEXT), 0);

	QgEvent e = { .text.ev = QGEV_TEXTINPUT, };
	bool ret = false;
	size_t pos = 0;
	size_t len = qn_u8len(text);
	while (pos < len)
	{
		size_t add = qn_u8lcpy(e.text.data, text + pos, QN_COUNTOF(e.text.data));
		if (add == 0)
			break;
		pos += add;
		ret = qg_add_event(&e) > 0;
	}
	return ret;
}

//
bool stub_event_on_keyboard(QikKey key, bool down)
{
	QgUimKey* uk = &qg_stub_instance->key;
	QgEvent e;

	QikMask mask;
	switch (key)
	{
		case QIK_LSHIFT:
		case QIK_RSHIFT:
			mask = QIKM_SHIFT;
			break;
		case QIK_LCTRL:
		case QIK_RCTRL:
			mask = QIKM_CTRL;
			break;
		case QIK_LALT:
		case QIK_RALT:
			mask = QIKM_ALT;
			break;
		case QIK_LWIN:
		case QIK_RWIN:
			mask = QIKM_WIN;
			break;
		default:
			mask = QIKM_NONE;
			break;
	}

	if (down)
	{
		e.key.ev = QGEV_KEYDOWN;
		if (qg_test_key(key))
			e.key.repeat = true;

		switch (key)
		{
			case QIK_CAPSLOCK:
				uk->mask ^= QIKM_CAPS;
				break;
			case QIK_SCRL:
				uk->mask ^= QIKM_SCRL;
				break;
			case QIK_NUMLOCK:
				uk->mask ^= QIKM_NUM;
				break;
			default:
				uk->mask |= mask;
				break;
		}
	}
	else
	{
		e.key.ev = QGEV_KEYUP;
		e.key.repeat = false;
		qg_set_key(key, false);
		uk->mask &= ~mask;
	}
	e.key.key = key;
	e.key.mask = uk->mask;

	return qg_add_event(&e) > 0;
}

//
bool stub_event_on_reset_keys(void)
{
	QgEvent e = { .key.ev = QGEV_KEYUP, .key.mask = 0, .key.repeat = false };
	QgUimKey* uk = &qg_stub_instance->key;

	bool ret = false;
	for (int i = 0; i < QIK_MAX_VALUE / 8 + 1; i++)
	{
		byte key = uk->key[i];
		if (key == 0)
			continue;
		for (int n = 0; n < 8; n++)
		{
			if (QN_TBIT(key, n) == false)
				continue;
			e.key.key = (QikKey)(i * 8 + n);
			ret = qg_add_event(&e) > 0;
		}
	}
	return ret;
}

//
bool stub_event_on_mouse_move(void)
{
	// 마우스 정보는 시스템 스터브에서 해와야함
	const QgUimMouse* um = &qg_stub_instance->mouse;
	const QgEvent e =
	{
		.mmove.ev = QGEV_MOUSEMOVE,
		.mmove.mask = um->mask,
		.mmove.pt.x = um->pt.x,
		.mmove.pt.y = um->pt.y,
		.mmove.delta.x = um->last.x - um->pt.x,
		.mmove.delta.y = um->last.y - um->pt.y,
	};
	return qg_add_event(&e) > 0;
}

//
bool stub_event_on_mouse_button(QimButton button, bool down)
{
	QgUimMouse* um = &qg_stub_instance->mouse;
	QN_SBIT(&um->mask, button, down);

	const QgEvent e =
	{
		.mbutton.ev = down ? QGEV_MOUSEDOWN : QGEV_MOUSEUP,
		.mbutton.pt = um->pt,
		.mbutton.button = button,
		.mbutton.mask = um->mask,
	};
	bool ret = qg_add_event(&e) > 0;

	if (stub_track_mouse_click(button, down ? QIMT_DOWN : QIMT_UP))
	{
		const QgEvent de =
		{
			.mbutton.ev = QGEV_MOUSEDOUBLE,
			.mbutton.pt = um->pt,
			.mbutton.button = button,
			.mbutton.mask = um->mask,
		};
		ret = qg_add_event(&de) > 0;
	}

	return ret;
}

//
bool stub_event_on_mouse_wheel(float x, float y, bool direction)
{
	QgUimMouse* um = &qg_stub_instance->mouse;

	if (qm_eqf(x, 0.0f) && qm_eqf(y, 0.0f))
		return 0;

	if (x > 0.0f)
	{
		if (um->wheel.accm.x < 0.0f)
			um->wheel.accm.x = 0.0f;
	}
	else if (x < 0.0f)
	{
		if (um->wheel.accm.x > 0.0f)
			um->wheel.accm.x = 0.0f;
	}
	um->wheel.accm.x += x;
	int ix =
		(um->wheel.accm.x > 0.0f) ? (int)floorf(um->wheel.accm.x) :
		(um->wheel.accm.x < 0.0f) ? (int)ceilf(um->wheel.accm.x) : 0;
	um->wheel.accm.x -= (float)ix;

	if (y > 0.0f)
	{
		if (um->wheel.accm.y < 0.0f)
			um->wheel.accm.y = 0.0f;
	}
	else if (y < 0.0f)
	{
		if (um->wheel.accm.y > 0.0f)
			um->wheel.accm.y = 0.0f;
	}
	um->wheel.accm.y += y;
	int iy =
		(um->wheel.accm.y > 0.0f) ? (int)floorf(um->wheel.accm.y) :
		(um->wheel.accm.y < 0.0f) ? (int)ceilf(um->wheel.accm.y) : 0;
	um->wheel.accm.y -= (float)iy;

	qm_point_set(&um->wheel.integral, ix, iy);
	qm_vec2_set(&um->wheel.precise, x, y);

	const QgEvent e =
	{
		.mwheel.ev = QGEV_MOUSEWHEEL,
		.mwheel.pt = um->pt,
		.mwheel.wheel = um->wheel.integral,
		.mwheel.precise = um->wheel.precise,
		.mwheel.direction = direction,
	};
	return qg_add_event(&e) > 0;
}

//
bool stub_event_on_active(bool active, double delta)
{
	const QgEvent e =
	{
		.active.ev = QGEV_ACTIVE,
		.active.active = active,
		.active.delta = delta,
	};
	return qg_add_event(&e) > 0;
}

//
bool stub_event_on_drop(char* data, int len, bool finish)
{
	static bool s_enter = false;

	if (data != NULL)
		shed_event_reserved_mem(data);

	bool ret = 0;
	if (s_enter == false)
	{
		s_enter = true;
		QN_SMASK(&qg_stub_instance->stats, QGSSTT_DROP, true);
		const QgEvent eb = { .drop.ev = QGEV_DROPBEGIN, };
		ret = qg_add_event(&eb) > 0;
		if (ret == false)
			return false;
	}

	const QgEvent e =
	{
		.drop.ev = finish ? QGEV_DROPEND : QGEV_DROPFILE,
		.drop.len = len,
		.drop.data = data,
	};
	ret = qg_add_event(&e) > 0;

	if (finish)
	{
		QN_SMASK(&qg_stub_instance->stats, QGSSTT_DROP, false);
		s_enter = false;
	}
	return ret;
}

//
void stub_toggle_keys(QikMask keymask, bool on)
{
	QgUimKey* uk = &qg_stub_instance->key;
	QN_SMASK(&uk->mask, keymask, on);
}
