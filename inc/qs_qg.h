/*
 * QsLib [QG Layer]
 * Made by kim 2004-2024
 *
 * 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
 * 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
 */
 /**
  * @file qs_qg.h
  *
  * [QG Layer]는 공통 그래픽 구현을 목표로 합니다.
  */
#pragma once

#include <qs_qn.h>
#include <qs_math.h>
#include <qs_kmc.h>

QN_EXTC_BEGIN

//
typedef struct QgRdh		QgRdh;							/** @Brief 렌더러 */
typedef struct QgGam		QgGam;							/** @Brief 런더 감 */
typedef struct QgBuffer		QgBuffer;						/** @Brief 버퍼 */
typedef struct QgRender		QgRender;						/** @brief 렌더 파이프라인 */


//////////////////////////////////////////////////////////////////////////
// property
#define QG_PROP_WINDOWS_ICON			"QG_PROP_WINDOWS_ICON"
#define QG_PROP_WINDOWS_SMALLICON		"QG_PROP_WINDOWS_SMALLICON"


//////////////////////////////////////////////////////////////////////////
// types

/** @brief 색깔 포맷 */
typedef enum QgClrFmt
{
	QGCF_NONE,
	QGCF8_L,												/** @brief 8비트 루미넌스 */
	QGCF16_L,												/** @brief 16비트 루미넌스 */
	QGCF16_RGB,												/** @brief 16비트 RGBX */
	QGCF16_RGBA,											/** @brief 16비트 RGBA */
	QGCF32_RGB,												/** @brief 32비트 RGBX */
	QGCF32_RGBA,											/** @brief 32비트 RGBA */
	QGCF32_BGRA,											/** @brief 32비트 BGRA */
	QGCF16F_R,												/** @brief 16비트 실수 R */
	QGCF32F_R,												/** @brief 32비트 실수 R */
	QGCF64F_BGRA,											/** @brief 64비트 실수 BGRA */
	QGCF128F_BGRA,											/** @brief 128비트 실수 BGRA */
	QGCF_MAX_VALUE,
} QgClrFmt;

/** @brief 토폴로지 */
typedef enum QgTopology
{
	QGTPG_POINT,											/** @brief 점 */
	QGTPG_LINE,												/** @brief 선분 */
	QGTPG_LINE_STRIP,										/** @brief 연결된 선분 */
	QGTPG_TRI,												/** @brief 삼각형 */
	QGTPG_TRI_STRIP,										/** @brief 연결된 삼각형 */
	QGTPG_MAX_VALUE,
} QgTopology;

/** @brief 렌더 스테이지 */
typedef enum QgRvStage
{
	QGRVS_1,												/** @brief 스테이지 1 */
	QGRVS_2,												/** @brief 스테이지 2 */
	QGRVS_3,												/** @brief 스테이지 3 */
	QGRVS_4,												/** @brief 스테이지 4 */
	QGRVS_MAX_VALUE,
} QgRvStage;

/** @brief 채우기 방식 */
typedef enum QgFill
{
	QGFILL_POINT,											/** @brief 꼭짓점 */
	QGFILL_WIRE,											/** @brief 외곽선 */
	QGFILL_SOLID,											/** @brief 채우기 */
	QGFILL_MAX_VALUE,
} QgFill;

/** @brief 면 제거 방법 */
typedef enum QgCull
{
	QGCULL_NONE,											/** @brief 몇 제거 없음 */
	QGCULL_FRONT,											/** @brief 앞면 제거 */
	QGCULL_BACK,											/** @brief 뒷면 제거 */
	QGCULL_MAX_VALUE
} QgCull;

/** @brief 레이아웃 스테이지 */
typedef enum QgLoStage
{
	QGLOS_1,												/** @brief 레이아웃 1 */
	QGLOS_2,												/** @brief 레이아웃 2 */
	QGLOS_3,												/** @brief 레이아웃 3 */
	QGLOS_4,												/** @brief 레이아웃 4 */
	QGLOS_MAX_VALUE,
} QgLoStage;

/** @brief 레아아웃 사용 방법 */
typedef enum QgLoUsage
{
	QGLOU_POSITION,											/** @brief position */
	QGLOU_COLOR,											/** @brief diffuse & specular */
	QGLOU_TEXTURE,											/** @brief texture coordinate */
	QGLOU_WEIGHT,											/** @brief blend weights */
	QGLOU_INDEX,											/** @brief blend indices */
	QGLOU_NORMAL,											/** @brief normal vector */
	QGLOU_TANGENT,											/** @brief tangent vector */
	QGLOU_BINORMAL,											/** @brief bi-normal vector */
	QGLOU_MAX_VALUE
} QgLoUsage;

/** @brief 레아아웃 타입 */
typedef enum QgLoType
{
	QGLOT_FLOAT1,											/** @brief 1개의 실수 */
	QGLOT_FLOAT2,											/** @brief 2개의 실수(Vector2) */
	QGLOT_FLOAT3,											/** @brief 3개의 실수(Vector3) */
	QGLOT_FLOAT4,											/** @brief 4개의 실수(Vector4) */
	QGLOT_HALF2,											/** @brief 2개의 하프 정수 */
	QGLOT_HALF4,											/** @brief 4개의 하프 정수 */
	QGLOT_BYTE2,											/** @brief 2개의 바이트 */
	QGLOT_BYTE4,											/** @brief 4개의 바이트 */
	QGLOT_BYTE4_N,											/** @brief 4개의 정규화된 바이트(Kolor) */
	QGLOT_SHORT2,											/** @brief 2개의 16비트 정수 */
	QGLOT_SHORT4,											/** @brief 4개의 16비트 정수 */
	QGLOT_SHORT2_N,											/** @brief 2개의 정규화된 16비트 정수(Texture Coordinate) */
	QGLOT_COLOR,											/** @brief 색깔 */
	QGLOT_COLORN,											/** @brief 정규화된 색깔 */
	QGLOT_MAX_VALUE
} QgLoType;

