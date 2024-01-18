//
// qg_kmc.c - 메시지 처리기
// 2023-12-13 by kim
//

#include "pch.h"
#include "qs_qg.h"

//////////////////////////////////////////////////////////////////////////
// qmkc supp

static const bool qik_key_enables[] =
{
	/*00*/ false, false, false, false, false, false, false, false, true, true, false, false, false, true, false, false,
	/*10*/ false, false, false, true, true, false, false, false, false, false, false, true, false, false, false, false,
	/*20*/ true, true, true, true, true, true, true, true, true, false, false, false, true, true, true, false,
	/*30*/ true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false,
	/*40*/ false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,
	/*50*/ true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false,
	/*60*/ true, true, true, true, true, true, true, true, true, true, true,  true,  false, true,  true,  true,
	/*70*/ true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,
	/*80*/ true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false,
	/*90*/ true,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	/*A0*/ true,  true,  true, true, true, true, true, true, true, true, true, true, true, true, true, true,
	/*B0*/ true, true, true, true, false, false, false, false, false, false, true, true, true, true, true, true,
	/*C0*/ true, false,  false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	/*D0*/ false,false, false, false, false, false, false, false, false, false, false, true, true,  true, true, false,
	/*E0*/ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	/*F0*/ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
};

//
bool qg_qik_usable(const QikKey key)
{
	return qik_key_enables[(byte)key];
}

//
const char* qg_qik_to_str(const QikKey key)
{
	static const char* key_names[] =
	{
		/*00*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "BACK", "TAB", NULL, NULL, "CLEAR", "RETURN", NULL, NULL,
		/*10*/ NULL, NULL, NULL, "PAUSE", "CAPSLOCK", "HANGUL/KANA", NULL, "JUNJA", "FINAL", "HANJA/KANJI", NULL, "ESCAPE", NULL, NULL, NULL, NULL,
		/*20*/ "SPACE", "PGUP", "PGDN", "END", "HOME", "LEFT", "UP", "RIGHT", "DOWN", NULL, NULL, NULL, "PRTSCR", "INS", "DEL", "HELP",
		/*30*/ "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL, NULL, NULL, NULL, NULL, NULL,
		/*40*/ NULL, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
		/*50*/ "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "LWIN", "RWIN", "APPS", NULL, "SLEEP",
		/*60*/ "NUM0", "NUM1", "NUM2", "NUM3", "NUM4", "NUM5", "NUM6", "NUM7", "NUM8", "NUM9", "NUMMUL", "NUMADD", "NUMSEP", "NUMSUB", "NUMDOT", "NUMDIV",
		/*70*/ "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16",
		/*80*/ "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		/*90*/ "NUMLOCK", "SCRL", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		/*A0*/ "LSHIFT", "RSHIFT", "LCTRL", "RCTRL", "LALT", "RALT", "NAVBACK", "NAVFWD", "NAVREL", "NAVSTOP", "NAVSRCH", "NAVFAV", "NAVHOM", "VOLMUT", "VOLDN", "VOLUP",
		/*B0*/ "MDNEXT", "MDPREV", "MDSTOP", "MDPLAY", NULL, NULL, NULL, NULL, NULL, NULL, "SEMI", "ADD", "COMMA", "SUB", "DOT", "SLASH",
		/*C0*/ "TILT", NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		/*D0*/ NULL,NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "LBR", "BACKSLASH", "RBR", "QUOTE", NULL,
		/*E0*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		/*F0*/ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	};
	return key_names[(byte)key];
}

//
const char* qg_qim_to_str(const QimButton button)
{
	static const char* mouse_button_names[] =
	{
		NULL,
		"LEFT",
		"RIGHT",
		"MIDDLE",
		"X1",
		"X2"
	};
	return mouse_button_names[(nuint)button < QN_COUNTOF(mouse_button_names) ? button : 0];
}

//
const char* qg_qic_to_str(const QicButton button)
{
#ifndef __EMSCRIPTEN__
	static const char* controller_button_names[] =
	{
		NULL,
		"UP",
		"DOWN",
		"LEFT",
		"RIGHT",
		"START",
		"BACK",
		"THUMB LEFT",
		"THUMB RIGHT",
		"SHOULDER LEFT",
		"SHOULDER RIGHT",
		"A",
		"B",
		"X",
		"Y"
	};
	return controller_button_names[(nuint)button < QN_COUNTOF(controller_button_names) ? button : 0];
#else
	return NULL;
#endif
}

