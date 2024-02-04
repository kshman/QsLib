//
// qn_mlu.c - 마크업 언어 유니트 (Markup Language Unit)
// 2023-12-27 by kim
//

#include "pch.h"

typedef struct QNREALTAG QnRealTag;

QN_DECL_ARR(ErrorArray, char*);
QN_DECL_ARR(SubArray, QnRealTag*);
QN_DECL_SLIST(StackList, QnRealTag*);

QN_DECL_HASH(ArgHash, char*, char*);
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
struct QNREALTAG
{
	QnMlTag				base;

	SubArray			subs;
	ArgHash				args;

	size_t				name_hash;
	size_t				context_hash;

	nint				index;
};

static void _qn_realtag_delete_ptr(QnRealTag** ptr);
static bool _qn_realtag_parse_args(QnRealTag* self, QnBstr4k* bs);
static bool _qn_realtag_write_file(const QnRealTag* self, QnStream* file, int ident);


//////////////////////////////////////////////////////////////////////////
// RML
struct QNMLU
{
	QnGamBase				base;
	SubArray			tags;
	ErrorArray			errs;

	nint				maxline;
};

//
static void qn_mlu_dispose(QnGamBase* gam);
static bool qn_create_mlu_from_buffer(QnMlu* self, const char* RESTRICT data, const int size);

//
QnMlu* qn_create_mlu(void)
{
	QnMlu* self = qn_alloc_zero_1(QnMlu);
	static QN_DECL_VTABLE(QNGAMBASE) qn_mlu_vt =
	{
		"MLU",
		qn_mlu_dispose,
	};
	return qn_gam_init(self, qn_mlu_vt);
}

//
QnMlu* qn_open_mlu(QnMount* mount, const char* RESTRICT filename)
{
	int size;
	char* data = qn_file_alloc_text(mount, filename, &size, NULL);
	qn_return_when_fail(data, NULL);

	QnMlu* self = qn_create_mlu();
	if (!self)
	{
		qn_free(data);
		return NULL;
	}

	const bool ret = qn_create_mlu_from_buffer(self, data, size);
	qn_free(data);

	if (!ret)
	{
		qn_free(self);
		return NULL;
	}

	return self;
}

//
QnMlu* qn_create_mlu_buffer(const void* RESTRICT data, const int size)
{
	qn_return_when_fail(data, NULL);
	qn_return_when_fail(size > 0, NULL);

	QnMlu* self = qn_create_mlu();
	qn_return_when_fail(self, NULL);

	qn_create_mlu_from_buffer(self, data, size);

	return self;
}

//
static void qn_mlu_dispose(QnGamBase* gam)
{
	QnMlu* self = (QnMlu*)gam;
	qn_mlu_clean_tags(self);
	qn_mlu_clean_errs(self);

	qn_arr_disp(&self->tags);
	qn_arr_disp(&self->errs);

	qn_free(self);
}

//
void qn_mlu_clean_tags(QnMlu* self)
{
	qn_arr_foreach_ptr_1(&self->tags, _qn_realtag_delete_ptr);
	qn_arr_clear(&self->tags);
}

//
void qn_mlu_clean_errs(QnMlu* self)
{
	qn_arr_foreach_ptr_1(&self->errs, error_array_delete_ptr);
	qn_arr_clear(&self->errs);
}

//
void qn_mlu_add_err(QnMlu* self, const char* RESTRICT msg)
{
	qn_return_when_fail(msg,/*void*/);

	char* dup = qn_strdup(msg);
	qn_arr_add(ErrorArray, &self->errs, dup);
}

//
void qn_mlu_add_errf(QnMlu* self, const char* RESTRICT fmt, ...)
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
void qn_mlu_print(const QnMlu* self)
{
	// NOT IMPL
	(void)self;
}

