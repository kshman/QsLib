//
// qg_image.c - 이미지, 글꼴
// 2024-1-30 by kim
//

#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qs_supp.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 6011)
#pragma warning(disable: 6260)
#pragma warning(disable: 6385)
#endif

#define STBI_MALLOC(x) qn_alloc(x, byte)
#define STBI_REALLOC(x, y) qn_realloc(x, y, byte)
#define STBI_FREE(x) qn_free(x)
#define STBI_ASSERT qn_debug_verify
#define STBI_NO_GIF
#define STBI_NO_PIC
#define STBI_NO_HDR
#define STBI_NO_PNM
#define STBI_NO_STDIO
#define STBI_NO_THREAD_LOCALS
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////
// 이미지 헤더

// DDS 픽셀 포맷
typedef struct DDS_PIXEL_FORMAT
{
	uint dwSize;
	uint dwFlags;
	uint dwFourCC;
	uint dwRGBBitCount;
	uint dwRBitMask;
	uint dwGBitMask;
	uint dwBBitMask;
	uint dwABitMask;
} DdsPixelFormat;

// DDS 헤더
typedef struct DDS_HEADER
{
	uint magic;
	uint dwSize;
	uint dwFlags;
	uint dwHeight;
	uint dwWidth;
	uint dwPitchOrLinearSize;
	uint dwDepth;
	uint dwMipMapCount;
	uint dwReserved1[11];
	DdsPixelFormat ddsFormat;
	uint dwCaps1;
	uint dwCaps2;
	uint dwReserved2[3];
} DdsHeader;

// PKM 헤더
typedef struct PKM_HEADER
{
	uint magic;
	ushort version;
	ushort type;
	ushort width;
	ushort height;
	ushort width_orig;
	ushort height_orig;
} PkmHeader;

// KTX 헤더
typedef struct KTX_HEADER
{
	byte identifier[12];
	uint endianness;
	uint gltype;
	uint gltypesize;
	uint glformat;
	uint glinternalformat;
	uint glbaseinternalformat;
	uint pixelwidth;
	uint pixelheight;
	uint pixeldepth;
	uint arrayelements;
	uint faces;
	uint miplevels;
	uint keypairbytes;
} KtxHeader;

// ASTC 헤더
typedef struct ASTC_HEADER
{
	byte magic[4];
	byte blockdim_x;
	byte blockdim_y;
	byte blockdim_z;
	byte xsize[3];
	byte ysize[3];
	byte zsize[3];
} AstcHeader;

// HXN 헤더
typedef struct HXN_HEADER
{
	uint magic;
	ushort type;
	ushort glyph_size;
	ushort width;
	ushort height;
	uint image_compressed;
	uint atlas;
	uint atlas_compressed;
	byte desc[64];
} HxnHeader;

// HXN 아틀라스
typedef struct HXN_ATLAS
{
	int code;
	ushort x;
	ushort y;
	byte width;
	byte height;
	char x_offset;
	char y_offset;
	byte x_advance;
	byte page;
	byte unused[2];
} HxnAtlas;


//////////////////////////////////////////////////////////////////////////
// 블릿

INLINE void QM_VECTORCALL pixel_color_to_r8_g8_b8(byte* v, const QMVEC color)
{
	v[0] = (byte)(qm_vec_get_x(color) * 255);
	v[1] = (byte)(qm_vec_get_y(color) * 255);
	v[2] = (byte)(qm_vec_get_z(color) * 255);
}

INLINE void QM_VECTORCALL pixel_color_to_r16_g16_b16(ushort* v, const QMVEC color)
{
	v[0] = (ushort)(qm_vec_get_x(color) * 65535);
	v[1] = (ushort)(qm_vec_get_y(color) * 65535);
	v[2] = (ushort)(qm_vec_get_z(color) * 65535);
}

INLINE void QM_VECTORCALL pixel_color_to_r16_g16_b16_a16(ullong* v, const QMVEC color)
{
	const ullong r = (ullong)(qm_vec_get_x(color) * 65535) << 32;
	const ullong g = (ullong)(qm_vec_get_y(color) * 65535) << 16;
	const ullong b = (ullong)(qm_vec_get_z(color) * 65535);
	const ullong a = (ullong)(qm_vec_get_w(color) * 65535) << 48;
	*v = r | g | b | a;
}

//
static QgPropPixel* qg_get_prop_pixel(QgClrFmt format)
{
	static QgPropPixel pixel_table[QGCF_MAX_VALUE] =
	{
		{QGCF_UNKNOWN, 0},

		{QGCF_R32G32B32A32F, 128, 16},
		{QGCF_R32G32B32F, 96, 12},
		{/*QGCF_R32G32F*/QGCF_UNKNOWN, 64, 8},
		{QGCF_R32F, 32, 4},

		{/*QGCF_R32G32B32A32*/QGCF_UNKNOWN, 128, 16},
		{/*QGCF_R32G32B32*/QGCF_UNKNOWN, 96, 12},
		{/*QGCF_R32G32*/QGCF_UNKNOWN, 64, 8},
		{/*QGCF_R32*/QGCF_UNKNOWN, 32, 4},

		{QGCF_R16G16B16A16F, 64, 8},
		{/*QGCF_R16G16F*/QGCF_UNKNOWN, 32, 4},
		{QGCF_R16F, 16, 2},
		{QGCF_R11G11B10F, 32, 4},

		{QGCF_R16G16B16A16, 64, 8},
		{/*QGCF_R16G16*/QGCF_UNKNOWN, 32, 4},
		{QGCF_R16, 16, 2},
		{QGCF_R10G10B10A2, 32, 4},

		{QGCF_R8G8B8A8, 32, 4},
		{QGCF_R8G8B8, 24, 3},
		{/*QGCF_R8G8*/QGCF_UNKNOWN, 16, 2},
		{QGCF_R8, 8, 1},
		{QGCF_A8, 8, 1},
		{QGCF_L8, 8, 1},
		{QGCF_A8L8, 16, 2},

		{QGCF_R5G6B5, 16, 2},
		{QGCF_R5G5B5A1, 16, 2},
		{QGCF_R4G4B4A4, 16, 2},

		{/*QGCF_D32F*/QGCF_UNKNOWN, 32, 4},
		{/*QGCF_D24S8*/QGCF_UNKNOWN, 32, 4},

		{QGCF_DXT1, 4, 8},
		{QGCF_DXT3, 8, 16},
		{QGCF_DXT5, 8, 16},
		{QGCF_EXT1, 4, 16},
		{QGCF_EXT2, 4, 16},
		{QGCF_EXT2_EAC, 8, 16},
		{QGCF_ASTC4, 8, 16},
		{QGCF_ASTC8, 2, 16},
	};
	if ((size_t)format < QN_COUNTOF(pixel_table))
	{
		QgPropPixel* prop = &pixel_table[format];
		if (prop->format != QGCF_UNKNOWN)
			return prop;
	}
	return NULL;
}

// DDS (ㅇㅋ:DXT3, DXT5, 확인못함:DXT1)
static bool image_loader_dds(const byte* data, uint size, QgImage* image)
{
	// 헤더 읽기
	if (size < sizeof(DdsHeader))
		return false;
	const DdsHeader* header = (const DdsHeader*)data;
	qn_return_when_fail(header->magic == 0x20534444, false);

	// 픽셀 포맷
	const QgPropPixel* prop = qg_get_prop_pixel(
		header->ddsFormat.dwFourCC == 0x31545844 ? QGCF_DXT1 :
		header->ddsFormat.dwFourCC == 0x33545844 ? QGCF_DXT3 :
		header->ddsFormat.dwFourCC == 0x35545844 ? QGCF_DXT5 : QGCF_UNKNOWN);
	qn_return_when_fail(prop != NULL, false);

	// 진행
	data += sizeof(DdsHeader);
	size -= sizeof(DdsHeader);
	// 크기
	const int w = (int)header->dwWidth;
	const int h = (int)header->dwHeight;
	const int m = (int)header->dwMipMapCount;
	image->width = w;
	image->height = h;
	image->pitch = 0;
	image->mipmaps = m;
	image->prop = *prop;

	// 그냥 복사해도 됨
	byte* buffer = qn_alloc(size, byte);
	memcpy(buffer, data, size);
	qn_set_gam_desc(image, buffer);

	return true;
}

// ETC (PKM) => BIG ENDIAN을 쓴다!
static bool image_loader_etc_pkm(const byte* data, uint size, QgImage* image)
{
	// 헤더 읽기
	if (size < sizeof(PkmHeader))
		return false;
	const PkmHeader* header = (const PkmHeader*)data;
	if (header->magic != 0x204D4B50)
		return false;

	// 픽셀 포맷
	const ushort type = (ushort)(((header->type & 0xFF) << 8) | ((header->type & 0xFF00) >> 8));
	const QgPropPixel* prop = qg_get_prop_pixel(
		type == 0x00 ? QGCF_EXT1 :
		type == 0x01 ? QGCF_EXT2 :
		type == 0x03 ? QGCF_EXT2_EAC : QGCF_UNKNOWN);
	qn_return_when_fail(prop != NULL, false);

	// 진행
	data += sizeof(PkmHeader);
	// 크기
	const int w = ((header->width & 0xFF) << 8) | ((header->width & 0xFF00) >> 8);
	const int h = ((header->height & 0xFF) << 8) | ((header->height & 0xFF00) >> 8);
	image->width = w;
	image->height = h;
	image->pitch = 0;
	image->mipmaps = 1;
	image->prop = *prop;

	// 읽기
	const size_t size_total = (size_t)(w * h * prop->bpp / 8);
	byte* buffer = qn_alloc(size_total, byte);
	memcpy(buffer, data, size_total);
	qn_set_gam_desc(image, buffer);

	return true;
}

// ETC (KTX) 그 KTX가 아니다
static bool image_loader_etc_ktx(const byte* data, uint size, QgImage* image)
{
	// 헤더 읽기
	if (size < sizeof(KtxHeader))
		return false;
	const KtxHeader* header = (const KtxHeader*)data;
	if (header->endianness != 0x04030201)
		return false;

	// 픽셀 포맷
	const QgPropPixel* prop = qg_get_prop_pixel(
		header->glinternalformat == 0x8D64 ? QGCF_EXT1 :
		header->glinternalformat == 0x9278 ? QGCF_EXT2 :
		header->glinternalformat == 0x9274 ? QGCF_EXT2_EAC : QGCF_UNKNOWN);
	qn_return_when_fail(prop != NULL, false);

	// 진행
	data += sizeof(KtxHeader) + header->keypairbytes;
	// 크기
	const int w = (int)header->pixelwidth;
	const int h = (int)header->pixelheight;
	const int m = (int)header->miplevels;
	image->width = w;
	image->height = h;
	image->pitch = 0;
	image->mipmaps = m;
	image->prop = *prop;

	// 읽기
	const size_t size_total = *(const int*)data;
	data += sizeof(int);
	byte* buffer = qn_alloc(size_total, byte);
	memcpy(buffer, data, size_total);
	qn_set_gam_desc(image, buffer);

	return true;
}

