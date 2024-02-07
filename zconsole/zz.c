// HFS 테스트
#include <qs.h>

#define TEST_STR 0

bool qn_inteqv(int left, int right)
{
	return left == right;
}

#if TEST_STR
#define KEYTYPE		char*
#define VALUETYPE	char*
#define KEYHASH		qn_strihash
#define KEYEQ		qn_strieqv
#define KEYFREE		qn_mem_free
#define VALUEFREE	qn_mem_free_ptr
#else
#define KEYTYPE		int
#define VALUETYPE	int
#define KEYHASH		(int)
#define KEYEQ		qn_inteqv
#define KEYFREE		(void)
#define VALUEFREE	(void)
#endif
typedef struct QN_CONCAT(TYPENAME, Node) { struct QN_CONCAT(TYPENAME, Node) *SIB, *NEXT, *PREV; size_t HASH; KEYTYPE KEY; VALUETYPE VALUE; } QN_CONCAT(TYPENAME, Node);
typedef struct TYPENAME { size_t COUNT, REVISION, BUCKET; QN_CONCAT(TYPENAME, Node) **NODES, *HEAD, *TAIL; } TYPENAME;
FINLINE void QN_CONCAT(TYPENAME, _init)(TYPENAME *hash)
{
	hash->COUNT = hash->REVISION = 0;
	hash->BUCKET = QN_MIN_HASH;
	hash->NODES = qn_alloc_zero(QN_MIN_HASH, QN_CONCAT(TYPENAME, Node)*);
	hash->HEAD = hash->TAIL = NULL;
}
FINLINE void QN_CONCAT(TYPENAME, _init_fast)(TYPENAME *hash)
{
	qn_debug_assert(hash->REVISION == 0 && hash->COUNT == 0 && hash->NODES == NULL, "cannot use fast init, use just init");
	hash->BUCKET = QN_MIN_HASH;
	hash->NODES = qn_alloc_zero(QN_MIN_HASH, QN_CONCAT(TYPENAME, Node)*);
}
FINLINE void QN_CONCAT(TYPENAME, _dispose)(TYPENAME *hash)
{
	for (QN_CONCAT(TYPENAME, Node) *next, *node = hash->HEAD; node; node = next)
	{
		next = node->NEXT;
		KEYFREE(node->KEY);
		VALUEFREE(&node->VALUE);
		qn_free(node);
	}
	qn_free(hash->NODES);
}
FINLINE size_t QN_CONCAT(TYPENAME, _count)(TYPENAME *hash)
{
	return hash->COUNT;
}
FINLINE size_t QN_CONCAT(TYPENAME, _revision)(TYPENAME *hash)
{
	return hash->REVISION;
}
FINLINE size_t QN_CONCAT(TYPENAME, _bucket)(TYPENAME *hash)
{
	return hash->BUCKET;
}
FINLINE bool QN_CONCAT(TYPENAME, _is_have)(TYPENAME *hash)
{
	return hash->COUNT != 0;
}
FINLINE bool QN_CONCAT(TYPENAME, _is_empty)(TYPENAME *hash)
{
	return hash->COUNT == 0;
}
FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(TYPENAME, _node_head)(TYPENAME *hash)
{
	return hash->HEAD;
}
FINLINE QN_CONCAT(TYPENAME, Node)* QN_CONCAT(TYPENAME, _node_tail)(TYPENAME *hash)
{
	return hash->TAIL;
}
FINLINE void QN_CONCAT(TYPENAME, _internal_test_size)(TYPENAME *hash)
{
	if ((hash->BUCKET >= 3 * hash->COUNT && hash->BUCKET > QN_MIN_HASH) ||
		(3 * hash->BUCKET <= hash->COUNT && hash->BUCKET < QN_MAX_HASH))
	{
		size_t new_bucket = qn_prime_near((uint)hash->COUNT);
		new_bucket = QN_CLAMP(new_bucket, QN_MIN_HASH, QN_MAX_HASH);
		QN_CONCAT(TYPENAME, Node)** new_nodes = qn_alloc_zero(new_bucket, QN_CONCAT(TYPENAME, Node)*);
		for (size_t i = 0; i < hash->BUCKET; ++i)
		{
			QN_CONCAT(TYPENAME, Node)* node = hash->NODES[i];
			while (node)
			{
				QN_CONCAT(TYPENAME, Node)* next = node->SIB;
				const size_t index = node->HASH % new_bucket;
				node->SIB = new_nodes[index];
				new_nodes[index] = node;
				node = next;
			}
		}
		qn_free(hash->NODES);
		hash->NODES = new_nodes;
		hash->BUCKET = new_bucket;
	}
}
FINLINE QN_CONCAT(TYPENAME, Node)** QN_CONCAT(TYPENAME, _internal_lookup)(TYPENAME* hash, KEYTYPE key)
{
	const size_t lh = KEYHASH(key);
	QN_CONCAT(TYPENAME, Node) *lnn, **ln = &hash->NODES[lh % hash->BUCKET];
	while ((lnn = *ln) != NULL)
	{
		if (lnn->HASH == lh && KEYEQ(lnn->KEY, key))
			break;
		ln = &lnn->SIB;
	}
	return ln;
}
FINLINE QN_CONCAT(TYPENAME, Node)** QN_CONCAT(TYPENAME, _internal_lookup_hash)(TYPENAME* hash, KEYTYPE key, size_t* ret_hash)
{
	const size_t lh = KEYHASH(key);
	QN_CONCAT(TYPENAME, Node) *lnn, **ln = &hash->NODES[lh % hash->BUCKET];
	while ((lnn = *ln) != NULL)
	{
		if (lnn->HASH == lh && KEYEQ(lnn->KEY, key))
			break;
		ln = &lnn->SIB;
	}
	*ret_hash = lh;
	return ln;
}
FINLINE void QN_CONCAT(TYPENAME, _internal_set)(TYPENAME* hash, KEYTYPE key, VALUETYPE* valueptr, bool replace)
{
	size_t ah;
	QN_CONCAT(TYPENAME, Node)** an = QN_CONCAT(TYPENAME, _internal_lookup_hash)(hash, key, &ah);
	QN_CONCAT(TYPENAME, Node)* ann = *an;
	if (ann != NULL)
	{
		KEYFREE(ann->KEY);
		VALUEFREE(&ann->VALUE);
		if (replace)
		{
			ann->KEY = key;
			ann->VALUE = *valueptr;
		}
	}
	else
	{
		ann = qn_alloc_1(QN_CONCAT(TYPENAME, Node));
		ann->SIB = NULL;
		ann->HASH = ah;
		ann->KEY = key;
		ann->VALUE = *valueptr;
		ann->NEXT = NULL;
		ann->PREV = hash->TAIL;
		if (hash->TAIL)
			hash->TAIL->NEXT = ann;
		else
			hash->HEAD = ann;
		hash->TAIL = ann;
		*an = ann;
		hash->REVISION++;
		hash->COUNT++;
		QN_CONCAT(TYPENAME, _internal_test_size)(hash);
	}
}
FINLINE void QN_CONCAT(TYPENAME, _internal_erase_node)(TYPENAME* hash, QN_CONCAT(TYPENAME, Node)** en)
{
	QN_CONCAT(TYPENAME, Node)* enn = *en;
	*en = enn->SIB;
	if (enn->PREV)
		enn->PREV->NEXT = enn->NEXT;
	else
		hash->HEAD = enn->NEXT;
	if (enn->NEXT)
		enn->NEXT->PREV = enn->PREV;
	else
		hash->TAIL = enn->PREV;
	const size_t ebk = enn->HASH % hash->BUCKET;
	if (hash->NODES[ebk] == enn)
		hash->NODES[ebk] = NULL;
	KEYFREE(enn->KEY);
	VALUEFREE(&enn->VALUE);
	qn_free(enn);
	hash->REVISION++;
	hash->COUNT--;
}
FINLINE bool QN_CONCAT(TYPENAME, _internal_erase)(TYPENAME* hash, KEYTYPE key)
{
	QN_CONCAT(TYPENAME, Node)** rn = QN_CONCAT(TYPENAME, _internal_lookup)(hash, key);
	if (*rn == NULL)
		return false;
	QN_CONCAT(TYPENAME, _internal_erase_node)(hash, rn);
	return true;
}
FINLINE void QN_CONCAT(TYPENAME, _internal_erase_all)(TYPENAME* hash)
{
	for (QN_CONCAT(TYPENAME, Node) *next, *node = hash->HEAD; node; node = next)
	{
		next = node->NEXT;
		KEYFREE(node->KEY);
		VALUEFREE(&node->VALUE);
		qn_free(node);
	}
	hash->HEAD = hash->TAIL = NULL;
	hash->COUNT = 0;
	memset(hash->NODES, 0, hash->BUCKET * sizeof(QN_CONCAT(TYPENAME, Node)*));
}
FINLINE void QN_CONCAT(TYPENAME, _clear)(TYPENAME* hash)
{
	QN_CONCAT(TYPENAME, _internal_erase_all)(hash);
	QN_CONCAT(TYPENAME, _internal_test_size)(hash);
}
FINLINE void QN_CONCAT(TYPENAME, _remove_node)(TYPENAME* hash, QN_CONCAT(TYPENAME, Node)* node)
{
	QN_CONCAT(TYPENAME, _internal_erase_node)(hash, &node);
	QN_CONCAT(TYPENAME, _internal_test_size)(hash);
}
FINLINE VALUETYPE* QN_CONCAT(TYPENAME, _get)(TYPENAME* hash, KEYTYPE key)
{
	QN_CONCAT(TYPENAME, Node)** gn = QN_CONCAT(TYPENAME, _internal_lookup)(hash, key);
	return *gn ? &(*gn)->VALUE : NULL;
}
FINLINE void QN_CONCAT(TYPENAME, _add)(TYPENAME* hash, KEYTYPE key, VALUETYPE value)
{
	QN_CONCAT(TYPENAME, _internal_set)(hash, key, &value, false);
}
FINLINE void QN_CONCAT(TYPENAME, _set)(TYPENAME* hash, KEYTYPE key, VALUETYPE value)
{
	QN_CONCAT(TYPENAME, _internal_set)(hash, key, &value, true);
}
FINLINE void QN_CONCAT(TYPENAME, _add_ptr)(TYPENAME* hash, KEYTYPE key, VALUETYPE* value)
{
	QN_CONCAT(TYPENAME, _internal_set)(hash, key, value, false);
}
FINLINE void QN_CONCAT(TYPENAME, _set_ptr)(TYPENAME* hash, KEYTYPE key, VALUETYPE* value)
{
	QN_CONCAT(TYPENAME, _internal_set)(hash, key, value, true);
}
FINLINE bool QN_CONCAT(TYPENAME, _remove)(TYPENAME* hash, KEYTYPE key)
{
	if (QN_CONCAT(TYPENAME, _internal_erase)(hash, key) == false)
		return false;
	QN_CONCAT(TYPENAME, _internal_test_size)(hash);
	return true;
}
FINLINE KEYTYPE QN_CONCAT(TYPENAME, _find)(TYPENAME* hash, bool(*func)(void*, KEYTYPE, void*), void* context)
{
	for (QN_CONCAT(TYPENAME, Node) *node = hash->HEAD; node; node = node->NEXT)
	{
		if (func(context, node->KEY, &node->VALUE))
			return node->KEY;
	}
	return (KEYTYPE)0;
}
FINLINE void QN_CONCAT(TYPENAME, _foreach_2)(TYPENAME* hash, void(*func)(KEYTYPE, void*))
{
	for (QN_CONCAT(TYPENAME, Node) *node = hash->HEAD; node; node = node->NEXT)
		func(node->KEY, &node->VALUE);
}
FINLINE void QN_CONCAT(TYPENAME, _foreach_3)(TYPENAME* hash, void(*func)(void*, KEYTYPE, void*), void* context)
{
	for (QN_CONCAT(TYPENAME, Node) *node = hash->HEAD; node; node = node->NEXT)
		func(context, node->KEY, &node->VALUE);
}

