#pragma once

#include <qn.h>
#include <qnctn.h>
#include <qnmath.h>
#include <qg.h>

#if _MSC_VER
#pragma comment(lib, "qn")
#pragma comment(lib, "qg")
#if _QN_64_
#pragma comment(lib, "SDL2x64")
#else
#pragma comment(lib, "SDL2")
#endif
#endif
