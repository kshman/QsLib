//
// QsLib [KMC Helper]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: UNLICENSE
//

#pragma once
#define __QS_KMC__

//////////////////////////////////////////////////////////////////////////
// 키보드

/// @brief 키보드 입력 키 정의
typedef enum QikKey
{
	QIK_NONE = 0,
	QIK_BS = 0x08,											// BACKSPACE
	QIK_TAB = 0x09,
	QIK_CLEAR = 0x0C,										// NUM5 without NUMLOCK
	QIK_RETURN = 0x0D,
	QIK_PAUSE = 0x13,
	QIK_CAPSLOCK = 0x14,
	QIK_HANGUL = 0x15,
	QIK_KANA = 0x15,
	QIK_JUNJA = 0x17,
	QIK_FINAL = 0x18,
	QIK_HANJA = 0x19,
	QIK_KANJI = 0x19,
	QIK_ESC = 0x1B,
	QIK_SPACE = 0x20,
	QIK_PGUP = 0x21,										// PRIOR PAGE
	QIK_PGDN = 0x22,										// NEXT PAGE
	QIK_END = 0x23,
	QIK_HOME = 0x24,
	QIK_LEFT = 0x25,
	QIK_UP = 0x26,
	QIK_RIGHT = 0x27,
	QIK_DOWN = 0x28,
	QIK_PRTSCR = 0x2C,										// SNAPSHOT
	QIK_INS = 0x2D,											// INSERT
	QIK_DEL = 0x2E,											// DELETE
	QIK_HELP = 0x2F,
	QIK_0 = 0x30,
	QIK_1 = 0x31,
	QIK_2 = 0x32,
	QIK_3 = 0x33,
	QIK_4 = 0x34,
	QIK_5 = 0x35,
	QIK_6 = 0x36,
	QIK_7 = 0x37,
	QIK_8 = 0x38,
	QIK_9 = 0x39,
	QIK_A = 0x41,
	QIK_B = 0x42,
	QIK_C = 0x43,
	QIK_D = 0x44,
	QIK_E = 0x45,
	QIK_F = 0x46,
	QIK_G = 0x47,
	QIK_H = 0x48,
	QIK_I = 0x49,											// Alphabet I / NOLINT
	QIK_J = 0x4A,
	QIK_K = 0x4B,
	QIK_L = 0x4C,
	QIK_M = 0x4D,
	QIK_N = 0x4E,
	QIK_O = 0x4F,											// Alphabet O / NOLINT
	QIK_P = 0x50,
	QIK_Q = 0x51,
	QIK_R = 0x52,
	QIK_S = 0x53,
	QIK_T = 0x54,
	QIK_U = 0x55,
	QIK_V = 0x56,
	QIK_W = 0x57,
	QIK_X = 0x58,
	QIK_Y = 0x59,
	QIK_Z = 0x5A,
	QIK_LWIN = 0x5B,										// LEFT WIN / META / COMMAND
	QIK_RWIN = 0x5C,										// RIGHT WIN / META / COMMAND
	QIK_APPS = 0x5D,
	QIK_SLEEP = 0x5F,
	QIK_NUM_0 = 0x60,
	QIK_NUM_1 = 0x61,
	QIK_NUM_2 = 0x62,
	QIK_NUM_3 = 0x63,
	QIK_NUM_4 = 0x64,
	QIK_NUM_5 = 0x65,
	QIK_NUM_6 = 0x66,
	QIK_NUM_7 = 0x67,
	QIK_NUM_8 = 0x68,
	QIK_NUM_9 = 0x69,
	QIK_NUM_MUL = 0x6A,										// *
	QIK_NUM_ADD = 0x6B,										// +
	QIK_NUM_SEP = 0x6C,										// ,
	QIK_NUM_SUB = 0x6D,										// -
	QIK_NUM_DOT = 0x6E,										// .
	QIK_NUM_DIV = 0x6F,										// /
	QIK_F1 = 0x70,
	QIK_F2 = 0x71,
	QIK_F3 = 0x72,
	QIK_F4 = 0x73,
	QIK_F5 = 0x74,
	QIK_F6 = 0x75,
	QIK_F7 = 0x76,
	QIK_F8 = 0x77,
	QIK_F9 = 0x78,
	QIK_F10 = 0x79,
	QIK_F11 = 0x7A,
	QIK_F12 = 0x7B,
	QIK_F13 = 0x7C,
	QIK_F14 = 0x7D,
	QIK_F15 = 0x7E,
	QIK_F16 = 0x7F,
	QIK_F17 = 0x80,
	QIK_F18 = 0x81,
	QIK_F19 = 0x82,
	QIK_F20 = 0x83,
	QIK_F21 = 0x84,
	QIK_F22 = 0x85,
	QIK_F23 = 0x86,
	QIK_F24 = 0x87,
	QIK_NUMLOCK = 0x90,
	QIK_SCRL = 0x91,										// SCROLL LOCK
	QIK_LSHIFT = 0xA0,										// LEFT SHIFT
	QIK_RSHIFT = 0xA1,										// RIGHT SHIFT
	QIK_LCTRL = 0xA2,										// LEFT CONTROL
	QIK_RCTRL = 0xA3,										// RIGHT CONTROL / KAIOS KEYPAD HASH
	QIK_LALT = 0xA4,										// LEFT ALT / MENU / OPTION
	QIK_RALT = 0xA5,										// RIGHT ALT / MENU / OPTION
	QIK_NAV_BACK = 0xA6,									// NAVIGATE BACK
	QIK_NAV_FORWARD = 0xA7,									// NAVIGATE FORWARD
	QIK_NAV_REFRESH = 0xA8,									// NAVIGATE REFRESH / RELOAD
	QIK_NAV_STOP = 0xA9,									// NAVIGATE STOP
	QIK_NAV_SEARCH = 0xAA,									// NAVIGATE SEARCH / KAIOS KEYPAD MULTIPLY
	QIK_NAV_FAVORITES = 0xAB,								// NAVIGATE FAVORITES
	QIK_NAV_HOME = 0xAC,									// NAVIGATE HOME
	QIK_VOL_MUTE = 0xAD,									// VOLUME MUTE
	QIK_VOL_DOWN = 0xAE,									// VOLUME DOWN
	QIK_VOL_UP = 0xAF,										// VOLUME UP
	QIK_MEDIA_NEXT = 0xB0,									// MEDIA NEXT
	QIK_MEDIA_PREV = 0xB1,									// MEDIA PREVIOUS
	QIK_MEDIA_STOP = 0xB2,									// MEDIA STOP
	QIK_MEDIA_PLAY = 0xB3,									// MEDIA PLAY / PAUSE
	QIK_SEMI = 0xBA,										// ;:
	QIK_ADD = 0xBB,											// =+
	QIK_COMMA = 0xBC,										// ,<
	QIK_SUB = 0xBD,											// -_
	QIK_DOT = 0xBE,											// .>
	QIK_SLASH = 0xBF,										// /?
	QIK_TILT = 0xC0,										// `~
	QIK_LBR = 0xDB,											// [{
	QIK_BACKSLASH = 0xDC,									// \|
	QIK_RBR = 0xDD,											// ]}
	QIK_QUOTE = 0xDE,										// '"
	QIK_MAX_VALUE = 0xFF + 1,
} QikKey;

