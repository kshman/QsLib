//
// QsLib [CONTAINER Layer]
// Made by kim 2004-2024
//
// 이 라이브러리는 연구용입니다. 사용 여부는 사용자 본인의 의사에 달려 있습니다.
// 라이브러리의 일부 또는 전부를 사용자 임의로 전제하거나 사용할 수 있습니다.
//
#pragma once

QN_EXTC_BEGIN

#define __QS_CTN__

//////////////////////////////////////////////////////////////////////////
// common

// 공통
#define qn_ctn_sizeof_type(name)		sizeof(name##Type)
#define qn_ctn_sizeof_node(name)		sizeof(name##Node)


//////////////////////////////////////////////////////////////////////////
// container

/// @brief 고정틱한 배열 정의(컨테이너)
/// @param name 배열 이름
/// @param type 배열 요소 타입
#define QN_DECL_CTNR(name,type)\
	typedef struct name name;\
	typedef type name##Type;\
	struct name { size_t count; type* data; }	// NOLINT
QN_DECL_CTNR(QnPtrCtnr, void*);								/// @brief 포인터 배열
QN_DECL_CTNR(QnByteCtn, byte);								/// @brief 바이트 배열
QN_DECL_CTNR(QnIntCtn, int);								/// @brief 정수 배열
QN_DECL_CTNR(QnAnyCtn, any_t);								/// @brief any_t 배열

#define qn_ctnr_init(name, p, cnt)\
	QN_STMT_BEGIN{\
		if ((size_t)(cnt)>0) {\
			(p)->data=qn_alloc((size_t)(cnt), name##Type);\
			(p)->count=(size_t)(cnt);\
		} else {\
			(p)->data=NULL;\
			(p)->count=0;\
		}\
	}QN_STMT_END
#define qn_ctnr_init_zero(name, p, cnt)\
	QN_STMT_BEGIN{\
		if ((size_t)(cnt)>0) {\
			(p)->data=qn_alloc_zero((size_t)(cnt), name##Type);\
			(p)->count=(size_t)(cnt);\
		} else {\
			(p)->data=NULL;\
			(p)->count=0;\
		}\
	}QN_STMT_END
#define qn_ctnr_init_copy(name, p, from)\
	QN_STMT_BEGIN{\
		(p)->count=(from)->count;\
		(p)->data=qn_memdup((from)->data, qn_ctn_sizeof_type(name) * ((from)->count));\
	}QN_STMT_END
#define qn_ctnr_init_data(p, data, count)\
	QN_STMT_BEGIN{\
		(p)->data=data;\
		(p)->count=count;\
	}QN_STMT_END

#define qn_ctnr_nth(p,n)				(((p)->data)[(size_t)(n)])
#define qn_ctnr_inv(p,n)				(((p)->data)[((p)->count - 1 - (size_t)(n))])
#define qn_ctnr_count(p)				((p)->count)
#define qn_ctnr_data(p)					((p)->data)
#define qn_ctnr_set(p,n,item)			(((p)->data)[(size_t)(n)]=(item))
#define qn_ctnr_is_empty(p)				((p)->count == 0)
#define qn_ctnr_is_have(p)				((p)->count != 0)

#define qn_ctnr_sort(name,p,func)		qn_qsort((p)->data, (p)->count, qn_ctn_sizeof_type(name), func)
#define qn_ctnr_sortc(name,p,func,data)	qn_qsortc((p)->data, (p)->count, qn_ctn_sizeof_type(name), func, data)

#define qn_ctnr_disp(p)					qn_free((p)->data)
#define qn_ctnr_clear(p)				((p)->count=0)
#define qn_ctnr_remove_nth(name,p,n)\
	QN_STMT_BEGIN{\
		qn_assert((size_t)(n)<(p)->count && "index overflow");\
		name##Type* __t=(name##Type*)(p)->data+(n);\
		if ((size_t)(n)!=(p)->count-1)\
			memmove(__t, __t+1, qn_ctn_sizeof_type(name)*((p)->count-((size_t)(n)+1)));\
		(p)->count--;\
	}QN_STMT_END
#define qn_ctnr_remove_range(name,p,index,count)\
	QN_STMT_BEGIN{\
		size_t __end=(size_t)(index)+(size_t)(count);\
		size_t __cnt=(p)->count;\
		if ((size_t)(index)<__cnt && __end<__cnt) {\
			if (__end!=__cnt-1)\
				memmove((p)->data+(index), (p)->data+__end, qn_ctn_sizeof_type(name)*(__cnt-__end));\
			(p)->count-=(size_t)(count);\
		}\
	}QN_STMT_END
#define qn_ctnr_remove(name,p,item)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++) {\
			name##Type* __t = (p)->data+__i;\
			if (*__t==(item)) {\
				memmove(__t, __t+1, qn_ctn_sizeof_type(name)*(__cnt-(__i+1)));\
				__cnt--;\
				break;\
			}\
		}\
		(p)->count=__cnt;\
	}QN_STMT_END
#define qn_ctnr_remove_cmp(name,p,func2,item)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++) {\
			name##Type* __t = (p)->data+__i;\
			if (func2(*__t,item)) {\
				memmove(__t, __t+1, qn_ctn_sizeof_type(name)*(__cnt-(__i+1)));\
				__cnt--;\
				break;\
			}\
		}\
		(p)->count=__cnt;\
	}QN_STMT_END
#define qn_ctnr_remove_cmp_ptr(name,p,func2,item)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++) {\
			name##Type* __t = (p)->data+__i;\
			if (func2(__t,&(item))) {\
				memmove(__t, __t+1, qn_ctn_sizeof_type(name)*(__cnt-(__i+1)));\
				__cnt--;\
				break;\
			}\
		}\
		(p)->count=__cnt;\
	}QN_STMT_END

#define qn_ctnr_contains(p,item,ret_pindex)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(ret_pindex)=-1, __i=0; __i<__cnt; __i++) {\
			if (*((p)->data+__i)==(item))\
			{\
				*(ret_pindex)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_ctnr_find(p,start,func2,data,ret_pindex)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(ret_pindex)=-1, __i=(size_t)(start); __i<__cnt; __i++) {\
			if (func2(data, *((p)->data+__i))) {\
				*(ret_pindex)=__i;\
				break;\
			}\
		}\
	}QN_STMT_END
#define qn_ctnr_find_ptr(p,start,func2,data,ret_pindex)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(ret_pindex)=-1, __i=(size_t)(start); __i<__cnt; __i++) {\
			if (func2(data, (p)->data+__i)) {\
				*(ret_pindex)=__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_ctnr_foreach(p, index)\
	size_t QN_CONCAT(count,__LINE__) = (p)->count;\
	for ((index)=0;(index)<QN_CONCAT(count,__LINE__);(index)++)
#define qn_ctnr_foreach_1(p,func1)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func1(*((p)->data+__i));\
	}QN_STMT_END
#define qn_ctnr_foreach_2(p,func2,data)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func2(data, *((p)->data+__i));\
	}QN_STMT_END
#define qn_ctnr_foreach_ptr_1(p,func1)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func1((p)->data+__i);\
	}QN_STMT_END
#define qn_ctnr_foreach_ptr_2(p,func2,data)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func2(data, (p)->data+__i);\
	}QN_STMT_END

