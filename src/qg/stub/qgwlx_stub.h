//
// qgwlx_stub.h - Wayland 스터브
// 2023-12-23 by kim
//

#pragma once

#include "qg_stub.h"
#include <wayland-cursor.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <libdecor-0/libdecor.h>

// 웨이랜드 모니터
typedef struct WaylandMonitor
{
	QgUdevMonitor		base;

	uint32_t			name;
	int					mode;
	float				scale_factor;

	enum wl_output_transform	transform;
} WaylandMonitor;

// 웨이랜드 스터브
typedef struct WaylandStub
{
	StubBase			base;

	struct wl_display*						display;
	struct wl_registry*						registry;
	struct wl_compositor*					compositor;
	struct wl_subcompositor*				subcompositor;
	struct wl_shm*							shm;
	struct wl_seat*							seat;
	struct wl_pointer*						pointer;
	struct wl_keyboard*						keyboard;
	struct wl_data_device_manager*			data_device_manager;
	struct wl_data_device*					data_device;
	struct xdg_wm_base*						wm_base;
	struct zxdg_decoration_manager_v1*		decoration_manager;
	struct wp_viewporter*					view_porter;
	struct zwp_relative_pointer_manager_v1*	relative_pointer_manager;
	struct zwp_pointer_constraints_v1*		pointer_constraints;
	struct zwp_idle_inhibit_manager_v1*		idle_inhibit_manager;

#if false
	HINSTANCE			instance;
	HWND				hwnd;

	wchar*				class_name;
	wchar*				window_title;
	DWORD				window_style;

	STICKYKEYS			acs_sticky;
	TOGGLEKEYS			acs_toggle;
	FILTERKEYS			acs_filter;
	HHOOK				key_hook;
	BYTE				key_hook_state[256];

	HIMC				himc;
	int					imcs;
	int					high_surrogate;

	int					deadzone_min;
	int					deadzone_max;

	HCURSOR				mouse_cursor;
	WPARAM				mouse_wparam;
	LPARAM				mouse_lparam;
	QimMask				mouse_pending;

	bool				class_registered;
	bool				clear_background;
	bool				bool_padding1;
	bool				bool_padding2;
#endif
} WaylandStub;

// 웨이랜드 스터브 선언
extern WaylandStub wlxStub;

// SO
#ifdef DEF_WLX_TYPEDEF
#define DEF_WLX_FUNC(ret,name,args)\
	extern QN_CONCAT(PFN_, name) QN_CONCAT(wlx_, name);
#else
#define DEF_WLX_FUNC(ret,name,args)\
	typedef ret(*QN_CONCAT(PFN_, name)) args;\
	extern QN_CONCAT(PFN_, name) QN_CONCAT(wlx_, name);
#endif
#include "qgwlx_stub_func.h"

// 별명
#define wl_display_cancel_read						wlx_wl_display_cancel_read
#define wl_display_disconnect						wlx_wl_display_disconnect
#define wl_display_dispatch_pending					wlx_wl_display_dispatch_pending
#define wl_display_flush							wlx_wl_display_flush
#define wl_display_get_fd							wlx_wl_display_get_fd
#define wl_display_prepare_read						wlx_wl_display_prepare_read
#define wl_display_read_events						wlx_wl_display_read_events
#define wl_display_roundtrip						wlx_wl_display_roundtrip
#define wl_proxy_add_listener						wlx_wl_proxy_add_listener
#define wl_display_connect							wlx_wl_display_connect
#define wl_proxy_destroy							wlx_wl_proxy_destroy
#define wl_proxy_get_tag							wlx_wl_proxy_get_tag
#define wl_proxy_get_user_data						wlx_wl_proxy_get_user_data
#define wl_proxy_get_version						wlx_wl_proxy_get_version
#define wl_proxy_marshal							wlx_wl_proxy_marshal
#define wl_proxy_marshal_constructor				wlx_wl_proxy_marshal_constructor
#define wl_proxy_marshal_constructor_versioned		wlx_wl_proxy_marshal_constructor_versioned
#define wl_proxy_marshal_flags						wlx_wl_proxy_marshal_flags
#define wl_proxy_set_tag							wlx_wl_proxy_set_tag
#define wl_proxy_set_user_data						wlx_wl_proxy_set_user_data

