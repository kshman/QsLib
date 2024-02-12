// stub 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	int flags = /*QGFLAG_BORDERLESS |*/ QGFLAG_RESIZE;
	int features = /*QGFEATURE_DISABLE_ACS |*/ QGFEATURE_DISABLE_SCRSAVE | QGFEATURE_ENABLE_IDLE | QGFEATURE_ENABLE_DROP;
	if (!qg_open_stub(NULL, 0, 0, 0, flags, features))
		return -1;

	QgEvent ev;
	while (qg_loop())
	{
		while (qg_poll(&ev))
		{
			const char* evstr = qg_event_to_str(ev.ev);

			if (ev.ev == QGEV_MOUSEMOVE)
			{
				qn_outputf("\t마우스 이동 => %d:%d (%d:%d)", ev.mmove.pt.X, ev.mmove.pt.Y, ev.mmove.delta.X, ev.mmove.delta.Y);
			}
			else if (ev.ev == QGEV_WINDOW)
			{
				const char* wevstr = qg_window_event_to_str(ev.wevent.mesg);
				qn_outputf("\tWINDOW EVENT => %d:%s", ev.wevent.mesg, wevstr);
			}
			else if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
			{
				qg_exit_loop();
			}
			else if (ev.ev == QGEV_TEXTINPUT && ev.text.len > 0)
			{
				char sz[32];
				qn_strncpy(sz, ev.text.data, (size_t)ev.text.len);
				qn_outputf("\t텍스트 입력: %s", sz);
			}
			else
			{
				qn_outputf("STUB EVENT => %d:%s", ev.ev, evstr);
				if (ev.ev == QGEV_EXIT)
					break;
			}
		}
	}

	return 0;
}
