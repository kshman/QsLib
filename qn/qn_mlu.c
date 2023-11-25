#include "pch.h"
#include "qn.h"
#include "qnctn.h"

typedef struct qnRealTag qnRealTag;

QN_ARR_DECL(ErrorArray, char*);
QN_ARR_DECL(SubArray, qnRealTag*);
QN_SLIST_DECL(StackList, qnRealTag*);

QN_HASH_DECL(ArgHash, char*, char*);
QN_HASH_HASHER_CHAR_PTR(ArgHash);
QN_HASH_KEY_DELETE(ArgHash);
QN_HASH_VALUE_DELETE(ArgHash);

// 스트링 제거
static void _error_array_delete_ptr(char** ptr)
{
	char* s = *ptr;
	qn_free(s);
}

// xml 버전
static const char* _ml_header_desc = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";


//////////////////////////////////////////////////////////////////////////
// 실제 노드
struct qnRealTag
{
	qnMlTag				base;

	SubArray			subs;
	ArgHash				harg;

	int					idn;
	bool				cls;

	size_t				nhash;
	size_t				chash;
};

static void _qn_realtag_delete_ptr(qnRealTag** ptr);
static bool _qn_realtag_parse_args(qnRealTag* self, qnBstr4k* bs);
static bool _qn_realtag_write_file(qnRealTag* self, qnFile* file, int ident);


//////////////////////////////////////////////////////////////////////////
// RML
struct qnMlu
{
	SubArray			tags;
	ErrorArray			errs;

	int					maxline;
};

/**
 * RML을 만든다.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlu*.
 */
qnMlu* qn_mlu_new(void)
{
	qnMlu* self = qn_alloc_zero_1(qnMlu);
	return self;
}

/**
 * 파일에서 RML을 만든다.
 * @param	filename	파일의 이름.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlu*.
 */
qnMlu* qn_mlu_new_file(const char* filename)
{
	int size;
	uint8_t* data = (uint8_t*)qn_file_alloc(filename, &size);
	qn_retval_if_fail(data, NULL);

	qnMlu* self = qn_mlu_new();
	bool ret = qn_mlu_load_buffer(self, data, size);

	qn_free(data);

	return self;
}

/**
 * 파일에서 RML을 만든다. 유니코드 파일 이름을 사용한다.
 * @param	filename	파일의 이름.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlu*.
 */
qnMlu* qn_mlu_new_file_l(const wchar_t* filename)
{
	int size;
	uint8_t* data = (uint8_t*)qn_file_alloc_l(filename, &size);
	qn_retval_if_fail(data, NULL);

	qnMlu* self = qn_mlu_new();
	bool ret = qn_mlu_load_buffer(self, data, size);

	qn_free(data);

	return self;
}

/**
 * 버퍼에서 RML을 만든다.
 * @param	data	버퍼.
 * @param	size	버퍼 크기.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlu*.
 */
qnMlu* qn_mlu_new_buffer(const pointer_t data, int size)
{
	qnMlu* self;

	qn_retval_if_fail(data, NULL);
	qn_retval_if_fail(size > 0, NULL);

	self = qn_mlu_new();
	qn_mlu_load_buffer(self, data, size);

	return self;
}

/**
 * RML을 제거한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 */
void qn_mlu_delete(qnMlu* self)
{
	qn_mlu_clean_tags(self);
	qn_mlu_clean_errs(self);

	qn_arr_disp(SubArray, &self->tags);
	qn_arr_disp(ErrorArray, &self->errs);

	qn_free(self);
}

/**
 * 모든 RML 태그를 삭제한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 */
void qn_mlu_clean_tags(qnMlu* self)
{
	qn_arr_loopeach(SubArray, &self->tags, _qn_realtag_delete_ptr);
	qn_arr_clear(SubArray, &self->tags);
}

/**
 * 모든 RML 오류를 삭제한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 */
void qn_mlu_clean_errs(qnMlu* self)
{
	qn_arr_loopeach(ErrorArray, &self->errs, _error_array_delete_ptr);
	qn_arr_clear(ErrorArray, &self->errs);
}

/**
 * 오류값을 추가한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	msg			메시지.
 */
void qn_mlu_add_err(qnMlu* self, const char* msg)
{
	char* dup;

	qn_ret_if_fail(msg);

	dup = _strdup(msg);
	qn_arr_add(ErrorArray, &self->errs, dup);
}

/**
 * 오류값을 포맷 방식으로 추가한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	fmt			포맷 문자열.
 */
void qn_mlu_add_errf(qnMlu* self, const char* fmt, ...)
{
	char* psz;
	va_list va;

	va_start(va, fmt);
	qn_vasprintf(&psz, fmt, va);
	va_end(va);

	qn_arr_add(ErrorArray, &self->errs, psz);
}

/**
 * 갖고 있는 오류를 디버그 콘솔로 출력한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 */
