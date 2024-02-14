//
// qg_font.c - 글꼴
// 2024-2-14 by kim
//

#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qs_supp.h"

#define STBRP_ASSERT		qn_debug_verify
#define STBRP_SORT			qn_qsort
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"
#define STBTT_STATIC
#define STBTT_malloc(x,u)  ((void)(u),qn_alloc(x, byte))
#define STBTT_free(x,u)    ((void)(u),qn_free(x))
#define STBTT_assert		qn_debug_verify
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

//////////////////////////////////////////////////////////////////////////
// 문자

typedef struct GLYPHKEY
{
	int					code;
	int					size;
} GlyphKey;

typedef struct GLYPHVALUE
{
	int					advance;
	QmPoint				offset;
	QgTexture*			tex;
} GlyphValue;

INLINE int glyph_key_hash(const GlyphKey* key)
{
	return key->code ^ key->size;
}

INLINE bool glyph_key_eq(const GlyphKey* a, const GlyphKey* b)
{
	return a->code == b->code && a->size == b->size;
}

INLINE void glyph_value_dispose(GlyphValue* value)
{
	qn_unload(value->tex);
}

QN_DECLIMPL_MUKUM(GlyphHash, GlyphKey, GlyphValue, glyph_key_hash, glyph_key_eq, (void), glyph_value_dispose, _glyph_hash);


//////////////////////////////////////////////////////////////////////////
// 글꼴

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"Font"

typedef struct QgRealFont
{
	QN_GAM_BASE(QGFONT);

	void*				data;
	size_t				data_size;

	stbtt_fontinfo		stbtt;
	float				scale;
	int					ascent, descent, linegap;

	GlyphHash			glyphs;
} QgRealFont;

//
static void qg_font_dispose(QnGam g)
{
	QgRealFont* self = qn_cast_type(g, QgRealFont);
	_glyph_hash_dispose(&self->glyphs);
	qn_free(self->base.name);
	qn_free(self->data);
	qn_free(self);
}

//
QgFont* qg_load_font_buffer(void* data, int data_size, int font_base_size)
{
	VAR_CHK_IF_NULL(data, NULL);
	VAR_CHK_IF_ZERO(data_size, NULL);

	int offset_index = stbtt_GetFontOffsetForIndex(data, 0);
	if (offset_index < 0)
		return NULL;

	QgRealFont* self = qn_alloc_zero_1(QgRealFont);
	if (stbtt_InitFont(&self->stbtt, data, offset_index) == 0)
	{
		qn_free(self);
		return NULL;
	}

	_glyph_hash_init_fast(&self->glyphs);
	self->base.color = QMCOLOR_WHITE;

	self->data = data;
	self->data_size = data_size;

	qg_font_set_size(qn_cast_type(self, QgFont), font_base_size < 8 ? 8 : font_base_size);

	static QN_DECL_VTABLE(QNGAMBASE) vt_qg_font =
	{
		.name = VAR_CHK_NAME,
		.dispose = qg_font_dispose,
	};
	return qn_gam_init(self, vt_qg_font);
}

//
QgFont* qg_load_font(int mount, const char* filename, int font_base_size)
{
	VAR_CHK_IF_NULL(filename, NULL);

	int size;
	byte* data = qn_file_alloc(qg_get_mount(mount), filename, &size);
	qn_return_when_fail(data != NULL, NULL);

	QgFont* font = qg_load_font_buffer(data, size, font_base_size);
	font->name = qn_strdup(filename);
	return font;
}

//
void qg_font_set_size(QgFont* self, int size)
{
	QgRealFont* font = qn_cast_type(self, QgRealFont);

	if (font->base.size == size)
		return;

	font->base.size = QN_CLAMP(size, 8, 256);

	font->scale = stbtt_ScaleForPixelHeight(&font->stbtt, (float)font->base.size);
	stbtt_GetFontVMetrics(&font->stbtt, &font->ascent, &font->descent, &font->linegap);
}

//
static QgImage* _font_generate_image(byte* bitmap, int width, int height)
{
	QgImage* img = qg_create_image(QGCF_A8L8, width, height);
	for (int y = 0; y < height; ++y)
	{
		byte* dst = img->data + y * img->pitch;
		byte* src = bitmap + y * width;
		for (int x = 0; x < width; ++x)
		{
			*dst++ = 255;
			*dst++ = *src++;
		}
	}
	qn_free(bitmap);
	return img;
}

//
static GlyphValue* _font_get_glyph(QgRealFont* self, int code)
{
	GlyphKey key = { code, self->base.size };
	GlyphValue* value = _glyph_hash_get_ptr(&self->glyphs, &key);
	if (value != NULL)
		return value;

	int index, x0, y0, x1, y1, advance, lsb;

	index = stbtt_FindGlyphIndex(&self->stbtt, code);
	if (index == 0)
		return NULL;
	byte* bitmap = stbtt_GetGlyphBitmapSubpixel(&self->stbtt, self->scale, self->scale, 0.0f, 0.0f, index, &x0, &y0, &x1, &y1);
	if (bitmap == NULL)
		return NULL;
	stbtt_GetGlyphHMetrics(&self->stbtt, index, &advance, &lsb);

	//QgImage* img = qg_create_image_buffer(QGCF_R8, x0, y0, bitmap);
	QgImage* img = _font_generate_image(bitmap, x0, y0);
	QgTexture* tex = qg_create_texture(NULL, img, QGTEXF_DISCARD_IMAGE | QGTEXF_CLAMP);
	if (tex == NULL)
	{
		qn_unload(img);
		return NULL;
	}

	value = _glyph_hash_set_key_ptr(&self->glyphs, &key);
	value->advance = (int)((float)advance * self->scale);
	value->offset = qm_point(x1, y1 + (int)((float)self->ascent * self->scale));
	value->tex = tex;

	//qn_mesgf(false, VAR_CHK_NAME, "%d: %d (%d, %d)", code, value->advance, value->offset.X, value->offset.Y);

	return value;
}

//
int qg_font_draw(QgFont* self, int x, int y, const char* text)
{
	VAR_CHK_IF_NULL(text, 0);

	QgRealFont* font = qn_cast_type(self, QgRealFont);
	QmPoint pt = qm_point(x, y);
	int maxx = 0;

	while (*text)
	{
		int len;
		int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case '\n':
				pt.X = x;
				pt.Y += font->base.size;
				break;
			case '\t':
				pt.X += font->base.size * 4;
				break;
			case ' ':
				pt.X += font->base.size / 2;
				break;
			default:
			{
				GlyphValue* value = _font_get_glyph(font, code);
				if (value != NULL)
				{
					QmRect rect = qm_rect_size(pt.X + value->offset.X, pt.Y + value->offset.Y, value->tex->width, value->tex->height);
					qg_draw_sprite(&rect, value->tex, &self->color, NULL);
					//qg_draw_texture(value->tex, pt.X + value->offset.x, pt.Y - value->offset.y, &rect);
					pt.X += value->advance;
				}
			} break;
		}

		if (pt.X > maxx)
			maxx = pt.X;
	}

	return maxx;
}

//
int qg_font_draw_format(QgFont* self, int x, int y, const char* fmt, ...)
{
	VAR_CHK_IF_NULL(fmt, 0);
	va_list va;
	va_start(va, fmt);
	char buffer[1024];
	int len = qn_vsnprintf(buffer, QN_COUNTOF(buffer), fmt, va);
	va_end(va);
	if (len <= 0)
		return 0;
	return qg_font_draw(self, x, y, buffer);
}