// ASTC
static bool image_loader_astc(const byte* data, uint size, QgImage* image)
{
	// 헤더 읽기
	if (size < sizeof(AstcHeader))
		return false;
	const AstcHeader* header = (const AstcHeader*)data;
	if (header->magic[0] != 0x13 || header->magic[1] != 0xAB || header->magic[2] != 0xA1 || header->magic[3] != 0x5C)
		return false;

	// 픽셀 포맷
	const int bpp = 128 / (header->blockdim_x * header->blockdim_y);
	qn_return_when_fail(bpp == 8 || bpp == 2, false);
	const QgPropPixel* prop = qg_get_prop_pixel(
		header->blockdim_x == 8 ? QGCF_ASTC4 :
		header->blockdim_x == 2 ? QGCF_ASTC8 : QGCF_UNKNOWN);
	qn_return_when_fail(prop != NULL, false);

	// 진행
	data += sizeof(AstcHeader);
	// 크기
	const int w = header->xsize[0] | header->xsize[1] << 8 | header->xsize[2] << 16;
	const int h = header->ysize[0] | header->ysize[1] << 8 | header->ysize[2] << 16;
	image->width = w;
	image->height = h;
	image->pitch = 0;
	image->mipmaps = 1;
	image->prop = *prop;

	// 읽기
	const size_t size_total = (size_t)(w * h * bpp / 8);
	byte* buffer = qn_alloc(size_total, byte);
	memcpy(buffer, data, size_total);
	qn_set_gam_desc(image, buffer);

	return true;
}

// HXN
static bool image_loader_hxn(const byte* data, uint size, QgImage* image, int* atlas, HxnAtlas** atlas_data)
{
	if (size < sizeof(HxnHeader))
		return false;
	const HxnHeader* header = (const HxnHeader*)data;
	if (header->magic != 0x004E5848)
		return false;

	const QgPropPixel* prop = qg_get_prop_pixel(
		header->type == 0x8000 ? QGCF_A8 :
		header->type == 0x8888 ? QGCF_R8G8B8A8 :
		header->type == 0x0888 ? QGCF_R8G8B8 : QGCF_UNKNOWN);
	qn_return_when_fail(prop != NULL, false);

	image->extra = header->glyph_size;
	image->width = header->width;
	image->height = header->height;
	image->pitch = header->width * prop->tbp;
	image->mipmaps = 1;
	image->prop = *prop;

	data += sizeof(HxnHeader);
	const size_t size_total = (size_t)(header->width * header->height * prop->tbp);
	byte* buffer;
	if (header->image_compressed > 0)
	{
		buffer = qn_memzucp_s(data, header->image_compressed, size_total);
		data += header->image_compressed;
	}
	else
	{
		buffer = qn_alloc(size_total, byte);
		memcpy(buffer, data, size_total);
		data += size_total;
	}
	qn_set_gam_desc(image, buffer);

	if (header->atlas != 0 && atlas != NULL && atlas_data != NULL)
	{
		const size_t atlas_total = header->atlas * sizeof(HxnAtlas);
		*atlas = header->atlas;
		if (header->atlas_compressed > 0)
			buffer = qn_memzucp_s(data, header->atlas_compressed, atlas_total);
		else
		{
			buffer = qn_alloc(atlas_total, byte);
			memcpy(buffer, data, atlas_total);
		}
		*atlas_data = (HxnAtlas*)buffer;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// 이미지 기본

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"Image"

//
static void qg_image_dispose(QnGam gam)
{
	QgImage* self = qn_cast_type(gam, QgImage);
	qn_free(qn_get_gam_pointer(self));
	qn_free(self);
}

// 퓨어 이미지
static QgImage* qg_image(void)
{
	QgImage* self = qn_alloc_1(QgImage);
	static QN_DECL_VTABLE(QNGAMBASE) vt_qg_image =
	{
		.name = VAR_CHK_NAME,
		.dispose = qg_image_dispose,
	};
	return qn_gam_init(self, vt_qg_image);
}

//
QgImage* qg_create_image(QgClrFmt fmt, int width, int height)
{
	const QgPropPixel* prop = qg_get_prop_pixel(fmt);
	VAR_CHK_IF_NULL(prop, NULL);
	VAR_CHK_IF_NEQ2(prop, format, QGCF_UNKNOWN, NULL);
	QgImage* self = qg_image();
	self->prop = *prop;
	self->width = width;
	self->height = height;
	self->pitch = width * prop->tbp;
	self->mipmaps = 1;
	const size_t size = (uint)qg_calc_image_block_size(prop, width, height);
	byte* data = qn_alloc(size, byte);
	qn_set_gam_desc(self, data);
	return self;
}

//
QgImage* qg_create_image_buffer(QgClrFmt fmt, int width, int height, void* data)
{
	QgImage* self = qg_image();
	self->prop = *qg_get_prop_pixel(fmt);
	self->width = width;
	self->height = height;
	self->pitch = width * self->prop.tbp;
	self->mipmaps = 1;
	qn_set_gam_desc(self, data);
	return self;
}

//
QgImage* qg_create_image_filled(int width, int height, const QMVEC* color)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	QmKolor* data = qn_get_gam_pointer(self);
	QmKolor c;
	qm_vec_to_kolor(*color, &c);
	for (int i = 0; i < width * height; i++)
		data[i] = c;
	return self;
}

//
QgImage* qg_create_image_gradient_linear(int width, int height, const QMVEC* begin, const QMVEC* end, float direction)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	QmKolor* data = qn_get_gam_pointer(self);
	float sn, cs;
	qm_sincosf(direction, &sn, &cs);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const float f = qm_clampf(((float)x * cs + (float)y * sn) / ((float)width * cs + (float)height * cs), 0.0f, 1.0f);
			const QMVEC c = qm_lerp(*begin, *end, f);
			qm_vec_to_kolor(c, &data[y * width + x]);
		}
	}
	return self;
}

//
QgImage* qg_create_image_gradient_radial(int width, int height, const QMVEC* inner, const QMVEC* outer, float density)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	QmKolor* data = qn_get_gam_pointer(self);
	const float r = (float)QN_MIN(width, height) * 0.5f;
	const float cx = (float)width * 0.5f;
	const float cy = (float)height * 0.5f;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const float h = hypotf((float)x - cx, (float)y - cy);
			const float m = qm_clampf((h - r * density) / (r * (1.0f - density)), 0.0f, 1.0f);
			const QMVEC c = qm_lerp(*outer, *inner, m);
			qm_vec_to_kolor(c, &data[y * width + x]);
		}
	}
	return self;
}

//
QgImage* qg_create_image_check_pattern(int width, int height, const QMVEC* odd, const QMVEC* even, int sx, int sy)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	QmKolor* data = qn_get_gam_pointer(self);
	QmKolor oc, ec;
	qm_vec_to_kolor(*odd, &oc);
	qm_vec_to_kolor(*even, &ec);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < height; x++)
			data[y * width + x] = ((x / sx + y / sy) % 2 == 0) ? oc : ec;
	}
	return self;
}

//
QgImage* qg_load_image_buffer(const void* data, int size)
{
	VAR_CHK_IF_NULL(data, NULL);
	VAR_CHK_IF_MIN(size, 0, NULL);

	int comp = 0;
	QgImage* self = qg_image();
	byte* buffer = stbi_load_from_memory(data, size, &self->width, &self->height, &comp, 0);
	if (buffer != NULL)
	{
		QgClrFmt fmt;
		const uint hdr = *(const uint*)data; // 진짜 HDR 헤더임 "#?RGBE" 또는 "#?RADIANCE"
		if (hdr == 0x41523F23 || hdr == 0x47523F23)
		{
			fmt =
				comp == 4 ? QGCF_R32G32B32A32F :
				comp == 3 ? QGCF_R32G32B32F :
				comp == 1 ? QGCF_R32F : QGCF_UNKNOWN;
		}
		else
		{
			fmt =
				comp == 4 ? QGCF_R8G8B8A8 :
				comp == 3 ? QGCF_R8G8B8 :
				comp == 1 ? QGCF_L8 : QGCF_UNKNOWN;
		}

		const QgPropPixel* prop = qg_get_prop_pixel(fmt);
		if (prop == NULL)
			goto pos_exit;

		self->pitch = self->width * prop->tbp;
		self->mipmaps = 1;
		self->prop = *prop;
		qn_set_gam_desc(self, buffer);
		return self;
	}
	if (image_loader_dds(data, size, self) ||
		image_loader_etc_pkm(data, size, self) ||
		image_loader_etc_ktx(data, size, self) ||
		image_loader_astc(data, size, self) ||
		image_loader_hxn(data, size, self, NULL, NULL))
		return self;

pos_exit:
	qg_image_dispose(qn_cast_type(self, QnGamBase));
	return NULL;
}

//
static QgImage* _load_image_hxn(const void* data, int size, int* atlas, HxnAtlas** atlas_data)
{
	QgImage* self = qg_image();
	if (image_loader_hxn(data, size, self, atlas, atlas_data))
		return self;
	qg_image_dispose(qn_cast_type(self, QnGamBase));
	return NULL;
}

//
QgImage* qg_load_image(int mount, const char* filename)
{
	int size;
	byte* data = qn_file_alloc(qg_get_mount(mount), filename, &size);
	qn_return_when_fail(data != NULL, NULL);

	QgImage* self = qg_load_image_buffer(data, size);
	qn_free(data);
	return self;
}

