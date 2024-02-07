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
#pragma warning(disable:4200)		// 비표준 확장이 사용됨: 구조체/공용 구조체의 배열 크기가 0입니다.
#pragma warning(disable:4710)		// 'function': 함수가 인라인되지 않음
#pragma warning(disable:5045)		// 컴파일러는 /Qspectre 스위치가 지정된 경우 메모리 로드를 위해 스펙터 완화를 삽입합니다.
#endif

//////////////////////////////////////////////////////////////////////////
// common

// 공통
#define qn_ctn_name_type(name)			name##Type
#define qn_ctn_name_node(name)			name##Node
#define qn_ctn_sizeof_type(name)		sizeof(name##Type)
#define qn_ctn_sizeof_node(name)		sizeof(name##Node)


//////////////////////////////////////////////////////////////////////////
// mukum

/// @brief 묶음 정의
/// @param name 묶음 이름
/// @param keytype 키 타입
/// @param valuetype 값 타입
#define QN_DECL_MUKUM(name,keytype,valuetype)\
	typedef keytype name##Key; typedef const keytype name##ConstKey;\
	typedef valuetype name##Value; typedef const valuetype name##ConstValue;\
	typedef struct name##Node { struct name##Node* SIB; size_t HASH; name##Key KEY; name##Value VALUE; } name##Node;\
	typedef struct name { size_t REVISION; size_t BUCKET; size_t COUNT; name##Node** NODES; } name
QN_DECL_MUKUM(QnInlineMukum, size_t, size_t);

/// @brief 키 해시
#define QN_HASH_HASH(name,fn1)			INLINE size_t name##_Hash(name##ConstKey* key) { return fn1(*key); }
/// @brief 키 비교
#define QN_HASH_EQ(name,fn2)			INLINE bool name##_Equal(name##ConstKey* l, name##ConstKey const* r) { return fn2(*l, *r); }
/// @brief 키 지우기
#define QN_HASH_KEY(name,fn1)			INLINE void name##_Key(name##Key* key) { fn1(*key); }
#define QN_HASH_KEY_FREE(name)			INLINE void name##_Key(name##Key* key) { qn_free(*key); }
#define QN_HASH_KEY_NONE(name)			INLINE void name##_Key(name##Key* key) { QN_DUMMY(key); }
/// @brief 값 지우기
#define QN_HASH_VALUE(name,fn1)			INLINE void name##_Value(name##Value* value) { fn1(*value); }
#define QN_HASH_VALUE_FREE(name)		INLINE void name##_Value(name##Value* value) { qn_free(*value); }
#define QN_HASH_VALUE_NONE(name)		INLINE void name##_Value(name##Value* value) { QN_DUMMY(value); }
/// @brief 정수 키의 해시/비교
#define QN_HASH_INT_KEY(name)			INLINE size_t name##_Hash(name##ConstKey* key) { return (size_t)(*key); }\
										INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return (*k1)==(*k2); }
/// @brief 부호없는 정수 키의 해시/비교
#define QN_HASH_UINT_KEY(name)			INLINE size_t name##_Hash(name##ConstKey* key) { return (size_t)(*key); }\
										INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return *k1==*k2; }
/// @brief size_t 키의 해시/비교
#define QN_HASH_SIZE_T_KEY(name)		INLINE size_t name##_Hash(name##ConstKey* key) { return *key; }\
										INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return (*k1)==(*k2); }
/// @brief char* 키의 해시/비교
#define QN_HASH_CHAR_PTR_KEY(name)		INLINE size_t name##_Hash(name##ConstKey* key) { return qn_strhash(*key); }\
										INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return qn_streqv(*k1, *k2); }

#define QN_MUKUM_HASH					QN_HASH_HASH
#define QN_MUKUM_EQ						QN_HASH_EQ
#define QN_MUKUM_KEY					QN_HASH_KEY
#define QN_MUKUM_KEY_FREE				QN_HASH_KEY_FREE
#define QN_MUKUM_KEY_NONE				QN_HASH_KEY_NONE
#define QN_MUKUM_VALUE					QN_HASH_VALUE
#define QN_MUKUM_VALUE_FREE				QN_HASH_VALUE_FREE
#define QN_MUKUM_VALUE_NONE				QN_HASH_VALUE_NONE
#define QN_MUKUM_INT_KEY				QN_HASH_INT_KEY
#define QN_MUKUM_UINT_KEY				QN_HASH_UINT_KEY
#define QN_MUKUM_SIZE_T_KEY				QN_HASH_SIZE_T_KEY
#define QN_MUKUM_CHAR_PTR_KEY			QN_HASH_CHAR_PTR_KEY

/// @brief 묶음 초기화
#define qn_mukum_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->REVISION=0;\
		(p)->COUNT=0;\
		(p)->BUCKET=QN_MIN_HASH;\
		(p)->NODES=qn_alloc_zero(QN_MIN_HASH, name##Node*);\
	}QN_STMT_END

/// @brief 묶음 제거
#define qn_mukum_dispose(name,p)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->BUCKET; __i++)\
			for (name##Node *__next, *__node=(p)->NODES[__i]; __node; __node=__next) {\
				__next=__node->SIB;\
				name##_Key(&__node->KEY);\
				name##_Value(&__node->VALUE);\
				qn_free(__node);\
			}\
		qn_free((p)->NODES);\
	}QN_STMT_END

/// @brief 묶음 비우기
#define qn_mukum_clear(name,p)\
	QN_STMT_BEGIN{\
		qn_inl_mukum_erase_all(name,p);\
		qn_inl_mukum_test_size(name,p);\
	}QN_STMT_END

/// @brief 아이템 갯수
#define qn_mukum_count(p)				((p)->COUNT)
/// @brief 묶음 버킷수
#define qn_mukum_bucket(p)				((p)->BUCKET)
/// @brief 묶음 변경치
#define qn_mukum_revision(p)			((p)->REVISION)
/// @brief 항목이 있다
#define qn_mukum_is_have(p)				((p)->COUNT>0)

/// @brief 묶음 loop each
/// @param func2 loop each 함수 포인터. 인수는(keyptr,valueptr)
/// @return
#define qn_mukum_foreach_2(name,p,func2)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->BUCKET; __i++)\
			for (name##Node* __node=(p)->NODES[__i]; __node; __node=__node->SIB)\
				func2(&__node->KEY, &__node->VALUE);\
	}QN_STMT_END

/// @brief 묶음 for each
/// @param func3 for each 함수 포인터. 인수는(data,keyptr,valueptr)
/// @param data for each 함수 포인터 첫 인수
/// @return
#define qn_mukum_foreach_3(name,p,func3,userdata)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->BUCKET; __i++)\
			for (name##Node* __node=(p)->NODES[__i]; __node; __node=__node->SIB)\
				func3(userdata, &__node->KEY, &__node->VALUE);\
	}QN_STMT_END

/// @brief 묶음 얻기
/// @param keyptr 키 포인터
/// @param retval 값의 포인터. NULL 이면 해당 키에 대한 값이 없는 것
/// @return
#define qn_mukum_get_ptr(name,p,keyptr,retval)\
	QN_STMT_BEGIN{\
		name##Node **__gn, *__node;\
		qn_inl_mukum_lookup(name,p,keyptr,__gn);\
		__node=*__gn;\
		*(retval)=(__node) ? &__node->VALUE : NULL;\
	}QN_STMT_END

/// @brief 묶음 추가 (중복 항목 안 덮어씀)
#define qn_mukum_add_ptr(name,p,keyptr,valueptr)\
	qn_inl_mukum_set(name,p,keyptr,valueptr,false);

/// @brief 묶음 설정 (중복 항목 덮어씀)
#define qn_mukum_set_ptr(name,p,keyptr,valueptr)\
	qn_inl_mukum_set(name,p,keyptr,valueptr,true);\

/// @brief 묶음 삭제
/// @param keyptr 키 포인터
/// @param ret_bool_ptr 삭제 결과를 담을 bool 타입 포인터. 필요없으면 NULL
#define qn_mukum_remove_ptr(name,p,keyptr,ret_bool_ptr)\
	QN_STMT_BEGIN{\
		qn_inl_mukum_erase(name,p,keyptr,ret_bool_ptr);\
		qn_inl_mukum_test_size(name,p);\
	}QN_STMT_END

/// @brief 묶음 얻기
/// @param key 키
/// @param retval 값의 포인터. NULL 이면 해당 키에 대한 값이 없는 것
/// @return
#define qn_mukum_get(name,p,key,retval)\
	QN_STMT_BEGIN{\
		name##ConstKey __k = (name##ConstKey)(key); name##ConstKey* __kp = &__k;\
		qn_mukum_get_ptr(name,p,__kp,retval);\
	}QN_STMT_END

/// @brief 묶음 추가 (중복 항목 안 덮어씀)
#define qn_mukum_add(name,p,key,value)\
	QN_STMT_BEGIN{\
		name##Key __key = key; name##Value __value = value;\
		qn_mukum_add_ptr(name,p,&__key,&__value);\
	}QN_STMT_END

