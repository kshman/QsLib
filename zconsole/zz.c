// 스프라이트 테스트
#include <qs.h>

int main(void)
{
	qn_runtime(NULL);

	int flags = QGFLAG_RESIZE | QGFLAG_MSAA /*| QGFLAG_VSYNC*/;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh("", "RDH", 0, 0, 0, flags, features) == false)
		return -1;
	qg_fuse(0, NULL, true, true);

	QmVec bgc = qm_vec(0.1f, 0.3f, 0.1f, 1.0f);
	qg_set_background(&bgc);
	qg_load_def_font(0, "/font/eunjin.ttf", 48);

	QgCamera* cam = qg_create_camera();

	static QgLayoutInput li_3d[] =
	{
		{ QGLOS_1, QGLOU_POSITION, QGLOT_FLOAT3, false },
		{ QGLOS_1, QGLOU_NORMAL1, QGLOT_FLOAT3, false},
		{ QGLOS_1, QGLOU_COORD1, QGLOT_FLOAT2, false },
		{ QGLOS_1, QGLOU_COLOR1, QGLOT_BYTE4, true },
	};
	static QgLayoutData ld_3d = { li_3d, QN_COUNTOF(li_3d) };

	QgMesh* mesh = qg_create_mesh("cube");
	qg_mesh_set_layout(mesh, &ld_3d);
	qg_mesh_gen_cube(mesh, 1.0f, 1.0f, 1.0f);
	qg_mesh_build(mesh);

	while (qg_loop())
	{
		QgEvent ev;
		while (qg_poll(&ev))
		{
			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
				qg_exit_loop();
			else if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_F1)
			{
				static bool fullscreen = false;
				fullscreen = !fullscreen;
				qg_toggle_fullscreen(fullscreen);
			}
		}

		qg_camera_update(cam);

		if (qg_begin_render(true))
		{
			qg_draw_text_format(0, 0, "FPS: %.2f", qg_get_afps());
			qg_end_render(true);
		}
	}

	qn_unload(mesh);
	qn_unload(cam);
	qg_close_rdh();

	return 0;
}
