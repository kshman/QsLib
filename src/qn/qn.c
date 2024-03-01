//
// qn.c - QN 시작 지점
// 2023-12-27 by kim
//

#include "pch.h"
#ifdef _QN_UNIX_
#include <unistd.h>
#endif
#ifdef _QN_EMSCRIPTEN_
#include <emscripten/console.h>
#endif
#ifdef _QN_ANDROID_
#include <android/log.h>
#endif

//
extern void qn_cycle_up(void);
extern void qn_cycle_down(void);
extern void qn_mpf_up(void);
extern void qn_mpf_down(void);
extern void qn_module_up(void);
extern void qn_module_down(void);
extern void qn_thread_up(void);
extern void qn_thread_down(void);

struct PROPDATA;
static nint _sym_set(const char* name);
static void _prop_data_dispose(struct PROPDATA* data);
static void _prop_set(nint key, const char* value);

// 심볼
typedef struct SYMDATA
{
	char			key[64];
	nint			value;
} SymData;
QN_DECLIMPL_MUKUM(SymMukum, char*, SymData, qn_str_phash, qn_str_peqv, (void), (void), _sym_mukum);
QN_DECLIMPL_ARRAY(SymArray, SymMukumNode*, _sym_array);

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
	cham			inited;
#ifndef QS_NO_SPINLOCK
	QnSpinLock		lock;
#endif

	Closure*		closures;
	Closure*		preclosures;

	SymMukum		symbols;
	SymArray		symarray;
	PropMukum		props;

	char			tag[32];
	char			out_buf[MAX_DEBUG_LENGTH];
	nint			out_pos;

#ifdef _QN_WINDOWS_
	HANDLE			fd;
#else
	int				fd;
#endif

	bool			debugger;
	bool			redirect;
} runtime_impl =
{
	.inited = false,
	.tag = "QS",
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
	qn_cycle_down();

#ifdef _QN_WINDOWS_
	if (runtime_impl.redirect && runtime_impl.fd != NULL)
		CloseHandle(runtime_impl.fd);
#else
	if (runtime_impl.redirect && runtime_impl.fd >= 3)
		close(runtime_impl.fd);
#endif
}

// 런타임 올림
static void qn_runtime_up(void)
{
	runtime_impl.inited = true;

#ifdef _QN_WINDOWS_
	// 콘솔창을 UTF-8로 설정!!!!!!!!
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);

	runtime_impl.fd = GetStdHandle(STD_OUTPUT_HANDLE);
	runtime_impl.debugger = IsDebuggerPresent();
#else
	runtime_impl.fd = STDOUT_FILENO;
#endif

	qn_cycle_up();
	qn_mpf_up();
	qn_module_up();
	qn_thread_up();
	qn_srand(NULL, 0);

	_sym_mukum_init_fast(&runtime_impl.symbols);
	_sym_array_init_fast(&runtime_impl.symarray, 32);
	_prop_mukum_init_fast(&runtime_impl.props);

	_sym_array_add(&runtime_impl.symarray, NULL);	// 심볼을 1번부터니깐 0을 채워 넣기 위함
	_prop_set(_sym_set("QSLIB"), qn_version());
#define MAKE_BUILD_DATIME	__DATE__ " " __TIME__
	_prop_set(_sym_set("KIM"), MAKE_BUILD_DATIME);
#undef MAKE_BUILD_DATIME

#if defined _LIB || defined _STATIC
	(void)atexit(qn_runtime_down);
#endif
}

// 
void qn_runtime(const char* tag)
{
	qn_return_on_ok(runtime_impl.inited,/*void*/);
	if (tag != NULL && *tag != '\0')
		qn_strncpy(runtime_impl.tag, tag, QN_COUNTOF(runtime_impl.tag) - 1);
	qn_runtime_up();
}

