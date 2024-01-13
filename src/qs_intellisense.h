#pragma once
#define __QN_INTELLISENSE__

#ifdef __INTELLISENSE__

#ifndef bool
#define bool	_Bool
#endif

#ifndef true
#define true	1
#endif

#ifndef false
#define false	0
#endif

#define QN_EXTC
#define QN_EXTC_BEGIN
#define QN_EXTC_END

typedef unsigned int	size_t;
typedef int				wchar_t;
typedef unsigned int	wint_t;
typedef void*			va_list;

#endif
