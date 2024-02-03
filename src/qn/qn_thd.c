//
// qn_thd.c - 스레드
// 2023-12-27 by kim
//

#include "pch.h"
#ifdef _QN_UNIX_
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#ifdef _QN_FREEBSD_
#include <sys/sysctl.h>
#endif
#endif
#ifdef _QN_EMSCRIPTEN_
#include <emscripten/atomic.h>
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
	return emscripten_atomic_exchange_u32((void*)lock, 1) == 0;
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
				_mm_pause();
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
			_yield();
		}
	}
#else
	for (; !qn_spin_try(lock); intrinsics++)
	{
		const uint backoff = 64;
		if (intrinsics < backoff)
		{
#if defined __GNUC__ && (defined __i386__ || defined __amd64__ || defined __x86_64__)
			__asm__ __volatile__("pause\n");
#elif defined __GNUC__ && defined __aarch64__
			__asm__ __volatile__("yield" ::: "memory");
#else
			qn_sleep(0);
#endif
		}
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
	emscripten_atomic_store_u32((void*)lock, 0);
#elif defined __GNUC__
	__sync_lock_release(lock);
#else
	*lock = 0;
#endif
}


//////////////////////////////////////////////////////////////////////////
// 모듈

// 모듈 플래그
typedef enum QNMODFLAG
{
	QNMDF_SYSTEM = QN_BIT(0),			// 시스템 모듈 (load 인수로 true)를 입력했을 때 반응하도록 함
	QNMDF_RESIDENT = QN_BIT(1),			// 로드된 모듈로 만들어졌을 때
	QNMDF_MULTIPLE = QN_BIT(2),			// 한번 이상 모듈이 만들어졌을 때
	QNMDF_SELF = QN_BIT(7),				// 실행파일
} QnModFlag;

// 단위 모듈 내용
struct QNMODULE
{
	QnGamBase				base;

	char*				filename;
	size_t				hash;
	QnModFlag			flags;

	QnModule*			next;
};

// 모듈 관리 구현
static struct MODULEIMPL
{
	QnSpinLock			lock;
	QnModule*			self;
	QnModule*			modules;
} module_impl = { false, };

//
void qn_module_up(void)
{
	// 딱히 할건 없다
}

//
void qn_module_down(void)
{
	QnModule* node = module_impl.modules;
	while (node)
	{
		QnModule* next = node->next;
		if (QN_TMASK(node->flags, QNMDF_SYSTEM | QNMDF_RESIDENT | QNMDF_MULTIPLE) == false)
			qn_mesgf(false, "MODULE", "'%s' not unloaded (ref: %d)", node->filename, qn_gam_ref(node));
		if (QN_TMASK(node->flags, QNMDF_RESIDENT) == false && qn_get_gam_pointer(node) != NULL)
#ifdef _QN_WINDOWS_
			FreeLibrary(qn_get_gam_handle(node));
#else
			dlclose(qn_get_gam_pointer(node));
#endif
		qn_free(node->filename);
		qn_free(node);
		node = next;
	}

	if (module_impl.self != NULL)
	{
		qn_free(module_impl.self->filename);
		qn_free(module_impl.self);
	}
}

//
static QnModule* _qn_module_find(const char* filename, const size_t hash)
{
	QnModule* find = NULL;
	QN_LOCK(module_impl.lock);
	for (QnModule* node = module_impl.modules; node; node = node->next)
	{
		if (hash != node->hash || qn_streqv(node->filename, filename) == false)
			continue;
		find = node;
		break;
	}
	QN_UNLOCK(module_impl.lock);
	return find;
}