/** @brief 뎁스 모드 */
typedef enum QgDepth
{
	QGDEPTH_OFF,											/** @brief 뎁스 테스트 하지 않음 */
	QGDEPTH_LE,												/** @brief 뎁스가 작으면 통과 */
	QGDEPTH_EQ,												/** @brief 뎁스가 같으면 통과 */
	QGDEPTH_LEQ,											/** @brief 뎁스가 작거나 같으면 통과 */
	QGDEPTH_GR,												/** @brief 뎁스가 크면 통과 */
	QGDEPTH_NEQ,											/** @brief 뎁스가 같지 않으면 통과 */
	QGDEPTH_ALWAYS,											/** @brief 뎁스는 언제나 통과 */
	QGDEPTH_MAX_VALUE,
} QgDepth;

/** @brief 블렌드 모드 */
typedef enum QgBlend
{
	QGBLEND_OFF,											/** @brief dstRGBA = srcRGBA */
	QGBLEND_BLEND,											/** @brief dstRGB = (srcRGB * srcA) + (dstRGB * (1 - srcA), dstA = srcA  +  (dstA * (1 - srcA)) */
	QGBLEND_ADD,											/** @brief dstRGB = (srgRGB * srcA) + dstRGB, dstA = dstA */
	QGBLEND_MOD,											/** @brief dstRGB = srcRGB * dstRGB, dstA = dstA */
	QGBLEND_MUL,											/** @brief dstRGB = (srcRGB * dstRGB) + (dstRGB * (1 - srcA)), dstA = dstA */
	QGBLEND_MAX_VALUE,
} QgBlend;

/** @brief 스텐실 모드 */
typedef enum QgStencil
{
	QGSTENCIL_OFF,											/** @brief 스텐실 테스트 하지 않음 */
	QGSTENCIL_ALWAY,										/** @brief 스텐실을 언제나 설정 (마스크: 0xFF) */
	QGSTENCIL_NEQ,											/** @brief 스텐실이 같지 않으면 설정 (마스크: 0) */
	QGSTENCIL_MAX_VALUE,
} QgStencil;

/** @brief 세이더 타입 */
typedef enum QgShdType
{
	QGSHADER_VS = QN_BIT(0),								/** @brief 정점 세이더 */
	QGSHADER_PS = QN_BIT(1),								/** @brief 픽셀(프래그먼트) 세이더 */
	QGSHADER_ALL = QGSHADER_VS | QGSHADER_PS,				/** @brief 모든 세이더 대리 */
} QgShdType;

/** @brief 세이더 전역 변수 타입 */
typedef enum QgShdConstType
{
	QGSCT_UNKNOWN,											/** @brief 알 수 없음 */
	QGSCT_FLOAT1,											/** @brief 1개의 실수 */
	QGSCT_FLOAT2,											/** @brief 2개의 실수(Vector2) */
	QGSCT_FLOAT3,											/** @brief 3개의 실수(Vector3) */
	QGSCT_FLOAT4,											/** @brief 4개의 실수(Vector4) */
	QGSCT_FLOAT16,											/** @brief 16개의 실수(Matrix4x4) */
	QGSCT_INT1,												/** @brief 1개의 정수 */
	QGSCT_INT2,												/** @brief 2개의 정수 */
	QGSCT_INT3,												/** @brief 3개의 정수 */
	QGSCT_INT4,												/** @brief 4개의 정수 */
	QGSCT_BYTE1,											/** @brief 1개의 바이트 */
	QGSCT_BYTE2,											/** @brief 2개의 바이트 */
	QGSCT_BYTE3,											/** @brief 3개의 바이트 */
	QGSCT_BYTE4,											/** @brief 4개의 바이트 */
	QGSCT_SPLR_1D,											/** @brief 1D 텍스쳐 샘플러 */
	QGSCT_SPLR_2D,											/** @brief 2D 텍스쳐 샘플러 */
	QGSCT_SPLR_3D,											/** @brief 3D 텍스쳐 샘플러 */
	QGSCT_SPLR_CUBE,										/** @brief 큐브 텍스쳐 샘플러 */
	QGSCT_MAX_VALUE
} QgShdConstType;

/** @brief 세이더 자동처리 전역 변수 */
typedef enum QgShdConstAuto
{
	QGSCA_ORTHO_PROJ,										/** @brief 평면 투영 행렬 */
	QGSCA_WORLD,											/** @brief 월드 행렬 */
	QGSCA_VIEW,												/** @brief 뷰 행렬 */
	QGSCA_PROJ,												/** @brief 투영 행렬 */
	QGSCA_VIEW_PROJ,										/** @brief 뷰와 투영의 곱행렬 */
	QGSCA_INV_VIEW,											/** @brief 뷰의 역행렬 */
	QGSCA_WORLD_VIEW_PROJ,									/** @brief 전역 투영 */
	QGSCA_TEX1,												/** @brief 텍스쳐 1번 */
	QGSCA_TEX2,												/** @brief 텍스쳐 2번 */
	QGSCA_TEX3,												/** @brief 텍스쳐 3번 */
	QGSCA_TEX4,												/** @brief 텍스쳐 4번 */
	QGSCA_TEX5,												/** @brief 텍스쳐 5번 */
	QGSCA_TEX6,												/** @brief 텍스쳐 6번 */
	QGSCA_TEX7,												/** @brief 텍스쳐 7번 */
	QGSCA_TEX8,												/** @brief 텍스쳐 8번 */
	QGSCA_PROP_VEC1,										/** @brief 4개 실수 1번 */
	QGSCA_PROP_VEC2,										/** @brief 4개 실수 2번 */
	QGSCA_PROP_VEC3,										/** @brief 4개 실수 3번 */
	QGSCA_PROP_VEC4,										/** @brief 4개 실수 4번 */
	QGSCA_PROP_MAT1,										/** @brief 행렬 1번 */
	QGSCA_PROP_MAT2,										/** @brief 행렬 2번 */
	QGSCA_PROP_MAT3,										/** @brief 행렬 3번 */
	QGSCA_PROP_MAT4,										/** @brief 행렬 4번 */
	QGSCA_MAT_PALETTE,										/** @brief 행렬 팔레트 */
	QGSCA_MAX_VALUE
} QgShdConstAuto;

