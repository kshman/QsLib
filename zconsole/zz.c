// 스프라이트 테스트
#include <qs.h>

typedef struct VERTEXTYPE
{
	QmFloat3 position;
	QmKolor color;
} VertexType;

int main(void)
{
	qn_runtime(NULL);

	const int flags = QGFLAG_RESIZE | QGFLAG_MSAA /*| QGFLAG_VSYNC*/;
	const int features = QGFEATURE_NONE;
	if (qg_open_rdh("", "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	qg_set_background_param(0.1f, 0.3f, 0.1f, 1.0f);
	qg_fuse(0, NULL, true, true);
	qg_set_def_font_size(32);

	// 항성 정보
	const float constant_dist = 5.0f;

	// 카메라 만들고
	QgCamera* cam = qg_create_camera();
	qg_camera_set_position_param(cam, 0.0f, 0.0f, -15.0f);

	// 메시용 레이아웃
	const QgLayoutData* layout = qg_get_layout_data(QGLDP_1PNC);
	const QgPropRender* render = qg_get_prop_render_default();
	QgRenderState* rs = qg_create_render_state_vsps(NULL, render, layout, 0, "/shader/simple_vs.glsl", "/shader/simple_ps.glsl");

	// 큐브 메시
	QgMesh* mesh = qg_create_mesh("cube");
	qg_mesh_set_layout(mesh, layout);
	qg_mesh_gen_cube(mesh, 1.0f, 1.0f, 1.0f);
	qg_mesh_build(mesh);
	qg_dpct_set_loc_param(mesh, 0.0f, 0.0f, 0.0f);

	// 구면 메시
	QgMesh* sphere = qg_create_mesh("sphere");
	qg_mesh_set_layout(sphere, layout);
	qg_mesh_gen_sphere(sphere, 0.5f, 16, 16);
	qg_mesh_build(sphere);

	// 항성 조절용
	bool manual_constant = false;
	QnDateTime dt;
	float sdist = 5.0f;
	float stime = 0.0f;
	QMVEC spos = qm_vec_zero();

	// 루프
	while (qg_loop_dispatch())
	{
		float advance = qg_get_advance();

		// 항성 컨트롤
		if (qg_get_key_state(QIK_1))
		{
			manual_constant = true;
			dt.stamp = qn_ptc();
			stime = (float)(dt.hour * 3600 + dt.minute * 60 + dt.second);
		}
		if (qg_get_key_state(QIK_2))
			manual_constant = false;
		if (manual_constant)
		{
			if (qg_get_key_state(QIK_PGUP))
				stime += 86400.0f * advance * 0.5f;
			if (qg_get_key_state(QIK_PGDN))
				stime -= 86400.0f * advance * 0.5f;
			stime = qm_ccyf(stime, 0.0f, 86400.0f);
		}

		// 카메라 컨트롤
		static const QgCamCtrl cam_ctrl = { QIK_W, QIK_S, QIK_A, QIK_D, QIK_R, QIK_F, QIK_NONE, QIK_NONE, QIK_NONE, QIK_NONE, QIM_LEFT, };
		qg_camera_control(cam, &cam_ctrl, advance);
		qg_camera_update(cam);

		// 큐브 업데이트
		qg_dpct_update(mesh, advance);

		// 항성 업데이트
		if (manual_constant)
		{
			// 사용자가 업데이트
			float s, c, hs = stime / 86400.0f;
			qm_sincosf(hs * QM_TAU, &s, &c);
			spos = qm_vec(-s, c, 0.0f, 0.0f);
			qg_set_constant_dir(&spos);
			spos = qm_vec_mag(spos, -sdist);
		}
		else
		{
			// PTC 시간으로 업데이트
			spos = qg_get_constant_dir();
			spos = qm_vec_mag(spos, -sdist);
		}
		qg_dpct_set_loc(sphere, &spos);
		qg_dpct_update(sphere, advance);

		// 그리기
		if (qg_begin_render(true))
		{
			// 큐브와 구를 그린다
			qg_set_render_state(rs);
			qg_dpct_draw(mesh);
			qg_dpct_draw(sphere);

			// 안내 메시ㅣㅈ
			qg_draw_text_format(0, QG_FGS(0), "평균 FPS: %.2f", qg_get_afps());
			qg_draw_text_format(0, QG_FGS(1), "카메라 상태: P(%.2f, %.2f, %.2f) A(%.2f, %.2f, %.2f)",
				cam->param.eye.X, cam->param.eye.Y, cam->param.eye.Z,
				cam->param.angle.X, cam->param.angle.Y, cam->param.angle.Z);

			// 시간
			dt.stamp = qn_now();
			qg_draw_text_format(0, QG_FGS(4), "실제 시간: %04d-%02d-%02d %02d:%02d:%02d", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
			if (manual_constant)
			{
				int hh = (int)stime / 3600;
				int mm = (int)(stime - hh * 3600) / 60;
				int ss = (int)(stime - hh * 3600 - mm * 60);
				qg_draw_text_format(0, QG_FGS(5), "항성 시간: %02d:%02d:%02d", hh, mm, ss);
			}
			else
			{
				dt.stamp = qn_ptc();
				qg_draw_text_format(0, QG_FGS(5), "내부 시간: %04d-%02d-%02d %02d:%02d:%02d", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
			}

			// 항성의 위치
			QmFloat3A a3; qm_vec_to_float3a(spos, &a3);
			qg_draw_text_format(0, QG_FGS(6), "항성 위치: (%.2f, %.2f, %.2f)", a3.X, a3.Y, a3.Z);

			// 안내
			QmSize size = qg_get_size();
			qg_draw_text(0, size.Height - qg_get_def_font_size(), "카메라 이동: WASDRF / 카메라 회전: 왼쪽 버튼 / 1-항성 조정, 2-내부 시간 조정");
			qg_end_render(true);
		}
	}

	qn_unload(sphere);
	qn_unload(mesh);
	qn_unload(rs);
	qn_unload(cam);
	qg_close_rdh();

	return 0;
}
