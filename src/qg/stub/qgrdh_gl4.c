//
// qgrdh_gl4.c - OPENGL 렌더 디바이스
// 2024-01-10 by kim
//

#include "pch.h"
#ifdef USE_GL
#include "qgrdh_gl4.h"
#include <qs_supp.h>
#include <limits.h>


//////////////////////////////////////////////////////////////////////////
// OPENGL 렌더 디바이스

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"GLRDH"

static void gl_rdh_dispose(QsGam* g);
static void gl_depth_range(float n, float f);
static void gl_clear_depth(float d);
static void gl_rdh_reset(void);
static void gl_rdh_flush(void);
static bool gl_rdh_draw(QgTopology tpg, int vertices);
static bool gl_rdh_draw_indexed(QgTopology tpg, int indices);
static QgBuffer* gl_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data);

qs_name_vt(QGLRDH) vt_gl_rdh =
{
	{
		{
			/* name */		VAR_CHK_NAME,
			/* dispose */	gl_rdh_dispose,
		},

		/* layout */		qgl_rdh_layout,
		/* reset */			gl_rdh_reset,
		/* clear */			qgl_rdh_clear,

		/* begin */			qgl_rdh_begin,
		/* end */			qgl_rdh_end,
		/* flush */			gl_rdh_flush,

		/* create_buffer */	gl_create_buffer,
		/* create_render */	qgl_create_render,

		/* set_vertex */	qgl_rdh_set_vertex,
		/* set_index */		qgl_rdh_set_index,
		/* set_render */	qgl_rdh_set_render,

		/* draw */			gl_rdh_draw,
		/* draw_indexed */	gl_rdh_draw_indexed,
	},

	/* gl_depth_range */	gl_depth_range,
	/* gl_clear_depth */	gl_clear_depth,
};

#if defined _QN_WINDOWS_
#include "glad/wglext.h"

extern void* stub_system_get_dummy_window(void);

//
static QnModule* gl_module = NULL;

typedef PROC(APIENTRY *PFNWGLGETPROCADDRESSPROC)(LPCSTR lpszProc);
typedef HGLRC(APIENTRY *PFNWGLCREATECONTEXTPROC)(HDC hDc);
typedef BOOL(APIENTRY *PFNWGLDELETECONTEXTPROC)(HGLRC oldContext);
typedef BOOL(APIENTRY *PFNWGLMAKECURRENTPROC)(HDC hDc, HGLRC newContext);
typedef HDC(APIENTRY *PFNWGLGETCURRENTDCPROC)(VOID);
typedef HGLRC(APIENTRY *PFNWGLGETCURRENTCONTEXTPROC)(VOID);

static PFNWGLGETPROCADDRESSPROC				wgl_GetProcAddress = NULL;
static PFNWGLCREATECONTEXTPROC				wgl_CreateContext = NULL;
static PFNWGLDELETECONTEXTPROC				wgl_DeleteContext = NULL;
static PFNWGLMAKECURRENTPROC				wgl_MakeCurrent = NULL;
static PFNWGLGETCURRENTDCPROC				wgl_GetCurrentDC = NULL;
static PFNWGLGETCURRENTCONTEXTPROC			wgl_GetCurrentContext = NULL;
#define wglGetProcAddress					wgl_GetProcAddress
#define wglCreateContext					wgl_CreateContext
#define wglDeleteContext					wgl_DeleteContext
#define wglMakeCurrent						wgl_MakeCurrent
#define wglGetCurrentDC						wgl_GetCurrentDC
#define wglGetCurrentContext				wgl_GetCurrentContext

