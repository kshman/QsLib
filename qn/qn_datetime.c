#include "pch.h"
#include "qn.h"

/**
 * ���� �ð����� ��ȭ.
 * @param [��ȯ]	ptm	�ð� ����ü.
 * @param	tt	   	time_t ������ �ð�.
 */
void qn_localtime(struct tm* ptm, const time_t tt)
{
#if _MSC_VER
	localtime_s(ptm, &tt);
#elif __GNUC__
	localtime_r(&tt, ptm);
#else
	if (ptm)
		*ptm = *localtime(&tt);
#endif
}

/**
 * UTC �ð����� ��ȭ.
 * @param [��ȯ]	ptm	�ð� ����ü.
 * @param	tt	   	time_t ������ �ð�.
 */
void qn_gmtime(struct tm* ptm, const time_t tt)
{
#if _MSC_VER
	gmtime_s(ptm, &tt);
#elif __GNUC__
	gmtime_r(&tt, ptm);
#else
	if (ptm)
		ptm = gmtime(&tt);
#endif
}

/**
 * ���� ��¥ �ð�.
 * @param [��ȯ]	dt	(�ΰ��� �ƴϸ�) ���� ��¥ �ð�.
 */
void qn_now(qnDateTime* dt)
{
	qn_ret_if_fail(dt != NULL);

#if _QN_WINDOWS_
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
	k_localtime(&tm, tv.tv_sec);

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

/**
 * ������ UTC ��¥ �ð�.
 * @param [��ȯ]	dt	(�ΰ��� �ƴϸ�) ���� ��¥ �ð�.
 */
void qn_utc(qnDateTime* dt)
{
	qn_ret_if_fail(dt != NULL);

#if _QN_WINDOWS_
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
	k_gmtime(&tm, tv.tv_sec);

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

/**
 * �ʸ� �ð�����.
 * @param	sec   	��.
 * @param [��ȯ]	dt	(�ΰ��� �ƴϸ�) ��ȯ�� �ð�.
 */
void qn_stod(double sec, qnDateTime* dt)
{
	qn_ret_if_fail(dt != NULL);

	uint32_t ns = (uint32_t)sec;
	dt->hour = ns / 3600;
	dt->minute = (ns % 3600) / 60;
	dt->second = (ns % 60);

	double m = (double)(uint64_t)sec;
	m = sec - m;
	m *= 1000.0;
	dt->millisecond = (uint32_t)m;
}

/**
 * �и��ʸ� �ð�����.
 * @param	msec  	�и���.
 * @param [��ȯ]	dt	(�ΰ��� �ƴϸ�) ��ȯ�� �ð�.
 */
void qn_mstod(uint32_t msec, qnDateTime* dt)
{
	qn_ret_if_fail(dt != NULL);

	uint32_t ns = msec / 1000;
	dt->hour = ns / 3600;
	dt->minute = (ns % 3600) / 60;
	dt->second = (ns % 60);
	dt->millisecond = msec - (ns * 1000);
}

/**
 * ����Ŭ 64��Ʈ.
 * @return	������ ����Ŭ TICK.
 */
uint64_t qn_cycle(void)
{
#if _QN_WINDOWS_
	LARGE_INTEGER ll;
	QueryPerformanceCounter(&ll);
	return ll.QuadPart;
#elif _QN_BSD_
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

/**
 * �ʴ��� TICK.
 * @return	������ TICK.
 */
double qn_stick(void)
{
	static double s_cycle_per_tick = 0.001;

#if _QN_WINDOWS_
	static bool s_init = FALSE;

	if (!s_init)
	{
		LARGE_INTEGER ll;

		if (QueryPerformanceFrequency(&ll))
			s_cycle_per_tick = 1.0 / (double)ll.QuadPart;

		s_init = TRUE;
	}
#endif

	double t = (double)qn_cycle();
	return t * s_cycle_per_tick;
}

/**
 * �и��� ������ TICK.
 * @return	������ TICK.
 */
uint64_t qn_tick(void)
{
#if _QN_WINDOWS_
	return (uint64_t)(qn_stick() * 1000.0);
#else
	return qn_cycle();
#endif
}

/**
 * �и��� ����.
 * @param	milliseconds	�и��� ������ ó���Ǵ� millisecond.
 */
void qn_sleep(uint32_t milliseconds)
{
#if _QN_WINDOWS_
	Sleep(milliseconds);
#else
	uint32_t s = milliseconds / 1000;
	uint32_t u = (milliseconds % 1000) * 1000;
	sleep(s);
	usleep(u);
#endif
}

/**
 * ����ũ���� ����.
 * @param	microseconds	����ũ���� ������ ó���Ǵ� microsecond.
 */
void qn_usleep(uint32_t microseconds)
{
#if _QN_WINDOWS_
	qn_sleep(microseconds / 1000);
#else
	usleep(microseconds);
#endif
}

/**
 * �� ����.
 * @param	seconds	�� ������ ó���Ǵ� second.
 */
void qn_ssleep(uint32_t seconds)
{
	qn_sleep(seconds * 1000);
}

/**
 * ����ũ�� ����, ���� �ð踦 �̿��ϸ� ������ ���ؽ�Ʈ�� �Ϲ� �������� ���ѵȴ�.
 * @param	microseconds	����ũ���� ������ ó���Ǵ� microsecond.
 */
void qn_msleep(uint64_t microseconds)
{
#if _QN_WINDOWS_
	double dms = (double)microseconds;
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
		} while (((double)(t2.QuadPart - t1.QuadPart) / freq.QuadPart * 1000000) < dms);
	}
#elif _SB_UNIX_
	struct timespec ts;
	ts.tv_sec = microseconds / 1000000;
	ts.tv_nsec = (microseconds % 1000000) * 1000;

	while (nanosleep(&ts, &ts))
	{
		if (errno != EINTR)
			break;
	}
#else
	usleep(microseconds);
#endif
}

// Ÿ�̸�
typedef struct qnRealTimer
{
	qnTimer				base;

	bool				stop;
	int					past;
	double              cut;

	vlong_t				basetime;
	vlong_t				stoptime;
	vlong_t				lasttime;
	vlong_t				curtime;
	vlong_t				frmtime;

	uint32_t			count;

	double				tick;
	vlong_t				frame;

	bool				manual;
	double				fps_abs;
	int					fps_frame;
} qnRealTimer;

/**
 * Ÿ�̸� �����.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� ������� Ÿ�̸�.
 */
qnTimer* qn_timer_new(void)
{
	qnRealTimer* self = qn_alloc_1(qnRealTimer);

#if _QN_WINDOWS_
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

	return (qnTimer*)self;
}

/**
 * Ÿ�̸� ����.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 */
void qn_timer_delete(qnTimer* self)
{
	qn_free(self);
}

/**
 * Ÿ�̸� ����.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 */
void qn_timer_reset(qnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->basetime.q = impl->curtime.q;
	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = 0;
	impl->count = impl->curtime.dw.l;

	impl->stop = FALSE;
}

/**
 * Ÿ�̸� ����.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 */
void qn_timer_start(qnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();

	if (impl->stop)
		impl->basetime.q += impl->curtime.q - impl->stoptime.q;

	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = 0;
	impl->count = impl->curtime.dw.l;

	impl->stop = FALSE;
}

/**
 * Ÿ�̸� ����.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 */
void qn_timer_stop(qnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = impl->curtime.q;
	impl->count = impl->curtime.dw.l;

	impl->stop = TRUE;
}

/**
 * Ÿ�̸� ����.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 * @return	�����ϸ� ��, �����ϸ� ����.
 */
bool qn_timer_update(qnTimer* self)
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
		ret = TRUE;
	}
	else
	{
		impl->base.advance = impl->cut * 0.001;
		ret = FALSE;
	}

	impl->lasttime.q = impl->curtime.q;
	impl->past = (int)(impl->base.advance * 1000.0);
	return ret;
}

