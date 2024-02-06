// 스프라이트 테스트
#include <qs.h>

static const char one_summer_night[] =
"One summer night, the stars were shining bright\n"
"One summer dream made with fancy whims\n"
"That summer night, my whole world tumbled down\n"
"I could have died, if not for you\n"
"\n"
"Each night I'd pray for you\n"
"My heart would cry for you\n"
"The sun won't shine again\n"
"Since you have gone\n"
"\n"
"Each time I'd think of you\n"
"My heart would beat for you\n"
"You are the one for me\n"
"Set me free like sparrows up the trees\n"
"\n"
"Give a sign\n"
"So I would ease my mind\n"
"Just say a word\n"
"And I'll come running wild\n"
"\n"
"Give me a chance to live again\n"
"Each night I'd pray for you\n"
"My heart would cry for you\n"
"\n"
"The sun won't shine again\n"
"Since you have gone\n"
"\n"
"Each time I'd think of you\n"
"My heart would beat for you\n"
"You are the one for me\n";

int main(void)
{
	qn_runtime();

	QnMount* mnt = qn_open_mount("test.hfs", "hc");
	if (mnt != NULL)
	{
		qn_outputs("test 디렉토리 만들기");
		qn_mount_mkdir(mnt, "test");
		qn_outputs("000 디렉토리 만들기");
		qn_mount_mkdir(mnt, "000");
		qn_outputs("test 디렉토리 들어가기");
		qn_mount_chdir(mnt, "test");
		qn_outputs("부모 디렉토리로 돌아가기");
		qn_mount_chdir(mnt, "..");
		qn_outputs("최상위 디렉토리로 돌아가기");
		qn_mount_chdir(mnt, "/");

		qn_outputs("test 디렉토리 들어가기");
		qn_mount_chdir(mnt, "test");

		qn_outputs("123 / 456 디렉토리 만들기");
		qn_mount_mkdir(mnt, "123");
		qn_mount_mkdir(mnt, "456");
		qn_mount_chdir(mnt, "456");
		qn_mount_mkdir(mnt, "3rd step directory");
		qn_mount_chdir(mnt, "..");

		qn_outputs("zzz(없는파일) 지우기");
		qn_mount_remove(mnt, "zzz");
		qn_outputs("123 디렉토리 지우기");
		qn_mount_remove(mnt, "123");
		qn_mount_chdir(mnt, "/");
		qn_mount_chdir(mnt, "/test/456/3rd step directory");
		qn_mount_mkdir(mnt, "/test/456/thisislongfilenamedirectoryisitwork");

		qn_mount_chdir(mnt, "/test");
		qn_hfs_store_file(mnt, "qlem.cmd", "QsLibEm.cmd", true, QNFTYPE_SCRIPT);
		qn_hfs_store_data(mnt, "one summer night.txt", one_summer_night, (uint)QN_COUNTOF(one_summer_night), false, QNFTYPE_TEXT);
		qn_hfs_store_data(mnt, "one summer night.txt", one_summer_night, (uint)QN_COUNTOF(one_summer_night), false, QNFTYPE_TEXT);
		qn_hfs_store_data(mnt, "one summer night.cmpr", one_summer_night, (uint)QN_COUNTOF(one_summer_night), true, QNFTYPE_TEXT);
		qn_hfs_store_file(mnt, NULL, "QsLib.vcxproj", true, QNFTYPE_MARKUP);
		qn_hfs_store_file(mnt, "qlem.html", "QsLibEm.html", true, QNFTYPE_MARKUP);
		qn_mount_chdir(mnt, "/");
		qn_mount_chdir(mnt, "test");

		qn_unload(mnt);
	}

	mnt = qn_open_mount("test.hfs", "hm");
	if (mnt)
	{
		QnFileAttr attr = qn_mount_exist(mnt, "/test/qlem.cmd");
		qn_outputf("qlem.cmd attribute: %d", attr);

		qn_mount_chdir(mnt, "/test");

		int size;
		char *psz;
		QnStream* stream = qn_mount_open_stream(mnt, "one summer night.txt", NULL);
		if (stream)
		{
			size = (int)qn_stream_size(stream);
			psz = qn_alloc(size + 1, char);
			if (qn_stream_read(stream, psz, 0, size) == size)
			{
				psz[size] = 0;
				qn_outputs(psz);
			}
			qn_free(psz);
			qn_unload(stream);
		}

		psz = qn_mount_read(mnt, "one summer night.cmpr", &size);
		qn_outputf("%*s", size, psz);
		qn_free(psz);

		psz = qn_mount_read_text(mnt, "/test/qlem.cmd", NULL, NULL);
		qn_outputs(psz);
		qn_free(psz);

		qn_unload(mnt);
	}

	return 0;
}