static PFNWGLSWAPINTERVALEXTPROC			wgl_SwapIntervalEXT = NULL;
static PFNWGLGETEXTENSIONSSTRINGARBPROC		wgl_GetExtensionsStringARB = NULL;
static PFNWGLGETEXTENSIONSSTRINGEXTPROC		wgl_GetExtensionsStringEXT = NULL;
static PFNWGLGETPIXELFORMATATTRIBIVARBPROC	wgl_GetPixelFormatAttribivARB = NULL;
static PFNWGLCREATECONTEXTATTRIBSARBPROC	wgl_CreateContextAttribsARB = NULL;
#define wglSwapIntervalEXT					wgl_SwapIntervalEXT
#define wglGetExtensionsStringARB			wgl_GetExtensionsStringARB
#define wglGetExtensionsStringEXT			wgl_GetExtensionsStringEXT
#define wglGetPixelFormatAttribivARB		wgl_GetPixelFormatAttribivARB
#define wglCreateContextAttribsARB			wgl_CreateContextAttribsARB

#ifdef _MSC_VER
#pragma warning(disable:4191)
#endif

//
static GLADapiproc gl_load_proc(const char* name)
{
	GLADapiproc proc = (GLADapiproc)wglGetProcAddress(name);
	if (proc == NULL)
		proc = (GLADapiproc)qn_mod_func(gl_module, name);
	return proc;
}

//
static bool gl_init_api(void)
{
	static bool init = false;
	if (init)
		return true;
	gl_module = qn_mod_load("opengl32", 1);
	VAR_CHK_IF_COND(gl_module == NULL, "cannot load gl library", false);

	wgl_GetProcAddress = (PFNWGLGETPROCADDRESSPROC)qn_mod_func(gl_module, "wglGetProcAddress");
	wgl_CreateContext = (PFNWGLCREATECONTEXTPROC)qn_mod_func(gl_module, "wglCreateContext");
	wgl_DeleteContext = (PFNWGLDELETECONTEXTPROC)qn_mod_func(gl_module, "wglDeleteContext");
	wgl_MakeCurrent = (PFNWGLMAKECURRENTPROC)qn_mod_func(gl_module, "wglMakeCurrent");
	wgl_GetCurrentDC = (PFNWGLGETCURRENTDCPROC)qn_mod_func(gl_module, "wglGetCurrentDC");
	wgl_GetCurrentContext = (PFNWGLGETCURRENTCONTEXTPROC)qn_mod_func(gl_module, "wglGetCurrentContext");

	init = true;
	return true;
}

//
static bool gl_has_extension(const char* extensions, const char* name)
{
	size_t len = strlen(name);
	const char *term, *loc;
	while (true)
	{
		loc = strstr(extensions, name);
		if (loc == NULL)
			break;
		term = loc + len;
		if ((loc == extensions || *(loc - 1) == ' ') && (*term == ' ' || *term == '\0'))
			return true;
		extensions = term;
	}
	return false;
}

//
static bool gl_init_api2(GlRdh* self)
{
	HWND hwnd = (HWND)stub_system_get_dummy_window();
	HDC hdc = GetDC(hwnd);
	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), };
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	if (!SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd))
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to set pixel format");
		return false;
	}
	HGLRC hglrc = wglCreateContext(hdc);
	if (hglrc == NULL)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to create context");
		return false;
	}
	HDC wdc = wglGetCurrentDC();
	HGLRC wglrc = wglGetCurrentContext();
	if (wglMakeCurrent(hdc, hglrc) == FALSE)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to make current");
		wglMakeCurrent(wdc, wglrc);
		wglDeleteContext(hglrc);
		return false;
	}

	wgl_SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wgl_GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	wgl_GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
	wgl_GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
	wgl_CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

	const char* extensions;
	if (wglGetExtensionsStringARB != NULL)
		extensions = wglGetExtensionsStringARB(hdc);
	else if (wglGetExtensionsStringEXT != NULL)
		extensions = wglGetExtensionsStringEXT();
	else
		extensions = NULL;
	if (extensions != NULL)
	{
		self->ext.ARB_multisample = gl_has_extension(extensions, "WGL_ARB_multisample");
		self->ext.ARB_create_context = gl_has_extension(extensions, "WGL_ARB_create_context");
		self->ext.ARB_create_context_robustness = gl_has_extension(extensions, "WGL_ARB_create_context_robustness");
		self->ext.EXT_swap_control = gl_has_extension(extensions, "WGL_EXT_swap_control");
		self->ext.EXT_colorspace = gl_has_extension(extensions, "WGL_EXT_colorspace");
		self->ext.ARB_pixel_format = gl_has_extension(extensions, "WGL_ARB_pixel_format");
	}

	wglMakeCurrent(wdc, wglrc);
	wglDeleteContext(hglrc);
	return true;
}