/** @brief 버퍼 타입 */
typedef enum QgBufType
{
	QGBUFFER_INDEX,											/** @brief 인덱스 버퍼 */
	QGBUFFER_VERTEX,										/** @brief 정점 버퍼 */
} QgBufType;

/** @brief 지우기 역할 */
typedef enum QgClear
{
	QGCLEAR_DEPTH = QN_BIT(0),								/** @brief 뎁스 버퍼 지우기 */
	QGCLEAR_STENCIL = QN_BIT(1),							/** @brief 스텐실 버퍼 지우기 */
	QGCLEAR_RENDER = QN_BIT(2),								/** @brief 렌더 타겟 영영 지우기 */
} QgClear;

/** @brief 스터브와 렌더러 만들 때 플래그 */
typedef enum QgFlag
{
	// 스터브 및 렌더러 플래그 (0~7)
	QGFLAG_FULLSCREEN = QN_BIT(0),							/** @brief 전체 화면 */
	QGFLAG_BORDERLESS = QN_BIT(1),							/** @brief 테두리 없는 윈도우 */
	QGFLAG_RESIZABLE = QN_BIT(2),							/** @brief 크기 변경할 수 있음 */
	QGFLAG_NOTITLE = QN_BIT(3),								/** @brief 타이틀 바가 없음 */
	QGFLAG_FOCUS = QN_BIT(4),								/** @brief 입력 포커스 받을 수 있음 */
	QGFLAG_TEXT = QN_BIT(5),									/** @brief 텍스트 입력을 받을 수 있음 */
	// 렌더러 플래그 (24~30)
	QGFLAG_VSYNC = QN_BIT(27),								/** @brief VSYNC 켜기 */
	QGFLAG_DITHER = QN_BIT(29),								/** @brief 16비트 모드 사용 */
	QGFLAG_MSAA = QN_BIT(30),								/** @brief 멀티 샘플링 사용 */
	// 스터브 사양 (8~15)
	QGFEATURE_DISABLE_ACS = QN_BIT(8),						/** @brief 접근성 끄기 */
	QGFEATURE_DISABLE_SCRSAVE = QN_BIT(9),					/** @brief 화면 보호기 끄기 */
	QGFEATURE_ENABLE_DROP = QN_BIT(10),						/** @brief 드래그 드랍 사용 */
	QGFEATURE_ENABLE_SYSWM = QN_BIT(11),					/** @brief 시스템 메시지 받기 */
	QGFEATURE_ENABLE_IDLE = QN_BIT(12),						/** @brief 비활성 대기 상태 사용 */
	// 사용자가 설정할 수 없는 플래그
	QGSPECIFIC_VIRTUAL = QN_BIT(31),						/** @brief 가상 스터브 사용 */
} QgFlag;

/** @brief 스터브 상태 */
typedef enum QgStubStat
{
	QGSSTT_EXIT = QN_BIT(0),								/** @brief 끝내기 */
	QGSSTT_ACTIVE = QN_BIT(1),								/** @brief 스터브가 활성 상태 */
	QGSSTT_LAYOUT = QN_BIT(2),								/** @brief 스터브 크기를 변경 */
	QGSSTT_FOCUS = QN_BIT(3),								/** @brief 스터브가 포커스 */
	QGSSTT_PAUSE = QN_BIT(4),								/** @brief 포즈 중 */
	QGSSTT_DROP = QN_BIT(5),								/** @brief 드래그 드랍 중 */
	QGSSTT_CURSOR = QN_BIT(6),								/** @brief 커서의 표시 */
	QGSSTT_MOUSEHOLD = QN_BIT(7),							/** @brief 마우스 홀드 */
} QgStubStat;

/** @brief 이벤트 타입 */
typedef enum QgEventType
{
	QGEV_NONE,												/** @brief 이벤트 없음 */
	QGEV_ACTIVE,											/** @brief 스터브 활성 상태 이벤트 */
	QGEV_LAYOUT,											/** @brief 화면 크기 이벤트 */
	QGEV_MOUSEMOVE,											/** @brief 마우스를 움직여요 */
	QGEV_MOUSEDOWN,											/** @brief 마우스 버튼을 눌렀어요 */
	QGEV_MOUSEUP,											/** @brief 마우스 버튼을 뗏어요 */
	QGEV_MOUSEWHEEL,										/** @brief 마우스 휠을 굴렸어요 */
	QGEV_MOUSEDOUBLE,										/** @brief 마우스 버튼을 두번 눌렀어요 */
	QGEV_KEYDOWN,											/** @brief 키보드를 눌렀어요 */
	QGEV_KEYUP,												/** @brief 키보드에서 손을 뗐어요 */
	QGEV_TEXTINPUT,											/** @brief 텍스트 입력 */
	QGEV_WINDOW,											/** @brief 윈도우 이벤트 */
	QGEV_SYSWM,												/** @brief 시스템 메시지 */
	QGEV_DROPBEGIN,											/** @brief 드랍을 시작한다 */
	QGEV_DROPEND,											/** @brief 드랍이 끝났다 */
	QGEV_DROPFILE,											/** @brief 파일 드랍 */
	QGEV_EXIT,												/** @brief 끝내기 */
	QGEV_MAX_VALUE,
} QgEventType;

typedef enum QgWindowEventType
{
	QGWEV_NONE,
	QGWEV_SHOW,												/** @brief 윈도우가 보인다 */
	QGWEV_HIDE,												/** @brief 윈도우가 안보인다 */
	QGWEV_PAINTED,
	QGWEV_RESTORED,
	QGWEV_MAXIMIZED,
	QGWEV_MINIMIZED,
	QGWEV_MOVED,
	QGWEV_SIZED,
	QGWEV_GOTFOCUS,
	QGWEV_LOSTFOCUS,
	QGWEV_CLOSE,
	QGWEV_MAX_VALUE,
} QgWindowEventType;

/** @brief 최대 이벤트 갯수 */
#define QGMAX_EVENTS		1000


//////////////////////////////////////////////////////////////////////////
// properties

