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
typedef struct wl_data_offer					wl_data_offer;
typedef struct wl_output						wl_output;
typedef struct wl_pointer						wl_pointer;
typedef struct wl_proxy							wl_proxy;
typedef struct wl_region						wl_region;
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
typedef struct zxdg_output_v1					zxdg_output_v1;
typedef struct zxdg_output_manager_v1			zxdg_output_manager_v1;
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
#include "wayland-xdg-output-unstable-v1.h"
#include "wayland-wayland-code.h"
#include "wayland-xdg-shell-code.h"
#include "wayland-xdg-output-unstable-v1-code.h"

typedef enum wl_output_transform			wl_output_transform;


//////////////////////////////////////////////////////////////////////////
// 스터브 선언

//
typedef struct nkk_window
{
	wl_surface*			surface;
	xdg_surface*		layer;
	xdg_toplevel*		toplevel;

	char*				title;
	char*				class_name;

	QmSize				size;
	QmSize				window_size;
	struct
	{
		QmSize				size;
	}					pending;
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

	wl_output*			output;
	zxdg_output_v1*		zxdg_output;
	wl_output_transform	transform;

	uint32_t			name;
	int					mode;
	int					scale_factor;

	char				description[32];
} nkk_monitor;

//
typedef struct nkk_offer_item
{
	wl_data_offer*		offer;
	bool				is_utf8;
	bool				is_url;
} nkk_offer_item;
QN_DECL_CTNR(nkk_offer_ctnr, nkk_offer_item);

//
typedef struct nkk_data_offer
{
	nkk_offer_ctnr		offers;
	wl_data_offer*		selection_offer;
	wl_data_offer*		drag_offer;
	uint32_t			drag_offer_serial;
} nkk_data_offer;

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
	zxdg_output_manager_v1*				zxdg_output_manager;	// 쓸필요 없는거 같은데...
	wl_data_device_manager*				data_device_manager;	// 끌어 넣기
	wl_data_device*						data_device;			// 끌어 받기

	// 언젠간 쓰겠지
	zxdg_decoration_manager_v1*			___decoration_manager;
	wp_viewporter*						___view_porter;
	zwp_relative_pointer_manager_v1*	___relative_pointer_manager;
	zwp_pointer_constraints_v1*			___pointer_constraints;
	zwp_idle_inhibit_manager_v1*		___idle_inhibit_manager;
} nkk_interface;

//
typedef struct WaylandStub
{
	StubBase			base;

	const char*			tag;
	nkk_interface		ifce;

	nkk_keyboard		keyboard;
	nkk_pointer			pointer;
	nkk_data_offer		offer;
	nkk_cursor			cursor;
	nkk_window			win;
} WaylandStub;

// 여기야 여기!
WaylandStub wlxStub;


//////////////////////////////////////////////////////////////////////////
// 웨이랜드 이벤트

#define WL_DESTROY(var,func)		QN_STMT_BEGIN{ if (var) func(var); }QN_STMT_END
#define WL_DESTROY_CLEAR(var,func)	QN_STMT_BEGIN{ if (var) { func(var); var=NULL; } }QN_STMT_END
#ifdef _DEBUG
#define WL_TRACE(...)				qn_debug_outputf(false, "WLX STUB", __VA_ARGS__)
#define WL_TRACE_FUNC()				qn_debug_outputf(false, "WLX STUB", "%llu: %s", qn_tick(), __FUNCTION__);
#else
#define WL_TRACE(...)
#define WL_TRACE_FUNC()
#endif

//
static void set_opaque_region(void)
{
	nkk_window* win = &wlxStub.win;
	wl_region* region = wl_compositor_create_region(wlxStub.ifce.compositor);
	qn_ret_if_ok(region);
	wl_region_add(region, 0, 0, win->width, win->height);
	wl_surface_set_opaque_region(win->surface, region);
	wl_region_destroy(region);
}

//
static void resize_window(void)
{
	//nkk_window* win = &wlxStub.win;
	//int width = (int)win->width * win->content_scale;
	//int height = (int)win->height * win->content_scale;
	set_opaque_region();
}

