//
// qgwlx_func.h - Wayland 함수
// 2023-12-23 by kim
//

#ifndef DEF_WL_SO_BEGIN
#define DEF_WL_SO_BEGIN(name)
#endif
#ifndef DEF_WL_SO_END
#define DEF_WL_SO_END
#endif
#ifndef DEF_WL_FUNC
#define DEF_WL_FUNC(ret,name,args)
#endif

DEF_WL_SO_BEGIN("libwayland-client")
DEF_WL_FUNC(const char* const*, wl_proxy_get_tag, (struct wl_proxy*))
DEF_WL_FUNC(int, wl_display_dispatch_pending, (struct wl_display* display))
DEF_WL_FUNC(int, wl_display_flush, (struct wl_display* display))
DEF_WL_FUNC(int, wl_display_get_fd, (struct wl_display*))
DEF_WL_FUNC(int, wl_display_prepare_read, (struct wl_display*))
DEF_WL_FUNC(int, wl_display_read_events, (struct wl_display* display))
DEF_WL_FUNC(int, wl_display_roundtrip, (struct wl_display*))
DEF_WL_FUNC(int, wl_proxy_add_listener, (struct wl_proxy*, void(**)(void), void*))
DEF_WL_FUNC(struct wl_display*, wl_display_connect, (const char*))
DEF_WL_FUNC(struct wl_proxy*, wl_proxy_marshal_constructor, (struct wl_proxy*, uint32_t, const struct wl_interface*, ...))
DEF_WL_FUNC(struct wl_proxy*, wl_proxy_marshal_constructor_versioned, (struct wl_proxy*, uint32_t, const struct wl_interface*, uint32_t, ...))
DEF_WL_FUNC(struct wl_proxy*, wl_proxy_marshal_flags, (struct wl_proxy*, uint32_t, const struct wl_interface*, uint32_t, uint32_t, ...))
DEF_WL_FUNC(uint32_t, wl_proxy_get_version, (struct wl_proxy*))
DEF_WL_FUNC(void*, wl_proxy_get_user_data, (struct wl_proxy*))
DEF_WL_FUNC(void, wl_display_cancel_read, (struct wl_display* display))
DEF_WL_FUNC(void, wl_display_disconnect, (struct wl_display*))
DEF_WL_FUNC(void, wl_proxy_destroy, (struct wl_proxy*))
DEF_WL_FUNC(void, wl_proxy_marshal, (struct wl_proxy*, uint32_t, ...))
DEF_WL_FUNC(void, wl_proxy_set_tag, (struct wl_proxy*, const char*const*))
DEF_WL_FUNC(void, wl_proxy_set_user_data, (struct wl_proxy*, void*))
DEF_WL_SO_END

DEF_WL_SO_BEGIN("libwayland-cursor")
DEF_WL_FUNC(struct wl_buffer*, wl_cursor_image_get_buffer, (struct wl_cursor_image*))
DEF_WL_FUNC(struct wl_cursor*, wl_cursor_theme_get_cursor, (struct wl_cursor_theme*, const char*))
DEF_WL_FUNC(struct wl_cursor_theme*, wl_cursor_theme_load, (const char*, int, struct wl_shm*))
DEF_WL_FUNC(void, wl_cursor_theme_destroy, (struct wl_cursor_theme*))
DEF_WL_SO_END

DEF_WL_SO_BEGIN("libwayland-egl")
DEF_WL_FUNC(struct wl_egl_window*, wl_egl_window_create, (struct wl_surface*, int, int))
DEF_WL_FUNC(void, wl_egl_window_destroy, (struct wl_egl_window*))
DEF_WL_FUNC(void, wl_egl_window_resize, (struct wl_egl_window*, int, int, int, int))
DEF_WL_SO_END

