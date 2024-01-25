//
// qgrdh_es.c - OPENGL ES 렌더 디바이스
// 2024-01-10 by kim
//

#include "pch.h"
#ifdef USE_ES
#ifndef _QN_EMSCRIPTEN_
#define GLAD_EGL_IMPLEMENTATION		1
#define GLAD_GLES2_IMPLEMENTATION	1
#else
#include <emscripten/html5_webgl.h>
#endif // _QN_EMSCRIPTEN_
#include "qgrdh_es.h"
#include <qs_supp.h>
#include <limits.h>

#define ES_MAX_LAYOUT_COUNT		16
#define ES_VERSION_2			200
#define ES_VERSION_3			300


//////////////////////////////////////////////////////////////////////////
// OPENGL ES 렌더 디바이스

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"ESRDH"

static void es_dispose(QsGam* g);
static void es_layout(void);
static void es_reset(void);
static void es_clear(QgClear flags);
static bool es_begin(bool clear);
static void es_end(void);
static void es_flush(void);
static bool es_set_index(QgBuffer* buffer);
static bool es_set_vertex(QgLayoutStage stage, QgBuffer* buffer);
static bool es_set_render(QgRender* render);
static bool es_draw(QgTopology tpg, int vertices);
static bool es_draw_indexed(QgTopology tpg, int indices);
static QgBuffer* es_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data);
static QgRender* es_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader);

qs_name_vt(RDHBASE) vt_es_rdh =
{
	.base.name = "ESRDH",
	.base.dispose = es_dispose,

	.layout = es_layout,
	.reset = es_reset,
	.clear = es_clear,

	.begin = es_begin,
	.end = es_end,
	.flush = es_flush,

	.create_buffer = es_create_buffer,
	.create_render = es_create_render,

	.set_index = es_set_index,
	.set_vertex = es_set_vertex,
	.set_render = es_set_render,

	.draw = es_draw,
	.draw_indexed = es_draw_indexed,
};

#ifndef ES_STATIC_LINK
//
static QnModule* es_egl_module = NULL;

//
static GLADapiproc es_load_egl(const char* name)
{
	return (GLADapiproc)qn_mod_func(es_egl_module, name);
}

//
static bool es_init_egl(void)
{
	static int version = 0;
	if (version != 0)
		return true;
	static const char* egllibs[] =
	{
#if defined _QN_WINDOWS_
		"libEGL",
#elif defined _QN_FREEBSD_ || defined _QN_LINUX_
		"libEGL.so",
#else
#error unknown EGL platform!
#endif
		NULL,
	};
	for (size_t i = 0; egllibs[i]; i++)
	{
		es_egl_module = qn_mod_load(egllibs[i], 1);
		if (es_egl_module != NULL)
			break;
	}
	VAR_CHK_IF_COND(es_egl_module == NULL, "cannot load egl library", false);

	version = gladLoadEGL(EGL_NO_DISPLAY, es_load_egl);
	if (version == 0)
	{
		qn_mod_unload(es_egl_module);
		return false;
	}
	return true;
}

//
static void es_reload_egl(EGLDisplay display)
{
	gladLoadEGL(display, es_load_egl);
}

//
static QnModule* es_api_module = NULL;

//
static GLADapiproc es_load_api(const char* name)
{
	return (GLADapiproc)qn_mod_func(es_api_module, name);
}

//
static bool es_init_api(int major)
{
	static int version = 0;
	if (version != 0)
		return true;
	static const char* es1libs[] =
	{
#if defined _QN_WINDOWS_
		"libGLESv1_CM",
#elif defined _QN_FREEBSD_ || defined _QN_LINUX_
		"libGLESv1_CM.so",
		"libGLES_CM.so",
#else
#error unknown EGL platform!
#endif
		NULL,
	};
	static const char* es2libs[] =
	{
#if defined _QN_WINDOWS_
		"libGLESv2",
#elif defined _QN_FREEBSD_ || defined _QN_LINUX_
		"libGLESv2.so",
#else
#error unknown EGL platform!
#endif
		NULL,
	};
	const char** apilibs = major == 1 ? es1libs : es2libs;
	for (size_t i = 0; apilibs[i]; i++)
	{
		es_api_module = qn_mod_load(apilibs[i], 1);
		if (es_api_module != NULL)
			break;
	}
	VAR_CHK_IF_COND(es_api_module == NULL, "cannot load es client library", false);

	version = gladLoadGLES2(es_load_api);
	if (version == 0)
	{
		qn_mod_unload(es_api_module);
		return false;
	}
	return true;
}
#endif // ES_STATIC_LINK

//
static const char* es_egl_error_string(EGLint error)
{
#define ERROR_CASE(x)		case x: return #x
	static char unknown[32];
	switch (error)
	{
		ERROR_CASE(EGL_SUCCESS);
		ERROR_CASE(EGL_NOT_INITIALIZED);
		ERROR_CASE(EGL_BAD_ACCESS);
		ERROR_CASE(EGL_BAD_ALLOC);
		ERROR_CASE(EGL_BAD_ATTRIBUTE);
		ERROR_CASE(EGL_BAD_CONTEXT);
		ERROR_CASE(EGL_BAD_CONFIG);
		ERROR_CASE(EGL_BAD_CURRENT_SURFACE);
		ERROR_CASE(EGL_BAD_DISPLAY);
		ERROR_CASE(EGL_BAD_SURFACE);
		ERROR_CASE(EGL_BAD_MATCH);
		ERROR_CASE(EGL_BAD_PARAMETER);
		ERROR_CASE(EGL_BAD_NATIVE_PIXMAP);
		ERROR_CASE(EGL_BAD_NATIVE_WINDOW);
		ERROR_CASE(EGL_CONTEXT_LOST);
		default:
			return qg_unknown_str(error);
	}
#undef ERROR_CASE
}

//
static EGLint es_get_config_attr(EGLDisplay display, EGLConfig config, EGLenum name)
{
	EGLint v;
	eglGetConfigAttrib(display, config, name, &v);
	return v;
}

