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

// ��Ʈ�� ����
static void _error_array_delete_ptr(char** ptr)
{
	char* s = *ptr;
	qn_free(s);
}

// xml ����
static const char* _ml_header_desc = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";


//////////////////////////////////////////////////////////////////////////
// ���� ���
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
 * RML�� �����.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlu*.
 */
qnMlu* qn_mlu_new(void)
{
	qnMlu* self = qn_alloc_zero_1(qnMlu);
	return self;
}

/**
 * ���Ͽ��� RML�� �����.
 * @param	filename	������ �̸�.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlu*.
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
 * ���Ͽ��� RML�� �����. �����ڵ� ���� �̸��� ����Ѵ�.
 * @param	filename	������ �̸�.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlu*.
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
 * ���ۿ��� RML�� �����.
 * @param	data	����.
 * @param	size	���� ũ��.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlu*.
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
 * RML�� �����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
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
 * ��� RML �±׸� �����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 */
void qn_mlu_clean_tags(qnMlu* self)
{
	qn_arr_loopeach(SubArray, &self->tags, _qn_realtag_delete_ptr);
	qn_arr_clear(SubArray, &self->tags);
}

/**
 * ��� RML ������ �����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 */
void qn_mlu_clean_errs(qnMlu* self)
{
	qn_arr_loopeach(ErrorArray, &self->errs, _error_array_delete_ptr);
	qn_arr_clear(ErrorArray, &self->errs);
}

/**
 * �������� �߰��Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	msg			�޽���.
 */
void qn_mlu_add_err(qnMlu* self, const char* msg)
{
	char* dup;

	qn_ret_if_fail(msg);

	dup = _strdup(msg);
	qn_arr_add(ErrorArray, &self->errs, dup);
}

/**
 * �������� ���� ������� �߰��Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	fmt			���� ���ڿ�.
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
 * ���� �ִ� ������ ����� �ַܼ� ����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 */
void qn_mlu_print_err(qnMlu* self)
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

/**
 * RML ���� ���� ������ ����� �ַܼ� ����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 */
void qn_mlu_print(qnMlu* self)
{
	// NOT IMPL
}

