#pragma once

//////////////////////////////////////////////////////////////////////////
// mouse button
#define QIM_LEFT						1
#define QIM_RIGHT						2
#define QIM_MIDDLE						3
#define QIM_XPREV						4
#define QIM_XNEXT						5
#define QIM_X1							QIM_XPREV
#define QIM_X2							QIM_XNEXT

#define QIMM_LEFT						QN_BIT(QIM_LEFT)
#define QIMM_RIGHT						QN_BIT(QIM_RIGHT)
#define QIMM_MIDDLE						QN_BIT(QIM_MIDDLE)
#define QIMM_XPREV						QN_BIT(QIM_XPREV)
#define QIMM_XNEXT						QN_BIT(QIM_XNEXT)
#define QIMM_X1							QIMM_XPREV
#define QIMM_X2							QIMM_XNEXT

#define QIMM_WHEELUP					0x0100
#define QIMM_WHEELDOWN					0x0200
#define QIMM_DOUBLE						0x1000
#define QIMM_TRIPLE						0x2000		// not use

#define QIMT_MOVE						1
#define QIMT_DOWN						2
#define QIMT_UP							3


//////////////////////////////////////////////////////////////////////////
// key board 
// !!! system dependant caution !!!
#define QIK_LBUTTON						0x01
#define QIK_RBUTTON						0x02
#define QIK_CANCEL						0x03
#define QIK_MBUTTON						0x04
#define QIK_XPBUTTON					0x05
#define QIK_XNBUTTON					0x06
#define QIK_BACK						0x08
#define QIK_TAB							0x09
#define QIK_CLEAR						0x0C
#define QIK_RETURN						0x0D
#define QIK_SHIFT						0x10
#define QIK_CTRL						0x11	// CONTROL
#define QIK_ALT							0x12	// MENU
#define QIK_PAUSE						0x13
#define QIK_CAPSLOCK					0x14	// CAPITAL
#define QIK_KANA						0x15
#define QIK_HANGEUL						0x15
#define QIK_HANGUL						0x15
#define QIK_JUNJA						0x17
#define QIK_FINAL						0x18
#define QIK_HANJA						0x19
#define QIK_KANJI						0x19
#define QIK_ESCAPE						0x1B
#define QIK_CONVERT						0x1C
#define QIK_NONCONVERT      			0x1D
#define QIK_ACCEPT						0x1E
#define QIK_MODECHANGE      			0x1F
#define QIK_SPACE						0x20
#define QIK_PGUP						0x21	// PRIOR
#define QIK_PGDN						0x22	// NEXT
#define QIK_END							0x23
#define QIK_HOME						0x24
#define QIK_LEFT						0x25
#define QIK_UP							0x26
#define QIK_RIGHT						0x27
#define QIK_DOWN						0x28
#define QIK_SELECT						0x29
#define QIK_PRINT						0x2A
#define QIK_EXECUTE						0x2B
#define QIK_SNAPSHOT        			0x2C
#define QIK_INSERT						0x2D
#define QIK_DELETE						0x2E
#define QIK_HELP						0x2F
#define QIK_0							0x30
#define QIK_1							0x31
#define QIK_2							0x32
#define QIK_3							0x33
#define QIK_4							0x34
#define QIK_5							0x35
#define QIK_6							0x36
#define QIK_7							0x37
#define QIK_8							0x38
#define QIK_9							0x39
#define QIK_A							0x41
#define QIK_B							0x42
#define QIK_C							0x43
#define QIK_D							0x44
#define QIK_E							0x45
#define QIK_F							0x46
#define QIK_G							0x47
#define QIK_H							0x48
#define QIK_I							0x49
#define QIK_J							0x4A
#define QIK_K							0x4B
#define QIK_L							0x4C
#define QIK_M							0x4D
#define QIK_N							0x4E
#define QIK_O							0x4F
#define QIK_P							0x50
#define QIK_Q							0x51
#define QIK_R							0x52
#define QIK_S							0x53
#define QIK_T							0x54
#define QIK_U							0x55
#define QIK_V							0x56
#define QIK_W							0x57
#define QIK_X							0x58
#define QIK_Y							0x59
#define QIK_Z							0x5A
#define QIK_LWIN						0x5B	// LMETA
#define QIK_RWIN						0x5C	// RMETA
#define QIK_APPS						0x5D
#define QIK_SLEEP						0x5F
#define QIK_NUMPAD0						0x60
#define QIK_NUMPAD1						0x61
#define QIK_NUMPAD2						0x62
#define QIK_NUMPAD3						0x63
#define QIK_NUMPAD4						0x64
#define QIK_NUMPAD5						0x65
#define QIK_NUMPAD6						0x66
#define QIK_NUMPAD7						0x67
#define QIK_NUMPAD8						0x68
#define QIK_NUMPAD9						0x69
#define QIK_MULTIPLY					0x6A
#define QIK_ADD							0x6B
#define QIK_SEPARATOR       			0x6C
#define QIK_SUBTRACT        			0x6D
#define QIK_DECIMAL						0x6E
#define QIK_DIVIDE						0x6F
#define QIK_F1							0x70
#define QIK_F2							0x71
#define QIK_F3							0x72
#define QIK_F4							0x73
#define QIK_F5							0x74
#define QIK_F6							0x75
#define QIK_F7							0x76
#define QIK_F8							0x77
#define QIK_F9							0x78
#define QIK_F10							0x79
#define QIK_F11							0x7A
#define QIK_F12							0x7B
#define QIK_F13							0x7C
#define QIK_F14							0x7D
#define QIK_F15							0x7E
#define QIK_F16							0x7F
#define QIK_F17							0x80
#define QIK_F18							0x81
#define QIK_F19							0x82
#define QIK_F20							0x83
#define QIK_F21							0x84
#define QIK_F22							0x85
#define QIK_F23							0x86
#define QIK_F24							0x87
#define QIK_NUMLOCK						0x90
#define QIK_SCROLL						0x91
#define QIK_LSHIFT						0xA0
#define QIK_RSHIFT						0xA1
#define QIK_LCTRL		     			0xA2	// LCONTROL
#define QIK_RCTRL	        			0xA3	// RCONTROL	
#define QIK_LALT						0xA4	// LMENU
#define QIK_RALT						0xA5	// RMENU
#define QIK_COMMA						0xBC	// , <
#define QIK_PLUS						0xBB
#define QIK_MINUS						0xBD
#define QIK_PERIOD						0xBE	// . >
#define QIK_SLASH						0xBF	// / ?
#define QIK_TILT						0xC0	// ~ `
#define QIK_ATTN						0xF6
#define QIK_CRSEL						0xF7
#define QIK_EXSEL						0xF8
#define QIK_EREOF						0xF9
#define QIK_PLAY						0xFA
#define QIK_ZOOM						0xFB
#define QIK_PA1							0xFD
#define QIK_OEM_CLEAR					0xFE
#define QIK_MAX_VALUE					0xFF