//
const char* qg_event_to_str(const QgEventType ev)
{
	static struct EventMap
	{
		nuint			ev;
		const char*		str;
	}
	event_map[] =
	{
#define DEF_EVENT(ev) { QGEV_##ev, #ev }
		DEF_EVENT(NONE),
		DEF_EVENT(SYSWM),
		DEF_EVENT(ACTIVE),
		DEF_EVENT(LAYOUT),
		DEF_EVENT(ENTER),
		DEF_EVENT(LEAVE),
		DEF_EVENT(MOUSEMOVE),
		DEF_EVENT(MOUSEDOWN),
		DEF_EVENT(MOUSEUP),
		DEF_EVENT(MOUSEWHEEL),
		DEF_EVENT(MOUSEDOUBLE),
		DEF_EVENT(KEYDOWN),
		DEF_EVENT(KEYUP),
		DEF_EVENT(TEXTINPUT),
		DEF_EVENT(WINDOW),
		DEF_EVENT(DROPBEGIN),
		DEF_EVENT(DROPEND),
		DEF_EVENT(DROPFILE),
		DEF_EVENT(MONITOR),
		DEF_EVENT(EXIT),
		{ 0, NULL },
#undef DEF_EVENT
	};
	for (const struct EventMap* p = event_map; p->str != NULL; p++)
	{
		if (p->ev == (nuint)ev)
			return p->str;
	}
	static char unknown_text[32];
	qn_snprintf(unknown_text, 32 - 1, "UNKNOWN(%d)", ev);
	return unknown_text;
}

//
const char* qg_window_event_to_str(const QgWindowEventType wev)
{
	static struct WindowEventMap
	{
		nuint				wev;
		const char*			str;
	}
	window_event_map[] =
	{
#define DEF_WINDOW_EVENT(ev) { QGWEV_##ev, #ev }
		DEF_WINDOW_EVENT(NONE),
		DEF_WINDOW_EVENT(SHOW),
		DEF_WINDOW_EVENT(HIDE),
		DEF_WINDOW_EVENT(PAINTED),
		DEF_WINDOW_EVENT(RESTORED),
		DEF_WINDOW_EVENT(MAXIMIZED),
		DEF_WINDOW_EVENT(MINIMIZED),
		DEF_WINDOW_EVENT(MOVED),
		DEF_WINDOW_EVENT(SIZED),
		DEF_WINDOW_EVENT(FOCUS),
		DEF_WINDOW_EVENT(LOSTFOCUS),
		DEF_WINDOW_EVENT(CLOSE),
		{ 0, NULL },
#undef DEF_WINDOW_EVENT
	};
	for (const struct WindowEventMap* p = window_event_map; p->str != NULL; p++)
	{
		if (p->wev == (nuint)wev)
			return p->str;
	}
	static char unknown_text[32];
	qn_snprintf(unknown_text, 32 - 1, "UNKNOWN(%d)", wev);
	return unknown_text;
}


//////////////////////////////////////////////////////////////////////////
// 내부 사용 용도

#ifdef USE_SDL2
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>