/**
 * ���ۿ��� RML�� �м��Ͽ� �д´�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	data		����.
 * @param	size		���� ũ��.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mlu_load_buffer(qnMlu* self, const pointer_t data, int size)
{
	int i, n, at;
	int idn = 0;
	const char* pos;
	char* cd;
	char* psz;

	qn_retval_if_fail(data, false);
	qn_retval_if_fail(size > 0, false);

	qn_mlu_clean_tags(self);
	qn_mlu_clean_errs(self);

	// UTF8 ���� ã��
	i = (*(const int*)data) & 0x00FFFFFF;

	if (i == 0x00BFBBEF)
	{
		// ��� ��ŭ ����
		pos = (const char*)data + 3;
		size -= 3;
	}
	else
	{
		// Ȥ�� 16��Ʈ �����ڵ��ΰ�?
		i = (*(const uint16_t*)data);

		if (i == 0)
		{
			// �̰� 32��Ʈ �����ڵ�, �Ƹ� UTF32 BE(0x0000FEFF)
			// ó���� �� ����
			return false;
		}
		else if (i == 0xFEFF)
		{
			// �̰� UTF16 LE
			// ó������ ����
			return false;
		}
		else if (i == 0xFFFE)
		{
			// �̰� UTF16 BE, �Ǵ� UTF32 LE(0xFFFE0000)
			// ó�� ���ҷ�
			return false;
		}

		// ���� ����
		pos = (const char*)data;
	}

	//
	bool ret = false;
	bool iscmt = false;
	int line = 1;
	int cdsize = 0;

	StackList* stack = NULL;
	qnRealTag* tmptag = NULL;
	qnRealTag* curtag = NULL;

	cd = qn_alloc(size, char);

	qnBstr4k* btag = qn_alloc_1(qnBstr4k);     // �±״� 4k
	qnBstr4k* bname = qn_alloc_1(qnBstr4k);    // �̸��� 4k
	qnBstr4k* barg = qn_alloc_1(qnBstr4k);     // �μ��� 4k

	for (i = 0; i < size; i++)
	{
		char ch = pos[i];

		if (ch == '\n')
		{
			line++;

			if (iscmt)
				iscmt = false;

#if 0
			// ���� ��� �ּ� '#' �Ǵ� "//"
			n = i + 1;

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
			// �±� ����
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
					// �ּ�
					i += 3;
					n = size - 2;

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

			// �̸� + �μ�
			qn_bstr_init(btag, NULL);

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
						n = (int)btag->len - 1;
						btag->data[n] = '\0';
						btag->len = n;
						hassingle = true;
					}

					break;
				}
				else
					qn_bstr_append_char(btag, pos[i]);
			}

			// �μ� �˻�
			if (hasclosure)
			{
				// �ݴ¾ִ� �μ��� �����Ƿ�
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
					// �̸��� ����
					qn_bstr_init(barg, NULL);
					qn_bstr_set_bstr(bname, btag);
					qn_bstr_upper(bname);
				}
				else
				{
					// �̸��� �μ��� ����
					qn_bstr_sub_bstr(barg, btag, at + 1, -1);
					qn_bstr_sub_bstr(bname, btag, 0, at);
					qn_bstr_upper(bname);
				}
			}

			// �̸� �˻�
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

				// ������ �׳� ����
				continue;
			}

			// �ݱ� ���� ����
			if (hasclosure)
			{
				// ����� �ݱ�
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

				// ����
				if (cdsize > 0)
				{
					cd[cdsize] = '\0';
					qn_strtrim(cd);

					cdsize = (int)strlen(cd);

					if (cdsize > 0)
						qn_mltag_add_context((qnMlTag*)tmptag, cd, cdsize);

					cdsize = 0;
				}

				// �߰�
				if (!stack)
				{
					// ���� �ֱ�
					curtag = NULL;
					qn_arr_add(SubArray, &self->tags, tmptag);
				}
				else
				{
					// �Ϻη� �ֱ�
					curtag = stack->data;
					qn_arr_add(SubArray, &curtag->subs, tmptag);
				}
			}
			else
			{
				// ����� ����

				// ���� �����ִ� �±׿� �ڷᰡ ������ �ִ´�
				if (curtag && cdsize > 0)
				{
					cd[cdsize] = '\0';
					qn_strrem(cd, " \n\r\t");

					cdsize = (int)strlen(cd);

					if (cdsize > 0)
						qn_mltag_add_context((qnMlTag*)curtag, cd, cdsize);
				}

				// ����
				curtag = (qnRealTag*)qn_mltag_new(bname->data);
				if (!curtag)
				{
					qn_mlu_add_errf(self, "line#%d, out of memory.", line);
					goto pos_exit;
				}
				curtag->base.line = line;
				curtag->idn = idn++;

				// �μ�
				if (!_qn_realtag_parse_args(curtag, (qnBstr4k*)barg))
				{
					qn_mlu_add_errf(self, "line#%d, invalid argument.", line);
					goto pos_exit;
				}

				// �غ�
				if (!hassingle)
				{
					// ���� �±װ� �ƴϸ� �ְ� �غ�
					qn_slist_prepend(StackList, stack, curtag, &stack);
				}
				else
				{
					// ���� �±��̸� �׳� ����
					tmptag = curtag;

					if (!stack)
					{
						// ���� �ֱ�
						curtag = NULL;
						qn_arr_add(SubArray, &self->tags, tmptag);
					}
					else
					{
						// �Ϻη� �ֱ�
						tmptag->cls = true;

						curtag = stack->data;
						qn_arr_add(SubArray, &curtag->subs, tmptag);
					}
				}

				// ���� ���� �ڸ�
				cdsize = 0;
			}
		}   // ch=='<'
		else if (ch == '>')
		{
			// �±� �ݱ�
			psz = curtag && curtag->base.name ? curtag->base.name : "unknown";
			qn_mlu_add_errf(self, "line#%d, invalid tag '%d'.", line, psz);
			goto pos_exit;
		}   // ch=='>'
		else
		{
			// �±� ������
			if (curtag)
				cd[cdsize++] = ch;
		}
	}   // for ~size

	if (stack)
	{
		// ��...
		qn_mlu_add_errf(self, "stack has left! (count:%d)", qn_slist_count(stack));
	}
	else
	{
		// ����
		ret = true;
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
 * RML ������ ���Ϸ� �����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	filename	������ �̸�.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mlu_write_file(qnMlu* self, const char* filename)
{
	qnFile* file;

	qn_retval_if_fail(filename, false);
	qn_retval_if_fail(qn_arr_count(&self->tags) > 0, false);

	file = qn_file_new(filename, "w");
	qn_retval_if_fail(file, false);

	// UTF8 BOM
	int bom = 0x00BFBBEF;
	qn_file_write(file, &bom, 0, 3);

	// xml ��ȣ
	qn_file_write(file, (const pointer_t)_ml_header_desc, 0, (int)strlen(_ml_header_desc));

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* tag = qn_arr_nth(&self->tags, i);
		_qn_realtag_write_file(tag, file, 0);
	}

	qn_file_delete(file);

	return true;
}

/**
 * ���� �ִ� �ֻ��� �±��� ������ ��ȯ�Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @return	�ֻ��� �±� ����.
 */
