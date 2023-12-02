#include "pch.h"
#include "qn.h"

#if _QN_WINDOWS_
#define debug_break()			DebugBreak()
#else
#define debug_break()			raise(SIGTRAP)
#endif

//
static struct qnDebugImpl
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
} _qn_dbg = { NULL, 3, "qs", false };

//
void _qn_dbg_init(void)
{
#if _QN_WINDOWS_
	_qn_dbg.handle = GetStdHandle(STD_OUTPUT_HANDLE);
	_qn_dbg.debugger = IsDebuggerPresent();
#else
	_qn_dbg.fp = stdout;
#endif
}

//
void _qn_dbg_dispose(void)
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
static int _qn_dbg_out_str(const char* s)
{
#if _QN_WINDOWS_
	DWORD len = (DWORD)strlen(s);
	if (_qn_dbg.handle != NULL)
	{
		DWORD wtn;
		if (_qn_dbg.redirect)
			WriteFile(_qn_dbg.handle, s, len, &wtn, NULL);
		else if (WriteConsoleA(_qn_dbg.handle, s, len, &wtn, NULL) == 0)
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
static int _qn_dbg_out_ch(int ch)
{
#if _QN_WINDOWS_
	if (_qn_dbg.handle != NULL)
	{
		DWORD wtn;
		if (_qn_dbg.redirect)
			WriteFile(_qn_dbg.handle, &ch, 1, &wtn, NULL);
		else if (WriteConsoleA(_qn_dbg.handle, &ch, 1, &wtn, NULL) == 0)
			WriteFile(_qn_dbg.handle, &ch, 1, &wtn, NULL);
	}
	if (_qn_dbg.debugger)
	{
		char sz[2] = { (char)ch, '\0' };
		OutputDebugStringA(sz);
	}
#else
	if (_qn_dbg.fp != NULL)
		fputs(s, _qn_dbg.fp);
#if _QN_ANDROID_
	if (_qn_dbg.debugger)
		__android_log_print(ANDROID_LOG_VERBOSE, _qn_dbg.tag, "%c", ch);
#endif
#endif

	return 1;
}

//
static int _qn_dbg_out_trace(const char* head, const char* text)
{
	if (head == NULL)
		head = "unknown";

	const char* fmt = "[%s] %s";
	size_t len = qn_snprintf(NULL, 0, fmt, head, text);
	char* buf = qn_alloca(len + 1, char);
	qn_snprintf(buf, len + 1, fmt, head, text);
	_qn_dbg_out_str(text);
	qn_freea(buf);

	return (int)len;
}

/**
 * @brief 오류처리용 assert
 * @param	expr		표현
 * @param	mesg		오류 내용
 * @param	filename	파일의 파일 이름
 * @param	line		파일의 줄 번호
 * @return	int
 */
int qn_debug_assert(const char* expr, const char* mesg, const char* filename, int line)
{
	qn_retval_if_fail(expr, -1);

	if (!mesg)
		mesg="";

	const char* fmt = "%s: %s(filename=\"%s\", line=%d)\n";
	size_t len = qn_snprintf(NULL, 0, fmt, expr, mesg, filename, line);
	char* buf = qn_alloca(len + 1, char);
	qn_snprintf(buf, len + 1, fmt, expr, mesg, filename, line);
	_qn_dbg_out_str(buf);
	qn_freea(buf);

	if (_qn_dbg.debugger) debug_break();

	return 0;
}

/**
 * @brief HALT 메시지
 * @param	cls	클래스 이름
 * @param	msg	출력할 메시지
 */
void qn_debug_halt(const char* cls, const char* msg)
{
	char* buf;

	if (!cls)
		cls = "unknown";
	if (!msg)
		msg ="";

	const char* fmt = "HALT [%s] %s\n";
	size_t len = qn_snprintf(NULL, 0, fmt, cls, msg);
	buf = qn_alloca(len + 1, char);
	qn_snprintf(buf, len + 1, fmt, cls, msg);
	_qn_dbg_out_str(buf);
	qn_freea(buf);

	if (_qn_dbg.debugger) debug_break();
	abort();
}

int qn_debug_outputs(bool breakpoint, const char* head, const char* mesg)
{
	int len = _qn_dbg_out_trace(head, mesg);
	_qn_dbg_out_ch('\n');

	if (breakpoint && _qn_dbg.debugger)
		debug_break();

	return len;
}

int qn_debug_outputf(bool breakpoint, const char* head, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	size_t size = qn_vsnprintf(NULL, 0, fmt, va);
	char* buf = qn_alloca(size + 1, char);
	qn_vsnprintf(buf, size + 1, fmt, va);
	va_end(va);
	_qn_dbg_out_trace(head, buf);
	qn_freea(buf);
	_qn_dbg_out_ch('\n');

	if (breakpoint && _qn_dbg.debugger)
		debug_break();

	return (int)size;
}

int qn_outputs(const char* mesg)
{
	int len = _qn_dbg_out_str(mesg);
	_qn_dbg_out_ch('\n');
	return len;
}

int qn_outputf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	size_t size = qn_vsnprintf(NULL, 0, fmt, va);
	char* buf = qn_alloca(size + 1, char);
	qn_vsnprintf(buf, size + 1, fmt, va);
	va_end(va);
	_qn_dbg_out_str(buf);
	qn_freea(buf);
	_qn_dbg_out_ch('\n');
	return (int)size;
}
