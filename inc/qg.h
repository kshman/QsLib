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
	QGTPG_EX_LINE_LOOP,
	QGTPG_EX_TRI_FAN,
	QGTPG_MAX_VALUE,
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

typedef enum qgShdType
{
	QGSHT_VS = QN_BIT(0),
	QGSHT_PS = QN_BIT(1),
	QGSHT_ALL = QGSHT_VS | QGSHT_PS,
} qgShdType;

typedef enum qgShdRole
{
	QGSHR_MANUAL,
	QGSHR_AUTO,
	QGSHR_DISCARD
} qgShdRole;

typedef enum qgShdConst
{
	QGSHC_UNKNOWN,
	QGSHC_FLOAT1,
	QGSHC_FLOAT2,
	QGSHC_FLOAT3,
	QGSHC_FLOAT4,
	QGSHC_FLOAT16,
	QGSHC_INT1,
	QGSHC_INT2,
	QGSHC_INT3,
	QGSHC_INT4,
	QGSHC_BYTE1,
	QGSHC_BYTE2,
	QGSHC_BYTE3,
	QGSHC_BYTE4,
	QGSHC_SPLR_1D,
	QGSHC_SPLR_2D,
	QGSHC_SPLR_3D,
	QGSHC_SPLR_CUBE,
	QGSHC_MAX_VALUE
} qgShdConst;

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
} qgShdAuto;

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
	int					max_vertex_attrs;
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
	intptr_t			primitives;
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

	qgShdRole			role : 8;			// 0=manual, 1=auto, 2=discard
	qgShdConst			cnst : 8;

	uint16_t			size;
	uint32_t			offset;

	pointer_t			aptr;
	pointer_t			xptr;
} qgVarShader;

typedef void(*qgVarShaderFunc)(pointer_t, int, qgVarShader*, qgShd*);


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
	bool (*_construct)(qgStub*, pointer_t);
	void (*_finalize)(qgStub*);
	bool (*_poll)(qgStub*);
};

QNAPI qgStub* qg_stub_new(const char* title, int width, int height, int flags);
QNAPI void qg_stub_close(qgStub* g);

QNAPI bool qg_stub_loop(qgStub* g);
QNAPI bool qg_stub_poll(qgStub* g, qgEvent* ev);

QNAPI const qgUimMouse* qg_stub_get_mouse(qgStub* g);
QNAPI const qgUimKey* qg_stub_get_key(qgStub* g);
QNAPI bool qg_stub_test_key(qgStub* g, qIkKey key);
QNAPI double qg_stub_get_runtime(qgStub* g);
QNAPI double qg_stub_get_fps(qgStub* g);
QNAPI double qg_stub_get_ref_adv(qgStub* g);
QNAPI double qg_stub_get_def_adv(qgStub* g);
QNAPI void qg_stub_set_delay(qgStub* g, int delay);
QNAPI int qg_stub_get_delay(qgStub* g);

QNAPI int qg_stub_left_events(qgStub* g);
QNAPI int qg_stub_add_event(qgStub* g, const qgEvent* ev);
QNAPI int qg_stub_add_event_type(qgStub* g, qgEventType type);
QNAPI bool qg_stub_pop_event(qgStub* g, qgEvent* ev);


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
	bool (*_construct)(qgRdh*, int);
	void (*_finalize)(qgRdh*);
	void (*_reset)(qgRdh*);
	void (*_clear)(qgRdh*, int, const qnColor*, int, float);

	bool (*begin)(qgRdh*);
	void (*end)(qgRdh*);
	void (*flush)(qgRdh*);

	qgVlo* (*create_layout)(qgRdh*, int, const qgVarLayout*);
	qgShd* (*create_shader)(qgRdh*, const char*);
	qgBuf* (*create_buffer)(qgRdh*, qgBufType, int, int, cpointer_t);

	void (*set_shader)(qgRdh*, qgShd*, qgVlo*);
	bool (*set_index)(qgRdh*, qgBuf*);
	bool (*set_vertex)(qgRdh*, int, qgBuf*);

	bool (*primitive_begin)(qgRdh*, qgTopology, int, int, pointer_t*);
	void (*primitive_end)(qgRdh*);
	bool (*indexed_primitive_begin)(qgRdh*, qgTopology, int, int, pointer_t*, int, int, pointer_t*);
	void (*indexed_primitive_end)(qgRdh*);
	bool (*draw)(qgRdh*, qgTopology, int);
	bool (*draw_indexed)(qgRdh*, qgTopology, int);
};