//
static bool qn_create_mlu_from_buffer(QnMlu* self, const char* RESTRICT data, const int size)
{
	qn_return_when_fail(data, false);
	qn_return_when_fail(size > 0, false);

	qn_mlu_clean_tags(self);
	qn_mlu_clean_errs(self);

	const char* pos = data;
	int len = size;

	//
	StackList* stack = NULL;
	QnRealTag* tmptag;
	QnRealTag* curtag = NULL;

	char* cd = qn_alloc(len, char);
	QnBstr1k* btag = qn_alloc_1(QnBstr1k);     // 태그는 1k
	QnBstr1k* bname = qn_alloc_1(QnBstr1k);    // 이름은 1k
	QnBstr4k* barg = qn_alloc_1(QnBstr4k);     // 인수는 4k

	int line = 1;
	int cdsize = 0;
	int idn = 0;
	bool iscmt = false;
	bool ret = false;

	for (int i = 0; i < len; i++)
	{
		const char ch = pos[i];

		if (ch == '\n')
		{
			line++;

			if (iscmt)
				iscmt = false;

#if false
			// 옛날 방식 주석 '#' 또는 "//"
			int n = i + 1;

			if (n < len)
			{
				if (pos[n] == '#')
					iscmt = true;
				else if (pos[n] == '/')
				{
					n++;

					if (n < len && pos[n] == '/')
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

			if (i < len)
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
				else if (pos[i] == '!' && (i + 2) < len && pos[i + 1] == '-' && pos[i + 2] == '-')
				{
					// 주석
					i += 3;
					for (int n = len - 2; i < n; i++)
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

			for (; i < len; i++)
			{
				if (pos[i] == '\n')
					line++;
				/*
				else if (pos[i]=='/')
				has_single=true;
				*/
				else if (pos[i] == '>')
				{
					if (pos[i - 1] == '/')
					{
						int n = (int)qn_bstr_length(btag) - 1;
						qn_bstr_nth(btag, n) = '\0';
						qn_bstr_length(btag) = (size_t)n;
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
					qn_bstr_sub_bstr(barg, btag, (size_t)(at + 1), -1);
					qn_bstr_sub_bstr(bname, btag, 0, at);
					qn_bstr_upper(bname);
				}
			}

			// 이름 검사
			if (qn_bstr_length(bname) == 0)
			{
				qn_mlu_add_errf(self, "line#%d, invalid tag name.", line);
				goto pos_exit;
			}

			if (hasintern)
			{
				if (qn_bstr_nth(bname, qn_bstr_length(bname) - 1) != '?')
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

				tmptag = qn_slist_data(stack);
				qn_slist_remove_link(StackList, stack, stack, &stack);

				if (strcmp(tmptag->base.name, qn_bstr_data(bname)) != 0)
				{
					qn_mlu_add_errf(self, "line#%d, found closure '%s' with entrance '%s'.",
						line, qn_bstr_data(bname), tmptag->base.name);
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
					curtag = qn_slist_data(stack);
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
				curtag = (QnRealTag*)qn_new_mltag(qn_bstr_data(bname));
				if (!curtag)
				{
					qn_mlu_add_errf(self, "line#%d, out of memory.", line);
					goto pos_exit;
				}
				curtag->base.line = line;
				curtag->index = idn++;

				// 인수
				if (!_qn_realtag_parse_args(curtag, (QnBstr4k*)barg))
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

						curtag = qn_slist_data(stack);
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
	}   // for ~len

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
			if (!stack || curtag != qn_slist_data(stack))
				qn_mltag_delete((QnMlTag*)curtag);
		}

		for (StackList* n = stack; n; n = qn_slist_next(n))
			qn_mltag_delete((QnMlTag*)qn_slist_data(n));

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
bool qn_mlu_write_file(const QnMlu* self, QnMount* mount, const char* RESTRICT filename)
{
	qn_return_when_fail(qn_arr_count(&self->tags) > 0, false);

	QnStream* file = qn_open_file(mount, filename, "w");
	qn_return_when_fail(file, false);
	if (qn_stream_can_write(file) == false)
	{
		qn_unloadu(file);
		return false;
	}

	// UTF8 BOM
	const int bom = 0x00BFBBEF;
	qn_stream_write(file, &bom, 0, 3);

	// xml 부호
	qn_stream_write(file, (const void*)ml_header_desc, 0, (int)strlen(ml_header_desc));

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		const QnRealTag* tag = qn_arr_nth(&self->tags, i);
		_qn_realtag_write_file(tag, file, 0);
	}

	qn_unloadu(file);

	return true;
}

//
int qn_mlu_get_count(const QnMlu* self)
{
	return (int)qn_arr_count(&self->tags);
}

//
const char* qn_mlu_get_err(const QnMlu* self, const int at)
{
	return at >= 0 && at < (int)qn_arr_count(&self->errs) ? qn_arr_nth(&self->errs, at) : NULL;
}

//
QnMlTag* qn_mlu_get_tag(const QnMlu* self, const char* RESTRICT name)
{
	qn_return_when_fail(name, NULL);

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
QnMlTag* qn_mlu_get_tag_nth(const QnMlu* self, const int at)
{
	return at >= 0 && at < (int)qn_arr_count(&self->tags) ? (QnMlTag*)qn_arr_nth(&self->tags, at) : NULL;
}

//
const char* qn_mlu_get_context(const QnMlu* self, const char* RESTRICT name, const char* RESTRICT if_not_exist)
{
	const QnMlTag* tag = qn_mlu_get_tag(self, name);
	return tag ? tag->context : if_not_exist;
}

//
const char* qn_mlu_get_context_nth(const QnMlu* self, int at, const char* RESTRICT if_not_exist)
{
	const QnMlTag* tag = qn_mlu_get_tag_nth(self, at);
	return tag ? tag->context : if_not_exist;
}

//
int qn_mlu_contains(const QnMlu* self, QnMlTag* RESTRICT tag)
{
	int ret;
	qn_arr_contains(&self->tags, (QnRealTag*)tag, &ret);
	return ret;
}

//
void qn_mlu_foreach(const QnMlu* self, void(*func)(void*, QnMlTag*), void* userdata)
{
	qn_return_when_fail(func,/*void*/);

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->tags, i);
		func(userdata, (QnMlTag*)mt);
	}
}

//
void qn_mlu_each(const QnMlu* self, void(*func)(QnMlTag* tag))
{
	qn_return_when_fail(func,/*void*/);

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->tags, i);
		func((QnMlTag*)mt);
	}
}

//
QnMlTag* qn_mlu_add(QnMlu* self, const char* RESTRICT name, const char* RESTRICT context, const int line)
{
	qn_return_when_fail(name, NULL);

	QnRealTag* tag = (QnRealTag*)qn_new_mltag(name);
	qn_return_when_fail(tag, NULL);

	tag->base.line = line;
	tag->index = -1;

	if (context)
		qn_mltag_set_context((QnMlTag*)tag, context, -1);

	qn_arr_add(SubArray, &self->tags, tag);

	return (QnMlTag*)tag;
}

//
QnMlTag* qn_mlu_add_tag(QnMlu* self, QnMlTag* RESTRICT tag)
{
	qn_return_when_fail(tag, NULL);

	qn_arr_add(SubArray, &self->tags, (QnRealTag*)tag);

	return tag;
}

//
int qn_mlu_remove(QnMlu* self, const char* RESTRICT name, const bool is_all)
{
	qn_return_when_fail(name, -1);

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

			if (is_all)
				i--;
			else
				break;
		}
	}

	return cnt;
}

//
bool qn_mlu_remove_nth(QnMlu* self, const int at)
{
	if (at < 0 || at >= (int)qn_arr_count(&self->tags))
		return false;

	QnRealTag* tag = qn_arr_nth(&self->tags, at);
	qn_arr_remove_nth(SubArray, &self->tags, at);
	qn_mltag_delete((QnMlTag*)tag);

	return true;
}

//
bool qn_mlu_remove_tag(QnMlu* self, QnMlTag* RESTRICT tag, const bool is_delete)
{
	qn_return_when_fail(tag, false);

	const QnRealTag* mt = (QnRealTag*)tag;
	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		if (qn_arr_nth(&self->tags, i) == mt)
		{
			qn_arr_remove_nth(SubArray, &self->tags, i);

			if (is_delete)
				qn_mltag_delete(tag);

			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
// 노드

//
QnMlTag* qn_new_mltag(const char* name)
{
	QnRealTag* self = qn_alloc_zero_1(QnRealTag);
	qn_return_when_fail(self, NULL);

	self->base.name = qn_strdup(name);
	qn_strupr(self->base.name);

	self->base.name_len = (int)strlen(self->base.name);
	self->name_hash = qn_strhash(self->base.name);

	return (QnMlTag*)self;
}

// 제거
static void _qn_realtag_delete_ptr(QnRealTag** ptr)
{
	QnRealTag* self = *ptr;

	qn_free(self->base.name);
	qn_free(self->base.context);

	qn_arr_foreach_ptr_1(&self->subs, _qn_realtag_delete_ptr);
	qn_arr_disp(&self->subs);

	qn_hash_disp(ArgHash, &self->args);

	qn_free(self);
}

//
void qn_mltag_delete(QnMlTag* self)
{
	QnRealTag* real = (QnRealTag*)self;
	_qn_realtag_delete_ptr(&real);
}

//
void qn_mltag_add_context(QnMlTag* ptr, const char* RESTRICT cntx, const int size)
{
	qn_return_when_fail(cntx && *cntx != '\0',/*void*/);

	QnRealTag* self = (QnRealTag*)ptr;
	const int len = size <= 0 ? (int)strlen(cntx) : size;

	if (self->base.context)
	{
		char* psz = qn_strdupcat(self->base.context, cntx, NULL);
		qn_free(self->base.context);
		self->base.context = psz;
		self->base.context_len = (int)strlen(psz);
		self->context_hash = qn_strhash(psz);
	}
	else
	{
		self->base.context = qn_alloc(len + 1, char);
		qn_return_when_fail(self->base.context,/*void*/);

		qn_strcpy(self->base.context, cntx);
		self->base.context_len = len;
		self->context_hash = qn_strhash(self->base.context);
	}
}

//
void qn_mltag_set_context(QnMlTag* ptr, const char* RESTRICT cntx, const int size)
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
static bool _qn_realtag_parse_args(QnRealTag* self, QnBstr4k* bs)
{
	qn_return_when_fail(qn_bstr_is_have(bs), true);

	for (;;)
	{
		// 키
		const int eq = qn_bstr_find_char(bs, 0, '=');

		if (eq < 0)
		{
			qn_bstr_trim(bs);

			if (qn_bstr_is_have(bs))
			{
				// 값이 없는데 키만 있으면 안됨
				return false;
			}

			break;
		}

		QnBstr1k k, v;
		qn_bstr_sub_bstr(&k, bs, 0, eq);
		qn_bstr_trim(&k);

		qn_bstr_sub_bstr(bs, bs, (size_t)(eq + 1), -1);
		qn_bstr_trim_left(bs);

		// 값
		if (qn_bstr_is_empty(bs))
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
			qn_bstr_sub_bstr(bs, bs, (size_t)(at + 1), -1);
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
			qn_bstr_sub_bstr(bs, bs, (size_t)(at + 1), -1);
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

				if (qn_bstr_is_empty(&v))
				{
					// 아마도 마지막에 '='만 있는 듯
					return false;
				}

				qn_bstr_clear(bs);
			}
			else
			{
				qn_bstr_sub_bstr(&v, bs, 0, at);
				qn_bstr_sub_bstr(bs, bs, (size_t)(at + 1), -1);
			}
		}

		// 넣자
		qn_mltag_set_arg((QnMlTag*)self, qn_bstr_data(&k), qn_bstr_data(&v));
	}

	return true;
}

// 인수 파일에 쓰기
static void _qn_realtag_write_file_arg(QnStream* file, char** pk, char** pv)
{
	char sz[3] = { 0, };
	const char* k = *pk;
	const char* v = *pv;

	sz[0] = ' ';
	qn_stream_write(file, sz, 0, sizeof(char) * 1);
	qn_stream_write(file, k, 0, (int)strlen(k));

	sz[0] = '=';
	sz[1] = '"';
	qn_stream_write(file, sz, 0, sizeof(char) * 2);
	qn_stream_write(file, v, 0, (int)strlen(v));

	qn_stream_write(file, sz + 1, 0, sizeof(char) * 1);
}

// 파일에 쓰기
static bool _qn_realtag_write_file(const QnRealTag* self, QnStream* file, int ident)
{
	char ch;
	char szident[260];
	QnBstr2k bs;

	qn_strfll(szident, 0, (size_t)ident, '\t');
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
				qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));
			}
			else
			{
				qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
				qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));

				qn_hash_foreach_3(ArgHash, &self->args, _qn_realtag_write_file_arg, file);

				qn_bstr_format(&bs, ">%s</%s>\n", self->base.context, self->base.name);
				qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));
			}
		}
		else
		{
			if (qn_hash_count(&self->args) == 0)
			{
				qn_bstr_format(&bs, "%s<%s/>\n",
					szident, self->base.name);
				qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));
			}
			else
			{
				qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
				qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));

				qn_hash_foreach_3(ArgHash, &self->args, _qn_realtag_write_file_arg, file);

				qn_bstr_set(&bs, "/>\n");
				qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));
			}
		}
	}
	else
	{
		if (qn_hash_count(&self->args) == 0)
		{
			qn_bstr_format(&bs, "%s<%s>\n", szident, self->base.name);
			qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));
		}
		else
		{
			qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
			qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));

			qn_hash_foreach_3(ArgHash, &self->args, _qn_realtag_write_file_arg, file);

			qn_bstr_set(&bs, ">\n");
			qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));
		}

		// 내용
		if (self->base.context_len > 0)
		{
			qn_stream_write(file, szident, 0, ident);

			ch = '\t';
			qn_stream_write(file, &ch, 0, sizeof(char));

			qn_stream_write(file, self->base.context, 0, self->base.context_len);

			ch = '\n';
			qn_stream_write(file, &ch, 0, sizeof(char));
		}

		// 자식
		ident++;

		for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
		{
			const QnRealTag* tag = qn_arr_nth(&self->subs, i);
			_qn_realtag_write_file(tag, file, ident);
		}

		//
		qn_bstr_format(&bs, "%s</%s>\n", szident, self->base.name);
		qn_stream_write(file, qn_bstr_data(&bs), 0, (int)qn_bstr_length(&bs));
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
QnMlTag* qn_mltag_get_sub(QnMlTag* ptr, const char* RESTRICT name)
{
	qn_return_when_fail(name, NULL);

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
QnMlTag* qn_mltag_get_sub_nth(QnMlTag* ptr, const int at)
{
	const QnRealTag* self = (QnRealTag*)ptr;
	return at >= 0 && at < (int)qn_arr_count(&self->subs) ? (QnMlTag*)qn_arr_nth(&self->subs, at) : NULL;
}

//
const char* qn_mltag_get_sub_context(QnMlTag* ptr, const char* RESTRICT name, const char* RESTRICT if_not_exist)
{
	const QnMlTag* tag = qn_mltag_get_sub(ptr, name);
	return tag ? tag->context : if_not_exist;
}

//
const char* qn_mltag_get_sub_context_nth(QnMlTag* ptr, const int at, const char* RESTRICT if_not_exist)
{
	const QnMlTag* tag = qn_mltag_get_sub_nth(ptr, at);
	return tag ? tag->context : if_not_exist;
}

//
int qn_mltag_contains_sub(QnMlTag* RESTRICT ptr, QnMlTag* RESTRICT tag)
{
	const QnRealTag* self = (QnRealTag*)ptr;
	int ret;

	qn_arr_contains(&self->subs, (QnRealTag*)tag, &ret);

	return ret;
}

//
void qn_mltag_foreach_sub(QnMlTag* ptr, void(*func)(void* userdata, QnMlTag* tag), void* userdata)
{
	qn_return_when_fail(func,/*void*/);

	const QnRealTag* self = (QnRealTag*)ptr;

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->subs, i);
		func(userdata, (QnMlTag*)mt);
	}
}