#define HASH_FOREACH(hash, node)\
	QN_CONCAT(TYPENAME, Node) *QN_CONCAT(NEXT,__LINE__);\
	for ((node) = (hash).HEAD; (node) && (QN_CONCAT(NEXT,__LINE__)=(node)->NEXT, true); (node) = QN_CONCAT(NEXT,__LINE__))

QN_DECLIMPL_INT_PCHAR_HASH(IntHash, int_hash);

int main(void)
{
	qn_runtime();

	TYPENAME hash;
	TYPENAME_init(&hash);
#if TEST_STR
	TYPENAME_set(&hash, qn_strdup("123"), qn_strdup("456"));
	TYPENAME_set(&hash, qn_strdup("abc"), qn_strdup("def"));
	TYPENAME_set(&hash, qn_strdup("000"), qn_strdup("111"));
	TYPENAMENode* node;
	HASH_FOREACH(hash, node)
		qn_outputf("%s => %s", node->KEY, node->VALUE);
	TYPENAME_set(&hash, qn_strdup("000"), qn_strdup("(SET)"));
	HASH_FOREACH(hash, node)
		qn_outputf("%s => %s", node->KEY, node->VALUE);
#else
	TYPENAME_set(&hash, 123, 456);
	TYPENAME_set(&hash, 789, 555);
	TYPENAME_set(&hash, 999, 111);
	TYPENAMENode* node;
	HASH_FOREACH(hash, node)
		qn_outputf("%d => %d", node->KEY, node->VALUE);
	TYPENAME_set(&hash, 999, 000);
	HASH_FOREACH(hash, node)
		qn_outputf("%d => %d", node->KEY, node->VALUE);
#endif
	TYPENAME_dispose(&hash);

	IntHash inthash;
	int_hash_init(&inthash);
	int_hash_set(&inthash, 1234, qn_strdup("웬더 샤도 폴스 다운 어폰미"));
	int_hash_set(&inthash, 5678, qn_strdup("잇 콜링미 섬웨인더월"));
	int_hash_set(&inthash, 9999, qn_strdup("Feeling bitter and twisted all alone!"));
	IntHashNode* inthashnode;
	QN_HASH_FOREACH(inthash, inthashnode)
		qn_outputf("%d => %s", inthashnode->KEY, inthashnode->VALUE);
	int_hash_dispose(&inthash);

	return 0;
}
