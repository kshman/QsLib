//
// qgx11_stub.c - X11 스터브
// 2023-12-23 by kim
//
// 많은 부분을 SDL3에서 베껴왔어요 (https://www.libsdl.org/)
//

#include "pch.h"
#include "qs_qn.h"
#if defined USE_X11
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qgx11_stub.h"
#include <locale.h>

#ifdef _DEBUG
#define DEBUG_X11SO_TRACE
#endif

//
static char* x11_get_class_name(void);
static void x11_init_atoms(void);
static void x11_check_wm(void);
static bool x11_check_modes(void);
static char* x11_get_title(Window window);
static bool x11_set_title(Window window, char* title);
static void x11_set_net_wm(QgFlag flags);

// x11 스터브 변수
static X11Stub x11stub;

// SO 선언
#define DEF_X11_FUNC(ret,name,args)		QN_CONCAT(PFNX11, name) QN_CONCAT(X11, name);
#define DEF_X11_VAFUNC(ret,name,arg0)	QN_CONCAT(PFNX11, name) QN_CONCAT(X11, name);
#include "qgx11_func.h"

// SO 함수
static void* x11_load_func(QnModule* module, const char* soname, const char* fnname)
{
	void* ret = qn_mod_func(module, fnname);
#ifdef DEBUG_X11SO_TRACE
	qn_debug_outputf(false, "X11 STUB", "\t%s: '%s' in '%s'",
		ret == NULL ? "load failed" : "loaded", fnname, soname);
#else
	QN_DUMMY(dllname);
#endif
	return ret;
}

