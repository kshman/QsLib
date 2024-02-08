//
// QsLib [CONTAINER Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: BSD-2-Clause
//

// ReSharper disable CppClangTidyBugproneMacroParentheses

#pragma once
#define __QS_CTN__

#ifndef __QS_QN__
#error include "qs_qn.h" first
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4710)		// 'function': 함수가 인라인되지 않음
#pragma warning(disable:5045)		// 컴파일러는 /Qspectre 스위치가 지정된 경우 메모리 로드를 위해 스펙터 완화를 삽입합니다.
#endif

//////////////////////////////////////////////////////////////////////////
// 파랑 문자열 인라인

/// @brief 파랑 문자열 인라인
/// @param NAME 타입 이름
/// @param SIZE 최대 길이
#define QN_DECL_BSTR(NAME, SIZE)																	\
	static_assert(SIZE > 0, "invalid size" QN_STRING(SIZE));										\
	typedef struct NAME {																			\
		size_t LENGTH;																				\
		char DATA[SIZE];																			\
	} NAME

/// @brief 파랑 문자열 함수
/// @param NAME 타입 이름
/// @param SIZE 최대 길이
/// @param PFX 함수 접두사
#define QN_IMPL_BSTR(NAME, SIZE, PFX)																\
	static_assert(SIZE > 0, "invalid size" QN_STRING(SIZE));										\
	FINLINE void PFX##_init(NAME* bstr, const char* str)											\
	{																								\
		if (str == NULL) {																			\
			bstr->LENGTH = 0;																		\
			bstr->DATA[0] = 0;																		\
		} else {																					\
			const char* ptr = qn_stpcpy(bstr->DATA, str);											\
			bstr->LENGTH = ptr - bstr->DATA;														\
			qn_debug_assert(bstr->LENGTH < SIZE, "string too long");								\
		}																							\
	}																								\
	FINLINE void PFX##_test_init(NAME* bstr)														\
	{																								\
		qn_debug_assert(bstr->LENGTH == 0 && bstr->DATA[0] == '\0', "call _init not _test_init");	\
	}																								\
	FINLINE void PFX##_init_char(NAME* bstr, char ch)												\
	{																								\
		bstr->LENGTH = 1;																			\
		bstr->DATA[0] = ch;																			\
		bstr->DATA[1] = '\0';																		\
	}																								\
	FINLINE void PFX##_init_bstr(NAME* bstr, const NAME* src)										\
	{																								\
		memcpy(bstr->DATA, src->DATA, src->LENGTH + 1);												\
		bstr->LENGTH = src->LENGTH;																	\
	}																								\
	FINLINE void PFX##_clear(NAME* bstr)															\
	{																								\
		bstr->LENGTH = 0;																			\
		bstr->DATA[0] = '\0';																		\
	}																								\
	FINLINE char* PFX##_data(NAME* bstr)															\
	{																								\
		return bstr->DATA;																			\
	}																								\
	FINLINE size_t PFX##_len(const NAME* bstr)														\
	{																								\
		return bstr->LENGTH;																		\
	}																								\
	FINLINE char PFX##_nth(const NAME* bstr, size_t nth)											\
	{																								\
		return bstr->DATA[nth]	;																	\
	}																								\
	FINLINE bool PFX##_is_empty(const NAME* bstr)													\
	{																								\
		return bstr->LENGTH == 0;																	\
	}																								\
	FINLINE bool PFX##_is_have(const NAME* bstr)													\
	{																								\
		return bstr->LENGTH != 0;																	\
	}																								\
	FINLINE void PFX##_intern(NAME* bstr)															\
	{																								\
		bstr->LENGTH = strlen(bstr->DATA);															\
	}																								\
	FINLINE bool PFX##_eq(const NAME* left, const char* right)										\
	{																								\
		return qn_strcmp(left->DATA, right) == 0;													\
	}																								\
	FINLINE bool PFX##_eq_bstr(const NAME* left, const NAME* right)									\
	{																								\
		return left->LENGTH == right->LENGTH && qn_strcmp(left->DATA, right->DATA) == 0;			\
	}																								\
	FINLINE void PFX##_set(NAME* bstr, const char* src)												\
	{																								\
		const char* ptr = qn_stpcpy(bstr->DATA, src);												\
		bstr->LENGTH = ptr - bstr->DATA;															\
	}																								\
	FINLINE void PFX##_set_char(NAME* bstr, char ch)												\
	{																								\
		bstr->LENGTH = 1;																			\
		bstr->DATA[0] = ch;																			\
		bstr->DATA[1] = '\0';																		\
	}																								\
	FINLINE void PFX##_set_len(NAME* bstr, const char* src, size_t len)								\
	{																								\
		memcpy(bstr->DATA, src, len);																\
		bstr->DATA[len] = '\0';																		\
		bstr->LENGTH = len;																			\
	}																								\
	FINLINE void PFX##_set_fill(NAME* bstr, char ch, size_t len)									\
	{																								\
		memset(bstr->DATA, ch, len);																\
		bstr->DATA[len] = '\0';																		\
		bstr->LENGTH = len;																			\
	}																								\
	FINLINE void PFX##_set_bstr(NAME* bstr, const NAME* src)										\
	{																								\
		memcpy(bstr->DATA, src->DATA, src->LENGTH + 1);												\
		bstr->LENGTH = src->LENGTH;																	\
	}																								\
	FINLINE void PFX##_set_bstr_len(NAME* bstr, const NAME* src, size_t len)						\
	{																								\
		memcpy(bstr->DATA, src->DATA, len);															\
		bstr->DATA[len] = '\0';																		\
		bstr->LENGTH = len;																			\
	}																								\
	FINLINE void PFX##_append(NAME* bstr, const char* src)											\
	{																								\
		const char* ptr = qn_stpcpy(bstr->DATA + bstr->LENGTH, src);								\
		bstr->LENGTH = ptr - bstr->DATA;															\
	}																								\
	FINLINE void PFX##_append_char(NAME* bstr, char ch)												\
	{																								\
		bstr->DATA[bstr->LENGTH] = ch;																\
		bstr->DATA[++bstr->LENGTH] = '\0';															\
	}																								\
	FINLINE void PFX##_append_len(NAME* bstr, const char* src, size_t len)							\
	{																								\
		memcpy(bstr->DATA + bstr->LENGTH, src, len);												\
		bstr->DATA[bstr->LENGTH + len] = '\0';														\
		bstr->LENGTH += len;																		\
	}																								\
	FINLINE void PFX##_append_fill(NAME* bstr, char ch, size_t len)									\
	{																								\
		memset(bstr->DATA + bstr->LENGTH, ch, len);													\
		bstr->DATA[bstr->LENGTH + len] = '\0';														\
		bstr->LENGTH += len;																		\
	}																								\
	FINLINE void PFX##_append_bstr(NAME* bstr, const NAME* src)										\
	{																								\
		memcpy(bstr->DATA + bstr->LENGTH, src->DATA, src->LENGTH + 1);								\
		bstr->LENGTH += src->LENGTH;																\
	}																								\
	FINLINE void PFX##_append_bstr_len(NAME* bstr, const NAME* src, size_t len)						\
	{																								\
		memcpy(bstr->DATA + bstr->LENGTH, src->DATA, len);											\
		bstr->DATA[bstr->LENGTH + len] = '\0';														\
		bstr->LENGTH += len;																		\
	}																								\
	FINLINE size_t PFX##_hash(const NAME* bstr)														\
	{																								\
		return qn_strhash(bstr->DATA);																\
	}																								\
	FINLINE size_t PFX##_ihash(const NAME* bstr)													\
	{																								\
		return qn_strihash(bstr->DATA);																\
	}																								\
	FINLINE uint PFX##_shash(const NAME* bstr)														\
	{																								\
		return qn_strshash(bstr->DATA);																\
	}																								\
	FINLINE int PFX##_cmp(const NAME* left, const char* right)										\
	{																								\
		return qn_strcmp(left->DATA, right);														\
	}																								\
	FINLINE int PFX##_cmp_len(const NAME* left, const char* right, size_t len)						\
	{																								\
		return qn_strncmp(left->DATA, right, len);													\
	}																								\
	FINLINE int PFX##_cmp_bstr(const NAME* left, const NAME* right)									\
	{																								\
		return qn_strcmp(left->DATA, right->DATA);													\
	}																								\
	FINLINE int PFX##_cmp_bstr_len(const NAME* left, const NAME* right, size_t len)					\
	{																								\
		return qn_strncmp(left->DATA, right->DATA, len);											\
	}																								\
	FINLINE int PFX##_icmp(const NAME* left, const char* right)										\
	{																								\
		return qn_stricmp(left->DATA, right);														\
	}																								\
	FINLINE int PFX##_icmp_len(const NAME* left, const char* right, size_t len)						\
	{																								\
		return qn_strnicmp(left->DATA, right, len);													\
	}																								\
	FINLINE int PFX##_icmp_bstr(const NAME* left, const NAME* right)								\
	{																								\
		return qn_stricmp(left->DATA, right->DATA);													\
	}																								\
	FINLINE int PFX##_icmp_bstr_len(const NAME* left, const NAME* right, size_t len)				\
	{																								\
		return qn_strnicmp(left->DATA, right->DATA, len);											\
	}																								\
	FINLINE void PFX##_lower(NAME* bstr)															\
	{																								\
		qn_strlwr(bstr->DATA);																		\
	}																								\
	FINLINE void PFX##_upper(NAME* bstr)															\
	{																								\
		qn_strupr(bstr->DATA);																		\
	}																								\
	FINLINE void PFX##_ltrim(NAME* bstr)															\
	{																								\
		qn_strltm(bstr->DATA);																		\
		bstr->LENGTH = strlen(bstr->DATA);															\
	}																								\
	FINLINE void PFX##_rtrim(NAME* bstr)															\
	{																								\
		qn_strrtm(bstr->DATA);																		\
		bstr->LENGTH = strlen(bstr->DATA);															\
	}																								\
	FINLINE void PFX##_trim(NAME* bstr)																\
	{																								\
		qn_strtrm(bstr->DATA);																		\
		bstr->LENGTH = strlen(bstr->DATA);															\
	}																								\
	FINLINE void PFX##_format_va(NAME* bstr, const char* fmt, va_list args)							\
	{																								\
		bstr->LENGTH = qn_vsnprintf(bstr->DATA, SIZE, fmt, args);									\
	}																								\
	FINLINE void PFX##_format(NAME* bstr, const char* fmt, ...)										\
	{																								\
		va_list args;																				\
		va_start(args, fmt);																		\
		PFX##_format_va(bstr, fmt, args);															\
		va_end(args);																				\
	}																								\
	FINLINE void PFX##_append_format_va(NAME* bstr, const char* fmt, va_list args)					\
	{																								\
		int len = qn_vsnprintf(bstr->DATA + bstr->LENGTH, SIZE - bstr->LENGTH, fmt, args);			\
		bstr->LENGTH += len;																		\
	}																								\
	FINLINE void PFX##_append_format(NAME* bstr, const char* fmt, ...)								\
	{																								\
		va_list args;																				\
		va_start(args, fmt);																		\
		PFX##_append_format_va(bstr, fmt, args);													\
		va_end(args);																				\
	}																								\
	FINLINE int PFX##_has_chars(const NAME* bstr, const char* chs)									\
	{																								\
		const char* p = qn_strbrk(bstr->DATA, chs);													\
		return p != NULL ? (int)(p - bstr->DATA) : -1;												\
	}																								\
	FINLINE int PFX##_find_char(const NAME* bstr, size_t nth, char ch)								\
	{																								\
		const char* p = qn_strchr(bstr->DATA + nth, ch);											\
		return p != NULL ? (int)(p - bstr->DATA) : -1;												\
	}																								\
	typedef NAME NAME##Type