#define QIS_SHIFT						0x0001
#define QIS_CTRL						0x0002
#define QIS_ALT							0x0004
#define QIS_CAPS						0x1000
#define QIS_SCRL						0x2000
#define QIS_NUM							0x4000


//////////////////////////////////////////////////////////////////////////
// controller key
#define QIC_UP							1
#define QIC_DOWN						2
#define QIC_LEFT						3
#define QIC_RIGHT						4
#define QIC_START						5
#define QIC_BACK						6
#define QIC_THB_LEFT					7
#define QIC_THB_RIGHT					8
#define QIC_SHD_LEFT					9
#define QIC_SHD_RIGHT					10
#define QIC_A							11
#define QIC_B							12
#define QIC_X							13
#define QIC_Y							14
#define QIC_PREV						QIC_BACK
#define QIC_NEXT						QIC_START

#define QICM_UP							0x0001
#define QICM_DOWN						0x0002
#define QICM_LEFT						0x0004
#define QICM_RIGHT						0x0008
#define QICM_START						0x0010
#define QICM_BACK						0x0020
#define QICM_THB_LEFT					0x0040
#define QICM_THB_RIGHT					0x0080
#define QICM_SHD_LEFT					0x0100
#define QICM_SHD_RIGHT					0x0200
#define QICM_A							0x1000
#define QICM_B							0x2000
#define QICM_X							0x4000
#define QICM_Y							0x8000
#define QICM_PREV						QICM_BACK
#define QICM_NEXT						QICM_START


//////////////////////////////////////////////////////////////////////////
// touch
#define QITT_BEGIN						1
#define QITT_MOVE						2
#define QITT_END						3


//////////////////////////////////////////////////////////////////////////
// input info

//
struct qgUimKey
{
	uint32_t			key[256];
	uint16_t			state;
};

//
struct qgUimMouse
{
	uint16_t			state;
	int16_t				wheel;
	qnPoint				point;
	qnPoint				last;

	struct
	{
		uint32_t		tick;
		int32_t			btn;
		qnPoint			loc;

		int32_t			lim_move;
		uint32_t		lim_tick;
	}					mc;
};

//
struct qgUimControllerInput
{
	int16_t				btn;
	qnPoint				trg;
	qnVec2				lthb;
	qnVec2				rthb;
};

//
struct qgUimControllerInfo
{
	int					flags;

	int					type : 16;
	int					extend : 16;

	int					battery_type : 8;
	int					battery_level : 8;

	int					headset_type : 8;
	int					headset_level : 8;
};

// 
struct qgUimControllerVibration
{
	uint16_t			left;
	uint16_t			right;
};

//
QNAPI const char* qikstr(uint8_t key);
QNAPI const char* qimstr(uint8_t button);
QNAPI const char* qicstr(uint16_t button);
