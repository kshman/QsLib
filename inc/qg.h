#pragma once

#include <qn.h>
#include <qkmc.h>

//////////////////////////////////////////////////////////////////////////
// version
#define _QG_VERSION_					202311L

// definition
typedef struct qgStub qgStub;			/**< STUB */
typedef struct qgRdh qgRdh;				/**< RENDERER */
typedef struct qgRdGam qgRdGam;			/**< RENDER OBJECT */
typedef struct qgRds qgRds;				/**< DEPTH&STENCIL */
typedef struct qgRsz qgRsz;				/**< RASTERIZER */
typedef struct qgBld qgBld;				/**< BLEND */
typedef struct qgSpr qgSpr;				/**< SAMPLER */
typedef struct qgShd qgShd;				/**< SHADER */
typedef struct qgVlo qgVlo;				/**< VERTEX LAYOUT */
typedef struct qgBuf qgBuf;				/**< BUFFER */

// instance
QNAPI qgStub* qg_stub_instance;
QNAPI qgRdh* qg_rdh_instance;

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

typedef enum qgLoUsage
{
	QGLOU_POSITION,
	QGLOU_COLOR,		// diffuse & specular
	QGLOU_WEIGHT,		// blend weights
	QGLOU_INDEX,		// blend indices
	QGLOU_NORMAL,
	QGLOU_TEXTURE,		// texture coord
	QGLOU_TANGENT,
	QGLOU_BINORMAL,
	QGLOU_MAX_VALUE
} qgLoUsage;

typedef enum qgLoType
{
	QGLOT_FLOAT1,
	QGLOT_FLOAT2,
	QGLOT_FLOAT3,
	QGLOT_FLOAT4,
	QGLOT_HALF2,
	QGLOT_HALF4,
	QGLOT_BYTE2,
	QGLOT_BYTE4,
	QGLOT_BYTE4N,
	QGLOT_SHORT2,
	QGLOT_SHORT4,
	QGLOT_SHORT2N,
	QGLOT_COLOR,
	QGLOT_MAX_VALUE
} qgLoType;

typedef enum qgLoStage
{
	QGLOS_1,
	QGLOS_2,
	QGLOS_3,
	QGLOS_4,
	QGLOS_MAX_VALUE,
} qgLoStage;

typedef enum qgShdName
{
	QGSHD_VS,
	QGSHD_PS,
} qgShdName;

typedef enum qgShdRole
{
	QGSHR_MANUAL,
	QGSHD_AUTO,
	QGSHD_DISCARD
} qgShdRole;

typedef enum qgShdType
{
	QGSHT_UNKNOWN,
	QGSHT_FLOAT1,
	QGSHT_FLOAT2,
	QGSHT_FLOAT3,
	QGSHT_FLOAT4,
	QGSHT_FLOAT16,
	QGSHT_INT1,
	QGSHT_INT2,
	QGSHT_INT3,
	QGSHT_INT4,
	QGSHT_BYTE1,
	QGSHT_BYTE2,
	QGSHT_BYTE3,
	QGSHT_BYTE4,
	QGSHT_SPLR_1D,
	QGSHT_SPLR_2D,
	QGSHT_SPLR_3D,
	QGSHT_SPLR_CUBE,
	QGSHT_MAX_VALUE
} qgShdType;

typedef enum qgShdAuto
{
	QGSHA_ORTHO_PROJ,
	QGSHA_WORLD,
	QGSHA_VIEW,
	QGSHA_PROJ,
	QGSHA_VIEW_PROJ,
	QGSHA_INV_VIEW,
	QGSHA_WORLD_VIEW_PROJ,
	QGSHA_TEX0,
	QGSHA_TEX1,
	QGSHA_TEX2,
	QGSHA_TEX3,
	QGSHA_TEX4,
	QGSHA_TEX5,
	QGSHA_TEX6,
	QGSHA_TEX7,
	QGSHA_PROP_VEC0,
	QGSHA_PROP_VEC1,
	QGSHA_PROP_VEC2,
	QGSHA_PROP_VEC3,
	QGSHA_PROP_MAT0,
	QGSHA_PROP_MAT1,
	QGSHA_PROP_MAT2,
	QGSHA_PROP_MAT3,
	QGSHA_MAT_PALETTE,
	QGSHA_MAX_VALUE
} qgShaderAuto;

typedef enum qgBufType
{
	QGBUF_UNKNOWN,
	QGBUF_INDEX,
	QGBUF_VERTEX,
} qgBufType;

typedef enum qgResAccess
{
	QGRSA_NONE = 0,
	QGRSA_RD = QN_BIT(0),
	QGRSA_WR = QN_BIT(1),
	QGRSA_RW = QGRSA_RD | QGRSA_WR,
} qgResAccess;

typedef enum qgClear
{
	QGCLR_DEPTH = QN_BIT(0),
	QGCLR_STENCIL = QN_BIT(1),
	QGCLR_RENDER = QN_BIT(2),
} qgClear;

typedef enum qgFlag
{
	QGFLAG_FULLSCREEN = QN_BIT(0),
	QGFLAG_BORDERLESS = QN_BIT(1),
	QGFLAG_RESIZABLE = QN_BIT(2),
	QGFLAG_FOCUS = QN_BIT(3),
	QGFLAG_IDLE = QN_BIT(4),
	QGFLAG_DITHER = QN_BIT(16),
	QGFLAG_MSAA = QN_BIT(17),
} qgFlag;

