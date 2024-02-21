//
// qg_image.c - 이미지
// 2024-1-30 by kim
//

#include "pch.h"
#include "qs_qg.h"
#include "qg_stub.h"
#include "qs_supp.h"

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


//////////////////////////////////////////////////////////////////////////
// 블릿

INLINE void QM_VECTORCALL pixel_color_to_r5_g6_b5(ushort* v, const QmVec4 color)
{
	v[0] = (ushort)(((ushort)(color.X * 31) << 11) | ((ushort)(color.Y * 63) << 5) | ((ushort)(color.Z * 31)));
}

INLINE void QM_VECTORCALL pixel_color_to_r5_b5_g5_a1(ushort* v, const QmVec4 color)
{
	v[0] = (ushort)(((ushort)(color.X * 31) << 10) | ((ushort)(color.Y * 31) << 5) | ((ushort)(color.Z * 31)) | ((ushort)(color.W * 1) << 15));
}

INLINE void QM_VECTORCALL pixel_color_to_r4_b4_g4_a4(ushort* v, const QmVec4 color)
{
	v[0] = (ushort)(((ushort)(color.X * 15) << 8) | ((ushort)(color.Y * 15) << 4) | ((ushort)(color.Z * 15)) | ((ushort)(color.W * 15) << 12));
}

INLINE void QM_VECTORCALL pixel_color_to_r8_g8_b8(byte* v, const QmVec4 color)
{
	v[0] = (byte)(color.X * 255);
	v[1] = (byte)(color.Y * 255);
	v[2] = (byte)(color.Z * 255);
}

INLINE void QM_VECTORCALL pixel_color_to_r8_g8_b8_a8(uint* v, const QmVec4 color)
{
	v[0] = ((uint)(color.X * 255) << 16) | ((uint)(color.Y * 255) << 8) | ((uint)(color.Z * 255)) | ((uint)(color.W * 255) << 24);
}

INLINE void QM_VECTORCALL pixel_color_to_r10_g10_b10_a2(uint* v, const QmVec4 color)
{
	v[0] = ((uint)(color.X * 1023) << 20) | ((uint)(color.Y * 1023) << 10) | ((uint)(color.Z * 1023)) | ((uint)(color.W * 3) << 30);
}

INLINE void QM_VECTORCALL pixel_color_to_r11_g11_b10_f(uint* v, const QmVec4 color)
{
	// UNDONE: 이거 아님. 실수 계산으로 바꿔야함. 당분간 안쓸거 같으니 냅두자
	v[0] = ((uint)(color.X * 2047) << 21) | ((uint)(color.Y * 2047) << 10) | ((uint)(color.Z * 1023));
}

INLINE void QM_VECTORCALL pixel_color_to_r16_g16_b16(ushort* v, const QmVec4 color)
{
	v[0] = (ushort)(color.X * 65535);
	v[1] = (ushort)(color.Y * 65535);
	v[2] = (ushort)(color.Z * 65535);
}

INLINE void QM_VECTORCALL pixel_color_to_r16_g16_b16_a16(ullong* v, const QmVec4 color)
{
	v[0] = ((ullong)(color.X * 65535) << 32) | ((ullong)(color.Y * 65535) << 16) | ((ullong)(color.Z * 65535)) | ((ullong)(color.W * 65535) << 48);
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
QgImage* qg_create_image_filled(int width, int height, const QmColor* color)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	uint* data = (uint*)self->data;
	uint c;
	pixel_color_to_r8_g8_b8_a8(&c, *color);
	for (int i = 0; i < width * height; i++)
		data[i] = c;
	return self;
}

//
QgImage* qg_create_image_gradient_linear(int width, int height, const QmColor* begin, const QmColor* end, float direction)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	uint* data = (uint*)self->data;
	float sn, cs;
	qm_sincosf(direction, &sn, &cs);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const float f = qm_clampf(((float)x * cs + (float)y * sn) / ((float)width * cs + (float)height * cs), 0.0f, 1.0f);
			const QmVec c = qm_lerp(*begin, *end, f);
			pixel_color_to_r8_g8_b8_a8(&data[y * width + x], c);
		}
	}
	return self;
}