void qn_mlu_print_err(qnMlu* self)
{
	for (int i = 0; i < qn_arr_count(&self->errs); i++)
	{
		char* psz = qn_arr_nth(&self->errs, i);
#if _QN_WINDOWS_
		OutputDebugStringA(psz);
#else
		puts(psz);
#endif
	}
}

/**
 * RML 정보 구성 내용을 디버그 콘솔로 출력한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 */
void qn_mlu_print(qnMlu* self)
{
	// NOT IMPL
}

/**
 * 버퍼에서 RML을 분석하여 읽는다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	data		버퍼.
 * @param	size		버퍼 크기.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mlu_load_buffer(qnMlu* self, const pointer_t data, int size)
{
	int i, n, at;
	int idn = 0;
	const char* pos;
	char* cd;
	char* psz;

	qn_retval_if_fail(data, FALSE);
	qn_retval_if_fail(size > 0, FALSE);

	qn_mlu_clean_tags(self);
	qn_mlu_clean_errs(self);

	// UTF8 사인 찾기
	i = (*(const int*)data) & 0x00FFFFFF;

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

		if (i == 0)
		{
			// 이건 32비트 유니코드, 아마 UTF32 BE(0x0000FEFF)
			// 처리할 수 없다
			return FALSE;
		}
		else if (i == 0xFEFF)
		{
			// 이건 UTF16 LE
			// 처리하지 않음
			return FALSE;
		}
		else if (i == 0xFFFE)
		{
			// 이건 UTF16 BE, 또는 UTF32 LE(0xFFFE0000)
			// 처리 안할래
			return FALSE;
		}

		// 사인 없음
		pos = (const char*)data;
	}

	//
	bool ret = FALSE;
	bool iscmt = FALSE;
	int line = 1;
	int cdsize = 0;

	StackList* stack = NULL;
	qnRealTag* tmptag = NULL;
	qnRealTag* curtag = NULL;

	cd = qn_alloc(size, char);

	qnBstr4k* btag = qn_alloc_1(qnBstr4k);     // 태그는 4k
	qnBstr4k* bname = qn_alloc_1(qnBstr4k);    // 이름은 4k
	qnBstr4k* barg = qn_alloc_1(qnBstr4k);     // 인수는 4k

	for (i = 0; i < size; i++)
	{
		char ch = pos[i];

		if (ch == '\n')
		{
			line++;

			if (iscmt)
				iscmt = FALSE;

#if 0
			// 옛날 방식 주석 '#' 또는 "//"
			n = i + 1;

			if (n < size)
			{
				if (pos[n] == '#')
					iscmt = TRUE;
				else if (pos[n] == '/')
				{
					n++;

					if (n < size && pos[n] == '/')
						iscmt = TRUE;
				}
			}
#endif
		}

		if (iscmt)
			continue;

		if (ch == '<')
		{
			// 태그 들어가기
			bool hasclosure = FALSE;
			bool hasintern = FALSE;
			bool hassingle = FALSE;

			i++;

			if (i < size)
			{
				if (pos[i] == '/')
				{
					hasclosure = TRUE;
					i++;
				}
				else if (pos[i] == '?')
				{
					hasclosure = TRUE;
					hasintern = TRUE;
					i++;
				}
				else if (pos[i] == '!' && (i + 2) < size && pos[i + 1] == '-' && pos[i + 2] == '-')
				{
					// 주석
					i += 3;
					n = size - 2;

					for (; i < n; i++)
					{
						if (pos[i] == '-' && pos[i + 1] == '-' && pos[i + 2] == '>')
						{
							i += 3;
							hasintern = TRUE;
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
			qn_bstr_init(btag, NULL);

			for (; i < size; i++)
			{
				if (pos[i] == '\n')
					line++;
				/*
				else if (pos[i]=='/')
				hassingle=TRUE;
				*/
				else if (pos[i] == '>')
				{
					if (pos[i - 1] == '/')
					{
						n = (int)btag->len - 1;
						btag->data[n] = '\0';
						btag->len = n;
						hassingle = TRUE;
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
				qn_bstr_init(barg, NULL);
				qn_bstr_set_bstr(bname, btag);
				qn_bstr_upper(bname);
			}
			else
			{
				at = qn_bstr_find_char(btag, 0, ' ');

				if (at < 0)
					at = qn_bstr_find_char(btag, 0, '\n');

				if (at < 0)
				{
					// 이름만 있음
					qn_bstr_init(barg, NULL);
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
					qn_strtrim(cd);

					cdsize = (int)strlen(cd);

					if (cdsize > 0)
						qn_mltag_add_context((qnMlTag*)tmptag, cd, cdsize);

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
						qn_mltag_add_context((qnMlTag*)curtag, cd, cdsize);
				}

				// 열기
				curtag = (qnRealTag*)qn_mltag_new(bname->data);
				if (!curtag)
				{
					qn_mlu_add_errf(self, "line#%d, out of memory.", line);
					goto pos_exit;
				}
				curtag->base.line = line;
				curtag->idn = idn++;

				// 인수
				if (!_qn_realtag_parse_args(curtag, (qnBstr4k*)barg))
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
						tmptag->cls = TRUE;

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
			psz = curtag && curtag->base.name ? curtag->base.name : "unknown";
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
		ret = TRUE;
	}

pos_exit:
	if (!ret)
	{
		StackList* n;

		if (curtag)
		{
			if (!stack || curtag != stack->data)
				qn_mltag_delete((qnMlTag*)curtag);
		}

		for (n = stack; n; n = n->next)
			qn_mltag_delete((qnMlTag*)n->data);

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

/**
 * RML 내용을 파일로 저장한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	filename	파일의 이름.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mlu_write_file(qnMlu* self, const char* filename)
{
	qnFile* file;

	qn_retval_if_fail(filename, FALSE);
	qn_retval_if_fail(qn_arr_count(&self->tags) > 0, FALSE);

	file = qn_file_new(filename, "w");
	qn_retval_if_fail(file, FALSE);

	// UTF8 BOM
	int i = 0x00BFBBEF;
	qn_file_write(file, &i, 0, 3);

	// xml 부호
	qn_file_write(file, (const pointer_t)_ml_header_desc, 0, (int)strlen(_ml_header_desc));

	for (i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* tag = qn_arr_nth(&self->tags, i);
		_qn_realtag_write_file(tag, file, 0);
	}

	qn_file_delete(file);

	return TRUE;
}

/**
 * 갖고 있는 최상위 태그의 갯수를 반환한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @return	최상위 태그 갯수.
 */
int qn_mlu_get_count(qnMlu* self)
{
	return (int)qn_arr_count(&self->tags);
}

/**
 * 갖고 있는 오류를 순번으로 얻는다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	at			오류 순번.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 char*.
 */
const char* qn_mlu_get_err(qnMlu* self, int at)
{
	return at >= 0 && at < qn_arr_count(&self->errs) ? qn_arr_nth(&self->errs, at) : NULL;
}

/**
 * 최상위 태그를 찾는다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	name		태그 이름.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlTag*.
 */
qnMlTag* qn_mlu_get_tag(qnMlu* self, const char* name)
{
	qn_retval_if_fail(name, NULL);

	size_t hash = qn_strhash(name);
	for (int i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->tags, i);

		if (mt->nhash == hash && strcmp(mt->base.name, name) == 0)
			return (qnMlTag*)mt;
	}

	return NULL;
}

/**
 * 최상위 태그를 순번으로 얻는다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	at			순번.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlTag*.
 */
qnMlTag* qn_mlu_get_tag_nth(qnMlu* self, int at)
{
	return at >= 0 && at < qn_arr_count(&self->tags) ? (qnMlTag*)qn_arr_nth(&self->tags, at) : NULL;
}

/**
 * 최상위 태그가 갖고 있는 컨텍스트를 얻는다.
 * @param [입력]	self  	개체나 인터페이스의 자기 자신 값.
 * @param	name	  	태그 이름.
 * @param	ifnotexist	태그가 존재하지 않을 경우 반환할 값.
 * @return	문제가 있거나 실패하면 ifnotexist를 반환, 성공할 때 반환값은 태그 컨텍스트.
 */
const char* qn_mlu_get_context(qnMlu* self, const char* name, const char* ifnotexist)
{
	qnMlTag* tag = qn_mlu_get_tag(self, name);
	return tag ? tag->cntx : ifnotexist;
}

/**
 * 최상위 태그가 갖고 있는 컨텍스트를 얻는다. 태그는 순번으로 검사한다.
 * @param [입력]	self  	개체나 인터페이스의 자기 자신 값.
 * @param	at		  	순번.
 * @param	ifnotexist	태그가 존재하지 않을 경우 반환할 값.
 * @return	문제가 있거나 실패하면 ifnotexist를 반환, 성공할 때 반환값은 태그 컨텍스트.
 */
const char* qn_mlu_get_context_nth(qnMlu* self, int at, const char* ifnotexist)
{
	qnMlTag* tag = qn_mlu_get_tag_nth(self, at);
	return tag ? tag->cntx : ifnotexist;
}

/**
 * 지정한 태그가 있나 검사한다.
 * @param [입력]	self   	개체나 인터페이스의 자기 자신 값.
 * @param [입력]	tag	(널값이 아님) 지정한 태그.
 * @return	태그가 없으면 -1을 있으면 해당 순번을 반환한다.
 */
int qn_mlu_contains(qnMlu* self, qnMlTag* tag)
{
	int ret;
	qn_arr_contains(SubArray, &self->tags, (qnRealTag*)tag, &ret);
	return ret;
}

/**
 * 최상위 태그에 대해 ForEach를 수행한다.
 * @param [입력]	self		개체나 인터페이스의 자기 자신 값.
 * @param [입력]	func	콜백 함수.
 * @param	userdata		콜백 데이터.
 */
void qn_mlu_foreach(qnMlu* self, void(*func)(pointer_t userdata, qnMlTag* tag), pointer_t userdata)
{
	qn_ret_if_fail(func);

	for (int i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->tags, i);
		func(userdata, (qnMlTag*)mt);
	}
}