//
bool qg_image_set_pixel(const QgImage* self, int x, int y, const QMVEC* color)
{
	qn_return_when_fail((size_t)x < (size_t)self->width && (size_t)y < (size_t)self->height, false);
	byte* ptr = (byte*)qn_get_gam_pointer(self) + (ptrdiff_t)((y * self->pitch) + (x * self->prop.tbp));
	switch (self->prop.format)
	{
		case QGCF_R32G32B32A32F:
			((QMVEC*)ptr)[0] = *color;
			break;
		case QGCF_R32G32B32F:
			qm_vec_to_float3(*color, (QmFloat3*)ptr);
			break;
		case QGCF_R32F:
			((float*)ptr)[0] = qm_vec_get_x(*color);
			break;
		case QGCF_R16G16B16A16F:
			qm_vec_to_half4(*color, (QmHalf4*)ptr);
			break;
		case QGCF_R16F:
			((half_t*)ptr)[0] = qm_f2hf(qm_vec_get_x(*color));
			break;
		case QGCF_R11G11B10F:
			qm_vec_to_f111110(*color, (QmF111110*)ptr);
			break;
		case QGCF_R16G16B16A16:
			pixel_color_to_r16_g16_b16_a16((ullong*)ptr, *color);
			break;
		case QGCF_R16:
			((ushort*)ptr)[0] = (ushort)(qm_vec_get_x(*color) * 0xFFFF);
			break;
		case QGCF_R10G10B10A2:
			qm_vec_to_u1010102(*color, (QmU1010102*)ptr);
			break;
		case QGCF_R8G8B8A8:
			qm_vec_to_kolor(*color, (QmKolor*)ptr);
			break;
		case QGCF_R8G8B8:
			pixel_color_to_r8_g8_b8(ptr, *color);
			break;
		case QGCF_R8:
			*ptr = (byte)(qm_vec_get_x(*color) * 0xFF);
			break;
		case QGCF_A8:
			*ptr = (byte)(qm_vec_get_w(*color) * 0xFF);
			break;
		case QGCF_L8:
			*ptr = (byte)(qm_vec_get_x(*color) * 0xFF);
			break;
		case QGCF_A8L8:
			((ushort*)ptr)[0] = (ushort)
				(((byte)(qm_vec_get_w(*color) * 0xFF) << 8) |
					(byte)(qm_vec_get_x(*color) * 0xFF));
			break;
		case QGCF_R5G6B5:
			qm_vec_to_u565(*color, (QmU565*)ptr);
			break;
		case QGCF_R5G5B5A1:
			qm_vec_to_u5551(*color, (QmU5551*)ptr);
			break;
		case QGCF_R4G4B4A4:
			qm_vec_to_u4444(*color, (QmU4444*)ptr);
			break;
		default:
			return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
// 기본 글꼴 데이터

static const uint default_font[] = {
	0x004E5848, 0x00108000, 0x02000200, 0x0000133D, 0x00000000, 0x00000000, 0x6D726574, 0x73756E69,
	0x6B202B20, 0x74736961, 0x6E6F6620, 0x00000074, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40000000, 0x928B9DEC, 0x450E2AEB,
	0xDE9FFFF9, 0x4F7D3553, 0x10F108C7, 0xD67704E0, 0x439B99AA, 0x1B31B627, 0xA4048C61, 0xFFA19004,
	0x43FF2FFF, 0x4CF087ED, 0xFD29AF81, 0xEFF7FFFC, 0xADFFF4FF, 0xEEFD7A07, 0x79D95FF7, 0x7E1D5B34,
	0x84FCA3AE, 0xBFAFD2FF, 0x9745E4FF, 0xED25D22F, 0xE2E87EFD, 0xD274F397, 0x7B3D72E9, 0x7BA2DFDD,
	0xCA2A5E5D, 0xD74655F0, 0x5F5C0FDF, 0x8FCDD7BF, 0x5153E7B7, 0x7D54CA4E, 0x52B94FE5, 0xE853FFCA,
	0xDCBCBAFF, 0xC67E517A, 0x3F5CBFF4, 0x25964ACB, 0xAFCDB515, 0x78D7EAAB, 0xDAADF7E5, 0xA832B279,
	0xF29CADDD, 0x9731F94B, 0x5CEA485B, 0x791FBFC7, 0x9FEAABD7, 0xEF766C17, 0xDBB55F32, 0xD9FF9973,
	0x6987966B, 0x995FFCCA, 0xEBAFE9AA, 0xFAC29535, 0x57B38FBB, 0xE5F6B9B9, 0xCFF4C67F, 0xFB974AAD,
	0xA7AF5C08, 0xDD4757BF, 0x1D295B0A, 0xF697FF2F, 0xD53CB1CE, 0x7EA607BA, 0xA97F5868, 0x15655392,
	0xBAE5B5D1, 0xE5E7F90E, 0xAFEB91F9, 0x6F55BD01, 0x2BFAEAED, 0xBFEAAD15, 0xFACDED67, 0x9FEBAD2B,
	0xFE437F19, 0x58D2FCF2, 0x2EA97073, 0xBA143B72, 0xCFB25836, 0xA87FD347, 0x7B7AAFEA, 0x6BED7BF0,
	0x97CF5FB9, 0xDD67BFF2, 0x4A4EDC93, 0xFD7C95FD, 0x6ADAA5AF, 0x2CCCA5F7, 0xE2A7FAAA, 0xE696EEB9,
	0xF297FEA4, 0xF572F42E, 0x6549FACF, 0x5FE8E27B, 0xED9450ED, 0xA247DCBA, 0x2DF768CD, 0x2F4CD645,
	0xFD80A6FD, 0xEBEFBF67, 0x28156C9F, 0x372D9BEB, 0x4D6566EF, 0x529BDAD3, 0x7BFAFF44, 0xCB5EA5C1,
	0xFB06DA16, 0x15463ACF, 0x37A99583, 0x627BFD2D, 0xD17C8ADB, 0xBFEDB35F, 0xD5FFED22, 0xF1D75AA6,
	0xFD0D6C7A, 0xCABE2955, 0xFF6EB4D3, 0xDF8071B5, 0xFA2348EB, 0xE5FF3E8F, 0xF7F3A2DF, 0x0D2BFD8A,
	0xB1FC5FE9, 0x6D2A41E8, 0xD6A9FFEC, 0xAF391BFF, 0xF45653BC, 0x6A977DB7, 0x7FAA3FB3, 0x579FFFA7,
	0xD51BDAB9, 0x51ABF970, 0x0684BDC5, 0x463CAFF4, 0xA2D5FD5A, 0xFF7D5DA9, 0x6FFDE553, 0xD7157F40,
	0x793B1BF3, 0xFD0D93EF, 0x2E41AB53, 0xCF2A7FC6, 0x24CFD7FF, 0xA69544B7, 0x5807CFF4, 0xDB1D719C,
	0x78CDB21F, 0xEDD42C3E, 0xFA552AF8, 0x5BF9F62B, 0xE3B1FF1B, 0x47FC7647, 0xF7E31C66, 0xFE32AA3D,
	0x7B9F38D3, 0x5989E69F, 0x7FC66FE8, 0x63BA37AE, 0x87DDF0BC, 0x7FBAEE3F, 0xF38CCF2E, 0x802F49FF,
	0x0022DD34, 0x09699FC0, 0xC02A0228, 0xA8F73D17, 0x50CAE1F2, 0x86360D0C, 0x6B178AFF, 0xDA7AAA7D,
	0xC23327F7, 0xB7FA7302, 0xFE857FDD, 0xBBFA53E9, 0x51DA40E3, 0xAFBFD123, 0x43FAA1CE, 0x953085EB,
	0xC6C95FE3, 0x75E3793B, 0xF9EF3B79, 0x7891A563, 0xD0419FA7, 0x3FC28BEC, 0x1F59F3F6, 0xB1FC2D4F,
	0x99311FE8, 0x54A478C9, 0x9EB27F8B, 0x459FAE8D, 0xAA31FB7E, 0x7D9D997F, 0xB79FD12D, 0x33FF9EFF,
	0x8CD2BFA6, 0x7F5469FE, 0x1D2FF115, 0xF7F22847, 0xFD2D969E, 0x46A9F8FD, 0xD9EF83FC, 0xAA7B8E1C,
	0xE2D4BFC4, 0x5FF17A1F, 0x7FA8BFE8, 0xDBB71EDB, 0x37B4AA7F, 0x3287F8B5, 0x97FD15B8, 0xFE92D7F5,
	0x9EFFCF73, 0xCA08D7FE, 0x7FA27C81, 0xE9DFF9F0, 0x5501E7FF, 0xE487FCEE, 0xBD0BC5F8, 0xECFFFDEA,
	0x26F1AFF4, 0xCF21A2F3, 0x8BC79E5F, 0x703FE379, 0x744160F3, 0x865B5578, 0xF470B0FA, 0x8C4FFF47,
	0x4F7F7917, 0x709FD09F, 0x80000006, 0x0313DF9B, 0xA7F683D1, 0x5D3F78F6, 0x3DEC7E3F, 0x7B20F876,
	0xB06BDE2B, 0xFBF32749, 0xD8279758, 0x0FEF5149, 0x794CF4D1, 0x75AB7AF8, 0x1A2640C0, 0x657FAD39,
	0x374EE2F3, 0x3A4766CC, 0x135337BF, 0xCBC4535B, 0xE9C37DB4, 0xEA2C3B44, 0x437CFDB5, 0x489E1BB6,
	0x698BBD15, 0x5AC13DFD, 0x60AEC32E, 0x7CFFA6DF, 0xFFAF3523, 0x0F1382D0, 0x565E5FD1, 0xD0FFA9DF,
	0x1EDAB3F3, 0xA57F427F, 0xDCDDFEA8, 0x6F7FD46D, 0x7F5F6840, 0x7B0ED7AB, 0x08FDDCDD, 0x96D36BDB,
	0xB685B86F, 0xDFF486BD, 0xB5DC0F1C, 0x570EC1D7, 0x86753BB3, 0x77F4B7FE, 0xC1FFE8B7, 0x063E9E2C,
	0xC3DFC7A9, 0x7FDBF1D9, 0xF4257F40, 0x91FD62FF, 0xD8F0C77F, 0xE427F353, 0x00000007, 0x03AE3800,
	0x7AEF4D5F, 0x9FF49841, 0xC627FAAF, 0xF316982E, 0xD27E4C97, 0x58DDFCE6, 0xFA4BFE7E, 0xFEBAFE7C,
	0x693FA088, 0xCFDA7FB7, 0x6BF9029F, 0xA4E856AF, 0xECE178D5, 0x695C55EA, 0xD4447C73, 0x57D7505F,
	0xADDAF45B, 0x01CFAA7F, 0xB36BD29A, 0x82BD5DFE, 0x3F6F1AF6, 0x2C68F447, 0xFEBB72D0, 0x7BE9FEEB,
	0xFED1DFF5, 0x4DD7D90F, 0xF9C8E7ED, 0xBCFFF456, 0xBFADBA7F, 0xFFA82BD5, 0xF5B65CB6, 0xB1BF2E99,
	0x67277FE7, 0xE783FD2D, 0xBA7F80FF, 0x4E74D834, 0xFF9FC6FE, 0x66BF2B1F, 0xEBFA74FB, 0xBFDC3A4D,
	0x7BF280F3, 0xFD048BB2, 0xBFFD1FD1, 0xF867FF58, 0x03F913CF, 0x00000000, 0xFBC6BF80, 0x57B1FBFD,
	0x477F49ED, 0xB9D20BCF, 0x93F922A0, 0x5F30DD33, 0x55A4D11E, 0x5756B7FA, 0x7817180D, 0xFAA93F8F,
	0xB7CFFBAB, 0x65547FEC, 0xE96AAF7E, 0xF9E55E96, 0x038FFD96, 0xFC6B1CBB, 0x65EB8FDB, 0xF78BD97F,
	0xDA2BE730, 0x75FCE5A7, 0xD7F32FF4, 0xDFCBDFD4, 0xF5F7FD9A, 0xF5FE7B57, 0xE63BFE9A, 0x53E2479F,
	0x6AB7CF56, 0x14FE5EFF, 0xF6FDDFC9, 0x3764766F, 0xFB949C9B, 0xFF416AB5, 0x6EC4FED5, 0xBF9FDFFB,
	0x3FBFF6BD, 0x388CA104, 0xFC5DEAB8, 0x01FCB7A3, 0xE527F3FE, 0x17B87CAF, 0xEE0DB7C2, 0xD3A20EC3,
	0x6FE753FD, 0x4F47A610, 0xF47F4480, 0x02C5FF47, 0xC005707C, 0xEDB7FE7F, 0x00000007, 0x96FB9E00,
	0x2E2F4B56, 0x0BBBBE31, 0xAF78974C, 0x1FEB77D8, 0x38294BD3, 0xDCB7B783, 0xE01BE0F3, 0xF1603BE8,
	0x9BBC7B53, 0x30609D6E, 0x17D69B7B, 0x4A93FBE8, 0xDBEB518F, 0xA6CE0CCA, 0x9FE7B65F, 0x2EB5F952,
	0x74DAFBCB, 0xA96FB967, 0x7F1177A5, 0xBB6BD6D2, 0xFFC87C7F, 0x19BFEE30, 0x3FB6EFB0, 0x2C0F48A9,
	0xFEA77EA7, 0xB2B9FF7D, 0x9AE99433, 0x7888FD37, 0xBFDBB496, 0x6E314EEF, 0x3FAD87B8, 0xF2F748A9,
	0xDB70165F, 0x5BFADB5E, 0x171F46F9, 0x8D6FA46A, 0xFF6F5967, 0xEEFDFEA3, 0x1875DAD3, 0x5BC3DCD6,
	0xFD0D13BA, 0xFA16BDAD, 0x7777CDB6, 0xDED2B1E7, 0xDBC1E7DB, 0xD8520FE2, 0x7ED13FBF, 0x3FB5FE5E,
	0x0AF6EEFF, 0x8ED65AF5, 0x1AEED09F, 0x5ED7B7DA, 0x4F5DCF07, 0xBBFD3EEA, 0xD3FEFDFE, 0x472FADFA,
	0x3EB10DE1, 0x6278F2FB, 0x2AFD1F0E, 0x46C1BBBB, 0x7FEA47FF, 0x7D380574, 0xFB278181, 0x85BF8FF7,
	0x33C767B3, 0x00000998, 0xA8F00000, 0xFF35A037, 0xFBB247EE, 0x53F9F5D1, 0x64FE7D74, 0x209C57F2,
	0x10E388A6, 0x362FBF6E, 0xFEFA77CF, 0xEAA58D01, 0xB46754FD, 0xECDBD4BC, 0xEBD1DBBE, 0xD58EF8EE,
	0xFB13C0F9, 0xDF940C33, 0xECF7DE88, 0x2DE9E01F, 0x1ADC4EE3, 0x7EDD11DF, 0xEE2FE3BB, 0x5035F7F8,
	0xE7FD57BF, 0x6DF519DD, 0xBDDA6C07, 0xFCC4FABF, 0xCBFD5CF7, 0x149FCF47, 0xB6F7EFF7, 0xF87596DF,
	0x7A3BE337, 0xDDB4FDFE, 0x9F8BDE7F, 0xF96EE930, 0x6D5FF475, 0x1FF6A29F, 0xC2676F5C, 0xAD2ADFBF,
	0x7B45DC3B, 0xF8BDE7FD, 0xF8BD2289, 0xFFB19EBC, 0xB5DAFDB1, 0xF5CF78AF, 0xFEDDA7EF, 0x9BFDA573,
	0x6AF5C6E2, 0xFCDE2977, 0x0BB670BE, 0x53FE01E5, 0xD7DAFE05, 0xABC5FDE3, 0xE5DBFE84, 0xC6F0CA7F,
	0xEF9FFAE1, 0xFCB87F3F, 0x13F11767, 0x7BE07BE2, 0x00066B61, 0xE8000000, 0xFD1518F5, 0x40FE247D,
	0x7E7F9E1F, 0x07C1FC4B, 0x6779E7F3, 0xBC0FE217, 0x611FD95B, 0x747940A2, 0x7F79700A, 0x955A76EB,
	0x0FE2B5EB, 0xF7A57ADA, 0x5AB12FAE, 0x7F59EFDF, 0x15EB8D3D, 0xBB7ADFDC, 0x5EAF6BFD, 0xAF0FF59B,
	0x5F9F1D1F, 0x8EBF96BA, 0xC8FDFD68, 0xB7FF6B3F, 0xF9FFBFEB, 0x1B1366A4, 0xFDA05F5F, 0x7D5F38B9,
	0x6E6FFDBB, 0x17FABABF, 0xFFB616F6, 0xEDB5FDCD, 0x84448D7A, 0xC193DB54, 0xFB78CBC7, 0xEFF5F6F5,
	0x4FD7B6D5, 0x1E12AFE7, 0xEB7CF8D7, 0xFB6102FA, 0xE87DEF3F, 0x63B9E5F8, 0xFCF1FAF6, 0xCB1BFFA1,
	0x84D180F7, 0xFE8FFB76, 0xF3FFDB9B, 0xFA85A832, 0x3DFD4871, 0xA24D427B, 0x13F5027E, 0xBE000000,
	0xFED17FA6, 0xFEF57C3C, 0xDA5DBFFF, 0xEA8F7BFD, 0x887AC932, 0xEEE976F7, 0xD978FF5E, 0x53FE9FBF,
	0xFFBFD064, 0xF1FA8573, 0x86F32D26, 0x040D89A4, 0xB6D46452, 0x6F1ECF94, 0xAE0532D2, 0xF778FD98,
	0xBAB9499F, 0xFD92F2FF, 0x76FF1CAB, 0x78DEBAF5, 0xFB2B5ED3, 0xBDFB9B6F, 0x9527EF9C, 0x7E4C07F9,
	0x7E57A035, 0x2D7B2DC3, 0xA0176BE5, 0x79D4099C, 0xFA91A5F9, 0xE4EB9A5E, 0xFD72EBFE, 0xAFF6B317,
	0xD7CE7F86, 0x3FAF63AB, 0xCF9D6EBF, 0xFB2D4FA9, 0xBCA5EAEF, 0x55ECF7A6, 0x75A8FDA5, 0x1DFD11BE,
	0x5FF5377B, 0x7D7EF6F5, 0x047EF5FF, 0xF95BF542, 0xF43793FB, 0x0DEBD8EA, 0xABD31FF6, 0xF5BABFA3,
	0x769FA8BD, 0x9F33797F, 0xA9F78E5A, 0xFC56FE28, 0x91406954, 0x886F2278, 0xAA2068A9, 0xFFB7EEDF,
	0x9E830EDD, 0x2D7EA5FE, 0x9D4E6D7B, 0x0EBEEE9F, 0xEF557FD8, 0xE5A778D5, 0xFD8EC9C1, 0xD3F00E77,
	0xF6AFEA37, 0x5EDF4C3B, 0x634FD6B7, 0x3D6469EC, 0x70243F2E, 0xBEB79AD8, 0xAB19E9BD, 0xBABC9F47,
	0xF6727C79, 0x0EEF059A, 0xFF5C394F, 0x3FF4F7F4, 0xFFE8FF2D, 0xDF9FFABD, 0x801D1EBE, 0xE797E3A2,
	0x00000001, 0xD7C53F80, 0xBD37CFFB, 0xFE9AAB02, 0xFE439EAC, 0x90BD2D73, 0xA5E7FCC6, 0xB9FF7D7E,
	0xF087219C, 0x94D2603B, 0xAC25DD9C, 0x2E0FF517, 0x3FE437D0, 0x55A7A72F, 0x007689FD, 0x2E9018BA,
	0xBD7C3202, 0x5155EDC6, 0xBB6B63F7, 0x557AFDE9, 0x9FAD57CE, 0x79061132, 0xEA6D7B7A, 0x5FA7ABD9,
	0xBB13F86A, 0x8A772FF6, 0xA07EDDB0, 0xAD3F6FA1, 0xF596F9D6, 0x19520997, 0xF2F25FD0, 0x932D3723,
	0xA03D3CE1, 0x978F3FFF, 0xD8EFF4C6, 0x6BFECD6B, 0x01D8B3FB, 0x7DFB3928, 0xCFADCB38, 0xBF772FEB,
	0x29F3D6F3, 0xA6D69EDB, 0x4281809D, 0x6616156F, 0xE2D9506F, 0x1FC32AF2, 0x3DEBF9DA, 0xC2E99CBC,
	0x97FDFD66, 0x59FFC6FE, 0xBFF67BA4, 0xA7B9407B, 0x3D3E597F, 0xBF9FDD7B, 0xF256BD96, 0x52D7B157,
	0x15DCC5FF, 0xF7A36CEE, 0xED7C79E5, 0xB9F6D98F, 0xFB5D9F6A, 0x277FADDD, 0xA529539E, 0x91A15622,
	0xEDC050DE, 0xACE9D8FA, 0x3B9AC13A, 0xF89C2403, 0xAF6F5CBE, 0x878F38C2, 0x484FBDCF, 0xD5DF4F87,
	0xA51E0EC9, 0x1FDF97D3, 0x47F45F2B, 0xF7CB43FF, 0x751B0DE8, 0xE1FC83BA, 0x00000BE5, 0xD9300000,
	0x6FA7ADCD, 0x7DF398DA, 0xF71FA02F, 0xC12EEF4E, 0xC9E9F3A5, 0x8EEE3BFA, 0x7BD3BD3B, 0x876077CA,
	0x96D5D8F2, 0x88AE8AF4, 0xFF7AB6BD, 0xAD07FAC4, 0xFCEA8DF8, 0xF180CAD8, 0xD6CDA37F, 0x3D90AFDF,
	0xFB92F484, 0x6745737D, 0x484B6A75, 0xEF6EDEAF, 0x8D0072DE, 0x0FF5B1CF, 0xDCC2FE7A, 0x3FD6671E,
	0xD8E9EAB8, 0xD89FD57F, 0xD88D27F1, 0xF92F4C6B, 0x3FBF36F6, 0xE77CC543, 0x61BE67D6, 0xFF318BFD,
	0xFDFE02BE, 0x7E85BE86, 0xFD649FD6, 0x527F5D89, 0x12CB5ED7, 0xE4AFE968, 0xBF3CA72F, 0xFF4C4CBE,
	0x5ECFAEF7, 0xC96BEFF7, 0x77D55FBF, 0x67A143BF, 0x2D76DEAF, 0xFE7D6A7F, 0xF66DCCC5, 0xDDF477EF,
	0xDAE850EE, 0xE96F44EB, 0x17F9F5D6, 0xB37B6BFB, 0xFFBFDDB7, 0xE1D18776, 0xD7C3A9DE, 0xFE03FF67,
	0x3447B3DD, 0x1BE6FFD0, 0xF6797E38, 0x0CF6FC74, 0xEFDA1589, 0x010A622D, 0x00000000, 0xCA02F2E0,
	0x0BDEFDFC, 0x81FD86EE, 0x079F3BA3, 0x9E0EC1E7, 0x9E0FE3BF, 0x61BFEFF0, 0x02C0FBFD, 0x3EBD3DA4,
	0x88719E37, 0x7EFF41FE, 0xAFE07B19, 0x01DAAF7F, 0x5387C5ED, 0xE45A43DF, 0x1637E6C7, 0xCFDF114F,
	0xC77C537B, 0xD777F5DB, 0xF7FA8DF1, 0x2CC97ABB, 0xEFF51009, 0xE967C795, 0x145BFE8E, 0xB6F92D4E,
	0xA6336FED, 0xFB6F5D4F, 0x39B1CD7D, 0xFEE3FF62, 0x435EA8FD, 0xFB9471CB, 0x53E7D855, 0xAFBFD7CA,
	0x8A35A785, 0xC77C5779, 0xB15FEC73, 0xDEE9E074, 0xAFD4BFD0, 0x0F6F5EDD, 0xAF6BD83F, 0x828BFA37,
	0x7B5EAAF6, 0x0FFCF730, 0x2FEFEFCB, 0xDFEBDA93, 0x3749C237, 0x7FA0806F, 0xAF64D9FF, 0xCEB3B9F7,
	0xE8CDFFE8, 0x11F77F8F, 0xA85F4BFA, 0x4FFF697F, 0x64647FD1, 0x87F3C6E6, 0x7BE7FE9E, 0x371FD1D0,
	0xFA03FA37, 0xFA03FA03, 0xB60C0003, 0xD275FD2F, 0xE7180363, 0xFB737F9D, 0xCE78F37D, 0x6EF4F96A,
	0x3FD1FD1F, 0x8BFCF74B, 0xC47FD5EF, 0xAD3769DD, 0x7FD1A46D, 0xF4C7FDB3, 0xC65B4DE9, 0x7FAFDFFE,
	0xDB37C047, 0xEA3B2C7F, 0xBFF63BD9, 0x619DFED9, 0xB1DA46F6, 0xFF9F93EF, 0xFD572543, 0x4EE93A91,
	0x7749D4FE, 0xA4FBD033, 0xFD1EF7FF, 0xA83D3F25, 0x9FC575FF, 0xA93BFB50, 0xE4BE347F, 0x0DFFEDBD,
	0x14AD4BED, 0x36C201AE, 0xF2F7D3DD, 0x2BE7E563, 0xF1CC8A62, 0x17FF8FCA, 0x3DD26D79, 0x282E657D,
	0xC67B032F, 0x91C3E317, 0xAB5F9CC7, 0xDF681A57, 0xCA83ED7B, 0xFAC1B410, 0xADDF6A3B, 0x7FE841F6,
	0xE39DFEB7, 0x73F7FA7B, 0xF3DF6B1F, 0xEE7FD1E3, 0xFBED234A, 0xF68DF1D0, 0x14F9F07D, 0x7FD1DF6C,
	0x96FEF9F4, 0xF8C36446, 0xEA064CC4, 0xF64DFBF7, 0xFFF796FF, 0xFFA7F523, 0x0000FE87, 0xF1ACE6EF,
	0x7FEFD3C2, 0xACE18387, 0xF47F40E2, 0x47F47F47, 0xFF47F47F, 0x3A3FEBF7, 0xC2B83FFF, 0xAA1FFA0E,
	0x82C772BF, 0x95A9C14C, 0xCFF4BBCF, 0xB49D5EC6, 0x5E3237D5, 0xFCFFE6BE, 0xD7AFF6B3, 0xDA6E2ED2,
	0xF8D09A56, 0x99F7E478, 0xD3FACA9F, 0x32D1E31B, 0xE2757DFE, 0xB1FE2305, 0xCCC07F55, 0xD7D6CFFD,
	0xDF6674C1, 0xBA08F1A3, 0xF77B66A9, 0xD0BF1433, 0x6BCA8FF0, 0xFFCF061F, 0x6EEEF6CE, 0xC65FAFFB,
	0x3C66EFB5, 0xAC396842, 0xED8F7EA2, 0x3950A79F, 0xA3C3FBCD, 0xFF53BFF9, 0xA8A3CF9D, 0xF2E7A1FE,
	0xB669391D, 0x881BEDEF, 0x89EEEBC4, 0xF3FCF87F, 0x4FA9477F, 0xFC8423C7, 0xE5B573DF, 0x6D5FFB04,
	0x05E216EF, 0x7BFFA2DE, 0x47BE775E, 0xFFD9A7FF, 0xD74FFA89, 0x186E9B9A, 0x5A0001FD, 0xF3FDEDED,
	0xFE369ECF, 0x8F4F7BAB, 0xFE8FE8FE, 0xE8FE8FE8, 0x9BFE827F, 0xFFA3FFE7, 0xFCBFFAE1, 0xD7CACA83,
	0xF70DEDA6, 0x05FBAE26, 0x05CF5C14, 0xFF66DFFD, 0xAAF60332, 0xE8CFE00D, 0xFFFF64FF, 0x975FD51F,
	0xCA46804D, 0xC277D33B, 0xA1FAFD7F, 0xE3EAF69F, 0xD3641FD5, 0xD2F1F5D3, 0x46D3B4F6, 0x423DD960,
	0x0FF427F4, 0xE1A14FAC, 0x0FA9D6F6, 0xF3A61332, 0xFABC63AD, 0x3DDFD0D6, 0xEC7F547F, 0xDBC6053F,
	0xFC19E5A3, 0x7D7DC5FD, 0x7A267E4F, 0x8B812146, 0xBA6682F0, 0xD3062770, 0xF37D55FF, 0x37FAB5FD,
	0xCFF3ABD3, 0x6D09F587, 0xFFF21908, 0x7A0FF5C7, 0x0329A56E, 0x51FD785F, 0xE3C5AD2C, 0xA2DEC853,
	0xF679DE0D, 0x2FF877AF, 0xFB58FFD1, 0x3FFCF3DF, 0xD372FF5C, 0x56FA7ACE, 0xAF7F27FD, 0x8FDE1CE3,
	0xDA6EDFD2, 0x00034CD9, 0x00000000, 0x00000000, 0xBAC39FC0, 0x19979787, 0xDBCFCA4A, 0xFB6FDB49,
	0xE9E4A7DB, 0x45947772, 0xE39EFBFF, 0xD7C87593, 0x7EFF85D2, 0xE1C6F1D7, 0x775B4F5F, 0x47FD1EC2,
	0xBBC9CB3F, 0x3347F5D3, 0x40FBAFF9, 0x6397C256, 0xC7391C0E, 0x9ACBED9B, 0x584B3969, 0x4F3F8EC4,
	0x589876FF, 0xFF2586BA, 0x47B4A1F7, 0x9FF63DFF, 0x79380122, 0xF7EC8449, 0xD5397A12, 0xB2BBFC7E,
	0xAAAFCB26, 0xC73F547F, 0x5690341A, 0xF5978351, 0x46BC75DF, 0xCDD8AAD2, 0x6547F2D8, 0xBF8F44E9,
	0x01EE7BFC, 0x6C9D7C58, 0x8BA76C8D, 0x09DAFBFA, 0xDAFD286B, 0xF01ACB37, 0xFD8A5A58, 0x6B78EC7E,
	0x6174F5D0, 0x67DAC9FF, 0xFF3C5D3B, 0x0003C783, 0x00000000, 0x00000000, 0xE6A49FC0, 0x3B4D1A3F,
	0xFC3B03F1, 0xE013C3D7, 0x64EFFE6B, 0xD257C427, 0xEBA9DBF6, 0x585F89CB, 0x3868FFF2, 0xC9BC94AD,
	0x9627217C, 0xB4205BBA, 0x719CAAD3, 0x2E772D34, 0x2AD386B2, 0x223C614E, 0x6ED2B49D, 0x7FA39EFD,
	0x6A3E7ED5, 0xBA3F99FF, 0xBAFD9699, 0xED3B4BF8, 0xDFAABF1C, 0x2F11FB6A, 0x5FDBCD7F, 0xF41C7FCD,
	0x1C67BBDF, 0x2FB17B49, 0xF4FCF9FE, 0xC381F8BE, 0x165F3FEC, 0xA5569AA9, 0xBE6E1C7D, 0x14E3FE6B,
	0x67FBDFF1, 0xD7FB4A1C, 0x38E0058F, 0x0FC78FF9, 0x6BEBF9A4, 0xF1ABFF31, 0x7703B613, 0x01A7E8FD,
	0x00000000, 0x00000000, 0xDD839E00, 0xFE97B99D, 0x92F2F2FA, 0x531C1E0B, 0xFCB4EDBE, 0xCFC707AF,
	0x6EE3B7E7, 0xEB858ED7, 0x14F8FA7F, 0x7FD66F4D, 0xFC55EFC8, 0xC17E2179, 0x9FD536A6, 0xC1D089D9,
	0x3D8A8BD8, 0x3C2FC2C6, 0x50CCA9F3, 0x489C7677, 0x57FD3ED2, 0xFF3F9FFD, 0x591F8A5E, 0xF6E3FE3B,
	0x06FFA6BB, 0x020BB5FB, 0x5772D2B4, 0xFFB35F97, 0x44FFFDD0, 0x3C6D283F, 0x1FF6D1FE, 0x7AC3805B,
	0x41ABAF85, 0xBF6D2068, 0x4FFCF2DD, 0x74FCCE5E, 0xE3E5FF99, 0x2D8FF557, 0xF3C27020, 0x0000001F,
	0x00000000, 0x39E00000, 0xFC045AEC, 0x19CACA2B, 0x1DF54F9D, 0x4DF4F3E6, 0xCC2FD7EB, 0xB0FF459F,
	0x99F945F8, 0x749D9119, 0xE3D62A91, 0xBCEDA7AE, 0xD29E57DC, 0x6674BA68, 0xBE7FE284, 0x96E21BDD,
	0xFC33B27E, 0xFA67696B, 0x31FEAAE9, 0x066371FF, 0x5F5A50F9, 0xA65406DE, 0x9FCDAFAD, 0xABFEA8FF,
	0xE77C501B, 0xC3FF2013, 0xF979FCF0, 0x92B3E7F3, 0xBF7FAE0D, 0xFF63FEDC, 0xE7FBDEDF, 0x00000FF9,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0xF0000000, 0x000FFC55, 0x03FFFF00, 0x00000000,
};


//////////////////////////////////////////////////////////////////////////
// 글꼴

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"Font"

//
static void _font_dispose(QnGam g)
{
	QgFont* self = qn_cast_type(g, QgFont);
	qn_free(self);
}

//
void qg_font_set_size(QgFont* self, int size)
{
	qn_cast_vtable(self, QGFONT)->set_size(self, size);
}

//
void qg_font_draw(QgFont* self, const QmRect* bound, const char* text)
{
	VAR_CHK_IF_NULL(bound, );
	VAR_CHK_IF_NULL(text, );
	qn_cast_vtable(self, QGFONT)->draw(self, bound, text);
}

//
void qg_font_write(QgFont* self, int x, int y, const char* text)
{
	VAR_CHK_IF_NULL(text, );
	const QmSize size = RDH_TRANSFORM->size;
	const QmRect bound = qm_rect(x, y, size.Width, size.Height);
	qn_cast_vtable(self, QGFONT)->draw(self, &bound, text);
}

//
void qg_font_write_format(QgFont* self, int x, int y, const char* fmt, ...)
{
	VAR_CHK_IF_NULL(fmt, );
	va_list va;
	va_start(va, fmt);
	char buffer[1024];
	const int len = qn_vsnprintf(buffer, QN_COUNTOF(buffer), fmt, va);
	va_end(va);
	if (len <= 0)
		return;

	const QmSize size = RDH_TRANSFORM->size;
	const QmRect bound = qm_rect(x, y, size.Width, size.Height);
	qn_cast_vtable(self, QGFONT)->draw(self, &bound, buffer);
}

//
static QmKolor _font_string_color(const QgFont* self, const char* text, int len)
{
	if (text[0] == '#')
	{
		if (len == 9)
			return (QmKolor) { qn_strtoi(text + 1, 16) };
		if (len == 7)
			return (QmKolor) { 0xFF000000 | qn_strtoi(text + 1, 16) };
		if (len == 4)
		{
			char buf[8];
			buf[0] = text[1];
			buf[1] = text[1];
			buf[2] = text[2];
			buf[3] = text[2];
			buf[4] = text[3];
			buf[5] = text[3];
			buf[6] = '\0';
			return (QmKolor) { 0xFF000000 | qn_strtoi(buf, 16) };
		}
	}
	if (qn_stricmp(text, "black") == 0)
		return (QmKolor) { QMKOLOR_BLACK };
	if (qn_stricmp(text, "white") == 0)
		return (QmKolor) { QMKOLOR_WHITE };
	if (qn_stricmp(text, "red") == 0)
		return (QmKolor) { QMKOLOR_RED };
	if (qn_stricmp(text, "green") == 0)
		return (QmKolor) { QMKOLOR_GREEN };
	if (qn_stricmp(text, "blue") == 0)
		return (QmKolor) { QMKOLOR_BLUE };
	if (qn_stricmp(text, "yellow") == 0)
		return (QmKolor) { QMKOLOR_YELLOW };
	if (qn_stricmp(text, "cyan") == 0)
		return (QmKolor) { QMKOLOR_CYAN };
	if (qn_stricmp(text, "magenta") == 0)
		return (QmKolor) { QMKOLOR_MAGENTA };
	if (qn_stricmp(text, "gray") == 0)
		return (QmKolor) { QMKOLOR_GRAY };
	return self->color;
}


//////////////////////////////////////////////////////////////////////////
// 트루타입 글꼴

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"TrueType"

typedef struct TRUETYPE_KEY
{
	int					code;
	int					size;
} TrueTypeKey;

typedef struct TRUETYPE_VALUE
{
	int					advance;
	QmPoint				offset;
	QgTexture*			tex;
} TrueTypeValue;

INLINE int truetype_key_hash(const TrueTypeKey* key)
{
	return key->code ^ key->size;
}

INLINE bool truetype_key_eq(const TrueTypeKey* a, const TrueTypeKey* b)
{
	return a->code == b->code && a->size == b->size;
}

INLINE void truetype_value_dispose(const TrueTypeValue* value)
{
	qn_unload(value->tex);
}

// 트루타입 묶음
QN_DECLIMPL_MUKUM(TrueTypeHash, TrueTypeKey, TrueTypeValue, truetype_key_hash, truetype_key_eq, (void), truetype_value_dispose, _truetype_hash);

// sbtt 트루타입 데이터
typedef struct STBTT_DATA
{
	stbtt_fontinfo		stbtt;
	float				scale;
	int					ascent, descent, linegap;
	struct STBTT_DATA*	next;
} StbttData;

// 트루타입 글꼴
typedef struct TRUETYPE_FONT
{
	QN_GAM_BASE(QGFONT);

	StbttData*			nodes;
	TrueTypeHash		glyphs;
} TrueTypeFont;

//
void _truetype_set_size(QnGam g, int size)
{
	TrueTypeFont* font = qn_cast_type(g, TrueTypeFont);

	if (font->base.size == size)
		return;

	font->base.size = QN_CLAMP(size, 8, 256);

	for (StbttData* node = font->nodes; node != NULL; node = node->next)
	{
		node->scale = stbtt_ScaleForPixelHeight(&node->stbtt, (float)font->base.size);
		stbtt_GetFontVMetrics(&node->stbtt, &node->ascent, &node->descent, &node->linegap);
	}
}

//
static QgImage* _truetype_generate_image(byte* bitmap, int width, int height)
{
	QgImage* img = qg_create_image(QGCF_A8, width, height);
	byte* buffer = qn_get_gam_pointer(img);
	for (int y = 0; y < height; ++y)
	{
		const byte* src = bitmap + y * width;
		byte* dst = buffer + y * img->pitch;
		for (int x = 0; x < width; ++x)
			*dst++ = *src++;
	}
	qn_free(bitmap);
	return img;
}

//
static TrueTypeValue* _truetype_get_glyph(TrueTypeFont* self, int code)
{
	TrueTypeKey key = { code, self->base.size };
	TrueTypeValue* value = _truetype_hash_get_ptr(&self->glyphs, &key);
	if (value != NULL)
		return value;

	StbttData* node;
	int index = 0;
	for (node = self->nodes; node != NULL; node = node->next)
	{
		index = stbtt_FindGlyphIndex(&node->stbtt, code);
		if (index != 0)
			break;
	}
	if (index == 0)
		return NULL;

	int x0, y0, x1, y1, advance, lsb;
	byte* bitmap = stbtt_GetGlyphBitmapSubpixel(&node->stbtt, node->scale, node->scale, 0.0f, 0.0f, index, &x0, &y0, &x1, &y1);
	if (bitmap == NULL)
		return NULL;
	stbtt_GetGlyphHMetrics(&node->stbtt, index, &advance, &lsb);

	QgImage* img = _truetype_generate_image(bitmap, x0, y0);
	QgTexture* tex = qg_create_texture(NULL, img, QGTEXF_DISCARD_IMAGE | QGTEXF_CLAMP);
	if (tex == NULL)
	{
		qn_unload(img);
		return NULL;
	}

	value = _truetype_hash_set_key_ptr(&self->glyphs, &key);
	value->advance = (int)((float)advance * node->scale);
	value->offset = qm_point(x1, y1 + (int)((float)node->ascent * node->scale));
	value->tex = tex;

	return value;
}

//
static void _truetype_draw(QnGam g, const QmRect* bound, const char* text)
{
	TrueTypeFont* self = qn_cast_type(g, TrueTypeFont);
	QmPoint pt = qm_point(bound->Left, bound->Top);
	QmKolor color = self->base.color;
	const QmSize step = self->base.step;
	const int height = self->base.size + step.Height;
	char clrbuf[16];
	int i;

	while (*text)
	{
		int len;
		const int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				pt.X += self->base.size / 3 + step.Width;
				break;
			case '\n':
				pt.X = bound->Left;
				pt.Y += height;
				if (pt.Y + height > bound->Bottom)
					goto pos_exit;
				break;
			case '\t':
				pt.X += (self->base.size + step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					const char ch = *(text + i);
					if (ch == '\0')
						goto pos_exit;
					if (ch == '\a')
						break;
					clrbuf[i] = ch;
				}
				if (i == 0)
				{
					text++;
					color = self->base.color;
				}
				else
				{
					text += i + 1;
					clrbuf[i] = '\0';
					color = _font_string_color(qn_cast_type(self, QgFont), clrbuf, i);
				}
				break;
			default:
				if (code > ' ')
				{
					const TrueTypeValue* value = _truetype_get_glyph(self, code);
					if (value != NULL)
					{
						if (pt.X + value->offset.X + value->tex->width >= bound->Right)
							break;
						QmRect rect = qm_rect_size(pt.X + value->offset.X, pt.Y + value->offset.Y, value->tex->width, value->tex->height);
						qg_draw_glyph(&rect, value->tex, color, NULL);
						pt.X += value->advance + step.Width;
					}
				}
				break;
		}
	}

pos_exit:
	return;
}

//
static QmPoint _truetype_calc(QnGam g, const char* text)
{
	TrueTypeFont* self = qn_cast_type(g, TrueTypeFont);
	const QmSize step = self->base.step;
	QmPoint ret = qm_point(0, 0);
	QmPoint pt = qm_point(0, 0);
	int i;

	while (*text)
	{
		int len;
		const int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				pt.X += self->base.size / 3 + step.Width;
				break;
			case '\n':
				if (ret.X < pt.X)
					ret.X = pt.X;
				pt.X = 0;
				pt.Y += self->base.size + step.Height;
				break;
			case '\t':
				pt.X += (self->base.size + step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					const char ch = *(text + i);
					if (ch == '\0')
						goto pos_exit;
					if (ch == '\a')
						break;
				}
				if (i == 0)
					text++;
				else
					text += i + 1;
				break;
			default:
				if (code > ' ')
				{
					const TrueTypeValue* value = _truetype_get_glyph(self, code);
					if (value != NULL)
						pt.X += value->advance + step.Width;
				}
				break;
		}
	}

pos_exit:
	if (ret.X < pt.X)
		ret.X = pt.X;
	ret.Y = pt.Y + self->base.size + self->base.step.Height;
	return ret;
}

