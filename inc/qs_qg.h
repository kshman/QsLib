//
// QsLib [QG Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once
#define __QS_QG__

#include <qs_qn.h>
#include <qs_math.h>

//////////////////////////////////////////////////////////////////////////
// property
#define QG_PROP_WINDOWS_ICON			"QG_PROP_WINDOWS_ICON"
#define QG_PROP_WINDOWS_SMALLICON		"QG_PROP_WINDOWS_SMALLICON"
#define QG_PROP_DRIVER_MAJOR			"QG_PROP_DRIVER_MAJOR"
#define QG_PROP_DRIVER_MINOR			"QG_PROP_DRIVER_MINOR"
#define QG_PROP_CAPABILITY				"QG_PROP_CAPABILITY"
#define QG_PROP_RGBA_SIZE				"QG_PROP_RGBA_SIZE"
#define QG_PROP_DEPTH_SIZE				"QG_PROP_DEPTH_SIZE"
#define QG_PROP_STENCIL_SIZE			"QG_PROP_STENCIL_SIZE"
#define QG_PROP_MSAA					"QG_PROP_MSAA"


//////////////////////////////////////////////////////////////////////////
// typedef

typedef struct QGBUFFER			QgBuffer;						/// @brief 버퍼
typedef struct QGRENDERSTATE	QgRenderState;					/// @brief 렌더 파이프라인
typedef struct QGTEXTURE		QgTexture;						/// @brief 텍스쳐
typedef struct QGIMAGE			QgImage;						/// @brief 이미지
typedef struct QGFONT			QgFont;							/// @brief 폰트
typedef struct QGDPCT			QgDpct;							/// @brief 노드
typedef struct QGRAY			QgRay;							/// @brief 레이
typedef struct QGCAMERA			QgCamera;						/// @brief 카메라
typedef struct QGMESH			QgMesh;							/// @brief 메시


//////////////////////////////////////////////////////////////////////////
// types

/// @brief 색깔 포맷
/// @note 항목을 변경하면 qg_clrfmt_to_str() 함수도 고칠 것
typedef enum QGCLRFMT
{
	QGCF_UNKNOWN,

	QGCF_R32G32B32A32F,										/// @brief 128비트 실수 RGBA
	QGCF_R32G32B32F,										/// @brief 96비트 실수 RGB
	QGCF_R32G32F,											/// @brief 64비트 실수 RG
	QGCF_R32F,												/// @brief 32비트 실수 R

	QGCF_R32G32B32A32,										/// @brief 128비트 정수 RGBA
	QGCF_R32G32B32,											/// @brief 96비트 정수 RGB
	QGCF_R32G32,											/// @brief 64비트 정수 RG
	QGCF_R32,												/// @brief 32비트 정수 R

	QGCF_R16G16B16A16F,										/// @brief 64비트 실수 RGBA
	QGCF_R16G16F,											/// @brief 32비트 실수 RG
	QGCF_R16F,												/// @brief 16비트 실수 R
	QGCF_R11G11B10F,										/// @brief 32비트 실수 RGB

	QGCF_R16G16B16A16,										/// @brief 64비트 정수 RGBA
	QGCF_R16G16,											/// @brief 32비트 정수 RG
	QGCF_R16,												/// @brief 16비트 정수 R
	QGCF_R10G10B10A2,										/// @brief 32비트 정수 RGBA

	QGCF_R8G8B8A8,											/// @brief 32비트 정수 RGBA
	QGCF_R8G8B8,											/// @brief 24비트 정수 RGB
	QGCF_R8G8,												/// @brief 16비트 정수 RG
	QGCF_R8,												/// @brief 8비트 정수 R
	QGCF_A8,												/// @brief 8비트 정수 A (표면 전용)
	QGCF_L8,												/// @brief 8비트 정수 L (표면 전용)
	QGCF_A8L8,												/// @brief 16비트 정수 AL (표면 전용)

	QGCF_R5G6B5,											/// @brief 16비트 RGB (표면 전용)
	QGCF_R5G5B5A1,											/// @brief 16비트 RGBA (표면 전용)
	QGCF_R4G4B4A4,											/// @brief 16비트 RGBA (표면 전용)

	QGCF_D32F,												/// @brief 32비트 실수 뎁스 (뎁스 전용)
	QGCF_D24S8,												/// @brief 32비트 뎁스 24 스텐실 8 (뎁스 스텐실 전용)

	QGCF_DXT1,												/// @brief DXT1 압축 포맷 (이미지 전용)
	QGCF_DXT3,												/// @brief DXT3 압축 포맷 (이미지 전용)
	QGCF_DXT5,												/// @brief DXT5 압축 포맷 (이미지 전용)
	QGCF_EXT1,												/// @brief ETC1 압축 포맷 (이미지 전용)
	QGCF_EXT2,												/// @brief ETC2 압축 포맷 (이미지 전용)
	QGCF_EXT2_EAC,											/// @brief ETC2 EAC 압축 포맷 (이미지 전용)
	QGCF_ASTC4,												/// @brief ASTC 4x4 압축 포맷 (이미지 전용)
	QGCF_ASTC8,												/// @brief ASTC 8x8 압축 포맷 (이미지 전용)

	QGCF_MAX_VALUE,
} QgClrFmt;

// 좌표 타입
#define QGCF_X32Y32Z32W32F	QGCF_R32G32B32A32F
#define QGCF_X32Y32Z32F		QGCF_R32G32B32F
#define QGCF_X32Y32F		QGCF_R32G32F
#define QGCF_X32F			QGCF_R32F
#define QGCF_X16Y16Z16W16F	QGCF_R16G16B16A16F
#define QGCF_X16Y16F		QGCF_R16G16F
#define QGCF_U32V32F		QGCF_R32G32F
#define QGCF_U16V16F		QGCF_R16G16F
#define QGCF_X8Y8Z8W8		QGCF_R8G8B8A8
#define QGCF_X8Y8			QGCF_R8G8

// 레이아웃 타입
#define QGLOT_FLOAT1		QGCF_R32F
#define QGLOT_FLOAT2		QGCF_R32G32F
#define QGLOT_FLOAT3		QGCF_R32G32B32F
#define QGLOT_FLOAT4		QGCF_R32G32B32A32F
#define QGLOT_INT1			QGCF_R32
#define QGLOT_INT2			QGCF_R32G32
#define QGLOT_INT3			QGCF_R32G32B32
#define QGLOT_INT4			QGCF_R32G32B32A32
#define QGLOT_HALF1			QGCF_R16F
#define QGLOT_HALF2			QGCF_R16G16F
#define QGLOT_HALF4			QGCF_R16G16B16A16F
#define QGLOT_SHORT1		QGCF_R16
#define QGLOT_SHORT2		QGCF_R16G16
#define QGLOT_SHORT4		QGCF_R16G16B16A16
#define QGLOT_BYTE1			QGCF_R8
#define QGLOT_BYTE2			QGCF_R8G8
#define QGLOT_BYTE3			QGCF_R8G8B8
#define QGLOT_BYTE4			QGCF_R8G8B8A8
#define QGLOT_MAX_VALUE		QGCF_D32F

// 별명
#define QCFG_BC1			QGCF_DXT1
#define QCFG_BC2			QGCF_DXT3
#define QCFG_BC3			QGCF_DXT5

/// @brief 토폴로지
typedef enum QGTOPOLOGY
{
	QGTPG_POINT,											/// @brief 점
	QGTPG_LINE,												/// @brief 선분
	QGTPG_LINE_STRIP,										/// @brief 연결된 선분
	QGTPG_TRI,												/// @brief 삼각형
	QGTPG_TRI_STRIP,										/// @brief 연결된 삼각형
	QGTPGEX_TRI_FAN,
	QGTPG_MAX_VALUE,
} QgTopology;

/// @brief 렌더 스테이지
typedef enum QGRVSTAGE
{
	QGRVS_1,												/// @brief 스테이지 1
	QGRVS_2,												/// @brief 스테이지 2
	QGRVS_3,												/// @brief 스테이지 3
	QGRVS_4,												/// @brief 스테이지 4
	QGRVS_MAX_VALUE,
} QgRvStage;

/// @brief 채우기 방식
typedef enum QGFILL
{
	QGFILL_POINT,											/// @brief 꼭짓점
	QGFILL_WIRE,											/// @brief 외곽선
	QGFILL_SOLID,											/// @brief 채우기
	QGFILL_MAX_VALUE,
} QgFill;

/// @brief 면 제거 방법
typedef enum QGCULL
{
	QGCULL_NONE,											/// @brief 몇 제거 없음
	QGCULL_FRONT,											/// @brief 앞면 제거
	QGCULL_BACK,											/// @brief 뒷면 제거
	QGCULL_BOTH,											/// @brief 양면 제거
	QGCULL_MAX_VALUE
} QgCull;

/// @brief 레이아웃 스테이지 → 정점 버퍼 인덱스
typedef enum QGLAYOUTSTAGE
{
	QGLOS_1,												/// @brief 레이아웃 1
	QGLOS_2,												/// @brief 레이아웃 2
	QGLOS_3,												/// @brief 레이아웃 3
	QGLOS_4,												/// @brief 레이아웃 4
	QGLOS_MAX_VALUE,
} QgLayoutStage;

/// @brief 레아아웃 사용 방법
typedef enum QGLAYOUTUSAGE
{
	QGLOU_UNKNOWN,											/// @brief 알 수 없음
	QGLOU_POSITION,											/// @brief XYZ 좌표
	QGLOU_COORD1,											/// @brief UV 좌표 1	
	QGLOU_COORD2,											/// @brief UV 좌표 2
	QGLOU_COORD3,											/// @brief UV 좌표 3
	QGLOU_COORD4,											/// @brief UV 좌표 4
	QGLOU_NORMAL1,											/// @brief 법선 1
	QGLOU_NORMAL2,											/// @brief 법선 2
	QGLOU_NORMAL3,											/// @brief 법선 3
	QGLOU_NORMAL4,											/// @brief 법선 4
	QGLOU_BINORMAL1,										/// @brief 종법선 1
	QGLOU_BINORMAL2,										/// @brief 종법선 2
	QGLOU_TANGENT1,											/// @brief 접선 1
	QGLOU_TANGENT2,											/// @brief 접선 2
	QGLOU_COLOR1,											/// @brief 색깔 1 (용도: D/확산광, A/주변광, S/반사광, E/방사광)
	QGLOU_COLOR2,											/// @brief 색깔 2
	QGLOU_COLOR3,											/// @brief 색깔 3
	QGLOU_COLOR4,											/// @brief 색깔 4
	QGLOU_COLOR5,											/// @brief 색깔 5
	QGLOU_COLOR6,											/// @brief 색깔 6
	QGLOU_BLEND_WEIGHT,										/// @brief 블렌드 세기
	QGLOU_BLEND_INDEX,										/// @brief 블렌드 영향 정점 인덱스
	QGLOU_BLEND_EXTRA,										/// @brief 블렌드 영향 정점 인덱스 추가
	QGLOU_MAX_VALUE
} QgLayoutUsage;
#define QGLOU_MAX_SIZE		((size_t)(QGLOU_MAX_VALUE+sizeof(size_t)-1))&((size_t)~(sizeof(size_t)-1))