//
static void es_get_config(EGLDisplay display, EGLConfig ec, EsConfig* config)
{
	config->handle = ec;
	config->red = es_get_config_attr(display, ec, EGL_RED_SIZE);
	config->green = es_get_config_attr(display, ec, EGL_GREEN_SIZE);
	config->blue = es_get_config_attr(display, ec, EGL_BLUE_SIZE);
	config->alpha = es_get_config_attr(display, ec, EGL_ALPHA_SIZE);
	config->depth = es_get_config_attr(display, ec, EGL_DEPTH_SIZE);
	config->stencil = es_get_config_attr(display, ec, EGL_STENCIL_SIZE);
	config->samples = es_get_config_attr(display, ec, EGL_SAMPLES);
	config->version = (es_get_config_attr(display, ec, EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES3_BIT) != 0 ? 3 : 2;
}

//
static const EsConfig* es_detect_config(const EsConfig* wanted, const EsConfig* configs, EGLint count)
{
	uint least_missing = UINT_MAX;
	uint least_color = UINT_MAX;
	uint least_extra = UINT_MAX;
	const EsConfig* found = NULL;
	for (EGLint i = 0; i < count; i++)
	{
		const EsConfig* c = &configs[i];

		uint missing = 0;
		if (wanted->alpha > 0 && c->alpha == 0) missing++;
		if (wanted->depth > 0 && c->depth == 0) missing++;
		if (wanted->stencil > 0 && c->stencil == 0) missing++;
		if (wanted->samples > 0 && c->samples == 0) missing++;

		uint color = 0;
		if (wanted->red > 0)
			color += (wanted->red - c->red) * (wanted->red - c->red);
		if (wanted->blue > 0)
			color += (wanted->blue - c->blue) * (wanted->blue - c->blue);
		if (wanted->green > 0)
			color += (wanted->green - c->green) * (wanted->green - c->green);

		uint extra = 0;
		if (wanted->alpha > 0)
			extra += (wanted->alpha - c->alpha) * (wanted->alpha - c->alpha);
		if (wanted->depth > 0)
			extra += (wanted->depth - c->depth) * (wanted->depth - c->depth);
		if (wanted->stencil > 0)
			extra += (wanted->stencil - c->stencil) * (wanted->stencil - c->stencil);
		if (wanted->samples > 0)
			extra += (wanted->samples - c->samples) * (wanted->samples - c->samples);
		if (wanted->version > 0)
			extra += (wanted->version - c->version) * (wanted->version - c->version);

		if (missing < least_missing)
			found = c;
		else if (missing == least_missing)
		{
			if (color < least_color || (color == least_color && extra < least_extra))
				found = c;
		}
		if (found == c)
		{
			least_missing = missing;
			least_color = color;
			least_extra = extra;
		}
	}
	return found;
}

//
static bool es_choose_config(EsRdh* self, const EsConfig* config, bool msaa)
{
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[i++] = e; attrs[i++] = v; }QN_STMT_END
	EGLint attrs[32], i = 0;
	ATTR_ADD(EGL_RED_SIZE, config->red);
	ATTR_ADD(EGL_GREEN_SIZE, config->green);
	ATTR_ADD(EGL_BLUE_SIZE, config->blue);
	if (config->alpha > 0)
		ATTR_ADD(EGL_ALPHA_SIZE, config->alpha);
	if (config->depth > 0)
		ATTR_ADD(EGL_DEPTH_SIZE, config->depth);
	if (config->stencil > 0)
		ATTR_ADD(EGL_STENCIL_SIZE, config->stencil);
	if (msaa && config->samples > 0)
		ATTR_ADD(EGL_SAMPLES, config->samples);
	ATTR_ADD(EGL_RENDERABLE_TYPE, config->version == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT);
	ATTR_ADD(EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER);
	ATTR_ADD(EGL_SURFACE_TYPE, EGL_WINDOW_BIT);
	ATTR_ADD(EGL_NONE, EGL_NONE);
	qn_verify(i < QN_COUNTOF(attrs));
#undef ATTR_ADD

	EGLint config_count = 0;
	EGLConfig* configs = qn_alloc(64, EGLConfig);
	if (eglChooseConfig(self->display, attrs, configs, 64, &config_count) == EGL_FALSE || config_count == 0)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to choose config: %s", es_egl_error_string(eglGetError()));
		qn_free(configs);
		return false;
	}

	bool ret;
	if (config_count == 1)
	{
		ret = true;
		es_get_config(self->display, configs[0], &self->config);
	}
	else
	{
		EsConfig* esconfigs = qn_alloc(config_count, EsConfig);
		for (i = 0; i < config_count; i++)
			es_get_config(self->display, configs[i], &esconfigs[i]);

		const EsConfig* found = es_detect_config(config, esconfigs, config_count);
		if (found == NULL)
			ret = false;
		else
		{
			self->config = *found;
			ret = true;
		}
		qn_free(esconfigs);
	}

	qn_free(configs);
	return ret;
}

//
RdhBase* es_allocator(QgFlag flags, QgFeature features)
{
	if (QN_TMASK(features, QGRENDERER_ES) == false)
		return NULL;

#ifndef ES_STATIC_LINK
	if (es_init_egl() == false)
		return NULL;
#endif // ES_STATIC_LINK

	//----- 설정 초기화. 프로퍼티에 있으면 가져온다
	bool msaa = QN_TMASK(flags, QGFLAG_MSAA);
	EsConfig config =
	{
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = qn_get_prop_int(QG_PROP_DEPTH_SIZE, 24, 4, 32),
		.stencil = qn_get_prop_int(QG_PROP_STENCIL_SIZE, 8, 4, 16),
		.samples = msaa ? qn_get_prop_int(QG_PROP_MSAA, 4, 0, 8) : 0,
		.version = 3,
	};
	const char* size_prop = qn_get_prop(QG_PROP_RGBA_SIZE);
	if (size_prop != NULL && strlen(size_prop) >= 4)
	{
		config.red = size_prop[0] - '0';
		config.green = size_prop[1] - '0';
		config.blue = size_prop[2] - '0';
		config.alpha = size_prop[3] - '0';
	}

	//----- 여기에 RDH가!
	EsRdh* self = qn_alloc_zero_1(EsRdh);
	self->native_window = (NativeWindowType)stub_system_get_window();
	self->native_display = (NativeDisplayType)stub_system_get_display();

#ifdef _QN_EMSCRIPTEN_
	// EMSCRIPTEN ES3 초기화 WEBGL 이용하는 듯
	EmscriptenWebGLContextAttributes emattrs =
	{
		.alpha = config.alpha > 0 ? EM_TRUE : EM_FALSE,
		.depth = config.depth > 0 ? EM_TRUE : EM_FALSE,
		.stencil = config.stencil > 0 ? EM_TRUE : EM_FALSE,
		.antialias = msaa ? EM_TRUE : EM_FALSE,
		//.premultipliedAlpha = 0,
		//.preserveDrawingBuffer = 0,
		.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT,
		//.failIfMajorPerformanceCaveat = 0,
		.majorVersion = 3,
		.minorVersion = 2,
		//.enableExtensionsByDefault = 0,
		//.explicitSwapControl = 0,	
	};
	emscripten_webgl_init_context_attributes(&emattrs);
	emattrs.majorVersion = 3;
	const char* canvas = stub_system_get_canvas();
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context = emscripten_webgl_create_context(canvas, &emattrs);
	emscripten_webgl_make_context_current(webgl_context);
#endif // _QN_EMSCRIPTEN_

	//----- EGL 초기화
	self->display = eglGetDisplay(self->native_display);
	if (self->display == EGL_NO_DISPLAY)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to get display");
		goto pos_fail_exit;
	}

	if (eglInitialize(self->display, NULL, NULL) == false)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to initialize: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}
