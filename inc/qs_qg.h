/*
 * QsLib <QG Layer>
 * Made by kim 2004-2024
 *
 * 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
 * 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
 */
/**
 * @file qs_qg.h
 *
 * <QG Layer>는 공통 그래픽 구현을 목표로 합니다.
 */
#pragma once

#include <qs_qn.h>
#include <qs_math.h>
#include <qs_kmc.h>

QN_EXTC_BEGIN

//
typedef struct QgRdh		QgRdh;							/** @Brief 렌더러 */
typedef struct QgGam		QgGam;							/** @Brief 런더 감 */
typedef struct QgDsm		QgDsm;							/** @Brief 뎁스 스텐실 */
typedef struct QgRsz		QgRsz;							/** @Brief 래스터라이저 */
typedef struct QgBld		QgBld;							/** @Brief 블렌드 */
typedef struct QgSpr		QgSpr;							/** @Brief 샘플러 */
typedef struct QgShd		QgShd;							/** @Brief 세이더 */
typedef struct QgVlo		QgVlo;							/** @Brief 정점 레이아웃 */
typedef struct QgBuf		QgBuf;							/** @Brief 버퍼 */


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

/** @brief 스터브와 렌더러 플래그 */
typedef enum QgFlag
{
	QGFLAG_FULLSCREEN = QN_BIT(0),							/** @brief 전체 화면 */
	QGFLAG_BORDERLESS = QN_BIT(1),							/** @brief 테두리 없는 윈도우 */
	QGFLAG_RESIZABLE = QN_BIT(2),							/** @brief 크기 변경할 수 있음 */
	QGFLAG_FOCUS = QN_BIT(3),								/** @brief 입력 포커스 받을 수 있음 */
	QGFLAG_IDLE = QN_BIT(4),								/** @brief 활성 중이 아니면 대기 */
	QGFLAG_VSYNC = QN_BIT(16),								/** @brief VSYNC 켜기 */
	QGFLAG_DITHER = QN_BIT(17),								/** @brief 16비트 모드 사용 */
	QGFLAG_MSAA = QN_BIT(18),								/** @brief 멀티 샘플링 사용 */
} QgFlag;

/** @brief 스터브 상태 */
typedef enum QgStubStat
{
	QGSTTI_VIRTUAL = QN_BIT(1),								/** @brief 가상 스터브 사용중 */
	QGSTTI_ACTIVE = QN_BIT(2),								/** @brief 스터브가 활성 상태 */
	QGSTTI_LAYOUT = QN_BIT(3),								/** @brief 스터브 크기를 변경 */
	QGSTTI_ACS = QN_BIT(13),								/** @brief 윈도우 도움 기능 켬 */
	QGSTTI_PAUSE = QN_BIT(14),								/** @brief 포즈 중 */
	QGSTTI_DROP = QN_BIT(15),								/** @brief 드래그 드랍 중 */
	QGSTTI_CURSOR = QN_BIT(16),								/** @brief 커서의 표시 */
	QGSTTI_SCRSAVE = QN_BIT(17),							/** @brief 스크린 세이브 상태 */
	QGSTTI_EXIT = QN_BIT(30),								/** @brief (사용안함) 끝내기 */
} QgStubStat;

/** @brief 이벤트 타입 */
typedef enum QgEventType
{
	QGEV_NONE,												/** @brief 이벤트 없음 */
	QGEV_ACTIVE,											/** @brief 스터브 활성 상태 이벤트 */
	QGEV_LAYOUT,											/** @brief 화면 크기 이벤트 */
	QGEV_KEYDOWN,											/** @brief 키보드를 눌렀어요 */
	QGEV_KEYUP,												/** @brief 키보드에서 손을 뗐어요 */
	QGEV_MOUSEMOVE,											/** @brief 마우스를 움직여요 */
	QGEV_MOUSEDOWN,											/** @brief 마우스 버튼을 눌렀어요 */
	QGEV_MOUSEUP,											/** @brief 마우스 버튼을 뗏어요 */
	QGEV_MOUSEWHEEL,										/** @brief 마우스 휠을 굴렸어요 */
	QGEV_MOUSEDOUBLE,										/** @brief 마우스 버튼을 두번 눌렀어요 */
	QGEV_EXIT = 255,										/** @brief (사용안함) 끝내기 */
} QgEventType;

/** @brief 최대 이벤트 갯수 */
#define QGMAX_EVENTS		5000


//////////////////////////////////////////////////////////////////////////
// properties

