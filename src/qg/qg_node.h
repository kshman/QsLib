#pragma once
#define __QG_NODE__

//////////////////////////////////////////////////////////////////////////
// 노드 해시

/// @brief 노드 해시
typedef struct QGNODEMUKUM
{
	size_t				REVISION;
	size_t				BUCKET;
	size_t				COUNT;
	QgNode**			NODES;
	QgNode*				FIRST;
	QgNode*				LAST;
} QgNodeMukum;

/// @brief 초기화
INLINE void qg_node_mukum_init(QgNodeMukum* mukum)
{
	mukum->REVISION = 0;
	mukum->BUCKET = QN_MIN_HASH;
	mukum->COUNT = 0;
	mukum->NODES = qn_alloc_zero(QN_MIN_HASH, QgNode*);
	mukum->FIRST = NULL;
	mukum->LAST = NULL;
}

/// @brief 간단 초기화
INLINE void qg_node_mukum_init_fast(QgNodeMukum* mukum)
{
	qn_assert(mukum->REVISION == 0 && mukum->COUNT == 0 && mukum->NODES == NULL, "cannot use fast init, use just init");
	mukum->BUCKET = QN_MIN_HASH;
	mukum->NODES = qn_alloc_zero(QN_MIN_HASH, QgNode*);
}

INLINE QnPtrCtnr qg_node_mukum_to_ctnr(QgNodeMukum* mukum)
{
	QnPtrCtnr ctnr = { mukum->COUNT, qn_alloc(mukum->COUNT, void*) };
	QgNode** ptr = (QgNode**)qn_ctnr_data(&ctnr);
	for (QgNode* node = mukum->FIRST; node; node = node->NEXT)
		*ptr++ = node;
	return ctnr;
}

/// @brief 해제
INLINE void qg_node_mukum_disp(QgNodeMukum* mukum)
{
	for (QgNode *next, *node = mukum->FIRST; node; node = next)
	{
		next = node->NEXT;
		qn_unload(node);
	}
	qn_free(mukum->NODES);
}

/// @brief 안전 해제
INLINE void qg_node_mukum_disp_safe(QgNodeMukum* mukum)
{
	if (mukum->COUNT > 0)
	{
		QnPtrCtnr ctnr = qg_node_mukum_to_ctnr(mukum);
		size_t i;
		qn_ctnr_foreach(&ctnr, i)
			qn_unload(qn_ctnr_nth(&ctnr, i));
		qn_ctnr_disp(&ctnr);
	}
	qn_free(mukum->NODES);
}

/// @brief 크기 검사
INLINE void qg_internal_node_mukum_test_size(QgNodeMukum* mukum)
{
	if ((mukum->BUCKET >= 3 * mukum->COUNT && mukum->BUCKET > QN_MIN_HASH) ||
		(3 * mukum->BUCKET <= mukum->COUNT && mukum->BUCKET < QN_MAX_HASH))
	{
		size_t pnear = qn_prime_near((uint)mukum->COUNT);
		size_t new_bucket = QN_CLAMP(pnear, QN_MIN_HASH, QN_MAX_HASH);
		QgNode** new_nodes = qn_alloc_zero(new_bucket, QgNode*);
		for (size_t i = 0; i < mukum->BUCKET; ++i)
		{
			QgNode* node = mukum->NODES[i];
			while (node)
			{
				QgNode* next = node->SIBLING;
				size_t hash = node->HASH % new_bucket;
				node->SIBLING = new_nodes[hash];
				new_nodes[hash] = node;
				node = next;
			}
		}
		qn_free(mukum->NODES);
		mukum->NODES = new_nodes;
		mukum->BUCKET = new_bucket;
	}
}

/// @brief 갯수
INLINE size_t qg_node_mukum_count(QgNodeMukum* mukum)
{
	return mukum->COUNT;
}

/// @brief 아이템이 있나
INLINE bool qg_node_mukum_is_have(QgNodeMukum* mukum)
{
	return mukum->COUNT > 0;
}

/// @brief 첫 노드
INLINE void* qg_node_mukum_first(QgNodeMukum* mukum)
{
	return mukum->FIRST;
}

/// @brief 마지막 노드
INLINE void* qg_node_mukum_last(QgNodeMukum* mukum)
{
	return mukum->LAST;
}

/// @brief 모두 삭제
INLINE void qg_node_mukum_clear(QgNodeMukum* mukum)
{
	for (QgNode *next, *node = mukum->FIRST; node; node = next)
	{
		next = node->NEXT;
		qn_unload(node);
	}
	mukum->FIRST = mukum->LAST = NULL;
	mukum->COUNT = 0;
	memset(mukum->NODES, 0, sizeof(QgNode*) * mukum->BUCKET);
	qg_internal_node_mukum_test_size(mukum);
}

