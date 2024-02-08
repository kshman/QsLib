// HFS 테스트
#include <qs.h>

QN_DECLIMPL_INT_PCHAR_HASH(IntHash, int_hash);
QN_DECLIMPL_PCHAR_INT_HASH(StrHash, str_hash);
QN_DECLIMPL_PCHAR_PCHAR_MUKUM(StrMukum, str_mukum);
QN_DECLIMPL_INT_INT_MUKUM(IntMukum, int_mukum);

int main(void)
{
	qn_runtime();

	qn_outputf("\n정수-문자열 해시 테스트");
	IntHash inthash;
	int_hash_init(&inthash);
	int_hash_set(&inthash, 1234, qn_strdup("웬더 샤도 폴스 다운 어폰미"));
	int_hash_set(&inthash, 5678, qn_strdup("잇 콜링미 섬웨인더월"));
	int_hash_set(&inthash, 9999, qn_strdup("Feeling bitter and twisted all alone!"));
	IntHashNode* inthashnode;
	QN_HASH_FOREACH(inthash, inthashnode)
		qn_outputf("%d => %s", inthashnode->KEY, inthashnode->VALUE);
	int_hash_dispose(&inthash);

	qn_outputf("\n문자열-정수 해시 테스트");
	StrHash strhash;
	str_hash_init(&strhash);
	str_hash_set(&strhash, qn_strdup("웬더 샤도 폴스 다운 어폰미"), 1234);
	str_hash_set(&strhash, qn_strdup("잇 콜링미 섬웨인더월"), 5678);
	str_hash_set(&strhash, qn_strdup("Feeling bitter and twisted all alone!"), 9999);
	StrHashNode* strhashnode;
	QN_HASH_FOREACH(strhash, strhashnode)
		qn_outputf("%s => %d", strhashnode->KEY, strhashnode->VALUE);
	str_hash_dispose(&strhash);

	qn_outputf("\n문자열-문자열 묶음 테스트");
	StrMukum strmukum;
	str_mukum_init(&strmukum);
	str_mukum_set(&strmukum, qn_strdup("123"), qn_strdup("456"));
	str_mukum_set(&strmukum, qn_strdup("abc"), qn_strdup("def"));
	str_mukum_set(&strmukum, qn_strdup("000"), qn_strdup("111"));
	StrMukumNode* strmukumnode;
	QN_MUKUM_FOREACH(strmukum, strmukumnode)
		qn_outputf("%s => %s", strmukumnode->KEY, strmukumnode->VALUE);
	str_mukum_set(&strmukum, qn_strdup("000"), qn_strdup("(SET)"));
	str_mukum_remove(&strmukum, "abc");
	QN_MUKUM_FOREACH(strmukum, strmukumnode)
		qn_outputf("%s => %s", strmukumnode->KEY, strmukumnode->VALUE);
	str_mukum_dispose(&strmukum);

	qn_outputf("\n정수-정수 묶음 테스트");
	IntMukum intmukum;
	int_mukum_init(&intmukum);
	int_mukum_set(&intmukum, 123, 456);
	int_mukum_set(&intmukum, 789, 555);
	int_mukum_set(&intmukum, 999, 111);
	IntMukumNode* intmukumnode;
	QN_MUKUM_FOREACH(intmukum, intmukumnode)
		qn_outputf("%d => %d", intmukumnode->KEY, intmukumnode->VALUE);
	int_mukum_set(&intmukum, 999, 000);
	int_mukum_remove(&intmukum, 789);
	QN_MUKUM_FOREACH(intmukum, intmukumnode)
		qn_outputf("%d => %d", intmukumnode->KEY, intmukumnode->VALUE);
	int_mukum_dispose(&intmukum);

	return 0;
}
