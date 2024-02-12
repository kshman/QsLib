//
// qgstub_emscripten.c - EMSCRIPTEN 스터브
// 2024-1-9 by kim
//

#include "pch.h"
#ifdef _QN_EMSCRIPTEN_
#include "qs_qg.h"
#include "qg/qg_stub.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <emscripten/html5.h>

//
static void emscripten_register_event_handler(void);
static void emscripten_unregister_event_handler(void);

// EMSCRIPTEN 스터브
typedef struct EmscriptenStub
{
	StubBase			base;

	QmSize				window_size;		// 전체화면이 아닐때 크기. 기억할 필요 있나. 자바스크립트가 기억하는데
	QmPoint				mouse_residual;

	bool				external_sizing;
	bool				lock_pointer;
} EmscriptenStub;

//
static const char* emCanvas = "#canvas";
static EmscriptenStub emStub;

//
bool stub_system_open(const char* title, int display, int width, int height, QgFlag flags, QgFeature features)
{
	qn_zero_1(&emStub);

	//
	stub_initialize((StubBase*)&emStub, flags);

	// 가짜 모니터
	QmSize scrsize;
	emscripten_get_screen_size(&scrsize.Width, &scrsize.Height);
	QgUdevMonitor* monitor = qn_alloc_zero_1(QgUdevMonitor);
	qn_strcpy(monitor->name, "webbrowser canvas");
	monitor->width = scrsize.Width;
	monitor->height = scrsize.Height;
	stub_event_on_monitor(monitor, true, true, false);

	// 웹브라우저는 윈도우 생성이 없다. 바로 크기 검사
	emscripten_set_canvas_element_size(emCanvas, 1, 1);
	double css_width, css_height;
	emscripten_get_element_css_size(emCanvas, &css_width, &css_height);
	emStub.external_sizing = (int)floorf(css_width) != 1 || (int)floorf(css_height) != 1;

	if (QN_TMASK(flags, QGFLAG_RESIZE) && emStub.external_sizing)
	{
		// 반드시 캔바스 CSS로 "aspect-ratio: 16 / 9"와 같이 종횡비 지정할 것!
		// 안하면 auto값으로 width == height 가 되버려서 난리남
		width = (int)css_width;
		height = (int)css_height;
	}
	else if (width < 128 || height < 128)
	{
		const int browser_width = MAIN_THREAD_EM_ASM_INT({ return window.innerWidth; });
		if (browser_width > 1300)
			width = 1280;
		else if (browser_width > 1000)
			width = 960;
		else if (browser_width > 800)
			width = 720;
		else
			width = (int)((float)browser_width * 0.95f);
		height = (int)((float)width * 0.5625f);
	}

	emscripten_set_canvas_element_size(emCanvas, width, height);
	stub_event_on_window_event(QGWEV_SIZED, width, height);
	stub_event_on_layout(false);
	emStub.window_size = qm_size(width, height);

	if (QN_TMASK(flags, QGFLAG_FULLSCREEN))
	{
		stub_system_fullscreen(true);
		QN_SMASK(emStub.base.stats, QGSST_FULLSCREEN, true);
	}

	// 핸들러
	emscripten_register_event_handler();

	// 값설정
	stub_system_set_title(title);
	stub_system_update_bound();

	// 렌더러가 없다면 표시하고, 있으면 렌더러 만들고 stub_system_actuate()를 호출할 것
	if (QN_TMASK(features, 0xFF000000) == false)
		stub_system_actuate();

	return true;
}

//
void stub_system_finalize(void)
{
	emscripten_unregister_event_handler();

	emscripten_set_canvas_element_size(emCanvas, 0, 0);
}

//
void stub_system_actuate(void)
{
	if (QN_TMASK(emStub.base.flags, QGFLAG_FULLSCREEN) && QN_TMASK(emStub.base.stats, QGSST_FULLSCREEN) == false)
	{
		// 풀스크린
		QN_SMASK(emStub.base.stats, QGSST_FULLSCREEN, true);
		stub_system_fullscreen(true);
	}
}

//
bool stub_system_poll(void)
{
	if (emscripten_has_asyncify())
		emscripten_sleep(10);
	return true;
}

//
bool stub_system_disable_acs(const bool enable)
{
	return false;
}

