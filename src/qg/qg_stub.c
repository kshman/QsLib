//
// qg_stub.c - 스터브
// 2023-12-13 by kim
//

// ReSharper disable CppParameterMayBeConstPtrOrRef

#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qs_supp.h"
#ifdef _QN_EMSCRIPTEN_
#include <emscripten.h>
#endif

#ifdef _MSC_VER
//#pragma warning(default:4820)		// 패딩 확인용
#pragma warning(disable:6011)
#endif

// 인스턴스 포인터
StubBase* qg_instance_stub = NULL;
RdhBase* qg_instance_rdh = NULL;

// atexit 지시자
bool qg_stub_atexit = false;


//////////////////////////////////////////////////////////////////////////
// 이벤트 창고

// 이벤트 노드
typedef struct EventNode	EventNode;
struct EventNode
{
	EventNode* PREV;
	EventNode* NEXT;
	QgEvent				event;
	size_t				key;
	/* 32비트에서 4바이트 패딩 */
};

// 컨테이너
QN_DECLIMPL_LNODE(EventNodeList, EventNode, event_lnode);
// 배열
QN_IMPL_ARRAY(QnPtrArray, void*, reserved_mems);

// 이벤트 창고
struct ShedEvent
{
	QnMutex* mutex;
	struct {
		cham				reset;				// loop()에서 리셋
		nuint				count;				// loop() 횟수
		nuint				poll;				// poll() 횟수
	}					loop;
	size_t				max_queue;				// 큐 갯수의 최대값
	EventNodeList		queue;					// 현재 메시지 큐
	EventNodeList		prior;					// 현재 우선 순위 메시지 큐
	EventNodeList		cache;					// 캐시
	QnPtrArray			reserved_mems;			// 할당자가 있는 메시지의 데이터 저장소
} shed_event =
{
	.mutex = NULL,
	.loop.reset = false,
	.loop.count = 0,
	.loop.poll = 0,
	.max_queue = 0,
};

// 초기화
static void shed_event_init(void)
{
	shed_event.mutex = qn_new_mutex();
	// 루프
	shed_event.loop.reset = false;
	shed_event.loop.count = 0;
	shed_event.loop.poll = 0;
	// 큐
	shed_event.max_queue = 0;
	event_lnode_init(&shed_event.queue);
	// 우선 순위 큐
	event_lnode_init(&shed_event.prior);
	// 캐시
	event_lnode_init(&shed_event.cache);
	// 예약 메모리
	reserved_mems_init(&shed_event.reserved_mems, 0);
}

// 모두 제거
static void shed_event_dispose(void)
{
	qn_return_when_fail(shed_event.mutex != NULL, /*void*/);

	// 예약 메모리
	reserved_mems_loop(&shed_event.reserved_mems, qn_mem_free_ptr);
	reserved_mems_dispose(&shed_event.reserved_mems);
	// 우선 순위 큐
	event_lnode_dispose_callback(&shed_event.prior, qn_mem_free);
	// 큐
	event_lnode_dispose_callback(&shed_event.queue, qn_mem_free);
	// 캐시
	event_lnode_dispose_callback(&shed_event.cache, qn_mem_free);

	qn_delete_mutex(shed_event.mutex);
}

// 하나 만들거나 캐시에서 꺼내서 큐에 넣기
static bool shed_event_queue(const QgEvent* e)
{
	qn_return_when_fail(shed_event.mutex, false);
	qn_mutex_enter(shed_event.mutex);

	bool ret = false;
	if (event_lnode_count(&shed_event.queue) < QGMAX_EVENTS)
	{
		EventNode* node = event_lnode_head(&shed_event.cache);
		if (node == NULL)
			node = qn_alloc_1(EventNode);
		else
			event_lnode_remove_head(&shed_event.cache, false);

		event_lnode_append(&shed_event.queue, node);
		if (shed_event.max_queue < event_lnode_count(&shed_event.queue))
			shed_event.max_queue = event_lnode_count(&shed_event.queue);

		node->key = 0;
		node->event = *e;
		ret = true;
	}

	qn_mutex_leave(shed_event.mutex);
	return ret;
}

// 우선 큐에서 찾기 콜백
static bool shed_event_prior_queue_key_callback(void* keyptr, const void* nodeptr)
{
	const size_t key = (size_t)keyptr;
	const EventNode* node = (const EventNode*)nodeptr;
	return key == node->key;
}