// SO 초기화
static bool x11_so_init(void)
{
	static bool loaded = false;
	qn_val_if_ok(loaded, true);
	QnModule* module;
	const char* soname;
#define DEF_X11_SO_BEGIN(name)\
	module = qn_mod_load(soname = QN_STRING(name) ".so", 1); if (module == NULL)\
	{ qn_debug_outputf(true, "X11 STUB", "no %s found!", soname); return false; } else {
#define DEF_X11_SO_END	}
#define DEF_X11_FUNC(ret,name,args)\
	QN_CONCAT(X11, name) = (QN_CONCAT(PFNX11, name))x11_load_func(module, soname, QN_STRING(name));
#define DEF_X11_VAFUNC(ret,name,args)\
	QN_CONCAT(X11, name) = (QN_CONCAT(PFNX11, name))x11_load_func(module, soname, QN_STRING(name));
#include "qgx11_func.h"
	return loaded = true;
}

//
StubBase* stub_system_open(const char* title, int display, int width, int height, QgFlag flags)
{
	if (strcmp(setlocale(LC_CTYPE, NULL), "C") == 0)
		setlocale(LC_CTYPE, "");

	if (x11_so_init() == false)
		return NULL;

	X11XInitThreads();

	// 디스플레이
	x11stub.display = X11XOpenDisplay(NULL);	// 디스플레이 이름을 넣어야 한다
	if (x11stub.display == NULL)
	{
		const char* name = getenv("DISPLAY");
		qn_debug_outputf(true, "X11 STUB", "cannot open display: %s", name ? name : "(unknown)");
		return NULL;
	}

	//
	x11stub.screen = DefaultScreen(x11stub.display);
	x11stub.root = DefaultRootWindow(x11stub.display);

	x11stub.class_name = x11_get_class_name();
	x11stub.pid = getpid();
	x11stub.group = (XID)(((size_t)x11stub.pid) ^ ((size_t)&x11stub));

	qn_pctnr_init(&x11stub.base.mon, 0);

	// X11
	x11_init_atoms();
	x11_check_wm();
	if (x11_check_modes() == false)
	{
		stub_system_finalize();
		return NULL;
	}

	// Xinput2
	// Xfixes
	// Keyboard
	// Mouse

	//
	x11stub.base.display = display > qn_pctnr_count(&x11stub.base.mon) ? 0 : display;

	X11Monitor* mon = qn_pctnr_nth(&x11stub.base.mon, x11stub.base.display);
	if (width < 256 || height < 256)
	{
		if (mon->base.width > 800)
			width = 1280, height = 720;
		else
			width = 720, height = 480;
	}
	const QmPoint pos =
	{
		/*mon->base.x +*/ (mon->base.width - width) / 2,
		/*mon->base.y +*/ (mon->base.height - height) / 2,
	};
	qm_set4(&x11stub.local_bound, pos.x, pos.y, pos.x + width, pos.y + height);
	x11stub.base.window_bound = x11stub.local_bound;

	//
	x11stub.window_title = qn_strdup(title != NULL ? title : "QS");

	return (StubBase*)&x11stub;
}

//
bool stub_system_create_window(void)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;
	X11Monitor* mon = qn_pctnr_nth(&x11stub.base.mon, x11stub.base.display);
	int screen = mon->base.no;
	Window root = RootWindow(x11stub.display, screen);
	Visual* visual;
	uint depth;

#if defined USE_GL || defined USE_ES3
	const char* glprop = qn_get_prop(QG_PROP_OPENGL);
	if (glprop == NULL)
		glprop = "ES3";
#endif
	{
		visual = mon->visual;
		depth = mon->base.depth;
	}

	//
	XSetWindowAttributes attr =
	{
		.override_redirect = False,
		.backing_store = NotUseful,
		.background_pixmap = None,
		.border_pixel = 0,
	};
	if (visual->class != DirectColor)
		attr.colormap = X11XCreateColormap(x11stub.display, root, visual, AllocNone);
	else
	{
		attr.colormap = X11XCreateColormap(x11stub.display, root, visual, AllocAll);
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

		X11XStoreColors(x11stub.display, attr.colormap, colors, entry_count);
		qn_free(colors);
	}

	QmRect* bound = &x11stub.local_bound;
	int width = qm_rect_width(bound), height = qm_rect_height(bound);
	x11stub.window = X11XCreateWindow(x11stub.display, root, bound->left, bound->top, width, height,
		0, depth, InputOutput, visual,
		(CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWBackingStore | CWColormap), &attr);
	if (x11stub.window == None)
	{
		qn_debug_outputs(true, "X11 STUB", "cannot create window");
		return false;
	}

	//
	Atom motif_wm_hints = X11XInternAtom(x11stub.display, "_MOTIF_WM_HINTS", True);
	if (motif_wm_hints == None)
		X11XSetTransientForHint(x11stub.display, x11stub.window, root);
	else
	{
		unsigned long data[5] =
		{
			1L << 1,
			0,
			QN_TMASK(x11stub.base.flags, QGFLAG_BORDERLESS) ? 0 : 1,
			0,
			0
		};
		X11XChangeProperty(x11stub.display, x11stub.window, motif_wm_hints, motif_wm_hints,
			32, PropModeReplace, (const byte*)data, QN_COUNTOF(data));
	}

	//
	XSizeHints* szh = X11XAllocSizeHints();
	szh->flags = USPosition;
	szh->x = bound->left;
	szh->y = bound->right;
	if (QN_TMASK(x11stub.base.flags, QGFLAG_RESIZABLE) == false)
	{
		szh->flags |= PMinSize | PMaxSize;
		szh->min_width = szh->max_width = width;
		szh->min_height = szh->max_height = height;
	}

	XWMHints* wmh = X11XAllocWMHints();
	wmh->flags = InputHint | WindowGroupHint;
	wmh->input = True;
	wmh->window_group = x11stub.group;

	XClassHint* csh = X11XAllocClassHint();
	csh->res_name = x11stub.class_name;
	csh->res_class = x11stub.class_name;

	X11XSetWMProperties(x11stub.display, x11stub.window, NULL, NULL, NULL, 0, szh, wmh, csh);
	X11XFree(csh);
	X11XFree(wmh);
	X11XFree(szh);

	if (x11stub.pid > 0)
	{
		long pid = (long)x11stub.pid;
		Atom nwpid = X11XInternAtom(x11stub.display, "_NET_WM_PID", false);
		X11XChangeProperty(x11stub.display, x11stub.window, nwpid, XA_CARDINAL, 32, PropModeReplace, (byte*)&pid, 1);
	}

	//
	x11_set_net_wm(x11stub.base.flags);

	Atom wm_wintype = X11XInternAtom(x11stub.display, "_NET_WM_WINDOW_TYPE", False);
	Atom wm_normal = X11XInternAtom(x11stub.display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	X11XChangeProperty(x11stub.display, x11stub.window, wm_wintype, XA_ATOM, 32, PropModeReplace, (byte*)&wm_normal, 1);

	Atom wm_bypass = X11XInternAtom(x11stub.display, "_NET_WM_BYPASS_COMPOSITOR", False);
	long compositor = 1;
	X11XChangeProperty(x11stub.display, x11stub.window, wm_bypass, XA_CARDINAL, 32, PropModeReplace, (byte*)&compositor, 1);

	Atom protocols[] =
	{
		x11stub.atoms.WM_DELETE_WINDOW,
		x11stub.atoms.WM_TAKE_FOCUS,
		x11stub.atoms._NET_WM_PING,
	};
	X11XSetWMProtocols(x11stub.display, x11stub.window, protocols, QN_COUNTOF(protocols));

	//
	if (x11stub.xim != NULL)
		x11stub.xic = X11XCreateIC(x11stub.xim, XNClientWindow, x11stub.window, XNFocusWindow, x11stub.window,
			XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);

	XWindowAttributes wattr;
	X11XGetWindowAttributes(x11stub.display, x11stub.window, &wattr);
	qm_set4(&x11stub.base.window_bound, wattr.x, wattr.y, wattr.x + wattr.width, wattr.y + wattr.height);
	qm_set2(&x11stub.base.client_size, wattr.width, wattr.height);
	x11stub.visual = wattr.visual;
	x11stub.colormap = wattr.colormap;

	Window fcwnd;
	int revert = 0;
	X11XGetInputFocus(x11stub.display, &fcwnd, &revert);
	if (fcwnd == x11stub.window)
		QN_SMASK(&x11stub.base.stats, QGSSTT_FOCUS, true);

	long fevent = 0;
	if (x11stub.xic != NULL)
		X11XGetICValues(x11stub.xic, XNFilterEvents, &fevent, NULL);
	X11XSelectInput(x11stub.display, x11stub.window,
		FocusChangeMask | EnterWindowMask | LeaveWindowMask | ExposureMask | ButtonPressMask | ButtonReleaseMask |
		PointerMotionMask | KeyPressMask | KeyReleaseMask | PropertyChangeMask | StructureNotifyMask | KeymapStateMask |
		fevent);
	X11XSelectInput(x11stub.display, root, PropertyChangeMask);

	X11XFlush(x11stub.display);

	return true;
}

//
void stub_system_finalize(void)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;

	if (x11stub.xic != NULL)
		X11XDestroyIC(x11stub.xic);

	if (x11stub.display != NULL && x11stub.window != 0)
	{
		X11XDestroyWindow(x11stub.display, x11stub.window);
		X11XFlush(x11stub.display);
	}

	if (x11stub.display != NULL)
		X11XCloseDisplay(x11stub.display);

	qn_pctnr_loopeach(&x11stub.base.mon, qn_memfre);
	qn_pctnr_disp(&x11stub.base.mon);

	qn_free(x11stub.window_title);
	qn_free(x11stub.class_name);
}