#ifdef _MSC_VER
#pragma warning(default:4191)
#endif


//
static int gl_find_format_attr(int count, const int* attrs, const int* values, int attr)
{
	for (int i = 0; i < count; i++)
	{
		if (attrs[i] == attr)
			return values[i];
	}
	return 0;
}

//
static int gl_choose_pixel_format(GlRdh* self, const QglConfig* wanted_config, QglConfig* found_config)
{
	HDC hdc = (HDC)stub_system_get_display();
	int desc_count = DescribePixelFormat(hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), NULL);
#define ATTR_ADD(e)			QN_STMT_BEGIN{ attrs[attr_count++]=e; }QN_STMT_END
	int attrs[32], values[32], attr_count = 0;
	if (self->ext.ARB_pixel_format)
	{
		ATTR_ADD(WGL_SUPPORT_OPENGL_ARB);
		ATTR_ADD(WGL_DRAW_TO_WINDOW_ARB);
		ATTR_ADD(WGL_PIXEL_TYPE_ARB);
		ATTR_ADD(WGL_ACCELERATION_ARB);
		ATTR_ADD(WGL_RED_BITS_ARB);
		ATTR_ADD(WGL_GREEN_BITS_ARB);
		ATTR_ADD(WGL_BLUE_BITS_ARB);
		ATTR_ADD(WGL_ALPHA_BITS_ARB);
		ATTR_ADD(WGL_DEPTH_BITS_ARB);
		ATTR_ADD(WGL_STENCIL_BITS_ARB);
		ATTR_ADD(WGL_AUX_BUFFERS_ARB);
		ATTR_ADD(WGL_STEREO_ARB);
		ATTR_ADD(WGL_DOUBLE_BUFFER_ARB);
		if (self->ext.ARB_multisample)
			ATTR_ADD(WGL_SAMPLES_ARB);
		if (self->ext.EXT_colorspace)
			ATTR_ADD(WGL_COLORSPACE_EXT);
	}
