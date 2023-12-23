//
// qgx11_stub.c - X11 스터브
// 2023-12-23 by kim
//
// 많은 부분을 SDL3에서 베껴왔어요 (https://www.libsdl.org/)
//

#include "pch.h"
#include "qs_qn.h"
#if defined USE_X11 && !defined USE_SDL2
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qg_stub.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/XKBlib.h>
#include <X11/Xlibint.h>
#include <X11/Xproto.h>
#include <X11/extensions/Xext.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xdbe.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/scrnsaver.h>
#include <X11/extensions/shape.h>
#include <X11/Xcursor/Xcursor.h>

#define DEBUG_X11_MODULE


//////////////////////////////////////////////////////////////////////////
// X11 스터브

// 링킹하기 귀찮을 땐 다이나믹
#define DEF_X11_FUNC(ret,name,decls,args)\
	typedef ret(*QN_CONCAT(PFNX11,name)) decls;\
	static QN_CONCAT(PFNX11,name) QN_CONCAT(x11,name);
#define DEF_X11_VAFUNC(ret,name,arg0)\
	typedef ret(*QN_CONCAT(PFNX11,name)) (arg0, ...);\
	static QN_CONCAT(PFNX11,name) QN_CONCAT(x11,name);
#include "qgx11_func.h"

static void* x11_internal_load_func(QnModule* module, const char* soname, const char* fnname)
{
	void* ret = qn_mod_func(module, fnname);
#ifdef DEBUG_X11_MODULE
	if (ret != NULL)
		qn_debug_outputf(false, "X11 MODULE", "\tfound '%s' in '%s'", fnname, soname);
	else
		qn_debug_outputf(false, "X11 MODULE", "\tno function found: '%s'", fnname);
#endif
	return ret;
}