/// @brief 블렌드 모드
typedef enum QGBLEND
{
	QGBLEND_OFF,											/// @brief dstRGBA = srcRGBA
	QGBLEND_BLEND,											/// @brief dstRGB = (srcRGB * srcA) + (dstRGB * (1 - srcA), dstA = srcA  +  (dstA * (1 - srcA))
	QGBLEND_ADD,											/// @brief dstRGB = (srgRGB * srcA) + dstRGB, dstA = dstA
	QGBLEND_SUB,											/// @brief dstRGB = (srcRGB * srcA) - dstRGB, dstA = dstA
	QGBLEND_REV_SUB,										/// @brief dstRGB = dstRGB - (srcRGB * srcA), dstA = dstA
	QGBLEND_MOD,											/// @brief dstRGB = srcRGB * dstRGB, dstA = dstA
	QGBLEND_MUL,											/// @brief dstRGB = (srcRGB * dstRGB) + (dstRGB * (1 - srcA)), dstA = dstA
	QGBLEND_MAX_VALUE,
} QgBlend;

/// @brief 뎁스 모드
typedef enum QGDEPTH
{
	QGDEPTH_OFF,											/// @brief 뎁스 테스트 하지 않음
	QGDEPTH_LE,												/// @brief 뎁스가 작으면 통과
	QGDEPTH_LEQ,											/// @brief 뎁스가 작거나 같으면 통과
	QGDEPTH_GR,												/// @brief 뎁스가 크면 통과
	QGDEPTH_GEQ,											/// @brief 뎁스가 크거나 같으면 통과
	QGDEPTH_EQ,												/// @brief 뎁스가 같으면 통과
	QGDEPTH_NEQ,											/// @brief 뎁스가 같지 않으면 통과
	QGDEPTH_ALWAYS,											/// @brief 뎁스는 언제나 통과
	QGDEPTH_MAX_VALUE,
} QgDepth;

/// @brief 스텐실 모드
typedef enum QGSTENCIL
{
	QGSTENCIL_OFF,											/// @brief 스텐실 테스트 하지 않음
	QGSTENCIL_WRITE,										/// @brief 스텐실을 언제나 설정 (1 / 0xFF / 0xFF)
	QGSTENCIL_EVADE,										/// @brief 스텐실을 피함 (1 / 0xFF / 0)
	QGSTENCIL_OVER,											/// @brief 스텐실을 덮음 (1 / 0xFF / 0)
	QGSTENCIL_MAX_VALUE,
} QgStencil;

/// @brief 세이더 타입
typedef enum QGSHADERTYPE
{
	QGSHADER_VS = QN_BIT(0),								/// @brief 정점 세이더
	QGSHADER_PS = QN_BIT(1),								/// @brief 픽셀(프래그먼트) 세이더
	QGSHADER_ALL = QGSHADER_VS | QGSHADER_PS,				/// @brief 모든 세이더 대리
} QgShaderType;

typedef enum QGSHADERFLAG
{
	QGSCF_TEXT = QN_BIT(0),									/// @brief 텍스트 세이더
	QGSCF_BINARY = QN_BIT(1),								/// @brief 바이너리 세이더
} QgScFlag;

/// @brief 세이더 전역 변수 타입
typedef enum QGSHADERCONSTTYPE
{
	QGSCT_UNKNOWN,											/// @brief 알 수 없음
	QGSCT_FLOAT1,											/// @brief 1개의 실수
	QGSCT_FLOAT2,											/// @brief 2개의 실수(Vector2)
	QGSCT_FLOAT3,											/// @brief 3개의 실수(Vector3)
	QGSCT_FLOAT4,											/// @brief 4개의 실수(Vector4)
	QGSCT_FLOAT16,											/// @brief 16개의 실수(Matrix4x4)
	QGSCT_INT1,												/// @brief 1개의 정수
	QGSCT_INT2,												/// @brief 2개의 정수
	QGSCT_INT3,												/// @brief 3개의 정수
	QGSCT_INT4,												/// @brief 4개의 정수
	QGSCT_UINT1,											/// @brief 1개의 정수
	QGSCT_UINT2,											/// @brief 2개의 정수
	QGSCT_UINT3,											/// @brief 3개의 정수
	QGSCT_UINT4,											/// @brief 4개의 정수
	QGSCT_BYTE1,											/// @brief 1개의 바이트
	QGSCT_BYTE2,											/// @brief 2개의 바이트
	QGSCT_BYTE3,											/// @brief 3개의 바이트
	QGSCT_BYTE4,											/// @brief 4개의 바이트
	QGSCT_SAMPLER1D,										/// @brief 1D 텍스쳐 샘플러
	QGSCT_SAMPLER2D,										/// @brief 2D 텍스쳐 샘플러
	QGSCT_SAMPLER3D,										/// @brief 3D 텍스쳐 샘플러
	QGSCT_SAMPLERCUBE,										/// @brief 큐브 텍스쳐 샘플러
	QGSCT_MAX_VALUE
} QgScType;

/// @brief 세이더 자동처리 전역 변수
typedef enum QGSHADERCONSTAUTO
{
	QGSCA_UNKNOWN,											/// @brief 알 수 없음
	QGSCA_ORTHO_PROJ,										/// @brief 평면 투영 행렬
	QGSCA_WORLD,											/// @brief 월드 행렬
	QGSCA_VIEW,												/// @brief 뷰 행렬
	QGSCA_PROJ,												/// @brief 투영 행렬
	QGSCA_VIEW_PROJ,										/// @brief 뷰와 투영의 곱행렬
	QGSCA_INV_VIEW,											/// @brief 뷰의 역행렬
	QGSCA_WORLD_VIEW_PROJ,									/// @brief 전역 투영
	QGSCA_TEX1,												/// @brief 텍스쳐 1번
	QGSCA_TEX2,												/// @brief 텍스쳐 2번
	QGSCA_TEX3,												/// @brief 텍스쳐 3번
	QGSCA_TEX4,												/// @brief 텍스쳐 4번
	QGSCA_TEX5,												/// @brief 텍스쳐 5번
	QGSCA_TEX6,												/// @brief 텍스쳐 6번
	QGSCA_TEX7,												/// @brief 텍스쳐 7번
	QGSCA_TEX8,												/// @brief 텍스쳐 8번
	QGSCA_PROP_VEC1,										/// @brief 벡터 1번
	QGSCA_PROP_VEC2,										/// @brief 벡터 2번
	QGSCA_PROP_VEC3,										/// @brief 벡터 3번
	QGSCA_PROP_VEC4,										/// @brief 벡터 4번
	QGSCA_PROP_MAT1,										/// @brief 행렬 1번
	QGSCA_PROP_MAT2,										/// @brief 행렬 2번
	QGSCA_PROP_MAT3,										/// @brief 행렬 3번
	QGSCA_PROP_MAT4,										/// @brief 행렬 4번
	QGSCA_MAT_PALETTE,										/// @brief 행렬 팔레트
	QGSCA_MAX_VALUE
} QgScAuto;

/// @brief 버퍼 타입
typedef enum QGBUFFERTYPE
{
	QGBUFFER_VERTEX,										/// @brief 정점 버퍼
	QGBUFFER_INDEX,											/// @brief 인덱스 버퍼
	QGBUFFER_CONSTANT,										/// @brief 상수 버퍼			
} QgBufferType;

/// @brief 지우기 역할
typedef enum QGCLEAR
{
	QGCLEAR_DEPTH = QN_BIT(0),								/// @brief 뎁스 버퍼 지우기
	QGCLEAR_STENCIL = QN_BIT(1),							/// @brief 스텐실 버퍼 지우기
	QGCLEAR_RENDER = QN_BIT(2),								/// @brief 렌더 타겟 영영 지우기
} QgClear;

/// @brief 텍스쳐 플래그
typedef enum QGTEXFLAG
{
	QGTEXF_NONE = 0,

	QGTEXF_DISCARD_IMAGE = QN_BIT(0),						/// @brief 사용한 이미지를 버림
	QGTEXF_READ = QN_BIT(1),								/// @brief 읽기 가능
	QGTEXF_WRITE = QN_BIT(2),								/// @brief 쓰기 가능
	QGTEXF_MIPMAP = QN_BIT(3),								/// @brief 밉맵을 만든다
	QGTEXF_LINEAR = QN_BIT(4),								/// @brief 선형 필터링
	QGTEXF_ANISO = QN_BIT(5),								/// @brief 아니소트로피 필터링 (지금은 안됨)
	QGTEXF_CLAMP = QN_BIT(6),								/// @brief 클램핑
	QGTEXF_BORDER = QN_BIT(7),								/// @brief 테두리 색 사용
	QGTEXF_ELAPSED = QN_BIT(8),								/// @brief 경과 시간 사용 애니메이션 텍스쳐, 포즈에도 움직일 수 있음

	QGTEXFS_COMPRESS = QN_BIT(16),							/// @brief 압축 텍스쳐
	QGTEXFS_FLOAT = QN_BIT(17),								/// @brief 실수 텍스쳐

	QGTEXSPEC_DEPTH = QN_BIT(23),							/// @brief 뎁스 텍스쳐
	QGTEXSPEC_STENCIL = QN_BIT(24),							/// @brief 스텐실 텍스쳐
	QFTEXSPEC_TARGET = QN_BIT(25),							/// @brief 렌더 타겟 텍스쳐
	QFTEXSPEC_ANIM = QN_BIT(26),							/// @brief 애니메이션 텍스쳐
	QGTEXSPEC_1D = QN_BIT(27),								/// @brief 1D 텍스쳐
	QGTEXSPEC_2D = QN_BIT(28),								/// @brief 2D 텍스쳐
	QGTEXSPEC_3D = QN_BIT(29),								/// @brief 3D 텍스쳐
	QGTEXSPEC_CUBE = QN_BIT(30),							/// @brief 큐브 텍스쳐
	QGTEXSPEC_ARRAY = QN_BIT(31),							/// @brief 배열 텍스쳐
} QgTexFlag;

/// @brief 글꼴 타입
typedef enum QGFONTTYPE
{
	QGFONT_BITMAP,
	QGFONT_TRUETYPE,
} QgFontType;

/// @brief 배치
typedef enum QGBATCHCMD
{
	QGBTC_NONE,
	QGBTC_LINE,
	QGBTC_TRI,
	QGBTC_RECT,
	QGBTC_MAX_VALE,
} QgBatchCmd;

/// @brief 스터브와 렌더러 만들 때 플래그
typedef enum QGFLAG
{
	QGFLAG_NONE = 0,
	// 스터브 및 렌더러 플래그 (0~7)
	QGFLAG_FULLSCREEN = QN_BIT(0),							/// @brief 시작할 때 전체 화면
	QGFLAG_BORDERLESS = QN_BIT(1),							/// @brief 테두리 없는 윈도우
	QGFLAG_RESIZE = QN_BIT(2),								/// @brief 크기 변경할 수 있음
	QGFLAG_NOTITLE = QN_BIT(3),								/// @brief 타이틀 바가 없음
	QGFLAG_FOCUS = QN_BIT(4),								/// @brief 입력 포커스 받을 수 있음
	QGFLAG_TEXT = QN_BIT(5),								/// @brief 텍스트 입력을 받을 수 있음
	QGFLAG_DPISCALE = QN_BIT(6),							/// @brief DPI 스케일
	QGFLAG_MAXIMIZE = QN_BIT(7),							/// @brief 시작할 때 최대화
	// 렌더러 플래그 (16~28)
	QGFLAG_VSYNC = QN_BIT(16),								/// @brief VSYNC 켜기
	QGFLAG_MSAA = QN_BIT(17),								/// @brief 멀티 샘플링 사용
	// 사용자가 설정할 수 없는 플래그
	QGSPECIFIC_CORE = QN_BIT(29),							/// @brief 코어 프로파일
	QGSPECIFIC_RDHSTUB = QN_BIT(30),						/// @brief 스터브 만들었음
	QGSPECIFIC_VIRTUAL = QN_BIT(31),						/// @brief 가상 스터브 사용
} QgFlag;

