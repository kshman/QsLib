#pragma once

#include <qs_qn.h>
#include <qs_math.h>
#include <qs_kmc.h>

//////////////////////////////////////////////////////////////////////////
// definition

// maximum event queue
#define QNEVENT_MAX_VALUE		5000

// typedef
typedef struct _QgRdh					QgRdh;		/**< RENDERER */
typedef struct _QgGam					QgGam;		/**< RENDER OBJECT */
typedef struct _QgRds					QgRds;		/**< DEPTH&STENCIL */
typedef struct _QgRsz					QgRsz;		/**< RASTERIZER */
typedef struct _QgBld					QgBld;		/**< BLEND */
typedef struct _QgSpr					QgSpr;		/**< SAMPLER */
typedef struct _QgShd					QgShd;		/**< SHADER */
typedef struct _QgVlo					QgVlo;		/**< VERTEX LAYOUT */
typedef struct _QgBuf					QgBuf;		/**< BUFFER */

typedef enum _QgClrFmt					QgClrFmt;
typedef enum _QgClrMask					QgClrMask;
typedef enum _QgTopology				QgTopology;
typedef enum _QgFill					QgFill;
typedef enum _QgCull					QgCull;
typedef enum _QgCmpOp					QgCmpOp;
typedef enum _QgStencilOp				QgStencilOp;
typedef enum _QgBlendOp					QgBlendOp;
typedef enum _QgFactor					QgFactor;
typedef enum _QgLoUsage					QgLoUsage;
typedef enum _QgLoType					QgLoType;
typedef enum _QgLoStage					QgLoStage;
typedef enum _QgShdType					QgShdType;
typedef enum _QgShdRole					QgShdRole;
typedef enum _QgShdConst				QgShdConst;
typedef enum _QgShdAuto					QgShdAuto;
typedef enum _QgBufType					QgBufType;
typedef enum _QgResAccess				QgResAccess;
typedef enum _QgClear					QgClear;
typedef enum _QgFlag					QgFlag;
typedef enum _QgStubStat				QgStubStat;
typedef enum _QgEventType				QgEventType;

typedef struct _QgPropPixel				QgPropPixel;
typedef struct _QgPropDepthStencil		QgPropDepthStencil;
typedef struct _QgPropRasterizer		QgPropRasterizer;
typedef struct _QgPropBlend				QgPropBlend;
typedef struct _QgPropLayout			QgPropLayout;
typedef struct _QgVarShader				QgVarShader;
typedef void(*QgVarShaderFunc)(void*, QgVarShader*, QgShd*);

typedef union _QgEvent					QgEvent;
typedef struct _QgUimKey                QgUimKey;
typedef struct _QgUimMouse              QgUimMouse;
typedef struct _QgUimCtrl               QgUimCtrl;
typedef struct _QgUimCtrlInfo           QgUimCtrlInfo;
typedef struct _QgUimCtrlVib            QgUimCtrlVib;
typedef struct _QgDeviceInfo            QgDeviceInfo;
typedef struct _QgRenderInvoke          QgRenderInvoke;
typedef struct _QgRenderTm              QgRenderTm;
typedef struct _QgRenderParam           QgRenderParam;


QN_EXTC_BEGIN

//////////////////////////////////////////////////////////////////////////
// types

enum _QgClrFmt
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
};

enum _QgClrMask
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
};

enum _QgTopology
{
	QGTPG_POINT,
	QGTPG_LINE,
	QGTPG_LINE_STRIP,
	QGTPG_TRI,
	QGTPG_TRI_STRIP,
	QGTPG_EX_LINE_LOOP,
	QGTPG_EX_TRI_FAN,
	QGTPG_MAX_VALUE,
};

enum _QgFill
{
	QGFLL_POINT,
	QGFLL_WIRE,
	QGFLL_SOLID,
};

enum _QgCull
{
	QGCUL_NONE,
	QGCUL_FRONT,
	QGCUL_BACK,
};

enum _QgCmpOp
{
	QGCMP_NEVER,
	QGCMP_LE,
	QGCMP_EQ,
	QGCMP_LEQ,
	QGCMP_GR,
	QGCMP_NEQ,
	QGCMP_GEQ,
	QGCMP_ALWAYS,
};

enum _QgStencilOp
{
	QGSTP_KEEP,
	QGSTP_ZERO,
	QGSTP_REPLACE,
	QGSTP_SAT_INC,
	QGSTP_SAT_DEC,
	QGSTP_INVERT,
	QGSTP_INC,
	QGSTP_DEC,
};

enum _QgBlendOp
{
	QGBLO_ADD,
	QGBLO_SUB,
	QGBLO_RSUB,
	QGBLO_MIN,
	QGBLO_MAX,
};

enum _QgFactor
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
};

enum _QgLoUsage
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
};

