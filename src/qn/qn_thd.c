//
// qn_thd.c - 스레드
// 2023-12-27 by kim
//

#include "pch.h"
#include "qs_qn.h"
#ifdef _MSC_VER
#include <intrin.h>
#endif
#ifdef __GNUC__
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#endif

#ifdef _QN_WINDOWS_
static_assert(sizeof(long) == sizeof(QnSpinLock), "Spinlock not equal to OS interlock");
#endif

//////////////////////////////////////////////////////////////////////////
// 스핀락

//
bool qn_spin_try(QnSpinLock* lock)
{
#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
	return _InterlockedExchange((long volatile*)lock, 1) == 0;
#elif defined _MSC_VER && (defined _M_ARM || defined _M_ARM64 || defined _M_ARM64EC)
	return _InterlockedExchange_acq(lock, 1) == 0;
#elif defined __EMSCRIPTEN__
	return emscripten_atomic_exchange_u32(lock, 1) == 0;
#elif defined __GNUC__
	return __sync_lock_test_and_set(lock, 1) == 0;
#elif defined _QN_OSX_
	return OSAtomicCompareAndSwap32Barrier(0, 1, lock);
#else
	if (*lock)
		return false;
	*lock = 1;
	return true;
#endif
}

//
uint qn_spin_enter(QnSpinLock* lock)
{
	uint intrinsics = 0;
#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
	// Intel(R) 64 and IA-32 Architectures Optimization Reference Manual, May 2020 Example 2-4
	// Contended Locks with Increasing Back-off Example - Improved Version, page 2-22 (0BSD license)
	uint current = 1;
	while (_InterlockedExchange((long volatile*)lock, 1) != 0)
	{
		const uint backoff = 64;
		while (__iso_volatile_load32(lock) != 0) {
			for (uint count = current; count != 0; --count)
			{
				intrinsics++;
				qn_pause();
			}
		}
		current = current < backoff ? current << 1 : backoff;
	}
#elif defined _MSC_VER && (defined _M_ARM || defined _M_ARM64 || defined _M_ARM64EC)
	while (_InterlockedExchange_acq((long volatile*)lock, 1) != 0)
	{
		while (__iso_volatile_load32((int*)lock) != 0)
		{
			intrinsics++;
			qn_pause();
		}
	}
#else
	for (; !qn_spin_try(lock); intrinsics++)
	{
		const uint backoff = 64;
		if (intrinsics < backoff)
			qn_pause();
		else
			qn_sleep(0);
	}
#endif
	return intrinsics;
}

//
void qn_spin_leave(QnSpinLock* lock)
{
#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
	_InterlockedExchange((long volatile*)lock, 0);
#elif defined _MSC_VER && (defined _M_ARM || defined _M_ARM64)
	_InterlockedExchange_rel((long volatile*)lock, 0);
#elif defined __EMSCRIPTEN__
	emscripten_atomic_store_u32(lock, 0);
#elif defined __GNUC__
	__sync_lock_release(lock);
#else
	*lock = 0;
#endif
}


//////////////////////////////////////////////////////////////////////////
// 스레드

#ifndef MAX_TLS
#define MAX_TLS	64
#endif

// 실제 스레드
typedef struct QnRealThread	QnRealThread;
struct QnRealThread
{
	QnThread			base;

#ifdef _QN_WINDOWS_
	HANDLE				handle;
	DWORD				id;
#else
	pthread_t			handle;
#endif
	void*				tls[MAX_TLS];

	QnRealThread*		next;
};

//
static struct ThreadImpl
{
	uint				tls_index;
	paramfunc_t			tls_callback[MAX_TLS];

#ifdef _QN_WINDOWS_
	DWORD				self_tls;
#else
	size_t				max_stack;
	pthread_t			null_pthread;
	pthread_key_t		self_tls;
#endif

	QnRealThread*		self;
	QnRealThread*		threads;

#ifndef USE_NO_LOCK
	QnSpinLock			lock;
#endif
} thread_impl = { 0, };