/// @brief 묶음 설정 (중복 항목 덮어씀)
#define qn_mukum_set(name,p,key,value)\
	QN_STMT_BEGIN{\
		name##Key __key = key; name##Value __value = value;\
		qn_mukum_set_ptr(name,p,&__key,&__value);\
	}QN_STMT_END

/// @brief 묶음 삭제
/// @param key 키
/// @param ret_bool_ptr 삭제 결과를 담을 bool 타입 포인터. 필요없으면 NULL
#define qn_mukum_remove(name,p,key,ret_bool_ptr)\
	QN_STMT_BEGIN{\
		name##ConstKey* __kp = (name##ConstKey*)&(key);\
		qn_mukum_remove_ptr(name,p,__kp,ret_bool_ptr);\
	}QN_STMT_END

/// @brief 묶음 검색
/// @param func3 bool (data, keyptr, valueptr) 타입의 검색 함수
/// @param data 검색 함수 첫 항목
/// @param ret_key_ptr 반환 키 포인터
#define qn_mukum_find(name,p,func3,data,ret_key_ptr)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->BUCKET; __i++) {\
			for (name##Node* __node=(p)->NODES[__i]; __node; __node=__node->SIB) {\
				if (func3(data, &__node->KEY, &__node->VALUE)) {\
					(ret_key_ptr)=&__node->KEY;\
					goto QN_CONCAT(pos_mukum_find_exit,__LINE__);\
				}\
			}\
		}\
		(ret_key_ptr)=NULL;\
		QN_CONCAT(pos_mukum_find_exit,__LINE__):;\
	}QN_STMT_END

