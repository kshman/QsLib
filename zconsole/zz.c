﻿// rdh 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();
	QnDateTime dt = { .stamp = qn_now() };
	qn_outputf("테스트할꺼임 %02d:%02d:%02d.%03d", dt.hour, dt.minute, dt.second, dt.millisecond);
	return 0;
}