static void _qn_thd_free(QnRealThread* self, uint tls_count, bool force);
#ifndef _QN_WINDOWS_
static void _qn_pthread_key_destroyer(void* p) {}
#endif

//
void qn_thread_up(void)
{
#ifdef _QN_WINDOWS_
	thread_impl.self_tls = TlsAlloc();
	if (thread_impl.self_tls == TLS_OUT_OF_INDEXES)
		qn_debug_halt("THREAD", "cannot allocate thread tls");
#else
#ifdef _SC_THREAD_STACK_MIN
	thread_impl.max_stack = (size_t)QN_MAX(sysconf(_SC_THREAD_STACK_MIN), 0);
#endif
	pthread_key_create(&thread_impl.self_tls, _qn_pthread_key_destroyer);
#endif
	thread_impl.self = (QnRealThread*)qn_thread_self();
}

//
void qn_thread_down(void)
{
#ifdef _QN_WINDOWS_
	if (thread_impl.self_tls != TLS_OUT_OF_INDEXES)
	{
		TlsFree(thread_impl.self_tls);
		thread_impl.self_tls = TLS_OUT_OF_INDEXES;
	}
#else
	if (thread_impl.self_tls != 0)
	{
		pthread_key_delete(thread_impl.self_tls);
		thread_impl.self_tls = 0;
	}
#endif

	for (QnRealThread *next, *node = thread_impl.threads; node; node = next)
	{
		next = node->next;
		_qn_thd_free(node, thread_impl.tls_index, true);
	}
}

#ifndef _QN_WINDOWS_
//
static bool _qn_pthread_is_null(pthread_t* p)
{
	return memcpy(p, &thread_impl.null_pthread, sizeof(pthread_t)) == 0;
}

//
static void _qn_pthread_make_null(pthread_t* p)
{
	memcpy(p, &thread_impl.null_pthread, sizeof(pthread_t));
}
#endif

//
static void _qn_thd_free(QnRealThread* self, const uint tls_count, const bool force)
{
	for (uint i = 0; i < tls_count; i++)
	{
		if (thread_impl.tls_callback[i] != NULL && self->tls[i] != NULL)
			thread_impl.tls_callback[i](self->tls[i]);
	}

	if (self->base.canwait)
	{
		if (force)
		{
			qn_free(self->base.name);
			qn_free(self);
		}
	}
	else
	{
#ifdef _QN_WINDOWS_
		if (self->handle)
			CloseHandle(self->handle);
#else
		if (_qn_pthread_is_null(&self->handle) == false)
			_qn_pthread_make_null(&self->handle);
#endif
		// 대기 안하는 애들은 강제로 지움
		qn_free(self->base.name);
		qn_free(self);
	}
}

//
static int _qn_thd_conv_busy(const QnRealThread* self)
{
#ifdef _QN_WINDOWS_
	const int n = GetThreadPriority(self->handle);
	switch (n)
	{
		case THREAD_PRIORITY_IDLE:			return -2;
		case THREAD_PRIORITY_LOWEST:		QN_FALL_THROUGH;
		case THREAD_PRIORITY_BELOW_NORMAL:	return -1;
		case THREAD_PRIORITY_ABOVE_NORMAL:	return 1;
		case THREAD_PRIORITY_HIGHEST:		return 2;
		default:							return 0;
	}
#else
	// pthread_getschedparam 를 써서 해야하는데 귀찮다
	return 0;
#endif
}

//
static bool _qn_thd_set_busy(const QnRealThread* self, const int busy)
{
#ifdef _QN_WINDOWS_
	switch (busy)
	{
		case -2:	return SetThreadPriority(self->handle, THREAD_PRIORITY_IDLE);
		case -1:	return SetThreadPriority(self->handle, THREAD_PRIORITY_BELOW_NORMAL);
		case 0:		return SetThreadPriority(self->handle, THREAD_PRIORITY_NORMAL);
		case 1:		return SetThreadPriority(self->handle, THREAD_PRIORITY_ABOVE_NORMAL);
		case 2:		return SetThreadPriority(self->handle, THREAD_PRIORITY_HIGHEST);
		default:	return FALSE;
	}
#else
	// pthread_getschedparam 를 써서 해야하는데 귀찮다
	return true;
#endif
}

