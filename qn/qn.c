#include "pch.h"
#include "qn.h"

enum
{
	qn_ver_major = 3,
	qn_ver_minor = 4,
	qn_ver_build = 0,
	qn_ver_rev = 0
};

//
typedef void (*func_closure_param)(pointer_t);

// 닫아라
struct Closure
{
	struct Closure* prev;
	funcparam_t		data;
	bool			zero;
};

// 구현
static struct QnRuntime
{
	bool			inited;
	struct Closure* closures;
	struct Closure* preclosures;
} _qn_rt = {
	false,
	NULL,
	NULL
};

//
static void _qn_dispose(void)
{
	qn_ret_if_fail(_qn_rt.inited);

	struct Closure* node;
	struct Closure* prev = NULL;

	for (node = _qn_rt.closures; node; node = prev)
	{
		prev = node->prev;
		if (node->zero)
			node->data.func();
		else
			((func_closure_param)node->data.func)(node->data.data);
		qn_free(node);
	}

	for (node = _qn_rt.preclosures; node; node = prev)
	{
		prev = node->prev;
		((func_closure_param)node->data.func)(node->data.data);
		qn_free(node);
	}
}

//
static void _qn_init(void)
{
	_qn_rt.inited = true;

#if _LIB
	(void)atexit(_qn_dispose);
#endif
}

//
void qn_runtime(int v[])
{
#if _LIB
	if (!_qn_rt.inited)
		_qn_init();
#endif

	if (v)
	{
		v[0] = qn_ver_major;
		v[1] = qn_ver_minor;
		v[2] = qn_ver_build;
		v[3] = qn_ver_rev;
	}
}

//
_Noreturn void qn_exit(const int exitcode)
{
	exit(exitcode);
}

//
void qn_atexit(void(*func)(pointer_t), pointer_t data)
{
	qn_ret_if_fail(func);

	struct Closure* node = qn_alloc_1(struct Closure);
	qn_ret_if_fail(node);
	node->zero = false;
	node->data.func = (func_t)func;
	node->data.data = data;
	node->prev = _qn_rt.closures;
	_qn_rt.closures = node;
}

//
void qn_atexit0(void(*func)(void))
{
	qn_ret_if_fail(func);

	struct Closure* node = qn_alloc_1(struct Closure);
	qn_ret_if_fail(node);
	node->zero = true;
	node->data.func = (func_t)func;
	node->data.data = NULL;
	node->prev = _qn_rt.closures;
	_qn_rt.closures = node;
}

//
void qn_atexitp(void(*func)(pointer_t), pointer_t data)
{
	qn_ret_if_fail(func);

	struct Closure* node = qn_alloc_1(struct Closure);
	qn_ret_if_fail(node);
	node->zero = false;
	node->data.func = (func_t)func;
	node->data.data = data;
	node->prev = _qn_rt.preclosures;
	_qn_rt.preclosures = node;
}