#define qn_ctnr_resize(name, p, cnt)\
	QN_STMT_BEGIN{\
		if ((p)->count!=(size_t)(cnt)) {\
			(p)->data=qn_realloc((p)->data, (size_t)(cnt), name##Type);\
			(p)->count=(size_t)(cnt);\
		}\
	}QN_STMT_END
#define qn_ctnr_expand(name, p, cnt)\
	QN_STMT_BEGIN{\
		if ((size_t)(cnt)!=0) {\
			(p)->data=qn_realloc((p)->data, (p)->count+(size_t)(cnt), name##Type);\
			(p)->count+=(size_t)(cnt);\
		}\
	}QN_STMT_END

#define qn_ctnr_add(name,p,v)\
	QN_STMT_BEGIN{\
		qn_ctnr_expand(name,p,1);\
		qn_ctnr_nth(p,(p)->count-1)=(v);\
	}QN_STMT_END
#define qn_ctnr_insert(name,p,nth,v)\
	QN_STMT_BEGIN{\
		qn_ctnr_expand(name,p,1);\
		memmove((p)->data+(nth)+1, (p)->data+(nth), ((p)->count-1)*sizeof(name##Type));\
		qn_ctnr_nth(p,nth)=(v);\
		/*오류처리 안하고 있음*/\
	}QN_STMT_END

#define qn_pctnr_init(p, count)			qn_ctnr_init(QnPtrCtnr, (QnPtrCtnr*)(p), count)
#define qn_pctnr_init_zero(p, count)	qn_ctnr_init_zero(QnPtrCtnr, (QnPtrCtnr*)(p), count)
#define qn_pctnr_init_copy(p, from)		qn_ctnr_init_copy(QnPtrCtnr, (QnPtrCtnr*)(p), (QnPtrCtnr*)(from))

#define qn_pctnr_nth(p,i)				qn_ctnr_nth(p,i)
#define qn_pctnr_inv(p,i)				qn_ctnr_inv(p,i)
#define qn_pctnr_count(p)				qn_ctnr_count(p)
#define qn_pctnr_data(p)				qn_ctnr_data(p)
#define qn_pctnr_set(p,n,item)			qn_ctnr_set(p,n,item)
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
#define qn_pctnr_remove_cmp(p,func_ptr_item,item)\
										qn_ctnr_remove_cmp(QnPtrArr, p, func_ptr_item, item)

#define qn_pctnr_contains(p, data, ret)	qn_ctnr_contains(p, data, ret)
#define qn_pctnr_find(p, start, func2, userdata, ret)\
										qn_ctnr_find(p, start, func2, userdata, ret)
#define qn_pctnr_find_ptr(p, start, func2, userdata, ret)\
										qn_ctnr_find_ptr(p, start, func2, userdata, ret)

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
	typedef struct name name;\
	typedef type name##Type;\
	struct name { size_t count; size_t capa; type* data; }	// NOLINT
QN_DECL_ARR(QnPtrArr, void*);								/// @brief 포인터 배열
QN_DECL_ARR(QnByteArr, byte);								/// @brief 바이트 배열
QN_DECL_ARR(QnIntArr, int);									/// @brief 정수 배열
QN_DECL_ARR(QnAnyArr, any_t);								/// @brief any_t 배열

#define qn_arr_init(name,p,capacity)\
	QN_STMT_BEGIN{\
		(p)->data=NULL;\
		(p)->count=0;\
		(p)->capa=0;\
		if ((capacity)>0) qn_inl_arr_expand((void*)(p), qn_ctn_sizeof_type(name), (capacity));\
	}QN_STMT_END
#define qn_arr_init_copy(name,p,from)\
	QN_STMT_BEGIN{\
		(p)->count=(from)->count;\
		(p)->capa=(from)->capa;\
		(p)->data=qn_memdup((from)->data, qn_ctn_sizeof_type(name) * ((from)->count));\
	}QN_STMT_END
#define qn_arr_init_data(p, data, count)\
	QN_STMT_BEGIN{\
		(p)->data=data;\
		(p)->count=count;\
		(p)->capa=count;\
	}QN_STMT_END

#define qn_arr_nth(p,n)					qn_ctnr_nth(p,n)
#define qn_arr_inv(p,n)					qn_ctnr_inv(p,n)
#define qn_arr_count(p)					qn_ctnr_count(p)
#define qn_arr_capacity(p)				((p)->capa)
#define qn_arr_data(p)					qn_ctnr_data(p)
#define qn_arr_set(p,n,item)			qn_ctnr_set(p,n,item)
#define qn_arr_is_empty(p)				qn_ctnr_is_empty(p)
#define qn_arr_is_have(p)				qn_ctnr_is_have(p)

#define qn_arr_sort(name,p,func)		qn_ctnr_sort(name,p,func)
#define qn_arr_sortc(name,p,func,data)	qn_ctnr_sortc(name,p,func,data)

#define qn_arr_disp(p)					qn_ctnr_disp(p)
#define qn_arr_clear(p)					qn_ctnr_clear(p)
#define qn_arr_remove_nth(name,p,n)		qn_ctnr_remove_nth(name,p,n)
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
#define qn_arr_foreach_ptr_2(p,f2,data)	qn_ctrn_foreach_ptr_2(p,f2,data)

#define qn_arr_resize(name,p,cnt)		qn_inl_arr_expand((void*)(p), qn_ctn_sizeof_type(name), cnt)
#define qn_arr_expand(name,p,cnt)		qn_inl_arr_expand((void*)(p), qn_ctn_sizeof_type(name), (p)->count+(cnt))

#define qn_arr_add(name,p,item)\
	QN_STMT_BEGIN{\
		qn_arr_expand(name, p, 1);\
		qn_arr_nth(p,(p)->count-1)=(item);\
	}QN_STMT_END
#define qn_arr_add_blob(name,p,items,cnt)\
	QN_STMT_BEGIN{\
		if ((size_t)(cnt)>0) {\
			name##Type* __d=(p)->data+(p)->count;\
			qn_arr_expand(name, p, cnt);\
			memcpy(__d, items, qn_ctn_sizeof_type(name) * (size_t)(cnt));\
		}\
	}QN_STMT_END
#define qn_arr_insert(name,p,idx,item)\
	QN_STMT_BEGIN{\
		if ((size_t)(idx)<=(p)->count) {\
			qn_arr_expand(name p, 1);\
			memmove((p)->data+(size_t)(idx)+1, (p)->data+(size_t)(idx), ((p)->count-1-(size_t)(idx))*qn_ctn_sizeof_type(name));\
			qn_arr_nth(p,idx)=(item);\
		}\
	}QN_STMT_END

QN_INLINE void qn_inl_arr_expand(void* arr, size_t size, size_t count)
{
	QnByteArr* p = (QnByteArr*)arr;
	if (p->capa < count)
	{
		while (p->capa < count)
			p->capa = p->capa + p->capa / 2 + 1;
		p->data = qn_realloc(p->data, p->capa * size, byte);
	}
	p->count = count;
}

#define qn_parr_init(p, capacity)		qn_arr_init(QnPtrArr, p, capacity)
#define qn_parr_init_data(p,data,count)	qn_arr_init_data(p,data,count)

#define qn_parr_nth(p,i)				qn_ctnr_nth(p,i)
#define qn_parr_inv(p,i)				qn_ctnr_inv(p,i)
#define qn_parr_count(p)				qn_ctnr_count(p)
#define qn_parr_capacity(p)				qn_arr_capacity(p)
#define qn_parr_data(p)					qn_ctnr_data(p)
#define qn_parr_set(p,n,item)			qn_ctnr_set(p,n,item)
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
#define qn_parr_remove_cmp(p,func_ptr_item,item)\
										qn_ctnr_remove_cmp(QnPtrArr, p, func_ptr_item, item)

#define qn_parr_contains(p, data, ret)	qn_ctnr_contains(p, data, ret)
#define qn_parr_find(p, start, func2, userdata, ret)\
										qn_ctnr_find(p, start, func2, userdata, ret)
#define qn_parr_find_ptr(p, start, func2, userdata, ret)\
										qn_ctnr_find_ptr(p, start, func2, userdata, ret)

#define qn_parr_foreach(p,index)		qn_ctnr_foreach(p,index)
#define qn_parr_foreach_1(p,func1)		qn_ctnr_foreach_1(p,func1)
#define qn_parr_foreach_2(p,f2,data)	qn_ctnr_foreach_2(p,f2,data)

#define qn_parr_set_count(p,count)		qn_arr_resize(QnPtrArr, p, count)
#define qn_parr_expand(p,count)			qn_arr_expand(QnPtrArr, p, count)

#define qn_parr_add(p, item)			qn_arr_add(QnPtrArr, p, item)
#define qn_parr_add_blob(p,items,cnt)	qn_parr_add_blob(QnPtrArr, p, items, cnt)
#define qn_parr_insert(p, index, item)	qn_arr_insert(QnPtrArr, index, item)


//////////////////////////////////////////////////////////////////////////
// fixed slice

/// @brief 슬라이스(최대 개수를 지정할 수 있는 반-동적 배열) 정의
/// @param name 슬라이스 이름
/// @param type 슬라이스 요소 타입
#define QN_DECL_SLICE(name, type)\
	typedef struct name name;\
	typedef type name##Type;\
	struct name { size_t max; size_t count; type* data; }	// NOLINT

#define qn_slice_test_init(name,p,_max)\
	QN_STMT_BEGIN{\
		qn_assert((p)->data==NULL && (p)->max==0 && (p)->count==0 && "uninitialized memory");\
		(p)->data=qn_alloc(_max, name##Type);\
		(p)->max=_max;\
	}QN_STMT_END
#define qn_slice_init(name,p,_max)\
	QN_STMT_BEGIN{\
		(p)->max=_max;\
		(p)->count=0;\
		(p)->data=qn_alloc((p)->max, name##Type);\
	}QN_STMT_END

#define qn_slice_nth(p,n)				qn_ctnr_nth(p,n)
#define qn_slice_inv(p,n)				qn_ctnr_inv(p,n)
#define qn_slice_count(p)				qn_ctnr_count(p)
#define qn_slice_maximum(p)				((p)->max)
#define qn_slice_data(p)				qn_ctnr_data(p)
#define qn_slice_set(p,n,item)			qn_ctnr_set(p,n,item)
#define qn_slice_is_empty(p)			qn_ctnr_is_empty(p)
#define qn_slice_is_have(p)				qn_ctnr_is_have(p)

#define qn_slice_sort(name,p,func)		qn_ctnr_sort(name,p,func)
#define qn_slice_sortc(name,p,func,data)\
										qn_ctnr_sortc(name,p,func,data)

#define qn_slice_disp(p)				qn_ctnr_disp(p)
#define qn_slice_clear(p)				qn_ctnr_clear(p)
#define qn_slice_disable(name,p)		QN_STMT_BEGIN{ (p)->max=(p)->count; }QN_STMT_END

#define qn_slice_contains(p,item,ret_pindex)\
										qn_ctnr_contains(p,item,ret_pindex)
#define qn_slice_find(p,start,func2,data,ret_pindex)\
										qn_ctnr_find(p,start,func2,data,ret_pindex)
#define qn_slice_find_ptr(p,start,func2,data,ret_pindex)\
										qn_ctnr_find_ptr(p,start,func2,data,ret_pindex)

#define qn_slice_remove_nth(name,p,n)	qn_ctnr_remove_nth(name,p,n)
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
#define qn_slice_foreach_ptr_2(p,f2,data)	qn_ctnr_foreach_ptr_2(p,f2,data)

#define qn_slice_expand(name,p,newmax)\
	QN_STMT_BEGIN{\
		if ((p)->count>(newmax))\
			(p)->count=(newmax);\
		(p)->data=qn_realloc((p)->data, newmax, name##Type);\
		(p)->max=(newmax);\
	}QN_STMT_END
#define qn_slice_push(name,p,itemref,retboolref)\
	QN_STMT_BEGIN{\
		bool* __r=(retboolref);\
		if ((p)->count==(p)->max) {\
			if (__r) *__r=false;\
		} else {\
			if (__r) *__r=true;\
			(p)->data[(p)->count]=*(itemref);\
			(p)->count++;\
		}\
	}QN_STMT_END
#define qn_slice_push_ptr(name,p,item,retboolref)\
	QN_STMT_BEGIN{\
		bool* __r=(retboolref);\
		if ((p)->count==(p)->max) {\
			if (__r) *__r=false;\
		} else {\
			if (__r) *__r=true;\
			(p)->data[(p)->count]=(item);\
			(p)->count++;\
		}\
	}QN_STMT_END
#define qn_slice_pop(name,p,retboolref)\
	QN_STMT_BEGIN{\
		bool* __r=(retboolref);\
		if ((p)->count==0) {\
			if (__r) *__r=false;\
		} else if ((p)->count==1) {\
			if (__r) *__r=true;\
			(p)->count=0;\
		} else {\
			if (__r) *__r=true;\
			(p)->count--;\
			(p)->data[0]=(p)->data[(p)->count];\
		}\
	}QN_STMT_END
#define qn_slice_pop_ptr(name,p,retboolref)\
	qn_slice_pop(name,p,retboolref)
#define qn_slice_pop_value(name,p,retvalueref,retboolref)\
	QN_STMT_BEGIN{\
		bool* __r=(retboolref);\
		if ((p)->count==0) {\
			if (__r) *__r=false;\
		} else if ((p)->count==1) {\
			if (__r) *__r=true;\
			*(retvalueref)=(p)->data[0];\
			(p)->count=0;\
		} else {\
			if (__r) *__r=true;\
			*(retvalueref)=(p)->data[0];\
			(p)->count--;\
			(p)->data[0]=(p)->data[(p)->count];\
		}\
	}QN_STMT_END
#define qn_slice_pop_value_ptr(name,p,retvalueref)\
	QN_STMT_BEGIN{\
		if ((p)->count==0) {\
			*(retvalueref)=NULL;\
		} else if ((p)->count==1) {\
			*(retvalueref)=(p)->data[0];\
			(p)->count=0;\
		} else {\
			*(retvalueref)=(p)->data[0];\
			(p)->count--;\
			(p)->data[0]=(p)->data[(p)->count];\
		}\
	}QN_STMT_END



//////////////////////////////////////////////////////////////////////////
// list

/// @brief 이중 연결 리스트 정의
/// @param name 리스트 이름
/// @param type 리스트 요소 타입
#define QN_DECL_LIST(name,type)\
	typedef struct name name;\
	typedef type name##Type;\
	struct name##Node { struct name##Node *next, *prev; type data; };\
	struct name { struct name##Node *frst, *last; size_t count; }
QN_DECL_LIST(QnPtrList, void*);

#define qn_list_count(p)					((p)->count)
#define qn_list_node_first(p)				((p)->last)
#define qn_list_node_last(p)				((p)->frst)
#define qn_list_data_first(p)				((p)->last->data)
#define qn_list_data_last(p)				((p)->frst->data)
#define qn_list_is_have(p)					((p)->count!=0)
#define qn_list_is_empty(p)					((p)->count==0)

#define qn_list_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->frst=NULL;\
		(p)->last=NULL;\
		(p)->count=0;\
	}QN_STMT_END

#define qn_list_disp(name,p)\
	QN_STMT_BEGIN{\
		struct name##Node *__node, *__next;\
		for (__node=(p)->frst; __node; __node=__next) {\
			__next=__node->next;\
			qn_free(__node);\
		}\
	}QN_STMT_END

#define qn_list_clear(name,p)\
	QN_STMT_BEGIN{\
		qn_list_disp(name,p);\
		(p)->frst=(p)->last=NULL;\
		(p)->count=0;\
	}QN_STMT_END

#define qn_list_remove_node(name,p,node)\
	QN_STMT_BEGIN{\
		struct name##Node* __node=(node);\
		if (__node) {\
			if (__node->next)\
				__node->next->prev=__node->prev;\
			else {\
				qn_assert((p)->last == __node);\
				(p)->last=__node->prev;\
			}\
			if (__node->prev)\
				__node->prev->next=__node->next;\
			else {\
				qn_assert((p)->frst == __node);\
				(p)->frst=__node->next;\
			}\
			(p)->count--;\
			qn_free(__node);\
		}\
	}QN_STMT_END

#define qn_list_remove_first(name,p)\
	qn_list_remove_node(name, p, (p)->last)

#define qn_list_remove_last(name,p)\
	qn_list_remove_node(name, p, (p)->frst)

// value type
#define qn_list_disp_cb(name,p,func_user_data,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node *__node, *__next;\
		for (__node=(p)->frst; __node; __node=__next) {\
			__next=__node->next;\
			func_user_data(userdata,__node->data);\
			qn_free(__node);\
		}\
	}QN_STMT_END

#define qn_list_append(name,p,item)\
	QN_STMT_BEGIN{\
		struct name##Node* __node=qn_alloc_1(struct name##Node);\
		if (__node) {\
			__node->data=item;\
			if ((p)->frst)\
				(p)->frst->prev=__node;\
			else\
				(p)->last=__node;\
			__node->next=(p)->frst;\
			__node->prev=NULL;\
			(p)->frst=__node;\
			(p)->count++;\
		}\
	}QN_STMT_END

#define qn_list_prepend(name,p,item)\
	QN_STMT_BEGIN{\
		struct name##Node* __node=qn_alloc_1(struct name##Node);\
		if (__node) {\
			__node->data=item;\
			if ((p)->last)\
				(p)->last->next=__node;\
			else\
				(p)->frst=__node;\
			__node->prev=(p)->last;\
			__node->next=NULL;\
			(p)->last=__node;\
			(p)->count++;\
		}\
	}QN_STMT_END

#define qn_list_remove(name,p,item)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->frst; __node; __node=__node->next) {\
			if (__node->data==(item)) {\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_remove_cmp(name,p,func_data_item,item)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->frst; __node; __node=__node->next) {\
			if (func_data_pitem(__node->data, item)) {\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_contains(name,p,item,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (*(retnode)=NULL, __node=(p)->frst; __node; __node=__node->next) {\
			if (__node->data==(item)) {\
				*(retnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_find(name,p,predicate_user_data,userdata,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (*(retnode)=NULL, __node=(p)->frst; __node; __node=__node->next) {\
			if (predicate_user_data(userdata, __node->data)) {\
				*(retnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_foreach_1(name,p,func_data)\
	QN_STMT_BEGIN{\
		struct name##Node *__node, *__prev;\
		for (__node=(p)->last; __node; __node=__prev) {\
			__prev = __node->prev; func_data(__node->data); }\
	}QN_STMT_END

#define qn_list_foreach_2(name,p,func2,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node *__node, *__prev;\
		for (__node=(p)->last; __node; __node=__prev) {\
			__prev = __node->prev; func2(userdata, __node->data); }\
	}QN_STMT_END

// reference type
#define qn_list_ref_disp_cb(name,p,func_user_pdata,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node *__node, *__next;\
		for (__node=(p)->frst; __node; __node=__next) {\
			__next=__node->next;\
			func_user_pdata(userdata,&__node->data);\
			qn_free(__node);\
		}\
	}QN_STMT_END

#define qn_list_ref_append(name,p,pitem)\
	QN_STMT_BEGIN{\
		struct name##Node* __node=qn_alloc_1(struct name##Node);\
		if (__node){ memcpy(&__node->data, pitem, sizeof(name##Type));\
			if ((p)->frst)\
				(p)->frst->prev=__node;\
			else\
				(p)->last=__node;\
			__node->next=(p)->frst;\
			__node->prev=NULL;\
			(p)->frst=__node;\
			(p)->count++;\
		}\
	}QN_STMT_END

#define qn_list_ref_prepend(name,p,pitem)\
	QN_STMT_BEGIN{\
		struct name##Node* __node=qn_alloc_1(struct name##Node);\
		if (__node) {\
			memcpy(&__node->data, pitem, sizeof(name##Type));\
			if ((p)->last)\
				(p)->last->next=__node;\
			else\
				(p)->frst=__node;\
			__node->prev=(p)->last;\
			__node->next=NULL;\
			(p)->last=__node;\
			(p)->count++;\
		}\
	}QN_STMT_END

#define qn_list_ref_remove(name,p,pitem)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->frst; __node; __node=__node->next) {\
			if (memcmp(&__node->data, pitem)==0) {\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_ref_remove_cmp(name,p,func_pdata_pitem,pitem)\
	QN_STMT_BEGIN{\
		struct name##Node* __rcnode;\
		for (__rcnode=(p)->frst; __rcnode; __rcnode=__rcnode->next) {\
			if (func_pdata_pitem(&__rcnode->data, pitem)) {\
				qn_list_remove_node(name, p, __rcnode);\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_ref_contains(name,p,pitem,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (*(retnode)=NULL, __node=(p)->frst; __node; __node=__node->next) {\
			if (memcmp(&__node->data, pitem)==0) {\
				*(retnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_ref_find(name,p,predicate_user_pdata,userdata,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (*(retnode)=NULL, __node=(p)->frst; __node; __node=__node->next) {\
			if (predicate_user_pdata(userdata, &__node->data)) {\
				*(retnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_ref_foreach_1(name,p,func_pdata)\
	QN_STMT_BEGIN{\
		struct name##Node *__node, *__prev;\
		for (__node=(p)->last; __node; __node=__prev) {\
			__prev = __node->prev; func_data(&__node->data); }\
	}QN_STMT_END

#define qn_list_ref_foreach_2(name,p,func_user_pdata,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node *__node, *__prev;\
		for (__node=(p)->last; __node; __node=__prev) {\
			__prev = __node->prev; func_user_pdata(userdata, &__node->data); }\
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
#define qn_plist_foreach_1(p,func_data)	qn_list_foreach_1(QnPtrList, p, func_data)
#define qn_plist_foreach_2(p,func_user_data,userdata)\
										qn_list_foreach_2(QnPtrList, p, func_user_data, userdata)

QN_INLINE bool qn_plist_contains(const QnPtrList* p, const void* item)
{
	const struct QnPtrListNode* node = NULL;
	qn_list_contains(QnPtrList, p, item, &node);
	return node != NULL;
}

QN_INLINE bool qn_plist_find(const QnPtrList* p, bool (*pred)(void*, void*), void* userdata)
{
	const struct QnPtrListNode* node = NULL;
	qn_list_find(QnPtrList, p, pred, userdata, &node);
	return node != NULL;
}


//////////////////////////////////////////////////////////////////////////
// node list

/// @brief 노드 리스트 정의
/// @param name 노드 리스트 이름
/// @param type 노드 리스트 요소 타입
#define QN_DECL_NODELIST(name,type)\
	typedef struct name name;\
	typedef type name##Type;\
	struct name { name##Type* frst; name##Type* last; size_t count; }	// NOLINT

#define qn_nodelist_count(p)			((p)->count)
#define qn_nodelist_first(p)			((p)->last)
#define qn_nodelist_last(p)				((p)->frst)
#define qn_nodelist_is_have(p)			((p)->count!=0)
#define qn_nodelist_is_empty(p)			((p)->count==0)

#define qn_nodelist_init(p)\
	QN_STMT_BEGIN{\
		(p)->frst=NULL;\
		(p)->last=NULL;\
		(p)->count=0;\
	}QN_STMT_END

#define qn_nodelist_disp(name,p)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__next;\
		for (__node=(p)->frst; __node; __node=__next) {\
			__next=__node->next;\
			qn_free(__node);\
		}\
	}QN_STMT_END

#define qn_nodelist_disp_cb(name,p,func1)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__next;\
		for (__node=(p)->frst; __node; __node=__next) {\
			__next=__node->next;\
			func1(__node);\
		}\
	}QN_STMT_END

#define qn_nodelist_append(name,p,item)\
	QN_STMT_BEGIN{\
		if ((p)->frst)\
			(p)->frst->prev=(item);\
		else\
			(p)->last=(item);\
		(item)->next=(p)->frst;\
		(item)->prev=NULL;\
		(p)->frst=(item);\
		(p)->count++;\
	}QN_STMT_END

#define qn_nodelist_prepend(name,p,item)\
	QN_STMT_BEGIN{\
		if ((p)->last)\
			(p)->last->next=(item);\
		else\
			(p)->frst=(item);\
		(item)->prev=(p)->last;\
		(item)->next=NULL;\
		(p)->last=(item);\
		(p)->count++;\
	}QN_STMT_END

#define qn_nodelist_clear(name,p)\
	QN_STMT_BEGIN{\
		qn_nodelist_disp(name,p);\
		(p)->frst=(p)->last=NULL;\
		(p)->count=0;\
	}QN_STMT_END

#define qn_nodelist_clear_cb(name,p,func_1)\
	QN_STMT_BEGIN{\
		qn_nodelist_disp_cb(name,p,func_1);\
		(p)->frst=(p)->last=NULL;\
		(p)->count=0;\
	}QN_STMT_END

#define qn_nodelist_remove(name,p,item)\
	QN_STMT_BEGIN{\
		if (item) {\
			name##Type* __node=(name##Type*)(item);\
			if (__node->next)\
				__node->next->prev=__node->prev;\
			else {\
				qn_assert((p)->last == __node);\
				(p)->last=__node->prev;\
			}\
			if (__node->prev)\
				__node->prev->next=__node->next;\
			else {\
				qn_assert((p)->frst == __node);\
				(p)->frst=__node->next;\
			}\
			(p)->count--;\
		}\
	}QN_STMT_END

#define qn_nodelist_remove_cb(name,p,item,func_1)\
	QN_STMT_BEGIN{\
		if (item) {\
			name##Type* __node=(name##Type*)(item);\
			if (__node->next)\
				__node->next->prev=__node->prev;\
			else {\
				qn_assert((p)->last == __node);\
				(p)->last=__node->prev;\
			}\
			if (__node->prev)\
				__node->prev->next=__node->next;\
			else {\
				qn_assert((p)->frst == __node);\
				(p)->frst=__node->next;\
			}\
			(p)->count--;\
			func_1(__node);\
		}\
	}QN_STMT_END

#define qn_nodelist_remove_first(name,p)\
	qn_nodelist_remove(name, p, (p)->last)

#define qn_nodelist_remove_first_cb(name,p,func_1)\
	qn_nodelist_remove_cb(name, p, (p)->last,func_1)

#define qn_nodelist_remove_last(name,p)\
	qn_nodelist_remove(name, p, (p)->frst)

#define qn_nodelist_remove_last_cb(name,p,func_1)\
	qn_nodelist_remove_cb(name, p, (p)->frst,func_1)

#define qn_nodelist_unlink(name,p,item)\
	QN_STMT_BEGIN{\
		if (item) {\
			name##Type* __node=(name##Type*)(item);\
			if (__node->next)\
				__node->next->prev=__node->prev;\
			else {\
				qn_assert((p)->last == __node);\
				(p)->last=__node->prev;\
			}\
			if (__node->prev)\
				__node->prev->next=__node->next;\
			else {\
				qn_assert((p)->frst == __node);\
				(p)->frst=__node->next;\
			}\
			(p)->count--;\
			__node->prev=__node->next=NULL;\
		}\
	}QN_STMT_END

#define qn_nodelist_contains(name,p,item,retbool)\
	QN_STMT_BEGIN{\
		name##Type* __node;\
		for (*(retbool)=false, __node=(p)->frst; __node; __node=__node->next) {\
			if (__node==(item)) {\
				*(retbool)=true;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_nodelist_find(name,p,predicate_user_node,userdata,retpnode)\
	QN_STMT_BEGIN{\
		name##Type* __node;\
		for (*(retpnode)=NULL, __node=(p)->last; __node; __node=__node->prev) {\
			if (predicate_user_node(userdata,__node)) {\
				*(retpnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END


#define qn_nodelist_foreach(name,p,node)\
	name##Type *QN_CONCAT(prev,__LINE__);\
	for (node=(p)->last; node && (QN_CONCAT(prev,__LINE__) = node->prev, true); node=QN_CONCAT(prev,__LINE__))
#define qn_nodelist_foreach_1(name,p,func_node)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__prev;\
		for (__node=(p)->last; __node; __node=__prev) {\
			__prev=__node->prev;\
			func_node(__node);\
		}\
	}QN_STMT_END
#define qn_nodelist_foreach_2(name,p,func_user_node,userdata)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__prev;\
		for (__node=(p)->last; __node; __node=__prev) {\
			__prev=__node->prev;\
			func_user_node(userdata,__node);\
		}\
	}QN_STMT_END


//////////////////////////////////////////////////////////////////////////
// solo list

/// @brief 단일 리스트 정의
/// @param name 단일 리스트 이름
/// @param type 단일 리스트 요소 타입
#define QN_DECL_SLIST(name,type)\
	typedef struct name name, name##Name;\
	typedef type name##Type;\
	struct name { struct name* next; type data; }
QN_DECL_SLIST(QnPtrSlist, void*);

#define qn_slist_last(p)				qn_inl_slist_last(p)
#define qn_slist_nth(p,nth)				qn_inl_slist_nth(p, nth)
#define qn_slist_data(p,nth)			((qn_inl_slist_nth(p, nth))->data)
#define qn_slist_count(p)				qn_inl_slist_count(p)

#define qn_slist_delete(name, p, ptrptr)\
	QN_STMT_BEGIN{\
		if (!(p))\
			*(ptrptr)=NULL;\
		else {\
			name##Name* __n=(p)->next;\
			qn_free(p);\
			*(ptrptr)=__n;\
		}\
	}QN_STMT_END

#define qn_slist_disp(name, p)\
	QN_STMT_BEGIN{\
		name##Name *__n, *__p;\
		for (__p=(p); __p; __p=__n) {\
			__n=__p->next;\
			qn_free(__p);\
		}\
	}QN_STMT_END

#define qn_slist_disp_func(name, p, func_1)\
	QN_STMT_BEGIN{\
		name##Name *__n, *__p;\
		for (__p=(p); __p; __p=__n) {\
			__n=__p->next;\
			func_1(__p);\
		}\
	}QN_STMT_END

#define qn_slist_concat(name, p1, p2, ptrptr)\
	QN_STMT_BEGIN{\
		if (!(p2))\
			*(ptrptr)=(p1);\
		else {\
			if (!(p1))\
				*(ptrptr)=(p2);\
			else {\
				name##Name* __l=(name##Name*)qn_slist_last(p1);\
				__l->next=(p2);\
				*(ptrptr)=(p1);\
			}\
		}\
	}QN_STMT_END

#define qn_slist_append(name, p, item, ptrptr)\
	QN_STMT_BEGIN{\
		name##Name* __np=qn_alloc_1(name##Name);\
		if (__np) {\
			__np->next=NULL;\
			__np->data=item;\
			if (!(p))\
				*(ptrptr)=__np;\
			else {\
				name##Name* __l=(name##Name*)qn_slist_last(p);\
				__l->next=__np;\
				*(ptrptr)=(p);\
			}\
		}\
	}QN_STMT_END

#define qn_slist_prepend(name, p, item, ptrptr)\
	QN_STMT_BEGIN{\
		name##Name* __np=qn_alloc_1(name##Name);\
		if (__np) {\
			__np->next=(p);\
			__np->data=item;\
			*(ptrptr)=__np;\
		}\
	}QN_STMT_END

#define qn_slist_reserve_append(name, p, ptrptr)\
	QN_STMT_BEGIN{\
		name##Name* __np=qn_alloc_zero_1(name##Name);\
		if (!(p))\
			*(ptrptr)=__np;\
		else {\
			name##Name* __l=(name##Name*)qn_slist_last(p);\
			__l->next=__np;\
			*(ptrptr)=(p);\
		}\
	}QN_STMT_END

#define qn_slist_reserve_prepend(name, p, ptrptr)\
	QN_STMT_BEGIN{\
		name##Name* __np=qn_alloc_zero_1(name##Name);\
		if (__np) __np->next=(p);\
		*(ptrptr)=__np;\
	}QN_STMT_END

#define qn_slist_insert(name, p, sib, item, ptrptr)\
	QN_STMT_BEGIN{\
		if (!(p)) {\
			name##Name* __np=qn_alloc_zero_1(name##Name);\
			if (__np) {\
				__np->next=NULL;\
				__np->data=item;\
			}\
			*(ptrptr)=__np;\
		} else {\
			name##Name *__np, *__l=NULL;\
			for (__np=(p); __np; __l=__np, __np=__l->next)\
			{\
				if (__np==(sib))\
					break;\
			}\
			__np=qn_alloc_1(name##Name);\
			if (__np) {\
				__np->data=item;\
				if (!__l) {\
					__np->next=(p);\
					*(ptrptr)=__np;\
				} else {\
					__np->next=__l->next;\
					__l->next=__np;\
					*(ptrptr)=(p);\
				}\
			}\
		}\
	}QN_STMT_END

#define qn_slist_insert_nth(name, p, nth, item, ptrptr)\
	QN_STMT_BEGIN{\
		int __h=nth;\
		if (__h<0) {\
			qn_slist_append(name, p, item, ptrptr);\
		} else if (__h==0) {\
			qn_slist_prepend(name, p, item, ptrptr);\
		} else {\
			name##Name* __np=qn_alloc_1(name##Name);\
			if (__np) {\
				__np->data=item;\
				if (!(p)) {\
					__np->next=NULL;\
					*(ptrptr)=__np;\
				} else {\
					name##Name *__v=NULL, *__t=(p);\
					while (__h-->0 && __t) {\
						__v=__t;\
						__t=__t->next;\
					}\
					if (__v) {\
						__np->next=__v->next;\
						__v->next=__np;\
						*(ptrptr)=(p);\
					} else {\
						__np->next=(p);\
						*(ptrptr)=__np;\
					}\
				}\
			}\
		}\
	}QN_STMT_END

#define qn_slist_remove(name, p, item, ptrptr)\
	QN_STMT_BEGIN{\
		name##Name *__r=(p), *__t=(p), *__v=NULL;\
		while (__t) {\
			if (__t->data==(item)) {\
				if (__v)\
					__v->next=__t->next;\
				else\
					__r=__t->next;\
				qn_free(__t);\
				break;\
			}\
			__v=__t;\
			__t=__t->next;\
		}\
		*(ptrptr)=__r;\
	}QN_STMT_END

#define qn_slist_remove_link(name, p, link, ptrptr)\
	QN_STMT_BEGIN{\
		name##Name *__r=(p), *__t=(p), *__v=NULL;\
		while (__t) {\
			if (__t==(link)) {\
				if (__v)\
					__v->next=__t->next;\
				if (__r==__t)\
					__r=__r->next;\
				__t->next=NULL;\
				break;\
			}\
			__v=__t;\
			__t=__t->next;\
		}\
		qn_free(link);\
		*(ptrptr)=__r;\
	}QN_STMT_END

#define qn_slist_copy(name, p, ptrptr)\
	QN_STMT_BEGIN{\
		if (!(p))\
			*(ptrptr)=NULL;\
		else {\
			name##Name *__p, *__l, *__n=qn_alloc_zero_1(name##Name);\
			if (!__n)\
				*(ptrptr)=NULL;\
			else {\
				__n->data=(p)->data;\
				__l=__n;\
				__p=(p)->next;\
				while (__p) {\
					__l->next=qn_alloc_1(name##Name);\
					if (!__l->next) break;\
					__l=__l->next;\
					__l->data=__p->data;\
					__p=__p->next;\
				}\
				if (__l) __l->next=NULL;\
				*(ptrptr)=__n;\
			}\
		}\
	}QN_STMT_END

#define qn_slist_contains(name,p,item,ptrptr)\
	QN_STMT_BEGIN{\
		name##Name* __p=(p);\
		while (__p) {\
			if (__p->data==(item))\
				break;\
			__p=__p->next;\
		}\
		*(ptrptr)=__p;\
	}QN_STMT_END

#define qn_slist_contains_func(name,p,func2,item,ptrptr)\
	QN_STMT_BEGIN{\
		name##Name* __p=(p);\
		while (__p) {\
			if (func2(__p->data, item))\
				break;\
			__p=__p->next;\
		}\
		*(ptrptr)=__p;\
	}QN_STMT_END

#define qn_slist_foreach_1(name,p,func_1)\
	QN_STMT_BEGIN{\
		name##Name* __p=(p);\
		while (__p) {\
			name##Name* __n=__p->next;\
			func_1(__p->data);\
			__p=__n;\
		}\
	}QN_STMT_END

#define qn_slist_foreach_2(name,p,func_2,userdata)\
	QN_STMT_BEGIN{\
		name##Name* __p=(p);\
		while (__p) {\
			name##Name* __n=__p->next;\
			func_2(userdata, __p->data);\
			__p=__n;\
		}\
	}QN_STMT_END

QN_INLINE void* qn_inl_slist_last(void* ptr)
{
	QnPtrSlist* p = (QnPtrSlist*)ptr;
	while (p->next)
		p = p->next;
	return p;
}

QN_INLINE void* qn_inl_slist_nth(void* ptr, size_t nth)
{
	QnPtrSlist* p = (QnPtrSlist*)ptr;
	while (nth-- > 0 && p)
		p = p->next;
	return (void*)p;
}

QN_INLINE size_t qn_inl_slist_count(void* ptr)
{
	const QnPtrSlist* p = (QnPtrSlist*)ptr;
	size_t n = 0;
	while (p) {
		n++;
		p = p->next;
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

QN_INLINE void qn_pslist_disp(QnPtrSlist* p, void (*func_1)(void*))
{
	if (!func_1) {
		qn_slist_disp(QnPtrSlist, p);
	}
	else {
		for (QnPtrSlist* n; p; p = n) {
			n = p->next;
			if (func_1)
				func_1(p->data);
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

#define qn_pslist_foreach_1(p,func_1)\
	qn_slist_foreach_1(QnPtrSlist, p, func_1);

#define qn_pslist_foreach_2(p,func_2,userdata)\
	qn_slist_foreach_2(QnPtrSlist, p, func_2, userdata)


//////////////////////////////////////////////////////////////////////////
// hash

/// @brief 해시 리스트 정의
/// @param name 해시 리스트 이름
/// @param keytype 키 타입
/// @param valuetype 값 타입
#define QN_DECL_HASH(name,keytype,valuetype)\
	typedef struct name name;\
	typedef keytype name##Key; typedef const keytype name##ConstKey;\
	typedef valuetype name##Value; typedef const valuetype name##ConstValue;\
	struct name##Node { struct name##Node *sib, *next, *prev; size_t hash; name##Key key; name##Value value; };\
	struct name { size_t revision; size_t bucket; size_t count; struct name##Node **nodes, *frst, *last; }
QN_DECL_HASH(QnInlineHash, size_t, size_t);

/// @brief 키 해시
#define QN_HASH_HASH(name,fn1)			QN_INLINE size_t name##_hash(name##ConstKey* key) { return fn1(*key); }
/// @brief 키 비교
#define QN_HASH_EQ(name,fn2)			QN_INLINE bool name##_eq(name##ConstKey* l, name##ConstKey const* r) { return fn2(*l, *r); }
/// @brief 키 지우기
#define QN_HASH_KEY(name,fn1)			QN_INLINE void name##_key(name##Key* key) { fn1(*key); }
#define QN_HASH_KEY_FREE(name)			QN_INLINE void name##_key(name##Key* key) { qn_free(*key); }
#define QN_HASH_KEY_NONE(name)			QN_INLINE void name##_key(name##Key* key) { QN_DUMMY(key); }
/// @brief 값 지우기
#define QN_HASH_VALUE(name,fn1)			QN_INLINE void name##_value(name##Value* value) { fn1(*value); }
#define QN_HASH_VALUE_FREE(name)		QN_INLINE void name##_value(name##Value* value) { qn_free(*value); }
#define QN_HASH_VALUE_NONE(name)		QN_INLINE void name##_value(name##Value* value) { QN_DUMMY(value); }
/// @brief 정수 키의 해시/비교
#define QN_HASH_INT_KEY(name)			QN_INLINE size_t name##_hash(name##ConstKey* key) { return (size_t)(*key); }\
										QN_INLINE bool name##_eq(name##ConstKey* k1, name##ConstKey* k2) { return (*k1)==(*k2); }
/// @brief 부호없는 정수 키의 해시/비교
#define QN_HASH_UINT_KEY(name)			QN_INLINE size_t name##_hash(name##ConstKey* key) { return (size_t)(*key); }\
										QN_INLINE bool name##_eq(name##ConstKey* k1, name##ConstKey* k2) { return *k1==*k2; }
/// @brief size_t 키의 해시/비교
#define QN_HASH_SIZE_T_KEY(name)		QN_INLINE size_t name##_hash(name##ConstKey* key) { return *key; }\
										QN_INLINE bool name##_eq(name##ConstKey* k1, name##ConstKey* k2) { return (*k1)==(*k2); }
/// @brief char* 키의 해시/비교
#define QN_HASH_CHAR_PTR_KEY(name)		QN_INLINE size_t name##_hash(name##ConstKey* key) { return qn_strhash(*key); }\
										QN_INLINE bool name##_eq(name##ConstKey* k1, name##ConstKey* k2) { return qn_streqv(*k1, *k2); }
/// @brief wchar* 키의 해시/비교
#define QN_HASH_WCHAR_PTR_KEY(name)		QN_INLINE size_t name##_hash(name##ConstKey* key) { return qn_wcshash(*key); }\
										QN_INLINE bool name##_eq(name##ConstKey* k1, name##ConstKey* k2) { return qn_wcseqv(*k1, *k2); }

/// @brief 아이템 갯수
#define qn_hash_count(p)				((p)->count)
/// @brief 해시 버킷수
#define qn_hash_bucket(p)				((p)->bucket)
/// @brief 해시 변경치
#define qn_hash_revision(p)				((p)->revision)
/// @brief 아이템이 있다
#define qn_hash_is_have(p)				((p)->count>0)
/// @brief 첫 노드
#define qn_hash_node_first(p)			((p)->last)
/// @brief 마지막 노드
#define qn_hash_node_last(p)			((p)->frst)

/// @brief 해시 초기화
#define qn_hash_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->revision=0;\
		(p)->count=0;\
		(p)->bucket=QN_MIN_HASH;\
		(p)->nodes=qn_alloc_zero(QN_MIN_HASH, struct name##Node*);\
		(p)->frst=(p)->last=NULL;\
	}QN_STMT_END

/// @brief 해시 제거
#define qn_hash_disp(name,p)\
	QN_STMT_BEGIN{\
		for (struct name##Node* __next, *__node=(p)->frst; __node; __node=__next) {\
			__next=__node->next;\
			name##_key(&__node->key);\
			name##_value(&__node->value);\
			qn_free(__node);\
		}\
		qn_free((p)->nodes);\
	}QN_STMT_END

/// @brief 해시 비우기
#define qn_hash_clear(name,p)\
	qn_inl_hash_erase_all(name,p); qn_inl_hash_test_size(name,p)

/// @brief 노드 제거
#define qn_hash_remove_node(name,p,node)\
	QN_STMT_BEGIN{\
		struct name##Node** __ppn=&(node);\
		qn_inl_hash_erase_node(name,p,&__ppn);\
		qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

/// @brief 해시 loop each
/// @param func2 loop each 함수 포인터. 인수는(keyptr,valueptr)
#define qn_hash_foreach(name,p,keyptr,valueptr)\
	struct name##Node* QN_CONCAT(n_,__LINE__);\
	for (QN_CONCAT(n_,__LINE__)=(p)->last;\
		QN_CONCAT(n_,__LINE__) && ((keyptr=&(QN_CONCAT(n_,__LINE__)->key), valueptr=&(QN_CONCAT(n_,__LINE__)->value)) || true);\
		QN_CONCAT(n_,__LINE__)=QN_CONCAT(n_,__LINE__)->prev)

/// @brief 해시 for each
/// @param func3 for each 함수 포인터. 인수는(data,keyptr,valueptr)
/// @param data for each 함수 포인터 첫 인수
#define qn_hash_foreach_2(name,p,func2)\
	QN_STMT_BEGIN{\
		for (struct name##Node* __node=(p)->last; __node; __node=__node->prev)\
			func2(&__node->key, &__node->value);\
	}QN_STMT_END

/// @brief 해시 for each
/// @param func3 for each 함수 포인터. 인수는(data,keyptr,valueptr)
/// @param data for each 함수 포인터 첫 인수
#define qn_hash_foreach_3(name,p,func3,data)\
	QN_STMT_BEGIN{\
		for (struct name##Node* __node=(p)->last; __node; __node=__node->prev)\
			func3(data, &__node->key, &__node->value);\
	}QN_STMT_END

/// @brief 해시 얻기
/// @param keyptr 키 포인터
/// @param retval 값의 포인터. NULL 이면 해당 키에 대한 값이 없는 것
#define qn_hash_get_ptr(name,p,keyptr,retval)\
	QN_STMT_BEGIN{\
		struct name##Node** __gn;\
		qn_inl_hash_lookup(name,p,keyptr,__gn);\
		struct name##Node* __node=*__gn;\
		*(retval)=(__node) ? &__node->value : NULL;\
	}QN_STMT_END

/// @brief 해시 추가 (중복 항목 안 덮어씀)
#define qn_hash_add_ptr(name,p,keyptr,valueptr)\
	qn_inl_hash_set(name,p,keyptr,valueptr,false)

/// @brief 해시 설정 (중복 항목 덮어씀)
#define qn_hash_set_ptr(name,p,keyptr,valueptr)\
	qn_inl_hash_set(name,p,keyptr,valueptr,true)

/// @brief 해시 얻기
/// @param key 키
/// @param retval 값의 포인터. NULL 이면 해당 키에 대한 값이 없는 것
#define qn_hash_get(name,p,key,retval)\
	QN_STMT_BEGIN{\
		name##ConstKey __k = (name##ConstKey)(key); name##ConstKey* __kp = &__k;\
		qn_hash_get_ptr(name,p,__kp,retval);\
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
/// @param ret_bool_ptr 삭제 결과를 담을 bool 타입 포인터. 필요없으면 NULL
#define qn_hash_remove(name,p,key,ret_bool_ptr)\
	QN_STMT_BEGIN{\
		const name##Key* __kp = (const name##Key*)&(key);\
		qn_inl_hash_erase(name,p,__kp,ret_bool_ptr);\
		qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

/// @brief 해시 검색
/// @param func3 bool (data, keyptr, valueptr) 타입의 검색 함수
/// @param data 검색 함수 첫 항목
/// @param ret_key_ptr 반환 키 포인터
#define qn_hash_find(name,p,func3,data,ret_key_ptr)\
	QN_STMT_BEGIN{\
		for (struct name##Node* __node=(p)->last; __node; __node=__node->prev) {\
			if (func3(data, &__node->key, &__node->value)) {\
				(ret_key_ptr)=&__node->key;\
				goto QN_CONCAT(pos_hash_find_exit_,__LINE__);\
			}\
		}\
		(ret_key_ptr)=NULL;\
		QN_CONCAT(pos_hash_find_exit_,__LINE__):;\
	}QN_STMT_END

/// @brief 해시 룩업
/// @param keyptr 키 포인터
/// @param ret_node 반환값 노드 포인터
#define qn_inl_hash_lookup(name,p,keyptr,ret_node)\
	const size_t __lh=name##_hash((name##ConstKey*)(keyptr));\
	struct name##Node *__lnn,**__ln=&(p)->nodes[__lh%(p)->bucket];\
	while ((__lnn=*__ln)!=NULL) {\
		if (__lnn->hash==__lh && name##_eq((name##ConstKey*)&__lnn->key, (name##ConstKey*)(keyptr))) break;\
		__ln=&__lnn->sib;\
	}\
	(ret_node)=__ln

/// @brief 해시 룩업
/// @param keyptr 키 포인터
/// @param ret_node 반환값 노드 포인터
/// @param ret_hash 반환값 해시
#define qn_inl_hash_lookup_hash(name,p,keyptr,ret_node,ret_hash)\
	qn_assert((p)->nodes!=NULL && "uninitialized memory");\
	size_t __lh=name##_hash((name##ConstKey*)(keyptr));\
	struct name##Node *__lnn, **__ln=&(p)->nodes[__lh%(p)->bucket];\
	while ((__lnn=*__ln)!=NULL) {\
		if (__lnn->hash==__lh && name##_eq((name##ConstKey*)&__lnn->key, (name##ConstKey*)(keyptr))) break;\
		__ln=&__lnn->sib;\
	}\
	(ret_node)=__ln;\
	(ret_hash)=__lh

/// @brief 해시 설정
/// @param keyptr 키 포인터
/// @param valueptr 값 포인터
/// @param replace 참=원래 키/값을 덮어씀, 거짓=주어진 키/값 삭제
#define qn_inl_hash_set(name,p,keyptr,valueptr,replace)\
	QN_STMT_BEGIN{\
		size_t __ah; struct name##Node** __an;\
		qn_inl_hash_lookup_hash(name,p,keyptr,__an,__ah);\
		if (__an) {\
			struct name##Node* __ann=*__an;\
			if (__ann) {\
				if (replace) {\
					name##_key(&__ann->key);\
					__ann->key=*(keyptr);\
					name##_value(&__ann->value);\
					__ann->value=*(valueptr);\
				} else {\
					name##_key(keyptr);\
					name##_value(valueptr);\
				}\
			} else {\
				/* step 1*/\
				__ann=qn_alloc_1(struct name##Node);\
				if (__ann) {\
					__ann->sib=NULL;\
					__ann->hash=__ah;\
					__ann->key=*(keyptr);\
					__ann->value=*(valueptr);\
					/* step 2*/\
					if ((p)->frst)\
						(p)->frst->prev=__ann;\
					else\
						(p)->last=__ann;\
					__ann->next=(p)->frst;\
					__ann->prev=NULL;\
				}\
				(p)->frst=__ann;\
				/* step 3 */\
				*__an=__ann;\
				(p)->revision++;\
				(p)->count++;\
				/* step 4 */\
				qn_inl_hash_test_size(name,p);\
			}\
		}\
	}QN_STMT_END

/// @brief 해시 삭제
/// @param keyptr 키 포인터
/// @param ret_bool_ptr 결과를 담을 bool 타입 포인터
#define qn_inl_hash_erase(name,p,keyptr,ret_bool_ptr)\
	QN_STMT_BEGIN{\
		struct name##Node** __rn;\
		qn_inl_hash_lookup(name,p,keyptr,__rn);\
		if (*__rn==NULL) {\
			if (ret_bool_ptr) {\
				bool* __c=ret_bool_ptr;\
				*__c=false;\
			}\
		} else {\
			qn_inl_hash_erase_node(name,p,&__rn);\
			if (ret_bool_ptr) {\
				bool* __c=ret_bool_ptr;\
				*__c=true;\
			}\
		}\
	}QN_STMT_END

/// @brief 해시 노드 삭제
/// @param pppnode 노드의 포인터의 포인터의 포인터
#define qn_inl_hash_erase_node(name,p,pppnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=*(pppnode);\
		struct name##Node* __enn=*__en;\
		/* step 1 */\
		*__en=__enn->sib;\
		/* step 2 */\
		if (__enn->next)\
			__enn->next->prev=__enn->prev;\
		else {\
			qn_assert((p)->last == __enn);\
			(p)->last=__enn->prev;\
		}\
		if (__enn->prev)\
			__enn->prev->next=__enn->next;\
		else {\
			qn_assert((p)->frst == __enn);\
			(p)->frst=__enn->next;\
		}\
		/* step3 */\
		size_t __ebkt=__enn->hash%(p)->bucket;\
		if ((p)->nodes[__ebkt]==__enn) (p)->nodes[__ebkt] = NULL;\
		name##_key(&__enn->key);\
		name##_value(&__enn->value);\
		qn_free(__enn);\
		(p)->count--;\
		(p)->revision++;\
	}QN_STMT_END

/// @brief 모두 삭제
#define qn_inl_hash_erase_all(name,p)\
	QN_STMT_BEGIN{\
		for (struct name##Node *__enx, *__enn=(p)->frst; __enn; __enn=__enx) {\
			__enx=__enn->next;\
			name##_key(&__enn->key);\
			name##_value(&__enn->value);\
			qn_free(__enn);\
		}\
		(p)->frst=(p)->last=NULL;\
		(p)->count=0;\
		memset((p)->nodes, 0, (p)->bucket*sizeof(struct name##Node*));\
	}QN_STMT_END

/// @brief 해시 크기 검사
#define qn_inl_hash_test_size(name,p)\
	QN_STMT_BEGIN{\
		size_t __cnt=(p)->count;\
		size_t __bkt=(p)->bucket;\
		if ((__bkt>=3*__cnt && __bkt>QN_MIN_HASH) ||\
			(3*__bkt<=__cnt && __bkt<QN_MAX_HASH))\
			qn_inl_hash_resize((QnInlineHash*)(p));\
	}QN_STMT_END

/// @brief 해시 리사이즈
QN_INLINE void qn_inl_hash_resize(QnInlineHash* p)
{
	size_t newbucket = qn_prime_near((uint32_t)p->count);
	newbucket = QN_CLAMP(newbucket, QN_MIN_HASH, QN_MAX_HASH);
	struct QnInlineHashNode **newnodes = qn_alloc_zero(newbucket, struct QnInlineHashNode*);
	if (!newnodes)
		return;
	for (size_t i = 0; i < p->bucket; i++) {
		for (struct QnInlineHashNode *node = p->nodes[i], *next; node; node = next) {
			next = node->sib;
			const size_t hashmask = node->hash % newbucket;
			node->sib = newnodes[hashmask];
			newnodes[hashmask] = node;
		}
	}
	qn_free(p->nodes);
	p->nodes = newnodes;
	p->bucket = newbucket;
}


//////////////////////////////////////////////////////////////////////////
// mukum

/// @brief 묶음 정의
/// @param name 묶음 이름
/// @param keytype 키 타입
/// @param valuetype 값 타입
#define QN_DECL_MUKUM(name,keytype,valuetype)\
	typedef struct name name;\
	typedef keytype name##Key; typedef const keytype name##ConstKey;\
	typedef valuetype name##Value; typedef const valuetype name##ConstValue;\
	struct name##Node { struct name##Node* sib; size_t hash; name##Key key; name##Value value; };\
	struct name { size_t revision; size_t bucket; size_t count; struct name##Node** nodes; }
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
#define qn_mukum_count(p)				((p)->count)
/// @brief 묶음 버킷수
#define qn_mukum_bucket(p)				((p)->bucket)
/// @brief 묶음 변경치
#define qn_mukum_revision(p)			((p)->revision)
/// @brief 항목이 있다
#define qn_mukum_is_have(p)				((p)->count>0)

/// @brief 묶음 초기화
#define qn_mukum_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->revision=0;\
		(p)->count=0;\
		(p)->bucket=QN_MIN_HASH;\
		(p)->nodes=qn_alloc_zero(QN_MIN_HASH, struct name##Node*);\
	}QN_STMT_END

/// @brief 묶음 제거
#define qn_mukum_disp(name,p)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->bucket; __i++)\
			for (struct name##Node *__next, *__node=(p)->nodes[__i]; __node; __node=__next) {\
				__next=__node->sib;\
				name##_key(&__node->key);\
				name##_value(&__node->value);\
				qn_free(__node);\
			}\
		qn_free((p)->nodes);\
	}QN_STMT_END

/// @brief 묶음 비우기
#define qn_mukum_clear(name,p)\
	qn_inl_mukum_erase_all(name,p); qn_inl_mukum_test_size(name,p)

/// @brief 묶음 loop each
/// @param func2 loop each 함수 포인터. 인수는(keyptr,valueptr)
/// @return
#define qn_mukum_foreach_2(name,p,func2)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->bucket; __i++)\
			for (struct name##Node* __node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func2(&__node->key, &__node->value);\
	}QN_STMT_END

/// @brief 묶음 for each
/// @param func3 for each 함수 포인터. 인수는(data,keyptr,valueptr)
/// @param data for each 함수 포인터 첫 인수
/// @return
#define qn_mukum_foreach_3(name,p,func3,userdata)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->bucket; __i++)\
			for (struct name##Node* __node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func3(userdata, &__node->key, &__node->value);\
	}QN_STMT_END

/// @brief 묶음 얻기
/// @param keyptr 키 포인터
/// @param retval 값의 포인터. NULL 이면 해당 키에 대한 값이 없는 것
/// @return
#define qn_mukum_get_ptr(name,p,keyptr,retval)\
	QN_STMT_BEGIN{\
		struct name##Node **__gn, *__node;\
		qn_inl_mukum_lookup(name,p,keyptr,__gn);\
		__node=*__gn;\
		*(retval)=(__node) ? &__node->value : NULL;\
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
		for (size_t __i=0; __i<(p)->bucket; __i++) {\
			for (struct name##Node* __node=(p)->nodes[__i]; __node; __node=__node->sib) {\
				if (func3(data, &__node->key, &__node->value)) {\
					(ret_key_ptr)=&__node->key;\
					goto QN_CONCAT(pos_mukum_find_exit,__LINE__);\
				}\
			}\
		}\
		(ret_key_ptr)=NULL;\
		QN_CONCAT(pos_mukum_find_exit,__LINE__):;\
	}QN_STMT_END

/// @brief 묶음 룩업
/// @param keyptr 키 포인터
/// @param ret_node 반환값 노드 포인터
#define qn_inl_mukum_lookup(name,p,keyptr,ret_node)\
	size_t __lh=name##_hash(keyptr);\
	struct name##Node *__lnn, **__ln=&(p)->nodes[__lh%(p)->bucket];\
	while ((__lnn=*__ln)!=NULL) {\
		if (__lnn->hash==__lh && name##_eq((name##ConstKey*)&__lnn->key, keyptr)) break;\
		__ln=&__lnn->sib;\
	}\
	(ret_node)=__ln

/// @brief 묶음 룩업
/// @param keyptr 키 포인터
/// @param ret_node 반환값 노드 포인터
/// @param ret_hash 반환값 묶음
#define qn_inl_mukum_lookup_hash(name,p,keyptr,ret_node,ret_hash)\
	qn_assert((p)->nodes!=NULL && "uninitialized memory");\
	size_t __lh=name##_hash((name##ConstKey*)(keyptr));\
	struct name##Node *__lnn, **__ln=&(p)->nodes[__lh%(p)->bucket];\
	while ((__lnn=*__ln)!=NULL) {\
		if (__lnn->hash==__lh && name##_eq((name##ConstKey*)&__lnn->key, (name##ConstKey*)(keyptr))) break;\
		__ln=&__lnn->sib;\
	}\
	(ret_node)=__ln;\
	(ret_hash)=__lh

/// @brief 묶음 설정
/// @param keyptr 키 포인터
/// @param valueptr 값 포인터
/// @param replace 참=원래 키/값을 덮어씀, 거짓=주어진 키/값 삭제
#define qn_inl_mukum_set(name,p,keyptr,valueptr,replace)\
	QN_STMT_BEGIN{\
		size_t __ah; struct name##Node** __an;\
		qn_inl_mukum_lookup_hash(name,p,keyptr,__an,__ah);\
		struct name##Node* __ann=*__an;\
		if (__ann) {\
			if (replace) {\
				name##_key(&__ann->key);		__ann->key=*(keyptr);\
				name##_value(&__ann->value);	__ann->value=*(valueptr);\
			} else {\
				name##_key(keyptr);\
				name##_value(valueptr);\
			}\
		} else {\
			/* step 1*/\
			__ann=qn_alloc_1(struct name##Node);\
			__ann->sib=NULL;\
			__ann->hash=__ah;\
			__ann->key=*(keyptr);\
			__ann->value=*(valueptr);\
			/* step 2 */\
			*__an=__ann;\
			(p)->revision++;\
			(p)->count++;\
			/* step 3 */\
			qn_inl_mukum_test_size(name,p);\
		}\
	}QN_STMT_END

/// @brief 묶음 삭제
/// @param keyptr 키 포인터
/// @param ret_bool_ptr 결과를 담을 bool 타입 포인터
#define qn_inl_mukum_erase(name,p,keyptr,ret_bool_ptr)\
	QN_STMT_BEGIN{\
		struct name##Node** __rn;\
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

/// @brief 묶음 노드 삭제
/// @param pppnode 노드의 포인터의 포인터의 포인터
#define qn_inl_mukum_erase_node(name,p,pppnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=*(pppnode);\
		struct name##Node* __enn=*__en;\
		/* step 1 */\
		*__en=__enn->sib;\
		/* step 2 */\
		size_t __ebkt=__enn->hash%(p)->bucket;\
		if ((p)->nodes[__ebkt]==__enn) (p)->nodes[__ebkt] = NULL;\
		name##_key(&__enn->key);\
		name##_value(&__enn->value);\
		qn_free(__enn);\
		(p)->count--;\
		(p)->revision++;\
	}QN_STMT_END

/// @brief 모두 삭제
#define qn_inl_mukum_erase_all(name,p)\
	QN_STMT_BEGIN{\
		for (size_t __i=0; __i<(p)->bucket; __i++)\
			for (struct name##Node *__next, *__node=(p)->nodes[__i]; __node; __node=__next) {\
				__next=__node->sib;\
				name##_key(&__node->key);\
				name##_value(&__node->value);\
				qn_free(__node);\
			}\
		(p)->count=0;\
		(p)->revision++;\
		memset((p)->nodes, 0, (p)->bucket*sizeof(struct name##Node*));\
	}QN_STMT_END

/// @brief 묶음 크기 검사
#define qn_inl_mukum_test_size(name,p)\
	QN_STMT_BEGIN{\
		size_t __cnt=(p)->count;\
		size_t __bkt=(p)->bucket;\
		if ((__bkt>=3*__cnt && __bkt>QN_MIN_HASH) ||\
			(3*__bkt<=__cnt && __bkt<QN_MAX_HASH))\
			qn_inl_mukum_resize((QnInlineMukum*)(p));\
	}QN_STMT_END

/// @brief 묶음 리사이즈
QN_INLINE void qn_inl_mukum_resize(QnInlineMukum* p)
{
	size_t newbucket = qn_prime_near((uint32_t)p->count);
	newbucket = QN_CLAMP(newbucket, QN_MIN_HASH, QN_MAX_HASH);
	struct QnInlineMukumNode** newnodes = qn_alloc_zero(newbucket, struct QnInlineMukumNode*);
	if (!newnodes)
		return;
	for (size_t i = 0; i < p->bucket; i++) {
		for (struct QnInlineMukumNode *node = p->nodes[i], *next; node; node = next) {
			next = node->sib;
			const size_t hashmask = node->hash % newbucket;
			node->sib = newnodes[hashmask];
			newnodes[hashmask] = node;
		}
	}
	qn_free(p->nodes);
	p->nodes = newnodes;
	p->bucket = newbucket;
}


//////////////////////////////////////////////////////////////////////////
// blue string

#ifdef _MSC_VER
/// @brief 고정 문자열 정의
/// @param name 고정 문자열 이름
/// @param size 고정 문자열 최대 길이
#define QN_DECL_BSTR(name,size)\
	typedef struct QnBstr##name QnBstr##name;\
	_Pragma("warning(suppress:4200)")\
	struct QnBstr##name { size_t len; char data[size]; }
#else
#define QN_DECL_BSTR(name,size)\
	typedef struct QnBstr##name QnBstr##name;\
	struct QnBstr##name { size_t len; char data[size]; }
#endif
QN_DECL_BSTR(, );
QN_DECL_BSTR(64, 64);
QN_DECL_BSTR(128, 128);
QN_DECL_BSTR(260, 264);
QN_DECL_BSTR(1k, 1024);
QN_DECL_BSTR(2k, 2048);
QN_DECL_BSTR(4k, 4096);

#define qn_bstr_length(p)				((p)->len)
#define qn_bstr_data(p)					((p)->data)
#define qn_bstr_nth(p,n)				((p)->data[(n)])
#define qn_bstr_inv(p,n)				((p)->data[((p)->len)-(n)-1])

#define qn_bstr_test_init(p)\
	qn_assert((p)->len==0 && (p)->data[0]=='\0')

#define qn_bstr_init(p,str)\
	QN_STMT_BEGIN{\
		if ((str)) { (p)->len=strlen(str); qn_strcpy((p)->data, QN_COUNTOF((p)->data), (str)); }\
		else { (p)->len=0; (p)->data[0]='\0'; }\
	}QN_STMT_END

#define qn_bstr_clear(p)\
	QN_STMT_BEGIN{\
		(p)->len=0; (p)->data[0]='\0';\
	}QN_STMT_END

#define qn_bstr_intern(p)\
	QN_STMT_BEGIN{\
		(p)->len=strlen((p)->data);\
	}QN_STMT_END

#define qn_bstr_eq(p1, p2)\
	strcmp((p1)->data, (p2)->data)==0

#define qn_bstr_set(p, str)\
	QN_STMT_BEGIN{\
		(p)->len=strlen(str);\
		qn_strcpy((p)->data, QN_COUNTOF((p)->data), (str));\
	}QN_STMT_END

#define qn_bstr_set_bstr(p, right)\
	QN_STMT_BEGIN{\
		(p)->len=(right)->len;\
		qn_strcpy((p)->data, QN_COUNTOF((p)->data), (right)->data);\
	}QN_STMT_END

#define qn_bstr_set_len(p, str, len)\
	QN_STMT_BEGIN{\
		if ((len)<0) qn_bstr_set(p,str);\
		else {\
			(p)->len=len;\
			qn_strncpy((p)->data, QN_COUNTOF((p)->data), (str), (len));\
		}\
	}QN_STMT_END

#define qn_bstr_set_char(p, ch)\
	QN_STMT_BEGIN{\
		(p)->len=1; (p)->data[0]=ch; (p)->data[1]='\0';\
	}QN_STMT_END

#define qn_bstr_set_rep(p, ch, cnt)\
	QN_STMT_BEGIN{\
		(p)->len=qn_strfll((p)->data, 0, cnt, ch);\
		(p)->data[cnt]='\0';\
	}QN_STMT_END

#define qn_bstr_append(p, str)\
	QN_STMT_BEGIN{\
		qn_strcpy(&(p)->data[(p)->len], QN_COUNTOF((p)->data)-(p)->len, str);\
		(p)->len+=strlen(str);\
	}QN_STMT_END

#define qn_bstr_append_bstr(p, right)\
	QN_STMT_BEGIN{\
		qn_strcpy(&(p)->data[(p)->len], QN_COUNTOF((p)->data)-(p)->len, (right)->data);\
		(p)->len+=(right)->len;\
	}QN_STMT_END

#define qn_bstr_append_char(p, ch)\
	QN_STMT_BEGIN{\
		(p)->data[(p)->len++]=(char)(ch);\
		(p)->data[(p)->len]='\0';\
	}QN_STMT_END

#define qn_bstr_append_rep(p, ch, cnt)\
	QN_STMT_BEGIN{\
		(p)->len=qn_strfll((p)->data, (p)->len, (p)->len+(cnt), ch);\
		(p)->data[(p)->len]='\0';\
	}QN_STMT_END

#define qn_bstr_is_empty(p)\
	((p)->len==0)

#define qn_bstr_is_have(p)\
	((p)->len!=0)

#define qn_bstr_hash(p, igcase)\
	((igcase) ? qn_strihash((p)->data) : qn_strhash((p)->data))

#define qn_bstr_compare(p, s, igcase)\
	((igcase) ? qn_stricmp((p)->data, s) : strcmp((p)->data, s))

#define qn_bstr_compare_bstr(p1, p2, igcase)\
	((igcase) ? qn_stricmp((p1)->data, (p2)->data) : strcmp((p1)->data, (p2)->data))

#define qn_bstr_compare_bstr_length(p1, p2, len, igcase)\
	((igcase) ? qn_strnicmp((p1)->data, (p2)->data, len) : qn_strncmp((p1)->data, (p2)->data, len))

#define qn_bstr_format_va(p, fmt, va)\
	QN_STMT_BEGIN{\
		(p)->len=(size_t)qn_vsnprintf((p)->data, QN_COUNTOF((p)->data)-1, fmt, va);\
	}QN_STMT_END

#define qn_bstr_format(p, fmt, ...)\
	qn_inl_bstr_format(((QnBstr*)(p)), QN_COUNTOF((p)->data)-1, fmt, __VA_ARGS__)
QN_INLINE void qn_inl_bstr_format(QnBstr* p, size_t size, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	p->len = (size_t)qn_vsnprintf(p->data, size, fmt, va);
	va_end(va);
}

#define qn_bstr_append_format_va(p, fmt, va)\
	QN_STMT_BEGIN{\
		(p)->len=(size_t)qn_vsnprintf(&(p)->data[(p)->len], QN_COUNTOF((p)->data)-1-(p)->len, fmt, va);\
	}QN_STMT_END

#define qn_bstr_append_format(p, fmt, ...)\
	qn_inl_bstr_append_format(((QnBstr*)(p)), QN_COUNTOF((p)->data)-1, fmt, __VA_ARGS__)
QN_INLINE void qn_inl_bstr_append_format(QnBstr* p, size_t size, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	p->len = (size_t)qn_vsnprintf(&p->data[p->len], size - p->len, fmt, va);
	va_end(va);
}

#define qn_bstr_upper(p)\
	qn_strupr(((QnBstr*)(p))->data, ((QnBstr*)(p))->len)

#define qn_bstr_lower(p)\
	qn_strlwr(((QnBstr*)(p))->data, ((QnBstr*)(p))->len)

#define qn_bstr_trim(p)\
	QN_STMT_BEGIN{\
		qn_strtrm((p)->data);\
		(p)->len=strlen((p)->data);\
	}QN_STMT_END

#define qn_bstr_trim_left(p)\
	QN_STMT_BEGIN{\
		qn_strltm((p)->data);\
		(p)->len=strlen((p)->data);\
	}QN_STMT_END

#define qn_bstr_trim_right(p)\
	QN_STMT_BEGIN{\
		qn_strrtm((p)->data);\
		(p)->len=strlen((p)->data);\
	}QN_STMT_END

#define qn_bstr_rem_chars(p, rmlist)\
	QN_STMT_BEGIN{\
		(p)->len=strlen(qn_strrem((p)->data, rmlist));\
	}QN_STMT_END

QN_INLINE int qn_bstr_has_char(const void* p, const char* chars)
{
	const char* s = qn_strbrk(((const QnBstr*)(p))->data, chars);
	return (s) ? (int)(s - ((const QnBstr*)(p))->data) : -1;
}

QN_INLINE int qn_bstr_find_char(const void* p, size_t at, char ch)
{
	const char* s = strchr(((const QnBstr*)(p))->data + at, ch);
	return (s) ? (int)(s - ((const QnBstr*)(p))->data) : -1;
}

#define qn_bstr_sub_bstr(p, s, pos, len)\
	qn_inl_bstr_sub_bstr(((QnBstr*)(p)), QN_COUNTOF((p)->data)-1, (const QnBstr*)(s), pos, len)
QN_INLINE bool qn_inl_bstr_sub_bstr(QnBstr* p, size_t psize, const QnBstr* s, size_t pos, int len)
{
	qn_val_if_fail(s->len >= pos, false);

	if (len > 0)
		qn_val_if_fail(s->len >= (pos + (size_t)len), false);
	else
		len = (int)s->len - (int)pos;

	qn_strmid(p->data, psize, s->data, pos, (size_t)len);
	p->len = (size_t)len;
	return true;
}


//////////////////////////////////////////////////////////////////////////
// blue wcs

#ifdef _MSC_VER
/// @brief 고정 문자열 정의
/// @param name 고정 문자열 이름
/// @param size 고정 문자열 최대 길이
#define QN_DECL_BWCS(name,size)\
	typedef struct QnBwcs##name QnBwcs##name;\
	_Pragma("warning(suppress:4200)")\
	struct QnBwcs##name { size_t len; wchar data[size]; }
#else
#define QN_DECL_BWCS(name,size)\
	typedef struct QnBwcs##name QnBwcs##name;\
	struct QnBwcs##name { size_t len; wchar data[size]; }
#endif
QN_DECL_BWCS(, );
QN_DECL_BWCS(64, 64);
QN_DECL_BWCS(128, 128);
QN_DECL_BWCS(260, 264);
QN_DECL_BWCS(1k, 1024);
QN_DECL_BWCS(2k, 2048);
QN_DECL_BWCS(4k, 4096);

#define qn_bwcs_length(p)				((p)->len)
#define qn_bwcs_data(p)					((p)->data)
#define qn_bwcs_nth(p,n)				((p)->data[(n)])
#define qn_bwcs_inv(p,n)				((p)->data[((p)->len)-(n)-1])

#define qn_bwcs_test_init(p)\
	qn_assert((p)->len==0 && (p)->data[0]==L'\0')

#define qn_bwcs_init(p,str)\
	QN_STMT_BEGIN{\
		if (str) { (p)->len=wcslen(str); wcscpy_s((p)->data, QN_COUNTOF((p)->data), (str)); }\
		else { (p)->len=0; (p)->data[0]=L'\0'; }\
	}QN_STMT_END

#define qn_bwcs_clear(p)\
	QN_STMT_BEGIN{\
		(p)->len=0; (p)->data[0]=L'\0';\
	}QN_STMT_END

#define qn_bwcs_intern(p)\
	QN_STMT_BEGIN{\
		(p)->len=wcslen((p)->data);\
	}QN_STMT_END

#define qn_bwcs_eq(p1, p2)\
	wcscmp((p1)->data, (p2)->data)==0

#define qn_bwcs_set(p, str)\
	QN_STMT_BEGIN{\
		(p)->len=wcslen(str);\
		wcscpy_s((p)->data, QN_COUNTOF((p)->data), (str));\
	}QN_STMT_END

#define qn_bwcs_set_bwcs(p, right)\
	QN_STMT_BEGIN{\
		(p)->len=(right)->len;\
		wcscpy_s((p)->data, QN_COUNTOF((p)->data), (right)->data);\
	}QN_STMT_END

#define qn_bwcs_set_len(p, str, len)\
	QN_STMT_BEGIN{\
		if ((len)<0) qn_bwcs_set(p,str);\
		else {\
			(p)->len=len;\
			qn_wcsncpy((p)->data, QN_COUNTOF((p)->data), (str), (len));\
		}\
	}QN_STMT_END

#define qn_bwcs_set_char(p, ch)\
	QN_STMT_BEGIN{\
		(p)->len=1; (p)->data[0]=ch; (p)->data[1]=L'\0';\
	}QN_STMT_END

#define qn_bwcs_set_rep(p, ch, cnt)\
	QN_STMT_BEGIN{\
		(p)->len=qn_wcsfll((p)->data, 0, cnt, ch);\
		(p)->data[cnt]=L'\0';\
	}QN_STMT_END

#define qn_bwcs_append(p, str)\
	QN_STMT_BEGIN{\
		wcscpy_s(&(p)->data[(p)->len], QN_COUNTOF((p)->data)-(p)->len, str);\
		(p)->len+=wcslen(str);\
	}QN_STMT_END

#define qn_bwcs_append_bwcs(p, right)\
	QN_STMT_BEGIN{\
		wcscpy_s(&(p)->data[(p)->len], QN_COUNTOF((p)->data)-(p)->len, (right)->data);\
		(p)->len+=(right)->len;\
	}QN_STMT_END

#define qn_bwcs_append_char(p, ch)\
	QN_STMT_BEGIN{\
		(p)->data[(p)->len++]=(char)(ch);\
		(p)->data[(p)->len]=L'\0';\
	}QN_STMT_END

#define qn_bwcs_append_rep(p, ch, cnt)\
	QN_STMT_BEGIN{\
		(p)->len=qn_wcsfll((p)->data, (p)->len, (p)->len+(cnt), ch);\
		(p)->data[(p)->len]=L'\0';\
	}QN_STMT_END

#define qn_bwcs_is_empty(p)\
	((p)->len==0)

#define qn_bwcs_is_have(p)\
	((p)->len!=0)

#define qn_bwcs_hash(p, igcase)\
	((igcase) ? qn_wcsihash((p)->data) : qn_wcshash((p)->data))

#define qn_bwcs_compare(p, s, igcase)\
	((igcase) ? qn_wcsicmp((p)->data, s) : wcscmp((p)->data, s))

#define qn_bwcs_compare_bwcs(p1, p2, igcase)\
	((igcase) ? qn_wcsicmp((p1)->data, (p2)->data) : wcscmp((p1)->data, (p2)->data))

#define qn_bwcs_compare_bwcs_length(p1, p2, len, igcase)\
	((igcase) ? qn_wcsnicmp((p1)->data, (p2)->data, len) : qn_wcsncmp((p1)->data, (p2)->data, len))

#define qn_bwcs_format_va(p, fmt, va)\
	QN_STMT_BEGIN{\
		(p)->len=(size_t)qn_vsnwprintf((p)->data, QN_COUNTOF((p)->data)-1, fmt, va);\
	}QN_STMT_END

#define qn_bwcs_format(p, fmt, ...)\
	qn_inl_bwcs_format(((QnBwcs*)(p)), QN_COUNTOF((p)->data)-1, fmt, __VA_ARGS__)
QN_INLINE void qn_inl_bwcs_format(QnBwcs* p, size_t size, const wchar_t* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	p->len = (size_t)qn_vsnwprintf(p->data, size, fmt, va);
	va_end(va);
}

#define qn_bwcs_append_format_va(p, fmt, va)\
	QN_STMT_BEGIN{\
		(p)->len=(size_t)qn_vsnwprintf(&(p)->data[(p)->len], QN_COUNTOF((p)->data)-1-(p)->len, fmt, va);\
	}QN_STMT_END

#define qn_bwcs_append_format(p, fmt, ...)\
	qn_inl_bwcs_append_format(((QnBwcs*)(p)), QN_COUNTOF((p)->data)-1, fmt, __VA_ARGS__)
QN_INLINE void qn_inl_bwcs_append_format(QnBwcs* p, size_t size, const wchar_t* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	p->len = (size_t)qn_vsnwprintf(&p->data[p->len], size - p->len, fmt, va);
	va_end(va);
}

#define qn_bwcs_upper(p)\
	_wcsupr(((QnBwcs*)(p))->data);

#define qn_bwcs_lower(p)\
	_wcslwr(((QnBwcs*)(p))->data);

#define qn_bwcs_trim(p)\
	QN_STMT_BEGIN{\
		qn_wcstrm((p)->data);\
		(p)->len=wcslen((p)->data);\
	}QN_STMT_END

#define qn_bwcs_trim_left(p)\
	QN_STMT_BEGIN{\
		qn_wcsltm((p)->data);\
		(p)->len=wcslen((p)->data);\
	}QN_STMT_END

#define qn_bwcs_trim_right(p)\
	QN_STMT_BEGIN{\
		qn_wcsrtm((p)->data);\
		(p)->len=wcslen((p)->data);\
	}QN_STMT_END

#define qn_bwcs_rem_chars(p, rmlist)\
	QN_STMT_BEGIN{\
		(p)->len=wcslen(qn_wcsrem((p)->data, rmlist));\
	}QN_STMT_END

QN_INLINE int qn_bwcs_has_char(const void* p, const wchar_t* chars)
{
	const wchar_t* s = qn_wcsbrk(((const QnBwcs*)(p))->data, chars);
	return (s) ? (int)(s - ((const QnBwcs*)(p))->data) : -1;
}

QN_INLINE int qn_bwcs_find_char(const void* p, size_t at, wchar_t ch)
{
	const wchar_t* s = wcschr(((const QnBwcs*)(p))->data + at, ch);
	return (s) ? (int)(s - ((const QnBwcs*)(p))->data) : -1;
}

#define qn_bwcs_sub_bwcs(p, s, pos, len)\
	qn_inl_bstr_sub_bstr(((QnBwcs*)(p)), QN_COUNTOF((p)->data)-1, ((QnBwcs*)(s)), pos, len)
QN_INLINE bool qn_inl_bwcs_sub_bwcs(QnBwcs* p, size_t psize, const QnBwcs* s, size_t pos, int len)
{
	qn_val_if_fail(s->len >= pos, false);

	if (len > 0)
		qn_val_if_fail(s->len >= (pos + (size_t)len), false);
	else
		len = (int)s->len - (int)pos;

	qn_wcsmid(p->data, psize, s->data, pos, (size_t)len);
	p->len = (size_t)len;
	return true;
}

QN_EXTC_END