/// @brief 해시 룩업
INLINE QgNode** qg_internal_node_mukum_lookup(QgNodeMukum* mukum, size_t hash, const char* name)
{
	QgNode** pnode = &mukum->NODES[hash % mukum->BUCKET];
	QgNode* node;
	while ((node = *pnode) != NULL)
	{
		if (node->HASH == hash && qn_streqv(node->NAME, name))
			break;
		pnode = &node->SIBLING;
	}
	qn_assert(pnode != NULL, "invalid node lookup");
	return pnode;
}

/// @brief 해시 셋
INLINE void qg_internal_node_mukum_input(QgNodeMukum* mukum, QgNode* item, bool replace)
{
	qn_ret_if_ok(item->HASH == 0);
	QgNode** pnode = qg_internal_node_mukum_lookup(mukum, item->HASH, item->NAME);
	QgNode* node = *pnode;
	if (node)
	{
		if (replace)
		{
			item->SIBLING = node->SIBLING;
			item->NEXT = node->NEXT;
			item->PREV = node->PREV;
			if (node->NEXT)
				node->NEXT->PREV = item;
			else
			{
				qn_assert(mukum->LAST == node, "invalid last node");
				mukum->LAST = item;
			}
			if (node->PREV)
				node->PREV->NEXT = item;
			else
			{
				qn_assert(mukum->FIRST == node, "invalid first node");
				mukum->FIRST = item;
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
		if (mukum->FIRST)
			mukum->FIRST->PREV = item;
		else
			mukum->LAST = item;
		item->NEXT = mukum->FIRST;
		item->PREV = NULL;
		item->SIBLING = NULL;
		mukum->FIRST = item;
		mukum->COUNT++;
		mukum->REVISION++;
		qg_internal_node_mukum_test_size(mukum);
	}
}

/// @brief 노드 제거, 링크만 해제하고 노드 자체를 해제하지 않는다
INLINE void qg_internal_node_mukum_unlink(QgNodeMukum* mukum, QgNode** pnode)
{
	QgNode* node = *pnode;
	*pnode = node->SIBLING;
	if (node->NEXT)
		node->NEXT->PREV = node->PREV;
	else
	{
		qn_assert(mukum->LAST == node, "invalid last node");
		mukum->LAST = node->PREV;
	}
	if (node->PREV)
		node->PREV->NEXT = node->NEXT;
	else
	{
		qn_assert(mukum->FIRST == node, "invalid first node");
		mukum->FIRST = node->NEXT;
	}
	size_t hash = node->HASH % mukum->BUCKET;
	if (mukum->NODES[hash] == node)
		mukum->NODES[hash] = node->SIBLING;
	mukum->COUNT--;
	mukum->REVISION++;
}

/// @brief 노드 제거, 실제로 노드를 제거한다!
INLINE bool qg_internal_node_mukum_erase(QgNodeMukum* mukum, size_t hash, const char* name)
{
	QgNode** pnode = qg_internal_node_mukum_lookup(mukum, hash, name);
	if (*pnode == NULL)
		return false;
	qg_internal_node_mukum_unlink(mukum, pnode);
	qn_unloadu(*pnode);
	return true;
}

/// @brief 노드 얻기, 참조 처리 하지 않는다!
INLINE void* qg_node_mukum_get(QgNodeMukum* mukum, const char* name)
{
	size_t hash = qn_strhash(name);
	QgNode** pnode = qg_internal_node_mukum_lookup(mukum, hash, name);
	return *pnode;
}

/// @brief 노드 추가, 참조 처리 하지 않는다!
INLINE void qg_node_mukum_add(QgNodeMukum* mukum, QgNode* node)
{
	qg_internal_node_mukum_input(mukum, node, false);
}

/// @brief 노드 설정, 참조 처리 하지 않는다!
INLINE void qg_node_mukum_set(QgNodeMukum* mukum, QgNode* node)
{
	qg_internal_node_mukum_input(mukum, node, true);
}

/// @brief 노드 제거, 실제 노드를 제거한다!
INLINE void qg_node_mukum_remove(QgNodeMukum* mukum, const char* name)
{
	size_t hash = qn_strhash(name);
	qg_internal_node_mukum_erase(mukum, hash, name);
	qg_internal_node_mukum_test_size(mukum);
}

/// @brief 링크를 제거한다, 노드를 제거하지 않는다! (노드 dispose에서 호출용)
INLINE void qg_node_mukum_unlink(QgNodeMukum* mukum, QgNode* node)
{
	qn_ret_if_ok(node->HASH == 0);
	qg_internal_node_mukum_unlink(mukum, &node);
}

/// @brief 찾기
INLINE void* qg_node_mukum_find(QgNodeMukum* mukum, bool(*func)(void*, void*), void* data)
{
	for (QgNode* node = mukum->FIRST; node; node = node->NEXT)
	{
		if (func(data, node))
			return node;
	}
	return NULL;
}

/// @brief foreach
#define qg_node_mukum_foreach(mukum, node) \
	for ((node) = (mukum)->FIRST; (node); (node) = (node)->NEXT)