#ifdef _QN_WINDOWS_
typedef HRESULT(WINAPI *PFWin32SetThreadDescription)(HANDLE, PCWSTR);
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadId; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
#endif

//
static void _qn_thd_set_name(QnRealThread* self)
{
	qn_ret_if_fail(self->base.name != NULL);

#ifdef _QN_WINDOWS_
#ifndef __WINRT__
	static QnModule* kernel32 = NULL;
	static PFWin32SetThreadDescription Win32SetThreadDescription = NULL;
	if (kernel32 == NULL)
	{
		kernel32 = qn_mod_load("KERNEL32", 0);
		if (kernel32 != NULL)
			Win32SetThreadDescription = (PFWin32SetThreadDescription)qn_mod_func(kernel32, "SetThreadDescription");
	}
	if (Win32SetThreadDescription != NULL)
	{
		wchar* pw = qn_u8to16_dup(self->base.name, 0);
		const HRESULT hr = Win32SetThreadDescription(self->handle, pw);
		qn_free(pw);
		if (hr == S_OK)
			return;
	}
#endif

	// https://learn.microsoft.com/ko-kr/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2022
	THREADNAME_INFO info =
	{
		.dwType = 0x1000,
		.szName = self->base.name,
		.dwThreadId = (DWORD)-1,
		.dwFlags = 0,
	};
#pragma warning(push)
#pragma warning(disable: 6320 6322)
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#pragma warning(pop)
#else
	pthread_setname_np(self->handle, self->base.name);
#endif
}

//
static void _qn_thd_exit(QnRealThread* self, const bool call_exit)
{
	if (self->base.managed)
	{
		self->base.managed = false;
		QN_LOCK(thread_impl.lock);
		for (QnRealThread *prev = NULL, *node = thread_impl.threads; node; prev = node, node = node->next)
		{
			if (node != self)
				continue;
			if (prev)
				prev->next = node->next;
			else
				thread_impl.threads = node->next;
			break;
		}
		QN_UNLOCK(thread_impl.lock);
	}

	const uint tls_count = thread_impl.tls_index;
	_qn_thd_free(self, tls_count, false);
#ifdef _QN_WINDOWS_
	TlsSetValue(thread_impl.self_tls, NULL);
#else
	pthread_setspecific(thread_impl.self_tls, NULL);
#endif

	if (call_exit)
#ifdef _QN_WINDOWS_
		ExitThread(0);
#else
		pthread_exit(NULL);
#endif
}

//
static QnRealThread* _qn_thd_test_self(void)
{
#ifdef _QN_WINDOWS_
	QnRealThread* self = (QnRealThread*)TlsGetValue(thread_impl.self_tls);
#else
	QnRealThread* self = (QnRealThread*)pthread_getspecific(thread_impl.self_tls);
#endif
	return self;
}

//
QnThread* qn_thread_self(void)
{
	QnRealThread* self = _qn_thd_test_self();
	if (self != NULL)
		return (QnThread*)self;

	self = qn_alloc_zero_1(QnRealThread);
#ifdef _QN_WINDOWS_
	const HANDLE process = GetCurrentProcess();
	self->id = GetCurrentThreadId();
	DuplicateHandle(process, GetCurrentThread(), process, &self->handle, 0, FALSE, DUPLICATE_SAME_ACCESS);
#else
	self->handle = pthread_self();
#endif

	self->base.name = NULL;
	self->base.canwait = false;
	self->base.busy = _qn_thd_conv_busy(self);
	self->base.stack_size = 0;
	self->base.cb_func = NULL;
	self->base.cb_data = NULL;
	self->base.cb_ret = NULL;
	qn_zero(self->tls, QN_COUNTOF(self->tls), void*);

	self->base.managed = true;
	QN_LOCK(thread_impl.lock);
	self->next = thread_impl.threads;
	thread_impl.threads = self;
	QN_UNLOCK(thread_impl.lock);

#ifdef _QN_WINDOWS_
	TlsSetValue(thread_impl.self_tls, self);
#else
	pthread_setspecific(thread_impl.self_tls, self);
#endif
	return (QnThread*)self;
}