/** @brief 픽셀 포맷 */
typedef struct QgPropPixel
{
	QgClrFmt			fmt;								/** @brief 픽셀 포맷 */
	uint				bpp;								/** @brief 픽셀 당 비트 수 */
	byte				rr, rl;								/** @brief 빨강 */
	byte				gr, gl;								/** @brief 녹색 */
	byte				br, bl;								/** @brief 파랑 */
	byte				ar, al;								/** @brief 알파 */
} QgPropPixel;

/** @brief 코드 데이터 */
typedef struct QgCodeData
{
	size_t				size;								/** @brief code 길이 */
	const void*			code;								/** @brief 코드 데이터 포인터 */
} QgCodeData;

/** @brief 레이아웃 요소 */
typedef struct QgLayoutInput
{
	QgLoStage			stage;								/** @brief 스테이지 구분 */
	int					index;								/** @brief 스테이지에서 인덱스 */
	QgLoUsage			usage;								/** @brief 사용법 */
	QgLoType			type;								/** @brief 데이터 타입 */
} QgLayoutInput;

/** @brief 레이아웃 */
typedef struct QgPropLayout
{
	size_t				count;								/** @brief 요소 갯수 */
	QgLayoutInput*		elements;							/** @brief 요소 데이터 포인트 */
} QgPropLayout;

/** @brief 블렌드 */
typedef struct QgPropBlend
{
	BOOL				use_coverage;						/** @brief sample coverage / alpha to coverage */
	BOOL				separate;							/** @brief 거짓이면 rb[0]만 사용 */
	QgBlend				rb[QGRVS_MAX_VALUE];				/** @brief 스테이지 별 블렌드 정보 */
} QgPropBlend;

/** @brief 래스터라이저 */
typedef struct QgPropRasterizer
{
	QgFill				fill;								/** @brief 채우기 방법 */
	QgCull				cull;								/** @brief 면 제거 방법 */
	float				depth_bias;							/** @brief 주어진 픽셀에 추가하는 깊이 값 */
	float				slope_scale;						/** @brief 픽셀 경사면에 주어지는 스칼라 값 */
} QgPropRasterizer;

/** @brief 렌더 포맷 */
typedef struct QgPropRvFormat
{
	uint				count;								/** @brief rtv 갯수 */
	QgClrFmt			rtv[QGRVS_MAX_VALUE];				/** @brief Render Target (color) Value */
	QgClrFmt			dsv;								/** @brief Depth Stencil (color) Value */
} QgPropRvFormat;

/** @brief 렌더 파이프라인 */
typedef struct QgPropRender
{
	QgCodeData			vs;									/** @brief 정점 세이더 데이터 */
	QgCodeData			ps;									/** @brief 픽셀 세이더 데이터 */
	QgPropBlend			blend;								/** @brief 블렌드 */
	QgPropRasterizer	rasterizer;							/** @brief 래스터라이저 */
	QgDepth				depth;								/** @brief 뎁스 */
	QgStencil			stencil;							/** @brief 스텐실 */
	QgPropLayout		layout;								/** @brief 정점 레이아웃 */
	QgPropRvFormat		format;								/** @brief 렌더 포맷 */
	QgTopology			topology;							/** @brief 토폴로지 */
	uint				__mask;								/** @brief 용도 못 찾음 */
} QgPropRender;

/** @brief 세이더 변수 */
typedef struct QgVarShader
{
	char				name[64];							/** @brief 변수 이름 */

	nuint				offset;								/** @brief 변수 옵셋 */
	ushort				size;								/** @brief 변수의 크기 */

	ushort				manual;								/** @brief 수동 모드 여부 */
	QgShdConstType		type;								/** @brief 변수 타입 */
} QgVarShader;

/** @brief 세이더 변수 콜백 함수 */
typedef void(*QgVarShaderFunc)(void*, const QgVarShader*);

/** @brief 이벤트 */
typedef union QgEvent
{
	QgEventType			ev;									/** @brief 이벤트 타입 */
	struct QgEventActive
	{
		QgEventType			ev;
		BOOL				active;							/** @brief 활성 상태면 참 */
		double				delta;							/** @brief 마지막 활성 상태로 부터의 지난 시간(초) */
	}					active;								/** @brief 액티브 이벤트 */
	struct QgEventLayout
	{
		QgEventType			ev;
		QnSize				size;							/** @brief 윈도우 크기 */
		QnRect				bound;							/** @brief 변경된 윈도우 바운드 */
	}					layout;								/** @brief 레이아웃 이벤트 */
	struct QgEventKeyboard
	{
		QgEventType			ev;
		QikKey				key;							/** @brief 이벤트에 해당하는 키 */
		QikMask				mask;							/** @brief 특수키 상태 */
		BOOL				repeat;							/** @brief 계속 눌려 있었다면 참 */
	}					key;								/** @brief 키 눌림 떼임 이벤트 */
	struct QgEventMouseMove
	{
		QgEventType			ev;
		QnPoint				pt;								/** @brief 마우스 좌표 */
		QnPoint				delta;							/** @brief 마우스 이동 거리 */
		QimMask				mask;							/** @brief 마우스 버튼의 상태 */
	}					mmove;								/** @brief 마우스 이동 이벤트 */
	struct QgEventMouseButton
	{
		QgEventType			ev;
		QnPoint				pt;								/** @brief 마우스 좌표 */
		QimButton			button;							/** @brief 이벤트에 해당하는 버튼 */
		QimMask				mask;							/** @brief 마으스 버튼의 상태 */
	}					mbutton;							/** @brief 마우스 버튼 눌림 떼임 이벤트 */
	struct QgEventMouseWheel
	{
		QgEventType			ev;
		QnPoint				pt;								/** @brief 마우스 좌표 */
		QnPoint				wheel;							/** @brief 휠 움직임 (Y가 기본휠, X는 틸트) */
		QnVec2				precise;						/** @brief 정밀한 움직임 (Y가 기본휠, X는 틸트) */
		int					direction;						/** @brief 참이면 뱡향이 반대 */
	}					mwheel;								/** @brief 마우스 휠 이벤트 */
	struct QgEventText
	{
		QgEventType			ev;
		int					len;							/** @brief 텍스트 길이 */
		char				data[16];						/** @brief 텍스트 내용 */
	}					text;
	struct QgEventDrop
	{
		QgEventType			ev;
		int					len;							/** @brief 데이타의 길이 */
		char*				data;							/** @brief 테이타 포인터. 데이터의 유효기간은 다음 loop 까지 */
	}					drop;
	struct QgEventWindowEvent
	{
		QgEventType			ev;
		QgWindowEventType	mesg;							/** @brief 윈도우 이벤트 메시지 */
		int					param1;							/** @brief 파라미터1 */
		int					param2;							/** @brief 파라미터2 */
	}					wevent;
#if _QN_WINDOWS_
	struct QgEventSysWm
	{
		QgEventType			ev;
		uint				mesg;							/** @brief 메시지 */
		nuint				wparam;							/** @brief WPARAM */
		nint				lparam;							/** @brief LPARAM */
		void*				hwnd;							/** @brief 윈도우 핸들 */
	}					syswm;
#endif
} QgEvent;

