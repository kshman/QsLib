#include "pch.h"
#include "qs_qn.h"
#include "qs_ctn.h"

typedef struct QnRealTag QnRealTag;

QN_DECL_ARR(ErrorArray, char*)
QN_DECL_ARR(SubArray, QnRealTag*)
QN_DECL_SLIST(StackList, QnRealTag*)

QN_DECL_HASH(ArgHash, char*, char*)
QN_HASH_CHAR_PTR_KEY(ArgHash)
QN_HASH_KEY_FREE(ArgHash)
QN_HASH_VALUE_FREE(ArgHash)

// 스트링 제거
static void error_array_delete_ptr(char** ptr)
{
	char* s = *ptr;
	qn_free(s);
}

// xml 버전
static const char* ml_header_desc = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";


//////////////////////////////////////////////////////////////////////////
// 실제 노드
struct QnRealTag
{
	QnMlTag				base;

	SubArray			subs;
	ArgHash				args;

	size_t				name_hash;
	size_t				context_hash;

	nint				index;
};

static void qn_realtag_delete_ptr(QnRealTag** ptr);
static bool qn_realtag_parse_args(QnRealTag* self, qnBstr4k* bs);
static bool qn_realtag_write_file(const QnRealTag* self, QnFile* file, int ident);


//////////////////////////////////////////////////////////////////////////
// RML
struct QnMlu
{
	SubArray			tags;
	ErrorArray			errs;

	nint				maxline;
};

//
QnMlu* qn_mlu_new(void)
{
	QnMlu* self = qn_alloc_zero_1(QnMlu);
	return self;
}

//
QnMlu* qn_mlu_new_file(const char* filename)
{
	int size;
	uint8_t* data = (uint8_t*)qn_file_alloc(filename, &size);
	qn_val_if_fail(data, NULL);

	QnMlu* self = qn_mlu_new();
	if (!self)
	{
		qn_free(data);
		return NULL;
	}

	const bool ret = qn_mlu_load_buffer(self, data, size);
	qn_free(data);

	if (!ret)
	{
		qn_free(self);
		return NULL;
	}

	return self;
}

//
QnMlu* qn_mlu_new_file_l(const wchar* filename)
{
	int size;
	uint8_t* data = (uint8_t*)qn_file_alloc_l(filename, &size);
	qn_val_if_fail(data, NULL);

	QnMlu* self = qn_mlu_new();
	if (!self)
	{
		qn_free(data);
		return NULL;
	}

	const bool ret = qn_mlu_load_buffer(self, data, size);
	qn_free(data);

	if (!ret)
	{
		qn_free(self);
		return NULL;
	}

	return self;
}

//
QnMlu* qn_mlu_new_buffer(const void* restrict data, int size)
{
	qn_val_if_fail(data, NULL);
	qn_val_if_fail(size > 0, NULL);

	QnMlu* self = qn_mlu_new();
	qn_val_if_fail(self, NULL);

	qn_mlu_load_buffer(self, data, size);

	return self;
}

//
void qn_mlu_delete(QnMlu* self)
{
	qn_mlu_clean_tags(self);
	qn_mlu_clean_errs(self);

	qn_arr_disp(SubArray, &self->tags);
	qn_arr_disp(ErrorArray, &self->errs);

	qn_free(self);
}

//
void qn_mlu_clean_tags(QnMlu* self)
{
	qn_arr_loopeach(SubArray, &self->tags, qn_realtag_delete_ptr);
	qn_arr_clear(SubArray, &self->tags);
}

//
void qn_mlu_clean_errs(QnMlu* self)
{
	qn_arr_loopeach(ErrorArray, &self->errs, error_array_delete_ptr);
	qn_arr_clear(ErrorArray, &self->errs);
}

//
void qn_mlu_add_err(QnMlu* self, const char* msg)
{
	qn_ret_if_fail(msg);

	char* dup = qn_strdup(msg);
	qn_arr_add(ErrorArray, &self->errs, dup);
}

//
void qn_mlu_add_errf(QnMlu* self, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char* psz = qn_vapsprintf(fmt, va);
	va_end(va);

	qn_arr_add(ErrorArray, &self->errs, psz);
}