int qn_mlu_get_count(qnMlu* self)
{
	return (int)qn_arr_count(&self->tags);
}

/**
 * ���� �ִ� ������ �������� ��´�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	at			���� ����.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� char*.
 */
const char* qn_mlu_get_err(qnMlu* self, int at)
{
	return at >= 0 && at < (int)qn_arr_count(&self->errs) ? qn_arr_nth(&self->errs, at) : NULL;
}

/**
 * �ֻ��� �±׸� ã�´�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	name		�±� �̸�.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlTag*.
 */
qnMlTag* qn_mlu_get_tag(qnMlu* self, const char* name)
{
	qn_retval_if_fail(name, NULL);

	size_t hash = qn_strhash(name);
	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->tags, i);

		if (mt->nhash == hash && strcmp(mt->base.name, name) == 0)
			return (qnMlTag*)mt;
	}

	return NULL;
}

/**
 * �ֻ��� �±׸� �������� ��´�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	at			����.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlTag*.
 */
qnMlTag* qn_mlu_get_tag_nth(qnMlu* self, int at)
{
	return at >= 0 && at < (int)qn_arr_count(&self->tags) ? (qnMlTag*)qn_arr_nth(&self->tags, at) : NULL;
}

/**
 * �ֻ��� �±װ� ���� �ִ� ���ؽ�Ʈ�� ��´�.
 * @param [�Է�]	self  	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	name	  	�±� �̸�.
 * @param	ifnotexist	�±װ� �������� ���� ��� ��ȯ�� ��.
 * @return	������ �ְų� �����ϸ� ifnotexist�� ��ȯ, ������ �� ��ȯ���� �±� ���ؽ�Ʈ.
 */
const char* qn_mlu_get_context(qnMlu* self, const char* name, const char* ifnotexist)
{
	qnMlTag* tag = qn_mlu_get_tag(self, name);
	return tag ? tag->cntx : ifnotexist;
}

/**
 * �ֻ��� �±װ� ���� �ִ� ���ؽ�Ʈ�� ��´�. �±״� �������� �˻��Ѵ�.
 * @param [�Է�]	self  	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	at		  	����.
 * @param	ifnotexist	�±װ� �������� ���� ��� ��ȯ�� ��.
 * @return	������ �ְų� �����ϸ� ifnotexist�� ��ȯ, ������ �� ��ȯ���� �±� ���ؽ�Ʈ.
 */
const char* qn_mlu_get_context_nth(qnMlu* self, int at, const char* ifnotexist)
{
	qnMlTag* tag = qn_mlu_get_tag_nth(self, at);
	return tag ? tag->cntx : ifnotexist;
}

/**
 * ������ �±װ� �ֳ� �˻��Ѵ�.
 * @param [�Է�]	self   	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param [�Է�]	tag	(�ΰ��� �ƴ�) ������ �±�.
 * @return	�±װ� ������ -1�� ������ �ش� ������ ��ȯ�Ѵ�.
 */
int qn_mlu_contains(qnMlu* self, qnMlTag* tag)
{
	int ret;
	qn_arr_contains(SubArray, &self->tags, (qnRealTag*)tag, &ret);
	return ret;
}

/**
 * �ֻ��� �±׿� ���� ForEach�� �����Ѵ�.
 * @param [�Է�]	self		��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param [�Է�]	func	�ݹ� �Լ�.
 * @param	userdata		�ݹ� ������.
 */
void qn_mlu_foreach(qnMlu* self, void(*func)(pointer_t userdata, qnMlTag* tag), pointer_t userdata)
{
	qn_ret_if_fail(func);

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->tags, i);
		func(userdata, (qnMlTag*)mt);
	}
}