//
const char* qn_version(void)
{
#define MAKE_VERSION_STRING(a,b)	QN_STRING(a) "." QN_STRING(b)
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
cham qn_p_debugger(void)
{
	return runtime_impl.debugger;
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
	_sym_array_add(&runtime_impl.symarray, node);

	return sym++;
}

//
static const char* _sym_get(nint value)
{
	if ((size_t)value >= _sym_array_count(&runtime_impl.symarray))
		return NULL;
	return _sym_array_nth(&runtime_impl.symarray, value)->KEY;
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
	qn_mesgf("SYMBOL", " %-8s | %-s", "symbol", "string");
#if false
	SymMukumNode* node;
	QN_MUKUM_FOREACH(runtime_impl.symbols, node)
		qn_mesgf("SYMBOL", " %-8d | %-s", node->VALUE.value, node->KEY);
#else
	size_t i;
	QN_ARRAY_FOREACH(runtime_impl.symarray, 1, i)
		qn_mesgf("SYMBOL", " %-8d | %-s", i, _sym_array_nth(&runtime_impl.symarray, i)->KEY);
#endif
	qn_mesgf("SYMBOL", "total symbols: %zu", _sym_mukum_count(&runtime_impl.symbols));
	QN_UNLOCK(runtime_impl.lock);
}

// 프로퍼티 데이터 지우기
static void _prop_data_dispose(struct PROPDATA* data)
{
	if (data->alloc)
		qn_free(data->value);
}

// 프로퍼티 설정
static void _prop_set(nint key, const char* value)
{
	PropMukumNode* node = qn_alloc_1(PropMukumNode);
	node->KEY = key;
	node->VALUE.key = key;
	if (strlen(value) < 64)
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

// 프로퍼티 얻기
static const char* _prop_get(nint key)
{
	PropData* ret = _prop_mukum_get(&runtime_impl.props, key);
	return ret == NULL ? NULL : ret->value;
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
		_prop_set(key, value);
	QN_UNLOCK(runtime_impl.lock);
}

//
const char* qn_get_prop(const char* name)
{
	nint key = qn_sym(name);
	qn_return_when_fail(key != 0, NULL);

	QN_LOCK(runtime_impl.lock);
	const char* ret = _prop_get(key);
	QN_UNLOCK(runtime_impl.lock);
	return ret;
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
		qn_mesgf("PROP", " %s = %-s", name, node->VALUE.intern);
	}
	qn_mesgf("PROP", "total properties: %zu", _prop_mukum_count(&runtime_impl.props));
	QN_UNLOCK(runtime_impl.lock);
}

//
static void _out_buf_ch(const int ch)
{
	if (1 + runtime_impl.out_pos > MAX_DEBUG_LENGTH - 1)
		return;
	runtime_impl.out_buf[runtime_impl.out_pos++] = (char)ch;
}

//
static void _out_buf_str(const char* s)
{
	nint len = (nint)strlen(s);
	if (len + runtime_impl.out_pos > MAX_DEBUG_LENGTH - 1)
		len = MAX_DEBUG_LENGTH - runtime_impl.out_pos - 1;
	if (len <= 0)
		return;
	memcpy(runtime_impl.out_buf + runtime_impl.out_pos, s, (size_t)len);
	runtime_impl.out_pos += len;
}

//
static void _out_buf_va(const char* fmt, va_list va)
{
	const int len = qn_vsnprintf(runtime_impl.out_buf + runtime_impl.out_pos, MAX_DEBUG_LENGTH - (size_t)runtime_impl.out_pos, fmt, va);
	if (len > 0)
		runtime_impl.out_pos += len;
}

//
static void _out_buf_int(const int value)
{
	const int len = qn_itoa(runtime_impl.out_buf + runtime_impl.out_pos, value, 10, true);
	runtime_impl.out_pos += len;
}

//
static void _out_buf_head(const char* head)
{
	if (head == NULL)
		return;
	_out_buf_ch('[');
	_out_buf_str(head);
	_out_buf_str("] ");
}

//
static nint _out_buf_flush(bool debug_output)
{
	qn_return_when_fail(runtime_impl.out_pos > 0, 0);
	runtime_impl.out_buf[runtime_impl.out_pos] = '\0';
#ifdef _QN_WINDOWS_
	if (runtime_impl.fd != NULL)
	{
		DWORD wtn;
		if (runtime_impl.redirect ||
			WriteConsoleA(runtime_impl.fd, runtime_impl.out_buf, (DWORD)runtime_impl.out_pos, &wtn, NULL) == 0)
			WriteFile(runtime_impl.fd, runtime_impl.out_buf, (DWORD)runtime_impl.out_pos, &wtn, NULL);
	}
	if (runtime_impl.debugger && debug_output)
	{
		// 유니코드로 보내야할지 나중에 확인해보자
		OutputDebugStringA(runtime_impl.out_buf);
	}
#else
	if (runtime_impl.fd != STDIN_FILENO)
		write(runtime_impl.fd, runtime_impl.out_buf, (size_t)runtime_impl.out_pos);
#ifdef __EMSCRIPTEN__
	// 콘솔에 두번 나오니깐 한번만 출력하자
	//if (debug_output)
	//	emscripten_console_log(runtime_impl.out_buf);
#endif
#ifdef _QN_ANDROID_
	if (debug_output)
		__android_log_print(ANDROID_LOG_VERBOSE, runtime_impl.tag, runtime_impl.out_buf);
#endif
#endif
	const nint ret = runtime_impl.out_pos;
	runtime_impl.out_pos = 0;
	return ret;
	}

//
int qn_asrt(const char* expr, const char* mesg, const char* filename, const int line)
{
	qn_return_when_fail(expr, -1);
	_out_buf_str("ASSERT FAILED : ");
	_out_buf_str(" (filename=\"");
	_out_buf_str(filename);
	_out_buf_str("\", line=");
	_out_buf_int(line);
	_out_buf_str(")\n\t: ");
	_out_buf_str(expr);
	if (mesg == NULL)
		_out_buf_str(">\n");
	else
	{
		_out_buf_str(">\n\t: [");
		_out_buf_str(mesg);
		_out_buf_str("]\n");
	}
	_out_buf_flush(true);

	return 0;
}

//
_Noreturn void qn_halt(const char* head, const char* mesg)
{
	_out_buf_str("HALT ");
	_out_buf_head(head);
	_out_buf_str(mesg);
	_out_buf_ch('\n');
	_out_buf_flush(true);

	qn_debug_break();
	abort();
}

//
int qn_mesg(const char* head, const char* mesg)
{
	_out_buf_head(head);
	_out_buf_str(mesg);
	_out_buf_ch('\n');
	const int len = (int)_out_buf_flush(true);
	return len;
}

//
int qn_mesgf(const char* head, const char* fmt, ...)
{
	_out_buf_head(head);
	va_list va;
	va_start(va, fmt);
	_out_buf_va(fmt, va);
	va_end(va);
	_out_buf_ch('\n');
	const int len = (int)_out_buf_flush(true);
	return len;
}

//
int qn_outputs(const char* mesg)
{
	_out_buf_str(mesg);
	_out_buf_ch('\n');
	return (int)_out_buf_flush(false);
}

//
int qn_outputf(const char* fmt, ...)
{
	qn_return_when_fail(fmt != NULL, -1);
	va_list va;
	va_start(va, fmt);
	_out_buf_va(fmt, va);
	va_end(va);
	_out_buf_ch('\n');
	return (int)_out_buf_flush(false);
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