static void x11_internal_func_init(void)
{
#define DEF_X11_SO(name)	static QnModule* QN_CONCAT(s_mod_,name);
#include "qgx11_func.h"
#define DEF_X11_SO(name)	QN_CONCAT(s_mod_,name) = qn_mod_open(QN_STRING(name) ".so", 0);
#include "qgx11_func.h"
	QnModule* module;
#ifdef DEBUG_X11_MODULE
	const char* soname;
#define DEF_X11_SO(name)	soname = QN_STRING(name) ".so"; module = QN_CONCAT(s_mod_,name); \
							if (module == NULL) qn_debug_outputf(true, "X11 MODULE", "module loading failed: '%s'", soname); \
							else { qn_debug_outputf(false, "X11 MODULE", "module loaded: '%s'", soname);
#define DEF_X11_FUNC(ret,name,decls,args)	QN_CONCAT(x11,name) = x11_internal_load_func(module, soname, QN_STRING(name));
#define DEF_X11_VAFUNC(ret,name,arg0)		QN_CONCAT(x11,name) = x11_internal_load_func(module, soname, QN_STRING(name));
#else
#define DEF_X11_SO(name)	module = QN_CONCAT(s_mod_,name); if (module != NULL) {
#define DEF_X11_FUNC(ret,name,decls,args)	QN_CONCAT(x11,name) = x11_internal_load_func(module, NULL, QN_STRING(name));
#define DEF_X11_VAFUNC(ret,name,arg0)		QN_CONCAT(x11,name) = x11_internal_load_func(module, NULL, QN_STRING(name));
#endif
#define DEF_X11_SO_END		}
#include "qgx11_func.h"
}

// 프로퍼티 얻기
typedef struct X11Property X11Property;
struct X11Property
{
	Display*			display;
	Window				window;
	Atom				atom;
	long				offset;
	long				length;
	Atom				request;

	Atom				type;
	unsigned long		read;
	unsigned long		left;
	byte*				data;
	int					format;
	bool				ok;
};

bool x11_get_prop(X11Property* prop)
{
	prop->ok = x11XGetWindowProperty(
		prop->display, prop->window, prop->atom, prop->offset, prop->length, False, prop->request,
		&prop->type, &prop->format, &prop->read, &prop->left, &prop->data) == Success;
	return prop->ok;
}

void x11_clean_prop(X11Property* prop)
{
	if (prop->ok && prop->data != NULL)
		x11XFree(prop->data);
}

/** @brief X11 스터브 */
typedef struct X11Stub X11Stub;
struct X11Stub
{
	StubBase			base;

	Display*			display;
	int					screen;
	Window				window;

	pid_t				pid;
	XID					group;

	int					depth;
	Visual*				visual;
	Colormap			colormap;

	GC					gc;
	XIM					xim;
	XIC					xic;

	struct X11Atoms
	{
		Atom				WM_PROTOCOLS;
		Atom				WM_DELETE_WINDOW;
		Atom				WM_TAKE_FOCUS;
		Atom				WM_NAME;
		Atom				_NET_WM_STATE;
		Atom				_NET_WM_STATE_HIDDEN;
		Atom				_NET_WM_STATE_FOCUSED;
		Atom				_NET_WM_STATE_MAXIMIZED_VERT;
		Atom				_NET_WM_STATE_MAXIMIZED_HORZ;
		Atom				_NET_WM_STATE_FULLSCREEN;
		Atom				_NET_WM_STATE_ABOVE;
		Atom				_NET_WM_STATE_SKIP_TASKBAR;
		Atom				_NET_WM_STATE_SKIP_PAGER;
		Atom				_NET_WM_ALLOWED_ACTIONS;
		Atom				_NET_WM_ACTION_FULLSCREEN;
		Atom				_NET_WM_NAME;
		Atom				_NET_WM_ICON_NAME;
		Atom				_NET_WM_ICON;
		Atom				_NET_WM_PING;
		Atom				_NET_WM_WINDOW_OPACITY;
		Atom				_NET_WM_USER_TIME;
		Atom				_NET_ACTIVE_WINDOW;
		Atom				_NET_FRAME_EXTENTS;
		Atom				_NET_SUPPORTING_WM_CHECK;
		Atom				UTF8_STRING;
		Atom				PRIMARY;
		Atom				XdndEnter;
		Atom				XdndPosition;
		Atom				XdndStatus;
		Atom				XdndTypeList;
		Atom				XdndActionCopy;
		Atom				XdndDrop;
		Atom				XdndFinished;
		Atom				XdndSelection;
		Atom				XKLAVIER_STATE;
	}					atoms;

	char*				class_name;
	char*				window_title;

	QmRect				local_bound;

	//HCURSOR				mouse_cursor;
	//llong				mouse_warp_time;
	//WPARAM				mouse_wparam;
	//LPARAM				mouse_lparam;
	//QimMask				mouse_pending;

	bool				has_wm;
	bool				enable_drop;

	//bool				clear_background;
};

// 에러 핸들러 플래그 (윈도우가 만들어지기 전에 호출될 수도 있기에 별도로 
static int (*s_x11_errorhandler)(Display*, XErrorEvent*);
static bool s_x11_handle_error = false;

// 정적 함수 미리 정의
static int x11_error_handler(Display* d, XErrorEvent* e);
static void x11_init_atoms(X11Stub* stub);
static char* x11_make_class_name(void);
static void x11_check_window_manager(X11Stub* stub);
static char* x11_get_title(Display* display, Window window);
static bool x11_set_title(Display* display, Window window, char* title);
static void x11_set_net_wm(X11Stub* stub, QgFlag flags);

//
StubBase* stub_system_open(const char* title, int width, int height, QgFlag flags)
{
	static X11Stub s_stub;
	X11Stub* stub = &s_stub;

	x11_internal_func_init();

	x11XInitThreads();

	// 디스플레이
	stub->display = x11XOpenDisplay(NULL);	// 디스플레이 이름을 넣어야 한다
	if (stub->display == NULL)
	{
		qn_debug_outputs(true, "X11 STUB", "cannot open display");
		return NULL;
	}

	s_x11_handle_error = false;
	s_x11_errorhandler = x11XSetErrorHandler(x11_error_handler);

	stub->screen = DefaultScreen(stub->display);
	stub->pid = getpid();
	stub->group = (XID)(((size_t)stub->pid) ^ ((size_t)stub));

	// 비주얼
	const int depth = DefaultDepth(stub->display, stub->screen);
	XVisualInfo vinfo;
	if (x11XMatchVisualInfo(stub->display, stub->screen, depth, DirectColor, &vinfo) == 0 &&
		x11XMatchVisualInfo(stub->display, stub->screen, depth, TrueColor, &vinfo) == 0 &&
		x11XMatchVisualInfo(stub->display, stub->screen, depth, PseudoColor, &vinfo) == 0 &&
		x11XMatchVisualInfo(stub->display, stub->screen, depth, StaticColor, &vinfo) == 0)
	{
		qn_debug_outputs(true, "X11 STUB", "cannot retrieve visual for primary display");
		stub_system_finalize();
		return NULL;
	}
	stub->visual = vinfo.visual;
	stub->depth = vinfo.depth;

	//
	const Screen* screen = ScreenOfDisplay(stub->display, stub->screen);
	const QmSize scrsize = { WidthOfScreen(screen), HeightOfScreen(screen) };
	if (width < 256 || height < 256)
	{
		if (scrsize.height > 800)
			width = 1280, height = 720;
		else
			width = 720, height = 450;
	}
	const QmPoint pos =
	{
		.x = (scrsize.width - width) / 2,
		.y = (scrsize.height - height) / 2,
	};
	qm_set4(&stub->local_bound, pos.x, pos.y, pos.x + width, pos.y + height);
	stub->base.window_bound = stub->local_bound;

	// 설정
	x11_init_atoms(stub);
	x11_check_window_manager(stub);
	// 키보드
	// 마우스

	//
	stub->class_name = x11_make_class_name();
	stub->window_title = qn_strdup(title != NULL ? title : "QS");

	return (StubBase*)stub;
}

//
bool stub_system_create_window(void)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;
	Visual* visual = stub->visual;
	Window root = RootWindow(stub->display, stub->screen);

	//
	XSetWindowAttributes attr =
	{
		.override_redirect = False,
		.backing_store = NotUseful,
		.background_pixmap = None,
		.border_pixel = 0,
	};
	if (stub->visual->class != DirectColor)
		attr.colormap = x11XCreateColormap(stub->display, root, visual, AllocNone);
	else
	{
		attr.colormap = x11XCreateColormap(stub->display, root, visual, AllocAll);
		if (attr.colormap == None)
		{
			qn_debug_outputs(true, "X11 STUB", "cannot create writable colormap");
			return false;
		}

		XColor* colors = qn_alloc(visual->map_entries, XColor);
		const int entry_count = visual->map_entries;
		int rmax = 0xFFFF, gmax = 0xFFFF, bmax = 0xFFFF;

		unsigned long sr = 0, mr = visual->red_mask;
		while ((mr & 1) == 0)
			sr++, mr >>= 1;
		unsigned long sg = 0, mg = visual->green_mask;
		while ((mg & 1) == 0)
			sg++, mg >>= 1;
		unsigned long sb = 0, mb = visual->blue_mask;
		while ((mb & 1) == 0)
			sb++, mb >>= 1;

		const uint under = entry_count - 1;
		for (int i = 0; i < entry_count; i++)
		{
			unsigned long tr = (mr * i) / under;
			unsigned long tg = (mg * i) / under;
			unsigned long tb = (mb * i) / under;
			colors[i].pixel = (tr << sr) | (tg << sg) | (tb << sb);
			colors[i].red = (ushort)((rmax * i) / under);
			colors[i].green = (ushort)((gmax * i) / under);
			colors[i].blue = (ushort)((bmax * i) / under);
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}

		x11XStoreColors(stub->display, attr.colormap, colors, entry_count);
		qn_free(colors);
	}

	QmRect* bound = &stub->local_bound;
	int width = qm_rect_width(bound), height = qm_rect_height(bound);
	stub->window = x11XCreateWindow(stub->display, root, bound->left, bound->top, width, height,
		0, stub->depth, InputOutput, visual,
		(CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWBackingStore | CWColormap), &attr);
	if (stub->window == None)
	{
		qn_debug_outputs(true, "X11 STUB", "cannot create window");
		return false;
	}

	//
	Atom motif_wm_hints = x11XInternAtom(stub->display, "_MOTIF_WM_HINTS", True);
	if (motif_wm_hints == None)
		x11XSetTransientForHint(stub->display, stub->window, root);
	else
	{
		unsigned long data[5] =
		{
			1L << 1,
			0,
			QN_TMASK(stub->base.flags, QGFLAG_BORDERLESS) ? 0 : 1,
			0,
			0
		};
		x11XChangeProperty(stub->display, stub->window, motif_wm_hints, motif_wm_hints,
			32, PropModeReplace, (const byte*)data, QN_COUNTOF(data));
	}

	//
	XSizeHints* szh = x11XAllocSizeHints();
	szh->flags = USPosition;
	szh->x = bound->left;
	szh->y = bound->right;
	if (QN_TMASK(stub->base.flags, QGFLAG_RESIZABLE) == false)
	{
		szh->flags |= PMinSize | PMaxSize;
		szh->min_width = szh->max_width = width;
		szh->min_height = szh->max_height = height;
	}

	XWMHints* wmh = x11XAllocWMHints();
	wmh->flags = InputHint | WindowGroupHint;
	wmh->input = True;
	wmh->window_group = stub->group;

	XClassHint* csh = x11XAllocClassHint();
	csh->res_name = stub->class_name;
	csh->res_class = stub->class_name;

	x11XSetWMProperties(stub->display, stub->window, NULL, NULL, NULL, 0, szh, wmh, csh);
	x11XFree(csh);
	x11XFree(wmh);
	x11XFree(szh);

	if (stub->pid > 0)
	{
		long pid = (long)stub->pid;
		Atom nwpid = x11XInternAtom(stub->display, "_NET_WM_PID", false);
		x11XChangeProperty(stub->display, stub->window, nwpid, XA_CARDINAL, 32, PropModeReplace, (byte*)&pid, 1);
	}

	//
	x11_set_net_wm(stub, stub->base.flags);

	Atom wm_wintype = x11XInternAtom(stub->display, "_NET_WM_WINDOW_TYPE", False);
	Atom wm_normal = x11XInternAtom(stub->display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	x11XChangeProperty(stub->display, stub->window, wm_wintype, XA_ATOM, 32, PropModeReplace, (byte*)&wm_normal, 1);

	Atom wm_bypass = x11XInternAtom(stub->display, "_NET_WM_BYPASS_COMPOSITOR", False);
	long compositor = 1;
	x11XChangeProperty(stub->display, stub->window, wm_bypass, XA_CARDINAL, 32, PropModeReplace, (byte*)&compositor, 1);

	Atom protocols[] =
	{
		stub->atoms.WM_DELETE_WINDOW,
		stub->atoms.WM_TAKE_FOCUS,
		stub->atoms._NET_WM_PING,
	};
	x11XSetWMProtocols(stub->display, stub->window, protocols, QN_COUNTOF(protocols));

	//
	if (stub->xim != NULL)
		stub->xic = x11XCreateIC(stub->xim, XNClientWindow, stub->window, XNFocusWindow, stub->window,
			XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);

	XWindowAttributes wattr;
	x11XGetWindowAttributes(stub->display, stub->window, &wattr);
	qm_set4(&stub->base.window_bound, wattr.x, wattr.y, wattr.x + wattr.width, wattr.y + wattr.height);
	qm_set4(&stub->base.bound, 0, 0, wattr.width, wattr.height);
	qm_set2(&stub->base.size, wattr.width, wattr.height);
	stub->visual = wattr.visual;
	stub->colormap = wattr.colormap;

	Window fcwnd;
	int revert = 0;
	x11XGetInputFocus(stub->display, &fcwnd, &revert);
	if (fcwnd == stub->window)
		QN_SMASK(&stub->base.stats, QGSSTT_FOCUS, true);

	long fevent = 0;
	if (stub->xic != NULL)
		x11XGetICValues(stub->xic, XNFilterEvents, &fevent, NULL);
	x11XSelectInput(stub->display, stub->window,
		FocusChangeMask | EnterWindowMask | LeaveWindowMask | ExposureMask | ButtonPressMask | ButtonReleaseMask |
		PointerMotionMask | KeyPressMask | KeyReleaseMask | PropertyChangeMask | StructureNotifyMask | KeymapStateMask |
		fevent);
	x11XSelectInput(stub->display, root, PropertyChangeMask);
	x11XFlush(stub->display);

	return true;
}

//
void stub_system_finalize(void)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;

	if (stub->xic != NULL)
		x11XDestroyIC(stub->xic);

	if (stub->display != NULL && stub->window != 0)
	{
		x11XDestroyWindow(stub->display, stub->window);
		x11XFlush(stub->display);
	}

	if (s_x11_errorhandler != NULL)
		x11XSetErrorHandler(s_x11_errorhandler);

	if (stub->display != NULL)
		x11XCloseDisplay(stub->display);

	qn_free(stub->window_title);
	qn_free(stub->class_name);
}