//
static void configure_geometry(void)
{
	nkk_window* win = &wlxStub.win;
	bool size_changed = win->pending.size.width != win->size.width || win->pending.size.width!=win->size.height;

	wl_surface_set_buffer_scale(win->surface, 1);

	if (size_changed)
	{
		xdg_surface_set_window_geometry(win->layer, 0, 0, win->pending.size.width, win->pending.size.height);

		wl_region* region = wl_compositor_create_region(wlxStub.ifce.compositor);
		wl_region_add(region, 0, 0, win->pending.size.width, win->pending.size.height);
		wl_surface_set_opaque_region(win->surface, region);
		wl_region_destroy(region);
	}

	QmSize min, max;
	if (QN_TMASK(wlxStub.base.flags, QGFLAG_FULLSCREEN))
	{
		qm_set2(&min, 0, 0);
		qm_set2(&max, 0, 0);
	}
	else if (QN_TMASK(wlxStub.base.flags, QGFLAG_RESIZE))
	{
		QgUdevMonitor* mon = qn_pctnr_nth(&wlxStub.base.monitors, wlxStub.base.display);
		qm_set2(&min, 256, 256);
		qm_set2(&max, mon->width, mon->height);
	}
	xdg_toplevel_set_min_size(win->toplevel, min.width, min.height);
	xdg_toplevel_set_max_size(win->toplevel, max.width, max.height);

	win->size = win->pending.size;
	stub_event_on_window_event(QGWEV_SIZED, win->size.width, win->size.height);

	if (size_changed)
		stub_event_on_window_event(QGWEV_PAINTED, 0, 0);
}

//
static void surface_enter(void* data, wl_surface* surface, wl_output* output)
{
	WL_TRACE_FUNC();
	qn_ret_if_fail(wl_proxy_get_tag((wl_proxy*)output) == &wlxStub.tag);
	nkk_monitor* mon = (nkk_monitor*)wl_output_get_user_data(output);
	qn_ret_if_fail(mon != NULL);
}

//
static void surface_leave(void* data, wl_surface* surface, wl_output* output)
{
	WL_TRACE_FUNC();
	qn_ret_if_fail(wl_proxy_get_tag((wl_proxy*)output) == &wlxStub.tag);
}

//
static void xdg_surface_configure(void *data, struct xdg_surface *surface, uint32_t serial)
{
	WL_TRACE_FUNC();
	xdg_surface_ack_configure(surface, serial);

	/*
	nkk_window* win = &wlxStub.win;
	if (QN_TMASK(wlxStub.base.stats, QGSST_ACTIVE) != win->pending.active)
		QN_SMASK(&wlxStub.base.stats, QGSST_ACTIVE, win->pending.active);

	if (QN_TBIT(wlxStub.base.window_stats, QGWEV_MAXIMIZED) != win->pending.maximized)
		stub_event_on_window_event(QGWEV_MAXIMIZED, 0, 0);

	int width = win->pending.width;
	int height = win->pending.height;

	if (win->width != width || win->height != height)
	{
		win->width = width;
		win->height = height;
		resize_window();
		stub_event_on_window_event(QGWEV_SIZED, width, height);
	}
	*/
}

// 윈도우가 닫히면 (사실은 디스플레이의 마지막 윈도우가 닫히면)
static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg)
{
	WL_TRACE_FUNC();
	qg_exit_loop();
}

//
static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg, int32_t width, int32_t height, struct wl_array *states)
{
	WL_TRACE_FUNC();
	/*
	nkk_window* win = &wlxStub.win;
	win->pending.active = false;
	win->pending.maximized = false;
	win->pending.fullscreen = false;

	uint32_t* s;
	wl_array_for_each(s, states)
	{
		switch (*s)
		{
			case XDG_TOPLEVEL_STATE_MAXIMIZED:
				win->pending.maximized = true;
				break;
			case XDG_TOPLEVEL_STATE_FULLSCREEN:
				win->pending.fullscreen = true;
				break;
			case XDG_TOPLEVEL_STATE_RESIZING:
				break;
			case XDG_TOPLEVEL_STATE_ACTIVATED:
				win->pending.active = true;
				break;
			default:
				break;
		}
	}

	if (width && height)
	{
		win->pending.width = width;
		win->pending.height = height;
	}
	*/
}

