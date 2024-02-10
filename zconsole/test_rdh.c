// rdh 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	int flags = /*QGFLAG_BORDERLESS |*/ QGFLAG_RESIZE | QGFLAG_VSYNC | QGFLAG_MSAA;
	if (qg_open_rdh(NULL, "RDH", 0, 0, 0, flags, 0) == false)
		return -1;

	while (qg_loop())
	{
		QgEvent ev;
		while (qg_poll(&ev))
		{
			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
				qg_exit_loop();
		}

		if (qg_begin_render(true))
		{
			qg_end_render(false);
			qg_flush();
		}
	}

	qg_close_rdh();

	return 0;
}
