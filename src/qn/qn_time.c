//
// qn_time.c - 날짜와 시간, 그리고 타이머
// 2023-12-27 by kim
//

#include "pch.h"
#ifdef __GNUC__
#include <unistd.h>
#include <sys/time.h>
#endif
#ifdef _MSC_VER
#pragma comment(lib, "winmm")
#endif

//
static struct CYCLEIMPL
{
	llong				start;		// 시작 카운트
	llong				tick;		// 카운터 틱, 보통 윈도우에서 1000000, 유닉스에서 1000
	llong				qtc_base;	// 기준 시간 변환 상수
	double				qtc_const;	// 시간 변환 상수
#ifdef _QN_WINDOWS_
	struct timespec		utc;	// UTC 시작 시간
#endif
} cycle_impl =
{
	0, 0,
#if true
	// qs time
	1577804400,			// 계산시작: 2020-01-01 00:00:00 (유닉스 시간)
	3600.0 / 60.0,		// 1초당 60분
#else
	// 비교용 eorzean time
	0,					// 계산시작: 1970-01-01 00:00:00 (유닉스 시간)
	3600.0 / 175.0,		// 1초당 175분
#endif
#ifdef _QN_WINDOWS_
	{ 0, 0 },
#endif
};

//
void qn_cycle_up(void)
{
#ifdef _QN_WINDOWS_
	timeBeginPeriod(1);

	LARGE_INTEGER ll;
	QueryPerformanceFrequency(&ll);
	cycle_impl.tick = ll.QuadPart;

	FILETIME ft;
	GetSystemTimePreciseAsFileTime(&ft);
	ULARGE_INTEGER ul = { .LowPart = ft.dwLowDateTime, .HighPart = ft.dwHighDateTime };
	ul.QuadPart -= 116444736000000000LL;
	cycle_impl.utc.tv_sec = (time_t)(ul.QuadPart / 10000000LL);
	cycle_impl.utc.tv_nsec = (long)((ul.QuadPart % 10000000LL) * 100LL);
#else
	cycle_impl.tick = 1000;
#endif
	cycle_impl.start = qn_cycle();
}

//
void qn_cycle_down(void)
{
#ifdef _QN_WINDOWS_
	timeEndPeriod(1);
#endif
}

//
void qn_set_qtc(const time_t base_time, const double conv_const)
{
	cycle_impl.qtc_base = (llong)base_time;
	cycle_impl.qtc_const = 3600.0 / conv_const;
}

//
llong qn_cycle(void)
{
#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	QueryPerformanceCounter(&ll);
	return ll.QuadPart;
#else
	llong n;
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
		n = ((llong)ts.tv_sec * QN_MSEC_PER_SEC) + ((llong)ts.tv_nsec / QN_USEC_PER_SEC);
	else
	{
		struct timeval tv;
		gettimeofday(&tv, 0);
		n = ((llong)tv.tv_sec * QN_MSEC_PER_SEC) + ((llong)tv.tv_usec / QN_MSEC_PER_SEC);
	}
	return n;
#endif
}

//
llong qn_tick(void)
{
	const llong cycle = qn_cycle();
	return ((cycle - cycle_impl.start) * 1000) / cycle_impl.tick;
}

//
uint qn_tick32(void)
{
	llong tick = qn_tick();
	return (uint)(tick % UINT32_MAX);
}

//
double qn_elapsed(void)
{
	const llong cycle = qn_cycle();
	return (double)(cycle - cycle_impl.start) / (double)cycle_impl.tick;
}

//
void qn_localtime(struct tm* ptm, const time_t tt)
{
#ifdef _MSC_VER
	(void)localtime_s(ptm, &tt);
#else
	(void)localtime_r(&tt, ptm);
#endif
}

//
void qn_gmtime(struct tm* ptm, const time_t tt)
{
#ifdef _MSC_VER
	(void)gmtime_s(ptm, &tt);
#else
	(void)gmtime_r(&tt, ptm);
#endif
}

