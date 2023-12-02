#include <qs.h>

int main()
{
	qn_runtime(NULL);

	qgRdh* rdh = qg_rdh_new(NULL, "QG TEST", 800, 600, QGFLAG_IDLE | QGFLAG_RESIZABLE);
	if (!rdh)
		return 1;

	while (qg_rdh_loop(rdh))
	{
		qgEvent ev;
		while (qg_rdh_poll(rdh, &ev))
		{
			
		}

		if (qg_rdh_begin(rdh))
		{
			qg_rdh_end(rdh);
			qg_rdh_flush(rdh);
		}
	}

	qm_unload(rdh);

	return 0;
}
