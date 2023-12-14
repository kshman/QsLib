// stub 테스트
#include <qs.h>

int main(void)
{
	qn_runtime(NULL);

	if (!qg_open_stub(NULL, 800, 600, QGFLAG_IDLE | QGFLAG_RESIZABLE))
		return -1;

	while (qg_loop())
	{
		QgEvent ev;
		while (qg_poll(&ev))
		{
			qn_outputf("STUB EVENT => %d", ev.ev);

			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
				qg_exit_loop();
		}
	}

	return 0;
}