//
bool stub_system_poll(void)
{
	QN_TODO("폴: 해야함");
	return false;
}

//
void stub_system_disable_acs(bool enable)
{
	QN_TODO("보고 기능: 이거 할 수 있나?");
}

//
void stub_system_diable_scrsave(bool enable)
{
	QN_TODO("스크린 세이버: 이거 할 수 있나?");
}

//
void stub_system_enable_drop(bool enable)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;
	Atom xdnd = X11XInternAtom(x11stub.display, "XdndAware", False);
	if (enable)
	{
		if (QN_TMASK(x11stub.base.flags, QGFEATURE_ENABLE_DROP))
			return;
		QN_SMASK(&x11stub.base.flags, QGFEATURE_ENABLE_DROP, true);
		Atom version = 5;
		X11XChangeProperty(x11stub.display, x11stub.window, xdnd, XA_ATOM, 32, PropModeReplace, (byte*)&version, 1);
	}
	else
	{
		if (QN_TMASK(x11stub.base.flags, QGFEATURE_ENABLE_DROP) == false)
			return;
		QN_SMASK(&x11stub.base.flags, QGFEATURE_ENABLE_DROP, false);
		X11XDeleteProperty(x11stub.display, x11stub.window, xdnd);
	}
}

// 
void stub_system_set_title(const char* u8text)
{
	X11Stub* stub = (X11Stub*)qg_stub_instance;
	qn_ret_if_fail(x11stub.display != NULL && x11stub.window != 0);

	qn_free(x11stub.window_title);
	x11stub.window_title = qn_strdup(u8text != NULL ? u8text : "QS");

	x11_set_title(x11stub.window, x11stub.window_title);
}