#undef ATTR_ADD

	QglConfig* app_configs = qn_alloc_zero(desc_count, QglConfig);
	int app_count = 0, pixel_format = 0;

	for (int i = 0; i < desc_count; i++)
	{
		QglConfig* c = app_configs + app_count;
		pixel_format = i + 1;
		if (self->ext.ARB_pixel_format)
		{
#define FIND_ATTR(e)		gl_find_format_attr(attr_count, attrs, values, e)
			if (wglGetPixelFormatAttribivARB(hdc, pixel_format, 0, attr_count, attrs, values) == false)
			{
				qn_debug_outputs(true, VAR_CHK_NAME, "failed to get pixel format attribute");
				qn_free(app_configs);
				return 0;
			}
			if (FIND_ATTR(WGL_SUPPORT_OPENGL_ARB) == false ||
				FIND_ATTR(WGL_DRAW_TO_WINDOW_ARB) == false)
				continue;
			if (FIND_ATTR(WGL_PIXEL_TYPE_ARB) != WGL_TYPE_RGBA_ARB)
				continue;
			if (FIND_ATTR(WGL_DOUBLE_BUFFER_ARB) == 0)
				continue;
			c->red = FIND_ATTR(WGL_RED_BITS_ARB);
			c->green = FIND_ATTR(WGL_GREEN_BITS_ARB);
			c->blue = FIND_ATTR(WGL_BLUE_BITS_ARB);
			c->alpha = FIND_ATTR(WGL_ALPHA_BITS_ARB);
			c->depth = FIND_ATTR(WGL_DEPTH_BITS_ARB);
			c->stencil = FIND_ATTR(WGL_STENCIL_BITS_ARB);
			c->samples = FIND_ATTR(WGL_SAMPLES_ARB);
#undef FIND_ATTR
		}
		else
		{
			PIXELFORMATDESCRIPTOR pfd;
			if (DescribePixelFormat(hdc, pixel_format, sizeof(pfd), &pfd) == 0)
			{
				qn_debug_outputs(true, VAR_CHK_NAME, "failed to describe pixel format");
				qn_free(app_configs);
				return 0;
			}
			if ((pfd.dwFlags & PFD_SUPPORT_OPENGL) == false ||
				(pfd.dwFlags & PFD_DRAW_TO_WINDOW) == false)
				continue;
			if ((pfd.dwFlags & PFD_GENERIC_ACCELERATED) == false ||
				(pfd.dwFlags & PFD_GENERIC_FORMAT))
				continue;
			if (pfd.iPixelType != PFD_TYPE_RGBA)
				continue;
			if ((pfd.dwFlags & PFD_DOUBLEBUFFER) == 0)
				continue;
			c->red = pfd.cRedBits;
			c->green = pfd.cGreenBits;
			c->blue = pfd.cBlueBits;
			c->alpha = pfd.cAlphaBits;
			c->depth = pfd.cDepthBits;
			c->stencil = pfd.cStencilBits;
			c->samples = 0;
		}
		c->handle = (void*)(nuint)pixel_format;
		app_count++;
	}

	if (app_count == 0)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "no pixel format found");
		qn_free(app_configs);
		return 0;
	}

	const QglConfig* found = qgl_detect_config(wanted_config, app_configs, app_count);
	if (found == NULL)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "no pixel format found");
		qn_free(app_configs);
		return 0;
	}

	pixel_format = (int)(nuint)found->handle;
	memcpy(found_config, found, sizeof(QglConfig));
	qn_free(app_configs);

	return pixel_format;
}

//
static bool gl_create_context(GlRdh* self, const QglConfig* wanted_config, QglConfig* config)
{
	int pixel_format = gl_choose_pixel_format(self, wanted_config, config);
	if (pixel_format == 0)
		return false;

	HDC hdc = (HDC)stub_system_get_display();

	PIXELFORMATDESCRIPTOR pfd;;
	if (DescribePixelFormat(hdc, pixel_format, sizeof(pfd), &pfd) == FALSE)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to describe pixel format");
		return false;
	}
	if (SetPixelFormat(hdc, pixel_format, &pfd) == FALSE)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to set pixel format");
		return false;
	}

	if (self->ext.ARB_create_context)
	{
		int driver_major = qn_get_prop_int(QG_PROP_DRIVER_MAJOR, 4, 1, 4);
		int driver_minor = qn_get_prop_int(QG_PROP_DRIVER_MINOR, 0, 0, 0);

#define ATTR_ADD(e,v)		QN_STMT_BEGIN{ attrs[attr_count++] = e; attrs[attr_count++] = v; }QN_STMT_END
		int attrs[32], attr_count = 0, flags = 0;
#ifdef _DEBUG
		flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif
		ATTR_ADD(WGL_CONTEXT_MAJOR_VERSION_ARB, driver_major);
		ATTR_ADD(WGL_CONTEXT_MINOR_VERSION_ARB, driver_minor);
		//ATTR_ADD(WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB);
		ATTR_ADD(WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_ES2_PROFILE_BIT_EXT);
		if (self->ext.ARB_create_context_robustness)
		{
			ATTR_ADD(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, WGL_NO_RESET_NOTIFICATION_ARB);
			flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
		}
		if (flags)
			ATTR_ADD(WGL_CONTEXT_FLAGS_ARB, flags);
		ATTR_ADD(0, 0);

		self->context = wglCreateContextAttribsARB(hdc, NULL, attrs);
		if (self->context == NULL)
		{
			const DWORD dw = GetLastError();
			if (dw == (0xc0070000 | ERROR_INVALID_VERSION_ARB))
				qn_debug_outputf(true, VAR_CHK_NAME, "driver not support version %d.%d", driver_major, driver_minor);
			else if (dw == (0xc0070000 | ERROR_INVALID_PROFILE_ARB))
				qn_debug_outputf(true, VAR_CHK_NAME, "driver not support profile");
			else if (dw == (0xc0070000 | ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB))
				qn_debug_outputf(true, VAR_CHK_NAME, "driver not support context");
			else if (dw == (0xc0070000 | ERROR_INVALID_PARAMETER))
				qn_debug_outputf(true, VAR_CHK_NAME, "invalid parameter");
			else if (dw == (0xc0070000 | ERROR_INVALID_PIXEL_FORMAT))
				qn_debug_outputf(true, VAR_CHK_NAME, "invalid pixel format");
			else if (dw == (0xc0070000 | ERROR_NO_SYSTEM_RESOURCES))
				qn_debug_outputf(true, VAR_CHK_NAME, "no system resources");
			else
				qn_debug_outputf(true, VAR_CHK_NAME, "failed to create context: %d", dw);
			return false;
#undef ATTR_ADD
		}
	}
	else
	{
		self->context = wglCreateContext(hdc);
		if (self->context == NULL)
		{
			qn_debug_outputs(true, VAR_CHK_NAME, "failed to create context");
			return false;
		}
	}

	return true;
}