//
bool stub_system_poll(void)
{
	return false;
}

//
void stub_system_disable_acs(bool enable)
{
}

//
void stub_system_diable_scrsave(bool enable)
{
}

//
void stub_system_enable_drop(bool enable)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;
	Atom xdnd = x11XInternAtom(stub->display, "XdndAware", False);
	if (enable)
	{
		if (stub->enable_drop)
			return;
		stub->enable_drop = true;
		QN_SMASK(&stub->base.flags, QGFEATURE_ENABLE_DROP, true);
		Atom version = 5;
		x11XChangeProperty(stub->display, stub->window, xdnd, XA_ATOM, 32, PropModeReplace, (byte*)&version, 1);
	}
	else
	{
		if (stub->enable_drop == false)
			return;
		stub->enable_drop = false;
		QN_SMASK(&stub->base.flags, QGFEATURE_ENABLE_DROP, false);
		x11XDeleteProperty(stub->display, stub->window, xdnd);
	}
}

// 
void stub_system_set_title(const char* u8text)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;
	qn_ret_if_fail(stub->display != NULL && stub->window != 0);

	qn_free(stub->window_title);
	stub->window_title = qn_strdup(u8text != NULL ? u8text : "QS");

	x11_set_title(stub->display, stub->window, stub->window_title);
}