// SDLK를 QIK로
QikKey sdlk_to_qik(uint sdlk)
{
	static byte s_key_codes[] =
	{
		/*0*/					0,
		/*1*/					0,
		/*2*/					0,
		/*3*/					0,
		/*4*/					0,
		/*5*/					0,
		/*6*/					0,
		/*7*/					0,
		/*8/BACKSPACE*/			QIK_BS,
		/*9/TAB*/				QIK_TAB,
		/*10*/					0,
		/*11*/					0,
		/*12*/					0,
		/*13/RETURN*/			QIK_RETURN,
		/*14*/					0,
		/*15*/					0,
		/*16*/					0,
		/*17*/					0,
		/*18*/					0,
		/*19*/					0,
		/*20*/					0,
		/*21*/					0,
		/*22*/					0,
		/*23*/					0,
		/*24*/					0,
		/*25*/					0,
		/*26*/					0,
		/*27/ESCAPE*/			QIK_ESC,
		/*28*/					0,
		/*29*/					0,
		/*30*/					0,
		/*31*/					0,
		/*32/SPACE*/			QIK_SPACE,
		/*33/EXCLAIM*/			QIK_1,
		/*34/QUOTEDBL*/			QIK_QUOTE,
		/*35/HASH*/				QIK_3,
		/*36/DOLLAR*/			QIK_4,
		/*37/PERCENT*/			QIK_5,
		/*38/AMPERSAND*/		QIK_7,
		/*39/QUOTE*/			QIK_QUOTE,
		/*40/LEFTPAREN*/		QIK_9,
		/*41/RIGHTPAREN*/		QIK_0,
		/*42/ASTERISK*/			QIK_8,
		/*43/PLUS*/				QIK_ADD,
		/*44/COMMA*/			QIK_COMMA,
		/*45/MINUS*/			QIK_SUB,
		/*46/PERIOD*/			QIK_DOT,
		/*47/SLASH*/			QIK_SLASH,
		/*48/0*/				QIK_0,
		/*49/1*/				QIK_1,
		/*50/2*/				QIK_2,
		/*51/3*/				QIK_3,
		/*52/4*/				QIK_4,
		/*53/5*/				QIK_5,
		/*54/6*/				QIK_6,
		/*55/7*/				QIK_7,
		/*56/8*/				QIK_8,
		/*57/9*/				QIK_9,
		/*58/COLON*/			QIK_SEMI,
		/*59/SEMICOLON*/		QIK_SEMI,
		/*60/LESS*/				QIK_COMMA,
		/*61/EQUALS*/			QIK_ADD,
		/*62/GREATER*/			QIK_DOT,
		/*63/QUESTION*/			QIK_SLASH,
		/*64/AT*/				QIK_2,
		/*65*/					0,
		/*66*/					0,
		/*67*/					0,
		/*68*/					0,
		/*69*/					0,
		/*70*/					0,
		/*71*/					0,
		/*72*/					0,
		/*73*/					0,
		/*74*/					0,
		/*75*/					0,
		/*76*/					0,
		/*77*/					0,
		/*78*/					0,
		/*79*/					0,
		/*80*/					0,
		/*81*/					0,
		/*82*/					0,
		/*83*/					0,
		/*84*/					0,
		/*85*/					0,
		/*86*/					0,
		/*87*/					0,
		/*88*/					0,
		/*89*/					0,
		/*90*/					0,
		/*91/LEFTBRACKET*/		QIK_LBR,
		/*92/BACKSLASH*/		QIK_BACKSLASH,
		/*93/RIGHTBRACKET*/		QIK_RBR,
		/*94/CARET*/			QIK_6,
		/*95/UNDERSCORE*/		QIK_SUB,
		/*96/BACKQUOTE*/		QIK_TILT,
		/*97/a*/				QIK_A,
		/*98/b*/				QIK_B,
		/*99/c*/				QIK_C,
		/*100/d*/				QIK_D,
		/*101/e*/				QIK_E,
		/*102/f*/				QIK_F,
		/*103/g*/				QIK_G,
		/*104/h*/				QIK_H,
		/*105/i*/				QIK_I,
		/*106/j*/				QIK_J,
		/*107/k*/				QIK_K,
		/*108/l*/				QIK_L,
		/*109/m*/				QIK_M,
		/*110/n*/				QIK_N,
		/*111/o*/				QIK_O,
		/*112/p*/				QIK_P,
		/*113/q*/				QIK_Q,
		/*114/r*/				QIK_R,
		/*115/s*/				QIK_S,
		/*116/t*/				QIK_T,
		/*117/u*/				QIK_U,
		/*118/v*/				QIK_V,
		/*119/w*/				QIK_W,
		/*120/x*/				QIK_X,
		/*121/y*/				QIK_Y,
		/*122/z*/				QIK_Z,
		/*123*/					0,
		/*124*/					0,
		/*125*/					0,
		/*126*/					0,
		/*127/DELETE*/			QIK_DEL,
	};
	static byte s_scan_code_57[] =
	{
		/*0/57/CAPSLOCK*/		QIK_CAPSLOCK,
		/*1/58/F1*/				QIK_F1,
		/*2/59/F2*/				QIK_F2,
		/*3/60/F3*/				QIK_F3,
		/*4/61/F4*/				QIK_F4,
		/*5/62/F5*/				QIK_F5,
		/*6/63/F6*/				QIK_F6,
		/*7/64/F7*/				QIK_F7,
		/*8/65/F8*/				QIK_F8,
		/*9/66/F9*/				QIK_F9,
		/*10/67/F10*/			QIK_F10,
		/*11/68/F11*/			QIK_F11,
		/*12/69/F12*/			QIK_F12,
		/*13/70/PRTSCR*/		QIK_PRTSCR,
		/*14/71/SCROLL*/		QIK_SCRL,
		/*15/72/PAUSE*/			QIK_PAUSE,
		/*16/73/INSERT*/		QIK_INS,
		/*17/74/HOME*/			QIK_HOME,
		/*18/75/PAGEUP*/		QIK_PGUP,
		/*19/76*/				0,
		/*20/77/END*/			QIK_END,
		/*21/78/PAGEDOWN*/		QIK_PGDN,
		/*22/79/RIGHT*/			QIK_RIGHT,
		/*23/80/LEFT*/			QIK_LEFT,
		/*24/81/DOWN*/			QIK_DOWN,
		/*25/82/UP*/			QIK_UP,
		/*26/83*/				0,
		/*27/84/KP_DIVIDE*/		QIK_NUM_DIV,
		/*28/85/KP_MULTIPLY*/	QIK_NUM_MUL,
		/*29/86/KP_MINUS*/		QIK_NUM_SUB,
		/*30/87/KP_PLUS*/		QIK_NUM_ADD,
		/*31/88/KP_ENTER*/		QIK_RETURN,
		/*32/89/KP_1*/			QIK_NUM_1,
		/*33/90/KP_2*/			QIK_NUM_2,
		/*34/91/KP_3*/			QIK_NUM_3,
		/*35/92/KP_4*/			QIK_NUM_4,
		/*36/93/KP_5*/			QIK_NUM_5,
		/*37/94/KP_6*/			QIK_NUM_6,
		/*38/95/KP_7*/			QIK_NUM_7,
		/*39/96/KP_8*/			QIK_NUM_8,
		/*40/97/KP_9*/			QIK_NUM_9,
		/*41/98/KP_0*/			QIK_NUM_0,
		/*42/99/KP_PERIOD*/		QIK_NUM_DOT,
		/*43/100*/				0,
		/*44/101/APPLICATION*/	0,
		/*45/102/POWER*/		0,
		/*46/103/KP_EQUALS*/	0,
		/*47/104/F13*/			QIK_F13,
		/*48/105/F14*/			QIK_F14,
		/*49/106/F15*/			QIK_F15,
		/*50/107/F16*/			QIK_F16,
		/*51/108/F17*/			QIK_F17,
		/*52/109/F18*/			QIK_F18,
		/*53/110/F19*/			QIK_F19,
		/*54/111/F20*/			QIK_F20,
		/*55/112/F21*/			QIK_F21,
		/*56/113/F22*/			QIK_F22,
		/*57/114/F23*/			QIK_F23,
		/*58/115/F24*/			QIK_F24,
	};
	static byte s_scan_code_224[] =
	{
		/*0/224/LCTRL*/			QIK_LCTRL,
		/*1/225/LSHIFT*/		QIK_LSHIFT,
		/*2/226/LALT*/			QIK_LALT,
		/*3/227/LGUI*/			QIK_LWIN,
		/*4/228/RCTRL*/			QIK_RCTRL,
		/*5/229/RSHIFT*/		QIK_RSHIFT,
		/*6/230/RALT*/			QIK_RALT,
		/*7/231/RGUI*/			QIK_RWIN,
	};
	if (sdlk < 128)
		return (QikKey)s_key_codes[sdlk];
	if (sdlk >= SDLK_CAPSLOCK && sdlk <= SDLK_F24)
		return (QikKey)s_scan_code_57[(sdlk & ~SDLK_SCANCODE_MASK) - SDL_SCANCODE_CAPSLOCK];
	if (sdlk >= SDLK_LCTRL && sdlk <= SDLK_RGUI)
		return (QikKey)s_scan_code_224[(sdlk & ~SDLK_SCANCODE_MASK) - SDL_SCANCODE_LCTRL];
	return QIK_NONE;
}

