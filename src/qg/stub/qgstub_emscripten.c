//
// qgstub_emscripten.c - EMSCRIPTEN 스터브
// 2024-1-9 by kim
//

#include "pch.h"
#if defined __EMSCRIPTEN__ && !defined USE_SDL2
#include "qs_qn.h"
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qg/qg_stub.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#ifdef _QN_EMSCRIPTEN_
#include <emscripten/html5.h>
#endif

//
static void emscripten_register_event_handler(void);
static void emscripten_unregister_event_handler(void);

// EMSCRIPTEN 스터브
typedef struct EmscriptenStub
{
	StubBase			base;

	char*				canvas_name;

	QmPoint				mouse_residual;

	bool				external_sizing;
	bool				lock_pointer;
} EmscriptenStub;

//
EmscriptenStub emnStub;

//
bool stub_system_open(const char* title, const int display, const int width, const int height, QgFlag flags)
{
	qn_zero_1(&emnStub);

	//
	stub_initialize((StubBase*)&emnStub, flags);

	//
	QmSize size;
	emscripten_get_screen_size(&size.Width, &size.Height);
	emnStub.base.window_bound = qm_rect_set_pos_size(qm_point(0, 0), size);
	emnStub.base.client_size = size;

	emnStub.canvas_name = qn_strdup("#canvas");
	emscripten_set_canvas_element_size(emnStub.canvas_name, 1, 1);
	double css_width, css_height;
	emscripten_get_element_css_size(emnStub.canvas_name, &css_width, &css_height);
	emnStub.external_sizing = (int)floor(css_width) != 1 || (int)floor(css_height) != 1;

	if (QN_TMASK(flags, QGFLAG_RESIZABLE) && emnStub.external_sizing)
	{
		emnStub.base.client_size = qm_size((int)css_width, (int)css_height);
		stub_event_on_window_event(QGWEV_SIZED, (int)css_width, (int)css_height);
	}
	emscripten_set_canvas_element_size(emnStub.canvas_name, emnStub.base.client_size.Width, emnStub.base.client_size.Height);

	// 값설정
	stub_system_set_title(title);
	stub_system_update_bound();

	// 핸들러
	emscripten_register_event_handler();

	return true;
}

//
void stub_system_finalize(void)
{
	emscripten_unregister_event_handler();

	emscripten_set_canvas_element_size(emnStub.canvas_name, 0, 0);
	qn_free(emnStub.canvas_name);
}

//
bool stub_system_poll(void)
{
	return true;
}

//
void stub_system_disable_acs(const bool enable)
{
	QN_DUMMY(enable);
}

//
void stub_system_disable_scr_save(const bool enable)
{
	QN_DUMMY(enable);
}

//
void stub_system_enable_drop(const bool enable)
{
	QN_DUMMY(enable);
}

//
void stub_system_set_title(const char* title)
{
	emscripten_set_window_title(title && *title ? title : "QS");
}

//
void stub_system_update_bound(void)
{
	QmSize size;
	emscripten_get_canvas_element_size(emnStub.canvas_name, &size.Width, &size.Height);
	emnStub.base.window_bound = qm_rect_set_pos_size(qm_point(0, 0), size);
	emnStub.base.client_size = size;
}

//
void stub_system_focus(void)
{
}

//
void* stub_system_get_window(void)
{
	return NULL;
}

//
void* stub_system_get_display(void)
{
	return EGL_DEFAULT_DISPLAY;
}

// 언로드 이벤트
static const char* handler_before_unload(int eventType, const void* reserved, void* userData)
{
	qg_exit_loop();
	return "";	// 여기 뭐 넣으면 확인창 뜬다
}

// 포커스 이벤트
static EM_BOOL handler_focus_blur(int eventType, const EmscriptenFocusEvent* focusEvent, void* userData)
{
	if (eventType == EMSCRIPTEN_EVENT_BLUR)
		stub_event_on_reset_keys();
	stub_event_on_window_event(eventType == EMSCRIPTEN_EVENT_FOCUS ? QGWEV_FOCUS : QGWEV_LOSTFOCUS, 0, 0);
	return true;
}

// 풀스크린
static EM_BOOL handler_fullscreen_change(int eventType, const EmscriptenFullscreenChangeEvent *fullscreenChangeEvent, void *userData)
{
	// 아직 준비가 안됐다
	return false;
}