/**
 * 최상위 태그에 대해 LoopEach를 수행한다.
 * @param [입력]	self		개체나 인터페이스의 자기 자신 값.
 * @param [입력]	func	콜백 함수.
 */
void qn_mlu_loopeach(qnMlu* self, void(*func)(qnMlTag* tag))
{
	qn_ret_if_fail(func);

	for (int i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->tags, i);
		func((qnMlTag*)mt);
	}
}

/**
 * 태그를 추가한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	name		태그 이름.
 * @param	context 	태그 컨텍스트.
 * @param	line		줄번호.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 만들어진 태그.
 */
qnMlTag* qn_mlu_add(qnMlu* self, const char* name, const char* context, int line)
{
	qnRealTag* tag;

	qn_retval_if_fail(name, NULL);

	tag = (qnRealTag*)qn_mltag_new(name);
	tag->base.line = line;
	tag->idn = -1;

	if (context)
		qn_mltag_set_context((qnMlTag*)tag, context, -1);

	qn_arr_add(SubArray, &self->tags, tag);

	return (qnMlTag*)tag;
}

/**
 * 태그를 추가한다.
 * @param [입력]	self   	개체나 인터페이스의 자기 자신 값.
 * @param [입력]	tag	(널값이 아님) 추가할 태그.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 추가한 태그.
 */