typedef enum qgStti
{
	QGSTTI_VIRTUAL = QN_BIT(1),
	QGSTTI_ACTIVE = QN_BIT(2),
	QGSTTI_LAYOUT = QN_BIT(3),
	QGSTTI_ACS = QN_BIT(13),
	QGSTTI_PAUSE = QN_BIT(14),
	QGSTTI_DROP = QN_BIT(15),
	QGSTTI_CURSOR = QN_BIT(16),
	QGSTTI_SCRSAVE = QN_BIT(17),
	QGSTTI_EXIT = QN_BIT(30),
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
	qgClrFmt			clrfmt;
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
// object

typedef struct qgPropPixel
{
	qgClrFmt			fmt;
	uint8_t				bpp;
	uint8_t				rr, rl;
	uint8_t				gr, gl;
	uint8_t				br, bl;
	uint8_t				ar, al;
} qgPixelProp;

typedef struct qgPropDepthStencil
{
	bool				d_write : 8;
	qgCmpOp				d_func : 8;

	bool				f_enable : 8;
	qgCmpOp				f_func : 8;
	qgStencilOp			f_pass : 8;
	qgStencilOp			f_fail : 8;
	qgStencilOp			f_depth : 8;

	bool				b_enable : 8;
	qgCmpOp				b_func : 8;
	qgStencilOp			b_pass : 8;
	qgStencilOp			b_fail : 8;
	qgStencilOp			b_depth : 8;

	uint8_t				m_read;
	uint8_t				m_write;
} qgPropDepthStencil;

typedef struct qgPropRasterizer
{
	qgFill				fill : 8;
	qgCull				cull : 8;
	float				depth_bias;
	float				slope_scale;
} qgPropRasterizer;

typedef struct qgPropBlend
{
	qgBlendOp			c_op : 16;
	qgFactor			c_src : 8;
	qgFactor			c_dst : 8;

	qgBlendOp			a_op : 16;
	qgFactor			a_src : 8;
	qgFactor			a_dst : 8;

	qgClrMask			mask;
} qgPropBlend;

typedef struct qgVarLayout
{
	qgLoUsage			usage : 16;
	int					index : 16;
	qgLoType			type : 16;
	int					slot : 16;
	int					zz;
} qgVarLayout;

typedef struct qgVarShader
{
	size_t				hash;
	char				name[32];

	int					role : 8;			// 0=manual, 1=auto, 2=discard
	qgShdType			type : 8;

	uint16_t			size;
	uintptr_t			offset;

	pointer_t			aptr;
	pointer_t			dptr;
} qgVarShader;


//////////////////////////////////////////////////////////////////////////
// stub & render device

// stub
struct qgStub
{
	qnGam				base;

	pointer_t			oshandle;

	qgFlag				flags;
	qgStti				sttis;
	uint32_t			delay;
	int32_t				_padding[1];

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
	void (*_clear)(pointer_t, int, const qnColor*, int, float);

	bool (*begin)(pointer_t);
	void (*end)(pointer_t);
	void (*flush)(pointer_t);

	bool (*set_index)(pointer_t, pointer_t);
	bool (*set_vertex)(pointer_t, int, pointer_t);

	pointer_t(*create_layout)(pointer_t, int, const qgVarLayout*);
	pointer_t(*create_buffer)(pointer_t, qgBufType, int, int, cpointer_t);

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

QNAPI bool qg_rdh_set_index(pointer_t g, pointer_t buffer);
QNAPI bool qg_rdh_set_vertex(pointer_t g, int stage, pointer_t buffer);

QNAPI qgBuf* qg_rdh_create_buffer(pointer_t g, qgBufType type, int count, int stride, cpointer_t data);

QNAPI void qg_rdh_draw_primitive(pointer_t g, qgTopology tpg, int count, int stride, cpointer_t data);
QNAPI void qg_rdh_draw_indexed_primitive(pointer_t g, qgTopology tpg, int vcount, int vstride, cpointer_t vdata, int icount, int istride, cpointer_t idata);


//////////////////////////////////////////////////////////////////////////
// object

//
struct qgRdGam
{
	qnGam				base;
};


// layout
struct qgVlo
{
	qgRdGam				base;

	int					stride;
	uint16_t			stage[QGLOS_MAX_VALUE];
};

QNAPI uint32_t qg_vlo_get_stride(pointer_t g);
QNAPI uint32_t qg_vlo_get_stage(pointer_t g, int stage);


// shader
struct qgShd
{
	qgRdGam				base;

	pointer_t			intr;
};

qvt_name(qgShd)
{
	qvt_name(qnGam)		base;
	bool(*bind)(pointer_t, qgShdName, cpointer_t, int, int);
	bool(*bind_shd)(pointer_t, qgShdName, pointer_t);
	bool(*bind_name)(pointer_t qgShdName, const char*);
	void(*add_condition)(pointer_t, const char*);
	void(*clear_condition)(pointer_t);
	void(*link)(pointer_t);
};


// buffer
struct qgBuf
{
	qgRdGam				base;

	qgBufType			type : 16;
	uint16_t			stride;
	uint32_t			size;
};

qvt_name(qgBuf)
{
	qvt_name(qnGam)		base;
	pointer_t(*map)(pointer_t);
	bool (*unmap)(pointer_t);
	bool (*data)(pointer_t, pointer_t);
};

QNAPI qgBufType qg_buf_get_type(pointer_t g);
QNAPI uint32_t qg_buf_get_stride(pointer_t g);
QNAPI uint32_t qg_buf_get_size(pointer_t g);
QNAPI bool qg_buf_mapped_data(pointer_t g, cpointer_t data, int size);

QN_EXTC_END