/** @brief 키 상태 */
typedef struct QgUimKey
{
	QikMask				mask;								/** @brief 특수 키 상태 */
	byte				key[QIK_MAX_VALUE / 8 + 1];			/** @brief 전체 키 상태 배열 */
} QgUimKey;

/** @brief 마우스 상태 */
typedef struct QgUimMouse
{
	QimMask				mask;								/** @brief 마우스 버튼 상태 */
	QnPoint				pt;									/** @brief 마우스 좌표 */
	QnPoint				last;								/** @brief 마우스의 이전 좌표 */

	struct QgUimMouseClick
	{
		uint				tick;							/** @brief 첫번째 눌렸을 때 */
		uint				ltick;							/** @brief 마지막으로 두번 눌렸을 때 */
		QimButton			btn;							/** @brief 두번 검새 때 눌린 마우스 버튼 */
		QnPoint				loc;							/** @brief 두번 검사 때 마우스 위치 */
	}					clk;								/** @brief 마우스 눌림 정보 */
	struct QgUimMouseLimit
	{
		int					move;							/** @brief 제한 이동 거리(포인트)의 제곱 */
		uint				tick;							/** @brief 제한 클릭 시간(밀리초) */
	}					lim;								/** @brief 마우스 더블 클릭 구현 정보 */
	struct QgUimMouseWheel
	{
		QnVec2				accm;							/** @brief 가속도 */
		QnVec2				precise;						/** @brief 정밀 값 */
		QnPoint				integral;						/** @brief 값 */
		BOOL				direction;						/** @brief 거짓=기본휠, 참=틸트휠 */
	}					wheel;
} QgUimMouse;

/** @brief 컨트롤러 상태 */
typedef struct QgUimCtrl
{
	QicButton			btn : 16;							/** @brief 컨트롤러 버튼 상태 */
	QnPoint				trg;
	QnVec2				lthb;
	QnVec2				rthb;
} QgUimCtrl;

/** @brief 컨트롤러 정보 */
typedef struct QgUimCtrlInfo
{
	QicMask				flags;

	int					type : 16;
	int					extend : 16;

	int					battery_type : 8;
	int					battery_level : 8;

	int					headset_type : 8;
	int					headset_level : 8;
} QgUimCtrlInfo;

/** @brief 컨트롤러 진동 */
typedef struct QgUimCtrlVib
{
	ushort				left;
	ushort				right;
} QgUimCtrlVib;

/** @brief 디바이스 정보 */
typedef struct QgDeviceInfo
{
	char				name[64];
	char				renderer[64];						/** @brief 렌더러 이름 */
	char				vendor[64];							/** @brief 디바이스 제조사 */
	int					renderer_version;					/** @brief 렌더러 버전 */
	int					shader_version;						/** @brief 세이더 버전 */
	int					max_layout_count;					/** @brief 최대 레이아웃(=정점 속성) 갯수 */
	int					max_indices;						/** @brief 최대 인덱스 갯수 */
	int					max_vertices;						/** @brief 최대 정점 갯수 */
	int					max_tex_dim;						/** @brief 최대 텍스쳐 크기 */
	int					max_tex_count;						/** @brief 최대 텍스쳐 갯수 */
	int					max_off_count;						/** @brief 최대 오프 텍스쳐(=렌더타겟/프레임버퍼) 갯수 */
	QgClrFmt			clr_fmt QN_PADDING_64(4, 0);		/** @brief 색깔 포맷 */
	
} QgDeviceInfo;

/** @brief 렌더러 추적 정보 */
typedef struct QgRenderInvoke
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

	BOOL				flush;
} QgRenderInvoke;

/** @brief 렌더러 트랜스 포매이션 */
typedef struct QgRenderTm
{
	QnVec2				size;
	QnDepth				depth;
	QnMat4				world;
	QnMat4				view;
	QnMat4				project;
	QnMat4				view_project;
	QnMat4				inv;								/** @brief inverse view */
	QnMat4				ortho;								/** @brief ortho transform */
	QnMat4				frm;								/** @brief tex formation */
	QnMat4				tex[4];
	QnRect				scissor;
} QgRenderTm;

/** @brief 렌더러 인수 */
typedef struct QgRenderParam
{
	QnColor				bgc;
	QnVec4				v[4];
	QnMat4				m[4];
	QnMat4*				bone_ptr;
	int					bone_count QN_PADDING_3264(8, 4, 0);
} QgRenderParam;


//////////////////////////////////////////////////////////////////////////
// stub

/**
 * @brief 스터브를 연다
 * @param title 윈도우 타이틀
 * @param width 윈도우 너비
 * @param height 윈도우 높이
 * @param flags 생성 플래그 (QgFlag)
 * @return 스터브가 만들어지면 참
 * @see QgFlag qg_close_Stub
*/
QSAPI bool qg_open_stub(const char* title, int width, int height, int flags);
/**
 * @brief 스터브를 닫는다
 * @see qg_open_Stub
*/
QSAPI void qg_close_stub(void);
/**
 * @brief 스터브 사양을 켜고 끈다
 * @param feature 대상 스터브 사양 (QGFEATURE_)
 * @param enable 켜려면 참, 끄려면 거짓
 * @return 처리한 사양 갯수
*/
QSAPI int qg_feature(int feature, bool enable);
/**
 * @brief 스터브 윈도우 타이틀 설정
 * @param title 타이틀 문자열
*/
QSAPI void qg_set_title(const char* u8text);