/// @brief 파랑 문자열 인라인 선언 및 구현
#define QN_DECLIMPL_BSTR(NAME, SIZE, PFX)															\
	QN_DECL_BSTR(NAME, SIZE);																		\
	QN_IMPL_BSTR(NAME, SIZE, PFX)


//////////////////////////////////////////////////////////////////////////
// 컨테이너 인라인

#ifdef _MSC_VER
/// @brief 컨테이너 foreach
#define QN_CTNR_FOREACH(CTNR, INDEX)																\
	size_t QN_CONCAT(COUNT,__LINE__) = (CTNR).COUNT;												\
	for ((INDEX) = 0; (INDEX) < QN_CONCAT(COUNT,__LINE__); (INDEX)++)
#else
#define QN_CTNR_FOREACH(CTNR, INDEX)																\
	for ((INDEX) = 0; (INDEX) < (CTNR).COUNT; (INDEX)++)
#endif

/// @brief 컨테이너 공통 인라인
#define QN_IMPL_CTNR_COMMON(NAME, TYPE, PFX)														\
	/* @brief 컨테이너 제거 */																		\
	FINLINE void PFX##_dispose(NAME* self)															\
	{																								\
		qn_free(self->DATA);																		\
	}																								\
	/* @brief 컨테이너 지우기(갯수를 0으로) */															\
	FINLINE void PFX##_clear(NAME* self)															\
	{																								\
		self->COUNT = 0;																			\
	}																								\
	/* @brief 컨테이너 갯수 */																		\
	FINLINE size_t PFX##_count(const NAME* self)													\
	{																								\
		return self->COUNT;																			\
	}																								\
	/* @brief 컨테이너 데이터 포인터 */																\
	FINLINE TYPE* PFX##_data(NAME* self)															\
	{																								\
		return self->DATA;																			\
	}																								\
	/* @brief 컨테이너 항목 얻기 */																	\
	FINLINE TYPE PFX##_nth(const NAME* self, size_t index)											\
	{																								\
		return self->DATA[index];																	\
	}																								\
	/* @brief 컨테이너 항목 뒤에서 부터 얻기 */															\
	FINLINE TYPE PFX##_inv(const NAME* self, size_t index)											\
	{																								\
		return self->DATA[self->COUNT - index - 1];													\
	}																								\
	/* @brief 컨테이너 항목 얻기 */																	\
	FINLINE TYPE* PFX##_nth_ptr(const NAME* self, size_t index)										\
	{																								\
		return &self->DATA[index];																	\
	}																								\
	/* @brief 컨테이너 항목 뒤에서 부터 얻기 */															\
	FINLINE TYPE* PFX##_inv_ptr(const NAME* self, size_t index)										\
	{																								\
		return &self->DATA[self->COUNT - index - 1];												\
	}																								\
	/* @brief 컨테이너 항목 설정 */																	\
	FINLINE void PFX##_set(NAME* self, size_t index, TYPE value)									\
	{																								\
		qn_debug_assert(index < self->COUNT, "invalid index!");										\
		self->DATA[index] = value;																	\
	}																								\
	/* @brief 컨테이너가 비었나? */																	\
	FINLINE bool PFX##_is_empty(const NAME* self)													\
	{																								\
		return self->COUNT == 0;																	\
	}																								\
	/* @brief 컨테이너 값이 있나? */																	\
	FINLINE bool PFX##_is_have(const NAME* self)													\
	{																								\
		return self->COUNT != 0;																	\
	}																								\
	/* @brief 컨테이너 정렬하기 */																		\
	FINLINE void PFX##_sort(NAME* self, sortfunc_t func)											\
	{																								\
		qn_qsort(self->DATA, self->COUNT, sizeof(TYPE), func);										\
	}																								\
	/* @brief 컨테이너 정렬하기(컨텍스트) */															\
	FINLINE void PFX##_sortc(NAME* self, sortcfunc_t func, void* context)							\
	{																								\
		qn_qsortc(self->DATA, self->COUNT, sizeof(TYPE), func, context);							\
	}																								\
	/* @brief 컨테이너 항목 지우기 */																	\
	FINLINE void PFX##_remove_nth(NAME* self, size_t nth)											\
	{																								\
		qn_debug_assert(nth < self->COUNT, "index overflow");										\
		if (nth != self->COUNT - 1)	{																\
			TYPE* ptr = self->DATA + nth;															\
			memmove(ptr, ptr + 1, sizeof(TYPE) * (self->COUNT - nth - 1));							\
		}																							\
		self->COUNT--;																				\
	}																								\
	/* @brief 컨테이너 범위 지정해서 지우기 */															\
	FINLINE void PFX##_remove_range(NAME* self, size_t start, size_t count)							\
	{																								\
		qn_debug_assert(start + count <= self->COUNT, "index overflow");							\
		size_t end = start + count;																	\
		if (start < self->COUNT && end < self->COUNT) {												\
			if (end != self->COUNT) {																\
				TYPE* ptr = self->DATA + start;														\
				memmove(ptr, ptr + count, sizeof(TYPE) * (self->COUNT - end));						\
			}																						\
			self->COUNT -= count;																	\
		}																							\
	}																								\
	/* @brief 컨테이너 아이템 지우기 */																\
	FINLINE void PFX##_remove(NAME* self, TYPE value)												\
	{																								\
		TYPE* ptr = self->DATA;																		\
		TYPE* end = ptr + self->COUNT;																\
		while (ptr < end) {																			\
			if (memcmp(ptr, &value, sizeof(TYPE)) != 0)												\
				ptr++;																				\
			else {																					\
				memmove(ptr, ptr + 1, sizeof(TYPE) * (end - ptr - 1));								\
				self->COUNT--;																		\
			}																						\
		}																							\
	}																								\
	/* @brief 컨테이너 아이템 지우기(콜백 사용) */														\
	FINLINE void PFX##_remove_callback(NAME* self, eqcfunc_t func, void* context)					\
	{																								\
		TYPE* ptr = self->DATA;																		\
		TYPE* end = ptr + self->COUNT;																\
		while (ptr < end) {																			\
			if (func(context, ptr) == false)														\
				ptr++;																				\
			else {																					\
				memmove(ptr, ptr + 1, sizeof(TYPE) * (end - ptr - 1));								\
				self->COUNT--;																		\
			}																						\
		}																							\
	}																								\
	/* @brief 컨테이너에 아이템이 몇번째 있나 조사 */													\
	FINLINE size_t PFX##_contains(const NAME* self, TYPE value)										\
	{																								\
		TYPE* ptr = self->DATA;																		\
		TYPE* end = ptr + self->COUNT;																\
		while (ptr < end) {																			\
			if (memcmp(ptr, &value, sizeof(TYPE)) == 0)												\
				return ptr - self->DATA;															\
			ptr++;																					\
		}																							\
		return (size_t)-1;																			\
	}																								\
	/* @brief 컨테이너에서 찾기 (콜백 사용) */															\
	FINLINE size_t PFX##_find(const NAME* self, eqcfunc_t func, void* context)						\
	{																								\
		TYPE* ptr = self->DATA;																		\
		TYPE* end = ptr + self->COUNT;																\
		while (ptr < end) {																			\
			if (func(context, ptr))																	\
				return ptr - self->DATA;															\
			ptr++;																					\
		}																							\
		return (size_t)-1;																			\
	}																								\
	/* @brief 컨테이너 포인터 foreach (콜백 사용) */													\
	FINLINE void PFX##_foreach_1(const NAME* self, paramfunc_t func)								\
	{																								\
		TYPE* ptr = self->DATA;																		\
		TYPE* end = ptr + self->COUNT;																\
		while (ptr < end) {																			\
			func(ptr);																				\
			ptr++;																					\
		}																							\
	}																								\
	/* @brief 컨테이너 포인터 foreach (콜백 사용) */													\
	FINLINE void PFX##_foreach_2(const NAME* self, paramfunc2_t func, void* context)				\
	{																								\
		TYPE* ptr = self->DATA;																		\
		TYPE* end = ptr + self->COUNT;																\
		while (ptr < end) {																			\
			func(context, ptr);																		\
			ptr++;																					\
		}																							\
	}


