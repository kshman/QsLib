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
#include <poll.h>
#include <syscall.h>
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
#define wl_display_dispatch							(*wayland_wl_display_dispatch)
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
// 스터브 선언

//
typedef struct nkk_window
{
	int32_t				width, height;	// 펜딩
	wl_surface*			surface;
	xdg_surface*		layer;
	xdg_toplevel*		toplevel;
} nkk_window;

//
typedef struct nkk_keyboard
{
	// XKB
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

	uint32_t			serial;		// 키보드 시리얼은 여기에
	uint32_t			depressed;
	uint32_t			latchlocked;

	// 반복
	struct
	{
		int					fd;
		uint32_t			rate;
		uint32_t			delay;
		uint32_t			scancode;
	}					repeat;
} nkk_keyboard;

//
typedef struct nkk_pointer
{
	struct wl_surface*	surface;
	wl_fixed_t			kx, ky;
	double				axis_x, axis_y;
	bool				axis;
	bool				hover;
	uint32_t			serial;		// 마우스 시리얼
} nkk_pointer;

//
typedef struct nkk_cursor
{
	wl_cursor_theme*	theme;
	wl_cursor_image*	image;
	wl_surface*			surface;
} nkk_cursor;

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
typedef struct nkk_interface
{
	wl_display*			display;
	wl_registry*		registry;
	wl_compositor*		compositor;
	wl_shm*				shm;
	wl_seat*			seat;
	wl_pointer*			pointer;
	wl_keyboard*		keyboard;
	xdg_wm_base*		xdg_shell;

	// 언젠간 쓰겠지
	wl_data_device_manager*				data_device_manager;
	wl_data_device*						data_device;
	zxdg_decoration_manager_v1*			decoration_manager;
	wp_viewporter*						view_porter;
	zwp_relative_pointer_manager_v1*	relative_pointer_manager;
	zwp_pointer_constraints_v1*			pointer_constraints;
	zwp_idle_inhibit_manager_v1*		idle_inhibit_manager;
} nkk_interface;

//
typedef struct WaylandStub
{
	StubBase			base;

	const char*			tag;
	nkk_interface		ifce;

	nkk_keyboard		keyboard;
	nkk_pointer			pointer;
	nkk_cursor			cursor;
	nkk_window			win;
} WaylandStub;

// 여기야 여기!
WaylandStub wlxStub;


//////////////////////////////////////////////////////////////////////////
// 웨이랜드 이벤트

#define WL_DESTROY(var,func)	QN_STMT_BEGIN{ if (var) func(var); }QN_STMT_END
#ifdef _DEBUG
#define WL_TRACE(...)			qn_debug_outputf(false, "WLX STUB", __VA_ARGS__)
#define WL_TRACE_FUNC()			qn_debug_outputs(false, "WLX STUB", __PRETTY_FUNCTION__);
#else
#define WL_TRACE(...)
#define WL_TRACE_FUNC()
#endif

static void nkk_window_close(void);
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
static void xdg_surface_configure(void *, struct xdg_surface *, uint32_t);
static void xdg_toplevel_close(void *, struct xdg_toplevel *);
static void xdg_toplevel_configure(void *, struct xdg_toplevel *, int32_t, int32_t, struct wl_array *);
static void buffer_release(void *, struct wl_buffer *);

static const struct wl_buffer_listener events_buffer = {
	.release = buffer_release,
};
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
static const struct xdg_surface_listener events_xdg_surface =
{
	.configure = xdg_surface_configure,
};
static const struct xdg_toplevel_listener events_xdg_toplevel =
{
	.close = xdg_toplevel_close,
	.configure = xdg_toplevel_configure,
};

//
static void buffer_release(void* data, struct wl_buffer* buffer)
{
	wl_buffer_destroy(buffer);
}

