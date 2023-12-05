// RDH 테스트
#include <qs.h>

int main()
{
	qn_runtime(NULL);

	QgRdh* rdh = qg_rdh_new(NULL, "RDH", 800, 600, QGFLAG_IDLE | QGFLAG_RESIZABLE | QGFLAG_VSYNC);
	qn_retval_if_fail(rdh, -1);

	while (qg_rdh_loop(rdh))
	{
		QgEvent ev;
		while (qg_rdh_poll(rdh, &ev))
		{
			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
				qg_rdh_exit_loop(rdh);
		}

		if (qg_rdh_begin(rdh, true))
		{
			qg_rdh_end(rdh);
			qg_rdh_flush(rdh);
		}
	}

	qm_unload(rdh);

	return 0;
}