// 우선 큐에 키가 이미 있으면 내용을 갱신하거나, 캐시에서 꺼내던가 새로 할당해서 우선 큐에 넣기
static void shed_event_prior_queue(const size_t key, const QgEvent* e)
{
	qn_return_when_fail(shed_event.mutex, /*void*/);
	qn_mutex_enter(shed_event.mutex);

	EventNode* node = event_lnode_find_callback(&shed_event.prior, shed_event_prior_queue_key_callback, (void*)key);

	if (node == NULL)
	{
		node = event_lnode_head(&shed_event.cache);
		if (node == NULL)
			node = qn_alloc_1(EventNode);
		else
			event_lnode_remove_head(&shed_event.cache, false);

		event_lnode_append(&shed_event.prior, node);
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

	if (event_lnode_is_have(&shed_event.prior))
	{
		EventNode* node = event_lnode_head(&shed_event.prior);
		event_lnode_remove_head(&shed_event.prior, false);
		event_lnode_append(&shed_event.cache, node);

		*e = node->event;
		ret = true;
	}
	else if (event_lnode_is_have(&shed_event.queue))
	{
		EventNode* node = event_lnode_head(&shed_event.queue);
		event_lnode_remove_head(&shed_event.queue, false);
		event_lnode_append(&shed_event.cache, node);

		*e = node->event;
		ret = true;
	}

	qn_mutex_leave(shed_event.mutex);
	return ret;
}

// 큐에 있던거 얻고, 큐에서 지우지는 않음
static bool shed_event_restore(QgEvent* e)
{
	bool ret = false;
	qn_mutex_enter(shed_event.mutex);

	if (event_lnode_is_have(&shed_event.prior))
	{
		const EventNode* node = event_lnode_head(&shed_event.prior);
		*e = node->event;
		ret = true;
	}
	else if (event_lnode_is_have(&shed_event.queue))
	{
		const EventNode* node = event_lnode_head(&shed_event.queue);
		*e = node->event;
		ret = true;
	}

	qn_mutex_leave(shed_event.mutex);
	return ret;
}

// 큐 비울 때 캐시로 옮기기
static void shed_event_flush_callback(void* nodeptr)
{
	EventNode* node = (EventNode*)nodeptr;
	event_lnode_append(&shed_event.cache, node);
}

// 큐 비우기
static void shed_event_flush(void)
{
	qn_mutex_enter(shed_event.mutex);
	if (event_lnode_is_have(&shed_event.prior))
	{
		event_lnode_loop(&shed_event.prior, shed_event_flush_callback);
		event_lnode_reset(&shed_event.prior);
	}
	if (event_lnode_is_have(&shed_event.queue))
	{
		event_lnode_loop(&shed_event.queue, shed_event_flush_callback);
		event_lnode_reset(&shed_event.queue);
	}
	qn_mutex_leave(shed_event.mutex);
}

// 보관 메모리에 넣기
static void shed_event_reserved_mem(void* ptr)
{
	qn_mutex_enter(shed_event.mutex);
	reserved_mems_add(&shed_event.reserved_mems, ptr);
	qn_mutex_leave(shed_event.mutex);
}

// 보관 메모리 지우기
static void shed_event_clear_reserved_mem(void)
{
	qn_mutex_enter(shed_event.mutex);
	if (reserved_mems_is_have(&shed_event.reserved_mems))
	{
		reserved_mems_loop(&shed_event.reserved_mems, qn_mem_free_ptr);
		reserved_mems_clear(&shed_event.reserved_mems);
	}
	qn_mutex_leave(shed_event.mutex);
}


//////////////////////////////////////////////////////////////////////////
// 스터브

// atexit 에 등록할 스터브 정리 함수
static void stub_atexit_callback(void* dummy)
{
	QN_DUMMY(dummy);
	qg_close_rdh();
	qg_close_stub();
}

//
bool qg_open_stub(const char* title, int display, int width, int height, int flags, int features)
{
	qn_runtime(NULL);

	if (qg_instance_stub)
	{
		qn_mesgb("STUB", "already opened");
		return false;
	}

	shed_event_init();

	// 진짜 만들기. 이 안에서 stub_initialize() 함수를 호출해서 기본 초기화
	if (stub_system_open(title, display, width, height, flags, features) == false)
	{
		qg_close_stub();
		return false;
	}

	// 사양 일괄 처리
	qg_feature(features, true);

	//
	if (qg_stub_atexit == false)
	{
		qg_stub_atexit = true;
		qn_p_atexit(stub_atexit_callback, NULL);
	}

	return true;
}

//
void stub_initialize(StubBase* stub, QgFlag flags)
{
	qg_instance_stub = stub;

	stub->mutex = qn_new_mutex();

	stub->flags = flags;
	stub->features = QGFEATURE_NONE;				// 나중에 설정해야 하므로 NONE
	stub->stats = QGSST_ACTIVE | QGSST_CURSOR;		// 기본으로 활성 상태랑 커서는 켠다

	stub->timer = qn_create_timer();
	if (QN_TMASK(flags, QGFLAG_VSYNC) == false)
		qn_timer_set_cut(stub->timer, 60);
	stub->active = stub->timer->runtime;

	stub->mouse.lim.move = 10 * 10 + 10 * 10;		// 제한 이동 거리(포인트)의 제곱
	stub->mouse.lim.tick = 500;						// 제한 클릭 시간(밀리초)

#ifdef _DEBUG
	stub->key_exit = QIK_ESC;
#endif

	monitor_ctn_init(&stub->monitors, 0);
	eventcb_list_init(&stub->callbacks);
}

//
void qg_close_stub(void)
{
	StubBase* self = qg_instance_stub;
	qn_return_when_fail(self, /*void*/);

	stub_system_finalize();

	size_t i;
	QN_CTNR_FOREACH(self->monitors, 0, i)
		qn_free(monitor_ctn_nth(&self->monitors, i));
	monitor_ctn_dispose(&self->monitors);
	eventcb_list_dispose(&self->callbacks);

	qn_delete_mutex(self->mutex);
	qn_unloadu(self->timer);

	for (i = 0; i < QN_COUNTOF(self->mount); i++)
		qn_unload(self->mount[i]);

	shed_event_dispose();

	qg_instance_stub = NULL;
}

//
int qg_feature(int features, bool enable)
{
	StubBase* self = qg_instance_stub;
	int count = 0;

	if (QN_TMASK(features, QGFEATURE_DISABLE_ACS))
	{
		count++;
		if (QN_TMASK(self->features, QGFEATURE_DISABLE_ACS) != enable)
			if (stub_system_disable_acs(enable))
				QN_SMASK(self->features, QGFEATURE_DISABLE_ACS, enable);
	}
	if (QN_TMASK(features, QGFEATURE_DISABLE_SCRSAVE))
	{
		count++;
		if (QN_TMASK(self->features, QGFEATURE_DISABLE_SCRSAVE) != enable)
			if (stub_system_disable_scr_save(enable))
				QN_SMASK(self->features, QGFEATURE_DISABLE_SCRSAVE, enable);
	}
	if (QN_TMASK(features, QGFEATURE_ENABLE_DROP))
	{
		count++;
		if (QN_TMASK(self->features, QGFEATURE_ENABLE_DROP) != enable)
			if (stub_system_enable_drop(enable))
				QN_SMASK(self->features, QGFEATURE_ENABLE_DROP, enable);
	}
	if (QN_TMASK(features, QGFEATURE_ENABLE_SYSWM))
	{
		count++;
		QN_SMASK(self->features, QGFEATURE_ENABLE_SYSWM, enable);
	}
	if (QN_TMASK(features, QGFEATURE_ENABLE_IDLE))
	{
		count++;
		QN_SMASK(self->features, QGFEATURE_ENABLE_IDLE, enable);
	}
	if (QN_TMASK(features, QGFEATURE_RELATIVE_MOUSE))
	{
		count++;
		if (QN_TMASK(self->features, QGFEATURE_RELATIVE_MOUSE) != enable)
			if (stub_system_relative_mouse(enable))
				QN_SMASK(self->features, QGFEATURE_RELATIVE_MOUSE, enable);
	}
	if (QN_TMASK(features, QGFEATURE_REMOVE_EVENTS))
	{
		count++;
		QN_SMASK(self->features, QGFEATURE_REMOVE_EVENTS, enable);
	}
	if (QN_TMASK(features, QGFEATURE_ENABLE_ASPECT))
	{
		count++;
		if (enable)
			stub_system_aspect();
		QN_SMASK(self->features, QGFEATURE_ENABLE_ASPECT, enable);
	}

	return count;
}

//
QgFeature qg_query_features(void)
{
	return qg_instance_stub->features;
}

//
void qg_toggle_fullscreen(bool fullscreen)
{
	StubBase* stub = qg_instance_stub;
	if (fullscreen)
	{
		if (QN_TMASK(stub->stats, QGSST_FULLSCREEN))
			return;
		QN_SMASK(stub->stats, QGSST_FULLSCREEN, true);
		stub_system_fullscreen(true);
	}
	else
	{
		if (QN_TMASK(stub->stats, QGSST_FULLSCREEN) == false)
			return;
		QN_SMASK(stub->stats, QGSST_FULLSCREEN, false);
		stub_system_fullscreen(false);
	}
}

//
bool qg_get_fullscreen_state(void)
{
	return QN_TMASK(qg_instance_stub->stats, QGSST_FULLSCREEN);
}

//
void qg_set_title(const char* title)
{
	stub_system_set_title(title);
}

//
bool qg_mount(int index, const char* path, const char* mode)
{
	StubBase* stub = qg_instance_stub;
	qn_return_when_fail((size_t)index < QN_COUNTOF(stub->mount), false);

	qn_unload(stub->mount[index]);
	stub->mount[index] = qn_open_mount(path, mode);
	return stub->mount[index] != NULL;
}

//
bool qg_fuse(int index, const char* path, bool diskfs, bool preload)
{
	StubBase* stub = qg_instance_stub;
	qn_return_when_fail((size_t)index < QN_COUNTOF(stub->mount), false);

	qn_unload(stub->mount[index]);
	stub->mount[index] = qn_create_fuse(path, diskfs, preload);
	return stub->mount[index] != NULL;
}

//
QnMount* qg_get_mount(int index)
{
	const StubBase* stub = qg_instance_stub;
	qn_return_when_fail((size_t)index < QN_COUNTOF(stub->mount), NULL);
	return stub->mount[index];
}

//
const QgUdevMonitor* qg_get_monitor(int index)
{
	const StubBase* stub = qg_instance_stub;
	qn_return_when_fail((size_t)index < monitor_ctn_count(&stub->monitors), NULL);
	return monitor_ctn_nth(&stub->monitors, index);
}

//
const QgUdevMonitor* qg_get_current_monitor(void)
{
	const StubBase* stub = qg_instance_stub;
	qn_debug_assert(monitor_ctn_is_have(&stub->monitors), "아니 왜 모니터가 없어");
	const uint index = stub->display < monitor_ctn_count(&stub->monitors) ? stub->display : 0;
	return monitor_ctn_nth(&stub->monitors, index);
}

//
const QgUimKey* qg_get_key_info(void)
{
	return &qg_instance_stub->key;
}

//
const QgUimMouse* qg_get_mouse_info(void)
{
	return &qg_instance_stub->mouse;
}

//
QnTimer* qg_get_timer(void)
{
	return qg_instance_stub->timer;
}

//
bool qg_set_double_click_prop(const uint density, const uint interval)
{
	qn_return_when_fail(density < 50, false);
	qn_return_when_fail(interval <= 5000, false);

	QgUimMouse* m = &qg_instance_stub->mouse;
	m->lim.move = density * density;
	m->lim.tick = interval;
	return true;
}

//
void qg_set_key_state(const QikKey key, const bool down)
{
	qn_return_when_fail((size_t)key < QIK_MAX_VALUE, /*void*/);
	qg_instance_stub->key.key[key] = down;
}

//
bool qg_get_key_state(const QikKey key)
{
	qn_return_when_fail((size_t)key < QIK_MAX_VALUE, false);
	return qg_instance_stub->key.key[key];
}

//
bool qg_get_key_press(const QikKey key)
{
	qn_return_when_fail((size_t)key < QIK_MAX_VALUE, false);
	return qg_instance_stub->key.prev[key] == false && qg_instance_stub->key.key[key];
}

//
bool qg_get_key_release(const QikKey key)
{
	qn_return_when_fail((size_t)key < QIK_MAX_VALUE, false);
	return qg_instance_stub->key.prev[key] && qg_instance_stub->key.key[key] == false;
}

//
bool qg_get_mouse_button_state(const QimButton button)
{
	qn_return_when_fail((size_t)button < QIM_MAX_VALUE, false);
	return QN_TBIT(qg_instance_stub->mouse.mask, button);
}

//
bool qg_get_mouse_button_press(const QimButton button)
{
	qn_return_when_fail((size_t)button < QIM_MAX_VALUE, false);
	return QN_TBIT(qg_instance_stub->mouse.prev, button) == false && QN_TBIT(qg_instance_stub->mouse.mask, button);
}

//
bool qg_get_mouse_button_release(const QimButton button)
{
	qn_return_when_fail((size_t)button < QIM_MAX_VALUE, false);
	return QN_TBIT(qg_instance_stub->mouse.prev, button) && QN_TBIT(qg_instance_stub->mouse.mask, button) == false;
}

//
const QimMask qg_get_mouse_button_mask(void)
{
	return qg_instance_stub->mouse.mask;
}

//
void qg_get_mouse_get_position(QmPoint* pos)
{
	if (pos)
		*pos = qg_instance_stub->mouse.pt;
}

//
void qg_get_mouse_get_delta(QmPoint* delta)
{
	if (delta)
		*delta = qg_instance_stub->mouse.delta;
}

//
float qg_get_fps(void)
{
	return qg_instance_stub->timer->fps;
}

//
float qg_get_afps(void)
{
	return qg_instance_stub->timer->afps;
}

//
float qg_get_elapsed(void)
{
	return qg_instance_stub->elapsed;
}

//
float qg_get_advance(void)
{
	return qg_instance_stub->advance;
}

//
double qg_get_run_time(void)
{
	return qg_instance_stub->timer->runtime;
}

//
float qg_get_aspect(void)
{
	return qg_instance_stub->aspect;
}

//
void qg_set_fps(int fps)
{
	if (QN_TMASK(qg_instance_stub->flags, QGFLAG_VSYNC) == false)
		qn_timer_set_cut(qg_instance_stub->timer, fps);
}

//
void qg_reset_timer(void)
{
	qn_timer_reset(qg_instance_stub->timer);
}

//
void qg_set_aspect(const int width, const int height)
{
	qg_instance_stub->aspect = (float)height / (float)width;
	if (QN_TMASK(qg_instance_stub->features, QGFEATURE_ENABLE_ASPECT))
		stub_system_aspect();
}

//
const QmSize qg_get_size(void)
{
	return qg_instance_stub->client_size;
}

//
void qg_set_exit_key(QikKey key)
{
	qg_instance_stub->key_exit = key;
}

//
void qg_set_fullscreen_key(QikKey key)
{
	qg_instance_stub->key_fullscreen = key;
}

//
int qg_left_events(void)
{
	return (int)(event_lnode_count(&shed_event.queue) + event_lnode_count(&shed_event.prior));
}

//
void qg_exit_loop(void)
{
	qn_return_when_fail(qg_instance_stub, /*void*/);
	QN_SMASK(qg_instance_stub->stats, QGSST_EXIT, true);
	qg_add_signal_event(QGEV_EXIT, true);
}

//
bool qg_loop(void)
{
	StubBase* stub = qg_instance_stub;
	qn_return_when_fail(stub, false);

	shed_event.loop.reset = true;
	shed_event.loop.count++;
	shed_event_clear_reserved_mem();

	memcpy(stub->key.prev, stub->key.key, sizeof(byte) * QIK_MAX_VALUE);
	stub->mouse.delta = qm_point_zero();

	if (QN_TMASK(stub->flags, QGSPECIFIC_VIRTUAL) == false)
	{
		if (QN_TMASK(stub->features, QGFEATURE_REMOVE_EVENTS))
			shed_event_flush();	// 이전 루프의 메시지는 모두 지워버려

		if (!stub_system_poll() || QN_TMASK(stub->stats, QGSST_EXIT))
			return false;
	}

	if (QN_TMASK(stub->flags, QGFLAG_VSYNC))
		qn_timer_update(stub->timer);
	else
	{
		if (QN_TMASK(stub->stats, QGSST_ACTIVE) == false &&
			QN_TMASK(stub->features, QGFEATURE_ENABLE_IDLE))
			qn_sleep(10);
		qn_timer_update(stub->timer);
	}

	const float adv = (float)stub->timer->elapsed;
	stub->elapsed = adv;
	stub->advance = QN_TMASK(stub->stats, QGSST_PAUSE) == false ? adv : 0.0f;

	if (qg_instance_rdh)
		rdh_internal_invoke_reset();

	if (stub->key_exit != QIK_NONE && stub->key.key[stub->key_exit])
	{
		qg_exit_loop();
		return false;
	}

	return true;
}

//
bool qg_loop_dispatch(void)
{
	if (qg_loop() == false)
		return false;
	qg_dispatch();
	return true;
}

//
static void qg_poll_check_shed(void)
{
	if (shed_event.loop.reset)
	{
		shed_event.loop.reset = false;
		shed_event.loop.poll++;
	}
	if (shed_event.loop.poll != shed_event.loop.count)
	{
		qn_mesgfb("STUB", "call qg_loop() before qg_poll()! [poll(%d) != loop(%d)]",
			shed_event.loop.poll, shed_event.loop.count);
		shed_event.loop.poll = shed_event.loop.count;
	}
}

//
static void qg_dispatch_event(const QgEvent* ev)
{
	const StubBase* stub = qg_instance_stub;
	const QgEventType type = ev->ev;
	StubListEventCbNode* node, * next;
	QN_LIST_FOREACH(stub->callbacks, node, next)
	{
		if (node->DATA.func(node->DATA.data, type, ev) > 0)
			break;
	}
}

//
bool qg_poll(QgEvent* ev)
{
	const StubBase* stub = qg_instance_stub;
	qn_return_when_fail(stub && ev, false);
	qg_poll_check_shed();
	if (shed_event_transition(ev))
	{
		if (eventcb_list_is_have(&stub->callbacks))
		{
			qn_mutex_enter(stub->mutex);
			qg_dispatch_event(ev);
			qn_mutex_leave(stub->mutex);
		}
		return true;
	}
	return false;
}

//
void qg_dispatch(void)
{
	const StubBase* stub = qg_instance_stub;
	qn_return_when_fail(stub, /*void*/);
	qg_poll_check_shed();

	qn_mutex_enter(stub->mutex);
	if (eventcb_list_is_have(&stub->callbacks))
	{
		QgEvent ev;
		while (shed_event_transition(&ev))
			qg_dispatch_event(&ev);
	}
	qn_mutex_leave(stub->mutex);
}

#ifdef _QN_EMSCRIPTEN_
// EMSCRIPTEN용 메인 루프
static void qg_emscripten_main_loop()
{
	if (!qg_loop())
	{
		emscripten_cancel_main_loop();
		return;
	}
	qg_dispatch();
	const funcparam_t* fp = &qg_instance_stub->main_loop_callback;
	fp->func(fp->data);
}
#endif

//
void qg_main_loop(paramfunc_t func, void* data)
{
	qn_return_when_fail(qg_instance_stub, /*void*/);
#ifndef _QN_EMSCRIPTEN_
	while (qg_loop())
	{
		qg_dispatch();
		func(data);
	}
#else
	StubBase* stub = qg_instance_stub;
	stub->main_loop_callback.func = func;
	stub->main_loop_callback.data = data;
	emscripten_set_main_loop(qg_emscripten_main_loop, 0, true);
#endif
}

//
nint qg_register_event_callback(QgEventCallback func, void* data)
{
	qn_return_when_fail(func, 0);
	StubBase* stub = qg_instance_stub;

	const StubEventCallback cb =
	{
		.func = func,
		.data = data,
		.key = qn_p_index()
	};
	qn_mutex_enter(stub->mutex);
	eventcb_list_prepend(&stub->callbacks, cb);
	qn_mutex_leave(stub->mutex);

	return (nint)cb.key;
}

//
bool qg_unregister_event_callback(nint key)
{
	qn_return_when_fail(key != 0, false);
	StubBase* stub = qg_instance_stub;
	qn_mutex_enter(stub->mutex);

	bool ret = false;
	StubListEventCbNode* node, * next;
	QN_LIST_FOREACH(stub->callbacks, node, next)
	{
		if (node->DATA.key != (size_t)key)
			continue;
		ret = true;
		break;
	}

	if (ret)
		eventcb_list_remove_node(&stub->callbacks, node);

	qn_mutex_leave(stub->mutex);
	return ret;
}

//
void qg_flush_event(void)
{
	shed_event_flush();
}

//
bool qg_pop_event(QgEvent* ev, bool peek)
{
	qn_return_when_fail(ev, false);
	if (peek)
		return shed_event_restore(ev);
	return shed_event_transition(ev);
}

//
int qg_add_event(const QgEvent* ev, const bool prior)
{
	qn_return_when_fail(ev, -1);

	if (prior)
	{
		// 우선 순위 이벤트. 키는 이벤트 번호로
		shed_event_prior_queue(ev->ev, ev);
	}
	else
	{
		// 그냥 이벤트는 리스트로 저장
		if (shed_event_queue(ev) == false)
			return -2;
	}

	return qg_left_events();
}

//
int qg_add_signal_event(const QgEventType type, const bool prior)
{
	const QgEvent e = { .ev = type };
	return qg_add_event(&e, prior);
}

//
int qg_add_key_event(const QgEvent* ev, const size_t key)
{
	qn_return_when_fail(ev, -1);
	qn_return_when_fail(key != 0, -3);
	shed_event_prior_queue(key, ev);
	return qg_left_events();
}

//
void stub_toggle_keys(const QikMask keymask, const bool on)
{
	QgUimKey* uk = &qg_instance_stub->key;
	QN_SMASK(uk->mask, keymask, on);
}

//
bool stub_track_mouse_click(const QimButton button, const QimTrack track)
{
	QgUimMouse* m = &qg_instance_stub->mouse;

	if (track == QIMT_MOVE)
	{
		if (m->clk.tick > 0)
		{
			const uint d = qm_point_len_sq(qm_sub(m->clk.loc, m->pt));
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
			m->clk.tick = qn_tick32();
			m->clk.loc = m->pt;
			m->clk.btn = button;
		}
		else
		{
			if (m->clk.btn == button)
			{
				const uint d = qn_tick32() - m->clk.tick;
				if (d < m->lim.tick)
				{
					// 더블 클릭으로 인정
					m->clk.ltick = m->clk.tick;
					m->clk.tick = 0;
					return true;
				}
			}
			// 취소 -> 새로운 시작
			m->clk.tick = 0;
			m->clk.loc = m->pt;
			m->clk.btn = button;
		}
	}
	else if (track == QIMT_UP)
	{
		// 버튼 업은 검사 안하지
	}

	return false;
}

//
bool stub_event_on_active_monitor(QgUdevMonitor* monitor)
{
	if (qg_instance_stub->display == (uint)monitor->no)
		return false;
	qg_instance_stub->display = monitor->no;
	QgUdevMonitor* another = qn_memdup(monitor, sizeof(QgUdevMonitor));
	const QgEvent e = {
		.monitor.ev = QGEV_MONITOR,
		.monitor.state = 3,
		.monitor.monitor = another
	};
	shed_event_reserved_mem(another);
	return qg_add_event(&e, true) > 0;
}

// monitor는 할당해서 와야한다!!!
bool stub_event_on_monitor(QgUdevMonitor* monitor, bool connected, bool primary, bool broadcast)
{
	StubBase* stub = qg_instance_stub;
	if (connected)
	{
		if (primary)
			monitor_ctn_ins(&stub->monitors, 0, monitor);
		else
			monitor_ctn_add(&stub->monitors, monitor);
	}
	else
	{
		size_t nth = monitor_ctn_find(&stub->monitors, monitor);
		if (nth != (size_t)-1)
			monitor_ctn_remove_nth(&stub->monitors, nth);
	}

	// 모니터 번호 재할당
	size_t i;
	QN_CTNR_FOREACH(stub->monitors, 0, i)
		monitor_ctn_nth(&stub->monitors, i)->no = (int)i;

	// 이벤트
	bool ret = true;
	if (broadcast)
	{
		QgUdevMonitor* another = qn_memdup(monitor, sizeof(QgUdevMonitor));
		const QgEvent e = {
			.monitor.ev = QGEV_MONITOR,
			.monitor.state = connected ? 1 : 0,
			.monitor.monitor = another
		};
		shed_event_reserved_mem(another);
		ret = qg_add_event(&e, false) > 0;
	}

	// 연결 끊긴 모니터는.. 뭐 그렇지
	if (connected == false)
	{
		qn_free(monitor);
		stub_system_update_bound();		// 위치가 바꼈을 수도 있다! 보통은 윈도우 메시지가 먼저 옴
	}
	return ret;
}

//
bool stub_event_on_layout(bool enter)
{
	StubBase* stub = qg_instance_stub;

	if (enter != false)
	{
		// 레이아웃 시작 메시지 안보냄
		QN_SMASK(stub->stats, QGSST_LAYOUT, true);
		return false;
	}
	QN_SMASK(stub->stats, QGSST_LAYOUT, false);

	QmSize prev = stub->client_size;
	stub_system_update_bound();
	if (qm_eq(prev, stub->client_size) == false)
	{
		// 크기가 변하면 레이아웃
		if (qg_instance_rdh)
			rdh_internal_check_layout();
		const QgEvent e =
		{
			.layout.ev = QGEV_LAYOUT,
			.layout.bound = stub->bound,
			.layout.size = stub->client_size,
		};
		return qg_add_event(&e, true) > 0;
	}

	return false;
}

//
bool stub_event_on_focus(bool enter)
{
	StubBase* stub = qg_instance_stub;
	qn_return_when_fail(QN_TMASK(stub->flags, QGFLAG_FOCUS), 0);

	const bool prev = QN_TMASK(stub->stats, QGSST_FOCUS);
	if (prev != enter)
	{
		QN_SMASK(stub->stats, QGSST_FOCUS, enter);
		return qg_add_signal_event(enter ? QGEV_ENTER : QGEV_LEAVE, false);
	}

	return false;
}

//
bool stub_event_on_window_event(const QgWindowEventType type, const int param1, const int param2)
{
	StubBase* stub = qg_instance_stub;

	switch (type)  // NOLINT
	{
		case QGWEV_SHOW:
			if (QN_TMASK(stub->stats, QGSST_SHOW))
				return false;
			QN_SMASK(stub->stats, QGSST_SHOW, true);
			QN_SMASK(stub->stats, QGSST_MINIMIZE, false);
			if (QN_TMASK(stub->flags, QGFLAG_FULLSCREEN))
			{
				// 풀스크린으로
			}
			break;

		case QGWEV_HIDE:
			if (QN_TMASK(stub->stats, QGSST_SHOW) == false)
				return false;
			QN_SMASK(stub->stats, QGSST_SHOW, false);
			if (QN_TMASK(stub->flags, QGFLAG_FULLSCREEN))
			{
				// 엥 풀스크린이었는데...
			}
			break;

		case QGWEV_PAINTED:
			break;

		case QGWEV_RESTORED:
			if (QN_TMASK(stub->stats, QGSST_SHOW) && QN_TMASK(stub->stats, QGSST_MAXIMIZE | QGSST_MINIMIZE) == false)
				return false;
			QN_SMASK(stub->stats, QGSST_MINIMIZE, false);
			QN_SMASK(stub->stats, QGSST_MAXIMIZE, false);
			break;

		case QGWEV_MAXIMIZED:
			if (QN_TMASK(stub->stats, QGSST_MAXIMIZE))
				return false;
			QN_SMASK(stub->stats, QGSST_MAXIMIZE, true);
			QN_SMASK(stub->stats, QGSST_MINIMIZE, false);
			break;

		case QGWEV_MINIMIZED:
			if (QN_TMASK(stub->stats, QGSST_MINIMIZE))
				return false;
			QN_SMASK(stub->stats, QGSST_MAXIMIZE, false);
			QN_SMASK(stub->stats, QGSST_MINIMIZE, true);
			break;

		case QGWEV_MOVED:
			if (QN_TMASK(stub->stats, QGSST_FULLSCREEN))
				return false;
			if (stub->bound.Left == param1 && stub->bound.Top == param2)
				return false;
			stub->bound = qm_rect_move(stub->bound, param1, param2);
			if (monitor_ctn_count(&stub->monitors) > 1)
			{
				size_t i;
				QN_CTNR_FOREACH(stub->monitors, 0, i)
				{
					const QgUdevMonitor* mon = monitor_ctn_nth(&stub->monitors, i);
					const QmRect bound = qm_rect_size((int)mon->x, (int)mon->y, (int)mon->width, (int)mon->height);
					if (qm_rect_include(bound, stub->bound))
						break;
				}
				if (i < monitor_ctn_count(&stub->monitors))
					stub_event_on_active_monitor(monitor_ctn_nth(&stub->monitors, i));
			}
			break;

		case QGWEV_SIZED:
			if (qm_rect_get_width(stub->bound) == param1 && qm_rect_get_height(stub->bound) == param2)
				return 0;
			stub->bound = qm_rect_set_size(stub->bound, param1, param2);
			break;

		case QGWEV_FOCUS:
			QN_SMASK(stub->stats, QGSST_FOCUS, true);
			break;

		case QGWEV_LOSTFOCUS:
			QN_SMASK(stub->stats, QGSST_FOCUS, false);
			break;

		case QGWEV_CLOSE:
			qg_add_signal_event(QGEV_EXIT, true);
			break;

		default:
			break;
	}

	const QgEvent e =
	{
		.wevent.ev = QGEV_WINDOW,
		.wevent.mesg = type,
		.wevent.param1 = param1,
		.wevent.param2 = param2,
	};
	return qg_add_key_event(&e, (QGEV_WINDOW << 16) | (ushort)type) > 0;
}

//
bool stub_event_on_text(const char* text)
{
	const StubBase* stub = qg_instance_stub;
	qn_return_when_fail(QN_TMASK(stub->flags, QGFLAG_TEXT), 0);

	QgEvent e = { .text.ev = QGEV_TEXTINPUT, };
	bool ret = false;
	const size_t len = qn_u8len(text);
	size_t pos = 0;
	while (pos < len)
	{
		const size_t add = qn_u8lcpy(e.text.data, text + pos, QN_COUNTOF(e.text.data));
		if (add == 0)
			break;
		pos += add;
		e.text.len = (int)add;
		ret = qg_add_event(&e, false) > 0;
	}
	return ret;
}

//
bool stub_event_on_keyboard(const QikKey key, const bool down)
{
	QgUimKey* uk = &qg_instance_stub->key;
	QgEvent e;

	QikMask mask;
	switch (key)	// NOLINT
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
		e.key.repeat = qg_get_key_state(key);
		qg_set_key_state(key, true);

		switch (key)	// NOLINT
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
		qg_set_key_state(key, false);
		uk->mask &= ~mask;
	}
	e.key.key = key;
	e.key.mask = uk->mask;

	return qg_add_event(&e, false) > 0;
}