//
static void xdg_surface_configure(void *data, struct xdg_surface *surface, uint32_t serial)
{
	WL_TRACE_FUNC();
	xdg_surface_ack_configure(surface, serial);

	// 임시로 그려보자
	int width = wlxStub.win.width;
	int height = wlxStub.win.height;
	int size = width * height * 4;
	int fd = (int)syscall(SYS_memfd_create, "buffer", 0);
	int ret = ftruncate(fd, size);
	if (ret == -1)
		return;
	byte* mapped = mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	uint* ptr = (uint*)mapped;
	for (int i = 0; i < width * height; i++)
		*ptr = 0xFF00AA00;
	struct wl_shm_pool* pool = wl_shm_create_pool(wlxStub.ifce.shm, fd, size);
	wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);
	munmap(mapped, (size_t)size);
	wl_buffer_add_listener(buffer, &events_buffer, NULL);
	wl_surface_attach(wlxStub.win.surface, buffer, 0, 0);
	wl_surface_damage_buffer(wlxStub.win.surface, 0, 0, width, height);
	wl_surface_commit(wlxStub.win.surface);
}

//
static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg)
{
	WL_TRACE_FUNC();
	nkk_window_close();
}

//
static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg, int32_t width, int32_t height, struct wl_array *states)
{
	WL_TRACE_FUNC();
	if (!width || !height)
		return;
	nkk_window* win = &wlxStub.win;
	win->width = width;
	win->height = height;
}

