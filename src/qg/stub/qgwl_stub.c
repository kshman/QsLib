//
// qgwlx_stub.c - 웨이랜드 스터브
// 2023-12-23 by kim
//

#include "pch.h"
#ifdef USE_WAYLAND
#include "qs_qn.h"
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qgwl_stub_so.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/timerfd.h>

#ifdef _DEBUG
#define DEBUG_WL_SO_TRACE
#endif

#define WL_DESTROY(var,func)	QN_STMT_BEGIN{ if (var) func(var); }QN_STMT_END

// 정적 함수 미리 선언
#pragma region 정적 함수 미리 선언
static bool nkk_display_open(void);
static void nkk_display_close(void);
#pragma endregion

#pragma region SO 처리
// SO 함수 정의
#define DEF_WL_FUNC(ret,name,args)\
	PFN_##name wayland_##name;
#include "qgwl_stub_func.h"

// SO 함수
static void* _load_so_func(QnModule* module, const char* func_name, const char* so_name)
{
	void* ret = qn_mod_func(module, func_name);
#ifdef DEBUG_WL_SO_TRACE
	static const char* current_so = NULL;
	if (current_so != so_name)
	{
		current_so = so_name;
		qn_debug_outputf(false, "WAYLAND STUB", "so: %s", so_name);
	}
	qn_debug_outputf(false, "WAYLAND STUB", "%s: %s",
		ret == NULL ? "load failed" : "loaded", func_name);
#else
	QN_DUMMY(so_name);
#endif
	return ret;
}

