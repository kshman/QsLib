//
// QsLib [CONTAINER Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
// SPDX-License-Identifier: UNLICENSE
//

#pragma once
#define __QS_CTN__

#ifndef __QS_QN__
#error include "qs_qn.h" first
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)		// 비표준 확장이 사용됨: 구조체/공용 구조체의 배열 크기가 0입니다.
#pragma warning(disable:5045)		// 컴파일러는 /Qspectre 스위치가 지정된 경우 메모리 로드를 위해 스펙터 완화를 삽입합니다.
#endif

QN_EXTC_BEGIN

//////////////////////////////////////////////////////////////////////////
// common

// 공통
#define qn_ctn_name_type(name)			name##Type
#define qn_ctn_name_node(name)			name##Node
#define qn_ctn_sizeof_type(name)		sizeof(name##Type)
#define qn_ctn_sizeof_node(name)		sizeof(name##Node)


//////////////////////////////////////////////////////////////////////////
// container

/// @brief 고정틱한 배열 정의(컨테이너)
/// @param name 배열 이름
/// @param type 배열 요소 타입
#define QN_DECL_CTNR(name,type)\
	typedef type name##Type;\
	typedef struct name { size_t COUNT; type* DATA; } name	// NOLINT
QN_DECL_CTNR(QnPtrCtnr, void*);								/// @brief 포인터 배열
QN_DECL_CTNR(QnByteCtn, byte);								/// @brief 바이트 배열
QN_DECL_CTNR(QnIntCtn, int);								/// @brief 정수 배열
QN_DECL_CTNR(QnAnyCtn, any_t);								/// @brief any_t 배열

#define qn_ctnr_init(name, p, count)\
	QN_STMT_BEGIN{\
		if ((size_t)(count)>0) {\
			(p)->DATA=qn_alloc((size_t)(count), name##Type);\
			(p)->COUNT=(size_t)(count);\
		} else {\
			(p)->DATA=NULL;\
			(p)->COUNT=0;\
		}\
	}QN_STMT_END
#define qn_ctnr_init_zero(name, p, count)\
	QN_STMT_BEGIN{\
		if ((size_t)(count)>0) {\
			(p)->DATA=qn_alloc_zero((size_t)(count), name##Type);\
			(p)->COUNT=(size_t)(count);\
		} else {\
			(p)->DATA=NULL;\
			(p)->COUNT=0;\
		}\
	}QN_STMT_END
#define qn_ctnr_init_copy(name, p, from)\
	QN_STMT_BEGIN{\
		(p)->COUNT=(from)->COUNT;\
		(p)->DATA=qn_memdup((from)->DATA, qn_ctn_sizeof_type(name) * ((from)->COUNT));\
	}QN_STMT_END
#define qn_ctnr_init_data(p, data, count)\
	QN_STMT_BEGIN{\
		(p)->DATA=data;\
		(p)->COUNT=count;\
	}QN_STMT_END

#define qn_ctnr_nth(p,nth)				(((p)->DATA)[(size_t)(nth)])
#define qn_ctnr_inv(p,nth)				(((p)->DATA)[((p)->COUNT - 1 - (size_t)(nth))])
#define qn_ctnr_count(p)				((p)->COUNT)
#define qn_ctnr_data(p)					((p)->DATA)
#define qn_ctnr_set(p,nth,item)			(((p)->DATA)[(size_t)(nth)]=(item))
#define qn_ctnr_is_empty(p)				((p)->COUNT == 0)
#define qn_ctnr_is_have(p)				((p)->COUNT != 0)

#define qn_ctnr_sort(name,p,func)		qn_qsort((p)->DATA, (p)->COUNT, qn_ctn_sizeof_type(name), func)
#define qn_ctnr_sortc(name,p,func,data)	qn_qsortc((p)->DATA, (p)->COUNT, qn_ctn_sizeof_type(name), func, data)

#define qn_ctnr_disp(p)					qn_free((p)->DATA)
#define qn_ctnr_clear(p)				((p)->COUNT=0)
#define qn_ctnr_remove_nth(name,p,nth)\
	QN_STMT_BEGIN{\
		qn_assert((size_t)(nth)<(p)->COUNT && "index overflow");\
		name##Type* __t=(name##Type*)(p)->DATA+(nth);\
		if ((size_t)(nth)!=(p)->COUNT-1)\
			memmove(__t, __t+1, qn_ctn_sizeof_type(name)*((p)->COUNT-((size_t)(nth)+1)));\
		(p)->COUNT--;\
	}QN_STMT_END
#define qn_ctnr_remove_range(name,p,index,count)\
	QN_STMT_BEGIN{\
		size_t __end=(size_t)(index)+(size_t)(count);\
		size_t __cnt=(p)->COUNT;\
		if ((size_t)(index)<__cnt && __end<__cnt) {\
			if (__end!=__cnt-1)\
				memmove((p)->DATA+(index), (p)->DATA+__end, qn_ctn_sizeof_type(name)*(__cnt-__end));\
			(p)->COUNT-=(size_t)(count);\
		}\
	}QN_STMT_END
#define qn_ctnr_remove(name,p,item)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (__i=0; __i<__cnt; __i++) {\
			name##Type* __t = (p)->DATA+__i;\
			if (*__t==(item)) {\
				memmove(__t, __t+1, qn_ctn_sizeof_type(name)*(__cnt-(__i+1)));\
				__cnt--;\
				break;\
			}\
		}\
		(p)->COUNT=__cnt;\
	}QN_STMT_END
#define qn_ctnr_remove_cmp(name,p,func2,data)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (__i=0; __i<__cnt; __i++) {\
			name##Type* __t = (p)->DATA+__i;\
			if (func2(data,*__t)) {\
				memmove(__t, __t+1, qn_ctn_sizeof_type(name)*(__cnt-(__i+1)));\
				__cnt--;\
				break;\
			}\
		}\
		(p)->COUNT=__cnt;\
	}QN_STMT_END
#define qn_ctnr_remove_cmp_ptr(name,p,func2,data)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (__i=0; __i<__cnt; __i++) {\
			name##Type* __t = (p)->DATA+__i;\
			if (func2(data,__t)) {\
				memmove(__t, __t+1, qn_ctn_sizeof_type(name)*(__cnt-(__i+1)));\
				__cnt--;\
				break;\
			}\
		}\
		(p)->COUNT=__cnt;\
	}QN_STMT_END

