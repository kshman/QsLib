//
// qg_stub_windows.c - 윈도우 스터브
// 2023-12-13 by kim
//

#include "pch.h"
#include "qs_qn.h"
#if defined USE_X11 && defined USE_ES3 && !defined USE_SDL2
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
#include "glad/glad.h"


//////////////////////////////////////////////////////////////////////////
// X11 스터브 / OPENGL

#if defined USE_ES3
void x11_es_load(void)
{
}


#endif

#endif