enum _QgLoType
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
	QGLOT_COLORN,
	QGLOT_MAX_VALUE
};

enum _QgLoStage
{
	QGLOS_1,
	QGLOS_2,
	QGLOS_3,
	QGLOS_4,
	QGLOS_MAX_VALUE,
};

enum _QgShdType
{
	QGSHT_VS = QN_BIT(0),
	QGSHT_PS = QN_BIT(1),
	QGSHT_ALL = QGSHT_VS | QGSHT_PS,
};

enum _QgShdRole
{
	QGSHR_MANUAL,
	QGSHR_AUTO,
	QGSHR_DISCARD
};

enum _QgShdConst
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
};

enum _QgShdAuto
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
};

enum _QgBufType
{
	QGBUF_UNKNOWN,
	QGBUF_INDEX,
	QGBUF_VERTEX,
};

enum _QgResAccess
{
	QGRSA_NONE = 0,
	QGRSA_RD = QN_BIT(0),
	QGRSA_WR = QN_BIT(1),
	QGRSA_RW = QGRSA_RD | QGRSA_WR,
};

enum _QgClear
{
	QGCLR_DEPTH = QN_BIT(0),
	QGCLR_STENCIL = QN_BIT(1),
	QGCLR_RENDER = QN_BIT(2),
};

enum _QgFlag
{
	QGFLAG_FULLSCREEN = QN_BIT(0),
	QGFLAG_BORDERLESS = QN_BIT(1),
	QGFLAG_RESIZABLE = QN_BIT(2),
	QGFLAG_FOCUS = QN_BIT(3),
	QGFLAG_IDLE = QN_BIT(4),
	QGFLAG_VSYNC = QN_BIT(16),
	QGFLAG_DITHER = QN_BIT(17),
	QGFLAG_MSAA = QN_BIT(18),
};

enum _QgStubStat
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
};

enum _QgEventType
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
};


//////////////////////////////////////////////////////////////////////////
// properties

//
struct _QgPropPixel
{
	QgClrFmt			fmt;
	byte				bpp;
	byte				rr, rl;
	byte				gr, gl;
	byte				br, bl;
	byte				ar, al;
};

//
struct _QgPropDepthStencil
{
	uint				d_write : 8;
	QgCmpOp				d_func : 8;

	uint				f_enable : 8;
	QgCmpOp				f_func : 8;
	QgStencilOp			f_pass : 8;
	QgStencilOp			f_fail : 8;
	QgStencilOp			f_depth : 8;

	uint				b_enable : 8;
	QgCmpOp				b_func : 8;
	QgStencilOp			b_pass : 8;
	QgStencilOp			b_fail : 8;
	QgStencilOp			b_depth : 8;

	byte				m_read;
	byte				m_write;
};

//
struct _QgPropRasterizer
{
	QgFill				fill : 8;
	QgCull				cull : 8;
	float				depth_bias;
	float				slope_scale;
};

//
struct _QgPropBlend
{
	QgBlendOp			c_op : 16;
	QgFactor			c_src : 8;
	QgFactor			c_dst : 8;

	QgBlendOp			a_op : 16;
	QgFactor			a_src : 8;
	QgFactor			a_dst : 8;

	QgClrMask			mask;
};

//
struct _QgPropLayout
{
	QgLoStage			stage : 8;			// QgLoStage
	int					index : 8;
	QgLoUsage			usage : 8;
	QgLoType			type : 8;
};

//
struct _QgVarShader
{
	char				name[64];

	QgShdRole			role : 8;			// 0=manual, 1=auto, 2=discard
	QgShdConst			cnst : 8;

	ushort				size;
	uint				offset;				// variable offset
};

//
union _QgEvent
{
	QgEventType			ev;
	struct Active
	{
		QgEventType			ev;
		int					active;	// bool
		double				delta;
	}					active;
	struct Layout
	{
		QgEventType			ev;
		QnRect				bound;
		int					_pad[1];
	}					layout;
	struct Keyboard
	{
		QgEventType			ev;
		int16_t				pressed;
		int16_t				repeat;
		QikKey				key;
		QikMask				state;
	}					key;
	struct MouseMove {
		QgEventType			ev;
		int					x;
		int					y;
		int					dx;
		int					dy;
		QimMask				state;
	}					mmove;
	struct MouseButton {
		QgEventType			ev;
		int					x;
		int					y;
		QimButton			button;
		QimMask				state;
		int					_pad[1];
	}					mbutton;
	struct MouseWheel {
		QgEventType			ev;
		int					dir;
		int					x;
		int					y;
	}					mwheel;
};

//
struct _QgUimKey
{
	QikMask				mask : 16;
	bool				key[QIK_MAX_VALUE];
};

//
struct _QgUimMouse
{
	QimMask				mask;
	int					wheel;
	QnPoint				pt;
	QnPoint				last;