//
static void data_offer_offer(void* data, wl_data_offer* offer, const char* mime_type)
{
	size_t i;
	qn_ctnr_foreach(&wlxStub.offer.offers, i)
	{
		nkk_offer_item* item = &qn_ctnr_nth(&wlxStub.offer.offers, i);
		if (item->offer != offer)
			continue;

		if (strcmp(mime_type, "text/plain;charset=utf-8") == 0)
			item->is_utf8 = true;
		else if (strcmp(mime_type, "text/uri-list") == 0)
			item->is_url = true;
		break;
	}
}

//
static void data_device_data_offer(void* data, wl_data_device* device, wl_data_offer* offer)
{
	static const struct wl_data_offer_listener events_data_offer =
	{
		.offer = data_offer_offer,
	};

	qn_ctnr_expand(nkk_offer_ctnr, &wlxStub.offer.offers, 1);
	nkk_offer_item* p = &qn_ctnr_inv(&wlxStub.offer.offers, 0);
	qn_zero_1(p);
	p->offer = offer;
	wl_data_offer_add_listener(offer, &events_data_offer, NULL);
}

//
static void data_device_enter(void* data, wl_data_device* device, uint32_t serial,
	wl_surface* surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer* offer)
{
	WL_DESTROY_CLEAR(wlxStub.offer.drag_offer, wl_data_offer_destroy);

	size_t i;
	qn_ctnr_foreach(&wlxStub.offer.offers, i)
	{
		nkk_offer_item* item = &qn_ctnr_nth(&wlxStub.offer.offers, i);
		if (item->offer != offer)
			continue;

		if (surface == wlxStub.win.surface && item->is_url)
		{
			wlxStub.offer.drag_offer = offer;
			wlxStub.offer.drag_offer_serial = serial;
		}

		qn_ctnr_remove_nth(nkk_offer_ctnr, &wlxStub.offer.offers, i);
		break;
	}

	if (wlxStub.offer.drag_offer)
		wl_data_offer_accept(offer, serial, "text/uri-list");
	else
	{
		wl_data_offer_accept(offer, serial, NULL);
		wl_data_offer_destroy(offer);
	}
}

//
static void data_device_leave(void* data, wl_data_device* device)
{
	WL_DESTROY_CLEAR(wlxStub.offer.drag_offer, wl_data_offer_destroy);
}

//
static void data_device_motion(void* data, wl_data_device* device, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
}

//
static void data_device_drop(void* data, wl_data_device* device)
{
	qn_ret_if_fail(wlxStub.offer.drag_offer);
	// TODO: 데이터 받기 해야하는데.. 일단 패스
}

//
static void data_device_selection(void* data, wl_data_device* device, wl_data_offer* offer)
{
	WL_DESTROY_CLEAR(wlxStub.offer.selection_offer, wl_data_offer_destroy);

	size_t i;
	qn_ctnr_foreach(&wlxStub.offer.offers, i)
	{
		nkk_offer_item* item = &qn_ctnr_nth(&wlxStub.offer.offers, i);
		if (item->offer != offer)
			continue;

		if (item->is_utf8)
			wlxStub.offer.selection_offer = offer;
		else
			wl_data_offer_destroy(offer);

		qn_ctnr_remove_nth(nkk_offer_ctnr, &wlxStub.offer.offers, i);
		break;
	}
}

//
static void output_geometry(void* data, wl_output* output,
	int32_t x, int32_t y, int32_t physical_width, int physical_height, int32_t subpixel,
	const char *make, const char *model, int32_t transform)
{
	WL_TRACE_FUNC();
	nkk_monitor* mon = data;
	mon->base.x = (uint)x;
	mon->base.y = (uint)y;
	mon->base.mmwidth = physical_width < 0 ? 0 : (uint)physical_width;
	mon->base.mmheight = physical_height < 0 ? 0 : (uint)physical_height;
	mon->transform = (enum wl_output_transform)transform;

	if (*mon->base.name == '\0')
		qn_snprintf(mon->base.name, QN_COUNTOF(mon->base.name), "%s %s", make, model);
}

//
static void output_mode(void* data, wl_output* output,
	uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	WL_TRACE_FUNC();
	if (QN_TMASK(flags, WL_OUTPUT_MODE_CURRENT) == false)
		return;

	nkk_monitor* mon = data;
	mon->base.width = (uint)width;
	mon->base.height = (uint)height;
	mon->base.refresh = (uint)round(refresh / 1000.0);
}

