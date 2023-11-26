#include "pch.h"
#include "qn.h"

/*!
 * @brief 오류처리용 assert
 * @param	expr		표현
 * @param	mesg		오류 내용
 * @param	filename	파일의 파일 이름
 * @param	line		파일의 줄 번호
 * @return	int
 */
int qn_debug_assert(const char* expr, const char* mesg, const char* filename, int line)
{
	size_t len = qn_snprintf(NULL, 0, "filename=\"%s\", line=%d", filename, line);
	char* buf = qn_alloca(len + 1, char);
	qn_snprintf(buf, len + 1, "filename=\"%s\", line=%d", filename, line);

#if _QN_WINDOWS_
	OutputDebugStringA(buf);
	if (mesg)
		OutputDebugStringA(mesg);
	if (expr)
		OutputDebugStringA(expr);
	if (IsDebuggerPresent())
		DebugBreak();
#else
	fputs(buf, stderr);
	if (mesg)
		fputs(mesg, stderr);
	if (expr)
		fputs(expr, stderr);
	raise(SIGTRAP);
#endif

	return 0;
}

/*!
 * @brief HALT 메시지
 * @param	cls	클래스 이름
 * @param	msg	출력할 메시지
 */
int qn_debug_halt(const char* cls, const char* msg)
{
	char* buf;

	if (!cls)
		cls = "unknown";

	if (msg)
	{
		const char* fmt = "HALT [%s] %s\n";
		size_t len = qn_snprintf(NULL, 0, fmt, cls, msg);
		buf = qn_alloca(len + 1, char);
		qn_snprintf(buf, len + 1, fmt, cls, msg);
	}
	else
	{
		const char* fmt = "HALT [%s]\n";
		size_t len = qn_snprintf(NULL, 0, fmt, cls);
		buf = qn_alloca(len + 1, char);
		qn_snprintf(buf, len + 1, fmt, cls);
	}

#if _QN_WINDOWS_
	OutputDebugStringA(buf);
	if (IsDebuggerPresent())
		DebugBreak();
#else
	fputs(buf, stderr);
	raise(SIGTRAP);
#endif

	qn_freea(buf);
	qn_exit(255);

	return 0;
}
