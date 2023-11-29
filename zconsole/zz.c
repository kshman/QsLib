#include <qs.h>

int main()
{
	qn_runtime(NULL);

	qgStub* stub = qg_stub_new(NULL, "QG TEST", 800, 600, QGSTUB_IDLE | QGSTUB_RESIZABLE);
	if (!stub)
		return 1;

	while (qg_stub_poll(stub))
	{
	}

	qm_unload(stub);

	return 0;
}
