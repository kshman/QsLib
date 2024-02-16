//
// qn_time.c - 날짜와 시간, 그리고 타이머
// 2023-12-27 by kim
//

#include "pch.h"
#ifdef __GNUC__
#include <unistd.h>
#include <sys/time.h>
#endif

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

#ifdef _QN_WINDOWS_
// 윈도우 SYSTEMTIME을 타임스탬프로 변환
QnTimeStamp qn_system_time_to_timestamp(const SYSTEMTIME* pst)
{
	QnDateTime dt;
	dt.year = pst->wYear;
	dt.month = pst->wMonth;
	dt.day = pst->wDay;
	dt.dow = pst->wDayOfWeek;
	dt.hour = pst->wHour;
	dt.minute = pst->wMinute;
	dt.second = pst->wSecond;
	dt.millisecond = pst->wMilliseconds;
	return dt.stamp;
}
#endif

#ifdef _QN_UNIX_
// tm을 타임스탬프로 변환
QnTimeStamp qn_tm_to_timestamp(struct tm* ptm)
{
	QnDateTime dt;
	dt.year = (uint)ptm->tm_year + 1900;
	dt.month = (uint)ptm->tm_mon + 1;
	dt.day = (uint)ptm->tm_mday;
	dt.dow = (uint)ptm->tm_wday;
	dt.hour = (uint)ptm->tm_hour;
	dt.minute = (uint)ptm->tm_min;
	dt.second = (uint)ptm->tm_sec;
	dt.millisecond = 0;
	return dt.stamp;
}
#endif

//
QnTimeStamp qn_now(void)
{
#ifdef _QN_WINDOWS_
	SYSTEMTIME st;
	GetLocalTime(&st);
	return qn_system_time_to_timestamp(&st);
#else
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv, NULL);
	(void)gmtime_r(&tv.tv_sec, &tm);
	return qn_tm_to_timestamp(&tm);
#endif
}

//
QnTimeStamp qn_utc(void)
{
#ifdef _QN_WINDOWS_
	SYSTEMTIME st;
	GetSystemTime(&st);
	return qn_system_time_to_timestamp(&st);
#else
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv, NULL);
	(void)gmtime_r(&tv.tv_sec, &tm);
	return qn_tm_to_timestamp(&tm);
#endif
}

//
QnTimeStamp qn_stod(const double sec)
{
	QnDateTime dt;
	const uint ns = (uint)sec;
	dt.hour = ns / 3600;
	dt.minute = (ns % 3600) / 60;
	dt.second = (ns % 60);
	double m = (double)(ullong)sec;
	m = sec - m;
	m *= 1000.0;
	dt.millisecond = (uint)m;
	return dt.stamp;
}

//
QnTimeStamp qn_mstod(const uint msec)
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
static struct CYCLEIMPL
{
	llong				start_count;	// 시작 카운트
	llong				tick_count;		// 카운터 틱, 보통 윈도우에서 1000000, 유닉스에서 1000
} cycle_impl = { 0, };

//
void qn_cycle_up(void)
{
#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	QueryPerformanceFrequency(&ll);
	cycle_impl.tick_count = ll.QuadPart;
#else
	cycle_impl.tick_count = 1000;
#endif
	cycle_impl.start_count = qn_cycle();
}

//
llong qn_cycle(void)
{
#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	QueryPerformanceCounter(&ll);
	return ll.QuadPart;
#else
	ullong n;
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
		n = ((llong)ts.tv_sec * 1000) + ((llong)ts.tv_nsec / 1000000);
	else
	{
		struct timeval tv;
		gettimeofday(&tv, 0);
		n = ((llong)tv.tv_sec * 1000) + ((llong)tv.tv_usec / 1000);
	}
	return n;
#endif
}

//
llong qn_tick(void)
{
	const llong cycle = qn_cycle();
	return ((cycle - cycle_impl.start_count) * 1000) / cycle_impl.tick_count;
}

//
uint qn_tick32(void)
{
	llong tick = qn_tick();
	return (uint)(tick % UINT32_MAX);
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
		.tv_sec = milliseconds / 1000,
		.tv_nsec = (milliseconds % 1000) * 1000000,
	};
	int error;
	do
	{
		errno = 0;
		struct timespec es =
		{
			.tv_sec = ts.tv_sec,
			.tv_nsec = ts.tv_nsec,
		};
		error = nanosleep(&es, &ts);
	} while (error && (errno == EINTR));