//
void stub_system_hold_mouse(bool hold)
{
	// X11_SetWindowMouseGrab
}

//
void stub_system_calc_layout(void)
{
}

// 에러 핸들리
static int x11_error_handler(Display* d, XErrorEvent* e)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;

	if (s_x11_handle_error == false)
	{
		s_x11_handle_error = true;
		if (stub != NULL)
		{
			// 디스플레이 모드 변경
		}
	}

	if (s_x11_errorhandler != NULL)
		return s_x11_errorhandler(d, e);
	return 0;
}

// 아톰 만들기, 너무 길어서 따로 뺌
static void x11_init_atoms(X11Stub* stub)
{
	Display* display = stub->display;
#define DEF_X11_ATOM(x) stub->atoms.x = x11XInternAtom(display, #x, False)
	DEF_X11_ATOM(WM_PROTOCOLS);
	DEF_X11_ATOM(WM_DELETE_WINDOW);
	DEF_X11_ATOM(WM_TAKE_FOCUS);
	DEF_X11_ATOM(WM_NAME);
	DEF_X11_ATOM(_NET_WM_STATE);
	DEF_X11_ATOM(_NET_WM_STATE_HIDDEN);
	DEF_X11_ATOM(_NET_WM_STATE_FOCUSED);
	DEF_X11_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
	DEF_X11_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
	DEF_X11_ATOM(_NET_WM_STATE_FULLSCREEN);
	DEF_X11_ATOM(_NET_WM_STATE_ABOVE);
	DEF_X11_ATOM(_NET_WM_STATE_SKIP_TASKBAR);
	DEF_X11_ATOM(_NET_WM_STATE_SKIP_PAGER);
	DEF_X11_ATOM(_NET_WM_ALLOWED_ACTIONS);
	DEF_X11_ATOM(_NET_WM_ACTION_FULLSCREEN);
	DEF_X11_ATOM(_NET_WM_NAME);
	DEF_X11_ATOM(_NET_WM_ICON_NAME);
	DEF_X11_ATOM(_NET_WM_ICON);
	DEF_X11_ATOM(_NET_WM_PING);
	DEF_X11_ATOM(_NET_WM_WINDOW_OPACITY);
	DEF_X11_ATOM(_NET_WM_USER_TIME);
	DEF_X11_ATOM(_NET_ACTIVE_WINDOW);
	DEF_X11_ATOM(_NET_FRAME_EXTENTS);
	DEF_X11_ATOM(_NET_SUPPORTING_WM_CHECK);
	DEF_X11_ATOM(UTF8_STRING);
	DEF_X11_ATOM(PRIMARY);
	DEF_X11_ATOM(XdndEnter);
	DEF_X11_ATOM(XdndPosition);
	DEF_X11_ATOM(XdndStatus);
	DEF_X11_ATOM(XdndTypeList);
	DEF_X11_ATOM(XdndActionCopy);
	DEF_X11_ATOM(XdndDrop);
	DEF_X11_ATOM(XdndFinished);
	DEF_X11_ATOM(XdndSelection);
	DEF_X11_ATOM(XKLAVIER_STATE);
#undef DEF_X11_ATOM
}