/// @brief 스터브 사양
typedef enum QGFEATURE
{
	QGFEATURE_NONE = 0,
	// 스터브 사양 (0~16)
	QGFEATURE_DISABLE_ACS = QN_BIT(0),						/// @brief 접근성 끄기
	QGFEATURE_DISABLE_SCRSAVE = QN_BIT(1),					/// @brief 화면 보호기 끄기
	QGFEATURE_ENABLE_DROP = QN_BIT(2),						/// @brief 드래그 드랍 사용
	QGFEATURE_ENABLE_SYSWM = QN_BIT(3),						/// @brief 시스템 메시지 받기
	QGFEATURE_ENABLE_IDLE = QN_BIT(4),						/// @brief 비활성 대기 상태 사용
	QGFEATURE_RELATIVE_MOUSE = QN_BIT(5),					/// @brief 마우스 잡기
	QGFEATURE_REMOVE_EVENTS = QN_BIT(6),					/// @brief 루프 때 사용하지 않은 이벤트를 삭제한다
	QGFEATURE_ENABLE_ASPECT = QN_BIT(7),
	// 렌더러 종류 (24~31)
	QGRENDERER_OPENGL = QN_BIT(30),
	QGRENDERER_DIRECTX = QN_BIT(31),
} QgFeature;

/// @brief 스터브 상태
typedef enum QGSTUBSTAT
{
	QGSST_NONE = 0,
	QGSST_EXIT = QN_BIT(0),									/// @brief 끝내기
	QGSST_PAUSE = QN_BIT(1),								/// @brief 포즈 중
	QGSST_ACTIVE = QN_BIT(2),								/// @brief 스터브가 활성 상태
	QGSST_LAYOUT = QN_BIT(3),								/// @brief 스터브 크기를 변경
	QGSST_FOCUS = QN_BIT(4),								/// @brief 스터브가 포커스
	QGSST_DROP = QN_BIT(5),									/// @brief 드래그 드랍 중
	QGSST_CURSOR = QN_BIT(6),								/// @brief 커서의 표시
	QGSST_HOLD = QN_BIT(7),									/// @brief 마우스 홀드
	QGSST_TEXT = QN_BIT(8),									/// @brief 텍스트 입력 받음
	QGSST_SHOW = QN_BIT(9),									/// @brief 보임
	QGSST_MAXIMIZE = QN_BIT(10),							/// @brief 최대화
	QGSST_MINIMIZE = QN_BIT(11),							/// @brief 최소화
	QGSST_FULLSCREEN = QN_BIT(15),							/// @brief 풀스크린
} QgStubStat;

/// @brief 이벤트 타입
/// @note 항목을 변경하면 qg_event_to_str() 함수도 고칠 것
typedef enum QGEVENTTYPE
{
	QGEV_NONE,												/// @brief 이벤트 없음
	QGEV_SYSWM,												/// @brief 시스템 메시지
	QGEV_ACTIVE,											/// @brief 스터브 활성 상태 이벤트
	QGEV_LAYOUT,											/// @brief 화면 크기 이벤트
	QGEV_ENTER,												/// @brief 포커스 들어옴 (마우스가)
	QGEV_LEAVE,												/// @brief 포커스 들어옴 (마우스가)
	QGEV_MOUSEMOVE,											/// @brief 마우스를 움직여요
	QGEV_MOUSEDOWN,											/// @brief 마우스 버튼을 눌렀어요
	QGEV_MOUSEUP,											/// @brief 마우스 버튼을 뗏어요
	QGEV_MOUSEWHEEL,										/// @brief 마우스 휠을 굴렸어요
	QGEV_MOUSEDOUBLE,										/// @brief 마우스 버튼을 두번 눌렀어요
	QGEV_KEYDOWN,											/// @brief 키보드를 눌렀어요
	QGEV_KEYUP,												/// @brief 키보드에서 손을 뗐어요
	QGEV_TEXTINPUT,											/// @brief 텍스트 입력
	QGEV_WINDOW,											/// @brief 윈도우 이벤트
	QGEV_DROPBEGIN,											/// @brief 드랍을 시작한다
	QGEV_DROPEND,											/// @brief 드랍이 끝났다
	QGEV_DROPFILE,											/// @brief 파일 드랍
	QGEV_MONITOR,											/// @brief 모니터 이벤트
	QGEV_EXIT,												/// @brief 끝내기
	QGEV_MAX_VALUE,
} QgEventType;

/// @brief 윈도우 이벤트
/// @note 항목을 변경하면 qg_window_event_to_str() 함수도 고칠 것
typedef enum QGWINDOWEVENTTYPE
{
	QGWEV_NONE,												/// @brief 이벤트 없음
	QGWEV_SHOW,												/// @brief 윈도우가 보인다
	QGWEV_HIDE,												/// @brief 윈도우가 안보인다
	QGWEV_PAINTED,											/// @brief 윈도우가 그려진다 (=EXPOSED)
	QGWEV_RESTORED,											/// @brief 윈도우 크기 복귀
	QGWEV_MAXIMIZED,										/// @brief 윈도우가 최대화
	QGWEV_MINIMIZED,										/// @brief 윈도우가 최소화
	QGWEV_MOVED,											/// @brief 윈도우가 이동했다
	QGWEV_SIZED,											/// @brief 윈도우 크기가 변했다
	QGWEV_FOCUS,											/// @brief 윈도우가 포커스를 얻었다
	QGWEV_LOSTFOCUS,										/// @brief 윈도우가 포커스를 잃었다
	QGWEV_CLOSE,											/// @brief 윈도우가 닫혔네
	QGWEV_MAX_VALUE,
} QgWindowEventType;

#ifndef QGMAX_EVENTS
/// @brief 최대 이벤트 갯수
#define QGMAX_EVENTS		1000
#endif

/// @brief 키보드 입력 키 정의
typedef enum QIKKEY
{
	QIK_NONE = 0,
	QIK_BS = 0x08,											// BACKSPACE
	QIK_TAB = 0x09,
	QIK_CLEAR = 0x0C,										// NUM5 without NUMLOCK
	QIK_RETURN = 0x0D,
	QIK_PAUSE = 0x13,
	QIK_CAPSLOCK = 0x14,
	QIK_HANGUL = 0x15,
	QIK_KANA = 0x15,
	QIK_JUNJA = 0x17,
	QIK_FINAL = 0x18,
	QIK_HANJA = 0x19,
	QIK_KANJI = 0x19,
	QIK_ESC = 0x1B,
	QIK_SPACE = 0x20,
	QIK_PGUP = 0x21,										// PRIOR PAGE
	QIK_PGDN = 0x22,										// NEXT PAGE
	QIK_END = 0x23,
	QIK_HOME = 0x24,
	QIK_LEFT = 0x25,
	QIK_UP = 0x26,
	QIK_RIGHT = 0x27,
	QIK_DOWN = 0x28,
	QIK_PRTSCR = 0x2C,										// SNAPSHOT
	QIK_INS = 0x2D,											// INSERT
	QIK_DEL = 0x2E,											// DELETE
	QIK_HELP = 0x2F,
	QIK_0 = 0x30,
	QIK_1 = 0x31,
	QIK_2 = 0x32,
	QIK_3 = 0x33,
	QIK_4 = 0x34,
	QIK_5 = 0x35,
	QIK_6 = 0x36,
	QIK_7 = 0x37,
	QIK_8 = 0x38,
	QIK_9 = 0x39,
	QIK_A = 0x41,
	QIK_B = 0x42,
	QIK_C = 0x43,
	QIK_D = 0x44,
	QIK_E = 0x45,
	QIK_F = 0x46,
	QIK_G = 0x47,
	QIK_H = 0x48,
	QIK_I = 0x49,											// Alphabet I / NOLINT
	QIK_J = 0x4A,
	QIK_K = 0x4B,
	QIK_L = 0x4C,
	QIK_M = 0x4D,
	QIK_N = 0x4E,
	QIK_O = 0x4F,											// Alphabet O / NOLINT
	QIK_P = 0x50,
	QIK_Q = 0x51,
	QIK_R = 0x52,
	QIK_S = 0x53,
	QIK_T = 0x54,
	QIK_U = 0x55,
	QIK_V = 0x56,
	QIK_W = 0x57,
	QIK_X = 0x58,
	QIK_Y = 0x59,
	QIK_Z = 0x5A,
	QIK_LWIN = 0x5B,										// LEFT WIN / META / COMMAND
	QIK_RWIN = 0x5C,										// RIGHT WIN / META / COMMAND
	QIK_APPS = 0x5D,
	QIK_SLEEP = 0x5F,
	QIK_NUM_0 = 0x60,
	QIK_NUM_1 = 0x61,
	QIK_NUM_2 = 0x62,
	QIK_NUM_3 = 0x63,
	QIK_NUM_4 = 0x64,
	QIK_NUM_5 = 0x65,
	QIK_NUM_6 = 0x66,
	QIK_NUM_7 = 0x67,
	QIK_NUM_8 = 0x68,
	QIK_NUM_9 = 0x69,
	QIK_NUM_MUL = 0x6A,										// *
	QIK_NUM_ADD = 0x6B,										// +
	QIK_NUM_SEP = 0x6C,										// ,
	QIK_NUM_SUB = 0x6D,										// -
	QIK_NUM_DOT = 0x6E,										// .
	QIK_NUM_DIV = 0x6F,										// /
	QIK_F1 = 0x70,
	QIK_F2 = 0x71,
	QIK_F3 = 0x72,
	QIK_F4 = 0x73,
	QIK_F5 = 0x74,
	QIK_F6 = 0x75,
	QIK_F7 = 0x76,
	QIK_F8 = 0x77,
	QIK_F9 = 0x78,
	QIK_F10 = 0x79,
	QIK_F11 = 0x7A,
	QIK_F12 = 0x7B,
	QIK_F13 = 0x7C,
	QIK_F14 = 0x7D,
	QIK_F15 = 0x7E,
	QIK_F16 = 0x7F,
	QIK_F17 = 0x80,
	QIK_F18 = 0x81,
	QIK_F19 = 0x82,
	QIK_F20 = 0x83,
	QIK_F21 = 0x84,
	QIK_F22 = 0x85,
	QIK_F23 = 0x86,
	QIK_F24 = 0x87,
	QIK_NUMLOCK = 0x90,
	QIK_SCRL = 0x91,										// SCROLL LOCK
	QIK_LSHIFT = 0xA0,										// LEFT SHIFT
	QIK_RSHIFT = 0xA1,										// RIGHT SHIFT
	QIK_LCTRL = 0xA2,										// LEFT CONTROL
	QIK_RCTRL = 0xA3,										// RIGHT CONTROL / KAIOS KEYPAD HASH
	QIK_LALT = 0xA4,										// LEFT ALT / MENU / OPTION
	QIK_RALT = 0xA5,										// RIGHT ALT / MENU / OPTION
	QIK_NAV_BACK = 0xA6,									// NAVIGATE BACK
	QIK_NAV_FORWARD = 0xA7,									// NAVIGATE FORWARD
	QIK_NAV_REFRESH = 0xA8,									// NAVIGATE REFRESH / RELOAD
	QIK_NAV_STOP = 0xA9,									// NAVIGATE STOP
	QIK_NAV_SEARCH = 0xAA,									// NAVIGATE SEARCH / KAIOS KEYPAD MULTIPLY
	QIK_NAV_FAVORITES = 0xAB,								// NAVIGATE FAVORITES
	QIK_NAV_HOME = 0xAC,									// NAVIGATE HOME
	QIK_VOL_MUTE = 0xAD,									// VOLUME MUTE
	QIK_VOL_DOWN = 0xAE,									// VOLUME DOWN
	QIK_VOL_UP = 0xAF,										// VOLUME UP
	QIK_MEDIA_NEXT = 0xB0,									// MEDIA NEXT
	QIK_MEDIA_PREV = 0xB1,									// MEDIA PREVIOUS
	QIK_MEDIA_STOP = 0xB2,									// MEDIA STOP
	QIK_MEDIA_PLAY = 0xB3,									// MEDIA PLAY / PAUSE
	QIK_SEMI = 0xBA,										// ;:
	QIK_ADD = 0xBB,											// =+
	QIK_COMMA = 0xBC,										// ,<
	QIK_SUB = 0xBD,											// -_
	QIK_DOT = 0xBE,											// .>
	QIK_SLASH = 0xBF,										// /?
	QIK_TILT = 0xC0,										// `~
	QIK_LBR = 0xDB,											// [{
	QIK_BACKSLASH = 0xDC,									// \|
	QIK_RBR = 0xDD,											// ]}
	QIK_QUOTE = 0xDE,										// '"
	QIK_MAX_VALUE = 0xFF + 1,
} QikKey;