/**
 * �ֻ��� �±׿� ���� LoopEach�� �����Ѵ�.
 * @param [�Է�]	self		��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param [�Է�]	func	�ݹ� �Լ�.
 */
void qn_mlu_loopeach(qnMlu* self, void(*func)(qnMlTag* tag))
{
	qn_ret_if_fail(func);

	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->tags, i);
		func((qnMlTag*)mt);
	}
}

/**
 * �±׸� �߰��Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	name		�±� �̸�.
 * @param	context 	�±� ���ؽ�Ʈ.
 * @param	line		�ٹ�ȣ.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� ������� �±�.
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
 * �±׸� �߰��Ѵ�.
 * @param [�Է�]	self   	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param [�Է�]	tag	(�ΰ��� �ƴ�) �߰��� �±�.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� �߰��� �±�.
 */
qnMlTag* qn_mlu_add_tag(qnMlu* self, qnMlTag* tag)
{
	qn_retval_if_fail(tag, NULL);

	qn_arr_add(SubArray, &self->tags, (qnRealTag*)tag);

	return tag;
}

/**
 * �±׸� �����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	name		�±� �̸�.
 * @param	isall   	���� �̸� �±׸� ��� ������� ������ �ִ´�.
 * @return	���� �±��� ����.
 */
int qn_mlu_remove(qnMlu* self, const char* name, bool isall)
{
	qn_retval_if_fail(name, -1);

	size_t hash = qn_strhash(name);
	int cnt = 0;
	for (size_t i = 0; i < qn_arr_count(&self->tags); i++)
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
 * �±׸� �������� �����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param	at			����.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mlu_remove_nth(qnMlu* self, int at)
{
	qnRealTag* tag;

	if (at < 0 || at >= (int)qn_arr_count(&self->tags))
		return false;

	tag = qn_arr_nth(&self->tags, at);
	qn_arr_remove_nth(SubArray, &self->tags, at);
	qn_mltag_delete((qnMlTag*)tag);

	return true;
}

/**
 * �±׸� �����Ѵ�.
 * @param [�Է�]	self   	��ü�� �������̽��� �ڱ� �ڽ� ��.
 * @param [�Է�]	tag	(�ΰ��� �ƴ�) ���� �±�.
 * @param	isdelete   	�±׸� �����Ϸ��� ������ �ִ´�.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mlu_remove_tag(qnMlu* self, qnMlTag* tag, bool isdelete)
{
	qnRealTag* mt = (qnRealTag*)tag;

	qn_retval_if_fail(tag, false);

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
// ���

/**
 * �±� ��带 �����.
 * @param	name	�±� �̸�.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlTag*.
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

// ����
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
 * �±� ��带 �����Ѵ�.
 * @param [�Է�]	self	��ü�� �������̽��� �ڱ� �ڽ� ��.
 */
void qn_mltag_delete(qnMlTag* self)
{
	qnRealTag* real = (qnRealTag*)self;
	_qn_realtag_delete_ptr(&real);
}

/**
 * �±� ������ �߰��Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param	cntx	   	���ؽ�Ʈ.
 * @param	size	   	���ؽ�Ʈ�� ũ��.
 */
void qn_mltag_add_context(qnMlTag* ptr, const char* cntx, int size)
{
	qnRealTag* self = (qnRealTag*)ptr;

	if (!cntx)
	{
		// ������� ��
	}
	else
	{
		// �������� ��
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
 * �±׿� ������ ����.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param	cntx	   	���ؽ�Ʈ.
 * @param	size	   	���ؽ�Ʈ�� ũ��.
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

// �μ� �м�
static bool _qn_realtag_parse_args(qnRealTag* self, qnBstr4k* bs)
{
	int at, eq;
	qnBstr1k k, v;   // Ű�� ���� ���� 1k

	qn_retval_if_fail(bs->len > 0, true);

	for (;;)
	{
		// Ű
		eq = qn_bstr_find_char(bs, 0, '=');

		if (eq < 0)
		{
			qn_bstr_trim(bs);

			if (bs->len > 0)
			{
				// ���� ���µ� Ű�� ������ �ȵ�
				return false;
			}

			break;
		}

		qn_bstr_sub_bstr(&k, bs, 0, eq);
		qn_bstr_trim(&k);

		qn_bstr_sub_bstr(bs, bs, eq + 1, -1);
		qn_bstr_trim_left(bs);

		// ��
		if (bs->len == 0)
		{
			// �Ƹ��� '='�ڿ� ���� ���� ��
			return false;
		}
		else if (qn_bstr_nth(bs, 0) == '"')
		{
			// ���� �ο��̸� ���� ���� �ο����
			at = qn_bstr_find_char(bs, 1, '"');

			if (at < 0)
			{
				// ������ ����
				return false;
			}

			qn_bstr_sub_bstr(&v, bs, 1, at - 1);
			qn_bstr_sub_bstr(bs, bs, at + 1, -1);
		}
		else if (qn_bstr_nth(bs, 0) == '\'')
		{
			// ���� �ο��̸� ���� ���� �ο����
			at = qn_bstr_find_char(bs, 1, '\'');

			if (at < 0)
			{
				// ������ ����
				return false;
			}

			qn_bstr_sub_bstr(&v, bs, 1, at - 1);
			qn_bstr_sub_bstr(bs, bs, at + 1, -1);
		}
		else
		{
			// ���� �������
			at = qn_bstr_find_char(bs, 0, ' ');

			if (at < 0)
			{
				// ��ü ��
				qn_bstr_set_bstr(&v, bs);
				qn_bstr_trim(&v);

				if (v.len == 0)
				{
					// �Ƹ��� �������� '='�� �ִ� ��
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

		// ����
		qn_mltag_set_arg((qnMlTag*)self, k.data, v.data);
	}

	return true;
}

// �μ� ���Ͽ� ����
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

// ���Ͽ� ����
static bool _qn_realtag_write_file(qnRealTag* self, qnFile* file, int ident)
{
	bool isbody = false;
	char ch;
	char szident[260];
	qnBstr2k bs;

	qn_strfll(szident, 0, ident, '\t');
	szident[ident] = '\0';

	// �Ʒ� ����. �Ǵ� ���� ���
	isbody = qn_arr_count(&self->subs) > 0 || self->base.clen > 64 ? true : false;

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

		// ����
		if (self->base.clen > 0)
		{
			qn_file_write(file, szident, 0, ident);

			ch = '\t';
			qn_file_write(file, &ch, 0, sizeof(char));

			qn_file_write(file, self->base.cntx, 0, self->base.clen);

			ch = '\n';
			qn_file_write(file, &ch, 0, sizeof(char));
		}

		// �ڽ�
		ident++;

		for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
		{
			qnRealTag* tag = qn_arr_nth(&self->subs, i);
			_qn_realtag_write_file(tag, file, ident);
		}

		//
		qn_bstr_format(&bs, "%s</%s>\n", szident, self->base.name);
		qn_file_write(file, bs.data, 0, (int)bs.len);
	}

	return true;
}

/**
 * �Ϻ� �±��� ������ ��´�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @return	�Ϻ� �±��� ����.
 */
int qn_mltag_get_sub_count(qnMlTag* ptr)
{
	qnRealTag* self = (qnRealTag*)ptr;

	return (int)qn_arr_count(&self->subs);
}

/**
 * �Ϻ� �±׸� ã�´�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param	name	   	ã�� �±� �̸�.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlTag*.
 */
qnMlTag* qn_mltag_get_sub(qnMlTag* ptr, const char* name)
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_retval_if_fail(name, NULL);

	size_t hash = qn_strhash(name);
	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->subs, i);

		if (mt->nhash == hash && strcmp(mt->base.name, name) == 0)
			return (qnMlTag*)mt;
	}

	return NULL;
}

/**
 * �Ϻ� �±׸� �������� ã�´�.
 * @param [�Է�] ptr ��ü�� �ڱ� �ڽ� ��.
 * @param	at		   	����.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlTag*.
 */
qnMlTag* qn_mltag_get_sub_nth(qnMlTag* ptr, int at)
{
	qnRealTag* self = (qnRealTag*)ptr;
	return at >= 0 && at < (int)qn_arr_count(&self->subs) ? (qnMlTag*)qn_arr_nth(&self->subs, at) : NULL;
}

/**
 * �Ϻ� �±��� ���ؽ�Ʈ�� ��´�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param	name	   	�±� �̸�.
 * @param	ifnotexist 	�±׸� ã�� �� ������ ��ȯ�� ��.
 * @return	������ �ְų� �����ϸ� ifnotexit�� ��ȯ, ������ �� ��ȯ���� char*.
 */
const char* qn_mltag_get_sub_context(qnMlTag* ptr, const char* name, const char* ifnotexist)
{
	qnMlTag* tag = qn_mltag_get_sub(ptr, name);
	return tag ? tag->cntx : ifnotexist;
}

/**
 * �Ϻ� �±��� ���ؽ�Ʈ�� �������� ��´�.
 * @param [�Է�] ptr ��ü�� �ڱ� �ڽ� ��.
 * @param	at			����.
 * @param	ifnotexist 	�±׸� ã�� �� ������ ��ȯ�� ��.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� char*.
 */
const char* qn_mltag_get_sub_context_nth(qnMlTag* ptr, int at, const char* ifnotexist)
{
	qnMlTag* tag = qn_mltag_get_sub_nth(ptr, at);
	return tag ? tag->cntx : ifnotexist;
}

/**
 * ������ �±׸� �Ϻ� �±׷� ���� �ֳ� ����.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param [�Է�]	tag	(�ΰ��� �ƴϸ�) ã�� �±�.
 * @return	ã�� ���ϸ� -1, �ƴϸ� �ش� ������ ��ȯ.
 */
int qn_mltag_contains_sub(qnMlTag* ptr, qnMlTag* tag)
{
	qnRealTag* self = (qnRealTag*)ptr;
	int ret;

	qn_arr_contains(SubArray, &self->subs, (qnRealTag*)tag, &ret);

	return ret;
}

/**
 * �Ϻ� �±׿� ���� ForEach ������ �����Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param [�Է�]	func	�ݹ� �Լ�.
 * @param	userdata		�ݹ� ������.
 */
void qn_mltag_foreach_sub(qnMlTag* ptr, void(*func)(pointer_t userdata, qnMlTag* tag), pointer_t userdata)
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_ret_if_fail(func);

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->subs, i);
		func(userdata, (qnMlTag*)mt);
	}
}

