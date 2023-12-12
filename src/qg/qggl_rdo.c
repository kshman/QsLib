#include "pch.h"
#include "qs_qg.h"
#include "qg_glbase.h"

//////////////////////////////////////////////////////////////////////////
// 뎁스 스텐실

static void gl_dsm_dispose(QmGam* g);

qvt_name(QmGam) vt_gl_dsm =
{
	.name = "GLDsm",
	.dispose = gl_dsm_dispose,
};

//
GlDsm* gl_dsm_allocator(QgRdh* rdh, const QgPropDepthStencil* prop)
{
	static const GLenum s_compare_op[] =
	{
		GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS
	};
	static const GLenum s_stencil_op[] =
	{
		GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_DECR, GL_INVERT, GL_INCR_WRAP, GL_DECR_WRAP,
	};

	GlDsm* self = qn_alloc_zero_1(GlDsm);
	GlDepthStencilProp* p = &self->prop;

	p->depth.enable = prop->depth_func != QGCMP_ALWAYS || prop->depth_write != 0;
	p->depth.write = prop->depth_write != 0;
	if (p->depth.enable)
		p->depth.func = s_compare_op[prop->depth_func];
	else
		p->depth.func = GL_NEVER;

	p->stencil.enable = prop->front_enable || prop->back_enable;
	p->stencil.two_side = prop->back_enable;

	if (p->stencil.enable)
	{
		p->stencil.front.func = s_compare_op[prop->front_func];
		p->stencil.front.fail = s_stencil_op[prop->front_fail];
		p->stencil.front.pass = s_stencil_op[prop->front_pass];
		p->stencil.front.z_fail = s_stencil_op[prop->front_depth];
	}
	else
	{
		p->stencil.front.func = GL_ALWAYS;
		p->stencil.front.fail = GL_KEEP;
		p->stencil.front.pass = GL_KEEP;
		p->stencil.front.z_fail = GL_KEEP;
	}

	if (p->stencil.two_side)
	{
		p->stencil.back.func = s_compare_op[prop->back_func];
		p->stencil.back.fail = s_stencil_op[prop->back_fail];
		p->stencil.back.pass = s_stencil_op[prop->back_pass];
		p->stencil.back.z_fail = s_stencil_op[prop->back_depth];
	}
	else
	{
		p->stencil.back.func = GL_ALWAYS;
		p->stencil.back.fail = GL_KEEP;
		p->stencil.back.pass = GL_KEEP;
		p->stencil.back.z_fail = GL_KEEP;
	}

	p->stencil.mask.read = prop->mask_read;
	p->stencil.mask.write = prop->mask_write;

	return qm_init(self, GlDsm, &vt_gl_dsm);
}

static void gl_dsm_dispose(QmGam* g)
{
	GlDsm* self = qm_cast(g, GlDsm);
	qn_free(self);
}


//////////////////////////////////////////////////////////////////////////
// 래스터라이저

static void gl_rsz_dispose(QmGam* g);

qvt_name(QmGam) vt_gl_rsz =
{
	.name = "GLRsz",
	.dispose = gl_rsz_dispose,
};

GlRsz* gl_rsz_allocator(QgRdh* rdh, const QgPropRasterizer* prop)
{
	GlRsz* self = qn_alloc_zero_1(GlRsz);
	GlRasterizeProp* p = &self->prop;

#if defined GL_FILL
	p->fill = prop->fill == QGFLL_SOLID ? GL_FILL : prop->fill == QGFLL_POINT : GL_POINT : GL_LINE;
#else
	p->fill = 0;
#endif
	p->cull = prop->cull == QGCUL_FRONT ? GL_FRONT : prop->cull == QGCUL_BACK ? GL_BACK : GL_NONE;
	p->depth_bias = prop->depth_bias;
	p->slope_scale = prop->slope_scale;

	return qm_init(self, GlRsz, &vt_gl_rsz);
}

static void gl_rsz_dispose(QmGam* g)
{
	GlRsz* self = qm_cast(g, GlRsz);
	qn_free(self);
}