// SDL 키보드 상태 변환
QikMask kmod_to_qikm(int modifier)
{
	QikMask m = (QikMask)0;
	if (modifier & (KMOD_LSHIFT | KMOD_RSHIFT)) m |= QIKM_SHIFT;
	if (modifier & (KMOD_LCTRL | KMOD_RCTRL)) m |= QIKM_CTRL;
	if (modifier & (KMOD_LALT | KMOD_RALT)) m |= QIKM_ALT;
	if (modifier & KMOD_NUM) m |= QIKM_NUM;
	if (modifier & KMOD_CAPS) m |= QIKM_CAPS;
	if (modifier & KMOD_MODE) m |= QIKM_SCRL;
	return m;
}

// SDL 마우스 버튼을 QIM으로
QimButton sdlm_to_qim(byte button)
{
	switch (button)
	{
		case SDL_BUTTON_LEFT:		return QIM_LEFT;
		case SDL_BUTTON_RIGHT:		return QIM_RIGHT;
		case SDL_BUTTON_MIDDLE:		return QIM_MIDDLE;
		case SDL_BUTTON_X1:			return QIM_X1;
		case SDL_BUTTON_X2:			return QIM_X2;
		default:					return QIM_NONE;
	}
}
#endif

#ifdef USE_WAYLAND
#include <xkbcommon/xkbcommon-keysyms.h>
//
QikKey xkbsym_to_qik(uint32_t sym)
{
	return QIK_NONE;
}
#endif