/// @brief 상태키 마스크
typedef enum QIKMASK
{
	QIKM_NONE = 0,
	QIKM_SHIFT = QN_BIT(0),
	QIKM_CTRL = QN_BIT(1),
	QIKM_ALT = QN_BIT(2),
	QIKM_CAPS = QN_BIT(12),
	QIKM_SCRL = QN_BIT(13),
	QIKM_NUM = QN_BIT(14),
	QIKM_WIN = QN_BIT(15),
} QikMask;

/// @brief 마우스 버튼
typedef enum QIMBUTTON
{
	QIM_NONE = 0,
	QIM_LEFT = 1,
	QIM_RIGHT = 2,
	QIM_MIDDLE = 3,
	QIM_X1 = 4,
	QIM_X2 = 5,
	QIM_MAX_VALUE,
} QimButton;

/// @brief 마스 버튼 마스크
typedef enum QIMMASK
{
	QIMM_LEFT = QN_BIT(QIM_LEFT),
	QIMM_RIGHT = QN_BIT(QIM_RIGHT),
	QIMM_MIDDLE = QN_BIT(QIM_MIDDLE),
	QIMM_X1 = QN_BIT(QIM_X1),
	QIMM_X2 = QN_BIT(QIM_X2),

	QIMM_WHEELUP = 0x0100,
	QIMM_WHEELDOWN = 0x0200,
	QIMM_DOUBLE = 0x1000,
} QimMask;

/// @brief 마우스 눌림 상태 추적
typedef enum QIMTRACK
{
	QIMT_MOVE = 0,
	QIMT_DOWN = 1,
	QIMT_UP = 2,
} QimTrack;

/// @brief 터치 상태 추적
typedef enum QIMTOUCH
{
	QITT_BEGIN = 1,
	QITT_MOVE = 2,
	QITT_END = 3,
} QimTouch;

/// @brief 컨트롤러 버튼
typedef enum QICBUTTON
{
	QIC_NONE = 0,
	QIC_UP = 1,
	QIC_DOWN = 2,
	QIC_LEFT = 3,
	QIC_RIGHT = 4,
	QIC_START = 5,
	QIC_BACK = 6,
	QIC_THB_LEFT = 7,
	QIC_THB_RIGHT = 8,
	QIC_SHD_LEFT = 9,
	QIC_SHD_RIGHT = 10,
	QIC_A = 11,
	QIC_B = 12,
	QIC_X = 13,
	QIC_Y = 14,
} QicButton;

/// @brief 컨트롤러 마스크
typedef enum QICMASK
{
	QICM_UP = QN_BIT(QIC_UP),
	QICM_DOWN = QN_BIT(QIC_DOWN),
	QICM_LEFT = QN_BIT(QIC_LEFT),
	QICM_RIGHT = QN_BIT(QIC_RIGHT),
	QICM_START = QN_BIT(QIC_START),
	QICM_BACK = QN_BIT(QIC_BACK),
	QICM_THB_LEFT = QN_BIT(QIC_THB_LEFT),
	QICM_THB_RIGHT = QN_BIT(QIC_THB_RIGHT),
	QICM_SHD_LEFT = QN_BIT(QIC_SHD_LEFT),
	QICM_SHD_RIGHT = QN_BIT(QIC_SHD_RIGHT),
	QICM_A = QN_BIT(QIC_A),
	QICM_B = QN_BIT(QIC_B),
	QICM_X = QN_BIT(QIC_X),
	QICM_Y = QN_BIT(QIC_Y),
} QicMask;


//////////////////////////////////////////////////////////////////////////
// properties

/// @brief 픽셀 포맷
typedef struct QGPROPPIXEL
{
	QgClrFmt			format;								/// @brief 픽셀 포맷
	ushort				bpp;								/// @brief 픽셀 당 비트 수
	ushort				tbp;								/// @brief 픽셀 당 바이트 수
} QgPropPixel;

/// @brief 레이아웃 요소
typedef struct QGLAYOUTINPUT
{
	QgLayoutStage		stage;								/// @brief 스테이지 구분
	QgLayoutUsage		usage;								/// @brief 사용법
	QgClrFmt			format;								/// @brief 포맷
	cham				normalized;							/// @brief 정규화	
} QgLayoutInput;

/// @brief 레이아웃 데이터
typedef struct QGLAYOUTDATA
{
	QgLayoutInput*		inputs;								///	@brief 요소 데이터 포인트
	size_t				count;								/// @brief 요소 갯수
} QgLayoutData;

/// @brief 코드 데이터
typedef struct QGCODEDATA
{
	void*				code;								/// @brief 코드 데이터
	size_t				size;								/// @brief 코드 크기 (텍스트 타입이면 반드시 0이어야 한다!)
} QgCodeData;

/// @brief 세이더 속성
typedef struct QGPROPSHADER
{
	QgLayoutData		layout;
	QgCodeData			vertex;
	QgCodeData			pixel;
	//QgCodeData		geometry;
} QgPropShader;

/// @brief 블렌드
typedef struct QGPROPBLEND
{
	cham				coverage;							/// @brief sample coverage / alpha to coverage
	cham				separate;							/// @brief 거짓이면 rb[0]만 사용
	QgBlend				rb[QGRVS_MAX_VALUE];				/// @brief 스테이지 별 블렌드 정보
} QgPropBlend;

/// @brief 래스터라이저
typedef struct QGPROPRASTERIZER
{
	QgFill				fill : 16;							/// @brief 채우기 방법
	QgCull				cull : 16;							/// @brief 면 제거 방법
	cham				scissor;							/// @brief 가위질
	float				depth_bias;							/// @brief 주어진 픽셀에 추가하는 깊이 값
	float				slope_scale;						/// @brief 픽셀 경사면에 주어지는 스칼라 값
} QgPropRasterizer;

/// @brief 렌더 포맷
typedef struct QGPROPRVFORMAT
{
	uint				count;								/// @brief rtv 갯수
	QgClrFmt			rtv[QGRVS_MAX_VALUE];				/// @brief Render Target format
	QgClrFmt			dsv;								/// @brief Depth Stencil format
} QgPropRvFormat;

/// @brief 렌더 파이프라인
typedef struct QGPROPRENDER
{
	QgPropRvFormat		format;								/// @brief 렌더 포맷
	QgPropBlend			blend;								/// @brief 블렌드
	QgPropRasterizer	rasterizer;							/// @brief 래스터라이저
	QgDepth				depth : 16;							/// @brief 뎁스
	QgStencil			stencil : 16;						/// @brief 스텐실
	QgTopology			topology;							/// @brief 토폴로지
} QgPropRender;

/// @brief 세이더 변수
typedef struct QGVARSHADER
{
#ifdef _QN_64_
	char				name[64 + 4];						/// @brief 변수 이름
#else
	char				name[64];							/// @brief 변수 이름
#endif
	size_t				hash;								/// @brief 변수 이름 해시

	ushort				offset;								/// @brief 변수 옵셋
	ushort				size;								/// @brief 변수의 갯수

	QgScType			sctype;								/// @brief 변수 타입
	QgScAuto			scauto;								/// @brief 자동 타입 또는 사용자 정의 키 값
} QgVarShader;

/// @brief 프로젝션 파라미터
typedef struct QGPARAMPROJ
{
	float				fov;								/// @brief 시야각
	float				aspect;								/// @brief 가로 세로 비율
	float				znear;								/// @brief 가까운 평면
	float				zfar;								/// @brief 먼 평면
} QgParamProj;

/// @brief 뷰 파라미터
typedef struct QGPARAMVIEW
{
	QmVec				eye;								/// @brief 눈 위치
	QmVec				at;									/// @brief 바라보는 위치
	QmVec				up;									/// @brief 위 방향
	QmVec				ahead;								/// @brief 앞 방향
} QgParamView;

/// @brief 카메라 파라미터
typedef struct QGPARAMCAMERA
{
	QmMat				proj;								/// @brief 프로젝션 행렬
	QmMat				view;								/// @brief 뷰 행렬
	QmMat				invv;								/// @brief 역 행렬
	QmMat				vipr;								/// @brief 프로젝션 곱하기 뷰 행렬

	QmVec				rot;								/// @brief 회전
	float				dist;								/// @brief 거리

	float				spd_move;							/// @brief 이동 속도
	float				spd_rot;							/// @brief 회전 속도

	uint				use_layout : 1;
	uint				use_pause : 1;
	uint				use_maya : 1;
	uint				__pad : 29;
} QgParamCamera;

/// @brief 메시 프로퍼티
typedef struct QGPROPMESH
{
	int					vertices;
	int					polygons;

	QmFloat3*			position;
	QmFloat2*			coord[2];
	QmFloat3*			normal[2];
	QmFloat3*			binormal;
	QmFloat3*			tangent;
	uint*				color[2];
	int*				index;
} QgPropMesh;

/// @brief 키 상태
typedef struct QGUIMKEY
{
	QikMask				mask;								/// @brief 특수 키 상태
	byte				key[QIK_MAX_VALUE];					/// @brief 전체 키 상태 배열
	byte				prev[QIK_MAX_VALUE];
} QgUimKey;

