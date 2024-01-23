﻿// rdh 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();
	QnDateTime dt = { .stamp = qn_now() };
	qn_outputf("테스트할꺼임 %02d:%02d:%02d.%03d", dt.hour, dt.minute, dt.second, dt.millisecond);

	qm_srand(0);
	float sum = 0.0f;
	for (int i = 0; i < 10000; i++)
	{
		float f = qm_randf();
		sum += f;
		qn_outputf("%d: %f", i, f);
	}
	qn_outputf("sum: %f, avg: %f", sum, sum / 10000.0f);

	return 0;
}