// UTC 시간 얻기
static void _timespec_now(struct timespec* ts)
{
#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	QueryPerformanceCounter(&ll);
	ll.QuadPart -= cycle_impl.start;

	time_t sec = ll.QuadPart / cycle_impl.tick + cycle_impl.utc.tv_sec;
	llong nsec = (ll.QuadPart % cycle_impl.tick) * 1000000000LL / cycle_impl.tick + cycle_impl.utc.tv_nsec;
	if (nsec >= 1000000000LL)
	{
		sec++;
		nsec -= 1000000000LL;
	}

	ts->tv_sec = sec;
	ts->tv_nsec = (long)nsec;
#else
	if (clock_gettime(CLOCK_REALTIME, ts) != 0)
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ts->tv_sec = tv.tv_sec;
		ts->tv_nsec = tv.tv_usec * 1000;
	}
#endif
}

// tm을 타임스탬프로 변환
QnTimeStamp _tm_to_timestamp(struct tm* ptm, uint ms)
{
	QnDateTime dt;
	dt.year = (uint)ptm->tm_year + 1900;
	dt.month = (uint)ptm->tm_mon + 1;
	dt.day = (uint)ptm->tm_mday;
	dt.dow = (uint)ptm->tm_wday;
	dt.hour = (uint)ptm->tm_hour;
	dt.minute = (uint)ptm->tm_min;
	dt.second = (uint)ptm->tm_sec;
	dt.millisecond = ms;
	return dt.stamp;
}

// time_t를 타임스탬프로 변환
QnTimeStamp _time_to_timestamp(const time_t tt, uint ms)
{
	struct tm tm;
	qn_localtime(&tm, tt);
	return _tm_to_timestamp(&tm, ms);
}

//
QnTimeStamp qn_now(void)
{
	struct timespec ts;
	_timespec_now(&ts);
	return _time_to_timestamp(ts.tv_sec, ts.tv_nsec / QN_USEC_PER_SEC);
}

//
QnTimeStamp qn_utc(void)
{
	struct timespec ts;
	_timespec_now(&ts);
	struct tm tm;
	qn_gmtime(&tm, ts.tv_sec);
	return _tm_to_timestamp(&tm, ts.tv_nsec / QN_USEC_PER_SEC);
}

//
QnTimeStamp qn_qtc(void)
{
	struct timespec ts;
	_timespec_now(&ts);
	llong n = ((llong)ts.tv_sec * QN_MSEC_PER_SEC) + ((llong)ts.tv_nsec / QN_USEC_PER_SEC);
	llong t = (llong)round((double)(n - cycle_impl.qtc_base * QN_MSEC_PER_SEC) * cycle_impl.qtc_const);
	QnDateTime dt;
	dt.dow = (uint)((t / QN_MSEC_PER_SEC / 86400 + 4) % 7);
	dt.millisecond = t % 1000; t /= 1000;
	dt.second = t % 60; t /= 60;
	dt.minute = t % 60; t /= 60;
	dt.hour = t % 24; t /= 24;
	dt.day = t % 30 + 1; t /= 30;
	dt.month = t % 12 + 1; t /= 12;
	dt.year = (uint)t;
	return dt.stamp;
}

//
QnTimeStamp qn_stots(const double sec)
{
	QnDateTime dt;
	const uint us = (uint)sec;
	dt.hour = us / 3600;
	dt.minute = (us % 3600) / 60;
	dt.second = (us % 60);
	double m = (double)(ullong)sec;
	m = sec - m;
	m *= 1000.0;
	dt.millisecond = (uint)m;
	return dt.stamp;
}

//
QnTimeStamp qn_mstots(const uint msec)
{
	QnDateTime dt;
	const uint ns = msec / 1000;
	dt.hour = ns / 3600;
	dt.minute = (ns % 3600) / 60;
	dt.second = (ns % 60);
	dt.millisecond = msec - (ns * 1000);
	return dt.stamp;
}

