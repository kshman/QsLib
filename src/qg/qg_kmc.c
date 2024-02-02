//
// qg_kmc.c - 메시지 처리기
// 2023-12-13 by kim
//

#include "pch.h"
#include "qs_qg.h"

// 모를 때 쓰는 문자열 (스레드 완전 위험)
const char* qg_unknown_str(int value, bool hex)
{
	static char unknown_text[64];
	if (hex)
		qn_snprintf(unknown_text, QN_COUNTOF(unknown_text), "UNKNOWN(%X)", value);
	else
		qn_snprintf(unknown_text, QN_COUNTOF(unknown_text), "UNKNOWN(%d)", value);
	return unknown_text;
}


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
	return key_names[(byte)key] == NULL ? "UNKNOWN" : key_names[(byte)key];
}

//
const char* qg_qim_to_str(const QimButton button)
{
	static const char* mouse_button_names[] =
	{
		"NONE",
		"LEFT",
		"RIGHT",
		"MIDDLE",
		"X1",
		"X2"
	};
	if ((size_t)button < QN_COUNTOF(mouse_button_names))
		return mouse_button_names[button];
	return qg_unknown_str(button, false);
}

//
const char* qg_qic_to_str(const QicButton button)
{
	static const char* controller_button_names[] =
	{
		"NONE",
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
	if ((size_t)button < QN_COUNTOF(controller_button_names))
		return controller_button_names[button];
	return qg_unknown_str(button, false);
}

//
const char* qg_event_to_str(const QgEventType ev)
{
	static const char* event_names[QGEV_MAX_VALUE] =
	{
		"NONE",				// QGEV_NONE
		"SYSWM",			// QGEV_SYSWM
		"ACTIVE",			// QGEV_ACTIVE
		"LAYOUT",			// QGEV_LAYOUT
		"ENTER",			// QGEV_ENTER
		"LEAVE",			// QGEV_LEAVE
		"MOUSEMOVE",		// QGEV_MOUSEMOVE
		"MOUSEDOWN",		// QGEV_MOUSEDOWN
		"MOUSEUP",			// QGEV_MOUSEUP
		"MOUSEWHEEL",		// QGEV_MOUSEWHEEL
		"MOUSEDOUBLE",		// QGEV_MOUSEDOUBLE
		"KEYDOWN",			// QGEV_KEYDOWN
		"KEYUP",			// QGEV_KEYUP
		"TEXTINPUT",		// QGEV_TEXTINPUT
		"WINDOW",			// QGEV_WINDOW
		"DROPBEGIN",		// QGEV_DROPBEGIN
		"DROPEND",			// QGEV_DROPEND
		"DROPFILE",			// QGEV_DROPFILE
		"MONITOR",			// QGEV_MONITOR
		"EXIT",				// QGEV_EXIT
	};
	if ((size_t)ev < QN_COUNTOF(event_names))
		return event_names[ev];
	return qg_unknown_str(ev, false);
}

//
const char* qg_window_event_to_str(const QgWindowEventType wev)
{
	static const char* window_event_names[QGWEV_MAX_VALUE] =
	{
		"NONE",				// QGWEV_NONE
		"SHOW",				// QGWEV_SHOW
		"HIDE",				// QGWEV_HIDE
		"PAINTED",			// QGWEV_PAINTED
		"RESTORED",			// QGWEV_RESTORED
		"MAXIMIZED",		// QGWEV_MAXIMIZED
		"MINIMIZED",		// QGWEV_MINIMIZED
		"MOVED",			// QGWEV_MOVED
		"SIZED",			// QGWEV_SIZED
		"FOCUS",			// QGWEV_FOCUS
		"LOSTFOCUS",		// QGWEV_LOSTFOCUS
		"CLOSE",			// QGWEV_CLOSE
	};
	if ((size_t)wev < QN_COUNTOF(window_event_names))
		return window_event_names[wev];
	return qg_unknown_str(wev, false);
}

//
static struct QGLOUSAGEMAP
{
	QgLayoutUsage	usage;
	const char*		name;
	size_t			hash;
} QgLayoutUsageMap[] =
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
void qg_init_converters(void)
{
	static bool inited = false;
	qn_ret_if_ok(inited);
	for (size_t i = 0; i < QN_COUNTOF(QgLayoutUsageMap); i++)
		QgLayoutUsageMap[i].hash = qn_strhash(QgLayoutUsageMap[i].name);
	for (size_t i = 0; i < QN_COUNTOF(QgScAutoMap); i++)
		QgScAutoMap[i].hash = qn_strhash(QgScAutoMap[i].name);
	inited = true;
}

//
QgClrFmt qg_rgba_to_clrfmt(int red, int green, int blue, int alpha, bool is_float)
{
	if (red == 8)
	{
		if (green == 8 && blue == 8)
			return alpha == 8 ? QGCF_R8G8B8A8 : QGCF_R8G8B8;
		return green == 8 ? QGCF_R8G8 : QGCF_R8;
	}
	if (red == 32)
	{
		if (is_float)
		{
			if (green == 32)
			{
				if (blue == 32)
					return alpha == 32 ? QGCF_R32G32B32A32F : QGCF_R32G32B32F;
				return QGCF_R32G32F;
			}
			return QGCF_R32F;
		}
		if (green == 32)
		{
			if (blue == 32)
				return alpha == 32 ? QGCF_R32G32B32A32 : QGCF_R32G32B32;
			return QGCF_R32G32;
		}
		return QGCF_R32;
	}
	if (red == 16)
	{
		if (is_float)
		{
			if (blue == 16)
				return alpha == 16 ? QGCF_R16G16B16A16F : QGCF_UNKNOWN;
			return green == 16 ? QGCF_R16G16F : QGCF_R16F;
		}
		if (blue == 16)
			return alpha == 16 ? QGCF_R16G16B16A16 : QGCF_UNKNOWN;
		return green == 16 ? QGCF_R16G16 : QGCF_R16;
	}
	if (red == 11 && green == 11 && blue == 10 && alpha == 2)
		return QGCF_R11G11B10F;
	if (red == 10 && green == 10 && blue == 10 && alpha == 2)
		return QGCF_R10G10B10A2;
	if (red == 5 && blue == 5)
	{
		if (green == 6)
			return QGCF_R5G6B5;
		if (green == 5)
			return alpha == 1 ? QGCF_R5G5B5A1 : QGCF_R5G6B5;
	}
	else if (red == 4 && blue == 4 && green == 4 && alpha == 4)
		return QGCF_R4G4B4A4;
	// QGCF_A8
	// QGCF_L8
	// QGCF_D32F
	// QGCF_D24S8
	return QGCF_UNKNOWN;
}

//
const char* qg_clrfmt_to_str(QgClrFmt fmt)
{
	static const char* fmt_names[QGCF_MAX_VALUE] =
	{
		"UNKNOWN",			// QGCF_UNKNOWN
		"R32G32B32A32F",	// QGCF_R32G32B32A32F
		"R32G32B32F",		// QGCF_R32G32B32F
		"R32G32F",			// QGCF_R32G32F
		"R32F",				// QGCF_R32F
		"R32G32B32A32",		// QGCF_R32G32B32A32
		"R32G32B32",		// QGCF_R32G32B32
		"R32G32",			// QGCF_R32G32
		"R32",				// QGCF_R32
		"R16G16B16A16F",	// QGCF_R16G16B16A16F
		"R16G16F",			// QGCF_R16G16F
		"R16F",				// QGCF_R16F
		"R11G11B10F",		// QGCF_R11G11B10F
		"R16G16B16A16",		// QGCF_R16G16B16A16
		"R16G16",			// QGCF_R16G16
		"R16",				// QGCF_R16
		"R10G10B10A2",		// QGCF_R10G10B10A2
		"R8G8B8A8",			// QGCF_R8G8B8A8
		"R8G8B8",			// QGCF_R8G8B8
		"R8G8",				// QGCF_R8G8
		"R8",				// QGCF_R8
		"A8",				// QGCF_A8
		"L8",				// QGCF_L8
		"A8L8",				// QGCF_A8L8
		"R5G6B5",			// QGCF_R5G6B5
		"R5G5B5A1",			// QGCF_R5G5B5A1
		"R4G4B4A4",			// QGCF_R4G4B4A4
		"D32F",				// QGCF_D32F
		"D24S8",			// QGCF_D24S8
		"DXT1",				// QGCF_DXT1,
		"DXT3",				// QGCF_DXT3,
		"DXT5",				// QGCF_DXT5,
		"EXT1",				// QGCF_EXT1,
		"EXT2",				// QGCF_EXT2,
		"EXT2_EAC",			// QGCF_EXT2_EAC,
		"ASTC4x8",			// QGCF_ASTC4,
		"ASTC8x8",			// QGCF_ASTC8,
	};
	if ((size_t)fmt < QN_COUNTOF(fmt_names))
		return fmt_names[fmt];
	return qg_unknown_str(fmt, false);
}

//
QgLayoutUsage qg_str_to_layout_usage(size_t hash, const char* name)
{
	for (size_t i = 0; i < QN_COUNTOF(QgLayoutUsageMap); i++)
	{
		if (hash == QgLayoutUsageMap[i].hash && qn_stricmp(name, QgLayoutUsageMap[i].name) == 0)
			return QgLayoutUsageMap[i].usage;
	}
	return QGLOU_UNKNOWN;
}

//
QgScAuto qg_str_to_shader_const_auto(size_t hash, const char* name)
{
	for (size_t i = 0; i < QN_COUNTOF(QgScAutoMap); i++)
	{
		if (hash == QgScAutoMap[i].hash && qn_stricmp(name, QgScAutoMap[i].name) == 0)
			return QgScAutoMap[i].value;
	}
	return QGSCA_UNKNOWN;
}

//
const char* qg_layout_usage_to_str(const QgLayoutUsage usage)
{
	for (size_t i = 0; i < QN_COUNTOF(QgLayoutUsageMap); i++)
	{
		if (usage == QgLayoutUsageMap[i].usage)
			return QgLayoutUsageMap[i].name + 1;	// 'a' 제거
	}
	return qg_unknown_str(usage, false);
}

//
const char* qg_shader_const_auto_to_str(const QgScAuto sca)
{
	for (size_t i = 0; i < QN_COUNTOF(QgScAutoMap); i++)
	{
		if (sca == QgScAutoMap[i].value)
			return QgScAutoMap[i].name;
	}
	return qg_unknown_str(sca, false);
}


//////////////////////////////////////////////////////////////////////////
// 다른 인터페이스와 맞추기 용도

#ifdef USE_WAYLAND
#include <xkbcommon/xkbcommon-keysyms.h>
//
QikKey xkbsym_to_qik(uint32_t sym)
{
	return QIK_NONE;
}
#endif