//
void qn_mltag_each_sub(QnMlTag* ptr, void(*func)(QnMlTag* tag))
{
	qn_return_when_fail(func,/*void*/);

	const QnRealTag* self = (QnRealTag*)ptr;

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		QnRealTag* mt = qn_arr_nth(&self->subs, i);
		func((QnMlTag*)mt);
	}
}

//
QnMlTag* qn_mltag_add_sub(QnMlTag* ptr, const char* RESTRICT name, const char* RESTRICT context, const int line)
{
	qn_return_when_fail(name, NULL);

	QnRealTag* self = (QnRealTag*)ptr;
	QnRealTag* mt = (QnRealTag*)qn_new_mltag(name);
	qn_return_when_fail(mt, NULL);

	mt->base.line = line;
	mt->index = -1;

	if (context)
		qn_mltag_set_context((QnMlTag*)mt, context, -1);

	qn_arr_add(SubArray, &self->subs, mt);

	return (QnMlTag*)mt;
}

//
QnMlTag* qn_mltag_add_sub_tag(QnMlTag* RESTRICT ptr, QnMlTag* RESTRICT tag)
{
	qn_return_when_fail(tag, NULL);

	QnRealTag* self = (QnRealTag*)ptr;

	qn_arr_add(SubArray, &self->subs, (QnRealTag*)tag);

	return tag;
}

