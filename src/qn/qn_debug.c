#include "pch.h"
#include "qs_qn.h"

#if _QN_WINDOWS_
#define debug_break()			DebugBreak()
#else
#define debug_break()			raise(SIGTRAP)
#endif

#ifndef MAX_DEBUG_LENGTH
#define MAX_DEBUG_LENGTH	1024
#endif
static_assert(MAX_DEBUG_LENGTH < 4096, "MAX_DEBUG_LENGTH must be less than 4096");

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
} debug_impl = { NULL, "QS", };

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
static int qn_debug_out_str(const char* restrict s)
{
	int len = (int)strlen(s);
	if (len + debug_impl.out_pos > MAX_DEBUG_LENGTH - 1)
		len = MAX_DEBUG_LENGTH - debug_impl.out_pos - 1;
	if (len <= 0)
		return 0;
	memcpy(debug_impl.out_buf + debug_impl.out_pos, s, len);
	debug_impl.out_pos += len;
	return len;
}

//
static int qn_debug_out_ch(int ch)
{
	if (1 + debug_impl.out_pos > MAX_DEBUG_LENGTH - 1)
		return 0;
	debug_impl.out_buf[debug_impl.out_pos++] = (char)ch;
	return 1;
}

//
static void qn_debug_out_flush(void)
{
	qn_ret_if_fail(debug_impl.out_pos > 0);
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
	debug_impl.out_pos = 0;
}

//
static int qn_debug_out_trace(const char* restrict head, const char* restrict text)
{
	if (head == NULL)
		head = "unknown";
	int len = 0;
	len += qn_debug_out_ch('[');
	len += qn_debug_out_str(head);
	len += qn_debug_out_str("] ");
	len += qn_debug_out_str(text);
	qn_debug_out_flush();
	return len;
}

///
int qn_debug_assert(const char* restrict expr, const char* restrict mesg, const char* restrict filename, int line)
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
	qn_debug_out_flush();

#ifndef __EMSCRIPTEN__
	if (debug_impl.debugger)
		debug_break();
#endif
	return 0;
	}

//
noreturn void qn_debug_halt(const char* restrict head, const char* restrict mesg)
{
	if (head == NULL)
		head = "unknown";
	qn_debug_out_str("HALT [");
	qn_debug_out_str(head);
	qn_debug_out_str("] ");
	if (mesg != NULL)
		qn_debug_out_str(mesg);
	qn_debug_out_ch('\n');
	qn_debug_out_flush();

#ifndef __EMSCRIPTEN__
	if (debug_impl.debugger)
		debug_break();
#endif
	abort();
}

//
int qn_debug_outputs(bool breakpoint, const char* restrict head, const char* restrict mesg)
{
	const int len = qn_debug_out_trace(head, mesg);
	qn_debug_out_ch('\n');
	qn_debug_out_flush();

#ifndef __EMSCRIPTEN__
	if (breakpoint && debug_impl.debugger)
		debug_break();
#endif
	return len;
}

//
int qn_debug_outputf(bool breakpoint, const char* restrict head, const char* restrict fmt, ...)
{
	va_list va, vq;
	va_start(va, fmt);
	va_copy(vq, va);
	const int size = qn_vsnprintf(NULL, 0, fmt, vq);
	va_end(vq);
	char buf[1024];
	qn_vsnprintf(buf, QN_COUNTOF(buf), fmt, va);
	va_end(va);
	qn_debug_out_trace(head, buf);
	qn_debug_out_ch('\n');
	qn_debug_out_flush();

#ifndef __EMSCRIPTEN__
	if (breakpoint && debug_impl.debugger)
		debug_break();
#endif
	return size;
}

//
int qn_debug_output_error(bool breakpoint, const char* head)
{
	const char* err = qn_get_error();
	const int len = qn_debug_out_trace(head, err);
	qn_debug_out_ch('\n');

	if (breakpoint && debug_impl.debugger)
		debug_break();
	return len;
}

//
int qn_outputs(const char* mesg)
{
	const int len = qn_debug_out_str(mesg);
	qn_debug_out_ch('\n');
	qn_debug_out_flush();
	return len;
}

//
int qn_outputf(const char* fmt, ...)
{
	va_list va, vq;
	va_start(va, fmt);
	va_copy(vq, va);
	const int size = qn_vsnprintf(NULL, 0, fmt, vq);
	va_end(vq);
	char buf[1024];
	qn_vsnprintf(buf, QN_COUNTOF(buf), fmt, va);
	va_end(va);
	qn_debug_out_str(buf);
	qn_debug_out_ch('\n');
	qn_debug_out_flush();
	return size;
}