//
bool stub_event_on_reset_keys(void)
{
	QgEvent e = { .key.ev = QGEV_KEYUP, .key.mask = 0, .key.repeat = false };
	QgUimKey* uk = &qg_instance_stub->key;

	bool ret = false;
	for (int i = 0; i < QIK_MAX_VALUE; i++)
	{
		if (uk->key[i] == false)
			continue;
		uk->key[i] = false;
		e.key.key = (QikKey)i;
		if (qg_add_event(&e, false) <= 0)
			ret = false;
	}
	return ret;
}

//
bool stub_event_on_mouse_move(int x, int y)
{
	// 이동 이외의 정보는 해와야함
	QgUimMouse* um = &qg_instance_stub->mouse;

	um->last = um->pt;
	um->delta.X = um->pt.X - x;
	um->delta.Y = um->pt.Y - y;
	um->pt = qm_point(x, y);

	if (um->delta.X == 0 && um->delta.Y == 0)
		return false;

	const QgEvent e =
	{
		.mmove.ev = QGEV_MOUSEMOVE,
		.mmove.mask = um->mask,
		.mmove.pt = um->pt,
		.mmove.delta.X = um->last.X - x,
		.mmove.delta.Y = um->last.Y - y,
	};
	return qg_add_event(&e, false) > 0;
}

