// HFS 테스트
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

static void optimize_callback(void* userdata, const struct HFSOPTIMIZEDATA* data)
{
	qn_outputf("진행파일#%u: %s (크기: %u)", data->count, data->name, data->size);
}

QN_IMPL_BSTR(QnPathStr, QN_MAX_PATH, qn_path_str);
QN_DECLIMPL_ARRAY(FileInfoArray, QnPathStr, file_infos);

static void subdirectory_list(QnMount* mnt, const char* path)
{
	qn_mount_chdir(mnt, path);
	QnDir* dir = qn_mount_list(mnt);
	if (dir == NULL)
		return;

	size_t i;
	FileInfoArray files;
	file_infos_init(&files, 0);

	QnFileInfo fi;
	while (qn_dir_read_info(dir, &fi))
	{
		int pos = qn_dir_tell(dir);
		QnDateTime ft = { fi.stc };
		if (QN_TMASK(fi.attr, QNFATTR_DIR))
		{
			if (fi.name[0] != '.')
			{
				QnPathStr s;
				qn_path_str_set(&s, fi.name);
				file_infos_add(&files, s);
			}

			qn_outputf("%d, [디렉토리] %s [%04d-%02d-%02d %02d:%02d:%02d]",
				pos, fi.name,
				ft.year, ft.month, ft.day, ft.hour, ft.minute, ft.second);
		}
		else
		{
			qn_outputf("%d, [파일] %s (%u) [%04d-%02d-%02d %02d:%02d:%02d]",
				pos, fi.name, fi.size,
				ft.year, ft.month, ft.day, ft.hour, ft.minute, ft.second);
		}
	}

	QN_CTNR_FOREACH(files, i)
	{
		QnPathStr* s = file_infos_nth_ptr(&files, i);
		qn_outputf("하위 디렉토리: %s", s->DATA);
		subdirectory_list(mnt, s->DATA);
	}

	file_infos_dispose(&files);
	qn_unload(dir);
	qn_mount_chdir(mnt, "..");
}

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
		qn_hfs_store_data(mnt, "one summer night.txt", one_summer_night, (uint)QN_COUNTOF(one_summer_night), false, QNFTYPE_TEXT);
		qn_hfs_store_data(mnt, "one summer night.txt", one_summer_night, (uint)QN_COUNTOF(one_summer_night), false, QNFTYPE_TEXT);
		qn_hfs_store_data(mnt, "one summer night.cmpr", one_summer_night, (uint)QN_COUNTOF(one_summer_night), true, QNFTYPE_TEXT);
		qn_hfs_store_file(mnt, NULL, "QsLib.vcxproj", true, QNFTYPE_MARKUP);
		qn_hfs_store_file(mnt, "qlem.html", "QsLibEm.html", true, QNFTYPE_MARKUP);
		qn_hfs_store_file(mnt, "qlem.cmd", "QsLibEm.cmd", true, QNFTYPE_SCRIPT);
		qn_mount_remove(mnt, "qlem.html");
		qn_mount_chdir(mnt, "/");
		qn_mount_chdir(mnt, "test");

		qn_unload(mnt);
	}

	// 만든 hfs 열어서 테스트 (메모리로 올리기)
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

		// 파일 목록 테스트
		QnDir* dir = qn_mount_list(mnt);
		if (dir != NULL)
		{
			QnFileInfo fi;
			while (qn_dir_read_info(dir, &fi))
			{
				int pos = qn_dir_tell(dir);
				QnDateTime ft = { fi.stc };
				qn_outputf("%d, [%s] %s (%u) [%04d-%02d-%02d %02d:%02d:%02d]",
					pos, QN_TMASK(fi.attr, QNFATTR_DIR) ? "디렉토리" : "파일", fi.name, fi.size,
					ft.year, ft.month, ft.day, ft.hour, ft.minute, ft.second);
			}
			qn_unload(dir);
		}

		// HS 최적화 테스트
		HfsOptimizeParam param =
		{
			"test_opt.hfs",
			"optimized hfs",
			NULL,
			optimize_callback
		};
		qn_hfs_optimize(mnt, &param);

		qn_unload(mnt);
	}

	// 옵티마이즈 파일 목록 확인
	mnt = qn_open_mount("test_opt.hfs", "h");
	if (mnt)
	{
		subdirectory_list(mnt, "/");
		qn_unload(mnt);
	}

	return 0;
}