//
static void _truetype_dispose(QnGam g)
{
	TrueTypeFont* self = qn_cast_type(g, TrueTypeFont);
	_truetype_hash_dispose(&self->glyphs);
	for (StbttData *next, *node = self->nodes; node != NULL; node = next)
	{
		next = node->next;
		qn_free(node->stbtt.data);
		qn_free(node);
	}
	_font_dispose(self);
}

// 트루타입 데이터 만들기
StbttData* _truetype_open(const void* data, int data_size, int font_base_size, int offset_index)
{
	StbttData* node = qn_alloc_zero_1(StbttData);
	if (stbtt_InitFont(&node->stbtt, data, offset_index) == 0)
	{
		qn_free(node);
		return NULL;
	}
	node->scale = stbtt_ScaleForPixelHeight(&node->stbtt, (float)font_base_size);
	stbtt_GetFontVMetrics(&node->stbtt, &node->ascent, &node->descent, &node->linegap);
	return node;
}

// 트루타입 만들기
QgFont* _truetype_create(void* data, int data_size, int offset_index)
{
	int font_base_size = 24;
	StbttData* node = _truetype_open(data, data_size, font_base_size, offset_index);
	qn_return_when_fail(node != NULL, NULL);

	TrueTypeFont* self = qn_alloc_zero_1(TrueTypeFont);
	self->nodes = node;
	_truetype_hash_init_fast(&self->glyphs);

	self->base.size = font_base_size;
	self->base.color.v = QMKOLOR_WHITE;
	self->base.flags = QGFONTTYPE_TRUETYPE;

	static const QN_DECL_VTABLE(QGFONT) vt_qg_truetype =
	{
		{
			VAR_CHK_NAME,
			_truetype_dispose,
		},
		_truetype_set_size,
		_truetype_draw,
		_truetype_calc,
	};
	return qn_gam_init(self, vt_qg_truetype);
}

