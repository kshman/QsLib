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
	ullong r = (ullong)(qm_vec_get_x(color) * 65535) << 32;
	ullong g = (ullong)(qm_vec_get_y(color) * 65535) << 16;
	ullong b = (ullong)(qm_vec_get_z(color) * 65535);
	ullong a = (ullong)(qm_vec_get_w(color) * 65535) << 48;
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
	// DDS 헤더
	const struct DDS_HEADER
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
		struct DDS_PIXELFORMAT
		{
			uint dwSize;
			uint dwFlags;
			uint dwFourCC;
			uint dwRGBBitCount;
			uint dwRBitMask;
			uint dwGBitMask;
			uint dwBBitMask;
			uint dwABitMask;
		} ddspf;
		uint dwCaps1;
		uint dwCaps2;
		uint dwReserved2[3];
	} *header;

	// 헤더 읽기
	if (size < sizeof(struct DDS_HEADER))
		return false;
	header = (const struct DDS_HEADER*)data;
	qn_return_when_fail(header->magic == 0x20534444, false);

	// 픽셀 포맷
	const QgPropPixel* prop = qg_get_prop_pixel(
		header->ddspf.dwFourCC == 0x31545844 ? QGCF_DXT1 :
		header->ddspf.dwFourCC == 0x33545844 ? QGCF_DXT3 :
		header->ddspf.dwFourCC == 0x35545844 ? QGCF_DXT5 : QGCF_UNKNOWN);
	qn_return_when_fail(prop != NULL, false);

	// 진행
	data += sizeof(struct DDS_HEADER);
	size -= sizeof(struct DDS_HEADER);
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
	image->data = qn_alloc(size, byte);
	memcpy(image->data, data, size);

	return true;
}

// ETC (PKM) => BIG ENDIAN을 쓴다!
static bool image_loader_etc_pkm(const byte* data, uint size, QgImage* image)
{
	// PKM 헤더
	const struct PKM_HEADER
	{
		uint magic;
		ushort version;
		ushort type;
		ushort width;
		ushort height;
		ushort width_orig;
		ushort height_orig;
	} *header;

	// 헤더 읽기
	if (size < sizeof(struct PKM_HEADER))
		return false;
	header = (const struct PKM_HEADER*)data;
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
	data += sizeof(struct PKM_HEADER);
	// 크기
	const int w = ((header->width & 0xFF) << 8) | ((header->width & 0xFF00) >> 8);
	const int h = ((header->height & 0xFF) << 8) | ((header->height & 0xFF00) >> 8);
	image->width = w;
	image->height = h;
	image->pitch = 0;
	image->mipmaps = 1;
	image->prop = *prop;

	// 읽기
	const size_t size_total = w * h * prop->bpp / 8;
	image->data = qn_alloc(size_total, byte);
	memcpy(image->data, data, size_total);

	return true;
}

// ETC (KTX) 그 KTX가 아니다
static bool image_loader_etc_ktx(const byte* data, uint size, QgImage* image)
{
	// KTX 헤더
	const struct KTX_HEADER
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
	} *header;

	// 헤더 읽기
	if (size < sizeof(struct KTX_HEADER))
		return false;
	header = (const struct KTX_HEADER*)data;
	if (header->endianness != 0x04030201)
		return false;

	// 픽셀 포맷
	const QgPropPixel* prop = qg_get_prop_pixel(
		header->glinternalformat == 0x8D64 ? QGCF_EXT1 :
		header->glinternalformat == 0x9278 ? QGCF_EXT2 :
		header->glinternalformat == 0x9274 ? QGCF_EXT2_EAC : QGCF_UNKNOWN);
	qn_return_when_fail(prop != NULL, false);

	// 진행
	data += sizeof(struct KTX_HEADER) + header->keypairbytes;
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
	image->data = qn_alloc(size_total, byte);
	memcpy(image->data, data, size_total);

	return true;
}

// ASTC
static bool image_loader_astc(const byte* data, uint size, QgImage* image)
{
	// ASTC 헤더
	const struct ASTC_HEADER
	{
		byte magic[4];
		byte blockdim_x;
		byte blockdim_y;
		byte blockdim_z;
		byte xsize[3];
		byte ysize[3];
		byte zsize[3];
	} *header;

	// 헤더 읽기
	if (size < sizeof(struct ASTC_HEADER))
		return false;
	header = (const struct ASTC_HEADER*)data;
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
	data += sizeof(struct ASTC_HEADER);
	// 크기
	const int w = header->xsize[0] | header->xsize[1] << 8 | header->xsize[2] << 16;
	const int h = header->ysize[0] | header->ysize[1] << 8 | header->ysize[2] << 16;
	image->width = w;
	image->height = h;
	image->pitch = 0;
	image->mipmaps = 1;
	image->prop = *prop;

	// 읽기
	const size_t size_total = w * h * bpp / 8;
	image->data = qn_alloc(size_total, byte);
	memcpy(image->data, data, size_total);

	return true;
}

// HXN
static bool image_loader_hxn(const byte* data, uint size, QgImage* image)
{
	const struct HXN_HEADER
	{
		uint magic;
		uint type;
		ushort width;
		ushort height;
		uint compressed;
		byte desc[64];
	} *header;

	if (size < sizeof(struct HXN_HEADER))
		return false;
	header = (const struct HXN_HEADER*)data;
	if (header->magic != 0x004E5848)
		return false;

	const QgPropPixel* prop = qg_get_prop_pixel(
		header->type == 0x0000004C ? QGCF_L8 :
		header->type == 0x00004C41 ? QGCF_A8L8 :
		header->type == 0x41424752 ? QGCF_R8G8B8A8 :
		header->type == 0x00424752 ? QGCF_R8G8B8 : QGCF_UNKNOWN);
	qn_return_when_fail(prop != NULL, false);

	image->width = header->width;
	image->height = header->height;
	image->pitch = header->width * prop->tbp;
	image->mipmaps = 1;
	image->prop = *prop;

	data += sizeof(struct HXN_HEADER);
	const size_t size_total = header->width * header->height * prop->tbp;
	if (header->compressed > 0)
		image->data = qn_memzucp_s(data, header->compressed, size_total);
	else
	{
		image->data = qn_alloc(size_total, byte);
		memcpy(image->data, data, size_total);
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
	qn_free(self->data);
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
	self->data = qn_alloc(size, byte);
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
	self->data = data;
	return self;
}

//
QgImage* qg_create_image_filled(int width, int height, const QMVEC* color)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	QmKolor* data = (QmKolor*)self->data;
	QmKolor c;
	qm_vec_st_kolor(&c, *color);
	for (int i = 0; i < width * height; i++)
		data[i] = c;
	return self;
}

//
QgImage* qg_create_image_gradient_linear(int width, int height, const QMVEC* begin, const QMVEC* end, float direction)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	QmKolor* data = (QmKolor*)self->data;
	float sn, cs;
	qm_sincosf(direction, &sn, &cs);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const float f = qm_clampf(((float)x * cs + (float)y * sn) / ((float)width * cs + (float)height * cs), 0.0f, 1.0f);
			const QMVEC c = qm_lerp(*begin, *end, f);
			qm_vec_st_kolor(&data[y * width + x], c);
		}
	}
	return self;
}

//
QgImage* qg_create_image_gradient_radial(int width, int height, const QMVEC* inner, const QMVEC* outer, float density)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	QmKolor* data = (QmKolor*)self->data;
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
			qm_vec_st_kolor(&data[y * width + x], c);
		}
	}
	return self;
}

//
QgImage* qg_create_image_check_pattern(int width, int height, const QMVEC* odd, const QMVEC* even, int sx, int sy)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	QmKolor* data = (QmKolor*)self->data;
	QmKolor oc, ec;
	qm_vec_st_kolor(&oc, *odd);
	qm_vec_st_kolor(&ec, *even);
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
	self->data = stbi_load_from_memory(data, size, &self->width, &self->height, &comp, 0);
	if (self->data != NULL)
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
		return self;
	}
	if (image_loader_dds(data, size, self) ||
		image_loader_etc_pkm(data, size, self) ||
		image_loader_etc_ktx(data, size, self) ||
		image_loader_astc(data, size, self) ||
		image_loader_hxn(data, size, self))
		return self;

pos_exit:
	qg_image_dispose(qn_cast_type(self, QnGamBase));
	return NULL;
}

