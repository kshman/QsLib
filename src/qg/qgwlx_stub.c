//
// qgwlx_stub.c - 웨이랜드 스터브
// 2023-12-23 by kim
//

#include "pch.h"
#include "qs_qn.h"
#ifdef USE_WAYLAND
#include "qs_qg.h"
#include "qs_kmc.h"
#include <wayland-cursor.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <libdecor-0/libdecor.h>

#ifdef _DEBUG
#define DEBUG_WLX_SO_TRACE
#endif

// 정적 함수 미리 선언
#pragma region 정적 함수 미리 선언
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
	qn_debug_outputf(false, "WAYLAND STUB", "\t%s: '%s' in '%s'",
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
	QN_CONCAT(wlx_, name) = (QN_CONCAT(PFN_, name))wlx_so_func(module, so_name, QN_STRING(name));
#include "qgwlx_func.h"
	return loaded = true;
}
#pragma endregion SO 처리


//////////////////////////////////////////////////////////////////////////

#pragma region 스터브 선언
// 본편 시작
#include "qgwlx_stub.h"

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

#endif