// SO 초기화
static bool _so_init(void)
{
	static bool loaded = false;
	qn_val_if_ok(loaded, true);
	QnModule* module;
	const char* so_name;
#define DEFL_WL_SO_BEGIN(name)\
	module = qn_mod_load(so_name = name".so", 1); if (module == NULL)\
	{ qn_debug_outputf(true, "WAYLAND STUB", "so load failed: %s", so_name); return false; } else {
#define DEFL_WL_SO_END	}
#define DEF_WL_FUNC(ret,name,args)\
	wayland_##name = (PFN_##name)_load_so_func(module, #name, so_name);\
	if (wayland_##name == NULL) {\
		qn_debug_outputf(true, "WAYLAND STUB", "%s: '%s' in '%s'", "load failed", #name, so_name);\
		return false; }
#include "qgwl_stub_func.h"
	return loaded = true;
}
#pragma endregion SO 처리


//////////////////////////////////////////////////////////////////////////

#pragma region 스터브 선언
#include "qgwl_stub.h"
// 웨이랜드 스터브 여기 있다!
WaylandStub wlStub;
#pragma endregion 스터브 선언

#pragma region 시스템 함수
//
bool stub_system_open(const char* title, const int display, const int width, const int height, QgFlag flags)
{
	qn_zero_1(&wlStub);

	//
	if (_so_init() == false)
	{
		qn_debug_outputs(true, "WAYLAND STUB", "so load failed");
		return false;
	}
	if (nkk_display_open() == false)
	{
		qn_debug_outputs(true, "WAYLAND STUB", "cannot open display");
		return false;
	}

	//
	stub_initialize((StubBase*)&wlStub, flags);

	return true;
}

//
void stub_system_finalize(void)
{
	nkk_display_close();
}

//
bool stub_system_poll(void)
{
	return QN_TMASK(wlStub.base.stats, QGSSTT_EXIT) == false;
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
	QN_DUMMY(title);
}

//
void stub_system_update_bound(void)
{
}

//
void stub_system_focus(void)
{
}
#pragma endregion 시스템 함수


//////////////////////////////////////////////////////////////////////////

#pragma region 레지스트리
#include "wayland-wayland.h"
#include "wayland-xdg-shell.h"
#include "wayland-wayland-code.h"
#include "wayland-xdg-shell-code.h"

//////////////////////////////////////////////////////////////////////////
// 디스플레이

static void keyboard_key(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t);
static void keyboard_enter(void*, wl_keyboard*, uint32_t, wl_surface*, struct wl_array*);
static void keyboard_keymap(void*, wl_keyboard*, uint32_t, int32_t, uint32_t);
static void keyboard_leave(void*, wl_keyboard*, uint32_t, wl_surface*);
static void keyboard_modifiers(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
static void keyboard_repeat_info(void*, wl_keyboard*, int32_t, int32_t);
static void pointer_axis(void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t);
static void pointer_axis_discrete(void*, wl_pointer*, uint32_t, int32_t);
static void pointer_axis_source(void*, wl_pointer*, uint32_t);
static void pointer_axis_stop(void*, wl_pointer*, uint32_t, uint32_t);
static void pointer_button(void*, wl_pointer*, uint32_t, uint32_t, uint32_t, uint32_t);
static void pointer_enter(void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t);
static void pointer_frame(void*, wl_pointer*);
static void pointer_leave(void*, wl_pointer*, uint32_t, wl_surface*);
static void pointer_motion(void*, wl_pointer*, uint32_t, wl_fixed_t, wl_fixed_t);
static void registry_global(void*, wl_registry*, uint32_t, const char*, uint32_t);
static void seat_name(void*, wl_seat*, const char*);
static void seat_capabilities(void*, wl_seat*, uint32_t);
static void xdg_shell_pong(void*, xdg_wm_base*, uint32_t);

static const struct wl_registry_listener events_registry =
{
	.global = registry_global,
	.global_remove = NULL,
};
static const struct wl_seat_listener events_seat =
{
	.capabilities = seat_capabilities,
	.name = seat_name,
};
static const struct wl_keyboard_listener events_keyboard =
{
	.keymap = keyboard_keymap,
	.enter = keyboard_enter,
	.leave = keyboard_leave,
	.key = keyboard_key,
	.modifiers = keyboard_modifiers,
#if defined WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION && WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION >= 4
	.repeat_info = keyboard_repeat_info,
#endif
};
static const struct wl_pointer_listener events_pointer =
{
	.enter = pointer_enter,
	.leave = pointer_leave,
	.motion = pointer_motion,
	.button = pointer_button,
	.axis = pointer_axis,
#if defined WL_POINTER_FRAME_SINCE_VERSION && WL_POINTER_FRAME_SINCE_VERSION >= 5
	.frame = pointer_frame,
	.axis_source = pointer_axis_source,
	.axis_stop = pointer_axis_stop,
	.axis_discrete = pointer_axis_discrete,
#endif
};
static const struct xdg_wm_base_listener events_xdg_shell =
{
	.ping = xdg_shell_pong,
};

//
bool nkk_display_open(void)
{
	if (!(wlStub.display = wl_display_connect(NULL)))
		return false;
	wlStub.registry = wl_display_get_registry(wlStub.display);
	wlStub.xkb.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	wl_registry_add_listener(wlStub.registry, &events_registry, wlStub.display);
	wl_display_roundtrip(wlStub.display);
	wl_display_roundtrip(wlStub.display);

	wlStub.repeat.fd = wl_seat_get_version(wlStub.seat) < WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION ?
		-1 : timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);

	wlStub.cursor_theme = wl_cursor_theme_load(NULL, 24, wlStub.shm);

	return true;
}

//
void nkk_display_close(void)
{
	qn_ret_if_fail(wlStub.display);

	WL_DESTROY(wlStub.pointer, wl_pointer_destroy);
	WL_DESTROY(wlStub.keyboard, wl_keyboard_destroy);

	WL_DESTROY(wlStub.seat, wl_seat_destroy);
	WL_DESTROY(wlStub.xdg_shell, xdg_wm_base_destroy);
	WL_DESTROY(wlStub.subcompositor, wl_subcompositor_destroy);
	WL_DESTROY(wlStub.compositor, wl_compositor_destroy);

	WL_DESTROY(wlStub.cursor_theme, wl_cursor_theme_destroy);
	WL_DESTROY(wlStub.registry, wl_registry_destroy);
	wl_display_disconnect(wlStub.display);

	if (wlStub.repeat.fd >= 0)
		close(wlStub.repeat.fd);
}

/* Local functions */

//
void keyboard_enter(void *data, wl_keyboard *kb, uint32_t n, wl_surface *surface, struct wl_array *keys)
{
	//nkk_display *dpy = data;
	//nkk_window *win, *tmp;
	//wl_list_for_each_safe(win, tmp, &wlStub.windows, link) {
	//	if (win->surface == surface) {
	//		wl_list_remove(&win->link);
	//		wl_list_insert(&wlStub.windows, &win->link);
	//		break;
	//	}
	//}
}

//
void keyboard_key(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	/*nkk_display *dpy = data;
	if (wlStub.xkb_state == NULL)
		return;
	nkk_window *win = wl_container_of(wlStub.windows.next, win, link);
	uint32_t keycode = key + 8;
	xkb_keysym_t sym = xkb_state_key_get_one_sym(wlStub.xkb_state, keycode);
	wlStub.key_repeat.win = win;
	wlStub.key_repeat.sym = sym;
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED && win->keydown)
	{
		gettimeofday(&wlStub.key_repeat.last, NULL);
		wlStub.key_repeat.interval = &wlStub.key_repeat.delay;
		win->keydown(win, wlStub.key_repeat.mods, sym);
	}
	else if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
	{
		if (win->keyup) win->keyup(win, wlStub.key_repeat.mods, sym);
		wlStub.key_repeat.interval = NULL;
	}*/
}

//
void keyboard_keymap(void *data, wl_keyboard *wl_keyboard, uint32_t fmt, int32_t fd, uint32_t size)
{
	/*if (fmt != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
	{
		close(fd);
		return;
	}
	nkk_display *dpy = data;
	char *map = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		const char* e = strerror(errno);
		fprintf(stderr, "아닛...%d => %s\n", errno, e);
		close(fd);
		return;
	}
	struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(wlStub.xkb_context, map,
		XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(map, size);
	close(fd);
	struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
	xkb_keymap_unref(wlStub.xkb_keymap);
	xkb_state_unref(wlStub.xkb_state);
	wlStub.xkb_keymap = xkb_keymap;
	wlStub.xkb_state = xkb_state;*/
}

//
static void keyboard_leave(void* data, wl_keyboard* wl_keyboard, uint32_t serial, wl_surface* surface)
{
}


//
void keyboard_modifiers(void *data, wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed,
	uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
	/*nkk_display *dpy = data;
	if (wlStub.xkb_keymap == NULL)
		return;
	xkb_state_update_mask(wlStub.xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
	wlStub.key_repeat.mods = 0;
	if (xkb_state_mod_name_is_active(wlStub.xkb_state, "Shift", XKB_STATE_MODS_EFFECTIVE) == 1)
		wlStub.key_repeat.mods |= NKK_MOD_SHIFT;
	if (xkb_state_mod_name_is_active(wlStub.xkb_state, "Control", XKB_STATE_MODS_EFFECTIVE) == 1)
		wlStub.key_repeat.mods |= NKK_MOD_CTRL;
	if (xkb_state_mod_name_is_active(wlStub.xkb_state, "Mod1", XKB_STATE_MODS_EFFECTIVE) == 1)
		wlStub.key_repeat.mods |= NKK_MOD_ALT;
	if (xkb_state_mod_name_is_active(wlStub.xkb_state, "Mod4", XKB_STATE_MODS_EFFECTIVE) == 1)
		wlStub.key_repeat.mods |= NKK_MOD_MOD4;*/
}

//
void keyboard_repeat_info(void *data, wl_keyboard *wl_keyboard, int32_t rate, int32_t delay)
{
	/*nkk_display *dpy = data;
	wlStub.key_repeat.delay.tv_usec = 1000 * delay;
	wlStub.key_repeat.repeat.tv_usec = 1000 * 1000 / rate;*/
}

//
void pointer_axis(void *data, wl_pointer *ptr, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	/*nkk_display *dpy = data;
	wlStub.ptr_ev.mask |= NkkPointerAxis;
	if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
		wlStub.ptr_ev.axis_x = value;
	else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		wlStub.ptr_ev.axis_y = value;*/
}

//
void pointer_axis_discrete(void *data, wl_pointer *ptr, uint32_t axis, int32_t discrete)
{
}

//
void pointer_axis_source(void *data, wl_pointer *ptr, uint32_t axis_source)
{
}

//
void pointer_axis_stop(void *data, wl_pointer *ptr, uint32_t time, uint32_t axis)
{
}

//
void pointer_button(void *data, wl_pointer *ptr, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	/*nkk_display *dpy = data;
	wlStub.ptr_ev.mask |= NkkPointerButton;
	wlStub.ptr_ev.serial = serial;
	wlStub.ptr_ev.button = button;
	wlStub.ptr_ev.state = state;*/
}

//
void pointer_enter(void *data, wl_pointer *ptr, uint32_t serial, wl_surface *surface,
	wl_fixed_t sx, wl_fixed_t sy)
{
	/*nkk_display *dpy = data;
	wlStub.ptr_ev.mask |= NkkPointerEnter;
	wlStub.ptr_ev.serial = serial;
	wlStub.ptr_ev.surface = surface;
	wlStub.ptr_ev.sx = sx,
		wlStub.ptr_ev.sy = sy;*/
}

//
void pointer_frame(void *data, wl_pointer *ptr)
{
	//nkk_display *dpy = data;
	//static wl_surface *target;
	//nkk_pointer_event *e = &wlStub.ptr_ev;

	//if (e->mask & (NkkPointerEnter | NkkPointerLeave) && e->surface)
	//	target = e->surface;
	//nkk_window *win;
	//wl_list_for_each(win, &wlStub.windows, link)
	//	if (win->surface == target) break;
	//if (e->mask & NkkPointerLeave)
	//	target = NULL;
	//if (!win) return;

	//if (e->mask & (NkkPointerEnter | NkkPointerMotion)) {
	//	win->mx = wl_fixed_to_double(e->sx);
	//	win->my = wl_fixed_to_double(e->sy);
	//}
	//if (e->mask & NkkPointerEnter)
	//	wl_pointer_set_cursor(ptr, wlStub.ptr_ev.serial, win->cursor_surface,
	//		win->cursor_image->hotspot_x, win->cursor_image->hotspot_y);

	//if (e->mask & NkkPointerEnter && win->mouseenter)
	//	win->mouseenter(win, win->mx, win->my);
	//if (e->mask & NkkPointerLeave && win->mouseleave)
	//	win->mouseleave(win);
	//if (e->mask & NkkPointerMotion && win->mousemotion)
	//	win->mousemotion(win, win->mx, win->my);
	//if (e->mask & NkkPointerButton && e->state && win->mousedown)
	//	win->mousedown(win, wlStub.key_repeat.mods, e->button, win->mx, win->my);
	//if (e->mask & NkkPointerButton && !e->state && win->mouseup)
	//	win->mouseup(win, wlStub.key_repeat.mods, e->button, win->mx, win->my);
	//if (e->mask & NkkPointerAxis && win->mousewheel)
	//	win->mousewheel(win, wl_fixed_to_double(e->axis_x), wl_fixed_to_double(e->axis_y));
	//// TODO need NkkPointerDiscrete?
	//memset(e, 0, sizeof(nkk_pointer_event));
}

//
void pointer_leave(void *data, wl_pointer *ptr, uint32_t serial, wl_surface *surface)
{
	/*nkk_display *dpy = data;
	wlStub.ptr_ev.mask |= NkkPointerLeave;
	wlStub.ptr_ev.surface = surface;*/
}

//
void pointer_motion(void *data, wl_pointer *ptr, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
	/*nkk_display *dpy = data;
	wlStub.ptr_ev.mask |= NkkPointerMotion;
	wlStub.ptr_ev.sx = sx;
	wlStub.ptr_ev.sy = sy;*/
};

//
void registry_global(void *data, wl_registry *reg, uint32_t name, const char *iface, uint32_t ver)
{
	if (strcmp(iface, "wl_compositor") == 0)
		wlStub.compositor = wl_registry_bind(reg, name, &wl_compositor_interface, 4);
	else if (strcmp(iface, "wl_subcompositor") == 0)
		wlStub.subcompositor = wl_registry_bind(reg, name, &wl_subcompositor_interface, ver);
	else if (strcmp(iface, "wl_shm") == 0)
		wlStub.shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
	else if (strcmp(iface, "xdg_wm_base") == 0)
	{
		wlStub.xdg_shell = wl_registry_bind(reg, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(wlStub.xdg_shell, &events_xdg_shell, NULL);
	}
	else if (strcmp(iface, "wl_seat") == 0) {
		wlStub.seat = wl_registry_bind(reg, name, &wl_seat_interface, 5);		// 7 마으스 됨, 4 키보드 됨, 5 둘다 됨
		wl_seat_add_listener(wlStub.seat, &events_seat, wlStub.display);
	}
}

//
void seat_capabilities(void *data, wl_seat *wl_seat, uint32_t capabilities)
{
	bool kb = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
	bool ptr = capabilities & WL_SEAT_CAPABILITY_POINTER;
	if (kb && wlStub.keyboard == NULL) {
		wlStub.keyboard = wl_seat_get_keyboard(wlStub.seat);
		wl_keyboard_add_listener(wlStub.keyboard, &events_keyboard, wlStub.display);
	}
	else if (!kb && wlStub.keyboard) {
		wl_keyboard_release(wlStub.keyboard);
		wlStub.keyboard = NULL;
	}
	if (ptr && wlStub.pointer == NULL) {
		wlStub.pointer = wl_seat_get_pointer(wlStub.seat);
		wl_pointer_add_listener(wlStub.pointer, &events_pointer, wlStub.display);
	}
	else if (!ptr && wlStub.pointer) {
		wl_pointer_release(wlStub.pointer);
		wlStub.pointer = NULL;
	}
}

//
void seat_name(void *data, wl_seat *seat, const char *name)
{
}

//
void xdg_shell_pong(void* data, xdg_wm_base* wm_base, uint32_t serial)
{
	xdg_wm_base_pong(wm_base, serial);
}

#pragma endregion 레지스트리

#endif	// USE_WAYLAND
