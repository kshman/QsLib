#include <qs.h>

static const char* vs =
"attribute vec4 aposition;    \n"
"attribute vec4 anormal;    \n"
"attribute vec3 acolor;    \n"
"attribute vec2 acoord;    \n"
"varying vec3 vcolor;\n"
"varying vec4 vnormal;\n"
"void main()                  \n"
"{                            \n"
"   gl_Position = vec4(aposition.xyz, 1.0);  \n"
"   vcolor=vec3(acolor.xy, acoord.x);\n"
"   vnormal=anormal;\n"
"}                            \n";
static const char* ps =
"precision mediump float;\n"
"varying vec3 vcolor;\n"
"varying vec4 vnormal;\n"
"void main()                                  \n"
"{                                            \n"
"  //gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 );\n"
"  gl_FragColor = vec4 (vnormal.x, vcolor.y, 1.0, 1.0 );\n"
"}                                            \n";
static qgVarLayout layout[1] =
{
	{QGLOU_POSITION, 0, QGLOT_FLOAT2, 0},
};
static float vertices[] =
{
	0.0f, 0.5f,
	-0.5f, -0.5f,
	0.5f, -0.5f,
};

int main()
{
	qn_runtime(NULL);

	qgRdh* rdh = qg_rdh_new(NULL, "QG TEST", 800, 600, QGFLAG_IDLE | QGFLAG_RESIZABLE);
	if (!rdh)
		return 1;

	qgVlo* vlo = qg_rdh_create_layout(rdh, 1, layout);
	qgBuf* buf = qg_rdh_create_buffer(rdh, QGBUF_VERTEX, QN_COUNTOF(vertices), sizeof(float), vertices);
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
			qg_rdh_set_vertex(rdh, 0, buf);
			qg_rdh_draw(rdh, QGTPG_TRI, 3);

			qg_rdh_end(rdh);
			qg_rdh_flush(rdh);
		}
	}

	qm_unload(shd);
	qm_unload(buf);
	qm_unload(vlo);

	qm_unload(rdh);

	return 0;
}
