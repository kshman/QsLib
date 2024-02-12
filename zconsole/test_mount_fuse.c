// 퓨즈 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	QnMount* fuse = qn_create_fuse(NULL, true, true);
	if (fuse != NULL)
	{
		qn_fuse_add_hfs(fuse, "res.hfs");

		QnStream* stream = qn_mount_open_stream(fuse, "/image/ff14_autumn.bmp", "");
		if (stream)
			qn_unload(stream);

		qn_sym_dbgout();
		qn_mpf_dbgout();
		qn_unload(fuse);
	}

	return 0;
}