// 클래스 이름 만들기
static char* x11_make_class_name(void)
{
#if defined _QN_FREEBSD_ || defined _QN_LINUX_
	char procfile[FILENAME_MAX];
	char linkfile[FILENAME_MAX];
	ssize_t linklen;

#if defined _QN_FREEBSD_
	qn_snprintf(procfile, QN_COUNTOF(procfile), "/proc/%d/exe", getpid());
#else
	qn_snprintf(procfile, QN_COUNTOF(procfile), "/proc/%d/file", getpid());
#endif
	linklen = readlink(procfile, linkfile, QN_COUNTOF(linkfile) - 1);
	if (linklen > 0)
	{
		linkfile[linklen] = '\0';
		char* p = strrchr(linkfile, '/');
		if (p != NULL)
			return qn_strdup(p + 1);
		else
			return qn_strdup(linkfile);
	}
#endif

	QnDateTime dt;
	qn_now(&dt);
	return qn_apsprintf("qs_stub_class_%llu", dt.stamp);
}

// 윈도우 메니저 체크용
static int (*s_x11_check_wm_error_handler)(Display*, XErrorEvent*);
static int x11_check_wm_error_handler(Display* d, XErrorEvent* e)
{
	if (e->error_code == BadWindow)
		return 0;
	return s_x11_check_wm_error_handler(d, e);
}