//
int qn_mltag_remove_sub(QnMlTag* ptr, const char* RESTRICT name, const bool is_all)
{
	qn_return_when_fail(name, -1);

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

			if (is_all)
				i--;
			else
				break;
		}
	}

	return cnt;
}

//
bool qn_mltag_remove_sub_nth(QnMlTag* ptr, const int at)
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
bool qn_mltag_remove_sub_tag(QnMlTag* RESTRICT ptr, QnMlTag* RESTRICT tag, const bool is_delete)
{
	qn_return_when_fail(tag, false);

	QnRealTag* self = (QnRealTag*)ptr;
	const QnRealTag* mt = (QnRealTag*)tag;

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		if (qn_arr_nth(&self->subs, i) == mt)
		{
			qn_arr_remove_nth(SubArray, &self->subs, i);

			if (is_delete)
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
const char* qn_mltag_get_arg(QnMlTag* ptr, const char* RESTRICT name, const char* RESTRICT if_not_exist)
{
	qn_return_when_fail(name, if_not_exist);

	const QnRealTag* self = (QnRealTag*)ptr;
	qn_return_when_fail(qn_hash_count(&self->args) > 0, if_not_exist);

	char** retvalue;
	qn_hash_get(ArgHash, &self->args, name, &retvalue);

	return retvalue ? *retvalue : if_not_exist;
}

//
bool qn_mltag_next_arg(QnMlTag* ptr, void** RESTRICT index, const char** RESTRICT name, const char** RESTRICT data)
{
	qn_return_when_fail(index, false);

	if (*index == (void*)(nint)-1)  // NOLINT
		return false;

	const QnRealTag* self = (QnRealTag*)ptr;
	const ArgHashNode* node = !*index ? self->args.FIRST : (ArgHashNode*)*index;

	if (name) *name = node->KEY;
	if (data) *data = node->VALUE;

	*index = node->NEXT ? node->NEXT : (void*)(intptr_t)-1;  // NOLINT

	return true;
}

//
bool qn_mltag_contains_arg(QnMlTag* ptr, const char* RESTRICT name)
{
	return qn_mltag_get_arg(ptr, name, NULL) != NULL;
}

//
void qn_mltag_foreach_arg(QnMlTag* ptr, void(*func)(void* userdata, char* const* name, char* const* data), void* userdata)
{
	const QnRealTag* self = (QnRealTag*)ptr;

	qn_hash_foreach_3(ArgHash, &self->args, func, userdata);
}

//
void qn_mltag_set_arg(QnMlTag* ptr, const char* RESTRICT name, const char* RESTRICT value)
{
	qn_return_when_fail(name,/*void*/);

	QnRealTag* self = (QnRealTag*)ptr;
	char* dn = qn_strdup(name);
	char* dv = qn_strdup(value);

	qn_strupr(dn);

	if (!self->args.BUCKET)
		qn_hash_init(ArgHash, &self->args);

	qn_hash_set(ArgHash, &self->args, dn, dv);
}

//
bool qn_mltag_remove_arg(QnMlTag* ptr, const char* RESTRICT name)
{
	qn_return_when_fail(name, false);
	QnRealTag* self = (QnRealTag*)ptr;
	bool ret;
	qn_hash_remove(ArgHash, &self->args, name, &ret);

	return ret;
}
