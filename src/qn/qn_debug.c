#include "pch.h"
#include "qs_qn.h"

#if _QN_WINDOWS_
#define debug_break()			DebugBreak()
#else
#define debug_break()			raise(SIGTRAP)
#endif

//
static struct DebugImpl
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
}
_qn_dbg =
{
	NULL,
	3,
	"QS",
};

//
void qn_debug_init(void)
{
#if _QN_WINDOWS_
	_qn_dbg.handle = GetStdHandle(STD_OUTPUT_HANDLE);
	_qn_dbg.debugger = IsDebuggerPresent();
#else
	_qn_dbg.fp = stdout;
#endif
}

//
void qn_debug_dispose(void)
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
static int qn_debug_out_str(const char* s)
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
static int qn_debug_out_ch(int ch)
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
static int qn_debug_out_trace(const char* head, const char* text)
{
	if (head == NULL)
		head = "unknown";
	int len = 0;
	len += qn_debug_out_ch('[');
	len += qn_debug_out_str(head);
	len += qn_debug_out_str("] ");
	len += qn_debug_out_str(text);
	return len;
}

///
int qn_debug_assert(const char* expr, const char* mesg, const char* filename, int line)
{
	qn_val_if_fail(expr, -1);
	qn_debug_out_str(expr);
	qn_debug_out_str(": ");
	if (mesg != NULL)
		qn_debug_out_str(mesg);
	qn_debug_out_str("(filename=\"");
	qn_debug_out_str(filename);
	qn_debug_out_str("\", line=");
	char sz[32];
	qn_itoa(sz, QN_COUNTOF(sz), line, 10);
	qn_debug_out_str(sz);
	qn_debug_out_ch('\n');

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
	qn_debug_out_str("HALT [");
	qn_debug_out_str(head);
	qn_debug_out_str("] ");
	if (mesg != NULL)
		qn_debug_out_str(mesg);
	qn_debug_out_ch('\n');

#ifndef __EMSCRIPTEN__
	if (_qn_dbg.debugger) debug_break();
	abort();
#endif
}

//
int qn_debug_outputs(bool breakpoint, const char* head, const char* mesg)
{
	const int len = qn_debug_out_trace(head, mesg);
	qn_debug_out_ch('\n');

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
	qn_debug_out_trace(head, buf);
	qn_debug_out_ch('\n');

#ifndef __EMSCRIPTEN__
	if (breakpoint && _qn_dbg.debugger)
		debug_break();
#endif

	return (int)size;
}

//
static char* qn_syserr_mesg(int errcode, int* size)
{
	const char s_unknown[] = "unknown error";
#ifdef _QN_WINDOWS_
	DWORD dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL, (DWORD)errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), NULL, 0, NULL) + 1;
	if (dw == 1)
	{
		if (size != NULL) *size = QN_COUNTOF(s_unknown) - 1;	// 널터미네이트 빼야함
		return qn_strdup(s_unknown);
	}
	wchar* pw = qn_alloc(dw, wchar);
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL, (DWORD)errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), pw, dw, NULL);
	pw[dw - 2] = L'\0';

	size_t len = qn_wcstombs(NULL, 0, pw, 0) + 1;
	char* buf = qn_alloc(len, char);
	qn_wcstombs(buf, len, pw, 0);
	qn_free(pw);

	if (size != NULL) *size = (int)len - 1;
	return buf;
#else
	char* psz = strerror(errcode);
	if (psz == NULL)
	{
		if (size != NULL) *size = QN_COUNTOF(s_unknown) - 1;	// 널터미네이트 빼야함
		return qn_strdup(s_unknown);
	}
	if (size != NULL) *size = (int)strlen(psz);
	return qn_strdup(psz);
#endif
}

//
char* qn_syserr(int errcode, int* len)
{
	if (errcode == 0)
#ifdef _QN_WINDOWS_
		errcode = GetLastError();
#else
		errno;
#endif
	if (errcode == 0)
		return NULL;
	return qn_syserr_mesg(errcode, len);
}

//
int qn_debug_output_syserr(bool breakpoint, const char* head, int errcode)
{
	char* ps = qn_syserr(errcode, NULL);
	const int len = qn_debug_out_trace(head, ps);
	qn_debug_out_ch('\n');
	qn_free(ps);

	if (breakpoint && _qn_dbg.debugger)
		debug_break();

	return len;
}

//
int qn_outputs(const char* mesg)
{
	const int len = qn_debug_out_str(mesg);
	qn_debug_out_ch('\n');
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
	qn_debug_out_str(buf);
	qn_debug_out_ch('\n');
	return (int)size;
}