/**
 * �Ϻ� �±׿� ���� LoopEach ������ �����Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param [�Է�]	func	�ݹ� �Լ�.
 */
void qn_mltag_loopeach_sub(qnMlTag* ptr, void(*func)(qnMlTag* tag))
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_ret_if_fail(func);

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->subs, i);
		func((qnMlTag*)mt);
	}
}

/**
 * �Ϻ� �±׸� �߰��Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param	name	   	�±� �̸�.
 * @param	context	   	�±� ���ؽ�Ʈ.
 * @param	line	   	�� ��ȣ.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlTag*.
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
 * �Ϻ� �±׸� �߰��Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param [�Է�]	tag	�߰��� �±�.
 * @return	������ �ְų� �����ϸ� �ΰ��� ��ȯ, ������ �� ��ȯ���� qnMlTag*.
 */
qnMlTag* qn_mltag_add_sub_tag(qnMlTag* ptr, qnMlTag* tag)
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_retval_if_fail(tag, NULL);

	qn_arr_add(SubArray, &self->subs, (qnRealTag*)tag);

	return tag;
}

/**
 * ������ �̸��� �±׸� �����Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param	name	   	�±� �̸�.
 * @param	isall	   	���� �̸��� ��� �±׸� ������� ������ �ִ´�.
 * @return	���� �±��� ����.
 */