#if !defined ES_STATIC_LINK
	es_reload_egl(self->display);	// eglInitialize 이후에나 버전을 알 수 있다. glad egl은 버전이 필요함
#endif

	// API 바인드
	if (eglBindAPI(EGL_OPENGL_ES_API) == false)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to initialize: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}

	// 컨피그 찾기
	if (es_choose_config(self, &config, msaa) == false)
		goto pos_fail_exit;

	// 컨텍스트 만들기
	EGLint context_attrs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, self->config.version,
		EGL_NONE, EGL_NONE,
	};
	self->context = eglCreateContext(self->display, self->config.handle, NULL, context_attrs);
	if (self->context == EGL_NO_CONTEXT)
	{
		context_attrs[1] = self->config.version == 2 ? 3 : 2;
		self->context = eglCreateContext(self->display, self->config.handle, NULL, context_attrs);
		if (self->context != EGL_NO_CONTEXT)
			goto pos_context_ok;

		qn_debug_outputf(true, VAR_CHK_NAME, "failed to create context: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}
pos_context_ok:
	eglQueryContext(self->display, self->context, EGL_CONTEXT_CLIENT_VERSION, &self->version);

	// 서피스 만들기
#ifdef _QN_ANDROID_
	ANativeWindow_setBuffersGeometry(self->native_window, 0, 0, self->base.base.info.visual_id);
#endif
	const EGLint surface_attrs[] =
	{
		EGL_NONE, EGL_NONE,
	};
	self->surface = eglCreateWindowSurface(self->display, self->config.handle, self->native_window, surface_attrs);
	if (self->surface == EGL_NO_SURFACE)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to create surface: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}
#ifdef _QN_ANDROID_
	int android_format = ANativeWindow_getFormat(self->native_window);
	Android_SetFormat(self->base.base.info.visual_id, android_format);
#endif

	// 서피스를 만들었으니 여기서 윈도우 표시
	stub_system_actuate();

	// 커런트 만들고
	if (eglMakeCurrent(self->display, self->surface, self->surface, self->context) == false)
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to make current: %s", es_egl_error_string(eglGetError()));
		goto pos_fail_exit;
	}

#ifndef ES_STATIC_LINK
	if (es_init_api(self->version) == false)	// eglMakeCurrent 하고 나야 GL 드라이버가 로드된다
		goto pos_fail_exit;
#endif // ES_STATIC_LINK

	eglSwapInterval(self->display, QN_TMASK(flags, QGFLAG_VSYNC) ? 1 : 0);

	//----- 정보 설정
	const char* gl_version = (const char*)glGetString(GL_VERSION);
	const char* gl_shader_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	const int max_layout_count = qgl_get_integer_v(GL_MAX_VERTEX_ATTRIBS);

	RendererInfo* info = &self->base.base.info;	// 여기서 RDH_INFO는 쓸 수 없다
	qgl_copy_string(info->renderer, QN_COUNTOF(info->renderer), GL_RENDERER);
	qgl_copy_string(info->vendor, QN_COUNTOF(info->vendor), GL_VENDOR);
	info->renderer_version = qgl_get_version(gl_version, "OPENGLES", "OPENGL ES");
	info->shader_version = qgl_get_version(gl_shader_version, "OPENGL ES GLSL ES ", "OPENGL ES GLSL ");
	info->max_layout_count = QN_MIN(max_layout_count, ES_MAX_LAYOUT_COUNT);
	info->max_tex_dim = qgl_get_integer_v(GL_MAX_TEXTURE_SIZE);
	info->max_tex_count = qgl_get_integer_v(GL_MAX_TEXTURE_IMAGE_UNITS);
	info->max_off_count = qgl_get_integer_v(GL_MAX_DRAW_BUFFERS);
	info->clr_fmt = qg_rgba_to_clrfmt(config.red, config.green, config.blue, config.alpha, false);

	//
	qn_debug_outputf(false, VAR_CHK_NAME, "OPENGLES %d/%d [%s by %s]",
		info->renderer_version, info->shader_version,
		info->renderer, info->vendor);
	qn_debug_outputs(false, VAR_CHK_NAME, gl_version);
	qn_debug_outputs(false, VAR_CHK_NAME, gl_shader_version);
	return qs_init(self, RdhBase, &vt_es_rdh);

pos_fail_exit:
	if (self->context != EGL_NO_CONTEXT)
		eglDestroyContext(self->display, self->context);
	if (self->surface != EGL_NO_SURFACE)
		eglDestroySurface(self->display, self->surface);
	if (self->display != EGL_NO_DISPLAY)
		eglTerminate(self->display);
	qn_free(self);
	return NULL;
}

//
static void es_dispose(QsGam* g)
{
	EsRdh* self = qs_cast_type(g, EsRdh);
	qn_ret_if_ok(self->base.disposed);
	self->base.disposed = true;

	//----- 펜딩
	const QglPending* pd = &self->base.pd;
	qs_unload(pd->render.index_buffer);
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qs_unload(pd->render.vertex_buffers[i]);
	qs_unload(pd->render.render);

	//----- 리소스
	rdh_internal_clean();

	//----- 장치 제거
	if (self->context != EGL_NO_CONTEXT)
		eglDestroyContext(self->display, self->context);
	if (self->surface != EGL_NO_SURFACE)
		eglDestroySurface(self->display, self->surface);
	if (self->display != EGL_NO_DISPLAY)
		eglTerminate(self->display);

	rdh_internal_dispose();
}