#endif
}

//
void qn_ssleep(double seconds)
{
	qn_sleep((uint)(seconds * 1000));
}

//
void qn_msleep(llong microseconds)
{
	if (microseconds <= 0)
		return;
#ifdef _QN_WINDOWS_
	const double dms = (double)microseconds;
	LARGE_INTEGER t1, t2;
	QueryPerformanceCounter(&t1);
	do
	{
		if (SwitchToThread())
#if defined _M_IX86 || defined _M_X64
			_mm_pause();
#elif defined _M_ARM || defined _M_ARM64
			_yield();
#else
#error unknown Windows platform! please place pause function on here!
#endif
			QueryPerformanceCounter(&t2);
	} while (((double)(t2.QuadPart - t1.QuadPart) / (double)(cycle_impl.tick_count * 1000000)) < dms);
#else
#if defined __EMSCRIPTEN__
	if (emscripten_has_asyncify())
	{
		emscripten_sleep(microseconds * 1000 / QN_USEC_PER_SEC);
		return;
	}
#endif
	struct timespec ts =
	{
		.tv_sec = microseconds / 1000000,
		.tv_nsec = (microseconds % 1000000) * 1000,
	};
	int error;
	do
	{
		errno = 0;
		struct timespec es =
		{
			.tv_sec = ts.tv_sec,
			.tv_nsec = ts.tv_nsec,
		};
		error = nanosleep(&es, &ts);
	} while (error && (errno == EINTR));
#endif
}


//////////////////////////////////////////////////////////////////////////
// 타이머
typedef struct QNREALTIMER
{
	QnTimer				base;

	llong				cur_time;		// 현재 시간
	llong				base_time;		// 기준 시간
	llong				last_time;		// 이전 시간

	llong				fps_time;		// FPS 초 계산용 시간
	llong				cut_time;		// FPS 제한용 시간
	uint				fps_count;		// FPS 계산용 초당 누적 갯수
	uint				cut_count;		// FPS 제한용 누적 갯수

	//ullong				fps_frame;		// FPS 계산 프레임
	//ullong				fps_count;		// FPS 계산 누적 갯수
	//double				fps_abs;		// FPS 계산 시간

	//double				prevtime;		// 이전 시간

	//ullong				frmtime;		// 프레임 시간
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

	real->cur_time = now;
	real->base_time = now;
	real->last_time = now;

	static QN_DECL_VTABLE(QNGAMBASE) qn_timer_vt =
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

	real->cur_time = now;
	real->base_time = now;
	real->last_time = now;

	real->fps_time = now;
	real->fps_count = 0;

	real->base.runtime = 0.0;
	real->base.elapsed = 0.0;
	real->base.advance = 0.0;

	real->base.fps = 0.0;
	real->base.pause = false;
}

//
void qn_timer_update(QnTimer* self)
{
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);
	llong now = qn_cycle();
	llong cbase = now - real->base_time;
	llong clast = now - real->last_time;

	real->last_time = real->cur_time;
	real->cur_time = now;

	real->base.runtime = (double)cbase / cycle_impl.tick_count;
	real->base.elapsed = (double)clast / cycle_impl.tick_count;
	real->base.advance = real->base.pause ? 0.0 : real->base.elapsed;

	if (real->base.manual == false)
		real->base.fps = (float)((double)cycle_impl.tick_count / clast);
	else
	{
		real->fps_count++;
		if (now - real->fps_time >= cycle_impl.tick_count)
		{
			real->base.fps = (float)real->fps_count;
			real->fps_time = now;
			real->fps_count = 0;
		}
	}

	if (real->base.cut > 0)
	{
		real->cut_count++;
		if (real->cut_count == real->base.cut)
		{
			real->cut_count = 0;
			real->cut_time = now;
			//qn_timer_sync(self);
		}
	}
}

//
void qn_timer_sync(QnTimer* self)
{
	qn_return_when_fail(self->cut > 0, );
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);
	llong now = qn_cycle();
	llong took = now - real->cut_time;
	llong wait = real->cut_count * cycle_impl.tick_count / real->base.cut - took;
	if (wait > 0)
		qn_ssleep((double)wait / cycle_impl.tick_count);
}

//
void qn_timer_set_cut(QnTimer* self, int cut)
{
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);
	real->base.cut = (ushort)cut;
}

//
void qn_timer_set_manual(QnTimer* self, bool manual)
{
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);
	real->base.manual = manual;
}