qnMlTag* qn_mlu_add_tag(qnMlu* self, qnMlTag* tag)
{
	qn_retval_if_fail(tag, NULL);

	qn_arr_add(SubArray, &self->tags, (qnRealTag*)tag);

	return tag;
}

/**
 * 태그를 제거한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	name		태그 이름.
 * @param	isall   	같은 이름 태그를 모두 지우려면 참으로 넣는다.
 * @return	지운 태그의 갯수.
 */
int qn_mlu_remove(qnMlu* self, const char* name, bool isall)
{
	qn_retval_if_fail(name, -1);

	size_t hash = qn_strhash(name);
	int cnt = 0;
	for (int i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* tag = qn_arr_nth(&self->tags, i);

		if (tag->nhash == hash && strcmp(tag->base.name, name) == 0)
		{
			cnt++;

			qn_arr_remove_nth(SubArray, &self->tags, i);
			qn_mltag_delete((qnMlTag*)tag);

			if (isall)
				i--;
			else
				break;
		}
	}

	return cnt;
}

/**
 * 태그를 순번으로 제거한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 * @param	at			순번.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mlu_remove_nth(qnMlu* self, int at)
{
	qnRealTag* tag;

	if (at < 0 || at >= qn_arr_count(&self->tags))
		return FALSE;

	tag = qn_arr_nth(&self->tags, at);
	qn_arr_remove_nth(SubArray, &self->tags, at);
	qn_mltag_delete((qnMlTag*)tag);

	return TRUE;
}

/**
 * 태그를 제거한다.
 * @param [입력]	self   	개체나 인터페이스의 자기 자신 값.
 * @param [입력]	tag	(널값이 아님) 지울 태그.
 * @param	isdelete   	태그를 삭제하려면 참으로 넣는다.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mlu_remove_tag(qnMlu* self, qnMlTag* tag, bool isdelete)
{
	qnRealTag* mt = (qnRealTag*)tag;

	qn_retval_if_fail(tag, FALSE);

	for (int i = 0; i < qn_arr_count(&self->tags); i++)
	{
		if (qn_arr_nth(&self->tags, i) == mt)
		{
			qn_arr_remove_nth(SubArray, &self->tags, i);

			if (isdelete)
				qn_mltag_delete(tag);

			return TRUE;
		}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// 노드

/**
 * 태그 노드를 만든다.
 * @param	name	태그 이름.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlTag*.
 */
qnMlTag* qn_mltag_new(const char* name)
{
	qnRealTag* self = qn_alloc_zero_1(qnRealTag);
	qn_retval_if_fail(self, NULL);

	self->base.name = _strdup(name);

	_strupr_s(self->base.name, strlen(self->base.name));

	self->base.nlen = (int)strlen(self->base.name);
	self->nhash = qn_strhash(self->base.name);

	return (qnMlTag*)self;
}

// 제거
static void _qn_realtag_delete_ptr(qnRealTag** ptr)
{
	qnRealTag* self = *ptr;

	qn_free(self->base.name);
	qn_free(self->base.cntx);

	qn_arr_loopeach(SubArray, &self->subs, _qn_realtag_delete_ptr);
	qn_arr_disp(SubArray, &self->subs);

	qn_hash_disp(ArgHash, &self->harg);

	qn_free(self);
}

