//
// qgwin_func.h - WINDOWS 함수
// 2023-12-23 by kim
//

#ifndef DEF_WIN_DLL_BEGIN
#define DEF_WIN_DLL_BEGIN(name)
#endif
#ifndef DEF_WIN_DLL_END
#define DEF_WIN_DLL_END
#endif
#ifndef DEF_WIN_FUNC
#define DEF_WIN_FUNC(ret,name,args)
#endif
#ifndef DEF_WIN_XIFUNC
#define DEF_WIN_XIFUNC(ret,name,args)
#endif

// XINPUT
DEF_WIN_XIFUNC(DWORD, XInputGetState, (_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState))
DEF_WIN_XIFUNC(DWORD, XInputSetState, (_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration))
DEF_WIN_XIFUNC(DWORD, XInputGetCapabilities, (_In_ DWORD dwUserIndex, _In_ DWORD dwFlags, _Out_ XINPUT_CAPABILITIES* pCapabilities))
DEF_WIN_XIFUNC(DWORD, XInputGetBatteryInformation, (_In_ DWORD dwUserIndex, _In_ BYTE devType, _Out_ XINPUT_BATTERY_INFORMATION* pBatteryInformation))

// NTDLL
/*
DEF_WIN_DLL_BEGIN(NTDLL)
DEF_WIN_DLL_END
*/

// USER32
DEF_WIN_DLL_BEGIN(USER32)
DEF_WIN_FUNC(BOOL, SetProcessDPIAware, (void))
DEF_WIN_FUNC(BOOL, SetProcessDpiAwarenessContext, (DPI_AWARENESS_CONTEXT))
DEF_WIN_FUNC(DPI_AWARENESS_CONTEXT, SetThreadDpiAwarenessContext, (DPI_AWARENESS_CONTEXT))
DEF_WIN_FUNC(DPI_AWARENESS_CONTEXT, GetThreadDpiAwarenessContext, (void))
DEF_WIN_FUNC(DPI_AWARENESS, GetAwarenessFromDpiAwarenessContext, (DPI_AWARENESS_CONTEXT))
DEF_WIN_FUNC(BOOL, EnableNonClientDpiScaling, (HWND))
DEF_WIN_FUNC(BOOL, AdjustWindowRectExForDpi, (LPRECT, DWORD, BOOL, DWORD, UINT))
DEF_WIN_FUNC(UINT, GetDpiForWindow, (HWND))
DEF_WIN_FUNC(BOOL, AreDpiAwarenessContextsEqual, (DPI_AWARENESS_CONTEXT, DPI_AWARENESS_CONTEXT))
DEF_WIN_FUNC(BOOL, IsValidDpiAwarenessContext, (DPI_AWARENESS_CONTEXT))
DEF_WIN_FUNC(BOOL, EnableNonClientDpiScaling, (HWND))
DEF_WIN_FUNC(LRESULT, CallNextHookEx, (HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam))
DEF_WIN_FUNC(int, GetSystemMetricsForDpi, (int, UINT))
DEF_WIN_FUNC(BOOL, ChangeWindowMessageFilterEx, (HWND, UINT, DWORD, PCHANGEFILTERSTRUCT))
DEF_WIN_FUNC(BOOL, UnhookWindowsHookEx, (HHOOK hhk))
DEF_WIN_FUNC(HHOOK, SetWindowsHookExW, (int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId))
DEF_WIN_DLL_END

// SHCORE
DEF_WIN_DLL_BEGIN(SHCORE)
DEF_WIN_FUNC(HRESULT, GetDpiForMonitor, (HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*))
DEF_WIN_FUNC(HRESULT, SetProcessDpiAwareness, (PROCESS_DPI_AWARENESS))
DEF_WIN_DLL_END

// IMM32
DEF_WIN_DLL_BEGIN(IMM32)
DEF_WIN_FUNC(BOOL, ImmAssociateContextEx, (HWND, HIMC, DWORD))
DEF_WIN_DLL_END

#undef DEF_WIN_DLL_BEGIN
#undef DEF_WIN_DLL_END
#undef DEF_WIN_FUNC
#undef DEF_WIN_XIFUNC