// 묶음 룩업
#define qn_inl_mukum_lookup(name,p,keyptr,ret_node)\
	size_t __lh=name##_Hash(keyptr);\
	name##Node *__lnn, **__ln=&(p)->NODES[__lh%(p)->BUCKET];\
	while ((__lnn=*__ln)!=NULL) {\
		if (__lnn->HASH==__lh && name##_Equal((name##ConstKey*)&__lnn->KEY, keyptr)) break;\
		__ln=&__lnn->SIB;\
	}\
	(ret_node)=__ln

// 묶음 룩업
#define qn_inl_mukum_lookup_hash(name,p,keyptr,ret_node,ret_hash)\
	/*qn_debug_assert((p)->NODES!=NULL, "uninitialized memory");*/\
	size_t __lh=name##_Hash((name##ConstKey*)(keyptr));\
	name##Node *__lnn, **__ln=&(p)->NODES[__lh%(p)->BUCKET];\
	while ((__lnn=*__ln)!=NULL) {\
		if (__lnn->HASH==__lh && name##_Equal((name##ConstKey*)&__lnn->KEY, (name##ConstKey*)(keyptr))) break;\
		__ln=&__lnn->SIB;\
	}\
	(ret_node)=__ln;\
	(ret_hash)=__lh

// 묶음 설정
#define qn_inl_mukum_set(name,p,keyptr,valueptr,replace)\
	QN_STMT_BEGIN{\
		size_t __ah; name##Node** __an;\
		qn_inl_mukum_lookup_hash(name,p,keyptr,__an,__ah);\
		name##Node* __ann=*__an;\
		if (__ann) {\
			if (replace) {\
				name##_Key(&__ann->KEY);		__ann->KEY=*(keyptr);\
				name##_Value(&__ann->VALUE);	__ann->VALUE=*(valueptr);\
			} else {\
				name##_Key(keyptr);\
				name##_Value(valueptr);\
			}\
		} else {\
			/* step 1*/\
			__ann=qn_alloc_1(name##Node);\
			__ann->SIB=NULL;\
			__ann->HASH=__ah;\
			__ann->KEY=*(keyptr);\
			__ann->VALUE=*(valueptr);\
			/* step 2 */\
			*__an=__ann;\
			(p)->REVISION++;\
			(p)->COUNT++;\
			/* step 3 */\
			qn_inl_mukum_test_size(name,p);\
		}\
	}QN_STMT_END

// 묶음 삭제
#define qn_inl_mukum_erase(name,p,keyptr,ret_bool_ptr)\
	QN_STMT_BEGIN{\
		name##Node** __rn;\
		qn_inl_mukum_lookup(name,p,keyptr,__rn);\
		if (*__rn==NULL) {\
			if (ret_bool_ptr) {\
				bool* __c=ret_bool_ptr;\
				*__c=false;\
			}\
		} else {\
			qn_inl_mukum_erase_node(name,p,&__rn);\
			if (ret_bool_ptr) {\
				bool* __c=ret_bool_ptr;\
				*__c=true;\
			}\
		}\
	}QN_STMT_END

// 묶음 노드 삭제
#define qn_inl_mukum_erase_node(name,p,pppnode)\
	QN_STMT_BEGIN{\
		name##Node** __en=*(pppnode);\
		name##Node* __enn=*__en;\
		/* step 1 */\
		*__en=__enn->SIB;\
		/* step 2 */\
		size_t __ebkt=__enn->HASH%(p)->BUCKET;\
		if ((p)->NODES[__ebkt]==__enn) (p)->NODES[__ebkt] = NULL;\
		name##_Key(&__enn->KEY);\
		name##_Value(&__enn->VALUE);\
		qn_free(__enn);\
		(p)->COUNT--;\
		(p)->REVISION++;\
	}QN_STMT_END

// 모두 삭제
#define qn_inl_mukum_erase_all(name,p)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->BUCKET; __i++)\
			for (name##Node *__next, *__node=(p)->NODES[__i]; __node; __node=__next) {\
				__next=__node->SIB;\
				name##_Key(&__node->KEY);\
				name##_Value(&__node->VALUE);\
				qn_free(__node);\
			}\
		(p)->COUNT=0;\
		(p)->REVISION++;\
		memset((p)->NODES, 0, (p)->BUCKET*sizeof(name##Node*));\
	}QN_STMT_END

// 묶음 크기 검사
#define qn_inl_mukum_test_size(name,p)\
	QN_STMT_BEGIN{\
		size_t __cnt=(p)->COUNT;\
		size_t __bkt=(p)->BUCKET;\
		if ((__bkt>=3*__cnt && __bkt>QN_MIN_HASH) ||\
			(3*__bkt<=__cnt && __bkt<QN_MAX_HASH))\
			qn_inl_mukum_resize((QnInlineMukum*)(p));\
	}QN_STMT_END

// 묶음 리사이즈
INLINE void qn_inl_mukum_resize(QnInlineMukum* p)
{
	size_t newbucket = qn_prime_near((uint)p->COUNT);
	newbucket = QN_CLAMP(newbucket, QN_MIN_HASH, QN_MAX_HASH);
	QnInlineMukumNode** newnodes = qn_alloc_zero(newbucket, QnInlineMukumNode*);
	if (!newnodes)
		return;
	for (size_t i = 0; i < p->BUCKET; i++) {
		for (QnInlineMukumNode *node = p->NODES[i], *next; node; node = next) {
			next = node->SIB;
			const size_t hashmask = node->HASH % newbucket;
			node->SIB = newnodes[hashmask];
			newnodes[hashmask] = node;
		}
	}
	qn_free(p->NODES);
	p->NODES = newnodes;
	p->BUCKET = newbucket;
}


//////////////////////////////////////////////////////////////////////////
// 파랑 문자열 인라인

/// @brief 파랑 문자열 인라인
/// @param TYPENAME 타입 이름
/// @param SIZE 최대 길이
#define QN_DECL_BSTR(TYPENAME, SIZE)	\
	typedef struct TYPENAME { size_t LENGTH; char DATA[SIZE]; } TYPENAME

/// @brief 파랑 문자열 함수
/// @param TYPENAME 타입 이름
/// @param SIZE 최대 길이
/// @param PREFIX 함수 접두사
#define QN_IMPL_BSTR(TYPENAME, SIZE, PREFIX)	\
	FINLINE void QN_CONCAT(PREFIX, _init)(struct TYPENAME* bstr, const char* str)	\
	{	\
		if (str == NULL)	\
		{	\
			bstr->LENGTH = 0;	\
			bstr->DATA[0] = 0;	\
		}	\
		else	\
		{	\
			const char* ptr = qn_stpcpy(bstr->DATA, str);	\
			bstr->LENGTH = ptr - bstr->DATA;	\
			qn_debug_assert(bstr->LENGTH < SIZE, "string too long");	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _test_init)(struct TYPENAME* bstr)	\
	{	\
		qn_debug_assert(bstr->LENGTH == 0 && bstr->DATA[0] == '\0', "call _init not _test_init");	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _init_char)(struct TYPENAME* bstr, char ch)	\
	{	\
		bstr->LENGTH = 1;	\
		bstr->DATA[0] = ch;	\
		bstr->DATA[1] = '\0';	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _init_bstr)(struct TYPENAME* bstr, const struct TYPENAME* src)	\
	{	\
		memcpy(bstr->DATA, src->DATA, src->LENGTH + 1);	\
		bstr->LENGTH = src->LENGTH;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _clear)(struct TYPENAME* bstr)	\
	{	\
		bstr->LENGTH = 0;	\
		bstr->DATA[0] = '\0';	\
	}	\
	FINLINE char* QN_CONCAT(PREFIX, _data)(struct TYPENAME* bstr)	\
	{	\
		return bstr->DATA;	\
	}	\
	FINLINE size_t QN_CONCAT(PREFIX, _len)(const struct TYPENAME* bstr)	\
	{	\
		return bstr->LENGTH;	\
	}	\
	FINLINE char QN_CONCAT(PREFIX, _nth)(const struct TYPENAME* bstr, size_t nth)	\
	{	\
		return bstr->DATA[nth];	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _is_empty)(const struct TYPENAME* bstr)	\
	{	\
		return bstr->LENGTH == 0;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _is_have)(const struct TYPENAME* bstr)	\
	{	\
		return bstr->LENGTH != 0;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _intern)(struct TYPENAME* bstr)	\
	{	\
		bstr->LENGTH = strlen(bstr->DATA);	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _eq)(const struct TYPENAME* left, const char* right)	\
	{	\
		return qn_strcmp(left->DATA, right) == 0;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _eq_bstr)(const struct TYPENAME* left, const struct TYPENAME* right)	\
	{	\
		return left->LENGTH == right->LENGTH && qn_strcmp(left->DATA, right->DATA) == 0;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _set)(struct TYPENAME* bstr, const char* src)	\
	{	\
		const char* ptr = qn_stpcpy(bstr->DATA, src);	\
		bstr->LENGTH = ptr - bstr->DATA;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _set_char)(struct TYPENAME* bstr, char ch)	\
	{	\
		bstr->LENGTH = 1;	\
		bstr->DATA[0] = ch;	\
		bstr->DATA[1] = '\0';	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _set_len)(struct TYPENAME* bstr, const char* src, size_t len)	\
	{	\
		memcpy(bstr->DATA, src, len);	\
		bstr->DATA[len] = '\0';	\
		bstr->LENGTH = len;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _set_fill)(struct TYPENAME* bstr, char ch, size_t len)	\
	{	\
		memset(bstr->DATA, ch, len);	\
		bstr->DATA[len] = '\0';	\
		bstr->LENGTH = len;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _set_bstr)(struct TYPENAME* bstr, const struct TYPENAME* src)	\
	{	\
		memcpy(bstr->DATA, src->DATA, src->LENGTH + 1);	\
		bstr->LENGTH = src->LENGTH;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _set_bstr_len)(struct TYPENAME* bstr, const struct TYPENAME* src, size_t len)	\
	{	\
		memcpy(bstr->DATA, src->DATA, len);	\
		bstr->DATA[len] = '\0';	\
		bstr->LENGTH = len;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append)(struct TYPENAME* bstr, const char* src)	\
	{	\
		const char* ptr = qn_stpcpy(bstr->DATA + bstr->LENGTH, src);	\
		bstr->LENGTH = ptr - bstr->DATA;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append_char)(struct TYPENAME* bstr, char ch)	\
	{	\
		bstr->DATA[bstr->LENGTH] = ch;	\
		bstr->DATA[++bstr->LENGTH] = '\0';	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append_len)(struct TYPENAME* bstr, const char* src, size_t len)	\
	{	\
		memcpy(bstr->DATA + bstr->LENGTH, src, len);	\
		bstr->DATA[bstr->LENGTH + len] = '\0';	\
		bstr->LENGTH += len;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append_fill)(struct TYPENAME* bstr, char ch, size_t len)	\
	{	\
		memset(bstr->DATA + bstr->LENGTH, ch, len);	\
		bstr->DATA[bstr->LENGTH + len] = '\0';	\
		bstr->LENGTH += len;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append_bstr)(struct TYPENAME* bstr, const struct TYPENAME* src)	\
	{	\
		memcpy(bstr->DATA + bstr->LENGTH, src->DATA, src->LENGTH + 1);	\
		bstr->LENGTH += src->LENGTH;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append_bstr_len)(struct TYPENAME* bstr, const struct TYPENAME* src, size_t len)	\
	{	\
		memcpy(bstr->DATA + bstr->LENGTH, src->DATA, len);	\
		bstr->DATA[bstr->LENGTH + len] = '\0';	\
		bstr->LENGTH += len;	\
	}	\
	FINLINE size_t QN_CONCAT(PREFIX, _hash)(const struct TYPENAME* bstr, bool igcase)	\
	{	\
		return igcase ? qn_strihash(bstr->DATA) : qn_strhash(bstr->DATA);	\
	}	\
	FINLINE uint QN_CONCAT(PREFIX, _shash)(const struct TYPENAME* bstr)	\
	{	\
		return qn_strshash(bstr->DATA);	\
	}	\
	FINLINE int QN_CONCAT(PREFIX, _compare)(const struct TYPENAME* left, const char* right, bool igcase)	\
	{	\
		return igcase ? qn_stricmp(left->DATA, right) : qn_strcmp(left->DATA, right);	\
	}	\
	FINLINE int QN_CONCAT(PREFIX, _compare_len)(const struct TYPENAME* left, const char* right, size_t len, bool igcase)	\
	{	\
		return igcase ? qn_strnicmp(left->DATA, right, len) : qn_strncmp(left->DATA, right, len);	\
	}	\
	FINLINE int QN_CONCAT(PREFIX, _compare_bstr)(const struct TYPENAME* left, const struct TYPENAME* right, bool igcase)	\
	{	\
		return igcase ? qn_stricmp(left->DATA, right->DATA) : qn_strcmp(left->DATA, right->DATA);	\
	}	\
	FINLINE int QN_CONCAT(PREFIX, _compare_bstr_len)(const struct TYPENAME* left, const struct TYPENAME* right, size_t len, bool igcase)	\
	{	\
		return igcase ? qn_strnicmp(left->DATA, right->DATA, len) : qn_strncmp(left->DATA, right->DATA, len);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _lower)(struct TYPENAME* bstr)	\
	{	\
		qn_strlwr(bstr->DATA);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _upper)(struct TYPENAME* bstr)	\
	{	\
		qn_strupr(bstr->DATA);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _trim_left)(struct TYPENAME* bstr)	\
	{	\
		qn_strltm(bstr->DATA);	\
		bstr->LENGTH = strlen(bstr->DATA);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _trim_right)(struct TYPENAME* bstr)	\
	{	\
		qn_strrtm(bstr->DATA);	\
		bstr->LENGTH = strlen(bstr->DATA);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _trim)(struct TYPENAME* bstr)	\
	{	\
		qn_strtrm(bstr->DATA);	\
		bstr->LENGTH = strlen(bstr->DATA);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _format_va)(struct TYPENAME* bstr, const char* fmt, va_list args)	\
	{	\
		bstr->LENGTH = qn_vsnprintf(bstr->DATA, SIZE, fmt, args);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _format)(struct TYPENAME* bstr, const char* fmt, ...)	\
	{	\
		va_list args;	\
		va_start(args, fmt);	\
		QN_CONCAT(PREFIX, _format_va)(bstr, fmt, args);	\
		va_end(args);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append_format_va)(struct TYPENAME* bstr, const char* fmt, va_list args)	\
	{	\
		int len = qn_vsnprintf(bstr->DATA + bstr->LENGTH, SIZE - bstr->LENGTH, fmt, args);	\
		bstr->LENGTH += len;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append_format)(struct TYPENAME* bstr, const char* fmt, ...)	\
	{	\
		va_list args;	\
		va_start(args, fmt);	\
		QN_CONCAT(PREFIX, _append_format_va)(bstr, fmt, args);	\
		va_end(args);	\
	}	\
	FINLINE int QN_CONCAT(PREFIX, _has_chars)(const struct TYPENAME* bstr, const char* chs)	\
	{	\
		const char* p = qn_strbrk(bstr->DATA, chs);	\
		return p != NULL ? (int)(p - bstr->DATA) : -1;	\
	}	\
	FINLINE int QN_CONCAT(PREFIX, _find_char)(const struct TYPENAME* bstr, size_t nth, char ch)	\
	{	\
		const char* p = qn_strchr(bstr->DATA + nth, ch);	\
		return p != NULL ? (int)(p - bstr->DATA) : -1;	\
	}	\
	typedef struct TYPENAME QN_CONCAT(TYPENAME, Type)


#define QN_DECLIMPL_BSTR(TYPENAME, SIZE, PREFIX)	\
	QN_DECL_BSTR(TYPENAME, SIZE);	\
	QN_IMPL_BSTR(TYPENAME, SIZE, PREFIX)


//////////////////////////////////////////////////////////////////////////
// 컨테이너 인라인

#ifdef _MSC_VER
#define QN_CTNR_FOREACH(CTNR, INDEX)\
	size_t QN_CONCAT(COUNT,__LINE__) = (CTNR).COUNT;\
	for ((INDEX) = 0; (INDEX) < QN_CONCAT(COUNT,__LINE__); (INDEX)++)
#else
#define QN_CTNR_FOREACH(CTNR, INDEX)\
	for ((INDEX) = 0; (INDEX) < (CTNR).COUNT; (INDEX)++)
#endif

// 컨테이너 종류 인라인
#define QN_IMPL_CTNR_COMMON(TYPENAME, DATATYPE, PREFIX)	\
	/* @brief 컨테이너 제거 */\
	FINLINE void QN_CONCAT(PREFIX, _dispose)(struct TYPENAME* self)	\
	{	\
		qn_free(self->DATA);	\
	}	\
	/* @brief 컨테이너 지우기(갯수를 0으로) */\
	FINLINE void QN_CONCAT(PREFIX, _clear)(struct TYPENAME* self)	\
	{	\
		self->COUNT = 0;	\
	}	\
	/* @brief 컨테이너 갯수 */\
	FINLINE size_t QN_CONCAT(PREFIX, _count)(const struct TYPENAME* self)	\
	{	\
		return self->COUNT;	\
	}	\
	/* @brief 컨테이너 데이터 포인터 */\
	FINLINE DATATYPE* QN_CONCAT(PREFIX, _data)(struct TYPENAME* self)	\
	{	\
		return self->DATA;	\
	}	\
	/* @brief 컨테이너 항목 얻기 */\
	FINLINE DATATYPE QN_CONCAT(PREFIX, _nth)(const struct TYPENAME* self, size_t index)	\
	{	\
		return self->DATA[index];	\
	}	\
	/* @brief 컨테이너 항목 뒤에서 부터 얻기 */\
	FINLINE DATATYPE QN_CONCAT(PREFIX, _inv)(const struct TYPENAME* self, size_t index)	\
	{	\
		return self->DATA[self->COUNT - index - 1];	\
	}	\
	/* @brief 컨테이너 항목 얻기 */\
	FINLINE DATATYPE* QN_CONCAT(PREFIX, _nth_ptr)(const struct TYPENAME* self, size_t index)	\
	{	\
		return &self->DATA[index];	\
	}	\
	/* @brief 컨테이너 항목 뒤에서 부터 얻기 */\
	FINLINE DATATYPE* QN_CONCAT(PREFIX, _inv_ptr)(const struct TYPENAME* self, size_t index)	\
	{	\
		return &self->DATA[self->COUNT - index - 1];	\
	}	\
	/* @brief 컨테이너 항목 설정 */\
	FINLINE void QN_CONCAT(PREFIX, _set)(struct TYPENAME* self, size_t index, DATATYPE value)	\
	{	\
		qn_debug_assert(index < self->COUNT, "invalid index!");	\
		self->DATA[index] = value;	\
	}	\
	/* @brief 컨테이너가 비었나? */\
	FINLINE bool QN_CONCAT(PREFIX, _is_empty)(const struct TYPENAME* self)	\
	{	\
		return self->COUNT == 0;	\
	}	\
	/* @brief 컨테이너 값이 있나? */\
	FINLINE bool QN_CONCAT(PREFIX, _is_have)(const struct TYPENAME* self)	\
	{	\
		return self->COUNT != 0;	\
	}	\
	/* @brief 컨테이너 정렬하기 */\
	FINLINE void QN_CONCAT(PREFIX, _sort)(struct TYPENAME* self, int(*compfunc)(const void*, const void*))	\
	{	\
		qn_qsort(self->DATA, self->COUNT, sizeof(DATATYPE), compfunc);	\
	}	\
	/* @brief 컨테이너 정렬하기(컨텍스트) */\
	FINLINE void QN_CONCAT(PREFIX, _sortc)(struct TYPENAME* self, int(*compfunc)(void*, const void*, const void*), void* context)	\
	{	\
		qn_qsortc(self->DATA, self->COUNT, sizeof(DATATYPE), compfunc, context);	\
	}	\
	/* @brief 컨테이너 항목 지우기 */\
	FINLINE void QN_CONCAT(PREFIX, _remove_nth)(struct TYPENAME* self, size_t nth)	\
	{	\
		qn_debug_assert(nth < self->COUNT, "index overflow");	\
		if (nth != self->COUNT - 1)	\
		{	\
			DATATYPE* ptr = self->DATA + nth;	\
			memmove(ptr, ptr + 1, sizeof(DATATYPE) * (self->COUNT - nth - 1));	\
		}	\
		self->COUNT--;	\
	}	\
	/* @brief 컨테이너 범위 지정해서 지우기 */\
	FINLINE void QN_CONCAT(PREFIX, _remove_range)(struct TYPENAME* self, size_t start, size_t count)	\
	{	\
		qn_debug_assert(start + count <= self->COUNT, "index overflow");	\
		size_t end = start + count;	\
		if (start < self->COUNT && end < self->COUNT)	\
		{	\
			if (end != self->COUNT)	\
			{	\
				DATATYPE* ptr = self->DATA + start;	\
				memmove(ptr, ptr + count, sizeof(DATATYPE) * (self->COUNT - end));	\
			}	\
			self->COUNT -= count;	\
		}	\
	}	\
	/* @brief 컨테이너 아이템 지우기 */\
	FINLINE void QN_CONCAT(PREFIX, _remove)(struct TYPENAME* self, DATATYPE value)	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			if (memcmp(ptr, &value, sizeof(DATATYPE)) != 0)	\
				ptr++;	\
			else	\
			{	\
				memmove(ptr, ptr + 1, sizeof(DATATYPE) * (end - ptr - 1));	\
				self->COUNT--;	\
			}	\
		}	\
	}	\
	/* @brief 컨테이너 아이템 지우기(콜백 사용) */\
	FINLINE void QN_CONCAT(PREFIX, _remove_callback)(struct TYPENAME* self, bool(*compfunc)(void*, DATATYPE), void* context)	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			if (compfunc(context, *ptr) == false)	\
				ptr++;	\
			else	\
			{	\
				memmove(ptr, ptr + 1, sizeof(DATATYPE) * (end - ptr - 1));	\
				self->COUNT--;	\
			}	\
		}	\
	}	\
	/* @brief 컨테이너에 아이템이 몇번째 있나 조사 */\
	FINLINE size_t QN_CONCAT(PREFIX, _contains)(const struct TYPENAME* self, DATATYPE value)	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			if (memcmp(ptr, &value, sizeof(DATATYPE)) == 0)	\
				return ptr - self->DATA;	\
			ptr++;	\
		}	\
		return (size_t)-1;	\
	}	\
	/* @brief 컨테이너에서 찾기 (콜백 사용) */\
	FINLINE size_t QN_CONCAT(PREFIX, _find)(const struct TYPENAME* self, bool(*compfunc)(void*, DATATYPE), void* context)	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			if (compfunc(context, *ptr))	\
				return ptr - self->DATA;	\
			ptr++;	\
		}	\
		return (size_t)-1;	\
	}	\
	/* @brief 컨테이너에서 찾기 (포인터 콜백 사용) */\
	FINLINE size_t QN_CONCAT(PREFIX, _find_ptr)(const struct TYPENAME* self, bool(*comfunc)(void*, void*), void* context)	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			if (comfunc(context, ptr))	\
				return ptr - self->DATA;	\
			ptr++;	\
		}	\
		return (size_t)-1;	\
	}	\
	/* @brief 컨테이너 foreach (콜백 사용) */\
	FINLINE void QN_CONCAT(PREFIX, _foreach_1)(const struct TYPENAME* self, void(*func)(DATATYPE))	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			func(*ptr);	\
			ptr++;	\
		}	\
	}	\
	/* @brief 컨테이너 foreach (콜백 사용) */\
	FINLINE void QN_CONCAT(PREFIX, _foreach_2)(const struct TYPENAME* self, void(*func)(void*, DATATYPE), void* context)	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			func(context, *ptr);	\
			ptr++;	\
		}	\
	}	\
	/* @brief 컨테이너 포인터 foreach (콜백 사용) */\
	FINLINE void QN_CONCAT(PREFIX, _foreach_ptr_1)(const struct TYPENAME* self, void(*func)(void*))	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			func(ptr);	\
			ptr++;	\
		}	\
	}	\
	/* @brief 컨테이너 포인터 foreach (콜백 사용) */\
	FINLINE void QN_CONCAT(PREFIX, _foreach_ptr_2)(const struct TYPENAME* self, void(*func)(void*, void*), void* context)	\
	{	\
		DATATYPE* ptr = self->DATA;	\
		DATATYPE* end = ptr + self->COUNT;	\
		while (ptr < end)	\
		{	\
			func(context, ptr);	\
			ptr++;	\
		}	\
	}