// 윈도우 매니저 체크
static void x11_check_window_manager(X11Stub* stub)
{
	static const char* s_atom_name = "_NET_SUPPORTING_WM_CHECK";

	x11XSync(stub->display, False);
	s_x11_check_wm_error_handler = x11XSetErrorHandler(x11_check_wm_error_handler);

	Window window = 0;

	X11Property prop = {
		.display = stub->display,
		.window = DefaultRootWindow(stub->display),
		.atom = stub->atoms._NET_SUPPORTING_WM_CHECK,
		.offset = 0L,
		.length = 1,
		.request = XA_WINDOW,
	};
	if (x11_get_prop(&prop))
	{
		if (prop.read != 0)
			window = ((Window*)prop.data)[0];
		x11_clean_prop(&prop);
	}

	if (window)
	{
		prop.window = window;
		if (x11_get_prop(&prop) == false || prop.read == 0 || window != ((Window*)prop.data)[0])
			window = None;
		x11_clean_prop(&prop);
	}

	x11XSync(stub->display, False);
	x11XSetErrorHandler(s_x11_check_wm_error_handler);

	if (window == 0)
	{
		qn_debug_outputf(false, "X11 STUB", "cannot retrieve %s property", s_atom_name);
		return;
	}

	stub->has_wm = true;
	char* wm_name = x11_get_title(stub->display, window);
	qn_debug_outputf(false, "X11 STUB", "window manager: %s", wm_name);
	qn_free(wm_name);
}