// 트루타입 추가
bool _truetype_add(QgFont* font, void* data, int size, int offset_index)
{
	TrueTypeFont* self = qn_cast_type(font, TrueTypeFont);

	StbttData* node = _truetype_open(data, size, self->base.size, offset_index);
	qn_return_when_fail(node != NULL, false);

	StbttData* find = self->nodes;
	while (find->next != NULL) find = find->next;
	find->next = node;
	return true;
}


//////////////////////////////////////////////////////////////////////////
// 조합형 글꼴

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"Johab"

// 트루타입 글꼴
typedef struct JOHAB_FONT
{
	QN_GAM_BASE(QGFONT);

	QgTexture*			tex;
	float				width;
	float				height;

	int 				per_ascii;
	int 				per_johab;
	QmPoint				johab_pos;
	QmVec2				uv_size;
} JohabFont;

//
static void _johab_set_size(QnGam g, int size)
{
	JohabFont* font = qn_cast_type(g, JohabFont);
	if (font->base.size == size)
		return;
	font->base.size = QN_CLAMP(size, 8, 256);
}

//
static void _johan_draw_char(QgTexture* tex, const QmRect* rect, const QmKolor color, float u, float v, float su, float sv)
{
	const QMVEC coord = qm_vec(u, v, u + su, v + sv);
	qg_draw_glyph(rect, tex, color, &coord);
}

