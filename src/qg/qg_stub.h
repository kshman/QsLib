#pragma once

#include <qs_ctn.h>

#ifdef USE_SDL2
//////////////////////////////////////////////////////////////////////////
// SDL

// SDLK를 QIK로
extern QikKey sdlk_to_qik(uint32_t sdlk);
//
extern QikMask kmod_to_qikm(int modifier);
#endif


//////////////////////////////////////////////////////////////////////////
// 스터브

// 기본 스터브 형식
typedef struct StubBase StubBase;
struct StubBase
{
	void*				handle;								// 시스템 스터브 관리
	QnTimer*			timer;

	QgFlag				flags;
	QgStubStat			stats;								// 시스템 스터브 관리
	uint				delay;

	float				fps;								/** @brief 프레임 당 시간 */
	float				reference;							/** @brief 프레임 시간 */
	float				advance;							/** @brief 프레임 시간, 포즈 상태일 때는 0 */
	double				run;								/** @brief 실행 시간 */
	double				active;								/** @brief 활성화된 시간 */

	QnRect				window_bound;
	QnRect				bound;								// 시스템 스터브 관리
	QnSize				size;								// 시스템 스터브 관리

	QgUimKey			key;
	QgUimMouse			mouse;
};

/** @brief 스터브 인스턴스 */
extern StubBase* qg_stub_instance;

// 시스템 스터브를 연다
extern StubBase* stub_system_open(const char* title, int width, int height, int flags);
// 시스템 윈도우를 만든다
extern bool stub_system_create_window(void);
// 시스템 스터브를 정리한다
extern void stub_system_finalize(void);
// 시스템 스터브 폴링 (프로그램이 종료되면 거짓)
extern bool stub_system_poll(void);
// 시스템 드래그 켜고 끄기
extern void stub_system_enable_drop(bool enable);
// 시스템 도움꾼 켜고 끄기
extern void stub_system_disable_acs(bool enable);
// 시스템 스크린 세이버 켜고 끄기
extern void stub_system_diable_scrsave(bool enable);
// 시스템 타이틀 설정
extern void stub_system_set_title(const char* u8text);
// 마우스 홀드
extern void stub_system_hold_mouse(bool hold);
// 레이아웃을 계산한다
extern void stub_system_calc_layout(void);

// 내부적으로 마우스 눌림을 연산한다
extern bool stub_internal_mouse_clicks(QimButton button, QimTrack track);
// 레이아웃 이벤트 추가
extern int stub_internal_on_event_layout(bool enter);
// 윈도우 이벤트 추가
extern int stub_internal_on_window_event(QgWindowEventType type, int param1, int param2);
// 텍스트 이벤트 추가
extern int stub_internal_on_text(const char* text);
// 키보드 이벤트 추가
extern int stub_internal_on_keyboard(QikKey key, bool down);
// 키보드 리셋 이벤츠 추가
extern int stub_internal_on_reset_keys(void);
// 마우스 이동 이벤트 추가
extern int stub_internal_on_mouse_move(void);
// 마우스 버튼 이벤트 추가
extern int stub_internal_on_mouse_button(QimButton button, bool down);
// 마우스 휠 이벤트 추가
extern int stub_internal_on_mouse_wheel(float x, float y, bool direction);
// 액티브 이벤트 추가
extern int stub_internal_on_active(bool active, double delta);

// 내부적으로 토글을 설정한다 (완전 언세이프)
extern void stub_internal_toggle_key(QikMask keymask, bool on);


//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스
extern QgRdh* qg_rdh_instance;

/**
 * @brief 렌더러 최종 제거
 * @param g 렌더러
*/
extern void rdh_internal_dispose(QmGam* g);
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