//
bool stub_system_disable_scr_save(const bool enable)
{
	return false;
}

//
bool stub_system_enable_drop(const bool enable)
{
	return false;
}

//
bool stub_system_relative_mouse(bool enable)
{
	return enable;
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
	emscripten_get_canvas_element_size(emCanvas, &size.Width, &size.Height);
	emStub.base.bound = qm_rect_size(0, 0, size.Width, size.Height);
	emStub.base.client_size = size;
}

//
void stub_system_focus(void)
{
}

//
void stub_system_aspect(void)
{
}

//
void stub_system_fullscreen(bool fullscreen)
{
	const bool now = EM_ASM_INT({ if (document.fullscreenElement) return 1; }, 0);
	const bool need = now ?
		fullscreen ? false : true :
		fullscreen ? true : false;

	if (fullscreen)
	{
		// Module.requestFullscreen 인수 (lockPointer, resizeCanvas, vrDevice)
		EM_ASM(setTimeout(function()
		{
			Module.requestFullscreen(false, true);	
		}, 100); );
	}
	else
	{
		// 아 짧다
		EM_ASM(document.exitFullscreen(););
	}
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

//
const char* stub_system_get_canvas(void)
{
	return emCanvas;
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
	return EM_TRUE;
}

// 풀스크린
static EM_BOOL handler_fullscreen_change(int eventType, const EmscriptenFullscreenChangeEvent *fullscreenChangeEvent, void *userData)
{
	QN_SMASK(emStub.base.stats, QGSST_FULLSCREEN, fullscreenChangeEvent->isFullscreen);
	return EM_TRUE;
}

// 크기 변경
static EM_BOOL handler_resize(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
{
	if (QN_TMASK(emStub.base.stats, QGSST_FULLSCREEN | QGFLAG_RESIZE) == false)
		return EM_FALSE;

	double width = emStub.base.client_size.Width;
	double height = emStub.base.client_size.Height;
	if (emStub.external_sizing)
		emscripten_get_element_css_size(emCanvas, &width, &height);
	emscripten_set_canvas_element_size(emCanvas, (int)width, (int)height);

	stub_event_on_window_event(QGWEV_SIZED, (int)width, (int)height);
	stub_event_on_layout(false);

	return EM_FALSE;
}

// 보기나 안보이나
static EM_BOOL handler_visibility_change(int eventType, const EmscriptenVisibilityChangeEvent *visibilityChangeEvent, void* userData)
{
	stub_event_on_window_event(visibilityChangeEvent->hidden ? QGWEV_HIDE : QGWEV_SHOW, 0, 0);
	return EM_FALSE;
}

// 마우스 홀드 변경
static EM_BOOL handler_pointer_lock_change(int eventType, const EmscriptenPointerlockChangeEvent* pointerlockChangeEvent, void* userdata)
{
	emStub.lock_pointer = pointerlockChangeEvent->isActive;
	return EM_FALSE;
}

// 마우스 들어왓다 나갓다
static EM_BOOL handler_mouse_enter_leave(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
	if (emStub.lock_pointer == false)
	{
		double width, height;
		emscripten_get_element_css_size(emCanvas, &width, &height);
		double x = mouseEvent->targetX * (emStub.base.client_size.Width / width);
		double y = mouseEvent->targetY * (emStub.base.client_size.Height / height);
		stub_event_on_mouse_move((int)x, (int)y);
	}

	if (QN_TMASK(emStub.base.flags, QGFLAG_FOCUS))
		stub_event_on_focus(eventType == EMSCRIPTEN_EVENT_MOUSEENTER);

	return EM_TRUE;
}

// 마우스 이동
static EM_BOOL handler_mouse_move(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
	QmPoint pt;
	if (emStub.lock_pointer == false)
		pt = qm_point(mouseEvent->targetX, mouseEvent->targetY);
	else
	{
		emStub.mouse_residual.X += mouseEvent->movementX;
		emStub.mouse_residual.Y += mouseEvent->movementY;
		pt = emStub.mouse_residual;
		emStub.mouse_residual = qm_sub(emStub.mouse_residual, pt);
	}
	stub_event_on_mouse_move(pt.X, pt.Y);
	return EM_FALSE;
}

// 마우스 버튼
static EM_BOOL handler_mouse_button(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
	QimButton button =
		mouseEvent->button == 0 ? QIM_LEFT :
		mouseEvent->button == 1 ? QIM_MIDDLE :
		mouseEvent->button == 2 ? QIM_RIGHT : QIM_NONE;
	qn_return_on_ok(button == QIM_NONE, EM_FALSE);

	bool down;
	EM_BOOL prevent_event;
	if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
	{
		if (QN_TMASK(emStub.base.features, QGFEATURE_RELATIVE_MOUSE) && emStub.lock_pointer == false)
			emscripten_request_pointerlock(emCanvas, 0);
		down = true;
		prevent_event = EM_FALSE;
	}
	else
	{
		down = false;
		prevent_event = EM_TRUE;
	}
	stub_event_on_mouse_button(button, down);

	double width, height;
	emscripten_get_element_css_size(emCanvas, &width, &height);
	if (mouseEvent->targetX < 0 || mouseEvent->targetX >= (int)width ||
		mouseEvent->targetY < 0 || mouseEvent->targetY >= (int)height)
		return EM_FALSE;

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
	return EM_TRUE;
}

// 키보드
static EM_BOOL handler_keyboard(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
	QikKey key = (QikKey)keyEvent->keyCode;	// 다행히 키가 호환된다

	if (qg_qik_usable(key) == false)
	{
		if (qn_strncmp(keyEvent->key, "SoftLeft", 9) == 0)
			key = QIK_NAV_FORWARD;
		else if (qn_strncmp(keyEvent->key, "SoftRight", 10) == 0)
			key = QIK_NAV_BACK;
		else
			return EM_FALSE;
	}
	else
	{
		switch (keyEvent->location)
		{
			case DOM_KEY_LOCATION_RIGHT:
				switch (key)
				{
					case QIK_LSHIFT:	key = QIK_RSHIFT; break;
					case QIK_LCTRL:		key = QIK_RCTRL; break;
					case QIK_LALT:		key = QIK_RALT; break;
					case QIK_LWIN:		key = QIK_RWIN; break;
				}
				break;
			case DOM_KEY_LOCATION_NUMPAD:
				switch (key)
				{
					case QIK_0:
					case QIK_INS:		key = QIK_NUM_0; break;
					case QIK_1:
					case QIK_END:		key = QIK_NUM_1; break;
					case QIK_2:
					case QIK_DOWN:		key = QIK_NUM_2; break;
					case QIK_3:
					case QIK_PGDN:		key = QIK_NUM_3; break;
					case QIK_4:
					case QIK_LEFT:		key = QIK_NUM_4; break;
					case QIK_5:			key = QIK_NUM_5; break;
					case QIK_6:
					case QIK_RIGHT:		key = QIK_NUM_6; break;
					case QIK_7:
					case QIK_HOME:		key = QIK_NUM_7; break;
					case QIK_8:
					case QIK_UP:		key = QIK_NUM_8; break;
					case QIK_9:
					case QIK_PGUP:		key = QIK_NUM_9; break;
					case QIK_DEL:		key = QIK_NUM_DOT; break;
				}
				break;
		}
	}

	if (eventType == EMSCRIPTEN_EVENT_KEYUP)
	{
		stub_event_on_keyboard(key, false);
		return EM_TRUE;
	}

	stub_event_on_keyboard(key, true);
#ifdef _DEBUG
	if (key == QIK_F12)
		return EM_FALSE;
#endif
	bool nav_down = keyEvent->ctrlKey ||
		key == QIK_BS || key == QIK_TAB ||
		key == QIK_LEFT || key == QIK_UP || key == QIK_RIGHT || key == QIK_DOWN ||
		(key >= QIK_F1 && key <= QIK_F24);
	return !nav_down ? EM_FALSE : EM_TRUE;
}

// 키보드 텍스트
static EM_BOOL handler_key_press(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
	if (QN_TMASK(emStub.base.flags, QGFLAG_TEXT) == false)
		return EM_FALSE;

	char u8[7];
	if (qn_u32ucb((uchar4)keyEvent->charCode, u8) > 0)
		stub_event_on_text(u8);
	return EM_TRUE;
}

// 이벤트 등록
static void emscripten_register_event_handler(void)
{
	const char* name = emCanvas;

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
	const char* name = emCanvas;

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

#endif // __EMSCRIPTEN__