//
static void output_done(void* data, struct wl_output* output)
{
	WL_TRACE_FUNC();
	nkk_monitor* mon = data;

	if (mon->base.mmwidth == 0 || mon->base.mmheight == 0)
	{
		mon->base.mmwidth = (uint)((float)mon->base.width * 25.4f / 96.0f);
		mon->base.mmheight = (uint)((float)mon->base.height * 25.4f / 96.0f);
	}

	int nth;
	qn_pctnr_contains(&wlxStub.base.monitors, (QgUdevMonitor*)mon, &nth);
	if (nth < 0)
		stub_event_on_monitor((QgUdevMonitor*)mon, true, false);
}

//
static void output_scale(void* data, struct wl_output* output, int32_t factor)
{
	WL_TRACE_FUNC();
	nkk_monitor* mon = data;
	mon->scale_factor = factor;
}

#ifdef WL_OUTPUT_NAME_SINCE_VERSION
//
static void output_name(void* data, struct wl_output* output, const char* name)
{
	WL_TRACE_FUNC();
	nkk_monitor* mon = data;
	qn_strncpy(mon->base.name, QN_COUNTOF(mon->base.name), name, QN_COUNTOF(mon->base.name) - 1);
}

//
static void output_description(void* data, struct wl_output* output, const char* description)
{
	WL_TRACE_FUNC();
	QN_DUMMY(data);
	QN_DUMMY(output);
	QN_DUMMY(description);
}
#endif

//
static void zxdg_output_logical_position(void* data, zxdg_output_v1* zxdg_output, int32_t x, int32_t y)
{
	WL_TRACE_FUNC();
	nkk_monitor* mon = (nkk_monitor*)data;
	mon->base.x = (uint)x;
	mon->base.y = (uint)y;
}

//
static void zxdg_output_logical_size(void* data, zxdg_output_v1* zxdg_output, int32_t width, int32_t height)
{
	WL_TRACE_FUNC();
	nkk_monitor* mon = (nkk_monitor*)data;
	mon->base.width = (uint)width;
	mon->base.height = (uint)height;
}

//
static void zxdg_output_done(void* data, zxdg_output_v1* zxdg_output)
{
	WL_TRACE_FUNC();
	nkk_monitor* mon = (nkk_monitor*)data;
	if (zxdg_output_v1_get_version(mon->zxdg_output) < 3)
		output_done(data, mon->output);
}

//
static void zxdg_output_name(void* data, zxdg_output_v1* zxdg_output, const char* name)
{
	WL_TRACE_FUNC();
}

//
static void zxdg_output_description(void* data, zxdg_output_v1* zxdg_output, const char* description)
{
	WL_TRACE_FUNC();
	nkk_monitor* mon = (nkk_monitor*)data;
	if (wl_output_get_version(mon->output) < WL_OUTPUT_DESCRIPTION_SINCE_VERSION)
		qn_strncpy(mon->description, QN_COUNTOF(mon->description), description, QN_COUNTOF(mon->description) - 1);
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
	const double now = wlxStub.base.timer->abstime;
	const double delta = now - wlxStub.base.active;
	wlxStub.base.active = now;
	QN_SMASK(&wlxStub.base.stats, QGSST_ACTIVE, true);
	stub_event_on_window_event(QGWEV_FOCUS, 0, 0);
	stub_event_on_active(true, delta);
}