/**
 * Ÿ�̸��� ���� �ð�.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 * @return	double.
 */
double qn_timer_get_abs(qnTimer* self)
{
	return self->abstime;
}

/**
 * Ÿ�̸��� ���� ������ ���� �ð�.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 * @return	double.
 */
double qn_timer_get_run(qnTimer* self)
{
	return  self->runtime;
}

/**
 * Ÿ�̸� ������ �ð� �ֱ�(Frame Per Second)
 * @param [�Է�]	self	Ÿ�̸� ��ü
 * @return	double.
 */
double qn_timer_get_fps(qnTimer* self)
{
	return self->fps;
}

/**
 * Ÿ�̸� ���ſ� ���� ��� ��.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 * @return	double.
 */
double qn_timer_get_adv(qnTimer* self)
{
	return self->advance;
}

/**
 * Ÿ�̸��� ���� ���࿡ ���� ���� ����� ���� ��.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 * @return	double.
 */
double qn_timer_get_cut(qnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	return impl->cut;
}

/**
 * Ÿ�̸� ���� ���࿡ ���� ���� ������� ����.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 * @param	cut			���� ��.
 */
void qn_timer_set_cut(qnTimer* self, double cut)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	impl->cut = cut;
}

/**
 * Ÿ�̸� �Ŵ��� Ÿ�� FPS ����.
 * @param [�Է�]	self	Ÿ�̸� ��ü
 * @param	value   	����� ������ ������ �ִ´�.
 */
void qn_timer_set_manual(qnTimer* self, bool value)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	impl->manual = value;
}