#define wl_cursor_image_get_buffer					wlx_wl_cursorimage_get_buffer
#define wl_cursor_theme_destroy						wlx_wl_cursortheme_destroy
#define wl_cursor_theme_get_cursor					wlx_wl_cursortheme_get_cursor
#define wl_cursor_theme_load						wlx_wl_cursortheme_load

#define wl_egl_window_create						wlx_wl_eglwindow_create
#define wl_egl_window_destroy						wlx_wl_eglwindow_destroy
#define wl_egl_window_resize						wlx_wl_eglwindow_resize

#define xkb_compose_state_feed						wlx_xkb_compose_state_feed
#define xkb_compose_state_get_one_sym				wlx_xkb_compose_state_get_one_sym
#define xkb_compose_state_get_status				wlx_xkb_compose_state_get_status
#define xkb_compose_state_new						wlx_xkb_compose_state_new
#define xkb_compose_state_unref						wlx_xkb_compose_state_unref
#define xkb_compose_table_new_from_locale			wlx_xkb_compose_table_new_from_locale
#define xkb_compose_table_unref						wlx_xkb_compose_table_unref
#define xkb_context_new								wlx_xkb_context_new
#define xkb_context_unref							wlx_xkb_context_unref
#define xkb_keymap_key_get_syms_by_level			wlx_xkb_keymap_key_get_syms_by_level
#define xkb_keymap_key_repeats						wlx_xkb_keymap_key_repeats
#define xkb_keymap_mod_get_index					wlx_xkb_keymap_mod_get_index
#define xkb_keymap_new_from_string					wlx_xkb_keymap_new_from_string
#define xkb_keymap_unref							wlx_xkb_keymap_unref
#define xkb_state_key_get_layout					wlx_xkb_state_key_get_layout
#define xkb_state_key_get_syms						wlx_xkb_state_key_get_syms
#define xkb_state_mod_index_is_active				wlx_xkb_state_mod_index_is_active
#define xkb_state_new								wlx_xkb_state_new
#define xkb_state_unref								wlx_xkb_state_unref
#define xkb_state_update_mask						wlx_xkb_state_update_mask

#define libdecor_configuration_get_content_size		wlx_libdecor_configuration_get_content_size
#define libdecor_configuration_get_window_state		wlx_libdecor_configuration_get_window_state
#define libdecor_decorate							wlx_libdecor_decorate
#define libdecor_dispatch							wlx_libdecor_dispatch
#define libdecor_frame_commit						wlx_libdecor_frame_commit
#define libdecor_frame_get_xdg_toplevel				wlx_libdecor_frame_get_xdg_toplevel
#define libdecor_frame_map							wlx_libdecor_frame_map
#define libdecor_frame_set_app_id					wlx_libdecor_frame_set_app_id
#define libdecor_frame_set_capabilities				wlx_libdecor_frame_set_capabilities
#define libdecor_frame_set_fullscreen				wlx_libdecor_frame_set_fullscreen
#define libdecor_frame_set_max_content_size			wlx_libdecor_frame_set_max_content_size
#define libdecor_frame_set_maximized				wlx_libdecor_frame_set_maximized
#define libdecor_frame_set_min_content_size			wlx_libdecor_frame_set_min_content_size
#define libdecor_frame_set_minimized				wlx_libdecor_frame_set_minimized
#define libdecor_frame_set_title					wlx_libdecor_frame_set_title
#define libdecor_frame_set_visibility				wlx_libdecor_frame_set_visibility
#define libdecor_frame_unref						wlx_libdecor_frame_unref
#define libdecor_frame_unset_capabilities			wlx_libdecor_frame_unset_capabilities
#define libdecor_frame_unset_fullscreen				wlx_libdecor_frame_unset_fullscreen
#define libdecor_frame_unset_maximized				wlx_libdecor_frame_unset_maximized
#define libdecor_get_fd								wlx_libdecor_get_fd
#define libdecor_new								wlx_libdecor_new
#define libdecor_state_free							wlx_libdecor_state_free
#define libdecor_state_new							wlx_libdecor_state_new
#define libdecor_unref								wlx_libdecor_unref
