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
	timeBeginPeriod(1);
	LARGE_INTEGER ll;
	QueryPerformanceFrequency(&ll);
	cycle_impl.tick_count = ll.QuadPart;
#else
	cycle_impl.tick_count = 1000;
#endif
	cycle_impl.start_count = qn_cycle();
}

//
void qn_cycle_down(void)
{
#ifdef _QN_WINDOWS_
	timeEndPeriod(1);
#endif
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
double qn_tickd(void)
{
	const llong cycle = qn_cycle();
	return (double)(cycle - cycle_impl.start_count) / (double)cycle_impl.tick_count;
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
	double until = qn_tickd() + seconds;
#if defined _QN_WINDOWS_
	double msec = floor(seconds * QN_MSEC_PER_SEC * 0.9);
	Sleep((DWORD)msec);
#elif defined _QN_EMSCRIPTEN_

#elif defined _QN_UNIX_
#ifdef _QN_EMSCRIPTEN_
	if (emscripten_has_asyncify())
	{
		double msec = floor(seconds * QN_MSEC_PER_SEC * 0.8);
		emscripten_sleep(msec);
	}
	else
#endif
	{
		llong nanosec = (llong)(seconds * 0.95 * QN_NSEC_PER_SEC);
		struct timespec ts =
		{
			.tv_sec = nano / QN_NSEC_PER_SEC,
			.tv_nsec = nano % QN_NSEC_PER_SEC,
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
	}
#else
#error unknown platform! please place sleep function on here!
#endif
	while (qn_tickd() < until)
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
		emscripten_sleep(0);
#endif
	}
}

//
void qn_msleep(llong microseconds)
{
	qn_ssleep((double)microseconds / QN_USEC_PER_SEC);
}


//////////////////////////////////////////////////////////////////////////
// 타이머
typedef struct QNREALTIMER
{
	QnTimer				base;

	double				inv_cut;		// 컷 역수

	llong				cur_time;		// 현재 시간
	llong				base_time;		// 기준 시간
	llong				last_time;		// 이전 시간

	llong				fps_time;		// FPS 초 계산용 시간
	uint				fps_count;		// FPS 계산용 초당 누적 갯수
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

	real->cur_time = qn_cycle();
	real->base.runtime = (double)(real->cur_time - real->base_time) / cycle_impl.tick_count;
	real->base.elapsed = (double)(real->cur_time - real->last_time) / cycle_impl.tick_count;
	real->last_time = real->cur_time;

	if (real->base.cut > 0 && real->base.elapsed < real->inv_cut)
	{
		qn_ssleep(real->inv_cut - real->base.elapsed);

		real->cur_time = qn_cycle();
		double wait = (double)(real->cur_time - real->last_time) / cycle_impl.tick_count;
		real->last_time = real->cur_time;
		real->base.elapsed += wait;
	}

	real->base.advance = real->base.pause ? 0.0 : real->base.elapsed;

	if (real->base.manual == false)
		real->base.fps = 1.0f / (float)real->base.elapsed;
	else
	{
		llong now = qn_cycle();
		real->fps_count++;
		if (now - real->fps_time >= cycle_impl.tick_count)
		{
			real->base.fps = (float)real->fps_count;
			real->fps_time = now;
			real->fps_count = 0;
		}
	}
}

//
void qn_timer_set_cut(QnTimer* self, int cut)
{
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);
	real->base.cut = (ushort)cut;
	real->inv_cut = 1.0 / (double)cut;
}

//
void qn_timer_set_manual(QnTimer* self, bool manual)
{
	QnRealTimer* real = qn_cast_type(self, QnRealTimer);
	real->base.manual = manual;
}

