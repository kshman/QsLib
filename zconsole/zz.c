// rdh 테스트
#include <qs.h>

struct LoopData
{
	float acc;
	float dir;
};
float acc = 0.0f;
float dir = 1.0f;

bool loop(void* loop_data)
{
	 struct LoopData* data = ( struct LoopData*)loop_data;

	QgEvent ev;
	while (qg_poll(&ev))
	{
		if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
		{
			qg_exit_loop();
			return false;
		}
	}

	float f = (float)qg_get_advance();
	data->acc += f * data->dir;
	if (data->acc > 1.0f)
	{
		data->acc = 1.0f;
		data->dir = -1.0f;
	}
	if (data->acc < 0.0f)
	{
		data->acc = 0.0f;
		data->dir = 1.0f;
	}

	QmColor cc = qm_color(data->acc, data->acc, data->acc, 1.0f);
	qg_rdh_set_background(&cc);

	if (qg_rdh_begin(true))
	{
		qg_rdh_end();
		qg_rdh_flush();
	}

	return true;
}

int main(void)
{
	qn_runtime();

	int flags = /*QGFLAG_BORDERLESS |*/ QGFLAG_RESIZABLE | QGFLAG_VSYNC | QGFLAG_MSAA;
	if (qg_open_rdh(NULL, "RDH", 0, 0, 0, flags) == false)
		return -1;

	static struct LoopData data = {.acc=0.0f, .dir=1.0f};
	qg_main_loop(loop, &data);

	qg_close_rdh();

	return 0;
}