//
static void _johab_ascii(const JohabFont* self, int code, const QmKolor color, const QmPoint* pt)
{
	qn_debug_verify(code >= 0x20 && code <= 0x7E);

	const QmRect rect = qm_rect_size(pt->X, pt->Y, self->base.size / 2, self->base.size);
	code = (code - 0x20);
	const int x = code % self->per_ascii;
	const int y = code / self->per_ascii;
	_johan_draw_char(self->tex, &rect, color,
		(float)x * self->uv_size.X * 0.5f, (float)y * self->uv_size.Y, self->uv_size.X * 0.5f, self->uv_size.Y);
}

//
bool _johab_hangul(const JohabFont* self, int f, int m, int l, const QmKolor color, const QmPoint* pt)
{
	// 모음 종류
	static const byte ini_maps[2][30] =
	{
		// 받침이 없는 모음에 따라 어떤 초성을 쓸지 정함
		{
			0, 0, 0, 0, 0, 0, 0, 0,			/* (채움),ㅏ,ㅐ,ㅑ,ㅒ,ㅓ */
			0, 0, 0, 0, 0, 1, 3, 3,			/* ㅔ,ㅕ,ㅖ,ㅗ,ㅗㅏ,ㅗㅐ */
			0, 0, 3, 1, 2, 4, 4, 4,			/* ㅗㅣ,ㅛ,ㅜ,ㅜㅓ,ㅜㅔ,ㅜㅣ */
			0, 0, 2, 1, 3, 0				/* ㅠ,ㅡ,ㅡㅣ,ㅣ */
		},
		// 받침이 있는 모음에 따라 어떤 초성을 쓸지 정함
		{
			0, 0, 5, 5, 5, 5, 5, 5,			/* (채움),ㅏ,ㅐ,ㅑ,ㅒ,ㅓ */
			0, 0, 5, 5, 5, 6, 8, 8,			/* ㅔ,ㅕ,ㅖ,ㅗ,ㅗㅏ,ㅗㅐ */
			0, 0, 8, 6, 7, 9, 9, 9,			/* ㅗㅣ,ㅛ,ㅜ,ㅜㅓ,ㅜㅔ,ㅜㅣ */
			0, 0, 7, 6, 8, 5				/* ㅠ,ㅡ,ㅡㅣ,ㅣ */
		},
	};
	// 모음 글꼴 인덱스
	static const ushort vow_base[30] =
	{
		0, 0, 0, 311, 314, 317, 320, 323,	/* (채움),ㅏ,ㅐ,ㅑ,ㅒ,ㅓ */
		0, 0, 326, 329, 332, 335, 339, 343,	/* ㅔ,ㅕ,ㅖ,ㅗ,ㅗㅏ,ㅗㅐ */
		0, 0, 347, 351, 355, 358, 361, 364,	/* ㅗㅣ,ㅛ,ㅜ,ㅜㅓ,ㅜㅔ,ㅜㅣ */
		0, 0, 367, 370, 374, 378			/* ㅠ,ㅡ,ㅡㅣ,ㅣ */
	};
	// 모음이 ㅗ 모양인지 구분
	static const byte vow_kind[30] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,				/* (채움),ㅏ,ㅐ,ㅑ,ㅒ,ㅓ */
		0, 0, 0, 0, 0, 1, 1, 1,				/* ㅔ,ㅕ,ㅖ,ㅗ,ㅗㅏ,ㅗㅐ */
		0, 0, 1, 1, 0, 0, 0, 0,				/* ㅗㅣ,ㅛ,ㅜ,ㅜㅓ,ㅜㅔ,ㅜㅣ */
		0, 0, 0, 1, 1, 0					/* ㅠ,ㅡ,ㅡㅣ,ㅣ */
	};
	// 받침에 따른 모음 모양에 영향
	static const byte fin_kind[30] =
	{
		0, 0, 1, 1, 1, 2, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1
	};
	// 모음에 따른 받침 모양
	static const byte fin_map[30] =
	{
		0, 0, 0, 0, 2, 0, 2, 1,
		0, 0, 2, 1, 2, 3, 0, 0,
		0, 0, 0, 3, 3, 1, 1, 1,
		0, 0, 3, 3, 0, 1
	};

	const QmRect rect = qm_rect_size(pt->X, pt->Y, self->base.size, self->base.size);
	int code, x, y, cnt = 0;

	// 초성
	if (f > 1 && f <= 20)
	{
		code = f * 10 + ini_maps[l > 1 ? 1 : 0][m] - 19;
		x = code % self->per_johab + self->johab_pos.X;
		y = code / self->per_johab + self->johab_pos.Y;
		_johan_draw_char(self->tex, &rect, color,
			(float)x * self->uv_size.X, (float)y * self->uv_size.Y, self->uv_size.X, self->uv_size.Y);
		cnt++;
	}
	// 중성
	if (m > 2 && m <= 29)
	{
		code = vow_base[m] + (vow_kind[m] == 0 ? fin_kind[l] : ((f == 2 || f == 17) ? 0 : 1) + (l > 1 ? 2 : 0));
		x = code % self->per_johab + self->johab_pos.X;
		y = code / self->per_johab + self->johab_pos.Y;
		_johan_draw_char(self->tex, &rect, color,
			(float)x * self->uv_size.X, (float)y * self->uv_size.Y, self->uv_size.X, self->uv_size.Y);
		cnt++;

		// 종성
		if (l > 1 && l <= 29)
		{
			code = l * 4 + fin_map[m] + (l < 18 ? 397 : 393);
			x = code % self->per_johab + self->johab_pos.X;
			y = code / self->per_johab + self->johab_pos.Y;
			_johan_draw_char(self->tex, &rect, color,
				(float)x * self->uv_size.X, (float)y * self->uv_size.Y, self->uv_size.X, self->uv_size.Y);
			cnt++;
		}
	}

	return cnt != 0;
}