/// @brief 상태키 마스크
typedef enum QikMask
{
	QIKM_NONE = 0,
	QIKM_SHIFT = QN_BIT(0),
	QIKM_CTRL = QN_BIT(1),
	QIKM_ALT = QN_BIT(2),
	QIKM_CAPS = QN_BIT(12),
	QIKM_SCRL = QN_BIT(13),
	QIKM_NUM = QN_BIT(14),
	QIKM_WIN = QN_BIT(15),
} QikMask;

static_assert(sizeof(QikMask) == sizeof(int), "QikMask size not equal to int");


//////////////////////////////////////////////////////////////////////////
// mouse button

/// @brief 마우스 버튼
typedef enum QimButton
{
	QIM_NONE = 0,
	QIM_LEFT = 1,
	QIM_RIGHT = 2,
	QIM_MIDDLE = 3,
	QIM_X1 = 4,
	QIM_X2 = 5,
	QIM_MAX_VALUE,
} QimButton;

/// @brief 마스 버튼 마스크
typedef enum QimMask
{
	QIMM_LEFT = QN_BIT(QIM_LEFT),
	QIMM_RIGHT = QN_BIT(QIM_RIGHT),
	QIMM_MIDDLE = QN_BIT(QIM_MIDDLE),
	QIMM_X1 = QN_BIT(QIM_X1),
	QIMM_X2 = QN_BIT(QIM_X2),

	QIMM_WHEELUP = 0x0100,
	QIMM_WHEELDOWN = 0x0200,
	QIMM_DOUBLE = 0x1000,
} QimMask;

