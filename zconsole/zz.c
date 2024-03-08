// 스프라이트 테스트
#include <qs.h>

typedef struct VERTEXTYPE
{
	QmFloat3 position;
	QmKolor color;
} VertexType;

void test(void)
{
	float l[] = { 23.5791397, 0.00000000, 0.00000000, 0.00000000 };
	float r[] = { 0.00000000, 0.00000000, 0.999999940, 0.00000000 };
	uint* ul = (uint*)l;
	uint* ur = (uint*)r;
	uint c[] = { 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0 };
	QmVecU v;
	v.u[0] = (ul[0] & ~c[0]) | (ur[0] & c[0]);
	v.u[1] = (ul[1] & ~c[1]) | (ur[1] & c[1]);
	v.u[2] = (ul[2] & ~c[2]) | (ur[2] & c[2]);
	v.u[3] = (ul[3] & ~c[3]) | (ur[3] & c[3]);
	QmVecF* vf = (QmVecF*)&v;
	qn_outputf("v: %f, %f, %f, %f", vf->X, vf->Y, vf->Z, vf->W);
}

int main(void)
{
	qn_runtime(NULL);
	test();

	const int flags = QGFLAG_RESIZE | QGFLAG_MSAA /*| QGFLAG_VSYNC*/;
	const int features = QGFEATURE_NONE;
	if (qg_open_rdh("gl", "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	const QMVEC bgc = qm_vec(0.1f, 0.3f, 0.1f, 1.0f);
	qg_set_background(&bgc);
	qg_fuse(0, NULL, true, true);
	qg_load_def_font(0, "/font/kopubs_l.hxn");

	QgCamera* cam = qg_create_camera();
	qg_camera_set_move_speed(cam, 5.0f, 5.0f, 5.0f);
	const QMVEC pos = qm_vec(0.0f, 0.0f, -10.0f, 0.0f);
	qg_camera_set_position(cam, &pos);

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

	static VertexType vertices[] =
	{
		{ { 0.0f, 1.0f, 5.0f }, 0xFFFF0000 },
		{ { 1.0f, -1.0f, 5.0f}, 0xFF00FF00 },
		{ { -1.0f, -1.0f, 5.0f}, 0xFF0000FF },
	};
	QgBuffer* vbuffer = qg_create_buffer(QGBUFFER_VERTEX, QN_COUNTOF(vertices), sizeof(VertexType), vertices);

	QmVec4 rot = { .s = qm_vec_zero() };

	while (qg_loop_dispatch())
	{
		float advance = qg_get_advance();

		if (qg_get_key_state(QIK_SUB))
			rot.Y -= 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_ADD))
			rot.Y += 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_PGUP))
			rot.X -= 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_PGDN))
			rot.X += 0.1f * QM_RPI;
		QMVEC q = qm_quat_rot_vec(rot.s);

		qg_camera_control(cam, NULL, advance);
		qg_camera_update(cam);
		qg_dpct_set_rot(mesh, &q);
		qg_dpct_update(mesh, advance);

		if (qg_begin_render(true))
		{
			qg_set_render_state(rs);
			qg_set_vertex(QGLOS_1, vbuffer);
			qg_draw(QGTPG_TRI, 3);

			qg_dpct_draw(mesh);

			qg_draw_text_format(0, 0, "평균 FPS: %.2f", qg_get_afps());
			qg_draw_text_format(0, 25, "카메라 상태: P(%.2f, %.2f, %.2f) A(%.2f, %.2f, %.2f)",
				cam->param.eye.X, cam->param.eye.Y, cam->param.eye.Z,
				cam->param.angle.X, cam->param.angle.Y, cam->param.angle.Z);
			qg_end_render(true);
		}
	}

	qn_unload(vbuffer);
	qn_unload(mesh);
	qn_unload(rs);
	qn_unload(cam);
	qg_close_rdh();

	return 0;
}
