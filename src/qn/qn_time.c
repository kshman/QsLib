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
void qn_now(QnDateTime* dt)
{
	qn_ret_if_fail(dt != NULL);

#ifdef _QN_WINDOWS_
	SYSTEMTIME st;
	GetLocalTime(&st);

	dt->year = st.wYear;
	dt->month = st.wMonth;
	dt->day = st.wDay;
	dt->dow = st.wDayOfWeek;
	dt->hour = st.wHour;
	dt->minute = st.wMinute;
	dt->second = st.wSecond;
	dt->millisecond = st.wMilliseconds;
#else
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv, NULL);
	qn_localtime(&tm, tv.tv_sec);

	dt->year = tm.tm_year + 1900;
	dt->month = tm.tm_mon + 1;
	dt->day = tm.tm_mday;
	dt->dow = tm.tm_wday;
	dt->hour = tm.tm_hour;
	dt->minute = tm.tm_min;
	dt->second = tm.tm_sec;
	dt->millisecond = tv.tv_usec / 1000;
#endif
}

//
void qn_utc(QnDateTime* dt)
{
	qn_ret_if_fail(dt != NULL);

#ifdef _QN_WINDOWS_
	SYSTEMTIME st;
	GetSystemTime(&st);

	dt->year = st.wYear;
	dt->month = st.wMonth;
	dt->day = st.wDay;
	dt->dow = st.wDayOfWeek;
	dt->hour = st.wHour;
	dt->minute = st.wMinute;
	dt->second = st.wSecond;
	dt->millisecond = st.wMilliseconds;
#else
	struct timeval tv;
	struct tm tm;
	gettimeofday(&tv, NULL);
	qn_gmtime(&tm, tv.tv_sec);

	dt->year = tm.tm_year + 1900;
	dt->month = tm.tm_mon + 1;
	dt->day = tm.tm_mday;
	dt->dow = tm.tm_wday;
	dt->hour = tm.tm_hour;
	dt->minute = tm.tm_min;
	dt->second = tm.tm_sec;
	dt->millisecond = tv.tv_usec / 1000;
#endif
}

//
void qn_stod(double sec, QnDateTime* dt)
{
	qn_ret_if_fail(dt != NULL);

	const uint32_t ns = (uint32_t)sec;
	dt->hour = ns / 3600;
	dt->minute = (ns % 3600) / 60;
	dt->second = (ns % 60);

	double m = (double)(uint64_t)sec;
	m = sec - m;
	m *= 1000.0;
	dt->millisecond = (uint32_t)m;
}

//
void qn_mstod(uint msec, QnDateTime* dt)
{
	qn_ret_if_fail(dt != NULL);

	const uint ns = msec / 1000;
	dt->hour = ns / 3600;
	dt->minute = (ns % 3600) / 60;
	dt->second = (ns % 60);
	dt->millisecond = msec - (ns * 1000);
}

//
ullong qn_cycle(void)
{
#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	QueryPerformanceCounter(&ll);
	return ll.QuadPart;
#elif defined _QN_BSD_
	uint64_t n;
	struct timespec tp;

	if (clock_gettime(CLOCK_REALTIME, &tp) == 0)
		n = ((uint64_t)tp.tv_sec * 1000) + ((uint64_t)tp.tv_nsec / 1000000);
	else
	{
		struct timeval tv;
		gettimeofday(&tv, 0);
		n = ((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000);
	}

	return n;
#else
	uint64_t n;
	struct timeval tv;
	gettimeofday(&tv, 0);
	n = (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
	return n;
#endif
}

//
double qn_stick(void)
{
	static double s_cycle_per_tick = 0.001;

#ifdef _QN_WINDOWS_
	static bool s_init = false;

	if (!s_init)
	{
		LARGE_INTEGER ll;

		if (QueryPerformanceFrequency(&ll))
			s_cycle_per_tick = 1.0 / (double)ll.QuadPart;

		s_init = true;
	}
#endif

	const double t = (double)qn_cycle();  // NOLINT
	return t * s_cycle_per_tick;
}

//
ullong qn_tick(void)
{
#ifdef _QN_WINDOWS_
	return (uint64_t)(qn_stick() * 1000.0);
#else
	return qn_cycle();
#endif
}

//
void qn_sleep(uint milliseconds)
{
#ifdef _QN_WINDOWS_
	Sleep(milliseconds);
#else
	uint32_t s = milliseconds / 1000;
	uint32_t u = (milliseconds % 1000) * 1000;
	sleep(s);
	usleep(u);
#endif
}

//
void qn_usleep(uint microseconds)
{
#ifdef _QN_WINDOWS_
	qn_sleep(microseconds / 1000);
#else
	usleep(microseconds);
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
	LARGE_INTEGER t1, t2, freq;

	if (!QueryPerformanceFrequency(&freq))
		qn_usleep((uint32_t)microseconds);
	else
	{
		QueryPerformanceCounter(&t1);

		do
		{
			SwitchToThread();
			QueryPerformanceCounter(&t2);
		} while (((double)(t2.QuadPart - t1.QuadPart) / freq.QuadPart * 1000000) < dms);  // NOLINT
	}
#else
	struct timespec ts;
	ts.tv_sec = microseconds / 1000000;
	ts.tv_nsec = (microseconds % 1000000) * 1000;

	while (nanosleep(&ts, &ts))
	{
		if (errno != EINTR)
			break;
	}
#endif
}

// 타이머
typedef struct QnRealTimer
{
	QnTimer				base;

	bool				stop;
	int					past;
	double              cut;

	vint64_t			basetime;
	vint64_t			stoptime;
	vint64_t			lasttime;
	vint64_t			curtime;
	vint64_t			frmtime;

	uint32_t			count;

	double				tick;
	vint64_t			frame;

	bool				manual;
	double				fps_abs;
	int					fps_frame;
} qnRealTimer;

//
QnTimer* qn_timer_new(void)
{
	qnRealTimer* self = qn_alloc_1(qnRealTimer);
	qn_retval_if_fail(self, NULL);

#ifdef _QN_WINDOWS_
	LARGE_INTEGER ll;
	if (!QueryPerformanceFrequency(&ll))
	{
		self->frame.q = 1000;
		self->tick = 0.001;
	}
	else
	{
		self->frame.q = ll.QuadPart;
		self->tick = 1.0 / (double)ll.QuadPart;
	}
#else
	self->frame.q = 1000;
	self->tick = 0.001;
#endif

	self->curtime.q = qn_cycle();
	self->basetime.q = self->curtime.q;
	self->lasttime.q = self->curtime.q;
	self->count = self->curtime.dw.l;

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
bool qn_timer_update(QnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	bool ret;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->base.abstime = (double)impl->curtime.q * impl->tick;
	impl->base.runtime = (double)(impl->curtime.q - impl->basetime.q) * impl->tick;

	if (!impl->manual)
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
double qn_timer_get_abs(const QnTimer* self)
{
	return self->abstime;
}

//
double qn_timer_get_run(const QnTimer* self)
{
	return  self->runtime;
}

//
double qn_timer_get_fps(const QnTimer* self)
{
	return self->fps;
}

//
double qn_timer_get_adv(const QnTimer* self)
{
	return self->advance;
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

//
void qn_timer_set_manual(QnTimer* self, bool value)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	impl->manual = value;
}

