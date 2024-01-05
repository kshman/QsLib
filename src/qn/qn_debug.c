//
// qn_debug.c - 디버그 출력 관리자
// 2023-12-27 by kim
//

#include "pch.h"
#include "qs_qn.h"

#ifdef DEBUG_BREAK
#error macro _DEBUG_BREAK already defined!
#endif
#ifdef _QN_WINDOWS_
#define DEBUG_BREAK(x)			if (x) DebugBreak()
#elif defined __EMSCRIPTEN__
#define DEBUG_BREAK(x)
#else
#define DEBUG_BREAK(x)			if (x) raise(SIGTRAP)
#endif

//
static struct DebugImpl
{
#ifdef _QN_WINDOWS_
	HANDLE			handle;
#else
	FILE*			fp;
#endif

	char			tag[32];
	char			out_buf[MAX_DEBUG_LENGTH];
	int				out_pos;

	bool			debugger;
	bool			redirect;
	bool			__dummy1;
	bool			__dummy2;
} debug_impl =
{
#ifdef _QN_WINDOWS_
	.handle = NULL,
#else
	.fp = NULL,
#endif
	.tag = "QS",
	.out_buf = "",
	.out_pos = 0,
	.debugger = false,
	.redirect = false,
};

//
void qn_debug_up(void)
{
#ifdef _QN_WINDOWS_
	debug_impl.handle = GetStdHandle(STD_OUTPUT_HANDLE);
	debug_impl.debugger = IsDebuggerPresent();
#else
	debug_impl.fp = stdout;
#endif
}

//
void qn_debug_down(void)
{
#ifdef _QN_WINDOWS_
	if (debug_impl.redirect && debug_impl.handle != NULL)
		CloseHandle(debug_impl.handle);
#else
	if (debug_impl.redirect && debug_impl.fp != NULL)
		fclose(debug_impl.fp);
#endif
}

//
static void qn_dbg_buf_ch(const int ch)
{
	if (1 + debug_impl.out_pos > MAX_DEBUG_LENGTH - 1)
		return;
	debug_impl.out_buf[debug_impl.out_pos++] = (char)ch;
}

//
static void qn_dbg_buf_str(const char* restrict s)
{
	int len = (int)strlen(s);
	if (len + debug_impl.out_pos > MAX_DEBUG_LENGTH - 1)
		len = MAX_DEBUG_LENGTH - debug_impl.out_pos - 1;
	if (len <= 0)
		return;
	memcpy(debug_impl.out_buf + debug_impl.out_pos, s, (size_t)len);
	debug_impl.out_pos += len;
}

//
static void qn_dbg_buf_va(const char* restrict fmt, va_list va)
{
	const int len = qn_vsnprintf(debug_impl.out_buf + debug_impl.out_pos, MAX_DEBUG_LENGTH - (size_t)debug_impl.out_pos, fmt, va);
	debug_impl.out_pos += len;
}

//
static void qn_dbg_buf_int(const int value)
{
	const int len = qn_itoa(debug_impl.out_buf + debug_impl.out_pos, value, 10, true);
	debug_impl.out_pos += len;
}

//
static void qn_dbg_buf_head(const char* restrict head)
{
	if (head == NULL)
		head = "(unknown)";
	qn_dbg_buf_ch('[');
	qn_dbg_buf_str(head);
	qn_dbg_buf_str("] ");
}

//
static int qn_dbg_buf_flush(void)
{
	qn_val_if_fail(debug_impl.out_pos > 0, 0);
	debug_impl.out_buf[debug_impl.out_pos] = '\0';
#ifdef _QN_WINDOWS_
	if (debug_impl.handle != NULL)
	{
		DWORD wtn;
		if (debug_impl.redirect || WriteConsoleA(debug_impl.handle, debug_impl.out_buf, debug_impl.out_pos, &wtn, NULL) == 0)
			WriteFile(debug_impl.handle, debug_impl.out_buf, debug_impl.out_pos, &wtn, NULL);
	}
	if (debug_impl.debugger)
		OutputDebugStringA(debug_impl.out_buf);
#else
	if (debug_impl.fp != NULL)
		fputs(debug_impl.out_buf, debug_impl.fp);
#ifdef __EMSCRIPTEN__
	emscripten_console_log(debug_impl.out_buf);
#endif
#ifdef _QN_ANDROID_
	__android_log_print(ANDROID_LOG_VERBOSE, debug_impl.tag, debug_impl.out_buf);
#endif
#endif
	const int ret = debug_impl.out_pos;
	debug_impl.out_pos = 0;
	return ret;
}

//
int qn_debug_assert(const char* restrict expr, const char* restrict filename, const int line)
{
	qn_val_if_fail(expr, -1);
	qn_dbg_buf_str("ASSERT FAILED : ");
	qn_dbg_buf_str(expr);
	qn_dbg_buf_str(" (filename=\"");
	qn_dbg_buf_str(filename);
	qn_dbg_buf_str("\", line=");
	qn_dbg_buf_int(line);
	qn_dbg_buf_ch('\n');
	qn_dbg_buf_flush();

	DEBUG_BREAK(debug_impl.debugger);
	return 0;
}

//
_Noreturn void qn_debug_halt(const char* restrict head, const char* restrict mesg)
{
	qn_dbg_buf_str("HALT ");
	qn_dbg_buf_head(head);
	qn_dbg_buf_str(mesg);
	qn_dbg_buf_ch('\n');
	qn_dbg_buf_flush();

	DEBUG_BREAK(debug_impl.debugger);
	abort();
}

//
int qn_debug_outputs(const bool breakpoint, const char* restrict head, const char* restrict mesg)
{
	qn_dbg_buf_head(head);
	qn_dbg_buf_str(mesg);
	qn_dbg_buf_ch('\n');
	const int len = qn_dbg_buf_flush();

	DEBUG_BREAK(breakpoint && debug_impl.debugger);
	return len;
}

//
int qn_debug_outputf(const bool breakpoint, const char* restrict head, const char* restrict fmt, ...)
{
	qn_dbg_buf_head(head);
	va_list va;
	va_start(va, fmt);
	qn_dbg_buf_va(fmt, va);
	va_end(va);
	qn_dbg_buf_ch('\n');
	const int len = qn_dbg_buf_flush();

	DEBUG_BREAK(breakpoint && debug_impl.debugger);
	return len;
}

//
int qn_debug_output_error(const bool breakpoint, const char* head)
{
	const char* err = qn_get_error();
	return qn_debug_outputs(breakpoint, head, err);
}

//
int qn_outputs(const char* mesg)
{
	qn_dbg_buf_str(mesg);
	qn_dbg_buf_ch('\n');
	return qn_dbg_buf_flush();
}

//
int qn_outputf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	qn_dbg_buf_va(fmt, va);
	va_end(va);
	qn_dbg_buf_ch('\n');
	return qn_dbg_buf_flush();
}
