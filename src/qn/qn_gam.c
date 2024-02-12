//
// qn_gam.c - 감 잡는 QnGamBase
// 2023-12-27 by kim
//

#include "pch.h"

//////////////////////////////////////////////////////////////////////////
// QNGAM

// 만들었슴
QnGam qn_sc_init(QnGam g, const void* vt)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	base->vt = vt;
	base->ref = 1;
	return g;
}

// 로드
QnGam qn_sc_load(QnGam g)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	base->ref++;
	return g;
}

// 언로드
QnGam qn_sc_unload(QnGam g)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	const volatile int ref = (int)--base->ref;
	if (ref != 0)
	{
		qn_debug_assert(ref > 0, "invalid reference value!");
		return g;
	}
	base->vt->dispose(g);
	return NULL;
}

//
nint qn_sc_get_ref(const QnGam g)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	return base->ref;
}

//
nuint qn_sc_get_desc(const QnGam g)
{
	const QnGamBase* base = qn_cast_type(g, QnGamBase);
	return base->desc;
}

//
nuint qn_sc_set_desc(QnGam g, const nuint ptr)
{
	QnGamBase* base = qn_cast_type(g, QnGamBase);
	const nuint prev = base->desc;
	base->desc = ptr;
	return prev;
}


//////////////////////////////////////////////////////////////////////////
// QNGAMNODE & QNNODEMUKUM

//
void qn_node_set_name(QnGamNode* self, const char* name)
{
	if (name)
	{
		qn_strncpy(self->NAME, name, QN_COUNTOF(self->NAME) - 1);
		self->HASH = qn_strhash(self->NAME);
	}
	else
	{
		size_t i = qn_p_index();
		qn_snprintf(self->NAME, QN_COUNTOF(self->NAME), "node_%zu", i);
		// 이름 없는 노드는 관리하지 않으므로 해시가 없다
		self->HASH = 0;
	}
}

QN_IMPL_CTNR(QnPtrCtnr, pointer_t, qn_pctnr);

/// @brief 초기화
void qn_node_mukum_init(QnNodeMukum* mukum)
{
	mukum->REVISION = 0;
	mukum->BUCKET = QN_MIN_HASH;
	mukum->COUNT = 0;
	mukum->NODES = qn_alloc_zero(QN_MIN_HASH, QnGamNode*);
	mukum->HEAD = NULL;
	mukum->TAIL = NULL;
}

/// @brief 간단 초기화
void qn_node_mukum_init_fast(QnNodeMukum* mukum)
{
	qn_debug_assert(mukum->REVISION == 0 && mukum->COUNT == 0 && mukum->NODES == NULL, "cannot use fast init, use just init");
	mukum->BUCKET = QN_MIN_HASH;
	mukum->NODES = qn_alloc_zero(QN_MIN_HASH, QnGamNode*);
}

//
QnPtrCtnr qn_node_mukum_to_ctnr(const QnNodeMukum* mukum)
{
	QnPtrCtnr ctnr = { mukum->COUNT, qn_alloc(mukum->COUNT, void*) };
	QnGamNode** ptr = (QnGamNode**)qn_pctnr_data(&ctnr);
	for (QnGamNode* node = mukum->HEAD; node; node = node->NEXT)
		*ptr++ = node;
	return ctnr;
}

/// @brief 해제
void qn_node_mukum_dispose(QnNodeMukum* mukum)
{
	for (QnGamNode *next, *node = mukum->HEAD; node; node = next)
	{
		next = node->NEXT;
		qn_unload(node);
	}
	qn_free(mukum->NODES);
}

/// @brief 안전 해제
void qn_node_mukum_safe_dispose(QnNodeMukum* mukum)
{
	if (mukum->COUNT > 0)
	{
		QnPtrCtnr ctnr = qn_node_mukum_to_ctnr(mukum);
		size_t i;
		QN_CTNR_FOREACH(ctnr, 0, i)
			qn_unload(qn_pctnr_nth(&ctnr, i));
		qn_pctnr_dispose(&ctnr);
	}
	qn_free(mukum->NODES);
}

/// @brief 크기 검사
static void qg_internal_node_mukum_test_size(QnNodeMukum* mukum)
{
	if ((mukum->BUCKET >= 3 * mukum->COUNT && mukum->BUCKET > QN_MIN_HASH) ||
		(3 * mukum->BUCKET <= mukum->COUNT && mukum->BUCKET < QN_MAX_HASH))
	{
		size_t new_bucket = qn_prime_near((uint)mukum->COUNT);
		new_bucket = QN_CLAMP(new_bucket, QN_MIN_HASH, QN_MAX_HASH);
		QnGamNode** new_nodes = qn_alloc_zero(new_bucket, QnGamNode*);
		for (size_t i = 0; i < mukum->BUCKET; ++i)
		{
			QnGamNode* node = mukum->NODES[i];
			while (node)
			{
				QnGamNode* next = node->SIB;
				const size_t hash = node->HASH % new_bucket;
				node->SIB = new_nodes[hash];
				new_nodes[hash] = node;
				node = next;
			}
		}
		qn_free(mukum->NODES);
		mukum->NODES = new_nodes;
		mukum->BUCKET = new_bucket;
	}
}

