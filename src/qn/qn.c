//
// qn.c - QN 시작 지점
// 2023-12-27 by kim
//

#include "pch.h"
#include "qs_qn.h"
#include "qs_ctn.h"

//
extern void qn_cycle_up(void);
extern void qn_mpf_up(void);
extern void qn_mpf_down(void);
extern void qn_debug_up(void);
extern void qn_debug_down(void);
extern void qn_module_up(void);
extern void qn_module_down(void);
extern void qn_thread_up(void);
extern void qn_thread_down(void);

// 프로퍼티
QN_DECL_MUKUM(QnPropMukum, char*, char*);
QN_MUKUM_CHAR_PTR_KEY(QnPropMukum)
QN_MUKUM_KEY_FREE(QnPropMukum)
QN_MUKUM_VALUE_FREE(QnPropMukum)

// 닫아라
struct Closure
{
	struct Closure* prev;
	funcparam_t		fp;
};

// 구현
static struct RuntimeImpl
{
	bool32			inited;
#ifndef USE_NO_LOCK
	QnSpinLock		lock;
#endif

	struct Closure* closures;
	struct Closure* preclosures;

	QnTls			error;

	QnPropMukum		props;
} runtime_impl =
{
	.inited = false,
};

#if !defined _LIB && !defined _STATIC
// 공유 라이브러리용 그냥 더미
static void qn_runtime_attach(void)
{
}
#endif

// 런타임 내림
static void qn_runtime_down(void)
{
	qn_ret_if_fail(runtime_impl.inited);
	runtime_impl.inited = false;

	QN_LOCK(runtime_impl.lock);
	for (struct Closure *prev, *node = runtime_impl.closures; node; node = prev)
	{
		prev = node->prev;
		node->fp.func(node->fp.data);
		qn_free(node);
	}

	for (struct Closure *prev, *node = runtime_impl.preclosures; node; node = prev)
	{
		prev = node->prev;
		node->fp.func(node->fp.data);
		qn_free(node);
	}
	QN_UNLOCK(runtime_impl.lock);

	qn_mukum_disp(QnPropMukum, &runtime_impl.props);

	qn_thread_down();
	qn_module_down();
	qn_mpf_down();
	qn_debug_down();
}

// 런타임 올림
static void qn_runtime_up(void)
{
	runtime_impl.inited = true;

	qn_cycle_up();
	qn_debug_up();
	qn_mpf_up();
	qn_module_up();
	qn_thread_up();

	runtime_impl.error = qn_tls(qn_memfre);

	qn_mukum_init(QnPropMukum, &runtime_impl.props);

#if defined _LIB || defined _STATIC
	(void)atexit(qn_runtime_down);
#endif
}

// 
void qn_runtime(void)
{
	qn_ret_if_ok(runtime_impl.inited);
	qn_runtime_up();
}

//
const char* qn_version(void)
{
#define MAKE_VERSION_STRING(a,b)	"QS VERSION " QN_STRING(a) "." QN_STRING(b)
	static const char* version_string = MAKE_VERSION_STRING(QN_VERSION_MAJOR, QN_VERSION_MINER);
	return version_string;
#undef MAKE_VERSION_STRING
}

//
void qn_atexit(paramfunc_t func, void* data)
{
	qn_ret_if_fail(runtime_impl.inited);
	qn_ret_if_fail(func);

	struct Closure* node = qn_alloc_1(struct Closure);
	qn_ret_if_fail(node);

	node->fp.func = func;
	node->fp.data = data;

	QN_LOCK(runtime_impl.lock);
	node->prev = runtime_impl.closures;
	runtime_impl.closures = node;
	QN_UNLOCK(runtime_impl.lock);
}

//
void qn_p_atexit(paramfunc_t func, void* data)
{
	qn_ret_if_fail(runtime_impl.inited);
	qn_ret_if_fail(func);

	struct Closure* node = qn_alloc_1(struct Closure);
	qn_ret_if_fail(node);

	node->fp.func = func;
	node->fp.data = data;

	QN_LOCK(runtime_impl.lock);
	node->prev = runtime_impl.preclosures;
	runtime_impl.preclosures = node;
	QN_UNLOCK(runtime_impl.lock);
}

//
size_t qn_p_index(void)
{
	static size_t s = 0;
	return ++s;
}

//
void qn_set_prop(const char* restrict name, const char* restrict value)
{
	qn_ret_if_fail(runtime_impl.inited);
	qn_ret_if_fail(name != NULL);
	QN_LOCK(runtime_impl.lock);
	if (value == NULL || *value == '\0')
		qn_mukum_remove(QnPropMukum, &runtime_impl.props, name, NULL);
	else
		qn_mukum_set(QnPropMukum, &runtime_impl.props, qn_strdup(name), qn_strdup(value));
	QN_UNLOCK(runtime_impl.lock);
}

//
const char* qn_get_prop(const char* name)
{
	qn_val_if_fail(runtime_impl.inited, NULL);
	qn_val_if_fail(name != NULL, NULL);
	char** ret;
	QN_LOCK(runtime_impl.lock);
	qn_mukum_get(QnPropMukum, &runtime_impl.props, name, &ret);
	QN_UNLOCK(runtime_impl.lock);
	return ret == NULL ? NULL : *ret;
}

//
const char* qn_get_error(void)
{
	const char* mesg = (const char*)qn_tlsget(runtime_impl.error);
	return mesg;
}

//
void qn_set_error(const char* mesg)
{
	char* prev = (char*)qn_tlsget(runtime_impl.error);
	qn_free(prev);
	qn_tlsset(runtime_impl.error, qn_strdup(mesg));
}

//
bool qn_set_syserror(const int errcode)
{
	char* prev = (char*)qn_tlsget(runtime_impl.error);
	qn_free(prev);

#ifdef _QN_WINDOWS_
	DWORD ec = errcode == 0 ? GetLastError() : (DWORD)errcode;
#else
	int ec = errcode == 0 ? errno : errcode;
#endif
	if (ec == 0)
	{
		qn_tlsset(runtime_impl.error, NULL);
		return false;	// 에러가 없다
	}

	char* buf;
#ifdef _QN_WINDOWS_
	DWORD dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), NULL, 0, NULL) + 1;
	if (dw == 1)
		buf = qn_apsprintf("unknown error: %u", ec);
	else
	{
		wchar* pw = qn_alloc(dw, wchar);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), pw, dw, NULL);
		dw -= 2;
		pw[dw] = L'\0';
		buf = qn_wcstombs_dup(pw, dw); // u16to8이 좋을지도 모르겠지만 윈도우 기본 출력은 멀티바이트 스트링
		qn_free(pw);
	}
#else
	const char* psz = strerror(ec);
	buf = psz == NULL ? qn_apsprintf("unknown error: %d", ec) : qn_strdup(psz);
#endif
	qn_tlsset(runtime_impl.error, buf);
	return true;
}

//
#if !defined _LIB || !defined _STATIC
#ifdef _QN_WINDOWS
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			qn_runtime_attach();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			qn_runtime_down();
			break;
	}
	return true;
}
#elif defined __GNUC__
void __attribute__((constructor)) _attach(void)
{
	qn_runtime_attach();
}

void __attribute__((destructor)) _detach(void)
{
	qn_runtime_down();
}
#endif
#endif