//
static void es_layout(void)
{
	rdh_internal_layout();

	RenderTransform* tm = RDH_TRANSFORM;
	QmMat4 ortho = ortho = qm_mat4_ortho_lh(tm->size.Width, -tm->size.Height, -1.0f, 1.0f);
	tm->ortho = qm_mat4_mul(ortho, qm_mat4_loc(-1.0f, 1.0f, 0.0f));

	// 뷰포트
	glViewport(0, 0, (GLsizei)tm->size.Width, (GLsizei)tm->size.Height);
	glDepthRangef(0.0f, 1.0f);
}

//
static void es_reset(void)
{
	rdh_internal_reset();

	const RendererInfo* info = RDH_INFO;
	RenderTransform* tm = RDH_TRANSFORM;
	QglSession* ss = QGL_SESSION;

	//----- 펜딩

	//----- 세션
	uint amask = ss->shader.amask;
	ss->shader.program = 0;
	ss->shader.amask = 0;
	qn_zero(ss->shader.lprops, QGLOU_MAX_SIZE, QglLayoutProperty);
	ss->buffer.vertex = GL_INVALID_HANDLE;
	ss->buffer.index = GL_INVALID_HANDLE;
	ss->buffer.uniform = GL_INVALID_HANDLE;
	ss->depth = QGDEPTH_LE;
	ss->stencil = QGSTENCIL_OFF;

	//----- 트랜스폼
	tm->frm = qgl_mat4_irrcht_texture(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f);

	//----- 장치 설정
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 뎁스 스텐실
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_STENCIL_TEST);

	// 래스터라이즈
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDisable(GL_POLYGON_OFFSET_FILL);

	// 블렌드
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

	glDisable(GL_BLEND);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// 텍스쳐
	for (size_t i = 0; i < info->max_tex_count; i++)
	{
		glActiveTexture((GLenum)(GL_TEXTURE0 + i));
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// 세이더
	for (size_t i = 0; i < info->max_layout_count; i++)
	{
		if (QN_TBIT(amask, i))
			glDisableVertexAttribArray((GLuint)i);
	}
	glUseProgram(0);

	// 가위질
	glDisable(GL_SCISSOR_TEST);
	glFrontFace(GL_CW);

	//----- 리소스
	static const char vs_ortho[] = \
		"uniform mat4 OrthoProj;" \
		"attribute vec4 aPosition;" \
		"attribute vec4 aColor;" \
		"varying vec2 vCoord;" \
		"varying vec4 vColor;" \
		"void main()" \
		"{" \
		"	gl_Position = OrthoProj * vec4(aPosition.xy, 0.0, 1.0);" \
		"	vCoord = aPosition.zw;"\
		"	vColor = aColor;" \
		"}";
	static const char ps_ortho[] = \
		"precision mediump float;" \
		"uniform sampler2D Texture;" \
		"varying vec2 vCoord;" \
		"varying vec4 vColor;" \
		"void main()" \
		"{" \
		"	gl_FragColor = texture2D(Texture, vCoord) * vColor;" \
		"}";
	static const char ps_glyph[] = \
		"precision mediump float;" \
		"uniform sampler2D Texture;" \
		"varying vec2 vCoord;" \
		"varying vec4 vColor;" \
		"void main()" \
		"{" \
		"	float a = texture2D(Texture, vCoord).r * vColor.a;"\
		"	gl_FragColor = vec4(vColor.rgb, a);" \
		"}";
	static QgLayoutInput inputs_ortho[] =
	{
		{ QGLOS_1, QGLOU_POSITION, QGCF_R32G32B32A32F, false },
		{ QGLOS_1, QGLOU_COLOR1, QGCF_R32G32B32A32F, false },
	};
	static QgPropRender render_ortho = QG_DEFAULT_PROP_RENDER;

	QgPropShader shader_ortho = { { inputs_ortho, QN_COUNTOF(inputs_ortho) }, { vs_ortho, 0 }, { ps_ortho, 0 } };
	QGL_RESOURCE->ortho_render = (QglRender*)qg_rdh_create_render("qg_ortho", &render_ortho, &shader_ortho);
	qs_unload(QGL_RESOURCE->ortho_render);

	QgPropShader shader_glyph = { { inputs_ortho, QN_COUNTOF(inputs_ortho) }, { vs_ortho, 0 }, { ps_glyph, 0 } };
	QGL_RESOURCE->glyph_render = (QglRender*)qg_rdh_create_render("qg_glyph", &render_ortho, &shader_glyph);
	qs_unload(QGL_RESOURCE->glyph_render);
}

// 지우기
static void es_clear(QgClear flags)
{
	// 도움: https://open.gl/depthstencils
	GLbitfield cf = 0;

	if (QN_TMASK(flags, QGCLEAR_STENCIL))
	{
		glClearStencil(0.0f);
		cf |= GL_STENCIL_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_DEPTH))
	{
		glDepthMask(GL_TRUE);
		glClearDepthf(1.0f);
		cf |= GL_DEPTH_BUFFER_BIT;
	}
	if (QN_TMASK(flags, QGCLEAR_RENDER))
	{
		const QmVec4 c = RDH_PARAM->bgc;
		glClearColor(c.X, c.Y, c.Z, c.W);
		cf |= GL_COLOR_BUFFER_BIT;
	}

	if (cf != 0)
		glClear(cf);
}

// 시작
static bool es_begin(bool clear)
{
	if (clear)
		es_clear(QGCLEAR_DEPTH | QGCLEAR_STENCIL | QGCLEAR_RENDER);
	return true;
}

// 끝
static void es_end(void)
{
}

// 플러시
static void es_flush(void)
{
	EsRdh* self = ES_RDH;

	glFlush();
	eglSwapBuffers(self->display, self->surface);
}

// 정점 버퍼 설정, stage에 대한 오류 설정은 rdh에서 하고 왔을 거임
static bool es_set_vertex(QgLayoutStage stage, QgBuffer* buffer)
{
	QglBuffer* buf = qs_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (pd->render.vertex_buffers[stage] != buf)
	{
		qs_unload(pd->render.vertex_buffers[stage]);
		pd->render.vertex_buffers[stage] = qs_loadc(buf, QglBuffer);
	}
	return true;
}

// 인덱스 버퍼 설정
static bool es_set_index(QgBuffer* buffer)
{
	QglBuffer* buf = qs_cast_type(buffer, QglBuffer);
	QglPending* pd = QGL_PENDING;
	if (pd->render.index_buffer != buf)
	{
		qs_unload(pd->render.index_buffer);
		pd->render.index_buffer = qs_loadc(buf, QglBuffer);
	}
	return true;
}