/** @brief 픽셀 포맷 */
typedef struct QgPropPixel
{
	QgClrFmt		fmt;									/** @brief 픽셀 포맷 */
	uint			bpp;									/** @brief 픽셀 당 비트 수 */
	byte			rr, rl;									/** @brief 빨강 */
	byte			gr, gl;									/** @brief 녹색 */
	byte			br, bl;									/** @brief 파랑 */
	byte			ar, al;									/** @brief 알파 */
} QgPropPixel;

/** @brief 코드 데이터 */
typedef struct QgCodeData
{
	size_t			size;									/** @brief code 길이 */
	const void*		code;									/** @brief 코드 데이터 포인터 */
} QgCodeData;

/** @brief 레이아웃 요소 */
typedef struct QgLayoutInput
{
	QgLoStage		stage;									/** @brief 스테이지 구분 */
	int				index;									/** @brief 스테이지에서 인덱스 */
	QgLoUsage		usage;									/** @brief 사용법 */
	QgLoType		type;									/** @brief 데이터 타입 */
} QgLayoutInput;

/** @brief 레이아웃 */
typedef struct QgPropLayout
{
	size_t			count;									/** @brief 요소 갯수 */
	QgLayoutInput*	elements;								/** @brief 요소 데이터 포인트 */
} QgPropLayout;

/** @brief 블렌드 */
typedef struct QgPropBlend
{
	BOOL			use_coverage;							/** @brief sample coverage / alpha to coverage */
	BOOL			separate;								/** @brief 거짓이면 rb[0]만 사용 */
	QgBlend			rb[QGRVS_MAX_VALUE];					/** @brief 스테이지 별 블렌드 정보 */
} QgPropBlend;

/** @brief 래스터라이저 */
typedef struct QgPropRaster
{
	QgFill			fill;									/** @brief 채우기 방법 */
	QgCull			cull;									/** @brief 면 제거 방법 */
	float			depth_bias;								/** @brief 주어진 픽셀에 추가하는 깊이 값 */
	float			slope_scale;							/** @brief 픽셀 경사면에 주어지는 스칼라 값 */
} QgPropRaster;

/** @brief 렌더 포맷 */
typedef struct QgPropRvFormat
{
	uint			count;									/** @brief rtv 갯수 */
	QgClrFmt		rtv[QGRVS_MAX_VALUE];					/** @brief Render Target (color) Value */
	QgClrFmt		dsv;									/** @brief Depth Stencil (color) Value */
} QgPropRvFormat;

/** @brief 파이프라인 */
typedef struct QgRenderPipeline
{
	QgCodeData		vs;										/** @brief 정점 세이더 데이터 */
	QgCodeData		ps;										/** @brief 픽셀 세이더 데이터 */
	QgPropBlend		blend;									/** @brief 블렌드 */
	QgPropRaster	rasterizer;								/** @brief 래스터라이저 */
	QgDepth			depth;									/** @brief 뎁스 */
	QgStencil		stencil;								/** @brief 스텐실 */
	QgPropLayout	layout;									/** @brief 정점 레이아웃 */
	QgPropRvFormat	format;									/** @brief 렌더 포맷 */
	QgTopology		topology;								/** @brief 토폴로지 */
	uint			__mask;									/** @brief 용도 못 찾음 */
} QgRenderPipeline;

/** @brief 세이더 변수 */
typedef struct QgVarShader
{
	char				name[64];							/** @brief 변수 이름 */

	uint				offset;								/** @brief 변수 옵셋 */
	ushort				size;								/** @brief 변수의 크기 */

	ushort				manual;								/** @brief 수동 모드 여부 */
	QgShdConstType		type;								/** @brief 변수 타입 */
} QgVarShader;

