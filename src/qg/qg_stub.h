﻿#pragma once

#include <qs_ctn.h>

//////////////////////////////////////////////////////////////////////////
// 스터브

// 모니터 타입
QN_DECL_CTNR(StubMonitorCtnr, QgUdevMonitor*);

// 스터브 베이스
typedef struct StubBase
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

	QmRect				window_bound;						// 실제 윈도우의 위치와 크기 정보
	QmSize				client_size;						// 시스템 스터브 관리, 그리기 영역 크기 (창 크기가 아님)

#ifndef __EMSCRIPTEN__
	StubMonitorCtnr		monitors;
#endif
	QgUimKey			key;
	QgUimMouse			mouse;
} StubBase;

// 스터브 인스턴스 
extern StubBase* qg_instance_stub;

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
// 시스템 핸들을 얻는다
extern void* stub_system_get_window(void);
// 시스템 디스플레이 핸들을 얻는다 (윈도우에서는 HDC)
extern void* stub_system_get_display(void);

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
extern bool stub_event_on_mouse_move(int x, int y);
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


//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

// 렌더러 정보
typedef struct RendererInfo
{
	char				name[64];
	char				renderer[64];						// 렌더러 이름
	char				vendor[64];							// 디바이스 제조사
	int					renderer_version;					// 렌더러 버전
	int					shader_version;						// 세이더 버전
	int					max_layout_count;					// 최대 레이아웃(=정점 속성) 갯수
	int					max_indices;						// 최대 인덱스 갯수
	int					max_vertices;						// 최대 정점 갯수
	int					max_tex_dim;						// 최대 텍스쳐 크기
	int					max_tex_count;						// 최대 텍스쳐 갯수
	int					max_off_count;						// 최대 오프 텍스쳐(=렌더타겟/프레임버퍼) 갯수
	QgClrFmt			clr_fmt;							// 색깔 포맷
	bool				open_stub;
} RendererInfo;

// 렌더 추적 정보
typedef struct RenderInvoke
{
	uint				frames;
	uint				creations;

	uint				invokes;
	uint				begins;
	uint				ends;
	uint				renders;
	uint				params;
	uint				vars;
	uint				transforms;
	uint				draws;
	uint				primitives;

	bool				flush;
} RenderInvoke;

// 렌더 트랜스포메이션
typedef struct RenderTransform
{
	QmVec2				size;
	QmDepth				depth;
	QmMat4				world;
	QmMat4				view;
	QmMat4				project;
	QmMat4				view_project;
	QmMat4				inv;								// inverse view
	QmMat4				ortho;								// ortho transform
	QmMat4				frm;								// tex formation
	QmMat4				tex[4];
	QmRect				scissor;
} RenderTransform;

// 렌더 인수
typedef struct RenderParam
{
	QmVec4				v[4];
	QmMat4				m[4];
	QmMat4*				bone_ptr;
	int					bone_count;
	QmColor				bgc;
} RenderParam;

// 렌더러 디바이스
typedef struct RdhBase
{
	QsGam				base;

	RendererInfo		info;

	RenderTransform		tm;
	RenderParam			param;
	RenderInvoke		invokes;
} RdhBase;

qv_name(RdhBase)
{
	qv_name(QsGam)	base;
	void (*reset)(void);
	void (*clear)(int, const QmColor*, int, float);

	bool (*begin)(bool);
	void (*end)(void);
	void (*flush)(void);

	QgBuffer* (*create_buffer)(QgBufType, int, int, const void*);
	QgRender* (*create_render)(const QgPropRender*, bool);

	bool (*set_index)(QgBuffer*);
	bool (*set_vertex)(QgLoStage, QgBuffer*);
	void (*set_render)(QgRender*);

	bool (*draw)(QgTopology, int);
	bool (*draw_indexed)(QgTopology, int);
	bool (*ptr_draw)(QgTopology, int, int, const void*);
	bool (*ptr_draw_indexed)(QgTopology, int, int, const void*, int, int, const void*);
};

// 렌더 디바이스
extern RdhBase* qg_instance_rdh;

#ifdef USE_ES
// OPENGL ES 할당
extern RdhBase* es_allocator(QgFlag flags);
#endif

// 렌더러 최종 제거
extern void rdh_internal_dispose(void);
// 렌더러 내부 리셋
extern void rdh_internal_reset(void);
// 인보크 리셋
extern void rdh_internal_invoke_reset(void);
// 레이아웃 검사
extern void rdh_internal_check_layout(void);

#define rdh_info()			(qs_cast(qg_instance_rdh, RdhBase)->info)
#define rdh_transform()		(qs_cast(qg_instance_rdh, RdhBase)->tm)
#define rdh_param()			(qs_cast(qg_instance_rdh, RdhBase)->param)
#define rdh_invokes()		(qs_cast(qg_instance_rdh, RdhBase)->invokes)

#define rdh_set_flush(v)	(qs_cast(qg_instance_rdh, RdhBase)->invokes.flush=(v))
#define rdh_inc_ends()		(qs_cast(qg_instance_rdh, RdhBase)->invokes.ends++)

// 색깔 변환
extern QgClrFmt qg_clrfmt_from_size(int red, int green, int blue, int alpha, bool is_float);
