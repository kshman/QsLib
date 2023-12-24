//
// qgx11_stub.h - X11 스터브
// 2023-12-23 by kim
//
// 많은 부분을 SDL3에서 베껴왔어요 (https://www.libsdl.org/)
//

#pragma once

#include "pch.h"
#include "qg_stub.h"
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/XKBlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xdbe.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/scrnsaver.h>
//#include <X11/Xresource.h>
//#include <X11/Xproto.h>
//#include <X11/extensions/Xext.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
//#include <X11/extensions/Xfixes.h>
//#include <X11/extensions/shape.h>

// SO
#define DEF_X11_FUNC(ret,name,args)		typedef ret(*QN_CONCAT(PFNX11, name)) args;
#define DEF_X11_VAFUNC(ret,name,arg0)	typedef ret(*QN_CONCAT(PFNX11, name)) (arg0, ...);
#include "qgx11_func.h"
#define DEF_X11_FUNC(ret,name,args)		extern QN_CONCAT(PFNX11, name) QN_CONCAT(X11, name);
#define DEF_X11_VAFUNC(ret,name,arg0)	extern QN_CONCAT(PFNX11, name) QN_CONCAT(X11, name);
#include "qgx11_func.h"

// X11 모니터
typedef struct X11Monitor X11Monitor;
struct X11Monitor
{
	StubMonitor			base;
	Visual*				visual;
	RROutput			xrand_output;
	RRMode				xrand_mode;
};

// 에러 핸들러
typedef struct X11ErrorHandler X11ErrorHandler;
struct X11ErrorHandler
{
	int					code;
	int (*handler)(Display*, XErrorEvent*);
};

// 아톰
typedef struct X11Atoms X11Atoms;
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
};

/// @brief X11 스터브
typedef struct X11Stub X11Stub;
struct X11Stub
{
	StubBase			base;

	Display*			display;
	int					screen;
	Window				root;
	Window				window;

	pid_t				pid;
	XID					group;

	Visual*				visual;
	Colormap			colormap;

	GC					gc;
	XIM					xim;
	XIC					xic;

	X11ErrorHandler		eh;
	X11Atoms			atoms;

	int					xrandr_event;

	char*				class_name;
	char*				window_title;

	QmRect				local_bound;

	//HCURSOR				mouse_cursor;
	//WPARAM				mouse_wparam;
	//LPARAM				mouse_lparam;
	//QimMask				mouse_pending;

	bool				has_wm;
	bool				broken_grab;

	//bool				clear_background;
};
extern X11Stub x11stub;

// 프로퍼티
typedef struct X11Property X11Property;
struct X11Property
{
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

// 프로퍼티 얻기
QN_INLINE bool x11_get_prop(X11Property* prop)
{
	prop->ok = X11XGetWindowProperty(x11stub.display, prop->window, prop->atom,
		prop->offset, prop->length, False, prop->request,
		&prop->type, &prop->format, &prop->read, &prop->left, &prop->data) == Success;
	return prop->ok;
}

// 간단 프로퍼티 얻기
QN_INLINE bool x11_get_prop_simple(Window window, Atom atom, Atom request, X11Property* prop)
{
	prop->window = window;
	prop->atom = atom;
	prop->offset = 0;
	prop->length = LONG_MAX;
	prop->request = request;
	return x11_get_prop(prop);
}

// 프로퍼티 해제
QN_INLINE void x11_clean_prop(X11Property* prop)
{
	if (prop->ok && prop->data != NULL)
		X11XFree(prop->data);
}
