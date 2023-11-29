#pragma once

#include <qn.h>
#include <qkmc.h>

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
// inputs

//
typedef struct qgUimKey
{
	qIkMask				mask : 16;
	bool				key[QIK_MAX_VALUE];
} qgUimKey;

//
typedef struct qgUimMouse
{
	qImMask				mask;
	int					wheel;
	qnPoint				pt;
	qnPoint				last;

	struct
	{
		uint32_t		tick;
		qImButton		btn;
		qnPoint			loc;
	}					clk;
	struct
	{
		int				move;
		uint32_t		tick;
	}					lim;
} qgUimMouse;

//
typedef struct qgUimCtrl
{
	qIcButton			btn : 16;
	qnPoint				trg;
	qnVec2				lthb;
	qnVec2				rthb;
} qgUimCtrl;

//
typedef struct qgUimCtrlInfo
{
	qIcMask				flags;

	int					type : 16;
	int					extend : 16;

	int					battery_type : 8;
	int					battery_level : 8;

	int					headset_type : 8;
	int					headset_level : 8;
} qgUimCtrlInfo;

// 
typedef struct qgUimCtrlCtrlVib
{
	uint16_t			left;
	uint16_t			right;
} qgUimCtrlCtrlVib;


//////////////////////////////////////////////////////////////////////////
// object

typedef enum
{
	QGSTUB_FULLSCREEN = QN_BIT(0),
	QGSTUB_BORDERLESS = QN_BIT(1),
	QGSTUB_RESIZABLE = QN_BIT(2),
	QGSTUB_FOCUS = QN_BIT(3),
	QGSTUB_IDLE = QN_BIT(4),
	QGSTUB_DITHER = QN_BIT(30),
} qgStubFlag;

typedef enum
{
	QGSTI_EXIT = QN_BIT(0),
	QGSTI_VIRTUAL = QN_BIT(1),
	QGSTI_ACTIVE = QN_BIT(2),
	QGSTI_LAYOUT = QN_BIT(3),
	// DROP / CURSOR / SCRSAVE
	QGSTI_ACS = QN_BIT(13),
	QGSTI_PAUSE = QN_BIT(14),
} qgStubStat;

struct qgStub
{
	qnGam				base;

	pointer_t			handle;

	qgStubFlag			flags;
	qgStubStat			stats;
	uintptr_t			polls;

	qnTimer*			timer;
	uint32_t			delay;
	double				fps;
	double				run;
	float				refadv;
	float				advance;

	qnRect				bound;
	qnPoint				size;
	qnPoint				limit;

	qgUimKey			key;
	qgUimMouse			mouse;
};

qvt_name(qgStub)
{
	qvt_name(qnGam)		base;
};

QNAPI qgStub* qg_stub_instance;

QNAPI qgStub* qg_stub_new(const char* driver, const char* title, int width, int height, int flags);
QNAPI bool qg_stub_close(pointer_t g);
QNAPI void qg_stub_dispose(pointer_t g);

QNAPI bool qg_stub_poll(pointer_t g);

QNAPI const qgUimMouse* qg_stub_get_mouse(pointer_t g);
QNAPI const qgUimKey* qg_stub_get_key(pointer_t g);
QNAPI bool qg_stub_test_key(pointer_t g, qIkKey key);
QNAPI double qg_stub_get_runtime(pointer_t g);
QNAPI double qg_stub_get_fps(pointer_t g);
QNAPI double qg_stub_get_ref_adv(pointer_t g);
QNAPI double qg_stub_get_def_adv(pointer_t g);
QNAPI void qg_stub_set_delay(pointer_t g, int delay);
QNAPI int qn_stub_get_delay(pointer_t g);

QN_EXTC_END
