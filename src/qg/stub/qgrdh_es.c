//
// qgrdh_es.c - OPENGL ES 렌더 디바이스
// 2024-01-10 by kim
//

#include "pch.h"
#ifdef USE_ES
#ifdef _QN_EMSCRIPTEN_
#include <emscripten/html5_webgl.h>
#else
#define GLAD_EGL_IMPLEMENTATION		1
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

static void es_rdh_dispose(QsGam* g);
static void es_depth_range(float n, float f);
static void es_clear_depth(float d);
static void es_rdh_reset(void);
static void es_rdh_flush(void);
static bool es_rdh_draw(QgTopology tpg, int vertices);
static bool es_rdh_draw_indexed(QgTopology tpg, int indices);
static QgBuffer* es_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data);

qs_name_vt(QGLRDH) vt_es_rdh =
{
	{
		{
			/* name */		VAR_CHK_NAME,
			/* dispose */	es_rdh_dispose,
		},

		/* layout */		qgl_rdh_layout,
		/* reset */			es_rdh_reset,
		/* clear */			qgl_rdh_clear,

		/* begin */			qgl_rdh_begin,
		/* end */			qgl_rdh_end,
		/* flush */			es_rdh_flush,

		/* create_buffer */	es_create_buffer,
		/* create_render */	qgl_create_render,

		/* set_vertex */	qgl_rdh_set_vertex,
		/* set_index */		qgl_rdh_set_index,
		/* set_render */	qgl_rdh_set_render,

		/* draw */			es_rdh_draw,
		/* draw_indexed */	es_rdh_draw_indexed,
	},

	/* gl_depth_range */	es_depth_range,
	/* gl_clear_depth */	es_clear_depth,
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
static void es_get_config(EGLDisplay display, EGLConfig ec, QglConfig* config)
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
static bool es_choose_config(EsRdh* self, const QglConfig* wanted_config, QglConfig* found_config)
{
#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[i++] = e; attrs[i++] = v; }QN_STMT_END
	EGLint attrs[32], i = 0;
	ATTR_ADD(EGL_RED_SIZE, wanted_config->red);
	ATTR_ADD(EGL_GREEN_SIZE, wanted_config->green);
	ATTR_ADD(EGL_BLUE_SIZE, wanted_config->blue);
	if (wanted_config->alpha > 0)
		ATTR_ADD(EGL_ALPHA_SIZE, wanted_config->alpha);
	if (wanted_config->depth > 0)
		ATTR_ADD(EGL_DEPTH_SIZE, wanted_config->depth);
	if (wanted_config->stencil > 0)
		ATTR_ADD(EGL_STENCIL_SIZE, wanted_config->stencil);
	if (wanted_config->samples > 0)
		ATTR_ADD(EGL_SAMPLES, wanted_config->samples);
	ATTR_ADD(EGL_RENDERABLE_TYPE, wanted_config->version == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT);
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
		es_get_config(self->display, configs[0], found_config);
	}
	else
	{
		QglConfig* esconfigs = qn_alloc(config_count, QglConfig);
		for (i = 0; i < config_count; i++)
			es_get_config(self->display, configs[i], &esconfigs[i]);

		const QglConfig* found = qgl_detect_config(wanted_config, esconfigs, config_count);
		if (found == NULL)
			ret = false;
		else
		{
			memcpy(found_config, found, sizeof(QglConfig));
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
	QglConfig wanted_config;
	qgl_wanted_config(&wanted_config, 3, flags);

	//----- 여기에 RDH가!
	EsRdh* self = qn_alloc_zero_1(EsRdh);
	self->native_window = (NativeWindowType)stub_system_get_window();
	self->native_display = (NativeDisplayType)stub_system_get_display();

#ifdef _QN_EMSCRIPTEN_
	// EMSCRIPTEN ES3 초기화 WEBGL 이용하는 듯
	EmscriptenWebGLContextAttributes emattrs =
	{
		.alpha = wanted_config.alpha > 0 ? EM_TRUE : EM_FALSE,
		.depth = wanted_config.depth > 0 ? EM_TRUE : EM_FALSE,
		.stencil = wanted_config.stencil > 0 ? EM_TRUE : EM_FALSE,
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
	QglConfig config;
	if (es_choose_config(self, &wanted_config, &config) == false)
		goto pos_fail_exit;

	// 컨텍스트 만들기
	EGLint context_attrs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, config.version,
		EGL_NONE, EGL_NONE,
	};
	self->context = eglCreateContext(self->display, config.handle, NULL, context_attrs);
	if (self->context == EGL_NO_CONTEXT)
	{
		context_attrs[1] = config.version == 2 ? 3 : 2;
		self->context = eglCreateContext(self->display, config.handle, NULL, context_attrs);
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
	self->surface = eglCreateWindowSurface(self->display, config.handle, self->native_window, surface_attrs);
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
	const QglInitialInfo initial =
	{
		"OPENGLES",
		{ "OPENGLES", "OPENGL ES" },
		{ "OPENGL ES GLSL ES ", "OPENGL ES GLSL " },
		ES_MAX_LAYOUT_COUNT,
	};
	qgl_rdh_init_info(qs_cast_type(self, QglRdh), &config, &initial);
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
static void es_rdh_dispose(QsGam* g)
{
	EsRdh* self = qs_cast_type(g, EsRdh);
	if (qgl_rdh_finalize(qs_cast_type(self, QglRdh)) == false)
		return;

	if (self->context != EGL_NO_CONTEXT)
		eglDestroyContext(self->display, self->context);
	if (self->surface != EGL_NO_SURFACE)
		eglDestroySurface(self->display, self->surface);
	if (self->display != EGL_NO_DISPLAY)
		eglTerminate(self->display);

	rdh_internal_dispose();
}

//
static void es_depth_range(float n, float f)
{
	glDepthRangef(n, f);
}

//
static void es_clear_depth(float d)
{
	glClearDepthf(d);
}

//
static void es_rdh_reset(void)
{
	qgl_rdh_reset();

	// 블렌드
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

	glDisable(GL_BLEND);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

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

// 플러시
static void es_rdh_flush(void)
{
	qgl_rdh_flush();
	EsRdh* self = ES_RDH;
	eglSwapBuffers(self->display, self->surface);
}

// 렌더 커밋
static bool es_rdh_commit_render(void)
{
	const QglRender* rdr = QGL_PENDING->render.render;
	VAR_CHK_IF_NULL(rdr, false);

	if (qgl_commit_shader_layout(rdr) == false)
		return false;

	qgl_commit_depth_stencil(rdr);

	return true;
}

// 그리기
static bool es_rdh_draw(QgTopology tpg, int vertices)
{
	if (es_rdh_commit_render() == false)
		return false;

	GLenum gl_tpg = qgl_topology_to_enum(tpg);
	glDrawArrays(gl_tpg, 0, (GLsizei)vertices);
	return true;
}

// 그리기 인덱스
static bool es_rdh_draw_indexed(QgTopology tpg, int indices)
{
	const QglBuffer* index = QGL_PENDING->render.index_buffer;
	VAR_CHK_IF_NULL(index, false);
	qgl_bind_index_buffer(index);

	if (es_rdh_commit_render() == false)
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
static void es_buffer_dispose(QsGam* g)
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
static void* es_buffer_map(QgBuffer* g)
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
		qgl_bind_buffer(self);
		self->lock_pointer = glMapBufferRange(self->gl_type, 0, self->base.size,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		return self->lock_pointer;
	}

	self->base.mapped = true;
	return self->lock_pointer;
}

//
static bool es_buffer_unmap(QgBuffer* g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	qn_assert(self->lock_pointer != NULL, "버퍼가 안 잠겼는데요!");

	qgl_bind_buffer(self);

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
static bool es_buffer_data(QgBuffer* g, const void* data)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEED2(self, gl_usage, GL_DYNAMIC_DRAW, false);

	qgl_bind_buffer(self);
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
		.base.name = VAR_CHK_NAME,
		.base.dispose = es_buffer_dispose,

		.map = es_buffer_map,
		.unmap = es_buffer_unmap,
		.data = es_buffer_data,
	};
	return qs_init(self, QgBuffer, &vt_es_buffer);
}

#endif // USE_ES
