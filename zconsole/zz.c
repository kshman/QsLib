// rdh 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	int flags = /*QGFLAG_BORDERLESS |*/ QGFLAG_RESIZABLE;
	QgRdh* rdh = qg_rdh_new(NULL, "RDH", 0, 0, 0, flags);
	qn_val_if_fail(rdh, -1);

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

	qs_unload(rdh);

	return 0;
}
