#pragma once

#include <qs_qg.h>

//////////////////////////////////////////////////////////////////////////
// 스터브

// 이벤트 핸들러 페어
typedef struct STUBEVENTCALLBACK
{
	QgEventCallback		func;
	void*				data;
	size_t				key;
} StubEventCallback;

// 컨테이너
QN_DECLIMPL_LIST(StubListEventCb, StubEventCallback, eventcb_list);
QN_DECLIMPL_CTNR(StubMonitorCtnr, QgUdevMonitor*, monitor_ctnr);

// 스터브 베이스
typedef struct STUBBASE
{
	void*				handle;								// 시스템 스터브 관리
	QnMutex*			mutex;
	QnTimer*			timer;
	QnMount*			mount[10];

	QgFlag				flags;								// 플래그
	QgFeature			features;							// 사양
	QgStubStat			stats;								// 시스템 스터브 관리

	uint				display;
	float				aspect;

	float				elapsed;							// 프레임 시간
	float				advance;							// 프레임 시간, 포즈 상태일 때는 0
	double				active;								// 활성화된 시간

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

#define STUB	(qg_instance_stub)		

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
// 시스템 인스턴스 핸들을 얻는다 (윈도우에서는 HINSTANCE)
extern void* stub_system_get_instance(void);
// 시스템 캔버스 이름을 얻는다 (EMSCRIPTEN)
extern const char* stub_system_get_canvas(void);
// 시스템 클래스 이름을 얻는다 (윈도우)
extern const wchar* stub_system_get_class_name(void);

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

// 렌더 노드 창고 번호
typedef enum RENDERNODESHED
{
	RDHNODE_NONE,
	RDHNODE_RENDER,
	RDHNODE_TEXTURE,
	RDHNODE_MAX_VALUE,
} RenderNodeShed;

// 렌더러 정보
typedef struct RENDERERINFO
{
	char				name[64];
	char				renderer[64];						// 렌더러 이름
	char				vendor[64];							// 디바이스 제조사
	int					renderer_version;					// 렌더러 버전
	int					shader_version;						// 세이더 버전

	uint				max_layout_count;					// 최대 레이아웃(=정점 속성) 갯수
	uint				max_indices;						// 최대 인덱스 갯수
	uint				max_vertices;						// 최대 정점 갯수
	uint				max_tex_dim;						// 최대 텍스쳐 크기
	uint				max_tex_count;						// 최대 텍스쳐 갯수
	uint				max_off_count;						// 최대 오프 텍스쳐(=렌더타겟/프레임버퍼) 갯수

	QgClrFmt			clr_fmt;							// 색깔 포맷
#ifdef _QN_UNIX_
	int					visual_id;							// 비주얼 아이디
#endif

	bool				enabled_stencil;					// 스텐실 사용 가능
} RendererInfo;

// 렌더 추적 정보
typedef struct RENDERERINVOKE
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
} RendererInvoke;

// 렌더 트랜스포메이션
typedef struct RENDERERTRANSFORM
{
	QmSize				size;
	float				Near, Far;
	QmMat4				ortho;								// ortho transform
	QmMat4				world;
	QmMat4				view;
	QmMat4				proj;
	QmMat4				view_proj;
	QmMat4				invv;								// inverse view
	QmMat4				frm;								// tex formation
	QmMat4				tex[4];
	QmRect				scissor;
} RendererTransform;

// 렌더 인수
typedef struct RENDERERPARAM
{
	QmVec4				v[4];
	QmMat4				m[4];
	QmMat4*				bone_ptr;
	int					bone_count;
	QmVec4				bgc;

	QgVarShaderFunc		callback_func;
	void*				callback_data;
} RendererParam;

// 렌더러 디바이스
typedef struct RDHBASE
{
	QN_GAM_BASE(QNGAMBASE);

	RendererInfo		info;

	RendererTransform	tm;
	RendererParam		param;
	RendererInvoke		invokes;

	QnNodeMukum			mukums[RDHNODE_MAX_VALUE];
} RdhBase;

