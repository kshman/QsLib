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
	FALSE,
	NULL,
	NULL
};
static void qn_dispose(void);

//
static void qn_init(void)
{
	_qn_rt.inited = TRUE;

#if _LIB
	(void)atexit(qn_dispose);
#endif
}

//
static void qn_dispose(void)
{
	if (!_qn_rt.inited)
		return;

	struct Closure* node;
	struct Closure* prev;

	for (node = _qn_rt.closures; node; node = prev)
	{
		prev = node->prev;
		if (node->zero)
			node->data.func();
		else
			((func_closure_param)node->data.func)(node->data.data);
		free(node);
	}

	for (node = _qn_rt.preclosures; node; node = prev)
	{
		prev = node->prev;
		((func_closure_param)node->data.func)(node->data.data);
		free(node);
	}
}

//
void qn_runtime(int32_t v[])
{
#if _LIB
	if (!_qn_rt.inited)
		qn_init();
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
