#pragma once

#include "qs_qn.h"

#define HAVE_VSNPRINTF
#define vsnprintf		qn_vsnprintf
#define snprintf		qn_snprintf

#define malloc(x)		((void*)qn_alloc(x,byte))
#define calloc(x,y)		((void*)qn_alloc_zero((x)*(y),byte))
#define free(x)			qn_free(x)

#if _MSC_VER
#pragma warning(disable:4061)	// 열거형 '열거형' 스위치의 열거자 'identifier'는 사례 레이블에 의해 명시적으로 처리되지 않습니다.
#pragma warning(disable:4242)	// 'identifier': 'type1'에서 'type2'로 변환, 데이터 손실 가능성
#pragma warning(disable:4244)	// 'conversion': 'type1'에서 'type2'(으)로 변환하면서 데이터가 손실될 수 있습니다.
#pragma warning(disable:4820)	// 'bytes'바이트 채움 문자가 construct 'member_name' 뒤에 추가되었습니다.
#pragma warning(disable:4996)	// 코드는 사용되지 않는 것으로 표시된 함수, 클래스 멤버, 변수 또는 typedef를 사용합니다.
#endif