// 키보드 제거 때 또는 포커스 잃음 둘 중 하나일듯
static void keyboard_leave(void* data, wl_keyboard* wl_keyboard, uint32_t serial, wl_surface* surface)
{
	WL_TRACE_FUNC();
	struct itimerspec ts = { 0, };
	timerfd_settime(wlxStub.keyboard.repeat.fd, 0, &ts, NULL);
	wlxStub.keyboard.serial = serial;

	// 포커스 잃음이라면 여기서 메시지
	const double now = wlxStub.base.timer->abstime;
	const double delta = now - wlxStub.base.active;
	wlxStub.base.active = now;
	QN_SMASK(&wlxStub.base.stats, QGSST_ACTIVE, false);
	stub_event_on_window_event(QGWEV_LOSTFOCUS, 0, 0);
	stub_event_on_active(false, delta);
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
	stub_event_on_keyboard((QikKey)keycode, down);

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
static void seat_capabilities(void *data, wl_seat *wl_seat, uint32_t capabilities)
{
	WL_TRACE_FUNC();

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

//
static void monitor_clean(QgUdevMonitor* monitor)
{
	nkk_monitor* mon = (nkk_monitor*)monitor;
	WL_DESTROY(mon->zxdg_output, zxdg_output_v1_destroy);
	if (wl_output_get_version(mon->output) >= WL_OUTPUT_RELEASE_SINCE_VERSION)
		wl_output_release(mon->output);
	else
		wl_output_destroy(mon->output);
}

//
static void registry_global_remove(void* data, wl_registry* reg, uint32_t name)
{
	size_t i;
	qn_pctnr_foreach(&wlxStub.base.monitors, i)
	{
		nkk_monitor* mon = (nkk_monitor*)qn_pctnr_nth(&wlxStub.base.monitors, i);
		if (mon->name != name)
			continue;

		monitor_clean((QgUdevMonitor*)mon);
		stub_event_on_monitor((QgUdevMonitor*)mon, false, false);
		break;
	}
}

//
#define CHECK_WAYLAND_VERSION(major, minor)\
	(WAYLAND_VERSION_MAJOR > major || (WAYLAND_VERSION_MAJOR == major && WAYLAND_VERSION_MINOR >= minor))
#if CHECK_WAYLAND_VERSION(1, 22)
#define WAYLAND_COMPOSITOR_VERSION		6
#else
#define WAYLAND_COMPOSITOR_VERSION		4
#endif
#if CHECK_WAYLAND_VERSION(1, 20)
#define WAYLAND_OUTPUT_VERSION			4
#else
#define WAYLAND_OUTPUT_VERSION			3
#endif
#if CHECK_WAYLAND_VERSION(1, 22)
#define WAYLAND_SEAT_VERSION			9
#elif CHECK_WAYLAND_VERSION(1, 21)
#define WAYLAND_SEAT_VERSION			8
#else
#define	WAYLAND_SEAT_VERSION			5
#endif

//
static void registry_global(void *data, wl_registry *reg, uint32_t name, const char *iface, uint32_t ver)
{
	WL_TRACE("레지스트리: %s (version: %u)", iface, ver);

	static const struct wl_output_listener events_output =
	{
		.geometry = output_geometry,
		.mode = output_mode,
		.done = output_done,
		.scale = output_scale,
#if defined WL_OUTPUT_NAME_SINCE_VERSION && WL_OUTPUT_NAME_SINCE_VERSION >= 4
		.name = output_name,
		.description = output_description,
#endif
	};
	static const struct wl_seat_listener events_seat =
	{
		.capabilities = seat_capabilities,
		.name = seat_name,
	};
	static const struct zxdg_output_v1_listener events_zxdg_output =
	{
		.logical_position = zxdg_output_logical_position,
		.logical_size = zxdg_output_logical_size,
		.done = zxdg_output_done,
		.name = zxdg_output_name,
		.description = zxdg_output_description,
	};
	static const struct xdg_wm_base_listener events_xdg_shell =
	{
		.ping = xdg_shell_pong,
	};

	if (strcmp(iface, "wl_compositor") == 0)				// 컴포지터
		wlxStub.ifce.compositor = wl_registry_bind(reg, name, &wl_compositor_interface, QN_MIN(WAYLAND_COMPOSITOR_VERSION, ver));
	else if (strcmp(iface, "wl_shm") == 0)					// 메모리 처리
		wlxStub.ifce.shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
	else if (strcmp(iface, "wl_seat") == 0)					// 입력
	{
		if (wlxStub.ifce.seat == NULL)
		{
			// 7 마으스 됨, 4 키보드 됨, 5 둘다 됨
			wlxStub.ifce.seat = wl_registry_bind(reg, name, &wl_seat_interface, QN_MIN(WAYLAND_SEAT_VERSION, ver));
			wl_seat_add_listener(wlxStub.ifce.seat, &events_seat, NULL);
		}
	}
	else if (qn_streqv(iface, "wl_output"))					// 출력 장치 (모니터)
	{
		wl_output* output = wl_registry_bind(reg, name, &wl_output_interface, QN_MIN(WAYLAND_OUTPUT_VERSION, ver));
		nkk_monitor* mon = qn_alloc_zero_1(nkk_monitor);
		mon->output = output;
		mon->name = name;
		mon->scale_factor = 1.0f;
		wl_proxy_set_tag((wl_proxy*)output, &wlxStub.tag);
		wl_output_add_listener(output, &events_output, mon);

		if (wlxStub.ifce.zxdg_output_manager != NULL)
		{
			mon->zxdg_output = zxdg_output_manager_v1_get_xdg_output(wlxStub.ifce.zxdg_output_manager, mon->output);
			zxdg_output_v1_add_listener(mon->zxdg_output, &events_zxdg_output, mon);
		}
	}
	else if (strcmp(iface, "zxdg_output_manager_v1") == 0)	// 출력 장치 (모니터)
	{
		wlxStub.ifce.zxdg_output_manager = wl_registry_bind(reg, name, &zxdg_output_manager_v1_interface, QN_MIN(3, ver));
		size_t i;
		qn_pctnr_foreach(&wlxStub.base.monitors, i)
		{
			nkk_monitor* mon = (nkk_monitor*)qn_pctnr_nth(&wlxStub.base.monitors, i);
			mon->zxdg_output = zxdg_output_manager_v1_get_xdg_output(wlxStub.ifce.zxdg_output_manager, mon->output);
			zxdg_output_v1_add_listener(mon->zxdg_output, &events_zxdg_output, mon);
		}
	}
	else if (strcmp(iface, "xdg_wm_base") == 0)				// XDG 쉘 윈도우
	{
		wlxStub.ifce.xdg_shell = wl_registry_bind(reg, name, &xdg_wm_base_interface, QN_MIN(6, ver));
		xdg_wm_base_add_listener(wlxStub.ifce.xdg_shell, &events_xdg_shell, NULL);
	}
	else if (strcmp(iface, "wl_data_device_manager") == 0)	// 끌어다 놓기
		wlxStub.ifce.data_device_manager = wl_registry_bind(reg, name, &wl_data_device_manager_interface, QN_MIN(1, ver));
}


//////////////////////////////////////////////////////////////////////////
// 웨이런댜 구현

// 실행 파일 이름
const char* get_program_name(void)
{
	static const char* proc_name = NULL;
	if (proc_name == NULL)
	{
#if defined _QN_FREEBSD_ || defined _QN_LINUX_
		static char link[1024];
#if defined _QN_FREEBSD_
		const char* path = "/proc/curproc/file";
#else
		const char* path = "/proc/self/exe";
#endif
		int size = readlink(path, link, QN_COUNTOF(link) - 1);
		if (size > 0)
		{
			link[size] = '\0';
			proc_name = strrchr(link, '/');
			if (proc_name)
				++proc_name;
			else
				proc_name = link;
		}
#else
		proc_name = "qg_wlx_stub";
#endif
	}
	return proc_name;
}

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
	static const struct wl_registry_listener events_registry =
	{
		.global = registry_global,
		.global_remove = registry_global_remove,
	};
	static const struct wl_data_device_listener events_data_device =
	{
		.data_offer = data_device_data_offer,
		.enter = data_device_enter,
		.leave = data_device_leave,
		.motion = data_device_motion,
		.drop = data_device_drop,
		.selection = data_device_selection,
	};

	if (!(wlxStub.ifce.display = wl_display_connect(NULL)))
		return false;

	wlxStub.ifce.registry = wl_display_get_registry(wlxStub.ifce.display);
	wl_registry_add_listener(wlxStub.ifce.registry, &events_registry, NULL);
	wlxStub.keyboard.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	wl_display_roundtrip(wlxStub.ifce.display);		// 레지스트리 에빈트
	wl_display_roundtrip(wlxStub.ifce.display);		// 아웃풋 이벤트

	wlxStub.keyboard.repeat.fd = wl_seat_get_version(wlxStub.ifce.seat) < WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION ?
		-1 : timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);

	qn_ctnr_init(nkk_offer_ctnr, &wlxStub.offer.offers, 0);
	if (wlxStub.ifce.data_device_manager != NULL && wlxStub.ifce.seat != NULL)
	{
		wlxStub.ifce.data_device = wl_data_device_manager_get_data_device(
			wlxStub.ifce.data_device_manager, wlxStub.ifce.seat);
		wl_data_device_add_listener(wlxStub.ifce.data_device, &events_data_device, NULL);
	}

	wlxStub.cursor.theme = wl_cursor_theme_load(NULL, 24, wlxStub.ifce.shm);
	nkk_set_system_cursor("left_ptr");

	return true;
}

