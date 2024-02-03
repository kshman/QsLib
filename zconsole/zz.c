// 스프라이트 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	QnMount* mnt = qn_open_mount(NULL, 0);
	const char* name = qn_mount_get_name(mnt);
	qn_outputf("Mount name: %s\n", name);
	const char* path = qn_mount_get_path(mnt);
	qn_outputf("Mount path: %s\n", path);
	qn_mount_mkdir(mnt, "test");
	qn_mount_chdir(mnt, "test");
	qn_mount_chdir(mnt, "..");
	qn_mount_remove(mnt, "test");
	qn_mount_chdir(mnt, "..");
	qn_mount_chdir(mnt, NULL);
	qn_unload(mnt);


	int flags = QGFLAG_RESIZE | QGFLAG_VSYNC | QGFLAG_MSAA;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh("gl", "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	QmVec bgc = qm_vec(0.1f, 0.3f, 0.1f, 1.0f);
	qg_set_background(&bgc);

	QgImage* img_puru = qg_load_image(0, "../res/image/ff14_puru.jpg");
	QgTexture* tex_puru = qg_create_texture("puru", img_puru, QGTEXF_DISCARD_IMAGE | QGTEXF_MIPMAP);
	QgTexture* tex_autumn = qg_load_texture(0, "../res/image/ff14_autumn.dds", QGTEXF_LINEAR);

	float f = 0.0f, angle = 0.0f;
	while (qg_loop())
	{
		QgEvent ev;
		while (qg_poll(&ev))
		{
			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
			{
				qn_mpfdbgprint();
				qg_exit_loop();
			}
			else if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_F1)
			{
				static bool fullscreen = false;
				fullscreen = !fullscreen;
				qg_toggle_fullscreen(fullscreen);
			}
		}

		f += qg_get_advance() * 0.5f;
		if (f > 1.0f)
			f = 0.0f;

		angle += qg_get_advance() * QM_DEG_360 * (-1.0f / 3.0f);
		if (angle >= QM_DEG_360)
			angle -= QM_DEG_360;
		else if (angle <= -QM_DEG_360)
			angle += QM_DEG_360;

		QmSize size;
		qg_get_size(&size);
		QmPoint pt_puru = qm_size_locate_center(size, tex_puru->width, tex_puru->height);

		if (qg_begin_render(true))
		{
			QmRect rt;
			QmVec color, coord;

			rt = qm_rect_size(10, 10, tex_autumn->width, tex_autumn->height);
			coord = qm_vec(0.0f, 0.0f, 4.0f, 4.0f);
			color = qm_vec(f, f, f, 1.0f);
			qg_draw_sprite(&rt, &color, tex_autumn, &coord);

			rt = qm_rect_size(size.Width - 10 - tex_autumn->width, size.Height - 10 - tex_autumn->height, tex_autumn->width, tex_autumn->height);
			color = qm_vec(1.0f, 1.0f, 1.0f, 1.0f - f);
			qg_draw_sprite(&rt, &color, tex_autumn, NULL);

			rt = qm_rect_size(pt_puru.X, pt_puru.Y, tex_puru->width, tex_puru->height);
			qg_draw_sprite_ex(&rt, angle, NULL, tex_puru, NULL);

			qg_end_render(true);
		}
	}

	qn_unload(tex_autumn);
	qn_unload(tex_puru);
	qg_close_rdh();

	return 0;
}