//
void stub_system_hold_mouse(bool hold)
{
	QN_SMASK(&x11stub.base.stats, QGSSTT_HOLD, false);

	if (hold)
	{
		if (QN_TMASK(x11stub.base.window_stats, QGWEV_SHOW) == false)
			return;

		if (x11stub.broken_grab == false)
		{
			const uint mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
			int res = 0;
			for (int i = 0; i < 100; i++)
			{
				res = X11XGrabPointer(x11stub.display, x11stub.window, False, mask,
					GrabModeAsync, GrabModeAsync, x11stub.window, None, CurrentTime);
				if (res == GrabSuccess)
				{
					QN_SMASK(&x11stub.base.stats, QGSSTT_HOLD, true);
					break;
				}
				qn_sleep(50);
			}
			if (res != GrabSuccess)
				x11stub.broken_grab = true;
		}
		X11XRaiseWindow(x11stub.display, x11stub.window);
	}
	else
	{
		// 푸는건 쉽네
		X11XUngrabPointer(x11stub.display, CurrentTime);
	}
	X11XSync(x11stub.display, False);
}

//
void stub_system_calc_layout(void)
{
	// 폭과 높이는 문제 없는데, 좌표가 안바뀌는거 같은데
	XWindowAttributes wattr;
	X11XGetWindowAttributes(x11stub.display, x11stub.window, &wattr);
	qm_set4(&x11stub.base.window_bound, wattr.x, wattr.y, wattr.x + wattr.width, wattr.y + wattr.height);
	qm_set2(&x11stub.base.client_size, wattr.width, wattr.height);
	x11stub.local_bound = x11stub.base.window_bound;
	x11stub.visual = wattr.visual;
	x11stub.colormap = wattr.colormap;
}

// 클래스 이름 만들기
static char* x11_get_class_name(void)
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