int qn_mltag_remove_sub(qnMlTag* ptr, const char* name, bool isall)
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_retval_if_fail(name, -1);

	size_t hash = qn_strhash(name);
	int cnt = 0;

	for (size_t i = 0; i < qn_arr_count(&self->subs); i++)
	{
		qnRealTag* mt = qn_arr_nth(&self->subs, i);

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
 * ������ ������ �Ϻ� �±׸� �����Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param	at			����.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mltag_remove_sub_nth(qnMlTag* ptr, int at)
{
	qnRealTag* self = (qnRealTag*)ptr;
	qnRealTag* mt;

	if (at < 0 || at >= (int)qn_arr_count(&self->subs))
		return false;

	mt = qn_arr_nth(&self->subs, at);
	qn_arr_remove_nth(SubArray, &self->subs, at);
	qn_mltag_delete((qnMlTag*)mt);

	return true;
}

/**
 * ������ �Ϻ� �±׸� �����Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @param [�Է�,��ȯ]	tag	(�ΰ��� �ƴ�) ���� �±�.
 * @param	isdelete   	�±� ��ü�� �����Ϸ��� ������ �ִ´�.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mltag_remove_sub_tag(qnMlTag* ptr, qnMlTag* tag, bool isdelete)
{
	qnRealTag* self = (qnRealTag*)ptr;
	qnRealTag* mt = (qnRealTag*)tag;

	qn_retval_if_fail(tag, false);

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

/**
 * �±��� �μ��� ������ ��ȯ�Ѵ�.
 * @param [�Է�]	ptr	��ü�� �ڱ� �ڽ� ��.
 * @return	�μ��� ����.
 */
int qn_mltag_get_arity(qnMlTag* ptr)
{
	qnRealTag* self = (qnRealTag*)ptr;

	return (int)qn_hash_count(&self->harg);
}

/**
 * �μ��� �̸����� ã�´�.
 * @param [�Է�] ptr ��ü�� �ڱ� �ڽ� ��.
 * @param	name	   	�μ� �̸�.
 * @param	ifnotexist 	�μ��� ã�� ���ϸ� ��ȯ�� ��.
 * @return	������ �ְų� �����ϸ� ifnotexist�� ��ȯ, ������ �� ��ȯ���� �μ��� ������.
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
 * ���� �μ��� ã�´�.
 * @param [�Է�,��ȯ]	ptr  	(�ΰ��� �ƴϸ�) ������.
 * @param [�Է�,��ȯ]	index	(�ΰ��� �ƴ�) ���� ã�� �ε��� ������.
 * @param	name		 	�μ� �̸�.
 * @param	data		 	�μ� �ڷ�.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mltag_next_arg(qnMlTag* ptr, pointer_t* index, const char** name, const char** data)
{
	qnRealTag* self = (qnRealTag*)ptr;
	ArgHashNode* node;

	qn_retval_if_fail(index, false);

	if (*index == (pointer_t)(intptr_t)-1)
		return false;

	node = !*index ? self->harg.frst : (ArgHashNode*)*index;

	if (name) *name = node->key;

	if (data) *data = node->value;

	*index = node->next ? node->next : (pointer_t)(intptr_t)-1;

	return true;
}

/**
 * �̸��� �ش��ϴ� �μ��� �ִ��� �����Ѵ�.
 * @param [�Է�] ptr ��ü�� �ڱ� �ڽ� ��.
 * @param	name	   	�μ� �̸�.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mltag_contains_arg(qnMlTag* ptr, const char* name)
{
	return qn_mltag_get_arg(ptr, name, NULL) != NULL;
}

/**
 * �μ��� ���� ForEach ������ �����Ѵ�.
 * @param [�Է�,��ȯ]	ptr 	(�ΰ��� �ƴϸ�) ������.
 * @param [�Է�]	func	�ݹ� �Լ�.
 * @param	userdata		�ݹ� ������.
 */
void qn_mltag_foreach_arg(qnMlTag* ptr, void(*func)(pointer_t userdata, const char* name, const char* data), pointer_t userdata)
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_hash_ptr_foreach(ArgHash, &self->harg, func, userdata);
}

