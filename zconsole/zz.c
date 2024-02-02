// rdh 테스트
#include <qs.h>

#define RENDERER "es"
static const char* vs =
"attribute vec4 aposition;\n"
"attribute vec4 acolor;\n"
"attribute vec2 acoord;\n"
"varying vec4 vcolor;\n"
"varying vec2 vcoord;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aposition.xy, 0.0, 1.0);\n"
"   vcolor = acolor;\n"
"   vcoord = acoord;\n"
"}\n";
static const char* ps =
"uniform sampler2D Texture;\n"
"varying vec4 vcolor;\n"
"varying vec2 vcoord;\n"
"void main()\n"
"{\n"
"  vec4 t = texture2D(Texture, vcoord);\n"
"  gl_FragColor = t * vcolor;\n"
"}\n";
static QgLayoutInput layouts[] =
{
	{ QGLOS_1, QGLOU_POSITION, QGLOT_FLOAT4, false },
	{ QGLOS_1, QGLOU_COLOR1, QGLOT_FLOAT4, false },
};
struct vertextype
{
	QmFloat4	pos;
	QmFloat4	color;
	QmFloat2	coord;
};
static struct vertextype quadverts[] =
{
	{ {-0.5f, -0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
	{ {0.5f, -0.5f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
	{ {-0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	{ {0.5f, 0.5f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
};

int main(void)
{
	qn_runtime();

	int flags = QGFLAG_RESIZE | QGFLAG_VSYNC | QGFLAG_MSAA;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh(RENDERER, "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	QgPropRender prop_render = QG_DEFAULT_PROP_RENDER;
	QgPropShader prop_shader = { { layouts, QN_COUNTOF(layouts) }, { vs, 0 }, { ps, 0 } };
	QgRenderState* render = qg_create_render_state("named", &prop_render, &prop_shader);
	QgBuffer* qbuffer = qg_create_buffer(QGBUFFER_VERTEX, 1024, sizeof(struct vertextype), NULL);
	qg_buffer_data(qbuffer, sizeof(struct vertextype) * 4, quadverts);

	QgImage* img_autumn = qg_load_image(0, "../res/image/ff14_autumn.png");
	QgImage* img_puru = qg_load_image(0, "../res/image/ff14_puru.jpg");
	QgTexture* tex_autumn = qg_load_texture(0, "../res/image/ff14_autumn.png", QGTEXF_LINEAR);
	QgTexture* tex_white = qg_create_texture(NULL, qg_new_image_filled(4, 4, qm_vec(0.0f, 1.0f, 0.0f, 1.0f)), QGTEXF_DISCARD_IMAGE);

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
		//qg_set_background(&c);

		if (qg_begin_render(true))
		{
			if (render)
			{
				qg_set_render_state(render);
				qg_set_texture(0, tex_autumn);
				qg_set_vertex(QGLOS_1, qbuffer);
				qg_draw(QGTPG_TRI_STRIP, 4);
			}

			QmRect rt = qm_rect(10, 10, 10+tex_autumn->width, 10+tex_autumn->height);
			qg_draw_sprite(&rt, &c, tex_autumn, NULL);

			qg_end_render(true);
		}
	}

	qn_unload(tex_white);
	qn_unload(tex_autumn);
	qn_unload(img_autumn);
	qn_unload(img_puru);

	qn_unload(qbuffer);
	qn_unload(render);
	qg_close_rdh();

	return 0;
}
