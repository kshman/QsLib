#include "pch.h"
#include "qs_qn.h"
#include "qs_ctn.h"
#ifdef _QN_UNIX_
#include <pthread.h>
#include <semaphore.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4702)
#endif

#ifdef _QN_WINDOWS_
#ifdef _QN_64_
QN_STATIC_ASSERT(sizeof(__int64) == sizeof(QnSpinLock), "Spinlock size not equal to OS interlock");
#else
QN_STATIC_ASSERT(sizeof(long) == sizeof(QnSpinLock), "Spinlock size not equal to OS interlock");
#endif
#endif


//////////////////////////////////////////////////////////////////////////
// 스핀락

//
bool qn_spin_try(QnSpinLock* lock)
{
#if defined _MSC_VER && defined _M_X64
	return _InterlockedExchange64((__int64 volatile*)lock, 1) == 0;
#elif defined _MSC_VER && defined _M_IX86
	return _InterlockedExchange((long volatile*)lock, 1) == 0;
#elif defined _MSC_VER && defined _M_ARM64
	return _InterlockedExchange64_acq(lock, 1) == 0;
#elif defined _MSC_VER && defined _M_ARM
	return _InterlockedExchange_acq(lock, 1) == 0;
#elif defined __GNUC__
	return __sync_lock_test_and_set(lock, 1) != 0;
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
size_t qn_spin_enter(QnSpinLock* lock)
{
#define MAX_INTRINSICS		64
	size_t intrinsics;
	for (intrinsics = 0; !qn_spin_try(lock); intrinsics++)
	{
		if (intrinsics < MAX_INTRINSICS)
			qn_pause();
		else
			qn_sleep(0);
	}
	return intrinsics;
#undef MAX_INTRINSICS
}

//
void qn_spin_leave(QnSpinLock* lock)
{
#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
	qn_barrier();
	*lock = 0;
#elif defined _MSC_VER && defined _M_ARM64
	_InterlockedExchange64_rel(lock, 0);
#elif defined _MSC_VER && defined _M_ARM
	_InterlockedExchange_rel(lock, 0);
#elif defined __GNUC__
	__sync_lock_release(lock);
#else
	*lock = 0;
#endif
}


//////////////////////////////////////////////////////////////////////////
// 스레드

/** @brief 실제 스레드 */
typedef struct QnRealThread	QnRealThread;
struct QnRealThread
{
	QnThread			base;

#ifdef _QN_WINDOWS_
	HANDLE				handle;
	DWORD				id QN_PADDING_64(4,0);
#else
	pthread_t			handle;
#endif
	void*				tls[64];

	QnRealThread*		next;
};

//
static struct ThreadImpl
{
#ifdef _QN_WINDOWS_
	DWORD				tls;
#else
	int					max_stack;

	pthread_key_t		tls;
	pthread_t			null_pthread;
#endif
	uint				tls_index;
	paramfunc_t			tls_callback[64];

	QnRealThread*		self;
	QnRealThread*		threads;

	QnSpinLock			lock;
}
_qn_thd =
{
	0,
};

static void qn_internal_thread_dispose(QnRealThread* self, uint tls_count, bool force);
#ifndef _QN_WINDOWS_
static void qn_internal_tls_dispose(void* p);
#endif

//
void qn_thread_init(void)
{
#ifdef _QN_WINDOWS_
	_qn_thd.tls = TlsAlloc();
	if (_qn_thd.tls == TLS_OUT_OF_INDEXES)
		qn_debug_halt("THREAD", "cannot allocate thread tls");
#else
#ifdef _SC_THREAD_STACK_MIN
	_qn_thd.max_stack = QN_MAX(sysconf(_SC_THREAD_STACK_MIN), 0);
#endif
	pthread_key_create(&_qn_thd.tls, qn_internal_tls_dispose);
#endif
	_qn_thd.self = (QnRealThread*)qn_thread_self();
}

//
void qn_thread_dispose(void)
{
#ifdef _QN_WINDOWS_
	if (_qn_thd.tls != TLS_OUT_OF_INDEXES)
	{
		TlsFree(_qn_thd.tls);
		_qn_thd.tls = TLS_OUT_OF_INDEXES;
	}
#else
	if (_qn_thd.tls != 0)
	{
		pthread_key_delete(_qn_thd.tls);
		_qn_thd.tls = 0;
	}
#endif

	for (QnRealThread *next = NULL, *node = _qn_thd.threads; node; node = next)
	{
		next = node->next;
		qn_internal_thread_dispose(node, _qn_thd.tls_index, true);
	}
}

//
static int qn_internal_thread_conv_busy(QnRealThread* self)
{
#ifdef _QN_WINDOWS_
	int n = GetThreadPriority(self->handle);
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
static bool qn_internal_thread_set_busy(QnRealThread* self, int busy)
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

#ifndef _QN_WINDOWS_
//
static bool qn_internal_thread_is_null(pthread_t* p)
{
	return memcpy(p, &_qn_thd.null_pthread, sizeof(pthread_t)) == 0;
}

//
static void qn_internal_thread_make_null(pthread_t* p)
{
	memcpy(p, &_qn_thd.null_pthread, sizeof(pthread_t));
}
#endif

//
static void qn_internal_thread_dispose(QnRealThread* self, uint tls_count, bool force)
{
	for (uint i = 0; i < tls_count; i++)
	{
		if (_qn_thd.tls_callback[i] != NULL && self->tls[i] != NULL)
			_qn_thd.tls_callback[i](self->tls[i]);
	}

	if (self->base.canwait)
	{
		if (force)
			qn_free(self);
	}
	else
	{
#ifdef _QN_WINDOWS_
		if (self->handle)
			CloseHandle(self->handle);
#else
		if (qn_internal_thread_is_null(&self->handle) == false)
			qn_internal_thread_make_null(&self->handle);
#endif
		// 대기 안하는 애들은 강제로 지움
		qn_free(self);
	}
}

//
static void qn_internal_thread_exit(QnRealThread* self, bool Exit)
{
	if (self->base.managed)
	{
		self->base.managed = false;
		QN_LOCK(_qn_thd.lock);
		for (QnRealThread *prev = NULL, *node = _qn_thd.threads; node; prev = node, node = node->next)
		{
			if (node != self)
				continue;
			if (prev)
				prev->next = node->next;
			else
				_qn_thd.threads = node->next;
			break;
		}
		QN_UNLOCK(_qn_thd.lock);
	}

	int tls_count = _qn_thd.tls_index;
	qn_internal_thread_dispose(self, tls_count, false);
#ifdef _QN_WINDOWS_
	TlsSetValue(_qn_thd.tls, NULL);
#else
	pthread_setspecific(_qn_thd.tls, NULL);
#endif

	if (Exit)
#ifdef _QN_WINDOWS_
		ExitThread(0);
#else
		pthread_exit(NULL);
#endif
}

//
static QnRealThread* qn_internal_thread_try(void)
{
	QnRealThread* self = (QnRealThread*)
#ifdef _QN_WINDOWS_
		TlsGetValue(_qn_thd.tls);
#else
		pthread_getspecific(_qn_thd.tls);
#endif
	return self;
}

//
QnThread* qn_thread_self(void)
{
	QnRealThread* self = (QnRealThread*)
#ifdef _QN_WINDOWS_
		TlsGetValue(_qn_thd.tls);
#else
		pthread_getspecific(_qn_thd.tls);
#endif
	if (self != NULL)
		return (QnThread*)self;

	self = qn_alloc_zero_1(QnRealThread);
#ifdef _QN_WINDOWS_
	HANDLE process = GetCurrentProcess();
	self->id = GetCurrentThreadId();
	DuplicateHandle(process, GetCurrentThread(), process, &self->handle, 0, FALSE, DUPLICATE_SAME_ACCESS);
#else
	self->handle = pthread_self();
#endif

	self->base.canwait = false;
	self->base.busy = qn_internal_thread_conv_busy(self);
	self->base.stack_size = 0;
	self->base.cb_func = NULL;
	self->base.cb_data = NULL;
	self->base.cb_ret = NULL;
	qn_zero(self->tls, QN_COUNTOF(self->tls), void*);

	self->base.managed = true;
	QN_LOCK(_qn_thd.lock);
	self->next = _qn_thd.threads;
	_qn_thd.threads = self;
	QN_UNLOCK(_qn_thd.lock);

#ifdef _QN_WINDOWS_
	TlsSetValue(_qn_thd.tls, self);
#else
	pthread_setspecific(_qn_thd.tls, self);
#endif
	return (QnThread*)self;
}

//
QnThread* qn_thread_new(QnThreadCallback func, void* data, uint stack_size, int busy)
{
	QnRealThread* self = qn_alloc_zero_1(QnRealThread);

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
	qn_internal_thread_make_null(&self->handle);
#endif
	qn_zero(self->tls, QN_COUNTOF(self->tls), void*);

	self->base.managed = true;
	QN_LOCK(_qn_thd.lock);
	self->next = _qn_thd.threads;
	_qn_thd.threads = self;
	QN_UNLOCK(_qn_thd.lock);

	return (QnThread*)self;
}

//
void qn_thread_delete(QnThread* thread)
{
	qn_ret_if_fail(thread->canwait != false);
	QnRealThread* self = (QnRealThread*)thread;

	qn_thread_wait(thread);

	if (self->base.managed)
	{
		self->base.managed = false;
		QN_LOCK(_qn_thd.lock);
		for (QnRealThread *prev = NULL, *node = _qn_thd.threads; node; prev = node, node = node->next)
		{
			if (node != (QnRealThread*)self)
				continue;
			if (prev)
				prev->next = node->next;
			else
				_qn_thd.threads = node->next;
			break;
		}
		QN_UNLOCK(_qn_thd.lock);
	}

	qn_free(self);
}

//
bool qn_thread_once(const char* name, QnThreadCallback func, void* data, uint stack_size, int busy)
{
	QnRealThread* self = qn_alloc_zero_1(QnRealThread);

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
	qn_internal_thread_make_null(&self->handle);
#endif
	qn_zero(self->tls, QN_COUNTOF(self->tls), void*);

	self->base.managed = true;
	QN_LOCK(_qn_thd.lock);
	self->next = _qn_thd.threads;
	_qn_thd.threads = self;
	QN_UNLOCK(_qn_thd.lock);

	return qn_thread_start((QnThread*)self, name);
}

#ifdef _QN_WINDOWS_
// 스레드 시작 지점
static DWORD WINAPI qn_internal_thread_entry(void* data)
#else
// 스레드 시작 지점
static void* qn_internal_thread_entry(void* data)
#endif
{
	QnRealThread* self = (QnRealThread*)data;
#ifdef _QN_WINDOWS_
	TlsSetValue(_qn_thd.tls, self);
#else
	static const int s_signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGALRM, SIGTERM, SIGCHLD, SIGWINCH, SIGVTALRM, SIGPROF };
	sigset_t mask;
	sigemptyset(&mask);
	for (size_t i = 0; i < QN_COUNTOF(s_signals); i++)
		sigaddset(&mask, s_signals[i]);
	pthread_sigmask(SIG_BLOCK, &mask, 0);

	pthread_setspecific(_qn_thd.tls, self);
#endif
	self->base.cb_ret = self->base.cb_func(self->base.cb_data);
	qn_internal_thread_exit(self, true);
	return 0;
}

//
bool qn_thread_start(QnThread* thread, const char* name)
{
	QnRealThread* self = (QnRealThread*)thread;
#ifdef _QN_WINDOWS_
	qn_val_if_fail(self->handle == NULL, false);

	self->handle = CreateThread(NULL, self->base.stack_size, &qn_internal_thread_entry, self, 0, &self->id);
	if (self->handle == NULL || self->handle == INVALID_HANDLE_VALUE)
	{
		qn_debug_halt("THREAD", "cannot start thread");
		return FALSE;
	}

	if (name != NULL && *name != '\0')
	{
		wchar wcs[64];
		qn_u8to16(wcs, 64 - 1, name, 0);
		SetThreadDescription(self->handle, wcs);
	}

	if (qn_internal_thread_set_busy(self, self->base.busy) == false)
		self->base.busy = qn_internal_thread_conv_busy(self);
#else
	qn_val_if_ok(qn_internal_thread_is_null(&self->handle), false);

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	int stack_size;
	if (self->base.stack_size <= 0)
		stack_size = 0;
	else
	{
		stack_size = QN_MAX(_qn_thd.max_stack, self->base.stack_size);
		pthread_attr_setstacksize(&attr, stack_size);
	}

	int detach = self->base.canwait ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED;
	pthread_attr_setdetachstate(&attr, detach);

	int result = pthread_create(&self->handle, &attr, qn_internal_thread_entry, self);
	if (result != 0)
		return false;

	if (name != NULL && *name != '\0')
		pthread_setname_np(self->handle, name);
#endif

	return true;
}

//
void* qn_thread_wait(QnThread* thread)
{
	QnRealThread* self = (QnRealThread*)thread;
	qn_val_if_fail(self->base.canwait != false, NULL);
#ifdef _QN_WINDOWS_
	qn_val_if_fail(self->handle != NULL, NULL);

	DWORD dw = WaitForSingleObjectEx(self->handle, INFINITE, FALSE);
	if (dw != WAIT_OBJECT_0)
		qn_debug_halt("THREAD", "thread wait failed");
	CloseHandle(self->handle);
	self->handle = NULL;
#else
	qn_val_if_ok(qn_internal_thread_is_null(&self->handle), NULL);

	void* ret;
	pthread_join(self->handle, &ret);
	qn_internal_thread_make_null(&self->handle);
#endif

	return self->base.cb_ret;
}

//
void qn_thread_exit(void* ret)
{
	QnThread* self = qn_thread_self();
	qn_ret_if_fail(self != NULL);
	self->cb_ret = ret;
	qn_internal_thread_exit((QnRealThread*)self, true);
}

int qn_thread_get_busy(QnThread* self)
{
	return self->busy;
}

//
bool qn_thread_set_busy(QnThread* thread, int busy)
{
	QnRealThread* self = (QnRealThread*)thread;
#ifdef _QN_WINDOWS_
	qn_val_if_fail(self->handle != NULL, false);
#else
	qn_val_if_fail(self->handle != 0, false);
#endif
	if (!qn_internal_thread_set_busy(self, busy))
		return false;
	self->base.busy = busy;
	return true;
}


//////////////////////////////////////////////////////////////////////////
// TLS

#ifndef _QN_WINDOWS_
//
static void qn_internal_tls_dispose(void* p)
{
}
#endif

//
QnTls* qn_tls(paramfunc_t callback)
{
	QnTls* self;

	QN_LOCK(_qn_thd.lock);
	if (_qn_thd.tls_index >= 64)
	{
		QN_UNLOCK(_qn_thd.lock);
		qn_debug_halt("THREAD", "too many TLS used");
		return NULL;
	}

	self = (void*)((nuint)_qn_thd.tls_index + 0x10);
	_qn_thd.tls_callback[_qn_thd.tls_index++] = callback;
	QN_UNLOCK(_qn_thd.lock);

	return self;
}

//
void qn_tlsset(QnTls* tls, void* data)
{
	uint nth = (uint)(nuint)tls - 0x10;
	qn_ret_if_fail(nth < QN_COUNTOF(_qn_thd.tls_callback));

	QnRealThread* thd = (QnRealThread*)qn_thread_self();
	thd->tls[nth] = data;
}

//
void* qn_tlsget(QnTls* tls)
{
	uint nth = (uint)(nuint)tls - 0x10;
	qn_val_if_fail(nth < QN_COUNTOF(_qn_thd.tls_callback), NULL);

	QnRealThread* thd = qn_internal_thread_try();
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
static void qn_timed_timeval(struct timespec* ts, int milliseconds)
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
		qn_debug_outputs(true, "COND", "wait failed");
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
	nint volatile		count;
#else
	sem_t				sem;
#endif
};

//
QnSem* qn_sem_new(int initial)
{
#ifdef _QN_WINDOWS_
	HANDLE handle = CreateSemaphoreEx(NULL, initial, 32 * 1024, NULL, 0, SEMAPHORE_ALL_ACCESS);
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
	{
		qn_debug_halt("SEM", "semaphore creation failed");
		return NULL;
	}

	QnSem* self = qn_alloc_1(QnSem);
	self->handle = handle;
	self->count = (nint)initial;
#else
	sem_t sem;
	if (sem_init(&sem, 0, initial) < 0)
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
#ifdef _QN_64_
		QN_STATIC_ASSERT(sizeof(__int64) == sizeof(self->count), "Semaphore size not equal to OS interlock");
		_InterlockedDecrement64((__int64 volatile*)&self->count);
#else
		QN_STATIC_ASSERT(sizeof(long) == sizeof(self->count), "Semaphore size not equal to OS interlock");
		_InterlockedDecrement((long volatile*)&self->count);
#endif
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
#ifdef _QN_64
	_InterlockedIncrement64((__int64 volatile*)&self->count);
	if (ReleaseSemaphore(self->handle, 1, NULL))
		return true;
	_InterlockedDecrement64((__int64 volatile*)&self->count);
#else
	_InterlockedIncrement((long volatile*)&self->count);
	if (ReleaseSemaphore(self->handle, 1, NULL))
		return true;
	_InterlockedDecrement((long volatile*)&self->count);
#endif
#else
	if (sem_post(&self->sem) == 0)
		return true;
#endif
	return false;
}
