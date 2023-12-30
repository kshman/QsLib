// 해시
#include <qs.h>

QN_DECL_HASH(StrHash, char*, char*);
QN_HASH_HASH(StrHash, qn_strhash);
QN_HASH_EQ(StrHash, qn_streqv);
QN_HASH_KEY_FREE(StrHash);
QN_HASH_VALUE_FREE(StrHash);

QN_DECL_HASH(StaticHash, char*, char*);
QN_HASH_CHAR_PTR_KEY(StaticHash);
QN_HASH_KEY_NONE(StaticHash);
QN_HASH_VALUE_NONE(StaticHash);

QN_DECL_HASH(IntHash, int, double);
QN_HASH_INT_KEY(IntHash);
QN_HASH_KEY_NONE(IntHash);
QN_HASH_VALUE_NONE(IntHash);

void str_for_each(void* dummy, char** key, char** value)
{
	qn_outputf("%s -> %s", *key, *value);
}

void int_for_loop(int* key, double* value)
{
	qn_outputf("%d -> %f", *key, *value);
}

int main(void)
{
	qn_runtime();

	char** ps;

	// 스트링 해시
	StrHash shash;
	qn_hash_init(StrHash, &shash);
	qn_hash_set(StrHash, &shash, qn_strdup("test"), qn_strdup("value"));
	qn_hash_set(StrHash, &shash, qn_strdup("bruce"), qn_strdup("kim"));
	qn_hash_foreach(StrHash, &shash, str_for_each, NULL);
	qn_hash_set(StrHash, &shash, qn_strdup("test"), qn_strdup("changed"));
	qn_hash_get(StrHash, &shash, "test", &ps);
	qn_outputf("test -> %s\n", ps == NULL ? "(null)" : *ps);
	qn_hash_disp(StrHash, &shash);

	// 스태틱 해시
	StaticHash thash;
	qn_hash_init(StaticHash, &thash);
	qn_hash_set(StaticHash, &thash, "test", "value");
	qn_hash_set(StaticHash, &thash, "bruce", "kim");
	qn_hash_foreach(StaticHash, &thash, str_for_each, NULL);
	qn_hash_set(StaticHash, &thash, "test", "changed");
	qn_hash_get(StaticHash, &thash, "test", &ps);
	qn_outputf("test -> %s\n", ps == NULL ? "(null)" : *ps);
	qn_hash_disp(StaticHash, &thash);

	// 정수 해시
	IntHash ihash;
	qn_hash_init(IntHash, &ihash);
	qn_hash_set(IntHash, &ihash, 10, 123.0);
	qn_hash_set(IntHash, &ihash, 20, 987.0);
	qn_hash_each(IntHash, &ihash, int_for_loop);
	qn_hash_set(IntHash, &ihash, 20, 999.0);
	double* pi;
	qn_hash_get(IntHash, &ihash, 20, &pi);
	qn_outputf("20 -> %f\n", pi == NULL ? 0.0 : *pi);
	qn_hash_disp(IntHash, &ihash);

	return 0;
}

