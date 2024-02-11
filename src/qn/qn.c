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

struct PROPDATA;
static nint _sym_set(const char* name);
static void _prop_data_dispose(struct PROPDATA* data);

// 심볼
typedef struct SYMDATA
{
	char			key[64];
	nint			value;
} SymData;
QN_DECLIMPL_MUKUM(SymMukum, char*, SymData, qn_str_phash, qn_str_peqv, (void), (void), _sym_mukum);
QN_DECLIMPL_ARRAY(SymArray, char*, _sym_array);

// 프로퍼티
typedef struct PROPDATA
{
	nint			key;
	char*			value;

	char			intern[64];
	bool			alloc;
} PropData;
QN_DECLIMPL_MUKUM(PropMukum, nint, PropData, qn_nint_phash, qn_nint_peqv, (void), _prop_data_dispose, _prop_mukum);

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

	SymMukum		symbols;
	SymArray		symarray;
	PropMukum		props;
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

	_sym_mukum_dispose(&runtime_impl.symbols);
	_sym_array_dispose(&runtime_impl.symarray);
	_prop_mukum_dispose(&runtime_impl.props);

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
	qm_srand(NULL, 0);

	_sym_mukum_init_fast(&runtime_impl.symbols);
	_sym_array_init_fast(&runtime_impl.symarray, 32);
	_prop_mukum_init_fast(&runtime_impl.props);

	_sym_array_add(&runtime_impl.symarray, NULL);	// 심볼을 1번부터니깐 0을 채워 넣기 위함
	_sym_set("QS");
	_sym_set("QSLIB");
	_sym_set("KIM");

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
static nint _sym_set(const char* name)
{
	static nint sym = 1;

	SymMukumNode* node = qn_alloc_1(SymMukumNode);
	node->VALUE.value = sym;
	qn_strcpy(node->VALUE.key, name);
	node->KEY = node->VALUE.key;
	if (!_sym_mukum_node_add(&runtime_impl.symbols, node))
	{
		qn_free(node);
		return 0;
	}
	_sym_array_add(&runtime_impl.symarray, node->VALUE.key);

	return sym++;
}

//
static const char* _sym_get(nint value)
{
	if ((size_t)value >= _sym_array_count(&runtime_impl.symarray))
		return NULL;
	return _sym_array_nth(&runtime_impl.symarray, value);
}

//
nint qn_sym(const char* name)
{
	qn_return_when_fail(runtime_impl.inited, 0);
	qn_return_when_fail(name != NULL, 0);
	QN_LOCK(runtime_impl.lock);

	SymData* ret = _sym_mukum_get(&runtime_impl.symbols, name);
	if (ret != NULL)
	{
		QN_UNLOCK(runtime_impl.lock);
		return ret->value;
	}

	size_t len = strlen(name);
	if (len > 63)
	{
		QN_UNLOCK(runtime_impl.lock);
		return 0;
	}

	const nint value = _sym_set(name);
	QN_UNLOCK(runtime_impl.lock);
	return value;
}

//
const char* qn_symstr(nint value)
{
	qn_return_when_fail(runtime_impl.inited, NULL);
	QN_LOCK(runtime_impl.lock);
	const char* ret = _sym_get(value);
	QN_UNLOCK(runtime_impl.lock);
	return ret;
}

//
void qn_sym_dbgout(void)
{
	qn_return_when_fail(runtime_impl.inited,/*void*/);
	QN_LOCK(runtime_impl.lock);
	qn_mesgf(false, "SYMBOL", " %-8s | %-s", "symbol", "string");
#if false
	SymMukumNode* node;
	QN_MUKUM_FOREACH(runtime_impl.symbols, node)
		qn_mesgf(false, "SYMBOL", " %-8d | %-s", node->VALUE.value, node->KEY);
#else
	size_t i;
	QN_ARRAY_FOREACH(runtime_impl.symarray, i)
		qn_mesgf(false, "SYMBOL", " %-8d | %-s", i, _sym_array_nth(&runtime_impl.symarray, i));
#endif
	qn_mesgf(false, "SYMBOL", "total symbols: %zu", _sym_mukum_count(&runtime_impl.symbols));
	QN_UNLOCK(runtime_impl.lock);
}

// 프로퍼티 데이터 지우기
static void _prop_data_dispose(struct PROPDATA* data)
{
	if (data->alloc)
		qn_free(data->value);
}

//
void qn_set_prop(const char* name, const char* value)
{
	nint key = qn_sym(name);
	qn_return_when_fail(key != 0,/*void*/);

	QN_LOCK(runtime_impl.lock);
	if (value == NULL || *value == '\0')
		_prop_mukum_remove(&runtime_impl.props, key);
	else
	{
		size_t len = strlen(value);
		PropMukumNode* node = qn_alloc_1(PropMukumNode);
		node->KEY = key;
		node->VALUE.key = key;
		if (len < 64)
		{
			node->VALUE.value = node->VALUE.intern;
			qn_strcpy(node->VALUE.intern, value);
			node->VALUE.alloc = false;
		}
		else
		{
			node->VALUE.value = qn_strdup(value);
			node->VALUE.intern[0] = '\0';
			node->VALUE.alloc = true;
		}
		_prop_mukum_node_add(&runtime_impl.props, node);
	}
	QN_UNLOCK(runtime_impl.lock);
}

//
const char* qn_get_prop(const char* name)
{
	nint key = qn_sym(name);
	qn_return_when_fail(key != 0, NULL);

	QN_LOCK(runtime_impl.lock);
	PropData* ret = _prop_mukum_get(&runtime_impl.props, key);
	QN_UNLOCK(runtime_impl.lock);
	return ret == NULL ? NULL : ret->value;
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
void qn_prop_dbgout(void)
{
	qn_return_when_fail(runtime_impl.inited,/*void*/);
	QN_LOCK(runtime_impl.lock);
	PropMukumNode* node;
	QN_MUKUM_FOREACH(runtime_impl.props, node)
	{
		const char* name = _sym_get(node->KEY);
		if (node->VALUE.alloc)
			qn_strncpy(node->VALUE.intern, node->VALUE.value, QN_COUNTOF(node->VALUE.intern) - 1);
		qn_mesgf(false, "PROP", " %s = %-s", name, node->VALUE.intern);
	}
	qn_mesgf(false, "PROP", "total properties: %zu", _prop_mukum_count(&runtime_impl.props));
	QN_UNLOCK(runtime_impl.lock);
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
