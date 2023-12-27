//
// qgwlx_stub.c - 웨이랜드 스터브
// 2023-12-23 by kim
//

#include "pch.h"
#ifdef USE_WAYLAND
#include "qs_qn.h"
#include "qs_qg.h"
#include "qs_kmc.h"

#ifdef _DEBUG
#define DEBUG_WLX_SO_TRACE
#endif

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

// 정적 함수 미리 선언
#pragma region 정적 함수 미리 선언
static void _wlx_display_registry(void);
#pragma endregion

#pragma region SO 처리
// SO 함수 정의
#define DEF_WLX_TYPEDEF
#define DEF_WLX_FUNC(ret,name,args)\
	typedef ret(*QN_CONCAT(PFN_, name)) args;\
	QN_CONCAT(PFN_, name) QN_CONCAT(wlx_, name);
#include "qgwlx_func.h"

// SO 함수
static void* wlx_so_func(QnModule* module, const char* so_name, const char* func_name)
{
	void* ret = qn_mod_func(module, func_name);
#ifdef DEBUG_WLX_SO_TRACE
	qn_debug_outputf(false, "WAYLAND STUB", "%s: '%s' in '%s'",
		ret == NULL ? "load failed" : "loaded", func_name, so_name);
#else
	QN_DUMMY(so_name);
#endif
	return ret;
}