/// @brief 컨테이너 인라인
/// @param TYPENAME 컨테이너 이름
/// @param DATATYPE 데이터 타입
#define QN_DECL_CTNR(TYPENAME, DATATYPE)	\
	typedef struct TYPENAME	{ size_t COUNT; DATATYPE* DATA;	} TYPENAME

/// @brief 컨테이너 함수
/// @param TYPENAME 컨테이너 이름
/// @param DATATYPE 데이터 타입
/// @param PREFIX 함수 접두사
#define QN_IMPL_CTNR(TYPENAME, DATATYPE, PREFIX)	\
	QN_IMPL_CTNR_COMMON(TYPENAME, DATATYPE, PREFIX)	\
	/* @brief 컨테이너 초기화 */\
	FINLINE void QN_CONCAT(PREFIX, _init)(struct TYPENAME* self, size_t count)	\
	{	\
		if (count > 0)	\
		{	\
			self->COUNT = count;	\
			self->DATA = qn_alloc(count, DATATYPE);	\
		}	\
		else	\
		{	\
			self->COUNT = 0;	\
			self->DATA = NULL;	\
		}	\
	}	\
	/* @brief 컨테이너 초기화 (데이터도 0으로 초기화) */\
	FINLINE void QN_CONCAT(PREFIX, _init_zero)(struct TYPENAME* self, size_t count)	\
	{	\
		if (count > 0)	\
		{	\
			self->COUNT = count;	\
			self->DATA = qn_alloc_zero(count, DATATYPE);	\
		}	\
		else	\
		{	\
			self->COUNT = 0;	\
			self->DATA = NULL;	\
		}	\
	}	\
	/* @brief 컨테이너 복사 */\
	FINLINE void QN_CONCAT(PREFIX, _init_copy)(struct TYPENAME* self, const struct TYPENAME* src)	\
	{	\
		self->COUNT = src->COUNT;	\
		self->DATA = qn_memdup(src->DATA, sizeof(DATATYPE) * src->COUNT);	\
	}	\
	/* @brief 지정한 갯수와 데이터로 컨테이너 초기화 */\
	FINLINE void QN_CONCAT(PREFIX, _init_data)(struct TYPENAME* self, size_t count, DATATYPE* data)	\
	{	\
		self->COUNT = count;	\
		self->DATA = data;	\
	}	\
	/* @brief 컨테이너 크기 조정 */\
	FINLINE void QN_CONCAT(PREFIX, _resize)(struct TYPENAME* self, size_t count)	\
	{	\
		if (self->COUNT != count)	\
		{	\
			self->DATA = qn_realloc(self->DATA, count, DATATYPE);	\
			self->COUNT = count;	\
		}	\
	}	\
	/* @brief 컨테이너 크기 늘리기 */\
	FINLINE void QN_CONCAT(PREFIX, _expand)(struct TYPENAME* self, size_t count)	\
	{	\
		if (count > 0)	\
		{	\
			self->DATA = qn_realloc(self->DATA, self->COUNT + count, DATATYPE);	\
			self->COUNT += count;	\
		}	\
	}	\
	/* @brief 컨테이너에 항목 추가 */\
	FINLINE void QN_CONCAT(PREFIX, _add)(struct TYPENAME* self, DATATYPE value)	\
	{	\
		QN_CONCAT(PREFIX, _expand)(self, 1);	\
		self->DATA[self->COUNT - 1] = value;	\
	}	\
	/* @brief 컨테이너에 항목 삽입 */\
	FINLINE void QN_CONCAT(PREFIX, _insert)(struct TYPENAME* self, size_t nth, DATATYPE value)	\
	{	\
		qn_debug_assert(nth <= self->COUNT, "index overflow");	\
		QN_CONCAT(PREFIX, _expand)(self, 1);	\
		if (nth != self->COUNT - 1)	\
		{	\
			DATATYPE* ptr = self->DATA + nth;	\
			memmove(ptr + 1, ptr, sizeof(DATATYPE) * (self->COUNT - nth));	\
		}	\
		self->DATA[nth] = value;	\
	}	\
	typedef struct TYPENAME QN_CONCAT(TYPENAME, Type)

