//
// qn.c - QN 시작 지점
// 2023-12-27 by kim
//

#include "pch.h"

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

// 심볼
QN_DECL_MUKUM(QnSymMukum, char*, nint);
QN_MUKUM_CHAR_PTR_KEY(QnSymMukum)
QN_MUKUM_KEY_FREE(QnSymMukum)
QN_MUKUM_VALUE_NONE(QnSymMukum)

// 닫아라
typedef struct CLOSURE
{
	struct CLOSURE* prev;
	funcparam_t		fp;
} Closure;

// 구현
static struct RUNTIMEIMPL
{
	bool32			inited;
#ifndef QS_NO_SPINLOCK
	QnSpinLock		lock;
#endif

	Closure*		closures;
	Closure*		preclosures;

	QnPropMukum		props;
	QnSymMukum		symbols;
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
	qn_return_when_fail(runtime_impl.inited,/*void*/);
	runtime_impl.inited = false;

	QN_LOCK(runtime_impl.lock);
	for (Closure *prev, *node = runtime_impl.closures; node; node = prev)
	{
		prev = node->prev;
		node->fp.func(node->fp.data);
		qn_free(node);
	}

	for (Closure *prev, *node = runtime_impl.preclosures; node; node = prev)
	{
		prev = node->prev;
		node->fp.func(node->fp.data);
		qn_free(node);
	}
	QN_UNLOCK(runtime_impl.lock);

	qn_mukum_disp(QnPropMukum, &runtime_impl.props);
	qn_mukum_disp(QnSymMukum, &runtime_impl.symbols);

	qn_thread_down();
	qn_module_down();
	qn_mpf_down();
	qn_debug_down();
}

// 런타임 올림
static void qn_runtime_up(void)
{
	runtime_impl.inited = true;

#ifdef _QN_WINDOWS_
	// 콘솔창을 UTF-8로 설정!!!!!!!!
	SetConsoleOutputCP(65001);
#endif

	qn_cycle_up();
	qn_debug_up();
	qn_mpf_up();
	qn_module_up();
	qn_thread_up();

	qn_mukum_init(QnPropMukum, &runtime_impl.props);

#if defined _LIB || defined _STATIC
	(void)atexit(qn_runtime_down);
#endif
}

// 
void qn_runtime(void)
{
	qn_return_on_ok(runtime_impl.inited,/*void*/);
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
	qn_return_when_fail(runtime_impl.inited,/*void*/);
	qn_return_when_fail(func,/*void*/);

	Closure* node = qn_alloc_1(Closure);
	qn_return_when_fail(node,/*void*/);

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
	qn_return_when_fail(runtime_impl.inited,/*void*/);
	qn_return_when_fail(func,/*void*/);

	Closure* node = qn_alloc_1(Closure);
	qn_return_when_fail(node,/*void*/);

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

// 모를 때 쓰는 문자열 (스레드 완전 위험)
const char* qn_p_unknown(int value, bool hex)
{
	static char unknown_text[64];
	if (hex)
		qn_snprintf(unknown_text, QN_COUNTOF(unknown_text), "UNKNOWN(0x%X)", value);
	else
		qn_snprintf(unknown_text, QN_COUNTOF(unknown_text), "UNKNOWN(%d)", value);
	return unknown_text;
}

//
void qn_set_prop(const char* RESTRICT name, const char* RESTRICT value)
{
	qn_return_when_fail(runtime_impl.inited,/*void*/);
	qn_return_when_fail(name != NULL,/*void*/);
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
	qn_return_when_fail(runtime_impl.inited, NULL);
	qn_return_when_fail(name != NULL, NULL);
	char** ret;
	QN_LOCK(runtime_impl.lock);
	qn_mukum_get(QnPropMukum, &runtime_impl.props, name, &ret);
	QN_UNLOCK(runtime_impl.lock);
	return ret == NULL ? NULL : *ret;
}

//
int qn_get_prop_int(const char* name, int default_value, int min_value, int max_value)
{
	const char* v = qn_get_prop(name);
	if (v == NULL)
		return default_value;
	const int i = qn_strtoi(v, 10);
	return QN_CLAMP(i, min_value, max_value);
}

//
float qn_get_prop_float(const char* name, float default_value, float min_value, float max_value)
{
	const char* v = qn_get_prop(name);
	if (v == NULL)
		return default_value;
	const float f = qn_strtof(v);
	return QN_CLAMP(f, min_value, max_value);
}

//
void qn_syssym(const char** names, int count, nint start_sym)
{
	qn_return_when_fail(runtime_impl.inited,/*void*/);
	qn_return_when_fail(names != NULL,/*void*/);
	qn_return_when_fail(count > 0,/*void*/);
	qn_return_when_fail(start_sym + count < 10000,/*void*/);

	QN_LOCK(runtime_impl.lock);
	for (nint i = 0; i < count; i++)
		qn_mukum_set(QnSymMukum, &runtime_impl.symbols, qn_strdup(names[i]), start_sym + i);
	QN_UNLOCK(runtime_impl.lock);
}

//
nint qn_sym(const char* name)
{
	static nint sym = 10000;
	qn_return_when_fail(runtime_impl.inited, 0);
	qn_return_when_fail(name != NULL, 0);
	nint* ret;
	QN_LOCK(runtime_impl.lock);
	qn_mukum_get(QnSymMukum, &runtime_impl.symbols, name, &ret);
	if (ret != NULL)
	{
		QN_UNLOCK(runtime_impl.lock);
		return *ret;
	}
	const nint value = sym++;
	qn_mukum_set(QnSymMukum, &runtime_impl.symbols, qn_strdup(name), value);
	QN_UNLOCK(runtime_impl.lock);
	return value;
}

//
#if !defined _LIB || !defined _STATIC
#ifdef _QN_WINDOWS_
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	QN_DUMMY(hModule);
	QN_DUMMY(lpReserved);
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
		default:
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