//
static bool gl_make_context_current(GlRdh* self)
{
	HDC hdc = (HDC)stub_system_get_display();
	if (wglMakeCurrent(hdc, self->context) == FALSE)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to make current");
		return false;
	}
	return true;
}

//
static void gl_swap_buffers(GlRdh* self)
{
	QN_DUMMY(self);
	HDC hdc = (HDC)stub_system_get_display();
	SwapBuffers(hdc);
}

//
static void gl_swap_interval(GlRdh* self, int interval)
{
	if (self->ext.EXT_swap_control)
		wglSwapIntervalEXT(interval);
}

//
static void gl_disp_context(GlRdh* self)
{
	wglDeleteContext(self->context);
}
#else
#error "not supported platform"
#endif // _QN_WINDOWS_

//
RdhBase* gl_allocator(QgFlag flags, QgFeature features)
{
	if (QN_TMASK(features, QGRENDERER_OPENGL) == false)
		return NULL;

	if (gl_init_api() == false)
		return NULL;

	// 설정 초기화. 프로퍼티에 있으면 가져온다
	QglConfig wanted_config;
	qgl_wanted_config(&wanted_config, 3, flags);

	// 여기에 RDH가!
	GlRdh* self = qn_alloc_zero_1(GlRdh);
	if (gl_init_api2(self) == false)
		goto pos_fail_exit;

	QglConfig config;
	if (gl_create_context(self, &wanted_config, &config) == false)
		goto pos_fail_exit;

	// 좋아, 여기서 윈도우 표시
	stub_system_actuate();

	// 커런트 만들고
	if (gl_make_context_current(self) == false)
	{
		qn_debug_outputs(true, VAR_CHK_NAME, "failed to make current");
		goto pos_fail_exit;
	}

	gladLoadGL(gl_load_proc);

	//
	gl_swap_interval(self, QN_TMASK(flags, QGFLAG_VSYNC) ? 1 : 0);

	// 정보 설정
	const QglInitialInfo initial =
	{
		"OPENGL",
		{ NULL, NULL },
		{ NULL, NULL },
		QGLOU_MAX_SIZE,
	};
	qgl_rdh_init_info(qs_cast_type(self, QglRdh), &config, &initial);
	return qs_init(self, RdhBase, &vt_gl_rdh);

pos_fail_exit:
	gl_disp_context(self);
	qn_free(self);
	return NULL;
}