//
bool stub_event_on_mouse_button(const QimButton button, const bool down)
{
	QgUimMouse* um = &qg_instance_stub->mouse;
	QN_SBIT(um->mask, button, down);

	const QgEvent e =
	{
		.mbutton.ev = down ? QGEV_MOUSEDOWN : QGEV_MOUSEUP,
		.mbutton.pt = um->pt,
		.mbutton.button = button,
		.mbutton.mask = um->mask,
	};
	bool ret = qg_add_event(&e, false) > 0;

	if (stub_track_mouse_click(button, down ? QIMT_DOWN : QIMT_UP))
	{
		const QgEvent de =
		{
			.mbutton.ev = QGEV_MOUSEDOUBLE,
			.mbutton.pt = um->pt,
			.mbutton.button = button,
			.mbutton.mask = um->mask,
		};
		ret = qg_add_event(&de, false) > 0;
	}

	return ret;
}

//
bool stub_event_on_mouse_wheel(const float x, const float y, const bool direction)
{
	QgUimMouse* um = &qg_instance_stub->mouse;

	if (qm_eqf(x, 0.0f) && qm_eqf(y, 0.0f))
		return 0;

	if (x > 0.0f)
	{
		if (um->wheel.accm.X < 0.0f)
			um->wheel.accm.X = 0.0f;
	}
	else if (x < 0.0f)
	{
		if (um->wheel.accm.X > 0.0f)
			um->wheel.accm.X = 0.0f;
	}
	um->wheel.accm.X += x;
	const int ix =
		(um->wheel.accm.X > 0.0f) ? (int)floorf(um->wheel.accm.X) :		// NOLINT
		(um->wheel.accm.X < 0.0f) ? (int)ceilf(um->wheel.accm.X) : 0;	// NOLINT
	um->wheel.accm.X -= (float)ix;

	if (y > 0.0f)
	{
		if (um->wheel.accm.Y < 0.0f)
			um->wheel.accm.Y = 0.0f;
	}
	else if (y < 0.0f)
	{
		if (um->wheel.accm.Y > 0.0f)
			um->wheel.accm.Y = 0.0f;
	}
	um->wheel.accm.Y += y;
	const int iy =
		(um->wheel.accm.Y > 0.0f) ? (int)floorf(um->wheel.accm.Y) :		// NOLINT
		(um->wheel.accm.Y < 0.0f) ? (int)ceilf(um->wheel.accm.Y) : 0;	// NOLINT
	um->wheel.accm.Y -= (float)iy;

	um->wheel.integral = qm_point(ix, iy);
	um->wheel.precise = qm_vec2(x, y);

	const QgEvent e =
	{
		.mwheel.ev = QGEV_MOUSEWHEEL,
		.mwheel.pt = um->pt,
		.mwheel.wheel = um->wheel.integral,
		.mwheel.precise = um->wheel.precise,
		.mwheel.direction = direction,
	};
	return qg_add_event(&e, false) > 0;
}