// 디스플레이 닫기
static void nkk_display_close(void)
{
	qn_ret_if_fail(wlxStub.ifce.display);
	size_t i;

	qn_ctnr_foreach(&wlxStub.offer.offers, i)
		wl_data_offer_destroy(qn_ctnr_nth(&wlxStub.offer.offers, i).offer);
	qn_ctnr_disp(&wlxStub.offer.offers);

	qn_pctnr_foreach_1(&wlxStub.base.monitors, monitor_clean);

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
static bool nkk_window_open(nkk_window* win, const nkk_monitor* monitor, const char* title, int32_t width, int32_t height)
{
	static const struct wl_surface_listener events_surface =
	{
		.enter = surface_enter,
		.leave = surface_leave,
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

	qn_val_if_fail(wlxStub.ifce.display, false);
	qn_val_if_fail(wlxStub.ifce.xdg_shell, false);

	// 크기
	const QmSize scrsize = { (int)monitor->base.width, (int)monitor->base.height };
	QmSize clientsize;
	if (width > 256 && height > 256)
		qm_set2(&clientsize, width, height);
	else
	{
		if (scrsize.height > 800)
			qm_set2(&clientsize, 1280, 720);
		else
			qm_set2(&clientsize, 720, 450);
	}
	QmPoint pos =
	{
		(int)monitor->base.x + (scrsize.width - clientsize.width) / 2,
		(int)monitor->base.y + (scrsize.height - clientsize.height) / 2,
	};

	// 값설정
	qm_rect_set_pos_size(&wlxStub.base.window_bound, &pos, &clientsize);
	wlxStub.base.client_size = clientsize;

	win->size = clientsize;
	win->window_size = clientsize;
	win->pending.size = clientsize;
	win->title = qn_strdup(title && *title ? title : "QS");
	win->class_name = qn_strdup(get_program_name());

	// 서피스
	win->surface = wl_compositor_create_surface(wlxStub.ifce.compositor);
	wl_proxy_set_tag((wl_proxy*)win->surface, &wlxStub.tag);
	wl_surface_add_listener(win->surface, &events_surface, NULL);

	// XDG 서피스
	win->layer = xdg_wm_base_get_xdg_surface(wlxStub.ifce.xdg_shell, win->surface);
	xdg_surface_add_listener(win->layer, &events_xdg_surface, NULL);

	win->toplevel = xdg_surface_get_toplevel(win->layer);
	xdg_toplevel_add_listener(win->toplevel, &events_xdg_toplevel, NULL);

	xdg_toplevel_set_app_id(win->toplevel, win->class_name);
	xdg_toplevel_set_title(win->toplevel, win->title);

	wl_surface_commit(win->surface);
	wl_display_roundtrip(wlxStub.ifce.display);

	//
	configure_geometry();

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

	qn_free(win->title);
	qn_free(win->class_name);
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
					uint32_t scancode = wlxStub.keyboard.repeat.scancode;
					stub_event_on_keyboard((QikKey)scancode + 8, true);
					keyboard_text(scancode);
				}
			}
			loop = false;
		}

		// 커서
		// libdecor
	}

	return true;
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

	wlxStub.tag = "qg_wlx_stub";

	// 먼저 안하면 안된다
	stub_initialize((StubBase*)&wlxStub, flags);

	//
	if (nkk_display_open() == false)
	{
		qn_debug_outputs(true, "WLX STUB", "cannot open display");
		return false;
	}

	//
	const nkk_monitor* monitor = (const nkk_monitor*)qn_pctnr_nth(&wlxStub.base.monitors,
		(size_t)display < qn_pctnr_count(&wlxStub.base.monitors) ? display : 0);
	nkk_window_open(&wlxStub.win, monitor, title, width, height);

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