//
void qn_mlu_print_err(const QnMlu* self)
{
	for (size_t i = 0; i < qn_arr_count(&self->errs); i++)
	{
		char* psz = qn_arr_nth(&self->errs, i);
#if _QN_WINDOWS_
		OutputDebugStringA(psz);
#else
		puts(psz);
#endif
	}
}

//
void qn_mlu_print(QnMlu* self)
{
	// NOT IMPL
}

//
bool qn_mlu_load_buffer(QnMlu* self, const void* restrict data, int size)
{
	qn_val_if_fail(data, false);
	qn_val_if_fail(size > 0, false);

	qn_mlu_clean_tags(self);
	qn_mlu_clean_errs(self);

	// UTF8 사인 찾기
	int i = (*(const int*)data) & 0x00FFFFFF;
	const char* pos;

	if (i == 0x00BFBBEF)
	{
		// 헤더 만큼 떼기
		pos = (const char*)data + 3;
		size -= 3;
	}
	else
	{
		// 혹시 16비트 유니코드인가?
		i = (*(const uint16_t*)data);

		if (i == 0 || i == 0xFEFF || i == 0xFFFE)
		{
			// 0 = 32비트 유니코드, 아마 UTF32 BE(0x0000FEFF) / 처리할 수 없다
			// 0xFEFF = UTF16 LE / 처리 안함
			// 0xFFFE = UTF16 BE, 또는 UTF32 LE(0xFFFE0000) / 처리 안함
			return false;
		}

		// 사인 없음
		pos = (const char*)data;
	}

	//
	StackList* stack = NULL;
	QnRealTag* tmptag;
	QnRealTag* curtag = NULL;

	char* cd = qn_alloc(size, char);
	qnBstr1k* btag = qn_alloc_1(qnBstr1k);     // 태그는 1k
	qnBstr1k* bname = qn_alloc_1(qnBstr1k);    // 이름은 1k
	qnBstr4k* barg = qn_alloc_1(qnBstr4k);     // 인수는 4k

	int line = 1;
	int cdsize = 0;
	int idn = 0;
	bool iscmt = false;
	bool ret = false;

	for (i = 0; i < size; i++)
	{
		const char ch = pos[i];

		if (ch == '\n')
		{
			line++;

			if (iscmt)
				iscmt = false;

#if 0
			// 옛날 방식 주석 '#' 또는 "//"
			int n = i + 1;

			if (n < size)
			{
				if (pos[n] == '#')
					iscmt = true;
				else if (pos[n] == '/')
				{
					n++;

					if (n < size && pos[n] == '/')
						iscmt = true;
				}
			}
#endif
		}

		if (iscmt)
			continue;

		if (ch == '<')
		{
			// 태그 들어가기
			bool hasclosure = false;
			bool hasintern = false;
			bool hassingle = false;

			i++;

			if (i < size)
			{
				if (pos[i] == '/')
				{
					hasclosure = true;
					i++;
				}
				else if (pos[i] == '?')
				{
					hasclosure = true;
					hasintern = true;
					i++;
				}
				else if (pos[i] == '!' && (i + 2) < size && pos[i + 1] == '-' && pos[i + 2] == '-')
				{
					// 주석
					i += 3;
					int n = size - 2;

					for (; i < n; i++)
					{
						if (pos[i] == '-' && pos[i + 1] == '-' && pos[i + 2] == '>')
						{
							i += 3;
							hasintern = true;
							break;
						}

						if (pos[i] == '\n')
							line++;
					}

					if (!hasintern)
					{
						qn_mlu_add_errf(self, "line#%d, invalid comment.", line);
						goto pos_exit;
					}

					continue;
				}
			}

			// 이름 + 인수
			qn_bstr_clear(btag);

			for (; i < size; i++)
			{
				if (pos[i] == '\n')
					line++;
				/*
				else if (pos[i]=='/')
				hassingle=true;
				*/
				else if (pos[i] == '>')
				{
					if (pos[i - 1] == '/')
					{
						int n = (int)btag->len - 1;
						btag->data[n] = '\0';
						btag->len = n;
						hassingle = true;
					}

					break;
				}
				else
					qn_bstr_append_char(btag, pos[i]);
			}

			// 인수 검사
			if (hasclosure)
			{
				// 닫는애는 인수가 없으므로
				qn_bstr_clear(barg);
				qn_bstr_set_bstr(bname, btag);
				qn_bstr_upper(bname);
			}
			else
			{
				int at = qn_bstr_find_char(btag, 0, ' ');

				if (at < 0)
					at = qn_bstr_find_char(btag, 0, '\n');

				if (at < 0)
				{
					// 이름만 있음
					qn_bstr_clear(barg);
					qn_bstr_set_bstr(bname, btag);
					qn_bstr_upper(bname);
				}
				else
				{
					// 이름과 인수가 있음
					qn_bstr_sub_bstr(barg, btag, at + 1, -1);
					qn_bstr_sub_bstr(bname, btag, 0, at);
					qn_bstr_upper(bname);
				}
			}

			// 이름 검사
			if (bname->len == 0)
			{
				qn_mlu_add_errf(self, "line#%d, invalid tag name.", line);
				goto pos_exit;
			}

			if (hasintern)
			{
				if (qn_bstr_nth(bname, bname->len - 1) != '?')
				{
					qn_mlu_add_errf(self, "line#%d, invalid intern.", line);
					goto pos_exit;
				}

				// 인턴은 그냥 무시
				continue;
			}

			// 닫기 열기 따로
			if (hasclosure)
			{
				// 여기는 닫기
				if (!stack)
				{
					qn_mlu_add_errf(self, "line#%d, invalid tag closure.", line);
					goto pos_exit;
				}

				tmptag = stack->data;
				qn_slist_remove_link(StackList, stack, stack, &stack);

				if (strcmp(tmptag->base.name, bname->data) != 0)
				{
					qn_mlu_add_errf(self, "line#%d, found closure '%s' with entrance '%s'.", line, bname->data, tmptag->base.name);
					goto pos_exit;
				}

				// 성공
				if (cdsize > 0)
				{
					cd[cdsize] = '\0';
					qn_strtrm(cd);

					cdsize = (int)strlen(cd);

					if (cdsize > 0)
						qn_mltag_add_context((QnMlTag*)tmptag, cd, cdsize);

					cdsize = 0;
				}

				// 추가
				if (!stack)
				{
					// 새로 넣기
					curtag = NULL;
					qn_arr_add(SubArray, &self->tags, tmptag);
				}
				else
				{
					// 하부로 넣기
					curtag = stack->data;
					qn_arr_add(SubArray, &curtag->subs, tmptag);
				}
			}
			else
			{
				// 여기는 열기

				// 현재 열려있는 태그와 자료가 있으면 넣는다
				if (curtag && cdsize > 0)
				{
					cd[cdsize] = '\0';
					qn_strrem(cd, " \n\r\t");

					cdsize = (int)strlen(cd);

					if (cdsize > 0)
						qn_mltag_add_context((QnMlTag*)curtag, cd, cdsize);
				}

				// 열기
				curtag = (QnRealTag*)qn_mltag_new(bname->data);
				if (!curtag)
				{
					qn_mlu_add_errf(self, "line#%d, out of memory.", line);
					goto pos_exit;
				}
				curtag->base.line = line;
				curtag->index = idn++;

				// 인수
				if (!qn_realtag_parse_args(curtag, (qnBstr4k*)barg))
				{
					qn_mlu_add_errf(self, "line#%d, invalid argument.", line);
					goto pos_exit;
				}

				// 준비
				if (!hassingle)
				{
					// 단일 태그가 아니면 넣고 준비
					qn_slist_prepend(StackList, stack, curtag, &stack);
				}
				else
				{
					// 단일 태그이면 그냥 넣자
					tmptag = curtag;

					if (!stack)
					{
						// 새로 넣기
						curtag = NULL;
						qn_arr_add(SubArray, &self->tags, tmptag);
					}
					else
					{
						// 하부로 넣기
						tmptag->base.sibling = true;

						curtag = stack->data;
						qn_arr_add(SubArray, &curtag->subs, tmptag);
					}
				}

				// 내용 길이 자름
				cdsize = 0;
			}
		}   // ch=='<'
		else if (ch == '>')
		{
			// 태그 닫기
			const char* psz = curtag && curtag->base.name ? curtag->base.name : "unknown";
			qn_mlu_add_errf(self, "line#%d, invalid tag '%d'.", line, psz);
			goto pos_exit;
		}   // ch=='>'
		else
		{
			// 태그 데이터
			if (curtag)
				cd[cdsize++] = ch;
		}
	}   // for ~size

	if (stack)
	{
		// 움...
		qn_mlu_add_errf(self, "stack has left! (count:%d)", qn_slist_count(stack));
	}
	else
	{
		// 성공
		ret = true;
	}

pos_exit:
	if (!ret)
	{
		if (curtag)
		{
			if (!stack || curtag != stack->data)
				qn_mltag_delete((QnMlTag*)curtag);
		}

		for (StackList* n = stack; n; n = n->next)
			qn_mltag_delete((QnMlTag*)n->data);

		qn_mlu_clean_tags(self);
	}

	qn_slist_disp(StackList, stack);
	qn_free(cd);
	qn_free(btag);
	qn_free(bname);
	qn_free(barg);

	self->maxline = line;

	return ret;
}