//
bool stub_event_on_active(const bool active, const double delta)
{
	const QgEvent e =
	{
		.active.ev = QGEV_ACTIVE,
		.active.active = active,
		.active.delta = delta,
	};
	return qg_add_event(&e, true) > 0;
}

//
bool stub_event_on_drop(char* data, const int len, const bool finish)
{
	static bool s_enter = false;

	if (data != NULL)
		shed_event_reserved_mem(data);

	if (s_enter == false)
	{
		s_enter = true;
		QN_SMASK(qg_instance_stub->stats, QGSST_DROP, true);
		const QgEvent eb = { .drop.ev = QGEV_DROPBEGIN, };
		if (qg_add_event(&eb, false) == 0)
			return false;
	}

	const QgEvent e =
	{
		.drop.ev = finish ? QGEV_DROPEND : QGEV_DROPFILE,
		.drop.len = len,
		.drop.data = data,
	};
	const bool ret = qg_add_event(&e, false) > 0;

	if (finish)
	{
		QN_SMASK(qg_instance_stub->stats, QGSST_DROP, false);
		s_enter = false;
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"RDH"

//
bool qg_open_rdh(const char* driver, const char* title, int display, int width, int height, int flags, int features)
{
	qg_init_converters();

	struct rdh_renderer
	{
		const char* name;
		const char* alias;
		RdhBase* (*allocator)(QgFlag, QgFeature);
		QgFlag flag;
		QgFeature feature;
	};
	static struct rdh_renderer renderers[] =
	{
#if defined USE_GL
		{ "GL", "OPENGL", qgl_allocator, QGSPECIFIC_CORE, QGRENDERER_OPENGL },
		{ "ES", "OPENGLES", qgl_allocator, QGFLAG_NONE, QGRENDERER_OPENGL },
#endif
#if defined USE_GL
		{ NULL, NULL, qgl_allocator, QGFLAG_NONE, QGRENDERER_OPENGL },
#else
		{ NULL, NULL, NULL, 0 },
#endif
	};
	struct rdh_renderer* renderer = &renderers[QN_COUNTOF(renderers) - 1];
	if (driver != NULL && *driver != '\0')
	{
		for (size_t i = 0; i < QN_COUNTOF(renderers) - 1; i++)
			if (qn_strieqv(renderers[i].name, driver) || qn_strieqv(renderers[i].alias, driver))
			{
				renderer = &renderers[i];
				break;
			}
	}

	VAR_CHK_IF_COND(renderer->allocator == NULL, "no valid renderer found", false);
	flags |= renderer->flag;
	features |= renderer->feature;

#ifdef _QN_EMSCRIPTEN_
	// emscripten의 sleep은 믿을 수가 없다
	flags |= QGFLAG_VSYNC;
#endif

	bool stub_created;
	if (STUB != NULL)
		stub_created = false;
	else
	{
		if (qg_open_stub(title, display, width, height, flags | QGSPECIFIC_RDHSTUB, features) == false)
			return false;
		stub_created = true;
	}

	// 개별 디바이스
	RdhBase* rdh = renderer->allocator(flags, features);
	if (rdh == NULL)
	{
		if (stub_created)
			qg_close_stub();
		return false;
	}
	RDH = rdh;

	// 한번 설정하면 바뀔일이 없거나 신경 안써도 되는것
	RendererTransform* tm = &rdh->tm;
	tm->world.s = qm_mat4_unit();
	tm->view.s = qm_mat4_unit();
	tm->invv.s = qm_mat4_unit();
	tm->frm.s = qm_mat4_unit();
	for (size_t i = 0; i < QN_COUNTOF(tm->tex); i++)		// 텍스쳐 행렬
		tm->tex[i].s = qm_mat4_unit();
	tm->Near = 1.0f;										// Z깊이
	tm->Far = 100000.0f;

	RendererParam* param = &rdh->param;
	param->bgc.s = QMCOLOR_BLACK.s;							// 배경색

	for (size_t i = 0; i < QN_COUNTOF(param->v); i++)		// 벡터 인수
		param->v[i].s = qm_vec_zero();
	for (size_t i = 0; i < QN_COUNTOF(param->m); i++)		// 행렬 인수
		param->m[i].s = qm_mat4_unit();

	param->diffuse.s = QMCOLOR_WHITE.s;						// 분산광
	param->specular.s = QMCOLOR_WHITE.s;					// 반사광
	param->ambient.s = QMCOLOR_SILVER.s;					// 주변광
	param->emissive.s = QMCOLOR_BLACK.s;					// 방사광
	param->shininess = 4.0f;								// 빛나는 정도
	param->constant_dir.s = qm_vec_zero();					// 항성 방향 벡터
	param->constant_color.s = QMCOLOR_LIGHT_CYAN.s;			// 항성 색깔

	// 묶음
	for (size_t i = 0; i < QN_COUNTOF(rdh->mukums); i++)
		qn_node_mukum_init_fast(&rdh->mukums[i]);

	//
	qn_cast_vtable(rdh, RDH)->layout();					// 레이아웃 재설정
	qn_cast_vtable(rdh, RDH)->reset();					// 장치 리셋
	qn_timer_reset(STUB->timer);							// 타이머도 리셋해둔다
	return true;
}

//
void qg_close_rdh(void)
{
	RDH = qn_unloadc(RDH);
}

//
void rdh_internal_clean(void)
{
	RdhBase* rdh = RDH;
	for (size_t i = 0; i < QN_COUNTOF(rdh->mukums); i++)
		qn_node_mukum_safe_dispose(&rdh->mukums[i]);
	qn_unload(rdh->font);
}

//
void rdh_internal_dispose(void)
{
	VAR_CHK_IF_COND(STUB == NULL, "스터브가 없어요", );
	VAR_CHK_IF_COND(RDH == NULL, "렌더러가 없어요", );

	RdhBase* rdh = RDH;
	qn_free(rdh);

	if (QN_TMASK(STUB->flags, QGSPECIFIC_RDHSTUB))
		qg_close_stub();
}

//
void rdh_internal_layout(void)
{
	const QmSize client_size = STUB->client_size;
	const float aspect = qm_size_get_aspect(client_size);

	// tm
	RendererTransform* tm = RDH_TRANSFORM;
	tm->size = client_size;
	tm->proj.s = qm_mat4_perspective_lh(QM_PI_H, aspect, tm->Near, tm->Far);
	tm->scissor = qm_rect_size(0, 0, client_size.Width, client_size.Height);
}

//
void rdh_internal_reset(void)
{
	// param
	RendererParam* param = RDH_PARAM;
	param->bone_ptr = NULL;									// 뼈대 포인터는 메모리 안전을 위해 초기화
	param->bone_count = 0;									// 뼈대 가중치 개수도 초기화

	RdhBase* rdh = RDH;
	rdh->font = _create_default_font();
}

//
void rdh_internal_invoke_reset(void)
{
	RdhBase* rdh = RDH;

	RendererInvoke* ivk = &rdh->invokes;
	ivk->invokes = 0;
	ivk->begins = 0;
	ivk->ends = 0;
	ivk->renders = 0;
	ivk->params = 0;
	ivk->vars = 0;
	ivk->transforms = 0;
	ivk->draws = 0;
	ivk->primitives = 0;
	ivk->flush = false;

	RendererParam* param = &rdh->param;
	QnDateTime dt = { .stamp = qn_qtc() };
	float s, c, hs = (float)(dt.hour * 3600 + dt.minute * 60 + dt.second) / 86400.0f;
	qm_sincosf(hs * QM_TAU, &s, &c);
	param->constant_dir.s = qm_vec3(-s, c, 0.0);
}

//
void rdh_internal_check_layout(void)
{
	RdhBase* rdh = RDH;
	const QmSize size = STUB->client_size;
	const int width = (int)rdh->tm.size.Width;
	const int height = (int)rdh->tm.size.Height;
	if (size.Width == width && size.Height == height)
		return;
	qn_cast_vtable(rdh, RDH)->layout();
}

//
void rdh_internal_add_node(RenderNodeShed shed, void* node)
{
	VAR_CHK_IF_MAX(shed, RDHNODE_MAX_VALUE, );
	RdhBase* rdh = RDH;
	qn_node_mukum_set(&rdh->mukums[shed], qn_loadu(node, QnNodeGam));
}

//
void rdh_internal_unlink_node(RenderNodeShed shed, void* node)
{
	VAR_CHK_IF_MAX(shed, RDHNODE_MAX_VALUE, );
	RdhBase* rdh = RDH;
	qn_node_mukum_unlink(&rdh->mukums[shed], qn_cast_type(node, QnNodeGam));
}

//
void qg_set_shader_var_callback(QgVarShaderFunc func, void* data)
{
	RendererParam* param = RDH_PARAM;
	param->callback_func = func;
	param->callback_data = data;
}

//
bool qg_begin_render(bool clear)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.begins++;
	rdh->invokes.flush = false;
	RendererTransform* tm = &rdh->tm;
	tm->vwpr.s = qm_mat4_mul(tm->view.s, tm->proj.s);
	tm->prvw.s = qm_mat4_mul(tm->proj.s, tm->view.s);
	return qn_cast_vtable(rdh, RDH)->begin(clear);
}

//
void qg_end_render(bool flush)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.ends++;
	rdh->invokes.flush = true;
	qn_cast_vtable(rdh, RDH)->end();
	if (flush)
		qn_cast_vtable(rdh, RDH)->flush();
}