//
QgClrFmt qg_rgba_to_clrfmt(int red, int green, int blue, int alpha, bool is_float)
{
	if (red == 8)
	{
		if (green == 8 && blue == 8)
			return alpha == 8 ? QGCF32_RGBA : QGCF32_RGB;
		if (green == 0 && blue == 0)
			return QGCF8_L;
	}
	else if (red == 5 && blue == 5)
	{
		if (green == 6)
			return QGCF16_RGB;
		if (green == 5)
			return alpha == 1 ? QGCF16_RGBA : QGCF16_RGB;
	}
	else if (red == 16)
	{
		if (green == 16 && blue == 16)
			return QGCF64F_BGRA;
		if (green == 0 && blue == 0)
			return is_float ? QGCF16F_R : QGCF16_L;
	}
	else if (red == 32)
	{
		if (green == 32 && blue == 32)
			return QGCF128F_BGRA;
		if (green == 0 && blue == 0)
			return QGCF32F_R;
	}
	return QGCF_NONE;
}

//
static struct QGLOUSAGEMAP
{
	QgLoUsage		usage;
	const char*		name;
	size_t			hash;
} QgLoUsageMap[] =
{
	{ QGLOU_POSITION,	"aposition",	0 },
	{ QGLOU_COORD1,		"atexcoord",	0 },
	{ QGLOU_COORD1,		"atexcoord1",	0 },
	{ QGLOU_COORD2,		"atexcoord2",	0 },
	{ QGLOU_COORD3,		"atexcoord3",	0 },
	{ QGLOU_COORD4,		"atexcoord4",	0 },
	{ QGLOU_NORMAL1,	"anormal",		0 },
	{ QGLOU_NORMAL1,	"anormal1",		0 },
	{ QGLOU_NORMAL2,	"anormal2",		0 },
	{ QGLOU_NORMAL3,	"anormal3",		0 },
	{ QGLOU_NORMAL4,	"anormal4",		0 },
	{ QGLOU_BINORMAL1,	"abinormal",	0 },
	{ QGLOU_BINORMAL1,	"abinormal1",	0 },
	{ QGLOU_BINORMAL2,	"abinormal2",	0 },
	{ QGLOU_TANGENT1,	"atangent",		0 },
	{ QGLOU_TANGENT1,	"atangent1",	0 },
	{ QGLOU_TANGENT2,	"atangent2",	0 },
	{ QGLOU_COLOR1,		"acolor",		0 },
	{ QGLOU_COLOR1,		"acolor1",		0 },
	{ QGLOU_COLOR2,		"acolor2",		0 },
	{ QGLOU_COLOR3,		"acolor3",		0 },
	{ QGLOU_COLOR4,		"acolor4",		0 },
	{ QGLOU_COLOR5,		"acolor5",		0 },
	{ QGLOU_COLOR6,		"acolor6",		0 },
	{ QGLOU_BLEND_WEIGHT,"aweight",		0 },
	{ QGLOU_BLEND_INDEX, "aindex",		0 },
	{ QGLOU_BLEND_EXTRA, "aextra",		0 },
	{ QGLOU_BLEND_EXTRA, "aindexextra",	0 },
	// 별명
	{ QGLOU_COORD1,		"acoord",		0 },
	{ QGLOU_COORD1,		"acoord1",		0 },
	{ QGLOU_COORD2,		"acoord2",		0 },
	{ QGLOU_COORD3,		"acoord3",		0 },
	{ QGLOU_COORD4,		"acoord4",		0 },
};

