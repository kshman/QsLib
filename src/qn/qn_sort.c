#include "pch.h"
#include "qs_qn.h"

//////////////////////////////////////////////////////////////////////////
// 퀵 소트

#define QSORT_STACK_SIZE (8 * sizeof(void*) - 2)

static void qsort_swap(uint8_t* a, uint8_t* b, size_t stride)
{
	if (a != b)
	{
		while (stride--)
		{
			const uint8_t n = *a;
			*a++ = *b;
			*b++ = n;
		}
	}
}

static void qsort_short_context(uint8_t* lo, uint8_t* hi, size_t stride, int(*func)(void*, const void*, const void*), void* context)
{
	while (hi > lo)
	{
		uint8_t* max = lo;

		for (uint8_t* p = lo + stride; p <= hi; p += stride)
		{
			if ((*func)(context, p, max) > 0)
				max = p;
		}

		qsort_swap(max, hi, stride);
		hi -= stride;
	}
}

/**
 * @brief 콘텍스트 입력 받는 퀵정렬
 * @param	ptr				정렬할 데이터의 포인터
 * @param	count			데이터의 갯수
 * @param	stride			데이터의 폭
 * @param[in]	compfunc	비교 연산 콜백 함수
 * @param	context			콜백 함수용 콘텍스트
 */
void qn_qsortc(void* ptr, size_t count, size_t stride, int(*compfunc)(void*, const void*, const void*), void* context)
{
	qn_ret_if_fail(ptr);
	qn_ret_if_fail(count > 1);
	qn_ret_if_fail(stride);
	qn_ret_if_fail(compfunc);

	// 스택&리미트 초기화
	int_fast32_t stkptr = 0;
	uint8_t* lo = (uint8_t*)ptr;
	uint8_t* hi = (uint8_t*)ptr + stride * (count - 1);

	uint8_t* lostk[QSORT_STACK_SIZE] = { 0, };
	uint8_t* histk[QSORT_STACK_SIZE] = { 0, };

	size_t size;
pos_recursive:
	size = (hi - lo) / stride + 1;

	// 중간값처리를 사용해서 O(n^2) 알고리즘으로 전환
	if (size <= 8)  // 최적화된 값을 사용해야 할 것이다 -> Cut off value
		qsort_short_context(lo, hi, stride, compfunc, context);
	else
	{
		uint8_t* mid = lo + (size / 2) * stride;

		// 처음, 중간, 끝 부터 정렬 시작
		if (compfunc(context, lo, mid) > 0)
			qsort_swap(lo, mid, stride);

		if (compfunc(context, lo, hi) > 0)
			qsort_swap(lo, hi, stride);

		if (compfunc(context, mid, hi) > 0)
			qsort_swap(mid, hi, stride);

		// 부분 정렬
		uint8_t* lopos = lo;
		uint8_t* hipos = hi;

		for (;;)
		{
			if (mid > lopos)
			{
				do
				{
					lopos += stride;
				} while (lopos < mid && compfunc(context, lopos, mid) <= 0);
			}

			if (mid <= lopos)
			{
				do
				{
					lopos += stride;
				} while (lopos <= hi && compfunc(context, lopos, mid) <= 0);
			}

			do
			{
				hipos -= stride;
			} while (hipos > mid && compfunc(context, hipos, mid) > 0);

			if (hipos < lopos)
				break;

			qsort_swap(lopos, hipos, stride);

			if (mid == hipos)
				mid = lopos;
		}

		hipos += stride;

		if (mid < hipos)
		{
			do
			{
				hipos -= stride;
			} while (hipos > mid && compfunc(context, hipos, mid) == 0);
		}

		if (mid >= hipos)
		{
			do
			{
				hipos -= stride;
			} while (hipos > lo && compfunc(context, hipos, mid) == 0);
		}

		if (hipos - lo >= hi - lopos)
		{
			if (lo < hipos)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = hipos;
				++stkptr;
			}

			if (lopos < hi)
			{
				lo = lopos;
				goto pos_recursive;
			}
		}
		else
		{
			if (lopos < hi)
			{
				lostk[stkptr] = lopos;
				histk[stkptr] = hi;
				++stkptr;
			}

			if (lo < hipos)
			{
				hi = hipos;
				goto pos_recursive;
			}
		}
	}

	--stkptr;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto pos_recursive;
	}
}