/*
참고 CUT 60프레임 때 타이머 오차 (E=타이머, T=시계)
E: 0.03, T: 0.00
E: 10.03, T: 10.00
E: 20.03, T: 20.00
E: 30.03, T: 30.00
E: 40.03, T: 40.00
E: 50.03, T: 50.00
E: 60.03, T: 60.01
E: 70.03, T: 70.00
E: 80.03, T: 80.01
E: 90.04, T: 90.01
E: 100.04, T: 100.01
E: 110.04, T: 110.01
E: 120.04, T: 120.01
E: 130.04, T: 130.01
E: 140.04, T: 140.01
E: 150.04, T: 150.01
E: 160.04, T: 160.01
E: 170.03, T: 170.01
E: 180.03, T: 180.01
E: 190.03, T: 190.01
E: 200.03, T: 200.01
E: 210.03, T: 210.01
E: 220.02, T: 220.01
E: 230.02, T: 230.02
E: 240.02, T: 240.02
E: 250.02, T: 250.02
E: 260.01, T: 260.02
E: 270.01, T: 270.02
E: 280.01, T: 280.02
E: 290.01, T: 290.02
E: 300.01, T: 300.02
E: 310.00, T: 310.02
E: 320.00, T: 320.02
E: 330.00, T: 330.02
E: 340.00, T: 340.03
E: 350.00, T: 350.03
E: 359.99, T: 360.03
E: 369.99, T: 370.03
E: 379.99, T: 380.03
E: 389.99, T: 390.03
E: 399.98, T: 400.03
E: 409.98, T: 410.03
E: 419.98, T: 420.03
E: 429.98, T: 430.03
E: 439.97, T: 440.03
E: 449.97, T: 450.03
E: 459.97, T: 460.03
E: 469.97, T: 470.04
E: 479.97, T: 480.04
E: 489.96, T: 490.04
E: 499.96, T: 500.04
E: 509.96, T: 510.04
E: 519.96, T: 520.04
E: 529.95, T: 530.04
E: 539.95, T: 540.04
E: 549.95, T: 550.04
E: 559.95, T: 560.04
E: 569.95, T: 570.04
E: 579.94, T: 580.04
E: 589.94, T: 590.04

참고 CUT 120프레임 때 타이머 오차 (E=타이머, T=시계)
E: 0.03, T: 0.00
E: 10.03, T: 10.01
E: 20.04, T: 20.01
E: 30.04, T: 30.01
E: 40.04, T: 40.01
E: 50.05, T: 50.02
E: 60.05, T: 60.02
E: 70.05, T: 70.03
E: 80.05, T: 80.03
E: 90.05, T: 90.03
E: 100.05, T: 100.04
E: 110.05, T: 110.04
E: 120.06, T: 120.04
E: 130.06, T: 130.05
E: 140.06, T: 140.05
E: 150.06, T: 150.06
E: 160.06, T: 160.06
E: 170.06, T: 170.06
E: 180.06, T: 180.07
E: 190.06, T: 190.07
E: 200.06, T: 200.08
E: 210.06, T: 210.08
E: 220.06, T: 220.08
E: 230.06, T: 230.09
E: 240.06, T: 240.09
E: 250.06, T: 250.10
E: 260.06, T: 260.10
E: 270.06, T: 270.10
E: 280.07, T: 280.11
E: 290.07, T: 290.11
E: 300.07, T: 300.12
E: 310.07, T: 310.12
E: 320.07, T: 320.12
E: 330.06, T: 330.12
E: 340.07, T: 340.13
E: 350.07, T: 350.14
E: 360.07, T: 360.14
E: 370.07, T: 370.14
E: 380.07, T: 380.15
E: 390.08, T: 390.16
E: 400.08, T: 400.16
E: 410.08, T: 410.17
E: 420.08, T: 420.18
E: 430.09, T: 430.18
E: 440.09, T: 440.19
E: 450.09, T: 450.19
E: 460.09, T: 460.20
E: 470.09, T: 470.20
E: 480.09, T: 480.21
E: 490.09, T: 490.21
E: 500.10, T: 500.22
E: 510.10, T: 510.22
E: 520.13, T: 520.22
E: 530.17, T: 530.23
E: 540.20, T: 540.24
E: 550.24, T: 550.24
E: 560.28, T: 560.25
E: 570.32, T: 570.25
E: 580.35, T: 580.26
E: 590.39, T: 590.26
*/