// 키보드 매핑
static void keyboard_keymap(void *data, wl_keyboard *wl_keyboard, uint32_t fmt, int32_t fd, uint32_t size)
{
	WL_TRACE_FUNC();
	nkk_keyboard* xkb = &wlxStub.keyboard;

	if (fmt != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
	{
		close(fd);
		return;
	}

	char *map = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		qn_debug_outputs(false, "WLX STUB", "invalid keymap handle");
		close(fd);
		return;
	}

	WL_DESTROY(xkb->keymap, xkb_keymap_unref);
	xkb->keymap = xkb_keymap_new_from_string(wlxStub.keyboard.context, map,
		XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

	munmap(map, size);
	close(fd);

	if (xkb->keymap == NULL)
	{
		qn_debug_outputs(false, "WLX STUB", "failed to create keymap");
		return;
	}

	WL_DESTROY(xkb->state, xkb_state_unref);
	xkb->state = xkb_state_new(xkb->keymap);
	if (xkb->state == NULL)
	{
		qn_debug_outputs(false, "WLX STUB", "failed to create xkb state");
		return;
	}

	const char* locale = getenv("LC_ALL");
	if (locale == NULL)
	{
		locale = getenv("LC_CTYPE");
		if (locale == NULL)
		{
			locale = getenv("LANG");
			if (locale == NULL)
				locale = "C";
		}
	}

	WL_DESTROY(xkb->compose_table, xkb_compose_table_unref);
	xkb->compose_table = xkb_compose_table_new_from_locale(xkb->context,
		locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
	if (xkb->compose_table == NULL)
		qn_debug_outputs(false, "WLX STUB", "failed to create xkb compose table");
	else
	{
		WL_DESTROY(xkb->compose_state, xkb_compose_state_unref);
		xkb->compose_state = xkb_compose_state_new(xkb->compose_table, XKB_COMPOSE_STATE_NO_FLAGS);
		if (xkb->compose_state == NULL)
		{
			qn_debug_outputs(false, "WLX STUB", "failed to create xkb compose state");
			xkb_compose_table_unref(xkb->compose_table);
			xkb->compose_table = NULL;
		}
	}

	xkb->mod_shift = QN_BIT(xkb_keymap_mod_get_index(xkb->keymap, XKB_MOD_NAME_SHIFT));
	xkb->mod_ctrl = QN_BIT(xkb_keymap_mod_get_index(xkb->keymap, XKB_MOD_NAME_CTRL));
	xkb->mod_alt = QN_BIT(xkb_keymap_mod_get_index(xkb->keymap, XKB_MOD_NAME_ALT));
	xkb->mod_win = QN_BIT(xkb_keymap_mod_get_index(xkb->keymap, XKB_MOD_NAME_LOGO));
	xkb->mod_caps = QN_BIT(xkb_keymap_mod_get_index(xkb->keymap, XKB_MOD_NAME_CAPS));
	xkb->mod_num = QN_BIT(xkb_keymap_mod_get_index(xkb->keymap, XKB_MOD_NAME_NUM));
}

// 키보드 맨 첨 초기화 또는 포커스일지도 모름. nkk에서는 윈도우 목록에 넣는 용도로 씀
static void keyboard_enter(void *data, wl_keyboard *kb, uint32_t serial, wl_surface *surface, struct wl_array *keys)
{
	WL_TRACE_FUNC();
	qn_ret_if_fail(surface);
	wlxStub.keyboard.serial = serial;
	// 포커스라면 여기에 메시지
}

// 키보드 제거 때 또는 포커스 잃음 둘 중 하나일듯
static void keyboard_leave(void* data, wl_keyboard* wl_keyboard, uint32_t serial, wl_surface* surface)
{
	WL_TRACE_FUNC();
	struct itimerspec ts = { 0, };
	timerfd_settime(wlxStub.keyboard.repeat.fd, 0, &ts, NULL);
	wlxStub.keyboard.serial = serial;
	// 포커스 읽음이라면 여기서 메시지
}

// 텍스트 메시지. 키 입력에서 쓰는데, 루프에서 반복 눌림 검사때도 쓴다
static void keyboard_text(xkb_keysym_t scancode)
{
	nkk_keyboard* xkb = &wlxStub.keyboard;
	const xkb_keysym_t keycode = scancode + 8;
	const xkb_keysym_t* syms;
	if (xkb_state_key_get_syms(xkb->state, keycode, &syms) != 1)
		return;
	xkb_keysym_t sym = syms[0];
	if (xkb->compose_state && xkb_compose_state_feed(xkb->compose_state, sym) == XKB_COMPOSE_FEED_ACCEPTED)
	{
		enum xkb_compose_status status = xkb_compose_state_get_status(xkb->compose_state);
		switch (status)
		{
			case XKB_COMPOSE_COMPOSING:
				return;
			case XKB_COMPOSE_NOTHING:
				break;
			case XKB_COMPOSE_COMPOSED:
				sym = xkb_compose_state_get_one_sym(xkb->compose_state);
				break;
			case XKB_COMPOSE_CANCELLED:
			default:
				sym = XKB_KEY_NoSymbol;
				break;
		}
	}

	char text[8];
	if (xkb_keysym_to_utf8(sym, text, 8) > 0)
		stub_event_on_text(text);
}

// 키 입력. 반복 누르기 기능이 없는지 여기서 해주더만
static void keyboard_key(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t scancode, uint32_t state)
{
	WL_TRACE_FUNC();
	qn_ret_if_fail(wlxStub.keyboard.state);

	nkk_keyboard* kbd = &wlxStub.keyboard;
	wlxStub.keyboard.serial = serial;

	const xkb_keycode_t keycode = scancode + 8;
	const bool down = state == WL_KEYBOARD_KEY_STATE_PRESSED;
	struct itimerspec ts = { 0, };

	if (down)
	{
		if (xkb_keymap_key_repeats(kbd->keymap, keycode) && kbd->repeat.rate > 0)
		{
			kbd->repeat.scancode = scancode;
			if (kbd->repeat.rate > 1)
				ts.it_interval.tv_nsec = QN_NSEC_PER_SEC / kbd->repeat.rate;
			else
				ts.it_interval.tv_sec = 1;
			ts.it_value.tv_sec = kbd->repeat.delay / QN_MSEC_PER_SEC;
			ts.it_value.tv_nsec = (kbd->repeat.delay % QN_MSEC_PER_SEC) * QN_USEC_PER_SEC;
		}
	}
	timerfd_settime(kbd->repeat.fd, 0, &ts, NULL);

	// 키 메시지, keycode 변환 해야하능데...
	stub_event_on_keyboard((QikKey)keycode, true);

	// 텍스트 메시지
	if (down)
		keyboard_text(scancode);
}

// 토글 키 처리
static void keyboard_modifiers(void *data, wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed,
	uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
	WL_TRACE_FUNC();
	nkk_keyboard* kbd = &wlxStub.keyboard;
	xkb_state_update_mask(kbd->state, mods_depressed, mods_latched, mods_locked, 0, 0, group);

	const uint32_t latchlocked = mods_latched | mods_locked;
	if (kbd->depressed != mods_depressed && kbd->latchlocked != latchlocked)
	{
		QikMask mask = 0;

		if (QN_TMASK(mods_depressed, kbd->mod_shift))
			mask |= QIKM_SHIFT;
		if (QN_TMASK(mods_depressed, kbd->mod_ctrl))
			mask |= QIKM_CTRL;
		if (QN_TMASK(mods_depressed, kbd->mod_alt))
			mask |= QIKM_ALT;
		if (QN_TMASK(mods_depressed, kbd->mod_win))
			mask |= QIKM_WIN;

		if (QN_TMASK(latchlocked, kbd->mod_caps))
			mask |= QIKM_CAPS;
		if (QN_TMASK(latchlocked, kbd->mod_num))
			mask |= QIKM_NUM;

		kbd->depressed = mods_depressed;
		kbd->latchlocked = latchlocked;

		wlxStub.base.key.mask = mask;
	}
}

//
static void keyboard_repeat_info(void *data, wl_keyboard* keyboard, int32_t rate, int32_t delay)
{
	WL_TRACE_FUNC();
	qn_ret_if_fail(wlxStub.ifce.keyboard == keyboard);
	wlxStub.keyboard.repeat.rate = (uint32_t)rate;
	wlxStub.keyboard.repeat.delay = (uint32_t)delay;
}

//
static void pointer_enter(void *data, wl_pointer *ptr, uint32_t serial, wl_surface *surface,
	wl_fixed_t sx, wl_fixed_t sy)
{
	WL_TRACE_FUNC();
	nkk_pointer* pm = &wlxStub.pointer;
	pm->serial = serial;
	pm->hover = true;

	// 커서 업데이트
	// 마우스 들어옴 이벤트?
}

//
static void pointer_leave(void *data, wl_pointer *ptr, uint32_t serial, wl_surface *surface)
{
	WL_TRACE_FUNC();
	nkk_pointer* pm = &wlxStub.pointer;
	pm->serial = serial;

	// 마우스 나감 이벤트?
}

//
static void pointer_motion(void *data, wl_pointer *ptr, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
	WL_TRACE_FUNC();
	nkk_pointer* pm = &wlxStub.pointer;

	if (pm->kx == sx && pm->ky == sy)
		return;

	pm->kx = sx;
	pm->ky = sy;
	int x = wl_fixed_to_int(sx);
	int y = wl_fixed_to_int(sy);

	stub_event_on_mouse_move(x, y);
	stub_track_mouse_click(QIM_NONE, QIMT_MOVE);
};

//
static void pointer_axis(void *data, wl_pointer *ptr, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	WL_TRACE_FUNC();
}

//
static void pointer_frame(void *data, wl_pointer *ptr)
{
	WL_TRACE_FUNC();
}

//
static void pointer_axis_source(void *data, wl_pointer *ptr, uint32_t axis_source)
{
	WL_TRACE_FUNC();
}

//
static void pointer_axis_stop(void *data, wl_pointer *ptr, uint32_t time, uint32_t axis)
{
	WL_TRACE_FUNC();
}

//
static void pointer_axis_discrete(void *data, wl_pointer *ptr, uint32_t axis, int32_t discrete)
{
	WL_TRACE_FUNC();
}

//
static void pointer_button(void *data, wl_pointer *ptr, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	WL_TRACE_FUNC();
}

//
static void registry_global(void *data, wl_registry *reg, uint32_t name, const char *iface, uint32_t ver)
{
	WL_TRACE("레지스트리: %s (version: %u)", iface, ver);
	if (strcmp(iface, "wl_compositor") == 0)			// 컴포지터
		wlxStub.ifce.compositor = wl_registry_bind(reg, name, &wl_compositor_interface, QN_MIN(4, ver));
	else if (strcmp(iface, "wl_shm") == 0)				// 메모리 처리
		wlxStub.ifce.shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
	else if (strcmp(iface, "xdg_wm_base") == 0)			// XDG 쉘 윈도우
	{
		wlxStub.ifce.xdg_shell = wl_registry_bind(reg, name, &xdg_wm_base_interface, QN_MIN(6, ver));
		xdg_wm_base_add_listener(wlxStub.ifce.xdg_shell, &events_xdg_shell, NULL);
	}
	else if (strcmp(iface, "wl_seat") == 0)				// 입력
	{
		// 7 마으스 됨, 4 키보드 됨, 5 둘다 됨
		wlxStub.ifce.seat = wl_registry_bind(reg, name, &wl_seat_interface, QN_MIN(5, ver));
		wl_seat_add_listener(wlxStub.ifce.seat, &events_seat, NULL);
	}
}

//
static void seat_capabilities(void *data, wl_seat *wl_seat, uint32_t capabilities)
{
	WL_TRACE_FUNC();
	bool kb = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
	bool ptr = capabilities & WL_SEAT_CAPABILITY_POINTER;
	if (kb && wlxStub.ifce.keyboard == NULL) {
		wlxStub.ifce.keyboard = wl_seat_get_keyboard(wlxStub.ifce.seat);
		wl_keyboard_add_listener(wlxStub.ifce.keyboard, &events_keyboard, NULL);
	}
	else if (!kb && wlxStub.ifce.keyboard) {
		wl_keyboard_release(wlxStub.ifce.keyboard);
		wlxStub.ifce.keyboard = NULL;
	}
	if (ptr && wlxStub.ifce.pointer == NULL) {
		wlxStub.ifce.pointer = wl_seat_get_pointer(wlxStub.ifce.seat);
		wl_pointer_add_listener(wlxStub.ifce.pointer, &events_pointer, NULL);
	}
	else if (!ptr && wlxStub.ifce.pointer) {
		wl_pointer_release(wlxStub.ifce.pointer);
		wlxStub.ifce.pointer = NULL;
	}
}

//
static void seat_name(void *data, wl_seat *seat, const char *name)
{
	WL_TRACE_FUNC();
}

//
static void xdg_shell_pong(void* data, xdg_wm_base* wm_base, uint32_t serial)
{
	WL_TRACE_FUNC();
	xdg_wm_base_pong(wm_base, serial);
}


//////////////////////////////////////////////////////////////////////////
// 웨이런댜 구현

// 커서 설정
static void nkk_set_system_cursor(const char* name)
{
	if (wlxStub.cursor.surface)
	{
		wl_surface_attach(wlxStub.cursor.surface, NULL, 0, 0);
		wl_surface_commit(wlxStub.cursor.surface);
		wl_surface_destroy(wlxStub.cursor.surface);
	}
	if (name == NULL)
	{
		wlxStub.cursor.surface = NULL;
		return;
	}
	wl_cursor *cursor = wl_cursor_theme_get_cursor(wlxStub.cursor.theme, name);
	if (cursor == NULL || (wlxStub.cursor.image = cursor->images[0]) == NULL)
		return;
	wl_buffer *cursor_buffer = wl_cursor_image_get_buffer(wlxStub.cursor.image);
	wlxStub.cursor.surface = wl_compositor_create_surface(wlxStub.ifce.compositor);
	wl_surface_attach(wlxStub.cursor.surface, cursor_buffer, 0, 0);
	wl_surface_commit(wlxStub.cursor.surface);
}

// 디스플레이 열기
static bool nkk_display_open(void)
{
	if (!(wlxStub.ifce.display = wl_display_connect(NULL)))
		return false;
	wlxStub.ifce.registry = wl_display_get_registry(wlxStub.ifce.display);
	wlxStub.keyboard.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	wl_registry_add_listener(wlxStub.ifce.registry, &events_registry, NULL);
	wl_display_roundtrip(wlxStub.ifce.display);		// 레지스트리 에빈트
	wl_display_roundtrip(wlxStub.ifce.display);		// 아웃풋 이벤트

	wlxStub.keyboard.repeat.fd = wl_seat_get_version(wlxStub.ifce.seat) < WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION ?
		-1 : timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);

	wlxStub.cursor.theme = wl_cursor_theme_load(NULL, 24, wlxStub.ifce.shm);
	//nkk_set_system_cursor("left_ptr");

	return true;
}

// 디스플레이 닫기
static void nkk_display_close(void)
{
	qn_ret_if_fail(wlxStub.ifce.display);

	WL_DESTROY(wlxStub.keyboard.keymap, xkb_keymap_unref);
	WL_DESTROY(wlxStub.keyboard.state, xkb_state_unref);
	WL_DESTROY(wlxStub.keyboard.compose_table, xkb_compose_table_unref);
	WL_DESTROY(wlxStub.keyboard.compose_state, xkb_compose_state_unref);

	WL_DESTROY(wlxStub.ifce.pointer, wl_pointer_destroy);
	WL_DESTROY(wlxStub.ifce.keyboard, wl_keyboard_destroy);

	WL_DESTROY(wlxStub.ifce.seat, wl_seat_destroy);
	WL_DESTROY(wlxStub.ifce.xdg_shell, xdg_wm_base_destroy);
	WL_DESTROY(wlxStub.ifce.compositor, wl_compositor_destroy);

	nkk_set_system_cursor(NULL);
	WL_DESTROY(wlxStub.cursor.theme, wl_cursor_theme_destroy);

	WL_DESTROY(wlxStub.ifce.shm, wl_shm_destroy);
	WL_DESTROY(wlxStub.ifce.registry, wl_registry_destroy);
	wl_display_disconnect(wlxStub.ifce.display);

	if (wlxStub.keyboard.repeat.fd >= 0)
		close(wlxStub.keyboard.repeat.fd);
}

// 윈도우 열기
static bool nkk_window_open(int32_t width, int32_t height)
{
	qn_val_if_fail(wlxStub.ifce.display, false);
	qn_val_if_fail(wlxStub.ifce.xdg_shell, false);
	nkk_window* win = &wlxStub.win;

	// 기본 서피스
	win->width = width;		// 펜딩!
	win->height = height;	// 펜딩!
	win->surface = wl_compositor_create_surface(wlxStub.ifce.compositor);

	// XDG
	win->layer = xdg_wm_base_get_xdg_surface(wlxStub.ifce.xdg_shell, win->surface);
	xdg_surface_add_listener(win->layer, &events_xdg_surface, NULL);
	win->toplevel = xdg_surface_get_toplevel(win->layer);
	xdg_toplevel_add_listener(win->toplevel, &events_xdg_toplevel, NULL);
	wl_surface_commit(win->surface);
	wl_display_roundtrip(wlxStub.ifce.display);

	return true;
}

// 윈도우 닫기
static void nkk_window_close(void)
{
	nkk_window* win = &wlxStub.win;

	WL_DESTROY(win->toplevel, xdg_toplevel_destroy);
	WL_DESTROY(win->layer, xdg_surface_destroy);

	if (win->surface)
	{
		wl_surface_attach(win->surface, NULL, 0, 0);
		wl_surface_commit(win->surface);
		wl_surface_destroy(win->surface);
	}
}

#if false
//
void nkk_window_set_fullscreen(nkk_window* win, bool on)
{

	if ((win->type & NkkTypeXdg) == 0) return;
	if (on) xdg_toplevel_set_fullscreen(win->parent.toplevel, NULL);
	else xdg_toplevel_unset_fullscreen(win->parent.toplevel);
}

//
void nkk_window_set_size(nkk_window* win, uint32_t w, uint32_t h)
{
	if ((win->type & NkkTypeXdg) == 0)
		return;
	win->w = w;
	win->h = h;
}

//
void nkk_window_set_title(nkk_window* win, const char *title)
{
	if ((win->type & NkkTypeXdg) == 0)
		return;
	xdg_toplevel_set_title(win->parent.toplevel, title);
}
#endif

//
static bool nkk_os_poll(struct pollfd* fds, nfds_t count, uint ms)
{
	llong timeout = ms;
	for (;;)
	{
		if (timeout != (uint)-1)
		{
			const ullong entry = qn_tick();
#if defined _QN_FREEBSD_ || defined _QN_LINUX_
			const struct timespec ts =
			{
				.tv_sec = timeout / 1000,
				.tv_nsec = (timeout % 1000) * 1000000,
			};
			const int res = ppoll(fds, count, &ts, NULL);
#else
			const int res = poll(fds, count, timeout);
#endif
			timeout -= (llong)(qn_tick() - entry);

			if (res > 0)
				return true;
			if (res == -1 && errno != EINTR && errno != EAGAIN)
				return false;
			if (timeout < 0)
				return false;
		}
		else
		{
			const int res = poll(fds, count, -1);
			if (res > 0)
				return true;
			if (res == -1 && errno != EINTR && errno != EAGAIN)
				return false;
		}
	}
}

//
static bool nkk_display_flush(void)
{
	wl_display* display = wlxStub.ifce.display;
	while (wl_display_flush(display) == -1)
	{
		if (errno != EAGAIN)
			return false;

		struct pollfd fd = { wl_display_get_fd(display), POLLOUT };
		while (nkk_os_poll(&fd, 1, 0) == false)
			return false;
	}
	return true;
}

//
static bool nkk_poll_events(uint ms/*타임아웃*/)
{
	bool loop = true;
	wl_display* display = wlxStub.ifce.display;
	struct pollfd fds[] =
	{
		{ wl_display_get_fd(display), POLLIN,},
		{ wlxStub.keyboard.repeat.fd, POLLIN,},
		{-1,POLLIN,},
	};

	while (loop)
	{
		while (wl_display_prepare_read(display) != 0)
		{
			if (wl_display_dispatch_pending(display) > 0)
				return false;
		}
		if (nkk_display_flush() == false)
		{
			// 오류 났네. 종료
			wl_display_cancel_read(display);
			qg_exit_loop();
			return false;
		}

		// 폴링
		if (nkk_os_poll(fds, QN_COUNTOF(fds), ms) == false)
		{
			wl_display_cancel_read(display);
			return true;
		}

		// 디스플레이
		if (QN_TMASK(fds[0].revents, POLLIN) == false)
			wl_display_cancel_read(display);
		else
		{
			wl_display_read_events(display);
			if (wl_display_dispatch_pending(display) > 0)
				loop = false;
		}

		// 키 눌림
		if (QN_TMASK(fds[1].revents, POLLIN))
		{
			ullong count;
			if (read(fds[1].fd, &count, sizeof(count)) == 0)
			{
				for (ullong i = 0; i < count; i++)
				{
					// 키 눌림 이벤트 보내야함
				}
			}
			loop = false;
		}

		// 커서
		// libdecor
	}

	return true;
}

//
static void nkk_main_loop(void)
{
	wl_display* display = wlxStub.ifce.display;
	wl_display_roundtrip(display);
	wl_display_dispatch(display);
	int fd = wl_display_get_fd(display);
	fd_set fds;
	for (;;)
	{
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		if (select(1, &fds, NULL, NULL, NULL) == -1)
			qn_debug_outputs(true, "WLX STUB", "select failed");

		wl_display_flush(display);
	}
}


//////////////////////////////////////////////////////////////////////////
// 시스템 함수

//
bool stub_system_open(const char* title, const int display, const int width, const int height, QgFlag flags)
{
	if (_so_init() == false)
	{
		qn_debug_outputs(true, "WLX STUB", "so load failed");
		return false;
	}
	if (nkk_display_open() == false)
	{
		qn_debug_outputs(true, "WLX STUB", "cannot open display");
		return false;
	}

	//
	stub_initialize((StubBase*)&wlxStub, flags);

	//
	nkk_window_open(width, height);
	wl_display_roundtrip(wlxStub.ifce.display);

	return true;
}

//
void stub_system_finalize(void)
{
	nkk_window_close();
	nkk_display_close();
}

//
bool stub_system_poll(void)
{
	if (false)
		nkk_main_loop();
	if (nkk_poll_events(0) == false)
		return false;
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

#endif	// USE_WAYLAND