//
static void gl_rdh_dispose(QsGam * g)
{
	GlRdh* self = qs_cast_type(g, GlRdh);
	if (qgl_rdh_finalize(qs_cast_type(self, QglRdh)) == false)
		return;

	gl_disp_context(self);
	rdh_internal_dispose();
}

//
static void gl_depth_range(float n, float f)
{
	glDepthRange(n, f);
}

//
static void gl_clear_depth(float d)
{
	glClearDepth(d);
}

//
static void gl_rdh_reset(void)
{
	qgl_rdh_reset();

	// 블렌드
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

	glDisable(GL_BLEND);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	//----- 리소스
#if false
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
#endif
}

// 플러시
static void gl_rdh_flush(void)
{
	qgl_rdh_flush();
	GlRdh* self = GL_RDH;
	gl_swap_buffers(self);
}

// 렌더 커밋
static bool gl_rdh_commit_render(void)
{
	const QglRender* rdr = QGL_PENDING->render.render;
	VAR_CHK_IF_NULL(rdr, false);

	if (qgl_commit_shader_layout(rdr) == false)
		return false;

	qgl_commit_depth_stencil(rdr);

	return true;
}

// 그리기
static bool gl_rdh_draw(QgTopology tpg, int vertices)
{
	if (gl_rdh_commit_render() == false)
		return false;

	GLenum gl_tpg = qgl_topology_to_enum(tpg);
	glDrawArrays(gl_tpg, 0, (GLsizei)vertices);
	return true;
}

// 그리기 인덱스
static bool gl_rdh_draw_indexed(QgTopology tpg, int indices)
{
	const QglBuffer* index = QGL_PENDING->render.index_buffer;
	VAR_CHK_IF_NULL(index, false);
	qgl_bind_index_buffer(index);

	if (gl_rdh_commit_render() == false)
		return false;

	const GLenum gl_tpg = qgl_topology_to_enum(tpg);
	const GLenum gl_stride = index->base.stride == sizeof(uint) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	glDrawElements(gl_tpg, (GLsizei)indices, gl_stride, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
// ES 버퍼

#undef VAR_CHK_NAME
#define VAR_CHK_NAME "GLBUFFER"

//
static void gl_buffer_dispose(QsGam * g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	if (self->base.mapped)
		glUnmapBuffer(self->gl_type);

	GLuint gl_handle = qs_get_desc(self, GLuint);
	glDeleteBuffers(1, &gl_handle);

	qn_free(self);
}

//
static void* gl_buffer_map(QgBuffer * g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEED2(self, gl_usage, GL_DYNAMIC_DRAW, NULL);
	qn_assert(self->lock_pointer == NULL, "버퍼가 잠겨있는데요!");

	qgl_bind_buffer(self);
	self->lock_pointer = glMapBufferRange(self->gl_type, 0, self->base.size,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	self->base.mapped = true;
	return self->lock_pointer;
}

//
static bool gl_buffer_unmap(QgBuffer * g)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	qn_assert(self->lock_pointer != NULL, "버퍼가 안 잠겼는데요!");

	qgl_bind_buffer(self);
	glUnmapBuffer(self->gl_type);

	self->lock_pointer = NULL;
	self->base.mapped = false;
	return true;
}

//
static bool gl_buffer_data(QgBuffer * g, const void* data)
{
	QglBuffer* self = qs_cast_type(g, QglBuffer);
	VAR_CHK_IF_NEED2(self, gl_usage, GL_DYNAMIC_DRAW, false);

	qgl_bind_buffer(self);
	glBufferSubData(self->gl_type, 0, self->base.size, data);

	return true;
}

//
static QgBuffer* gl_create_buffer(QgBufferType type, uint count, uint stride, const void* initial_data)
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
		.base.dispose = gl_buffer_dispose,

		.map = gl_buffer_map,
		.unmap = gl_buffer_unmap,
		.data = gl_buffer_data,
	};
	return qs_init(self, QgBuffer, &vt_es_buffer);
}

#endif // USE_GL