// 렌더 설정
static bool es_set_render(QgRender* render)
{
	QglRender* rdr = qs_cast_type(render, QglRender);
	QglPending* pd = QGL_PENDING;
	if (pd->render.render != rdr)
	{
		qs_unload(pd->render.render);
		pd->render.render = qs_loadc(rdr, QglRender);
	}
	return true;
}

// 정점 바인딩
static void es_bind_vertex_buffer(const QglBuffer* buffer)
{
	qn_assert(buffer->base.type == QGBUFFER_VERTEX, "try to bind non-vertex buffer");
	QglSession* ss = QGL_SESSION;
	GLuint gl_id = qs_get_desc(buffer, GLuint);
	if (ss->buffer.vertex != gl_id)
	{
		glBindBuffer(GL_ARRAY_BUFFER, gl_id);
		ss->buffer.vertex = gl_id;
	}
}

// 인덱스 바인딩
static void es_bind_index_buffer(const QglBuffer* buffer)
{
	qn_assert(buffer->base.type == QGBUFFER_INDEX, "try to bind non-index buffer");
	QglSession* ss = QGL_SESSION;
	GLuint gl_id = qs_get_desc(buffer, GLuint);
	if (ss->buffer.index != gl_id)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_id);
		ss->buffer.index = gl_id;
	}
}

// 유니폼 바인딩
static void es_bind_uniform_buffer(const QglBuffer* buffer)
{
	qn_assert(buffer->base.type == QGBUFFER_CONSTANT, "try to bind non-uniform buffer");
	QglSession* ss = QGL_SESSION;
	GLuint gl_id = qs_get_desc(buffer, GLuint);
	if (ss->buffer.uniform != gl_id)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, gl_id);
		ss->buffer.uniform = gl_id;
	}
}

// 버퍼 바인딩
static void es_bind_buffer(const QglBuffer* buffer)
{
	switch (buffer->base.type)
	{
		case QGBUFFER_VERTEX:
			es_bind_vertex_buffer(buffer);
			break;
		case QGBUFFER_INDEX:
			es_bind_index_buffer(buffer);
			break;
		case QGBUFFER_CONSTANT:
			es_bind_uniform_buffer(buffer);
			break;
		default:
			qn_debug_outputs(true, VAR_CHK_NAME, "invalid buffer type");
			break;
	}
}

// 오토 세이더 변수 (오토가 아니면 RDH의 사용자 함수로 떠넘긴다)
static void es_process_shader_variable(const QgVarShader* var)
{
	switch (var->scauto)
	{
		case QGSCA_ORTHO_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->ortho._11);
			break;
		case QGSCA_WORLD:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->world._11);
			break;
		case QGSCA_VIEW:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->view._11);
			break;
		case QGSCA_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->proj._11);
			break;
		case QGSCA_VIEW_PROJ:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->view_proj._11);
			break;
		case QGSCA_INV_VIEW:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_TRANSFORM->invv._11);
			break;
		case QGSCA_WORLD_VIEW_PROJ:
		{
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			QmMat4 m = qm_mat4_mul(RDH_TRANSFORM->world, RDH_TRANSFORM->view_proj);
			glUniformMatrix4fv(var->offset, 1, false, &m._11);
		} break;
		case QGSCA_TEX1:
		case QGSCA_TEX2:
		case QGSCA_TEX3:
		case QGSCA_TEX4:
		case QGSCA_TEX5:
		case QGSCA_TEX6:
		case QGSCA_TEX7:
		case QGSCA_TEX8:
			switch (var->sctype)
			{
				case QGSCT_SAMPLER1D:
				case QGSCT_SAMPLER2D:
				case QGSCT_SAMPLER3D:
				case QGSCT_SAMPLERCUBE:
					glUniform1i(var->offset, var->scauto - QGSCA_TEX1);
					break;
				default:
					qn_debug_outputs(true, VAR_CHK_NAME, "invalid auto shader texture");
					break;
			}
			break;
		case QGSCA_PROP_VEC1:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[0].f);
			break;
		case QGSCA_PROP_VEC2:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[1].f);
			break;
		case QGSCA_PROP_VEC3:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[2].f);
			break;
		case QGSCA_PROP_VEC4:
			qn_verify(var->sctype == QGSCT_FLOAT4 && var->size == 1);
			glUniform4fv(var->offset, 1, RDH_PARAM->v[3].f);
			break;
		case QGSCA_PROP_MAT1:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[0]._11);
			break;
		case QGSCA_PROP_MAT2:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[1]._11);
			break;
		case QGSCA_PROP_MAT3:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[2]._11);
			break;
		case QGSCA_PROP_MAT4:
			qn_verify(var->sctype == QGSCT_FLOAT16 && var->size == 1);
			glUniformMatrix4fv(var->offset, 1, false, &RDH_PARAM->m[3]._11);
			break;
		case QGSCA_MAT_PALETTE:
			qn_verify(var->sctype == QGSCT_FLOAT16);
			glUniformMatrix4fv(var->offset, RDH_PARAM->bone_count, false, (const GLfloat*)RDH_PARAM->bone_ptr);
			break;
		default:
			if (RDH_PARAM->callback_func != NULL)
				RDH_PARAM->callback_func(RDH_PARAM->callback_data, (nint)var->hash, var);
			break;
	}
}