//
QnThread* qn_thread_new(const char* restrict name, const QnThreadCallback func, void* data, const uint stack_size, const int busy)
{
	QnRealThread* self = qn_alloc_zero_1(QnRealThread);

	self->base.name = name == NULL ? NULL : qn_strdup(name);
	self->base.canwait = true;
	self->base.busy = QN_CLAMP(busy, -2, 2);
	self->base.stack_size = stack_size;
	self->base.cb_func = func;
	self->base.cb_data = data;
	self->base.cb_ret = NULL;
#ifdef _QN_WINDOWS_
	self->handle = NULL;
	self->id = 0;
#else
	_qn_pthread_make_null(&self->handle);
#endif
	qn_zero(self->tls, QN_COUNTOF(self->tls), void*);

	self->base.managed = true;
	QN_LOCK(thread_impl.lock);
	self->next = thread_impl.threads;
	thread_impl.threads = self;
	QN_UNLOCK(thread_impl.lock);

	return (QnThread*)self;
}

//
void qn_thread_delete(QnThread* self)
{
	qn_ret_if_fail(self->canwait != false);
	QnRealThread* real = (QnRealThread*)self;

	qn_thread_wait(self);

	if (real->base.managed)
	{
		real->base.managed = false;
		QN_LOCK(thread_impl.lock);
		for (QnRealThread *prev = NULL, *node = thread_impl.threads; node; prev = node, node = node->next)
		{
			if (node != (QnRealThread*)real)
				continue;
			if (prev)
				prev->next = node->next;
			else
				thread_impl.threads = node->next;
			break;
		}
		QN_UNLOCK(thread_impl.lock);
	}

	qn_free(real->base.name);
	qn_free(real);
}

//
bool qn_thread_once(const char* restrict name, const QnThreadCallback func, void* data, const uint stack_size, const int busy)
{
	QnRealThread* self = qn_alloc_zero_1(QnRealThread);

	self->base.name = name == NULL ? NULL : qn_strdup(name);
	self->base.canwait = false;
	self->base.busy = QN_CLAMP(busy, -2, 2);
	self->base.stack_size = stack_size;
	self->base.cb_func = func;
	self->base.cb_data = data;
	self->base.cb_ret = NULL;
#ifdef _QN_WINDOWS_
	self->handle = NULL;
	self->id = 0;
#else
	_qn_pthread_make_null(&self->handle);
#endif
	qn_zero(self->tls, QN_COUNTOF(self->tls), void*);

	self->base.managed = true;
	QN_LOCK(thread_impl.lock);
	self->next = thread_impl.threads;
	thread_impl.threads = self;
	QN_UNLOCK(thread_impl.lock);

	return qn_thread_start((QnThread*)self);
}

#ifdef _QN_WINDOWS_
// 스레드 시작 지점
static DWORD WINAPI _qn_thd_entry(void* data)
#else
// 스레드 시작 지점
static void* _qn_thd_entry(void* data)
#endif
{
	QnRealThread* self = (QnRealThread*)data;
#ifdef _QN_WINDOWS_
	TlsSetValue(thread_impl.self_tls, self);
#else
	static const int accept_signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGALRM, SIGTERM, SIGCHLD, SIGWINCH, SIGVTALRM, SIGPROF };
	sigset_t mask;
	sigemptyset(&mask);
	for (size_t i = 0; i < QN_COUNTOF(accept_signals); i++)
		sigaddset(&mask, accept_signals[i]);
	pthread_sigmask(SIG_BLOCK, &mask, 0);

	pthread_setspecific(thread_impl.self_tls, self);