//
void qg_flush(void)
{
	RdhBase* rdh = RDH;
	if (!rdh->invokes.flush)
	{
		qn_mesgb("RDH", "call end() before flush");
		qg_end_render(false);
	}
	qn_cast_vtable(rdh, RDH)->flush();
	rdh->invokes.invokes++;
	rdh->invokes.frames++;
}

//
void qg_rdh_reset(void)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	qn_cast_vtable(rdh, RDH)->reset();
}

//
void qg_clear_render(QgClear clear)
{
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	qn_cast_vtable(rdh, RDH)->clear(clear);
}

//
void qg_set_diffuse(const QMVEC* diffuse)
{
	RdhBase* rdh = RDH;
	rdh->param.diffuse.s = *diffuse;
	rdh->invokes.invokes++;
}

//
void qg_set_specular(const QMVEC* specular)
{
	RdhBase* rdh = RDH;
	rdh->param.specular.s = *specular;
	rdh->invokes.invokes++;
}

//
void qg_set_ambient(const QMVEC* ambient)
{
	RdhBase* rdh = RDH;
	rdh->param.ambient.s = *ambient;
	rdh->invokes.invokes++;
}

//
void qg_set_emissive(const QMVEC* emissive)
{
	RdhBase* rdh = RDH;
	rdh->param.emissive.s = *emissive;
	rdh->invokes.invokes++;
}