DEF_WL_SO_BEGIN("libxkbcommon")
DEF_WL_FUNC(enum xkb_compose_feed_result, xkb_compose_state_feed, (struct xkb_compose_state*, xkb_keysym_t))
DEF_WL_FUNC(enum xkb_compose_status, xkb_compose_state_get_status, (struct xkb_compose_state*))
DEF_WL_FUNC(enum xkb_state_component, xkb_state_update_mask, (struct xkb_state*, xkb_mod_mask_t, xkb_mod_mask_t, xkb_mod_mask_t, xkb_layout_index_t, xkb_layout_index_t, xkb_layout_index_t))
DEF_WL_FUNC(int, xkb_keymap_key_get_syms_by_level, (struct xkb_keymap*, xkb_keycode_t, xkb_layout_index_t, xkb_level_index_t, const xkb_keysym_t**))
DEF_WL_FUNC(int, xkb_keymap_key_repeats, (struct xkb_keymap*, xkb_keycode_t))
DEF_WL_FUNC(int, xkb_state_key_get_syms, (struct xkb_state*, xkb_keycode_t, const xkb_keysym_t**))
DEF_WL_FUNC(int, xkb_state_mod_index_is_active, (struct xkb_state*, xkb_mod_index_t, enum xkb_state_component))
DEF_WL_FUNC(struct xkb_compose_state*, xkb_compose_state_new, (struct xkb_compose_table*, enum xkb_compose_state_flags))
DEF_WL_FUNC(struct xkb_compose_table*, xkb_compose_table_new_from_locale, (struct xkb_context*, const char*, enum xkb_compose_compile_flags))
DEF_WL_FUNC(struct xkb_context*, xkb_context_new, (enum xkb_context_flags))
DEF_WL_FUNC(struct xkb_keymap*, xkb_keymap_new_from_string, (struct xkb_context*, const char*, enum xkb_keymap_format, enum xkb_keymap_compile_flags))
DEF_WL_FUNC(struct xkb_state*, xkb_state_new, (struct xkb_keymap*))
DEF_WL_FUNC(void, xkb_compose_state_unref, (struct xkb_compose_state*))
DEF_WL_FUNC(void, xkb_compose_table_unref, (struct xkb_compose_table*))
DEF_WL_FUNC(void, xkb_context_unref, (struct xkb_context*))
DEF_WL_FUNC(void, xkb_keymap_unref, (struct xkb_keymap*))
DEF_WL_FUNC(void, xkb_state_unref, (struct xkb_state*))
DEF_WL_FUNC(xkb_keysym_t, xkb_compose_state_get_one_sym, (struct xkb_compose_state*))
DEF_WL_FUNC(xkb_layout_index_t, xkb_state_key_get_layout, (struct xkb_state*, xkb_keycode_t))
DEF_WL_FUNC(xkb_mod_index_t, xkb_keymap_mod_get_index, (struct xkb_keymap*, const char*))
DEF_WL_FUNC(int, xkb_keysym_to_utf8, (xkb_keysym_t keysym, char *buffer, size_t size))
DEF_WL_SO_END

DEF_WL_SO_BEGIN("libdecor-0")
DEF_WL_FUNC(bool, libdecor_configuration_get_content_size, (struct libdecor_configuration*, struct libdecor_frame*, int*, int*))
DEF_WL_FUNC(bool, libdecor_configuration_get_window_state, (struct libdecor_configuration*, enum libdecor_window_state*))
DEF_WL_FUNC(int, libdecor_dispatch, (struct libdecor*, int))
DEF_WL_FUNC(int, libdecor_get_fd, (struct libdecor*))
DEF_WL_FUNC(struct libdecor*, libdecor_new, (struct wl_display*, const struct libdecor_interface*))
DEF_WL_FUNC(struct libdecor_frame*, libdecor_decorate, (struct libdecor*, struct wl_surface*, const struct libdecor_frame_interface*, void*))
DEF_WL_FUNC(struct libdecor_state*, libdecor_state_new, (int, int))
DEF_WL_FUNC(struct xdg_toplevel*, libdecor_frame_get_xdg_toplevel, (struct libdecor_frame*))
DEF_WL_FUNC(void, libdecor_frame_commit, (struct libdecor_frame*, struct libdecor_state*, struct libdecor_configuration*))
DEF_WL_FUNC(void, libdecor_frame_map, (struct libdecor_frame*))
DEF_WL_FUNC(void, libdecor_frame_set_app_id, (struct libdecor_frame*, const char*))
DEF_WL_FUNC(void, libdecor_frame_set_capabilities, (struct libdecor_frame*, enum libdecor_capabilities))
DEF_WL_FUNC(void, libdecor_frame_set_fullscreen, (struct libdecor_frame*, struct wl_output*))
DEF_WL_FUNC(void, libdecor_frame_set_max_content_size, (struct libdecor_frame*, int, int))
DEF_WL_FUNC(void, libdecor_frame_set_maximized, (struct libdecor_frame*))
DEF_WL_FUNC(void, libdecor_frame_set_min_content_size, (struct libdecor_frame*, int, int))
DEF_WL_FUNC(void, libdecor_frame_set_minimized, (struct libdecor_frame*))
DEF_WL_FUNC(void, libdecor_frame_set_title, (struct libdecor_frame*, const char*))
DEF_WL_FUNC(void, libdecor_frame_set_visibility, (struct libdecor_frame*, bool visible))
DEF_WL_FUNC(void, libdecor_frame_unref, (struct libdecor_frame*))
DEF_WL_FUNC(void, libdecor_frame_unset_capabilities, (struct libdecor_frame*, enum libdecor_capabilities))
DEF_WL_FUNC(void, libdecor_frame_unset_fullscreen, (struct libdecor_frame*))
DEF_WL_FUNC(void, libdecor_frame_unset_maximized, (struct libdecor_frame*))
DEF_WL_FUNC(void, libdecor_state_free, (struct libdecor_state*))
DEF_WL_FUNC(void, libdecor_unref, (struct libdecor*))
DEF_WL_SO_END

#undef DEF_WL_SO_BEGIN
#undef DEF_WL_SO_END
#undef DEF_WL_FUNC