#endif
	_qn_thd_set_name(self);
	self->base.cb_ret = self->base.cb_func(self->base.cb_data);
	_qn_thd_exit(self, false);
	return 0;
}

//
bool qn_thread_start(QnThread* self)
{
	QnRealThread* real = (QnRealThread*)self;

#ifdef _QN_WINDOWS_
	qn_val_if_fail(real->handle == NULL, false);

	real->handle = CreateThread(NULL, real->base.stack_size, &_qn_thd_entry, real, 0, &real->id);
	if (real->handle == NULL || real->handle == INVALID_HANDLE_VALUE)
		qn_debug_halt("THREAD", "cannot start thread");
#else
	qn_val_if_ok(_qn_pthread_is_null(&real->handle), false);

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	if (real->base.stack_size != 0)
	{
		size_t stack_size = QN_MAX(thread_impl.max_stack, real->base.stack_size);
		pthread_attr_setstacksize(&attr, stack_size);
	}

	int detach = real->base.canwait ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED;
	pthread_attr_setdetachstate(&attr, detach);

	int result = pthread_create(&real->handle, &attr, _qn_thd_entry, real);
	if (result != 0)
		qn_debug_halt("THREAD", "cannot start thread");
#endif

	if (_qn_thd_set_busy(real, real->base.busy) == false)
		real->base.busy = _qn_thd_conv_busy(real);
	return true;
}

//
void* qn_thread_wait(QnThread* self)
{
	QnRealThread* real = (QnRealThread*)self;
	qn_val_if_fail(real->base.canwait != false, NULL);
#ifdef _QN_WINDOWS_
	qn_val_if_fail(real->handle != NULL, NULL);

	const DWORD dw = WaitForSingleObjectEx(real->handle, INFINITE, FALSE);
	if (dw != WAIT_OBJECT_0)
		qn_debug_halt("THREAD", "thread wait failed");
	CloseHandle(real->handle);
	real->handle = NULL;
#else
	qn_val_if_ok(_qn_pthread_is_null(&real->handle), NULL);

	void* ret;
	pthread_join(real->handle, &ret);
	_qn_pthread_make_null(&real->handle);
#endif

	return real->base.cb_ret;
}

//
void qn_thread_exit(void* ret)
{
	QnThread* self = qn_thread_self();
	qn_ret_if_fail(self != NULL);
	self->cb_ret = ret;
	_qn_thd_exit((QnRealThread*)self, true);
}

int qn_thread_get_busy(const QnThread* self)
{
	return self->busy;
}

//
bool qn_thread_set_busy(QnThread* self, const int busy)
{
	QnRealThread* real = (QnRealThread*)self;
#ifdef _QN_WINDOWS_
	qn_val_if_fail(real->handle != NULL, false);
#else
	qn_val_if_fail(real->handle != 0, false);
#endif
	if (!_qn_thd_set_busy(real, busy))
		return false;
	real->base.busy = busy;
	return true;
}


//////////////////////////////////////////////////////////////////////////
// TLS

//
QnTls qn_tls(const paramfunc_t callback)
{
	QN_LOCK(thread_impl.lock);
	if (thread_impl.tls_index >= MAX_TLS)
	{
		QN_UNLOCK(thread_impl.lock);
		qn_debug_halt("THREAD", "too many TLS used");
	}

	const QnTls self = (int)thread_impl.tls_index;
	thread_impl.tls_callback[thread_impl.tls_index++] = callback;
	QN_UNLOCK(thread_impl.lock);

	return self;
}

//
void qn_tlsset(const QnTls tls, void* restrict data)
{
	const uint nth = (uint)tls;
	qn_ret_if_fail(nth < (uint)QN_COUNTOF(thread_impl.tls_callback));

	QnRealThread* thd = (QnRealThread*)qn_thread_self();
	thd->tls[nth] = data;
}

//
void* qn_tlsget(const QnTls tls)
{
	const uint nth = (uint)tls;
	qn_val_if_fail(nth < (uint)QN_COUNTOF(thread_impl.tls_callback), NULL);

	const QnRealThread* thd = _qn_thd_test_self();
	return thd == NULL ? NULL : thd->tls[nth];
}