//
void qg_set_constant_dir(const QMVEC* dir)
{
	RdhBase* rdh = RDH;
	if (dir != NULL)
		rdh->param.constant_dir.s = *dir;
	rdh->invokes.invokes++;
}

//
void qg_set_constant_color(const QMVEC* color)
{
	RdhBase* rdh = RDH;
	rdh->param.constant_color.s = color != NULL ? *color : QMCOLOR_WHITE.s;
	rdh->invokes.invokes++;
}

//
const QMVEC qg_get_constant_dir(void)
{
	return RDH_PARAM->constant_dir.s;
}

//
void qg_set_param_vec4(int at, const QMVEC* v)
{
	RdhBase* rdh = RDH;
	VAR_CHK_IF_MAX(at, QN_COUNTOF(rdh->param.v), );
	rdh->param.v[at].s = *v;
	rdh->invokes.invokes++;
}

//
void qg_set_param_mat4(int at, const QMMAT* m)
{
	RdhBase* rdh = RDH;
	VAR_CHK_IF_MAX(at, QN_COUNTOF(rdh->param.m), );
	rdh->param.m[at].s = *m;
	rdh->invokes.invokes++;
}

//
void qg_set_param_weight(int count, QMMAT* weight)
{
	VAR_CHK_IF_MIN(count, 0, );
	VAR_CHK_IF_NULL(weight, );
	RdhBase* rdh = RDH;
	rdh->param.bone_count = count;
	rdh->param.bone_ptr = (QmMat4*)weight;
	rdh->invokes.invokes++;
}

//
void qg_set_background(const QMVEC* color)
{
	RdhBase* rdh = RDH;
	rdh->param.bgc.s = *color;
	rdh->invokes.invokes++;
}

//
void qg_set_background_param(float r, float g, float b, float a)
{
	RdhBase* rdh = RDH;
	rdh->param.bgc.s = qm_vec4(r, g, b, a);
	rdh->invokes.invokes++;
}

//
void qg_set_world(const QMMAT* world)
{
	RdhBase* rdh = RDH;
	rdh->tm.world.s = *world;
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_set_view(const QMMAT* view)
{
	RdhBase* rdh = RDH;
	rdh->tm.view.s = *view;
	rdh->tm.invv.s = qm_mat4_inv(*view);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_set_project(const QMMAT* proj)
{
	RdhBase* rdh = RDH;
	rdh->tm.proj.s = *proj;
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_set_view_project(const QMMAT* proj, const QMMAT* view)
{
	RdhBase* rdh = RDH;
	rdh->tm.proj.s = *proj;
	rdh->tm.view.s = *view;
	rdh->tm.invv.s = qm_mat4_inv(*view);
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
void qg_set_camera(QgCamera* camera)
{
	VAR_CHK_IF_NULL(camera, );
	RdhBase* rdh = RDH;
	rdh->tm.proj.s = camera->mat.proj;
	rdh->tm.view.s = camera->mat.view;
	rdh->tm.invv.s = camera->mat.invv;
	rdh->invokes.invokes++;
	rdh->invokes.transforms++;
}

//
QgBuffer* qg_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	VAR_CHK_IF_ZERO(count, NULL);
	VAR_CHK_IF_MIN(stride, 1, NULL);
	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDH)->create_buffer(type, count, stride, initial_data);
}

//
QgRenderState* qg_create_render_state(const char* name, const QgPropRender* render, const QgLayoutData* layout, const QgPropShader* shader)
{
	VAR_CHK_IF_NULL(render, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, fill, QGFILL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, cull, QGCULL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, depth, QGDEPTH_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, stencil, QGSTENCIL_MAX_VALUE, NULL);
	VAR_CHK_IF_ZERO3(render, format, count, NULL);
	VAR_CHK_IF_MAX3(render, format, count, QGRVS_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, topology, QGTPG_MAX_VALUE, NULL);

	VAR_CHK_IF_NULL(layout, NULL);
	VAR_CHK_IF_NULL2(layout, inputs, NULL);
	VAR_CHK_IF_ZERO2(layout, count, NULL);

	VAR_CHK_IF_NULL(shader, NULL);
	VAR_CHK_IF_NULL3(shader, vertex, code, NULL);
	VAR_CHK_IF_NULL3(shader, pixel, code, NULL);

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDH)->create_render(name, render, layout, shader);
}

//
QgRenderState* qg_create_render_state_vsps(const char* name, const QgPropRender* render, const QgLayoutData* layout, int mount, const char* vsfile, const char* psfile)
{
	VAR_CHK_IF_NULL(render, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, fill, QGFILL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, cull, QGCULL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, depth, QGDEPTH_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, stencil, QGSTENCIL_MAX_VALUE, NULL);
	VAR_CHK_IF_ZERO3(render, format, count, NULL);
	VAR_CHK_IF_MAX3(render, format, count, QGRVS_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, topology, QGTPG_MAX_VALUE, NULL);

	VAR_CHK_IF_NULL(layout, NULL);
	VAR_CHK_IF_NULL2(layout, inputs, NULL);
	VAR_CHK_IF_ZERO2(layout, count, NULL);

	int size;
	QgPropShader shader;
	shader.vertex.code = qn_file_alloc(qg_get_mount(mount), vsfile, &size);
	shader.vertex.size = (size_t)size;
	shader.pixel.code = qn_file_alloc(qg_get_mount(mount), psfile, &size);
	shader.pixel.size = (size_t)size;
	if (shader.vertex.code == NULL || shader.pixel.code == NULL)
	{
		qn_free(shader.vertex.code);
		qn_free(shader.pixel.code);
		return NULL;
	}
	((char*)shader.vertex.code)[shader.vertex.size] = '\0';
	((char*)shader.pixel.code)[shader.pixel.size] = '\0';

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	QgRenderState* rs = qn_cast_vtable(rdh, RDH)->create_render(name, render, layout, &shader);
	qn_free(shader.vertex.code);
	qn_free(shader.pixel.code);
	return rs;
}

//
QgRenderState* qg_create_render_state_decl(const char* name, const QgPropRender* render, QgLayoutDecl layout, const QgPropShader* shader)
{
	VAR_CHK_IF_NULL(render, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, fill, QGFILL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, cull, QGCULL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, depth, QGDEPTH_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, stencil, QGSTENCIL_MAX_VALUE, NULL);
	VAR_CHK_IF_ZERO3(render, format, count, NULL);
	VAR_CHK_IF_MAX3(render, format, count, QGRVS_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, topology, QGTPG_MAX_VALUE, NULL);

	VAR_CHK_IF_MAX(layout, QGLAYOUT_MAX_VALUE, NULL);

	VAR_CHK_IF_NULL(shader, NULL);
	VAR_CHK_IF_NULL3(shader, vertex, code, NULL);
	VAR_CHK_IF_NULL3(shader, pixel, code, NULL);

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	const QgLayoutData* layout_data = qg_get_layout_data(layout);
	return qn_cast_vtable(rdh, RDH)->create_render(name, render, layout_data, shader);
}

//
QgRenderState* qg_create_render_state_decl_vsps(const char* name, const QgPropRender* render, QgLayoutDecl layout, int mount, const char* vsfile, const char* psfile)
{
	VAR_CHK_IF_NULL(render, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, fill, QGFILL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX3(render, rasterizer, cull, QGCULL_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, depth, QGDEPTH_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, stencil, QGSTENCIL_MAX_VALUE, NULL);
	VAR_CHK_IF_ZERO3(render, format, count, NULL);
	VAR_CHK_IF_MAX3(render, format, count, QGRVS_MAX_VALUE, NULL);
	VAR_CHK_IF_MAX2(render, topology, QGTPG_MAX_VALUE, NULL);

	VAR_CHK_IF_MAX(layout, QGLAYOUT_MAX_VALUE, NULL);

	int size;
	QgPropShader shader;
	shader.vertex.code = qn_file_alloc(qg_get_mount(mount), vsfile, &size);
	shader.vertex.size = (size_t)size;
	shader.pixel.code = qn_file_alloc(qg_get_mount(mount), psfile, &size);
	shader.pixel.size = (size_t)size;
	if (shader.vertex.code == NULL || shader.pixel.code == NULL)
	{
		qn_free(shader.vertex.code);
		qn_free(shader.pixel.code);
		return NULL;
	}
	((char*)shader.vertex.code)[shader.vertex.size] = '\0';
	((char*)shader.pixel.code)[shader.pixel.size] = '\0';

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	const QgLayoutData* layout_data = qg_get_layout_data(layout);
	QgRenderState* rs = qn_cast_vtable(rdh, RDH)->create_render(name, render, layout_data, &shader);
	qn_free(shader.vertex.code);
	qn_free(shader.pixel.code);
	return rs;
}

//
QgTexture* qg_create_texture(const char* name, const QgImage* image, QgTexFlag flags)
{
	VAR_CHK_IF_NULL(image, NULL);

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDH)->create_texture(name, image, flags);
}

//
QgTexture* qg_load_texture(int mount, const char* filename, QgTexFlag flags)
{
	VAR_CHK_IF_NULL(filename, NULL);

	QgImage* image = qg_load_image(mount, filename);
	VAR_CHK_IF_NULL(image, NULL);

	RdhBase* rdh = RDH;
	rdh->invokes.creations++;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDH)->create_texture(filename, image, flags | QGTEXF_DISCARD_IMAGE);
}