// dlerror 처리용
static void _qn_module_set_error(void)
{
#ifdef _QN_WINDOWS_
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
	qn_mesg(false, "MODULE", (const char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
#else
	const char* err = dlerror();
	if (err != NULL)
		qn_mesg(false, "MODULE", err);
#endif
}

//
static void _qn_mod_free(QnModule* self, const bool dispose)
{
	if (qn_get_gam_pointer(self) != NULL)
	{
#ifdef _QN_WINDOWS_
		if (FreeLibrary(qn_get_gam_handle(self)) == FALSE)
#else
		if (dlclose(qn_get_gam_pointer(self)) != 0)
#endif
			_qn_module_set_error();
	}
	if (dispose)
	{
		qn_free(self->filename);
		qn_free(self);
	}
}

//
static void qn_mod_dispose(QnGamBase* gam)
{
	QnModule* self = qn_cast_type(gam, QnModule);
	qn_ret_if_ok(QN_TMASK(self->flags, QNMDF_SELF));

	QN_LOCK(module_impl.lock);
	for (QnModule *last = NULL, *node = module_impl.modules; node; )
	{
		if (node == self)
		{
			if (last)
				last->next = node->next;
			else
				module_impl.modules = node->next;
			break;
		}
		last = node;
		node = node->next;
	}
	self->next = NULL;
	QN_UNLOCK(module_impl.lock);

	_qn_mod_free(self, true);
}

//
static qn_gam_vt(QNGAMBASE) qn_mod_vt =
{
	"MODULE",
	qn_mod_dispose,
};

//
QnModule* qn_mod_self(void)
{
	if (module_impl.self != NULL)
		return qn_loadu(module_impl.self, QnModule);

#ifdef _QN_WINDOWS_
	HMODULE handle =
#ifdef __WINRT__
		NULL;
#else
		GetModuleHandle(NULL);
#endif
#else
	void* handle = dlopen(NULL, RTLD_GLOBAL | RTLD_LAZY);	// 이 호출에 오류가 발생하면 프로그램은 끝이다
#endif

	QnModule* self = qn_alloc_zero_1(QnModule);
	qn_set_gam_desc(self, handle);
	self->flags = (QnModFlag)(QNMDF_RESIDENT | QNMDF_SELF);

	QN_LOCK(module_impl.lock);
	module_impl.self = self;
	QN_UNLOCK(module_impl.lock);

	return qn_gam_init(self, qn_mod_vt);
}

//
QnModule* qn_load_mod(const char* filename, const int flags)
{
	qn_val_if_fail(filename != NULL, NULL);

	const size_t hash = qn_strhash(filename);
	QnModule* self = _qn_module_find(filename, hash);
	if (self != NULL)
	{
		QN_SMASK(&self->flags, QNMDF_MULTIPLE, true);
		return qn_loadu(module_impl.self, QnModule);
	}

	QnModFlag mod_flag = (QnModFlag)flags;
#ifdef _QN_WINDOWS_
	wchar* pw = qn_u8to16_dup(filename, 0);
#ifdef __WINRT__
	HMODULE handle = LoadPackagedLibrary(pw, 0);
#else
	HMODULE handle = GetModuleHandle(pw);
	if (handle != NULL)
		mod_flag |= QNMDF_RESIDENT;
	else
		handle = LoadLibrary(pw);
#endif
	qn_free(pw);
#else
	void* handle = dlopen(filename, RTLD_LOCAL | RTLD_NOW);
#endif
	if (handle == NULL)
	{
		_qn_module_set_error();
		return NULL;
	}

	self = qn_alloc_1(QnModule);
	self->hash = hash;
	self->filename = qn_strdup(filename);
	self->flags = mod_flag;
	qn_set_gam_desc(self, handle);

	QN_LOCK(module_impl.lock);
	self->next = module_impl.modules;
	module_impl.modules = self;
	QN_UNLOCK(module_impl.lock);
	return qn_gam_init(self, qn_mod_vt);
}

//
void* qn_mod_func(QnModule* self, const char* RESTRICT name)
{
	qn_val_if_fail(name != NULL && *name != '\0', NULL);
#ifdef _QN_WINDOWS_
#ifdef __WINRT__
	void* ptr = (void*)GetProcAddress(NULL, name);
#else
	void* ptr = (void*)GetProcAddress(qn_get_gam_handle(self), name);
#endif
#else
	void* ptr = dlsym(qn_get_gam_pointer(self), name);
#endif
	if (ptr == NULL)
		_qn_module_set_error();
	return ptr;
}


//////////////////////////////////////////////////////////////////////////
// 스레드

// 실제 스레드
typedef struct QNREALTHREAD
{
	QnThread			base;

#ifdef _QN_WINDOWS_
	HANDLE				handle;
	DWORD				id;
#else
	pthread_t			handle;
#endif
	void*				tls[MAX_TLS];

	struct QNREALTHREAD*	next;
} QnRealThread;

//
static struct THREADIMPL
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

#ifndef QS_NO_SPINLOCK
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
		qn_halt("THREAD", "cannot allocate thread tls");
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
		case THREAD_PRIORITY_LOWEST:		FALL_THROUGH;
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
		kernel32 = qn_load_mod("KERNEL32", 0);
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
QnThread* qn_new_thread(const char* RESTRICT name, const QnThreadCallback func, void* data, const uint stack_size, const int busy)
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
bool qn_thread_once(const char* RESTRICT name, const QnThreadCallback func, void* data, const uint stack_size, const int busy)
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
	pthread_sigmask(SIG_BLOCK, &mask, NULL);

	pthread_setspecific(thread_impl.self_tls, self);
#endif
	_qn_thd_set_name(self);
	self->base.cb_ret = self->base.cb_func(self->base.cb_data);
	_qn_thd_exit(self, false);
#ifdef _QN_WINDOWS_
	return 0;
#else
	return NULL;
#endif
}

//
bool qn_thread_start(QnThread* self)
{
	QnRealThread* real = (QnRealThread*)self;

#ifdef _QN_WINDOWS_
	qn_val_if_fail(real->handle == NULL, false);

	real->handle = CreateThread(NULL, real->base.stack_size, &_qn_thd_entry, real, 0, &real->id);
	if (real->handle == NULL || real->handle == INVALID_HANDLE_VALUE)
		qn_halt("THREAD", "cannot start thread");
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
		qn_halt("THREAD", "cannot start thread");
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
		qn_halt("THREAD", "thread wait failed");
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
		qn_halt("THREAD", "too many TLS used");
	}

	const QnTls self = (int)thread_impl.tls_index;
	thread_impl.tls_callback[thread_impl.tls_index++] = callback;
	QN_UNLOCK(thread_impl.lock);

	return self;
}