/**
 * 태그 노드를 제거한다.
 * @param [입력]	self	개체나 인터페이스의 자기 자신 값.
 */
void qn_mltag_delete(qnMlTag* self)
{
	qnRealTag* real = (qnRealTag*)self;
	_qn_realtag_delete_ptr(&real);
}

/**
 * 태그 내용을 추가한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param	cntx	   	컨텍스트.
 * @param	size	   	컨텍스트의 크기.
 */
void qn_mltag_add_context(qnMlTag* ptr, const char* cntx, int size)
{
	qnRealTag* self = (qnRealTag*)ptr;

	if (!cntx)
	{
		// 내용없을 때
	}
	else
	{
		// 내용있을 때
		if (size <= 0)
			size = (int)strlen(cntx);

		if (self->base.cntx)
		{
			char* psz = qn_strcat(self->base.cntx, cntx, NULL);
			qn_free(self->base.cntx);
			self->base.cntx = psz;
			self->base.clen = (int)strlen(psz);
			self->chash = qn_strhash(psz);
		}
		else
		{
			self->base.cntx = qn_alloc(size + 1, char);
			qn_ret_if_fail(self->base.cntx);

			strcpy_s(self->base.cntx, size + 1, cntx);
			self->base.clen = size;
			self->chash = qn_strhash(self->base.cntx);
		}
	}
}

/**
 * 태그에 내용을 쓴다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param	cntx	   	컨텍스트.
 * @param	size	   	컨텍스트의 크기.
 */
void qn_mltag_set_context(qnMlTag* ptr, const char* cntx, int size)
{
	qnRealTag* self = (qnRealTag*)ptr;

	if (self->base.cntx)
	{
		qn_free(self->base.cntx);
		self->base.cntx = NULL;
	}

	qn_mltag_add_context(ptr, cntx, size);
}

