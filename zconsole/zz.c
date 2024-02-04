// 스프라이트 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	printf("테스트!!!\n");

	QnMount* mnt = qn_open_mount(NULL, 0);
	QnStream* st = qn_mount_open_stream(mnt, "test.txt", "wt");
	qn_stream_write(st, "Hello, world!\n", 0, 14);
	qn_stream_printf(st, "This is a test. int: %d, float: %f, llong: %llu, size: %zu\n",
		123, 456.789f, 123456789LL, sizeof(QnMount));
	QnDateTime dt = { qn_now() };
	qn_stream_printf(st, "NOW: %04d-%02d-%02d %02d:%02d:%02d\n",
		dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
	qn_unload(st);
	qn_mount_remove(mnt, "test.txt");
	qn_unload(mnt);

	return 0;
}
