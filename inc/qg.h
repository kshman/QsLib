#pragma once

#include <qn.h>
#include <qkmc.h>

//////////////////////////////////////////////////////////////////////////
// version
#define _QG_VERSION_					202311L

// definition
typedef struct qgStub qgStub;
typedef struct qgRdh qgRdh;

// instance
QNAPI qgStub* qg_stub_instance;

QN_EXTC_BEGIN

//////////////////////////////////////////////////////////////////////////
// types

typedef enum qgClrFmt
{
	QGCF_NONE,
	QGCF8_L,
	QGCF16_L,
	QGCF16_RGB,
	QGCF16_RGBA,
	QGCF32_RGB,
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

typedef enum qgClrMask
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

typedef enum qgTopology
{
	QGTPG_POINT,
	QGTPG_LINE,
	QGTPG_LINE_STRIP,
	QGTPG_TRI,
	QGTPG_TRI_STRIP,
} qgTopology;

typedef enum qgFill
{
	QGFLL_POINT,
	QGFLL_WIRE,
	QGFLL_SOLID,
} qgFill;

typedef enum qgCull
{
	QGCUL_NONE,
	QGCUL_FRONT,
	QGCUL_BACK,
} qgCull;

typedef enum qgCmpOp
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

typedef enum qgStencilOp
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

typedef enum qgBlendOp
{
	QGBLO_ADD,
	QGBLO_SUB,
	QGBLO_RSUB,
	QGBLO_MIN,
	QGBLO_MAX,
} qgBlendOp;

typedef enum qgFactor
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

typedef enum qgFlag
{
	QGFLAG_FULLSCREEN	= QN_BIT(0),
	QGFLAG_BORDERLESS	= QN_BIT(1),
	QGFLAG_RESIZABLE	= QN_BIT(2),
	QGFLAG_FOCUS		= QN_BIT(3),
	QGFLAG_IDLE			= QN_BIT(4),
	QGFLAG_DITHER		= QN_BIT(16),
	QGFLAG_MSAA			= QN_BIT(17),
} qgFlag;

typedef enum qgStti
{
	QGSTTI_VIRTUAL		= QN_BIT(1),
	QGSTTI_ACTIVE		= QN_BIT(2),
	QGSTTI_LAYOUT		= QN_BIT(3),
	QGSTTI_ACS			= QN_BIT(13),
	QGSTTI_PAUSE		= QN_BIT(14),
	QGSTTI_DROP			= QN_BIT(15),
	QGSTTI_CURSOR		= QN_BIT(16),
	QGSTTI_SCRSAVE		= QN_BIT(17),
	QGSTTI_EXIT			= QN_BIT(30),
} qgStti;

typedef enum qgEventType
{
	QGEV_NONE,
	QGEV_ACTIVE,
	QGEV_LAYOUT,
	QGEV_KEYDOWN,
	QGEV_KEYUP,
	QGEV_MOUSEMOVE,
	QGEV_MOUSEDOWN,
	QGEV_MOUSEUP,
	QGEV_MOUSEWHEEL,
	QGEV_MOUSEDOUBLE,
	QGEV_EXIT = 255,
} qgEventType;


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
// stub data

#define QNEVENT_MAX_VALUE		5000

typedef union qgEvent
{
	qgEventType			ev;
	struct Active
	{
		qgEventType			ev;
		int32_t				active;	// bool
		double				delta;
	}						active;
	struct Layout
	{
		qgEventType			ev;
		qnRect				bound;
		int32_t				_pad[1];
	}					layout;
	struct Keyboard
	{
		qgEventType			ev;
		int16_t				pressed;
		int16_t				repeat;
		qIkKey				key;
		qIkMask				state;
	}					key;
	struct MouseMove {
		qgEventType			ev;
		int32_t				x;
		int32_t				y;
		int32_t				dx;
		int32_t				dy;
		qImMask				state;
	}					mmove;
	struct MouseButton {
		qgEventType			ev;
		int32_t				x;
		int32_t				y;
		qImButton			button;
		qImMask				state;
		int32_t				_pad[1];
	}					mbutton;
	struct MouseWheel {
		qgEventType			ev;
		int32_t				dir;
		int32_t				x;
		int32_t				y;
	}					mwheel;
} qgEvent;

QN_LIST_DECL(qgListEvent, qgEvent);

typedef struct qgDeviceInfo
{
	char				name[64];
	char				renderer[64];
	char				vendor[64];
	int					renderer_version;
	int					shader_version;
	int					max_tex_dim;
	int					max_tex_count;
	int					max_off_count;
	int					tex_image_flag;
} qgDeviceInfo;

typedef struct qgRenderInfo
{
	intptr_t			frames;
	intptr_t			begins;
	intptr_t			ends;
	intptr_t			invokes;
	intptr_t			shaders;
	intptr_t			transforms;
	intptr_t			draws;
	intptr_t			vertices;
	bool				flush;
} qgRenderInfo;

typedef struct qgRenderTm
{
	qnMat4				world;
	qnMat4				view;
	qnMat4				proj;
	qnMat4				vp;
	qnMat4				inv;		// inverse view
	qnMat4				ortho;		// ortho transform
	qnMat4				frm;		// tex formation
	qnMat4				tex[4];
} qgRenderTm;

typedef struct qgRenderParam
{
	int					bones;
	qnMat4*				bonptr;
	qnVec4				v[4];
	qnMat4				m[4];
	qnColor				clear;
} qgRenderParam;


//////////////////////////////////////////////////////////////////////////
// stub

struct qgStub
{
	qnGam				base;

	pointer_t			handle;
	pointer_t			oshandle;

	qgFlag				flags;
	qgStti				sttis;
	uint32_t			delay;
	qgClrFmt			clrfmt;

	qnTimer*			timer;
	double				fps;
	double				run;

	float				refadv;
	float				advance;

	qnRect				bound;
	qnPoint				size;

	qgUimKey			key;
	qgUimMouse			mouse;

	qgListEvent			events;
};

qvt_name(qgStub)
{
	qvt_name(qnGam)		base;
	bool (*_construct)(pointer_t, pointer_t);
	void (*_finalize)(pointer_t);
	bool (*_poll)(pointer_t);
};

QNAPI qgStub* qg_stub_new(const char* title, int width, int height, int flags);
QNAPI void qg_stub_close(pointer_t g);

QNAPI bool qg_stub_loop(pointer_t g);
QNAPI bool qg_stub_poll(pointer_t g, qgEvent* ev);

QNAPI const qgUimMouse* qg_stub_get_mouse(pointer_t g);
QNAPI const qgUimKey* qg_stub_get_key(pointer_t g);
QNAPI bool qg_stub_test_key(pointer_t g, qIkKey key);
QNAPI double qg_stub_get_runtime(pointer_t g);
QNAPI double qg_stub_get_fps(pointer_t g);
QNAPI double qg_stub_get_ref_adv(pointer_t g);
QNAPI double qg_stub_get_def_adv(pointer_t g);
QNAPI void qg_stub_set_delay(pointer_t g, int delay);
QNAPI int qg_stub_get_delay(pointer_t g);

QNAPI int qg_stub_left_events(pointer_t g);
QNAPI int qg_stub_add_event(pointer_t g, const qgEvent* ev);
QNAPI int qg_stub_add_event_type(pointer_t g, qgEventType type);
QNAPI bool qg_stub_pop_event(pointer_t g, qgEvent* ev);


//////////////////////////////////////////////////////////////////////////
// render device

struct qgRdh
{
	qnGam				base;

	qgStub*				stub;

	qgRenderInfo		info;
	qgRenderTm			tm;
	qgRenderParam		param;

	qgDeviceInfo		caps;
};

qvt_name(qgRdh)
{
	qvt_name(qnGam)		base;
	bool (*_construct)(pointer_t, int);
	void (*_finalize)(pointer_t);
	void (*_reset)(pointer_t);

	bool (*begin)(pointer_t);
	void (*end)(pointer_t);
	void (*flush)(pointer_t);

	bool (*primitive_begin)(pointer_t, qgTopology, int, int, pointer_t*);
	void (*primitive_end)(pointer_t);
	bool (*indexed_primitive_begin)(pointer_t, qgTopology, int, int, pointer_t*, int, int, pointer_t*);
	void (*indexed_primitive_end)(pointer_t);
};

QNAPI qgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags);