#define qn_ctnr_contains(p,item,ret_pindex)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (*(ret_pindex)=-1, __i=0; __i<__cnt; __i++) {\
			if (*((p)->DATA+__i)==(item))\
			{\
				*(ret_pindex)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_ctnr_find(p,start,func2,data,ret_pindex)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (*(ret_pindex)=-1, __i=(size_t)(start); __i<__cnt; __i++) {\
			if (func2(data, *((p)->DATA+__i))) {\
				*(ret_pindex)=__i;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_ctnr_find_ptr(p,start,func2,data,ret_pindex)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (*(ret_pindex)=-1, __i=(size_t)(start); __i<__cnt; __i++) {\
			if (func2(data, (p)->DATA+__i)) {\
				*(ret_pindex)=__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_ctnr_foreach(p, index)\
	size_t QN_CONCAT(count,__LINE__) = (p)->COUNT;\
	for ((index)=0;(index)<QN_CONCAT(count,__LINE__);(index)++)
#define qn_ctnr_foreach_1(p,func1)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (__i=0; __i<__cnt; __i++)\
			func1(*((p)->DATA+__i));\
	}QN_STMT_END
#define qn_ctnr_foreach_2(p,func2,data)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (__i=0; __i<__cnt; __i++)\
			func2(data, *((p)->DATA+__i));\
	}QN_STMT_END
#define qn_ctnr_foreach_ptr_1(p,func1)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (__i=0; __i<__cnt; __i++)\
			func1((p)->DATA+__i);\
	}QN_STMT_END
#define qn_ctnr_foreach_ptr_2(p,func2,data)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->COUNT;\
		for (__i=0; __i<__cnt; __i++)\
			func2(data, (p)->DATA+__i);\
	}QN_STMT_END

#define qn_ctnr_resize(name, p, count)\
	QN_STMT_BEGIN{\
		if ((p)->COUNT!=(size_t)(count)) {\
			(p)->DATA=qn_realloc((p)->DATA, (size_t)(count), name##Type);\
			(p)->COUNT=(size_t)(count);\
		}\
	}QN_STMT_END
#define qn_ctnr_expand(name, p, count)\
	QN_STMT_BEGIN{\
		if ((size_t)(count)!=0) {\
			(p)->DATA=qn_realloc((p)->DATA, (p)->COUNT+(size_t)(count), name##Type);\
			(p)->COUNT+=(size_t)(count);\
		}\
	}QN_STMT_END

#define qn_ctnr_add(name,p,item)\
	QN_STMT_BEGIN{\
		qn_ctnr_expand(name,p,1);\
		qn_ctnr_set(p,(p)->COUNT-1,item);\
	}QN_STMT_END
#define qn_ctnr_insert(name,p,nth,item)\
	QN_STMT_BEGIN{\
		qn_ctnr_expand(name,p,1);\
		memmove((p)->DATA+(nth)+1, (p)->DATA+(nth), ((p)->COUNT-1)*sizeof(name##Type));\
		qn_ctnr_set(p,nth,item);\
		/*오류처리 안하고 있음*/\
	}QN_STMT_END

#define qn_pctnr_init(p, count)			qn_ctnr_init(QnPtrCtnr, (QnPtrCtnr*)(p), count)
#define qn_pctnr_init_zero(p, count)	qn_ctnr_init_zero(QnPtrCtnr, (QnPtrCtnr*)(p), count)
#define qn_pctnr_init_copy(p, from)		qn_ctnr_init_copy(QnPtrCtnr, (QnPtrCtnr*)(p), (QnPtrCtnr*)(from))

#define qn_pctnr_nth(p,nth)				qn_ctnr_nth(p,nth)
#define qn_pctnr_inv(p,nth)				qn_ctnr_inv(p,nth)
#define qn_pctnr_count(p)				qn_ctnr_count(p)
#define qn_pctnr_data(p)				qn_ctnr_data(p)
#define qn_pctnr_set(p,nth,item)		qn_ctnr_set(p,nth,item)
#define qn_pctnr_is_empty(p)			qn_ctnr_is_empty(p)
#define qn_pctnr_is_have(p)				qn_ctnr_is_have(p)

#define qn_pctnr_sort(name,p,func)		qn_ctnr_sort(name,p,func)
#define qn_pctnr_sortc(name,p,func,data)\
										qn_ctnr_sortc(name,p,func,data)

#define qn_pctnr_disp(p)				qn_ctnr_disp(p)
#define qn_pctnr_clear(p)				qn_ctnr_clear(p)
#define qn_pctnr_remove_nth(p, index)	qn_ctnr_remove_nth(QnPtrArr, p, index)
#define qn_pctnr_remove_range(p, index, count)\
										qn_ctnr_remove_range(QnPtrArr, p, index, count)
#define qn_pctnr_remove(p, item)		qn_ctnr_remove(QnPtrArr, p, item)
#define qn_pctnr_remove_cmp(p,func2,item)\
										qn_ctnr_remove_cmp(QnPtrArr, p, func2, item)

#define qn_pctnr_contains(p, data, ret_pindex)\
										qn_ctnr_contains(p, data, ret_pindex)
#define qn_pctnr_find(p, start, func2, userdata, ret_pindex)\
										qn_ctnr_find(p, start, func2, userdata, ret_pindex)
#define qn_pctnr_find_ptr(p, start, func2, userdata, ret_pindex)\
										qn_ctnr_find_ptr(p, start, func2, userdata, ret_pindex)

#define qn_pctnr_foreach(p,index)		qn_ctnr_foreach(p,index)
#define qn_pctnr_foreach_1(p,func1)		qn_ctnr_foreach_1(p,func1)
#define qn_pctnr_foreach_2(p,f2,data)	qn_ctnr_foreach_2(p,f2,data)

#define qn_pctnr_resize(p, count)		qn_ctnr_resize(QnPtrCtnr, (QnPtrCtnr*)(p), count)
#define qn_pctnr_expand(p, count)		qn_ctnr_expand(QnPtrCtnr, (QnPtrCtnr*)(p), count)

#define qn_pctnr_add(p, item)			qn_ctnr_add(QnPtrCtnr, (QnPtrCtnr*)(p), (void*)(item))
#define qn_pctnr_insert(p, nth, item)	qn_ctnr_insert(QnPtrCtnr, (QnPtrCtnr*)(p), nth, (void*)(item))


//////////////////////////////////////////////////////////////////////////
// array > ctnr

/// @brief 동적 배열 정의
/// @param name 배열 이름
/// @param type 배열 요소 타입
#define QN_DECL_ARR(name,type)\
	typedef type name##Type;\
	typedef struct name { size_t COUNT; size_t CAPA; type* DATA; } name	// NOLINT
QN_DECL_ARR(QnPtrArr, void*);								/// @brief 포인터 배열
QN_DECL_ARR(QnByteArr, byte);								/// @brief 바이트 배열
QN_DECL_ARR(QnIntArr, int);									/// @brief 정수 배열
QN_DECL_ARR(QnAnyArr, any_t);								/// @brief any_t 배열

#define qn_arr_init(name,p,capacity)\
	QN_STMT_BEGIN{\
		(p)->DATA=NULL;\
		(p)->COUNT=0;\
		(p)->CAPA=0;\
		if ((capacity)>0) qn_inl_arr_extend((void*)(p), qn_ctn_sizeof_type(name), (capacity));\
	}QN_STMT_END
#define qn_arr_init_copy(name,p,from)\
	QN_STMT_BEGIN{\
		(p)->COUNT=(from)->COUNT;\
		(p)->CAPA=(from)->CAPA;\
		(p)->DATA=qn_memdup((from)->DATA, qn_ctn_sizeof_type(name) * ((from)->COUNT));\
	}QN_STMT_END
#define qn_arr_init_data(p, data, count)\
	QN_STMT_BEGIN{\
		(p)->DATA=data;\
		(p)->COUNT=count;\
		(p)->CAPA=count;\
	}QN_STMT_END

#define qn_arr_nth(p,nth)				qn_ctnr_nth(p,nth)
#define qn_arr_inv(p,nth)				qn_ctnr_inv(p,nth)
#define qn_arr_count(p)					qn_ctnr_count(p)
#define qn_arr_capacity(p)				((p)->CAPA)
#define qn_arr_data(p)					qn_ctnr_data(p)
#define qn_arr_set(p,th,item)			qn_ctnr_set(p,nth,item)
#define qn_arr_is_empty(p)				qn_ctnr_is_empty(p)
#define qn_arr_is_have(p)				qn_ctnr_is_have(p)

#define qn_arr_sort(name,p,func)		qn_ctnr_sort(name,p,func)
#define qn_arr_sortc(name,p,func,data)	qn_ctnr_sortc(name,p,func,data)

#define qn_arr_disp(p)					qn_ctnr_disp(p)
#define qn_arr_clear(p)					qn_ctnr_clear(p)
#define qn_arr_remove_nth(name,p,nth)	qn_ctnr_remove_nth(name,p,nth)
#define qn_arr_remove_range(name,p,index,count)\
										qn_ctnr_remove_range(name,p,index,count)
#define qn_arr_remove(name,p,item)		qn_ctnr_remove(name,p,item)
#define qn_arr_remove_cmp(name,p,func2,item)\
										qn_ctnr_remove_cmp(name,p,func2,item)
#define qn_arr_remove_cmp_ptr(name,p,func2,item)\
										qn_ctnr_remove_cmp_ptr(name,p,func2,item)

#define qn_arr_contains(p,item,ret_pindex)\
										qn_ctnr_contains(p,item,ret_pindex)
#define qn_arr_find(p,start,func2,data,ret_pindex)\
										qn_ctnr_find(p,start,func2,data,ret_pindex)
#define qn_arr_find_ptr(p,start,func2,data,ret_pindex)\
										qn_ctnr_find_ptr(p,start,func2,data,ret_pindex)

#define qn_arr_foreach(p,index)			qn_ctnr_foreach(p,index)
#define qn_arr_foreach_1(p,func1)		qn_ctnr_foreach_1(p,func1)
#define qn_arr_foreach_2(p,f2,data)		qn_ctnr_foreach_2(p,f2,data)
#define qn_arr_foreach_ptr_1(p,func1)	qn_ctnr_foreach_ptr_1(p,func1)
#define qn_arr_foreach_ptr_2(p,f2,data)	qn_ctnr_foreach_ptr_2(p,f2,data)

#define qn_arr_resize(name,p,count)\
	QN_STMT_BEGIN{\
		qn_inl_arr_extend((void*)(p), qn_ctn_sizeof_type(name), count);\
		(p)->COUNT=count;\
	}QN_STMT_END
#define qn_arr_expand(name,p,count)		qn_arr_resize(name,p,(p)->COUNT+(count))

#define qn_arr_add(name,p,item)\
	QN_STMT_BEGIN{\
		qn_arr_expand(name, p, 1);\
		qn_arr_nth(p,(p)->COUNT-1)=(item);\
	}QN_STMT_END
#define qn_arr_add_blob(name,p,items,count)\
	QN_STMT_BEGIN{\
		if ((size_t)(count)>0) {\
			name##Type* __d=(p)->DATA+(p)->COUNT;\
			qn_arr_expand(name, p, count);\
			memcpy(__d, items, qn_ctn_sizeof_type(name) * (size_t)(count));\
		}\
	}QN_STMT_END
#define qn_arr_insert(name,p,nth,item)\
	QN_STMT_BEGIN{\
		if ((size_t)(nth)<=(p)->COUNT) {\
			qn_arr_expand(name p, 1);\
			memmove((p)->DATA+(size_t)(nth)+1, (p)->DATA+(size_t)(nth), ((p)->COUNT-1-(size_t)(nth))*qn_ctn_sizeof_type(name));\
			qn_arr_nth(p,nth)=(item);\
		}\
	}QN_STMT_END

QN_INLINE void qn_inl_arr_extend(void* arr, size_t size, size_t capa)
{
	QnByteArr* p = (QnByteArr*)arr;
	if (p->CAPA < capa)
	{
		while (p->CAPA < capa)
			p->CAPA = p->CAPA + p->CAPA / 2 + 1;
		p->DATA = qn_realloc(p->DATA, p->CAPA * size, byte);
	}
}

#define qn_parr_init(p, capacity)		qn_arr_init(QnPtrArr, p, capacity)
#define qn_parr_init_data(p,data,count)	qn_arr_init_data(p,data,count)

#define qn_parr_nth(p,nth)				qn_ctnr_nth(p,nth)
#define qn_parr_inv(p,nth)				qn_ctnr_inv(p,nth)
#define qn_parr_count(p)				qn_ctnr_count(p)
#define qn_parr_capacity(p)				qn_arr_capacity(p)
#define qn_parr_data(p)					qn_ctnr_data(p)
#define qn_parr_set(p,nth,item)			qn_ctnr_set(p,nth,item)
#define qn_parr_is_empty(p)				qn_ctnr_is_empty(p)
#define qn_parr_is_have(p)				qn_ctnr_is_have(p)

#define qn_parr_sort(name,p,func)		qn_ctnr_sort(name,p,func)
#define qn_parr_sortc(name,p,func,data)	qn_ctnr_sortc(name,p,func,data)

#define qn_parr_disp(p)					qn_ctnr_disp(p)
#define qn_parr_clear(p)				qn_ctnr_clear(p)
#define qn_parr_remove_nth(p, index)	qn_ctnr_remove_nth(QnPtrArr, p, index)
#define qn_parr_remove_range(p, index, count)\
										qn_ctnr_remove_range(QnPtrArr, p, index, count)
#define qn_parr_remove(p, item)			qn_ctnr_remove(QnPtrArr, p, item)
#define qn_parr_remove_cmp(p,func2,item)\
										qn_ctnr_remove_cmp(QnPtrArr, p, func2, item)

#define qn_parr_contains(p, data, ret_pindex)\
										qn_ctnr_contains(p, data, ret_pindex)
#define qn_parr_find(p, start, func2, userdata, ret_pindex)\
										qn_ctnr_find(p, start, func2, userdata, ret_pindex)
#define qn_parr_find_ptr(p, start, func2, userdata, ret_pindex)\
										qn_ctnr_find_ptr(p, start, func2, userdata, ret_pindex)

#define qn_parr_foreach(p,index)		qn_ctnr_foreach(p,index)
#define qn_parr_foreach_1(p,func1)		qn_ctnr_foreach_1(p,func1)
#define qn_parr_foreach_2(p,func2,data)	qn_ctnr_foreach_2(p,func2,data)

#define qn_parr_set_count(p,count)		qn_arr_resize(QnPtrArr, p, count)
#define qn_parr_expand(p,count)			qn_arr_expand(QnPtrArr, p, count)

#define qn_parr_add(p, item)			qn_arr_add(QnPtrArr, p, item)
#define qn_parr_add_blob(p,items,cnt)	qn_arr_add_blob(QnPtrArr, p, items, cnt)
#define qn_parr_insert(p, index, item)	qn_arr_insert(QnPtrArr, index, item)


//////////////////////////////////////////////////////////////////////////
// fixed slice

/// @brief 슬라이스(최대 개수를 지정할 수 있는 반-동적 배열) 정의
/// @param name 슬라이스 이름
/// @param type 슬라이스 요소 타입
#define QN_DECL_SLICE(name, type)\
	typedef type name##Type;\
	typedef struct name { size_t MAX; size_t COUNT; type* DATA; } name	// NOLINT

#define qn_slice_init(name,p,maximum)\
	QN_STMT_BEGIN{\
		(p)->MAX=maximum;\
		(p)->COUNT=0;\
		(p)->DATA=qn_alloc(maximum, name##Type);\
	}QN_STMT_END
#define qn_slice_test_init(name,p,maximum)\
	QN_STMT_BEGIN{\
		qn_assert((p)->DATA==NULL && (p)->MAX==0 && (p)->COUNT==0 && "uninitialized memory");\
		(p)->DATA=qn_alloc(maximum, name##Type);\
		(p)->MAX=maximum;\
	}QN_STMT_END

#define qn_slice_nth(p,nth)				qn_ctnr_nth(p,nth)
#define qn_slice_inv(p,nth)				qn_ctnr_inv(p,nth)
#define qn_slice_count(p)				qn_ctnr_count(p)
#define qn_slice_maximum(p)				((p)->MAX)
#define qn_slice_data(p)				qn_ctnr_data(p)
#define qn_slice_set(p,nth,item)		qn_ctnr_set(p,nth,item)
#define qn_slice_is_empty(p)			qn_ctnr_is_empty(p)
#define qn_slice_is_have(p)				qn_ctnr_is_have(p)

#define qn_slice_sort(name,p,func)		qn_ctnr_sort(name,p,func)
#define qn_slice_sortc(name,p,func,data)\
										qn_ctnr_sortc(name,p,func,data)

#define qn_slice_disp(p)				qn_ctnr_disp(p)
#define qn_slice_clear(p)				qn_ctnr_clear(p)
#define qn_slice_disable(name,p)		((p)->MAX=(p)->COUNT)

#define qn_slice_contains(p,item,ret_pindex)\
										qn_ctnr_contains(p,item,ret_pindex)
#define qn_slice_find(p,start,func2,data,ret_pindex)\
										qn_ctnr_find(p,start,func2,data,ret_pindex)
#define qn_slice_find_ptr(p,start,func2,data,ret_pindex)\
										qn_ctnr_find_ptr(p,start,func2,data,ret_pindex)

#define qn_slice_remove_nth(name,p,nth)	qn_ctnr_remove_nth(name,p,nth)
#define qn_slice_remove_range(name,p,index,count)\
										qn_ctnr_remove_range(name,p,index,count)
#define qn_slice_remove(name,p,item)	qn_ctnr_remove(name,p,item)
#define qn_slice_remove_cmp(name,p,func2,item)\
										qn_ctnr_remove_cmp(name,p,func2,item)
#define qn_slice_remove_cmp_ptr(name,p,func2,item)\
										qn_ctnr_remove_cmp_ptr(name,p,func2,item)


#define qn_slice_foreach(p,index)		qn_ctnr_foreach(p,index)
#define qn_slice_foreach_1(p,func1)		qn_ctnr_foreach_1(p,func1)
#define qn_slice_foreach_2(p,f2,data)	qn_ctnr_foreach_2(p,f2,data)
#define qn_slice_foreach_ptr_1(p,func1)	qn_ctnr_foreach_ptr_1(p,func1)
#define qn_slice_foreach_ptr_2(p,func2,data)\
										qn_ctnr_foreach_ptr_2(p,func2,data)

#define qn_slice_expand(name,p,newmax)\
	QN_STMT_BEGIN{\
		if ((p)->COUNT>(newmax))\
			(p)->COUNT=(newmax);\
		(p)->DATA=qn_realloc((p)->DATA, newmax, name##Type);\
		(p)->MAX=(newmax);\
	}QN_STMT_END
#define qn_slice_push(name,p,itemref,ret_pbool_nullable)\
	QN_STMT_BEGIN{\
		bool* __r=(ret_pbool_nullable);\
		if ((p)->COUNT==(p)->MAX) {\
			if (__r) *__r=false;\
		} else {\
			if (__r) *__r=true;\
			(p)->DATA[(p)->COUNT]=*(itemref);\
			(p)->COUNT++;\
		}\
	}QN_STMT_END
#define qn_slice_push_ptr(name,p,item,ret_pbool_nullable)\
	QN_STMT_BEGIN{\
		bool* __r=(ret_pbool_nullable);\
		if ((p)->COUNT==(p)->MAX) {\
			if (__r) *__r=false;\
		} else {\
			if (__r) *__r=true;\
			(p)->DATA[(p)->COUNT]=(item);\
			(p)->COUNT++;\
		}\
	}QN_STMT_END
#define qn_slice_pop(name,p,ret_pbool_nullable)\
	QN_STMT_BEGIN{\
		bool* __r=(ret_pbool_nullable);\
		if ((p)->COUNT==0) {\
			if (__r) *__r=false;\
		} else if ((p)->COUNT==1) {\
			if (__r) *__r=true;\
			(p)->COUNT=0;\
		} else {\
			if (__r) *__r=true;\
			(p)->COUNT--;\
			(p)->DATA[0]=(p)->DATA[(p)->COUNT];\
		}\
	}QN_STMT_END
#define qn_slice_pop_ptr(name,p,ret_pbool_nullable)\
	qn_slice_pop(name,p,ret_pbool_nullable)
#define qn_slice_pop_value(name,p,ret_pvalue,ret_pbool_nullable)\
	QN_STMT_BEGIN{\
		bool* __r=(ret_pbool_nullable);\
		if ((p)->COUNT==0) {\
			if (__r) *__r=false;\
		} else if ((p)->COUNT==1) {\
			if (__r) *__r=true;\
			*(ret_pvalue)=(p)->DATA[0];\
			(p)->COUNT=0;\
		} else {\
			if (__r) *__r=true;\
			*(ret_pvalue)=(p)->DATA[0];\
			(p)->COUNT--;\
			(p)->DATA[0]=(p)->DATA[(p)->COUNT];\
		}\
	}QN_STMT_END
#define qn_slice_pop_value_ptr(name,p,ret_pvalue)\
	QN_STMT_BEGIN{\
		if ((p)->COUNT==0) {\
			*(ret_pvalue)=NULL;\
		} else if ((p)->COUNT==1) {\
			*(ret_pvalue)=(p)->DATA[0];\
			(p)->COUNT=0;\
		} else {\
			*(ret_pvalue)=(p)->DATA[0];\
			(p)->COUNT--;\
			(p)->DATA[0]=(p)->DATA[(p)->COUNT];\
		}\
	}QN_STMT_END



//////////////////////////////////////////////////////////////////////////
// list

/// @brief 이중 연결 리스트 정의
/// @param name 리스트 이름
/// @param type 리스트 요소 타입
#define QN_DECL_LIST(name,type)\
	typedef type name##Type;\
	typedef struct name##Node { struct name##Node *NEXT, *PREV; type DATA; } name##Node;\
	typedef struct name { name##Node *FIRST, *LAST; size_t COUNT; } name
QN_DECL_LIST(QnPtrList, void*);

#define qn_list_count(p)					((p)->COUNT)
#define qn_list_node_first(p)				((p)->LAST)
#define qn_list_node_last(p)				((p)->FIRST)
#define qn_list_data_first(p)				((p)->LAST->DATA)
#define qn_list_data_last(p)				((p)->FIRST->DATA)
#define qn_list_is_have(p)					((p)->COUNT!=0)
#define qn_list_is_empty(p)					((p)->COUNT==0)

#define qn_list_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->FIRST=NULL;\
		(p)->LAST=NULL;\
		(p)->COUNT=0;\
	}QN_STMT_END

#define qn_list_disp(name,p)\
	QN_STMT_BEGIN{\
		for (name##Node *__next, *__node=(p)->FIRST; __node; __node=__next) {\
			__next=__node->NEXT;\
			qn_free(__node);\
		}\
	}QN_STMT_END
#define qn_list_clear(name,p)\
	QN_STMT_BEGIN{\
		qn_list_disp(name,p);\
		(p)->FIRST=(p)->LAST=NULL;\
		(p)->COUNT=0;\
	}QN_STMT_END

#define qn_list_remove_node(name,p,node)\
	QN_STMT_BEGIN{\
		name##Node* __node=(node);\
		if (__node) {\
			if (__node->NEXT)\
				__node->NEXT->PREV=__node->PREV;\
			else {\
				qn_assert((p)->LAST == __node);\
				(p)->LAST=__node->PREV;\
			}\
			if (__node->PREV)\
				__node->PREV->NEXT=__node->NEXT;\
			else {\
				qn_assert((p)->FIRST == __node);\
				(p)->FIRST=__node->NEXT;\
			}\
			(p)->COUNT--;\
			qn_free(__node);\
		}\
	}QN_STMT_END
#define qn_list_remove_first(name,p)\
	qn_list_remove_node(name, p, (p)->LAST)
#define qn_list_remove_last(name,p)\
	qn_list_remove_node(name, p, (p)->FIRST)

// value type
#define qn_list_disp_cb(name,p,func2,data)\
	QN_STMT_BEGIN{\
		name##Node *__node, *__next;\
		for (__node=(p)->FIRST; __node; __node=__next) {\
			__next=__node->NEXT;\
			func2(data,__node->DATA);\
			qn_free(__node);\
		}\
	}QN_STMT_END
#define qn_list_append(name,p,item)\
	QN_STMT_BEGIN{\
		name##Node* __node=qn_alloc_1(name##Node);\
		if (__node) {\
			__node->DATA=item;\
			if ((p)->FIRST)\
				(p)->FIRST->PREV=__node;\
			else\
				(p)->LAST=__node;\
			__node->NEXT=(p)->FIRST;\
			__node->PREV=NULL;\
			(p)->FIRST=__node;\
			(p)->COUNT++;\
		}\
	}QN_STMT_END
#define qn_list_prepend(name,p,item)\
	QN_STMT_BEGIN{\
		name##Node* __node=qn_alloc_1(name##Node);\
		if (__node) {\
			__node->DATA=item;\
			if ((p)->LAST)\
				(p)->LAST->NEXT=__node;\
			else\
				(p)->FIRST=__node;\
			__node->PREV=(p)->LAST;\
			__node->NEXT=NULL;\
			(p)->LAST=__node;\
			(p)->COUNT++;\
		}\
	}QN_STMT_END
#define qn_list_remove(name,p,item)\
	QN_STMT_BEGIN{\
		name##Node* __node;\
		for (__node=(p)->FIRST; __node; __node=__node->NEXT) {\
			if (__node->DATA==(item)) {\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_list_remove_cmp(name,p,func2,data)\
	QN_STMT_BEGIN{\
		name##Node* __node;\
		for (__node=(p)->FIRST; __node; __node=__node->NEXT) {\
			if (func2(data, __node->DATA)) {\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_list_contains(name,p,item,ret_pnode)\
	QN_STMT_BEGIN{\
		name##Node* __node;\
		for (*(ret_pnode)=NULL, __node=(p)->FIRST; __node; __node=__node->NEXT) {\
			if (__node->DATA==(item)) {\
				*(ret_pnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_list_find(name,p,func2,data,ret_pnode)\
	QN_STMT_BEGIN{\
		name##Node* __node;\
		for (*(ret_pnode)=NULL, __node=(p)->FIRST; __node; __node=__node->NEXT) {\
			if (func2(data, __node->DATA)) {\
				*(ret_pnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_list_foreach(name,p,node)\
	name##Node* QN_CONCAT(prev,__LINE__);\
	for ((node)=(p)->LAST; (node) && (QN_CONCAT(prev,__LINE__)=(node)->PREV, true); (node)=QN_CONCAT(prev,__LINE__))
#define qn_list_foreach_1(name,p,func1)\
	QN_STMT_BEGIN{\
		name##Node *__node, *__prev;\
		for (__node=(p)->LAST; __node; __node=__prev) {\
			__prev = __node->PREV; func1(__node->DATA); }\
	}QN_STMT_END
#define qn_list_foreach_2(name,p,func2,data)\
	QN_STMT_BEGIN{\
		name##Node *__node, *__prev;\
		for (__node=(p)->LAST; __node; __node=__prev) {\
			__prev = __node->PREV; func2(data, __node->DATA); }\
	}QN_STMT_END

// reference type
#define qn_list_ref_disp_cb(name,p,func2,data)\
	QN_STMT_BEGIN{\
		name##Node *__node, *__next;\
		for (__node=(p)->FIRST; __node; __node=__next) {\
			__next=__node->NEXT;\
			func2(data,&__node->DATA);\
			qn_free(__node);\
		}\
	}QN_STMT_END
#define qn_list_ref_append(name,p,pitem)\
	QN_STMT_BEGIN{\
		name##Node* __node=qn_alloc_1(name##Node);\
		if (__node){ memcpy(&__node->DATA, pitem, sizeof(name##Type));\
			if ((p)->FIRST)\
				(p)->FIRST->PREV=__node;\
			else\
				(p)->LAST=__node;\
			__node->NEXT=(p)->FIRST;\
			__node->PREV=NULL;\
			(p)->FIRST=__node;\
			(p)->COUNT++;\
		}\
	}QN_STMT_END
#define qn_list_ref_prepend(name,p,pitem)\
	QN_STMT_BEGIN{\
		name##Node* __node=qn_alloc_1(name##Node);\
		if (__node) {\
			memcpy(&__node->DATA, pitem, sizeof(name##Type));\
			if ((p)->LAST)\
				(p)->LAST->NEXT=__node;\
			else\
				(p)->FIRST=__node;\
			__node->PREV=(p)->LAST;\
			__node->NEXT=NULL;\
			(p)->LAST=__node;\
			(p)->COUNT++;\
		}\
	}QN_STMT_END
#define qn_list_ref_remove(name,p,pitem)\
	QN_STMT_BEGIN{\
		name##Node* __node;\
		for (__node=(p)->FIRST; __node; __node=__node->NEXT) {\
			if (memcmp(&__node->DATA, pitem)==0) {\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_list_ref_remove_cmp(name,p,func2,data)\
	QN_STMT_BEGIN{\
		name##Node* __rcnode;\
		for (__rcnode=(p)->FIRST; __rcnode; __rcnode=__rcnode->NEXT) {\
			if (func2(data, &__rcnode->DATA)) {\
				qn_list_remove_node(name, p, __rcnode);\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_list_ref_contains(name,p,pitem,ret_pnode)\
	QN_STMT_BEGIN{\
		name##Node* __node;\
		for (*(ret_pnode)=NULL, __node=(p)->FIRST; __node; __node=__node->NEXT) {\
			if (memcmp(&__node->DATA, pitem)==0) {\
				*(ret_pnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_list_ref_find(name,p,func2,data,ret_pnode)\
	QN_STMT_BEGIN{\
		name##Node* __node;\
		for (*(ret_pnode)=NULL, __node=(p)->FIRST; __node; __node=__node->NEXT) {\
			if (func2(data, &__node->DATA)) {\
				*(ret_pnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_list_ref_foreach_1(name,p,func1)\
	QN_STMT_BEGIN{\
		name##Node *__node, *__prev;\
		for (__node=(p)->LAST; __node; __node=__prev) {\
			__prev = __node->PREV; func1(&__node->DATA); }\
	}QN_STMT_END
#define qn_list_ref_foreach_2(name,p,func2,data)\
	QN_STMT_BEGIN{\
		name##Node *__node, *__prev;\
		for (__node=(p)->LAST; __node; __node=__prev) {\
			__prev = __node->PREV; func2(data, &__node->DATA); }\
	}QN_STMT_END

#define qn_plist_count(p)				qn_list_count(p)
#define qn_plist_first(p)				qn_list_data_first(p)
#define qn_plist_last(p)				qn_list_data_last(p)
#define qn_plist_peek_first(p)			qn_list_node_first(p)
#define qn_plist_peek_last(p)			qn_list_node_last(p)
#define qn_plist_is_have(p)				qn_list_is_have(p)
#define qn_plist_is_empty(p)			qn_list_is_empty(p)

#define qn_plist_init(p)				qn_list_init(QnPtrList, p)
#define qn_plist_disp(p)				qn_list_disp(QnPtrList, p)
#define qn_plist_disp_cb(p,func_user_data,userdata)\
										qn_list_disp_cb(QnPtrList, p, func_user_data, userdata)
#define qn_plist_append(p,item)			qn_list_append(QnPtrList, p, item)
#define qn_plist_prepend(p,item)		qn_list_prepend(QnPtrList, p, item)
#define qn_plist_clear(p)				qn_list_clear(QnPtrList, p)
#define qn_plist_remove(p,item)			qn_list_remove(QnPtrList, p, item)
#define qn_plist_remove_first(p)		qn_list_remove_first(QnPtrList, p)
#define qn_plist_remove_last(p)			qn_list_remove_last(QnPtrList, p)
#define qn_plist_foreach_1(p,func1)		qn_list_foreach_1(QnPtrList, p, func1)
#define qn_plist_foreach_2(p,func1,data)\
										qn_list_foreach_2(QnPtrList, p, func1, data)

QN_INLINE bool qn_plist_contains(const QnPtrList* p, const void* item)
{
	const QnPtrListNode* node = NULL;
	qn_list_contains(QnPtrList, p, item, &node);
	return node != NULL;
}

QN_INLINE bool qn_plist_find(const QnPtrList* p, bool (*func2)(void* data, void* node), void* data)
{
	const QnPtrListNode* node = NULL;
	qn_list_find(QnPtrList, p, func2, data, &node);
	return node != NULL;
}


//////////////////////////////////////////////////////////////////////////
// node list

/// @brief 노드 리스트 정의
/// @param name 노드 리스트 이름
/// @param type 노드 리스트 요소 타입
#define QN_DECL_NODELIST(name,type)\
	typedef type name##Type;\
	typedef struct name { name##Type* FIRST; name##Type* LAST; size_t COUNT; } name	// NOLINT

#define qn_nodelist_count(p)			((p)->COUNT)
#define qn_nodelist_first(p)			((p)->LAST)
#define qn_nodelist_last(p)				((p)->FIRST)
#define qn_nodelist_is_have(p)			((p)->COUNT!=0)
#define qn_nodelist_is_empty(p)			((p)->COUNT==0)

#define qn_nodelist_init(p)\
	QN_STMT_BEGIN{\
		(p)->FIRST=NULL;\
		(p)->LAST=NULL;\
		(p)->COUNT=0;\
	}QN_STMT_END

#define qn_nodelist_disp(name,p)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__next;\
		for (__node=(p)->FIRST; __node; __node=__next) {\
			__next=__node->NEXT;\
			qn_free(__node);\
		}\
	}QN_STMT_END
#define qn_nodelist_disp_cb(name,p,func1)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__next;\
		for (__node=(p)->FIRST; __node; __node=__next) {\
			__next=__node->NEXT;\
			func1(__node);\
		}\
	}QN_STMT_END

#define qn_nodelist_append(name,p,item)\
	QN_STMT_BEGIN{\
		if ((p)->FIRST)\
			(p)->FIRST->PREV=(item);\
		else\
			(p)->LAST=(item);\
		(item)->NEXT=(p)->FIRST;\
		(item)->PREV=NULL;\
		(p)->FIRST=(item);\
		(p)->COUNT++;\
	}QN_STMT_END
#define qn_nodelist_prepend(name,p,item)\
	QN_STMT_BEGIN{\
		if ((p)->LAST)\
			(p)->LAST->NEXT=(item);\
		else\
			(p)->FIRST=(item);\
		(item)->PREV=(p)->LAST;\
		(item)->NEXT=NULL;\
		(p)->LAST=(item);\
		(p)->COUNT++;\
	}QN_STMT_END

#define qn_nodelist_clear_nodes(p)\
	QN_STMT_BEGIN{\
		(p)->FIRST=(p)->LAST=NULL;\
		(p)->COUNT=0;\
	}QN_STMT_END
#define qn_nodelist_clear(name,p)\
	QN_STMT_BEGIN{\
		qn_nodelist_disp(name,p);\
		qn_nodelist_clear_nodes(p);\
	}QN_STMT_END
#define qn_nodelist_clear_cb(name,p,func1)\
	QN_STMT_BEGIN{\
		qn_nodelist_disp_cb(name,p,func1);\
		qn_nodelist_clear_nodes(p);\
	}QN_STMT_END
#define qn_nodelist_remove(name,p,item)\
	QN_STMT_BEGIN{\
		if (item) {\
			name##Type* __node=(name##Type*)(item);\
			if (__node->NEXT)\
				__node->NEXT->PREV=__node->PREV;\
			else {\
				qn_assert((p)->LAST == __node);\
				(p)->LAST=__node->PREV;\
			}\
			if (__node->PREV)\
				__node->PREV->NEXT=__node->NEXT;\
			else {\
				qn_assert((p)->FIRST == __node);\
				(p)->FIRST=__node->NEXT;\
			}\
			(p)->COUNT--;\
		}\
	}QN_STMT_END
#define qn_nodelist_remove_cb(name,p,item,func1)\
	QN_STMT_BEGIN{\
		if (item) {\
			name##Type* __node=(name##Type*)(item);\
			if (__node->NEXT)\
				__node->NEXT->PREV=__node->PREV;\
			else {\
				qn_assert((p)->LAST == __node);\
				(p)->LAST=__node->PREV;\
			}\
			if (__node->PREV)\
				__node->PREV->NEXT=__node->NEXT;\
			else {\
				qn_assert((p)->FIRST == __node);\
				(p)->FIRST=__node->NEXT;\
			}\
			(p)->COUNT--;\
			func1(__node);\
		}\
	}QN_STMT_END
#define qn_nodelist_remove_first(name,p)\
	qn_nodelist_remove(name, p, (p)->LAST)
#define qn_nodelist_remove_first_cb(name,p,func1)\
	qn_nodelist_remove_cb(name, p, (p)->LAST,func1)
#define qn_nodelist_remove_last(name,p)\
	qn_nodelist_remove(name, p, (p)->FIRST)
#define qn_nodelist_remove_last_cb(name,p,func1)\
	qn_nodelist_remove_cb(name, p, (p)->FIRST,func1)
#define qn_nodelist_unlink(name,p,item)\
	QN_STMT_BEGIN{\
		if (item) {\
			name##Type* __node=(name##Type*)(item);\
			if (__node->NEXT)\
				__node->NEXT->PREV=__node->PREV;\
			else {\
				qn_assert((p)->LAST == __node);\
				(p)->LAST=__node->PREV;\
			}\
			if (__node->PREV)\
				__node->PREV->NEXT=__node->NEXT;\
			else {\
				qn_assert((p)->FIRST == __node);\
				(p)->FIRST=__node->NEXT;\
			}\
			(p)->COUNT--;\
			__node->PREV=__node->NEXT=NULL;\
		}\
	}QN_STMT_END

#define qn_nodelist_contains(name,p,item,ret_pbool)\
	QN_STMT_BEGIN{\
		name##Type* __node;\
		for (*(ret_pbool)=false, __node=(p)->FIRST; __node; __node=__node->NEXT) {\
			if (__node==(item)) {\
				*(ret_pbool)=true;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_nodelist_find(name,p,func2,data,ret_pnode)\
	QN_STMT_BEGIN{\
		name##Type* __node;\
		for (*(ret_pnode)=NULL, __node=(p)->LAST; __node; __node=__node->PREV) {\
			if (func2(data,__node)) {\
				*(ret_pnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_nodelist_foreach(name,p,node)\
	name##Type *QN_CONCAT(prev,__LINE__);\
	for ((node)=(p)->LAST; (node) && (QN_CONCAT(prev,__LINE__) = (node)->PREV, true); (node)=QN_CONCAT(prev,__LINE__))
#define qn_nodelist_foreach_1(name,p,func1)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__prev;\
		for (__node=(p)->LAST; __node; __node=__prev) {\
			__prev=__node->PREV;\
			func1(__node);\
		}\
	}QN_STMT_END
#define qn_nodelist_foreach_2(name,p,func2,data)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__prev;\
		for (__node=(p)->LAST; __node; __node=__prev) {\
			__prev=__node->PREV;\
			func2(data,__node);\
		}\
	}QN_STMT_END


//////////////////////////////////////////////////////////////////////////
// solo list

/// @brief 단일 리스트 정의
/// @param name 단일 리스트 이름
/// @param type 단일 리스트 요소 타입
#define QN_DECL_SLIST(name,type)\
	typedef type name##Type;\
	typedef struct name { struct name* NEXT; type DATA; } name, name##Name	// NOLINT
QN_DECL_SLIST(QnPtrSlist, void*);

#define qn_slist_last(p)				qn_inl_slist_last(p)
#define qn_slist_data(p)				((p)->DATA)
#define qn_slist_next(p)				((p)->NEXT)
#define qn_slist_count(p)				qn_inl_slist_count(p)
#define qn_slist_nth(p,nth)				qn_inl_slist_nth(p, nth)
#define qn_slist_data_nth(p,nth)		((qn_inl_slist_nth(p, nth))->DATA)

#define qn_slist_delete(name, p, ret_ptr)\
	QN_STMT_BEGIN{\
		if (!(p))\
			*(ret_ptr)=NULL;\
		else {\
			name##Name* __n=(p)->NEXT;\
			qn_free(p);\
			*(ret_ptr)=__n;\
		}\
	}QN_STMT_END
#define qn_slist_disp(name, p)\
	QN_STMT_BEGIN{\
		name##Name *__n, *__p;\
		for (__p=(p); __p; __p=__n) {\
			__n=__p->NEXT;\
			qn_free(__p);\
		}\
	}QN_STMT_END
#define qn_slist_disp_func(name, p, func1)\
	QN_STMT_BEGIN{\
		name##Name *__n, *__p;\
		for (__p=(p); __p; __p=__n) {\
			__n=__p->NEXT;\
			func1(__p);\
		}\
	}QN_STMT_END

#define qn_slist_concat(name, p1, p2, ret_ptr)\
	QN_STMT_BEGIN{\
		if (!(p2))\
			*(ret_ptr)=(p1);\
		else {\
			if (!(p1))\
				*(ret_ptr)=(p2);\
			else {\
				name##Name* __l=(name##Name*)qn_slist_last(p1);\
				__l->NEXT=(p2);\
				*(ret_ptr)=(p1);\
			}\
		}\
	}QN_STMT_END
#define qn_slist_append(name, p, item, ret_ptr)\
	QN_STMT_BEGIN{\
		name##Name* __np=qn_alloc_1(name##Name);\
		if (__np) {\
			__np->NEXT=NULL;\
			__np->DATA=item;\
			if (!(p))\
				*(ret_ptr)=__np;\
			else {\
				name##Name* __l=(name##Name*)qn_slist_last(p);\
				__l->NEXT=__np;\
				*(ret_ptr)=(p);\
			}\
		}\
	}QN_STMT_END
#define qn_slist_prepend(name, p, item, ret_ptr)\
	QN_STMT_BEGIN{\
		name##Name* __np=qn_alloc_1(name##Name);\
		if (__np) {\
			__np->NEXT=(p);\
			__np->DATA=item;\
			*(ret_ptr)=__np;\
		}\
	}QN_STMT_END
#define qn_slist_reserve_append(name, p, ret_ptr)\
	QN_STMT_BEGIN{\
		name##Name* __np=qn_alloc_zero_1(name##Name);\
		if (!(p))\
			*(ret_ptr)=__np;\
		else {\
			name##Name* __l=(name##Name*)qn_slist_last(p);\
			__l->NEXT=__np;\
			*(ret_ptr)=(p);\
		}\
	}QN_STMT_END
#define qn_slist_reserve_prepend(name, p, ret_ptr)\
	QN_STMT_BEGIN{\
		name##Name* __np=qn_alloc_zero_1(name##Name);\
		if (__np) __np->NEXT=(p);\
		*(ret_ptr)=__np;\
	}QN_STMT_END
#define qn_slist_insert(name, p, sib, item, ret_ptr)\
	QN_STMT_BEGIN{\
		if (!(p)) {\
			name##Name* __np=qn_alloc_zero_1(name##Name);\
			if (__np) {\
				__np->NEXT=NULL;\
				__np->DATA=item;\
			}\
			*(ret_ptr)=__np;\
		} else {\
			name##Name *__np, *__l=NULL;\
			for (__np=(p); __np; __l=__np, __np=__l->NEXT)\
			{\
				if (__np==(sib))\
					break;\
			}\
			__np=qn_alloc_1(name##Name);\
			if (__np) {\
				__np->DATA=item;\
				if (!__l) {\
					__np->NEXT=(p);\
					*(ret_ptr)=__np;\
				} else {\
					__np->NEXT=__l->NEXT;\
					__l->NEXT=__np;\
					*(ret_ptr)=(p);\
				}\
			}\
		}\
	}QN_STMT_END
#define qn_slist_insert_nth(name, p, nth, item, ret_ptr)\
	QN_STMT_BEGIN{\
		size_t __h=(size_t)(nth);\
		if (__h<0) {\
			qn_slist_append(name, p, item, ret_ptr);\
		} else if (__h==0) {\
			qn_slist_prepend(name, p, item, ret_ptr);\
		} else {\
			name##Name* __np=qn_alloc_1(name##Name);\
			if (__np) {\
				__np->DATA=item;\
				if (!(p)) {\
					__np->NEXT=NULL;\
					*(ret_ptr)=__np;\
				} else {\
					name##Name *__v=NULL, *__t=(p);\
					while (__h-->0 && __t) {\
						__v=__t;\
						__t=__t->NEXT;\
					}\
					if (__v) {\
						__np->NEXT=__v->NEXT;\
						__v->NEXT=__np;\
						*(ret_ptr)=(p);\
					} else {\
						__np->NEXT=(p);\
						*(ret_ptr)=__np;\
					}\
				}\
			}\
		}\
	}QN_STMT_END

#define qn_slist_remove(name, p, item, ret_ptr)\
	QN_STMT_BEGIN{\
		name##Name *__r=(p), *__t=(p), *__v=NULL;\
		while (__t) {\
			if (__t->DATA==(item)) {\
				if (__v)\
					__v->NEXT=__t->NEXT;\
				else\
					__r=__t->NEXT;\
				qn_free(__t);\
				break;\
			}\
			__v=__t;\
			__t=__t->NEXT;\
		}\
		*(ret_ptr)=__r;\
	}QN_STMT_END
#define qn_slist_remove_link(name, p, link, ret_ptr)\
	QN_STMT_BEGIN{\
		name##Name *__r=(p), *__t=(p), *__v=NULL;\
		while (__t) {\
			if (__t==(link)) {\
				if (__v)\
					__v->NEXT=__t->NEXT;\
				if (__r==__t)\
					__r=__r->NEXT;\
				__t->NEXT=NULL;\
				break;\
			}\
			__v=__t;\
			__t=__t->NEXT;\
		}\
		qn_free(link);\
		*(ret_ptr)=__r;\
	}QN_STMT_END

#define qn_slist_copy(name, p, ret_ptr)\
	QN_STMT_BEGIN{\
		if (!(p))\
			*(ret_ptr)=NULL;\
		else {\
			name##Name *__p, *__l, *__n=qn_alloc_zero_1(name##Name);\
			if (!__n)\
				*(ret_ptr)=NULL;\
			else {\
				__n->DATA=(p)->DATA;\
				__l=__n;\
				__p=(p)->NEXT;\
				while (__p) {\
					__l->NEXT=qn_alloc_1(name##Name);\
					if (!__l->NEXT) break;\
					__l=__l->NEXT;\
					__l->DATA=__p->DATA;\
					__p=__p->NEXT;\
				}\
				if (__l) __l->NEXT=NULL;\
				*(ret_ptr)=__n;\
			}\
		}\
	}QN_STMT_END
#define qn_slist_contains(name,p,item,ret_ptr)\
	QN_STMT_BEGIN{\
		name##Name* __p=(p);\
		while (__p) {\
			if (__p->DATA==(item))\
				break;\
			__p=__p->NEXT;\
		}\
		*(ret_ptr)=__p;\
	}QN_STMT_END
#define qn_slist_contains_func(name,p,func2,data,ret_ptr)\
	QN_STMT_BEGIN{\
		name##Name* __p=(p);\
		while (__p) {\
			if (func2(data,__p->DATA))\
				break;\
			__p=__p->NEXT;\
		}\
		*(ret_ptr)=__p;\
	}QN_STMT_END

#define qn_slist_foreach_1(name,p,func1)\
	QN_STMT_BEGIN{\
		name##Name* __p=(p);\
		while (__p) {\
			name##Name* __n=__p->NEXT;\
			func1(__p->DATA);\
			__p=__n;\
		}\
	}QN_STMT_END
#define qn_slist_foreach_2(name,p,func2,data)\
	QN_STMT_BEGIN{\
		name##Name* __p=(p);\
		while (__p) {\
			name##Name* __n=__p->NEXT;\
			func2(data, __p->DATA);\
			__p=__n;\
		}\
	}QN_STMT_END

QN_INLINE void* qn_inl_slist_last(void* ptr)
{
	QnPtrSlist* p = (QnPtrSlist*)ptr;
	while (p->NEXT)
		p = p->NEXT;
	return p;
}

QN_INLINE void* qn_inl_slist_nth(void* ptr, size_t nth)
{
	QnPtrSlist* p = (QnPtrSlist*)ptr;
	while (nth-- > 0 && p)
		p = p->NEXT;
	return (void*)p;
}

QN_INLINE size_t qn_inl_slist_count(void* ptr)
{
	const QnPtrSlist* p = (QnPtrSlist*)ptr;
	size_t n = 0;
	while (p) {
		n++;
		p = p->NEXT;
	}
	return n;
}

#define qn_pslist_last(p)				qn_slist_last(p)
#define qn_pslist_nth(p, nth)			qn_slist_nth(p, nth)
#define qn_pslist_data(p, nth)			qn_slist_data(p, nth)
#define qn_pslist_count(p)				qn_slist_count(p)

QN_INLINE QnPtrSlist* qn_pslist_delete(QnPtrSlist* p)
{
	qn_slist_delete(QnPtrSlist, p, &p);
	return p;
}

QN_INLINE void qn_pslist_disp(QnPtrSlist* p, void (*func1)(void*))
{
	if (!func1) {
		qn_slist_disp(QnPtrSlist, p);
	}
	else {
		for (QnPtrSlist* n; p; p = n) {
			n = p->NEXT;
			if (func1)
				func1(p->DATA);
			qn_free(p);
		}
	}
}

QN_INLINE QnPtrSlist* qn_pslist_concat(QnPtrSlist* p1, QnPtrSlist* p2)
{
	qn_slist_concat(QnPtrSlist, p1, p2, &p1);
	return p1;
}

QN_INLINE QnPtrSlist* qn_pslist_append(QnPtrSlist* p, void* item)
{
	qn_slist_append(QnPtrSlist, p, item, &p);
	return p;
}

QN_INLINE QnPtrSlist* qn_pslist_prepend(QnPtrSlist* p, void* item)
{
	qn_slist_prepend(QnPtrSlist, p, item, &p);
	return p;
}

QN_INLINE QnPtrSlist* qn_pslist_insert(QnPtrSlist* p, const QnPtrSlist* sib, void* item)
{
	qn_slist_insert(QnPtrSlist, p, sib, item, &p);
	return p;
}

QN_INLINE QnPtrSlist* qn_pslist_insert_nth(QnPtrSlist* p, int nth, void* item)
{
	qn_slist_insert_nth(QnPtrSlist, p, nth, item, &p);
	return p;
}

QN_INLINE QnPtrSlist* qn_pslist_remove(QnPtrSlist* p, const void* item)
{
	qn_slist_remove(QnPtrSlist, p, item, &p);
	return p;
}

QN_INLINE QnPtrSlist* qn_pslist_remove_link(QnPtrSlist* p, QnPtrSlist* link)
{
	qn_slist_remove_link(QnPtrSlist, p, link, &p);
	return p;
}

QN_INLINE QnPtrSlist* qn_pslist_copy(const QnPtrSlist* p)
{
	QnPtrSlist* n = NULL;
	qn_slist_copy(QnPtrSlist, p, &n);
	return n;
}

QN_INLINE QnPtrSlist* qn_pslist_contains(QnPtrSlist* p, const void* item)
{
	qn_slist_contains(QnPtrSlist, p, item, &p);
	return p;
}

#define qn_pslist_foreach_1(p,func1)\
	qn_slist_foreach_1(QnPtrSlist, p, func1);

#define qn_pslist_foreach_2(p,func2,data)\
	qn_slist_foreach_2(QnPtrSlist, p, func2, data)


//////////////////////////////////////////////////////////////////////////
// hash

/// @brief 해시 리스트 정의
/// @param name 해시 리스트 이름
/// @param keytype 키 타입
/// @param valuetype 값 타입
#define QN_DECL_HASH(name,keytype,valuetype)\
	typedef keytype name##Key; typedef const keytype name##ConstKey;\
	typedef valuetype name##Value; typedef const valuetype name##ConstValue;\
	typedef struct name##Node { struct name##Node *SIB, *NEXT, *PREV; size_t HASH; name##Key KEY; name##Value VALUE; } name##Node;\
	typedef struct name { size_t REVISION; size_t BUCKET; size_t COUNT; name##Node **NODES, *FIRST, *LAST; } name
QN_DECL_HASH(QnInlineHash, size_t, size_t);

/// @brief 키 해시
#define QN_HASH_HASH(name,fn1)			QN_INLINE size_t name##_Hash(name##ConstKey* key) { return fn1(*key); }
/// @brief 키 비교
#define QN_HASH_EQ(name,fn2)			QN_INLINE bool name##_Equal(name##ConstKey* l, name##ConstKey const* r) { return fn2(*l, *r); }
/// @brief 키 지우기
#define QN_HASH_KEY(name,fn1)			QN_INLINE void name##_Key(name##Key* key) { fn1(*key); }
#define QN_HASH_KEY_FREE(name)			QN_INLINE void name##_Key(name##Key* key) { qn_free(*key); }
#define QN_HASH_KEY_NONE(name)			QN_INLINE void name##_Key(name##Key* key) { QN_DUMMY(key); }
/// @brief 값 지우기
#define QN_HASH_VALUE(name,fn1)			QN_INLINE void name##_Value(name##Value* value) { fn1(*value); }
#define QN_HASH_VALUE_FREE(name)		QN_INLINE void name##_Value(name##Value* value) { qn_free(*value); }
#define QN_HASH_VALUE_NONE(name)		QN_INLINE void name##_Value(name##Value* value) { QN_DUMMY(value); }
/// @brief 정수 키의 해시/비교
#define QN_HASH_INT_KEY(name)			QN_INLINE size_t name##_Hash(name##ConstKey* key) { return (size_t)(*key); }\
										QN_INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return (*k1)==(*k2); }
/// @brief 부호없는 정수 키의 해시/비교
#define QN_HASH_UINT_KEY(name)			QN_INLINE size_t name##_Hash(name##ConstKey* key) { return (size_t)(*key); }\
										QN_INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return *k1==*k2; }
/// @brief size_t 키의 해시/비교
#define QN_HASH_SIZE_T_KEY(name)		QN_INLINE size_t name##_Hash(name##ConstKey* key) { return *key; }\
										QN_INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return (*k1)==(*k2); }
/// @brief char* 키의 해시/비교
#define QN_HASH_CHAR_PTR_KEY(name)		QN_INLINE size_t name##_Hash(name##ConstKey* key) { return qn_strhash(*key); }\
										QN_INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return qn_streqv(*k1, *k2); }
/// @brief wchar* 키의 해시/비교
#define QN_HASH_WCHAR_PTR_KEY(name)		QN_INLINE size_t name##_Hash(name##ConstKey* key) { return qn_wcshash(*key); }\
										QN_INLINE bool name##_Equal(name##ConstKey* k1, name##ConstKey* k2) { return qn_wcseqv(*k1, *k2); }

/// @brief 아이템 갯수
#define qn_hash_count(p)				((p)->COUNT)
/// @brief 해시 버킷수
#define qn_hash_bucket(p)				((p)->BUCKET)
/// @brief 해시 변경치
#define qn_hash_revision(p)				((p)->REVISION)
/// @brief 아이템이 있다
#define qn_hash_is_have(p)				((p)->COUNT>0)
/// @brief 첫 노드
#define qn_hash_node_first(p)			((p)->LAST)
/// @brief 마지막 노드
#define qn_hash_node_last(p)			((p)->FIRST)

/// @brief 해시 초기화
#define qn_hash_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->REVISION=0;\
		(p)->COUNT=0;\
		(p)->BUCKET=QN_MIN_HASH;\
		(p)->NODES=qn_alloc_zero(QN_MIN_HASH, name##Node*);\
		(p)->FIRST=(p)->LAST=NULL;\
	}QN_STMT_END

/// @brief 해시 제거
#define qn_hash_disp(name,p)\
	QN_STMT_BEGIN{\
		for (name##Node* __next, *__node=(p)->FIRST; __node; __node=__next) {\
			__next=__node->NEXT;\
			name##_Key(&__node->KEY);\
			name##_Value(&__node->VALUE);\
			qn_free(__node);\
		}\
		qn_free((p)->NODES);\
	}QN_STMT_END

/// @brief 해시 비우기
#define qn_hash_clear(name,p)\
	qn_inl_hash_erase_all(name,p); qn_inl_hash_test_size(name,p)

/// @brief 노드 제거
#define qn_hash_remove_node(name,p,node)\
	QN_STMT_BEGIN{\
		name##Node** __ppn=&(node);\
		qn_inl_hash_erase_node(name,p,&__ppn);\
		qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

/// @brief 해시 loop each
/// @param func2 loop each 함수 포인터. 인수는(keyptr,valueptr)
#define qn_hash_foreach(name,p,keyptr,valueptr)\
	name##Node* QN_CONCAT(n_,__LINE__);\
	for (QN_CONCAT(n_,__LINE__)=(p)->LAST;\
		QN_CONCAT(n_,__LINE__) && (((keyptr)=&(QN_CONCAT(n_,__LINE__)->KEY), (valueptr)=&(QN_CONCAT(n_,__LINE__)->VALUE)), true);\
		QN_CONCAT(n_,__LINE__)=QN_CONCAT(n_,__LINE__)->PREV)

/// @brief 해시 for each
/// @param func3 for each 함수 포인터. 인수는(data,keyptr,valueptr)
/// @param data for each 함수 포인터 첫 인수
#define qn_hash_foreach_2(name,p,func2)\
	QN_STMT_BEGIN{\
		for (name##Node* __node=(p)->LAST; __node; __node=__node->PREV)\
			func2(&__node->KEY, &__node->VALUE);\
	}QN_STMT_END

/// @brief 해시 for each
/// @param func3 for each 함수 포인터. 인수는(data,keyptr,valueptr)
/// @param data for each 함수 포인터 첫 인수
#define qn_hash_foreach_3(name,p,func3,data)\
	QN_STMT_BEGIN{\
		for (name##Node* __node=(p)->LAST; __node; __node=__node->PREV)\
			func3(data, &__node->KEY, &__node->VALUE);\
	}QN_STMT_END

/// @brief 해시 얻기
/// @param keyptr 키 포인터
/// @param ret_pvalue 값의 포인터. NULL 이면 해당 키에 대한 값이 없는 것
#define qn_hash_get_ptr(name,p,keyptr,ret_pvalue)\
	QN_STMT_BEGIN{\
		name##Node** __gn;\
		qn_inl_hash_lookup(name,p,keyptr,__gn);\
		name##Node* __node=*__gn;\
		*(ret_pvalue)=(__node) ? &__node->VALUE : NULL;\
	}QN_STMT_END

/// @brief 해시 추가 (중복 항목 안 덮어씀)
#define qn_hash_add_ptr(name,p,keyptr,valueptr)\
	qn_inl_hash_set(name,p,keyptr,valueptr,false)

/// @brief 해시 설정 (중복 항목 덮어씀)
#define qn_hash_set_ptr(name,p,keyptr,valueptr)\
	qn_inl_hash_set(name,p,keyptr,valueptr,true)

/// @brief 해시 얻기
/// @param key 키
/// @param ret_pvalue 값의 포인터. NULL 이면 해당 키에 대한 값이 없는 것
#define qn_hash_get(name,p,key,ret_pvalue)\
	QN_STMT_BEGIN{\
		name##ConstKey __k = (name##ConstKey)(key); name##ConstKey* __kp = &__k;\
		qn_hash_get_ptr(name,p,__kp,ret_pvalue);\
	}QN_STMT_END

/// @brief 해시 추가 (중복 항목 안 덮어씀)
#define qn_hash_add(name,p,key,value)\
	QN_STMT_BEGIN{\
		name##Key __key = key; name##Value __value = value;\
		qn_hash_add_ptr(name,p,&__key,&__value);\
	}QN_STMT_END

/// @brief 해시 설정 (중복 항목 덮어씀)
#define qn_hash_set(name,p,key,value)\
	QN_STMT_BEGIN{\
		name##Key __key = key; name##Value __value = value;\
		qn_hash_set_ptr(name,p,&__key,&__value);\
	}QN_STMT_END

/// @brief 해시 삭제
/// @param key 키
/// @param ret_pbool 삭제 결과를 담을 bool 타입 포인터. 필요없으면 NULL
#define qn_hash_remove(name,p,key,ret_pbool)\
	QN_STMT_BEGIN{\
		const name##Key* __kp = (const name##Key*)&(key);\
		qn_inl_hash_erase(name,p,__kp,ret_pbool);\
		qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

/// @brief 해시 검색
/// @param func3 bool (data, keyptr, valueptr) 타입의 검색 함수
/// @param data 검색 함수 첫 항목
/// @param ret_pkey 반환 키 포인터
#define qn_hash_find(name,p,func3,data,ret_pkey)\
	QN_STMT_BEGIN{\
		for (name##Node* __node=(p)->LAST; __node; __node=__node->PREV) {\
			if (func3(data, &__node->KEY, &__node->VALUE)) {\
				(ret_pkey)=&__node->KEY;\
				goto QN_CONCAT(pos_hash_find_exit_,__LINE__);\
			}\
		}\
		(ret_pkey)=NULL;\
		QN_CONCAT(pos_hash_find_exit_,__LINE__):;\
	}QN_STMT_END

/// @brief 해시 룩업
/// @param keyptr 키 포인터
/// @param ret_ppnode 반환값 노드 포인터
#define qn_inl_hash_lookup(name,p,keyptr,ret_ppnode)\
	const size_t __lh=name##_Hash((name##ConstKey*)(keyptr));\
	name##Node *__lnn,**__ln=&(p)->NODES[__lh%(p)->BUCKET];\
	while ((__lnn=*__ln)!=NULL) {\
		if (__lnn->HASH==__lh && name##_Equal((name##ConstKey*)&__lnn->KEY, (name##ConstKey*)(keyptr))) break;\
		__ln=&__lnn->SIB;\
	}\
	(ret_ppnode)=__ln

// 해시 룩업
#define qn_inl_hash_lookup_hash(name,p,keyptr,ret_ppnode,ret_hash)\
	qn_assert((p)->NODES!=NULL && "uninitialized memory");\
	size_t __lh=name##_Hash((name##ConstKey*)(keyptr));\
	name##Node *__lnn, **__ln=&(p)->NODES[__lh%(p)->BUCKET];\
	while ((__lnn=*__ln)!=NULL) {\
		if (__lnn->HASH==__lh && name##_Equal((name##ConstKey*)&__lnn->KEY, (name##ConstKey*)(keyptr))) break;\
		__ln=&__lnn->SIB;\
	}\
	(ret_ppnode)=__ln;\
	(ret_hash)=__lh

// 해시 설정
#define qn_inl_hash_set(name,p,keyptr,valueptr,replace)\
	QN_STMT_BEGIN{\
		size_t __ah; name##Node** __an;\
		qn_inl_hash_lookup_hash(name,p,keyptr,__an,__ah);\
		if (__an) {\
			name##Node* __ann=*__an;\
			if (__ann) {\
				if (replace) {\
					name##_Key(&__ann->KEY);\
					__ann->KEY=*(keyptr);\
					name##_Value(&__ann->VALUE);\
					__ann->VALUE=*(valueptr);\
				} else {\
					name##_Key(keyptr);\
					name##_Value(valueptr);\
				}\
			} else {\
				/* step 1*/\
				__ann=qn_alloc_1(name##Node);\
				if (__ann) {\
					__ann->SIB=NULL;\
					__ann->HASH=__ah;\
					__ann->KEY=*(keyptr);\
					__ann->VALUE=*(valueptr);\
					/* step 2*/\
					if ((p)->FIRST)\
						(p)->FIRST->PREV=__ann;\
					else\
						(p)->LAST=__ann;\
					__ann->NEXT=(p)->FIRST;\
					__ann->PREV=NULL;\
				}\
				(p)->FIRST=__ann;\
				/* step 3 */\
				*__an=__ann;\
				(p)->REVISION++;\
				(p)->COUNT++;\
				/* step 4 */\
				qn_inl_hash_test_size(name,p);\
			}\
		}\
	}QN_STMT_END

// 해시 삭제
#define qn_inl_hash_erase(name,p,keyptr,ret_pbool)\
	QN_STMT_BEGIN{\
		name##Node** __rn;\
		qn_inl_hash_lookup(name,p,keyptr,__rn);\
		if (*__rn==NULL) {\
			if (ret_pbool) {\
				bool* __c=ret_pbool;\
				*__c=false;\
			}\
		} else {\
			qn_inl_hash_erase_node(name,p,&__rn);\
			if (ret_pbool) {\
				bool* __c=ret_pbool;\
				*__c=true;\
			}\
		}\
	}QN_STMT_END

// 해시 노드 삭제
#define qn_inl_hash_erase_node(name,p,pppnode)\
	QN_STMT_BEGIN{\
		name##Node** __en=*(pppnode);\
		name##Node* __enn=*__en;\
		/* step 1 */\
		*__en=__enn->SIB;\
		/* step 2 */\
		if (__enn->NEXT)\
			__enn->NEXT->PREV=__enn->PREV;\
		else {\
			qn_assert((p)->LAST == __enn);\
			(p)->LAST=__enn->PREV;\
		}\
		if (__enn->PREV)\
			__enn->PREV->NEXT=__enn->NEXT;\
		else {\
			qn_assert((p)->FIRST == __enn);\
			(p)->FIRST=__enn->NEXT;\
		}\
		/* step3 */\
		size_t __ebkt=__enn->HASH%(p)->BUCKET;\
		if ((p)->NODES[__ebkt]==__enn) (p)->NODES[__ebkt] = NULL;\
		name##_Key(&__enn->KEY);\
		name##_Value(&__enn->VALUE);\
		qn_free(__enn);\
		(p)->COUNT--;\
		(p)->REVISION++;\
	}QN_STMT_END

// 모두 삭제
#define qn_inl_hash_erase_all(name,p)\
	QN_STMT_BEGIN{\
		for (name##Node *__enx, *__enn=(p)->FIRST; __enn; __enn=__enx) {\
			__enx=__enn->NEXT;\
			name##_Key(&__enn->KEY);\
			name##_Value(&__enn->VALUE);\
			qn_free(__enn);\
		}\
		(p)->FIRST=(p)->LAST=NULL;\
		(p)->COUNT=0;\
		memset((p)->NODES, 0, (p)->BUCKET*sizeof(name##Node*));\
	}QN_STMT_END

// 해시 크기 검사
#define qn_inl_hash_test_size(name,p)\
	QN_STMT_BEGIN{\
		size_t __cnt=(p)->COUNT;\
		size_t __bkt=(p)->BUCKET;\
		if ((__bkt>=3*__cnt && __bkt>QN_MIN_HASH) ||\
			(3*__bkt<=__cnt && __bkt<QN_MAX_HASH))\
			qn_inl_hash_resize((QnInlineHash*)(p));\
	}QN_STMT_END

// 해시 리사이즈
QN_INLINE void qn_inl_hash_resize(QnInlineHash* p)
{
	size_t newbucket = qn_prime_near((uint)p->COUNT);
	newbucket = QN_CLAMP(newbucket, QN_MIN_HASH, QN_MAX_HASH);
	QnInlineHashNode **newnodes = qn_alloc_zero(newbucket, QnInlineHashNode*);
	if (!newnodes)
		return;
	for (size_t i = 0; i < p->BUCKET; i++) {
		for (QnInlineHashNode *node = p->NODES[i], *next; node; node = next) {
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
#define QN_MUKUM_WCHAR_PTR_KEY			QN_HASH_WCHAR_PTR_KEY

/// @brief 아이템 갯수
#define qn_mukum_count(p)				((p)->COUNT)
/// @brief 묶음 버킷수
#define qn_mukum_bucket(p)				((p)->BUCKET)
/// @brief 묶음 변경치
#define qn_mukum_revision(p)			((p)->REVISION)
/// @brief 항목이 있다
#define qn_mukum_is_have(p)				((p)->COUNT>0)

/// @brief 묶음 초기화
#define qn_mukum_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->REVISION=0;\
		(p)->COUNT=0;\
		(p)->BUCKET=QN_MIN_HASH;\
		(p)->NODES=qn_alloc_zero(QN_MIN_HASH, name##Node*);\
	}QN_STMT_END

/// @brief 묶음 제거
#define qn_mukum_disp(name,p)\
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
	qn_inl_mukum_erase_all(name,p); qn_inl_mukum_test_size(name,p)

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
	/*qn_assert((p)->NODES!=NULL && "uninitialized memory");*/\
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
QN_INLINE void qn_inl_mukum_resize(QnInlineMukum* p)
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
// blue string

/// @brief 고정 문자열 정의
/// @param name 고정 문자열 이름
/// @param size 고정 문자열 최대 길이
#define QN_DECL_BSTR(name,size)\
	typedef struct QnBstr##name { size_t LENGTH; char DATA[size]; } QnBstr##name
QN_DECL_BSTR(, );
QN_DECL_BSTR(64, 64);
QN_DECL_BSTR(128, 128);
QN_DECL_BSTR(260, 264);
QN_DECL_BSTR(1k, 1024);
QN_DECL_BSTR(2k, 2048);
QN_DECL_BSTR(4k, 4096);

#define qn_bstr_length(p)				((p)->LENGTH)
#define qn_bstr_data(p)					((p)->DATA)
#define qn_bstr_nth(p,n)				((p)->DATA[(n)])
#define qn_bstr_inv(p,n)				((p)->DATA[((p)->LENGTH)-(n)-1])

#define qn_bstr_test_init(p)\
	qn_assert((p)->LENGTH==0 && (p)->DATA[0]=='\0')

#define qn_bstr_init(p,str)\
	QN_STMT_BEGIN{\
		if ((str)) { (p)->LENGTH=strlen(str); qn_strcpy((p)->DATA, (str)); }\
		else { (p)->LENGTH=0; (p)->DATA[0]='\0'; }\
	}QN_STMT_END

#define qn_bstr_clear(p)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=0; (p)->DATA[0]='\0';\
	}QN_STMT_END

#define qn_bstr_intern(p)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=strlen((p)->DATA);\
	}QN_STMT_END

#define qn_bstr_eq(p1, p2)\
	strcmp((p1)->DATA, (p2)->DATA)==0

#define qn_bstr_set(p, str)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=strlen(str);\
		qn_strcpy((p)->DATA, (str));\
	}QN_STMT_END

#define qn_bstr_set_bstr(p, right)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=(right)->LENGTH;\
		qn_strcpy((p)->DATA, (right)->DATA);\
	}QN_STMT_END

#define qn_bstr_set_len(p, str, len)\
	QN_STMT_BEGIN{\
		if ((len)<0) qn_bstr_set(p,str);\
		else {\
			(p)->LENGTH=len;\
			qn_strncpy((p)->DATA, (str), (len));\
		}\
	}QN_STMT_END

#define qn_bstr_set_char(p, ch)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=1; (p)->DATA[0]=ch; (p)->DATA[1]='\0';\
	}QN_STMT_END

#define qn_bstr_set_rep(p, ch, cnt)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=qn_strfll((p)->DATA, 0, cnt, ch);\
		(p)->DATA[cnt]='\0';\
	}QN_STMT_END

#define qn_bstr_append(p, str)\
	QN_STMT_BEGIN{\
		qn_strcpy(&(p)->DATA[(p)->LENGTH], QN_COUNTOF((p)->DATA)-(p)->LENGTH, str);\
		(p)->LENGTH+=strlen(str);\
	}QN_STMT_END

#define qn_bstr_append_bstr(p, right)\
	QN_STMT_BEGIN{\
		qn_strcpy(&(p)->DATA[(p)->LENGTH], QN_COUNTOF((p)->DATA)-(p)->LENGTH, (right)->DATA);\
		(p)->LENGTH+=(right)->LENGTH;\
	}QN_STMT_END

#define qn_bstr_append_char(p, ch)\
	QN_STMT_BEGIN{\
		(p)->DATA[(p)->LENGTH++]=(char)(ch);\
		(p)->DATA[(p)->LENGTH]='\0';\
	}QN_STMT_END

#define qn_bstr_append_rep(p, ch, cnt)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=qn_strfll((p)->DATA, (p)->LENGTH, (p)->LENGTH+(cnt), ch);\
		(p)->DATA[(p)->LENGTH]='\0';\
	}QN_STMT_END

#define qn_bstr_is_empty(p)\
	((p)->LENGTH==0)

#define qn_bstr_is_have(p)\
	((p)->LENGTH!=0)

#define qn_bstr_hash(p, igcase)\
	((igcase) ? qn_strihash((p)->DATA) : qn_strhash((p)->DATA))

#define qn_bstr_compare(p, s, igcase)\
	((igcase) ? qn_stricmp((p)->DATA, s) : qn_strcmp((p)->DATA, s))

#define qn_bstr_compare_bstr(p1, p2, igcase)\
	((igcase) ? qn_stricmp((p1)->DATA, (p2)->DATA) : qn_strcmp((p1)->DATA, (p2)->DATA))

#define qn_bstr_compare_bstr_length(p1, p2, len, igcase)\
	((igcase) ? qn_strnicmp((p1)->DATA, (p2)->DATA, len) : qn_strncmp((p1)->DATA, (p2)->DATA, len))

#define qn_bstr_format_va(p, fmt, va)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=(size_t)qn_vsnprintf((p)->DATA, QN_COUNTOF((p)->DATA)-1, fmt, va);\
	}QN_STMT_END

#define qn_bstr_format(p, fmt, ...)\
	qn_inl_bstr_format(((QnBstr*)(p)), QN_COUNTOF((p)->DATA)-1, fmt, __VA_ARGS__)
QN_INLINE void qn_inl_bstr_format(QnBstr* p, size_t size, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	p->LENGTH = (size_t)qn_vsnprintf(p->DATA, size, fmt, va);
	va_end(va);
}

#define qn_bstr_append_format_va(p, fmt, va)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=(size_t)qn_vsnprintf(&(p)->DATA[(p)->LENGTH], QN_COUNTOF((p)->DATA)-1-(p)->LENGTH, fmt, va);\
	}QN_STMT_END

#define qn_bstr_append_format(p, fmt, ...)\
	qn_inl_bstr_append_format(((QnBstr*)(p)), QN_COUNTOF((p)->DATA)-1, fmt, __VA_ARGS__)
QN_INLINE void qn_inl_bstr_append_format(QnBstr* p, size_t size, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	p->LENGTH = (size_t)qn_vsnprintf(&p->DATA[p->LENGTH], size - p->LENGTH, fmt, va);
	va_end(va);
}

#define qn_bstr_upper(p)\
	qn_strupr(((QnBstr*)(p))->DATA)

#define qn_bstr_lower(p)\
	qn_strlwr(((QnBstr*)(p))->DATA)

#define qn_bstr_trim(p)\
	QN_STMT_BEGIN{\
		qn_strtrm((p)->DATA);\
		(p)->LENGTH=strlen((p)->DATA);\
	}QN_STMT_END

#define qn_bstr_trim_left(p)\
	QN_STMT_BEGIN{\
		qn_strltm((p)->DATA);\
		(p)->LENGTH=strlen((p)->DATA);\
	}QN_STMT_END

#define qn_bstr_trim_right(p)\
	QN_STMT_BEGIN{\
		qn_strrtm((p)->DATA);\
		(p)->LENGTH=strlen((p)->DATA);\
	}QN_STMT_END

#define qn_bstr_rem_chars(p, rmlist)\
	QN_STMT_BEGIN{\
		(p)->LENGTH=strlen(qn_strrem((p)->DATA, rmlist));\
	}QN_STMT_END

QN_INLINE int qn_bstr_has_char(const void* p, const char* chars)
{
	const char* s = qn_strbrk(((const QnBstr*)(p))->DATA, chars);
	return (s) ? (int)(s - ((const QnBstr*)(p))->DATA) : -1;
}

QN_INLINE int qn_bstr_find_char(const void* p, size_t at, char ch)
{
	const char* s = qn_strchr(((const QnBstr*)(p))->DATA + at, ch);
	return (s) ? (int)(s - ((const QnBstr*)(p))->DATA) : -1;
}

#define qn_bstr_sub_bstr(p, s, pos, len)\
	qn_inl_bstr_sub_bstr(((QnBstr*)(p)), (const QnBstr*)(s), pos, len)
QN_INLINE bool qn_inl_bstr_sub_bstr(QnBstr* p, const QnBstr* s, size_t pos, int len)
{
	qn_val_if_fail(s->LENGTH >= pos, false);

	if (len > 0)
		qn_val_if_fail(s->LENGTH >= (pos + (size_t)len), false);
	else
		len = (int)s->LENGTH - (int)pos;

	qn_strmid(p->DATA, s->DATA, pos, (size_t)len);
	p->LENGTH = (size_t)len;
	return true;
}

QN_EXTC_END

#ifdef _MSC_VER
#pragma warning(pop)
#endif
