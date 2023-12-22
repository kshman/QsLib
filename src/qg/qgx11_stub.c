//
// qg_stub_windows.c - 윈도우 스터브
// 2023-12-13 by kim
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
#include <X11/extensions/scrnsaver.h>
#include <X11/extensions/shape.h>


//////////////////////////////////////////////////////////////////////////
// X11 스터브

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
	int					ok;
};

bool x11_get_prop(X11Property* prop)
{
	prop->ok = XGetWindowProperty(
		prop->display, prop->window, prop->atom, prop->offset, prop->length, False, prop->request,
		&prop->type, &prop->format, &prop->read, &prop->left, &prop->data);
	return prop->ok != False;
}

void x11_clean_prop(X11Property* prop)
{
	if (prop->ok && prop->data != NULL)
		XFree(prop);
}

/** @brief X11 스터브 */
typedef struct X11Stub X11Stub;
struct X11Stub
{
	StubBase			base;

	Display*			display;
	int					screen;
	Window				window;

	GC					gc;
	pid_t				pid;

	Visual*				visual;
	int					depth;

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

	//HCURSOR				mouse_cursor;
	//llong				mouse_warp_time;
	//WPARAM				mouse_wparam;
	//LPARAM				mouse_lparam;
	//QimMask				mouse_pending;

	bool				has_wm;

	//bool				enable_drop;

	//bool				clear_background;
};

// 에러 핸들러 플래그 (윈도우가 만들어지기 전에 호출될 수도 있기에 별도로 
static int (*s_x11_errorhandler)(Display*, XErrorEvent*);
static bool s_x11_handle_error = false;

// 정적 함수 미리 정의
static char* x11_make_class_name(void);
static int x11_error_handler(Display* d, XErrorEvent* e);
static void x11_check_wm(X11Stub* stub);
static char* x11_get_title(Display* display, Window window);
static bool x11_set_title(Display* display, Window window, char* title);

//
StubBase* stub_system_open(const char* title, int width, int height, int flags)
{
	static X11Stub s_stub;
	X11Stub* stub = &s_stub;

	XInitThreads();

	stub->display = XOpenDisplay(NULL);	// 디스플레이 이름을 넣어야 한다
	if (stub->display == NULL)
	{
		qn_debug_outputs(true, "X11 STUB", "cannot open display");
		return NULL;
	}

	s_x11_handle_error = false;
	s_x11_errorhandler = XSetErrorHandler(x11_error_handler);

	stub->screen = DefaultScreen(stub->display);
	stub->pid = getpid();

	int depth = DefaultDepth(stub->display, stub->screen);
	XVisualInfo vinfo;
	if (XMatchVisualInfo(stub->display, stub->screen, depth, DirectColor, &vinfo) != Success ||
		XMatchVisualInfo(stub->display, stub->screen, depth, TrueColor, &vinfo) != Success ||
		XMatchVisualInfo(stub->display, stub->screen, depth, PseudoColor, &vinfo) != Success ||
		XMatchVisualInfo(stub->display, stub->screen, depth, StaticColor, &vinfo) != Success)
	{
		qn_debug_outputs(true, "X11 STUB", "cannot retrieve visual for primary display");
		stub_system_finalize();
		return NULL;
	}

	stub->visual = vinfo.visual;
	stub->depth = vinfo.depth;

#define DEF_X11_ATOM(x) stub->atoms.x = XInternAtom(stub->display, #x, False)
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

	x11_check_wm(stub);

	// 풀스크린을 위해서 여기서 디스플레이 모드를 검사해야 한다

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

	XSetWindowAttributes attr =
	{
		.override_redirect = False,
		.backing_store = NotUseful,
		.background_pixmap = None,
		.border_pixel = 0,
	};
	if (stub->visual->class != DirectColor)
		attr.colormap = XCreateColormap(stub->display, root, visual, AllocNone);
	else
	{
		attr.colormap = XCreateColormap(stub->display, root), visual, AllocAll);
		if (attr.colormap == NULL)
		{
			qn_debug_outputs(true, "X11 STUB", "cannot create writable colormap");
			return false;
		}

		XColor* colors = qn_alloc(visual->map_entries, XColor);
		const int entry_count = visual->map_entries;
		int rmax = 0xFFFF, gmax = 0xFFFF, bmax = 0xFFFF;

		int sr = 0, mr = visual->red_mask;
		while ((mr & 1) == 0)
			sr++, mr >>= 1;
		int sg = 0, mg = visual->green_mask;
		while ((mg & 1) == 0)
			sg++, mg >>= 1;
		int sb = 0, mb = visual->blue_mask;
		while ((mb & 1) == 0)
			sb++, mb >>= 1;

		const uint under = entry_count - 1;
		for (int i = 0; i < entry_count; i++)
		{
			uint tr = (mr * i) / under;
			uint tg = (mg * i) / under;
			uint tb = (mb * i) / under;
			colors[i].pixel = (tr << sr) | (tg << sg) | (tb << sb);
			colors[i].red = (rmax * i) / under;
			colors[i].green = (gmax * i) / under;
			colors[i].blue = (bmax * i) / under;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}

		XStoreColors(stub->display, attr.colormap, colors, entry_count);
		qn_free(colors);
	}

	stub->window = XCreateWindow(stub->display, root);

	return false;
}

//
void stub_system_finalize(void)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;

	if (s_x11_errorhandler != NULL)
		XSetErrorHandler(s_x11_errorhandler);

	if (stub->display != NULL)
		XCloseDisplay(stub->display);

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
}

//
void stub_system_calc_layout(void)
{
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

// 윈도우 메니저 체크용
static int (*s_x11_check_wm_error_handler)(Display*, XErrorEvent*);
static int x11_check_wm_error_handler(Display* d, XErrorEvent* e)
{
	if (e->error_code == BadWindow)
		return 0;
	return s_x11_check_wm_error_handler(d, e);
}

// 윈도우 매니저 체크
static void x11_check_wm(X11Stub* stub)
{
	static const char* s_atom_name = "_NET_SUPPORTING_WM_CHECK";

	XSync(stub->display, False);
	s_x11_check_wm_error_handler = XSetErrorHandler(x11_check_wm_error_handler);

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

	XSync(stub->display, False);
	XSetErrorHandler(s_x11_check_wm_error_handler);

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
		.atom = XInternAtom(display, "_NET_WM_NAME", False),
		.offset = 0L,
		.length = 8192L,
		.request = XInternAtom(display, "UTF8_STRING", False),
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
bool x11_set_title(Display* display, Window window, char* title)
{
	XTextProperty prop;
	int res = XmbTextListToTextProperty(display, &title, 1, XTextStyle, &prop);
	if (XSupportsLocale() == False)
	{
		qn_debug_outputs(false, "X11 STUB", "current locale not supported by Xorg");
		return false;
	}
	if (res != Success)
	{
		qn_debug_outputs(false, "X11 STUB", "error on set title");
		return false;
	}
	XSetTextProperty(display, window, &prop, XA_WM_NAME);
	XFree(prop.value);

	res = Xutf8TextListToTextProperty(display, &title, 1, XUTF8StringStyle, &prop);
	if (res != Success)
	{
		qn_debug_outputs(false, "X11 STUB", "error on set utf8 title");
		return false;
	}
	Atom atom = XInternAtom(display, "_NET_WM_NAME", False);
	XSetTextProperty(display, window, &prop, atom);
	XFree(prop.value);

	XFlush(display);
	return true;
}

#endif
