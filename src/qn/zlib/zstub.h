#pragma once

#include "qs_qn.h"

#define HAVE_VSNPRINTF
#define vsnprintf		qn_vsnprintf
#define snprintf		qn_snprintf

#define malloc(x)		((void*)qn_alloc(x,uint8_t))
#define calloc(x,y)		((void*)qn_alloc_zero((x)*(y),uint8_t))
#define free(x)			qn_free(x)

#if _MSC_VER
#pragma warning(disable:4131)	// 'function': 이전 스타일의 선언자를 사용합니다.
#pragma warning(disable:4244)	// 'conversion': 'type1'에서 'type2'(으)로 변환하면서 데이터가 손실될 수 있습니다.
#pragma warning(disable:4996)	// 코드는 사용되지 않는 것으로 표시된 함수, 클래스 멤버, 변수 또는 typedef를 사용합니다.
#endif
