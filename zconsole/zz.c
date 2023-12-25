// stub 테스트
#include <qs.h>

int main()
{
	qn_runtime();

	int flags = /*QGFEATURE_DISABLE_ACS |*/ QGFEATURE_DISABLE_SCRSAVE | QGFEATURE_ENABLE_IDLE | QGFEATURE_ENABLE_DROP;
	if (!qg_open_stub(NULL, 0, 0, 0, QGFLAG_RESIZABLE | flags))
		return -1;

	while (qg_loop())
	{
		QgEvent ev;
		while (qg_poll(&ev))
		{
			const char* evstr = qg_event_str(ev.ev);
			qn_outputf("STUB EVENT => %d:%s", ev.ev, evstr);

			if (ev.ev == QGEV_WINDOW)
			{
				const char* wevstr = qg_window_event_str(ev.wevent.mesg);
				qn_outputf("\tWINDOW EVENT => %d:%s", ev.wevent.mesg, wevstr);
			}

			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
				qg_exit_loop();
		}
	}

	return 0;
}