/// @brief 마우스 상태
typedef struct QGUIMMOUSE
{
	QimMask				mask;								/// @brief 마우스 버튼 상태
	QimMask				prev;								/// @brief 이전 마우스 버튼 상태
	QmPoint				pt;									/// @brief 마우스 좌표
	QmPoint				last;								/// @brief 마우스의 이전 좌표
	QmPoint				delta;								/// @brief 이동 거리 차이

	struct QGUIMMOUSE_CLICK
	{
		uint				tick;							/// @brief 첫번째 눌렸을 때
		uint				ltick;							/// @brief 마지막으로 두번 눌렸을 때
		QimButton			btn;							/// @brief 두번 검새 때 눌린 마우스 버튼
		QmPoint				loc;							/// @brief 두번 검사 때 마우스 위치
	}					clk;								/// @brief 마우스 눌림 정보
	struct QGUIMMOUSE_LIMIT
	{
		uint				move;							/// @brief 제한 이동 거리(포인트)의 제곱
		uint				tick;							/// @brief 제한 클릭 시간(밀리초)
	}					lim;								/// @brief 마우스 더블 클릭 구현 정보
	struct QGUIMMOUSE_WHEEL
	{
		QmVec2				accm;							/// @brief 가속도
		QmVec2				precise;						/// @brief 정밀 값
		QmPoint				integral;						/// @brief 값
	}					wheel;
} QgUimMouse;

/// @brief 컨트롤러 상태
typedef struct QGUIMCTRL
{
	QicButton			btn;								/// @brief 컨트롤러 버튼 상태
	QmPoint				trg;
	QmVec2				lthb;
	QmVec2				rthb;
} QgUimCtrl;

/// @brief 컨트롤러 정보
typedef struct QGUIMCTRLINFO
{
	QicMask				flags;

	int					type : 16;
	int					extend : 16;

	int					battery_type : 8;
	int					battery_level : 8;

	int					headset_type : 8;
	int					headset_level : 8;
} QgUimCtrlInfo;

/// @brief 컨트롤러 진동
typedef struct QGUIMCTRLVIB
{
	ushort				left;
	ushort				right;
} QgUimCtrlVib;

/// @brief 모니터 정보
typedef struct QGUDEVMONITOR
{
	char				name[64];
	int					no;
	uint				x;
	uint				y;
	uint				width;
	uint				height;
	uint				refresh;
	uint				mmwidth;
	uint				mmheight;
} QgUdevMonitor;

/// @brief 이벤트
typedef union QGEVENT
{
	QgEventType			ev;									/// @brief 이벤트 타입
#ifdef _QN_WINDOWS_
	struct QGEVENT_SYSWINDOWS
	{
		QgEventType			ev;
		uint				mesg;							/// @brief 메시지
		nuint				wparam;							/// @brief WPARAM
		nint				lparam;							/// @brief LPARAM
		void*				hwnd;							/// @brief 윈도우 핸들
	}					windows;
#endif
	struct QGEVENT_ACTIVE
	{
		QgEventType			ev;
		cham				active;							/// @brief 활성 상태면 참
		double				delta;							/// @brief 마지막 활성 상태로 부터의 지난 시간(초)
	}					active;								/// @brief 액티브 이벤트
	struct QGEVENT_LAYOUT
	{
		QgEventType			ev;
		QmRect				bound;							/// @brief 실제 윈도우의 사각 영역
		QmSize				size;							/// @brief 그리기 영역 크기
	}					layout;								/// @brief 레이아웃 이벤트
	struct QGEVENT_KEYBOARD
	{
		QgEventType			ev;
		QikKey				key;							/// @brief 이벤트에 해당하는 키
		QikMask				mask;							/// @brief 특수키 상태
		cham				repeat;							/// @brief 계속 눌려 있었다면 참
	}					key;								/// @brief 키 눌림 떼임 이벤트
	struct QGEVENT_MOUSEMOVE
	{
		QgEventType			ev;
		QmPoint				pt;								/// @brief 마우스 좌표
		QmPoint				delta;							/// @brief 마우스 이동 거리
		QimMask				mask;							/// @brief 마우스 버튼의 상태
	}					mmove;								/// @brief 마우스 이동 이벤트
	struct QGEVENT_MOUSEBUTTON
	{
		QgEventType			ev;
		QmPoint				pt;								/// @brief 마우스 좌표
		QimButton			button;							/// @brief 이벤트에 해당하는 버튼
		QimMask				mask;							/// @brief 마으스 버튼의 상태
	}					mbutton;							/// @brief 마우스 버튼 눌림 떼임 이벤트
	struct QGEVENT_MOUSEWHEEL
	{
		QgEventType			ev;
		QmPoint				pt;								/// @brief 마우스 좌표
		QmPoint				wheel;							/// @brief 휠 움직임 (Y가 기본휠, X는 틸트)
		QmVec2				precise;						/// @brief 정밀한 움직임 (Y가 기본휠, X는 틸트)
		int					direction;						/// @brief 참이면 뱡향이 반대
	}					mwheel;								/// @brief 마우스 휠 이벤트
	struct QGEVENT_TEXT
	{
		QgEventType			ev;
		int					len;							/// @brief 텍스트 길이
		char				data[16];						/// @brief 텍스트 내용
	}					text;
	struct QGEVENT_DROP
	{
		QgEventType			ev;
		int					len;							/// @brief 데이타의 길이
		char*				data;							/// @brief 테이타 포인터. 데이터의 유효기간은 다음 loop 까지
	}					drop;
	struct QGEVENT_WINDOWEVENT
	{
		QgEventType			ev;
		QgWindowEventType	mesg;							/// @brief 윈도우 이벤트 메시지
		int					param1;							/// @brief 파라미터1
		int					param2;							/// @brief 파라미터2
	}					wevent;
	struct QGEVENT_MONITOR
	{
		QgEventType			ev;
		int					state;							/// @brief 0=연결 끊김, 1=연결, 3=스터브의 활성 모니터가 바뀜
		QgUdevMonitor*		monitor;						/// @brief 모니터 정보 포인터
	}					monitor;
} QgEvent;

/// @brief 이벤트 콜백, 반환값이 0이 아니면 다른 핸들러로 이벤트를 전달하지 않는다
typedef int (*QgEventCallback)(void* data, const QgEventType type, const QgEvent* param);


//////////////////////////////////////////////////////////////////////////
// stub

/// @brief 스터브를 연다
/// @param title 윈도우 타이틀
/// @param display 디스플레이 순번 (기본 디스플레이를 쓰려면 0)
/// @param width 윈도우 너비
/// @param height 윈도우 높이
/// @param flags 생성 플래그 (QgFlag)
/// @param features 사양 (QgFeature)
/// @return 스터브가 만들어지면 참
/// @see QgFlag qg_close_Stub
QSAPI bool qg_open_stub(const char* title, int display, int width, int height, int flags, int features);

/// @brief 스터브를 닫는다
/// @see qg_open_Stub
QSAPI void qg_close_stub(void);

/// @brief 스터브 사양을 켜고 끈다
/// @param features 대상 스터브 사양 (QGFEATURE_)
/// @param enable 켜려면 참, 끄려면 거짓
/// @return 처리한 사양 갯수
QSAPI int qg_feature(int features, bool enable);

/// @brief 스터브 사양을 묻는다
/// @return 스터브 사양
QSAPI QgFeature qg_query_features(void);

/// @brief 풀스크린을 전환한다
/// @param fullscreen 풀스크린으로 가려면 참, 아니면 거짓
QSAPI void qg_toggle_fullscreen(bool fullscreen);

/// @brief 풀스크린인지 확인한다
/// @return 풀스크린이면 참
QSAPI bool qg_get_fullscreen_state(void);

/// @brief 스터브 윈도우 타이틀 설정
/// @param title 타이틀 문자열(UTF-8)
QSAPI void qg_set_title(const char* title);

/// @brief 파일 시스템을 마운트 한다
/// @param index 마운트 번호 (0~9)
/// @param path 마운트 경로 (널이면 현재 디렉토리)
/// @param mode 마운트 모드 (HFS를 읽으려면 반드시 "h"를 포함해야 한다)
/// @return 마운트에 성공하면 참
QSAPI bool qg_mount(int index, const char* path, const char* mode);

/// @brief 퓨즈 마운트로 마운트 한다
/// @param index 마운트 번호 (0~9)
/// @param path 마운트 경로 (널이면 현재 디렉토리)
/// @param diskfs 디스크 파일 시스템으로 마운트 할 것인가
/// @param preload HFS를 미리 로드 할 것인가
/// @return 마운트에 성공하면 참
QSAPI bool qg_fuse(int index, const char* path, bool diskfs, bool preload);

/// @brief 번호에 해당하는 마운트를 얻는다
/// @param index 마운트 순번. 이 값은 0과 9사이의 값을 갖는다
/// @return 해당하는 마운트 포인터를 반환하는데, 인덱스에 해당하는 마운트가 없으면 NULL이며 그 이상으로는 없음
QSAPI QnMount* qg_get_mount(int index);

/// @brief 모니터 정보를 얻는다
/// @param index 모니터 순번
/// @return 모니터 정보를 반환하는데, 인덱스에 해당하는 모니터가 없으면 NULL이며 그 이상으로는 없음
QSAPI const QgUdevMonitor* qg_get_monitor(int index);

/// @brief 현재 스터브 윈도우가 있는 곳의 모니터 정보를 얻는다
/// @return 모니터 정보를 반환하는데, 모니터가 없거나 오류일 경우 NULL
QSAPI const QgUdevMonitor* qg_get_current_monitor(void);

/// @brief 전체 키 정보를 얻는다
/// @return 키 정보 포인터
QSAPI const QgUimKey* qg_get_key_info(void);

/// @brief 전체 마우스 정보를 얻는다
/// @return 마우스 정보 포인터
QSAPI const QgUimMouse* qg_get_mouse_info(void);

/// @brief 타이머를 얻는다
/// @return 타이머 포인터
QSAPI QnTimer* qg_get_timer(void);

/// @brief 마우스 더블 클릭 정밀도를 설정한다
/// @param density 더블 클릭하면서 마우스를 움직여도 되는 거리 (포인트, 최대값 50)
/// @param interval 클릭과 클릭 사이의 시간 (밀리초, 최대값 5000)
/// @return 인수의 범위를 벗어나면 거짓을 반환
/// @see qg_get_mouse_info
QSAPI bool qg_set_double_click_prop(uint density, uint interval);

/// @brief 키의 눌림 상태를 설정한다
/// @param key 설정할 키
/// @param down 참이면 눌림, 거짓이면 안눌림
QSAPI void qg_set_key_state(QikKey key, bool down);

/// @brief 키가 눌렸나 테스트 한다
/// @param key 테스트할 키
/// @return 눌렸으면 참
QSAPI bool qg_get_key_state(QikKey key);

/// @brief 키가 눌려졌는가 테스트 한다 (키 반복 X)
/// @param key 
/// @return 
QSAPI bool qg_get_key_press(const QikKey key);

/// @brief 키가 눌렸다 떼졌나 테스트 한다
/// @param key 테스트할 키
/// @return 눌렸다 떼졌으면 참
QSAPI bool qg_get_key_release(const QikKey key);

/// @brief 마우스 버튼이 눌렸다 테스트한다
/// @param button 마우스 버튼
/// @return 눌렸으면 참
QSAPI bool qg_get_mouse_button_state(const QimButton button);

/// @brief 마우스 버튼이 눌려졌는가 테스트 한다 (계속 눌림 X)
/// @param button 마우스 버튼
/// @return 눌렸으면 참
QSAPI bool qg_get_mouse_button_press(const QimButton button);

/// @brief 마우스 버튼이 눌렸다 떼졌나 테스트 한다
/// @param button 마우스 버튼
/// @return 눌렸다 떼졌으면 참
QSAPI bool qg_get_mouse_button_release(const QimButton button);