//
static QgImage* _load_image_hxn(const void* data, int size)
{
	QgImage* self = qg_image();
	if (image_loader_hxn(data, size, self))
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
	byte* ptr = self->data + (ptrdiff_t)((y * self->pitch) + (x * self->prop.tbp));
	switch (self->prop.format)
	{
		case QGCF_R32G32B32A32F:
			((QMVEC*)ptr)[0] = *color;
			break;
		case QGCF_R32G32B32F:
			qm_vec_st_float3((QmFloat3*)ptr, *color);
			break;
		case QGCF_R32F:
			((float*)ptr)[0] = qm_vec_get_x(*color);
			break;
		case QGCF_R16G16B16A16F:
			qm_vec_st_half4((QmHalf4*)ptr, *color);
			break;
		case QGCF_R16F:
			((half_t*)ptr)[0] = qm_f2hf(qm_vec_get_x(*color));
			break;
		case QGCF_R11G11B10F:
			qm_vec_st_f111110((QmF111110*)ptr, *color);
			break;
		case QGCF_R16G16B16A16:
			pixel_color_to_r16_g16_b16_a16((ullong*)ptr, *color);
			break;
		case QGCF_R16:
			((ushort*)ptr)[0] = (ushort)(qm_vec_get_x(*color) * 0xFFFF);
			break;
		case QGCF_R10G10B10A2:
			qm_vec_st_u1010102((QmU1010102*)ptr, *color);
			break;
		case QGCF_R8G8B8A8:
			qm_vec_st_kolor((QmKolor*)ptr, *color);
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
			qm_vec_st_u565((QmU565*)ptr, *color);
			break;
		case QGCF_R5G5B5A1:
			qm_vec_st_u5551((QmU5551*)ptr, *color);
			break;
		case QGCF_R4G4B4A4:
			qm_vec_st_u4444((QmU4444*)ptr, *color);
			break;
		default:
			return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
// 기본 글꼴 데이터

static const uint default_font[] = {
	0x004E5848, 0x00004C41, 0x02000200, 0x00001D56, 0x746E6168, 0x206D7265, 0x61686F6A, 0x677A6E62,
	0x00003631, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x40000000, 0x990B9DEC, 0x8616BCEA, 0x1801002B, 0x30060140,
	0xE01C0280, 0x18580B00, 0x2ED8980D, 0x4CDACD46, 0xF72DE9AF, 0xF3BC2934, 0x1FECE73E, 0x3574DA52,
	0x2B6E565F, 0x00004A95, 0x00000000, 0xFE0578DC, 0xFDBF26FE, 0xFFEF5E39, 0x805361BE, 0xFDFDB555,
	0x5352BCBD, 0xEF7F9470, 0x5FA885FF, 0x3E7CACE4, 0x93DF3FBF, 0xAE57FAE7, 0xA5D82594, 0x5C7507FE,
	0x7E78E5DF, 0xFF5039C4, 0xDFE35132, 0xEA51D737, 0x7A8A1E5D, 0xF0E71A9B, 0xE77A74FE, 0x6ACA4647,
	0x1455F9BE, 0x5DE89A7E, 0xBEBC4A66, 0xB5243BBF, 0x2EB760E1, 0x7EFEA77F, 0x77F28DFE, 0x486EEFAF,
	0x5FC7B7BF, 0xFEAFCFFB, 0x4878FFFF, 0x8FD437F7, 0x737DBECF, 0xD9EFE196, 0x5F3D44E7, 0xD59259F3,
	0x9FFCD34E, 0xD6A18F6A, 0xFD51B786, 0x77FDED2B, 0xBE7787E9, 0xD5FEFEFF, 0xE7EEEA36, 0xF0CFB7A7,
	0x7CFC5F77, 0x94DB1F7A, 0xBA5F5397, 0x93CEE39A, 0x2812D5E3, 0xFC70FA6D, 0xCE256DAB, 0xFBCFC77D,
	0xF8E51FDF, 0x4BB8E8E7, 0xC997FF7B, 0x3C7775FD, 0x724F3F4E, 0x9F77A5DF, 0x9BFB93EF, 0xBDA3CFDE,
	0xF7FEF57A, 0x39BAE4A9, 0x37FFDFCF, 0xFEF0FD25, 0x351FD2F7, 0x9796AF1C, 0x97FE82F3, 0xFF58D8FD,
	0x7AB3F7F7, 0x3CFFEB06, 0xFFBDFFD1, 0x7FB5A974, 0xA8FD736B, 0xFEFC3DF5, 0xB71E7EA7, 0x3E98BEBE,
	0xE8E8F3B3, 0x3EFE23A5, 0x7EA9351D, 0xB7ADE875, 0x4E7EA3DF, 0xAF7E7DBF, 0xEF8FF9CA, 0xF8E79B23,
	0xE7FD2FAF, 0xF1DD2356, 0x5FFE918F, 0xA9CBB693, 0xB6E7812E, 0xF6EEEB3E, 0xD4F7FEF8, 0x2DADCBDE,
	0xD7F7DD07, 0x9B4BB3F6, 0xC7CEF2EA, 0x5C13C579, 0xFE87CF94, 0xFDFBBA7E, 0x43DB97D7, 0xB7774DFF,
	0xFB87AD83, 0xD2BEF7FD, 0xD066FEB1, 0xF5AEA1EF, 0x66CF8B0E, 0x5B50F9FC, 0x62FF50E3, 0xCF6BFF59,
	0xA1FFAA77, 0xFFEA98BE, 0x3872E755, 0x59CEB7FE, 0xEFB3B9F3, 0x70EFB7B5, 0x9FCDB9DC, 0xEF5DDE9A,
	0xE75E793C, 0xF21E53D1, 0xA5DFCD30, 0x28D4DCFF, 0x4AE8E943, 0x1CFCB8CC, 0x271EEBDA, 0xDED1FFCF,
	0x2D84BCC5, 0xFD5400B5, 0x2B746737, 0x77FD734D, 0x50BEFBFE, 0xAB94DFBF, 0xF63FFA75, 0x5235EEE5,
	0x37FCFCFE, 0xDBCDACA5, 0xFEDE8D38, 0x5A54F96C, 0x1FFAA104, 0xA949FB4A, 0x6FE5757F, 0xF70D4B5A,
	0xAC568CDC, 0xF477FFEB, 0x771ED4F9, 0xA0DA9FEB, 0x0F456AF7, 0xF8D2F6D3, 0x973EA1F3, 0x3E99F6B4,
	0xFF5BC69F, 0x9EDC71E5, 0xAA7E14EA, 0xDE7F365B, 0xF39ED5FC, 0xC7F7D6D0, 0x265FFD45, 0x52B3B32D,
	0xF42CDC2F, 0x56949B7F, 0x145FFD5F, 0xFFF54DFB, 0xA9F45B78, 0x2DAF7F1C, 0xEFC7DDDC, 0xE79B7E4A,
	0xB7E9A836, 0xFEFCF9D5, 0xB56F1A6E, 0x5A9D3F54, 0xBB6A1F3D, 0xFFBADBDF, 0xA6E4FADE, 0x6EF60DE4,
	0x4A8E7FBE, 0x977479CD, 0xA32612D3, 0xFFAA7E1A, 0x9F7FED7B, 0xD1002D4E, 0xE755E76A, 0xD5B4A2E1,
	0xB4DAB2D5, 0xDFF5FE9E, 0x69E93574, 0x2FBA06A5, 0xFD0655F6, 0xFCBAEBDF, 0xFA36454F, 0xFA4FD79F,
	0xF963541E, 0xCAE899E6, 0x6F11EA9F, 0xDABE8B6F, 0x86F574DC, 0xEFE6FFAA, 0xBBF3FBD1, 0xB51AB9F2,
	0xA81C9A3A, 0xC4D3A3FD, 0x906D751F, 0x65BC68E3, 0xDBAD7CF8, 0xC2BEFABB, 0x1AFE9DE1, 0xBD1E28F5,
	0xF5D73B37, 0x98FBCFF3, 0xFAD7072E, 0x8E73864E, 0xA7D39F5B, 0xC7F79435, 0x6CBD9DA4, 0xBBDFF5CB,
	0x6A30FFE8, 0x7FF54FBF, 0xC3C4DBA9, 0x9AFAC7F1, 0x5F74E7F3, 0xBD34FFD0, 0xB7A9DFCB, 0xB3F16358,
	0xEA7FFEB5, 0x17F3A2FA, 0xFD9EBAF7, 0x5F4FC753, 0x3AA36F06, 0x0E0FABA5, 0x4965A58B, 0xC50573FF,
	0xE9AFFFEC, 0xC794ACFF, 0x5597BEDF, 0xF4D777D5, 0xD8ED2E90, 0xED7F1FE0, 0xE8CAEBCE, 0x42AD3FDF,
	0xB50B754F, 0xBE9DBEB3, 0x577FBD6D, 0x3E267E3E, 0xC6FE5A75, 0x8BDAA7A9, 0x7BCF473C, 0xBED70678,
	0x6FB5D8DD, 0x18670BAF, 0xB57FFBEA, 0x8DB5CE9D, 0x9B83F186, 0x47C71B65, 0x6E5943FF, 0xEB63669E,
	0xEF5E5F57, 0xDAA71D6D, 0x9E39EFE2, 0x7E3FF52C, 0xDDCFE33A, 0x5FDA8968, 0x4D6FF69E, 0xF5D47FFA,
	0xFFAAB565, 0xD42BED43, 0xAFA7E2FA, 0x7FF4D7DF, 0xF21F495E, 0xE78D1D77, 0xEA9FC8C8, 0xC76B7FF4,
	0xBB6D6E23, 0x79E7AFE5, 0x8C33AFAF, 0x14DB8C04, 0xBED949B7, 0x25FF6D2B, 0xF6661FFD, 0x4BFFD50A,
	0xA4B47A33, 0x82DF57BF, 0xE4E5D539, 0xF8D075AF, 0x735D6791, 0x7E1F6B3D, 0xAEA3555A, 0xFB6D7FFF,
	0xFD68B747, 0x9B52A7FF, 0xCFFFB4DA, 0xC771A30B, 0xF6D3F75E, 0x7FD179F6, 0x6A0E23A8, 0xFFD9626B,
	0x136DFDA9, 0x93FFEDEE, 0xFF8DFF72, 0x7FFE2A7F, 0xE7F71BF8, 0xAEA9769B, 0x6A777FFD, 0xC716BFFF,
	0xFE9F96CB, 0x1CC3A75C, 0xCD59FCF1, 0xDAD70BDB, 0xFFF6D32E, 0x737C617D, 0x661FFFD9, 0x2A34E025,
	0x1FFD3ACF, 0x73A9FD47, 0xBCBD92DD, 0x6FED4AEF, 0x73FD279D, 0xCF968CD1, 0x2B54BFF5, 0xDA7FE887,
	0xEB5E9CDE, 0x6BB0E75E, 0x1CEB1EFF, 0xA077469D, 0x1EA7FFEA, 0x88CD11ED, 0xBFF8F2FE, 0xA2E27DBE,
	0xA705B1FF, 0x6DBF2163, 0xE72B22FD, 0x57FA4E7F, 0xE9643B43, 0x4BE37409, 0x4C3FFF9D, 0xCB7DAB3E,
	0xF07F7B69, 0xEA6BE3C9, 0xF8DB879F, 0x3AAC775F, 0xFA54733B, 0xAAF1D3FF, 0xE377FFC1, 0x6E6DBF18,
	0xECF3F58E, 0x8C9F71F4, 0xFFC699A7, 0xF83FF56C, 0xC8F8F1E5, 0x71BC6575, 0x5D55FF59, 0xFFC6EA43,
	0x63B32DA6, 0xF68604FB, 0xFF9B6FDD, 0xF1DD7D4B, 0xEF38ECF8, 0xD60CC9B7, 0x39D6A53F, 0xAF93E3B6,
	0xD4FF79FE, 0x7943EAF5, 0xFDF3E326, 0xCD9F8D9B, 0x94987FF5, 0x9FFD30E9, 0xF4EAFAB4, 0xABFB1C3F,
	0x2D0D66CC, 0x877EC69A, 0xD4FC3FB6, 0xCD2C41CC, 0xF31CDFFF, 0x9FBEB27B, 0x74C848E9, 0x1EDA6FFE,
	0x6DFE7E9D, 0x2B2EBF67, 0x3475F035, 0xDF9F343F, 0xDB9D969B, 0xDD67F8E4, 0x9E6938B7, 0xBF70D9C7,
	0xD42DEB36, 0x591D5ABA, 0x0D57CE9D, 0x788DDFF7, 0xBFF9DB1C, 0xCD335EA1, 0x52FB4DFF, 0x35C78A3F,
	0xE78DFFCF, 0xFD4ACF5F, 0xB8BF34B4, 0x66A7FFA6, 0x693CEFFE, 0xE64EFE1D, 0x1EA52B6B, 0x592DD53F,
	0xA9BACC99, 0xEF3F98E7, 0x47F468C7, 0x6BDFB6EA, 0x187A9E32, 0xFAE3F8D5, 0xB7FF3F4F, 0x1BCFAA54,
	0xD439FFAE, 0xD063D77D, 0xFFD6C239, 0xCADBAD9D, 0x484CF395, 0xAFDCB889, 0x99FBF3CD, 0x9DBFBEF4,
	0x17D5C24B, 0x0DFFD6A9, 0xFEBF5AE5, 0x59F8C62F, 0xA1BFF5FC, 0x7DFF87AB, 0x65BCF955, 0x23E7F7B6,
	0x11DF40AC, 0x4DD5C777, 0x57982FDC, 0x00760A87, 0x00000000, 0xDFE38F80, 0x287D0AC2, 0x8464C799,
	0x5C3ABC77, 0xA7001F18, 0x5E090ED1, 0x939FA36B, 0x1BE6687A, 0xAD3FCE96, 0xEF19D491, 0xADF27AE9,
	0x63283B77, 0xFA24EFFE, 0xE6CC3635, 0xF00CFD81, 0xA628823C, 0xF5DAAFFE, 0x7FE74B2A, 0xD55FE64C,
	0xE3F37015, 0xBFF599EF, 0x868EDAB5, 0xE063E878, 0xBEF820FA, 0xA133E4F6, 0xF6DA1FFE, 0xD07AABA8,
	0xBA8B10FF, 0xE2DB839C, 0x3600309E, 0xBAF5DB3C, 0x8D6CBE87, 0x76A7EF5B, 0x7B4FD185, 0x3ADD1744,
	0xAF1E08AC, 0x57AE9DA7, 0x64BF62EC, 0xD2ABFE72, 0x0AFB557F, 0xC7EAC9F7, 0xD3869F61, 0xF5CFFE82,
	0xF57FCCFA, 0xDDF5BA59, 0xE7F9D1F7, 0xA31F7DEB, 0x28DBDCF8, 0xB7F6E77F, 0x2E91B7D2, 0x79D00AED,
	0x0F5A8FFC, 0xEA7595DE, 0xEDAE75DD, 0xA3137DDC, 0xE7C6D3F6, 0x07FFF10C, 0x7532D751, 0xFBE7482B,
	0x2B669CE1, 0xA7FAE724, 0x3D210FFD, 0x67DE9F82, 0xC349A337, 0xBADFFB1A, 0x6BB6386E, 0xE9B5BBD4,
	0xF49D654F, 0xF741F15D, 0xE9E5FAC9, 0xC7F9DDF1, 0x15F36EF1, 0x5FFD53CE, 0xFAC7E394, 0x070F3A97,
	0xF3E33F45, 0x67DE24CB, 0x56A77F7C, 0xB77A43E7, 0x8DB3E11B, 0xDE4D10D0, 0x94AFFAE6, 0xDF469238,
	0xEC063AAF, 0x3603C7DC, 0xA25179BD, 0x1C7CBAC1, 0xDBF1E794, 0x21BB8F3F, 0x00C76B07, 0xAB2D8ED8,
	0xFDE36A63, 0x38934E11, 0x9D5FCEBE, 0x39AFF617, 0x7A79A82E, 0x275F7CFC, 0x45FFFD1B, 0xF5575FA3,
	0xCD2744DF, 0xF4BBBBE3, 0x6C8C68E3, 0xFCF37F7C, 0x5C1DFB9D, 0xFBDB6D4E, 0x95D78B4F, 0xFAE648DD,
	0x5DF1B67F, 0xAF017EDD, 0xAAEA823A, 0x18F290EC, 0x4E89B5A9, 0x70768DBE, 0x1D4EC71A, 0x91FFFC2D,
	0x735E0C92, 0xB9D94D01, 0xEC3FF4CF, 0x791C7163, 0x75FEB99A, 0x08F74AF1, 0x8EFE6FFA, 0x793FFA5E,
	0xD63A7FE9, 0x6449ECD1, 0xFF6EFFA7, 0xC93F61B4, 0x93FBBD47, 0x8C93E3BA, 0xDC9BF222, 0xFC6B14DF,
	0x5FFFEA73, 0x2783FFE9, 0x91AF7AB7, 0xCC2BBD63, 0x5D9AB650, 0xFB4A0E24, 0xCFF652BF, 0x6FE7B69B,
	0x525AD7AD, 0xEEF1D2F3, 0xC4A0FFFE, 0xCA75360B, 0x7FACC6A1, 0x78FFF69B, 0xFEA9CBBF, 0x5339B38F,
	0xFB0F7BD5, 0xBFE90EB8, 0x7378F47D, 0xED3FB774, 0x68F3FE70, 0x28DFE606, 0xEFD9F742, 0xF0DAA40C,
	0x63FF9868, 0x7AFC1FFC, 0xD06ECDFE, 0x2E930AC3, 0x97D8FBBE, 0x78D19FBE, 0x9E3263D4, 0xBCEFEC37,
	0x64C7E6AC, 0xCFAE70BC, 0x69C4F8A5, 0x8CA4FFFF, 0x08DB1DE8, 0xFE9593FA, 0xF77FE34D, 0x3C73FC76,
	0x7252F7BE, 0x3A8EED22, 0x7755BFAA, 0xEFF6EBC8, 0xF7FA2D7A, 0x7EAFFF46, 0xF946E13F, 0xD7009EEA,
	0x5ABC4636, 0xCD33FF19, 0xFF991FFF, 0x38B9C287, 0x8CA557C5, 0x7D9597DA, 0x7FC67273, 0x30407699,
	0x3D63FFAD, 0xB1CC9427, 0xEEDE45DE, 0x57BD53D8, 0xFF08BA6A, 0xAC7FF4B2, 0x7D748877, 0x1EFE09FB,
	0x7AD342F6, 0xF0975A7E, 0x97E3E8E6, 0xBFDEFFE8, 0x00000719, 0xBF600000, 0x5FF9FE2D, 0x5CFBF39A,
	0xE7DD25DF, 0xAFEEF1FA, 0xD325704A, 0xCDA71953, 0xAF4BB48F, 0x92AFE70C, 0xE96D2FDA, 0x75223D0E,
	0xC3E556FA, 0x7DE2DF45, 0xF0963885, 0xAFCFA9B3, 0xE931764C, 0xEE66CD67, 0x4FD9D7E7, 0x8627F78C,
	0x4F8B153B, 0xECB73862, 0xE7E55F50, 0x4DDF9E2F, 0xCFF4AFAD, 0xDBFD7B62, 0x6A2C4C2A, 0x10F4F592,
	0xCFFFA185, 0x968E7FD6, 0x4BFD0E54, 0xFF413BC4, 0xD62FF4F1, 0xFD54FD56, 0xBA7FEBC7, 0x45BF8FB7,
	0x4EF512FF, 0x4AA9F7E2, 0xBB24BFD6, 0xD7FA32EB, 0xE2957D26, 0xFD1ECB5F, 0xE5F40D90, 0xF1F797EB,
	0x6BFC4214, 0xD14BFD3A, 0xBFDC3925, 0xF2E11DC6, 0xFF429D1F, 0xD961DFA5, 0x2B4EFFFA, 0x328E1EF2,
	0x4873C7EC, 0xDCA07EA9, 0x9E3F6C95, 0xFF47FA02, 0x9E3F6ADF, 0x67CFEA01, 0xD87BD2BD, 0x6C3F6D9F,
	0x0001FA8E, 0x00000000, 0x00000000, 0xEF023CC0, 0x0CDA5E9E, 0xE7F1B6C2, 0x6227E3B9, 0xD99561DE,
	0x3EC1957E, 0xDBF6E155, 0x9973F967, 0xBF61CF49, 0xB488F8F0, 0xC7A549F1, 0x4FD8BB4F, 0x76CFD912,
	0xA778B165, 0x2497229B, 0xF7CCBE99, 0x11FEF654, 0x7C0BE3A9, 0xE9052BF4, 0x57FD74AB, 0xFD7FE556,
	0x2AFAD498, 0xFAE1FF5B, 0xB75FEB4A, 0xD2C67F68, 0xFA547D87, 0x84BFE967, 0xEB3DE9F8, 0x7FF694BF,
	0xAFB5FF3C, 0xFCAC67F6, 0xF13FD2E5, 0x37FA61D3, 0xFEAD2BE9, 0x8865B3CB, 0x7F2FB67E, 0x61FFFB31,
	0x7FD5F336, 0x477FFCDA, 0x878DB4FC, 0x46DFB0AD, 0x2BEFFFFD, 0x27EDFE7F, 0xD87FAD48, 0x4F6EFB6F,
	0xE896C3F6, 0x7FA3FD1F, 0x1FE8FF40, 0xABF6C3FD, 0xEC48B09F, 0x3F603FFE, 0x00000000, 0x00000000,
	0xD8000000, 0xFEC4FCD7, 0xCE53F7E3, 0xDF1B6F6A, 0x120B1A11, 0xCC928776, 0x3F61627E, 0x1CD5E591,
	0xB062E976, 0x187116DF, 0x25C14592, 0x9FB927C5, 0x97F213EB, 0xAC51FE3F, 0xBB8627EC, 0xEAFBA5D2,
	0xF97F26CB, 0x8A251FE3, 0x6FC925BE, 0x9CE997D1, 0xE87BFBA1, 0x3F6E67F3, 0x985FB7E5, 0xE7EC6DFE,
	0xFB5B7FAF, 0x31FFF4E5, 0xF2122FF4, 0xFA9CEBFE, 0xE7FDE52F, 0xC94FF6A9, 0xFEDCBF9D, 0x927ED6DF,
	0xE09F7ECC, 0xBBF445FF, 0xCBFF4D28, 0x9D306EBE, 0x73FEAD3B, 0xEE944F8F, 0xBF60F7FF, 0x56FFFD92,
	0x3D94EFBD, 0xCDBE9D77, 0xFDBC84DF, 0xE273F89D, 0x4E5FB1CF, 0x3E28DBFD, 0x777DC3C4, 0x53F11104,
	0x843FFDA9, 0xFD01ECB4, 0x0FF47FA3, 0x7FFD1FE8, 0x7AC40972, 0x015FF665, 0x007FE1FB, 0x00000000,
	0x00000000, 0xDF4CB180, 0x3E6F1CF4, 0x3028CFDE, 0xB250C4A3, 0xFFAEF7B2, 0xC712FA94, 0x118FF7BB,
	0xC41C8090, 0x3D8C039B, 0xAB3E680F, 0xE9F6FAE9, 0x6B8839BE, 0x07EFB8BD, 0x3B3EB18C, 0x94C93E34,
	0xB75BF21E, 0xC7FA7077, 0x4FC686CB, 0x97AFD589, 0x1E9364CE, 0x89537FA2, 0xF30E97D3, 0xFA2127FE,
	0x3C7E3F2A, 0x8A43FC5E, 0xFE389BF8, 0xC93E243B, 0x7E5E1E9C, 0x18FFACF7, 0xC9A2E7F1, 0x4DBA4D83,
	0xABE972FE, 0x6BC7FD6C, 0xDF8AA71F, 0x10DFCBB7, 0x339EECDF, 0x39927C7D, 0x36FED43D, 0xBFC43825,
	0xA4DFE886, 0x0BBDD8BE, 0xD72F4CF1, 0x2FA8DABF, 0x753FFB77, 0xDF787EFC, 0xD0C23EDA, 0x9CB87A35,
	0xFD3F09D1, 0x59ABFC9B, 0xB7DE21BF, 0xB74791F4, 0x7FA9BDF6, 0x1FFB79F0, 0xED8FB784, 0xFEDF49BE,
	0x0F6EFE5C, 0xD6EB17F6, 0x165D4F81, 0x1484FFB2, 0xF0C6875F, 0x7823A1AF, 0x7DE7FDB6, 0xC7878F8E,
	0x5BE8C7EF, 0xD1CCEE7F, 0xE6266F78, 0xF999CA6A, 0x621B26F8, 0xE7D6DCBB, 0xF47FA029, 0xE7FAEDFF,
	0x9F58A439, 0x1F6FD800, 0xF829E7D6, 0xF680A0F7, 0x00000013, 0x00000000, 0x40000000, 0xB917F359,
	0xC7BF77E2, 0x1FDE29BA, 0xA7E9F719, 0x2CAE9F8A, 0xBFFAA7D7, 0xD832B23F, 0x94839CCF, 0x108BB11A,
	0xF45F990E, 0xE5395FCF, 0x7DF8CCAF, 0xB9FB7497, 0x5D34974E, 0x3672D9BA, 0xC2CDD6FA, 0xEF6BEA86,
	0x7EE477FB, 0xF2EA703F, 0xC5FCB31B, 0xFF1A77D2, 0xDCCF0E28, 0xC51CDD27, 0x57288620, 0xEDF2F5DD,
	0xFEFA11DF, 0xF9DC9B53, 0xC58717C7, 0xFAA44BEF, 0x79FC773B, 0xCAC235D8, 0x4AFA56DF, 0xFEF4C563,
	0x1C1F14DF, 0xAB649F12, 0xE3FB5CCF, 0xC20BA398, 0x8D08EFF6, 0xB92FB7DF, 0x43CFE2F9, 0xA7D09FF7,
	0x30BF9787, 0x3F6EE7FD, 0x12FFF6A5, 0xDC972964, 0xF7DF8BA6, 0x897B5B2D, 0xFE392A2E, 0x09FF743C,
	0xFEDE7A73, 0x5FFE56F5, 0x128796A2, 0xFBF12F4B, 0x3B9FF429, 0xE8925CFE, 0x343AFB76, 0x93FFFB67,
	0xDF882EE7, 0xB172518F, 0x8FFA495C, 0x286F8FAD, 0xF46294BD, 0x03F50FBF, 0xEF4BDFFA, 0x9EFF5009,
	0xC24F2F4A, 0x3B62E4BF, 0xF4000FD4, 0x000001AE, 0x00000000, 0xBB600000, 0x7E494631, 0x1FFB54FE,
	0xFE9E8DA4, 0xE269F8AC, 0x9FB7B7CF, 0x9E6CEC6D, 0x9F6C0FFD, 0x30F61ECA, 0x23E54491, 0xC58604AC,
	0x4AC78785, 0xB9C64684, 0xFEEF9FDD, 0x32FA2107, 0xFBC947DF, 0x9FD6841F, 0xDCF7EBA9, 0x4A9833FB,
	0x08E95DAA, 0xFD6847D1, 0x2FFF77B0, 0x6CABEB45, 0xDF89A7FD, 0x9076DF8B, 0xA617ED62, 0xBF508934,
	0xDFFFB37D, 0xD0F7FEBF, 0x5680927F, 0x97F7FA6C, 0xBCE7D04B, 0xDF2F57EF, 0xFEDD97FF, 0xD9BFD70A,
	0xFFF7A5BF, 0x95AF8F65, 0x7B58AFAF, 0x4FB7B884, 0x525EF6E2, 0xEC72BFB4, 0x5C63929F, 0x7EBFD7C5,
	0x76DCFDBB, 0x9CFBF38F, 0xC65FFDBA, 0xFF1A5E98, 0x7FEDE10F, 0xB9BF2B08, 0x6EF668E7, 0xFE8FF43F,
	0x62FCB33F, 0x3FF6D0BF, 0x65FF4436, 0x6CC1FFB7, 0x7F54C941, 0x582C7B60, 0xEA3AFAFA, 0xFEA0004F,
	0x00000004, 0x00000000, 0xEC073FC8, 0x7D6C39FE, 0x661FB627, 0x2FE3D690, 0x8D8BBF78, 0xC3EB4D0D,
	0x8B9FB4CB, 0x4FC55266, 0xF563A298, 0xC773E3AB, 0x8D1357EF, 0x57DF8DC4, 0xFAE7C1EF, 0xCC3174FA,
	0x63D29921, 0x73FE30DB, 0xE99F51B2, 0x3174FAFA, 0xEA497094, 0xFF9F6C7F, 0xE3ED2ABA, 0x7E2D3E77,
	0x9D4A2C4A, 0x9ED6BE8B, 0x357FBFB8, 0xB2B0FDBD, 0xF4BA6678, 0xFC7ADA71, 0x12BEDE80, 0x27BA5F49,
	0x6D5FEBEC, 0x025217D4, 0xD4B5EF4D, 0xAEB4DBA5, 0x1B77EADC, 0x3F521AEF, 0x1B57FAB8, 0xFFCF9BEF,
	0x313778DB, 0xFAF489B6, 0xEAAFF30A, 0x67F37DB9, 0xFAF93F57, 0x897D70FF, 0xF50F4EB6, 0x7FE562AF,
	0xB17F3BC4, 0xDFE76FF4, 0x6DBFF5F5, 0xFE597F84, 0x6985A6DF, 0x7FDD0253, 0x3B7FA76B, 0xAFF1A49B,
	0x29FFF0FB, 0x7FA615F5, 0x9BBF5089, 0x27F5CFEB, 0xD3B74FA4, 0x7DBE9DBF, 0xE9657FFD, 0x4FED5281,
	0xC98ABB64, 0xFF7082DC, 0xBFFC1EED, 0x8CE5FF49, 0xDFE9F91F, 0xFEBE7F46, 0xF3ADFFD5, 0xC6BE8FDB,
	0xFAEEFFF1, 0x69E1F7B6, 0x6BCA77F6, 0xB377937F, 0xBBF1531F, 0x3EFA4B82, 0x9ECE2FF5, 0xDA7FDEC0,
	0xEB72D7F9, 0x7767BD4F, 0x18FDB7D1, 0x072ECFBF, 0xDCB9BE61, 0x3E2937B9, 0x7DCFE587, 0xEA4BF609,
	0x34D6E3FF, 0xA08CB992, 0xC12BCFFF, 0xFD7CAFD8, 0xFF47FA03, 0xFB5C1FE8, 0x3BDFE0FF, 0x3FFEFB07,
	0x00012BAB, 0x00000000, 0xE0000000, 0x88406773, 0xE1B30F3F, 0xCBCFE33E, 0x8F16059A, 0x67D66FF4,
	0x3173F8BC, 0x798B7F5E, 0xA4AB14AC, 0x118BFACC, 0xE5562E7F, 0xC0798B7B, 0xBEFC492A, 0xEBA6BCEE,
	0xEAF3D3EB, 0x62EDA53F, 0xBCDB9FC6, 0x3E3DCD3D, 0xDA02B4D1, 0xBCDA42BC, 0xBE8B8643, 0xFB57F852,
	0x46FA83F7, 0x63EC9A3F, 0xF552BF63, 0xF73F96E5, 0xBF4453FA, 0xBD2E8187, 0xD429BAFC, 0xAFEADABF,
	0x0C4F8CF2, 0xF536FFF7, 0x62DF59B2, 0xFF5F7FA1, 0xDE2E936A, 0x7FAFCBF9, 0x2A4EFD11, 0xAFAFF471,
	0x7F32C55F, 0x583174F3, 0x471E9758, 0x2F18D3FF, 0x30FFAA89, 0xFFEAEAFF, 0x5ED7EF75, 0x57EBFF6E,
	0x48579940, 0xC95779D9, 0xFD3B7FA6, 0x869FF585, 0x7DD9D354, 0x6FA36FF4, 0xA661F0B1, 0xB9E6B827,
	0xE10142C5, 0x79ED0BF6, 0x785DBDD9, 0x4D7A163D, 0x4FF8FAFF, 0xFFA87A73, 0x1DFFA2DF, 0x79587FAB,
	0xF9534FFA, 0x7BAFF509, 0x7FAF9FB0, 0x9B9FB375, 0x7FD1B57F, 0xA399DB5E, 0xA4A0C5B3, 0xDAF1FEA5,
	0xFDA998F8, 0xC33FEEEC, 0xFAE96FFD, 0x539FDB0F, 0xCC749E16, 0x704A32F4, 0x3618FD48, 0xB3B76AB2,
	0x483BA966, 0xA7DFEDD8, 0x0D6DB7D3, 0xF87BD763, 0x2109F9C6, 0x04B02A7E, 0xF75FEB97, 0xF5863B34,
	0x405F97F2, 0xFD1FE8FF, 0x1EEB0943, 0x49F5A5BE, 0x17E53904, 0x00000000, 0x00000000, 0xA3CC0000,
	0x12E93EB9, 0x9D086E78, 0xBA762501, 0x2F9832F4, 0x0BFE827F, 0x63BEE7D5, 0xBF2A4BA7, 0x0851B622,
	0x3E6FCAB9, 0x1D7F997D, 0x76D31DE7, 0xE6A9F4B3, 0x295C53A0, 0xF70DFF74, 0xADBAC7F7, 0x6DAF1E30,
	0x4BF7C536, 0x4E90FF42, 0x7BF0E25F, 0x49A0B7BF, 0xEAD20E60, 0x667ECC2B, 0xE7D0484D, 0xCB53F790,
	0xF5DDEEEC, 0x431FEB6D, 0xD3F0DFF7, 0xE3FBEBAF, 0xD513E3DD, 0x93E3DD6F, 0x94D7F962, 0xFF2712FA,
	0x748EFFE6, 0xBA2837FF, 0xCFF75F79, 0xE6D5FEFE, 0xF5F99FAD, 0x537D62EF, 0x8A24BEFC, 0xAFD3F7DF,
	0xF8A67F59, 0x627C5324, 0xB8527C78, 0x0329B57F, 0x0717FABE, 0xE316B70F, 0xF70F4BA6, 0x1FF569CD,
	0xB1736EA7, 0x7FED76E0, 0xC97DF89F, 0xAEEFBF18, 0x28443DDF, 0xB357FA84, 0xE6DB42FE, 0x99208F65,
	0xBA5CCEDF, 0xAE5AF3FE, 0x30FDF8EC, 0xA11F96FD, 0xB75FEE10, 0xFB2A5B19, 0xF6A6C61D, 0xB5F1BE4D,
	0x7E57AD92, 0x47E7FEFC, 0x153B1118, 0x0B1F4C6B, 0xFB1FE886, 0x1FB17E51, 0x3D1C87FA, 0x6977A230,
	0x7D2CE53F, 0x5DF610D8, 0x0003B18A, 0x00000000, 0x00000000, 0x80CF2C00, 0x4127C4FA, 0x3F81FBF8,
	0xF7570669, 0xF49B47FE, 0x92C43338, 0xFBF53D34, 0x3FE6E360, 0x9B783E2C, 0xFB94FDF8, 0x2FA8DB78,
	0x52AA5BD6, 0xEA87D0CC, 0xF7BAA62B, 0xFD0226B6, 0xB7DF8FA7, 0x56C0F7D9, 0xBFAC955F, 0x631F5221,
	0xB2F18A6A, 0x0C4FB7B7, 0x263ED9A9, 0x9FDCA6FB, 0x8D7FBE22, 0xFBF88D7B, 0xFEBE7C77, 0x63DE22C5,
	0x8FC7FCA9, 0x97D70FDF, 0x55295ACC, 0x176AFF44, 0xFEBA6F3D, 0xFE987A7A, 0xE570D1CB, 0xF19CBEAD,
	0xE3F5E985, 0xEFD36FA3, 0x6336235D, 0xF114FFF9, 0x15F5A9FB, 0xFAB37FA6, 0x76172564, 0xA7BB61E9,
	0xD2E695F9, 0xFFC0EFF6, 0x92FFF7C7, 0xDF4D3D34, 0x36FF4FDD, 0x5F690BFA, 0x41FBBF97, 0x9F75FEC9,
	0xFD6945F1, 0x0FB704AB, 0xA7FFECB6, 0x7E29BF57, 0xB76FAF5A, 0x7767C5DA, 0xDDE8AAD2, 0x5FF6640F,
	0x3DD7F965, 0xFE316746, 0x89FEBDB7, 0xA92D0A9E, 0x6FFD0FD8, 0xFF417ED1, 0x10FFF37B, 0xD6B1FF57,
	0xFF33ECCC, 0x94FEFAE9, 0x7FFD14E0, 0x8000FF40, 0xFD0001FE, 0x07FA0003, 0x000FF400, 0xD2100000,
	0xBF5F097E, 0x06057690, 0x8DBD2FF9, 0x53D35928, 0xF97737DB, 0xE8FA7D09, 0x7FA3FD07, 0x1E9EBFF4,
	0xC9F8BF1A, 0xDD158FFA, 0xEF6D5BA9, 0xD1FE8E9F, 0x3FFED97F, 0xDF427EA5, 0x47FF8CB6, 0xFFA0DFFF,
	0xB67E0530, 0x1D694FFF, 0xF8EFF609, 0xB65FF47F, 0xCD5BB7FF, 0xE76E97EE, 0xE7FFE9BF, 0x99C86D51,
	0xCC3D3024, 0x1E98127E, 0xCFD0816E, 0xFF91FF8C, 0x68BFD24F, 0xFF7B1F15, 0x3078310D, 0x6570F671,
	0xFEF53FEB, 0xFEDC9F25, 0xC5FB47DF, 0xEDAC2CB5, 0x0F4A6C20, 0x99F2EFD5, 0xF33F9B45, 0xEBFA453E,
	0xD3FAF68B, 0xE9F5AF3D, 0xC315FAA1, 0x05B58A1D, 0x2DFFF9E2, 0xFE45497C, 0xA1DFF60B, 0xBF6B0ED7,
	0x4C1FB687, 0xFF514CFA, 0xBEDFF6C6, 0x3F4107ED, 0xFAF77FE8, 0xA87EBB0F, 0x3ED863FE, 0x775DFB65,
	0xFE8FF47D, 0x58A7BCB7, 0xD778F7ED, 0xC7BF6BF7, 0x6DA53FED, 0xF27FA3BF, 0xFEFFBB1F, 0x32912D39,
	0x656C4FD6, 0x8BE79F01, 0xFF3F3E7E, 0x47FF9EFB, 0x21FA15FF, 0xFD1FE8FF, 0x00000003, 0xE419A240,
	0x5BC3F3B4, 0xBFE79FE6, 0x074A74F7, 0xE8FF47FA, 0xFE81D29F, 0xA7FA3FD1, 0xF47FA074, 0x1D29FE8F,
	0xA3FD1FE8, 0x1F0AF37F, 0xA65E15D7, 0xBDA583F5, 0xA3FD0FE1, 0xBFD64FFF, 0xC4162368, 0xDBC4E250,
	0xFFD37B9E, 0xBB57BAFA, 0x537E9374, 0x19CF2FAD, 0xF7FE89FF, 0xFAE7FFE6, 0xB60FD536, 0xFAF53FFE,
	0x5FE754F1, 0xC532A78F, 0x8FADFBA7, 0xDD6FD996, 0xEFF3EDD2, 0xF3D739F7, 0x6FF9E52D, 0x4DC6BF5D,
	0xB1DFF75F, 0x9D9047F7, 0xCFB6D9D5, 0x0BFEDE27, 0x62A3F17E, 0x9B887EDD, 0xDF6D4FFF, 0xFBFFDDED,
	0xFFBD8DDC, 0x211FDFF6, 0x7DA1DC7E, 0x27DE1DFD, 0xCDFFD74F, 0xDA9E1F1D, 0x7FD57BFE, 0xD4A3FDA9,
	0x5FD74FFA, 0xFC4D8BDE, 0x4F21EFDB, 0xBAFED520, 0x40FFED25, 0xB29CBFFF, 0x08FEFDBE, 0xB7BFE2E9,
	0xFFF05BAC, 0xF2EFB465, 0x39F02FEC, 0xD9A4FFFA, 0xFF8FF41D, 0xFFA3FFC7, 0xF66BA8CF, 0x3FD184F5,
	0x000007FA, 0xDF6DF000, 0xCFFBC27E, 0xB39BFF58, 0x65EACBFD, 0xD1FE8FF4, 0xFF47FA3F, 0xA3FD1FE8,
	0xFE8FF47F, 0xDFFA3FD1, 0xDFFDFE87, 0xD1FE8F70, 0x5D2FFA3F, 0x83F14BFF, 0xA6BF29A2, 0x0E13D374,
	0x4BBAE37D, 0xBDAF0A09, 0xFFFA3FD0, 0x1CBFFF2E, 0x9E9F6853, 0xFFC4FF4F, 0x53707FA3, 0xBDF2FC7C,
	0xE26DDFAE, 0xE8CE97AF, 0x3F5B6E92, 0x9907C5AE, 0x1FAFB7D2, 0x213DDB97, 0x8B057AE9, 0x98127721,
	0x7FA9887B, 0xB5D90B53, 0xFDB31B7D, 0x2C7F56EE, 0x872A4FB5, 0xCDFAFB74, 0xF9F6FF4F, 0xFC7FAD4F,
	0xEFB7FA3F, 0x51E8C69F, 0xCBFCE5EC, 0xB27BD7D1, 0x466336CF, 0x6BE7C255, 0x3FC0BDDA, 0x989D1CDD,
	0xEB5C7D36, 0x56BF7FEB, 0xE7A7FBFB, 0x7C79FF7E, 0x2F8FC4FD, 0xA787C515, 0x42DFFF89, 0x263892FF,
	0xE0F4364A, 0x247F6679, 0xBE64BA69, 0xBFD0957E, 0xE1E16EA5, 0x7FEB73FF, 0xFA3C16DB, 0xFFF8FF47,
	0xE8947BF2, 0xC5FE853F, 0xF4DF0670, 0xDB7FEAFD, 0x7D827FF3, 0xF55EFFD0, 0x94399A2F, 0x000074B0,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x28000000, 0xF0F4531D, 0xD33137F2, 0xBA8F22C5,
	0x9BA586FC, 0xA1CF3B7E, 0xDD5DA218, 0xB56397D2, 0x8E519ED7, 0xAE92F7F8, 0xE423195F, 0xF1CDD35D,
	0x7F5DA5F7, 0xA6F8D4BD, 0x5B29BF8F, 0xDB3FB1FF, 0xFB7779BC, 0xB33647FA, 0x8CF978BF, 0x2329E93C,
	0xFAF2C470, 0x6FBF3862, 0xBA0F78FA, 0xFF47D73C, 0xEC87FD66, 0x6E94B626, 0x04ABC9DA, 0xFFE497FD,
	0x737F832B, 0xD7D5374D, 0xF2EFFDA9, 0xB5DB97AE, 0x2CF7A63F, 0x773F971B, 0x9FD6A7FD, 0x48906F9D,
	0x075162E9, 0x697FD7E7, 0xE96897D7, 0xC46FE8E2, 0x74D77E70, 0x7D49D953, 0xB7EFE5FD, 0x7F9A01EC,
	0xC8EB195D, 0xFD737490, 0x63F9DD3B, 0xD27CFEFD, 0xF46269E7, 0xC5D331C0, 0xAFB9FEC1, 0x3ED0F26F,
	0x5FFF66E9, 0x4867FD19, 0x9FFFB9BA, 0x0000FAF6, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0xB0000000, 0x8FE35C17, 0xEDDBA762, 0xFC4A503F, 0xC037FC75, 0x577FC6BB, 0xBBC437C6, 0x76318BA5,
	0x17FBAB61, 0xE617FDCB, 0xFF156CDF, 0x65D20923, 0x22CAA8F5, 0x6DD2F167, 0xA5538FE3, 0x631E638B,
	0xBFD88BA7, 0xA4125917, 0xF42E458B, 0xE9612231, 0xF412B4A6, 0xEBE3FE87, 0x7CFFDDDF, 0x535FFFAF,
	0x18BA4868, 0x263E6D7F, 0xFAEEDD24, 0xBB97F5B9, 0xF945B1FD, 0xCFFE2C97, 0x6D38FE35, 0x1E7DBDFC,
	0xEEDBA563, 0x3F92E2FB, 0xF712F497, 0xEBFFC01F, 0xF97AF7FC, 0xC71749F6, 0xD6FC63F6, 0xF1FC6BB7,
	0xC46385B2, 0x31E4BBDF, 0xFDCBBA5E, 0xC701DCA7, 0xEC5BFECE, 0xB965503F, 0xFF6CC97F, 0x984FFBDC,
	0x3ED6E046, 0x000748C5, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xF8800000, 0x4DDDE188,
	0xEDF0E9A1, 0x3536CBE5, 0xBE8E0C4F, 0x8BA486FC, 0xAEE3AFD3, 0xF5AB173F, 0x2DD4E5E7, 0xDBFFEE79,
	0x4F255FAF, 0xF1FF5B2F, 0x47CCBDE2, 0xC17FD9E7, 0xFD565694, 0xA11BF5B3, 0xE17BEED3, 0xEE531FE6,
	0x7DEB617E, 0x6EA1DEDC, 0xFC43C67E, 0xCFFA8959, 0x7EA9FF1D, 0x91FF6F9F, 0xFFC7D79C, 0x197FD4E6,
	0x253457FE, 0xF4DD2425, 0x7FFC52DB, 0xEEAFE7BA, 0xFC1920FD, 0xF1F5DD8F, 0x2FF4F9FF, 0x629B3EC9,
	0x7515AB85, 0xD374A44A, 0xFFE9CB6F, 0x7336D657, 0x7FB1B749, 0xD6E7EBFE, 0x42DCE57F, 0xFFA74FB0,
	0x0000000F, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x15C8A900, 0x5E9F087F, 0xCED6ECAE,
	0xD4158657, 0xEFAEE9EE, 0xFEF9CE1F, 0x9FEF5CDF, 0x5E90FF45, 0xA7F2E6F9, 0x5864477F, 0x3D6D39BA,
	0xDD24376A, 0x25DCBEF4, 0xD3D1553B, 0xD233226D, 0x6C43FFED, 0xBB4AAD08, 0xDE195FB2, 0x6E9EEE61,
	0xDCD3F6F7, 0x7F194FFA, 0xF20EF7EC, 0x3B42E978, 0x975BF500, 0x9FEF9CDE, 0x4BFD5EFF, 0x9FE8CFC2,
	0xF5B6C01A, 0xF746D6BA, 0xF7A7174F, 0x5997AFCF, 0xF1A5F4FF, 0xFFDAF28A, 0x6A007FF8, 0x00FFFA69,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x007FC81F,
	0xFFFF0000, 0x00000003,
};


//////////////////////////////////////////////////////////////////////////
// 글꼴

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"Font"

//
static void _font_dispose(QnGam g)
{
	QgFont* self = qn_cast_type(g, QgFont);
	qn_free(self->name);
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
	QmSize size = RDH_TRANSFORM->size;
	QmRect bound = qm_rect(x, y, size.Width, size.Height);
	qn_cast_vtable(self, QGFONT)->draw(self, &bound, text);
}

//
void qg_font_write_format(QgFont* self, int x, int y, const char* fmt, ...)
{
	VAR_CHK_IF_NULL(fmt, );
	va_list va;
	va_start(va, fmt);
	char buffer[1024];
	int len = qn_vsnprintf(buffer, QN_COUNTOF(buffer), fmt, va);
	va_end(va);
	if (len <= 0)
		return;

	QmSize size = RDH_TRANSFORM->size;
	QmRect bound = qm_rect(x, y, size.Width, size.Height);
	qn_cast_vtable(self, QGFONT)->draw(self, &bound, buffer);
}

//
static QmKolor _font_string_color(QgFont* self, const char* text, int len)
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
		return QMKOLOR_BLACK;
	if (qn_stricmp(text, "white") == 0)
		return QMKOLOR_WHITE;
	if (qn_stricmp(text, "red") == 0)
		return QMKOLOR_RED;
	if (qn_stricmp(text, "green") == 0)
		return QMKOLOR_GREEN;
	if (qn_stricmp(text, "blue") == 0)
		return QMKOLOR_BLUE;
	if (qn_stricmp(text, "yellow") == 0)
		return QMKOLOR_YELLOW;
	if (qn_stricmp(text, "cyan") == 0)
		return QMKOLOR_CYAN;
	if (qn_stricmp(text, "magenta") == 0)
		return QMKOLOR_MAGENTA;
	if (qn_stricmp(text, "gray") == 0)
		return QMKOLOR_GRAY;
	if (qn_stricmp(text, "lightgray") == 0)
		return QMKOLOR_LIGHTGRAY;
	if (qn_stricmp(text, "darkgray") == 0)
		return QMKOLOR_DARKGRAY;
	return self->color;
}


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
// 트루타입 글꼴

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"TrueType"

// 트루타입 글꼴
typedef struct QGTRUETYPE
{
	QN_GAM_BASE(QGFONT);

	void*				data;
	size_t				data_size;

	stbtt_fontinfo		stbtt;
	float				scale;
	int					ascent, descent, linegap;

	GlyphHash			glyphs;
} QgTrueType;

//
void _truetype_set_size(QnGam g, int size)
{
	QgTrueType* font = qn_cast_type(g, QgTrueType);

	if (font->base.size == size)
		return;

	font->base.size = QN_CLAMP(size, 8, 256);

	font->scale = stbtt_ScaleForPixelHeight(&font->stbtt, (float)font->base.size);
	stbtt_GetFontVMetrics(&font->stbtt, &font->ascent, &font->descent, &font->linegap);
}

//
static QgImage* _truetype_generate_image(byte* bitmap, int width, int height)
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
static GlyphValue* _truetype_get_glyph(QgTrueType* self, int code)
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
	QgImage* img = _truetype_generate_image(bitmap, x0, y0);
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

	return value;
}