/// @brief 배열 인라인
/// @param TYPENAME 배열 이름
/// @param DATATYPE 데이터 타입
#define QN_DECL_ARRAY(TYPENAME, DATATYPE)	\
	typedef struct TYPENAME	{ size_t COUNT, CAPA; DATATYPE* DATA; } TYPENAME

/// @brief 배열 함수
/// @param TYPENAME 배열 이름
/// @param DATATYPE 데이터 타입
/// @param PREFIX 함수 접두사
#define QN_IMPL_ARRAY(TYPENAME, DATATYPE, PREFIX)	\
	QN_IMPL_CTNR_COMMON(TYPENAME, DATATYPE, PREFIX)	\
	/* @brief 배열 초기화 (용량 지정 가능) */\
	FINLINE void QN_CONCAT(PREFIX, _init)(struct TYPENAME* self, size_t capacity)	\
	{	\
		if (capacity > 0)	\
		{	\
			self->COUNT = 0;	\
			self->CAPA = capacity;	\
			self->DATA = qn_alloc(capacity, DATATYPE);	\
		}	\
		else	\
		{	\
			self->COUNT = 0;	\
			self->CAPA = 0;		\
			self->DATA = NULL;	\
		}	\
	}	\
	/* @brief 갯수와 데이터를 가지고 배열 초기화 */\
	FINLINE void QN_CONCAT(PREFIX, _init_data)(struct TYPENAME* self, size_t count, DATATYPE* data)	\
	{	\
		self->COUNT = count;	\
		self->CAPA = count;	\
		self->DATA = data;	\
	}	\
	/* @brief 내부적으로 배열 확장 */\
	FINLINE void QN_CONCAT(PREFIX, _internal_expand)(struct TYPENAME* self, size_t capa)\
	{	\
		capa += self->COUNT;	\
		if (self->CAPA < capa)	\
		{	\
			while (self->CAPA < capa)	\
				self->CAPA += self->CAPA / 2 + 1;	\
			self->DATA = qn_realloc(self->DATA, self->CAPA, DATATYPE);	\
		}	\
	}	\
	/* @brief 배열 크기 조정 (용량 조정이 아닌 크기 조정) */\
	FINLINE void QN_CONCAT(PREFIX, _resize)(struct TYPENAME* self, size_t count)	\
	{	\
		if (self->COUNT != count)	\
		{	\
			QN_CONCAT(PREFIX, _internal_expand)(self, count);	\
			self->COUNT = count;	\
		}	\
	}	\
	/* @brief 배열에 항목 추가 */\
	FINLINE void QN_CONCAT(PREFIX, _add)(struct TYPENAME* self, DATATYPE value)	\
	{	\
		QN_CONCAT(PREFIX, _internal_expand)(self, 1);	\
		self->DATA[self->COUNT++] = value;	\
	}	\
	/* @brief 배열에 항목 삽입 */\
	FINLINE void QN_CONCAT(PREFIX, _insert)(struct TYPENAME* self, size_t nth, DATATYPE value)	\
	{	\
		qn_debug_assert(nth <= self->COUNT, "index overflow");	\
		QN_CONCAT(PREFIX, _internal_expand)(self, 1);	\
		if (nth != self->COUNT - 1)	\
		{	\
			DATATYPE* ptr = self->DATA + nth;	\
			memmove(ptr + 1, ptr, sizeof(DATATYPE) * (self->COUNT - nth));	\
		}	\
		self->DATA[nth] = value;	\
		self->COUNT++;	\
	}	\
	typedef struct TYPENAME QN_CONCAT(TYPENAME, Type)

#define QN_DECLIMPL_CTNR(TYPENAME, DATATYPE, PREFIX)	\
	QN_DECL_CTNR(TYPENAME, DATATYPE);	\
	QN_IMPL_CTNR(TYPENAME, DATATYPE, PREFIX)

#define QN_DECLIMPL_ARRAY(TYPENAME, DATATYPE, PREFIX)	\
	QN_DECL_ARRAY(TYPENAME, DATATYPE);	\
	QN_IMPL_ARRAY(TYPENAME, DATATYPE, PREFIX)

// 기본 컨테이너
QN_DECLIMPL_CTNR(QnPtrCtnr, pointer_t, qn_pctnr);
QN_DECLIMPL_ARRAY(QnPtrArray, pointer_t, qn_parray);


//////////////////////////////////////////////////////////////////////////
// 리스트 인라인

#define QN_LIST_FOREACH(LIST,NODE,NEXTNODE)\
	for ((NODE) = (LIST).HEAD; (NODE) && ((NEXTNODE)=(NODE)->NEXT, true); (NODE)=(NEXTNODE))

/// @brief 리스트 인라인
/// @param TYPENAME 리스트 이름
/// @param DATATYPE 데이터 타입
#define QN_DECL_LIST(TYPENAME, DATATYPE)	\
	typedef struct QN_CONCAT(TYPENAME, Node) { struct QN_CONCAT(TYPENAME, Node) *NEXT, *PREV; DATATYPE DATA; } QN_CONCAT(TYPENAME, Node);	\
	typedef struct TYPENAME { QN_CONCAT(TYPENAME, Node) *HEAD, *TAIL; size_t COUNT; } TYPENAME