QNAPI qgRdh* qg_rdh_new(const char* driver, const char* title, int width, int height, int flags);

QNAPI const qgDeviceInfo* qg_rdh_get_device_info(qgRdh* g);
QNAPI const qgRenderInfo* qg_rdh_get_render_info(qgRdh* g);
QNAPI const qgRenderTm* qg_rdh_get_render_tm(qgRdh* g);
QNAPI const qgRenderParam* qg_rdh_get_render_param(qgRdh* g);

QNAPI bool qg_rdh_loop(qgRdh* g);
QNAPI bool qg_rdh_poll(qgRdh* g, qgEvent* ev);

QNAPI bool qg_rdh_begin(qgRdh* g);
QNAPI void qg_rdh_end(qgRdh* g);
QNAPI void qg_rdh_flush(qgRdh* g);

QNAPI void qg_rdh_set_param_vec3(qgRdh* g, int at, const qnVec3* v);
QNAPI void qg_rdh_set_param_vec4(qgRdh* g, int at, const qnVec4* v);
QNAPI void qg_rdh_set_param_mat4(qgRdh* g, int at, const qnMat4* m);
QNAPI void qg_rdh_set_param_weight(qgRdh* g, int count, qnMat4* weight);
QNAPI void qg_rdh_set_clear(qgRdh* g, const qnColor* color);
QNAPI void qg_rdh_set_proj(qgRdh* g, const qnMat4* m);
QNAPI void qg_rdh_set_view(qgRdh* g, const qnMat4* m);
QNAPI void qg_rdh_set_world(qgRdh* g, const qnMat4* m);

QNAPI qgVlo* qg_rdh_create_layout(qgRdh* self, int count, const qgVarLayout* layouts);
QNAPI qgShd* qg_rdh_create_shader(qgRdh* self, const char* name);
QNAPI qgBuf* qg_rdh_create_buffer(qgRdh* g, qgBufType type, int count, int stride, cpointer_t data);

QNAPI void qg_set_shader(qgRdh* self, qgShd* shader, qgVlo* layout);
QNAPI bool qg_rdh_set_index(qgRdh* g, pointer_t buffer);
QNAPI bool qg_rdh_set_vertex(qgRdh* g, int stage, pointer_t buffer);

QNAPI void qg_rdh_primitive_draw(qgRdh* g, qgTopology tpg, int count, int stride, cpointer_t data);
QNAPI void qg_rdh_primitive_draw_indexed(qgRdh* g, qgTopology tpg, int vcount, int vstride, cpointer_t vdata, int icount, int istride, cpointer_t idata);
QNAPI bool qg_rdh_draw(qgRdh* self, qgTopology tpg, int vcount);
QNAPI bool qg_rdh_draw_indexed(qgRdh* self, qgTopology tpg, int icount);


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

QNAPI uint32_t qg_vlo_get_stride(qgVlo* g);
QNAPI uint32_t qg_vlo_get_stage(qgVlo* g, int stage);


// shader
struct qgShd
{
	qgRdGam				base;

	char				name[64];
	funcparam_t			intr;
};

qvt_name(qgShd)
{
	qvt_name(qnGam)		base;
	bool(*bind)(qgShd*, qgShdType, cpointer_t, int, int);
	bool(*bind_shd)(qgShd*, qgShdType, qgShd*);
	//bool(*bind_name)(qgShd* qgShdType, const char*);
	//void(*add_condition)(qgShd*, const char*);
	//void(*clear_condition)(qgShd*);
	bool(*link)(qgShd*);
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
	pointer_t(*map)(qgBuf*);
	bool (*unmap)(qgBuf*);
	bool (*data)(qgBuf*, cpointer_t);
};

QNAPI qgBufType qg_buf_get_type(qgBuf* g);
QNAPI uint32_t qg_buf_get_stride(qgBuf* g);
QNAPI uint32_t qg_buf_get_size(qgBuf* g);

QNAPI pointer_t qg_buf_map(qgBuf* self);
QNAPI bool qg_buf_unmap(qgBuf* self);
QNAPI bool qg_buf_data(qgBuf* self, cpointer_t data);

QN_EXTC_END
