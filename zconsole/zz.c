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
//"precision mediump float;\n"
"varying vec4 vcolor;\n"
"void main()\n"
"{\n"
"  gl_FragColor = vcolor;\n"
"}\n";
static QgLayoutInput layouts[] =
{
	{ QGLOS_1, QGLOU_POSITION, QGLOT_FLOAT2, true },
	{ QGLOS_1, QGLOU_COLOR1, QGLOT_FLOAT4, false },
};
struct vertextype
{
	QmFloat2	pos;
	QmFloat4	color;
};
static struct vertextype vertices[] =
{
	{ {0.0f, 0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}},
	{ {0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}},
	{ {-0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},
};

int main(void)
{
	qn_runtime();

	int flags = QGFLAG_RESIZE | QGFLAG_VSYNC | QGFLAG_MSAA;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh("es", "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	QgPropRender prop_render = QG_DEFAULT_PROP_RENDER;
	QgPropShader prop_shader = { { layouts, QN_COUNTOF(layouts) }, { vs, 0 }, { ps, 0 } };
	QgRenderState* render = qg_create_render_state("named", &prop_render, &prop_shader);
	QgBuffer* buffer = qg_create_buffer(QGBUFFER_VERTEX, QN_COUNTOF(vertices), sizeof(struct vertextype), vertices);

	QgImage* img_autumn = qg_load_image(0, "../res/image/ff14_autumn.png");
	QgImage* img_puru = qg_load_image(0, "../res/image/ff14_puru.jpg");

	float f = 0.0f;
	while (qg_loop())
	{
		QgEvent ev;
		while (qg_poll(&ev))
		{
			if (ev.ev == QGEV_KEYDOWN && ev.key.key == QIK_ESC)
				qg_exit_loop();
		}

		f += qg_get_advance() * 0.5f;
		if (f > 1.0f)
			f = 0.0f;
		QmVec c = { f, f, f, 1.0f };
		qg_set_background(&c);

		if (qg_begin_render(true))
		{
			if (render)
			{
				qg_set_render_state(render);
				qg_set_vertex(QGLOS_1, buffer);
				qg_draw(QGTPG_TRI, 3);
			}

			qg_end_render(true);
		}
	}

	qn_unload(img_autumn);
	qn_unload(img_puru);

	qn_unload(buffer);
	qn_unload(render);
	qg_close_rdh();

	return 0;
}
