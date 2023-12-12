#include "pch.h"
#include "qs_qn.h"

#if _QN_WINDOWS_
#define debug_break()			DebugBreak()
#else
#define debug_break()			raise(SIGTRAP)
#endif

//
static struct QnDebugImpl
{
#if _QN_WINDOWS_
	HANDLE			handle;
#else
	FILE*			fp;
#endif

	int				level;
	char			tag[32];

	bool			debugger;
	bool			redirect;
} _qn_dbg = { NULL, 3, "qs", };  // NOLINT

//
void qn_dbg_init(void)
{
#if _QN_WINDOWS_
	_qn_dbg.handle = GetStdHandle(STD_OUTPUT_HANDLE);
	_qn_dbg.debugger = IsDebuggerPresent();
#else
	_qn_dbg.fp = stdout;
#endif
}

//
void qn_dbg_dispose(void)
{
#if _QN_WINDOWS_
	if (_qn_dbg.redirect && _qn_dbg.handle != NULL)
		CloseHandle(_qn_dbg.handle);
#else
	if (_qn_dbg.redirect && _qn_dbg.fp != NULL)
		fclose(_qn_dbg.fp);
#endif
}

//
static int qn_dbg_out_str(const char* s)
{
#if _QN_WINDOWS_
	DWORD len = (DWORD)strlen(s);
	if (_qn_dbg.handle != NULL)
	{
		DWORD wtn;
		if (_qn_dbg.redirect || WriteConsoleA(_qn_dbg.handle, s, len, &wtn, NULL) == 0)
			WriteFile(_qn_dbg.handle, s, len, &wtn, NULL);
	}
	if (_qn_dbg.debugger)
		OutputDebugStringA(s);
#else
	int len = strlen(s);
	if (_qn_dbg.fp != NULL)
		fputs(s, _qn_dbg.fp);
#if _QN_ANDROID_
	if (_qn_dbg.debugger)
		__android_log_print(ANDROID_LOG_VERBOSE, _qn_dbg.tag, s);
#endif
#endif
	return (int)len;
}

//
static int qn_dbg_out_ch(int ch)
{
#if _QN_WINDOWS_
	if (_qn_dbg.handle != NULL)
	{
		DWORD wtn;
		if (_qn_dbg.redirect || WriteConsoleA(_qn_dbg.handle, &ch, 1, &wtn, NULL) == 0)
			WriteFile(_qn_dbg.handle, &ch, 1, &wtn, NULL);
	}
	if (_qn_dbg.debugger)
	{
		const char sz[2] = { (char)ch, '\0' };
		OutputDebugStringA(sz);
	}
#else
	if (_qn_dbg.fp != NULL)
		fputc(ch, _qn_dbg.fp);
#if _QN_ANDROID_
	if (_qn_dbg.debugger)
		__android_log_print(ANDROID_LOG_VERBOSE, _qn_dbg.tag, "%c", ch);
#endif
#endif

	return 1;
}

//
static int qn_dbg_out_trace(const char* head, const char* text)
{
	if (head == NULL)
		head = "unknown";

	const char* fmt = "[%s] %s";
	const size_t len = qn_snprintf(NULL, 0, fmt, head, text);
	char* buf = qn_alloca(len + 1, char);
	qn_snprintf(buf, len + 1, fmt, head, text);
	qn_dbg_out_str(buf);
	qn_freea(buf);

	return (int)len;
}

///
int qn_debug_assert(const char* expr, const char* mesg, const char* filename, int line)
{
	qn_retval_if_fail(expr, -1);

	if (!mesg)
		mesg = "";

	const char* fmt = "%s: %s(filename=\"%s\", line=%d)\n";
	const size_t len = qn_snprintf(NULL, 0, fmt, expr, mesg, filename, line);
	char* buf = qn_alloca(len + 1, char);
	qn_snprintf(buf, len + 1, fmt, expr, mesg, filename, line);
	qn_dbg_out_str(buf);
	qn_freea(buf);

#ifndef __EMSCRIPTEN__
	if (_qn_dbg.debugger) debug_break();
#endif

	return 0;
}

//
#if _MSC_VER
__declspec(noreturn)
#endif
void qn_debug_halt(const char* head, const char* mesg)
{
	if (!head)
		head = "unknown";
	if (!mesg)
		mesg = "";

	const char* fmt = "HALT [%s] %s\n";
	const size_t len = qn_snprintf(NULL, 0, fmt, head, mesg);
	char* buf = qn_alloca(len + 1, char);
	qn_snprintf(buf, len + 1, fmt, head, mesg);
	qn_dbg_out_str(buf);
	qn_freea(buf);

#ifndef __EMSCRIPTEN__
	if (_qn_dbg.debugger) debug_break();
	abort();
#endif
}

//
int qn_debug_outputs(bool breakpoint, const char* head, const char* mesg)
{
	const int len = qn_dbg_out_trace(head, mesg);
	qn_dbg_out_ch('\n');

	if (breakpoint && _qn_dbg.debugger)
		debug_break();

	return len;
}

//
int qn_debug_outputf(bool breakpoint, const char* head, const char* fmt, ...)
{
	va_list va, vq;
	va_start(va, fmt);
	va_copy(vq, va);
	const size_t size = qn_vsnprintf(NULL, 0, fmt, vq);
	va_end(vq);
	char* buf = qn_alloca(size + 1, char);
	qn_vsnprintf(buf, size + 1, fmt, va);
	va_end(va);
	qn_dbg_out_trace(head, buf);
	qn_freea(buf);
	qn_dbg_out_ch('\n');

#ifndef __EMSCRIPTEN__
	if (breakpoint && _qn_dbg.debugger)
		debug_break();
#endif

	return (int)size;
}

//
int qn_outputs(const char* mesg)
{
	const int len = qn_dbg_out_str(mesg);
	qn_dbg_out_ch('\n');
	return len;
}

//
int qn_outputf(const char* fmt, ...)
{
	va_list va, vq;
	va_start(va, fmt);
	va_copy(vq, va);
	const size_t size = qn_vsnprintf(NULL, 0, fmt, vq);
	va_end(vq);
	char* buf = qn_alloca(size + 1, char);
	qn_vsnprintf(buf, size + 1, fmt, va);
	va_end(va);
	qn_dbg_out_str(buf);
	qn_freea(buf);
	qn_dbg_out_ch('\n');
	return (int)size;
}