/// @brief 리스트 함수
/// @param TYPENAME 리스트 이름
/// @param DATATYPE 데이터 타입
/// @param PREFIX 함수 접두사
#define QN_IMPL_LIST(TYPENAME, DATATYPE, PREFIX)	\
	FINLINE size_t QN_CONCAT(PREFIX, _count)(const struct TYPENAME *list)	\
	{	\
		return list->COUNT;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _is_have)(const struct TYPENAME *list)	\
	{	\
		return list->COUNT != 0;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _is_empty)(const struct TYPENAME *list)	\
	{	\
		return list->COUNT == 0;	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(PREFIX, _head)(const struct TYPENAME *list)	\
	{	\
		return list->HEAD;	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(PREFIX, _tail)(const struct TYPENAME *list)	\
	{	\
		return list->TAIL;	\
	}	\
	FINLINE DATATYPE QN_CONCAT(PREFIX, _head_data)(const struct TYPENAME *list)	\
	{	\
		return list->HEAD->DATA;	\
	}	\
	FINLINE DATATYPE QN_CONCAT(PREFIX, _tail_data)(const struct TYPENAME *list)	\
	{	\
		return list->TAIL->DATA;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _init)(struct TYPENAME *list)	\
	{	\
		list->HEAD = list->TAIL = NULL;	\
		list->COUNT = 0;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _dispose)(struct TYPENAME *list)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD, *next; node; node = next)	\
		{	\
			next = node->NEXT;	\
			qn_free(node);	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _dispose_callback)(struct TYPENAME *list, void(*callback)(void*, DATATYPE), void* context)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD, *next; node; node = next)	\
		{	\
			next = node->NEXT;	\
			callback(context, node->DATA);	\
			qn_free(node);	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _dispose_ptr_callback)(struct TYPENAME *list, void(*callback)(void*, void*), void* context)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD, *next; node; node = next)	\
		{	\
			next = node->NEXT;	\
			callback(context, &node->DATA);	\
			qn_free(node);	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _clear)(struct TYPENAME *list)	\
	{	\
		QN_CONCAT(PREFIX, _dispose)(list);	\
		QN_CONCAT(PREFIX, _init)(list);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_node)(struct TYPENAME *list, QN_CONCAT(TYPENAME, Node) *node)	\
	{	\
		if (node == NULL)	\
			return;	\
		if (node->PREV)	\
			node->PREV->NEXT = node->NEXT;	\
		else	\
			list->HEAD = node->NEXT;	\
		if (node->NEXT)	\
			node->NEXT->PREV = node->PREV;	\
		else	\
			list->TAIL = node->PREV;	\
		qn_free(node);	\
		list->COUNT--;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_head)(struct TYPENAME *list)	\
	{	\
		QN_CONCAT(PREFIX, _remove_node)(list, list->HEAD);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_tail)(struct TYPENAME *list)	\
	{	\
		QN_CONCAT(PREFIX, _remove_node)(list, list->TAIL);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append)(struct TYPENAME *list, DATATYPE data)	\
	{	\
		QN_CONCAT(TYPENAME, Node) *node = qn_alloc_1(QN_CONCAT(TYPENAME, Node));	\
		node->DATA = data;	\
		node->NEXT = NULL;	\
		node->PREV = list->TAIL;	\
		if (list->TAIL)	\
			list->TAIL->NEXT = node;	\
		else	\
			list->HEAD = node;	\
		list->TAIL = node;	\
		list->COUNT++;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _prepend)(struct TYPENAME *list, DATATYPE data)	\
	{	\
		QN_CONCAT(TYPENAME, Node) *node = qn_alloc_1(QN_CONCAT(TYPENAME, Node));	\
		node->DATA = data;	\
		node->NEXT = list->HEAD;	\
		node->PREV = NULL;	\
		if (list->HEAD)	\
			list->HEAD->PREV = node;	\
		else	\
			list->TAIL = node;	\
		list->HEAD = node;	\
		list->COUNT++;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove)(struct TYPENAME *list, DATATYPE data, bool once)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD, *next; node; node = next)	\
		{	\
			next = node->NEXT;	\
			if (memcmp(&node->DATA, &data, sizeof(DATATYPE)))	\
			{	\
				QN_CONCAT(PREFIX, _remove_node)(list, node);	\
				if (once)	\
					break;	\
			}	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_callback)(struct TYPENAME *list, bool(*compfunc)(void*, DATATYPE), void* context, bool once)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD, *next; node; node = next)	\
		{	\
			next = node->NEXT;	\
			if (compfunc(context, node->DATA))	\
			{	\
				QN_CONCAT(PREFIX, _remove_node)(list, node);	\
				if (once)	\
					break;	\
			}	\
		}	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(PREFIX, _contains)(const struct TYPENAME *list, DATATYPE data)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD; node; node = node->NEXT)	\
			if (memcmp(&node->DATA, &data, sizeof(DATATYPE)))	\
				return node;	\
		return NULL;	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(PREFIX, _find)(const struct TYPENAME *list, bool(*compfunc)(void*, DATATYPE), void* context)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD; node; node = node->NEXT)	\
			if (compfunc(context, node->DATA))	\
				return node;	\
		return NULL;	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(PREFIX, _find_ptr)(const struct TYPENAME *list, bool(*compfunc)(void*, void*), void* context)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD; node; node = node->NEXT)	\
			if (compfunc(context, &node->DATA))	\
				return node;	\
		return NULL;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _foreach_1)(const struct TYPENAME *list, void(*callback)(DATATYPE))	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD; node; node = node->NEXT)	\
			callback(node->DATA);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _foreach_2)(const struct TYPENAME *list, void(*callback)(void*, DATATYPE), void* context)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD; node; node = node->NEXT)	\
			callback(context, node->DATA);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _foreach_ptr_1)(const struct TYPENAME *list, void(*callback)(void*))	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD; node; node = node->NEXT)	\
			callback(&node->DATA);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _foreach_ptr_2)(const struct TYPENAME *list, void(*callback)(void*, void*), void* context)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = list->HEAD; node; node = node->NEXT)	\
			callback(context, &node->DATA);	\
	}	\
	typedef struct TYPENAME QN_CONCAT(TYPENAME, Type)

/// @brief 노드 리스트 인라인
/// @param TYPENAME 리스트 이름
/// @param NODETYPE 노드 타입
#define QN_DECL_LNODE(TYPENAME, NODETYPE)	\
	/* struct NODETYPE { NODETYPE *PREV, *NEXT; } */	\
	typedef struct TYPENAME { NODETYPE *HEAD, *TAIL; size_t COUNT; } TYPENAME