//
static bool _is_leap_year(uint year)
{
	return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

static llong _elapsed_leap_years(uint year)
{
	return (year / 4) - (year / 100) + (year / 400);
}

//
static ullong _timstamp_to_linear(const QnTimeStamp ts)
{
	static const ushort days_of_months[2][12] =
	{
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },	// 평년
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 },	// 윤년
	};
	const QnDateTime* dt = (const QnDateTime*)&ts;
	// 년
	ullong vt = dt->year;
	// 월
	uint vm = dt->month - 1;
	if (vm >= 12)
	{
		vt += vm / 12;
		vm %= 12;
	}
	// 년과 월의 합체
	vt = (vt * 365) + _elapsed_leap_years(dt->year - 1) + days_of_months[_is_leap_year(dt->year)][vm];
	// 일
	vt = vt + dt->day;
	// 시간
	vt = (vt * 24) + dt->hour;
	// 분
	vt = (vt * 60) + dt->minute;
	// 초
	vt = (vt * 60) + dt->second;
	// 밀리초
	vt = (vt * 1000) + dt->millisecond;
	return vt;
}

//
double qn_tstos(const QnDateTime dt)
{
	llong vt = (llong)_timstamp_to_linear(dt.stamp);
	return (double)vt / 1000.0;
}

//
double qn_diffts(const QnTimeStamp left, const QnTimeStamp right)
{
	llong vt1 = (llong)_timstamp_to_linear(left);
	llong vt2 = (llong)_timstamp_to_linear(right);
	return (double)(vt1 - vt2) / 1000.0;
}

//
void _internal_yield(double until)
{
	do
	{
#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
		_mm_pause();
#elif defined _MSC_VER && (defined _M_ARM || defined _M_ARM64 || defined _M_ARM64EC)
		__yield();
#elif defined __GNUC__ && (defined __i386__ || defined __x86_64__)
		__asm__ __volatile__("pause");
#elif defined __GNUC__ && (defined __arm__ || defined __aarch64__)
		__asm__ __volatile__("yield");
#elif _QN_EMSCRIPTEN_
		//emscripten_sleep(0);
#endif
	} while (qn_elapsed() < until);
}

//
void qn_sleep(uint milliseconds)
{
#ifdef _QN_WINDOWS_
	Sleep(milliseconds);
#else
#if defined __EMSCRIPTEN__
	if (emscripten_has_asyncify())
	{
		emscripten_sleep(milliseconds);
		return;
	}
#endif
	struct timespec ts =
	{
		.tv_sec = milliseconds / QN_MSEC_PER_SEC,
		.tv_nsec = (milliseconds % QN_MSEC_PER_SEC) * QN_USEC_PER_SEC,
	};
	int error;
	do
	{
		errno = 0;
		struct timespec es = ts;
		error = nanosleep(&es, &ts);
	} while (error && (errno == EINTR));
#endif
}

//
void qn_ssleep(double seconds)
{
	double until = qn_elapsed() + seconds;
#if defined _QN_WINDOWS_
	DWORD msec = (DWORD)(seconds * QN_MSEC_PER_SEC * 0.9);
	Sleep(msec);
#elif defined _QN_UNIX_
#if defined _QN_EMSCRIPTEN_ //&& false
	if (emscripten_has_asyncify())
	{
		uint msec = (uint)(seconds * QN_MSEC_PER_SEC * 0.9);
		emscripten_sleep(msec);
	}
	else
#endif
	{
		llong nanosec = (llong)(seconds * 0.9 * QN_NSEC_PER_SEC);
		struct timespec ts =
		{
			.tv_sec = nanosec / QN_NSEC_PER_SEC,
			.tv_nsec = nanosec % QN_NSEC_PER_SEC,
		};
		int error;
		do
		{
			errno = 0;
			struct timespec es = ts;
			error = nanosleep(&es, &ts);
		} while (error && (errno == EINTR));
	}
#else
#error unknown platform! please place seconds sleep function on here!
#endif
	_internal_yield(until);
}