/**
 * @brief 스터브 루프를 처리한다
 * @return 거짓이면 프로그램을 종료한다
*/
QSAPI bool qg_loop(void);
/**
 * @brief 스터브 이벤트를 폴링한다
 * @param[out] ev 폴링한 이벤트를 반환
 * @return 처리할 이벤트가 더 이상 없으면 거짓
*/
QSAPI bool qg_poll(QgEvent* ev);
/**
 * @brief 스터브 루프를 중단한다
 * @see qg_loop qg_poll
 * 다만, 바로 프로그램을 종료하는 것은 아니며. 실제로 이벤트를 처리하지 않도록 하는 역할을 한다
*/
QSAPI void qg_exit_loop(void);

/**
 * @brief 전체 키 정보를 얻는다
 * @return 키 정보 포인터
*/
QSAPI const QgUimKey* qg_get_key_info(void);
/**
 * @brief 전체 마우스 정보를 얻는다
 * @param  마우스 정보 포인터
 * @return
*/
QSAPI const QgUimMouse* qg_get_mouse_info(void);
/**
 * @brief 마우스 더블 클릭 정밀도를 설정한다
 * @param density 더블 클릭하면서 마우스를 움직여도 되는 거리 (포인트, 최대값 50)
 * @param interval 클릭과 클릭 사이의 시간 (밀리초, 최대값 5000)
 * @return 인수의 범위를 벗어나면 거짓을 반환
 * @see qg_get_mouse_info
*/
QSAPI bool qg_set_double_click(int density, int interval);
/**
 * @brief 키가 눌렸나 테스트 한다
 * @param key 테스트할 키
 * @return 눌렸으면 참
*/
QSAPI bool qg_test_key(QikKey key);
/**
 * @brief 키의 눌림 상태를 설정한다
 * @param key 설정할 키
 * @param down 참이면 눌림, 거짓이면 안눌림
*/
QSAPI void qg_set_key(QikKey key, bool down);
/**
 * @brief 초당 프레임(FPS)를 얻는다
 * @return 초당 프레임 수
*/
QSAPI float qg_get_fps(void);
/**
 * @brief 실행 시간을 얻는다
 * @return 실행 시간
*/
QSAPI double qg_get_run(void);
/**
 * @brief 프레임 당 시간을 얻는다
 * @return 리퍼런스 시간
 * @note 포즈 중에도 이 시간은 계산된다
*/
QSAPI double qg_get_reference(void);
/**
 * @brief 수행 시간을 얻는다. 포즈 중에는 0
 * @return 수행 시간
 * @note 포즈 중에는 계산되지 않으므로 0이다
*/
QSAPI double qg_get_advance(void);
/**
 * @brief 대기 상태(IDLE)일 때 대기할 밀리초
 * @return 대기 상태에서의 대기 밀리초
*/
QSAPI int qg_get_delay(void);
/**
 * @brief 대기 상태(IDLE)일 때 대기할 밀리초를 설정한다
 * @param delay 대기 상태에서의 대기 밀리초
*/
QSAPI void qg_set_delay(int delay);

/**
 * @brief 남은 이벤트 갯수를 얻는다
 * @return 남은 이벤트 갯수
*/
QSAPI int qg_left_events(void);
/**
 * @brief 이벤트를 모두 지운다
 */
QSAPI void qg_flush_event(void);
/**
 * @brief 이벤트를 추가한다
 * @param[in] ev 이벤트 정보
 * @return 총 이벤트 갯수
*/
QSAPI int qg_add_event(const QgEvent* ev);
/**
 * @brief 이벤트를 추가하지만. 데이타는 없이 이벤트 종류만 추가한다
 * @param type 이벤트 종류
 * @return 총 이벤트 갯수
*/
QSAPI int qg_add_event_type(QgEventType type);
/**
 * @brief 맨 앞 이벤트를 꺼낸다. 꺼내면서 해당 이벤트는 큐에서 삭제
 * @param[in] ev 얻은 이벤트 정보
 * @return 이벤트가 있었다면 참. 없으면 거짓
*/
QSAPI bool qg_pop_event(QgEvent* ev);
/**
 * @brief 이벤트를 문자열로
 * @param ev 이벤트
 * @return 이벤트 문자열
*/
QSAPI const char* qg_event_str(QgEventType ev);
/**
 * @brief 윈도우 이벤트를 문자열로
 * @param wev 윈도우 이벤트
 * @return 윈도우 이벤트 문자열
*/
QSAPI const char* qg_window_event_str(QgWindowEventType wev);



//////////////////////////////////////////////////////////////////////////
// render device

/** @brief 렌더러 디바이스 */
struct QgRdh
{
	QmGam				base;

	QgDeviceInfo		caps;

	QgRenderTm			tm;
	QgRenderParam		param;
	QgRenderInvoke		invokes;
};

qv_name(QgRdh)
{
	qv_name(QmGam)	base;
	void (*reset)(QgRdh*);
	void (*clear)(QgRdh*, int, const QnColor*, int, float);

	bool (*begin)(QgRdh*, bool);
	void (*end)(QgRdh*);
	void (*flush)(QgRdh*);

	QgBuffer* (*create_buffer)(QgRdh*, QgBufType, int, int, const void*);
	QgRender* (*create_render)(QgRdh*, const QgPropRender*, bool);

	bool (*set_index)(QgRdh*, QgBuffer*);
	bool (*set_vertex)(QgRdh*, QgLoStage, QgBuffer*);
	void (*set_render)(QgRdh*, QgRender*);

	bool (*draw)(QgRdh*, QgTopology, int);
	bool (*draw_indexed)(QgRdh*, QgTopology, int);
	bool (*ptr_draw)(QgRdh*, QgTopology, int, int, const void*);
	bool (*ptr_draw_indexed)(QgRdh*, QgTopology, int, int, const void*, int, int, const void*);
};