//
void qn_tlsset(const QnTls tls, void* RESTRICT data)
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

struct QNMUTEX
{
#ifdef _QN_WINDOWS_
	CRITICAL_SECTION	cs;
#else
	pthread_mutex_t		mutex;
#endif
};

QnMutex* qn_new_mutex(void)
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

struct QNCOND
{
#ifdef _QN_WINDOWS_
	CONDITION_VARIABLE	cond;
#else
	pthread_cond_t		cond;
#endif
};

//
QnCond* qn_new_cond(void)
{
#ifdef _QN_WINDOWS_
	QnCond* self = qn_alloc_zero_1(QnCond);
#else
	QnCond* self = qn_alloc_1(QnCond);
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

struct QNSEM
{
#ifdef _QN_WINDOWS_
	HANDLE				handle;
	int volatile		count;
#else
	sem_t				sem;
#endif
};

//
QnSem* qn_new_sem(int initial)
{
#ifdef _QN_WINDOWS_
	const HANDLE handle = CreateSemaphoreEx(NULL, initial, 32 * 1024, NULL, 0, SEMAPHORE_ALL_ACCESS);
	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		qn_halt("SEM", "semaphore creation failed");

	QnSem* self = qn_alloc_1(QnSem);
	self->handle = handle;
	self->count = (int)initial;
#else
	sem_t sem;
	if (sem_init(&sem, 0, (uint)initial) < 0)
	{
		qn_halt("SEM", "semaphore creation failed");
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
	const DWORD dw = WaitForSingleObjectEx(self->handle, milliseconds, FALSE);
	if (dw == WAIT_OBJECT_0)
	{
		static_assert(sizeof(long) == sizeof(self->count), "Semaphore size not equal to OS interlock");
		_InterlockedDecrement((long volatile*)&self->count);
		return true;
	}
	if (dw != WAIT_TIMEOUT)
		qn_halt("SEM", "semaphore wait failed");
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