//
static void _truetype_draw(QnGam g, const QmRect* bound, const char* text)
{
	QgTrueType* self = qn_cast_type(g, QgTrueType);
	QmPoint pt = qm_point(bound->Left, bound->Top);
	QmKolor color = self->base.color;
	int i, height = self->base.size + self->base.step.Height;
	char clrbuf[16];

	while (*text)
	{
		int len;
		int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				pt.X += self->base.size / 3 + self->base.step.Width;
				break;
			case '\n':
				pt.X = bound->Left;
				pt.Y += height;
				if (pt.Y + height > bound->Bottom)
					goto pos_exit;
				break;
			case '\t':
				pt.X += (self->base.size + self->base.step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					char ch = *(text + i);
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
					GlyphValue* value = _truetype_get_glyph(self, code);
					if (value != NULL)
					{
						if (pt.X + value->offset.X + value->tex->width >= bound->Right)
							break;
						QmRect rect = qm_rect_size(pt.X + value->offset.X, pt.Y + value->offset.Y, value->tex->width, value->tex->height);
						qg_draw_glyph(&rect, value->tex, color, NULL);
						pt.X += value->advance + self->base.step.Width;
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
	QgTrueType* self = qn_cast_type(g, QgTrueType);
	QmPoint ret = qm_point(0, 0);
	QmPoint pt = qm_point(0, 0);
	int i;

	while (*text)
	{
		int len;
		int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				pt.X += self->base.size / 3 + self->base.step.Width;
				break;
			case '\n':
				if (ret.X < pt.X)
					ret.X = pt.X;
				pt.X = 0;
				pt.Y += self->base.size + self->base.step.Height;
				break;
			case '\t':
				pt.X += (self->base.size + self->base.step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					char ch = *(text + i);
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
					GlyphValue* value = _truetype_get_glyph(self, code);
					if (value != NULL)
						pt.X += value->advance + self->base.step.Width;
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
	QgTrueType* self = qn_cast_type(g, QgTrueType);
	_glyph_hash_dispose(&self->glyphs);
	qn_free(self->data);
	_font_dispose(self);
}

// 트루타입 만들기
QgFont* _truetype_create(void* data, int data_size, int font_base_size, int offset_index)
{
	QgTrueType* self = qn_alloc_zero_1(QgTrueType);
	if (stbtt_InitFont(&self->stbtt, data, offset_index) == 0)
	{
		qn_free(self);
		return NULL;
	}

	_glyph_hash_init_fast(&self->glyphs);
	self->base.color = QMKOLOR_WHITE;

	self->data = data;
	self->data_size = data_size;

	_truetype_set_size(qn_cast_type(self, QgFont), font_base_size < 8 ? 8 : font_base_size);

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


//////////////////////////////////////////////////////////////////////////
// 조합형 글꼴

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"Johab"

// 트루타입 글꼴
typedef struct QGJOHAB
{
	QN_GAM_BASE(QGFONT);

	QgTexture*			tex;
	float				width;
	float				height;
	int					half_size;

	int 				per_ascii;
	int 				per_hangul;

	float				step_ascii;
	float				step_hangul;
	float				step_height;
} QgJohab;

//
void _johab_set_size(QnGam g, int size)
{
	QgJohab* font = qn_cast_type(g, QgJohab);

	if (font->base.size == size)
		return;

	font->base.size = QN_CLAMP(size, 8, 256);
	font->half_size = font->base.size / 2;
}

//
void _johab_ascii(QgJohab* self, int code, const QmKolor color, const QmPoint* pt)
{
	qn_debug_verify(code >= 0x20 && code <= 0x7E);

	code = (code - 0x20);
	int x = code % self->per_ascii;
	int y = code / self->per_ascii;
	float u = (float)x * self->step_ascii;
	float v = (float)y * self->step_height;
	QMVEC coord = qm_vec(u, v, u + self->step_ascii, v + self->step_height);
	QmRect rect = qm_rect_size(pt->X, pt->Y, self->half_size, self->base.size);
	qg_draw_glyph(&rect, self->tex, color, &coord);
}

//
static void _johab_draw(QnGam g, const QmRect* bound, const char* text)
{
	QgJohab* self = qn_cast_type(g, QgJohab);
	QmPoint pt = qm_point(bound->Left, bound->Top);
	QmKolor color = self->base.color;
	int i, height = self->base.size + self->base.step.Height;
	char clrbuf[16];

	while (*text)
	{
		int len;
		int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				pt.X += self->half_size / 2 + self->base.step.Width;
				break;
			case '\n':
				pt.X = bound->Left;
				pt.Y += height;
				if (pt.Y + height > bound->Bottom)
					goto pos_exit;
				break;
			case '\t':
				pt.X += (self->half_size + self->base.step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					char ch = *(text + i);
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
				if (code > 0x20 && code <= 0x7E)
				{
					if (pt.X + self->half_size >= bound->Right)
						break;
					_johab_ascii(self, code, color, &pt);
					pt.X += self->half_size + self->base.step.Width;
				}
				break;
		}
	}

pos_exit:
	return;
}

//
static QmPoint _johab_calc(QnGam g, const char* text)
{
	QgJohab* self = qn_cast_type(g, QgJohab);
	QmPoint ret = qm_point(0, 0);
	QmPoint pt = qm_point(0, 0);
	int i;

	while (*text)
	{
		int len;
		int code = (int)qn_u8cbc(text, &len);
		if (code < 0)
			break;
		text += len;

		switch (code)
		{
			case ' ':
				pt.X += self->half_size + self->base.step.Width;
				break;
			case '\n':
				if (ret.X < pt.X)
					ret.X = pt.X;
				pt.X = 0;
				pt.Y += self->base.size + self->base.step.Height;
				break;
			case '\t':
				pt.X += (self->half_size + self->base.step.Width) * 4;
				break;
			case '\a':
				for (i = 0; i < 15; i++)
				{
					char ch = *(text + i);
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
				if (code > 0x20 && code <= 0x7E)
					pt.X += self->half_size + self->base.step.Width;
				else if (code > 0x100)
					pt.X += self->base.size + self->base.step.Width;
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
static void _johab_dispose(QnGam g)
{
	QgJohab* self = qn_cast_type(g, QgJohab);
	qn_unload(self->tex);
	_font_dispose(self);
}

// 조합 만들기 공용
static QgFont* _johab_init(QgJohab* self, QgTexture* tex, int font_base_size)
{
	self->tex = tex;
	self->width = (float)tex->width;
	self->height = (float)tex->height;

	self->per_ascii = tex->width / 8;
	self->per_hangul = tex->width / 16;

	self->step_ascii = 8.0f / self->width;
	self->step_hangul = 16.0f / self->height;
	self->step_height = 16.0f / self->height;

	self->half_size = font_base_size / 2;
	self->base.size = font_base_size;
	self->base.color = QMKOLOR_WHITE;

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

// 조합 만들기
static QgFont* _johab_create(void* data, int data_size, int font_base_size)
{
	QgImage* img = qg_load_image_buffer(data, data_size);
	qn_free(data);
	qn_return_when_fail(img != NULL, NULL);
	QgTexture* tex = qg_create_texture(NULL, img, QGTEXF_DISCARD_IMAGE | QGTEXF_CLAMP);
	qn_return_when_fail(tex != NULL, NULL);
	return _johab_init(qn_alloc_zero_1(QgJohab), tex, font_base_size);
}

// 기본 글꼴 만들기
QgFont* _create_default_font(void)
{
	QgImage* img = _load_image_hxn(default_font, sizeof(default_font));
	QgTexture* tex = qg_create_texture(NULL, img, QGTEXF_DISCARD_IMAGE | QGTEXF_CLAMP);
	return _johab_init(qn_alloc_zero_1(QgJohab), tex, 16);
}


//////////////////////////////////////////////////////////////////////////
// 글꼴 공통

//
QgFont* qg_load_font_buffer(void* data, int data_size, int font_base_size, int cjk)
{
	VAR_CHK_IF_NULL(data, NULL);
	VAR_CHK_IF_ZERO(data_size, NULL);

	int offset_index = stbtt_GetFontOffsetForIndex(data, 0);
	if (offset_index >= 0)
	{
		// 트루타입
		return _truetype_create(data, data_size, font_base_size, offset_index);
	}

	// 한국어 조합형
	if (cjk & 0x1)
	{
		QgFont* johab = _johab_create(data, data_size, font_base_size);
		if (johab != NULL)
			return johab;
	}

	// 다른거는.. 후
	return NULL;
}

//
QgFont* qg_load_font(int mount, const char* filename, int font_base_size, int cjk)
{
	VAR_CHK_IF_NULL(filename, NULL);

	int size;
	byte* data = qn_file_alloc(qg_get_mount(mount), filename, &size);
	qn_return_when_fail(data != NULL, NULL);

	QgFont* font = qg_load_font_buffer(data, size, font_base_size, cjk);
	if (font == NULL)
	{
		qn_free(data);
		return NULL;
	}

	font->name = qn_strdup(filename);
	return font;
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
	int len = qn_vsnprintf(buffer, QN_COUNTOF(buffer), fmt, va);
	va_end(va);
	if (len <= 0)
		return;
	qg_font_write(font, x, y, buffer);
}