//
bool qn_mlu_write_file(const QnMlu* self, const char* filename)
{
	qn_val_if_fail(filename, false);
	qn_val_if_fail(qn_arr_count(&self->tags) > 0, false);

	QnFile* file = qn_file_new(filename, "w");
	qn_val_if_fail(file, false);

	// UTF8 BOM
	const int bom = 0x00BFBBEF;
	qn_file_write(file, &bom, 0, 3);

	// xml 부호
	qn_file_write(file, (const void*)ml_header_desc, 0, (int)strlen(ml_header_desc));

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		const QnRealTag* tag = qn_arr_nth(&self->tags, i);
		qn_realtag_write_file(tag, file, 0);
	}

	qn_file_delete(file);

	return true;
}

//
int qn_mlu_get_count(const QnMlu* self)
{
	return (int)qn_arr_count(&self->tags);
}

//
const char* qn_mlu_get_err(const QnMlu* self, int at)
{
	return at >= 0 && at < (int)qn_arr_count(&self->errs) ? qn_arr_nth(&self->errs, at) : NULL;
}

//
QnMlTag* qn_mlu_get_tag(const QnMlu* self, const char* name)
{
	qn_val_if_fail(name, NULL);

	const size_t hash = qn_strhash(name);
	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->tags, i);

		if (mt->name_hash == hash && strcmp(mt->base.name, name) == 0)
			return (QnMlTag*)mt;
	}

	return NULL;
}