// SO 초기화
static bool wlx_so_init(void)
{
	static bool loaded = false;
	qn_val_if_ok(loaded, true);
	QnModule* module;
	const char* so_name;
#define DEF_WLX_SO_BEGIN(name)\
	module = qn_mod_load(so_name = name".so", 1); if (module == NULL)\
	{ qn_debug_outputf(true, "WAYLAND STUB", "no '%s' found!", so_name); return false; } else {
#define DEF_WLX_SO_END	}
#define DEF_WLX_FUNC(ret,name,args)\
	QN_CONCAT(wlx_, name) = (QN_CONCAT(PFN_, name))wlx_so_func(module, so_name, QN_STRING(name));\
	if (QN_CONCAT(wlx_, name) == NULL) {\
		qn_debug_outputf(true, "WAYLAND STUB", "%s: '%s' in '%s'", "load failed", QN_STRING(NAME), so_name);\
		return false; }
#include "qgwlx_func.h"
	return loaded = true;
}
#pragma endregion SO 처리


//////////////////////////////////////////////////////////////////////////

#pragma region 스터브 선언
// 본편 시작
#include "qgwlx_stub.h"
#include "wayland/wayland-client-protocol.h"
#include "wayland/wayland-client-protocol-code.h"
#include "wayland/wayland-xdg-shell-client-protocol.h"
#include "wayland/wayland-xdg-shell-client-protocol-code.h"
#include "wayland/wayland-xdg-decoration-client-protocol.h"
#include "wayland/wayland-xdg-decoration-client-protocol-code.h"
#include "wayland/wayland-viewporter-client-protocol.h"
#include "wayland/wayland-viewporter-client-protocol-code.h"
#include "wayland/wayland-relative-pointer-unstable-v1-client-protocol.h"
#include "wayland/wayland-relative-pointer-unstable-v1-client-protocol-code.h"
#include "wayland/wayland-pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland/wayland-pointer-constraints-unstable-v1-client-protocol-code.h"
#include "wayland/wayland-idle-inhibit-unstable-v1-client-protocol.h"
#include "wayland/wayland-idle-inhibit-unstable-v1-client-protocol-code.h"

// 웨이랜드 스터브 여기 있다!
WaylandStub wlxStub;
#pragma endregion 스터브 선언

#pragma region 시스템 함수
//
bool stub_system_open(const char* title, const int display, const int width, const int height, QgFlag flags)
{
	qn_zero_1(&wlxStub);

	//
	if (wlx_so_init() == false)
	{
		qn_debug_outputs(true, "WAYLAND STUB", "so load failed");
		return false;
	}

	wlxStub.display = wl_display_connect(NULL);
	if (wlxStub.display == NULL)
	{
		qn_debug_outputs(true, "WAYLAND STUB", "cannot create display");
		return false;
	}

	wlxStub.registry = wl_display_get_registry(wlxStub.display);
	wlx_display_registry();

	//
	stub_initialize((StubBase*)&wlxStub, flags);


	return true;
}

//
void stub_system_finalize(void)
{
}

//
bool stub_system_poll(void)
{
	return QN_TMASK(wlxStub.base.stats, QGSSTT_EXIT) == false;
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
//
static const char* _wlx_tag_output = "qgwlx_stub_output";

//
static void _wlx_output_handle_geometry(void* data, struct wl_output* output,
	int x, int y, int physical_width, int physical_height, int subpixel,
	const char *make, const char *model, int transform)
{
	WaylandMonitor* mon = data;
	mon->base.x = x;
	mon->base.y = y;
	mon->base.mmwidth = physical_width < 0 ? 0 : physical_width;
	mon->base.mmheight = physical_height < 0 ? 0 : physical_height;
	mon->transform = (enum wl_output_transform)transform;

	if (*mon->base.name == '\0')
		qn_snprintf(mon->base.name, QN_COUNTOF(mon->base.name), "%s %s", make, model);
}

//
static void _wlx_output_handle_mode(void* data, struct wl_output* output,
	uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	if (QN_TMASK(flags, WL_OUTPUT_MODE_CURRENT) == false)
		return;

	WaylandMonitor* mon = data;
	mon->base.width = width;
	mon->base.height = height;
	mon->base.refresh = (uint)round(refresh / 1000.0);
}

//
static void _wlx_output_handle_done(void* data, struct wl_output* output)
{
	WaylandMonitor* mon = data;

	if (mon->base.mmwidth == 0 || mon->base.mmheight == 0)
	{
		mon->base.mmwidth = (int)(mon->base.width * 25.4f / 96.0f);
		mon->base.mmheight = (int)(mon->base.height * 25.4f / 96.0f);
	}

	int nth;
	qn_pctnr_contains(&wlxStub.base.monitors, mon, &nth);
	if (nth < 0)
		stub_event_on_monitor((QgUdevMonitor*)mon, true, false);
}

//
static void _wlx_output_handle_scale(void* data, struct wl_output* output, int32_t factor)
{
	WaylandMonitor* mon = data;
	mon->scale_factor = (float)factor;
}

#ifdef WL_OUTPUT_NAME_SINCE_VERSION
//
static void _wlx_output_handle_name(void* data, struct wl_output* output, const char* name)
{
	WaylandMonitor* mon = data;
	qn_strncpy(mon->base.name, QN_COUNTOF(mon->base.name), name, QN_COUNTOF(mon->base.name) - 1);
}

//
static void _wlx_output_handle_description(void* data, struct wl_output* output, const char* description)
{
	QN_DUMMY(data);
	QN_DUMMY(output);
	QN_DUMMY(description);
}
#endif

//
static void _wlx_add_display(uint32_t name, uint32_t version)
{
	qn_assert(version >= 2 && "not enought wayland version! need >= 2");
	struct wl_output* output = wl_registry_bind(wlxStub.registry, name, &wl_output_interface, 2);
	if (output == NULL)
	{
		qn_debug_outputs(true, "WAYLAND STUB", "failed to retrieve output(display)");
		return;
	}

	WaylandMonitor* mon = qn_alloc_zero_1(WaylandMonitor);
	mon->base.oshandle = output;
	mon->name = name;
	mon->scale_factor = 1.0f;

	wl_proxy_set_tag((struct wl_proxy*)output, &_wlx_tag_output);
	static const struct wl_output_listener output_listener =
	{
		_wlx_output_handle_geometry,
		_wlx_output_handle_mode,
		_wlx_output_handle_done,
		_wlx_output_handle_scale,
#ifdef WL_OUTPUT_NAME_SINCE_VERSION
		_wlx_output_handle_name,
		_wlx_output_handle_description,
#endif
	};
	wl_output_add_listener(output, &output_listener, mon);
}

//
static void _wlx_xdg_wm_base_pong(void* data, struct xdg_wm_base* wm_base, uint32_t serial)
{
	xdg_wm_base_pong(wm_base, serial);
}

//
static void _wlx_display_handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
	if (qn_streqv(interface, "wl_compositor"))
		wlxStub.compositor = wl_registry_bind(registry, name, &wl_compositor_interface, QN_MIN(3, version));
	else if (qn_streqv(interface, "wl_subcompositor"))
		wlxStub.subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
	else if (qn_streqv(interface, "wl_shm"))
		wlxStub.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	else if (qn_streqv(interface, "wl_output"))
		_wlx_add_display(name, version);
	else if (qn_streqv(interface, "wl_seat"))
	{
		if (wlxStub.seat == NULL)
		{
			wlxStub.seat = wl_registry_bind(registry, name, &wl_seat_interface, QN_MIN(4, version));
			//_glfwAddSeatListenerWayland(wlxStub.seat);
		}
	}
	else if (qn_streqv(interface, "wl_data_device_manager"))
	{
		if (!wlxStub.data_device_manager)
			wlxStub.data_device_manager = wl_registry_bind(registry, name, &wl_data_device_manager_interface, 1);
	}
	else if (qn_streqv(interface, "xdg_wm_base"))
	{
		wlxStub.wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		static const struct xdg_wm_base_listener wm_base_listener =
		{
			_wlx_xdg_wm_base_pong,
		};
		xdg_wm_base_add_listener(wlxStub.wm_base, &wm_base_listener, NULL);
	}
	else if (qn_streqv(interface, "zxdg_decoration_manager_v1"))
		wlxStub.decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
	else if (qn_streqv(interface, "wp_viewporter"))
		wlxStub.view_porter = wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
	else if (qn_streqv(interface, "zwp_relative_pointer_manager_v1"))
		wlxStub.relative_pointer_manager = wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
	else if (qn_streqv(interface, "zwp_pointer_constraints_v1"))
		wlxStub.pointer_constraints = wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1);
	else if (qn_streqv(interface, "zwp_idle_inhibit_manager_v1"))
		wlxStub.idle_inhibit_manager = wl_registry_bind(registry, name, &zwp_idle_inhibit_manager_v1_interface, 1);
}

//
static void _wlx_display_global_remove(void* data, struct wl_registry* registry, uint32_t name)
{
	WaylandMonitor* mon;
	qn_pctnr_each_item(&wlxStub.base.monitors, mon,
		{
			if (mon->name != name)
				continue;
			stub_event_on_monitor((QgUdevMonitor*)mon, false, false);
			return;
		});
}

// 디스플레이 레지스트리
static void _wlx_display_registry(void)
{
	static const struct wl_registry_listener registry_listener =
	{
		_wlx_display_handle_global,
		_wlx_display_global_remove,
	};
	wl_registry_add_listener(wlxStub.registry, &registry_listener, NULL);
}
#pragma endregion 레지스트리

#endif