//
bool qg_load_def_font(int mount, const char* filename)
{
	QgFont* font = qg_load_font(mount, filename, 0x1);
	qn_return_when_fail(font != NULL, false);

	qn_unload(RDH->font);
	RDH->font = font;
	return true;
}

//
QgFont* qg_get_def_font(void)
{
	return RDH->font;
}

//
int qg_get_def_font_size(void)
{
	return RDH->font != NULL ? qg_font_get_size(RDH->font) : 0;
}

//
void qg_set_def_font_size(int size)
{
	QgFont* font = RDH->font;
	if (font != NULL)
		qg_font_set_size(font, size);
}

//
bool qg_set_index(QgBuffer* buffer)
{
	VAR_CHK_IF_NULL(buffer, false);
	VAR_CHK_IF_COND(buffer->type != QGBUFFER_INDEX, "cannot set non-index buffer as index buffer", false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDH)->set_index(buffer);
}

//
bool qg_set_vertex(QgLayoutStage stage, QgBuffer* buffer)
{
	VAR_CHK_IF_NULL(buffer, false);
	VAR_CHK_IF_COND(buffer->type != QGBUFFER_VERTEX, "cannot set non-vertex buffer as vertex buffer", false);
	VAR_CHK_IF_MAX(stage, QGLOS_MAX_VALUE, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDH)->set_vertex(stage, buffer);
}

//
bool qg_set_render_state(QgRenderState* render)
{
	VAR_CHK_IF_NULL(render, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDH)->set_render(render);
}

//
bool qg_set_render_named(const char* name)
{
	VAR_CHK_IF_NULL(name, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	QgRenderState* rdr = qn_node_mukum_get(&rdh->mukums[RDHNODE_RENDER], name);
	return rdr == NULL ? false : qn_cast_vtable(rdh, RDH)->set_render(rdr);
}

//
bool qg_set_texture(int stage, QgTexture* texture)
{
	VAR_CHK_IF_MAX(stage, 8/*RDH_INFO->max_tex_count*/, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	return qn_cast_vtable(rdh, RDH)->set_texture(stage, texture);
}

//
bool qg_draw(QgTopology tpg, int vertices)
{
	VAR_CHK_IF_MAX(tpg, QGTPG_MAX_VALUE, false);
	VAR_CHK_IF_MIN(vertices, 0, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qn_cast_vtable(rdh, RDH)->draw(tpg, vertices);
}

//
bool qg_draw_indexed(QgTopology tpg, int indices)
{
	VAR_CHK_IF_MAX(tpg, QGTPG_MAX_VALUE, false);
	VAR_CHK_IF_MIN(indices, 0, false);
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	return qn_cast_vtable(rdh, RDH)->draw_indexed(tpg, indices);
}

//
void qg_draw_sprite(const QmRect* bound, QgTexture* texture, const QmKolor color, const QMVEC* coord)
{
	VAR_CHK_IF_NULL(bound, );
	if (coord == NULL)
		coord = &QMCONST_00ZW.s;
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	qn_cast_vtable(rdh, RDH)->draw_sprite(bound, texture, color, coord);
}

//
void qg_draw_sprite_ex(const QmRect* bound, float angle, QgTexture* texture, const QmKolor color, const QMVEC* coord)
{
	VAR_CHK_IF_NULL(bound, );
	if (coord == NULL)
		coord = &QMCONST_00ZW.s;
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	qn_cast_vtable(rdh, RDH)->draw_sprite_ex(bound, angle, texture, color, coord);
}

//
void qg_draw_glyph(const QmRect* bound, QgTexture* texture, const QmKolor color, const QMVEC* coord)
{
	VAR_CHK_IF_NULL(bound, );
	if (coord == NULL)
		coord = &QMCONST_00ZW.s;
	RdhBase* rdh = RDH;
	rdh->invokes.invokes++;
	rdh->invokes.draws++;
	qn_cast_vtable(rdh, RDH)->draw_glyph(bound, texture, color, coord);
}


//////////////////////////////////////////////////////////////////////////
// 버퍼

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"BUFFER"

//
void* qg_buffer_map(QgBuffer* self, bool synchronized)
{
	VAR_CHK_IF_COND(self->mapped != false, "buffer already mapped", NULL);
	return qn_cast_vtable(self, QGBUFFER)->map(self, synchronized);
}

//
bool qg_buffer_unmap(QgBuffer* self)
{
	VAR_CHK_IF_COND(self->mapped == false, "buffer not mapped", false);
	return qn_cast_vtable(self, QGBUFFER)->unmap(self);
}

//
bool qg_buffer_data(QgBuffer* self, int size, const void* data)
{
	VAR_CHK_IF_COND(self->mapped != false, "buffer already mapped", false);
	VAR_CHK_IF_NULL(data, false);
	return qn_cast_vtable(self, QGBUFFER)->data(self, size, data);
}
