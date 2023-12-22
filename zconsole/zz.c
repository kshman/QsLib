// printf 와 outputf
#include <qs.h>

int main(void)
{
	int v[2];
	qn_runtime(v);

	printf("PRINT 256\n");
	qn_outputf("PRINT 256\n");

	printf("%d\n", 256);
	qn_outputf("%d\n", 256);

	printf("버전: %d.%d\n", v[0], v[1]);
	qn_outputf("버전: %d.%d\n", v[0], v[1]);

	QnDateTime dt;
	qn_now(&dt);
	printf("시간: %02d:%02d:%02d.%03d\n", dt.hour, dt.minute, dt.second, dt.millisecond);
	qn_outputf("시간: %02d:%02d:%02d.%03d\n", dt.hour, dt.minute, dt.second, dt.millisecond);

	char* str;
	int len = qn_asprintf(&str, "이것은 숫자:%d, 실수:%f, 문자열:%s", 12, 12.345, "문자열");
	printf("길이: %d, 내용:%s\n", len, str);
	qn_outputf("길이: %d, 내용:%s\n", len, str);
	qn_free(str);

	return 0;
}
