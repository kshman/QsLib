// 다중 버퍼와 그에 따른 레이아웃
// 정점(스테이지1)과 색깔(스테이지) 버퍼가 두개
#include <qs.h>

static const char* vs =
"attribute vec4 aposition;\n"
"attribute vec4 acolor;\n"
"varying vec4 vcolor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aposition.xyz, 1.0);\n"
"   vcolor = acolor;\n"
"}\n";
static const char* ps =
"precision mediump float;\n"
"varying vec4 vcolor;\n"
"void main()\n"
"{\n"
"  //gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 );\n"
"  gl_FragColor = vcolor;\n"
"}\n";
static qgPropLayout layout[] =
{
	{QGLOS_1, 0, QGLOU_POSITION, QGLOT_FLOAT2},
	{QGLOS_2, 0, QGLOU_COLOR, QGLOT_FLOAT4},
};
static float vertices[] =
{
	0.0f, 0.5f,
	0.5f, -0.5f,
	-0.5f, -0.5f,
};
static float colors[] =
{
	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f
};

int main()
{
	qn_runtime(NULL);

	qgRdh* rdh = qg_rdh_new(NULL, "QG TEST", 800, 600, QGFLAG_IDLE | QGFLAG_RESIZABLE);
	if (!rdh)
		return 1;

	qgVlo* vlo = qg_rdh_create_layout(rdh, QN_COUNTOF(layout), layout);
	qgBuf* vertexbuf = qg_rdh_create_buffer(rdh, QGBUF_VERTEX, QN_COUNTOF(vertices), sizeof(float), vertices);
	qgBuf* colorbuf = qg_rdh_create_buffer(rdh, QGBUF_VERTEX, QN_COUNTOF(colors), sizeof(float), colors);
	qgShd* shd = qg_rdh_create_shader(rdh, "zz shader");
	qg_shd_bind(shd, QGSHT_VS, vs, 0, 0);
	qg_shd_bind(shd, QGSHT_PS, ps, 0, 0);
	qg_shd_link(shd);

	while (qg_rdh_loop(rdh))
	{
		qgEvent ev;
		while (qg_rdh_poll(rdh, &ev))
		{

		}

		if (qg_rdh_begin(rdh))
		{
			qg_rdh_set_shader(rdh, shd, vlo);
			qg_rdh_set_vertex(rdh, QGLOS_1, vertexbuf);
			qg_rdh_set_vertex(rdh, QGLOS_2, colorbuf);
			qg_rdh_draw(rdh, QGTPG_TRI, 3);

			qg_rdh_end(rdh);
			qg_rdh_flush(rdh);
		}
	}

	qm_unload(shd);
	qm_unload(colorbuf);
	qm_unload(vertexbuf);
	qm_unload(vlo);

	qm_unload(rdh);

	return 0;
}