/// @brief 컨테이너 인라인
/// @param NAME 컨테이너 이름
/// @param TYPE 데이터 타입
#define QN_DECL_CTNR(NAME, TYPE)																	\
	typedef struct NAME {																			\
		size_t COUNT;																				\
		TYPE* DATA;																					\
	} NAME

/// @brief 컨테이너 함수
/// @param NAME 컨테이너 이름
/// @param TYPE 데이터 타입
/// @param PFX 함수 접두사
#define QN_IMPL_CTNR(NAME, TYPE, PFX)																\
	QN_IMPL_CTNR_COMMON(NAME, TYPE, PFX)															\
	/* @brief 컨테이너 초기화 */																		\
	FINLINE void PFX##_init(NAME* self, size_t count)												\
	{																								\
		if (count > 0) {																			\
			self->COUNT = count;																	\
			self->DATA = qn_alloc(count, TYPE);														\
		} else {																					\
			self->COUNT = 0;																		\
			self->DATA = NULL;																		\
		}																							\
	}																								\
	/* @brief 컨테이너 초기화 (데이터도 0으로 초기화) */													\
	FINLINE void PFX##_init_zero(NAME* self, size_t count)											\
	{																								\
		if (count > 0) {																			\
			self->COUNT = count;																	\
			self->DATA = qn_alloc_zero(count, TYPE);												\
		} else {																					\
			self->COUNT = 0;																		\
			self->DATA = NULL;																		\
		}																							\
	}																								\
	/* @brief 컨테이너 복사 */																		\
	FINLINE void PFX##_init_copy(NAME* self, const NAME* src)										\
	{																								\
		self->COUNT = src->COUNT;																	\
		self->DATA = qn_memdup(src->DATA, sizeof(TYPE) * src->COUNT);								\
	}																								\
	/* @brief 지정한 갯수와 데이터로 컨테이너 초기화 */													\
	FINLINE void PFX##_init_data(NAME* self, size_t count, TYPE* data)								\
	{																								\
		self->COUNT = count;																		\
		self->DATA = data;																			\
	}																								\
	/* @brief 컨테이너 크기 조정 */																	\
	FINLINE void PFX##_resize(NAME* self, size_t count)												\
	{																								\
		if (self->COUNT != count) {																	\
			self->DATA = qn_realloc(self->DATA, count, TYPE);										\
			self->COUNT = count;																	\
		}																							\
	}																								\
	/* @brief 컨테이너 크기 늘리기 */																	\
	FINLINE void PFX##_expand(NAME* self, size_t count)												\
	{																								\
		if (count > 0) {																			\
			self->DATA = qn_realloc(self->DATA, self->COUNT + count, TYPE);							\
			self->COUNT += count;																	\
		}																							\
	}																								\
	/* @brief 컨테이너에 항목 추가 */																	\
	FINLINE void PFX##_add(NAME* self, TYPE value)													\
	{																								\
		PFX##_expand(self, 1);																		\
		self->DATA[self->COUNT - 1] = value;														\
	}																								\
	/* @brief 컨테이너에 항목 삽입 */																	\
	FINLINE void PFX##_insert(NAME* self, size_t nth, TYPE value)									\
	{																								\
		qn_debug_assert(nth <= self->COUNT, "index overflow");										\
		PFX##_expand(self, 1);																		\
		if (nth != self->COUNT - 1) {																\
			TYPE* ptr = self->DATA + nth;															\
			memmove(ptr + 1, ptr, sizeof(TYPE) * (self->COUNT - nth));								\
		}																							\
		self->DATA[nth] = value;																	\
	}																								\
	typedef NAME NAME##Type


/// @brief 배열 인라인
/// @param NAME 배열 이름
/// @param TYPE 데이터 타입
#define QN_DECL_ARRAY(NAME, TYPE)																	\
	typedef struct NAME {																			\
		size_t COUNT, CAPA;																			\
		TYPE* DATA;																					\
	} NAME