// 인수 분석
static bool _qn_realtag_parse_args(qnRealTag* self, qnBstr4k* bs)
{
	int at, eq;
	qnBstr1k k, v;   // 키와 값은 각각 1k

	qn_retval_if_fail(bs->len > 0, TRUE);

	for (;;)
	{
		// 키
		eq = qn_bstr_find_char(bs, 0, '=');

		if (eq < 0)
		{
			qn_bstr_trim(bs);

			if (bs->len > 0)
			{
				// 값이 없는데 키만 있으면 안됨
				return FALSE;
			}

			break;
		}

		qn_bstr_sub_bstr(&k, bs, 0, eq);
		qn_bstr_trim(&k);

		qn_bstr_sub_bstr(bs, bs, eq + 1, -1);
		qn_bstr_trim_left(bs);

		// 값
		if (bs->len == 0)
		{
			// 아마도 '='뒤에 값이 없는 듯
			return FALSE;
		}
		else if (qn_bstr_nth(bs, 0) == '"')
		{
			// 이중 인용이면 다음 이중 인용까지
			at = qn_bstr_find_char(bs, 1, '"');

			if (at < 0)
			{
				// 없으면 오류
				return FALSE;
			}

			qn_bstr_sub_bstr(&v, bs, 1, at - 1);
			qn_bstr_sub_bstr(bs, bs, at + 1, -1);
		}
		else if (qn_bstr_nth(bs, 0) == '\'')
		{
			// 단일 인용이면 다음 단일 인용까지
			at = qn_bstr_find_char(bs, 1, '\'');

			if (at < 0)
			{
				// 없으면 오류
				return FALSE;
			}

			qn_bstr_sub_bstr(&v, bs, 1, at - 1);
			qn_bstr_sub_bstr(bs, bs, at + 1, -1);
		}
		else
		{
			// 다음 공백까지
			at = qn_bstr_find_char(bs, 0, ' ');

			if (at < 0)
			{
				// 전체 다
				qn_bstr_set_bstr(&v, bs);
				qn_bstr_trim(&v);

				if (v.len == 0)
				{
					// 아마도 마지막에 '='만 있는 듯
					return FALSE;
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
		qn_mltag_set_arg((qnMlTag*)self, k.data, v.data);
	}

	return TRUE;
}

// 인수 파일에 쓰기
static void _qn_realtag_write_file_arg(qnFile* file, char** pk, char** pv)
{
	char sz[3];
	char* k = *pk;
	char* v = *pv;

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
static bool _qn_realtag_write_file(qnRealTag* self, qnFile* file, int ident)
{
	bool isbody = FALSE;
	char ch;
	char szident[260];
	qnBstr2k bs;
	int i;

	qn_strfll(szident, 0, ident, '\t');
	szident[ident] = '\0';

	// 아래 많다. 또는 내용 길다
	isbody = qn_arr_count(&self->subs) > 0 || self->base.clen > 64 ? TRUE : FALSE;

	if (!isbody)
	{
		if (self->base.clen > 0)
		{
			if (qn_hash_count(&self->harg) == 0)
			{
				qn_bstr_format(&bs, "%s<%s>%s</%s>\n",
					szident, self->base.name, self->base.cntx, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);
			}
			else
			{
				qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);

				qn_hash_foreach(ArgHash, &self->harg, _qn_realtag_write_file_arg, file);

				qn_bstr_format(&bs, ">%s</%s>\n", self->base.cntx, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);
			}
		}
		else
		{
			if (qn_hash_count(&self->harg) == 0)
			{
				qn_bstr_format(&bs, "%s<%s/>\n",
					szident, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);
			}
			else
			{
				qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
				qn_file_write(file, bs.data, 0, (int)bs.len);

				qn_hash_foreach(ArgHash, &self->harg, _qn_realtag_write_file_arg, file);

				qn_bstr_set(&bs, "/>\n");
				qn_file_write(file, bs.data, 0, (int)bs.len);
			}
		}
	}
	else
	{
		if (qn_hash_count(&self->harg) == 0)
		{
			qn_bstr_format(&bs, "%s<%s>\n", szident, self->base.name);
			qn_file_write(file, bs.data, 0, (int)bs.len);
		}
		else
		{
			qn_bstr_format(&bs, "%s<%s", szident, self->base.name);
			qn_file_write(file, bs.data, 0, (int)bs.len);

			qn_hash_foreach(ArgHash, &self->harg, _qn_realtag_write_file_arg, file);

			qn_bstr_set(&bs, ">\n");
			qn_file_write(file, bs.data, 0, (int)bs.len);
		}

		// 내용
		if (self->base.clen > 0)
		{
			qn_file_write(file, szident, 0, ident);

			ch = '\t';
			qn_file_write(file, &ch, 0, sizeof(char));

			qn_file_write(file, self->base.cntx, 0, self->base.clen);

			ch = '\n';
			qn_file_write(file, &ch, 0, sizeof(char));
		}

		// 자식
		ident++;

		for (i = 0; i < qn_arr_count(&self->subs); i++)
		{
			qnRealTag* tag = qn_arr_nth(&self->subs, i);
			_qn_realtag_write_file(tag, file, ident);
		}

		//
		qn_bstr_format(&bs, "%s</%s>\n", szident, self->base.name);
		qn_file_write(file, bs.data, 0, (int)bs.len);
	}

	return TRUE;
}

/**
 * 하부 태그의 갯수를 얻는다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @return	하부 태그의 갯수.
 */
int qn_mltag_get_sub_count(qnMlTag* ptr)
{
	qnRealTag* self = (qnRealTag*)ptr;

	return (int)qn_arr_count(&self->subs);
}

/**
 * 하부 태그를 찾는다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param	name	   	찾을 태그 이름.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlTag*.
 */
qnMlTag* qn_mltag_get_sub(qnMlTag* ptr, const char* name)
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_retval_if_fail(name, NULL);

	size_t hash = qn_strhash(name);
	for (int i = 0; i < qn_arr_count(&self->subs); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->subs, i);

		if (mt->nhash == hash && strcmp(mt->base.name, name) == 0)
			return (qnMlTag*)mt;
	}

	return NULL;
}

/**
 * 하부 태그를 순번으로 찾는다.
 * @param [입력] ptr 개체의 자기 자신 값.
 * @param	at		   	순번.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlTag*.
 */
qnMlTag* qn_mltag_get_sub_nth(qnMlTag* ptr, int at)
{
	qnRealTag* self = (qnRealTag*)ptr;
	return at >= 0 && at < qn_arr_count(&self->subs) ? (qnMlTag*)qn_arr_nth(&self->subs, at) : NULL;
}

/**
 * 하부 태그의 컨텍스트를 얻는다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param	name	   	태그 이름.
 * @param	ifnotexist 	태그를 찾을 수 없으면 반환할 값.
 * @return	문제가 있거나 실패하면 ifnotexit를 반환, 성공할 때 반환값은 char*.
 */
const char* qn_mltag_get_sub_context(qnMlTag* ptr, const char* name, const char* ifnotexist)
{
	qnMlTag* tag = qn_mltag_get_sub(ptr, name);
	return tag ? tag->cntx : ifnotexist;
}

/**
 * 하부 태그의 컨텍스트를 순번으로 얻는다.
 * @param [입력] ptr 개체의 자기 자신 값.
 * @param	at			순번.
 * @param	ifnotexist 	태그를 찾을 수 없으면 반환할 값.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 char*.
 */
const char* qn_mltag_get_sub_context_nth(qnMlTag* ptr, int at, const char* ifnotexist)
{
	qnMlTag* tag = qn_mltag_get_sub_nth(ptr, at);
	return tag ? tag->cntx : ifnotexist;
}

/**
 * 지정한 태그를 하부 태그로 갖고 있나 조사.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param [입력]	tag	(널값이 아니면) 찾을 태그.
 * @return	찾지 못하면 -1, 아니면 해당 순번을 반환.
 */
int qn_mltag_contains_sub(qnMlTag* ptr, qnMlTag* tag)
{
	qnRealTag* self = (qnRealTag*)ptr;
	int ret;

	qn_arr_contains(SubArray, &self->subs, (qnRealTag*)tag, &ret);

	return ret;
}

/**
 * 하부 태그에 대해 ForEach 연산을 수행한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param [입력]	func	콜백 함수.
 * @param	userdata		콜백 데이터.
 */
void qn_mltag_foreach_sub(qnMlTag* ptr, void(*func)(pointer_t userdata, qnMlTag* tag), pointer_t userdata)
{
	qnRealTag* self = (qnRealTag*)ptr;
	int i;

	qn_ret_if_fail(func);

	for (i = 0; i < qn_arr_count(&self->subs); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->subs, i);
		func(userdata, (qnMlTag*)mt);
	}
}