/// @brief 모두 삭제
void qn_node_mukum_clear(QnNodeMukum* mukum)
{
	for (QnGamNode *next, *node = mukum->HEAD; node; node = next)
	{
		next = node->NEXT;
		qn_unload(node);
	}
	mukum->HEAD = mukum->TAIL = NULL;
	mukum->COUNT = 0;
	memset(mukum->NODES, 0, sizeof(QnGamNode*) * mukum->BUCKET);
	qg_internal_node_mukum_test_size(mukum);
}

/// @brief 해시 룩업
static QnGamNode** qg_internal_node_mukum_lookup(const QnNodeMukum* mukum, size_t hash, const char* name)
{
	QnGamNode** pnode = &mukum->NODES[hash % mukum->BUCKET];
	QnGamNode* node;
	while ((node = *pnode) != NULL)
	{
		if (node->HASH == hash && qn_streqv(node->NAME, name))
			break;
		pnode = &node->SIB;
	}
	qn_debug_assert(pnode != NULL, "invalid node lookup");
	return pnode;
}

/// @brief 해시 셋
static void qg_internal_node_mukum_input(QnNodeMukum* mukum, QnGamNode* item, bool replace)
{
	qn_return_on_ok(item->HASH == 0, /*void*/);
	QnGamNode** pnode = qg_internal_node_mukum_lookup(mukum, item->HASH, item->NAME);
	QnGamNode* node = *pnode;
	if (node)
	{
		if (replace)
		{
			item->SIB = node->SIB;
			item->NEXT = node->NEXT;
			item->PREV = node->PREV;
			if (node->NEXT)
				node->NEXT->PREV = item;
			else
			{
				qn_debug_assert(mukum->TAIL == node, "invalid last node");
				mukum->TAIL = item;
			}
			if (node->PREV)
				node->PREV->NEXT = item;
			else
			{
				qn_debug_assert(mukum->HEAD == node, "invalid first node");
				mukum->HEAD = item;
			}
			*pnode = item;
			qn_unload(node);
		}
		else
		{
			// 헐 지워야하나
			qn_unload(item);
		}
	}
	else
	{
		*pnode = item;
		if (mukum->HEAD)
			mukum->HEAD->PREV = item;
		else
			mukum->TAIL = item;
		item->NEXT = mukum->HEAD;
		item->PREV = NULL;
		item->SIB = NULL;
		mukum->HEAD = item;
		mukum->COUNT++;
		mukum->REVISION++;
		qg_internal_node_mukum_test_size(mukum);
	}
}

/// @brief 노드 제거, 링크만 해제하고 노드 자체를 해제하지 않는다
static void qg_internal_node_mukum_unlink(QnNodeMukum* mukum, QnGamNode** pnode)
{
	const QnGamNode* node = *pnode;
	*pnode = node->SIB;
	if (node->NEXT)
		node->NEXT->PREV = node->PREV;
	else
	{
		qn_debug_assert(mukum->TAIL == node, "invalid last node");
		mukum->TAIL = node->PREV;
	}
	if (node->PREV)
		node->PREV->NEXT = node->NEXT;
	else
	{
		qn_debug_assert(mukum->HEAD == node, "invalid first node");
		mukum->HEAD = node->NEXT;
	}
	const size_t hash = node->HASH % mukum->BUCKET;
	if (mukum->NODES[hash] == node)
		mukum->NODES[hash] = node->SIB;
	mukum->COUNT--;
	mukum->REVISION++;
}

/// @brief 노드 제거, 실제로 노드를 제거한다!
static bool qg_internal_node_mukum_erase(QnNodeMukum* mukum, size_t hash, const char* name)
{
	QnGamNode** pnode = qg_internal_node_mukum_lookup(mukum, hash, name);
	if (*pnode == NULL)
		return false;
	qg_internal_node_mukum_unlink(mukum, pnode);
	qn_unloadu(*pnode);
	return true;
}

/// @brief 노드 얻기, 참조 처리 하지 않는다!
void* qn_node_mukum_get(const QnNodeMukum* mukum, const char* name)
{
	const size_t hash = qn_strhash(name);
	QnGamNode** pnode = qg_internal_node_mukum_lookup(mukum, hash, name);
	return *pnode;
}

/// @brief 노드 추가, 참조 처리 하지 않는다!
void qn_node_mukum_add(QnNodeMukum* mukum, QnGamNode* node)
{
	qg_internal_node_mukum_input(mukum, node, false);
}

/// @brief 노드 설정, 참조 처리 하지 않는다!
void qn_node_mukum_set(QnNodeMukum* mukum, QnGamNode* node)
{
	qg_internal_node_mukum_input(mukum, node, true);
}

/// @brief 노드 제거, 실제 노드를 제거한다!
void qn_node_mukum_remove(QnNodeMukum* mukum, const char* name)
{
	const size_t hash = qn_strhash(name);
	qg_internal_node_mukum_erase(mukum, hash, name);
	qg_internal_node_mukum_test_size(mukum);
}

/// @brief 링크를 제거한다, 노드를 제거하지 않는다! (노드 dispose에서 호출용)
void qn_node_mukum_unlink(QnNodeMukum* mukum, QnGamNode* node)
{
	qn_return_on_ok(node->HASH == 0, /*void*/);
	qg_internal_node_mukum_unlink(mukum, &node);
}

/// @brief 찾기
void* qn_node_mukum_find(const QnNodeMukum* mukum, eqcfunc_t func, void* data)
{
	for (QnGamNode* node = mukum->HEAD; node; node = node->NEXT)
	{
		if (func(data, node))
			return node;
	}
	return NULL;
}
