#pragma once

#include <qs_ctn.h>

//////////////////////////////////////////////////////////////////////////
// SDL
#if USE_SDL2
/**
 * @brief SDLK를 QIK로
 * @param sdlk SDLK
 * @return 변환한 QIK
*/
extern QikKey sdlk_to_qik(uint32_t sdlk);
/**
 * @brief 
 * @param modifier 
 * @return 
*/
extern QikMask kmod_to_qikm(int modifier);
#endif


//////////////////////////////////////////////////////////////////////////
// 스터브
QN_DECL_LIST(QgListEvent, QgEvent);

/** @brief 기본 스터브 형식 */
typedef struct StubBase StubBase;
struct StubBase
{
	void*				handle;
	QnTimer*			timer;

	QgFlag				flags;
	QgStubStat			stats;
	uint				delay;

	float				fps;								/** @brief 프레임 당 시간 */
	double				run;								/** @brief 실행 시간 */
	double				active;								/** @brief 활성화된 시간 */

	float				reference;							/** @brief 프레임 시간 */
	float				advance;							/** @brief 프레임 시간, 포즈 상태일 때는 0 */

	QnPoint				pos;
	QnSize				size;
	QnRect				bound;

	QgUimKey			key;
	QgUimMouse			mouse;

	QgListEvent			events;

	QN_PADDING_32(4, 0)
};

/** @brief 스터브 인스턴스 */
extern StubBase* qg_stub_instance;

/**
 * @brief 시스템 스터브를 연다
 * @param title 윈도우 제목
 * @param width 윈도우 너비
 * @param height 윈도우 높이
 * @param flags 스터브 플래그
 * @return 만들어진 스터브
*/
extern StubBase* stub_system_open(const char* title, int width, int height, int flags);
/**
 * @brief 시스템 스터브를 정리한다
*/
extern void stub_system_finalize(void);
/**
 * @brief 시스템 스터브 폴링
 * @return 프로그램이 종료되면 거짓
*/
extern bool stub_system_poll(void);

/**
 * @brief 내부적으로 마우스 눌림을 연산한다
 * @param button 눌린 마우스
 * @param track 추적할 방식
 * @return 더블 클릭이면 참, 한번 클릭이면 거짓
*/
extern bool stub_internal_mouse_clicks(QimButton button, QimTrack track);


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