/// @brief 배열 함수
/// @param NAME 배열 이름
/// @param TYPE 데이터 타입
/// @param PFX 함수 접두사
#define QN_IMPL_ARRAY(NAME, TYPE, PFX)																\
	QN_IMPL_CTNR_COMMON(NAME, TYPE, PFX)															\
	/* @brief 배열 초기화 (용량 지정 가능) */															\
	FINLINE void PFX##_init(NAME* self, size_t capacity)											\
	{																								\
		if (capacity > 0) {																			\
			self->COUNT = 0;																		\
			self->CAPA = capacity;																	\
			self->DATA = qn_alloc(capacity, TYPE);													\
		} else {																					\
			self->COUNT = 0;																		\
			self->CAPA = 0;																			\
			self->DATA = NULL;																		\
		}																							\
	}																								\
	/* @brief 갯수와 데이터를 가지고 배열 초기화 */														\
	FINLINE void PFX##_init_data(NAME* self, size_t count, TYPE* data)								\
	{																								\
		self->COUNT = count;																		\
		self->CAPA = count;																			\
		self->DATA = data;																			\
	}																								\
	/* @brief 내부적으로 배열 확장 */																	\
	FINLINE void PFX##_internal_expand(NAME* self, size_t capa)										\
	{																								\
		capa += self->COUNT;																		\
		if (self->CAPA < capa) {																	\
			while (self->CAPA < capa)																\
				self->CAPA += self->CAPA / 2 + 1;													\
			self->DATA = qn_realloc(self->DATA, self->CAPA, TYPE);									\
		}																							\
	}																								\
	/* @brief 배열 크기 조정 (용량 조정이 아닌 크기 조정) */												\
	FINLINE void PFX##_resize(NAME* self, size_t count)												\
	{																								\
		if (self->COUNT != count) {																	\
			PFX##_internal_expand(self, count);														\
			self->COUNT = count;																	\
		}																							\
	}																								\
	/* @brief 배열에 항목 추가 */																		\
	FINLINE void PFX##_add(NAME* self, TYPE value)													\
	{																								\
		PFX##_internal_expand(self, 1);																\
		self->DATA[self->COUNT++] = value;															\
	}																								\
	/* @brief 배열에 항목 삽입 */																		\
	FINLINE void PFX##_insert(NAME* self, size_t nth, TYPE value)									\
	{																								\
		qn_debug_assert(nth <= self->COUNT, "index overflow");										\
		PFX##_internal_expand(self, 1);																\
		if (nth != self->COUNT - 1) {																\
			TYPE* ptr = self->DATA + nth;															\
			memmove(ptr + 1, ptr, sizeof(TYPE) * (self->COUNT - nth));								\
		}																							\
		self->DATA[nth] = value;																	\
		self->COUNT++;																				\
	}																								\
	typedef NAME NAME##Type

/// @brief 컨테이너 인라인 선언 및 구현
#define QN_DECLIMPL_CTNR(NAME, TYPE, PFX)															\
	QN_DECL_CTNR(NAME, TYPE);																		\
	QN_IMPL_CTNR(NAME, TYPE, PFX)

/// @brief 배열 인라인 선언 및 구현
#define QN_DECLIMPL_ARRAY(NAME, TYPE, PFX)															\
	QN_DECL_ARRAY(NAME, TYPE);																		\
	QN_IMPL_ARRAY(NAME, TYPE, PFX)

/// @brief 포인터 컨테이너
QN_DECLIMPL_CTNR(QnPtrCtnr, pointer_t, qn_pctnr);
/// @brief 포인터 배열
QN_DECLIMPL_ARRAY(QnPtrArray, pointer_t, qn_parray);


//////////////////////////////////////////////////////////////////////////
// 리스트 인라인

/// @brief 리스트용 foreach
#define QN_LIST_FOREACH(LIST,NODE,NEXTNODE)															\
	for ((NODE) = (LIST).HEAD; (NODE) && ((NEXTNODE)=(NODE)->NEXT, true); (NODE)=(NEXTNODE))

/// @brief 리스트 인라인
/// @param NAME 리스트 이름
/// @param TYPE 데이터 타입
#define QN_DECL_LIST(NAME, TYPE)																	\
	typedef struct NAME##Node {																		\
		struct NAME##Node *NEXT, *PREV;																\
		TYPE DATA;																					\
	} NAME##Node;																					\
	typedef struct NAME {																			\
		NAME##Node *HEAD, *TAIL;																	\
		size_t COUNT;																				\
	} NAME