//
QnMlTag* qn_mlu_get_tag_nth(const QnMlu* self, int at)
{
	return at >= 0 && at < (int)qn_arr_count(&self->tags) ? (QnMlTag*)qn_arr_nth(&self->tags, at) : NULL;
}

//
const char* qn_mlu_get_context(const QnMlu* self, const char* name, const char* ifnotexist)
{
	const QnMlTag* tag = qn_mlu_get_tag(self, name);
	return tag ? tag->context : ifnotexist;
}

//
const char* qn_mlu_get_context_nth(const QnMlu* self, int at, const char* ifnotexist)
{
	const QnMlTag* tag = qn_mlu_get_tag_nth(self, at);
	return tag ? tag->context : ifnotexist;
}

//
int qn_mlu_contains(const QnMlu* self, QnMlTag* tag)
{
	int ret;
	qn_arr_contains(SubArray, &self->tags, (QnRealTag*)tag, &ret);
	return ret;
}

//
void qn_mlu_foreach(const QnMlu* self, void(*func)(void*, QnMlTag*), void* userdata)
{
	qn_ret_if_fail(func);

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->tags, i);
		func(userdata, (QnMlTag*)mt);
	}
}

//
void qn_mlu_loopeach(const QnMlu* self, void(*func)(QnMlTag* tag))
{
	qn_ret_if_fail(func);

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->tags, i);
		func((QnMlTag*)mt);
	}
}

//
QnMlTag* qn_mlu_add(QnMlu* self, const char* name, const char* context, int line)
{
	qn_val_if_fail(name, NULL);

	QnRealTag* tag = (QnRealTag*)qn_mltag_new(name);
	qn_val_if_fail(tag, NULL);

	tag->base.line = line;
	tag->index = -1;

	if (context)
		qn_mltag_set_context((QnMlTag*)tag, context, -1);

	qn_arr_add(SubArray, &self->tags, tag);

	return (QnMlTag*)tag;
}

