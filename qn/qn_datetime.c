#include "pch.h"
#include "qn.h"

/**
 * 로컬 시간으로 변화.
 * @param [반환]	ptm	시간 구조체.
 * @param	tt	   	time_t 형식의 시간.
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
 * UTC 시간으로 변화.
 * @param [반환]	ptm	시간 구조체.
 * @param	tt	   	time_t 형식의 시간.
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
 * 현재 날짜 시간.
 * @param [반환]	dt	(널값이 아니면) 현재 날짜 시간.
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
 * 현재의 UTC 날짜 시간.
 * @param [반환]	dt	(널값이 아니면) 현재 날짜 시간.
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
 * 초를 시간으로.
 * @param	sec   	초.
 * @param [반환]	dt	(널값이 아니면) 변환된 시간.
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
 * 밀리초를 시간으로.
 * @param	msec  	밀리초.
 * @param [반환]	dt	(널값이 아니면) 변환된 시간.
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
 * 사이클 64비트.
 * @return	현재의 사이클 TICK.
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
 * 초단위 TICK.
 * @return	현재의 TICK.
 */
double qn_stick(void)
{
	static double s_cycle_per_tick = 0.001;

#if _QN_WINDOWS_
	static bool s_init = false;

	if (!s_init)
	{
		LARGE_INTEGER ll;

		if (QueryPerformanceFrequency(&ll))
			s_cycle_per_tick = 1.0 / (double)ll.QuadPart;

		s_init = true;
	}
#endif

	double t = (double)qn_cycle();
	return t * s_cycle_per_tick;
}

/**
 * 밀리초 단위의 TICK.
 * @return	현재의 TICK.
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
 * 밀리초 슬립.
 * @param	milliseconds	밀리초 단위로 처리되는 millisecond.
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
 * 마이크로초 슬립.
 * @param	microseconds	마이크로초 단위로 처리되는 microsecond.
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
 * 초 슬립.
 * @param	seconds	초 단위로 처리되는 second.
 */
void qn_ssleep(uint32_t seconds)
{
	qn_sleep(seconds * 1000);
}

/**
 * 마이크로 슬립, 정밀 시계를 이용하며 스레드 콘텍스트가 일반 슬립보다 제한된다.
 * @param	microseconds	마이크로초 단위로 처리되는 microsecond.
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

// 타이머
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
 * 타이머 만들기.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 만들어진 타이머.
 */
qnTimer* qn_timer_new(void)
{
	qnRealTimer* self = qn_alloc_1(qnRealTimer);
	qn_retval_if_fail(self, NULL);

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
 * 타이머 제거.
 * @param [입력]	self	타이머 개체
 */
void qn_timer_delete(qnTimer* self)
{
	qn_free(self);
}

/**
 * 타이머 리셋.
 * @param [입력]	self	타이머 개체
 */
void qn_timer_reset(qnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->basetime.q = impl->curtime.q;
	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = 0;
	impl->count = impl->curtime.dw.l;

	impl->stop = false;
}

/**
 * 타이머 시작.
 * @param [입력]	self	타이머 개체
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

	impl->stop = false;
}

/**
 * 타이머 정지.
 * @param [입력]	self	타이머 개체
 */
void qn_timer_stop(qnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;

	impl->curtime.q = (impl->stoptime.q != 0) ? impl->stoptime.q : qn_cycle();
	impl->lasttime.q = impl->curtime.q;
	impl->stoptime.q = impl->curtime.q;
	impl->count = impl->curtime.dw.l;

	impl->stop = true;
}

/**
 * 타이머 갱신.
 * @param [입력]	self	타이머 개체
 * @return	성공하면 참, 실패하면 거짓.
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

/**
 * 타이머의 절대 시간.
 * @param [입력]	self	타이머 개체
 * @return	double.
 */
double qn_timer_get_abs(qnTimer* self)
{
	return self->abstime;
}

/**
 * 타이머의 시작 부터의 실행 시간.
 * @param [입력]	self	타이머 개체
 * @return	double.
 */
double qn_timer_get_run(qnTimer* self)
{
	return  self->runtime;
}

/**
 * 타이머 갱신의 시간 주기(Frame Per Second)
 * @param [입력]	self	타이머 개체
 * @return	double.
 */
double qn_timer_get_fps(qnTimer* self)
{
	return self->fps;
}

/**
 * 타이머 갱신에 따른 경과 값.
 * @param [입력]	self	타이머 개체
 * @return	double.
 */
double qn_timer_get_adv(qnTimer* self)
{
	return self->advance;
}

/**
 * 타이머의 과다 수행에 따른 갱신 경과의 제한 값.
 * @param [입력]	self	타이머 개체
 * @return	double.
 */
double qn_timer_get_cut(qnTimer* self)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	return impl->cut;
}

/**
 * 타이머 과다 수행에 따른 갱신 경과값의 설정.
 * @param [입력]	self	타이머 개체
 * @param	cut			제한 값.
 */
void qn_timer_set_cut(qnTimer* self, double cut)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	impl->cut = cut;
}

/**
 * 타이머 매뉴얼 타입 FPS 측정.
 * @param [입력]	self	타이머 개체
 * @param	value   	기능을 쓰려면 참으로 넣는다.
 */
void qn_timer_set_manual(qnTimer* self, bool value)
{
	qnRealTimer* impl = (qnRealTimer*)self;
	impl->manual = value;
}