/// @brief 마우스 눌림 상태 추적
typedef enum QimTrack
{
	QIMT_MOVE = 0,
	QIMT_DOWN = 1,
	QIMT_UP = 2,
} QimTrack;

/// @brief 터치 상태 추적
typedef enum QimTouch
{
	QITT_BEGIN = 1,
	QITT_MOVE = 2,
	QITT_END = 3,
} QimTouch;

static_assert(sizeof(QimMask) == sizeof(int), "QimMask size not equal to int");


//////////////////////////////////////////////////////////////////////////
// controller key

/// @brief 컨트롤러 버튼
typedef enum QicButton
{
	QIC_NONE = 0,
	QIC_UP = 1,
	QIC_DOWN = 2,
	QIC_LEFT = 3,
	QIC_RIGHT = 4,
	QIC_START = 5,
	QIC_BACK = 6,
	QIC_THB_LEFT = 7,
	QIC_THB_RIGHT = 8,
	QIC_SHD_LEFT = 9,
	QIC_SHD_RIGHT = 10,
	QIC_A = 11,
	QIC_B = 12,
	QIC_X = 13,
	QIC_Y = 14,
} QicButton;

/// @brief 컨트롤러 마스크
typedef enum QicMask
{
	QICM_UP = QN_BIT(QIC_UP),
	QICM_DOWN = QN_BIT(QIC_DOWN),
	QICM_LEFT = QN_BIT(QIC_LEFT),
	QICM_RIGHT = QN_BIT(QIC_RIGHT),
	QICM_START = QN_BIT(QIC_START),
	QICM_BACK = QN_BIT(QIC_BACK),
	QICM_THB_LEFT = QN_BIT(QIC_THB_LEFT),
	QICM_THB_RIGHT = QN_BIT(QIC_THB_RIGHT),
	QICM_SHD_LEFT = QN_BIT(QIC_SHD_LEFT),
	QICM_SHD_RIGHT = QN_BIT(QIC_SHD_RIGHT),
	QICM_A = QN_BIT(QIC_A),
	QICM_B = QN_BIT(QIC_B),
	QICM_X = QN_BIT(QIC_X),
	QICM_Y = QN_BIT(QIC_Y),
} QicMask;

static_assert(sizeof(QicMask) == sizeof(int), "QicMask size not equal to int");


//////////////////////////////////////////////////////////////////////////
// function

/// @brief 사용가능한 키인가
/// @param key QikKey
/// @return 사용가능하면 참
QSAPI bool qg_qik_usable(QikKey key);

/// @brief QikKey를 문자열로
/// @param key QikKey
/// @return QikKey의 이름
QSAPI const char* qg_qik_str(QikKey key);

/// @brief QimButton를 문자열로
/// @param button QimButton
/// @return QimButton의 이름
QSAPI const char* qg_qim_str(QimButton button);

/// @brief QicButton를 문자열로
/// @param button QicButton
/// @return QicButton의 이름
QSAPI const char* qg_qic_str(QicButton button);