// 아톰 만들기, 너무 길어서 따로 뺌
static void x11_init_atoms(void)
{
	Display* display = x11stub.display;
#define DEF_X11_ATOM(x) x11stub.atoms.x = X11XInternAtom(display, #x, False)
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

// 에러 핸들러 설정
static void x11_set_error_handler(int (*handler)(Display*, XErrorEvent*))
{
	qn_assert(x11stub.eh.handler == NULL, "previous handler not released");
	X11XSync(x11stub.display, False);
	x11stub.eh.code = Success;
	x11stub.eh.handler = X11XSetErrorHandler(handler);
}

// 에러 핸들러 해제
static void x11_reset_error_handler(void)
{
	X11XSync(x11stub.display, False);
	X11XSetErrorHandler(x11stub.eh.handler);
	x11stub.eh.handler = NULL;
}

// 윈도우 메니저 체크용
static int x11_check_wm_error_handler(Display* d, XErrorEvent* e)
{
	if ((x11stub.eh.code = e->error_code) == BadWindow)
		return 0;
	return x11stub.eh.handler(d, e);
}

// 윈도우 매니저 체크
static void x11_check_wm(void)
{
	x11_set_error_handler(x11_check_wm_error_handler);

	Window window = 0;
	X11Property prop;
	if (x11_get_prop_simple(x11stub.root, x11stub.atoms._NET_SUPPORTING_WM_CHECK, XA_WINDOW, &prop))
	{
		if (prop.read != 0)
			window = ((Window*)prop.data)[0];
		x11_clean_prop(&prop);
	}

	if (window != 0)
	{
		prop.window = window;
		if (x11_get_prop(&prop) == false || prop.read == 0 || window != ((Window*)prop.data)[0])
			window = None;
		x11_clean_prop(&prop);
	}

	x11_reset_error_handler();

	x11stub.has_wm = window != 0;

#ifdef _DEBUG
	if (window != 0)
	{
		char* wm_name = x11_get_title(window);
		qn_debug_outputf(false, "X11 STUB", "window manager: %s", wm_name);
		qn_free(wm_name);
	}
#endif
}

// 타이틀 얻기
static char* x11_get_title(Window window)
{
	X11Property prop;
	if (x11_get_prop_simple(window, x11stub.atoms._NET_WM_NAME, x11stub.atoms.UTF8_STRING, &prop))
	{
		char* title = qn_strdup((char*)prop.data);
		x11_clean_prop(&prop);
		return title;
	}
	if (x11_get_prop_simple(window, XA_WM_NAME, XA_STRING, &prop))
	{
		char* title = qn_strdup((char*)prop.data);
		x11_clean_prop(&prop);
		return title;
	}
	qn_debug_outputs(false, "X11 STUB", "failed to read title");
	return qn_strdup("(unknown)");
}

// 타이틀 설정
static bool x11_set_title(Window window, char* title)
{
	XTextProperty prop;
	int res;

	if (X11Xutf8TextListToTextProperty != NULL)
	{
		res = X11Xutf8TextListToTextProperty(x11stub.display, &title, 1, XUTF8StringStyle, &prop);
		if (res == Success)
		{
			X11XSetTextProperty(x11stub.display, window, &prop, x11stub.atoms._NET_WM_NAME);
			X11XFree(prop.value);
			X11XFlush(x11stub.display);
			return true;
		}
	}

	X11XmbTextListToTextProperty(x11stub.display, &title, 1, XTextStyle, &prop);
	if (X11XSupportsLocale() == False)
	{
		qn_debug_outputs(false, "X11 STUB", "current locale not supported");
		return false;
	}
	if (res < 0)	// -1이면 메모리없음, -2이면 로캘 지원 안함, -3이면 컨버트 실패
		return false;
	if (res > 0)	// res번째 글자가 못쓴다
		return false;

	X11XSetTextProperty(x11stub.display, window, &prop, XA_WM_NAME);
	X11XFree(prop.value);
	X11XFlush(x11stub.display);
	return true;
}

// 윈도우 매니저 상태 설정
static void x11_set_net_wm(QgFlag flags)
{
	Atom atoms[16];
	int atom_count = 0;

	if (QN_TMASK(flags, QGFLAG_FOCUS))
		atoms[atom_count++] = x11stub.atoms._NET_WM_STATE_FOCUSED;
	if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
		atoms[atom_count++] = x11stub.atoms._NET_WM_STATE_FULLSCREEN;

	if (atom_count == 0)
	{
		X11XDeleteProperty(x11stub.display, x11stub.window, x11stub.atoms._NET_WM_STATE);
		return;
	}
	X11XChangeProperty(x11stub.display, x11stub.window, x11stub.atoms._NET_WM_STATE,
		XA_ATOM, 32, PropModeReplace, (byte*)atoms, atom_count);
}

// 비주얼 정보
static bool x11_get_visual_info(int screen, XVisualInfo* info)
{
	const int depth = DefaultDepth(x11stub.display, screen);
	if (X11XMatchVisualInfo(x11stub.display, x11stub.screen, depth, DirectColor, info) == 0 &&
		X11XMatchVisualInfo(x11stub.display, x11stub.screen, depth, TrueColor, info) == 0 &&
		X11XMatchVisualInfo(x11stub.display, x11stub.screen, depth, PseudoColor, info) == 0 &&
		X11XMatchVisualInfo(x11stub.display, x11stub.screen, depth, StaticColor, info) == 0)
		return false;
	return true;
}

// XRandR 디스플레이 추가
static bool x11_add_xrandr_display(int screen, RROutput output, XRRScreenResources* res, bool push_event)
{
	Atom EDID = X11XInternAtom(x11stub.display, "EDID", False);
	XVisualInfo vinfo;

	if (x11_get_visual_info(screen, &vinfo) == false ||
		vinfo.class != DirectColor || vinfo.class != TrueColor)
		return false;

	XRROutputInfo* outinfo = X11XRRGetOutputInfo(x11stub.display, res, output);
	if (outinfo == NULL || outinfo->crtc == 0 || outinfo->connection == RR_Disconnected)
	{
		X11XRRFreeOutputInfo(outinfo);
		return false;
	}

	char name[128];
	qn_strncpy(name, QN_COUNTOF(name), outinfo->name, QN_COUNTOF(name));
	unsigned long mmwidth = outinfo->mm_width, mmheight = outinfo->mm_height;
	RRCrtc ocrtc = outinfo->crtc;
	X11XRRFreeOutputInfo(outinfo);

	XRRCrtcInfo* crtc = X11XRRGetCrtcInfo(x11stub.display, res, ocrtc);
	if (crtc == NULL)
		return false;

	RRMode rrmode = crtc->mode;
	X11Monitor* mon = qn_alloc_1(X11Monitor);
	mon->base.width = crtc->width;
	mon->base.height = crtc->height;
	int dx = crtc->x, dy = crtc->y;
	X11XRRFreeCrtcInfo(crtc);

	qn_strncpy(mon->base.name, QN_COUNTOF(mon->base.name), name, QN_COUNTOF(mon->base.name));
	mon->base.no = screen;
	mon->base.x = dx;
	mon->base.y = dy;
	mon->base.depth = vinfo.depth;
	mon->base.dpi = stub_calc_diagonal_dpi(mon->base.width, mon->base.height, (float)mmwidth / 25.4f, (float)mmheight / 25.4f);
	mon->base.hdpi = mmwidth ? ((float)mon->base.width * 25.4f / mmwidth) : 0.0f;
	mon->base.vdpi = mmheight ? ((float)mon->base.height * 25.4 / mmheight) : 0.0f;
	mon->visual = vinfo.visual;
	mon->xrand_mode = rrmode;
	mon->xrand_output = output;
	qn_pctnr_add(&x11stub.base.mon, mon);

	if (qg_stub_instance != NULL && push_event)
	{ /* 여기서 이벤트 */
	}

	return true;
}

// XRandR로 검사
static bool x11_check_xrandr_modes(void)
{
	int xrandr_error;
	if (X11XRRQueryExtension(x11stub.display, &x11stub.xrandr_event, &xrandr_error) == False)
		return false;

	RROutput primary = X11XRRGetOutputPrimary(x11stub.display, x11stub.root);
	const int screen_count = ScreenCount(x11stub.display);
	const int default_screen = DefaultScreen(x11stub.display);
	for (int i = 1; i >= 0; i--)
	{
		for (int s = 0; s < screen_count; s++)
		{
			if (i && (s != default_screen))
				continue;

			XRRScreenResources* res = X11XRRGetScreenResourcesCurrent(x11stub.display, x11stub.root);
			if (res == NULL || res->noutput == 0)
			{
				if (res != NULL)
					X11XRRFreeScreenResources(res);
				res = X11XRRGetScreenResources(x11stub.display, x11stub.root);
				if (res == NULL)
					continue;
			}

			for (int o = 0; o < res->noutput; o++)
			{
				if ((i && res->outputs[o] != primary) ||
					(!i && s == default_screen && res->outputs[i] == primary))
					continue;
				if (x11_add_xrandr_display(s, res->outputs[o], res, false))
					break;
			}

			X11XRRFreeScreenResources(res);
			X11XRRSelectInput(x11stub.display, RootWindow(x11stub.display, s), RROutputChangeNotifyMask);
		}
	}

	return qn_pctnr_count(&x11stub.base.mon) > 0;
}

// Xlib로 검사
static bool x11_check_xlib_modes(void)
{
	const int screen_count = ScreenCount(x11stub.display);
	const int default_screen = DefaultScreen(x11stub.display);
	const Screen* screen = ScreenOfDisplay(x11stub.display, x11stub.screen);

	XVisualInfo vinfo;
	if (x11_get_visual_info(default_screen, &vinfo) == false)
		return false;

	X11Monitor* mon = qn_alloc_1(X11Monitor);
	mon->base.width = WidthOfScreen(screen);
	mon->base.height = HeightOfScreen(screen);
	int mmwidth = WidthMMOfScreen(screen);
	int mmheight = HeightMMOfScreen(screen);

	qn_strcpy(mon->base.name, QN_COUNTOF(mon->base.name), "Generic X11 Display");
	mon->base.no = default_screen;
	mon->base.x = 0;
	mon->base.y = 0;
	mon->base.depth = vinfo.depth;
	mon->base.dpi = stub_calc_diagonal_dpi(mon->base.width, mon->base.height, (float)mmwidth / 25.4f, (float)mmheight / 25.4f);
	mon->base.hdpi = mmwidth ? ((float)mon->base.width * 25.4f / mmwidth) : 0.0f;
	mon->base.vdpi = mmheight ? ((float)mon->base.height * 25.4 / mmheight) : 0.0f;
	mon->visual = vinfo.visual;
	mon->xrand_mode = (XID)-1;
	mon->xrand_output = (XID)-1;

	char* xft = X11XGetDefault(x11stub.display, "Xft", "dpi");
	if (xft != NULL)
	{
		uint dpi = qn_strtoi(xft, 10);
		if (dpi > 0)
		{
			mon->base.hdpi = (float)dpi;
			mon->base.vdpi = (float)dpi;
		}
	}

	qn_pctnr_add(&x11stub.base.mon, mon);

	if (qg_stub_instance != NULL)
	{ /* 여기서 이벤트 */
	}

	return true;
}

// 모드 검사
static bool x11_check_modes(void)
{
	int major, minor;
	if (X11XRRQueryVersion != NULL && XRRQueryVersion(x11stub.display, &major, &minor) &&
		(major >= 2 || (major == 1 && minor >= 3)) && x11_check_xrandr_modes())
		return true;
	return x11_check_xlib_modes();
}


#endif
