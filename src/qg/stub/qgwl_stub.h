//
// qgwl_stub.h - Wayland 스터브
// 2023-12-23 by kim
//

#pragma once

#include "qg_stub.h"

// 길어서 줄이자
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

//
typedef struct nkk_window
{
	uint32_t			width, height;
	wl_surface*			surface;
	wl_surface*			cursor_surface;
	wl_cursor_image*	cursor_image;
	xdg_surface*		layer;
	xdg_toplevel*		toplevel;
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
typedef struct nkk_repeat
{
	int					fd;
	uint32_t			rate;
	uint32_t			delay;
	uint32_t			key;
} nkk_repeat;

//
typedef struct nkk_pointer_event
{
	uint32_t			serial;
	wl_surface*			surface;
	wl_fixed_t			kx, ky;
	double				axis_x, axis_y;
	bool				axis;
	bool				hover;
} nkk_pointer_event;

// 웨이랜드 모니터
typedef struct WaylandMonitor
{
	QgUdevMonitor		base;

	uint32_t			name;
	int					mode;
	float				scale_factor;

	uint32_t			transform;	// wl_output_transform
} WaylandMonitor;

// 웨이랜드 스터브
typedef struct WaylandStub
{
	StubBase			base;
	const char*			tag;

	wl_display*			display;
	wl_registry*		registry;
	wl_compositor*		compositor;
	wl_subcompositor*	subcompositor;
	wl_shm*				shm;
	wl_seat*			seat;
	wl_pointer*			pointer;
	wl_keyboard*		keyboard;
	xdg_wm_base*		xdg_shell;

	wl_cursor_theme*	cursor_theme;

	nkk_xkb				xkb;
	nkk_repeat			repeat;
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

// 웨이랜드 스터브 선언
extern WaylandStub wlStub;