	struct
	{
		uint				tick;
		QimButton			btn;
		QnPoint				loc;
	}					clk;
	struct
	{
		int					move;
		uint				tick;
	}					lim;
};

//
struct _QgUimCtrl
{
	QicButton			btn : 16;
	QnPoint				trg;
	QnVec2				lthb;
	QnVec2				rthb;
};

//
struct _QgUimCtrlInfo
{
	QicMask				flags;

	int					type : 16;
	int					extend : 16;

	int					battery_type : 8;
	int					battery_level : 8;

	int					headset_type : 8;
	int					headset_level : 8;
};

// 
struct _QgUimCtrlVib
{
	ushort				left;
	ushort				right;
};

//
struct _QgDeviceInfo
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
	QgClrFmt			clrfmt;
	bool				test_stage_valid;
};

struct _QgRenderInvoke
{
	nint				invokes;
	nint				begins;
	nint				ends;
	nint				shaders;
	nint				params;
	nint				transforms;
	nint				draws;
	nint				primitives;

	nint				frames;
	bool				flush;
};

struct _QgRenderTm
{
	QnPoint				size;
	QnVec2				z;
	QnMat4				world;
	QnMat4				view;
	QnMat4				proj;
	QnMat4				vipr;
	QnMat4				inv;		// inverse view
	QnMat4				ortho;		// ortho transform
	QnMat4				frm;		// tex formation
	QnMat4				tex[4];
};

struct _QgRenderParam
{
	int					bones;
	QnMat4*				bonptr;
	QnVec4				v[4];
	QnMat4				m[4];
	QnColor				bgc;
};


//////////////////////////////////////////////////////////////////////////
// stub & render device

// stub
QSAPI bool qg_open_stub(const char* title, int width, int height, int flags);
QSAPI void qg_close_stub(void);

QSAPI bool qg_loop(void);
QSAPI bool qg_poll(QgEvent* ev);
QSAPI void qg_exit_loop(void);

QSAPI const QgUimKey* qg_get_key_info(void);
QSAPI const QgUimMouse* qg_get_mouse_info(void);
QSAPI bool qg_test_key(QikKey key);
QSAPI double qg_get_run_time(void);
QSAPI double qg_get_fps(void);
QSAPI double qg_get_ref_adv(void);
QSAPI double qg_get_advance(void);
QSAPI int qg_get_delay(void);
QSAPI void qg_set_delay(int delay);

QSAPI int qg_left_events(void);
QSAPI int qg_add_event(const QgEvent* ev);
QSAPI int qg_add_event_type(QgEventType type);
QSAPI bool qg_pop_event(QgEvent* ev);


// render device
struct _QgRdh
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

	QgVlo* (*create_layout)(QgRdh*, int, const QgPropLayout*);
	QgShd* (*create_shader)(QgRdh*, const char*);
	QgBuf* (*create_buffer)(QgRdh*, QgBufType, int, int, const void*);

	void (*set_shader)(QgRdh*, QgShd*, QgVlo*);
	bool (*set_index)(QgRdh*, QgBuf*);
	bool (*set_vertex)(QgRdh*, QgLoStage, QgBuf*);

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

QSAPI QgVlo* qg_rdh_create_layout(QgRdh* self, int count, const QgPropLayout* layouts);
QSAPI QgShd* qg_rdh_create_shader(QgRdh* self, const char* name);
QSAPI QgBuf* qg_rdh_create_buffer(QgRdh* g, QgBufType type, int count, int stride, const void* data);

QSAPI void qg_rdh_set_shader(QgRdh* self, QgShd* shader, QgVlo* layout);
QSAPI bool qg_rdh_set_index(QgRdh* g, QgBuf* buffer);
QSAPI bool qg_rdh_set_vertex(QgRdh* g, QgLoStage stage, QgBuf* buffer);

QSAPI bool qg_rdh_draw(QgRdh* self, QgTopology tpg, int vcount);
QSAPI bool qg_rdh_draw_indexed(QgRdh* self, QgTopology tpg, int icount);
QSAPI bool qg_rdh_ptr_draw(QgRdh* g, QgTopology tpg, int vcount, int vstride, const void* vdata);
QSAPI bool qg_rdh_ptr_draw_indexed(QgRdh* g, QgTopology tpg, int vcount, int vstride, const void* vdata, int icount, int istride, const void* idata);


//////////////////////////////////////////////////////////////////////////
// object

//
struct _QgGam
{
	QmGam				base;
};


// layout
struct _QgVlo
{
	QgGam				base;

	ushort				stride[QGLOS_MAX_VALUE];
};

QSAPI uint qg_vlo_get_stride(QgVlo* g, QgLoStage stage);


// shader
struct _QgShd
{
	QgGam				base;

	char				name[64];
	funcparam_t			intr;
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
struct _QgBuf
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

QN_EXTC_END