//
static struct QGSHADERCONSTAUTOMAP
{
	QgScAuto		value;
	const char*		name;
	size_t			hash;
} QgScAutoMap[] =
{
	{ QGSCA_ORTHO_PROJ,		"orthoproj",	0 },
	{ QGSCA_WORLD,			"world",		0 },
	{ QGSCA_VIEW,			"view",			0 },
	{ QGSCA_PROJ,			"proj",			0 },
	{ QGSCA_VIEW_PROJ,		"viewproj",		0 },
	{ QGSCA_INV_VIEW,		"invview",		0 },
	{ QGSCA_WORLD_VIEW_PROJ,"worldviewproj", 0 },
	{ QGSCA_TEX1,			"texture",		0 },
	{ QGSCA_TEX1,			"texture1",		0 },
	{ QGSCA_TEX2,			"texture2",		0 },
	{ QGSCA_TEX3,			"texture3",		0 },
	{ QGSCA_TEX4,			"texture4",		0 },
	{ QGSCA_TEX5,			"texture5",		0 },
	{ QGSCA_TEX6,			"texture6",		0 },
	{ QGSCA_TEX7,			"texture7",		0 },
	{ QGSCA_TEX8,			"texture8",		0 },
	{ QGSCA_PROP_VEC1,		"propvec",		0 },
	{ QGSCA_PROP_VEC1,		"propvec1",		0 },
	{ QGSCA_PROP_VEC2,		"propvec2",		0 },
	{ QGSCA_PROP_VEC3,		"propvec3",		0 },
	{ QGSCA_PROP_VEC4,		"propvec4",		0 },
	{ QGSCA_PROP_MAT1,		"propmat",		0 },
	{ QGSCA_PROP_MAT1,		"propmat1",		0 },
	{ QGSCA_PROP_MAT2,		"propmat2",		0 },
	{ QGSCA_PROP_MAT3,		"propmat3",		0 },
	{ QGSCA_PROP_MAT4,		"propmat4",		0 },
	{ QGSCA_MAT_PALETTE,	"matpalette",	0 },
	// 별명
	{ QGSCA_TEX1,			"tex",			0 },
	{ QGSCA_TEX1,			"tex1",			0 },
	{ QGSCA_TEX2,			"tex2",			0 },
	{ QGSCA_TEX3,			"tex3",			0 },
	{ QGSCA_TEX4,			"tex4",			0 },
	{ QGSCA_TEX5,			"tex5",			0 },
	{ QGSCA_TEX6,			"tex6",			0 },
	{ QGSCA_TEX7,			"tex7",			0 },
	{ QGSCA_TEX8,			"tex8",			0 },
};

//
void qg_init_enum_convs(void)
{
	for (size_t i = 0; i < QN_COUNTOF(QgLoUsageMap); i++)
		QgLoUsageMap[i].hash = qn_strhash(QgLoUsageMap[i].name);
	for (size_t i = 0; i < QN_COUNTOF(QgScAutoMap); i++)
		QgScAutoMap[i].hash = qn_strhash(QgScAutoMap[i].name);
}

//
QgLoUsage qg_str_to_layout_usage(size_t hash, const char* name)
{
	for (size_t i = 0; i < QN_COUNTOF(QgLoUsageMap); i++)
	{
		if (hash == QgLoUsageMap[i].hash && qn_stricmp(name, QgLoUsageMap[i].name) == 0)
			return QgLoUsageMap[i].usage;
	}
	return QGLOU_INVALID;
}

//
QgScAuto qg_str_to_shader_const_auto(size_t hash, const char* name)
{
	for (size_t i = 0; i < QN_COUNTOF(QgScAutoMap); i++)
	{
		if (hash == QgScAutoMap[i].hash && qn_stricmp(name, QgScAutoMap[i].name) == 0)
			return QgScAutoMap[i].value;
	}
	return QGSCA_INVALID;
}
