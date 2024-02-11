// 퓨즈 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	QnMount* fuse = qn_create_fuse(NULL, true, true);
	if (fuse!= NULL)
	{
		qn_sym_dbgout();
		qn_mpf_dbgout();
		qn_unload(fuse);
	}

	return 0;
}
