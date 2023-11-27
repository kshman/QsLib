﻿#pragma once

#if _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127)
#pragma warning(disable:4200)
#endif

#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

#include <qn.h>

QN_EXTC_BEGIN

//////////////////////////////////////////////////////////////////////////
// array
typedef struct qnArr
{
	uint8_t* data;
	size_t				count;
	size_t				capa;
} qnArr;

#define QN_ARR_DECL(name,type)\
	typedef struct name name;\
	typedef type name##Type;\
	struct name { type* data; size_t count; size_t capa; };

#define qn_arr_nth(p,n)						(((p)->data)[(size_t)(n)])
#define qn_arr_inv(p,n)						(((p)->data)[((p)->count-1-(size_t)(n))])
#define qn_arr_count(p)						((p)->count)
#define qn_arr_capacity(p)					((p)->capa)
#define qn_arr_data(p)						((p)->data)
#define qn_arr_sizeof(name)					(sizeof(name##Type))
#define qn_arr_is_empty(p)					((p)->count==0)
#define qn_arr_is_have(p)					((p)->count!=0)

#define qn_arr_init(name,p,initcapa)\
	QN_STMT_BEGIN{\
		(p)->data=NULL;\
		(p)->count=0;\
		(p)->capa=0;\
		if ((initcapa)>0)\
			_qn_inl_arr_expand((pointer_t)(p), qn_arr_sizeof(name), (initcapa));\
	}QN_STMT_END

#define qn_arr_disp(name,p)\
	QN_STMT_BEGIN{ qn_free((p)->data); }QN_STMT_END

#define qn_arr_set_capacity(name,p,newcapa)\
	QN_STMT_BEGIN{ _qn_inl_arr_expand((pointer_t)(p), qn_arr_sizeof(name), newcapa); }QN_STMT_END

#define qn_arr_set_count(name,p,newcnt)\
	QN_STMT_BEGIN{ _qn_inl_arr_expand((pointer_t)(p), qn_arr_sizeof(name),(newcnt)); (p)->count=(newcnt); }QN_STMT_END

#define qn_arr_copy(name,p,o)\
	QN_STMT_BEGIN{\
		if (!(o) || !(o)->count)\
			(p)->count=0;\
		else\
		{\
			size_t __n=(o)->count;\
			qn_arr_set_count(name,p,__n);\
			memcpy((p)->data, (o)->data, __n*qn_arr_sizeof(name));\
		}\
	}QN_STMT_END

#define qn_arr_blob(name,p,items,cnt)\
	QN_STMT_BEGIN{\
		if ((size_t)(cnt)>0)\
		{\
			_qn_inl_arr_expand((pointer_t)(p), qn_arr_sizeof(name), (p)->count+(size_t)(cnt));\
			memcpy(&qn_arr_nth(p, (p)->count), (items), qn_arr_sizeof(name)*(size_t)(cnt));\
			(p)->count+=(size_t)(cnt);\
		}\
	}QN_STMT_END

#define qn_arr_remove_nth(name,p,n)\
	QN_STMT_BEGIN{\
		qn_assert((size_t)(n)<(p)->count, "remove index overflow!");\
		{\
			name##Type* __t=&qn_arr_nth(p,n);\
			if ((size_t)(n)!=(p)->count-1)\
				memmove(__t, __t+1, qn_arr_sizeof(name)*((p)->count-((size_t)(n)+1)));\
			(p)->count--;\
		}\
	}QN_STMT_END

#define qn_arr_remove_range(name,p,idx,cnt)\
	QN_STMT_BEGIN{\
		size_t __s=(size_t)(idx)+(size_t)(cnt);\
		size_t __cnt=(p)->count;\
		if ((size_t)(idx)<__cnt && __s<__cnt)\
		{\
			if (__s!=__cnt-1)\
				memmove(&qn_arr_nth(p,idx), &qn_arr_nth(p,__s), (__cnt-__s)*qn_arr_sizeof(name));\
			(p)->count-=(size_t)cnt;\
		}\
	}QN_STMT_END

#define qn_arr_clear(name,p)\
	((p)->count=0)

#define qn_arr_find(name,p,start,func_ud_ptr,userdata,ret)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(ret)=-1, __i=(size_t)(start); __i<__cnt; __i++)\
		{\
			if (func_ud_ptr(userdata, &qn_arr_nth(p,__i)))\
			{\
				*(ret)=__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_arr_sort(name,p,func)\
	(qn_qsort((p)->data, (p)->count, qn_arr_sizeof(name), func))

#define qn_arr_sort_context(name,p,func,userdata)\
	(qn_qsortc((p)->data, (p)->count, qn_arr_sizeof(name), func, userdata))

/* value type */
#define qn_arr_add(name,p,item)\
	QN_STMT_BEGIN{\
		if ((p)->count>=(p)->capa)\
			_qn_inl_arr_expand((pointer_t)(p), qn_arr_sizeof(name), (p)->capa+1);\
		qn_arr_nth(p,(p)->count)=(item);\
		(p)->count++;\
	}QN_STMT_END

#define qn_arr_reserve(name,p)\
	QN_STMT_BEGIN{\
		if ((p)->count>=(p)->capa)\
			_qn_inl_arr_expand((pointer_t)(p), qn_arr_sizeof(name), (p)->capa+1);\
		(p)->count++;\
	}QN_STMT_END

#define qn_arr_insert(name,p,idx,item)\
	QN_STMT_BEGIN{\
		if ((size_t)(idx)<=(p)->count)\
		{\
			if ((p)->count>=(p)->capa)\
				_qn_inl_arr_expand((pointer_t)(p), qn_arr_sizeof(name), (p)->capa+1);\
			memmove((p)->data+(size_t)(idx)+1, (p)->data+(size_t)(idx), ((p)->count-(size_t)(idx))*qn_arr_sizeof(name));\
			qn_arr_nth(p,idx)=(item);\
			(p)->count++;\
		}\
	}QN_STMT_END

#define qn_arr_remove(name,p,item)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
		{\
			if (qn_arr_nth(p,__i)==(item))\
			{\
				name##Type* __t=&qn_arr_nth(p,__i);\
				memmove(__t, __t+1, qn_arr_sizeof(name)*(__cnt-(__i+1)));\
				__cnt--;\
				break;\
			}\
		}\
		(p)->count=__cnt;\
	}QN_STMT_END

#define qn_arr_remove_cmp(name,p,func_ptr_item,item)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
		{\
			name##Type* __t=&qn_arr_nth(p,__i);\
			if (func_ptr_item(__t,(item)))\
			{\
				memmove(__t, __t+1, qn_arr_sizeof(name)*(__cnt-(__i+1)));\
				__cnt--;\
				break;\
			}\
		}\
		(p)->count=__cnt;\
	}QN_STMT_END

#define qn_arr_contains(name,p,item,retintptr)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(retintptr)=-1, __i=0; __i<__cnt; __i++)\
		{\
			if (qn_arr_nth(p,__i)==(item))\
			{\
				*(retintptr)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_arr_contains_cmp(name,p,func_ptr_item,item,retintptr)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(retintptr)=-1, __i=0; __i<__cnt; __i++)\
		{\
			if (func_ptr_item(&qn_arr_nth(p,__i),(item)))\
			{\
				*(retintptr)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_arr_foreach(name,p,func_ud_ptr,userdata)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func_ud_ptr(userdata, &qn_arr_nth(p,__i));\
	}QN_STMT_END

#define qn_arr_loopeach(name,p,func_ptr)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func_ptr(&qn_arr_nth(p,__i));\
	}QN_STMT_END

QN_INLINE void _qn_inl_arr_expand(pointer_t arr, size_t size, size_t cnt)
{
	qnArr* p = (qnArr*)arr;
	if (p->count < cnt)
	{
		p->capa = p->capa + p->capa / 2;
		if (p->capa < cnt)
			p->capa = cnt;
		p->data = !p->data ? qn_alloc(p->capa * size, uint8_t) : qn_realloc(p->data, p->capa * size, uint8_t);
	}
}


//////////////////////////////////////////////////////////////////////////
// pointer array
QN_ARR_DECL(qnPtrArr, pointer_t);

#define qn_parr_nth(p,i)			qn_arr_nth(p,i)
#define qn_parr_count(p)			qn_arr_count(p)
#define qn_parr_capacity(p)			qn_arr_capacity(p)
#define qn_parr_is_empty(p)			qn_arr_is_empty(p)
#define qn_parr_is_have(p)			qn_arr_is_have(p)

#define qn_parr_init(p, capacity)\
	qn_arr_init(qnPtrArr, p, capacity)

#define qn_parr_disp(p)\
	qn_arr_disp(qnPtrArr, p)

#define qn_parr_clear(p)\
	qn_arr_clear(qnPtrArr, p)

#define qn_parr_set_capacity(p,capa)\
	qn_arr_set_capacity(qnPtrArr, p, capa)

#define qn_parr_set_count(p,count)\
	qn_arr_set_count(qnPtrArr, p, count)

#define qn_parr_remove_nth(p, index)\
	qn_arr_remove_nth(qnPtrArr, p, index)

#define qn_parr_remove_range(p, index, count)\
	qn_arr_remove_range(qnPtrArr, p, index, count)

#define qn_parr_remove(p, item)\
	qn_arr_remove(qnPtrArr, p, item)

#define qn_parr_add(p, item)\
	qn_arr_add(qnPtrArr, p, item)

#define qn_parr_reserve(p)\
	qn_arr_reserve(qnPtrArr, p)

#define qn_parr_insert(p, index, item)\
	qn_arr_insert(qnPtrArr, index, item)

#define qn_parr_sort(p, func)\
	qn_arr_sort(qnPtrArr, p, func)

#define qn_parr_sort_context(p, func, userdata)\
	qn_arr_sort_context(qnPtrArr, p, func, userdata)

#define qn_parr_find(p, start, pred_ud_ptr, userdata, ret)\
	qn_arr_find(qnPtrArr, p, start, pred_ud_ptr, userdata, ret)

#define qn_parr_contains(p, data, ret)\
	qn_arr_contains(qnPtrArr, p, data, ret)

#define qn_parr_swap_data(p, newdata, newcount, olddataptr)\
	QN_STMT_BEGIN{\
		pointer_t __save=((qnPtrArr*)(p))->data;\
		((qnPtrArr*)(p))->data=data;\
		((qnPtrArr*)(p))->count=newcount;\
		((qnPtrArr*)(p))->capa=newcount;\
		if (*(olddataptr)) *(olddataptr)=__save;\
	}QN_STMT_END

#define qn_parr_copy(p, o)\
	qn_arr_copy(qnPtrArr, p, o)

#define qn_parr_foreach(p,func_ud_ptr,userdata)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=((qnPtrArr*)(p))->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(userdata, qn_arr_nth(p,__i));\
	}QN_STMT_END

#define qn_parr_loopeach(p,func_ptr)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=((qnPtrArr*)(p))->count;\
		for (__i=0; __i<__cnt; __i++)\
			func_ptr(qn_arr_nth(p,__i));\
	}QN_STMT_END


//////////////////////////////////////////////////////////////////////////
// container
typedef struct qnCtnr
{
	uint8_t* data;
	size_t				count;
} qnCtnr;

#define QN_CTNR_DECL(name,type)\
	typedef type name##Type;\
	typedef struct name { type* data; size_t count; } name

#if _DEBUG
#define qn_ctnr_at_safe(p,n)				(((p)->data)[((size_t)(n)<(p)->count) ? (size_t)(n) : qn_debug_assert(#n, "overflow", __FILE__, __LINE__)])
#else
#define qn_ctnr_at_safe(p,n)				(((p)->data)[(size_t)(n)])
#endif
#define qn_ctnr_nth(p,n)					(((p)->data)[(size_t)(n)])
#define qn_ctnr_count(p)					((p)->count)
#define qn_ctnr_data(p)					((p)->data)
#define qn_ctnr_size(name)				(sizeof(name##Type))
#define qn_ctnr_set(p,n,i)				(((p)->data)[(size_t)(n)]=(i))
#define qn_ctnr_is_empty(p)				((p)->count==0)
#define qn_ctnr_is_have(p)				((p)->count!=0)