/// @brief 노드 리스트 함수
/// @param TYPENAME 리스트 이름
/// @param NODETYPE 노드 타입
/// @param PREFIX 함수 접두사
#define QN_IMPL_LNODE(TYPENAME, NODETYPE, PREFIX)	\
	FINLINE size_t QN_CONCAT(PREFIX, _count)(const struct TYPENAME *list)	\
	{	\
		return list->COUNT;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _is_have)(const struct TYPENAME *list)	\
	{	\
		return list->COUNT != 0;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _is_empty)(const struct TYPENAME *list)	\
	{	\
		return list->COUNT == 0;	\
	}	\
	FINLINE NODETYPE* QN_CONCAT(PREFIX, _head)(const struct TYPENAME *list)	\
	{	\
		return list->HEAD;	\
	}	\
	FINLINE NODETYPE* QN_CONCAT(PREFIX, _tail)(const struct TYPENAME *list)	\
	{	\
		return list->TAIL;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _init)(struct TYPENAME *list)	\
	{	\
		list->HEAD = list->TAIL = NULL;	\
		list->COUNT = 0;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _dispose)(struct TYPENAME *list)	\
	{	\
		for (NODETYPE *node = list->HEAD, *next; node; node = next)	\
		{	\
			next = node->NEXT;	\
			qn_free(node);	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _dispose_callback)(struct TYPENAME *list, void(*callback)(void*))	\
	{	\
		for (NODETYPE *node = list->HEAD, *next; node; node = next)	\
		{	\
			next = node->NEXT;	\
			callback((void*)node);	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _reset)(struct TYPENAME *list)	\
	{	\
		QN_CONCAT(PREFIX, _init)(list);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _clear)(struct TYPENAME *list)	\
	{	\
		QN_CONCAT(PREFIX, _dispose)(list);	\
		QN_CONCAT(PREFIX, _init)(list);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _clear_callback)(struct TYPENAME *list, void(*callback)(void*))	\
	{	\
		QN_CONCAT(PREFIX, _dispose_callback)(list, callback);	\
		QN_CONCAT(PREFIX, _init)(list);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove)(struct TYPENAME *list, NODETYPE *node, bool freenode)	\
	{	\
		if (node == NULL)	\
			return;	\
		if (node->PREV)	\
			node->PREV->NEXT = node->NEXT;	\
		else	\
			list->HEAD = node->NEXT;	\
		if (node->NEXT)	\
			node->NEXT->PREV = node->PREV;	\
		else	\
			list->TAIL = node->PREV;	\
		if (freenode)	\
			qn_free(node);	\
		list->COUNT--;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_callback)(struct TYPENAME *list, NODETYPE *node, void(*callback)(void*))	\
	{	\
		if (node == NULL)	\
			return;	\
		if (node->PREV)	\
			node->PREV->NEXT = node->NEXT;	\
		else	\
			list->HEAD = node->NEXT;	\
		if (node->NEXT)	\
			node->NEXT->PREV = node->PREV;	\
		else	\
			list->TAIL = node->PREV;	\
		callback((void*)node);	\
		list->COUNT--;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_head)(struct TYPENAME *list, bool freenode)	\
	{	\
		QN_CONCAT(PREFIX, _remove)(list, list->HEAD, freenode);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_head_callback)(struct TYPENAME *list, void(*callback)(void*))	\
	{	\
		QN_CONCAT(PREFIX, _remove_callback)(list, list->HEAD, callback);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_tail)(struct TYPENAME *list, bool freenode)	\
	{	\
		QN_CONCAT(PREFIX, _remove)(list, list->TAIL, freenode);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_tail_callback)(struct TYPENAME *list, void(*callback)(void*))	\
	{	\
		QN_CONCAT(PREFIX, _remove_callback)(list, list->TAIL, callback);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _unlink)(struct TYPENAME *list, NODETYPE *node)	\
	{	\
		if (node == NULL)	\
			return;	\
		if (node->PREV)	\
			node->PREV->NEXT = node->NEXT;	\
		else	\
			list->HEAD = node->NEXT;	\
		if (node->NEXT)	\
			node->NEXT->PREV = node->PREV;	\
		else	\
			list->TAIL = node->PREV;	\
		node->PREV = node->NEXT = NULL;	\
		list->COUNT--;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _append)(struct TYPENAME *list, NODETYPE* node)	\
	{	\
		node->NEXT = NULL;	\
		node->PREV = list->TAIL;	\
		if (list->TAIL)	\
			list->TAIL->NEXT = node;	\
		else	\
			list->HEAD = node;	\
		list->TAIL = node;	\
		list->COUNT++;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _prepend)(struct TYPENAME *list, NODETYPE* node)	\
	{	\
		node->NEXT = list->HEAD;	\
		node->PREV = NULL;	\
		if (list->HEAD)	\
			list->HEAD->PREV = node;	\
		else	\
			list->TAIL = node;	\
		list->HEAD = node;	\
		list->COUNT++;	\
	}	\
	FINLINE NODETYPE* QN_CONCAT(PREFIX, _contains)(const struct TYPENAME *list, NODETYPE* dest)	\
	{	\
		for (NODETYPE *node = list->HEAD; node; node = node->NEXT)	\
			if (node == dest)	\
				return node;	\
		return NULL;	\
	}	\
	FINLINE NODETYPE* QN_CONCAT(PREFIX, _find)(const struct TYPENAME *list, bool(*compfunc)(void*, void*), void* context)	\
	{	\
		for (NODETYPE *node = list->HEAD; node; node = node->NEXT)	\
			if (compfunc(context, node))	\
				return node;	\
		return NULL;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _foreach_1)(const struct TYPENAME *list, void(*callback)(void*))	\
	{	\
		for (NODETYPE *next, *node = list->HEAD; node; node = next)	\
		{	\
			next = node->NEXT;	\
			callback(node);	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _foreach_2)(const struct TYPENAME *list, void(*callback)(void*, void*), void* context)	\
	{	\
		for (NODETYPE *next, *node = list->HEAD; node; node = next)	\
		{	\
			next = node->NEXT;	\
			callback(context, node);	\
		}	\
	}	\
	typedef struct TYPENAME QN_CONCAT(TYPENAME, Type)

#define QN_DECLIMPL_LIST(TYPENAME, DATATYPE, PREFIX)	\
	QN_DECL_LIST(TYPENAME, DATATYPE);	\
	QN_IMPL_LIST(TYPENAME, DATATYPE, PREFIX)

#define QN_DECLIMPL_LNODE(TYPENAME, NODETYPE, PREFIX)	\
	QN_DECL_LNODE(TYPENAME, NODETYPE);	\
	QN_IMPL_LNODE(TYPENAME, NODETYPE, PREFIX)


//////////////////////////////////////////////////////////////////////////	\
// 해시

/// @brief 해시용 foreach
#define QN_HASH_FOREACH(hash, node)	\
	for ((node) = (hash).HEAD; (node); (node) = (node)->NEXT)

/// @brief 해시 인라인
///	@param TYPENAME 해시 이름
///	@param KEYTYPE 키 타입
///	@param VALUETYPE 값 타입
#define QN_DECL_HASH(TYPENAME, KEYTYPE, VALUETYPE)	\
	typedef struct QN_CONCAT(TYPENAME, Node) { struct QN_CONCAT(TYPENAME, Node) *SIB, *NEXT, *PREV; size_t HASH; KEYTYPE KEY; VALUETYPE VALUE; } QN_CONCAT(TYPENAME, Node);	\
	typedef struct TYPENAME { size_t COUNT, REVISION, BUCKET; QN_CONCAT(TYPENAME, Node) **NODES, *HEAD, *TAIL; } TYPENAME

/// @brief 해시 함수
///	@param TYPENAME 해시 이름
///	@param KEYTYPE 키 타입
///	@param VALUETYPE 값 타입
///	@param KEYHASH 키 해시 함수
///	@param KEYEQ 키 비교 함수
///	@param KEYFREE 키 해제 함수
///	@param VALUEFREE 값 해제 함수
///	@param PREFIX 함수 접두사
#define QN_IMPL_HASH(TYPENAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PREFIX)	\
	FINLINE void QN_CONCAT(PREFIX, _init)(TYPENAME *hash)	\
	{	\
		hash->COUNT = hash->REVISION = 0;	\
		hash->BUCKET = QN_MIN_HASH;	\
		hash->NODES = qn_alloc_zero(QN_MIN_HASH, QN_CONCAT(TYPENAME, Node)*);	\
		hash->HEAD = hash->TAIL = NULL;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _init_fast)(TYPENAME *hash)	\
	{	\
		qn_debug_assert(hash->REVISION == 0 && hash->COUNT == 0 && hash->NODES == NULL, "cannot use fast init, use just init");	\
		hash->BUCKET = QN_MIN_HASH;	\
		hash->NODES = qn_alloc_zero(QN_MIN_HASH, QN_CONCAT(TYPENAME, Node)*);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _dispose)(TYPENAME *hash)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *next, *node = hash->HEAD; node; node = next)	\
		{	\
			next = node->NEXT;	\
			KEYFREE(node->KEY);	\
			VALUEFREE(&node->VALUE);	\
			qn_free(node);	\
		}	\
		qn_free(hash->NODES);	\
	}	\
	FINLINE size_t QN_CONCAT(PREFIX, _count)(TYPENAME *hash)	\
	{	\
		return hash->COUNT;	\
	}	\
	FINLINE size_t QN_CONCAT(PREFIX, _revision)(TYPENAME *hash)	\
	{	\
		return hash->REVISION;	\
	}	\
	FINLINE size_t QN_CONCAT(PREFIX, _bucket)(TYPENAME *hash)	\
	{	\
		return hash->BUCKET;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _is_have)(TYPENAME *hash)	\
	{	\
		return hash->COUNT != 0;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _is_empty)(TYPENAME *hash)	\
	{	\
		return hash->COUNT == 0;	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(PREFIX, _node_head)(TYPENAME *hash)	\
	{	\
		return hash->HEAD;	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(PREFIX, _node_tail)(TYPENAME *hash)	\
	{	\
		return hash->TAIL;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _internal_test_size)(TYPENAME *hash)	\
	{	\
		if ((hash->BUCKET >= 3 * hash->COUNT && hash->BUCKET > QN_MIN_HASH) ||	\
			(3 * hash->BUCKET <= hash->COUNT && hash->BUCKET < QN_MAX_HASH))	\
		{	\
			size_t new_bucket = qn_prime_near((uint)hash->COUNT);	\
			new_bucket = QN_CLAMP(new_bucket, QN_MIN_HASH, QN_MAX_HASH);	\
			QN_CONCAT(TYPENAME, Node)** new_nodes = qn_alloc_zero(new_bucket, QN_CONCAT(TYPENAME, Node)*);	\
			for (size_t i = 0; i < hash->BUCKET; ++i)	\
			{	\
				QN_CONCAT(TYPENAME, Node)* node = hash->NODES[i];	\
				while (node)	\
				{	\
					QN_CONCAT(TYPENAME, Node)* next = node->SIB;	\
					const size_t index = node->HASH % new_bucket;	\
					node->SIB = new_nodes[index];	\
					new_nodes[index] = node;	\
					node = next;	\
				}	\
			}	\
			qn_free(hash->NODES);	\
			hash->NODES = new_nodes;	\
			hash->BUCKET = new_bucket;	\
		}	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)** QN_CONCAT(PREFIX, _internal_lookup)(TYPENAME* hash, KEYTYPE key)	\
	{	\
		const size_t lh = KEYHASH(key);	\
		QN_CONCAT(TYPENAME, Node) *lnn, **ln = &hash->NODES[lh % hash->BUCKET];	\
		while ((lnn = *ln) != NULL)	\
		{	\
			if (lnn->HASH == lh && KEYEQ(lnn->KEY, key))	\
				break;	\
			ln = &lnn->SIB;	\
		}	\
		return ln;	\
	}	\
	FINLINE QN_CONCAT(TYPENAME, Node)** QN_CONCAT(PREFIX, _internal_lookup_hash)(TYPENAME* hash, KEYTYPE key, size_t* ret_hash)	\
	{	\
		const size_t lh = KEYHASH(key);	\
		QN_CONCAT(TYPENAME, Node) *lnn, **ln = &hash->NODES[lh % hash->BUCKET];	\
		while ((lnn = *ln) != NULL)	\
		{	\
			if (lnn->HASH == lh && KEYEQ(lnn->KEY, key))	\
				break;	\
			ln = &lnn->SIB;	\
		}	\
		*ret_hash = lh;	\
		return ln;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _internal_set)(TYPENAME* hash, KEYTYPE key, VALUETYPE* valueptr, bool replace)	\
	{	\
		size_t ah;	\
		QN_CONCAT(TYPENAME, Node)** an = QN_CONCAT(PREFIX, _internal_lookup_hash)(hash, key, &ah);	\
		QN_CONCAT(TYPENAME, Node)* ann = *an;	\
		if (ann != NULL)	\
		{	\
			KEYFREE(ann->KEY);	\
			VALUEFREE(&ann->VALUE);	\
			if (replace)	\
			{	\
				ann->KEY = key;	\
				ann->VALUE = *valueptr;	\
			}	\
		}	\
		else	\
		{	\
			ann = qn_alloc_1(QN_CONCAT(TYPENAME, Node));	\
			ann->SIB = NULL;	\
			ann->HASH = ah;	\
			ann->KEY = key;	\
			ann->VALUE = *valueptr;	\
			ann->NEXT = NULL;	\
			ann->PREV = hash->TAIL;	\
			if (hash->TAIL)	\
				hash->TAIL->NEXT = ann;	\
			else	\
				hash->HEAD = ann;	\
			hash->TAIL = ann;	\
			*an = ann;	\
			hash->REVISION++;	\
			hash->COUNT++;	\
			QN_CONCAT(PREFIX, _internal_test_size)(hash);	\
		}	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _internal_erase_node)(TYPENAME* hash, QN_CONCAT(TYPENAME, Node)** en)	\
	{	\
		QN_CONCAT(TYPENAME, Node)* enn = *en;	\
		*en = enn->SIB;	\
		if (enn->PREV)	\
			enn->PREV->NEXT = enn->NEXT;	\
		else	\
			hash->HEAD = enn->NEXT;	\
		if (enn->NEXT)	\
			enn->NEXT->PREV = enn->PREV;	\
		else	\
			hash->TAIL = enn->PREV;	\
		const size_t ebk = enn->HASH % hash->BUCKET;	\
		if (hash->NODES[ebk] == enn)	\
			hash->NODES[ebk] = NULL;	\
		KEYFREE(enn->KEY);	\
		VALUEFREE(&enn->VALUE);	\
		qn_free(enn);	\
		hash->REVISION++;	\
		hash->COUNT--;	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _internal_erase)(TYPENAME* hash, KEYTYPE key)	\
	{	\
		QN_CONCAT(TYPENAME, Node)** rn = QN_CONCAT(PREFIX, _internal_lookup)(hash, key);	\
		if (*rn == NULL)	\
			return false;	\
		QN_CONCAT(PREFIX, _internal_erase_node)(hash, rn);	\
		return true;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _internal_erase_all)(TYPENAME* hash)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *next, *node = hash->HEAD; node; node = next)	\
		{	\
			next = node->NEXT;	\
			KEYFREE(node->KEY);	\
			VALUEFREE(&node->VALUE);	\
			qn_free(node);	\
		}	\
		hash->HEAD = hash->TAIL = NULL;	\
		hash->COUNT = 0;	\
		memset(hash->NODES, 0, hash->BUCKET * sizeof(QN_CONCAT(TYPENAME, Node)*));	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _clear)(TYPENAME* hash)	\
	{	\
		QN_CONCAT(PREFIX, _internal_erase_all)(hash);	\
		QN_CONCAT(PREFIX, _internal_test_size)(hash);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _remove_node)(TYPENAME* hash, QN_CONCAT(TYPENAME, Node)* node)	\
	{	\
		QN_CONCAT(PREFIX, _internal_erase_node)(hash, &node);	\
		QN_CONCAT(PREFIX, _internal_test_size)(hash);	\
	}	\
	FINLINE VALUETYPE* QN_CONCAT(PREFIX, _get)(TYPENAME* hash, KEYTYPE key)	\
	{	\
		QN_CONCAT(TYPENAME, Node)** gn = QN_CONCAT(PREFIX, _internal_lookup)(hash, key);	\
		return *gn ? &(*gn)->VALUE : NULL;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _add)(TYPENAME* hash, KEYTYPE key, VALUETYPE value)	\
	{	\
		QN_CONCAT(PREFIX, _internal_set)(hash, key, &value, false);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _set)(TYPENAME* hash, KEYTYPE key, VALUETYPE value)	\
	{	\
		QN_CONCAT(PREFIX, _internal_set)(hash, key, &value, true);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _add_ptr)(TYPENAME* hash, KEYTYPE key, VALUETYPE* value)	\
	{	\
		QN_CONCAT(PREFIX, _internal_set)(hash, key, value, false);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _set_ptr)(TYPENAME* hash, KEYTYPE key, VALUETYPE* value)	\
	{	\
		QN_CONCAT(PREFIX, _internal_set)(hash, key, value, true);	\
	}	\
	FINLINE bool QN_CONCAT(PREFIX, _remove)(TYPENAME* hash, KEYTYPE key)	\
	{	\
		if (QN_CONCAT(PREFIX, _internal_erase)(hash, key) == false)	\
			return false;	\
		QN_CONCAT(PREFIX, _internal_test_size)(hash);	\
		return true;	\
	}	\
	FINLINE KEYTYPE QN_CONCAT(PREFIX, _find)(TYPENAME* hash, bool(*func)(void*, KEYTYPE, void*), void* context)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = hash->HEAD; node; node = node->NEXT)	\
		{	\
			if (func(context, node->KEY, &node->VALUE))	\
				return node->KEY;	\
		}	\
		return (KEYTYPE)0;	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _foreach_2)(TYPENAME* hash, void(*func)(KEYTYPE, void*))	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = hash->HEAD; node; node = node->NEXT)	\
			func(node->KEY, &node->VALUE);	\
	}	\
	FINLINE void QN_CONCAT(PREFIX, _foreach_3)(TYPENAME* hash, void(*func)(void*, KEYTYPE, void*), void* context)	\
	{	\
		for (QN_CONCAT(TYPENAME, Node) *node = hash->HEAD; node; node = node->NEXT)	\
			func(context, node->KEY, &node->VALUE);	\
	}	\
	typedef struct TYPENAME QN_CONCAT(TYPENAME, Type)