/**
 * �μ��� ���� LoopEach ������ �����Ѵ�.
 * @param [�Է�,��ȯ]	ptr 	(�ΰ��� �ƴϸ�) ������.
 * @param [�Է�]	func	�ݹ� �Լ�.
 */
void qn_mltag_loopeach_arg(qnMlTag* ptr, void(*func)(const char* name, const char* data))
{
	qnRealTag* self = (qnRealTag*)ptr;

	qn_hash_ptr_loopeach(ArgHash, &self->harg, func);
}

/**
 * �μ��� �߰��Ѵ�.
 * @param [�Է�] ptr ��ü�� �ڱ� �ڽ� ��.
 * @param	name	   	�μ� �̸�.
 * @param	value	   	�μ� ��.
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
 * �μ��� �����Ѵ�.
 * @param [�Է�] ptr ��ü�� �ڱ� �ڽ� ��.
 * @param	name	   	������ �μ� �̸�.
 * @return	�����ϸ� ����, �����ϸ� ������ ��ȯ�Ѵ�.
 */
bool qn_mltag_remove_arg(qnMlTag* ptr, const char* name)
{
	qn_retval_if_fail(name, false);

	qnRealTag* self = (qnRealTag*)ptr;
#if !__GNUC__

	bool ret;
	qn_hash_remove(ArgHash, &self->harg, &name, &ret);

	return ret;
#else
	qn_hash_remove(ArgHash, &self->harg, &name, NULL);

	return true;
#endif
}