/**
 * @brief 렌더러를 만든다
 * @param driver 드라이버 이름 (NULL로 지정하여 기본값)
 * @param title 윈도우 타이틀
 * @param width 윈도우 너비
 * @param height 윈도우 높이
 * @param flags 스터브 및 렌더러 플래그
 * @return 만들어진 랜더러
 * @retval NULL 랜더러를 만들 수가 없다
 * @note 내부에서 qg_open_stub 함수를 호출한다
*/
QSAPI QgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags);

/**
 * @brief 루프를 시작한다
 * @param g 렌더러
 * @return 루프 확인 결과
 * @retval true 루프를 진행한다
 * @retval false 프로그램의 종료
 * @note 내부에서 qg_loop 함수를 호출한다
*/
QSAPI bool qg_rdh_loop(QgRdh* g);
/**
 * @brief 이벤트를 폴링한다
 * @param g 렌더러
 * @param ev 이벤트 결과
 * @return 폴링 결과
 * @retval true 이벤트가 남아있다
 * @retval false 처리할 이벤트가 없다
 * @note 내부에서 qg_poll 함수를 호출한다
*/
QSAPI bool qg_rdh_poll(QgRdh* g, QgEvent* ev);
/**
 * @brief 루프를 종료한다. 자세한 사항은 qg_exit_loop 함수 참조
 * @param g 렌더러
*/
QSAPI void qg_rdh_exit_loop(QgRdh* g);

/**
 * @brief 렌더러를 준비한다
 * @param g 렌더러
 * @param clear 배경, 스텐실, 뎁스를 초기화 하려면 true 로 한다
 * @return 렌더러가 준비됐으면 참
 * @retval true 렌더러가 준비됐다
 * @retval false 렌더러가 준비되지 않았거나, 프로그램이 종료했다
*/
QSAPI bool qg_rdh_begin(QgRdh* g, bool clear);
/**
 * @brief 렌더러를 끝낸다
 * @param g 렌더러
*/
QSAPI void qg_rdh_end(QgRdh* g);
/**
 * @brief 렌더러 결과를 화면으로 출력한다
 * @param g 렌더러
*/
QSAPI void qg_rdh_flush(QgRdh* g);

/**
 * @brief 렌더러 상태를 초기화 한다
 * @param g 렌더러
 * @note 이 함수는 직접 호출하지 않아도 좋다. 외부 스터브 사용할 때 화면 크기가 바뀔 때 사용하면 좋음
*/
QSAPI void qg_rdh_reset(QgRdh* g);
/**
 * @brief 렌더러를 지운다
 * @param g 렌더러
 * @param clear 지우기 플래그
 * @param color 배경색
 * @param stencil 스텐실 값
 * @param depth 뎁스 값
*/
QSAPI void qg_rdh_clear(QgRdh* g, QgClear clear, const QnColor* color, int stencil, float depth);

/**
 * @brief 세이더 vec3 타입 파라미터 설정
 * @param g 렌더러
 * @param at 0부터 3까지 총 4가지
 * @param v vec3 타입 값
 * @note 내부에서 vec4 타입으로 처리한다
 * @see qg_rdh_set_param_vec4
*/
QSAPI void qg_rdh_set_param_vec3(QgRdh* g, int at, const QnVec3* v);
/**
 * @brief 세이더 vec4 타입 파라미터 설정
 * @param g 렌더러
 * @param at 0부터 3까지 총 4가지
 * @param v vec4 타입 값
*/
QSAPI void qg_rdh_set_param_vec4(QgRdh* g, int at, const QnVec4* v);
/**
 * @brief 세이더 mat4 타입 파라미터 설정
 * @param g 렌더러
 * @param at 0부터 3까지 총 4가지
 * @param m mat4 타입 값
*/
QSAPI void qg_rdh_set_param_mat4(QgRdh* g, int at, const QnMat4* m);
/**
 * @brief 세이더 영향치(주로 뼈대 팔레트) 파라미터 설정
 * @param g 렌더러
 * @param count 행렬 갯수
 * @param weight 영향치 행렬
*/
QSAPI void qg_rdh_set_param_weight(QgRdh* g, int count, QnMat4* weight);
/**
 * @brief 배경색을 설정한다
 * @param g 렌더러
 * @param background_color 배경색
*/
QSAPI void qg_rdh_set_background(QgRdh* g, const QnColor* background_color);
/**
 * @brief 월드 행렬을 설정한다
 * @param g 렌더러
 * @param world 월드 행렬
*/
QSAPI void qg_rdh_set_world(QgRdh* g, const QnMat4* world);
/**
 * @brief 뷰 행렬을 설정한다
 * @param g 렌더러
 * @param view 뷰 행렬
*/
QSAPI void qg_rdh_set_view(QgRdh* g, const QnMat4* view);
/**
 * @brief 투영 행렬을 설정한다
 * @param g 렌더러
 * @param proj 투영 행렬
*/
QSAPI void qg_rdh_set_project(QgRdh* g, const QnMat4* proj);
/**
 * @brief 뷰와 투영 행렬을 설정한다
 * @param g 렌더러
 * @param proj 투영 행렬
 * @param view 뷰 행렬
*/
QSAPI void qg_rdh_set_view_project(QgRdh* g, const QnMat4* proj, const QnMat4* view);

/**
 * @brief 버퍼를 만든다
 * @param g 렌더러
 * @param type 버퍼 타입
 * @param count 요소 개수
 * @param stride 요소의 너비
 * @param data 초기화할 요소 데이터로 이 값이 NULL이면 동적 버퍼, 값이 있으면 정적 버퍼로 만들어진다
 * @return 만들어진 버퍼
 * @details data 파라미터의 설명에도 있지만, 정적 버퍼로 만들고 나중에 데이터를 넣으면 문제가 생길 수도 있다
*/
QSAPI QgBuffer* qg_rdh_create_buffer(QgRdh* g, QgBufType type, int count, int stride, const void* data);
/**
 * @brief 렌더 파이프라인을 만든다
 * @param g 렌더러
 * @param prop 렌더 파이프라인 속성
 * @param compile_shader 속성에 있는 세이더의 값이 소스임을 지정, 내부에서 컴파일을 지시
 * @return 만들어진 렌더 파이프라인
*/
QSAPI QgRender* qg_rdh_create_render(QgRdh* g, const QgPropRender* prop, bool compile_shader);