//
QnMlTag* qn_mlu_add_tag(QnMlu* self, QnMlTag* tag)
{
	qn_val_if_fail(tag, NULL);

	qn_arr_add(SubArray, &self->tags, (QnRealTag*)tag);

	return tag;
}

//
int qn_mlu_remove(QnMlu* self, const char* name, bool isall)
{
	qn_val_if_fail(name, -1);

	const size_t hash = qn_strhash(name);
	int cnt = 0;
	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		QnRealTag* tag = qn_arr_nth(&self->tags, i);

		if (tag->name_hash == hash && strcmp(tag->base.name, name) == 0)
		{
			cnt++;

			qn_arr_remove_nth(SubArray, &self->tags, i);
			qn_mltag_delete((QnMlTag*)tag);

			if (isall)
				i--;
			else
				break;
		}
	}

	return cnt;
}

//
bool qn_mlu_remove_nth(QnMlu* self, int at)
{
	if (at < 0 || at >= (int)qn_arr_count(&self->tags))
		return false;

	QnRealTag* tag = qn_arr_nth(&self->tags, at);
	qn_arr_remove_nth(SubArray, &self->tags, at);
	qn_mltag_delete((QnMlTag*)tag);

	return true;
}

//
bool qn_mlu_remove_tag(QnMlu* self, QnMlTag* tag, bool isdelete)
{
	qn_val_if_fail(tag, false);

	const QnRealTag* mt = (QnRealTag*)tag;
	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		if (qn_arr_nth(&self->tags, i) == mt)
		{
			qn_arr_remove_nth(SubArray, &self->tags, i);

			if (isdelete)
				qn_mltag_delete(tag);

			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
// 노드

//
QnMlTag* qn_mltag_new(const char* name)
{
	QnRealTag* self = qn_alloc_zero_1(QnRealTag);
	qn_val_if_fail(self, NULL);

	self->base.name = qn_strdup(name);
	qn_strupr(self->base.name, strlen(self->base.name));

	self->base.name_len = (int)strlen(self->base.name);
	self->name_hash = qn_strhash(self->base.name);

	return (QnMlTag*)self;
}

// 제거
static void qn_realtag_delete_ptr(QnRealTag** ptr)
{
	QnRealTag* self = *ptr;

	qn_free(self->base.name);
	qn_free(self->base.context);

	qn_arr_loopeach(SubArray, &self->subs, qn_realtag_delete_ptr);
	qn_arr_disp(SubArray, &self->subs);

	qn_hash_disp(ArgHash, &self->args);

	qn_free(self);
}

//
void qn_mltag_delete(QnMlTag* self)
{
	QnRealTag* real = (QnRealTag*)self;
	qn_realtag_delete_ptr(&real);
}

//
void qn_mltag_add_context(QnMlTag* ptr, const char* cntx, int size)
{
	qn_ret_if_fail(cntx && *cntx != '\0');

	QnRealTag* self = (QnRealTag*)ptr;

	if (size <= 0)
		size = (int)strlen(cntx);

	if (self->base.context)
	{
		char* psz = qn_strcat(self->base.context, cntx, NULL);
		qn_free(self->base.context);
		self->base.context = psz;
		self->base.context_len = (int)strlen(psz);
		self->context_hash = qn_strhash(psz);
	}
	else
	{
		self->base.context = qn_alloc(size + 1, char);
		qn_ret_if_fail(self->base.context);

		qn_strcpy(self->base.context, size + 1, cntx);
		self->base.context_len = size;
		self->context_hash = qn_strhash(self->base.context);
	}
}

//
void qn_mltag_set_context(QnMlTag* ptr, const char* cntx, int size)
{
	QnRealTag* self = (QnRealTag*)ptr;

	if (self->base.context)
	{
		qn_free(self->base.context);
		self->base.context = NULL;
	}

	qn_mltag_add_context(ptr, cntx, size);
}

// 인수 분석
static bool qn_realtag_parse_args(QnRealTag* self, qnBstr4k* bs)
{
	qn_val_if_fail(bs->len > 0, true);

	for (;;)
	{
		// 키
		const int eq = qn_bstr_find_char(bs, 0, '=');

		if (eq < 0)
		{
			qn_bstr_trim(bs);

			if (bs->len > 0)
			{
				// 값이 없는데 키만 있으면 안됨
				return false;
			}

			break;
		}

		qnBstr1k k, v;
		qn_bstr_sub_bstr(&k, bs, 0, eq);
		qn_bstr_trim(&k);

		qn_bstr_sub_bstr(bs, bs, eq + 1, -1);
		qn_bstr_trim_left(bs);

		// 값
		if (bs->len == 0)
		{
			// 아마도 '='뒤에 값이 없는 듯
			return false;
		}
		else if (qn_bstr_nth(bs, 0) == '"')
		{
			// 이중 인용이면 다음 이중 인용까지
			const int at = qn_bstr_find_char(bs, 1, '"');

			if (at < 0)
			{
				// 없으면 오류
				return false;
			}

			qn_bstr_sub_bstr(&v, bs, 1, at - 1);
			qn_bstr_sub_bstr(bs, bs, at + 1, -1);
		}
		else if (qn_bstr_nth(bs, 0) == '\'')
		{
			// 단일 인용이면 다음 단일 인용까지
			const int at = qn_bstr_find_char(bs, 1, '\'');

			if (at < 0)
			{
				// 없으면 오류
				return false;
			}

			qn_bstr_sub_bstr(&v, bs, 1, at - 1);
			qn_bstr_sub_bstr(bs, bs, at + 1, -1);
		}
		else
		{
			// 다음 공백까지
			const int at = qn_bstr_find_char(bs, 0, ' ');

			if (at < 0)
			{
				// 전체 다
				qn_bstr_set_bstr(&v, bs);
				qn_bstr_trim(&v);

				if (v.len == 0)
				{
					// 아마도 마지막에 '='만 있는 듯
					return false;
				}

				qn_bstr_clear(bs);
			}
			else
			{
				qn_bstr_sub_bstr(&v, bs, 0, at);
				qn_bstr_sub_bstr(bs, bs, at + 1, -1);
			}
		}

		// 넣자
		qn_mltag_set_arg((QnMlTag*)self, k.data, v.data);
	}

	return true;
}

// 인수 파일에 쓰기
static void qn_realtag_write_file_arg(QnFile* file, char** pk, char** pv)
{
	char sz[3] = { 0, };
	const char* k = *pk;
	const char* v = *pv;

	sz[0] = ' ';
	qn_file_write(file, sz, 0, sizeof(char) * 1);
	qn_file_write(file, k, 0, (int)strlen(k));

	sz[0] = '=';
	sz[1] = '"';
	qn_file_write(file, sz, 0, sizeof(char) * 2);
	qn_file_write(file, v, 0, (int)strlen(v));

	qn_file_write(file, sz + 1, 0, sizeof(char) * 1);
}

// 파일에 쓰기
static bool qn_realtag_write_file(const QnRealTag* self, QnFile* file, int ident)
{
	char ch;
	char szident[260];
	qnBstr2k bs;

	qn_strfll(szident, 0, ident, '\t');
	szident[ident] = '\0';

	// 아래 많다. 또는 내용 길다
	const bool isbody = qn_arr_count(&self->subs) > 0 || self->base.context_len > 64 ? true : false;

	if (!isbody)
	{
		if (self->base.context_len > 0)
		{
			if (qn_hash_count(&self->args) == 0)
			{
				qn_bstr_format(&bs, "%s<%s>%s</%s>\n",
					szident, self->base.name, self->base.context, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);
			}
			else
			{
				qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);

				qn_hash_foreach(ArgHash, &self->args, qn_realtag_write_file_arg, file);

				qn_bstr_format(&bs, ">%s</%s>\n", self->base.context, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);
			}
		}
		else
		{
			if (qn_hash_count(&self->args) == 0)
			{
				qn_bstr_format(&bs, "%s<%s/>\n",
					szident, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);
			}
			else
			{
				qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);

				qn_hash_foreach(ArgHash, &self->args, qn_realtag_write_file_arg, file);

				qn_bstr_set(&bs, "/>\n");
				qn_file_write(file, bs.data, 0, (int)bs.len);
			}
		}
	}
	else
	{
		if (qn_hash_count(&self->args) == 0)
		{
			qn_bstr_format(&bs, "%s<%s>\n", szident, self->base.name);
			qn_file_write(file, bs.data, 0, (int)bs.len);
		}
		else
		{
			qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
			qn_file_write(file, bs.data, 0, (int)bs.len);

			qn_hash_foreach(ArgHash, &self->args, qn_realtag_write_file_arg, file);

			qn_bstr_set(&bs, ">\n");
			qn_file_write(file, bs.data, 0, (int)bs.len);
		}

		// 내용
		if (self->base.context_len > 0)
		{
			qn_file_write(file, szident, 0, ident);

			ch = '\t';
			qn_file_write(file, &ch, 0, sizeof(char));

			qn_file_write(file, self->base.context, 0, self->base.context_len);

			ch = '\n';
			qn_file_write(file, &ch, 0, sizeof(char));
		}

		// 자식
		ident++;

		for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
		{
			const QnRealTag* tag = qn_arr_nth(&self->subs, i);
			qn_realtag_write_file(tag, file, ident);
		}

		//
		qn_bstr_format(&bs, "%s</%s>\n", szident, self->base.name);
		qn_file_write(file, bs.data, 0, (int)bs.len);
	}

	return true;
}