//////////////////////////////////////////////////////////////////////////
// 뮤텍스

struct QnMutex
{
#ifdef _QN_WINDOWS_
	CRITICAL_SECTION	cs;
#else
	pthread_mutex_t		mutex;
#endif
};

QnMutex* qn_mutex_new(void)
{
	QnMutex* self = qn_alloc_1(QnMutex);
#ifdef _QN_WINDOWS_
	InitializeCriticalSectionEx(&self->cs, 4000, 0);
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&self->mutex, &attr);
#endif
	return self;
}

void qn_mutex_delete(QnMutex* self)
{
#ifdef _QN_WINDOWS_
	DeleteCriticalSection(&self->cs);
#else
	pthread_mutex_destroy(&self->mutex);
#endif
	qn_free(self);
}

bool qn_mutex_try(QnMutex* self)
{
#ifdef _QN_WINDOWS_
	return TryEnterCriticalSection(&self->cs);
#else
	return pthread_mutex_trylock(&self->mutex) == 0;
#endif
}

void qn_mutex_enter(QnMutex* self)
{
#ifdef _QN_WINDOWS_
	EnterCriticalSection(&self->cs);
#else
	pthread_mutex_lock(&self->mutex);
#endif
}

void qn_mutex_leave(QnMutex* self)
{
#ifdef _QN_WINDOWS_
	LeaveCriticalSection(&self->cs);
#else
	pthread_mutex_unlock(&self->mutex);
#endif
}


//////////////////////////////////////////////////////////////////////////
// 컨디션

struct QnCond
{
#ifdef _QN_WINDOWS_
	CONDITION_VARIABLE	cond;
#else
	pthread_cond_t		cond;
#endif
};

//
QnCond* qn_cond_new(void)
{
	QnCond* self = qn_alloc_1(QnCond);
#ifndef _QN_WINDOWS_
	//static const pthread_condattr_t s_attr = PTHREAD_COND_INITIALIZER;
	//pthread_cond_init(&self->cond, &s_attr);
	pthread_cond_init(&self->cond, NULL);
#endif
	return self;
}

//
void qn_cond_delete(QnCond* self)
{
#ifndef _QN_WINDOWS_
	pthread_cond_destroy(&self->cond);
#endif
	qn_free(self);
}

//
void qn_cond_signal(QnCond* self)
{
#ifdef _QN_WINDOWS_
	WakeConditionVariable(&self->cond);
#else
	pthread_cond_signal(&self->cond);
#endif
}

//
void qn_cond_broadcast(QnCond* self)
{
#ifdef _QN_WINDOWS_
	WakeAllConditionVariable(&self->cond);
#else
	pthread_cond_broadcast(&self->cond);
#endif
}

#ifndef _QN_WINDOWS_
//
static void qn_timed_timeval(struct timespec* ts, uint milliseconds)
{
	if (clock_gettime(CLOCK_REALTIME, ts) != 0)
	{
		struct timeval tv;
		gettimeofday(&tv, 0);
		ts->tv_sec = tv.tv_sec;
		ts->tv_nsec = tv.tv_usec * 1000;
	}
	ts->tv_nsec += (milliseconds % 1000) * 1000000;
	ts->tv_sec += milliseconds / 1000;
	if (ts->tv_nsec > 1000000000)
	{
		ts->tv_nsec -= 1000000000;
		ts->tv_sec++;
	}
}
#endif