/**
 * @brief 인덱스 버퍼를 설정한다
 * @param g 렌더러
 * @param buffer 설정할 버퍼
 * @return 실패하면 거짓을 반환
 * @retval true 문제 없이 인덱스 버퍼를 설정했다
 * @retval false buffer 인수에 문제가 있거나 인덱스 버퍼가 아니다
*/
QSAPI bool qg_rdh_set_index(QgRdh* g, QgBuffer* buffer);
/**
 * @brief 정점 버퍼를 설정한다
 * @param g 렌더러
 * @param stage 버퍼를 지정할 스테이지
 * @param buffer 설정할 버퍼
 * @return 실패하면 거짓을 반환
 * @retval true 문제 없이 정점 버퍼를 설정했다
 * @retval false buffer 인수에 문제가 있거나 정점 버퍼가 아니다
*/
QSAPI bool qg_rdh_set_vertex(QgRdh* g, QgLoStage stage, QgBuffer* buffer);
/**
 * @brief 렌더 파이프라인을 설정한다
 * @param g 렌더러
 * @param render 렌더 파이프라인
*/
QSAPI void qg_rdh_set_render(QgRdh* g, QgRender* render);

/**
 * @brief 정점으로 그리기
 * @param g 렌더러
 * @param tpg 그릴 모양의 토폴로지
 * @param vertices 정점 갯수
 * @return 문제없이 그리면 참
*/
QSAPI bool qg_rdh_draw(QgRdh* g, QgTopology tpg, int vertices);
/**
 * @brief 인덱스와 정점으로 그리기
 * @param g 렌더러
 * @param tpg 그릴 모양의 토폴로지
 * @param indices 그릴 인덱스 갯수
 * @return 문제없이 그리면 참
*/
QSAPI bool qg_rdh_draw_indexed(QgRdh* g, QgTopology tpg, int indices);
/**
 * @brief 사용자 지정 정점 데이터로 그리기
 * @param g 렌더러
 * @param tpg 그릴 모양의 토폴로지
 * @param vertices 정점 갯수
 * @param vertex_stride 정점 너비
 * @param vertex_data 정점 데이터
 * @return 문제없이 그리면 참
*/
QSAPI bool qg_rdh_ptr_draw(QgRdh* g, QgTopology tpg, int vertices, int vertex_stride, const void* vertex_data);
/**
 * @brief 사용자 지정 인덱스와 정점 데이터로 그리기
 * @param g 렌더러
 * @param tpg 그릴 모양의 토폴로지
 * @param vertices 정점 갯수
 * @param vertex_stride 정점 너비
 * @param vertex_data 정점 데이터
 * @param indices 인덱스 갯수
 * @param index_stride 인덱스 너비
 * @param index_data 인덱스 데이터
 * @return 문제없이 그리면 참
*/
QSAPI bool qg_rdh_ptr_draw_indexed(QgRdh* g, QgTopology tpg,
	int vertices, int vertex_stride, const void* vertex_data,
	int indices, int index_stride, const void* index_data);


//////////////////////////////////////////////////////////////////////////
// 버퍼

/** @brief 버퍼 */
struct QgBuffer
{
	QmGam				base;

	QgBufType			type;
	uint				size;
	uint				count;
	ushort				stride;
	ushort				mapped;
};

qv_name(QgBuffer)
{
	qv_name(QmGam)		base;
	void*(*map)(QgBuffer*);
	bool (*unmap)(QgBuffer*);
	bool (*data)(QgBuffer*, const void*);
};

/**
 * @brief 버퍼 설정을 위해 잠근다
 * @param g 버퍼
 * @return 버퍼 데이터 설정을 위한 포인터
 * @retval NULL 버퍼를 잠글 수 없다
*/
QSAPI void* qg_buf_map(QgBuffer* g);
/**
 * @brief 잠궛던 버퍼를 푼다
 * @param g 버퍼
 * @return 버퍼 설정에 성공하면 참
*/
QSAPI bool qg_buf_unmap(QgBuffer* g);
/**
 * @brief 버퍼에 데이터를 설정한다
 * @param g 버퍼
 * @param data 설정할 데이터
 * @return 버퍼에 데이터가 들어갔으면 참
 * @note data 는 반드시 size 만큼 데이터를 갖고 있어야한다
*/
QSAPI bool qg_buf_data(QgBuffer* g, const void* data);

#if 0
// render device


//////////////////////////////////////////////////////////////////////////
// object

//
struct QgGam
{
	QmGam				base;
};

// Depth Stencil
struct QgDsm
{
	QgGam				base;
};

// Rasterize
struct QgRsz
{
	QgGam				base;
};

// layout
struct QgVlo
{
	QgGam				base;

	ushort				stride[QGLOS_MAX_VALUE];
};

QSAPI uint qg_vlo_get_stride(QgVlo* g, QgLoStage stage);


// shader
struct QgShd
{
	QgGam				base;

	char				name[64];

	QgVarShaderFunc		intr_func;
	void*				intr_data;
};

qvt_name(QgShd)
{
	qvt_name(QmGam)		base;
	bool(*bind)(QgShd*, QgShdType, const void*, int, int);
	bool(*bind_shd)(QgShd*, QgShdType, QgShd*);
	//bool(*bind_name)(QgShd* QgShdType, const char*);
	//void(*add_condition)(QgShd*, const char*);
	//void(*clear_condition)(QgShd*);
	bool(*link)(QgShd*);
};

QSAPI const char* qg_shd_get_name(QgShd* self);
QSAPI void qg_shd_set_intr(QgShd* self, QgVarShaderFunc func, void* data);

QSAPI bool qg_shd_bind(QgShd* self, QgShdType type, const void* data, int size, int flags);
QSAPI bool qg_shd_bind_shd(QgShd* self, QgShdType type, QgShd* shader);
QSAPI bool qg_shd_bind_file(QgShd* self, QgShdType type, const char* filename, int flags);
QSAPI bool qg_shd_link(QgShd* self);


// buffer
#endif

QN_EXTC_END