//
static void _johab_draw(QnGam g, const QmRect* bound, const char* text)
{
	JohabFont* self = qn_cast_type(g, JohabFont);
	QmPoint pt = qm_point(bound->Left, bound->Top);
	QmKolor color = self->base.color;
	const QmSize step = self->base.step;
	const int size = self->base.size, half_size = size / 2;
	char cbuf[16];
	int i;

	while (*text)
	{
		int len, local_glyph[3];
		const int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				pt.X += half_size + step.Width;
				break;
			case '\n':
				pt.X = bound->Left;
				pt.Y += size + step.Height;
				if (pt.Y + size + step.Height > bound->Bottom)
					goto pos_exit;
				break;
			case '\t':
				pt.X += (half_size + step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					const char ch = *(text + i);
					if (ch == '\0')
						goto pos_exit;
					if (ch == '\a')
						break;
					cbuf[i] = ch;
				}
				if (i == 0)
				{
					text++;
					color = self->base.color;
				}
				else
				{
					text += i + 1;
					cbuf[i] = '\0';
					color = _font_string_color(qn_cast_type(self, QgFont), cbuf, i);
				}
				break;
			default:
				if (code > 0x20 && code <= 0x7E)
				{
					if (pt.X + half_size >= bound->Right)
						break;
					_johab_ascii(self, code, color, &pt);
					pt.X += half_size + step.Width;
					break;
				}
				if (qn_hangul_dcp(code, local_glyph))
				{
					if (pt.X + size >= bound->Right)
						break;
					if (_johab_hangul(self, local_glyph[0], local_glyph[1], local_glyph[2], color, &pt))
					{
						pt.X += size + step.Width;
						break;
					}
				}
				if (pt.X + half_size >= bound->Right)
					break;
				_johab_ascii(self, 0x20, color, &pt);
				pt.X += half_size + step.Width;
				break;
		}
	}

pos_exit:
	return;
}

//
static QmPoint _johab_calc(QnGam g, const char* text)
{
	const JohabFont* self = qn_cast_type(g, JohabFont);
	QmPoint ret = qm_point(0, 0);
	QmPoint pt = qm_point(0, 0);
	const QmSize step = self->base.step;
	const int size = self->base.size, half_size = size / 2;
	int i;

	while (*text)
	{
		int len, local_glyph[3];
		const int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				pt.X += half_size + step.Width;
				break;
			case '\n':
				if (ret.X < pt.X)
					ret.X = pt.X;
				pt.X = 0;
				pt.Y += size + step.Height;
				break;
			case '\t':
				pt.X += (half_size + step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					const char ch = *(text + i);
					if (ch == '\0')
						goto pos_exit;
					if (ch == '\a')
						break;
				}
				if (i == 0)
					text++;
				else
					text += i + 1;
				break;
			default:
				if (qn_hangul_dcp(code, local_glyph))
					pt.X += size + step.Width;
				else
					pt.X += half_size + step.Width;
				break;
		}
	}

pos_exit:
	if (ret.X < pt.X)
		ret.X = pt.X;
	ret.Y = pt.Y + size + step.Height;
	return ret;
}

