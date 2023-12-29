#pragma once

#include <qs_ctn.h>

// 모니터 타입
QN_DECL_CTNR(StubMonitorCtnr, QgUdevMonitor*);

// 스터브 기본 타입
typedef struct StubBase StubBase;
struct StubBase
{
	void*				handle;								// 시스템 스터브 관리
	QnTimer*			timer;

	QgFlag				flags;
	QgStubStat			stats;								// 시스템 스터브 관리
	uint				window_stats;
	uint				display;

	uint				delay;
	float				fps;								/** @brief 프레임 당 시간 */
	float				reference;							/** @brief 프레임 시간 */
	float				advance;							/** @brief 프레임 시간, 포즈 상태일 때는 0 */
	double				run;								/** @brief 실행 시간 */
	double				active;								/** @brief 활성화된 시간 */

	QmRect				window_bound;						/// @brief 실제 윈도우의 위치와 크기 정보
	QmSize				client_size;						// 시스템 스터브 관리, 그리기 영역 크기 (창 크기가 아님)

	StubMonitorCtnr		monitors;
	QgUimKey			key;
	QgUimMouse			mouse;
};

/** @brief 스터브 인스턴스 */
extern StubBase* qg_stub_instance;

// 시스템 스터브를 연다
extern bool stub_system_open(const char* title, int display, int width, int height, QgFlag flags);
// 시스템 스터브를 정리한다
extern void stub_system_finalize(void);
// 시스템 스터브 폴링 (프로그램이 종료되면 거짓)
extern bool stub_system_poll(void);
// 시스템 드래그 켜고 끄기
extern void stub_system_enable_drop(bool enable);
// 시스템 도움꾼 켜고 끄기
extern void stub_system_disable_acs(bool enable);
// 시스템 스크린 세이버 켜고 끄기
extern void stub_system_disable_scr_save(bool enable);
// 시스템 타이틀 설정
extern void stub_system_set_title(const char* title);
// 시스템 바운드 영역을 업데이트한다
extern void stub_system_update_bound(void);
// 시스템 스터브를 포커스로 만든다
extern void stub_system_focus(void);

// 내부적으로 마우스 눌림을 연산한다
extern bool stub_track_mouse_click(QimButton button, QimTrack track);
// 모니터 이벤트 추가
extern bool stub_event_on_monitor(QgUdevMonitor* monitor, bool connected, bool primary);
// 레이아웃 이벤트 추가
extern bool stub_event_on_layout(bool enter);
// 윈도우 이벤트 추가
extern bool stub_event_on_window_event(QgWindowEventType type, int param1, int param2);
// 텍스트 이벤트 추가
extern bool stub_event_on_text(const char* text);
// 키보드 이벤트 추가
extern bool stub_event_on_keyboard(QikKey key, bool down);
// 키보드 리셋 이벤츠 추가
extern bool stub_event_on_reset_keys(void);
// 마우스 이동 이벤트 추가
extern bool stub_event_on_mouse_move(void);
// 마우스 버튼 이벤트 추가
extern bool stub_event_on_mouse_button(QimButton button, bool down);
// 마우스 휠 이벤트 추가
extern bool stub_event_on_mouse_wheel(float x, float y, bool direction);
// 액티브 이벤트 추가
extern bool stub_event_on_active(bool active, double delta);
// 드랍 이벤트 추가
extern bool stub_event_on_drop(char* data, int len, bool finish);

// 스터보 기본 사양을 초기화 stub_system_open() 함수가 호출해야 한다
extern void stub_initialize(StubBase* stub, int flags);
// 내부적으로 토글을 설정한다 (완전 언세이프)
extern void stub_toggle_keys(QikMask keymask, bool on);


// 렌더 디바이스
extern QgRdh* qg_rdh_instance;

/**
 * @brief 렌더러 최종 제거
 * @param g 렌더러
*/
extern void rdh_internal_dispose(QsGam* g);
/**
 * @brief 렌더러 내부 리셋
 * @param self 렌더러
*/
extern void rdh_internal_reset(QgRdh* self);

/**
 * @brief OPENGL ES2 할당
 * @param oshandle OS 핸들
 * @param flags 스터브 & 렌더러 플래그
 * @return 만들어진 ES2 렌더러
*/
extern QgRdh* es2_allocator(void* oshandle, int flags);

#define rdh_caps(rdh)			(qm_cast(rdh, QgRdh)->caps)
#define rdh_tm(rdh)				(qm_cast(rdh, QgRdh)->tm)
#define rdh_param(rdh)			(qm_cast(rdh, QgRdh)->param)
#define rdh_invokes(rdh)		(qm_cast(rdh, QgRdh)->invokes)

#define rdh_set_flush(rdh,v)	(qm_cast(rdh, QgRdh)->invokes.fluash=(v))
#define rdh_inc_ends(rdh)		(qm_cast(rdh, QgRdh)->invokes.ends++)


#ifdef USE_SDL2
// SDLK를 QIK로
extern QikKey sdlk_to_qik(uint32_t sdlk);
// SDL 키보드 상태 변환
extern QikMask kmod_to_qikm(int modifier);
// SDL 마우스 버튼을 QIM으로
extern QimButton sdlm_to_qim(byte button);
#endif