// 세이더랑 레이아웃
static bool es_commit_shader_layout(const QglRender* rdr)
{
	const size_t max_attrs = RDH_INFO->max_layout_count;

	// 프로그램 설정
	if (QGL_SESSION->shader.program != rdr->shader.program)
	{
		QGL_SESSION->shader.program = rdr->shader.program;
		glUseProgram(rdr->shader.program);
	}

	// 유니폼 값 넣고
	size_t s, i;
	qn_ctnr_foreach(&rdr->shader.uniforms, i)
	{
		const QgVarShader* var = &qn_ctnr_nth(&rdr->shader.uniforms, i);
		es_process_shader_variable(var);
	}

	// 어트리뷰트 + 레이아웃
	uint aok = 0, amask = QGL_SESSION->shader.amask;
	for (s = 0; s < QGLOS_MAX_VALUE; s++)
	{
		QglBuffer* buf = QGL_PENDING->render.vertex_buffers[s];
		if (buf == NULL)
			continue;
		es_bind_vertex_buffer(buf);

		const QglLayoutInput * stages = rdr->layout.stages[s];
		const GLsizei gl_stride = (GLsizei)rdr->layout.strides[s];
		const size_t count = rdr->layout.counts[s];
		for (i = 0; i < count; i++)
		{
			const QglLayoutInput* input = &stages[i];
			const GLint gl_attr = rdr->shader.usages[input->usage];
			if (gl_attr == 0xFF)
			{
				const char* name = qg_layout_usage_to_str(input->usage);
				qn_debug_outputf(true, VAR_CHK_NAME, "shader attribute not found: %s", name);
				continue;
			}

			aok |= QN_BIT(gl_attr);
			if (QN_TBIT(amask, gl_attr) == false)
			{
				QN_SBIT(&amask, gl_attr, true);
				glEnableVertexAttribArray(gl_attr);
			}

			const GLuint gl_offset = input->offset;
			QglLayoutProperty* lp = &QGL_SESSION->shader.lprops[input->usage];
			if (lp->offset != gl_offset ||
				lp->format != input->format ||
				lp->stride != gl_stride ||
				lp->count != input->count ||
				lp->normalized != input->normalized)
			{
				glVertexAttribPointer(gl_attr, input->count, input->format, input->normalized, gl_stride, (GLvoid*)(size_t)gl_offset);
				lp->offset = gl_offset;
				lp->format = input->format;
				lp->stride = gl_stride;
				lp->count = input->count;
				lp->normalized = input->normalized;
			}
		}
	}

	// 레이아웃 정리
	uint aftermask = amask & ~aok;
	for (i = 0; i < max_attrs && aftermask; i++)
	{
		if (QN_TBIT(aftermask, 0))
		{
			QN_SBIT(&amask, i, false);
			glDisableVertexAttribArray((GLuint)i);
		}
		aftermask >>= 1;
	}
	QGL_SESSION->shader.amask = amask;

	return true;
}

// 렌더 커밋
static bool es_commit_render(void)
{
	const QglRender* rdr = QGL_PENDING->render.render;
	VAR_CHK_IF_NULL(rdr, false);

	if (es_commit_shader_layout(rdr) == false)
		return false;

	return true;
}

// 그리기
static bool es_draw(QgTopology tpg, int vertices)
{
	if (es_commit_render() == false)
		return false;

	GLenum gl_tpg = qgl_topology_to_enum(tpg);
	glDrawArrays(gl_tpg, 0, (GLsizei)vertices);
	return true;
}

