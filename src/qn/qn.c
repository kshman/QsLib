#include "pch.h"
#include "qs_qn.h"

//
extern void _qn_mp_init(void);
extern void _qn_mp_dispose(void);
extern void _qn_dbg_init(void);
extern void _qn_dbg_dispose(void);

// 닫아라
struct Closure
{
	struct Closure* prev;
	funcparam_t		fp;
};

// 구현
static struct QnRuntime
{
	bool			inited;
	struct Closure* closures;
	struct Closure* preclosures;
} _qn_rt = { false, NULL, NULL };

//
static void _qn_dispose(void)
{
	qn_ret_if_fail(_qn_rt.inited);

	for (struct Closure *prev = NULL, *node = _qn_rt.closures; node; node = prev)
	{
		prev = node->prev;
		node->fp.func(node->fp.data);
		qn_free(node);
	}

	for (struct Closure *prev = NULL, *node = _qn_rt.preclosures; node; node = prev)
	{
		prev = node->prev;
		node->fp.func(node->fp.data);
		qn_free(node);
	}

	_qn_mp_dispose();
	_qn_dbg_dispose();
}

//
static void _qn_init(void)
{
	_qn_rt.inited = true;

	_qn_dbg_init();
	_qn_mp_init();

#if _LIB || _STATIC
	(void)atexit(_qn_dispose);
#endif
}

//
void qn_runtime(int v[2])
{
#if _LIB || _STATIC
	if (!_qn_rt.inited)
		_qn_init();
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
	node->prev = _qn_rt.closures;
	_qn_rt.closures = node;
}

//
void qn_atexitp(paramfunc_t func, void* data)
{
	qn_ret_if_fail(func);

	struct Closure* node = qn_alloc_1(struct Closure);
	qn_ret_if_fail(node);

	node->fp.func = func;
	node->fp.data = data;
	node->prev = _qn_rt.preclosures;
	_qn_rt.preclosures = node;
}

//
size_t qn_number(void)
{
	static size_t s = 0;
	return ++s;
}

//
#if !_LIB && !_STATIC
#if _QN_WINDOWS
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			_qn_init();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			_qn_disp();
			break;
	}
	return true;
}
#elif __GNUC__
void __attribute__((constructor)) _attach(void)
{
	_qn_init();
}

void __attribute__((destructor)) _detach(void)
{
	_qn_dispose();
}
#endif
#endif