/** @brief 세이더 변수 콜백 함수 */
typedef void(*QgVarShaderFunc)(void*, QgVarShader*, QgShd*);

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
		QnRect				bound;							/** @brief 변경된 윈도우 크기 */
	}					layout;								/** @brief 레이아웃 이벤트 */
	struct QgEventKeyboard
	{
		QgEventType			ev;
		QikKey				key;							/** @brief 이벤트에 해당하는 키 */
		QikMask				state;							/** @brief 특수키 상태 */
		BOOL				pressed;						/** @brief 눌려 있었다면 참 */
		BOOL				repeat;							/** @brief 계속 눌려 있었다면 참 */
	}					key;								/** @brief 키 눌림 떼임 이벤트 */
	struct QgEventMouseMove
	{
		QgEventType			ev;
		QnPoint				pt;								/** @brief 마우스 좌표 */
		QnPoint				delta;							/** @brief 마우스 이동 거리 */
		QimMask				state;							/** @brief 마우스 버튼의 상태 */
	}					mmove;								/** @brief 마우스 이동 이벤트 */
	struct QgEventMouseButton
	{
		QgEventType			ev;
		QnPoint				pt;								/** @brief 마우스 좌표 */
		QimButton			button;							/** @brief 이벤트에 해당하는 버튼 */
		QimMask				state;							/** @brief 마으스 버튼의 상태 */
	}					mbutton;							/** @brief 마우스 버튼 눌림 떼임 이벤트 */
	struct QgEventMouseWheel
	{
		QgEventType			ev;
		QnPoint				pt;								/** @brief 마우스 좌표 */
		int					dir;							/** @brief 휠 방향, 0보다 작으면 아래로 크면 위로 */
	}					mwheel;								/** @brief 마우스 휠 이벤트 */
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
	int					wheel;								/** @brief 마우스 휠 상태 */
	QnPoint				pt;									/** @brief 마우스 좌표 */
	QnPoint				last;								/** @brief 마우스의 이전 좌표 */

	struct				QgUimMouseClick
	{
		uint				tick;							/** @brief 첫번째 눌렸을 때 */
		uint				ltick;							/** @brief 마지막으로 두번 눌렸을 때 */
		QimButton			btn;							/** @brief 두번 검새 때 눌린 마우스 버튼 */
		QnPoint				loc;							/** @brief 두번 검사 때 마우스 위치 */
	}					clk;								/** @brief 마우스 눌림 정보 */
	struct				QgUimMouseLimit
	{
		int					move;							/** @brief 제한 이동 거리(포인트)의 제곱 */
		uint				tick;							/** @brief 제한 클릭 시간(밀리초) */
	}					lim;								/** @brief 마우스 더블 클릭 구현 정보 */
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
	int					max_tex_dim;						/** @brief 최대 텍스쳐 크기 */
	int					max_tex_count;						/** @brief 최대 텍스쳐 갯수 */
	int					max_off_count;						/** @brief 최대 오프 텍스쳐(=렌더타겟/프레임버퍼) 갯수 */
	int					tex_image_flag;						/** @brief 텍스쳐 이미지 플래그 */
	QgClrFmt			clr_fmt;							/** @brief 색깔 포맷 */
} QgDeviceInfo;

/** @brief 렌더러 추적 정보 */
typedef struct QgRenderInvoke
{
	uint				invokes;
	uint				begins;
	uint				ends;
	uint				shaders;
	uint				params;
	uint				transforms;
	uint				draws;
	uint				primitives;

	uint				frames;
	BOOL				flush;
} QgRenderInvoke;

/** @brief 렌더러 트랜스 포매이션 */
typedef struct QgRenderTm
{
	QnSize				size;
	QnDepth				depth;
	QnMat4				world;
	QnMat4				view;
	QnMat4				project;
	QnMat4				view_project;
	QnMat4				inv;								/** @brief inverse view */
	QnMat4				ortho;								/** @brief ortho transform */
	QnMat4				frm;								/** @brief tex formation */
	QnMat4				tex[4];
} QgRenderTm;

/** @brief 렌더러 인수 */
typedef struct QgRenderParam
{
	QnColor				bgc;
	QnVec4				v[4];
	QnMat4				m[4];
	QnMat4*				bone_ptr;
	int					bone_count;

	QN_PADDING_32(8, 0)
	QN_PADDING_64(4, 0)
} QgRenderParam;


//////////////////////////////////////////////////////////////////////////
// stub & render device

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

#if 0
// render device
struct QgRdh
{
	QmGam				base;

	QgDeviceInfo		caps;

	QgRenderTm			tm;
	QgRenderParam		param;
	QgRenderInvoke		invokes;
};

qvt_name(QgRdh)
{
	qvt_name(QmGam)		base;
	void (*reset)(QgRdh*);
	void (*clear)(QgRdh*, int, const QnColor*, int, float);

	bool (*begin)(QgRdh*, bool);
	void (*end)(QgRdh*);
	void (*flush)(QgRdh*);

	QgVlo* (*create_layout)(QgRdh*, int, const QgLayoutElement*);
	QgShd* (*create_shader)(QgRdh*, const char*);
	QgBuf* (*create_buffer)(QgRdh*, QgBufType, int, int, const void*);
	QgDsm* (*create_depth_stencil)(QgRdh*, const QgDepthStencilProp*);
	QgRsz* (*create_rasterizer)(QgRdh*, const QgRasterizerProp*);

	void (*set_shader)(QgRdh*, QgShd*, QgVlo*);
	bool (*set_index)(QgRdh*, QgBuf*);
	bool (*set_vertex)(QgRdh*, QgLoStage, QgBuf*);
	bool (*set_depth_stencil)(QgRdh*, QgDsm*);
	bool (*set_rasterizer)(QgRdh*, QgRsz*);

	bool (*draw)(QgRdh*, QgTopology, int);
	bool (*draw_indexed)(QgRdh*, QgTopology, int);
	bool (*ptr_draw)(QgRdh*, QgTopology, int, int, const void*);
	bool (*ptr_draw_indexed)(QgRdh*, QgTopology, int, int, const void*, int, int, const void*);
};