QN_DECL_VTABLE(RDHBASE)
{
	QN_DECL_VTABLE(QNGAMBASE)	base;
	void (*layout)(void);
	void (*reset)(void);
	void (*clear)(QgClear);

	bool (*begin)(bool);
	void (*end)(void);
	void (*flush)(void);

	QgBuffer* (*create_buffer)(QgBufferType, uint, uint, const void*);
	QgRenderState* (*create_render)(const char*, const QgPropRender*, const QgPropShader*);
	QgTexture* (*create_texture)(const char*, const QgImage*, QgTexFlag);

	bool (*set_vertex)(QgLayoutStage, void/*QgBuffer*/*);
	bool (*set_index)(void/*QgBuffer*/*);
	bool (*set_render)(void/*QgRenderState*/*);
	bool (*set_texture)(int stage, void/*QgTexture*/*);

	bool (*draw)(QgTopology, int);
	bool (*draw_indexed)(QgTopology, int);
	void (*draw_sprite)(const QmRect*, void/*QgTexture*/*, const QmVec*, const QmVec*);
	void (*draw_sprite_ex)(const QmRect*, float, void/*QgTexture*/*, const QmVec*, const QmVec*);
};

// 렌더 디바이스
extern RdhBase* qg_instance_rdh;

#define RDH					(qg_instance_rdh)		
#define RDH_INFO			(&qg_instance_rdh->info)
#define RDH_TRANSFORM		(&qg_instance_rdh->tm)
#define RDH_PARAM			(&qg_instance_rdh->param)
#define RDH_INVOKES			(&qg_instance_rdh->invokes)
#define RDH_VERSION			(qg_instance_rdh->info.renderer_version)

#define rdh_set_flush(v)	(qg_instance_rdh->invokes.flush=(v))
#define rdh_inc_ends()		(qg_instance_rdh->invokes.ends++)

#ifdef USE_GL
extern RdhBase* qgl_allocator(QgFlag flags, QgFeature features);			// OPENGL 할당 (+ES)
#endif

// 렌더러 리소스 제거
extern void rdh_internal_clean(void);
// 렌더러 최종 제거
extern void rdh_internal_dispose(void);
// 렌더러 내부 레이아웃
extern void rdh_internal_layout(void);
// 렌더러 내부 리셋
extern void rdh_internal_reset(void);
// 인보크 리셋
extern void rdh_internal_invoke_reset(void);
// 레이아웃 검사
extern void rdh_internal_check_layout(void);
// 노드 추가요
extern void rdh_internal_add_node(RenderNodeShed shed, void* node);
// 노드 제거요 (dispose에서 호출용)
extern void rdh_internal_unlink_node(RenderNodeShed shed, void* node);

// 열거자 컨버터 초기화
extern void qg_init_converters(void);
// 색깔 변환
extern QgClrFmt qg_rgba_to_clrfmt(int red, int green, int blue, int alpha, bool is_float);
// LAYOUT USAGE 변환
extern QgLayoutUsage qg_str_to_layout_usage(size_t hash, const char* name);
// 세이더 자동 상수 변환
extern QgScAuto qg_str_to_shader_const_auto(size_t hash, const char* name);
// 색깔 포맷 문자열로 변환
extern const char* qg_clrfmt_to_str(QgClrFmt fmt);
// LAYOTU USAGE 문자열로 변환
extern const char* qg_layout_usage_to_str(const QgLayoutUsage usage);
// 세이더 자동 상수 문자열로 변환
extern const char* qg_shader_const_auto_to_str(const QgScAuto sca);


//////////////////////////////////////////////////////////////////////////
// 인라인

// 이미지 크기 계산
INLINE size_t qg_calc_image_block_size(const QgPropPixel* prop, int width, int height)
{
	switch (prop->format)
	{
		case QGCF_DXT1:
		case QGCF_DXT3:
		case QGCF_DXT5:
		case QGCF_EXT1:
		case QGCF_EXT2:
		case QGCF_ASTC4:
		case QGCF_ASTC8:
			return ((width + 3) / 4) * ((height + 3) / 4) * prop->tbp;
		default:
			break;
	}
	// 그냥 계산
	return width * height * prop->tbp;
}
