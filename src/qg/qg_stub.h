#pragma once

#include <qs_ctn.h>

//////////////////////////////////////////////////////////////////////////
// 스터브

// 모니터 타입
QN_DECL_CTNR(StubMonitorCtnr, QgUdevMonitor*);

// 이벤트 핸들러 페어
typedef struct STUBEVENTCALLBACK
{
	QgEventCallback		func;
	void*				data;
	size_t				key;
} StubEventCallback;
QN_DECL_LIST(StubListEventCb, StubEventCallback);

// 스터브 베이스
typedef struct STUBBASE
{
	void*				handle;								// 시스템 스터브 관리
	QnMutex*			mutex;
	QnTimer*			timer;

	QgFlag				flags;								// 플래그
	QgFeature			features;							// 사양
	QgStubStat			stats;								// 시스템 스터브 관리

	uint				display;
	float				aspect;

	float				fps;								// 프레임 당 시간
	float				elapsed;							// 프레임 시간
	float				advance;							// 프레임 시간, 포즈 상태일 때는 0
	double				run;								// 실행 시간
	double				active;								// 활성화된 시간
	double				frames;								// 목표 프레임

	QmRect				bound;								// 현재 윈도우의 위치와 크기 정보
	QmSize				client_size;						// 시스템 스터브 관리, 그리기 영역 크기 (창 크기가 아님)

	QgUimKey			key;
	QgUimMouse			mouse;
	StubMonitorCtnr		monitors;

	StubListEventCb		callbacks;
#ifdef _QN_EMSCRIPTEN_
	funcparam_t			main_loop_callback;
#endif
} StubBase;

// 스터브 인스턴스 
extern StubBase* qg_instance_stub;

// 시스템 스터브를 연다
extern bool stub_system_open(const char* title, int display, int width, int height, QgFlag flags, QgFeature features);
// 시스템 스터브를 정리한다
extern void stub_system_finalize(void);
// 시스템 디바이스가 만들어진 담에 최초 가동
extern void stub_system_actuate(void);
// 시스템 스터브 폴링 (프로그램이 종료되면 거짓)
extern bool stub_system_poll(void);
// 시스템 드래그 켜고 끄기
extern bool stub_system_enable_drop(bool enable);
// 시스템 도움꾼 켜고 끄기
extern bool stub_system_disable_acs(bool enable);
// 시스템 스크린 세이버 켜고 끄기
extern bool stub_system_disable_scr_save(bool enable);
// 시스템 마우스 잡기
extern bool stub_system_relative_mouse(bool enable);
// 시스템 타이틀 설정
extern void stub_system_set_title(const char* title);
// 시스템 바운드 영역을 업데이트한다
extern void stub_system_update_bound(void);
// 시스템 스터브를 포커스로 만든다
extern void stub_system_focus(void);
// 시스템 종횡비를 맞춘다
extern void stub_system_aspect(void);
// 시스템 풀스크린을 전환한다
extern void stub_system_fullscreen(bool fullscreen);
// 시스템 핸들을 얻는다
extern void* stub_system_get_window(void);
// 시스템 디스플레이 핸들을 얻는다 (윈도우에서는 HDC)
extern void* stub_system_get_display(void);

// 내부적으로 마우스 눌림을 연산한다
extern bool stub_track_mouse_click(QimButton button, QimTrack track);
// 내부적으로 토글을 설정한다 (완전 언세이프)
extern void stub_toggle_keys(QikMask keymask, bool on);
// 모니터 이벤트 추가
extern bool stub_event_on_monitor(QgUdevMonitor* monitor, bool connected, bool primary, bool broadcast);
// 레이아웃 이벤트 추가
extern bool stub_event_on_layout(bool enter);
// 포커스 이벤트 추가
extern bool stub_event_on_focus(bool enter);
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
extern void stub_initialize(StubBase* stub, QgFlag flags);


//////////////////////////////////////////////////////////////////////////
// 렌더 디바이스

// 렌더러 정보
typedef struct RENDERERINFO
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
} RendererInfo;

// 렌더 추적 정보
typedef struct RENDERINVOKE
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
typedef struct RENDERTRANSFORM
{
	QmSizeF				size;
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
typedef struct RENDERPARAM
{
	QmVec4				v[4];
	QmMat4				m[4];
	QmMat4*				bone_ptr;
	int					bone_count;
	QmColor				bgc;
} RenderParam;

// 렌더러 디바이스
typedef struct RDHBASE
{
	QsGam				base;

	RendererInfo		info;

	RenderTransform		tm;
	RenderParam			param;
	RenderInvoke		invokes;
} RdhBase;

qs_name_vt(RDHBASE)
{
	qs_name_vt(QSGAM)	base;
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
extern RdhBase* es_allocator(QgFlag flags, QgFeature features);
#endif

// 렌더러 최종 제거
extern void rdh_internal_dispose(void);
// 렌더러 내부 리셋
extern void rdh_internal_reset(void);
// 인보크 리셋
extern void rdh_internal_invoke_reset(void);
// 레이아웃 검사
extern void rdh_internal_check_layout(void);

#define rdh_info()			(&qs_cast_type(qg_instance_rdh, RdhBase)->info)
#define rdh_transform()		(&qs_cast_type(qg_instance_rdh, RdhBase)->tm)
#define rdh_param()			(&qs_cast_type(qg_instance_rdh, RdhBase)->param)
#define rdh_invokes()		(&qs_cast_type(qg_instance_rdh, RdhBase)->invokes)

#define rdh_set_flush(v)	(qs_cast_type(qg_instance_rdh, RdhBase)->invokes.flush=(v))
#define rdh_inc_ends()		(qs_cast_type(qg_instance_rdh, RdhBase)->invokes.ends++)

// 색깔 변환
extern QgClrFmt qg_clrfmt_from_size(int red, int green, int blue, int alpha, bool is_float);
