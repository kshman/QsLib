// 스프라이트 테스트
#include <qs.h>

int main(void)
{
	qn_runtime(NULL);

	const int flags = QGFLAG_RESIZE | QGFLAG_MSAA /*| QGFLAG_VSYNC*/;
	const int features = QGFEATURE_NONE;
	if (qg_open_rdh("gl", "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	const QMVEC bgc = qm_vec(0.1f, 0.3f, 0.1f, 1.0f);
	qg_set_background(&bgc);
	qg_font_set_size(qg_get_def_font(), 32);
	qg_fuse(0, NULL, true, true);

	QgFont* truetype = qg_load_font(0, "/font/leesunshin_dotum_l.ttf", 0);
	//qg_font_add(truetype, 0, "/font/mikachan.ttf");

	QgFont* atlas = qg_load_font(0, "/font/kopubs_l.hxn", 0);

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

		if (qg_begin_render(true))
		{
			qg_draw_text_format(0, 0, "FPS: %.2f", qg_get_afps());
			qg_draw_text(0, 50, "ABCD 한글이 될까?!@#$%");
			qg_draw_text(0, 85, "안나오는 글자는 ＃ <- 이렇게 나온다");

			qg_font_write(truetype, 0, 130, "트루타입 폰트: ABCD 이건 한글이 된다.!@#$%");
			qg_font_write(truetype, 0, 165, "한자나 외국어가 안나옴: トルータイプは色んな言語も支援します。漢字！＠＃＄％");

			qg_font_write(atlas, 0, 200, "아틀라스 폰트: ABCDEFGHijklmnopqrsUVWzyx,./;'[]");
			qg_font_write(atlas, 0, 235, "さようならする為に出会ったのならなぜ僕たち結ばれたの");
			qg_font_write(atlas, 0, 270, "`1234567890-=~!@#$%^&*()_+");
			qg_font_write(atlas, 0, 305, "アトラスフォントは色んな言語を支援します。漢字！＠＃＄％");

			qg_end_render(true);
		}
	}

#ifndef _QN_EMSCRIPTEN_
	qn_mpf_dbgout();
#endif

	qn_unload(atlas);
	qn_unload(truetype);
	qg_close_rdh();

	return 0;
}