QNAPI const qgDeviceInfo* qg_rdh_get_device_info(pointer_t g);
QNAPI const qgRenderInfo* qg_rdh_get_render_info(pointer_t g);
QNAPI const qgRenderTm* qg_rdh_get_render_tm(pointer_t g);
QNAPI const qgRenderParam* qg_rdh_get_render_param(pointer_t g);

QNAPI bool qg_rdh_loop(pointer_t g);
QNAPI bool qg_rdh_poll(pointer_t g, qgEvent* ev);

QNAPI bool qg_rdh_begin(pointer_t g);
QNAPI void qg_rdh_end(pointer_t g);
QNAPI void qg_rdh_flush(pointer_t g);

QNAPI void qg_rdh_set_param_vec3(pointer_t g, int at, const qnVec3* v);
QNAPI void qg_rdh_set_param_vec4(pointer_t g, int at, const qnVec4* v);
QNAPI void qg_rdh_set_param_mat4(pointer_t g, int at, const qnMat4* m);
QNAPI void qg_rdh_set_param_weight(pointer_t g, int count, qnMat4* weight);
QNAPI void qg_rdh_set_clear(pointer_t g, const qnColor* color);
QNAPI void qg_rdh_set_proj(pointer_t g, const qnMat4* m);
QNAPI void qg_rdh_set_view(pointer_t g, const qnMat4* m);
QNAPI void qg_rdh_set_world(pointer_t g, const qnMat4* m);

QNAPI void qg_rdh_draw_primitive(pointer_t g, qgTopology tpg, int count, int stride, cpointer_t data);
QNAPI void qg_rdh_draw_indexed_primitive(pointer_t g, qgTopology tpg, int vcount, int vstride, cpointer_t vdata, int icount, int istride, cpointer_t idata);

QN_EXTC_END
