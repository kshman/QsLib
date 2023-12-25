#include "pch.h"
#include "qs_qn.h"
#ifdef _QN_UNIX_
#include <unistd.h>
#include <errno.h>
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

//
QnTimeStamp qn_now(void)
{
	QnDateTime dt;
#ifdef _QN_WINDOWS_
	SYSTEMTIME st;
	GetLocalTime(&st);

	dt.year = st.wYear;
	dt.month = st.wMonth;
	dt.day = st.wDay;
	dt.dow = st.wDayOfWeek;
	dt.hour = st.wHour;
	dt.minute = st.wMinute;
	dt.second = st.wSecond;
	dt.millisecond = st.wMilliseconds;
#else
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv, NULL);
	qn_localtime(&tm, tv.tv_sec);

	dt.year = (uint)tm.tm_year + 1900;
	dt.month = (uint)tm.tm_mon + 1;
	dt.day = (uint)tm.tm_mday;
	dt.dow = (uint)tm.tm_wday;
	dt.hour = (uint)tm.tm_hour;
	dt.minute = (uint)tm.tm_min;
	dt.second = (uint)tm.tm_sec;
	dt.millisecond = (uint)tv.tv_usec / 1000;
#endif
	return dt.stamp;
}

//
QnTimeStamp qn_utc(void)
{
	QnDateTime dt;
#ifdef _QN_WINDOWS_
	SYSTEMTIME st;
	GetSystemTime(&st);

	dt.year = st.wYear;
	dt.month = st.wMonth;
	dt.day = st.wDay;
	dt.dow = st.wDayOfWeek;
	dt.hour = st.wHour;
	dt.minute = st.wMinute;
	dt.second = st.wSecond;
	dt.millisecond = st.wMilliseconds;
#else
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv, NULL);
	qn_gmtime(&tm, tv.tv_sec);

	dt.year = (uint)tm.tm_year + 1900;
	dt.month = (uint)tm.tm_mon + 1;
	dt.day = (uint)tm.tm_mday;
	dt.dow = (uint)tm.tm_wday;
	dt.hour = (uint)tm.tm_hour;
	dt.minute = (uint)tm.tm_min;
	dt.second = (uint)tm.tm_sec;
	dt.millisecond = (uint)tv.tv_usec / 1000;
#endif
	return dt.stamp;
}

//
QnTimeStamp qn_stod(double sec)
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
QnTimeStamp qn_mstod(uint msec)
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
static struct CycleImpl
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
	ullong cycle = qn_cycle();
	return ((cycle - cycle_impl.start_count) * 1000) / cycle_impl.tick_count;
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
void qn_ssleep(uint seconds)
{
	qn_sleep(seconds * 1000);
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
			qn_pause();
		QueryPerformanceCounter(&t2);
	} while (((double)(t2.QuadPart - t1.QuadPart) / cycle_impl.tick_count * 1000000) < dms);
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
typedef struct QnRealTimer
{
	QnTimer				base;

	bool32				stop;
	int					past;
	double              cut;

	vint64_t			basetime;
	vint64_t			stoptime;
	vint64_t			lasttime;
	vint64_t			curtime;
	vint64_t			frmtime;

	double				tick;
	vint64_t			frame;

	double				fps_abs;
	int					fps_frame;

	uint			count;
} qnRealTimer;

//
QnTimer* qn_timer_new(void)
{
	qnRealTimer* self = qn_alloc_1(qnRealTimer);
	qn_val_if_fail(self, NULL);

	self->frame.q = cycle_impl.tick_count;
	self->tick = 1.0 / (double)cycle_impl.tick_count;

	self->curtime.q = qn_cycle();
	self->basetime.q = self->curtime.q;
	self->lasttime.q = self->curtime.q;

	self->cut = 9999999.0;  //10.0;

	return (QnTimer*)self;
}

//
void qn_timer_delete(QnTimer* self)
{
	qn_free(self);
}

//
void qn_timer_reset(QnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->basetime.q = impl->curtime.q;
	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = 0;
	impl->count = impl->curtime.dw.l;

	impl->stop = false;
}

//
void qn_timer_start(QnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();

	if (impl->stop)
		impl->basetime.q += impl->curtime.q - impl->stoptime.q;

	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = 0;
	impl->count = impl->curtime.dw.l;

	impl->stop = false;
}

//
void qn_timer_stop(QnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = impl->curtime.q;
	impl->count = impl->curtime.dw.l;

	impl->stop = true;
}

//
bool qn_timer_update(QnTimer* self, bool manual)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	bool ret;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->base.abstime = (double)impl->curtime.q * impl->tick;
	impl->base.runtime = (double)(impl->curtime.q - impl->basetime.q) * impl->tick;

	if (manual == false)
		impl->base.fps = (double)impl->frame.dw.l / (double)(impl->curtime.dw.l - impl->count);
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

	impl->count = impl->curtime.dw.l;

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
double qn_timer_get_cut(const QnTimer* self)
{
	const qnRealTimer* impl = (const qnRealTimer*)self;
	return impl->cut;
}

//
void qn_timer_set_cut(QnTimer* self, double cut)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	impl->cut = cut;
}