//
void qn_msleep(llong microseconds)
{
	qn_ssleep((double)microseconds / QN_USEC_PER_SEC);
}

//
void qn_yield(double seconds)
{
	double until = qn_elapsed() + seconds;
	_internal_yield(until);
}


//////////////////////////////////////////////////////////////////////////
// 타이머
typedef struct QNREALTIMER
{
	QnTimer				base;

	llong				base_time;		// 기준 시간
	llong				last_time;		// 이전 시간
	llong				accum;			// 검사용 누적 시간

	llong				cut_last;		// 컷 마지막
	double				cut_elapsed;	// 컷 경과
	double				inv_cut;		// 컷 역수

	llong				fps_time;		// FPS 초 계산용 시간
	uint				fps_index;		// FPS 인덱스
	float				fps_average;	// FPS 평균
	float				fps_array[30];
} QnRealTimer;

//
static void qn_timer_dispose(QnGam gam)
{
	qn_free(gam);
}

//
QnTimer* qn_create_timer(void)
{
	QnRealTimer* real = qn_alloc_zero_1(QnRealTimer);
	llong now = qn_cycle();

	real->base_time = now;
	real->last_time = now;

	static const QnVtableGam qn_timer_vt =
	{
		"TIMER",
		qn_timer_dispose,
	};
	return qn_gam_init(real, qn_timer_vt);
}

//
void qn_timer_reset(QnTimer* self)
{
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);
	llong now = qn_cycle();

	real->base.fps = 0.0f;
	real->base.afps = 0.0f;
	real->base.pause = false;
	real->base.frame = 0;

	real->base_time = now;
	real->last_time = now;
	real->accum = 0;

	real->cut_last = now;
	real->cut_elapsed = 0.0;

	real->fps_time = now;
	real->fps_index = 0;
	real->fps_average = 0.0f;
	memset(real->fps_array, 0, sizeof(real->fps_array));
}

//
void qn_timer_update(QnTimer* self)
{
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);

	llong now = qn_cycle(), last = real->last_time;
	real->last_time = now;

	llong runtime = now - real->base_time;
	llong elapsed = now - last;
	llong accum = real->accum + elapsed;
	if (accum != runtime)
		elapsed -= accum - runtime;

	real->accum = accum;
	real->base.runtime = (double)runtime / cycle_impl.tick;
	real->base.elapsed = (double)elapsed / cycle_impl.tick;
	real->base.advance = real->base.pause ? 0.0 : real->base.elapsed;

	// 컷
	double fps_elapsed;
	if (real->base.cut == 0)
		fps_elapsed = real->base.elapsed;
	else
	{
		real->cut_elapsed = (double)(now - real->cut_last) / cycle_impl.tick;
		real->cut_last = now;

		if (real->cut_elapsed < real->inv_cut)
		{
			qn_ssleep(real->inv_cut - real->cut_elapsed);

			now = qn_cycle();
			double wait = (double)(now - real->cut_last) / cycle_impl.tick;
			real->cut_last = now;
			real->cut_elapsed += wait;
		}

		fps_elapsed = real->cut_elapsed;
	}

	// 순간 FPS
	real->base.fps = (float)(1.0 / fps_elapsed);

	// 누적 FPS
	now = qn_cycle();
	if (now - real->fps_time >= cycle_impl.tick / (llong)QN_COUNTOF(real->fps_array) / 2)
	{
		real->fps_time = now;
		real->fps_index = (real->fps_index + 1) % QN_COUNTOF(real->fps_array);
		real->fps_average -= real->fps_array[real->fps_index];
		real->fps_array[real->fps_index] = (float)fps_elapsed / QN_COUNTOF(real->fps_array);
		real->fps_average += real->fps_array[real->fps_index];
	}
	real->base.afps = 1.0f / real->fps_average;

	real->base.frame++;
}

//
void qn_timer_set_cut(QnTimer* self, int cut)
{
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);
	real->base.cut = (ushort)cut;
	real->inv_cut = 1.0 / (double)cut;
}
