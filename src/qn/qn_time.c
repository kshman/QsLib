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
	ullong				tick_count;		// 초 당 틱
	ullong				start_count;	// 시작 카운트
} cycle_impl = { 0, };

//
void qn_cycle_up(void)
{
#ifdef _QN_WINDOWS_
	QueryPerformanceFrequency((LARGE_INTEGER*)&cycle_impl.tick_count);
#else
	cycle_impl.tick_count = 1000;
#endif
	cycle_impl.start_count = qn_cycle();
}

//
ullong qn_cycle(void)
{
#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	QueryPerformanceCounter(&ll);
	return ll.QuadPart;
#else
	ullong n;
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
		n = ((ullong)ts.tv_sec * 1000) + ((ullong)ts.tv_nsec / 1000000);
	else
	{
		struct timeval tv;
		gettimeofday(&tv, 0);
		n = ((ullong)tv.tv_sec * 1000) + ((ullong)tv.tv_usec / 1000);
	}
	return n;
#endif
}

//
ullong qn_tick(void)
{
	const ullong cycle = qn_cycle();
	return ((cycle - cycle_impl.start_count) * 1000) / cycle_impl.tick_count;
}

//
uint qn_tick32(void)
{
	ullong tick = qn_tick();
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
void qn_msleep(ullong microseconds)
{
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

	bool32				stop;
	uint				past;		// 왜 이걸 만들었는지 기억이 안난다

	uint				fps_count;
	int					fps_frame;
	double				fps_abs;

	double              cut;
	double				prevtime;

	vint64_t			basetime;
	vint64_t			stoptime;
	vint64_t			lasttime;
	vint64_t			curtime;
	vint64_t			frmtime;

	double				tick;
	vint64_t			frame;
} QnRealTimer;

//
static void qn_timer_dispose(QnGam gam)
{
	qn_free(gam);
}

//
QnTimer* qn_create_timer(void)
{
	QnRealTimer* self = qn_alloc_zero_1(QnRealTimer);

	self->frame.q = cycle_impl.tick_count;
	self->tick = 1.0 / (double)cycle_impl.tick_count;

	self->curtime.q = qn_cycle();
	self->basetime.q = self->curtime.q;
	self->lasttime.q = self->curtime.q;

	self->base.abstime = (double)self->curtime.q * self->tick;
	self->cut = 9999999.0;  //10.0;

	static QN_DECL_VTABLE(QNGAMBASE) qn_timer_vt =
	{
		"TIMER",
		qn_timer_dispose,
	};
	return qn_gam_init(self, qn_timer_vt);
}

//
void qn_timer_reset(QnTimer* self)
{
	QnRealTimer* impl = (QnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->basetime.q = impl->curtime.q;
	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = 0;
	impl->fps_count = impl->curtime.dw.l;

	impl->stop = false;
}

//
void qn_timer_start(QnTimer* self)
{
	QnRealTimer* impl = (QnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();

	if (impl->stop)
		impl->basetime.q += impl->curtime.q - impl->stoptime.q;

	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = 0;
	impl->fps_count = impl->curtime.dw.l;

	impl->stop = false;
}

//
void qn_timer_stop(QnTimer* self)
{
	QnRealTimer* impl = (QnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = impl->curtime.q;
	impl->fps_count = impl->curtime.dw.l;

	impl->stop = true;
}

//
bool qn_timer_update(QnTimer* self, bool manual)
{
	QnRealTimer* impl = (QnRealTimer*)self;
	bool ret;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->base.abstime = (double)impl->curtime.q * impl->tick;
	impl->base.runtime = (double)(impl->curtime.q - impl->basetime.q) * impl->tick;

	if (manual == false)
		impl->base.fps = (double)impl->frame.dw.l / (double)(impl->curtime.dw.l - impl->fps_count);
	else
	{
		impl->fps_frame++;

		if ((impl->base.abstime - impl->fps_abs) >= 1.0)
		{
			impl->base.fps = (float)impl->fps_frame;
			impl->fps_abs = impl->base.abstime;
			impl->fps_frame = 0;
		}
	}

	impl->fps_count = impl->curtime.dw.l;

	if (impl->base.fps < impl->cut)
	{
		impl->base.advance = (double)(impl->curtime.q - impl->lasttime.q) * impl->tick;
		ret = true;
	}
	else
	{
		impl->base.advance = impl->cut * 0.001;
		ret = false;
	}

	impl->lasttime.q = impl->curtime.q;
	impl->past = (int)(impl->base.advance * 1000.0);
	return ret;
}

//
bool qn_timer_update_fps(QnTimer* self, bool manual, double target_fps)
{
	QnRealTimer* impl = (QnRealTimer*)self;
	impl->prevtime = impl->base.runtime;

	bool ret = qn_timer_update(self, manual);
	double delta = impl->base.runtime - impl->prevtime;

	if (delta < target_fps)
	{
		qn_ssleep(target_fps - delta);

		/*
		impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
		impl->base.abstime = (double)impl->curtime.q * impl->tick;
		impl->base.runtime = (double)(impl->curtime.q - impl->basetime.q) * impl->tick;

		double wait = impl->base.runtime - impl->prevtime;
		impl->prevtime = impl->base.runtime;

		impl->base.advance += wait;
		*/
	}

	return ret;
}

//
double qn_timer_get_cut(const QnTimer* self)
{
	const QnRealTimer* impl = (const QnRealTimer*)self;
	return impl->cut;
}

//
void qn_timer_set_cut(QnTimer* self, const double cut)
{
	QnRealTimer* impl = (QnRealTimer*)self;
	impl->cut = cut;
}
