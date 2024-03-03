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

	const QMVEC bgc = qm_vec(0.1f, 0.3f, 0.1f, 1.0f);
	qg_set_background(&bgc);
	qg_fuse(0, NULL, true, true);

	QgCamera* cam = qg_create_camera();

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
	qg_dpct_set_loc_param(mesh, 0.0f, 0.0f, 8.0f);

	static VertexType vertices[] = 
	{
		{ { 0.0f, 1.0f, 5.0f }, 0xFFFF0000 },
		{ { 1.0f, -1.0f, 5.0f}, 0xFF00FF00 },
		{ { -1.0f, -1.0f, 5.0f}, 0xFF0000FF },
	};
	QgBuffer* vbuffer = qg_create_buffer(QGBUFFER_VERTEX, QN_COUNTOF(vertices), sizeof(VertexType), vertices);

	QmVec4 rot; rot.s = qm_vec_zero();

	while (qg_loop_dispatch())
	{
		float advance = qg_get_advance();

		if (qg_get_key_state(QIK_LEFT))
			rot.Y -= 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_RIGHT))
			rot.Y += 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_UP))
			rot.X -= 0.1f * QM_RPI;
		if (qg_get_key_state(QIK_DOWN))
			rot.X += 0.1f * QM_RPI;
		QMVEC q = qm_quat_rot_vec(rot.s);

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
