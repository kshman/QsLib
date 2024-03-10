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

	QgCamera* cam = qg_create_camera();
	qg_camera_set_position_param(cam, 0.0f, 0.0f, -10.0f);

	static QgLayoutInput li_3d[] =
	{
		{ QGLOS_1, QGLOU_POSITION, QGLOT_FLOAT3, false },
		//{ QGLOS_1, QGLOU_NORMAL1, QGLOT_FLOAT3, false},
		//{ QGLOS_1, QGLOU_COORD1, QGLOT_FLOAT2, false },
		{ QGLOS_1, QGLOU_COLOR1, QGLOT_BYTE4, true },
	};
	static QgLayoutData ld_3d = { QN_COUNTOF(li_3d), li_3d };
	static QgPropRender pr_3d = QG_DEFAULT_PROP_RENDER;
	QgRenderState* rs = qg_create_render_state_vsps(NULL, &pr_3d, &ld_3d, 0, "/shader/simple_vs.glsl", "/shader/simple_ps.glsl");

	QgMesh* mesh = qg_create_mesh("cube");
	qg_mesh_set_layout(mesh, &ld_3d);
	qg_mesh_gen_cube(mesh, 1.0f, 1.0f, 1.0f);
	qg_mesh_build(mesh);
	qg_dpct_set_loc_param(mesh, 0.0f, 0.0f, 0.0f);

	QgMesh* sphere = qg_create_mesh("sphere");
	qg_mesh_set_layout(sphere, &ld_3d);
	qg_mesh_gen_sphere(sphere, 1.0f, 16, 16);
	qg_mesh_build(sphere);

	static VertexType vertices[] =
	{
		{ { 0.0f, 1.0f, 5.0f }, 0xFFFF0000 },
		{ { 1.0f, -1.0f, 5.0f}, 0xFF00FF00 },
		{ { -1.0f, -1.0f, 5.0f}, 0xFF0000FF },
	};
	QgBuffer* vbuffer = qg_create_buffer(QGBUFFER_VERTEX, QN_COUNTOF(vertices), sizeof(VertexType), vertices);

	QmVec4 rot = { .s = qm_vec_zero() };
	QmVec4 pos = { .s = qm_vec_zero() };

	while (qg_loop_dispatch())
	{
		float advance = qg_get_advance();

		if (qg_get_key_state(QIK_LEFT))
			pos.X -= 0.1f;
		if (qg_get_key_state(QIK_RIGHT))
			pos.X += 0.1f;
		if (qg_get_key_state(QIK_UP))
			pos.Y += 0.1f;
		if (qg_get_key_state(QIK_DOWN))
			pos.Y -= 0.1f;
		if (qg_get_key_state(QIK_SUB))
			rot.Y -= 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_ADD))
			rot.Y += 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_PGUP))
			rot.X -= 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_PGDN))
			rot.X += 0.1f * QM_RPI;
		QMVEC q = qm_quat_rot_vec(rot.s);

		static const QgCamCtrl cam_ctrl =
		{
			QIK_W, QIK_S, QIK_A, QIK_D, QIK_R, QIK_F,
			QIK_NONE, QIK_NONE, QIK_NONE, QIK_NONE,
			QIM_LEFT,
		};
		qg_camera_control(cam, &cam_ctrl, advance);
		qg_camera_update(cam);
		qg_dpct_set_loc(mesh, &pos.s);
		qg_dpct_set_rot(mesh, &q);
		qg_dpct_update(mesh, advance);

		if (qg_begin_render(true))
		{
			qg_set_render_state(rs);
			qg_set_vertex(QGLOS_1, vbuffer);
			qg_draw(QGTPG_TRI, 3);

			qg_dpct_draw(mesh);

			qg_draw_text_format(0, 0, "평균 FPS: %.2f", qg_get_afps());
			qg_draw_text_format(0, 32, "카메라 상태: P(%.2f, %.2f, %.2f) A(%.2f, %.2f, %.2f)",
				cam->param.eye.X, cam->param.eye.Y, cam->param.eye.Z,
				cam->param.angle.X, cam->param.angle.Y, cam->param.angle.Z);
			qg_draw_text(0, 64, "카메라 이동: WASDRF / 카메라 회전: 왼쪽 버튼");

			QnDateTime dt;
			dt.stamp = qn_utc();
			qg_draw_text_format(0, 120, "%04d-%02d-%02d %02d:%02d:%02d", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
			dt.stamp = qn_ptc();
			qg_draw_text_format(0, 152, "%04d-%02d-%02d %02d:%02d:%02d", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);

			qg_end_render(true);
		}
	}

	qn_unload(vbuffer);
	qn_unload(sphere);
	qn_unload(mesh);
	qn_unload(rs);
	qn_unload(cam);
	qg_close_rdh();

	return 0;
}
