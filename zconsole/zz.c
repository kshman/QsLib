// rdh 테스트
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
static QgLayoutInput layouts[] =
{
	{ QGLOS_1, QGLOU_POSITION, QGLOT_FLOAT2 },
	{ QGLOS_2, QGLOU_COLOR1, QGLOT_FLOAT4 },
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

int main(void)
{
	qn_runtime();

	int flags = QGFLAG_RESIZE | QGFLAG_VSYNC;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh(NULL, "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	QgShaderCode svc = { 0, vs };
	QgShaderCode spc = { 0, ps };
	QgPropRender pr = qg_default_prop_render(layouts, QN_COUNTOF(layouts));
	QgShader* shader = qg_rdh_create_shader_buffer(NULL, &svc, &spc, QGSCF_TEXT);
	QgRender* render = qg_rdh_create_render(&pr, shader);
	QgBuffer* vertexbuf = qg_rdh_create_buffer(QGBUFFER_VERTEX, QN_COUNTOF(vertices), sizeof(float), vertices);
	QgBuffer* colorbuf = qg_rdh_create_buffer(QGBUFFER_VERTEX, QN_COUNTOF(colors), sizeof(float), colors);

	while (qg_loop())
	{
		QgEvent ev;
		while (qg_poll(&ev))
		{
			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
				qg_exit_loop();
		}

		if (qg_rdh_begin(true))
		{
			qg_rdh_set_render(render);
			qg_rdh_set_vertex(QGLOS_1, vertexbuf);
			qg_rdh_set_vertex(QGLOS_2, colorbuf);
			qg_rdh_draw(QGTPG_TRI, 3);

			qg_rdh_end(true);
		}
	}

	qs_unload(vertexbuf);
	qs_unload(colorbuf);
	qs_unload(render);
	qs_unload(shader);
	qg_close_rdh();

	return 0;
}
