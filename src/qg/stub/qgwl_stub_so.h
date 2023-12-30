//
// qgwl_stub_so.h - 웨이랜드 SO 로드용
// 2023-12-31 by kim
//

#pragma once

//
struct wl_interface;
struct wl_proxy;
struct wl_event_queue;
struct wl_display;
struct wl_surface;
struct wl_shm;
struct wl_seat;
struct wl_output;
struct libdecor;
struct libdecor_frame;
struct libdecor_state;
struct libdecor_configuration;
struct libdecor_interface;
struct libdecor_frame_interface;
enum libdecor_resize_edge;
enum libdecor_capabilities;
enum libdecor_window_state;

#include <wayland-cursor.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <wayland-client-core.h>

#define DEF_WL_FUNC(ret,name,args)\
	typedef ret (*QN_CONCAT(PFN_, name)) args;\
	extern QN_CONCAT(PFN_, name) QN_CONCAT(wayland_, name);
#include "qgwl_stub_func.h"

#if defined(_WAYLAND_CLIENT_H) || defined(WAYLAND_CLIENT_H)
#error Do not include <wayland-client.h>!!!
#endif

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

#define wl_cursor_image_get_buffer					(*wayland_wl_cursor_image_get_buffer)
#define wl_cursor_theme_destroy						(*wayland_wl_cursor_theme_destroy)
#define wl_cursor_theme_get_cursor					(*wayland_wl_cursor_theme_get_cursor)
#define wl_cursor_theme_load						(*wayland_wl_cursor_theme_load)

#define wl_egl_window_create						(*wayland_wl_eglwindow_create)
#define wl_egl_window_destroy						(*wayland_wl_eglwindow_destroy)
#define wl_egl_window_resize						(*wayland_wl_eglwindow_resize)
#include <wayland-client-protocol.h>
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