// 크기 변경
static EM_BOOL handler_resize(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
{
	if (QN_TMASK(emnStub.base.stats, QGSSTT_FULLSCREEN) ||
		QN_TMASK(emnStub.base.flags, QGFLAG_RESIZABLE) == false)
		return false;
	double width = emnStub.base.client_size.Width;
	double height = emnStub.base.client_size.Height;
	if (emnStub.external_sizing)
		emscripten_get_element_css_size(emnStub.canvas_name, &width, &height);
	emscripten_set_canvas_element_size(emnStub.canvas_name, (int)width, (int)height);
	stub_event_on_window_event(QGWEV_SIZED, (int)width, (int)height);
	return false;
}

// 보기나 안보이나
static EM_BOOL handler_visibility_change(int eventType, const EmscriptenVisibilityChangeEvent *visibilityChangeEvent, void* userData)
{
	stub_event_on_window_event(visibilityChangeEvent->hidden ? QGWEV_HIDE : QGWEV_SHOW, 0, 0);
	return false;
}

// 마우스 홀드 변경
static EM_BOOL handler_pointer_lock_change(int eventType, const EmscriptenPointerlockChangeEvent* pointerlockChangeEvent, void* userdata)
{
	emnStub.lock_pointer = pointerlockChangeEvent->isActive;
	return false;
}

// 마우스 들어왓다 나갓다
static EM_BOOL handler_mouse_enter_leave(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
	if (emnStub.lock_pointer == false)
		stub_event_on_mouse_move(mouseEvent->targetX, mouseEvent->targetY);
	return false;	// 마우스 포커스 이벤트는 우리는 없다
}

// 마우스 이동
static EM_BOOL handler_mouse_move(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
	QmPoint pt;
	if (emnStub.lock_pointer == false)
		pt = qm_point(mouseEvent->targetX, mouseEvent->targetY);
	else
	{
		emnStub.mouse_residual.X += mouseEvent->movementX;
		emnStub.mouse_residual.Y += mouseEvent->movementY;
		pt = emnStub.mouse_residual;
		emnStub.mouse_residual = qm_sub(emnStub.mouse_residual, pt);
	}
	stub_event_on_mouse_move(pt.X, pt.Y);
	return false;
}

// 마우스 버튼
static EM_BOOL handler_mouse_button(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
	QimButton button =
		mouseEvent->button == 0 ? QIM_LEFT :
		mouseEvent->button == 1 ? QIM_MIDDLE :
		mouseEvent->button == 2 ? QIM_RIGHT : QIM_NONE;
	qn_val_if_ok(button == QIM_NONE, 0);

	bool prevent_event = false;
	if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
	{
		if (emnStub.lock_pointer == false)
			emscripten_request_pointerlock(emnStub.canvas_name, 0);
		stub_event_on_mouse_button(button, true);
	}
	else
	{
		prevent_event = true;
		stub_event_on_mouse_button(button, false);
	}

	double width, height;
	emscripten_get_element_css_size(emnStub.canvas_name, &width, &height);
	if (mouseEvent->targetX < 0 || mouseEvent->targetX >= (int)width ||
		mouseEvent->targetY < 0 || mouseEvent->targetY >= (int)height)
		return false;

	return prevent_event;
}

// 마우스 휠
static EM_BOOL handler_mouse_wheel(int eventType, const EmscriptenWheelEvent* wheelEvent, void* userData)
{
	float wheel = wheelEvent->deltaY;
	switch (wheelEvent->deltaMode)
	{
		case DOM_DELTA_PIXEL:
			wheel /= 100.0f;
			break;
		case DOM_DELTA_LINE:
			wheel /= 3;
			break;
		case DOM_DELTA_PAGE:
			wheel *= 80;
			break;
	}
	stub_event_on_mouse_wheel(0.0f, wheel, false);
	return true;
}

// 키보드
static EM_BOOL handler_keyboard(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
	const QikKey key = (QikKey)keyEvent->keyCode;
	const bool usable = qg_qik_usable(key);
	if (usable)
		stub_event_on_keyboard(key, eventType == EMSCRIPTEN_EVENT_KEYDOWN);
	bool nav_key =
		key == QIK_BS || key == QIK_TAB ||
		key == QIK_LEFT || key == QIK_UP || key == QIK_RIGHT || key == QIK_DOWN ||
		(key >= QIK_F1 && key <= QIK_F24);
	if (eventType == EMSCRIPTEN_EVENT_KEYDOWN && !nav_key)
		return false;
	return true;
}

// 키보드 텍스트
static EM_BOOL handler_key_press(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
	char u8[7];
	if (qn_u32ucb((uchar4)keyEvent->charCode, u8) > 0)
		stub_event_on_text(u8);
	return true;
}

// 이벤트 등록
static void emscripten_register_event_handler(void)
{
	const char* name = emnStub.canvas_name;

	emscripten_set_beforeunload_callback(NULL, handler_before_unload);

	emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, handler_focus_blur);
	emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, handler_focus_blur);
	emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, handler_fullscreen_change);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, handler_resize);
	emscripten_set_visibilitychange_callback(NULL, 0, handler_visibility_change);

	emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, handler_pointer_lock_change);
	emscripten_set_mouseenter_callback(name, NULL, 0, handler_mouse_enter_leave);
	emscripten_set_mouseleave_callback(name, NULL, 0, handler_mouse_enter_leave);
	emscripten_set_mousemove_callback(name, NULL, 0, handler_mouse_move);
	emscripten_set_mousedown_callback(name, NULL, 0, handler_mouse_button);
	emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, handler_mouse_button);
	emscripten_set_wheel_callback(name, NULL, 0, handler_mouse_wheel);

	emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, handler_keyboard);
	emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, handler_keyboard);
	emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, handler_key_press);
}

// 이벤트 끝냄
static void emscripten_unregister_event_handler(void)
{
	const char* name = emnStub.canvas_name;

	emscripten_set_beforeunload_callback(NULL, NULL);

	emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
	emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
	emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, NULL);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);

	emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, NULL);
	emscripten_set_mouseenter_callback(name, NULL, 0, NULL);
	emscripten_set_mouseleave_callback(name, NULL, 0, NULL);
	emscripten_set_mousemove_callback(name, NULL, 0, NULL);
	emscripten_set_mousedown_callback(name, NULL, 0, NULL);
	emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, NULL);
	emscripten_set_wheel_callback(name, NULL, 0, NULL);

	emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
	emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
	emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
}

#endif // __EMSCRIPTEN__ && !USE_SDL2