// 그리기 인덱스
static bool es_draw_indexed(QgTopology tpg, int indices)
{
	const QglBuffer* index = QGL_PENDING->render.index_buffer;
	VAR_CHK_IF_NULL(index, false);
	es_bind_index_buffer(index);

	if (es_commit_render() == false)
		return false;

	const GLenum gl_tpg = qgl_topology_to_enum(tpg);
	const GLenum gl_stride = index->base.stride == sizeof(uint) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	glDrawElements(gl_tpg, (GLsizei)indices, gl_stride, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
// ES 버퍼

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "ESBUFFER"

//
static void esbuffer_dispose(QsGam* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	if (self->base.mapped)
	{
		if (RDH_INFO->renderer_version >= 300)
			glUnmapBuffer(self->gl_type);
		else
			qn_free(self->lock_pointer);
	}

	GLuint gl_handle = qs_get_desc(self, GLuint);
	glDeleteBuffers(1, &gl_handle);

	qn_free(self);
}

//
static void* esbuffer_map(QgBuffer* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEED2(self, gl_usage, GL_DYNAMIC_DRAW, NULL);
	qn_assert(self->lock_pointer == NULL, "버퍼가 잠겨있는데요!");

	if (RDH_INFO->renderer_version < ES_VERSION_3)
	{
		// ES2
		self->lock_pointer = qn_alloc(self->base.size, byte);
	}
	else
	{
		// ES3
		es_bind_buffer(self);
		self->lock_pointer = glMapBufferRange(self->gl_type, 0, self->base.size,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		return self->lock_pointer;
	}

	self->base.mapped = true;
	return self->lock_pointer;
}

//
static bool esbuffer_unmap(QgBuffer* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	qn_assert(self->lock_pointer != NULL, "버퍼가 안 잠겼는데요!");

	es_bind_buffer(self);

	if (RDH_INFO->renderer_version >= ES_VERSION_3)
		glUnmapBuffer(self->gl_type);
	else
	{
		glBufferSubData(self->gl_type, 0, self->base.size, self->lock_pointer);
		qn_free(self->lock_pointer);
	}

	self->lock_pointer = NULL;
	self->base.mapped = false;
	return true;
}

//
static bool esbuffer_data(QgBuffer* g, const void* data)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEED2(self, gl_usage, GL_DYNAMIC_DRAW, false);

	es_bind_buffer(self);
	glBufferSubData(self->gl_type, 0, self->base.size, data);

	return true;
}

//
static QgBuffer* es_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
{
	// 우선 만들자
	GLsizeiptr gl_size = (GLsizeiptr)count * stride;
	GLenum gl_type;
	if (type == QGBUFFER_VERTEX)
	{
		gl_type = GL_ARRAY_BUFFER;
		QGL_SESSION->buffer.vertex = GL_INVALID_HANDLE;
	}
	else if (type == QGBUFFER_INDEX)
	{
		VAR_CHK_IF_COND(stride != 2 && stride != 4, "invalid index buffer stride. require 2 or 4", NULL);
		gl_type = GL_ELEMENT_ARRAY_BUFFER;
		QGL_SESSION->buffer.index = GL_INVALID_HANDLE;
	}
	else if (type == QGBUFFER_CONSTANT)
	{
		gl_type = GL_UNIFORM_BUFFER;
		// 256 바이트 정렬
		gl_size = (gl_size + 255) & ~255;
		QGL_SESSION->buffer.uniform = GL_INVALID_HANDLE;
	}
	else
	{
		qn_debug_outputf(true, VAR_CHK_NAME, "invalid buffer type");
		return NULL;
	}

	GLuint gl_id;
	glGenBuffers(1, &gl_id);
	glBindBuffer(gl_type, gl_id);

	GLenum gl_usage = initial_data != NULL ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	glBufferData(gl_type, gl_size, initial_data, gl_usage);		// 여기까지 초기 메모리 설정 또는 데이터 넣기(STATIC)

	// 진짜 만듦
	QglBuffer* self = qn_alloc_zero_1(QglBuffer);
	qs_set_desc(self, gl_id);
	self->base.type = type;
	self->base.size = (uint)gl_size;
	self->base.count = count;
	self->base.stride = (ushort)stride;
	self->gl_type = gl_type;
	self->gl_usage = gl_usage;

	// VT 여기서 설정
	static qs_name_vt(QGBUFFER) vt_es_buffer =
	{
		.base.name = "ESBUFFER",
		.base.dispose = esbuffer_dispose,

		.map = esbuffer_map,
		.unmap = esbuffer_unmap,
		.data = esbuffer_data,
	};
	return qs_init(self, QgBuffer, &vt_es_buffer);
}


//////////////////////////////////////////////////////////////////////////
// 렌더러

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "ESRENDER"

//
static void esrender_delete_shader(QglRender* self, bool check_rdh)
{
	if (self->shader.program == 0)
		return;
	if (check_rdh && QGL_RDH->disposed == false && QGL_SESSION->shader.program == self->shader.program)
	{
		QGL_SESSION->shader.program = 0;
		glUseProgram(0);
	}
	if (self->shader.vertex != 0)
	{
		glDetachShader(self->shader.program, self->shader.vertex);
		glDeleteShader(self->shader.vertex);
	}
	if (self->shader.fragment != 0)
	{
		glDetachShader(self->shader.program, self->shader.fragment);
		glDeleteShader(self->shader.fragment);
	}
	glDeleteProgram(self->shader.program);
}

//
static void esrender_dispose(QsGam* g)
{
	QglRender* self = qs_cast_type(g, QglRender);

	esrender_delete_shader(self, true);
	qn_ctnr_disp(&self->shader.uniforms);
	qn_ctnr_disp(&self->shader.attrs);
	qn_ctnr_disp(&self->layout.inputs);

	rdh_internal_unlink_node(RDHNODE_RENDER, self);
	qn_free(self);
}

// 세이더 컴파일
static GLuint esrender_compile_shader(GLenum gl_type, const char* code)
{
	// http://msdn.microsoft.com/ko-kr/library/windows/apps/dn166905.aspx
	GLuint gl_shader = glCreateShader(gl_type);
	glShaderSource(gl_shader, 1, &code, NULL);
	glCompileShader(gl_shader);

	if (qgl_get_shader_iv(gl_shader, GL_COMPILE_STATUS) != GL_FALSE)
		return gl_shader;

	const char* type = gl_type == GL_VERTEX_SHADER ? "vertex" : gl_type == GL_FRAGMENT_SHADER ? "fragment" : "unknown";
	GLint gl_log_len = qgl_get_shader_iv(gl_shader, GL_INFO_LOG_LENGTH);
	if (gl_log_len <= 0)
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to %s shader compile", type);
	else
	{
		GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
		glGetShaderInfoLog(gl_shader, gl_log_len, &gl_log_len, gl_log);
		qn_debug_outputf(true, VAR_CHK_NAME, "failed to %s shader compile: %s", type, gl_log);
		qn_free(gl_log);
	}

	glDeleteShader(gl_shader);
	return 0;
}

// 세이더 만들기
static bool esrender_bind_shader(QglRender* self, const QgCodeData* vertex, const QgCodeData* fragment)
{
	// 프로그램이랑 세이더 만들고
	GLuint gl_vertex_shader = esrender_compile_shader(GL_VERTEX_SHADER, vertex->code);
	GLuint gl_fragment_shader = esrender_compile_shader(GL_FRAGMENT_SHADER, fragment->code);
	if (gl_vertex_shader == 0 || gl_fragment_shader == 0)
	{
		if (gl_vertex_shader != 0)
			glDeleteShader(gl_vertex_shader);
		if (gl_fragment_shader != 0)
			glDeleteShader(gl_fragment_shader);
		return false;
	}

	self->shader.vertex = gl_vertex_shader;
	self->shader.fragment = gl_fragment_shader;
	self->shader.program = glCreateProgram();
	glAttachShader(self->shader.program, gl_vertex_shader);
	glAttachShader(self->shader.program, gl_fragment_shader);

	// 링크
	glLinkProgram(self->shader.program);
	if (qgl_get_program_iv(self->shader.program, GL_LINK_STATUS) == GL_FALSE)
	{
		GLsizei gl_log_len = qgl_get_program_iv(self->shader.program, GL_INFO_LOG_LENGTH);
		if (gl_log_len <= 0)
			qn_debug_outputf(true, VAR_CHK_NAME, "failed to link shader");
		else
		{
			GLchar* gl_log = qn_alloc(gl_log_len, GLchar);
			glGetProgramInfoLog(self->shader.program, gl_log_len, &gl_log_len, gl_log);
			qn_debug_outputf(true, VAR_CHK_NAME, "failed to link shader: %s", gl_log);
			qn_free(gl_log);
		}
		return false;
	}

	// 분석
	GLint i, index, gl_count, gl_size, gl_index;
	GLenum gl_type;
	GLchar sz[64];

	// 유니폼
	gl_count = qgl_get_program_iv(self->shader.program, GL_ACTIVE_UNIFORMS);
	if (gl_count > 0)
	{
		qn_ctnr_init(QglCtnUniform, &self->shader.uniforms, gl_count);
		for (index = i = 0; i < gl_count; i++)
		{
			glGetActiveUniform(self->shader.program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz);
			gl_index = glGetUniformLocation(self->shader.program, sz);
			if (gl_index < 0)
			{
				qn_debug_outputf(true, VAR_CHK_NAME, "failed to get uniform location: %s", sz);
				continue;
			}

			QgScType sctype = qgl_enum_to_shader_const(gl_type);
			if (sctype == QGSCT_UNKNOWN)
				qn_debug_outputf(true, VAR_CHK_NAME, "unsupported uniform type: %s (type: %X)", sz, gl_type);

			QgVarShader* var = &qn_ctnr_nth(&self->shader.uniforms, index++);
			qn_strcpy(var->name, sz);
			var->hash = qn_strihash(sz);
			var->offset = (ushort)glGetUniformLocation(self->shader.program, sz);
			var->size = (ushort)gl_size;
			var->sctype = sctype;
			var->scauto = qg_str_to_shader_const_auto(var->hash, sz);
			if (var->scauto == QGSCA_UNKNOWN)
				var->hash = (size_t)qn_get_key(sz);
		}
	}

	// 어트리뷰트
	memset(self->shader.usages, 0xFF, QN_COUNTOF(self->shader.usages));
	gl_count = qgl_get_program_iv(self->shader.program, GL_ACTIVE_ATTRIBUTES);
	if (gl_count > 0)
	{
		qn_ctnr_init(QglCtnAttr, &self->shader.attrs, gl_count);
		for (index = i = 0; i < gl_count; i++)
		{
			glGetActiveAttrib(self->shader.program, i, QN_COUNTOF(sz), NULL, &gl_size, &gl_type, sz);
			gl_index = glGetAttribLocation(self->shader.program, sz);
			if (gl_index < 0)
			{
				qn_debug_outputf(true, VAR_CHK_NAME, "failed to get attribute location: %s", sz);
				continue;
			}

			QgScType sctype = qgl_enum_to_shader_const(gl_type);
			if (sctype == QGSCT_UNKNOWN)
				qn_debug_outputf(true, VAR_CHK_NAME, "unsupported attribute type: %s (type: %X)", sz, gl_type);

			QglVarAttr* var = &qn_ctnr_nth(&self->shader.attrs, index++);
			qn_strcpy(var->name, sz);
			var->hash = qn_strihash(sz);
			var->attrib = gl_index;
			var->size = gl_size;
			var->usage = qg_str_to_layout_usage(var->hash, sz);
			var->sctype = sctype;
			self->shader.usages[var->usage] = (byte)gl_index;
		}
	}

	return true;
}

// 버텍스 레이아웃 만들기
static bool esrender_bind_layout_input(QglRender* self, const QgLayoutData* layout)
{
	static byte lo_count[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4, 3, 2, 1,
		/* INT     */ 4, 3, 2, 1,
		/* HALF-F  */ 4, 2, 1, 1,
		/* HALF-I  */ 4, 2, 1, 1,
		/* BYTE    */ 4, 3, 2, 1, 1, 1,
		/* USHORT  */ 1, 1, 1,
		/* NONE    */ 0, 0,
	};
	static GLenum lo_format[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ GL_NONE,
		/* FLOAT   */ GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT,
		/* INT     */ GL_INT, GL_INT, GL_INT, GL_INT,
		/* HALF-F  */ GL_HALF_FLOAT, GL_HALF_FLOAT, GL_HALF_FLOAT, GL_UNSIGNED_INT_10F_11F_11F_REV,
		/* HALF-I  */ GL_SHORT, GL_SHORT, GL_SHORT, GL_UNSIGNED_INT_2_10_10_10_REV,
		/* BYTE    */ GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE,
		/* USHORT  */ GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_4_4_4_4,
		/* NONE    */ GL_NONE, GL_NONE,
	};
	static byte lo_size[QGCF_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4 * sizeof(float), 3 * sizeof(float), 2 * sizeof(float), 1 * sizeof(float),
		/* INT     */ 4 * sizeof(int), 3 * sizeof(int), 2 * sizeof(int), 1 * sizeof(int),
		/* HALF-F  */ 4 * sizeof(halffloat), 2 * sizeof(halffloat), 1 * sizeof(halffloat), 1 * sizeof(int),
		/* HALF-I  */ 4 * sizeof(short), 2 * sizeof(short), 1 * sizeof(short), 1 * sizeof(int),
		/* BYTE    */ 4 * sizeof(byte), 3 * sizeof(byte), 2 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte),
		/* USHORT  */ 2 * sizeof(ushort), 2 * sizeof(ushort), 2 * sizeof(ushort),
		/* NONE    */ 0, 0,
	};

	size_t i;

	// 갯수 검사
	ushort loac[QGLOS_MAX_VALUE] = { 0, };
	for (i = 0; i < layout->count; i++)
	{
		const QgLayoutInput* input = &layout->inputs[i];
		if ((size_t)input->stage >= QGLOS_MAX_VALUE)
		{
			qn_debug_outputf(true, VAR_CHK_NAME, "invalid layout stage: %d", input->stage);
			return false;
		}
		loac[input->stage]++;
	}

	// 레이아웃
	qn_ctnr_init(QglCtnLayoutInput, &self->layout.inputs, layout->count);
	QglLayoutInput* pstage = qn_ctnr_data(&self->layout.inputs);
	QglLayoutInput* stages[QGLOS_MAX_VALUE];
	for (i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (loac[i] == 0)
			stages[i] = NULL;
		else
		{
			self->layout.counts[i] = loac[i];
			self->layout.stages[i] = stages[i] = pstage;
			pstage += loac[i];
		}
	}

	// 항목별로 만들어서 스테이지로 구분
	for (i = 0; i < layout->count; i++)
	{
		const QgLayoutInput* input = &layout->inputs[i];
		const GLenum gl_format = lo_format[input->format];
		if (gl_format == GL_NONE)
		{
			const char* name = qg_clrfmt_to_str(input->format);
			qn_debug_outputf(true, VAR_CHK_NAME, "cannot use or unknown layout format: %s", name);
			return false;
		}

		QglLayoutInput* stage = stages[input->stage]++;
		stage->stage = input->stage;
		stage->usage = input->usage;
		stage->offset = self->layout.strides[input->stage];
		stage->format = gl_format;
		stage->count = lo_count[input->format];
		stage->normalized = input->normalized;
		self->layout.strides[input->stage] += lo_size[input->format];
	}

	return true;
}

// 렌더 만들기. 오류 처리는 다하고 왔을 것이다
static QgRender* es_create_render(const char* name, const QgPropRender* prop, const QgPropShader* shader)
{
	QglRender* self = qn_alloc_zero_1(QglRender);
	qg_node_set_name(qs_cast_type(self, QgNode), name);

	// 세이더
	if (esrender_bind_shader(self, &shader->vertex, &shader->pixel) == false)
		goto pos_error;

	// 레이아웃
	if (esrender_bind_layout_input(self, &shader->layout) == false)
		goto pos_error;

	//
	static qs_name_vt(QSGAM) vt_es_render =
	{
		.name = "ESRENDER",
		.dispose = esrender_dispose,
	};
	qs_init(self, QgRender, &vt_es_render);
	if (name)
		rdh_internal_add_node(RDHNODE_RENDER, self);
	return qs_cast_type(self, QgRender);

pos_error:
	esrender_delete_shader(self, false);
	qn_ctnr_disp(&self->shader.uniforms);
	qn_ctnr_disp(&self->shader.attrs);
	qn_ctnr_disp(&self->layout.inputs);
	qn_free(self);
	return NULL;
}

#endif // USE_ES
