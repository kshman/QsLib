// rdh 테스트
#include <qs.h>

#define QM_1_120 0.00833333f
#define QM_1_5040 0.000198413f
#define QM_1_362880 0.00000275573f
#define QM_0_5 0.5f
#define QM_1_24 0.04166667f
#define QM_1_720 0.000138889f
#define QM_1_40320 0.00000248016f

INLINE void g_sincosf(float x, float* sin_val, float* cos_val) {
	float x2 = x * x;

	*sin_val = x * (1 - x2 * (QM_1_120 - x2 * (QM_1_5040 - x2 * QM_1_362880)));
	*cos_val = 1 - x2 * (QM_0_5 - x2 * (QM_1_24 - x2 * (QM_1_720 - x2 * QM_1_40320)));
}

INLINE void x_sincosf(float v, float* s, float* c)
{
	float q = QM_DIV_PI2 * v;
	if (v >= 0.0f)
		q = (float)((int)(q + 0.5f));
	else
		q = (float)((int)(q - 0.5f));
	float y = v - QM_PI2 * q;
	float u;
	if (y > QM_PI_H)
	{
		y = QM_PI - y;
		u = -1.0f;
	}
	else if (y < -QM_PI_H)
	{
		y = -QM_PI - y;
		u = -1.0f;
	}
	else
		u = 1.0f;
	float y2 = y * y;
	*s = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;
	*c = (((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f) * u;
}

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
"  gl_FragColor = vcolor;\n"
"}\n";
static QgLayoutInput layouts[] =
{
	{ QGLOS_1, QGLOU_POSITION, QGLOT_FLOAT2, true },
	{ QGLOS_1, QGLOU_COLOR1, QGLOT_FLOAT4, false },
};
struct vertextype
{
#if false
	// 16바이트 정렬 때문에 구조체를 사용하면 안된다.
	QmVec2		pos;
	QmColor		color;
#else
	float		pos[2];
	float		color[4];
#endif
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

	for (float f = -180.0f; f <= 180.0f; f += 20.0f)
	{
		float s, c;
		float r = QM_TORADIAN(f);
		qm_sincosf(r, &s, &c);
		qn_outputf("qm: %f %f %f", f, s, c);
		x_sincosf(r, &s, &c);
		qn_outputf(" x: %f %f %f", f, s, c);
		//g_sincosf(r, &s, &c);
		//qn_outputf(" g: %f %f %f", f, s, c);
	}

	int flags = QGFLAG_RESIZE | QGFLAG_VSYNC | QGFLAG_MSAA;
	int features = QGFEATURE_NONE;
	if (qg_open_rdh(NULL, "RDH", 0, 0, 0, flags, features) == false)
		return -1;

	QgPropRender prop_render = QG_DEFAULT_PROP_RENDER;
	QgPropShader prop_shader = { { layouts, QN_COUNTOF(layouts) }, { vs, 0 }, { ps, 0 } };
	QgRender* render = qg_rdh_create_render("named", &prop_render, &prop_shader);
	QgBuffer* buffer = qg_rdh_create_buffer(QGBUFFER_VERTEX, QN_COUNTOF(vertices), sizeof(struct vertextype), vertices);

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
			qg_rdh_set_vertex(QGLOS_1, buffer);
			qg_rdh_draw(QGTPG_TRI, 3);

			qg_rdh_end(true);
		}
	}

	qs_unload(buffer);
	qs_unload(render);
	qg_close_rdh();

	return 0;
}
