//
// qgwlx_stub.c - 웨이랜드 스터브... 몇번째냐
// 2023-12-31 by kim
//

#include "pch.h"
#ifdef USE_WAYLAND
#include "qs_qn.h"
#include "qs_qg.h"
#include "qs_kmc.h"
#include "qg_stub.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/timerfd.h>

// SO 처리를 위한 도전
typedef struct wl_buffer						wl_buffer;
typedef struct wl_compositor					wl_compositor;
typedef struct wl_cursor						wl_cursor;
typedef struct wl_cursor_image					wl_cursor_image;
typedef struct wl_cursor_theme					wl_cursor_theme;
typedef struct wl_display						wl_display;
typedef struct wl_event_queue					wl_event_queue;
typedef struct wl_keyboard						wl_keyboard;
typedef struct wl_interface						wl_interface;
typedef struct wl_list							wl_list;
typedef struct wl_output						wl_output;
typedef struct wl_pointer						wl_pointer;
typedef struct wl_proxy							wl_proxy;
typedef struct wl_registry						wl_registry;
typedef struct wl_seat							wl_seat;
typedef struct wl_shm							wl_shm;
typedef struct wl_subcompositor					wl_subcompositor;
typedef struct wl_subsurface					wl_subsurface;
typedef struct wl_surface						wl_surface;
typedef struct xdg_surface						xdg_surface;
typedef struct xdg_toplevel						xdg_toplevel;
typedef struct xdg_wm_base						xdg_wm_base;
typedef struct wp_viewporter					wp_viewporter;
typedef struct wl_data_device_manager			wl_data_device_manager;
typedef struct wl_data_device					wl_data_device;
typedef struct zxdg_decoration_manager_v1		zxdg_decoration_manager_v1;
typedef struct zwp_relative_pointer_manager_v1	zwp_relative_pointer_manager_v1;
typedef struct zwp_pointer_constraints_v1		zwp_pointer_constraints_v1;
typedef struct zwp_idle_inhibit_manager_v1		zwp_idle_inhibit_manager_v1;
typedef struct libdecor							libdecor;
typedef struct libdecor_frame					libdecor_frame;
typedef struct libdecor_state					libdecor_state;
typedef struct libdecor_configuration			libdecor_configuration;
typedef struct libdecor_interface				libdecor_interface;
typedef struct libdecor_frame_interface			libdecor_frame_interface;
enum libdecor_resize_edge;
enum libdecor_capabilities;
enum libdecor_window_state;

#include <wayland-cursor.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <wayland-client-core.h>

#ifdef _DEBUG
#define DEBUG_WLX_TRACE
#endif

//
#define DEF_WLX_FUNC(ret,name,args)\
	ret (*QN_CONCAT(wayland_, name)) args;
#include "qgwlx_func.h"

// 함수 읽기
static void* _load_so_func(QnModule* module, const char* func_name, const char* so_name)
{
	void* ret = qn_mod_func(module, func_name);
#ifdef DEBUG_WLX_TRACE
	static const char* current_so = NULL;
	if (current_so != so_name)
	{
		current_so = so_name;
		qn_debug_outputf(false, "WLX STUB", "so: %s", so_name);
	}
	qn_debug_outputf(false, "WLX STUB", "%s: %s",
		ret == NULL ? "load failed" : "loaded", func_name);
#else
	QN_DUMMY(so_name);
#endif
	return ret;
}