/// @brief 초당 프레임(FPS)를 얻는다
/// @return 초당 프레임 수
QSAPI float qg_get_fps(void);

/// @brief 초당 평균 프레임(FPS)를 얻는다
/// @return 초당 평균 프레임 수
QSAPI float qg_get_afps(void);

/// @brief 프레임 당 시간을 얻는다
/// @return 리퍼런스 시간
/// @note 포즈 중에도 이 시간은 계산된다
QSAPI float qg_get_elapsed(void);

/// @brief 수행 시간을 얻는다. 포즈 중에는 0
/// @return 수행 시간
/// @note 포즈 중에는 계산되지 않으므로 0이다
QSAPI float qg_get_advance(void);

/// @brief 실행 시간을 얻는다
/// @return 실행 시간
QSAPI double qg_get_run_time(void);

/// @brief 화면 비율을 얻는다
/// @return 화면 비율
QSAPI float qg_get_aspect(void);

/// @brief 초 당 프레임(FPS)을 설정한다
/// @param fps 초당 프레임 수
/// @note 0이면 프레임 제한을 푼다, VSYNC가 켜져 있으면 이 값은 무시된다
QSAPI void qg_set_fps(int fps);

/// @brief 타이머를 리셋한다
QSAPI void qg_reset_timer(void);

/// @brief 화면 비율을 설정한다
/// @param width 너비
/// @param height 높이
QSAPI void qg_set_aspect(const int width, const int height);

/// @brief 스터브 크기를 얻는다
QSAPI void qg_get_size(_Out_ QmSize* size);

/// @brief 스터브 루프를 처리한다
/// @return 거짓이면 프로그램을 종료한다
QSAPI bool qg_loop(void);

/// @brief 스터브 이벤트를 폴링한다
/// @param[out] ev 폴링한 이벤트를 반환
/// @return 처리할 이벤트가 더 이상 없으면 거짓
QSAPI bool qg_poll(QgEvent* ev);

/// @brief 스터브 이벤트를 등록된 이벤트 핸들러로 전달한다
QSAPI void qg_dispatch(void);

/// @brief 메인 루프
/// @param func 메인 루프 함수
/// @param data 메인 루프에 전달할 포인터
QSAPI void qg_main_loop(paramfunc_t func, void* data);

/// @brief 스터브 루프를 탈출한다
/// @see qg_poll
/// 
/// 다만, 바로 프로그램을 종료하는 것은 아니며.실제로 이벤트를 처리하지 않도록 하는 역할을 한다
QSAPI void qg_exit_loop(void);

/// @brief 스터브 이벤트 콜백을 등록한다
/// @param func 콜백 함수
/// @param data 콜백 함수 사용자 데이터
/// @return 등록 키. 이 키로 qg_unregister_event_callback 로 등록을 해제할 수 있다 
QSAPI nint qg_register_event_callback(QgEventCallback func, void* data);

/// @brief 스터브 이벤트 콜백의 등록을 해제한다
/// @param key 등록키
/// @return 제거에 성공하면 참
QSAPI bool qg_unregister_event_callback(nint key);

/// @brief 남은 이벤트 갯수를 얻는다
/// @return 남은 이벤트 갯수
QSAPI int qg_left_events(void);

/// @brief 이벤트를 모두 지운다
QSAPI void qg_flush_event(void);

/// @brief 이벤트를 추가한다
/// @param[in] ev 이벤트 정보
/// @param[in] prior 우선 순위 이벤트
/// @return 총 이벤트 갯수
QSAPI int qg_add_event(const QgEvent* ev, bool prior);

/// @brief 이벤트를 추가하지만. 데이타는 없이 이벤트 종류만 추가한다
/// @param type 이벤트 종류
/// @param[in] prior 우선 순위 이벤트
/// @return 총 이벤트 갯수
QSAPI int qg_add_signal_event(QgEventType type, bool prior);

/// @brief 키를 갖고 있는 이벤트를 추가한다. 키 갖는 이벤트는 우선 순위 이벤트로 처리한다
/// @param ev 이벤트 정보
/// @param key 이벤트에 대한 키
/// @return 총 이벤트 갯수
QSAPI int qg_add_key_event(const QgEvent* ev, size_t key);

/// @brief 맨 앞 이벤트를 꺼낸다. 꺼내면서 해당 이벤트는 큐에서 삭제
/// @param[in] ev 얻은 이벤트 정보
/// @param[in] peek 이 값이 참이면 큐에서 삭제는 안한다
/// @return 이벤트가 있었다면 참. 없으면 거짓
QSAPI bool qg_pop_event(QgEvent* ev, bool peek);

/// @brief 이벤트를 문자열로
/// @param ev 이벤트
/// @return 이벤트 문자열
QSAPI const char* qg_event_to_str(QgEventType ev);

/// @brief 윈도우 이벤트를 문자열로
/// @param wev 윈도우 이벤트
/// @return 윈도우 이벤트 문자열
QSAPI const char* qg_window_event_to_str(QgWindowEventType wev);

/// @brief 사용가능한 키인가
/// @param key QikKey
/// @return 사용가능하면 참
QSAPI bool qg_qik_usable(QikKey key);

/// @brief QikKey를 문자열로
/// @param key QikKey
/// @return QikKey의 이름
QSAPI const char* qg_qik_to_str(QikKey key);

/// @brief QimButton를 문자열로
/// @param button QimButton
/// @return QimButton의 이름
QSAPI const char* qg_qim_to_str(QimButton button);

/// @brief QicButton를 문자열로
/// @param button QicButton
/// @return QicButton의 이름
QSAPI const char* qg_qic_to_str(QicButton button);


//////////////////////////////////////////////////////////////////////////
// render device

/// @brief 세이더 콜백
/// @details 두번째 인수(int)는 qn_get_key로 얻어진 키 값을 전달하므로 자동 변수가 아닐 경우
/// 미리 qn_set_key로 키 값을 등록해야 한다. 키 값이 없다면 0으로 전달하므로,
/// 그 경우 세이더 변수의 name으로 변수를 특정해야 한다
typedef void(*QgVarShaderFunc)(void*, nint, const QgVarShader*);

/// @brief 렌더러를 연다
/// @param driver 드라이버 이름 (NULL로 지정하여 기본값)
/// @param title 윈도우 타이틀
/// @param display 디스플레이 번호
/// @param width 윈도우 너비
/// @param height 윈도우 높이
/// @param flags 스터브 및 렌더러 플래그
/// @param features 스터브 및 렌더러 사양
/// @return 만들어졌으면 참
/// @note 내부에서 qg_open_stub 함수를 호출한다 (미리 만들어 놔도 된다)
QSAPI bool qg_open_rdh(const char* driver, const char* title, int display, int width, int height, int flags, int features);

/// @brief 렌더러를 닫는다
QSAPI void qg_close_rdh(void);

/// @brief 세이더 변수 콜백을 등록한다
/// @param func 세이더 콜백 함수
/// @param data 세이더 콜백 함수의 인수
QSAPI void qg_set_shader_var_callback(QgVarShaderFunc func, void* data);

/// @brief 렌더러를 준비한다
/// @param clear 배경, 스텐실, 뎁스를 초기화 하려면 true 로 한다
/// @return 렌더러가 준비됐으면 참
/// @retval true 렌더러가 준비됐다
/// @retval false 렌더러가 준비되지 않았거나, 프로그램이 종료했다
QSAPI bool qg_begin_render(bool clear);

/// @brief 렌더러를 끝낸다
/// @param flush 화면 갱신을 하려면 참으로
QSAPI void qg_end_render(bool flush);

/// @brief 렌더러 결과를 화면으로 출력한다
QSAPI void qg_flush(void);

/// @brief 렌더러 상태를 초기화 한다
/// @note 이 함수는 직접 호출하지 않아도 좋다. 외부 스터브 사용할 때 화면 크기가 바뀔 때 사용하면 좋음
QSAPI void qg_rdh_reset(void);

/// @brief 렌더러를 지운다 (배경은 지정간 배경색으로, 뎁스는 1로, 스텐실은 0으로)
/// @param clear 지우기 플래그
QSAPI void qg_clear_render(QgClear clear);

/// @brief 세이더 vec4 타입 파라미터 설정
/// @param at 0부터 3까지 총 4가지
/// @param v vec4 타입 값
QSAPI void qg_set_param_vec4(int at, const QmVec4* v);

/// @brief 세이더 mat4 타입 파라미터 설정
/// @param at 0부터 3까지 총 4가지
/// @param m mat4 타입 값
QSAPI void qg_set_param_mat4(int at, const QmMat4* m);

/// @brief 세이더 영향치(주로 뼈대 팔레트) 파라미터 설정
/// @param count 행렬 갯수
/// @param weight 영향치 행렬
QSAPI void qg_set_param_weight(int count, QmMat4* weight);

/// @brief 배경색을 설정한다
/// @param background_color 배경색 (널값일 경우 (0.0f, 0.0f, 0.0f, 1.0f)로 초기화)
QSAPI void qg_set_background(const QmColor* background_color);

/// @brief 월드 행렬을 설정한다
/// @param world 월드 행렬
QSAPI void qg_set_world(const QmMat4* world);

/// @brief 뷰 행렬을 설정한다
/// @param view 뷰 행렬
QSAPI void qg_set_view(const QmMat4* view);

/// @brief 투영 행렬을 설정한다
/// @param proj 투영 행렬
QSAPI void qg_set_project(const QmMat4* proj);

/// @brief 뷰와 투영 행렬을 설정한다
/// @param proj 투영 행렬
/// @param view 뷰 행렬
QSAPI void qg_set_view_project(const QmMat4* proj, const QmMat4* view);

/// @brief 카메라를 설정한다 (프로젝션과 뷰	행렬을 설정한다)
/// @param camera 카메라
QSAPI void qg_set_camera(QgCamera* camera);

/// @brief 정점 버퍼를 설정한다
/// @param stage 버퍼를 지정할 스테이지
/// @param buffer 설정할 버퍼
/// @return 실패하면 거짓을 반환
/// @retval true 문제 없이 정점 버퍼를 설정했다
/// @retval false buffer 인수에 문제가 있거나 정점 버퍼가 아니다
QSAPI bool qg_set_vertex(QgLayoutStage stage, QgBuffer* buffer);

/// @brief 인덱스 버퍼를 설정한다
/// @param buffer 설정할 버퍼
/// @return 실패하면 거짓을 반환
/// @retval true 문제 없이 인덱스 버퍼를 설정했다
/// @retval false buffer 인수에 문제가 있거나 인덱스 버퍼가 아니다
QSAPI bool qg_set_index(QgBuffer* buffer);

/// @brief 렌더 파이프라인을 설정한다
/// @param render 렌더 파이프라인
QSAPI bool qg_set_render_state(QgRenderState* render);

/// @brief 렌더 파이프라인을 캐시에서 설정한다
/// @param name 설정할 렌더의 이름
/// @return 캐시에 해당 이름이 없으면 거짓
QSAPI bool qg_set_render_named(const char* name);

/// @brief 텍스쳐를 설정한다
/// @param stage 텍스쳐 스테이지 번호 (최대 8)
/// @param texture 설정할 텍스쳐 (널로 텍스쳐를 해제할 수 있다)
QSAPI bool qg_set_texture(int stage, QgTexture* texture);