#define QN_DECLIMPL_HASH(TYPENAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PREFIX)	\
	QN_DECL_HASH(TYPENAME, KEYTYPE, VALUETYPE);	\
	QN_IMPL_HASH(TYPENAME, KEYTYPE, VALUETYPE, KEYHASH, KEYEQ, KEYFREE, VALUEFREE, PREFIX)

FINLINE bool qn_ctn_inteqv(int left, int right) { return left == right; }
FINLINE bool qn_ctn_uinteqv(uint left, uint right) { return left == right; }

// 키 정수
#define QN_DECLIMPL_INT_HASH(TYPENAME, VALUETYPE, VALUEFREE, PREFIX)\
	QN_DECLIMPL_HASH(TYPENAME, int, VALUETYPE, (int), qn_ctn_inteqv, (void), VALUEFREE, PREFIX)
// 키 정수 / 값 정수
#define QN_DECLIMPL_INT_INT_HASH(TYPENAME, PREFIX)\
	QN_DECLIMPL_INT_HASH(TYPENAME, int, (void), PREFIX)
// 키 정수 / 값 문자열
#define QN_DECLIMPL_INT_PCHAR_HASH(TYPENAME, PREFIX)\
	QN_DECLIMPL_INT_HASH(TYPENAME, char*, qn_mem_free_ptr, PREFIX)

// 키 부호없는 정수
#define QN_DECLIMPL_UINT_HASH(TYPENAME, VALUETYPE, VALUEFREE, PREFIX)\
	QN_DECLIMPL_HASH(TYPENAME, uint, VALUETYPE, (uint), qn_ctn_uinteqv, (void), VALUEFREE, PREFIX)
// 키 부호없는 정수 / 값 부호없는 정수
#define QN_DECLIMPL_UINT_UINT_HASH(TYPENAME, PREFIX)\
	QN_DECLIMPL_UINT_HASH(TYPENAME, uint, (void), PREFIX)
// 키 부호없는 정수 / 값 문자열
#define QN_DECLIMPL_UINT_PCHAR_HASH(TYPENAME, PREFIX)\
	QN_DECLIMPL_UINT_HASH(TYPENAME, char*, qn_mem_free_ptr, PREFIX)

// 키 문자열
#define QN_DECLIMPL_PCHAR_HASH(TYPENAME, VALUETYPE, VALUEFREE, PREFIX)\
	QN_DECLIMPL_HASH(TYPENAME, char*, VALUETYPE, qn_strhash, qn_streqv, qn_mem_free, VALUEFREE, PREFIX)
// 키 문자열 / 값 정수
#define QN_DECLIMPL_PCHAR_INT_HASH(TYPENAME, PREFIX)\
	QN_DECLIMPL_PCHAR_HASH(TYPENAME, int, (void), PREFIX)
// 키 문자열 / 값 문자열
#define QN_DECLIMPL_PCHAR_PCHAR_HASH(TYPENAME, PREFIX)\
	QN_DECLIMPL_PCHAR_HASH(TYPENAME, char*, qn_mem_free_ptr, PREFIX)

#ifdef _MSC_VER
#pragma warning(pop)
#endif
