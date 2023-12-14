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
	QN_PADDING(2, 0)
}
_qn_dbg =
{
	NULL,
	3,
	"QS",
};

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
	int len = 0;
	len += qn_dbg_out_ch('[');
	len += qn_dbg_out_str(head);
	len += qn_dbg_out_str("] ");
	len += qn_dbg_out_str(text);
	return len;
}

///
int qn_debug_assert(const char* expr, const char* mesg, const char* filename, int line)
{
	qn_val_if_fail(expr, -1);
	qn_dbg_out_str(expr);
	qn_dbg_out_str(": ");
	if (mesg != NULL)
		qn_dbg_out_str(mesg);
	qn_dbg_out_str("(filename=\"");
	qn_dbg_out_str(filename);
	qn_dbg_out_str("\", line=");
	char sz[32];
	qn_itoa(sz, QN_COUNTOF(sz), line, 10);
	qn_dbg_out_str(sz);
	qn_dbg_out_ch('\n');

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
	if (head == NULL)
		head = "unknown";
	qn_dbg_out_str("HALT [");
	qn_dbg_out_str(head);
	qn_dbg_out_str("] ");
	if (mesg != NULL)
		qn_dbg_out_str(mesg);
	qn_dbg_out_ch('\n');

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
	char buf[1024];
	qn_vsnprintf(buf, QN_COUNTOF(buf), fmt, va);
	va_end(va);
	qn_dbg_out_trace(head, buf);
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
	char buf[1024];
	qn_vsnprintf(buf, QN_COUNTOF(buf), fmt, va);
	va_end(va);
	qn_dbg_out_str(buf);
	qn_dbg_out_ch('\n');
	return (int)size;
}
