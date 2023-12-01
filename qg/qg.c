#include "pch.h"
#include "qg.h"
#include "qg_stub.h"

size_t qg_number(void)
{
	static size_t s = 0;
	return ++s;
}
