#include <qs.h>

int main()
{
	qn_runtime(NULL);

	qgStub* stub = qg_stub_new(NULL, "QG TEST", 800, 600, QGFLAG_IDLE | QGFLAG_RESIZABLE);
	if (!stub)
		return 1;

	while (qg_stub_update(stub))
	{
		qgEvent ev;
		while (qg_stub_poll(stub, &ev))
		{
			qn_debug_output(false, "msg[%d]\n", ev.ev);
		}
	}

	qm_unload(stub);

	return 0;
}
