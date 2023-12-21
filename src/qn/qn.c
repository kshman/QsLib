#include "pch.h"
#include "qs_qn.h"
#include "qs_ctn.h"

//
extern void qn_cycle_init(void);
extern void qn_mpf_init(void);
extern void qn_mpf_dispose(void);
extern void qn_debug_init(void);
extern void qn_debug_dispose(void);
extern void qn_thread_init(void);
extern void qn_thread_dispose(void);

// 프로퍼티
QN_DECL_MUKUM(QnPropMukum, char*, char*);
QN_MUKUM_CHAR_PTR_KEY(QnPropMukum);
QN_MUKUM_KEY_FREE(QnPropMukum);
QN_MUKUM_VALUE_FREE(QnPropMukum);

// 닫아라
struct Closure
{
	struct Closure* prev;
	funcparam_t		fp;
};

// 구현
static struct RuntimeImpl
{
	BOOL			inited;
	QnSpinLock		lock;

	struct Closure* closures;
	struct Closure* preclosures;

	QnPropMukum		props;
} _qn_rt = { false, };

//
static void qn_dispose(void)
{
	qn_ret_if_fail(_qn_rt.inited);

	QN_LOCK(_qn_rt.lock);
	for (struct Closure *prev, *node = _qn_rt.closures; node; node = prev)
	{
		prev = node->prev;
		node->fp.func(node->fp.data);
		qn_free(node);
	}

	for (struct Closure *prev, *node = _qn_rt.preclosures; node; node = prev)
	{
		prev = node->prev;
		node->fp.func(node->fp.data);
		qn_free(node);
	}
	QN_UNLOCK(_qn_rt.lock);

	qn_mukum_disp(QnPropMukum, &_qn_rt.props);

	qn_thread_dispose();
	qn_mpf_dispose();
	qn_debug_dispose();
	_qn_rt.inited = false;
}

//
static void qn_init(void)
{
	_qn_rt.inited = true;

	qn_cycle_init();
	qn_debug_init();
	qn_mpf_init();
	qn_thread_init();

	qn_mukum_init(QnPropMukum, &_qn_rt.props);

#if defined _LIB || defined _STATIC
	(void)atexit(qn_dispose);
#endif
}

//
void qn_runtime(int v[2])
{
#if defined _LIB || defined _STATIC
	if (!_qn_rt.inited)
		qn_init();
#endif

	enum Version
	{
		major = 3,
		minor = 5,
		build = 0,
		rev = 0
	};

	if (v)
	{
		v[0] = major;
		v[1] = minor;
	}
}

//
void qn_atexit(paramfunc_t func, void* data)
{
	qn_ret_if_fail(func);

	struct Closure* node = qn_alloc_1(struct Closure);
	qn_ret_if_fail(node);

	node->fp.func = func;
	node->fp.data = data;

	QN_LOCK(_qn_rt.lock);
	node->prev = _qn_rt.closures;
	_qn_rt.closures = node;
	qn_spin_leave(&_qn_rt.lock);
}

//
void qn_internal_atexit(paramfunc_t func, void* data)
{
	qn_ret_if_fail(func);

	struct Closure* node = qn_alloc_1(struct Closure);
	qn_ret_if_fail(node);

	node->fp.func = func;
	node->fp.data = data;

	QN_LOCK(_qn_rt.lock);
	node->prev = _qn_rt.preclosures;
	_qn_rt.preclosures = node;
	QN_UNLOCK(_qn_rt.lock);
}

//
size_t qn_number(void)
{
	static size_t s = 0;
	return ++s;
}

//
void qn_set_prop(const char* restrict name, const char* restrict value)
{
	qn_ret_if_fail(name != NULL);
	QN_LOCK(_qn_rt.lock);
	if (value == NULL || *value == '\0')
		qn_mukum_remove(QnPropMukum, &_qn_rt.props, name, NULL);
	else
		qn_mukum_set(QnPropMukum, &_qn_rt.props, qn_strdup(name), qn_strdup(value));
	QN_UNLOCK(_qn_rt.lock);
}

//
const char* qn_get_prop(const char* restrict name)
{
	qn_val_if_fail(name != NULL, NULL);
	char** ret;
	QN_LOCK(_qn_rt.lock);
	qn_mukum_get(QnPropMukum, &_qn_rt.props, name, &ret);
	QN_UNLOCK(_qn_rt.lock);
	return ret == NULL ? NULL : *ret;
}

//
#if !defined _LIB || !defined _STATIC
#ifdef _QN_WINDOWS
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			qn_init();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			qn_disp();
			break;
	}
	return true;
}
#elif __GNUC__
void __attribute__((constructor)) _attach(void)
{
	qn_init();
}

void __attribute__((destructor)) _detach(void)
{
	qn_dispose();
}
#endif
#endif
