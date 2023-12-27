#include "pch.h"
#include "qs_qn.h"

//////////////////////////////////////////////////////////////////////////
// 퀵 소트

#define QSORT_STACK_SIZE (8 * sizeof(void*) - 2)

static void qsort_swap(byte* a, byte* b, size_t stride)
{
	if (a != b)
	{
		while (stride--)
		{
			const byte n = *a;
			*a++ = *b;
			*b++ = n;
		}
	}
}

static void qsort_short_context(byte* lo, byte* hi, const size_t stride, int(*func)(void*, const void*, const void*), void* context)
{
	while (hi > lo)
	{
		byte* max = lo;

		for (byte* p = lo + stride; p <= hi; p += stride)
		{
			if ((*func)(context, p, max) > 0)
				max = p;
		}

		qsort_swap(max, hi, stride);
		hi -= stride;
	}
}

//
void qn_qsortc(void* ptr, const size_t count, const size_t stride, int(*compfunc)(void*, const void*, const void*), void* context)
{
	qn_ret_if_fail(ptr);
	qn_ret_if_fail(count > 1);
	qn_ret_if_fail(stride);
	qn_ret_if_fail(compfunc);

	// 스택&리미트 초기화
	int_fast32_t stkptr = 0;
	byte* lo = (byte*)ptr;
	byte* hi = (byte*)ptr + stride * (count - 1);

	byte* lostk[QSORT_STACK_SIZE] = { 0, };
	byte* histk[QSORT_STACK_SIZE] = { 0, };

	size_t size;
pos_recursive:
	size = (size_t)(hi - lo) / stride + 1;

	// 중간값처리를 사용해서 O(n^2) 알고리즘으로 전환
	if (size <= 8)  // 최적화된 값을 사용해야 할 것이다 -> Cut off value
		qsort_short_context(lo, hi, stride, compfunc, context);
	else
	{
		byte* mid = lo + (size / 2) * stride;

		// 처음, 중간, 끝 부터 정렬 시작
		if (compfunc(context, lo, mid) > 0)
			qsort_swap(lo, mid, stride);

		if (compfunc(context, lo, hi) > 0)
			qsort_swap(lo, hi, stride);

		if (compfunc(context, mid, hi) > 0)
			qsort_swap(mid, hi, stride);

		// 부분 정렬
		byte* lopos = lo;
		byte* hipos = hi;

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

static void qsort_short(byte* lo, byte* hi, const size_t stride, int(*func)(const void*, const void*))
{
	while (hi > lo)
	{
		byte* max = lo;

		for (byte* p = lo + stride; p <= hi; p += stride)
		{
			if ((*func)(p, max) > 0)
				max = p;
		}

		qsort_swap(max, hi, stride);
		hi -= stride;
	}
}

//
void qn_qsort(void* ptr, const size_t count, const size_t stride, int(*compfunc)(const void*, const void*))
{
	qn_ret_if_fail(ptr);
	qn_ret_if_fail(count > 1);
	qn_ret_if_fail(stride);
	qn_ret_if_fail(compfunc);

	// 스택&리미트 초기화
	int_fast32_t stkptr = 0;
	byte* lo = (byte*)ptr;
	byte* hi = (byte*)ptr + stride * (count - 1);

	byte* lostk[QSORT_STACK_SIZE] = { 0, };
	byte* histk[QSORT_STACK_SIZE] = { 0, };

	size_t size;
pos_recursive:
	size = (size_t)(hi - lo) / stride + 1;

	// 중간값처리를 사용해서 O(n^2) 알고리즘으로 전환
	if (size <= 8)  // 최적화된 값을 사용해야 할 것이다 -> Cut off value
		qsort_short(lo, hi, stride, compfunc);
	else
	{
		byte* mid = lo + (size / 2) * stride;

		// 처음, 중간, 끝 부터 정렬 시작
		if (compfunc(lo, mid) > 0)
			qsort_swap(lo, mid, stride);

		if (compfunc(lo, hi) > 0)
			qsort_swap(lo, hi, stride);

		if (compfunc(mid, hi) > 0)
			qsort_swap(mid, hi, stride);

		// 부분 정렬
		byte* lopos = lo;
		byte* hipos = hi;

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