//
bool qn_cond_wait_for(QnCond* self, QnMutex* lock, uint milliseconds)
{
	qn_val_if_fail(lock != NULL, false);

#ifdef _QN_WINDOWS_
	if (SleepConditionVariableCS(&self->cond, &lock->cs, milliseconds) == FALSE)
	{
		if (GetLastError() == ERROR_TIMEOUT)
			return false;
	}
	return true;
#else
	bool timeout;
	int result;

	if (milliseconds >= INT32_MAX)
	{
		result = pthread_cond_wait(&self->cond, &lock->mutex);
		timeout = false;
	}
	else
	{
		struct timespec ts;
		qn_timed_timeval(&ts, milliseconds);
		qn_val_if_fail(ts.tv_nsec < QN_NSEC_PER_SEC, true);
		result = pthread_cond_timedwait(&self->cond, &lock->mutex, &ts);
		timeout = result == ETIMEDOUT;
	}

	return timeout == false;
#endif
}

//
void qn_cond_wait(QnCond* self, QnMutex* lock)
{
	qn_cond_wait_for(self, lock, INT32_MAX);
}


//////////////////////////////////////////////////////////////////////////
// 세마포어

struct QnSem
{
#ifdef _QN_WINDOWS_
	HANDLE				handle;
	int volatile		count;
#else
	sem_t				sem;
#endif
};

//
QnSem* qn_sem_new(int initial)
{
#ifdef _QN_WINDOWS_
	const HANDLE handle = CreateSemaphoreEx(NULL, initial, 32 * 1024, NULL, 0, SEMAPHORE_ALL_ACCESS);
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		qn_debug_halt("SEM", "semaphore creation failed");

	QnSem* self = qn_alloc_1(QnSem);
	self->handle = handle;
	self->count = (nint)initial;
#else
	sem_t sem;
	if (sem_init(&sem, 0, (uint)initial) < 0)
	{
		qn_debug_halt("SEM", "semaphore creation failed");
		return NULL;
	}

	QnSem* self = qn_alloc_1(QnSem);
	self->sem = sem;
#endif
	return self;
}

//
void qn_sem_delete(QnSem* self)
{
#ifdef _QN_WINDOWS_
	CloseHandle(self->handle);
#else
	sem_destroy(&self->sem);
#endif
	qn_free(self);
}

//
bool qn_sem_wait_for(QnSem* self, uint milliseconds)
{
#ifdef _QN_WINDOWS_
	DWORD dw = WaitForSingleObjectEx(self->handle, milliseconds, FALSE);
	if (dw == WAIT_OBJECT_0)
	{
		static_assert(sizeof(long) == sizeof(self->count), "Semaphore size not equal to OS interlock");
		_InterlockedDecrement((long volatile*)&self->count);
		return true;
	}
	if (dw != WAIT_TIMEOUT)
		qn_debug_halt("SEM", "semaphore wait failed");
#else
	if (milliseconds == 0)
		return sem_trywait(&self->sem) == 0;
	if (milliseconds >= INT32_MAX)
		return sem_wait(&self->sem) == 0;

	struct timespec ts;
	qn_timed_timeval(&ts, milliseconds);
	int result;
	do
	{
		result = sem_timedwait(&self->sem, &ts);
	} while (result < 0 && errno == EINTR);
	if (result == 0)
		return true;
#endif
	return false;
}

//
bool qn_sem_wait(QnSem* self)
{
#ifdef _QN_WINDOWS_
	return qn_sem_wait_for(self, INT32_MAX);
#else
	return sem_wait(&self->sem) == 0;
#endif
}

//
bool qn_sem_try(QnSem* self)
{
#ifdef _QN_WINDOWS_
	return qn_sem_wait_for(self, 0);
#else
	return sem_trywait(&self->sem) == 0;
#endif
}

//
int qn_sem_count(QnSem* self)
{
#ifdef _QN_WINDOWS_
	return (int)self->count;
#else
	int count;
	sem_getvalue(&self->sem, &count);
	return count < 0 ? 0 : count;
#endif
}

//
bool qn_sem_post(QnSem* self)
{
#ifdef _QN_WINDOWS_
	_InterlockedIncrement((long volatile*)&self->count);
	if (ReleaseSemaphore(self->handle, 1, NULL))
		return true;
	_InterlockedDecrement((long volatile*)&self->count);
#else
	if (sem_post(&self->sem) == 0)
		return true;
#endif
	return false;
}