static void qsort_short(uint8_t* lo, uint8_t* hi, size_t stride, int(*func)(const void*, const void*))
{
	while (hi > lo)
	{
		uint8_t* max = lo;

		for (uint8_t* p = lo + stride; p <= hi; p += stride)
		{
			if ((*func)(p, max) > 0)
				max = p;
		}

		qsort_swap(max, hi, stride);
		hi -= stride;
	}
}

/**
 * @brief 퀵정렬
 * @param	ptr				정렬할 데이터의 포인터
 * @param	count			데이터의 갯수
 * @param	stride			데이터의 폭
 * @param[in]	compfunc	비교 연산 콜백 함수
 */
void qn_qsort(void* ptr, size_t count, size_t stride, int(*compfunc)(const void*, const void*))
{
	qn_ret_if_fail(ptr);
	qn_ret_if_fail(count > 1);
	qn_ret_if_fail(stride);
	qn_ret_if_fail(compfunc);

	// 스택&리미트 초기화
	int_fast32_t stkptr = 0;
	uint8_t* lo = (uint8_t*)ptr;
	uint8_t* hi = (uint8_t*)ptr + stride * (count - 1);

	uint8_t* lostk[QSORT_STACK_SIZE] = { 0, };
	uint8_t* histk[QSORT_STACK_SIZE] = { 0, };

	size_t size;
pos_recursive:
	size = (hi - lo) / stride + 1;

	// 중간값처리를 사용해서 O(n^2) 알고리즘으로 전환
	if (size <= 8)  // 최적화된 값을 사용해야 할 것이다 -> Cut off value
		qsort_short(lo, hi, stride, compfunc);
	else
	{
		uint8_t* mid = lo + (size / 2) * stride;

		// 처음, 중간, 끝 부터 정렬 시작
		if (compfunc(lo, mid) > 0)
			qsort_swap(lo, mid, stride);

		if (compfunc(lo, hi) > 0)
			qsort_swap(lo, hi, stride);

		if (compfunc(mid, hi) > 0)
			qsort_swap(mid, hi, stride);

		// 부분 정렬
		uint8_t* lopos = lo;
		uint8_t* hipos = hi;

		for (;;)
		{
			if (mid > lopos)
			{
				do
				{
					lopos += stride;
				} while (lopos < mid && compfunc(lopos, mid) <= 0);
			}

			if (mid <= lopos)
			{
				do
				{
					lopos += stride;
				} while (lopos <= hi && compfunc(lopos, mid) <= 0);
			}

			do
			{
				hipos -= stride;
			} while (hipos > mid && compfunc(hipos, mid) > 0);

			if (hipos < lopos)
				break;

			qsort_swap(lopos, hipos, stride);

			if (mid == hipos)
				mid = lopos;
		}

		hipos += stride;

		if (mid < hipos)
		{
			do
			{
				hipos -= stride;
			} while (hipos > mid && compfunc(hipos, mid) == 0);
		}

		if (mid >= hipos)
		{
			do
			{
				hipos -= stride;
			} while (hipos > lo && compfunc(hipos, mid) == 0);
		}

		if (hipos - lo >= hi - lopos)
		{
			if (lo < hipos)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = hipos;
				++stkptr;
			}

			if (lopos < hi)
			{
				lo = lopos;
				goto pos_recursive;
			}
		}
		else
		{
			if (lopos < hi)
			{
				lostk[stkptr] = lopos;
				histk[stkptr] = hi;
				++stkptr;
			}

			if (lo < hipos)
			{
				hi = hipos;
				goto pos_recursive;
			}
		}
	}

	--stkptr;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto pos_recursive;
	}
}