#define qn_ctnr_init(name, p, cnt)\
	QN_STMT_BEGIN{\
		if ((size_t)(cnt)>0)\
		{\
			(p)->data=qn_alloc((size_t)(cnt), name##Type);\
			(p)->count=(size_t)(cnt);\
		}\
		else\
		{\
			(p)->data=NULL;\
			(p)->count=0;\
		}\
	}QN_STMT_END

#define qn_ctnr_init_zero(name, p, cnt)\
	QN_STMT_BEGIN{\
		if ((size_t)(cnt)>0)\
		{\
			(p)->data=qn_alloc_zero((size_t)(cnt), name##Type);\
			(p)->count=(size_t)(cnt);\
		}\
		else\
		{\
			(p)->data=NULL;\
			(p)->count=0;\
		}\
	}QN_STMT_END

#define qn_ctnr_disp(name, p)\
	qn_free((pointer_t)(p)->data)

#define qn_ctnr_reset(name, p)\
	QN_STMT_BEGIN{ (p)->count=0; }QN_STMT_END

#define qn_ctnr_resize(name, p, cnt)\
	qn_ctnr_set_count(name, p, cnt)

#define qn_ctnr_expand(name, p, cnt)\
	qn_ctnr_set_count(name, p, cnt)

#define qn_ctnr_set_count(name, p, cnt)\
	QN_STMT_BEGIN{\
		if ((p)->count!=(size_t)(cnt))\
		{\
			(p)->data=qn_realloc((p)->data, (size_t)(cnt), name##Type);\
			(p)->count=(size_t)(cnt);\
		}\
	}QN_STMT_END

#define qn_ctnr_add_count(name, p, cnt)\
	QN_STMT_BEGIN{\
		if ((size_t)(cnt)!=0)\
		{\
			(p)->data=qn_realloc((p)->data, (p)->count+(size_t)(cnt), name##Type);\
			(p)->count+=(size_t)(cnt);\
		}\
	}QN_STMT_END

#define qn_ctnr_add(name,p,v)\
	QN_STMT_BEGIN{\
		qn_ctnr_add_count(name,p,1);\
		qn_ctnr_nth(p,(p)->count-1)=(v);\
	}QN_STMT_END

#define qn_ctnr_zero(name, p)\
	memset((p)->data, 0, (p)->count*sizeof(name##Type))

#define qn_ctnr_contains(name,p,item,ret)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(ret)=-1, __i=0; __i<__cnt; __i++)\
		{\
			if (qn_ctnr_nth(p,__i)==item)\
			{\
				*(ret)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_ctnr_foreach(name,p,func,userdata)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(userdata, qn_ctnr_nth(p, __i));\
	}QN_STMT_END

#define qn_ctnr_loopeach(name,p,func)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(qn_ctnr_nth(p, __i));\
	}QN_STMT_END

#define qn_ctnr_ptr_foreach(name,p,func,userdata)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(userdata, &qn_ctnr_nth(p, __i));\
	}QN_STMT_END

#define qn_ctnr_ptr_loopeach(name,p,func)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(&qn_ctnr_nth(p, __i));\
	}QN_STMT_END


//////////////////////////////////////////////////////////////////////////
// pointer container 
QN_CTNR_DECL(qnPtrCtnr, pointer_t);

#define qn_pctnr_at_safe(p,n)			qn_ctnr_at_safe(p,n)
#define qn_pctnr_nth(p,n)				qn_ctnr_nth(p,n)
#define qn_pctnr_count(p,n)				qn_ctnr_count(p,n)
#define qn_pctnr_data(p)				qn_ctnr_data(p)

#define qn_pctnr_init(p, count)\
	qn_ctnr_init(qnPtrCtnr, (qnPtrCtnr*)p, count)

#define qn_pctnr_initZero(p, count)\
	qn_ctnr_init_zero(qnPtrCtnr, (qnPtrCtnr*)p, count)

#define qn_pctnr_disp(p)\
	qn_ctnr_disp(qnPtrCtnr, (qnPtrCtnr*)p)

#define qn_pctnr_set_count(p, count)\
	qn_ctnr_set_count(qnPtrCtnr, (qnPtrCtnr*)p, count)

#define qn_pctnr_add_count(p, count)\
	qn_ctnr_add_count(qnPtrCtnr, (qnPtrCtnr*)p, count)

#define qn_pctnr_add(p, item)\
	qn_ctnr_add(qnPtrCtnr, (qnPtrCtnr*)p, (pointer_t)item)

#define qn_pctnr_zero(p)\
	qn_ctnr_zero(qnPtrCtnr, (qnPtrCtnr*)p)

#define qn_pctnr_foreach(p, func, userdata)\
	qn_ctnr_foreach(qnPtrCtnr, (qnPtrCtnr*)p, func, userdata)

#define qn_pctnr_loopeach(p, func)\
	qn_ctnr_loopeach(qnPtrCtnr, (qnPtrCtnr*)p, func)


//////////////////////////////////////////////////////////////////////////
// common array & container
QN_ARR_DECL(qnByteArr, uint8_t);
QN_ARR_DECL(qnIntArr, int);
QN_ARR_DECL(qnUintArr, uint32_t);
QN_ARR_DECL(qnPcharArr, char*)						// qnPtrArr
QN_ARR_DECL(qnPccharArr, const char*);				// qnPtrArr
QN_ARR_DECL(qnAnyArr, any_t);

QN_CTNR_DECL(qnByteCtn, uint8_t);
QN_CTNR_DECL(qnIntCtn, int);
QN_CTNR_DECL(qnUintCtn, uint32_t);
QN_CTNR_DECL(qnPcharCtn, char*);					// qnPtrCtnr
QN_CTNR_DECL(qnPccharCtn, const char*);				// qnPtrCtnr
QN_CTNR_DECL(qnAnyCtn, any_t);


//////////////////////////////////////////////////////////////////////////
// list
typedef struct qnListNode
{
	struct qnListNode* next;
	struct qnListNode* prev;
	uint8_t				data[];
} qnListNode;

typedef struct qnList
{
	qnListNode* frst;
	qnListNode* last;
	size_t				count;
} qnList;

#define QN_LIST_DECL(name,type)\
	typedef struct name name;\
	typedef type name##Type;\
	struct name##Node { struct name##Node *next, *prev; type data; };\
	struct name { struct name##Node *frst, *last; size_t count; }

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
		for (__node=(p)->frst; __node; __node=__next)\
		{\
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
		if (__node)\
		{\
			if (__node->next)\
				__node->next->prev=__node->prev;\
			else\
			{\
				qn_assert((p)->last == __node, NULL);\
				(p)->last=__node->prev;\
			}\
			if (__node->prev)\
				__node->prev->next=__node->next;\
			else\
			{\
				qn_assert((p)->frst == __node, NULL);\
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
		for (__node=(p)->frst; __node; __node=__next)\
		{\
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
		for (__node=(p)->frst; __node; __node=__node->next)\
		{\
			if (__node->data==(item))\
			{\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_remove_cmp(name,p,func_data_item,item)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->frst; __node; __node=__node->next)\
		{\
			if (func_data_pitem(__node->data, item))\
			{\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_contains(name,p,item,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (*(retnode)=NULL, __node=(p)->frst; __node; __node=__node->next)\
		{\
			if (__node->data==(item))\
			{\
				*(retnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_find(name,p,predicate_user_data,userdata,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (*(retnode)=NULL, __node=(p)->frst; __node; __node=__node->next)\
		{\
			if (predicate_user_data(userdata, __node->data))\
			{\
				*(retnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_foreach(name,p,func_user_data,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func_user_data(userdata, __node->data);\
	}QN_STMT_END

#define qn_list_loopeach(name,p,func_data)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func_data(__node->data);\
	}QN_STMT_END

// reference type
#define qn_list_ref_disp_cb(name,p,func_user_pdata,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node *__node, *__next;\
		for (__node=(p)->frst; __node; __node=__next)\
		{\
			__next=__node->next;\
			func_user_pdata(userdata,&__node->data);\
			qn_free(__node);\
		}\
	}QN_STMT_END

#define qn_list_ref_append(name,p,pitem)\
	QN_STMT_BEGIN{\
		struct name##Node* __node=qn_alloc_1(struct name##Node);\
		if (__node){\
			memcpy(&__node->data, pitem, sizeof(name##Type));\
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
		if (__node){\
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
		for (__node=(p)->frst; __node; __node=__node->next)\
		{\
			if (memcmp(&__node->data, pitem)==0)\
			{\
				qn_list_remove_node(name, p, __node);\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_ref_remove_cmp(name,p,func_pdata_pitem,pitem)\
	QN_STMT_BEGIN{\
		struct name##Node* __rcnode;\
		for (__rcnode=(p)->frst; __rcnode; __rcnode=__rcnode->next)\
		{\
			if (func_pdata_pitem(&__rcnode->data, pitem))\
			{\
				qn_list_remove_node(name, p, __rcnode);\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_ref_contains(name,p,pitem,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (*(retnode)=NULL, __node=(p)->frst; __node; __node=__node->next)\
		{\
			if (memcmp(&__node->data, pitem)==0)\
			{\
				*(retnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_ref_find(name,p,predicate_user_pdata,userdata,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (*(retnode)=NULL, __node=(p)->frst; __node; __node=__node->next)\
		{\
			if (predicate_user_pdata(userdata, &__node->data))\
			{\
				*(retnode)=__node;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_list_ref_foreach(name,p,func_user_pdata,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func_user_pdata(userdata, &__node->data);\
	}QN_STMT_END

#define qn_list_ref_loopeach(name,p,func_pdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func_pdata(&__node->data);\
	}QN_STMT_END


//////////////////////////////////////////////////////////////////////////
// node list
typedef struct qnNlNode
{
	struct qnNlNode* next;
	struct qnNlNode* prev;
} qnNlNode;

typedef struct qnNodeList
{
	qnNlNode* frst;
	qnNlNode* last;
	size_t				count;
} qnNodeList;

#define QN_NODELIST_DECL(name,type)\
	typedef struct name name;\
	typedef type name##Type;\
	struct name { type* frst; type* last; size_t count; }

#define qn_nodelist_count(p)			((p)->count)
#define qn_nodelist_first(p)			((p)->last)
#define qn_nodelist_last(p)				((p)->frst)
#define qn_nodelist_is_have(p)			((p)->count!=0)
#define qn_nodelist_is_empty(p)			((p)->count==0)

#define qn_nodelist_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->frst=NULL;\
		(p)->last=NULL;\
		(p)->count=0;\
	}QN_STMT_END

#define qn_nodelist_disp(name,p)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__next;\
		for (__node=(p)->frst; __node; __node=__next)\
		{\
			__next=__node->next;\
			qn_free(__node);\
		}\
	}QN_STMT_END

#define qn_nodelist_disp_cb(name,p,func1)\
	QN_STMT_BEGIN{\
		name##Type *__node, *__next;\
		for (__node=(p)->frst; __node; __node=__next)\
		{\
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

#define qn_nodelist_remove(name,p,item,func_1)\
	QN_STMT_BEGIN{\
		if (item)\
		{\
			name##Type* __node=(name##Type*)(item);\
			if (__node->next)\
				__node->next->prev=__node->prev;\
			else\
			{\
				qn_assert((p)->last == __node, NULL);\
				(p)->last=__node->prev;\
			}\
			if (__node->prev)\
				__node->prev->next=__node->next;\
			else\
			{\
				qn_assert((p)->frst == __node, NULL);\
				(p)->frst=__node->next;\
			}\
			(p)->count--;\
			func_1(__node);\
		}\
	}QN_STMT_END

#define qn_nodelist_remove_first(name,p,func_1)\
	qn_nodelist_remove(name, (p)->last,func_1)

#define qn_nodelist_remove_last(name,p,func_1)\
	qn_nodelist_remove(name, (p)->frst,func_1)

#define qn_nodelist_unlink(name,p,item)\
	QN_STMT_BEGIN{\
		if (item)\
		{\
			name##Type* __node=(name##Type*)(item);\
			if (__node->next)\
				__node->next->prev=__node->prev;\
			else\
			{\
				qn_assert((p)->last == __node, NULL);\
				(p)->last=__node->prev;\
			}\
			if (__node->prev)\
				__node->prev->next=__node->next;\
			else\
			{\
				qn_assert((p)->frst == __node, NULL);\
				(p)->frst=__node->next;\
			}\
			(p)->count--;\
			__node->prev=__node->next=NULL;\
		}\
	}QN_STMT_END

#define qn_nodelist_contains(name,p,item,retbool)\
	QN_STMT_BEGIN{\
		name##Type* __node;\
		for (*(retbool)=false, __node=(p)->frst; __node; __node=__node->next)\
		{\
			if (__node==(item))\
			{\
				*(retbool)=true;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_nodelist_find(name,p,predicate_user_node,userdata,retbool)\
	QN_STMT_BEGIN{\
		name##Type* __node;\
		for (*(retbool)=false, __node=(p)->last; __node; __node=__node->prev)\
		{\
			if (predicate_user_node(userdata,__node))\
			{\
				*(retbool)=true;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_nodelist_foreach(name,p,func_user_node,userdata)\
	QN_STMT_BEGIN{\
		name##Type* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func_user_node(userdata,__node);\
	}QN_STMT_END

#define qn_nodelist_loopeach(name,p,func_node)\
	QN_STMT_BEGIN{\
		name##Type* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func_node(__node);\
	}QN_STMT_END


//////////////////////////////////////////////////////////////////////////
// pointer list
QN_LIST_DECL(qnPtrList, pointer_t);

#define qn_plist_count(p)				qn_list_count(p)
#define qn_plist_first(p)				qn_list_data_first(p)
#define qn_plist_last(p)				qn_list_data_last(p)
#define qn_plist_peek_first(p)			qn_list_node_first(p)
#define qn_plist_peek_last(p)			qn_list_node_last(p)
#define qn_plist_is_have(p)				qn_list_is_have(p)
#define qn_plist_is_empty(p)			qn_list_is_empty(p)

#define qn_plist_init(p)\
	qn_list_init(qnPtrList, p)

#define qn_plist_disp(p)\
	qn_list_disp(qnPtrList, p)

#define qn_plist_disp_cb(p,func_user_data,userdata)\
	qn_list_disp_cb(qnPtrList, p, func_user_data, userdata)

#define qn_plist_append(p,item)\
	qn_list_append(qnPtrList, p, item)

#define qn_plist_prepend(p,item)\
	qn_list_prepend(qnPtrList, p, item)

#define qn_plist_clear(p)\
	qn_list_clear(qnPtrList, p)

#define qn_plist_remove(p,item)\
	qn_list_remove(qnPtrList, p, item)

#define qn_plist_remove_first(p)\
	qn_list_remove_first(qnPtrList, p)

#define qn_plist_remove_last(p)\
	qn_list_remove_last(qnPtrList, p)

QN_INLINE bool qn_plist_contains(qnPtrList* p, pointer_t item)
{
	struct qnPtrListNode* node = NULL;
	qn_list_contains(qnPtrList, p, item, &node);
	return node != NULL;
}

QN_INLINE bool qn_plist_find(qnPtrList* p, bool (*pred)(pointer_t, pointer_t), pointer_t userdata)
{
	struct qnPtrListNode* node = NULL;
	qn_list_find(qnPtrList, p, pred, userdata, &node);
	return node != NULL;
}

#define qn_plist_foreach(p,func_user_data,userdata)\
	qn_list_foreach(qnPtrList, p, func_user_data, userdata)

#define qn_plist_loopeach(p,func_data)\
	qn_list_loopeach(qnPtrList, p, func_data)


//////////////////////////////////////////////////////////////////////////
// solo list
typedef struct qnSlist
{
	struct qnSlist* next;
	uint8_t				data[];
} qnSlist;

#define QN_SLIST_DECL(name,type)\
	typedef type name##Type;\
	typedef struct name { struct name* next; type data; } name;

#define qn_slist_last(p)\
	_qn_inl_slist_last(p)

#define qn_slist_nth(p,nth)\
	_qn_inl_slist_nth(p, nth)

#define qn_slist_data(p,nth)\
	(((qnSlist*)_qn_inl_slist_nth(p, nth))->data)

#define qn_slist_count(p)\
	_qn_inl_slist_count(p)

#define qn_slist_delete(name, p, ptrptr)\
	QN_STMT_BEGIN{\
		if (!(p))\
			*(ptrptr)=NULL;\
		else\
		{\
			name* __n=(p)->next;\
			qn_free(p);\
			*(ptrptr)=__n;\
		}\
	}QN_STMT_END

#define qn_slist_disp(name, p)\
	QN_STMT_BEGIN{\
		name* __n;\
		name* __p;\
		for (__p=(p); __p; __p=__n)\
		{\
			__n=__p->next;\
			qn_free(__p);\
		}\
	}QN_STMT_END

#define qn_slist_disp_func(name, p, func_1)\
	QN_STMT_BEGIN{\
		name* __n;\
		name* __p;\
		for (__p=(p); __p; __p=__n)\
		{\
			__n=__p->next;\
			func_1(__p);\
		}\
	}QN_STMT_END

#define qn_slist_concat(name, p1, p2, ptrptr)\
	QN_STMT_BEGIN{\
		if (!(p2))\
			*(ptrptr)=(p1);\
		else\
		{\
			if (!(p1))\
				*(ptrptr)=(p2);\
			else\
			{\
				name* __l=(name*)qn_slist_last(p1);\
				__l->next=(p2);\
				*(ptrptr)=(p1);\
			}\
		}\
	}QN_STMT_END

#define qn_slist_append(name, p, item, ptrptr)\
	QN_STMT_BEGIN{\
		name* __np;\
		__np=qn_alloc_1(name);\
		if (__np) {\
			__np->next=NULL;\
			__np->data=item;\
			if (!(p))\
				*(ptrptr)=__np;\
			else\
			{\
				name* __l=(name*)qn_slist_last(p);\
				__l->next=__np;\
				*(ptrptr)=(p);\
			}\
		}\
	}QN_STMT_END

#define qn_slist_prepend(name, p, item, ptrptr)\
	QN_STMT_BEGIN{\
		name* __np;\
		__np=qn_alloc_1(name);\
		if (__np) {\
			__np->next=(p);\
			__np->data=item;\
			*(ptrptr)=__np;\
		}\
	}QN_STMT_END

#define qn_slist_reserve_append(name, p, ptrptr)\
	QN_STMT_BEGIN{\
		name* __np;\
		__np=qn_alloc_zero_1(name);\
		if (!(p))\
			*(ptrptr)=__np;\
		else\
		{\
			name* __l=(name*)qn_slist_last(p);\
			__l->next=__np;\
			*(ptrptr)=(p);\
		}\
	}QN_STMT_END

#define qn_slist_reserve_prepend(name, p, ptrptr)\
	QN_STMT_BEGIN{\
		name* __np;\
		__np=qn_alloc_zero_1(name);\
		if (__np) __np->next=(p);\
		*(ptrptr)=__np;\
	}QN_STMT_END

#define qn_slist_insert(name, p, sib, item, ptrptr)\
	QN_STMT_BEGIN{\
		if (!(p))\
		{\
			name* __np=qn_alloc_zero_1(name);\
			if (__np) {\
				__np->next=NULL;\
				__np->data=item;\
			}\
			*(ptrptr)=__np;\
		}\
		else\
		{\
			name* __l=NULL;\
			name* __np;\
			for (__np=(p); __np; __l=__np, __np=__l->next)\
			{\
				if (__np==(sib))\
					break;\
			}\
			__np=qn_alloc_1(name);\
			if (__np) {\
				__np->data=item;\
				if (!__l)\
				{\
					__np->next=(p);\
					*(ptrptr)=__np;\
				}\
				else\
				{\
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
		if (__h<0)\
		{\
			qn_slist_append(name, p, item, ptrptr);\
		}\
		else if (__h==0)\
		{\
			qn_slist_prepend(name, p, item, ptrptr);\
		}\
		else\
		{\
			name* __np=qn_alloc_1(name);\
			if (__np) {\
				__np->data=item;\
				if (!p)\
				{\
					__np->next=NULL;\
					*(ptrptr)=__np;\
				}\
				else\
				{\
					name* __t=(p);\
					name* __v=NULL;\
					while (__h-->0 && __t)\
					{\
						__v=__t;\
						__t=__t->next;\
					}\
					if (__v)\
					{\
						__np->next=__v->next;\
						__v->next=__np;\
						*(ptrptr)=(p);\
					}\
					else\
					{\
						__np->next=(p);\
						*(ptrptr)=__np;\
					}\
				}\
			}\
		}\
	}QN_STMT_END

#define qn_slist_remove(name, p, item, ptrptr)\
	QN_STMT_BEGIN{\
		name* __r=(p);\
		name* __t=(p);\
		name* __v=NULL;\
		while (__t)\
		{\
			if (__t->data==item)\
			{\
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
		name* __r=(p);\
		name* __t=(p);\
		name* __v=NULL;\
		while (__t)\
		{\
			if (__t==(link))\
			{\
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
		else\
		{\
			name* __p;\
			name* __n;\
			name* __l;\
			__n=qn_alloc_zero_1(name);\
			if (!__n) *(ptrptr)=NULL;\
			else{\
				__n->data=(p)->data;\
				__l=__n;\
				__p=(p)->next;\
				while (__p)\
				{\
					__l->next=qn_alloc_1(name);\
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
		name* __p=(p);\
		while (__p)\
		{\
			if (__p->data==item)\
				break;\
			__p=__p->next;\
		}\
		*(ptrptr)=__p;\
	}QN_STMT_END

#define qn_slist_contains_func(name,p,func2,item,ptrptr)\
	QN_STMT_BEGIN{\
		name* __p=(p);\
		while (__p)\
		{\
			if (func2(__p->data, item))\
				break;\
			__p=__p->next;\
		}\
		*(ptrptr)=__p;\
	}QN_STMT_END

#define qn_slist_foreach(name,p,func_2,userdata)\
	QN_STMT_BEGIN{\
		name* __p=(p);\
		while (__p)\
		{\
			name* __n=__p->next;\
			func_2(userdata, __p->data);\
			__p=__n;\
		}\
	}QN_STMT_END

#define qn_slist_loopeach(name,p,func_1)\
	QN_STMT_BEGIN{\
		name* __p=(p);\
		while (__p)\
		{\
			name* __n=__p->next;\
			func_1(__p->data);\
			__p=__n;\
		}\
	}QN_STMT_END

QN_INLINE pointer_t _qn_inl_slist_last(pointer_t ptr)
{
	qnSlist* p = (qnSlist*)ptr;
	if (p)
	{
		while (p->next)
			p = p->next;
	}
	return p;
}

QN_INLINE pointer_t _qn_inl_slist_nth(pointer_t ptr, size_t nth)
{
	qnSlist* p = (qnSlist*)ptr;
	while (nth-- > 0 && p)
		p = p->next;
	return (pointer_t)p;
}

QN_INLINE size_t _qn_inl_slist_count(pointer_t ptr)
{
	qnSlist* p = (qnSlist*)ptr;
	size_t n = 0;
	while (p)
	{
		n++;
		p = p->next;
	}
	return n;
}


//////////////////////////////////////////////////////////////////////////
// pointer solo list
QN_SLIST_DECL(qnPtrSlist, pointer_t);

#define qn_pslist_last(p)\
	qn_slist_last(p)

#define qn_pslist_nth(p, nth)\
	qn_slist_nth(p, nth)

#define qn_pslist_data(p, nth)\
	qn_slist_data(p, nth)

#define qn_pslist_count(p)\
	qn_slist_count(p)

QN_INLINE qnPtrSlist* qn_pslist_delete(qnPtrSlist* p)
{
	qn_slist_delete(qnPtrSlist, p, &p);
	return p;
}

QN_INLINE void qn_pslist_disp(qnPtrSlist* p, void (*func_1)(pointer_t))
{
	if (!func_1)
	{
		qn_slist_disp(qnPtrSlist, p);
	}
	else
	{
		qnPtrSlist* n = NULL;
		for (; p; p = n)
		{
			n = p->next;
			if (func_1)
				func_1(p->data);
			qn_free(p);
		}
	}
}

QN_INLINE qnPtrSlist* qn_pslist_concat(qnPtrSlist* p1, qnPtrSlist* p2)
{
	qn_slist_concat(qnPtrSlist, p1, p2, &p1);
	return p1;
}

QN_INLINE qnPtrSlist* qn_pslist_append(qnPtrSlist* p, pointer_t item)
{
	qn_slist_append(qnPtrSlist, p, item, &p);
	return p;
}

QN_INLINE qnPtrSlist* qn_pslist_prepend(qnPtrSlist* p, pointer_t item)
{
	qn_slist_prepend(qnPtrSlist, p, item, &p);
	return p;
}

QN_INLINE qnPtrSlist* qn_pslist_insert(qnPtrSlist* p, qnPtrSlist* sib, pointer_t item)
{
	qn_slist_insert(qnPtrSlist, p, sib, item, &p);
	return p;
}

QN_INLINE qnPtrSlist* qn_pslist_insert_nth(qnPtrSlist* p, int nth, pointer_t item)
{
	qn_slist_insert_nth(qnPtrSlist, p, nth, item, &p);
	return p;
}

QN_INLINE qnPtrSlist* qn_pslist_remove(qnPtrSlist* p, pointer_t item)
{
	qn_slist_remove(qnPtrSlist, p, item, &p);
	return p;
}

QN_INLINE qnPtrSlist* qn_pslist_remove_link(qnPtrSlist* p, qnPtrSlist* link)
{
	qn_slist_remove_link(qnPtrSlist, p, link, &p);
	return p;
}

QN_INLINE qnPtrSlist* qn_pslist_copy(qnPtrSlist* p)
{
	qnPtrSlist* n = NULL;
	qn_slist_copy(qnPtrSlist, p, &n);
	return n;
}

QN_INLINE qnPtrSlist* qn_pslist_contains(qnPtrSlist* p, pointer_t item)
{
	qn_slist_contains(qnPtrSlist, p, item, &p);
	return p;
}

#define qn_pslist_foreach(p,func_2,userdata)\
	qn_slist_foreach(qnPtrSlist, p, func_2, userdata)

#define qn_pslist_loopeach(p,func_1)\
	qn_slist_loopeach(qnPtrSlist, p, func_1);


//////////////////////////////////////////////////////////////////////////
// fixed slice
typedef struct qnSlice
{
	uint8_t* data;
	size_t				max;
	size_t				count;
} qnSlice;

#define QN_SLICE_DECL(name, type)\
	typedef type name##Type;\
	typedef struct name { type* data; size_t max; size_t count; } name

#define qn_slice_nth(p,n)				(((p)->data)[(size_t)(n)])
#define qn_slice_invat(p,n)				(((p)->data)[((p)->count-1-(size_t)(n)])
#define qn_slice_count(p)				((p)->count)
#define qn_slice_maximum(p)				((p)->max)
#define qn_slice_data(p)				((p)->data)
#define qn_slice_sizeof(name)			(sizeof(name##Type))
#define qn_slice_is_empty(p)			((p)->count==0)
#define qn_slice_is_have(p)				((p)->count!=0)

#define qn_slice_test_init(name,p,_max)\
	QN_STMT_BEGIN{\
		qn_assert((p)->data==NULL && (p)->max==0 && (p)->count==0, "uninitialized slice memory.");\
		(p)->data=qn_alloc(_max, name##Type);\
		(p)->max=_max;\
	}QN_STMT_END

#define qn_slice_init(name,p,_max)\
	QN_STMT_BEGIN{\
		(p)->max=_max;\
		(p)->count=0;\
		(p)->data=qn_alloc((p)->max, name##Type);\
	}QN_STMT_END

#define qn_slice_disp(name,p)\
	QN_STMT_BEGIN{ qn_free((p)->data); }QN_STMT_END

#define qn_slice_clear(name,p)\
	QN_STMT_BEGIN{ (p)->count=0; }QN_STMT_END

#define qn_slice_disable(name,p)\
	QN_STMT_BEGIN{ (p)->max=(p)->count; }QN_STMT_END

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
		if ((p)->count==(p)->max)\
		{\
			if (__r) *__r=false;\
		}\
		else\
		{\
			if (__r) *__r=true;\
			(p)->data[(p)->count]=*(itemref);\
			(p)->count++;\
		}\
	}QN_STMT_END

#define qn_slice_pop(name,p,retboolref)\
	QN_STMT_BEGIN{\
		bool* __r=(retboolref);\
		if ((p)->count==0)\
		{\
			if (__r) *__r=false;\
		}\
		else if ((p)->count==1)\
		{\
			if (__r) *__r=true;\
			(p)->count=0;\
		}\
		else\
		{\
			if (__r) *__r=true;\
			(p)->count--;\
			(p)->data[0]=(p)->data[(p)->count];\
		}\
	}QN_STMT_END

#define qn_slice_pop_value(name,p,retvalueref,retboolref)\
	QN_STMT_BEGIN{\
		bool* __r=(retboolref);\
		if ((p)->count==0)\
		{\
			if (__r) *__r=false;\
		}\
		else if ((p)->count==1)\
		{\
			if (__r) *__r=true;\
			*(retvalueref)=(p)->data[0];\
			(p)->count=0;\
		}\
		else\
		{\
			if (__r) *__r=true;\
			*(retvalueref)=(p)->data[0];\
			(p)->count--;\
			(p)->data[0]=(p)->data[(p)->count];\
		}\
	}QN_STMT_END

#define qn_slice_find(name,p,start,func2,userdata,retintref)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(retintref)=-1, __i=(size_t)(start); __i<__cnt; __i++)\
		{\
			if (func2(userdata, &qn_slice_nth(p,__i)))\
			{\
				*(retintref)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_slice_contains(name,p,itemptr,retintref)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(retintref)=-1, __i=0; __i<__cnt; __i++)\
		{\
			if (memcmp(&qn_slice_nth(p,__i), itemptr, sizeof(name##Type))==0)\
			{\
				*(retintref)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_slice_contains_func(name,p,func,itemptr,retintref)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(retintref)=-1, __i=0; __i<__cnt; __i++)\
		{\
			if (func(&qn_slice_nth(p,__i),(itemptr)))\
			{\
				*(retintref)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_slice_foreach(name,p,func,userdata)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(userdata, &qn_slice_nth(p,__i));\
	}QN_STMT_END

#define qn_slice_loopeach(name,p,func)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(&qn_slice_nth(p,__i));\
	}QN_STMT_END

#define qn_slice_ptr_push(name,p,item,retboolref)\
	QN_STMT_BEGIN{\
		bool* __r=(retboolref);\
		if ((p)->count==(p)->max)\
		{\
			if (__r) *__r=false;\
		}\
		else\
		{\
			if (__r) *__r=true;\
			(p)->data[(p)->count]=(item);\
			(p)->count++;\
		}\
	}QN_STMT_END

#define qn_slice_ptr_pop(name,p,retboolref)\
	qn_slice_Pop(name,p,retboolref)

#define qn_slice_ptr_pop_value(name,p,retvalueref)\
	QN_STMT_BEGIN{\
		if ((p)->count==0)\
		{\
			*(retvalueref)=NULL;\
		}\
		else if ((p)->count==1)\
		{\
			*(retvalueref)=(p)->data[0];\
			(p)->count=0;\
		}\
		else\
		{\
			*(retvalueref)=(p)->data[0];\
			(p)->count--;\
			(p)->data[0]=(p)->data[(p)->count];\
		}\
	}QN_STMT_END


#define qn_slice_ptr_find(name,p,start,func2,userdata,retintref)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(retintref)=-1, __i=(size_t)(start); __i<__cnt; __i++)\
		{\
			if (func2(userdata, qn_slice_nth(p,__i)))\
			{\
				*(retintref)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_slice_ptr_contains(name,p,item,retintref)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(retintref)=-1, __i=0; __i<__cnt; __i++)\
		{\
			if (qn_slice_nth(p,__i)==item)\
			{\
				*(retintref)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_slice_ptr_contains_func(name,p,func,item,retintref)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (*(retintref)=-1, __i=0; __i<__cnt; __i++)\
		{\
			if (func(qn_slice_nth(p,__i),(item)))\
			{\
				*(retintref)=(int)__i;\
				break;\
			}\
		}\
	}QN_STMT_END

#define qn_slice_ptr_foreach(name,p,func,userdata)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(userdata, qn_slice_nth(p,__i));\
	}QN_STMT_END

#define qn_slice_ptr_loopeach(name,p,func)\
	QN_STMT_BEGIN{\
		size_t __i, __cnt=(p)->count;\
		for (__i=0; __i<__cnt; __i++)\
			func(qn_slice_nth(p,__i));\
	}QN_STMT_END


//////////////////////////////////////////////////////////////////////////
// hash
typedef struct qnHashNode
{
	struct qnHashNode* sib;
	struct qnHashNode* next;
	struct qnHashNode* prev;
	size_t				hash;
	uint64_t* key;
	uint64_t* value;
} qnHashNode;

typedef struct qnHash
{
	size_t				revision;
	size_t				bucket;
	size_t				count;
	struct qnHashNode** nodes;
	struct qnHashNode* frst;
	struct qnHashNode* last;
} qnHash;

#define QN_HASH_DECL(name,keytype,valuetype)\
	typedef keytype name##Key;\
	typedef valuetype name##Value;\
	typedef struct name##Node { struct name##Node* sib; struct name##Node* next; struct name##Node* prev; size_t hash; name##Key key; name##Value value; } name##Node;\
	typedef struct name { size_t revision; size_t bucket; size_t count; struct name##Node** nodes; struct name##Node* frst; struct name##Node* last; } name

#define QN_HASH_HASH(name,func1)\
	QN_INLINE size_t name##_Hash_Cb_Hash(name##Key* key) { return func1(key); }
#define QN_HASH_HASH_PURE(name)\
	QN_INLINE size_t name##_Hash_Cb_Hash(name##Key* key) { return (size_t)(uintptr_t)(*key); }

#define QN_HASH_EQ(name,func2)\
	QN_INLINE bool name##_Hash_Cb_Eq(name##Key* k1, name##Key* k2) { return func2(k1, k2); }
#define QN_HASH_EQ_PURE(name)\
	QN_INLINE bool name##_Hash_Cb_Eq(name##Key* k1, name##Key* k2) { return *k1==*k2; }

#define QN_HASH_KEY(name,func1)\
	QN_INLINE void name##_Hash_Cb_Key(name##Key* key) { func1(key); }
#define QN_HASH_KEY_DELETE(name)\
	QN_INLINE void name##_Hash_Cb_Key(name##Key* key) { qn_free(*key); }
#define QN_HASH_KEY_NONE(name)\
	QN_INLINE void name##_Hash_Cb_Key(name##Key* key) { (void)key; }

#define QN_HASH_VALUE(name,func1)\
	QN_INLINE void name##_Hash_Cb_Value(name##Value* value) { func1(value); }
#define QN_HASH_VALUE_DELETE(name)\
	QN_INLINE void name##_Hash_Cb_Value(name##Value* value) { qn_free(*value); }
#define QN_HASH_VALUE_NONE(name)\
	QN_INLINE void name##_Hash_Cb_Value(name##Value* value) { (void)value; }

#define QN_HASH_BOTH_NONE(name)\
	QN_HASH_KEY_NONE(name);\
	QN_HASH_VALUE_NONE(name)

#define QN_HASH_HASHER_INT(name)\
	QN_INLINE size_t name##_Hash_Cb_Hash(int* key) { return (size_t)(*key); }\
	QN_INLINE bool name##_Hash_Cb_Eq(int* k1, int* k2) { return (*k1)==(*k2); }
#define QN_HASH_HASHER_UINT(name)\
	QN_INLINE size_t name##_Hash_Cb_Hash(kuint* key) { return (size_t)(*key); }\
	QN_INLINE bool name##_Hash_Cb_Eq(kuint* k1, kuint* k2) { return ((int)*k1)==((int)*k2); }
#define QN_HASH_HASHER_SIZET(name)\
	QN_INLINE size_t name##_Hash_Cb_Hash(size_t* key) { return *key; }\
	QN_INLINE bool name##_Hash_Cb_Eq(size_t* k1, size_t* k2) { return (*k1)==(*k2); }
#define QN_HASH_HASHER_CHAR_PTR(name)\
	QN_INLINE size_t name##_Hash_Cb_Hash(cpointer_t key) { return qn_strhash(*(const char**)key); }\
	QN_INLINE bool name##_Hash_Cb_Eq(cpointer_t k1, cpointer_t k2) { return strcmp(*(const char**)k1, *(const char**)k2)==0; }
#define QN_HASH_HASHER_WCHAR_PTR(name)\
	QN_INLINE size_t name##_Hash_Cb_Hash(cpointer_t key) { return qn_wcshash(*(const wchar_t**)key); }\
	QN_INLINE bool name##_Hash_Cb_Eq(cpointer_t k1, cpointer_t k2) { return wcscmp(*(const wchar_t**)k1, *(const wchar_t**)k2)==0; }
#define QN_HASH_HASHER_CHAR_PTR_INCASE(name)\
	QN_INLINE size_t name##_Hash_Cb_Hash(const char** key) { return qn_strihash(*key); }\
	QN_INLINE bool name##_Hash_Cb_Eq(const char** k1, const char** k2) { return stricmp(*k1, *k2)==0; }
#define QN_HASH_HASHER_WCHAR_PTR_INCASE(name)\
	QN_INLINE size_t name##_Hash_Cb_Hash(const wchar_t** key) { return qn_wcsihash(*key); }\
	QN_INLINE bool name##_Hash_Cb_Eq(const wchar_t** k1, const wchar_t** k2) { return wcsicmp(*k1, *k2)==0; }

#define QN_HASH_PTR_HASH(name,func1)\
	QN_INLINE size_t name##_Hash_Cb_PtrHash(name##Key key) { return func1(key); }
#define QN_HASH_PTR_HASH_PURE(name)\
	QN_INLINE size_t name##_Hash_Cb_PtrHash(name##Key key) { return (size_t)(uintptr_t)(key); }

#define QN_HASH_PTR_EQ(name,func2)\
	QN_INLINE bool name##_Hash_Cb_PtrEq(name##Key k1, name##Key k2) { return func2(k1, k2); }
#define QN_HASH_PTR_EQ_PURE(name)\
	QN_INLINE bool name##_Hash_Cb_PtrEq(name##Key k1, name##Key k2) { return k1==k2; }

#define QN_HASH_PTR_KEY(name,func1)\
	QN_INLINE void name##_Hash_Cb_PtrKey(name##Key key) { func1(key); }
#define QN_HASH_PTR_KEY_DELETE(name)\
	QN_INLINE void name##_Hash_Cb_PtrKey(name##Key key) { qn_free(key); }
#define QN_HASH_PTR_KEY_NONE(name)\
	QN_INLINE void name##_Hash_Cb_PtrKey(name##Key key) { (void)key; }

#define QN_HASH_PTR_VALUE(name,func1)\
	QN_INLINE void name##_Hash_Cb_PtrValue(name##Value value) { func1(value); }
#define QN_HASH_PTR_VALUE_DELETE(name)\
	QN_INLINE void name##_Hash_Cb_PtrValue(name##Value value) { qn_free(value); }
#define QN_HASH_PTR_VALUE_NONE(name)\
	QN_INLINE void name##_Hash_Cb_PtrValue(name##Value value) { (void)value; }

#define QN_HASH_PTR_BOTH_NONE(name)\
	QN_HASH_PTR_KEY_NONE(name);\
	QN_HASH_PTR_VALUE_NONE(name)

#define QN_HASH_PTR_HASHER_CHAR_PTR(name)\
	QN_INLINE size_t name##_Hash_Cb_PtrHash(const char* key) { return qn_strhash(key); }\
	QN_INLINE bool name##_Hash_Cb_PtrEq(const char* k1, const char* k2) { return strcmp(k1, k2)==0; }
#define QN_HASH_PTR_HASHER_WCHAR_PTR(name)\
	QN_INLINE size_t name##_Hash_Cb_PtrHash(const wchar_t* key) { return qn_wcshash(key); }\
	QN_INLINE bool name##_Hash_Cb_PtrEq(const wchar_t* k1, const wchar_t* k2) { return wcscmp(k1, k2)==0; }

//
#define qn_hash_count(p)				((p)->count)
#define qn_hash_bucket(p)				((p)->bucket)
#define qn_hash_revision(p)				((p)->revision)
#define qn_hash_node_first(p)			((p)->last)
#define qn_hash_node_last(p)			((p)->frst)

#define qn_hash_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->revision=0;\
		(p)->count=0;\
		(p)->bucket=QN_MIN_HASH;\
		(p)->nodes=qn_alloc_zero(QN_MIN_HASH, struct name##Node*);\
		(p)->frst=(p)->last=NULL;\
	}QN_STMT_END

//
#define qn_hash_disp(name,p)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		struct name##Node* __next;\
		for (__node=(p)->frst; __node; __node=__next)\
		{\
			__next=__node->next;\
			name##_Hash_Cb_Key(&__node->key);\
			name##_Hash_Cb_Value(&__node->value);\
			qn_free(__node);\
		}\
		qn_free((p)->nodes);\
	}QN_STMT_END

#define qn_hash_remove_node(name,p,node)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=&(node);\
		_qn_inl_hash_erase_node(name,p,&__en);\
		_qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

#define qn_hash_clear(name,p)\
	QN_STMT_BEGIN{\
		_qn_inl_hash_erase_all(name,p);\
		_qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

#define qn_hash_foreach(name,p,func3,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func3(userdata, &__node->key, &__node->value);\
	}QN_STMT_END

#define qn_hash_loopeach(name,p,func2)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func2(&__node->key, &__node->value);\
	}QN_STMT_END

#define qn_hash_foreach_value(name,p,func2,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func2(userdata, &__node->value);\
	}QN_STMT_END

#define qn_hash_loopeach_value(name,p,func1)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func1(&__node->value);\
	}QN_STMT_END

#define qn_hash_node(name,p,keyptr,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __gn;\
		_qn_inl_hash_lookup(name,p,keyptr,__gn);\
		(retnode)=*__gn;\
	}QN_STMT_END

#define qn_hash_get(name,p,keyptr,retvalue)\
	QN_STMT_BEGIN{\
		/* retvalue: pointer of 'value' */\
		struct name##Node** __gn;\
		struct name##Node* __node;\
		_qn_inl_hash_lookup(name,p,keyptr,__gn);\
		__node=*__gn;\
		*(retvalue)=(__node) ? &__node->value : NULL;\
	}QN_STMT_END

#define qn_hash_add(name,p,keyptr,valueptr)\
	_qn_inl_hash_set(name,p,keyptr,valueptr,false)

#define qn_hash_set(name,p,keyptr,valueptr)\
	_qn_inl_hash_set(name,p,keyptr,valueptr,true);

#define qn_hash_remove(name,p,keyptr,retbool_can_be_null)\
	QN_STMT_BEGIN{\
		_qn_inl_hash_erase(name,p,keyptr,retbool_can_be_null);\
		_qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

#define qn_hash_find(name,p,func,userdata,retkey,gotoname)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
		{\
			if (func(userdata, &__node->key, &__node->value))\
			{\
				retkey=&__node->key;\
				goto K_CONCAT(pos_hash_find_exit,gotoname);\
			}\
		}\
		retkey=NULL;\
		K_CONCAT(pos_hash_find_exit,gotoname):;\
	}QN_STMT_END

//
#define _qn_inl_hash_lookup(name,p,keyptr,retnode)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* retnode: pointer's pointer of Node */\
		size_t __lh=name##_Hash_Cb_Hash(keyptr);\
		struct name##Node** __ln=&(p)->nodes[__lh%(p)->bucket];\
		struct name##Node* __lnn;\
		while ((__lnn=*__ln)!=NULL)\
		{\
			if (__lnn->hash==__lh && name##_Hash_Cb_Eq(&__lnn->key, keyptr))\
				break;\
			__ln=&__lnn->sib;\
		}\
		(retnode)=__ln;\
	}QN_STMT_END

#define _qn_inl_hash_lookup_hash(name,p,keyptr,retnode,rethash)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* retnode: pointer's pointer of Node */\
		/* rethash: pointer of kuint */\
		if ((p)->nodes) {\
			size_t __lh=name##_Hash_Cb_Hash(keyptr);\
			struct name##Node** __ln=&(p)->nodes[__lh%(p)->bucket];\
			struct name##Node* __lnn;\
			while ((__lnn=*__ln)!=NULL)\
			{\
				if (__lnn->hash==__lh && name##_Hash_Cb_Eq(&__lnn->key, keyptr))\
					break;\
				__ln=&__lnn->sib;\
			}\
			(retnode)=__ln;\
			*(rethash)=__lh;\
		}\
	}QN_STMT_END

#define _qn_inl_hash_set(name,p,keyptr,valueptr,replace)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* valuedata : data of value */\
		/* replace: true=replace original key, false=discard given key */\
		size_t __ah=0;\
		struct name##Node** __an=NULL;\
		_qn_inl_hash_lookup_hash(name,p,keyptr,__an,&__ah);\
		if (__an) {\
			struct name##Node* __ann=*__an;\
			if (__ann)\
			{\
				if (replace)\
				{\
					name##_Hash_Cb_Key(&__ann->key);\
					__ann->key=*(keyptr);\
					name##_Hash_Cb_Value(&__ann->value);\
					__ann->value=*(valueptr);\
				}\
				else\
				{\
					name##_Hash_Cb_Key(keyptr);\
					name##_Hash_Cb_Value(valueptr);\
				}\
			}\
			else\
			{\
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
				_qn_inl_hash_test_size(name,p);\
			}\
		}\
	}QN_STMT_END

#define _qn_inl_hash_erase(name,p,keyptr,retbool)\
	QN_STMT_BEGIN{\
		struct name##Node** __rn;\
		_qn_inl_hash_lookup(name,p,keyptr,__rn);\
		if (*__rn==NULL)\
		{\
			if (retbool)\
			{\
				bool* __c=retbool;\
				*__c=false;\
			}\
		}\
		else\
		{\
			_qn_inl_hash_erase_node(name,p,&__rn);\
			if (retbool)\
			{\
				bool* __c=retbool;\
				*__c=true;\
			}\
		}\
	}QN_STMT_END

#define _qn_inl_hash_erase_node(name,p,pppnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=*pppnode;\
		struct name##Node* __enn=*__en;\
		/* step 1 */\
		*__en=__enn->sib;\
		/* step 2 */\
		if (__enn->next)\
			__enn->next->prev=__enn->prev;\
		else\
		{\
			qn_assert((p)->last == __enn, NULL);\
			(p)->last=__enn->prev;\
		}\
		if (__enn->prev)\
			__enn->prev->next=__enn->next;\
		else\
		{\
			qn_assert((p)->frst == __enn, NULL);\
			(p)->frst=__enn->next;\
		}\
		/* step3 */\
		name##_Hash_Cb_Key(&__enn->key);\
		name##_Hash_Cb_Value(&__enn->value);\
		qn_free(__enn);\
		(p)->count--;\
		(p)->revision++;\
	}QN_STMT_END

#define _qn_inl_hash_erase_all(name,p)\
	QN_STMT_BEGIN{\
		struct name##Node* __enn;\
		struct name##Node* __enx;\
		for (__enn=(p)->frst; __enn; __enn=__enx)\
		{\
			__enx=__enn->next;\
			name##_Hash_Cb_Key(&__enn->key);\
			name##_Hash_Cb_Value(&__enn->value);\
			qn_free(__enn);\
		}\
		(p)->frst=(p)->last=NULL;\
		(p)->count=0;\
		memset((p)->nodes, 0, (p)->bucket*sizeof(struct name##Node*));\
	}QN_STMT_END

//
#define qn_hash_ptr_disp(name,p)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		struct name##Node* __next;\
		for (__node=(p)->frst; __node; __node=__next)\
		{\
			__next=__node->next;\
			name##_Hash_Cb_PtrKey(__node->key);\
			name##_Hash_Cb_PtrValue(__node->value);\
			qn_free(__node);\
		}\
		qn_free((p)->nodes);\
	}QN_STMT_END

#define qn_hash_ptr_remove_node(name,p,node)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=&(node);\
		_qn_inl_hash_ptr_erase_node(name,p,&__en);\
		_qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

#define qn_hash_ptr_clear(name,p)\
	QN_STMT_BEGIN{\
		_qn_inl_hash_ptr_erase_all(name,p);\
		_qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

#define qn_hash_ptr_foreach(name,p,func3,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func3(userdata, __node->key, __node->value);\
	}QN_STMT_END

#define qn_hash_ptr_loopeach(name,p,func2)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func2(__node->key, __node->value);\
	}QN_STMT_END

#define qn_hash_ptr_foreach_value(name,p,func2,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func2(userdata, __node->value);\
	}QN_STMT_END

#define qn_hash_ptr_loopeach_value(name,p,func1)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->last; __node; __node=__node->prev)\
			func1(__node->value);\
	}QN_STMT_END

#define qn_hash_ptr_loopeach_inv_value(name,p,func1)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		for (__node=(p)->frst; __node; __node=__node->next)\
			func1(__node->value);\
	}QN_STMT_END

#define qn_hash_ptr_node(name,p,keyptr,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __gn;\
		_qn_inl_hash_ptr_lookup(name,p,keyptr,__gn);\
		(retnode)=*__gn;\
	}QN_STMT_END

#define qn_hash_ptr_get(name,p,keyptr,retvalue)\
	QN_STMT_BEGIN{\
		/* retvalue: pointer of 'value' */\
		struct name##Node** __gn;\
		struct name##Node* __node;\
		_qn_inl_hash_ptr_lookup(name,p,keyptr,__gn);\
		__node=*__gn;\
		*(retvalue)=(__node) ? __node->value : NULL;\
	}QN_STMT_END

#define qn_hash_ptr_add(name,p,keyptr,valueptr)\
	_qn_inl_hash_ptr_set(name,p,keyptr,valueptr,false)

#define qn_hash_ptr_set(name,p,keyptr,valueptr)\
	_qn_inl_hash_ptr_set(name,p,keyptr,valueptr,true);

#define qn_hash_ptr_remove(name,p,keyptr,retbool_can_be_null)\
	QN_STMT_BEGIN{\
		_qn_inl_hash_ptr_erase(name,p,keyptr,retbool_can_be_null);\
		_qn_inl_hash_test_size(name,p);\
	}QN_STMT_END

//
#define _qn_inl_hash_ptr_lookup(name,p,keyptr,retnode)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* retnode: pointer's pointer of Node */\
		size_t __lh=name##_Hash_Cb_PtrHash(keyptr);\
		struct name##Node** __ln=&(p)->nodes[__lh%(p)->bucket];\
		struct name##Node* __lnn;\
		while ((__lnn=*__ln)!=NULL)\
		{\
			if (__lnn->hash==__lh && name##_Hash_Cb_PtrEq(__lnn->key, keyptr))\
				break;\
			__ln=&__lnn->sib;\
		}\
		(retnode)=__ln;\
	}QN_STMT_END

#define _qn_inl_hash_ptr_lookup_hash(name,p,keyptr,retnode,rethash)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* retnode: pointer's pointer of Node */\
		/* rethash: pointer of kuint */\
		size_t __lh=name##_Hash_Cb_PtrHash(keyptr);\
		struct name##Node** __ln=&(p)->nodes[__lh%(p)->bucket];\
		struct name##Node* __lnn;\
		while ((__lnn=*__ln)!=NULL)\
		{\
			if (__lnn->hash==__lh && name##_Hash_Cb_PtrEq(__lnn->key, keyptr))\
				break;\
			__ln=&__lnn->sib;\
		}\
		(retnode)=__ln;\
		*(rethash)=__lh;\
	}QN_STMT_END

#define _qn_inl_hash_ptr_set(name,p,keyptr,valueptr,replace)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* valuedata : data of value */\
		/* replace: true=replace original key, false=discard given key */\
		size_t __ah;\
		struct name##Node** __an;\
		struct name##Node* __ann;\
		_qn_inl_hash_ptr_lookup_hash(name,p,keyptr,__an,&__ah);\
		__ann=*__an;\
		if (__ann)\
		{\
			if (replace)\
			{\
				name##_Hash_Cb_PtrKey(__ann->key);\
				__ann->key=keyptr;\
				name##_Hash_Cb_PtrValue(__ann->value);\
				__ann->value=valueptr;\
			}\
			else\
			{\
				name##_Hash_Cb_PtrKey(keyptr);\
				name##_Hash_Cb_PtrValue(valueptr);\
			}\
		}\
		else\
		{\
			/* step 1*/\
			__ann=qn_alloc_1(struct name##Node);\
			__ann->sib=NULL;\
			__ann->hash=__ah;\
			__ann->key=keyptr;\
			__ann->value=valueptr;\
			/* step 2*/\
			if ((p)->frst)\
				(p)->frst->prev=__ann;\
			else\
				(p)->last=__ann;\
			__ann->next=(p)->frst;\
			__ann->prev=NULL;\
			(p)->frst=__ann;\
			/* step 3 */\
			*__an=__ann;\
			(p)->revision++;\
			(p)->count++;\
			/* step 4 */\
			_qn_inl_hash_test_size(name,p);\
		}\
	}QN_STMT_END

#define _qn_inl_hash_ptr_erase(name,p,keyptr,retbool)\
	QN_STMT_BEGIN{\
		struct name##Node** __rn;\
		_qn_inl_hash_ptr_lookup(name,p,keyptr,__rn);\
		if (*__rn==NULL)\
		{\
			if (retbool)\
			{\
				bool* __c=retbool;\
				*__c=false;\
			}\
		}\
		else\
		{\
			_qn_inl_hash_ptr_erase_node(name,p,&__rn);\
			if (retbool)\
			{\
				bool* __c=retbool;\
				*__c=true;\
			}\
		}\
	}QN_STMT_END

#define _qn_inl_hash_ptr_erase_node(name,p,pppnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=*pppnode;\
		struct name##Node* __enn=*__en;\
		/* step 1 */\
		*__en=__enn->sib;\
		/* step 2 */\
		if (__enn->next)\
			__enn->next->prev=__enn->prev;\
		else\
		{\
			qn_assert((p)->last == __enn, NULL);\
			(p)->last=__enn->prev;\
		}\
		if (__enn->prev)\
			__enn->prev->next=__enn->next;\
		else\
		{\
			qn_assert((p)->frst == __enn, NULL);\
			(p)->frst=__enn->next;\
		}\
		/* step3 */\
		name##_Hash_Cb_PtrKey(__enn->key);\
		name##_Hash_Cb_PtrValue(__enn->value);\
		qn_free(__enn);\
		(p)->count--;\
		(p)->revision++;\
	}QN_STMT_END

#define _qn_inl_hash_ptr_erase_all(name,p)\
	QN_STMT_BEGIN{\
		struct name##Node* __enn;\
		struct name##Node* __enx;\
		for (__enn=(p)->frst; __enn; __enn=__enx)\
		{\
			__enx=__enn->next;\
			name##_Hash_Cb_PtrKey(__enn->key);\
			name##_Hash_Cb_PtrValue(__enn->value);\
			qn_free(__enn);\
		}\
		(p)->frst=(p)->last=NULL;\
		(p)->count=0;\
		memset((p)->nodes, 0, (p)->bucket*sizeof(struct name##Node*));\
	}QN_STMT_END

//
#define _qn_inl_hash_test_size(name,p)\
	QN_STMT_BEGIN{\
		size_t __cnt=(p)->count;\
		size_t __bkt=(p)->bucket;\
		if ((__bkt>=3*__cnt && __bkt>QN_MIN_HASH) ||\
			(3*__bkt<=__cnt && __bkt<QN_MAX_HASH))\
			_qn_inl_hash_resize((qnHash*)p);\
	}QN_STMT_END

QN_INLINE void _qn_inl_hash_resize(qnHash* p)
{
	size_t i, newbucket;
	qnHashNode** newnodes;

	newbucket = qn_primenear((uint32_t)p->count);
	newbucket = QN_CLAMP(newbucket, QN_MIN_HASH, QN_MAX_HASH);
	newnodes = qn_alloc_zero(newbucket, qnHashNode*);
	if (!newnodes)
		return;

	for (i = 0; i < p->bucket; i++)
	{
		qnHashNode* node;
		qnHashNode* next = NULL;
		size_t hashmask;
		for (node = p->nodes[i]; node; node = next)
		{
			next = node->sib;
			hashmask = node->hash % newbucket;
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
typedef struct qnMukumNode
{
	struct qnMukumNode* sib;
	size_t				hash;
	uint32_t* key;
	uint32_t* value;
} qnMukumNode;

typedef struct qnMukum
{
	size_t				revision;
	size_t				bucket;
	size_t				count;
	qnMukumNode** nodes;
} qnMukum;

#define QN_MUKUM_DECL(name,keytype,valuetype)\
	typedef keytype name##Key;\
	typedef valuetype name##Value;\
	typedef struct name##Node { struct name##Node* sib; size_t hash; name##Key key; name##Value value; } name##Node;\
	typedef struct name { size_t revision; size_t bucket; size_t count; struct name##Node** nodes; } name

#define QN_MUKUM_HASH(name,func1)				QN_HASH_HASH(name,func1)
#define QN_MUKUM_HASH_PURE(name)				QN_HASH_HASH_PURE(name)

#define QN_MUKUM_EQ(name,func2)					QN_HASH_EQ(name,func2)
#define QN_MUKUM_EQ_PURE(name)					QN_HASH_EQ_PURE(name)

#define QN_MUKUM_KEY(name,func1)				QN_HASH_KEY(name,func1)
#define QN_MUKUM_KEY_DELETE(name)				QN_HASH_KEY_DELETE(name)
#define QN_MUKUM_KEY_NONE(name)					QN_HASH_KEY_NONE(name)

#define QN_MUKUM_VALUE(name,func1)				QN_HASH_VALUE(name,func1)
#define QN_MUKUM_VALUE_DELETE(name)				QN_HASH_VALUE_DELETE(name)
#define QN_MUKUM_VALUE_NONE(name)				QN_HASH_VALUE_NONE(name)

#define QN_MUKUM_BOTH_NONE(name)				QN_HASH_BOTH_NONE(name)

#define QN_MUKUM_HASHER_INT(name)				QN_HASH_HASHER_INT(name)
#define QN_MUKUM_HASHER_UINT(name)				QN_HASH_HASHER_UINT(name)
#define QN_MUKUM_HASHER_CHAR_PTR(name)			QN_HASH_HASHER_CHAR_PTR(name)
#define QN_MUKUM_HASHER_WCHAR_PTR(name)			QN_HASH_HASHER_WCHAR_PTR(name)
#define QN_MUKUM_HASHER_CHAR_PTR_INCASE(name)	QN_HASH_HASHER_CHAR_PTR_INCASE(name)
#define QN_MUKUM_HASHER_WCHAR_PTR_INCASE(name)	QN_HASH_HASHER_WCHAR_PTR_INCASE(name)

#define QN_MUKUM_PTR_HASH(name,func1)			QN_HASH_PTR_HASH(name,func1)
#define QN_MUKUM_PTR_HASH_PURE(name)			QN_HASH_PTR_HASH_PURE(name)

#define QN_MUKUM_PTR_EQ(name,func2)				QN_HASH_PTR_EQ(name,func2)
#define QN_MUKUM_PTR_EQ_PURE(name)				QN_HASH_PTR_EQ_PURE(name)

#define QN_MUKUM_PTR_KEY(name,func1)			QN_HASH_PTR_KEY(name,func1)
#define QN_MUKUM_PTR_KEY_DELETE(name)			QN_HASH_PTR_KEY_DELETE(name)
#define QN_MUKUM_PTR_KEY_NONE(name)				QN_HASH_PTR_KEY_NONE(name)

#define QN_MUKUM_PTR_VALUE(name,func1)			QN_HASH_PTR_VALUE(name,func1)
#define QN_MUKUM_PTR_VALUE_DELETE(name)			QN_HASH_PTR_VALUE_DELETE(name)
#define QN_MUKUM_PTR_VALUE_NONE(name)			QN_HASH_PTR_VALUE_NONE(name)

#define QN_MUKUM_PTR_BOTH_NONE(name)			QN_HASH_PTR_BOTH_NONE(name)

#define QN_MUKUM_PTR_HASHER_CHAR_PTR(name)		QN_HASH_PTR_HASHER_CHAR_PTR(name)
#define QN_MUKUM_PTR_HASHER_WCHAR_PTR(name)		QN_HASH_PTR_HASHER_WCHAR_PTR(name)

//
#define qn_mukum_count(p)					((p)->count)
#define qn_mukum_bucket(p)					((p)->bucket)
#define qn_mukum_revision(p)				((p)->revision)

#define qn_mukum_init(name,p)\
	QN_STMT_BEGIN{\
		(p)->revision=0;\
		(p)->count=0;\
		(p)->bucket=QN_MIN_HASH;\
		(p)->nodes=qn_alloc_zero(QN_MIN_HASH, struct name##Node*);\
	}QN_STMT_END

//
#define qn_mukum_disp(name,p)\
	QN_STMT_BEGIN{\
		size_t __i;\
		struct name##Node* __node;\
		struct name##Node* __next;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__next)\
			{\
				__next=__node->sib;\
				name##_Hash_Cb_Key(&__node->key);\
				name##_Hash_Cb_Value(&__node->value);\
				qn_free(__node);\
			}\
		qn_free((p)->nodes);\
	}QN_STMT_END

#define qn_mukum_remove_node(name,p,node)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=&(node);\
		_qn_inl_mukum_erase_node(name,p,&__en);\
		_qn_inl_mukum_test_size(name,p);\
	}QN_STMT_END

#define qn_mukum_clear(name,p)\
	QN_STMT_BEGIN{\
		_qn_inl_mukum_erase_all(name,p);\
		_qn_inl_mukum_test_size(name,p);\
	}QN_STMT_END

#define qn_mukum_foreach(name,p,func3,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func3(userdata, &__node->key, &__node->value);\
	}QN_STMT_END

#define qn_mukum_loopeach(name,p,func2)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func2(&__node->key, &__node->value);\
	}QN_STMT_END

#define qn_mukum_foreach_value(name,p,func2,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func2(userdata, &__node->value);\
	}QN_STMT_END

#define qn_mukum_loopeach_value(name,p,func1)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func1(&__node->value);\
	}QN_STMT_END

// value type
#define qn_mukum_node(name,p,keyptr,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __gn;\
		_qn_inl_mukum_lookup(name,p,keyptr,__gn);\
		(retnode)=*__gn;\
	}QN_STMT_END

#define qn_mukum_get(name,p,keyptr,retvalue)\
	QN_STMT_BEGIN{\
		/* retvalue: pointer of 'value' */\
		struct name##Node** __gn;\
		struct name##Node* __node;\
		_qn_inl_mukum_lookup(name,p,keyptr,__gn);\
		__node=*__gn;\
		*(retvalue)=(__node) ? &__node->value : NULL;\
	}QN_STMT_END

#define qn_mukum_add(name,p,keyptr,valueptr)\
	_qn_inl_mukum_set(name,p,keyptr,valueptr,false);

#define qn_mukum_set(name,p,keyptr,valueptr)\
	_qn_inl_mukum_set(name,p,keyptr,valueptr,true);\

#define qn_mukum_remove(name,p,keyptr,retbool_can_be_null)\
	QN_STMT_BEGIN{\
		_qn_inl_mukum_erase(name,p,keyptr,retbool_can_be_null);\
		_qn_inl_mukum_test_size(name,p);\
	}QN_STMT_END

#define qn_mukum_find(name,p,func,userdata,retkey,gotoname)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
		{\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
			{\
				if (func(userdata, &__node->key, &__node->value))\
				{\
					retkey=&__node->key;\
					goto K_CONCAT(pos_mukum_find_exit,gotoname);\
				}\
			}\
		}\
		retkey=NULL;\
	K_CONCAT(pos_mukum_find_exit,gotoname):;\
	}QN_STMT_END

//
#define _qn_inl_mukum_lookup(name,p,keyptr,retnode)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* retnode: pointer's pointer of Node */\
		size_t __lh=name##_Hash_Cb_Hash(keyptr);\
		struct name##Node** __ln=&(p)->nodes[__lh%(p)->bucket];\
		struct name##Node* __lnn;\
		while ((__lnn=*__ln)!=NULL)\
		{\
			if (__lnn->hash==__lh && name##_Hash_Cb_Eq(&__lnn->key, keyptr))\
				break;\
			__ln=&__lnn->sib;\
		}\
		(retnode)=__ln;\
	}QN_STMT_END

#define _qn_inl_mukum_lookup_hash(name,p,keyptr,retnode,rethash)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* retnode: pointer's pointer of Node */\
		/* rethash: pointer of kuint */\
		size_t __lh=name##_Hash_Cb_Hash(keyptr);\
		struct name##Node** __ln=&(p)->nodes[__lh%(p)->bucket];\
		struct name##Node* __lnn;\
		while ((__lnn=*__ln)!=NULL)\
		{\
			if (__lnn->hash==__lh && name##_Hash_Cb_Eq(&__lnn->key, keyptr))\
				break;\
			__ln=&__lnn->sib;\
		}\
		(retnode)=__ln;\
		*(rethash)=__lh;\
	}QN_STMT_END

#define _qn_inl_mukum_set(name,p,keyptr,valueptr,replace)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* valuedata : data of value */\
		/* replace: true=replace original key, false=discard given key */\
		size_t __ah;\
		struct name##Node** __an;\
		struct name##Node* __ann;\
		_qn_inl_mukum_lookup_hash(name,p,keyptr,__an,&__ah);\
		__ann=*__an;\
		if (__ann)\
		{\
			if (replace)\
			{\
				name##_Hash_Cb_Key(&__ann->key);\
				__ann->key=*(keyptr);\
				name##_Hash_Cb_Value(&__ann->value);\
				__ann->value=*(valueptr);\
			}\
			else\
			{\
				name##_Hash_Cb_Key(keyptr);\
				name##_Hash_Cb_Value(valueptr);\
			}\
		}\
		else\
		{\
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
			_qn_inl_mukum_test_size(name,p);\
		}\
	}QN_STMT_END

#define _qn_inl_mukum_erase(name,p,keyptr,retbool)\
	QN_STMT_BEGIN{\
		struct name##Node** __rn;\
		_qn_inl_mukum_lookup(name,p,keyptr,__rn);\
		if (*__rn==NULL)\
		{\
			if (retbool)\
			{\
				bool* __c=retbool;\
				*__c=false;\
			}\
		}\
		else\
		{\
			_qn_inl_mukum_erase_node(name,p,&__rn);\
			if (retbool)\
			{\
				bool* __c=retbool;\
				*__c=true;\
			}\
		}\
	}QN_STMT_END

#define _qn_inl_mukum_erase_node(name,p,pppnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=*pppnode;\
		struct name##Node* __enn=*__en;\
		/* step 1 */\
		*__en=__enn->sib;\
		/* step 2 */\
		name##_Hash_Cb_Key(&__enn->key);\
		name##_Hash_Cb_Value(&__enn->value);\
		qn_free(__enn);\
		(p)->count--;\
		(p)->revision++;\
	}QN_STMT_END

#define _qn_inl_mukum_erase_all(name,p)\
	QN_STMT_BEGIN{\
		size_t __i;\
		struct name##Node* __node;\
		struct name##Node* __next;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__next)\
			{\
				__next=__node->sib;\
				name##_Hash_Cb_Key(&__node->key);\
				name##_Hash_Cb_Value(&__node->value);\
				qn_free(__node);\
			}\
		(p)->count=0;\
		(p)->revision++;\
		memset((p)->nodes, 0, (p)->bucket*sizeof(struct name##Node*));\
	}QN_STMT_END

// 
#define qn_mukum_ptr_disp(name,p)\
	QN_STMT_BEGIN{\
		size_t __i;\
		struct name##Node* __node;\
		struct name##Node* __next;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__next)\
			{\
				__next=__node->sib;\
				name##_Hash_Cb_PtrKey(__node->key);\
				name##_Hash_Cb_PtrValue(__node->value);\
				qn_free(__node);\
			}\
		qn_free((p)->nodes);\
	}QN_STMT_END

#define qn_mukum_ptr_remove_node(name,p,node)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=&(node);\
		_qn_inl_mukum_ptr_erase_node(name,p,&__en);\
		_qn_inl_mukum_test_size(name,p);\
	}QN_STMT_END

#define qn_mukum_ptr_clear(name,p)\
	QN_STMT_BEGIN{\
		_qn_inl_mukum_ptr_erase_all(name,p);\
		_qn_inl_mukum_test_size(name,p);\
	}QN_STMT_END

#define qn_mukum_ptr_foreach(name,p,func3,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func3(userdata, __node->key, __node->value);\
	}QN_STMT_END

#define qn_mukum_ptr_loopeach(name,p,func2)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func2(__node->key, __node->value);\
	}QN_STMT_END

#define qn_mukum_ptr_foreach_value(name,p,func2,userdata)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func2(userdata, __node->value);\
	}QN_STMT_END

#define qn_mukum_ptr_loopeach_value(name,p,func1)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
				func1(__node->value);\
	}QN_STMT_END

// value type
#define qn_mukum_ptr_node(name,p,keyptr,retnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __gn;\
		_qn_inl_mukum_ptr_lookup(name,p,keyptr,__gn);\
		(retnode)=*__gn;\
	}QN_STMT_END

#define qn_mukum_ptr_get(name,p,keyptr,retvalue)\
	QN_STMT_BEGIN{\
		/* retvalue: pointer of 'value' */\
		struct name##Node** __gn;\
		struct name##Node* __node;\
		_qn_inl_mukum_ptr_lookup(name,p,keyptr,__gn);\
		__node=*__gn;\
		*(retvalue)=(__node) ? __node->value : NULL;\
	}QN_STMT_END

#define qn_mukum_ptr_add(name,p,keyptr,valueptr)\
	_qn_inl_mukum_ptr_set(name,p,keyptr,valueptr,false)

#define qn_mukum_ptr_set(name,p,keyptr,valueptr)\
	_qn_inl_mukum_ptr_set(name,p,keyptr,valueptr,true)

#define qn_mukum_ptr_remove(name,p,keyptr,retbool_can_be_null)\
	QN_STMT_BEGIN{\
		_qn_inl_mukum_ptr_erase(name,p,keyptr,retbool_can_be_null);\
		_qn_inl_mukum_test_size(name,p);\
	}QN_STMT_END

#define qn_mukum_ptr_find(name,p,func,userdata,retkey,gotoname)\
	QN_STMT_BEGIN{\
		struct name##Node* __node;\
		size_t __i;\
		for (__i=0; __i<(p)->bucket; __i++)\
		{\
			for (__node=(p)->nodes[__i]; __node; __node=__node->sib)\
			{\
				if (func(userdata, __node->key, __node->value))\
				{\
					*(retkey)=__node->key;\
					goto K_CONCAT(pos_mukum_find_exit,gotoname);\
				}\
			}\
		}\
		retkey=NULL;\
		K_CONCAT(pos_mukum_find_exit,gotoname):;\
	}QN_STMT_END

//
#define _qn_inl_mukum_ptr_lookup(name,p,keyptr,retnode)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* retnode: pointer's pointer of Node */\
		size_t __lh=name##_Hash_Cb_PtrHash(keyptr);\
		struct name##Node** __ln=&(p)->nodes[__lh%(p)->bucket];\
		struct name##Node* __lnn;\
		while ((__lnn=*__ln)!=NULL)\
		{\
			if (__lnn->hash==__lh && name##_Hash_Cb_PtrEq(__lnn->key, keyptr))\
				break;\
			__ln=&__lnn->sib;\
		}\
		(retnode)=__ln;\
	}QN_STMT_END

#define _qn_inl_mukum_ptr_lookup_hash(name,p,keyptr,retnode,rethash)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* retnode: pointer's pointer of Node */\
		/* rethash: pointer of kuint */\
		size_t __lh=name##_Hash_Cb_PtrHash(keyptr);\
		struct name##Node** __ln=&(p)->nodes[__lh%(p)->bucket];\
		struct name##Node* __lnn;\
		while ((__lnn=*__ln)!=NULL)\
		{\
			if (__lnn->hash==__lh && name##_Hash_Cb_PtrEq(__lnn->key, keyptr))\
				break;\
			__ln=&__lnn->sib;\
		}\
		(retnode)=__ln;\
		*(rethash)=__lh;\
	}QN_STMT_END

#define _qn_inl_mukum_ptr_set(name,p,keyptr,valueptr,replace)\
	QN_STMT_BEGIN{\
		/* keyptr: pointer of key */\
		/* valuedata : data of value */\
		/* replace: true=replace original key, false=discard given key */\
		size_t __ah;\
		struct name##Node** __an;\
		struct name##Node* __ann;\
		_qn_inl_mukum_ptr_lookup_hash(name,p,keyptr,__an,&__ah);\
		__ann=*__an;\
		if (__ann)\
		{\
			if (replace)\
			{\
				name##_Hash_Cb_PtrKey(__ann->key);\
				__ann->key=keyptr;\
				name##_Hash_Cb_PtrValue(__ann->value);\
				__ann->value=valueptr;\
			}\
			else\
			{\
				name##_Hash_Cb_PtrKey(keyptr);\
				name##_Hash_Cb_PtrValue(valueptr);\
			}\
		}\
		else\
		{\
			/* step 1*/\
			__ann=qn_alloc_1(struct name##Node);\
			__ann->sib=NULL;\
			__ann->hash=__ah;\
			__ann->key=keyptr;\
			__ann->value=valueptr;\
			/* step 2 */\
			*__an=__ann;\
			(p)->revision++;\
			(p)->count++;\
			/* step 3 */\
			_qn_inl_mukum_test_size(name,p);\
		}\
	}QN_STMT_END

#define _qn_inl_mukum_ptr_erase(name,p,keyptr,retbool)\
	QN_STMT_BEGIN{\
		struct name##Node** __rn;\
		_qn_inl_mukum_ptr_lookup(name,p,keyptr,__rn);\
		if (*__rn==NULL)\
		{\
			if (retbool)\
			{\
				bool* __c=retbool;\
				*__c=false;\
			}\
		}\
		else\
		{\
			_qn_inl_mukum_ptr_erase_node(name,p,&__rn);\
			if (retbool)\
			{\
				bool* __c=retbool;\
				*__c=true;\
			}\
		}\
	}QN_STMT_END

#define _qn_inl_mukum_ptr_erase_node(name,p,pppnode)\
	QN_STMT_BEGIN{\
		struct name##Node** __en=*pppnode;\
		struct name##Node* __enn=*__en;\
		/* step 1 */\
		*__en=__enn->sib;\
		/* step 2 */\
		name##_Hash_Cb_PtrKey(__enn->key);\
		name##_Hash_Cb_PtrValue(__enn->value);\
		qn_free(__enn);\
		(p)->count--;\
		(p)->revision++;\
	}QN_STMT_END

#define _qn_inl_mukum_ptr_erase_all(name,p)\
	QN_STMT_BEGIN{\
		size_t __i;\
		struct name##Node* __node;\
		struct name##Node* __next;\
		for (__i=0; __i<(p)->bucket; __i++)\
			for (__node=(p)->nodes[__i]; __node; __node=__next)\
			{\
				__next=__node->sib;\
				name##_Hash_Cb_PtrKey(__node->key);\
				name##_Hash_Cb_PtrValue(__node->value);\
				qn_free(__node);\
			}\
		(p)->count=0;\
		(p)->revision++;\
		memset((p)->nodes, 0, (p)->bucket*sizeof(struct name##Node*));\
	}QN_STMT_END

//
#define _qn_inl_mukum_test_size(name,p)\
	QN_STMT_BEGIN{\
		size_t __cnt=(p)->count;\
		size_t __bkt=(p)->bucket;\
		if ((__bkt>=3*__cnt && __bkt>QN_MIN_HASH) ||\
			(3*__bkt<=__cnt && __bkt<QN_MAX_HASH))\
			_qn_inl_mukum_resize((qnMukum*)p);\
	}QN_STMT_END

QN_INLINE void _qn_inl_mukum_resize(qnMukum* p)
{
	size_t i, newbucket;
	qnMukumNode** newnodes;

	newbucket = qn_primenear((uint32_t)p->count);
	newbucket = QN_CLAMP(newbucket, QN_MIN_HASH, QN_MAX_HASH);
	newnodes = qn_alloc_zero(newbucket, qnMukumNode*);
	if (!newnodes)
		return;

	for (i = 0; i < p->bucket; i++)
	{
		qnMukumNode* node;
		qnMukumNode* next = NULL;
		size_t hashmask;
		for (node = p->nodes[i]; node; node = next)
		{
			next = node->sib;
			hashmask = node->hash % newbucket;
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
typedef struct qnBstr
{
	size_t			len;
	char			data[];
} qnBstr;

typedef struct qnBstr64
{
	size_t			len;
	char			data[64];
} qnBstr64;

typedef struct qnBstr260
{
	size_t			len;
	char			data[260];
} qnBstr260;

typedef struct qnBstr1k
{
	size_t			len;
	char			data[1024];
} qnBstr1k;

typedef struct qnBstr2k
{
	size_t			len;
	char			data[2048];
} qnBstr2k;

typedef struct qnBstr4k
{
	size_t			len;
	char			data[4096];
} qnBstr4k;

typedef struct qnBstr8k
{
	size_t			len;
	char			data[8192];
} qnBstr8k;

#define qn_bstr_length(p)				((p)->len)
#define qn_bstr_data(p)					((p)->data)
#define qn_bstr_nth(p,n)				((p)->data[(n)])
#define qn_bstr_inv(p,n)				((p)->data[((p)->len)-(n)-1])

#define qn_bstr_test_init(p)\
	qn_assert((p)->len==0 && (p)->data[0]=='\0', NULL)

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
		(p)->len=qn_strfll((p)->data, (p)->len, (p)->len+cnt, ch);\
		(p)->data[(p)->len]='\0';\
	}QN_STMT_END

#define qn_bstr_is_empty(p)\
	((p)->len==0)

#define qn_bstr_is_have(p)\
	((p)->len!=0)

#define qn_bstr_hash(p, igcase)\
	(igcase ? qn_strihash((p)->data) : qn_strhash((p)->data))

#define qn_bstr_compare(p, s, igcase)\
	(igcase ? _stricmp((p)->data, s) : strcmp((p)->data, s))

#define qn_bstr_compare_bstr(p1, p2, igcase)\
	(igcase ? _stricmp((p1)->data, (p2)->data) : strcmp((p1)->data, (p2)->data))

#define qn_bstr_compare_bstr_length(p1, p2, len, igcase)\
	(igcase ? _strnicmp((p1)->data, (p2)->data, len) : strncmp((p1)->data, (p2)->data, len))

#define qn_bstr_format_va(p, fmt, va)\
	QN_STMT_BEGIN{\
		(p)->len=(size_t)qn_vsnprintf((p)->data, QN_COUNTOF((p)->data)-1, fmt, va);\
	}QN_STMT_END

#define qn_bstr_format(p, fmt, ...)\
	_qn_inl_bstr_format(((qnBstr*)(p)), QN_COUNTOF((p)->data)-1, fmt, __VA_ARGS__)
QN_INLINE void _qn_inl_bstr_format(qnBstr* p, size_t size, const char* fmt, ...)
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
	_qn_inl_bstr_append_format(((qnBstr*)(p)), QN_COUNTOF((p)->data)-1, fmt, __VA_ARGS__)
QN_INLINE void _qn_inl_bstr_append_format(qnBstr* p, size_t size, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	p->len = (size_t)qn_vsnprintf(&p->data[p->len], size - p->len, fmt, va);
	va_end(va);
}

#define qn_bstr_upper(p)\
	qn_strupr(((qnBstr*)p)->data, ((qnBstr*)p)->len);

#define qn_bstr_lower(p)\
	qn_strlwr(((qnBstr*)p)->data, ((qnBstr*)p)->len);

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

QN_INLINE int qn_bstr_has_char(cpointer_t p, const char* chars)
{
	char* s = qn_strbrk(((qnBstr*)p)->data, chars);
	return (s) ? (int)(s - ((qnBstr*)p)->data) : -1;
}

QN_INLINE int qn_bstr_find_char(cpointer_t p, size_t at, char ch)
{
	char* s = strchr(((qnBstr*)p)->data + at, ch);
	return (s) ? (int)(s - ((qnBstr*)p)->data) : -1;
}

#define qn_bstr_sub_bstr(p, s, pos, len)\
	_qn_inl_bstr_sub_bstr(((qnBstr*)(p)), QN_COUNTOF((p)->data)-1, (const qnBstr*)(s), pos, len)
QN_INLINE bool _qn_inl_bstr_sub_bstr(qnBstr* p, size_t psize, const qnBstr* s, size_t pos, intptr_t len)
{
	qn_retval_if_fail(pos >= 0, false);
	qn_retval_if_fail(s->len >= pos, false);

	if (len > 0)
		qn_retval_if_fail(s->len >= (pos + len), false);
	else
		len = s->len - pos;

	qn_strmid(p->data, psize, s->data, pos, len);
	p->len = len;
	return true;
}


//////////////////////////////////////////////////////////////////////////
// blue wcs
typedef struct qnBwcs
{
	size_t				len;
	wchar_t				data[];
} qnBwcs;

typedef struct qnBwcs64
{
	size_t				len;
	wchar_t				data[64];
} qnBwcs64;

typedef struct qnBwcs260
{
	size_t				len;
	wchar_t				data[260];
} qnBwcs260;

typedef struct qnBwcs1k
{
	size_t				len;
	wchar_t				data[1024];
} qnBwcs1k;

typedef struct qnBwcs2k
{
	size_t				len;
	wchar_t				data[2048];
} qnBwcs2k;

typedef struct qnBwcs4k
{
	size_t				len;
	wchar_t				data[4096];
} qnBwcs4k;

typedef struct qnBwcs8k
{
	size_t				len;
	wchar_t				data[8192];
} qnBwcs8k;

#define qn_bwcs_length(p)				((p)->len)
#define qn_bwcs_data(p)					((p)->data)
#define qn_bwcs_nth(p,n)				((p)->data[(n)])
#define qn_bwcs_inv(p,n)				((p)->data[((p)->len)-(n)-1])

#define qn_bwcs_test_init(p)\
	qn_assert((p)->len==0 && (p)->data[0]==L'\0', NULL)

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
		(p)->len=qn_wcsfll((p)->data, (p)->len, (p)->len+cnt, ch);\
		(p)->data[(p)->len]=L'\0';\
	}QN_STMT_END

#define qn_bwcs_is_empty(p)\
	((p)->len==0)

#define qn_bwcs_is_have(p)\
	((p)->len!=0)

#define qn_bwcs_hash(p, igcase)\
	(igcase ? qn_wcsihash((p)->data) : qn_wcshash((p)->data))

#define qn_bwcs_compare(p, s, igcase)\
	(igcase ? _wcsicmp((p)->data, s) : wcscmp((p)->data, s))

#define qn_bwcs_compare_bwcs(p1, p2, igcase)\
	(igcase ? _wcsicmp((p1)->data, (p2)->data) : wcscmp((p1)->data, (p2)->data))

#define qn_bwcs_compare_bwcs_length(p1, p2, len, igcase)\
	(igcase ? _wcsnicmp((p1)->data, (p2)->data, len) : wcsncmp((p1)->data, (p2)->data, len))

#define qn_bwcs_format_va(p, fmt, va)\
	QN_STMT_BEGIN{\
		(p)->len=(size_t)qn_vsnwprintf((p)->data, QN_COUNTOF((p)->data)-1, fmt, va);\
	}QN_STMT_END

#define qn_bwcs_format(p, fmt, ...)\
	_qn_inl_bwcs_format(((qnBwcs*)(p)), QN_COUNTOF((p)->data)-1, fmt, __VA_ARGS__)
QN_INLINE void _qn_inl_bwcs_format(qnBwcs* p, size_t size, const wchar_t* fmt, ...)
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
	_qn_inl_bwcs_append_format(((qnBwcs*)(p)), QN_COUNTOF((p)->data)-1, fmt, __VA_ARGS__)
QN_INLINE void _qn_inl_bwcs_append_format(qnBwcs* p, size_t size, const wchar_t* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	p->len = (size_t)qn_vsnwprintf(&p->data[p->len], size - p->len, fmt, va);
	va_end(va);
}

#define qn_bwcs_upper(p)\
	_wcsupr(((qnBwcs*)p)->data);

#define qn_bwcs_lower(p)\
	_wcslwr(((qnBwcs*)p)->data);

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

QN_INLINE int qn_bwcs_has_char(cpointer_t p, const wchar_t* chars)
{
	wchar_t* s = qn_wcsbrk(((qnBwcs*)p)->data, chars);
	return (s) ? (int)(s - ((qnBwcs*)p)->data) : -1;
}

QN_INLINE int qn_bwcs_find_char(cpointer_t p, size_t at, wchar_t ch)
{
	wchar_t* s = wcschr(((qnBwcs*)p)->data + at, ch);
	return (s) ? (int)(s - ((qnBwcs*)p)->data) : -1;
}

#define qn_bwcs_sub_bwcs(p, s, pos, len)\
	_qn_inl_bstr_sub_bstr(((qnBwcs*)(p)), QN_COUNTOF((p)->data)-1, ((qnBwcs*)(s)), pos, len)
QN_INLINE bool _qn_inl_bwcs_sub_bwcs(qnBwcs* p, size_t psize, const qnBwcs* s, size_t pos, intptr_t len)
{
	qn_retval_if_fail(pos >= 0, false);
	qn_retval_if_fail(s->len >= pos, false);

	if (len > 0)
		qn_retval_if_fail(s->len >= (pos + len), false);
	else
		len = s->len - pos;

	qn_wcsmid(p->data, psize, s->data, pos, len);
	p->len = len;
	return true;
}

QN_EXTC_END

#if __GNUC__
#pragma GCC diagnostic pop
#endif

#if _MSC_VER
#pragma warning(pop)
#endif
