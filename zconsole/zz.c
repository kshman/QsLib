// 스프라이트 테스트
#include <qs.h>

int main(void)
{
	qn_runtime(NULL);

	int flags = QGFLAG_RESIZE | QGFLAG_MSAA | QGFLAG_VSYNC;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh("", "RDH", 0, 0, 0, flags, features) == false)
		return -1;
	qg_fuse(0, NULL, false, true);

	QmVec bgc = qm_vec(0.1f, 0.3f, 0.1f, 1.0f);
	qg_set_background(&bgc);

	QgImage* img_puru = qg_load_image(0, "/image/ff14_puru.jpg");
	QgTexture* tex_puru = qg_create_texture("puru", img_puru, QGTEXF_DISCARD_IMAGE | QGTEXF_MIPMAP);
#ifdef _QN_EMSCRIPTEN_
	QgTexture* tex_autumn = qg_load_texture(0, "/image/ff14_autumn.bmp", QGTEXF_LINEAR);
#else
	QgTexture* tex_autumn = qg_load_texture(0, "/image/ff14_autumn.dds", QGTEXF_LINEAR);
#endif

	QgFont* font = qg_load_font(0, "/font/eunjin.ttf", 48);

	float f = 0.0f, dir = 1.0f, angle = 0.0f;
	while (qg_loop())
	{
		QgEvent ev;
		while (qg_poll(&ev))
		{
			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
			{
				qn_sym_dbgout();
				qn_prop_dbgout();
#ifndef _QN_EMSCRIPTEN_
				qn_mpf_dbgout();
#endif
				qg_exit_loop();
			}
			else if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_F1)
			{
				static bool fullscreen = false;
				fullscreen = !fullscreen;
				qg_toggle_fullscreen(fullscreen);
			}
		}

		f += qg_get_advance() * 0.5f * dir;
		if (f > 1.0f)
		{
			f = 1.0f;
			dir = -1.0f;
		}
		else if (f < 0.0f)
		{
			f = 0.0f;
			dir = 1.0f;
		}

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
			qg_draw_sprite(&rt, tex_autumn, &color, &coord);

			rt = qm_rect_size(size.Width - 10 - tex_autumn->width, size.Height - 10 - tex_autumn->height, tex_autumn->width, tex_autumn->height);
			color = qm_vec(1.0f, 1.0f, 1.0f, 1.0f - f);
			qg_draw_sprite(&rt, tex_autumn, &color, NULL);

			rt = qm_rect_size(pt_puru.X, pt_puru.Y, tex_puru->width, tex_puru->height);
			qg_draw_sprite_ex(&rt, angle, tex_puru, NULL, NULL);

			qg_font_draw_format(font, 0, 0, "World, Hello! 한글도 나오나? (%.2f)", qg_get_fps());

			qg_end_render(true);
		}
	}

	qn_unload(font);
	qn_unload(tex_autumn);
	qn_unload(tex_puru);
	qg_close_rdh();

	return 0;
}