//
int qn_mltag_get_sub_count(QnMlTag* ptr)
{
	const QnRealTag* self = (QnRealTag*)ptr;

	return (int)qn_arr_count(&self->subs);
}

//
QnMlTag* qn_mltag_get_sub(QnMlTag* ptr, const char* name)
{
	qn_val_if_fail(name, NULL);

	const QnRealTag* self = (QnRealTag*)ptr;

	const size_t hash = qn_strhash(name);
	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->subs, i);

		if (mt->name_hash == hash && strcmp(mt->base.name, name) == 0)
			return (QnMlTag*)mt;
	}

	return NULL;
}

//
QnMlTag* qn_mltag_get_sub_nth(QnMlTag* ptr, int at)
{
	const QnRealTag* self = (QnRealTag*)ptr;
	return at >= 0 && at < (int)qn_arr_count(&self->subs) ? (QnMlTag*)qn_arr_nth(&self->subs, at) : NULL;
}

//
const char* qn_mltag_get_sub_context(QnMlTag* ptr, const char* name, const char* ifnotexist)
{
	const QnMlTag* tag = qn_mltag_get_sub(ptr, name);
	return tag ? tag->context : ifnotexist;
}

//
const char* qn_mltag_get_sub_context_nth(QnMlTag* ptr, int at, const char* ifnotexist)
{
	const QnMlTag* tag = qn_mltag_get_sub_nth(ptr, at);
	return tag ? tag->context : ifnotexist;
}

