#pragma once

#include "qs_qn.h"
#include <io.h>
#ifdef _QN_UNIX_
#include <unistd.h>
#endif

#define HAVE_VSNPRINTF
#define vsnprintf		qn_vsnprintf
#define snprintf		qn_snprintf

#define malloc(x)		((void*)qn_alloc(x,byte))
#define calloc(x,y)		((void*)qn_alloc_zero((x)*(y),byte))
#define free(x)			qn_free(x)

#define wcstombs(x,y,z)	qn_wcstombs(x, z, y, 0)

#ifdef _MSC_VER
#define _open			zlib_proxy_open
#define _wopen			zlib_proxy_wopen
#define strerror		zlib_proxy_strerr

QN_INLINE int zlib_proxy_open(const char* filename, int openflag, int permission)
{
	int fd;
	_sopen_s(&fd, filename, openflag, _SH_DENYNO, permission);
	return fd;
}
QN_INLINE int zlib_proxy_wopen(const wchar_t* filename, int openflag, int permission)
{
	int fd;
	_wsopen_s(&fd, filename, openflag, _SH_DENYNO, permission);
	return fd;
}
QN_INLINE char* zlib_proxy_strerr(int err)
{
	static char zlib_proxy_buffer[256];
	strerror_s(zlib_proxy_buffer, QN_COUNTOF(zlib_proxy_buffer)-1, err);
	return zlib_proxy_buffer;
}
#endif

#ifdef _MSC_VER
#pragma warning(disable:4127)	// 조건식이 상수입니다.
#pragma warning(disable:4242)	// 'identifier': 'type1'에서 'type2'로 변환, 데이터 손실 가능성
#pragma warning(disable:4244)	// 'conversion': 'type1'에서 'type2'(으)로 변환하면서 데이터가 손실될 수 있습니다.
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#endif
