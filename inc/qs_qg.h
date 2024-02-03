//
// QsLib [QG Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: UNLICENSE
//

#pragma once
#define __QS_QG__

#include <qs_qn.h>
#include <qs_math.h>
#include <qs_kmc.h>

QN_EXTC_BEGIN


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
	bool32				normalized;							/// @brief 정규화	
} QgLayoutInput;

/// @brief 레이아웃 데이터
typedef struct QGLAYOUTDATA
{
	const QgLayoutInput*	inputs;							/// @brief 요소 데이터 포인트
	size_t					count;							/// @brief 요소 갯수
} QgLayoutData;

/// @brief 코드 데이터
typedef struct QGCODEDATA
{
	const void*			code;								/// @brief 코드 데이터
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
	bool32				use_coverage;						/// @brief sample coverage / alpha to coverage
	bool32				separate;							/// @brief 거짓이면 rb[0]만 사용
	QgBlend				rb[QGRVS_MAX_VALUE];				/// @brief 스테이지 별 블렌드 정보
} QgPropBlend;

/// @brief 래스터라이저
typedef struct QGPROPRASTERIZER
{
	QgFill				fill : 16;							/// @brief 채우기 방법
	QgCull				cull : 16;							/// @brief 면 제거 방법
	bool32				scissor;							/// @brief 가위질
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
		bool32				active;							/// @brief 활성 상태면 참
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
		bool32				repeat;							/// @brief 계속 눌려 있었다면 참
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


//////////////////////////////////////////////////////////////////////////
// render device

typedef struct QGGAM			QgGam;							/// @brief 런더 감
typedef struct QGNODE			QgNode;							/// @brief 노드
typedef struct QGBUFFER			QgBuffer;						/// @brief 버퍼
typedef struct QGRENDERSTATE	QgRenderState;					/// @brief 렌더 파이프라인
typedef struct QGIMAGE			QgImage;						/// @brief 이미지
typedef struct QGTEXTURE		QgTexture;						/// @brief 텍스쳐

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

/// @brief 버퍼를 만든다
/// @param type 버퍼 타입
/// @param count 요소 개수
/// @param stride 요소의 너비
/// @param initial_data 초기화할 요소 데이터로 이 값이 NULL이면 동적 버퍼, 값이 있으면 정적 버퍼로 만들어진다
/// @return 만들어진 버퍼
/// @details data 파라미터의 설명에도 있지만, 정적 버퍼로 만들고 나중에 데이터를 넣으면 문제가 생길 수도 있다
QSAPI QgBuffer* qg_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data);

/// @brief 렌더 파이프라인을 만든다
/// @param name 렌더 이름 (이름을 지정하면 캐시한다)
/// @param render 렌더 파이프라인 속성
/// @param shader 세이더 속성
/// @return 만들어진 렌더 파이프라인
QSAPI QgRenderState* qg_create_render_state(const char* name, const QgPropRender* render, const QgPropShader* shader);

/// @brief 텍스쳐를 만든다
/// @param name 텍스쳐 이름
/// @param image 이미지
/// @param flags 텍스쳐 플래그 
/// @return 만들어진 텍스쳐
QSAPI QgTexture* qg_create_texture(const char* name, const QgImage* image, QgTexFlag flags);

/// @brief 텍스쳐를 파일에서 읽어 만든다
/// @param fuse 퓨즈 번호
/// @param filename 파일 이름
/// @param flags 텍스쳐 플래그
/// @return 읽어서 만든 텍스쳐
QSAPI QgTexture* qg_load_texture(int fuse, const char* filename, QgTexFlag flags);

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
/// @param color 색깔 (널이면 흰색)
/// @param texture 텍스쳐
/// @param coord 텍스쳐 좌표 (널이면 전체 텍스쳐)
QSAPI void qg_draw_sprite(const QmRect* bound, const QmColor* color, QgTexture* texture, const QmVec* coord);

/// @brief 텍스쳐 스프라이트를 회전시켜 그린다
/// @param bound 그릴 영영역
/// @param angle 회전 각도(호도)
/// @param color 색깔 (널이면 흰색)
/// @param texture 텍스쳐
/// @param coord 텍스쳐 좌표 (널이면 전체 텍스쳐)
QSAPI void qg_draw_sprite_ex(const QmRect* bound, float angle, const QmColor* color, QgTexture* texture, const QmVec* coord);


//////////////////////////////////////////////////////////////////////////
// 렌더 오브젝트

/// @brief 노드
struct QGNODE
{
	QnGamBase				base;

	char				NAME[64];
	size_t				HASH;
	QgNode*				NEXT;
	QgNode*				PREV;
	QgNode*				SIBLING;
};

/// @brief 노드의 이름과 그 이름으로 해시를 설정한다
/// @param self 노드
/// @param name 설정할 이름
QSAPI void qg_node_set_name(QgNode * self, const char* name);

/// @brief 버퍼
struct QGBUFFER
{
	QnGamBase				base;

	QgBufferType		type;
	uint				size;
	uint				count;
	ushort				stride;

	bool16				mapped;
};

qn_gam_vt(QGBUFFER)
{
	qn_gam_vt(QNGAMBASE)	base;
	void*(*map)(void*);
	bool (*unmap)(void*);
	bool (*data)(void*, int, const void*);
};

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
	QgNode				base;

	nuint				ref;
};

/// @brief 텍스쳐
struct QGTEXTURE
{
	QnGamBase				base;

	QgPropPixel			prop;
	int					width;
	int					height;
	int					mipmaps;
	QgTexFlag			flags;
};

qn_gam_vt(QGTEXTURE)
{
	qn_gam_vt(QNGAMBASE)	base;
	bool (*bind)(QgTexture*, int);
};


//////////////////////////////////////////////////////////////////////////
// 도움 오브젝트

// 이미지
struct QGIMAGE
{
	QnGamBase				base;

	QgPropPixel			prop;
	int					width;
	int					height;
	int					pitch;
	int					mipmaps;
	byte*				data;
};

QSAPI QgImage* qg_new_image(QgClrFmt fmt, int width, int height);
QSAPI QgImage* qg_new_image_filled(int width, int height, const QmColor* color);
QSAPI QgImage* qg_new_image_gradient_linear(int width, int height, const QmColor* begin, const QmColor* end, float direction);
QSAPI QgImage* qg_new_image_gradient_radial(int width, int height, const QmColor* inner, const QmColor* outer, float density);
QSAPI QgImage* qg_new_image_check_pattern(int width, int height, const QmColor* oddColor, const QmColor* evenColor, int checkWidth, int checkHeight);

QSAPI QgImage* qg_create_image(const void* data, int size);
QSAPI QgImage* qg_load_image(int fuse, const char* filename);

QSAPI bool qg_image_set_pixel(const QgImage* self, int x, int y, const QmColor* color);


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

QN_EXTC_END