// SO 전부 초기화
static bool _so_init(void)
{
	static bool loaded = false;
	qn_val_if_ok(loaded, true);
	QnModule* module = NULL;
	const char* so_name = NULL;
#define DEF_WLX_SO_BEGIN(name)\
	module = qn_mod_load(so_name = name".so", 1); if (module==NULL)\
	{ qn_debug_outputf(true, "WLX STUB", "so load failed: %s", so_name); return false; } else {
#define DEF_WLX_SO_END	}
#define DEF_WLX_FUNC(ret,name,args) \
	QN_CONCAT(wayland_,name) = (ret(*)args)_load_so_func(module, QN_STRING(name), so_name);\
	if (QN_CONCAT(wayland_,name) == NULL) {\
		qn_debug_outputf(true, "WLX STUB", "%s: '%s'", "load failed", QN_STRING(name), so_name); return false; }
#include "qgwlx_func.h"
	return loaded = true;
}

// 별명
#define wl_display_cancel_read						(*wayland_wl_display_cancel_read)
#define wl_display_disconnect						(*wayland_wl_display_disconnect)
#define wl_display_dispatch_pending					(*wayland_wl_display_dispatch_pending)
#define wl_display_flush							(*wayland_wl_display_flush)
#define wl_display_get_fd							(*wayland_wl_display_get_fd)
#define wl_display_prepare_read						(*wayland_wl_display_prepare_read)
#define wl_display_read_events						(*wayland_wl_display_read_events)
#define wl_display_roundtrip						(*wayland_wl_display_roundtrip)
#define wl_proxy_add_listener						(*wayland_wl_proxy_add_listener)
#define wl_display_connect							(*wayland_wl_display_connect)
#define wl_proxy_destroy							(*wayland_wl_proxy_destroy)
#define wl_proxy_get_tag							(*wayland_wl_proxy_get_tag)
#define wl_proxy_get_user_data						(*wayland_wl_proxy_get_user_data)
#define wl_proxy_get_version						(*wayland_wl_proxy_get_version)
#define wl_proxy_marshal							(*wayland_wl_proxy_marshal)
#define wl_proxy_marshal_constructor				(*wayland_wl_proxy_marshal_constructor)
#define wl_proxy_marshal_constructor_versioned		(*wayland_wl_proxy_marshal_constructor_versioned)
#define wl_proxy_marshal_flags						(*wayland_wl_proxy_marshal_flags)
#define wl_proxy_set_tag							(*wayland_wl_proxy_set_tag)
#define wl_proxy_set_user_data						(*wayland_wl_proxy_set_user_data)
#include <wayland-client-protocol.h>

#define wl_cursor_image_get_buffer					(*wayland_wl_cursor_image_get_buffer)
#define wl_cursor_theme_destroy						(*wayland_wl_cursor_theme_destroy)
#define wl_cursor_theme_get_cursor					(*wayland_wl_cursor_theme_get_cursor)
#define wl_cursor_theme_load						(*wayland_wl_cursor_theme_load)

#define wl_egl_window_create						(*wayland_wl_eglwindow_create)
#define wl_egl_window_destroy						(*wayland_wl_eglwindow_destroy)
#define wl_egl_window_resize						(*wayland_wl_eglwindow_resize)
#include <wayland-egl.h>

#define xkb_compose_state_feed						(*wayland_xkb_compose_state_feed)
#define xkb_compose_state_get_one_sym				(*wayland_xkb_compose_state_get_one_sym)
#define xkb_compose_state_get_status				(*wayland_xkb_compose_state_get_status)
#define xkb_compose_state_new						(*wayland_xkb_compose_state_new)
#define xkb_compose_state_unref						(*wayland_xkb_compose_state_unref)
#define xkb_compose_table_new_from_locale			(*wayland_xkb_compose_table_new_from_locale)
#define xkb_compose_table_unref						(*wayland_xkb_compose_table_unref)
#define xkb_context_new								(*wayland_xkb_context_new)
#define xkb_context_unref							(*wayland_xkb_context_unref)
#define xkb_keymap_key_get_syms_by_level			(*wayland_xkb_keymap_key_get_syms_by_level)
#define xkb_keymap_key_repeats						(*wayland_xkb_keymap_key_repeats)
#define xkb_keymap_mod_get_index					(*wayland_xkb_keymap_mod_get_index)
#define xkb_keymap_new_from_string					(*wayland_xkb_keymap_new_from_string)
#define xkb_keymap_unref							(*wayland_xkb_keymap_unref)
#define xkb_state_key_get_layout					(*wayland_xkb_state_key_get_layout)
#define xkb_state_key_get_syms						(*wayland_xkb_state_key_get_syms)
#define xkb_state_mod_index_is_active				(*wayland_xkb_state_mod_index_is_active)
#define xkb_state_new								(*wayland_xkb_state_new)
#define xkb_state_unref								(*wayland_xkb_state_unref)
#define xkb_state_update_mask						(*wayland_xkb_state_update_mask)
#define xkb_keysym_to_utf8							(*wayland_xkb_keysym_to_utf8)

#include <libdecor-0/libdecor.h>
#define libdecor_configuration_get_content_size		(*wayland_libdecor_configuration_get_content_size)
#define libdecor_configuration_get_window_state		(*wayland_libdecor_configuration_get_window_state)
#define libdecor_decorate							(*wayland_libdecor_decorate)
#define libdecor_dispatch							(*wayland_libdecor_dispatch)
#define libdecor_frame_commit						(*wayland_libdecor_frame_commit)
#define libdecor_frame_get_xdg_toplevel				(*wayland_libdecor_frame_get_xdg_toplevel)
#define libdecor_frame_map							(*wayland_libdecor_frame_map)
#define libdecor_frame_set_app_id					(*wayland_libdecor_frame_set_app_id)
#define libdecor_frame_set_capabilities				(*wayland_libdecor_frame_set_capabilities)
#define libdecor_frame_set_fullscreen				(*wayland_libdecor_frame_set_fullscreen)
#define libdecor_frame_set_max_content_size			(*wayland_libdecor_frame_set_max_content_size)
#define libdecor_frame_set_maximized				(*wayland_libdecor_frame_set_maximized)
#define libdecor_frame_set_min_content_size			(*wayland_libdecor_frame_set_min_content_size)
#define libdecor_frame_set_minimized				(*wayland_libdecor_frame_set_minimized)
#define libdecor_frame_set_title					(*wayland_libdecor_frame_set_title)
#define libdecor_frame_set_visibility				(*wayland_libdecor_frame_set_visibility)
#define libdecor_frame_unref						(*wayland_libdecor_frame_unref)
#define libdecor_frame_unset_capabilities			(*wayland_libdecor_frame_unset_capabilities)
#define libdecor_frame_unset_fullscreen				(*wayland_libdecor_frame_unset_fullscreen)
#define libdecor_frame_unset_maximized				(*wayland_libdecor_frame_unset_maximized)
#define libdecor_get_fd								(*wayland_libdecor_get_fd)
#define libdecor_new								(*wayland_libdecor_new)
#define libdecor_state_free							(*wayland_libdecor_state_free)
#define libdecor_state_new							(*wayland_libdecor_state_new)
#define libdecor_unref								(*wayland_libdecor_unref)

#include "wayland-wayland.h"
#include "wayland-xdg-shell.h"
#include "wayland-wayland-code.h"
#include "wayland-xdg-shell-code.h"


//////////////////////////////////////////////////////////////////////////
// 스터브 본문

//
typedef struct nkk_window
{
	uint32_t			width, height;	// 목적 수치
	struct wl_surface*	surface;
	struct wl_surface*	cursor_surface;
	struct wl_cursor_image*	cursor_image;
	struct xdg_surface*		layer;
	struct xdg_toplevel*	toplevel;
} nkk_window;

//
typedef struct nkk_xkb
{
	struct xkb_keymap*	keymap;
	struct xkb_context*	context;
	struct xkb_state*	state;
	struct xkb_compose_table*	compose_table;
	struct xkb_compose_state*	compose_state;

	xkb_mod_index_t		mod_shift;
	xkb_mod_index_t		mod_ctrl;
	xkb_mod_index_t		mod_alt;
	xkb_mod_index_t		mod_win;
	xkb_mod_index_t		mod_caps;
	xkb_mod_index_t		mod_num;
} nkk_xkb;

//
typedef struct nkk_key_repeat
{
	int					fd;
	uint32_t			rate;
	uint32_t			delay;
	uint32_t			key;
	uint32_t			serial;
} nkk_key_repeat;

//
typedef struct nkk_pointer_event
{
	struct wl_surface*	surface;
	wl_fixed_t			kx, ky;
	double				axis_x, axis_y;
	bool				axis;
	bool				hover;
} nkk_pointer_event;

//
typedef struct nkk_monitor
{
	QgUdevMonitor		base;

	uint32_t			name;
	int					mode;
	float				scale_factor;
	enum wl_output_transform	transform;
} nkk_monitor;

//
typedef struct WaylandStub
{
	StubBase			base;
	const char*			tag;

	wl_display*			display;
	wl_registry*		registry;
	wl_compositor*		compositor;
	wl_shm*				shm;
	wl_seat*			seat;
	wl_pointer*			pointer;
	wl_keyboard*		keyboard;
	xdg_wm_base*		xdg_shell;

	wl_cursor_theme*	cursor_theme;

	nkk_xkb				xkb;
	nkk_key_repeat		key_repeat;
	nkk_pointer_event	pointer_event;
	nkk_window			win;

	// 언젠간 쓰겠지
	wl_data_device_manager*				data_device_manager;
	wl_data_device*						data_device;
	zxdg_decoration_manager_v1*			decoration_manager;
	wp_viewporter*						view_porter;
	zwp_relative_pointer_manager_v1*	relative_pointer_manager;
	zwp_pointer_constraints_v1*			pointer_constraints;
	zwp_idle_inhibit_manager_v1*		idle_inhibit_manager;
} WaylandStub;

// 여기야 여기!
WaylandStub wlxStub;

//
static bool nkk_display_open(void);
static void nkk_display_close(void);

//
bool stub_system_open(const char* title, const int display, const int width, const int height, QgFlag flags)
{
	if (_so_init() == false)
	{
		qn_debug_outputs(true, "WLX STUB", "so load failed");
		return false;
	}
	if (nkk_display_open()==false)
	{
		qn_debug_outputs(true, "WLX STUB", "cannot open display");
		return false;
	}

	//
	stub_initialize((StubBase*)&wlxStub, flags);

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
	return false;
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

//////////////////////////////////////////////////////////////////////////
// 웨이랜드 이벤트

#define WL_DESTROY(var,func)	QN_STMT_BEGIN{ if (var) func(var); }QN_STMT_END

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
static void keyboard_enter(void *data, wl_keyboard *kb, uint32_t n, wl_surface *surface, struct wl_array *keys)
{
	//nkk_display *dpy = data;
	//nkk_window *win, *tmp;
	//wl_list_for_each_safe(win, tmp, &wlxStub.windows, link) {
	//	if (win->surface == surface) {
	//		wl_list_remove(&win->link);
	//		wl_list_insert(&wlxStub.windows, &win->link);
	//		break;
	//	}
	//}
}

//
static void keyboard_key(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	/*nkk_display *dpy = data;
	if (wlxStub.xkb_state == NULL)
		return;
	nkk_window *win = wl_container_of(wlxStub.windows.next, win, link);
	uint32_t keycode = key + 8;
	xkb_keysym_t sym = xkb_state_key_get_one_sym(wlxStub.xkb_state, keycode);
	wlxStub.key_repeat.win = win;
	wlxStub.key_repeat.sym = sym;
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED && win->keydown)
	{
		gettimeofday(&wlxStub.key_repeat.last, NULL);
		wlxStub.key_repeat.interval = &wlxStub.key_repeat.delay;
		win->keydown(win, wlxStub.key_repeat.mods, sym);
	}
	else if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
	{
		if (win->keyup) win->keyup(win, wlxStub.key_repeat.mods, sym);
		wlxStub.key_repeat.interval = NULL;
	}*/
}

//
static void keyboard_keymap(void *data, wl_keyboard *wl_keyboard, uint32_t fmt, int32_t fd, uint32_t size)
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
	struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(wlxStub.xkb_context, map,
		XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(map, size);
	close(fd);
	struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
	xkb_keymap_unref(wlxStub.xkb_keymap);
	xkb_state_unref(wlxStub.xkb_state);
	wlxStub.xkb_keymap = xkb_keymap;
	wlxStub.xkb_state = xkb_state;*/
}

//
static void keyboard_leave(void* data, wl_keyboard* wl_keyboard, uint32_t serial, wl_surface* surface)
{
}


//
static void keyboard_modifiers(void *data, wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed,
	uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
	/*nkk_display *dpy = data;
	if (wlxStub.xkb_keymap == NULL)
		return;
	xkb_state_update_mask(wlxStub.xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
	wlxStub.key_repeat.mods = 0;
	if (xkb_state_mod_name_is_active(wlxStub.xkb_state, "Shift", XKB_STATE_MODS_EFFECTIVE) == 1)
		wlxStub.key_repeat.mods |= NKK_MOD_SHIFT;
	if (xkb_state_mod_name_is_active(wlxStub.xkb_state, "Control", XKB_STATE_MODS_EFFECTIVE) == 1)
		wlxStub.key_repeat.mods |= NKK_MOD_CTRL;
	if (xkb_state_mod_name_is_active(wlxStub.xkb_state, "Mod1", XKB_STATE_MODS_EFFECTIVE) == 1)
		wlxStub.key_repeat.mods |= NKK_MOD_ALT;
	if (xkb_state_mod_name_is_active(wlxStub.xkb_state, "Mod4", XKB_STATE_MODS_EFFECTIVE) == 1)
		wlxStub.key_repeat.mods |= NKK_MOD_MOD4;*/
}

//
static void keyboard_repeat_info(void *data, wl_keyboard *wl_keyboard, int32_t rate, int32_t delay)
{
	/*nkk_display *dpy = data;
	wlxStub.key_repeat.delay.tv_usec = 1000 * delay;
	wlxStub.key_repeat.repeat.tv_usec = 1000 * 1000 / rate;*/
}

//
static void pointer_axis(void *data, wl_pointer *ptr, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	/*nkk_display *dpy = data;
	wlxStub.ptr_ev.mask |= NkkPointerAxis;
	if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
		wlxStub.ptr_ev.axis_x = value;
	else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		wlxStub.ptr_ev.axis_y = value;*/
}

//
static void pointer_axis_discrete(void *data, wl_pointer *ptr, uint32_t axis, int32_t discrete)
{
}

//
static void pointer_axis_source(void *data, wl_pointer *ptr, uint32_t axis_source)
{
}

//
static void pointer_axis_stop(void *data, wl_pointer *ptr, uint32_t time, uint32_t axis)
{
}

//
static void pointer_button(void *data, wl_pointer *ptr, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	/*nkk_display *dpy = data;
	wlxStub.ptr_ev.mask |= NkkPointerButton;
	wlxStub.ptr_ev.serial = serial;
	wlxStub.ptr_ev.button = button;
	wlxStub.ptr_ev.state = state;*/
}

//
static void pointer_enter(void *data, wl_pointer *ptr, uint32_t serial, wl_surface *surface,
	wl_fixed_t sx, wl_fixed_t sy)
{
	/*nkk_display *dpy = data;
	wlxStub.ptr_ev.mask |= NkkPointerEnter;
	wlxStub.ptr_ev.serial = serial;
	wlxStub.ptr_ev.surface = surface;
	wlxStub.ptr_ev.sx = sx,
		wlxStub.ptr_ev.sy = sy;*/
}

//
static void pointer_frame(void *data, wl_pointer *ptr)
{
	//nkk_display *dpy = data;
	//static wl_surface *target;
	//nkk_pointer_event *e = &wlxStub.ptr_ev;

	//if (e->mask & (NkkPointerEnter | NkkPointerLeave) && e->surface)
	//	target = e->surface;
	//nkk_window *win;
	//wl_list_for_each(win, &wlxStub.windows, link)
	//	if (win->surface == target) break;
	//if (e->mask & NkkPointerLeave)
	//	target = NULL;
	//if (!win) return;

	//if (e->mask & (NkkPointerEnter | NkkPointerMotion)) {
	//	win->mx = wl_fixed_to_double(e->sx);
	//	win->my = wl_fixed_to_double(e->sy);
	//}
	//if (e->mask & NkkPointerEnter)
	//	wl_pointer_set_cursor(ptr, wlxStub.ptr_ev.serial, win->cursor_surface,
	//		win->cursor_image->hotspot_x, win->cursor_image->hotspot_y);

	//if (e->mask & NkkPointerEnter && win->mouseenter)
	//	win->mouseenter(win, win->mx, win->my);
	//if (e->mask & NkkPointerLeave && win->mouseleave)
	//	win->mouseleave(win);
	//if (e->mask & NkkPointerMotion && win->mousemotion)
	//	win->mousemotion(win, win->mx, win->my);
	//if (e->mask & NkkPointerButton && e->state && win->mousedown)
	//	win->mousedown(win, wlxStub.key_repeat.mods, e->button, win->mx, win->my);
	//if (e->mask & NkkPointerButton && !e->state && win->mouseup)
	//	win->mouseup(win, wlxStub.key_repeat.mods, e->button, win->mx, win->my);
	//if (e->mask & NkkPointerAxis && win->mousewheel)
	//	win->mousewheel(win, wl_fixed_to_double(e->axis_x), wl_fixed_to_double(e->axis_y));
	//// TODO need NkkPointerDiscrete?
	//memset(e, 0, sizeof(nkk_pointer_event));
}

//
static void pointer_leave(void *data, wl_pointer *ptr, uint32_t serial, wl_surface *surface)
{
	/*nkk_display *dpy = data;
	wlxStub.ptr_ev.mask |= NkkPointerLeave;
	wlxStub.ptr_ev.surface = surface;*/
}

//
static void pointer_motion(void *data, wl_pointer *ptr, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
	/*nkk_display *dpy = data;
	wlxStub.ptr_ev.mask |= NkkPointerMotion;
	wlxStub.ptr_ev.sx = sx;
	wlxStub.ptr_ev.sy = sy;*/
};

//
static void registry_global(void *data, wl_registry *reg, uint32_t name, const char *iface, uint32_t ver)
{
	if (strcmp(iface, "wl_compositor") == 0)			// 컴포지터
		wlxStub.compositor = wl_registry_bind(reg, name, &wl_compositor_interface, QN_MIN(4, ver));
	else if (strcmp(iface, "wl_shm") == 0)				// 메모리 처리
		wlxStub.shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
	else if (strcmp(iface, "xdg_wm_base") == 0)			// XDG 쉘 윈도우
	{
		wlxStub.xdg_shell = wl_registry_bind(reg, name, &xdg_wm_base_interface, QN_MIN(6, ver));
		xdg_wm_base_add_listener(wlxStub.xdg_shell, &events_xdg_shell, NULL);
	}
	else if (strcmp(iface, "wl_seat") == 0)				// 입력
	{
		// 7 마으스 됨, 4 키보드 됨, 5 둘다 됨
		wlxStub.seat = wl_registry_bind(reg, name, &wl_seat_interface, QN_MIN(5,ver));		
		wl_seat_add_listener(wlxStub.seat, &events_seat, wlxStub.display);
	}
}

//
static void seat_capabilities(void *data, wl_seat *wl_seat, uint32_t capabilities)
{
	bool kb = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
	bool ptr = capabilities & WL_SEAT_CAPABILITY_POINTER;
	if (kb && wlxStub.keyboard == NULL) {
		wlxStub.keyboard = wl_seat_get_keyboard(wlxStub.seat);
		wl_keyboard_add_listener(wlxStub.keyboard, &events_keyboard, wlxStub.display);
	}
	else if (!kb && wlxStub.keyboard) {
		wl_keyboard_release(wlxStub.keyboard);
		wlxStub.keyboard = NULL;
	}
	if (ptr && wlxStub.pointer == NULL) {
		wlxStub.pointer = wl_seat_get_pointer(wlxStub.seat);
		wl_pointer_add_listener(wlxStub.pointer, &events_pointer, wlxStub.display);
	}
	else if (!ptr && wlxStub.pointer) {
		wl_pointer_release(wlxStub.pointer);
		wlxStub.pointer = NULL;
	}
}

//
static void seat_name(void *data, wl_seat *seat, const char *name)
{
}

//
static void xdg_shell_pong(void* data, xdg_wm_base* wm_base, uint32_t serial)
{
	xdg_wm_base_pong(wm_base, serial);
}

//
static bool nkk_display_open(void)
{
	if (!(wlxStub.display = wl_display_connect(NULL)))
		return false;
	wlxStub.registry = wl_display_get_registry(wlxStub.display);
	wlxStub.xkb.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	wl_registry_add_listener(wlxStub.registry, &events_registry, wlxStub.display);
	wl_display_roundtrip(wlxStub.display);
	wl_display_roundtrip(wlxStub.display);

	wlxStub.key_repeat.fd = wl_seat_get_version(wlxStub.seat) < WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION ?
		-1 : timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);

	wlxStub.cursor_theme = wl_cursor_theme_load(NULL, 24, wlxStub.shm);

	return true;
}

static void nkk_display_close(void)
{
	qn_ret_if_fail(wlxStub.display);

	WL_DESTROY(wlxStub.pointer, wl_pointer_destroy);
	WL_DESTROY(wlxStub.keyboard, wl_keyboard_destroy);

	WL_DESTROY(wlxStub.seat, wl_seat_destroy);
	WL_DESTROY(wlxStub.xdg_shell, xdg_wm_base_destroy);
	WL_DESTROY(wlxStub.compositor, wl_compositor_destroy);

	WL_DESTROY(wlxStub.cursor_theme, wl_cursor_theme_destroy);
	WL_DESTROY(wlxStub.registry, wl_registry_destroy);
	wl_display_disconnect(wlxStub.display);

	if (wlxStub.key_repeat.fd >= 0)
		close(wlxStub.key_repeat.fd);
}

#endif	// USE_WAYLAND
