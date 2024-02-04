// 스프라이트 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	char sz[24];
	qn_snprintf(sz, QN_COUNTOF(sz), "Hello, %s!", "123456789012345678901234567890");
	printf("%s\n", sz);

	char* psz;
	psz = qn_apsprintf("Hello, %s!. This is city light. int: %d, float: %f, llong: %llu, size: %zu",
		"world", 123, 456.789f, 123456789LL, sizeof(psz));
	printf("%s\n", psz);
	qn_free(psz);

	float f = 123.456f;
	qn_outputf("실수표시: %f / %g / %a / %A", f, f, f, f);
	printf("실수표시: %f / %g / %a / %A\n", f, f, f, f);

	QnMount* mnt = qn_open_mount(NULL, 0);
	QnStream* st = qn_mount_open_stream(mnt, "test.txt", "wt");
	qn_stream_write(st, "Hello, world!\n", 0, 14);
	qn_stream_printf(st, "This is a test. int: %d, float: %f, llong: %llu, size: %zu\n",
		123, 456.789f, 123456789LL, sizeof(psz));
	QnDateTime dt = { qn_now() };
	qn_stream_printf(st, "NOW: %04d-%02d-%02d %02d:%02d:%02d\n",
		dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
	qn_unload(st);
	qn_unload(mnt);

	return 0;
}
