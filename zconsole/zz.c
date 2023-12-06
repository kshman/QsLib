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
static QgPropLayout layout[] =
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
	qn_outputf("정수: %d, 실수: %f, 문자: %s, 유니코드: %ls", 123, 456.789, "문자", L"유니코드");

	QgRdh* rdh = qg_rdh_new(NULL, "RDH", 800, 600, QGFLAG_IDLE | QGFLAG_RESIZABLE | QGFLAG_VSYNC);
	qn_retval_if_fail(rdh, -1);

	QgVlo* vlo = qg_rdh_create_layout(rdh, QN_COUNTOF(layout), layout);
	QgBuf* vertexbuf = qg_rdh_create_buffer(rdh, QGBUF_VERTEX, QN_COUNTOF(vertices), sizeof(float), vertices);
	QgBuf* colorbuf = qg_rdh_create_buffer(rdh, QGBUF_VERTEX, QN_COUNTOF(colors), sizeof(float), colors);
	QgShd* shd = qg_rdh_create_shader(rdh, NULL);
	qg_shd_bind(shd, QGSHT_VS, vs, 0, 0);
	qg_shd_bind(shd, QGSHT_PS, ps, 0, 0);
	qg_shd_link(shd);

	while (qg_rdh_loop(rdh))
	{
		QgEvent ev;
		while (qg_rdh_poll(rdh, &ev))
		{
			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
				qg_rdh_exit_loop(rdh);
		}

		if (qg_rdh_begin(rdh, true))
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