/// @brief 정점으로 그리기
/// @param tpg 그릴 모양의 토폴로지
/// @param vertices 정점 갯수
/// @return 문제없이 그리면 참
QSAPI bool qg_draw(QgTopology tpg, int vertices);

/// @brief 인덱스와 정점으로 그리기
/// @param tpg 그릴 모양의 토폴로지
/// @param indices 그릴 인덱스 갯수
/// @return 문제없이 그리면 참
QSAPI bool qg_draw_indexed(QgTopology tpg, int indices);

/// @brief 텍스쳐 스프라이트를 그린다
/// @param bound 그릴 영역
/// @param texture 텍스쳐 (널이면 색깔만 사용)
/// @param color 색깔 (널이면 흰색)
/// @param coord 텍스쳐 좌표 (널이면 전체 텍스쳐)
QSAPI void qg_draw_sprite(const QmRect* bound, QgTexture* texture, const QmColor* color, const QmVec* coord);

/// @brief 텍스쳐 스프라이트를 회전시켜 그린다
/// @param bound 그릴 영영역
/// @param angle 회전 각도(호도)
/// @param texture 텍스쳐 (널이면 색깔만 사용)
/// @param color 색깔 (널이면 흰색)
/// @param coord 텍스쳐 좌표 (널이면 전체 텍스쳐)
QSAPI void qg_draw_sprite_ex(const QmRect* bound, float angle, QgTexture* texture, const QmColor* color, const QmVec* coord);


//////////////////////////////////////////////////////////////////////////
// 렌더 오브젝트

/// @brief 버퍼
struct QGBUFFER
{
	QN_GAM_BASE(QNGAMBASE);

	QgBufferType		type;
	uint				size;
	uint				count;
	ushort				stride;

	halfcham			mapped;
};

QN_DECL_VTABLE(QGBUFFER)
{
	QN_DECL_VTABLE(QNGAMBASE)	base;
	void*(*map)(void*);
	bool (*unmap)(void*);
	bool (*data)(void*, int, const void*);
};

/// @brief 버퍼를 만든다
/// @param type 버퍼 타입
/// @param count 요소 개수
/// @param stride 요소의 너비
/// @param initial_data 초기화할 요소 데이터로 이 값이 NULL이면 동적 버퍼, 값이 있으면 정적 버퍼로 만들어진다
/// @return 만들어진 버퍼
/// @details data 파라미터의 설명에도 있지만, 정적 버퍼로 만들고 나중에 데이터를 넣으면 문제가 생길 수도 있다
QSAPI QgBuffer* qg_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data);

/// @brief 버퍼 설정을 위해 잠근다
/// @param g 버퍼
/// @return 버퍼 데이터 설정을 위한 포인터
/// @retval NULL 버퍼를 잠글 수 없다
QSAPI void* qg_buffer_map(QgBuffer* g);

/// @brief 잠궛던 버퍼를 푼다
/// @param g 버퍼
/// @return 버퍼 설정에 성공하면 참
QSAPI bool qg_buffer_unmap(QgBuffer* g);

/// @brief 버퍼에 데이터를 설정한다
/// @param g 버퍼
/// @param size 데이터의 크기
/// @param data 설정할 데이터
/// @return 버퍼에 데이터가 들어갔으면 참
/// @note data 는 반드시 size 만큼 데이터를 갖고 있어야한다
QSAPI bool qg_buffer_data(QgBuffer* g, int size, const void* data);


/// @brief 렌더 파이프라인 상태
struct QGRENDERSTATE
{
	QN_GAM_BASE(QNGAMNODE);

	nuint				ref;
};

/// @brief 렌더 파이프라인을 만든다
/// @param name 렌더 이름 (이름을 지정하면 캐시한다)
/// @param render 렌더 파이프라인 속성
/// @param shader 세이더 속성
/// @return 만들어진 렌더 파이프라인
QSAPI QgRenderState* qg_create_render_state(const char* name, const QgPropRender* render, const QgPropShader* shader);


/// @brief 텍스쳐
struct QGTEXTURE
{
	QN_GAM_BASE(QNGAMBASE);

	QgPropPixel			prop;
	int					width;
	int					height;
	int					mipmaps;
	QgTexFlag			flags;
};

QN_DECL_VTABLE(QGTEXTURE)
{
	QN_DECL_VTABLE(QNGAMBASE)	base;
	bool (*bind)(QgTexture*, int);
};

/// @brief 텍스쳐를 만든다
/// @param name 텍스쳐 이름
/// @param image 이미지
/// @param flags 텍스쳐 플래그 
/// @return 만들어진 텍스쳐
QSAPI QgTexture* qg_create_texture(const char* name, const QgImage* image, QgTexFlag flags);

/// @brief 텍스쳐를 파일에서 읽어 만든다
/// @param mount 마운트 번호
/// @param filename 파일 이름
/// @param flags 텍스쳐 플래그
/// @return 읽어서 만든 텍스쳐
QSAPI QgTexture* qg_load_texture(int mount, const char* filename, QgTexFlag flags);


//////////////////////////////////////////////////////////////////////////
// 도움 오브젝트

// 이미지
struct QGIMAGE
{
	QN_GAM_BASE(QNGAMBASE);

	QgPropPixel			prop;
	int					width;
	int					height;
	int					pitch;
	int					mipmaps;
	byte*				data;
};

/// @brief 빈 이미지를 만든다
/// @param fmt 이미지 포맷
/// @param width 이미지 너비
/// @param height 이미지 높이
/// @return 만들어진 이미지
QSAPI QgImage* qg_create_image(QgClrFmt fmt, int width, int height);

/// @brief 버퍼로 이미지를 만든다. 버퍼는 압축되지 않은 날(RAW) 이미지 데이터이어야 한다
/// @param fmt 이미지 포맷
/// @param width 이미지 너비
/// @param height 이미지 높이
/// @param data 이미지 데이터 
/// @return 만들어진 이미지
/// @warning 각 인수에 대해 유효성을 검사하지 않는다! 즉, 안전하지 않은 함수이다. 한편,
/// 이미지 데이터는 이미지가 관리하기 때문에 해제하면 안된다. 임시 메모리를 전달해도 안되며 반드시 할당한 데이터 일 것
QSAPI QgImage* qg_create_image_buffer(QgClrFmt fmt, int width, int height, void* data);

/// @brief 색깔로 채운 이미지를 만든다
/// @param width 이미지 너비
/// @param height 이미지 높이
/// @param color 채울 색깔
/// @return 만들어진 이미지
QSAPI QgImage* qg_create_image_filled(int width, int height, const QmColor* color);

/// @brief 선형 그라디언트 이미지를 만든다
/// @param width 이미지 너비
/// @param height 이미지 높이
/// @param begin 시작 색깔
/// @param end 끝 색깔
/// @param direction 방향
/// @return 만들어진 이미지
QSAPI QgImage* qg_create_image_gradient_linear(int width, int height, const QmColor* begin, const QmColor* end, float direction);

/// @brief 원형 그라디언트 이미지를 만든다
/// @param width 이미지 너비
/// @param height 이미지 높이
/// @param inner 안쪽 색깔
/// @param outer 바깥 색깔
/// @param density 밀도
/// @return 만들어진 이미지
QSAPI QgImage* qg_create_image_gradient_radial(int width, int height, const QmColor* inner, const QmColor* outer, float density);

/// @brief 격자 패턴	이미지를 만든다
/// @param width 이미지 너비
/// @param height 이미지 높이
/// @param oddColor 홀수 색깔
/// @param evenColor 짝수 색깔
/// @param checkWidth 체크 너비
/// @param checkHeight 체크 높이
/// @return 만들어진 이미지
QSAPI QgImage* qg_create_image_check_pattern(int width, int height, const QmColor* oddColor, const QmColor* evenColor, int checkWidth, int checkHeight);

/// @brief 이미지를 이미지 형식이 담긴 데이터로 부터 만든다
/// @param data 이미지 데이터
/// @param size 이미지 데이터 크기
/// @return 만들어진 이미지
QSAPI QgImage* qg_load_image_buffer(const void* data, int size);

/// @brief 이미지를 파일에서 읽어 만든다
/// @param mount 마운트 번호
/// @param filename 파일 이름
/// @return 읽어서 만든 이미지
QSAPI QgImage* qg_load_image(int mount, const char* filename);

/// @brief 이미지에 점을 찍는다
QSAPI bool qg_image_set_pixel(const QgImage* self, int x, int y, const QmColor* color);


// 글꼴
struct QGFONT
{
	QN_GAM_BASE(QNGAMBASE);

	char*				name;
	QgFontType			type;
	int					size;
	QmVec4				color;
	QmSize				step;
};

QN_DECL_VTABLE(QGFONT)
{
	QN_GAM_VTABLE(QNGAMBASE);
	void (*set_size)(QnGam, int);
	void (*draw)(QnGam, const QmRect*, const char*);
	QmPoint(*calc)(QnGam, const char*);
};

/// @brief 글꼴을 만든다
/// @param mount 마운트 번호
/// @param filename 파일 이름
/// @param font_base_size 기본 글꼴 크기
/// @return 만들어진 글꼴
QSAPI QgFont* qg_load_font(int mount, const char* filename, int font_base_size);

/// @brief 글꼴을 버퍼에서 만든다
/// @param data 글꼴 데이터
/// @param data_size 글꼴 데이터 크기
/// @param font_base_size 기본 글꼴 크기
/// @return 만들어진 글꼴
/// @warning 글꼴 데이터는 글꼴이 관리하기 때문에 해제하면 안된다. 임시 메모리를 전달해도 안되며 반드시 할당한 데이터 일 것
QSAPI QgFont* qg_load_font_buffer(void* data, int data_size, int font_base_size);

/// @brief 글꼴 크기를 설정한다
/// @param self 글꼴
/// @param size 글꼴 크기
QSAPI void qg_font_set_size(QgFont* self, int size);

/// @brief 글꼴 크기를 얻는다
/// @param self 글꼴
/// @return 글꼴 크기
INLINE int qg_font_get_size(QgFont* self) { return self->size; }

/// @brief 글꼴 색깔을 설정한다
/// @param self 글꼴
/// @param color 글꼴 색깔
INLINE void QM_VECTORCALL qg_font_set_color(QgFont* self, const QmColor color) { self->color = color; }

/// @brief 글꼴 색깔을 얻는다
/// @param self 글꼴
/// @return 글꼴 색깔
INLINE QmVec QM_VECTORCALL qg_font_get_color(QgFont* self) { return self->color; }

/// @brief 글꼴 스텝을 설정한다
/// @param self 글꼴
/// @param step 글꼴 스텝
INLINE void qg_font_set_step(QgFont* self, const QmSize step) { self->step = step; }

/// @brief 글꼴 스텝을 얻는다
/// @param self 글꼴
/// @return 글꼴 스텝
INLINE QmSize qg_font_get_step(QgFont* self) { return self->step; }

/// @brief 문자열을 그린다
/// @param font 글꼴
/// @param bound 문자열을 그릴 영역
/// @param text 문자열
QSAPI void qg_font_draw(QgFont* self, const QmRect* bound, const char* text);

/// @brief 문자열을 그린다
/// @param font 글꼴
/// @param x x 좌표
/// @param y y 좌표
/// @param text 문자열
QSAPI void qg_font_write(QgFont* self, int x, int y, const char* text);

/// @brief 문자열을 그린다
/// @param font 글꼴
/// @param x x 좌표
/// @param y y 좌표
/// @param fmt 문자열
/// @param ... 가변 인수
QSAPI void qg_font_write_format(QgFont* self, int x, int y, const char* fmt, ...);