// 타이틀 얻기
static char* x11_get_title(Display* display, Window window)
{
	X11Property prop = {
		.display = display,
		.window = window,
		.atom = x11XInternAtom(display, "_NET_WM_NAME", False),
		.offset = 0L,
		.length = 8192L,
		.request = x11XInternAtom(display, "UTF8_STRING", False),
	};
	if (x11_get_prop(&prop))
	{
		char* title = qn_strdup((char*)prop.data);
		x11_clean_prop(&prop);
		return title;
	}

	prop.atom = XA_WM_NAME;
	prop.request = XA_STRING;
	if (x11_get_prop(&prop))
	{
		char* title = qn_strdup((char*)prop.data);
		qn_debug_outputf(false, "X11 STUB", "cannot retrieve UTF8 title (XASTRING: %s)", title);
		x11_clean_prop(&prop);
		return title;
	}

	qn_debug_outputs(false, "X11 STUB", "cannot retrieve any titles from Xorg");
	return qn_strdup("(unknown)");
}

// 타이틀 설정
static bool x11_set_title(Display* display, Window window, char* title)
{
	XTextProperty prop;
	int res = x11XmbTextListToTextProperty(display, &title, 1, XTextStyle, &prop);
	if (x11XSupportsLocale() == False)
	{
		qn_debug_outputs(false, "X11 STUB", "current locale not supported by Xorg");
		return false;
	}
	if (res != Success)
	{
		qn_debug_outputs(false, "X11 STUB", "error on set title");
		return false;
	}
	x11XSetTextProperty(display, window, &prop, XA_WM_NAME);
	x11XFree(prop.value);

	res = x11Xutf8TextListToTextProperty(display, &title, 1, XUTF8StringStyle, &prop);
	if (res != Success)
	{
		qn_debug_outputs(false, "X11 STUB", "error on set utf8 title");
		return false;
	}
	Atom atom = x11XInternAtom(display, "_NET_WM_NAME", False);
	x11XSetTextProperty(display, window, &prop, atom);
	x11XFree(prop.value);

	x11XFlush(display);
	return true;
}

// 윈도우 매니저 상태 설정
static void x11_set_net_wm(X11Stub* stub, QgFlag flags)
{
	Atom atoms[16];
	int atom_count = 0;

	if (QN_TMASK(flags, QGFLAG_FOCUS))
		atoms[atom_count++] = stub->atoms._NET_WM_STATE_FOCUSED;
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
		atoms[atom_count++] = stub->atoms._NET_WM_STATE_FULLSCREEN;

	if (atom_count == 0)
	{
		x11XDeleteProperty(stub->display, stub->window, stub->atoms._NET_WM_STATE);
		return;
	}
	x11XChangeProperty(stub->display, stub->window, stub->atoms._NET_WM_STATE,
		XA_ATOM, 32, PropModeReplace, (byte*)atoms, atom_count);
}

#endif
