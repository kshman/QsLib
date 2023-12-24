// 미러 컴파일 하는 헤더
#include "pch.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4061)	// 열거형 '열거형' 스위치의 열거자 'identifier'는 사례 레이블에 의해 명시적으로 처리되지 않습니다.
#pragma warning(disable:4062)	// 열거형 '열거형' 스위치의 열거자 'identifier'가 처리되지 않음
#pragma warning(disable:4127)	// 조건식이 상수입니다.
#pragma warning(disable:4200)	// L4, nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable:4514)	// 'function': 참조되지 않은 인라인 함수가 제거되었습니다.
#pragma warning(disable:4710)	// 'function': 함수가 인라인되지 않음
#pragma warning(disable:4711)	// 인라인 확장을 위해 'function' 함수가 선택되었습니다.
#pragma warning(disable:4820)	// 'bytes'바이트 채움 문자가 construct 'member_name' 뒤에 추가되었습니다. (패딩)
#pragma warning(disable:5045)	// 컴파일러는 /Qspectre 스위치가 지정된 경우 메모리 로드를 위해 스펙터 완화를 삽입합니다.
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic pop
#endif