//
static void _johab_dispose(QnGam g)
{
	JohabFont* self = qn_cast_type(g, JohabFont);
	qn_unload(self->tex);
	_font_dispose(self);
}

// 조합 만들기
static QgFont* _johab_create(QgImage* image, int font_display_size)
{
	const int font_base_size = image->extra == 0 ? 16 : image->extra;
	QgTexture* tex = qg_create_texture(NULL, image, QGTEXF_DISCARD_IMAGE | QGTEXF_CLAMP);
	qn_return_when_fail(tex != NULL, NULL);

	JohabFont* self = qn_alloc_zero_1(JohabFont);

	self->tex = tex;
	self->width = (float)tex->width;
	self->height = (float)tex->height;

	self->per_ascii = tex->width / (font_base_size / 2);
	self->per_johab = tex->width / font_base_size;

	const int ascheight = (0x7E - 0x20 + 1) * (font_base_size / 2) / tex->width;
	self->johab_pos = qm_point(0, ascheight + 1);
	self->uv_size = qm_vec2(16.0f / self->width, 16.0f / self->height);

	self->base.size = font_display_size == 0 ? font_base_size : font_display_size;
	self->base.color.v = QMKOLOR_WHITE;
	self->base.flags = QGFONTTYPE_JOHAB;

	static const QN_DECL_VTABLE(QGFONT) vt_qg_johab =
	{
		{
			VAR_CHK_NAME,
			_johab_dispose,
		},
		_johab_set_size,
		_johab_draw,
		_johab_calc,
	};
	return qn_gam_init(self, vt_qg_johab);
}

// 기본 글꼴 만들기
QgFont* _create_default_font(void)
{
	QgImage* image = _load_image_hxn(default_font, sizeof(default_font), NULL, NULL);
	return _johab_create(image, 32);
}


//////////////////////////////////////////////////////////////////////////
// 아틀라스 글꼴

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"AtlasFont"

typedef struct ATLAS_GLYPH
{
	QMVEC				coord;
	byte				width;
	byte				height;
	char				off_x;
	char				off_y;
	byte				step;
} AtlasGlyph;

// 아틀라스 글꼴
typedef struct ATLASFONT
{
	QN_GAM_BASE(QGFONT);

	QgTexture*			tex;
	AtlasGlyph*			glyphs;
	AtlasGlyph**		index;
} AtlasFont;

//
static void _atlas_set_size(QnGam g, int size)
{
	AtlasFont* font = qn_cast_type(g, AtlasFont);
	if (font->base.size == size)
		return;
	font->base.size = QN_CLAMP(size, 8, 256);
}

//
static AtlasGlyph* _atlas_get_glyph(AtlasFont* self, int code)
{
	AtlasGlyph* value = self->index[code];
	return value != NULL ? value : self->index['?'];
}

//
static void _atlas_draw(QnGam g, const QmRect* bound, const char* text)
{
	AtlasFont* self = qn_cast_type(g, AtlasFont);
	QmPoint pt = qm_point(bound->Left, bound->Top);
	QmKolor color = self->base.color;
	const QmSize step = self->base.step;
	const int size = self->base.size;
	char clrbuf[16];
	int i;
	AtlasGlyph* h;

	while (*text)
	{
		int len;
		const int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				h = _atlas_get_glyph(self, ' ');
				pt.X += h->step + step.Width;
				break;
			case '\n':
				pt.X = bound->Left;
				pt.Y += size + step.Height;
				if (pt.Y + size + step.Height > bound->Bottom)
					goto pos_exit;
				break;
			case '\t':
				h = _atlas_get_glyph(self, ' ');
				pt.X += (h->step + step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					const char ch = *(text + i);
					if (ch == '\0')
						goto pos_exit;
					if (ch == '\a')
						break;
					clrbuf[i] = ch;
				}
				if (i == 0)
				{
					text++;
					color = self->base.color;
				}
				else
				{
					text += i + 1;
					clrbuf[i] = '\0';
					color = _font_string_color(qn_cast_type(self, QgFont), clrbuf, i);
				}
				break;
			default:
				h = _atlas_get_glyph(self, code);
				if (h != NULL)
				{
					if (pt.X + h->step >= bound->Right)
						break;
					QmRect rect = qm_rect_size(pt.X + h->off_x, pt.Y + h->off_y, h->width, h->height);
					qg_draw_glyph(&rect, self->tex, color, &h->coord);
					pt.X += h->step + step.Width;
				}
				break;
		}
	}

pos_exit:
	return;
}

//
static QmPoint _atlas_calc(QnGam g, const char* text)
{
	AtlasFont* self = qn_cast_type(g, AtlasFont);
	QmPoint ret = qm_point(0, 0);
	QmPoint pt = qm_point(0, 0);
	const QmSize step = self->base.step;
	const int size = self->base.size;
	int i;
	AtlasGlyph* h;

	while (*text)
	{
		int len;
		const int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				h = _atlas_get_glyph(self, ' ');
				pt.X += h->step + step.Width;
				break;
			case '\n':
				if (ret.X < pt.X)
					ret.X = pt.X;
				pt.X = 0;
				pt.Y += size + step.Height;
				break;
			case '\t':
				h = _atlas_get_glyph(self, ' ');
				pt.X += (h->step + step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					const char ch = *(text + i);
					if (ch == '\0')
						goto pos_exit;
					if (ch == '\a')
						break;
				}
				if (i == 0)
					text++;
				else
					text += i + 1;
				break;
			default:
				h = _atlas_get_glyph(self, code);
				pt.X += h->step + step.Width;
				break;
		}
	}

pos_exit:
	if (ret.X < pt.X)
		ret.X = pt.X;
	ret.Y = pt.Y + size + step.Height;
	return ret;
}

//
static void _atlas_dispose(QnGam g)
{
	AtlasFont* self = qn_cast_type(g, AtlasFont);
	qn_free(self->index);
	qn_free(self->glyphs);
	qn_unload(self->tex);
	_font_dispose(self);
}

//
static QgFont* _atlas_create(QgImage* image, int atlas, HxnAtlas* atlas_data)
{
	const int font_base_size = image->extra;
	QgTexture* tex = qg_create_texture(NULL, image, QGTEXF_DISCARD_IMAGE | QGTEXF_CLAMP);
	qn_return_when_fail(tex != NULL, NULL);

	AtlasFont* self = qn_alloc_zero_1(AtlasFont);

	self->glyphs = qn_alloc(atlas, AtlasGlyph);
	int max_code = 0;
	QMVEC dcrd = qm_vec_div(QMCONST_ONE.s, qm_vec((float)tex->width, (float)tex->height, (float)tex->width, (float)tex->height));
	for (size_t i = 0; i < (size_t)atlas; i++)
	{
		HxnAtlas* s = &atlas_data[i];
		AtlasGlyph* h = &self->glyphs[i];
		h->coord = qm_vec_mul(qm_vec((float)s->x, (float)s->y, (float)(s->x + s->width), (float)(s->y + s->height)), dcrd);
		h->width = s->width;
		h->height = s->height;
		h->off_x = s->x_offset;
		h->off_y = s->y_offset;
		h->step = s->x_offset + s->x_advance;
		if (max_code < s->code)
			max_code = s->code;
	}

	self->index = qn_alloc_zero(max_code + 1, AtlasGlyph*);
	for (size_t i = 0; i < (size_t)atlas; i++)
	{
		HxnAtlas* s = &atlas_data[i];
		self->index[s->code] = &self->glyphs[i];
	}

	self->tex = tex;
	self->base.size = font_base_size;
	self->base.color.v = QMKOLOR_WHITE;
	self->base.flags = QGFONTTYPE_ATLAS;

	static const QN_DECL_VTABLE(QGFONT) vt_qg_atlas =
	{
		{
			VAR_CHK_NAME,
			_atlas_dispose,
		},
		_atlas_set_size,
		_atlas_draw,
		_atlas_calc,
	};
	return qn_gam_init(self, vt_qg_atlas);
}


//////////////////////////////////////////////////////////////////////////
// 글꼴 공통

//
QgFont* qg_load_font_buffer(void* data, int data_size, int cjk)
{
	VAR_CHK_IF_NULL(data, NULL);
	VAR_CHK_IF_ZERO(data_size, NULL);

	const int offset_index = stbtt_GetFontOffsetForIndex(data, 0);
	if (offset_index >= 0)
	{
		// 트루타입
		return _truetype_create(data, data_size, offset_index);
	}

	// HXN
	int atlas;
	HxnAtlas* atlas_data;
	QgImage* image = _load_image_hxn(data, data_size, &atlas, &atlas_data);
	qn_return_when_fail(image != NULL, NULL);

	QgFont* font;
	if (atlas == 0)
	{
		// 조합형 글꼴
		font = _johab_create(image, 0);
	}
	else
	{
		// 아틀라스 글꼴
		font = _atlas_create(image, atlas, atlas_data);
		qn_free(atlas_data);
	}
	qn_free(data);

	return font;
}

//
QgFont* qg_load_font(int mount, const char* filename, int cjk)
{
	VAR_CHK_IF_NULL(filename, NULL);

	int size;
	byte* data = qn_file_alloc(qg_get_mount(mount), filename, &size);
	qn_return_when_fail(data != NULL, NULL);

	QgFont* font = qg_load_font_buffer(data, size, cjk);
	if (font == NULL)
	{
		qn_free(data);
		return NULL;
	}

	return font;
}

//
bool qg_font_add_buffer(QgFont* font, void* data, int size)
{
	VAR_CHK_IF_NULL(data, false);
	VAR_CHK_IF_ZERO(size, false);

	if (QN_TMASK(font->flags, QGFONTTYPE_TRUETYPE))
	{
		const int offset_index = stbtt_GetFontOffsetForIndex(data, 0);
		if (offset_index < 0)
			return false;
		return _truetype_add(font, data, size, offset_index);
	}

	return false;
}

//
bool qg_font_add(QgFont* font, int mount, const char* filename)
{
	VAR_CHK_IF_NULL(filename, false);

	int size;
	byte* data = qn_file_alloc(qg_get_mount(mount), filename, &size);
	qn_return_when_fail(data != NULL, false);

	if (qg_font_add_buffer(font, data, size))
		return true;

	qn_free(data);
	return false;
}

//
void qg_draw_text(int x, int y, const char* text)
{
	QgFont* font = RDH->font;
	qn_return_when_fail(font != NULL, );
	qg_font_write(font, x, y, text);
}

//
void qg_draw_text_format(int x, int y, const char* fmt, ...)
{
	QgFont* font = RDH->font;
	qn_return_when_fail(font != NULL, );
	va_list va;
	va_start(va, fmt);
	char buffer[1024];
	const int len = qn_vsnprintf(buffer, QN_COUNTOF(buffer), fmt, va);
	va_end(va);
	if (len <= 0)
		return;
	qg_font_write(font, x, y, buffer);
}
