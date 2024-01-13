// rdh 테스트
#include <qs.h>

typedef struct LoopData
{
	float acc;
	float dir;
} LoopData;

void loop(void* loop_data)
{
	LoopData* data = loop_data;

	float f = qg_get_advance() / 5.0f;
	data->acc += f * data->dir;
	if (data->acc > 1.0f)
	{
		data->acc = 1.0f;
		data->dir = -1.0f;
	}
	if (data->acc < 0.0f)
	{
		data->acc = 0.0f;
		data->dir = 1.0f;
	}

	QmColor cc = qm_color(data->acc, data->acc, data->acc, 1.0f);
	qg_rdh_set_background(&cc);

	if (qg_rdh_begin(true))
	{
		qg_rdh_end();
		qg_rdh_flush();
	}
}

int event_callback(void* event_data, QgEventType event_type, const QgEvent* event_param)
{
	LoopData* data = event_data;
	const char* evstr = qg_string_event(event_type);

	if (event_type == QGEV_MOUSEMOVE)
	{
		//qn_outputf("마우스 이동 => %d, %d", event_param->mmove.pt.X, event_param->mmove.pt.Y);
	}
	else if (event_type == QGEV_WINDOW)
	{
		const char* wevstr = qg_string_window_event(event_param->wevent.mesg);
		qn_outputf("\tWINDOW EVENT => %d:%s", event_param->wevent.mesg, wevstr);
	}
	else if (event_type == QGEV_KEYDOWN)
	{
		QikKey key = event_param->key.key;
		if (key == QIK_ESC)
			qg_exit_loop();
		else if (key == QIK_SPACE)
		{
			data->acc = 0.0f;
			data->dir = 1.0f;
		}
		else if (key == QIK_F1)
		{
			bool fullscreen = qg_query_fullscreen();
			qg_toggle_fullscreen(!fullscreen);
		}
		const char* key_name = qg_qik_str(key);
		if (key_name)
			qn_outputf("키 눌림 => <%s>(%X)", key_name, key);
	}
	else if (event_type == QGEV_KEYUP)
	{
		const char* key_name = qg_qik_str(event_param->key.key);
		if (key_name)
			qn_outputf("키 떼임 => <%s>(%X)", key_name, event_param->key.key);
	}
	else if (event_type == QGEV_TEXTINPUT && event_param->text.len > 0)
	{
#ifdef _QN_WINDOWS_
		wchar wz[32];
		qn_u8to16(wz, QN_COUNTOF(wz), event_param->text.data, event_param->text.len);
		qn_outputf("\t텍스트 입력: <%ls>(%d)", wz, event_param->text.len);
#else
		char sz[32];
		qn_strncpy(sz, event_param->text.data, (size_t)event_param->text.len);
		qn_outputf("\t텍스트 입력: <%s>(%d)", sz, event_param->text.len);
#endif
	}
	else
	{
		qn_outputf("STUB EVENT => %d:%s", event_type, evstr);
	}
	return 0;
}

int main(void)
{
	qn_runtime();

	int flags = QGFLAG_RESIZE | QGFLAG_VSYNC | QGFLAG_MSAA /*| QGFLAG_TEXT*/;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh(NULL, "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	for (int i = 0; ; i++)
	{
		const QgUdevMonitor* mon = qg_get_monitor(i);
		if (mon == NULL)
			break;
		qn_outputf("모니터%d: %s (%d,%d):(%d,%d)", i, mon->name, mon->x, mon->y, mon->width, mon->height);
	}

	LoopData data = { .acc = 0.0f, .dir = 1.0f };
	qg_register_event_callback(event_callback, &data);
	qg_main_loop(loop, &data);

	qg_close_rdh();

	return 0;
}