//
int qn_mltag_contains_sub(QnMlTag* ptr, QnMlTag* tag)
{
	const QnRealTag* self = (QnRealTag*)ptr;
	int ret;

	qn_arr_contains(SubArray, &self->subs, (QnRealTag*)tag, &ret);

	return ret;
}

//
void qn_mltag_foreach_sub(QnMlTag* ptr, void(*func)(void* userdata, QnMlTag* tag), void* userdata)
{
	qn_ret_if_fail(func);

	const QnRealTag* self = (QnRealTag*)ptr;

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->subs, i);
		func(userdata, (QnMlTag*)mt);
	}
}

//
void qn_mltag_loopeach_sub(QnMlTag* ptr, void(*func)(QnMlTag* tag))
{
	qn_ret_if_fail(func);

	const QnRealTag* self = (QnRealTag*)ptr;

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->subs, i);
		func((QnMlTag*)mt);
	}
}

//
QnMlTag* qn_mltag_add_sub(QnMlTag* ptr, const char* name, const char* context, int line)
{
	qn_val_if_fail(name, NULL);

	QnRealTag* self = (QnRealTag*)ptr;
	QnRealTag* mt = (QnRealTag*)qn_mltag_new(name);
	qn_val_if_fail(mt, NULL);

	mt->base.line = line;
	mt->index = -1;

	if (context)
		qn_mltag_set_context((QnMlTag*)mt, context, -1);

	qn_arr_add(SubArray, &self->subs, mt);

	return (QnMlTag*)mt;
}

//
QnMlTag* qn_mltag_add_sub_tag(QnMlTag* ptr, QnMlTag* tag)
{
	qn_val_if_fail(tag, NULL);

	QnRealTag* self = (QnRealTag*)ptr;

	qn_arr_add(SubArray, &self->subs, (QnRealTag*)tag);

	return tag;
}