/// @brief 리스트 함수
/// @param NAME 리스트 이름
/// @param TYPE 데이터 타입
/// @param PFX 함수 접두사
#define QN_IMPL_LIST(NAME, TYPE, PFX)																\
	FINLINE size_t PFX##_count(const NAME *list)													\
	{																								\
		return list->COUNT;																			\
	}																								\
	FINLINE bool PFX##_is_have(const NAME *list)													\
	{																								\
		return list->COUNT != 0;																	\
	}																								\
	FINLINE bool PFX##_is_empty(const NAME *list)													\
	{																								\
		return list->COUNT == 0;																	\
	}																								\
	FINLINE NAME##Node* PFX##_head(const NAME *list)												\
	{																								\
		return list->HEAD;																			\
	}																								\
	FINLINE NAME##Node* PFX##_tail(const NAME *list)												\
	{																								\
		return list->TAIL;																			\
	}																								\
	FINLINE TYPE PFX##_head_data(const NAME *list)													\
	{																								\
		return list->HEAD->DATA;																	\
	}																								\
	FINLINE TYPE PFX##_tail_data(const NAME *list)													\
	{																								\
		return list->TAIL->DATA;																	\
	}																								\
	FINLINE void PFX##_init(NAME *list)																\
	{																								\
		list->HEAD = list->TAIL = NULL;																\
		list->COUNT = 0;																			\
	}																								\
	FINLINE void PFX##_dispose(NAME *list)															\
	{																								\
		for (NAME##Node *node = list->HEAD, *next; node; node = next) {								\
			next = node->NEXT;																		\
			qn_free(node);																			\
		}																							\
	}																								\
	FINLINE void PFX##_dispose_callback(NAME *list, paramfunc2_t func, void* context)				\
	{																								\
		for (NAME##Node *node = list->HEAD, *next; node; node = next) {								\
			next = node->NEXT;																		\
			func(context, &node->DATA);																\
			qn_free(node);																			\
		}																							\
	}																								\
	FINLINE void PFX##_clear(NAME *list)															\
	{																								\
		PFX##_dispose(list);																		\
		PFX##_init(list);																			\
	}																								\
	FINLINE void PFX##_remove_node(NAME *list, NAME##Node *node)									\
	{																								\
		if (node == NULL)																			\
			return;																					\
		if (node->PREV)																				\
			node->PREV->NEXT = node->NEXT;															\
		else																						\
			list->HEAD = node->NEXT;																\
		if (node->NEXT)																				\
			node->NEXT->PREV = node->PREV;															\
		else																						\
			list->TAIL = node->PREV;																\
		qn_free(node);																				\
		list->COUNT--;																				\
	}																								\
	FINLINE void PFX##_remove_head(NAME *list)														\
	{																								\
		PFX##_remove_node(list, list->HEAD);														\
	}																								\
	FINLINE void PFX##_remove_tail(NAME *list)														\
	{																								\
		PFX##_remove_node(list, list->TAIL);														\
	}																								\
	FINLINE void PFX##_append(NAME *list, TYPE data)												\
	{																								\
		NAME##Node *node = qn_alloc_1(NAME##Node);													\
		node->DATA = data;																			\
		node->NEXT = NULL;																			\
		node->PREV = list->TAIL;																	\
		if (list->TAIL)																				\
			list->TAIL->NEXT = node;																\
		else																						\
			list->HEAD = node;																		\
		list->TAIL = node;																			\
		list->COUNT++;																				\
	}																								\
	FINLINE void PFX##_prepend(NAME *list, TYPE data)												\
	{																								\
		NAME##Node *node = qn_alloc_1(NAME##Node);													\
		node->DATA = data;																			\
		node->NEXT = list->HEAD;																	\
		node->PREV = NULL;																			\
		if (list->HEAD)																				\
			list->HEAD->PREV = node;																\
		else																						\
			list->TAIL = node;																		\
		list->HEAD = node;																			\
		list->COUNT++;																				\
	}																								\
	FINLINE void PFX##_remove(NAME *list, TYPE data, bool once)										\
	{																								\
		for (NAME##Node *node = list->HEAD, *next; node; node = next) {								\
			next = node->NEXT;																		\
			if (memcmp(&node->DATA, &data, sizeof(TYPE))) {											\
				PFX##_remove_node(list, node);														\
				if (once) break;																	\
			}																						\
		}																							\
	}																								\
	FINLINE void PFX##_remove_callback(NAME *list, eqcfunc_t func, void* context, bool once)		\
	{																								\
		for (NAME##Node *node = list->HEAD, *next; node; node = next) {								\
			next = node->NEXT;																		\
			if (func(context, &node->DATA)) {														\
				PFX##_remove_node(list, node);														\
				if (once) break;																	\
			}																						\
		}																							\
	}																								\
	FINLINE NAME##Node* PFX##_contains(const NAME *list, TYPE data)									\
	{																								\
		for (NAME##Node *node = list->HEAD; node; node = node->NEXT)								\
			if (memcmp(&node->DATA, &data, sizeof(TYPE)))											\
				return node;																		\
		return NULL;																				\
	}																								\
	FINLINE NAME##Node* PFX##_find(const NAME *list, eqcfunc_t func, void* context)					\
	{																								\
		for (NAME##Node *node = list->HEAD; node; node = node->NEXT)								\
			if (func(context, &node->DATA))															\
				return node;																		\
		return NULL;																				\
	}																								\
	FINLINE void PFX##_foreach_1(const NAME *list, paramfunc_t func)								\
	{																								\
		for (NAME##Node *node = list->HEAD; node; node = node->NEXT)								\
			func(&node->DATA);																		\
	}																								\
	FINLINE void PFX##_foreach_2(const NAME *list, paramfunc2_t func, void* context)				\
	{																								\
		for (NAME##Node *node = list->HEAD; node; node = node->NEXT)								\
			func(context, &node->DATA);																\
	}																								\
	typedef NAME NAME##Type


/// @brief 노드 리스트 인라인
/// @param NAME 리스트 이름
/// @param NODETYPE 노드 타입
#define QN_DECL_LNODE(NAME, NODETYPE)																\
	/* struct NODETYPE { NODETYPE *PREV, *NEXT; } */												\
	typedef struct NAME {																			\
		NODETYPE *HEAD, *TAIL;																		\
		size_t COUNT;																				\
	} NAME

/// @brief 노드 리스트 함수
/// @param NAME 리스트 이름
/// @param NODETYPE 노드 타입
/// @param PFX 함수 접두사
#define QN_IMPL_LNODE(NAME, NODETYPE, PFX)															\
	FINLINE size_t PFX##_count(const NAME *list)													\
	{																								\
		return list->COUNT;																			\
	}																								\
	FINLINE bool PFX##_is_have(const NAME *list)													\
	{																								\
		return list->COUNT != 0;																	\
	}																								\
	FINLINE bool PFX##_is_empty(const NAME *list)													\
	{																								\
		return list->COUNT == 0;																	\
	}																								\
	FINLINE NODETYPE* PFX##_head(const NAME *list)													\
	{																								\
		return list->HEAD;																			\
	}																								\
	FINLINE NODETYPE* PFX##_tail(const NAME *list)													\
	{																								\
		return list->TAIL;																			\
	}																								\
	FINLINE void PFX##_init(NAME *list)																\
	{																								\
		list->HEAD = list->TAIL = NULL;																\
		list->COUNT = 0;																			\
	}																								\
	FINLINE void PFX##_dispose(NAME *list)															\
	{																								\
		for (NODETYPE *node = list->HEAD, *next; node; node = next) {								\
			next = node->NEXT;																		\
			qn_free(node);																			\
		}																							\
	}																								\
	FINLINE void PFX##_dispose_callback(NAME *list, paramfunc_t func)								\
	{																								\
		for (NODETYPE *node = list->HEAD, *next; node; node = next) {								\
			next = node->NEXT;																		\
			func((void*)node);																		\
		}																							\
	}																								\
	FINLINE void PFX##_reset(NAME *list)															\
	{																								\
		PFX##_init(list);																			\
	}																								\
	FINLINE void PFX##_clear(NAME *list)															\
	{																								\
		PFX##_dispose(list);																		\
		PFX##_init(list);																			\
	}																								\
	FINLINE void PFX##_clear_callback(NAME *list, paramfunc_t func)									\
	{																								\
		PFX##_dispose_callback(list, func);															\
		PFX##_init(list);																			\
	}																								\
	FINLINE void PFX##_remove(NAME *list, NODETYPE *node, bool freenode)							\
	{																								\
		if (node == NULL)																			\
			return;																					\
		if (node->PREV)																				\
			node->PREV->NEXT = node->NEXT;															\
		else																						\
			list->HEAD = node->NEXT;																\
		if (node->NEXT)																				\
			node->NEXT->PREV = node->PREV;															\
		else																						\
			list->TAIL = node->PREV;																\
		if (freenode)																				\
			qn_free(node);																			\
		list->COUNT--;																				\
	}																								\
	FINLINE void PFX##_remove_callback(NAME *list, NODETYPE *node, paramfunc_t func)				\
	{																								\
		if (node == NULL)																			\
			return;																					\
		if (node->PREV)																				\
			node->PREV->NEXT = node->NEXT;															\
		else																						\
			list->HEAD = node->NEXT;																\
		if (node->NEXT)																				\
			node->NEXT->PREV = node->PREV;															\
		else																						\
			list->TAIL = node->PREV;																\
		func((void*)node);																			\
		list->COUNT--;																				\
	}																								\
	FINLINE void PFX##_remove_head(NAME *list, bool freenode)										\
	{																								\
		PFX##_remove(list, list->HEAD, freenode);													\
	}																								\
	FINLINE void PFX##_remove_head_callback(NAME *list, paramfunc_t func)							\
	{																								\
		PFX##_remove_callback(list, list->HEAD, func);												\
	}																								\
	FINLINE void PFX##_remove_tail(NAME *list, bool freenode)										\
	{																								\
		PFX##_remove(list, list->TAIL, freenode);													\
	}																								\
	FINLINE void PFX##_remove_tail_callback(NAME *list, paramfunc_t func)							\
	{																								\
		PFX##_remove_callback(list, list->TAIL, func);												\
	}																								\
	FINLINE void PFX##_unlink(NAME *list, NODETYPE *node)											\
	{																								\
		if (node == NULL)																			\
			return;																					\
		if (node->PREV)																				\
			node->PREV->NEXT = node->NEXT;															\
		else																						\
			list->HEAD = node->NEXT;																\
		if (node->NEXT)																				\
			node->NEXT->PREV = node->PREV;															\
		else																						\
			list->TAIL = node->PREV;																\
		node->PREV = node->NEXT = NULL;																\
		list->COUNT--;																				\
	}																								\
	FINLINE void PFX##_append(NAME *list, NODETYPE* node)											\
	{																								\
		node->NEXT = NULL;																			\
		node->PREV = list->TAIL;																	\
		if (list->TAIL)																				\
			list->TAIL->NEXT = node;																\
		else																						\
			list->HEAD = node;																		\
		list->TAIL = node;																			\
		list->COUNT++;																				\
	}																								\
	FINLINE void PFX##_prepend(NAME *list, NODETYPE* node)											\
	{																								\
		node->NEXT = list->HEAD;																	\
		node->PREV = NULL;																			\
		if (list->HEAD)																				\
			list->HEAD->PREV = node;																\
		else																						\
			list->TAIL = node;																		\
		list->HEAD = node;																			\
		list->COUNT++;																				\
	}																								\
	FINLINE NODETYPE* PFX##_contains(const NAME *list, NODETYPE* dest)								\
	{																								\
		for (NODETYPE *node = list->HEAD; node; node = node->NEXT)									\
			if (node == dest)																		\
				return node;																		\
		return NULL;																				\
	}																								\
	FINLINE NODETYPE* PFX##_find(const NAME *list, eqcfunc_t func, void* context)					\
	{																								\
		for (NODETYPE *node = list->HEAD; node; node = node->NEXT)									\
			if (func(context, node))																\
				return node;																		\
		return NULL;																				\
	}																								\
	FINLINE void PFX##_foreach_1(const NAME *list, paramfunc_t func)								\
	{																								\
		for (NODETYPE *next, *node = list->HEAD; node; node = next)									\
		{																							\
			next = node->NEXT;																		\
			func(node);																				\
		}																							\
	}																								\
	FINLINE void PFX##_foreach_2(const NAME *list, paramfunc2_t func, void* context)				\
	{																								\
		for (NODETYPE *next, *node = list->HEAD; node; node = next)									\
		{																							\
			next = node->NEXT;																		\
			func(context, node);																	\
		}																							\
	}																								\
	typedef NAME NAME##Type

#define QN_DECLIMPL_LIST(NAME, TYPE, PFX)															\
	QN_DECL_LIST(NAME, TYPE);																		\
	QN_IMPL_LIST(NAME, TYPE, PFX)

#define QN_DECLIMPL_LNODE(NAME, NODETYPE, PFX)														\
	QN_DECL_LNODE(NAME, NODETYPE);																	\
	QN_IMPL_LNODE(NAME, NODETYPE, PFX)


//////////////////////////////////////////////////////////////////////////
// 해시

/// @brief 해시용 foreach
#define QN_HASH_FOREACH(hash, node)																	\
	for ((node) = (hash).HEAD; (node); (node) = (node)->NEXT)

/// @brief 묶음용 foreach
#define QN_MUKUM_FOREACH(mukum, node)																\
	for (size_t mukum_iter = 0; mukum_iter < (mukum).BUCKET; ++mukum_iter)							\
		for ((node) = (mukum).NODES[mukum_iter]; (node); (node) = (node)->SIB)

/// @brief 해시 함수 만들기 (정수형)
#define QN_DECL_HASH_FUNC(TYPE, PFX)																\
	FINLINE bool PFX##_eqv(TYPE left, TYPE right) { return left == right; }							\
	FINLINE size_t PFX##_hash(TYPE key) { return (size_t)key; }
/// @brief 해시용 정수 비교
QN_DECL_HASH_FUNC(int, qn_ctn_int);
/// @brief 해시용 부호없는 정수 비교
QN_DECL_HASH_FUNC(uint, qn_ctn_uint);
/// @brief 해시용 플랫폼 정수 비교
QN_DECL_HASH_FUNC(nint, qn_ctn_nint);
/// @brief 해시용 플랫폼 부호없는 정수 비교
QN_DECL_HASH_FUNC(nuint, qn_ctn_nuint);
/// @brief 해시용 size_t 비교
QN_DECL_HASH_FUNC(size_t, qn_ctn_size);

// 해시 공용
#define QN_IMPL_HASH_COMMON(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)		\
	typedef bool(*PFX##_find_t)(void*, KEYTYPE, void*);												\
	typedef void(*PFX##_each2_t)(KEYTYPE, void*);													\
	typedef void(*PFX##_each3_t)(void*, KEYTYPE, void*);											\
	FINLINE void PFX##_internal_erase_all(NAME* hash);												\
	FINLINE void PFX##_internal_erase_node(NAME* hash, NAME##Node** en);							\
	FINLINE void PFX##_internal_set(NAME* hash, KEYTYPE key, VALUETYPE* valueptr, bool replace);	\
	FINLINE size_t PFX##_count(const NAME *hash)													\
	{																								\
		return hash->COUNT;																			\
	}																								\
	FINLINE size_t PFX##_revision(const NAME *hash)													\
	{																								\
		return hash->REVISION;																		\
	}																								\
	FINLINE size_t PFX##_bucket(const NAME *hash)													\
	{																								\
		return hash->BUCKET;																		\
	}																								\
	FINLINE bool PFX##_is_have(const NAME *hash)													\
	{																								\
		return hash->COUNT != 0;																	\
	}																								\
	FINLINE bool PFX##_is_empty(const NAME *hash)													\
	{																								\
		return hash->COUNT == 0;																	\
	}																								\
	FINLINE void PFX##_internal_test_size(NAME *hash)												\
	{																								\
		if ((hash->BUCKET >= 3 * hash->COUNT && hash->BUCKET > QN_MIN_HASH) ||						\
			(3 * hash->BUCKET <= hash->COUNT && hash->BUCKET < QN_MAX_HASH)) {						\
			size_t new_bucket = qn_prime_near((uint)hash->COUNT);									\
			new_bucket = QN_CLAMP(new_bucket, QN_MIN_HASH, QN_MAX_HASH);							\
			NAME##Node** new_nodes = qn_alloc_zero(new_bucket, NAME##Node*);						\
			for (size_t i = 0; i < hash->BUCKET; ++i) {												\
				for (NAME##Node *next, *node = hash->NODES[i]; node; node = next) {					\
					next = node->SIB;																\
					const size_t index = node->HASH % new_bucket;									\
					node->SIB = new_nodes[index];													\
					new_nodes[index] = node;														\
				}																					\
			}																						\
			qn_free(hash->NODES);																	\
			hash->NODES = new_nodes;																\
			hash->BUCKET = new_bucket;																\
		}																							\
	}																								\
	FINLINE NAME##Node** PFX##_internal_lookup(const NAME* hash, const KEYTYPE key)					\
	{																								\
		const size_t lh = KEYHASH(key);																\
		NAME##Node *lnn, **ln = &hash->NODES[lh % hash->BUCKET];									\
		while ((lnn = *ln) != NULL) {																\
			if (lnn->HASH == lh && KEYEQ(lnn->KEY, key))											\
				break;																				\
			ln = &lnn->SIB;																			\
		}																							\
		return ln;																					\
	}																								\
	FINLINE NAME##Node** PFX##_internal_lookhash(const NAME* hash, const KEYTYPE key, size_t* ret)	\
	{																								\
		const size_t lh = KEYHASH(key);																\
		NAME##Node *lnn, **ln = &hash->NODES[lh % hash->BUCKET];									\
		while ((lnn = *ln) != NULL) {																\
			if (lnn->HASH == lh && KEYEQ(lnn->KEY, key))											\
				break;																				\
			ln = &lnn->SIB;																			\
		}																							\
		*ret = lh;																					\
		return ln;																					\
	}																								\
	FINLINE bool PFX##_internal_erase(NAME* hash, const KEYTYPE key)								\
	{																								\
		NAME##Node** rn = PFX##_internal_lookup(hash, key);											\
		if (*rn == NULL)																			\
			return false;																			\
		PFX##_internal_erase_node(hash, rn);														\
		return true;																				\
	}																								\
	FINLINE void PFX##_clear(NAME* hash)															\
	{																								\
		PFX##_internal_erase_all(hash);																\
		PFX##_internal_test_size(hash);																\
	}																								\
	FINLINE void PFX##_remove_node(NAME* hash, NAME##Node* node)									\
	{																								\
		PFX##_internal_erase_node(hash, &node);														\
		PFX##_internal_test_size(hash);																\
	}																								\
	FINLINE VALUETYPE* PFX##_get(const NAME* hash, const KEYTYPE key)								\
	{																								\
		NAME##Node** gn = PFX##_internal_lookup(hash, key);											\
		return *gn ? &(*gn)->VALUE : NULL;															\
	}																								\
	FINLINE void PFX##_add(NAME* hash, KEYTYPE key, VALUETYPE value)								\
	{																								\
		PFX##_internal_set(hash, key, &value, false);												\
	}																								\
	FINLINE void PFX##_set(NAME* hash, KEYTYPE key, VALUETYPE value)								\
	{																								\
		PFX##_internal_set(hash, key, &value, true);												\
	}																								\
	FINLINE void PFX##_add_ptr(NAME* hash, KEYTYPE key, VALUETYPE* value)							\
	{																								\
		PFX##_internal_set(hash, key, value, false);												\
	}																								\
	FINLINE void PFX##_set_ptr(NAME* hash, KEYTYPE key, VALUETYPE* value)							\
	{																								\
		PFX##_internal_set(hash, key, value, true);													\
	}																								\
	FINLINE bool PFX##_remove(NAME* hash, const KEYTYPE key)										\
	{																								\
		if (PFX##_internal_erase(hash, key) == false)												\
			return false;																			\
		PFX##_internal_test_size(hash);																\
		return true;																				\
	}


/// @brief 해시 인라인
///	@param NAME 해시 이름
///	@param KEYTYPE 키 타입
///	@param VALUETYPE 값 타입
#define QN_DECL_HASH(NAME, KEYTYPE, VALUETYPE)														\
	typedef struct NAME##Node {																		\
		struct NAME##Node *SIB, *NEXT, *PREV;														\
		size_t HASH;																				\
		KEYTYPE KEY;																				\
		VALUETYPE VALUE;																			\
	} NAME##Node;																					\
	typedef struct NAME {																			\
		size_t COUNT, REVISION, BUCKET;																\
		NAME##Node **NODES, *HEAD, *TAIL;															\
	} NAME

/// @brief 해시 함수
///	@param NAME 해시 이름
///	@param KEYTYPE 키 타입
///	@param VALUETYPE 값 타입
///	@param KEYHASH 키 해시 함수
///	@param KEYEQ 키 비교 함수
///	@param KEYFREE 키 해제 함수
///	@param VALUEFREE 값 해제 함수
///	@param PFX 함수 접두사
#define QN_IMPL_HASH(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)				\
	QN_IMPL_HASH_COMMON(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)			\
	FINLINE void PFX##_init(NAME *hash)																\
	{																								\
		hash->COUNT = hash->REVISION = 0;															\
		hash->BUCKET = QN_MIN_HASH;																	\
		hash->NODES = qn_alloc_zero(QN_MIN_HASH, NAME##Node*);										\
		hash->HEAD = hash->TAIL = NULL;																\
	}																								\
	FINLINE void PFX##_init_fast(NAME *hash)														\
	{																								\
		qn_debug_assert(hash->REVISION == 0 && hash->COUNT == 0 && hash->NODES == NULL, "cannot use fast init, use just init");	\
		hash->BUCKET = QN_MIN_HASH;																	\
		hash->NODES = qn_alloc_zero(QN_MIN_HASH, NAME##Node*);										\
	}																								\
	FINLINE void PFX##_dispose(NAME *hash)															\
	{																								\
		for (NAME##Node *next, *node = hash->HEAD; node; node = next) {								\
			next = node->NEXT;																		\
			KEYFREE(node->KEY);																		\
			VALUEFREE(&node->VALUE);																\
			qn_free(node);																			\
		}																							\
		qn_free(hash->NODES);																		\
	}																								\
	FINLINE NAME##Node* PFX##_node_head(const NAME *hash)											\
	{																								\
		return hash->HEAD;																			\
	}																								\
	FINLINE NAME##Node* PFX##_node_tail(const NAME *hash)											\
	{																								\
		return hash->TAIL;																			\
	}																								\
	FINLINE void PFX##_internal_set(NAME* hash, KEYTYPE key, VALUETYPE* valueptr, bool replace)		\
	{																								\
		size_t ah;																					\
		NAME##Node** an = PFX##_internal_lookhash(hash, key, &ah);									\
		NAME##Node* ann = *an;																		\
		if (ann != NULL) {																			\
			if (replace) {																			\
				KEYFREE(ann->KEY);																	\
				VALUEFREE(&ann->VALUE);																\
				ann->KEY = key;																		\
				ann->VALUE = *valueptr;																\
			} else {																				\
				KEYFREE(key);																		\
				VALUEFREE(valueptr);																\
			}																						\
		} else {																					\
			ann = qn_alloc_1(NAME##Node);															\
			ann->SIB = NULL;																		\
			ann->HASH = ah;																			\
			ann->KEY = key;																			\
			ann->VALUE = *valueptr;																	\
			ann->NEXT = NULL;																		\
			ann->PREV = hash->TAIL;																	\
			if (hash->TAIL)																			\
				hash->TAIL->NEXT = ann;																\
			else																					\
				hash->HEAD = ann;																	\
			hash->TAIL = ann;																		\
			*an = ann;																				\
			hash->REVISION++;																		\
			hash->COUNT++;																			\
			PFX##_internal_test_size(hash);															\
		}																							\
	}																								\
	FINLINE void PFX##_internal_erase_node(NAME* hash, NAME##Node** en)								\
	{																								\
		NAME##Node* enn = *en;																		\
		*en = enn->SIB;																				\
		if (enn->PREV)																				\
			enn->PREV->NEXT = enn->NEXT;															\
		else																						\
			hash->HEAD = enn->NEXT;																	\
		if (enn->NEXT)																				\
			enn->NEXT->PREV = enn->PREV;															\
		else																						\
			hash->TAIL = enn->PREV;																	\
		const size_t ebk = enn->HASH % hash->BUCKET;												\
		if (hash->NODES[ebk] == enn)																\
			hash->NODES[ebk] = NULL;																\
		KEYFREE(enn->KEY);																			\
		VALUEFREE(&enn->VALUE);																		\
		qn_free(enn);																				\
		hash->REVISION++;																			\
		hash->COUNT--;																				\
	}																								\
	FINLINE void PFX##_internal_erase_all(NAME* hash)												\
	{																								\
		for (NAME##Node *next, *node = hash->HEAD; node; node = next) {								\
			next = node->NEXT;																		\
			KEYFREE(node->KEY);																		\
			VALUEFREE(&node->VALUE);																\
			qn_free(node);																			\
		}																							\
		hash->HEAD = hash->TAIL = NULL;																\
		hash->REVISION++;																			\
		hash->COUNT = 0;																			\
		memset(hash->NODES, 0, hash->BUCKET * sizeof(NAME##Node*));									\
	}																								\
	FINLINE KEYTYPE PFX##_find(const NAME* hash, PFX##_find_t func, void* context)					\
	{																								\
		for (NAME##Node *node = hash->HEAD; node; node = node->NEXT) {								\
			if (func(context, node->KEY, &node->VALUE))												\
				return node->KEY;																	\
		}																							\
		return (KEYTYPE)0;																			\
	}																								\
	FINLINE void PFX##_foreach_2(const NAME* hash, PFX##_each2_t func)								\
	{																								\
		for (NAME##Node *node = hash->HEAD; node; node = node->NEXT)								\
			func(node->KEY, &node->VALUE);															\
	}																								\
	FINLINE void PFX##_foreach_3(const NAME* hash, PFX##_each3_t func, void* context)				\
	{																								\
		for (NAME##Node *node = hash->HEAD; node; node = node->NEXT)								\
			func(context, node->KEY, &node->VALUE);													\
	}																								\
	typedef NAME NAME##Type

/// @brief 해시 해시 선언 및 구현
#define QN_DECLIMPL_HASH(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)			\
	QN_DECL_HASH(NAME, KEYTYPE, VALUETYPE);															\
	QN_IMPL_HASH(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)

// 키 정수
#define QN_DECLIMPL_INT_HASH(NAME, VALUETYPE, VALUEFREE, PFX)										\
	QN_DECLIMPL_HASH(NAME, int, VALUETYPE, (int), qn_ctn_int_eqv, (void), VALUEFREE, PFX)
// 키 정수 / 값 정수
#define QN_DECLIMPL_INT_INT_HASH(NAME, PFX)															\
	QN_DECLIMPL_INT_HASH(NAME, int, (void), PFX)
// 키 정수 / 값 문자열
#define QN_DECLIMPL_INT_PCHAR_HASH(NAME, PFX)														\
	QN_DECLIMPL_INT_HASH(NAME, char*, qn_mem_free_ptr, PFX)

// 키 부호없는 정수
#define QN_DECLIMPL_UINT_HASH(NAME, VALUETYPE, VALUEFREE, PFX)										\
	QN_DECLIMPL_HASH(NAME, uint, VALUETYPE, (uint), qn_ctn_uint_eqv, (void), VALUEFREE, PFX)
// 키 부호없는 정수 / 값 부호없는 정수
#define QN_DECLIMPL_UINT_UINT_HASH(NAME, PFX)														\
	QN_DECLIMPL_UINT_HASH(NAME, uint, (void), PFX)
// 키 부호없는 정수 / 값 문자열
#define QN_DECLIMPL_UINT_PCHAR_HASH(NAME, PFX)														\
	QN_DECLIMPL_UINT_HASH(NAME, char*, qn_mem_free_ptr, PFX)

// 키 문자열
#define QN_DECLIMPL_PCHAR_HASH(NAME, VALUETYPE, VALUEFREE, PFX)										\
	QN_DECLIMPL_HASH(NAME, char*, VALUETYPE, qn_strhash, qn_streqv, qn_mem_free, VALUEFREE, PFX)
// 키 문자열 / 값 정수
#define QN_DECLIMPL_PCHAR_INT_HASH(NAME, PFX)														\
	QN_DECLIMPL_PCHAR_HASH(NAME, int, (void), PFX)
// 키 문자열 / 값 문자열
#define QN_DECLIMPL_PCHAR_PCHAR_HASH(NAME, PFX)														\
	QN_DECLIMPL_PCHAR_HASH(NAME, char*, qn_mem_free_ptr, PFX)


/// @brief 묶음 인라인
///	@param NAME 묶음 이름
/// @param KEYTYPE 키 타입
/// @param VALUETYPE 값 타입
#define QN_DECL_MUKUM(NAME, KEYTYPE, VALUETYPE)														\
	typedef struct NAME##Node {																		\
		struct NAME##Node *SIB;																		\
		size_t HASH;																				\
		KEYTYPE KEY;																				\
		VALUETYPE VALUE;																			\
	} NAME##Node;																					\
	typedef struct NAME {																			\
		size_t COUNT, REVISION, BUCKET;																\
		NAME##Node **NODES;																			\
	} NAME;																							\
/// @brief 묶음 함수
///	@param NAME 묶음 이름
/// @param KEYTYPE 키 타입
/// @param VALUETYPE 값 타입
/// @param KEYHASH 키 해시 함수
/// @param KEYEQ 키 비교 함수
/// @param KEYFREE 키 해제 함수
/// @param VALUEFREE 값 해제 함수
/// @param PFX 함수 접두사
#define QN_IMPL_MUKUM(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)			\
	QN_IMPL_HASH_COMMON(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)			\
	FINLINE void PFX##_init(NAME *mukum)															\
	{																								\
		mukum->COUNT = mukum->REVISION = 0;															\
		mukum->BUCKET = QN_MIN_HASH;																\
		mukum->NODES = qn_alloc_zero(QN_MIN_HASH, NAME##Node*);										\
	}																								\
	FINLINE void PFX##_init_fast(NAME *mukum)														\
	{																								\
		qn_debug_assert(mukum->REVISION == 0 && mukum->COUNT == 0 && mukum->NODES == NULL, "cannot use fast init, use just init");	\
		mukum->BUCKET = QN_MIN_HASH;																\
		mukum->NODES = qn_alloc_zero(QN_MIN_HASH, NAME##Node*);										\
	}																								\
	FINLINE void PFX##_dispose(NAME *mukum)															\
	{																								\
		for (size_t i = 0; i < mukum->BUCKET; ++i) {												\
			for (NAME##Node *next = NULL, *node = mukum->NODES[i]; node; node = next) {				\
				next = node->SIB;																	\
				KEYFREE(node->KEY);																	\
				VALUEFREE(&node->VALUE);															\
				qn_free(node);																		\
			}																						\
		}																							\
		qn_free(mukum->NODES);																		\
	}																								\
	FINLINE void PFX##_internal_set(NAME* mukum, KEYTYPE key, VALUETYPE* valueptr, bool replace)	\
	{																								\
		size_t ah;																					\
		NAME##Node** an = PFX##_internal_lookhash(mukum, key, &ah);									\
		NAME##Node* ann = *an;																		\
		if (ann != NULL) {																			\
			if (replace) {																			\
				KEYFREE(ann->KEY);																	\
				VALUEFREE(&ann->VALUE);																\
				ann->KEY = key;																		\
				ann->VALUE = *valueptr;																\
			} else {																				\
				KEYFREE(key);																		\
				VALUEFREE(valueptr);																\
			}																						\
		} else {																					\
			ann = qn_alloc_1(NAME##Node);															\
			ann->SIB = NULL;																		\
			ann->HASH = ah;																			\
			ann->KEY = key;																			\
			ann->VALUE = *valueptr;																	\
			*an = ann;																				\
			mukum->REVISION++;																		\
			mukum->COUNT++;																			\
			PFX##_internal_test_size(mukum);														\
		}																							\
	}																								\
	FINLINE void PFX##_internal_erase_node(NAME* mukum, NAME##Node** en)							\
	{																								\
		NAME##Node* enn = *en;																		\
		*en = enn->SIB;																				\
		const size_t ebk = enn->HASH % mukum->BUCKET;												\
		if (mukum->NODES[ebk] == enn)																\
			mukum->NODES[ebk] = NULL;																\
		KEYFREE(enn->KEY);																			\
		VALUEFREE(&enn->VALUE);																		\
		qn_free(enn);																				\
		mukum->REVISION++;																			\
		mukum->COUNT--;																				\
	}																								\
	FINLINE void PFX##_internal_erase_all(NAME* mukum)												\
	{																								\
		for (size_t i = 0; i < mukum->BUCKET; ++i) {												\
			for (NAME##Node *next = NULL, *node = mukum->NODES[i]; node; node = next) {				\
				next = node->SIB;																	\
				KEYFREE(node->KEY);																	\
				VALUEFREE(&node->VALUE);															\
				qn_free(node);																		\
			}																						\
		}																							\
		mukum->REVISION++;																			\
		mukum->COUNT = 0;																			\
		memset(mukum->NODES, 0, mukum->BUCKET * sizeof(NAME##Node*));								\
	}																								\
	FINLINE KEYTYPE PFX##_find(const NAME* mukum, PFX##_find_t func, void* context)					\
	{																								\
		for (size_t i = 0; i < mukum->BUCKET; ++i)													\
			for (NAME##Node *next = NULL, *node = mukum->NODES[i]; node; node = next) {				\
				next = node->SIB;																	\
				if (func(context, node->KEY, &node->VALUE))											\
					return node->KEY;																\
			}																						\
		return (KEYTYPE)0;																			\
	}																								\
	FINLINE void PFX##_foreach_2(const NAME* mukum, PFX##_each2_t func)								\
	{																								\
		for (size_t i = 0; i < mukum->BUCKET; ++i)													\
			for (NAME##Node *next = NULL, *node = mukum->NODES[i]; node; node = next) {				\
				next = node->SIB;																	\
				func(node->KEY, &node->VALUE);														\
			}																						\
	}																								\
	FINLINE void PFX##_foreach_3(const NAME* mukum, PFX##_each3_t func, void* context)				\
	{																								\
		for (size_t i = 0; i < mukum->BUCKET; ++i)													\
			for (NAME##Node *next = NULL, *node = mukum->NODES[i]; node; node = next) {				\
				next = node->SIB;																	\
				func(context, node->KEY, &node->VALUE);												\
			}																						\
	}																								\
	typedef NAME NAME##Type

/// @brief 묶음 선언 및 구현
#define QN_DECLIMPL_MUKUM(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)		\
	QN_DECL_MUKUM(NAME, KEYTYPE, VALUETYPE);														\
	QN_IMPL_MUKUM(NAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PFX)

// 키 정수
#define QN_DECLIMPL_INT_MUKUM(NAME, VALUETYPE, VALUEFREE, PFX)										\
	QN_DECLIMPL_MUKUM(NAME, int, VALUETYPE, (int), qn_ctn_int_eqv, (void), VALUEFREE, PFX)
// 키 정수 / 값 정수
#define QN_DECLIMPL_INT_INT_MUKUM(NAME, PFX)														\
	QN_DECLIMPL_INT_MUKUM(NAME, int, (void), PFX)
// 키 정수 / 값 문자열
#define QN_DECLIMPL_INT_PCHAR_MUKUM(NAME, PFX)														\
	QN_DECLIMPL_INT_MUKUM(NAME, char*, qn_mem_free_ptr, PFX)

// 키 부호없는 정수
#define QN_DECLIMPL_UINT_MUKUM(NAME, VALUETYPE, VALUEFREE, PFX)										\
	QN_DECLIMPL_MUKUM(NAME, uint, VALUETYPE, (uint), qn_ctn_uint_eqv, (void), VALUEFREE, PFX)
// 키 부호없는 정수 / 값 부호없는 정수
#define QN_DECLIMPL_UINT_UINT_MUKUM(NAME, PFX)														\
	QN_DECLIMPL_UINT_MUKUM(NAME, uint, (void), PFX)
// 키 부호없는 정수 / 값 문자열
#define QN_DECLIMPL_UINT_PCHAR_MUKUM(NAME, PFX)														\
	QN_DECLIMPL_UINT_MUKUM(NAME, char*, qn_mem_free_ptr, PFX)

// 키 문자열
#define QN_DECLIMPL_PCHAR_MUKUM(NAME, VALUETYPE, VALUEFREE, PFX)									\
	QN_DECLIMPL_MUKUM(NAME, char*, VALUETYPE, qn_strhash, qn_streqv, qn_mem_free, VALUEFREE, PFX)
// 키 문자열 / 값 정수
#define QN_DECLIMPL_PCHAR_INT_MUKUM(NAME, PFX)														\
	QN_DECLIMPL_PCHAR_MUKUM(NAME, int, (void), PFX)
// 키 문자열 / 값 문자열
#define QN_DECLIMPL_PCHAR_PCHAR_MUKUM(NAME, PFX)													\
	QN_DECLIMPL_PCHAR_MUKUM(NAME, char*, qn_mem_free_ptr, PFX)

#ifdef _MSC_VER
#pragma warning(pop)
#endif