//
QgImage* qg_create_image_gradient_radial(int width, int height, const QmColor* inner, const QmColor* outer, float density)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	uint* data = (uint*)self->data;
	const float r = (float)QN_MIN(width, height) * 0.5f;
	const float cx = (float)width * 0.5f;
	const float cy = (float)height * 0.5f;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const float h = hypotf((float)x - cx, (float)y - cy);
			const float m = qm_clampf((h - r * density) / (r * (1.0f - density)), 0.0f, 1.0f);
			const QmVec c = qm_lerp(*outer, *inner, m);
			pixel_color_to_r8_g8_b8_a8(&data[y * width + x], c);
		}
	}
	return self;
}

//
QgImage* qg_create_image_check_pattern(int width, int height, const QmColor* oddColor, const QmColor* evenColor, int checkWidth, int checkHeight)
{
	QgImage* self = qg_create_image(QGCF_R8G8B8A8, width, height);
	uint* data = (uint*)self->data;
	uint oc, ec;
	pixel_color_to_r8_g8_b8_a8(&oc, *oddColor);
	pixel_color_to_r8_g8_b8_a8(&ec, *evenColor);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < height; x++)
			data[y * width + x] = ((x / checkWidth + y / checkHeight) % 2 == 0) ? oc : ec;
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
		image_loader_astc(data, size, self))
		return self;

pos_exit:
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
bool qg_image_set_pixel(const QgImage* self, int x, int y, const QmColor* color)
{
	qn_return_when_fail((size_t)x < (size_t)self->width && (size_t)y < (size_t)self->height, false);
	byte* ptr = self->data + (ptrdiff_t)((y * self->pitch) + (x * self->prop.tbp));
	switch (self->prop.format)
	{
		case QGCF_R32G32B32A32F:
			((QmVec*)ptr)[0] = *color;
			break;
		case QGCF_R32G32B32F:
			((QmFloat3*)ptr)[0] = (QmFloat3){ color->X, color->Y, color->Z };
			break;
		case QGCF_R32F:
			((float*)ptr)[0] = color->X;
			break;
		case QGCF_R16G16B16A16F:
			((QmHalf4*)ptr)[0] = (QmHalf4){ {qm_f2hf(color->W), qm_f2hf(color->X), qm_f2hf(color->Y), qm_f2hf(color->Z) } };
			break;
		case QGCF_R16F:
			((halffloat*)ptr)[0] = qm_f2hf(color->X);
			break;
		case QGCF_R11G11B10F:
			pixel_color_to_r11_g11_b10_f((uint*)ptr, *color);
			break;
		case QGCF_R16G16B16A16:
			pixel_color_to_r16_g16_b16_a16((ullong*)ptr, *color);
			break;
		case QGCF_R16:
			((ushort*)ptr)[0] = (ushort)(color->X * 0xFFFF);
			break;
		case QGCF_R10G10B10A2:
			pixel_color_to_r10_g10_b10_a2((uint*)ptr, *color);
			break;
		case QGCF_R8G8B8A8:
			pixel_color_to_r8_g8_b8_a8((uint*)ptr, *color);
			break;
		case QGCF_R8G8B8:
			pixel_color_to_r8_g8_b8(ptr, *color);
			break;
		case QGCF_R8:
			*ptr = (byte)(color->X * 0xFF);
			break;
		case QGCF_A8:
			*ptr = (byte)(color->W * 0xFF);
			break;
		case QGCF_L8:
			*ptr = (byte)(color->X * 0xFF);
			break;
		case QGCF_A8L8:
			((ushort*)ptr)[0] = (ushort)(((byte)(color->W * 0xFF) << 8) | (byte)(color->X * 0xFF));
			break;
		case QGCF_R5G6B5:
			pixel_color_to_r5_g6_b5(&((ushort*)ptr)[0], *color);
			break;
		case QGCF_R5G5B5A1:
			pixel_color_to_r5_b5_g5_a1(&((ushort*)ptr)[0], *color);
			break;
		case QGCF_R4G4B4A4:
			pixel_color_to_r4_b4_g4_a4(&((ushort*)ptr)[0], *color);
			break;
		default:
			return false;
	}
	return true;
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

	static const QN_DECL_VTABLE(QNGAMBASE) vt_qg_font =
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