//
int qn_mltag_remove_sub(QnMlTag* ptr, const char* name, bool isall)
{
	qn_val_if_fail(name, -1);

	QnRealTag* self = (QnRealTag*)ptr;
	const size_t hash = qn_strhash(name);
	int cnt = 0;

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->subs, i);

		if (mt->name_hash == hash && strcmp(mt->base.name, name) == 0)
		{
			cnt++;

			qn_arr_remove_nth(SubArray, &self->subs, i);
			qn_mltag_delete((QnMlTag*)mt);

			if (isall)
				i--;
			else
				break;
		}
	}

	return cnt;
}

//
bool qn_mltag_remove_sub_nth(QnMlTag* ptr, int at)
{
	QnRealTag* self = (QnRealTag*)ptr;

	if (at < 0 || at >= (int)qn_arr_count(&self->subs))
		return false;

	QnRealTag* mt = qn_arr_nth(&self->subs, at);
	qn_arr_remove_nth(SubArray, &self->subs, at);
	qn_mltag_delete((QnMlTag*)mt);

	return true;
}

//
bool qn_mltag_remove_sub_tag(QnMlTag* ptr, QnMlTag* tag, bool isdelete)
{
	qn_val_if_fail(tag, false);

	QnRealTag* self = (QnRealTag*)ptr;
	const QnRealTag* mt = (QnRealTag*)tag;

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		if (qn_arr_nth(&self->subs, i) == mt)
		{
			qn_arr_remove_nth(SubArray, &self->subs, i);

			if (isdelete)
				qn_mltag_delete(tag);

			return true;
		}
	}

	return false;
}

//
int qn_mltag_get_arity(QnMlTag* ptr)
{
	const QnRealTag* self = (QnRealTag*)ptr;

	return (int)qn_hash_count(&self->args);
}

//
const char* qn_mltag_get_arg(QnMlTag* ptr, const char* name, const char* ifnotexist)
{
	qn_val_if_fail(name, ifnotexist);

	const QnRealTag* self = (QnRealTag*)ptr;
	qn_val_if_fail(qn_hash_count(&self->args) > 0, ifnotexist);

	char** retvalue;
	qn_hash_get(ArgHash, &self->args, name, &retvalue);

	return retvalue ? *retvalue : ifnotexist;
}

//
bool qn_mltag_next_arg(QnMlTag* ptr, void** index, const char** name, const char** data)
{
	qn_val_if_fail(index, false);

	if (*index == (void*)(nint)-1)  // NOLINT
		return false;

	const QnRealTag* self = (QnRealTag*)ptr;
	const struct ArgHashNode* node = !*index ? self->args.frst : (struct ArgHashNode*)*index;

	if (name) *name = node->key;
	if (data) *data = node->value;

	*index = node->next ? node->next : (void*)(intptr_t)-1;  // NOLINT

	return true;
}

//
bool qn_mltag_contains_arg(QnMlTag* ptr, const char* name)
{
	return qn_mltag_get_arg(ptr, name, NULL) != NULL;
}

//
void qn_mltag_foreach_arg(QnMlTag* ptr, void(*func)(void* userdata, char* const* name, char* const* data), void* userdata)
{
	const QnRealTag* self = (QnRealTag*)ptr;

	qn_hash_foreach(ArgHash, &self->args, func, userdata);
}

//
void qn_mltag_loopeach_arg(QnMlTag* ptr, void(*func)(char* const* name, char* const* data))
{
	const QnRealTag* self = (QnRealTag*)ptr;

	qn_hash_loopeach(ArgHash, &self->args, func);
}

//
void qn_mltag_set_arg(QnMlTag* ptr, const char* name, const char* value)
{
	qn_ret_if_fail(name);

	QnRealTag* self = (QnRealTag*)ptr;
	char* dn = qn_strdup(name);
	char* dv = qn_strdup(value);

	qn_strupr(dn, strlen(dn));

	if (!self->args.bucket)
		qn_hash_init(ArgHash, &self->args);

	qn_hash_set(ArgHash, &self->args, dn, dv);
}

//
bool qn_mltag_remove_arg(QnMlTag* ptr, const char* name)
{
	qn_val_if_fail(name, false);
	QnRealTag* self = (QnRealTag*)ptr;
	bool ret;
	qn_hash_remove(ArgHash, &self->args, name, &ret);
	return ret;
}