/**
 * 하부 태그에 대해 LoopEach 연산을 수행한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param [입력]	func	콜백 함수.
 */
void qn_mltag_loopeach_sub(qnMlTag* ptr, void(*func)(qnMlTag* tag))
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_ret_if_fail(func);

	for (int i = 0; i < qn_arr_count(&self->subs); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->subs, i);
		func((qnMlTag*)mt);
	}
}

/**
 * 하부 태그를 추가한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param	name	   	태그 이름.
 * @param	context	   	태그 콘텍스트.
 * @param	line	   	줄 번호.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlTag*.
 */
qnMlTag* qn_mltag_add_sub(qnMlTag* ptr, const char* name, const char* context, int line)
{
	qnRealTag* self = (qnRealTag*)ptr;
	qnRealTag* mt;

	qn_retval_if_fail(name, NULL);

	mt = (qnRealTag*)qn_mltag_new(name);
	mt->base.line = line;
	mt->idn = -1;

	if (context)
		qn_mltag_set_context((qnMlTag*)mt, context, -1);

	qn_arr_add(SubArray, &self->subs, mt);

	return (qnMlTag*)mt;
}

/**
 * 하부 태그를 추가한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param [입력]	tag	추가할 태그.
 * @return	문제가 있거나 실패하면 널값을 반환, 성공할 때 반환값은 qnMlTag*.
 */
qnMlTag* qn_mltag_add_sub_tag(qnMlTag* ptr, qnMlTag* tag)
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_retval_if_fail(tag, NULL);

	qn_arr_add(SubArray, &self->subs, (qnRealTag*)tag);

	return tag;
}

/**
 * 지정한 이름의 태그를 제거한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param	name	   	태그 이름.
 * @param	isall	   	같은 이름의 모든 태그를 지우려면 참으로 넣는다.
 * @return	지운 태그의 갯수.
 */
int qn_mltag_remove_sub(qnMlTag* ptr, const char* name, bool isall)
{
	qnRealTag* self = (qnRealTag*)ptr;
	qnRealTag* mt;
	int i, cnt;
	size_t hash;

	qn_retval_if_fail(name, -1);

	hash = qn_strhash(name);
	cnt = 0;

	for (i = 0; i < qn_arr_count(&self->subs); i++)
	{
		mt = qn_arr_nth(&self->subs, i);

		if (mt->nhash == hash && strcmp(mt->base.name, name) == 0)
		{
			cnt++;

			qn_arr_remove_nth(SubArray, &self->subs, i);
			qn_mltag_delete((qnMlTag*)mt);

			if (isall)
				i--;
			else
				break;
		}
	}

	return cnt;
}

/**
 * 지정한 순번의 하부 태그를 삭제한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param	at			순번.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mltag_remove_sub_nth(qnMlTag* ptr, int at)
{
	qnRealTag* self = (qnRealTag*)ptr;
	qnRealTag* mt;

	if (at < 0 || at >= qn_arr_count(&self->subs))
		return FALSE;

	mt = qn_arr_nth(&self->subs, at);
	qn_arr_remove_nth(SubArray, &self->subs, at);
	qn_mltag_delete((qnMlTag*)mt);

	return TRUE;
}

/**
 * 지정한 하부 태그를 삭제한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @param [입력,반환]	tag	(널값이 아님) 지울 태그.
 * @param	isdelete   	태그 자체를 삭제하려면 참으로 넣는다.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mltag_remove_sub_tag(qnMlTag* ptr, qnMlTag* tag, bool isdelete)
{
	qnRealTag* self = (qnRealTag*)ptr;
	qnRealTag* mt = (qnRealTag*)tag;

	qn_retval_if_fail(tag, FALSE);

	for (int i = 0; i < qn_arr_count(&self->subs); i++)
	{
		if (qn_arr_nth(&self->subs, i) == mt)
		{
			qn_arr_remove_nth(SubArray, &self->subs, i);

			if (isdelete)
				qn_mltag_delete(tag);

			return TRUE;
		}
	}

	return FALSE;
}

/**
 * 태그의 인수의 개수를 반환한다.
 * @param [입력]	ptr	개체의 자기 자신 값.
 * @return	인수의 개수.
 */
