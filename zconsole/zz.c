// rdh 테스트
#include <qs.h>

static void main_loop(void* data);
static int event_callback(void* data, QgEventType type, const QgEvent* param);

int main(void)
{
	qn_runtime();

	int flags = QGFLAG_RESIZE | QGFLAG_VSYNC /*| QGFLAG_MSAA*/ /*| QGFLAG_TEXT*/;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh(NULL, "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	qg_register_event_callback(event_callback, NULL);
	qg_main_loop(main_loop, NULL);

	qg_close_rdh();

	return 0;
}

static int event_callback(void* data, QgEventType type, const QgEvent* param)
{
	QN_DUMMY(data);
	if (type == QGEV_KEYDOWN)
	{
		if (param->key.key == QIK_ESC)
			qg_exit_loop();
	}

	return 0;
}

static void main_loop(void* data)
{
	QN_DUMMY(data);
}