QSAPI QgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags);

QSAPI const QgDeviceInfo* qg_rdh_get_device_info(QgRdh* g);
QSAPI const QgRenderInvoke* qg_rdh_get_render_invokes(QgRdh* g);
QSAPI const QgRenderTm* qg_rdh_get_render_tm(QgRdh* g);
QSAPI const QgRenderParam* qg_rdh_get_render_param(QgRdh* g);

QSAPI bool qg_rdh_loop(QgRdh* g);
QSAPI bool qg_rdh_poll(QgRdh* g, QgEvent* ev);
QSAPI void qg_rdh_exit_loop(QgRdh* self);

QSAPI bool qg_rdh_begin(QgRdh* g, bool clear);
QSAPI void qg_rdh_end(QgRdh* g);
QSAPI void qg_rdh_flush(QgRdh* g);

QSAPI void qg_rdh_reset(QgRdh* self);
QSAPI void qg_rdh_clear(QgRdh* self, QgClear clear, const QnColor* color, int stencil, float depth);

QSAPI void qg_rdh_set_param_vec3(QgRdh* g, int at, const QnVec3* v);
QSAPI void qg_rdh_set_param_vec4(QgRdh* g, int at, const QnVec4* v);
QSAPI void qg_rdh_set_param_mat4(QgRdh* g, int at, const QnMat4* m);
QSAPI void qg_rdh_set_param_weight(QgRdh* g, int count, QnMat4* weight);
QSAPI void qg_rdh_set_background(QgRdh* g, const QnColor* background_color);
QSAPI void qg_rdh_set_view_proj(QgRdh* g, const QnMat4* proj, const QnMat4* view);
QSAPI void qg_rdh_set_proj(QgRdh* g, const QnMat4* proj);
QSAPI void qg_rdh_set_view(QgRdh* g, const QnMat4* view);
QSAPI void qg_rdh_set_world(QgRdh* g, const QnMat4* workd);

QSAPI QgVlo* qg_rdh_create_layout(QgRdh* self, int count, const QgLayoutElement* layouts);
QSAPI QgShd* qg_rdh_create_shader(QgRdh* self, const char* name);
QSAPI QgBuf* qg_rdh_create_buffer(QgRdh* g, QgBufType type, int count, int stride, const void* data);
QSAPI QgDsm* qg_rdh_create_depth_stencil(QgRdh* rdh, const QgDepthStencilProp* prop);
QSAPI QgRsz* qg_rdh_create_rasterizer(QgRdh* rdh, const QgRasterizerProp* prop);

QSAPI void qg_rdh_set_shader(QgRdh* self, QgShd* shader, QgVlo* layout);
QSAPI bool qg_rdh_set_index(QgRdh* g, QgBuf* buffer);
QSAPI bool qg_rdh_set_vertex(QgRdh* g, QgLoStage stage, QgBuf* buffer);
QSAPI bool qg_rdh_set_depth_stencil(QgRdh* self, QgDsm* depth_stencil);
QSAPI bool qg_rdh_set_rasterizer(QgRdh* self, QgRsz* rasterizer);

QSAPI bool qg_rdh_draw(QgRdh* self, QgTopology tpg, int vertices);
QSAPI bool qg_rdh_draw_indexed(QgRdh* self, QgTopology tpg, int indices);
QSAPI bool qg_rdh_ptr_draw(QgRdh* self, QgTopology tpg, int vertices, int vertex_stride, const void* vertex_data);
QSAPI bool qg_rdh_ptr_draw_indexed(QgRdh* self, QgTopology tpg,
	int vertices, int vertex_stride, const void* vertex_data,
	int indices, int index_stride, const void* index_data);


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
struct QgBuf
{
	QgGam				base;

	QgBufType			type : 16;
	ushort				stride;
	int					size;
};

qvt_name(QgBuf)
{
	qvt_name(QmGam)		base;
	void*(*map)(QgBuf*);
	bool (*unmap)(QgBuf*);
	bool (*data)(QgBuf*, const void*);
};

QSAPI QgBufType qg_buf_get_type(QgBuf* g);
QSAPI ushort qg_buf_get_stride(QgBuf* g);
QSAPI int qg_buf_get_size(QgBuf* g);

QSAPI void* qg_buf_map(QgBuf* self);
QSAPI bool qg_buf_unmap(QgBuf* self);
QSAPI bool qg_buf_data(QgBuf* self, const void* data);
#endif

QN_EXTC_END
