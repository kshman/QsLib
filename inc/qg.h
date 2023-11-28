#pragma once

#include <qn.h>

//////////////////////////////////////////////////////////////////////////
// version
#define _QG_VERSION_					202311L

// definition
typedef struct qgStub qgStub;

QN_EXTC_BEGIN

//////////////////////////////////////////////////////////////////////////
// data

typedef enum
{
	QGSTUB_FULLSCREEN = QN_BIT(0),
	QGSTUB_BORDERLESS = QN_BIT(1),
	QGSTUB_RESIZABLE = QN_BIT(2),
	QGSTUB_FOCUS = QN_BIT(3),
} qgStubFlag;

typedef enum
{
	QGCF_NONE,
	QGCF8_L,
	QGCF16_L,
	QGCF16_RGB,
	QGCF16_RGBA,
	QGCF32_RGBA,
	QGCF32_BGRA,
	QGCF16F_R,
	QGCF64F_BGRA,
	QGCF32F_R,
	QGCF128F_BGRA,
	QGCF_DXT1,
	QGCF_DXT3,
	QGCF_DXT5,
	QGCF_MAX_VALUE,
} qgClrFmt;

typedef enum
{
	QGCM_RED = QN_BIT(0),
	QGCM_GREEN = QN_BIT(1),
	QGCM_BLUE = QN_BIT(2),
	QGCM_ALPHA = QN_BIT(3),
	QGCM_CMPR = QN_BIT(4),
	QGCM_FLOAT = QN_BIT(5),
	QGCM_REV = QN_BIT(6),

	QGCM_RGB = QGCM_RED | QGCM_GREEN | QGCM_BLUE,
	QGCM_RGBA = QGCM_RED | QGCM_GREEN | QGCM_BLUE | QGCM_ALPHA,
} qgClrMask;

typedef enum
{
	QGTPG_POINT,
	QGTPG_LINE,
	QGTPG_LINE_STRIP,
	QGTPG_TRI,
	QGTPG_TRI_STRIP,
} qgTopology;

typedef enum
{
	QGFLL_POINT,
	QGFLL_WIRE,
	QGFLL_SOLID,
} qgFill;

typedef enum
{
	QGCUL_NONE,
	QGCUL_FRONT,
	QGCUL_BACK,
} qgCull;

typedef enum
{
	QGCMP_NEVER,
	QGCMP_LE,
	QGCMP_EQ,
	QGCMP_LEQ,
	QGCMP_GR,
	QGCMP_NEQ,
	QGCMP_GEQ,
	QGCMP_ALWAYS,
} qgCmpOp;

typedef enum
{
	QGSTP_KEEP,
	QGSTP_ZERO,
	QGSTP_REPLACE,
	QGSTP_SAT_INC,
	QGSTP_SAT_DEC,
	QGSTP_INVERT,
	QGSTP_INC,
	QGSTP_DEC,
} qgStencilOp;

typedef enum
{
	QGBLO_ADD,
	QGBLO_SUB,
	QGBLO_RSUB,
	QGBLO_MIN,
	QGBLO_MAX,
} qgBlendOp;

typedef enum
{
	QGBFT_ZERO,
	QGBFT_ONE,
	QGBFT_SC,
	QGBFT_SC_INV,
	QGBFT_SA,
	QGBFT_SA_INV,
	QGBFT_DA,
	QGBFT_DA_INV,
	QGBFT_DC,
	QGBFT_DC_INV,
	QGBFT_SAS,
} qgFactor;


//////////////////////////////////////////////////////////////////////////
// object

struct qgStub
{
	qnGam				base;

	pointer_t			handle;
	qnTimer*			timer;

	uint32_t			delay;
	double				active;
	double				running;
	double				fps;

	qnRect				bound;
	qnPoint				size;
	qnPoint				limit;
	qgClrFmt			format;
};

struct _qgvt_stub
{
	struct _qnvt_gam	base;
};

QNAPI qgStub* qg_stub_instance;

QNAPI bool qg_stub_on_init(pointer_t g);
QNAPI bool qg_stub_on_disp(pointer_t g);
QNAPI void qg_stub_dispose(pointer_t g);

QN_EXTC_END
