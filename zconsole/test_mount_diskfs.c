﻿// DISK FS 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	QnMount* mnt = qn_open_mount(NULL, 0);

	QnStream* st = qn_open_stream(mnt, "test.txt", "wt");
	if (st != NULL)
	{
		qn_stream_write(st, "Hello, world!\n", 0, 14);
		qn_stream_printf(st, "This is a test. int: %d, float: %f, llong: %llu, size: %zu\n",
			123, 456.789f, 123456789LL, sizeof(QnMount));
		QnDateTime dt = { qn_now() };
		qn_stream_printf(st, "NOW: %04d-%02d-%02d %02d:%02d:%02d\n",
			dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
		qn_unload(st);
	}

	QnDir* dir = qn_open_dir(mnt, NULL, NULL);
	if (dir != NULL)
	{
		QnFileInfo fi;
		while (qn_dir_read_info(dir, &fi))
		{
			QnDateTime ft = { fi.stc };
			qn_outputf("[%s] %s (%u) [%04d-%02d-%02d %02d:%02d:%02d]",
				QN_TMASK(fi.attr, QNFATTR_DIR) ? "디렉토리" : "파일", fi.name, fi.size,
				ft.year, ft.month, ft.day, ft.hour, ft.minute, ft.second);
		}
		qn_unload(dir);
	}

	qn_remove_file(mnt, "test.txt");
	qn_unload(mnt);

	return 0;
}