int qn_mltag_get_arity(qnMlTag* ptr)
{
	qnRealTag* self = (qnRealTag*)ptr;

	return (int)qn_hash_count(&self->harg);
}

/**
 * 인수를 이름으로 찾는다.
 * @param [입력] ptr 개체의 자기 자신 값.
 * @param	name	   	인수 이름.
 * @param	ifnotexist 	인수를 찾지 못하면 반환할 값.
 * @return	문제가 있거나 실패하면 ifnotexist를 반환, 성공할 때 반환값은 인수의 데이터.
 */
const char* qn_mltag_get_arg(qnMlTag* ptr, const char* name, const char* ifnotexist)
{
	qnRealTag* self = (qnRealTag*)ptr;
	char** retvalue;

	qn_retval_if_fail(name, ifnotexist);
	qn_retval_if_fail(qn_hash_count(&self->harg) > 0, ifnotexist);

	qn_hash_get(ArgHash, &self->harg, &name, &retvalue);

	return retvalue ? *retvalue : ifnotexist;
}

/**
 * 다음 인수를 찾는다.
 * @param [입력,반환]	ptr  	(널값이 아니면) 포인터.
 * @param [입력,반환]	index	(널값이 아님) 내부 찾기 인덱스 데이터.
 * @param	name		 	인수 이름.
 * @param	data		 	인수 자료.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mltag_next_arg(qnMlTag* ptr, pointer_t* index, const char** name, const char** data)
{
	qnRealTag* self = (qnRealTag*)ptr;
	ArgHashNode* node;

	qn_retval_if_fail(index, FALSE);

	if (*index == (pointer_t)(intptr_t)-1)
		return FALSE;

	node = !*index ? self->harg.frst : (ArgHashNode*)*index;

	if (name) *name = node->key;

	if (data) *data = node->value;

	*index = node->next ? node->next : (pointer_t)(intptr_t)-1;

	return TRUE;
}

/**
 * 이름에 해당하는 인수가 있는지 조사한다.
 * @param [입력] ptr 개체의 자기 자신 값.
 * @param	name	   	인수 이름.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mltag_contains_arg(qnMlTag* ptr, const char* name)
{
	return qn_mltag_get_arg(ptr, name, NULL) != NULL;
}

/**
 * 인수에 대해 ForEach 연산을 수행한다.
 * @param [입력,반환]	ptr 	(널값이 아니면) 포인터.
 * @param [입력]	func	콜백 함수.
 * @param	userdata		콜백 데이터.
 */
void qn_mltag_foreach_arg(qnMlTag* ptr, void(*func)(pointer_t userdata, const char* name, const char* data), pointer_t userdata)
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_hash_ptr_foreach(ArgHash, &self->harg, func, userdata);
}

/**
 * 인수에 대해 LoopEach 연산을 수행한다.
 * @param [입력,반환]	ptr 	(널값이 아니면) 포인터.
 * @param [입력]	func	콜백 함수.
 */
void qn_mltag_loopeach_arg(qnMlTag* ptr, void(*func)(const char* name, const char* data))
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_hash_ptr_loopeach(ArgHash, &self->harg, func);
}

/**
 * 인수를 추가한다.
 * @param [입력] ptr 개체의 자기 자신 값.
 * @param	name	   	인수 이름.
 * @param	value	   	인수 값.
 */
void qn_mltag_set_arg(qnMlTag* ptr, const char* name, const char* value)
{
	qnRealTag* self = (qnRealTag*)ptr;
	char* dn;
	char* dv;

	qn_ret_if_fail(name);

	dn = _strdup(name);
	dv = _strdup(value);

	_strupr_s(dn, strlen(dn));

	if (!self->harg.bucket)
		qn_hash_init(ArgHash, &self->harg);

	qn_hash_set(ArgHash, &self->harg, &dn, &dv);
}

/**
 * 인수를 제거한다.
 * @param [입력] ptr 개체의 자기 자신 값.
 * @param	name	   	제거할 인수 이름.
 * @return	성공하면 참을, 실패하면 거짓을 반환한다.
 */
bool qn_mltag_remove_arg(qnMlTag* ptr, const char* name)
{
	qn_retval_if_fail(name, FALSE);

	qnRealTag* self = (qnRealTag*)ptr;
#if !__GNUC__

	bool ret;
	qn_hash_remove(ArgHash, &self->harg, &name, &ret);

	return ret;
#else
	qn_hash_remove(ArgHash, &self->harg, &name, NULL);

	return TRUE;
#endif
}