/// @brief 기본 글꼴을 로드한다
/// @param mount 마운트 번호
/// @param filename 글꼴 파일 이름
/// @param font_base_size 기본 글꼴 크기
/// @return 로드에 성공하면 참
QSAPI bool qg_load_def_font(int mount, const char* filename, int font_base_size);

/// @brief 기본 글꼴로 그린다
/// @param x,y x,y 좌표
/// @param text 문자열
QSAPI void qg_draw_text(int x, int y, const char* text);

/// @brief 기본 글꼴로 그린다
/// @param x,y x,y 좌표
/// @param fmt 문자열
/// @param ... 가변 인수
QSAPI void qg_draw_text_format(int x, int y, const char* fmt, ...);


//////////////////////////////////////////////////////////////////////////
// 노드

struct QGDPCT
{
	QN_GAM_BASE(QNGAMBASE);

	char				name[64];
	size_t				hash;
	QgDpct*				parent;

	struct
	{
		QmMat				mdef;
		QmMat				mcalc;
		QmMat				mlocal;
		QmVec				vloc;
		QmVec				qrot;
		QmVec				vscl;
	}					trfm;
	struct
	{
		QmVec				min;
		QmVec				max;
		QmVec				ctr;
		float				rad;
	}					bound;
};

QN_DECL_VTABLE(QGDPCT)
{
	QN_GAM_VTABLE(QNGAMBASE);
	bool (*update)(QnGam, float);
	void (*draw)(QnGam);
	void (*set_loc)(QnGam, const QmVec*);
	void (*set_rot)(QnGam, const QmVec*);
	void (*set_scl)(QnGam, const QmVec*);
};

QSAPI bool qg_dpct_update(QgDpct* self, float advance);
QSAPI void qg_dpct_draw(QgDpct* self);
QSAPI void qg_dpct_set_loc(QgDpct* self, const QmVec* loc);
QSAPI void qg_dpct_set_rot(QgDpct* self, const QmVec* rot);
QSAPI void qg_dpct_set_scl(QgDpct* self, const QmVec* scl);
QSAPI void qg_dpct_set_name(QgDpct* self, const char* name);
QSAPI void qg_dpct_update_tm(QgDpct* self);

INLINE const char* qg_dpct_get_name(QgDpct* self) { return self->name; }
INLINE size_t qg_dpct_get_hash(QgDpct* self) { return self->hash; }


// 광선
struct QGRAY
{
	QN_GAM_BASE(QNGAMBASE);

	QmVec				location;
	QmVec				direction;
	QmVec				origin;

	QgCamera*			camera;
};

/// @brief 광선을 만든다
/// @param camera 카메라
/// @return 만들어진 광선
QSAPI QgRay* qg_create_ray(QgCamera* camera);

/// @brief 광선의 위치를 설정한다
/// @param self 광선
/// @param x,y x, y 좌표
QSAPI void qg_ray_set_point(QgRay* self, float x, float y);

/// @brief 광선의 위치를 설정한다
/// @param self 광선
/// @param bound 바운드
/// @param x,y x, y 좌표
QSAPI void qg_ray_set_bound_point(QgRay* self, const QmRect* bound, float x, float y);

/// @brief 삼각형과 광선이 교차하는지 테스트한다
/// @param self 광선
/// @param v1 삼각형 첫번째 점
/// @param v2 삼각형 두번째 점
/// @param v3 삼각형 세번째 점
/// @param distance 교차점까지의 거리
/// @return 교차하면 참
QSAPI bool qg_ray_intersect_tri(const QgRay* self, const QmVec* v1, const QmVec* v2, const QmVec* v3, float* distance);

/// @brief 평면과 광선이 교차하는지 테스트한다
/// @param self 광선
/// @param plane 평면
/// @param normal 평면의 법선
/// @param distance 교차점까지의 거리
/// @return 교차하면 참
QSAPI bool qg_ray_intersect_plane(const QgRay* self, const QmVec* plane, const QmVec* normal, float* distance);

/// @brief 광선의 위치를 얻는다
/// @param self 광선
/// @param dist 거리
/// @return 광선의 위치
QSAPI const QmVec QM_VECTORCALL qg_ray_get_loc(const QgRay* self, float dist);


// 카메라
struct QGCAMERA
{
	QN_GAM_BASE(QNGAMBASE);

	QgParamCamera		param;
	QgParamProj			proj;
	QgParamView			view;
};

QN_DECL_VTABLE(QGCAMERA)
{
	QN_GAM_VTABLE(QNGAMBASE);
	void (*update)(QnGam);
};

/// @brief 카메라를 만든다
/// @return 만들어진 카메라
QSAPI QgCamera* qg_create_camera(void);

/// @brief 마야 카메라를 만든다
/// @return 만들어진 카메라
QSAPI QgCamera* qg_create_maya_camera(void);

/// @brief 프로젝션 속성을 설정한다
/// @param self 카메라
/// @param fov 시야각
/// @param znear 가까운 평면
/// @param zfar 먼 평면
QSAPI void qg_camera_set_proj(QgCamera* self, float fov, float znear, float zfar);

/// @brief 프로젝션 속성을 설정한다 (추가로 종횡비 설정)
/// @param self 카메라
/// @param ascpect 종횡비
/// @param fov 시야각
/// @param znear 가까운 평면
/// @param zfar 먼 평면
QSAPI void qg_camera_set_proj_aspect(QgCamera* self, float ascpect, float fov, float znear, float zfar);

/// @brief 뷰 속성을 설정한다
/// @param self 카메라
/// @param eye 위치
/// @param at 시선
/// @param ahead 앞 방향
/// @return 마야 카메라를 사용하면 무조건 거짓을 반환한다
/// @note at과 ahead는 카메라의 위치와 방향을 설정한다
QSAPI bool qg_camera_set_view(QgCamera* self, const QmVec* eye, const QmVec* at, const QmVec* ahead);

/// @brief 카메라 회전을 설정한다 (마야/FPS 카메라)
/// @param self 카메라
/// @param rot 회전
QSAPI void qg_camera_set_rot(QgCamera* self, const QmVec* rot);

/// @brief 카메라 회전 속도를 설정한다 (마야/FPS 카메라)
/// @param self 카메라
/// @param spd 회전 속도
QSAPI void qg_camera_set_rot_speed(QgCamera* self, float spd);

/// @brief 카메라 이동 속도를 설정한다 (마야/FPS 카메라)
/// @param self 카메라
/// @param spd 이동 속도
QSAPI void qg_camera_set_move_speed(QgCamera* self, float spd);

/// @brief 점과 카메라의 거리의 제곱을 얻는다
/// @param self 카메라
/// @param pos 점
/// @return 거리의 제곱
QSAPI float qg_camera_get_distsq(const QgCamera* self, const QmVec* pos);

/// @brief 점과 카메라의 거리를 얻는다
/// @param self 카메라
/// @param pos 점
/// @return 거리
QSAPI float qg_camera_get_dist(const QgCamera* self, const QmVec* pos);

/// @brief 점을 카메라 공간으로 변환한다
/// @param self 카메라
/// @param v 점
/// @param world 월드 행렬 (이 값이 널이면 RDH가 가지고 있는 world 행렬을 사용한다)
/// @return 변환된 점
QSAPI QmVec QM_VECTORCALL qg_camera_project(const QgCamera* self, const QmVec* v, const QmMat4* world);

/// @brief 카메라를 업데이트한다
/// @param self 카메라
QSAPI void qg_camera_update(QgCamera* self);


/// @brief 메시
struct QGMESH
{
	QN_GAM_BASE(QGDPCT);

	QgPropMesh			mesh;
	QgLayoutData		layout;

	QgBuffer*			vbuffers[QGLOS_MAX_VALUE];
	QgBuffer*			ibuffer;
};

/// @brief 빈 메시를 만든다
/// @param name 메시 이름
/// @return 만들어진 메시
QSAPI QgMesh* qg_create_mesh(const char* name);

/// @brief 메시를 빌드한다
/// @param self 메시
/// @return 빌드에 성공하면 참
QSAPI bool qg_mesh_build(QgMesh* self);

/// @brief 레이아웃을 설정한다
/// @param self 메시
/// @param lo 레이아웃 데이터
QSAPI void qg_mesh_set_layout(QgMesh* self, const QgLayoutData* lo);

/// @brief 사면체 메시를 만든다
/// @param self 메시
/// @param width 너비
/// @param height 높이
/// @param depth 깊이
/// @return 문제가 없다면 참
/// @note 메시 데이터만 만든다. 버퍼는 따로 만들어야 한다
QSAPI bool qg_mesh_gen_cube(QgMesh* self, float width, float height, float depth);

/// @brief 평면 메시를 만든다
/// @param self 메시
/// @param width 너비
/// @param depth 깊이
/// @param split_width 너비 분할
/// @param split_depth 깊이 분할
/// @return 문제가 없다면 참
/// @note 메시 데이터만 만든다. 버퍼는 따로 만들어야 한다
QSAPI bool qg_mesh_gen_plane(QgMesh* self, float width, float depth, int split_width, int split_depth);

/// @brief 폴리곤 메시를 만든다
/// @param self 메시
/// @param sides 변 갯수
/// @param radius 반지름
/// @return 문제가 없다면 참
/// @note 메시 데이터만 만든다. 버퍼는 따로 만들어야 한다
QSAPI bool qg_mesh_gen_poly(QgMesh* self, int sides, float radius);

/// @brief 구 메시를 만든다
/// @param self 메시
/// @param radius 반지름
/// @param slices 슬라이스
/// @param stacks 스택
/// @return 문제가 없다면 참
/// @note 메시 데이터만 만든다. 버퍼는 따로 만들어야 한다
QSAPI bool qg_mesh_gen_sphere(QgMesh* self, float radius, int slices, int stacks);

/// @brief 원환체 메시를 만든다
/// @param self 메시
/// @param radius 반지름
/// @param size 크기
/// @param segment 세그먼트
/// @param sides 변 갯수
/// @return 문제가 없다면 참
/// @note 메시 데이터만 만든다. 버퍼는 따로 만들어야 한다
QSAPI bool qg_mesh_gen_torus(QgMesh* self, float radius, float size, int segment, int sides);


//////////////////////////////////////////////////////////////////////////
// 인라인

// 기본 렌더 프로퍼티
#define QG_DEFAULT_PROP_RENDER\
	{\
		.rasterizer.fill = QGFILL_SOLID,\
		.rasterizer.cull = QGCULL_BACK,\
		.depth = QGDEPTH_LE,\
		.stencil = QGSTENCIL_OFF,\
		.format.count = 1,\
		.format.rtv[0] = QGCF_R8G8B8A8,\
		.topology = QGTPG_TRI,\
	}
// 알파 블렌드 렌더 프로퍼티
#define QGL_PROP_RENDER_BLEND\
	{\
		.rasterizer.fill = QGFILL_SOLID,\
		.rasterizer.cull = QGCULL_BACK,\
		.depth = QGDEPTH_LE,\
		.stencil = QGSTENCIL_OFF,\
		.format.count = 1,\
		.format.rtv[0] = QGCF_R8G8B8A8,\
		.topology = QGTPG_TRI,\
		.blend.coverage = false,\
		.blend.separate = false,\
		.blend.rb[0] = QGBLEND_BLEND,\
	}
