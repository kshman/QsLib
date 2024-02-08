//
// qn_str.c - CRT에 없는거나, 공통이 아닌것 재구현 (mbs,wcs,utf8,utf변환,해시)
// 2023-12-27 by kim
//

// ReSharper disable CppParameterMayBeConst

#include "pch.h"
#include <ctype.h>
#include <wctype.h>
#include "PatrickPowell_snprintf.h"
#define SINFL_IMPLEMENTATION
#define SDEFL_IMPLEMENTATION
#include "vurtun/sdefl.h"
#include "vurtun/sinfl.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4706)
#endif

//////////////////////////////////////////////////////////////////////////
// 해시

//
size_t qn_hash_ptr(const void* ptr)
{
#ifdef _QN_64_
	lldiv_t t = lldiv((long long)(size_t)ptr, 127773);
	t.rem = 16807 * t.rem - 2836 * t.quot;

	if (t.rem < 0)
		t.rem += INT64_MAX;
#else
	ldiv_t t = ldiv((long)(size_t)ptr, 127773);
	t.rem = 16807 * t.rem - 2836 * t.quot;

	if (t.rem < 0)
		t.rem += INT32_MAX;
#endif

	return (size_t)t.rem;
}

//
size_t qn_hash_now(void)
{
	static size_t dif = 0;

	time_t t = time(NULL);
	clock_t c = clock();

	const byte* p = (byte*)&t;
	size_t h1 = 0;
	for (size_t i = 0; i < sizeof(time_t); i++)
	{
		h1 *= UINT8_MAX + 2;
		h1 += p[i];
	}

	p = (byte*)&c;
	size_t h2 = 0;
	for (size_t i = 0; i < sizeof(clock_t); i++)
	{
		h2 *= UINT8_MAX + 2;
		h2 += p[i];
	}

	return (h1 + dif++) ^ h2;
}

size_t qn_hash_func(const int prime8, const func_t func, const void* data)
{
	// PP FF FF FF FD DD DD DD
	size_t h = (size_t)prime8 & 0xFFULL << 56ULL;
	const any_t v = { .func = func };
	h |= (qn_hash_ptr(v.p) & 0xFFFFFFF) << 28;
	h |= qn_hash_ptr(data) & 0xFFFFFFF;
	return h;
}

#ifdef _QN_64_
/**
 * @brief 지정한 데이터로 CRC64를 만든다
 * @date 2013-12-22
 * @param[in] data 자료
 * @param[in] size 크기
 * @return 정수 값
 */
static uint64_t qn_crc64(const byte* data, const size_t size)
{
	// https://github.com/srned/baselib/blob/master/crc64.c
	/* Redis uses the CRC64 variant with "Jones" coefficients and init value of 0.
	 *
	 * Specification of this CRC64 variant follows:
	 * Name: crc-64-jones
	 * Width: 64 bites
	 * Poly: 0xad93d23594c935a9
	 * Reflected In: True
	 * Xor_In: 0xffffffffffffffff
	 * Reflected_Out: True
	 * Xor_Out: 0x0
	 * Check("123456789"): 0xe9c6d914c4b8d9ca
	 *
	 * Copyright (c) 2012, Salvatore Sanfilippo <antirez at gmail dot com>
	 * All rights reserved.
	 *
	 * Redistribution and use in source and binary forms, with or without
	 * modification, are permitted provided that the following conditions are met:
	 *
	 *   * Redistributions of source code must retain the above copyright notice,
	 *     this list of conditions and the following disclaimer.
	 *   * Redistributions in binary form must reproduce the above copyright
	 *     notice, this list of conditions and the following disclaimer in the
	 *     documentation and/or other materials provided with the distribution.
	 *   * Neither the name of Redis nor the names of its contributors may be used
	 *     to endorse or promote products derived from this software without
	 *     specific prior written permission.
	 *
	 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
	 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	 * POSSIBILITY OF SUCH DAMAGE. */
	static const uint64_t crc64_table[] =
	{
		0x0000000000000000ULL, 0x7ad870c830358979ULL, 0xf5b0e190606b12f2ULL, 0x8f689158505e9b8bULL,
		0xc038e5739841b68fULL, 0xbae095bba8743ff6ULL, 0x358804e3f82aa47dULL, 0x4f50742bc81f2d04ULL,
		0xab28ecb46814fe75ULL, 0xd1f09c7c5821770cULL, 0x5e980d24087fec87ULL, 0x24407dec384a65feULL,
		0x6b1009c7f05548faULL, 0x11c8790fc060c183ULL, 0x9ea0e857903e5a08ULL, 0xe478989fa00bd371ULL,
		0x7d08ff3b88be6f81ULL, 0x07d08ff3b88be6f8ULL, 0x88b81eabe8d57d73ULL, 0xf2606e63d8e0f40aULL,
		0xbd301a4810ffd90eULL, 0xc7e86a8020ca5077ULL, 0x4880fbd87094cbfcULL, 0x32588b1040a14285ULL,
		0xd620138fe0aa91f4ULL, 0xacf86347d09f188dULL, 0x2390f21f80c18306ULL, 0x594882d7b0f40a7fULL,
		0x1618f6fc78eb277bULL, 0x6cc0863448deae02ULL, 0xe3a8176c18803589ULL, 0x997067a428b5bcf0ULL,
		0xfa11fe77117cdf02ULL, 0x80c98ebf2149567bULL, 0x0fa11fe77117cdf0ULL, 0x75796f2f41224489ULL,
		0x3a291b04893d698dULL, 0x40f16bccb908e0f4ULL, 0xcf99fa94e9567b7fULL, 0xb5418a5cd963f206ULL,
		0x513912c379682177ULL, 0x2be1620b495da80eULL, 0xa489f35319033385ULL, 0xde51839b2936bafcULL,
		0x9101f7b0e12997f8ULL, 0xebd98778d11c1e81ULL, 0x64b116208142850aULL, 0x1e6966e8b1770c73ULL,
		0x8719014c99c2b083ULL, 0xfdc17184a9f739faULL, 0x72a9e0dcf9a9a271ULL, 0x08719014c99c2b08ULL,
		0x4721e43f0183060cULL, 0x3df994f731b68f75ULL, 0xb29105af61e814feULL, 0xc849756751dd9d87ULL,
		0x2c31edf8f1d64ef6ULL, 0x56e99d30c1e3c78fULL, 0xd9810c6891bd5c04ULL, 0xa3597ca0a188d57dULL,
		0xec09088b6997f879ULL, 0x96d1784359a27100ULL, 0x19b9e91b09fcea8bULL, 0x636199d339c963f2ULL,
		0xdf7adabd7a6e2d6fULL, 0xa5a2aa754a5ba416ULL, 0x2aca3b2d1a053f9dULL, 0x50124be52a30b6e4ULL,
		0x1f423fcee22f9be0ULL, 0x659a4f06d21a1299ULL, 0xeaf2de5e82448912ULL, 0x902aae96b271006bULL,
		0x74523609127ad31aULL, 0x0e8a46c1224f5a63ULL, 0x81e2d7997211c1e8ULL, 0xfb3aa75142244891ULL,
		0xb46ad37a8a3b6595ULL, 0xceb2a3b2ba0eececULL, 0x41da32eaea507767ULL, 0x3b024222da65fe1eULL,
		0xa2722586f2d042eeULL, 0xd8aa554ec2e5cb97ULL, 0x57c2c41692bb501cULL, 0x2d1ab4dea28ed965ULL,
		0x624ac0f56a91f461ULL, 0x1892b03d5aa47d18ULL, 0x97fa21650afae693ULL, 0xed2251ad3acf6feaULL,
		0x095ac9329ac4bc9bULL, 0x7382b9faaaf135e2ULL, 0xfcea28a2faafae69ULL, 0x8632586aca9a2710ULL,
		0xc9622c4102850a14ULL, 0xb3ba5c8932b0836dULL, 0x3cd2cdd162ee18e6ULL, 0x460abd1952db919fULL,
		0x256b24ca6b12f26dULL, 0x5fb354025b277b14ULL, 0xd0dbc55a0b79e09fULL, 0xaa03b5923b4c69e6ULL,
		0xe553c1b9f35344e2ULL, 0x9f8bb171c366cd9bULL, 0x10e3202993385610ULL, 0x6a3b50e1a30ddf69ULL,
		0x8e43c87e03060c18ULL, 0xf49bb8b633338561ULL, 0x7bf329ee636d1eeaULL, 0x012b592653589793ULL,
		0x4e7b2d0d9b47ba97ULL, 0x34a35dc5ab7233eeULL, 0xbbcbcc9dfb2ca865ULL, 0xc113bc55cb19211cULL,
		0x5863dbf1e3ac9decULL, 0x22bbab39d3991495ULL, 0xadd33a6183c78f1eULL, 0xd70b4aa9b3f20667ULL,
		0x985b3e827bed2b63ULL, 0xe2834e4a4bd8a21aULL, 0x6debdf121b863991ULL, 0x1733afda2bb3b0e8ULL,
		0xf34b37458bb86399ULL, 0x8993478dbb8deae0ULL, 0x06fbd6d5ebd3716bULL, 0x7c23a61ddbe6f812ULL,
		0x3373d23613f9d516ULL, 0x49aba2fe23cc5c6fULL, 0xc6c333a67392c7e4ULL, 0xbc1b436e43a74e9dULL,
		0x95ac9329ac4bc9b5ULL, 0xef74e3e19c7e40ccULL, 0x601c72b9cc20db47ULL, 0x1ac40271fc15523eULL,
		0x5594765a340a7f3aULL, 0x2f4c0692043ff643ULL, 0xa02497ca54616dc8ULL, 0xdafce7026454e4b1ULL,
		0x3e847f9dc45f37c0ULL, 0x445c0f55f46abeb9ULL, 0xcb349e0da4342532ULL, 0xb1eceec59401ac4bULL,
		0xfebc9aee5c1e814fULL, 0x8464ea266c2b0836ULL, 0x0b0c7b7e3c7593bdULL, 0x71d40bb60c401ac4ULL,
		0xe8a46c1224f5a634ULL, 0x927c1cda14c02f4dULL, 0x1d148d82449eb4c6ULL, 0x67ccfd4a74ab3dbfULL,
		0x289c8961bcb410bbULL, 0x5244f9a98c8199c2ULL, 0xdd2c68f1dcdf0249ULL, 0xa7f41839ecea8b30ULL,
		0x438c80a64ce15841ULL, 0x3954f06e7cd4d138ULL, 0xb63c61362c8a4ab3ULL, 0xcce411fe1cbfc3caULL,
		0x83b465d5d4a0eeceULL, 0xf96c151de49567b7ULL, 0x76048445b4cbfc3cULL, 0x0cdcf48d84fe7545ULL,
		0x6fbd6d5ebd3716b7ULL, 0x15651d968d029fceULL, 0x9a0d8ccedd5c0445ULL, 0xe0d5fc06ed698d3cULL,
		0xaf85882d2576a038ULL, 0xd55df8e515432941ULL, 0x5a3569bd451db2caULL, 0x20ed197575283bb3ULL,
		0xc49581ead523e8c2ULL, 0xbe4df122e51661bbULL, 0x3125607ab548fa30ULL, 0x4bfd10b2857d7349ULL,
		0x04ad64994d625e4dULL, 0x7e7514517d57d734ULL, 0xf11d85092d094cbfULL, 0x8bc5f5c11d3cc5c6ULL,
		0x12b5926535897936ULL, 0x686de2ad05bcf04fULL, 0xe70573f555e26bc4ULL, 0x9ddd033d65d7e2bdULL,
		0xd28d7716adc8cfb9ULL, 0xa85507de9dfd46c0ULL, 0x273d9686cda3dd4bULL, 0x5de5e64efd965432ULL,
		0xb99d7ed15d9d8743ULL, 0xc3450e196da80e3aULL, 0x4c2d9f413df695b1ULL, 0x36f5ef890dc31cc8ULL,
		0x79a59ba2c5dc31ccULL, 0x037deb6af5e9b8b5ULL, 0x8c157a32a5b7233eULL, 0xf6cd0afa9582aa47ULL,
		0x4ad64994d625e4daULL, 0x300e395ce6106da3ULL, 0xbf66a804b64ef628ULL, 0xc5bed8cc867b7f51ULL,
		0x8aeeace74e645255ULL, 0xf036dc2f7e51db2cULL, 0x7f5e4d772e0f40a7ULL, 0x05863dbf1e3ac9deULL,
		0xe1fea520be311aafULL, 0x9b26d5e88e0493d6ULL, 0x144e44b0de5a085dULL, 0x6e963478ee6f8124ULL,
		0x21c640532670ac20ULL, 0x5b1e309b16452559ULL, 0xd476a1c3461bbed2ULL, 0xaeaed10b762e37abULL,
		0x37deb6af5e9b8b5bULL, 0x4d06c6676eae0222ULL, 0xc26e573f3ef099a9ULL, 0xb8b627f70ec510d0ULL,
		0xf7e653dcc6da3dd4ULL, 0x8d3e2314f6efb4adULL, 0x0256b24ca6b12f26ULL, 0x788ec2849684a65fULL,
		0x9cf65a1b368f752eULL, 0xe62e2ad306bafc57ULL, 0x6946bb8b56e467dcULL, 0x139ecb4366d1eea5ULL,
		0x5ccebf68aecec3a1ULL, 0x2616cfa09efb4ad8ULL, 0xa97e5ef8cea5d153ULL, 0xd3a62e30fe90582aULL,
		0xb0c7b7e3c7593bd8ULL, 0xca1fc72bf76cb2a1ULL, 0x45775673a732292aULL, 0x3faf26bb9707a053ULL,
		0x70ff52905f188d57ULL, 0x0a2722586f2d042eULL, 0x854fb3003f739fa5ULL, 0xff97c3c80f4616dcULL,
		0x1bef5b57af4dc5adULL, 0x61372b9f9f784cd4ULL, 0xee5fbac7cf26d75fULL, 0x9487ca0fff135e26ULL,
		0xdbd7be24370c7322ULL, 0xa10fceec0739fa5bULL, 0x2e675fb4576761d0ULL, 0x54bf2f7c6752e8a9ULL,
		0xcdcf48d84fe75459ULL, 0xb71738107fd2dd20ULL, 0x387fa9482f8c46abULL, 0x42a7d9801fb9cfd2ULL,
		0x0df7adabd7a6e2d6ULL, 0x772fdd63e7936bafULL, 0xf8474c3bb7cdf024ULL, 0x829f3cf387f8795dULL,
		0x66e7a46c27f3aa2cULL, 0x1c3fd4a417c62355ULL, 0x935745fc4798b8deULL, 0xe98f353477ad31a7ULL,
		0xa6df411fbfb21ca3ULL, 0xdc0731d78f8795daULL, 0x536fa08fdfd90e51ULL, 0x29b7d047efec8728ULL,
	};
	uint64_t crc64 = UINT64_MAX;
	for (size_t i = 0; i < size; i++)
		crc64 = crc64_table[(crc64 ^ data[i]) & 0xFF] ^ (crc64 >> 8);
	return ~crc64;
}
#else
/**
 * @brief 지정한 데이터로 CRC32를 만든다
 * @date 2013-12-22
 * @param[in] data 자료
 * @param[in] size 크기
 * @return 정수 값
 */
static uint qn_crc32(const byte* data, const size_t size)
{
	static const uint crc32_table[] =
	{
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
		0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
		0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
		0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
		0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
		0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
		0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
		0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
		0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
		0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
		0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
		0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
		0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
		0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
		0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
		0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
		0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
		0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};
	uint crc32 = UINT32_MAX;
	for (size_t i = 0; i < size; i++)
		crc32 = crc32_table[(crc32 ^ data[i]) & 0xFF] ^ (crc32 >> 8);
	return ~crc32;
}
#endif

//
size_t qn_hash_crc(const byte* data, const size_t size)
{
#ifdef _QN_64_
	return qn_crc64(data, size);
#else
	return qn_crc32(data, size);
#endif
}

//
uint qn_prime_near(const uint value)
{
	static const uint s_prime_table[] =
	{
		11, 19, 37, 73, 109, 163, 251, 367, 557, 823, 1237, 1861, 2777, 4177, 6247,
		9371, 14057, 21089, 31627, 47431, 71143, 106721, 160073, 240101, 360163, 540217,
		810343, 1215497, 1823231, 2734867, 4102283, 6153409, 9230113, 13845163,
	};
	for (size_t i = 0; i < QN_COUNTOF(s_prime_table); i++)
		if (s_prime_table[i] > value)
			return s_prime_table[i];
	return s_prime_table[QN_COUNTOF(s_prime_table) - 1];
}

//
uint qn_prime_shift(const uint value, const uint min, uint* shift)
{
	static const uint s_prime_shift_table[] =
	{
		/* 1 << 0 */
		1, 2, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 32749,
		/* 1 << 16 */
		65521, 131071, 262139, 524287, 1048573, 2097143, 4194301, 8388593,
		16777213, 33554393, 67108859, 134217689, 268435399, 536870909, 1073741789,
		/* 1 << 31 */
		2147483647
	};

	uint ts = 0;
	for (uint vs = value; vs; ts++)
		vs >>= 1;

	ts = QN_MAX(min, ts);

	if (shift) *shift = ts;

	return s_prime_shift_table[ts];
}


//////////////////////////////////////////////////////////////////////////
// 메모리 조작

//
void* qn_memenc(void* dest, const void* src, const size_t size)
{
	const byte* ps = (const byte*)src;
	byte* pd = (byte*)dest;

	for (size_t i = size; i; --i, ++pd, ++ps)
	{
		const byte z = (byte)(255 - *ps);
		*pd = (byte)(z >> 4 | (z & 0xF) << 4);
	}

	return dest;
}

//
void* qn_memdec(void* dest, const void* src, const size_t size)
{
	const byte* ps = (const byte*)src;
	byte* pd = (byte*)dest;

	for (size_t i = size; i; --i, ++pd, ++ps)
	{
		const byte z = (byte)(255 - *ps);
		*pd = (byte)(z << 4 | (z & 0xF0) >> 4);
	}

	return dest;
}

//
void* qn_memzcpr(const void* src, const size_t srcsize, /*NULLABLE*/size_t* destsize)
{
	qn_return_when_fail(src != NULL, NULL);
	qn_return_when_fail(srcsize > 0, NULL);

	struct sdefl* s = qn_alloc_zero_1(struct sdefl);
	const int bound = sdefl_bound((int)srcsize);
	byte* p = qn_alloc(bound, byte);
	const int ret = sdeflate(s, p, src, (int)srcsize, 5);	// 압축 레벨은 5
	qn_free(s);

	if (destsize)
		*destsize = (size_t)ret;
	return p;
}

//
void* qn_memzucp(const void* src, const size_t srcsize, /*NULLABLE*/size_t* destsize)
{
	qn_return_when_fail(src != NULL, NULL);
	qn_return_when_fail(srcsize > 0, NULL);

	int size = (int)(srcsize * 3 + 12);
	byte* p = NULL;
	int ret;
	do
	{
		size *= 2;
		p = qn_realloc(p, size, byte);
		ret = sinflate(p, size, src, (int)srcsize);
	} while (ret == size);

	if (destsize)
		*destsize = ret;
	return p;
}

//
void* qn_memzucp_s(const void* src, const size_t srcsize, const size_t destsize)
{
	qn_return_when_fail(src != NULL, NULL);
	qn_return_when_fail(srcsize > 0, NULL);
	qn_return_when_fail(destsize > 0, NULL);
	
	byte* p = qn_alloc(destsize + 4, byte);
	const int ret = sinflate(p, (int)destsize, src, (int)srcsize);
	if (ret != (int)destsize)
	{
		qn_free(p);
		return NULL;
	}
	return p;
}

//
char qn_memhrb(const size_t size, double* out)
{
	qn_return_when_fail(out != NULL, ' ');
	if (size > 1024ULL * 1024ULL * 1024ULL)
	{
		*out = (double)size / (double)(1024ULL * 1024ULL * 1024ULL);
		return 'g';
	}
	if (size > 1024ULL * 1024ULL)
	{
		*out = (double)size / (double)(1024ULL * 1024ULL);
		return 'm';
	}
	if (size > 1024ULL)
	{
		*out = (double)size / (double)(1024ULL);
		return 'k';
	}
	*out = (double)size;
	return ' ';
}

//
char* qn_memdmp(const void* ptr, const size_t size, char* outbuf, const size_t buflen)
{
	qn_return_when_fail(ptr != NULL, NULL);
	qn_return_when_fail(outbuf != NULL, NULL);

	if (size == 0 || buflen == 0)
	{
		*outbuf = '\0';
		return outbuf;
	}

	const byte* mem = (const byte*)ptr;
	char* ind = outbuf;
	for (size_t i = 0, cnt = 1; i < size && cnt < buflen; i++, cnt++)
	{
		const byte m = *mem;
		*ind = (m < 0x20 || m > 0x7F) ? '.' : (const char)m;  // NOLINT
		mem++;
		ind++;
	}
	*ind = '\0';

	return outbuf;
}


//////////////////////////////////////////////////////////////////////////
// 퀵 소트

#define QSORT_STACK_SIZE (8 * sizeof(void*) - 2)

static void qsort_swap(byte* a, byte* b, size_t stride)
{
	if (a != b)
	{
		while (stride--)
		{
			const byte n = *a;
			*a++ = *b;
			*b++ = n;
		}
	}
}

static void qsort_short_context(byte* lo, byte* hi, const size_t stride, int(*func)(void*, const void*, const void*), void* context)
{
	while (hi > lo)
	{
		byte* max = lo;

		for (byte* p = lo + stride; p <= hi; p += stride)
		{
			if ((*func)(context, p, max) > 0)
				max = p;
		}

		qsort_swap(max, hi, stride);
		hi -= stride;
	}
}

//
void qn_qsortc(void* ptr, size_t count, size_t stride, sortcfunc_t compfunc, void* context)
{
	qn_return_when_fail(ptr,/*void*/);
	qn_return_when_fail(count > 1,/*void*/);
	qn_return_when_fail(stride,/*void*/);
	qn_return_when_fail(compfunc,/*void*/);

	// 스택&리미트 초기화
	int_fast32_t stkptr = 0;
	byte* lo = (byte*)ptr;
	byte* hi = (byte*)ptr + stride * (count - 1);

	byte* lostk[QSORT_STACK_SIZE] = { 0, };
	byte* histk[QSORT_STACK_SIZE] = { 0, };

	size_t size;
pos_recursive:
	size = (size_t)(hi - lo) / stride + 1;

	// 중간값처리를 사용해서 O(n^2) 알고리즘으로 전환
	if (size <= 8)  // 최적화된 값을 사용해야 할 것이다 -> Cut off value
		qsort_short_context(lo, hi, stride, compfunc, context);
	else
	{
		byte* mid = lo + (size / 2) * stride;

		// 처음, 중간, 끝 부터 정렬 시작
		if (compfunc(context, lo, mid) > 0)
			qsort_swap(lo, mid, stride);

		if (compfunc(context, lo, hi) > 0)
			qsort_swap(lo, hi, stride);

		if (compfunc(context, mid, hi) > 0)
			qsort_swap(mid, hi, stride);

		// 부분 정렬
		byte* lopos = lo;
		byte* hipos = hi;

		for (;;)
		{
			if (mid > lopos)
			{
				do
				{
					lopos += stride;
				} while (lopos < mid && compfunc(context, lopos, mid) <= 0);
			}

			if (mid <= lopos)
			{
				do
				{
					lopos += stride;
				} while (lopos <= hi && compfunc(context, lopos, mid) <= 0);
			}

			do
			{
				hipos -= stride;
			} while (hipos > mid && compfunc(context, hipos, mid) > 0);

			if (hipos < lopos)
				break;

			qsort_swap(lopos, hipos, stride);

			if (mid == hipos)
				mid = lopos;
		}

		hipos += stride;

		if (mid < hipos)
		{
			do
			{
				hipos -= stride;
			} while (hipos > mid && compfunc(context, hipos, mid) == 0);
		}

		if (mid >= hipos)
		{
			do
			{
				hipos -= stride;
			} while (hipos > lo && compfunc(context, hipos, mid) == 0);
		}

		if (hipos - lo >= hi - lopos)
		{
			if (lo < hipos)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = hipos;
				++stkptr;
			}

			if (lopos < hi)
			{
				lo = lopos;
				goto pos_recursive;
			}
		}
		else
		{
			if (lopos < hi)
			{
				lostk[stkptr] = lopos;
				histk[stkptr] = hi;
				++stkptr;
			}

			if (lo < hipos)
			{
				hi = hipos;
				goto pos_recursive;
			}
		}
	}

	--stkptr;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto pos_recursive;
	}
}

static void qsort_short(byte* lo, byte* hi, const size_t stride, int(*func)(const void*, const void*))
{
	while (hi > lo)
	{
		byte* max = lo;

		for (byte* p = lo + stride; p <= hi; p += stride)
		{
			if ((*func)(p, max) > 0)
				max = p;
		}

		qsort_swap(max, hi, stride);
		hi -= stride;
	}
}

//
void qn_qsort(void* ptr, size_t count, size_t stride, sortfunc_t compfunc)
{
	qn_return_when_fail(ptr,/*void*/);
	qn_return_when_fail(count > 1,/*void*/);
	qn_return_when_fail(stride,/*void*/);
	qn_return_when_fail(compfunc,/*void*/);

	// 스택&리미트 초기화
	int_fast32_t stkptr = 0;
	byte* lo = (byte*)ptr;
	byte* hi = (byte*)ptr + stride * (count - 1);

	byte* lostk[QSORT_STACK_SIZE] = { 0, };
	byte* histk[QSORT_STACK_SIZE] = { 0, };

	size_t size;
pos_recursive:
	size = (size_t)(hi - lo) / stride + 1;

	// 중간값처리를 사용해서 O(n^2) 알고리즘으로 전환
	if (size <= 8)  // 최적화된 값을 사용해야 할 것이다 -> Cut off value
		qsort_short(lo, hi, stride, compfunc);
	else
	{
		byte* mid = lo + (size / 2) * stride;

		// 처음, 중간, 끝 부터 정렬 시작
		if (compfunc(lo, mid) > 0)
			qsort_swap(lo, mid, stride);

		if (compfunc(lo, hi) > 0)
			qsort_swap(lo, hi, stride);

		if (compfunc(mid, hi) > 0)
			qsort_swap(mid, hi, stride);

		// 부분 정렬
		byte* lopos = lo;
		byte* hipos = hi;

		for (;;)
		{
			if (mid > lopos)
			{
				do
				{
					lopos += stride;
				} while (lopos < mid && compfunc(lopos, mid) <= 0);
			}

			if (mid <= lopos)
			{
				do
				{
					lopos += stride;
				} while (lopos <= hi && compfunc(lopos, mid) <= 0);
			}

			do
			{
				hipos -= stride;
			} while (hipos > mid && compfunc(hipos, mid) > 0);

			if (hipos < lopos)
				break;

			qsort_swap(lopos, hipos, stride);

			if (mid == hipos)
				mid = lopos;
		}

		hipos += stride;

		if (mid < hipos)
		{
			do
			{
				hipos -= stride;
			} while (hipos > mid && compfunc(hipos, mid) == 0);
		}

		if (mid >= hipos)
		{
			do
			{
				hipos -= stride;
			} while (hipos > lo && compfunc(hipos, mid) == 0);
		}

		if (hipos - lo >= hi - lopos)
		{
			if (lo < hipos)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = hipos;
				++stkptr;
			}

			if (lopos < hi)
			{
				lo = lopos;
				goto pos_recursive;
			}
		}
		else
		{
			if (lopos < hi)
			{
				lostk[stkptr] = lopos;
				histk[stkptr] = hi;
				++stkptr;
			}

			if (lo < hipos)
			{
				hi = hipos;
				goto pos_recursive;
			}
		}
	}

	--stkptr;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto pos_recursive;
	}
}


//////////////////////////////////////////////////////////////////////////
// 공용

// 문자를 진수 숫자로 (32진수까지)
const byte* qn_num_base_table(void)
{
	static const byte nbase_table[] =
	{
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,	// 0~15
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 16~31
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 32~47
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255, // 48~63
		255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 64~79
		25, 26, 27, 28, 29, 30, 31, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 80~95
		255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 96~111
		25, 26, 27, 28, 29, 30, 31, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 112~127
	};
	return nbase_table;
}

// 숫자를 문자로 바꾸기 (32진수까지)
const char* qn_char_base_table(bool upper)
{
	static const char* nchar_table[] =
	{
		"0123456789abcdefghijklmnopqrstuv",
		"0123456789ABCDEFGHIJKLMNOPQRSTUV",
	};
	return nchar_table[upper == false ? 0 : 1];
}


//////////////////////////////////////////////////////////////////////////
// 아스키/멀티바이트 버전

// DOPR 정리 공용
static void printf_finish(PatrickPowellSprintfState* state)
{
	if (state->maxlen > 0)
		state->buf[state->currlen < state->maxlen - 1 ? state->currlen : state->maxlen - 1] = '\0';
}

// DOPR 고정 문자열 버전
static void printf_outch(PatrickPowellSprintfState* state, int ch)
{
	if (state->currlen < state->maxlen)
		state->buf[state->currlen] = (char)ch;
	state->currlen++;
}

// DOPR 할당 문자열 버전
static void printf_alloc_outch(PatrickPowellSprintfState* state, int ch)
{
	if (state->currlen >= state->maxlen)
	{
		state->maxlen = state->currlen + 256;
#ifdef QS_NO_MEMORY_PROFILE
		state->buf = (char*)qn_a_realloc(state->buf, state->maxlen);
#else
		state->buf = (char*)qn_a_i_realloc(state->buf, state->maxlen, state->desc, state->line);
#endif
	}
	state->buf[state->currlen] = (char)ch;
	state->currlen++;
}

#ifdef QS_NO_MEMORY_PROFILE
//
static int qn_a_printf(char** out, const char* fmt, va_list va)
{
	PatrickPowellSprintfState state =
	{
		printf_alloc_outch,
		printf_finish,
	};
	dopr(&state, fmt, va);
	*out = state.buf;
	return (int)state.currlen;
}

//
int qn_a_vsprintf(char** out, const char* fmt, va_list va)
{
	if (out == NULL)
		return qn_vsnprintf(NULL, 0, fmt, va);
	qn_return_when_fail(fmt != NULL, -1);
	return qn_a_printf(out, fmt, va);
}

//
char* qn_a_vpsprintf(const char* fmt, va_list va)
{
	qn_return_when_fail(fmt != NULL, NULL);
	char* buf;
	qn_a_printf(&buf, fmt, va);
	return buf;
}

//
int qn_a_sprintf(char** out, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_a_vsprintf(out, fmt, va);
	va_end(va);
	return ret;
}

//
char* qn_a_psprintf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char* buf;
	qn_a_printf(&buf, fmt, va);
	va_end(va);
	return buf;
}

//
char* qn_a_str_dup(const char* p)
{
	qn_return_when_fail(p != NULL, NULL);
	const size_t len = strlen(p) + 1;
	char* d = (char*)qn_a_alloc(len, false);
	qn_strcpy(d, p);
	return d;
}

//
char* qn_a_str_dup_cat(const char* p, ...)
{
	va_list va, vq;
	va_start(va, p);
	va_copy(vq, va);

	size_t size = strlen(p) + 1;
	const char* s = va_arg(va, char*);
	while (s)
	{
		size += strlen(s);
		s = va_arg(va, char*);
	}

	char* str = (char*)qn_a_alloc(size, false);
	char* c = qn_stpcpy(str, p);
	s = va_arg(vq, char*);
	while (s)
	{
		c = qn_stpcpy(c, s);
		s = va_arg(vq, char*);
	}
	va_end(vq);

	va_end(va);
	return str;
}
#else
static int qn_a_i_printf(const char* desc, size_t line, char** out, const char* fmt, va_list va)
{
	PatrickPowellSprintfState state =
	{
		printf_alloc_outch,
		printf_finish,
		desc, line,
	};
	dopr(&state, fmt, va);
	*out = state.buf;
	return (int)state.currlen;
}

//
int qn_a_i_vsprintf(const char* desc, size_t line, char** out, const char* fmt, va_list va)
{
	if (out == NULL)
		return qn_vsnprintf(NULL, 0, fmt, va);
	qn_return_when_fail(fmt != NULL, -1);
	return qn_a_i_printf(desc, line, out, fmt, va);
}

//
char* qn_a_i_vpsprintf(const char* desc, size_t line, const char* fmt, va_list va)
{
	qn_return_when_fail(fmt != NULL, NULL);
	char* buf;
	qn_a_i_printf(desc, line, &buf, fmt, va);
	return buf;
}

//
int qn_a_i_sprintf(const char* desc, size_t line, char** out, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_a_i_vsprintf(desc, line, out, fmt, va);
	va_end(va);
	return ret;
}

//
char* qn_a_i_psprintf(const char* desc, size_t line, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char* buf;
	qn_a_i_printf(desc, line, &buf, fmt, va);
	va_end(va);
	return buf;
}

//
char* qn_a_i_str_dup(const char* desc, size_t line, const char* p)
{
	qn_return_when_fail(p != NULL, NULL);
	const size_t len = strlen(p) + 1;
	char* d = (char*)qn_a_i_alloc(len, false, desc, line);
	qn_strcpy(d, p);
	return d;
}

//
char* qn_a_i_str_dup_cat(const char* desc, size_t line, const char* p, ...)
{
	va_list va, vq;
	va_start(va, p);
	va_copy(vq, va);

	size_t size = strlen(p) + 1;
	const char* s = va_arg(va, char*);
	while (s)
	{
		size += strlen(s);
		s = va_arg(va, char*);
	}

	char* str = (char*)qn_a_i_alloc(size, false, desc, line);
	char* c = qn_stpcpy(str, p);
	s = va_arg(vq, char*);
	while (s)
	{
		c = qn_stpcpy(c, s);
		s = va_arg(vq, char*);
	}
	va_end(vq);

	va_end(va);
	return str;
}
#endif

//
int qn_vsnprintf(char* out, size_t len, const char* fmt, va_list va)
{
	qn_return_when_fail(fmt != NULL, -1);
	PatrickPowellSprintfState state =
	{
		printf_outch,
		printf_finish,
	};
	state.maxlen = out != NULL ? len : 0;
	state.buf = out;
	dopr(&state, fmt, va);
	return (int)state.currlen;
}

//
int qn_snprintf(char* out, const size_t len, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	const int ret = qn_vsnprintf(out, len, fmt, va);
	va_end(va);
	return ret;
}

//
char* qn_strcpy(char* p, const char* src)
{
#ifdef __GNUC__
	return strcpy(p, src);
#else
	char* o = p;
	while ((*p++ = *src++)) {}
	return o;
#endif
}

//
char* qn_strncpy(char* p, const char* src, size_t len)
{
#ifdef __GNUC__
	return strncpy(p, src, len);
#else
	char* o = p;
	while (len && ((*p++ = *src++)))
		--len;
	*p = '\0';
#if false
	if (len)
	{
		while (--len)
			*p++ = '\0';
	}
#endif
	return o;
#endif
}

//
char* qn_stpcpy(char* dest, const char* src)
{
	do (*dest++ = *src);
	while (*src++ != '\0');
	return dest - 1;
}

//
char* qn_strcat(char* dest, const char* src)
{
#ifdef __GNUC__
	return strcat(dest, src);
#else
	char* o = dest;
	while (*dest) dest++;
	while ((*dest++ = *src++)) {}
	return o;
#endif
}

//
char* qn_strncat(char* dest, const char* src, size_t len)
{
#ifdef __GNUC__
	return strncat(dest, src, len);
#else
	char* o = dest;
	while (*dest) dest++;
	while (len && ((*dest++ = *src++)))
		--len;
	*dest = '\0';
	return o;
#endif
}

//
char* qn_strconcat(size_t max_len, char* dest, ...)
{
	va_list va;
	char* c = dest;
	char* s;

	va_start(va, dest);
	s = va_arg(va, char*);
	while (s)
	{
		while (*s)
		{
			if (--max_len == 0)
				goto pos_exit;
			*c++ = *s++;
		}
		s = va_arg(va, char*);
	}
pos_exit:
	va_end(va);
	*c = '\0';
	return dest;
}

//
size_t qn_strfll(char* dest, const size_t pos, const size_t end, const int ch)
{
	if (pos >= end)
		return pos;
	const size_t cnt = end - pos;
	for (size_t i = 0; i < cnt; i++)
		dest[pos + i] = (char)ch;
	return pos + cnt;
}

//
size_t qn_strhash(const char* p)
{
	const char* sz = p;
	size_t h = (size_t)*sz++;
	if (!h)
		return 0;
	size_t c;
	for (c = 0; *sz && c < 256; sz++, c++)
		h = (h << 5) - h + (size_t)*sz;
	h = (h << 5) - h + c;
	return h;
}

//
size_t qn_strihash(const char* p)
{
	const char* sz = p;
	size_t h = (size_t)tolower(*sz);
	if (!h)
		return 0;
	size_t c;
	sz++;
	for (c = 0; *sz && c < 256; sz++, c++)
		h = (h << 5) - h + (size_t)tolower(*sz);
	h = (h << 5) - h + c;
	return h;
}

//
uint qn_strshash(const char* p)
{
	const char* sz = p;
	uint h = (uint)tolower(*sz);
	if (!h)
		return 0;
	uint c;
	sz++;
	for (c = 0; *sz && c < 256; sz++, c++)
		h = (h << 5) - h + (uint)tolower(*sz);
	h = (h << 5) - h + c;
	return h;
}

//
int qn_strcmp(const char* p1, const char* p2)
{
#if defined _MSC_VER || defined __GNUC__
	return strcmp(p1, p2);
#else
	while (*p1 && *p1 == *p2)
		++p1, ++p2;
	return *p1 - *p2;
#endif
}

//
int qn_strncmp(const char* p1, const char* p2, size_t len)
{
#if defined _MSC_VER || defined __GNUC__
	return strncmp(p1, p2, len);
#else
	if (!len)
		return 0;
	while (--len && *p1 && *p1 == *p2)
		++p1, ++p2;
	return *p1 - *p2;
#endif
}

//
int qn_stricmp(const char* p1, const char* p2)
{
#ifdef _MSC_VER
	return _stricmp(p1, p2);
#elif defined __GNUC__
	return strcasecmp(p1, p2);
#else
	int	f, l;
	do
	{
		f = (unsigned char)(*(p1++));
		l = (unsigned char)(*(p2++));
		if (f >= 'A' && f <= 'Z')	f -= 'A' - 'a';
		if (l >= 'A' && l <= 'Z')	l -= 'A' - 'a';
	} while (f && (f == l));
	return (f - l);
#endif
}

//
int qn_strnicmp(const char* p1, const char* p2, size_t len)
{
#ifdef _MSC_VER
	return _strnicmp(p1, p2, len);
#elif defined __GNUC__
	return strncasecmp(p1, p2, len);
#else
	int	f, l;
	while (len && *p1 && *p2)
	{
		--len;
		f = (unsigned char)(*p1);
		l = (unsigned char)(*p2);
		if (f >= 'A' && f <= 'Z')	f -= 'A' - 'a';
		if (l >= 'A' && l <= 'Z')	l -= 'A' - 'a';
		if (f != l) return (f - l);
		++p1;
		++p2;
	}
	return (len) ? (int)(p1 - p2) : 0;
#endif
}

//
int qn_strfnd(const char* src, const char* find, const size_t index)
{
	const char* p = src + index;
	while (*p)
	{
		const char* s1 = p;
		const char* s2 = find;
		while (*s1 && *s2 && !(*s1 - *s2))
		{
			++s1;
			++s2;
		}
		if (!*s2)
			return (int)(src + index - p);
	}
	return -1;
}

//
bool qn_strwcm(const char* string, const char* wild)
{
	const char *cp = NULL, *mp = NULL;
	while ((*string) && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
			return false;
		wild++;
		string++;
	}
	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
				return 1;
			mp = wild;
			cp = string + 1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}
	while (*wild == '*')
		wild++;
	return *wild != '\0';
}

//
bool qn_striwcm(const char* string, const char* wild)
{
	const char *cp = NULL, *mp = NULL;
	while ((*string) && (*wild != '*'))
	{
		if ((toupper(*wild) != toupper(*string)) && (*wild != '?'))
			return false;
		wild++;
		string++;
	}
	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
				return 1;
			mp = wild;
			cp = string + 1;
		}
		else if ((toupper(*wild) == toupper(*string)) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}
	while (*wild == '*')
		wild++;
	return *wild != '\0';
}

//
bool qn_streqv(const char* p1, const char* p2)
{
	return p1 == p2 ? true : strcmp(p1, p2) == 0;
}

//
bool qn_strieqv(const char* p1, const char* p2)
{
	return p1 == p2 ? true : qn_stricmp(p1, p2) == 0;
}

//
const char* qn_strbrk(const char* p, const char* c)
{
#if defined _MSC_VER || defined __GNUC__
	return strpbrk(p, c);
#else
	// microsoft c library 13.10
	const char* str = p;
	const char* ctn = c;
	int map[32] = { 0, };
	while (*ctn)
	{
		map[*ctn >> 3] |= 1 << (*ctn & 7);
		ctn++;
	}
	while (*str)
	{
		if (map[*str >> 3] & (1 << (*str & 7)))
			return str;
		str++;
	}
	return NULL;
#endif
}

//
char* qn_strchr(const char* p, int ch)
{
#if defined _MSC_VER || defined __GNUC__
	return strchr(p, ch);
#else
	while (*p && *p != (char)ch) p++;
	return (*p == (char)ch) ? (char*)p : NULL;
#endif
}

//
char* qn_strrchr(const char* p, int ch)
{
#if defined _MSC_VER || defined __GNUC__
	return strrchr(p, ch);
#else
	char* s = (char*)p;
	while (*s++);
	while (--s != p && *s != (char)ch);
	return (*s == (char)ch) ? (char*)s : NULL;
#endif
}

//
char* qn_strtok(_Inout_opt_z_ char* p, _In_z_ const char* sep, _Inout_ char** ctx)
{
#if defined _MSC_VER
	return strtok_s(p, sep, ctx);
#elif defined __GNUC__
	return strtok_r(p, sep, ctx);
#else
	qn_return_when_fail(p != NULL, NULL);
	qn_return_when_fail(sep != NULL, NULL);
	qn_return_when_fail(ctx != NULL, NULL);
	if (p)
		*ctx = (char*)p;
	else if (!*ctx)
		return NULL;
	char* s = *ctx;
	while (*s)
	{
		const char* d = sep;
		while (*d)
		{
			if (*s == *d)
			{
				*s++ = '\0';
				*ctx = s;
				return s - 1;
			}
			++d;
		}
		++s;
	}
	*ctx = NULL;
	return NULL;
#endif
}

//
const char* qn_strext(const char* p, const char* name, int separator)
{
	const size_t len = strlen(name);
	for (;;)
	{
		const char* t = strstr(p, name);
		if (t == NULL)
			break;
		if (t == p || *(t - 1) == separator)
		{
			const char* e = t + len;
			if (*e == '\0' || *e == separator)
				return t;
		}
		p = t + len;
	}
	return NULL;
}

//
char* qn_strmid(char* dest, const char* src, const size_t pos, const size_t len)
{
	const size_t size = strlen(src);
	if (pos > size)
		*dest = '\0';
	else
	{
		qn_strncpy(dest, src + pos, len);
		*(dest + len) = '\0';
	}
	return dest;
}

//
char* qn_strltm(char* dest)
{
	char* s;
	for (s = dest; *s && isspace(*s); s++) {}
	if (dest != s)
		memmove(dest, s, strlen(s) + 1);
	return dest;
}

//
char* qn_strrtm(char* dest)
{
	size_t len = strlen(dest);
	while (len--)
	{
		if (!isspace(dest[len]))
			break;
		dest[len] = '\0';
	}
	return dest;
}

//
char* qn_strtrm(char* dest)
{
	return qn_strrtm(qn_strltm(dest));
}

//
char* qn_strrem(char* p, const char* rmlist)
{
	const char* p1 = p;
	char* p2 = p;
	while (*p1)
	{
		const char* ps = (const char*)rmlist;
		bool b = false;
		while (*ps)
		{
			if (*p1 == *ps)
			{
				b = true;
				break;
			}
			++ps;
		}
		if (!b)
		{
			*p2 = *p1;
			++p2;
		}
		++p1;
	}
	*p2 = '\0';
	return p;
}

//
char* qn_strupr(char* p)
{
	for (char* s = p; *s; ++s)
		if ((*s >= 'a') && (*s <= 'z'))
			*s -= 'a' - 'A';
	return p;
}

//
char* qn_strlwr(char* p)
{
	for (char* s = p; *s; ++s)
		if ((*s >= 'A') && (*s <= 'Z'))
			*s += 'a' - 'A';
	return p;
}

//
int qn_strtoi(const char* p, const uint base)
{
	qn_return_when_fail(p != NULL, 0);
	qn_return_when_fail(base >= 2 && base < 32, 0);
	const byte* table = qn_num_base_table();
	uint v = 0;
	int sign = 1;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	if (*p == '+')
		p++;
	else if (*p == '-')
	{
		sign = -1;
		p++;
	}
	uint ch = table[(byte)*p++];
	while (ch < base)
	{
		v = v * base + ch;
		ch = table[(byte)*p++];
	}
	return sign * (int)v;
}

//
llong qn_strtoll(const char* p, const uint base)
{
	qn_return_when_fail(p != NULL, 0);
	qn_return_when_fail(base >= 2 && base < 32, 0);
	const byte* table = qn_num_base_table();
	ullong v = 0;
	llong sign = 1LL;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	if (*p == '+')
		p++;
	else if (*p == '-')
	{
		sign = -1LL;
		p++;
	}
	uint ch = table[(byte)*p++];
	while (ch < base)
	{
		v = v * base + ch;
		ch = table[(byte)*p++];
	}
	return sign * (llong)v;
}

//
float qn_strtof(const char* p)
{
	float f = 0.0f;
	int rsign = 1;
	int e = 0;
	int ch;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	if (*p == '+')
		p++;
	else if (*p == '-')
	{
		rsign = -1;
		p++;
	}
	while ((ch = (int)*p++) != '\0' && isdigit(ch))
		f = f * 10.0f + (float)(ch - '0');
	if (ch == '.')
	{
		while ((ch = (int)*p++) != '\0' && isdigit(ch))
		{
			f = f * 10.0f + (float)(ch - '0');
			e--;
		}
	}
	if (ch == 'e' || ch == 'E')
	{
		int sign = 1;
		int n = 0;
		ch = (int)*p++;
		if (ch == '+')
			ch = (int)*p++;
		else if (ch == '-')
		{
			ch = (int)*p++;
			sign = -1;
		}
		while (isdigit(ch))
		{
			n = n * 10 + (ch - '0');
			ch = (int)*p++;
		}
		e += n * sign;
	}
	while (e > 0)
	{
		f *= 10.0f;
		e--;
	}
	while (e < 0)
	{
		f *= 0.1f;
		e++;
	}
	return rsign < 0 ? -f : f;
}

//
double qn_strtod(const char* p)
{
	double d = 0.0;
	int rsign = 1;
	int e = 0;
	int ch;
	while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
	if (*p == '+')
		p++;
	else if (*p == '-')
	{
		rsign = -1;
		p++;
	}
	while ((ch = (int)*p++) != '\0' && isdigit(ch))
		d = d * 10.0 + (ch - '0');
	if (ch == '.')
	{
		while ((ch = (int)*p++) != '\0' && isdigit(ch))
		{
			d = d * 10.0 + (ch - '0');
			e--;
		}
	}
	if (ch == 'e' || ch == 'E')
	{
		int sign = 1;
		int n = 0;
		ch = (int)*p++;
		if (ch == '+')
			ch = (int)*p++;
		else if (ch == '-')
		{
			ch = (int)*p++;
			sign = -1;
		}
		while (isdigit(ch))
		{
			n = n * 10 + (ch - '0');
			ch = (int)*p++;
		}
		e += n * sign;
	}
	while (e > 0)
	{
		d *= 10.0;
		e--;
	}
	while (e < 0)
	{
		d *= 0.1;
		e++;
	}
	return rsign < 0 ? -d : d;
}

//
int qn_itoa(char* p, const int n, const uint base, bool upper)
{
	qn_return_when_fail(base <= 32, -1);
	const char* table = qn_char_base_table(upper);
	char conv[32];
	int place = 0;
	uint uvalue;
	if (n >= 0 || base != 10)
		uvalue = (uint)n;
	else
	{
		conv[place++] = '-';
		uvalue = (uint)-n;
	}
	do
	{
		conv[place++] = table[uvalue % base];
		uvalue = uvalue / base;
	} while (uvalue && place < (int)QN_COUNTOF(conv));
	if (place == (int)QN_COUNTOF(conv))
		place--;
	conv[place] = '\0';
	if (p != NULL)
		qn_strcpy(p, conv);
	return place;
}

//
int qn_lltoa(char* p, const llong n, const uint base, bool upper)
{
	qn_return_when_fail(base <= 32, -1);
	const char* table = qn_char_base_table(upper);
	char conv[64];
	int place = 0;
	ullong uvalue;
	if (n >= 0 || base != 10)
		uvalue = (ullong)n;
	else
	{
		conv[place++] = '-';
		uvalue = (ullong)-n;
	}
	do
	{
		conv[place++] = table[uvalue % base];
		uvalue = uvalue / base;
	} while (uvalue && place < (int)QN_COUNTOF(conv));
	if (place == (int)QN_COUNTOF(conv))
		place--;
	conv[place] = '\0';
	if (p != NULL)
		qn_strcpy(p, conv);
	return place;
}

//
void qn_divpath(const char* p, char* dir, char* filename)
{
	const char* end = p;
	const char* sep = NULL;
	while (*end)
	{
		if (*end == '/' || *end == '\\')
			sep = end;
		++end;
	}
	if (sep)
	{
		if (dir)
			qn_strncpy(dir, p, sep - p + 1);
		if (filename)
			qn_strcpy(filename, sep + 1);
	}
	else
	{
		if (dir)
			*dir = '\0';
		if (filename)
			qn_strcpy(filename, p);
	}
}

//
void qn_splitpath(const char* p, char* drive, char* dir, char* name, char* ext)
{
	const char* path = p;
	const char* s = path;
	const char* end = path;
	const char* dot = NULL;
	const char* sep = NULL;
	while (*end)
	{
		if (*end == '/' || *end == '\\')
			sep = end;
		else if (*end == '.')
			dot = end;
		++end;
	}
	if (sep)
	{
		if (drive)
		{
			if (sep - s > 1 && s[1] == ':')
			{
				if (drive)
					qn_strncpy(drive, path, 2);
			}
			else
				*drive = '\0';
		}
		if (dir)
			qn_strncpy(dir, path, sep - path + 1);
	}
	else
	{
		if (drive)
			*drive = '\0';
		if (dir)
			*dir = '\0';
	}
	if (dot && dot >= sep)
	{
		if (name)
			qn_strncpy(name, sep + 1, dot - sep - 1);
		if (ext)
		{
			qn_strcpy(ext, dot);
			ext[dot - sep] = '\0';
		}
	}
	else
	{
		if (name)
			qn_strcpy(name, sep + 1);
		if (ext)
			*ext = '\0';
	}
}


//////////////////////////////////////////////////////////////////////////
// 유니코드 변환

//
uchar4 qn_u8cbn(const char* p)
{
	int len, mask;
	const byte ch = (byte)*p;

	if (ch < 128)
	{
		len = 1;
		mask = 0x7f;
	}
	else if ((ch & 0xe0) == 0xc0)
	{
		len = 2;
		mask = 0x1f;
	}
	else if ((ch & 0xf0) == 0xe0)
	{
		len = 3;
		mask = 0x0f;
	}
	else if ((ch & 0xf8) == 0xf0)
	{
		len = 4;
		mask = 0x07;
	}
	else if ((ch & 0xfc) == 0xf8)
	{
		len = 5;
		mask = 0x03;
	}
	else if ((ch & 0xfe) == 0xfc)
	{
		len = 6;
		mask = 0x01;
	}
	else
	{
		len = -1;
		mask = 0;
	}

	if (len < 0)
	{
		// 사용하지 않는 문자 코드
		// 0xFFFFFFFF
		return (uchar4)-1;
	}
	else
	{
		// UCS4로 변환
		uchar4 ret = (uchar4)(p[0] & mask);
		for (int i = 1; i < len; i++)
		{
			if ((p[i] & 0xC0) != 0x80)
				return 0;

			ret <<= 6;
			ret |= (p[i] & 0x3F);
		}

		return ret;
	}
}

//
const char* qn_u8nch(const char* s)
{
	static const char utf8_skips[256] =
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
	};
	return s + utf8_skips[*(const byte*)s];
}

//
size_t qn_u8len(const char* s)
{
#if false
	// 새니타이저에 안걸리는 방법
	size_t cnt = 0;
	while (*s)
		cnt += (*s++ & 0xC0) != 0x80;
	return cnt;
#else
	// 이 루틴은 새니타이저에 걸린다
#define MASK    ((size_t)(-1)/0xFF)
	const char* t;
	size_t cnt;

	// 초기 정렬되지 않은 아무 바이트 계산
	for (cnt = 0, t = s; (uintptr_t)t & (sizeof(size_t) - 1); t++)
	{
		const byte b = (byte)*t;

		if (b == '\0')
			goto pos_done;

		cnt += (b >> 7) & ((~b) >> 6);
	}

	// 완전한 블럭 계산
	for (;; t += sizeof(size_t))
	{
#ifdef __GNUC__
		__builtin_prefetch(&t[256], 0, 0);
#endif

		size_t i = *(const size_t*)t;	// NOLINT

		if ((i - MASK) & (~i) & (MASK * 0x80))
			break;

		i = ((i & (MASK * 0x80)) >> 7) & ((~i) >> 6);
		cnt += (i * MASK) >> ((sizeof(size_t) - 1) * 8);
	}

	//
	for (;; t++)
	{
		const byte b = (byte)*t;

		if (b == '\0')
			break;

		cnt += (b >> 7) & ((~b) >> 6);
	}

pos_done:
	return ((size_t)(t - s) - cnt);
#undef MASK
#endif
		}

//
char* qn_u8ncpy(char* dest, const char* src, size_t len)
{
	const char* t = src;

	while (len && *t)
	{
		t = qn_u8nch(t);
		len--;
	}

	len = (size_t)(t - src);
	memcpy(dest, src, len);
	dest[len] = '\0';

	return dest;
}

//
size_t qn_u8lcpy(char* dest, const char* src, size_t len)
{
	const char* t = src;

	while (len && *t)
	{
		t = qn_u8nch(t);
		len--;
	}

	len = (size_t)(t - src);
	memcpy(dest, src, len);
	dest[len] = '\0';

	return len;
}

//
int qn_u32ucb(uchar4 c, char* out)
{
	uchar4 first;
	int len;

	if (c < 0x80)
	{
		first = 0;
		len = 1;
	}
	else if (c < 0x800)
	{
		first = 0xc0;
		len = 2;
	}
	else if (c < 0x10000)
	{
		first = 0xe0;
		len = 3;
	}
	else if (c < 0x200000)
	{
		first = 0xf0;
		len = 4;
	}
	else if (c < 0x4000000)
	{
		first = 0xf8;
		len = 5;
	}
	else
	{
		first = 0xfc;
		len = 6;
	}

	if (out)
	{
		out[len] = '\0';
		for (int i = len - 1; i > 0; --i)
		{
			out[i] = (char)((c & 0x3f) | 0x80);
			c >>= 6;
		}
		out[0] = (char)(c | first);
	}

	return len;
}

// utf16 서로게이트를 ucs4로 바꿈
static uchar4 _utf16_surrogate(const uchar2 h, const uchar2 l)
{
	return (((uchar4)h - 0xD800) * 0x0400 + (uchar4)l - 0xDC00 + 0x010000);
}

//
int qn_u16ucb(const uchar2 high, const uchar2 low, char* out)
{
	const uchar4 ucs4 = _utf16_surrogate(high, low);
	return qn_u32ucb(ucs4, out);
}


//////////////////////////////////////////////////////////////////////////
// 문자열 변환

//
size_t qn_mbstowcs(wchar* outwcs, const size_t outsize, const char* inmbs, const size_t insize)
{
#ifdef _QN_WINDOWS_
	int len = MultiByteToWideChar(CP_THREAD_ACP, 0, inmbs, insize == 0 ? -1 : (int)insize, outwcs, (int)outsize);
	if (outwcs && len >= 0 && outsize > 0)
		outwcs[QN_MIN((int)outsize - 1, len)] = L'\0';
	return (int)len;
#else
	const size_t len = mbstowcs(outwcs, inmbs, outsize);
	if (outwcs && len >= 0 && outsize > 0)
		outwcs[QN_MIN(outsize - 1, len)] = L'\0';
	return (size_t)len;
#endif
}

//
size_t qn_wcstombs(char* outmbs, const size_t outsize, const wchar* inwcs, const size_t insize)
{
#ifdef _QN_WINDOWS_
	int len = WideCharToMultiByte(CP_THREAD_ACP, 0, inwcs, insize == 0 ? -1 : (int)insize, outmbs, (int)outsize, NULL, NULL);
	if (outmbs && len >= 0 && outsize > 0)
		outmbs[QN_MIN((int)outsize - 1, len)] = '\0';
	return (int)len;
#else
	size_t len = wcstombs(outmbs, inwcs, outsize);
	if (outmbs && len >= 0 && outsize > 0)
		outmbs[QN_MIN(outsize - 1, len)] = L'\0';
	return (size_t)len;
#endif
}

//
size_t qn_u8to32(uchar4* dest, const size_t destsize, const char* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	if (dest == NULL || destsize == 0)
	{
		// utf-8 -> ucs-4의 길이는.... 그냥 utf-8의 길이.
		return qn_u8len(src);
	}
	else
	{
		const size_t slen = srclen == 0 ? qn_u8len(src) : srclen;
		const size_t size = QN_MIN(destsize - 1, slen);

		const char* t;
		size_t i;
		for (t = src, i = 0; i < size; i++)
		{
			dest[i] = qn_u8cbn(t);
			t = qn_u8nch(t);
		}

		dest[i] = (uchar4)'\0';

		return size;
	}
}

//
size_t qn_u8to16(uchar2* dest, const size_t destsize, const char* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	if (dest == NULL || destsize == 0)
	{
		// utf-8 -> ucs-4의 길이는.... 그냥 utf-8의 길이.
		// 다만 서로게이트 문제는 어떻게 하나... 일단 이걸로 하고 나중에 고치자
		// .... 서로게이트 문제 원래 발생 안한다 [2024-01-12 kim]
		return qn_u8len(src);
	}
	else
	{
		const size_t slen = srclen == 0 ? qn_u8len(src) : srclen;
		const size_t size = QN_MIN(destsize - 1, slen);

		const char* t;
		size_t i;
		for (t = src, i = 0; i < size;)
		{
			const uchar4 ch = qn_u8cbn(t);

			if (ch < 0x010000)
				dest[i++] = (uchar2)ch;
			else
			{
				dest[i++] = (uchar2)((ch - 0x010000) / 0x0400 + 0xD800);
				dest[i++] = (uchar2)((ch - 0x010000) % 0x0400 + 0xDC00);
			}

			t = qn_u8nch(t);
		}

		dest[i] = (uchar2)'\0';

		return size;
	}
}

//
size_t qn_u32to8(char* dest, size_t destsize, const uchar4* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	const nint slen = (nint)srclen;
	size_t n, size;
	intptr_t i;
	uchar4 uc;

	if (destsize > 0)
	{
		destsize--;
		for (size = 0, i = 0; slen == 0 || i < slen; i++)
		{
			uc = src[i];

			if (!uc)
				break;

			if ((uc & 0x80000000) != 0)	// if (uc >= 0x80000000)
			{
				// 변경할 수 없는 문자가 포함됨..
				// 일단 그냥 못한다고 보내자
				return 0;
			}

			n = (uc < 0x80) ? 1 :
				(uc < 0x0800) ? 2 :
				(uc < 0x010000) ? 3 :
				(uc < 0x200000) ? 4 :
				(uc < 0x04000000) ? 5 :
				6;

			const size_t z = size + n;

			if (z >= destsize)
				break;

			size = z;
		}
	}
	else
	{
		for (size = 0, i = 0; slen == 0 || i < slen; i++)
		{
			uc = src[i];

			if (!uc)
				break;

			if ((uc & 0x80000000) != 0)	// if (uc >= 0x80000000)
			{
				// 변경할 수 없는 문자가 포함됨..
				// 일단 그냥 못한다고 보내자
				return 0;
			}

			n = (uc < 0x80) ? 1 :
				(uc < 0x0800) ? 2 :
				(uc < 0x010000) ? 3 :
				(uc < 0x200000) ? 4 :
				(uc < 0x04000000) ? 5 :
				6;

			size += n;
		}
	}

	if (dest && destsize > 0)
	{
		char* p = dest;

		for (i = 0; p < (dest + size); i++)
			p += qn_u32ucb(src[i], p);

		*p = '\0';
	}

	return size;
}

//
size_t qn_u16to8(char* dest, size_t destsize, const uchar2* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	const uchar2* cp = src;
	uchar4 hsg = 0;
	size_t size = 0;
	uchar4 uc;
	uchar2 ch;
	size_t n;

	if (destsize > 0)
	{
		destsize--;
		for (; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					uc = _utf16_surrogate((uchar2)hsg, ch);
					hsg = 0;
				}
				else
				{
					// 음... 변경할 수 없는 문자가 있네...
					// 일단 못한다고 보내자
					return 0;
				}
			}
			else
			{
				// 일반
				if (hsg)
				{
					// 상위 서로게이트가 있네?
					// 일단.. 문제가 있다고 판단
					return 0;
				}

				if (ch >= 0xD800 && ch < 0xDC00)
				{
					// 상위 서로게이트
					hsg = ch;
					continue;
				}

				uc = ch;
			}

			n = (uc < 0x80) ? 1 :
				(uc < 0x0800) ? 2 :
				(uc < 0x010000) ? 3 :
				(uc < 0x200000) ? 4 :
				(uc < 0x04000000) ? 5 :
				6;

			const size_t z = size + n;
			if (z >= destsize)
				break;
			size = z;
		}
	}
	else
	{
		for (; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					uc = _utf16_surrogate((uchar2)hsg, ch);
					hsg = 0;
				}
				else
				{
					// 음... 변경할 수 없는 문자가 있네...
					// 일단 못한다고 보내자
					return 0;
				}
			}
			else
			{
				// 일반
				if (hsg)
				{
					// 상위 서로게이트가 있네?
					// 일단.. 문제가 있다고 판단
					return 0;
				}

				if (ch >= 0xD800 && ch < 0xDC00)
				{
					// 상위 서로게이트
					hsg = ch;
					continue;
				}

				uc = ch;
			}

			n = (uc < 0x80) ? 1 :
				(uc < 0x0800) ? 2 :
				(uc < 0x010000) ? 3 :
				(uc < 0x200000) ? 4 :
				(uc < 0x04000000) ? 5 :
				6;

			size += n;
		}
	}

	if (hsg)
	{
		// 하위 서로게이트가 남음...
		// 못함
		return 0;
	}

	if (destsize > 0 && dest)
	{
		char* p = dest;
		hsg = 0;
		cp = src;

		for (; p < dest + size; cp++)
		{
			ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				uc = _utf16_surrogate((uchar2)hsg, ch);
				hsg = 0;
			}
			else if (ch >= 0xD800 && ch < 0xDC00)
			{
				// 상위 서로게이트
				hsg = ch;
				continue;
			}
			else
			{
				// 일반 글자
				uc = ch;
			}

			p += qn_u32ucb(uc, p);
		}

		*p = '\0';
	}

	return size;
}

//
size_t qn_u16to32(uchar4* dest, size_t destsize, const uchar2* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	uchar4 hsg = 0;
	size_t size = 0;

	if (destsize > 0)
	{
		destsize--;
		for (const uchar2* cp = src; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			const uchar2 ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					_utf16_surrogate((uchar2)hsg, ch);
					hsg = 0;
				}
				else
				{
					// 음... 변경할 수 없는 문자가 있네...
					// 일단 못한다고 보내자
					return 0;
				}
			}
			else
			{
				// 일반
				if (hsg)
				{
					// 상위 서로게이트가 있네?
					// 일단.. 문제가 있다고 판단
					return 0;
				}

				if (ch >= 0xD800 && ch < 0xDC00)
				{
					// 상위 서로게이트
					hsg = ch;
					continue;
				}
			}

			const size_t z = size + 1;

			if (z >= destsize)
				break;

			size = z;
		}
	}
	else
	{
		for (const uchar2* cp = src; (srclen == 0 || (size_t)(cp - src) < srclen) && *cp; cp++)
		{
			const uchar2 ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				if (hsg)
				{
					_utf16_surrogate((uchar2)hsg, ch);
					hsg = 0;
				}
				else
				{
					// 음... 변경할 수 없는 문자가 있네...
					// 일단 못한다고 보내자
					return 0;
				}
			}
			else
			{
				// 일반
				if (hsg)
				{
					// 상위 서로게이트가 있네?
					// 일단.. 문제가 있다고 판단
					return 0;
				}

				if (ch >= 0xD800 && ch < 0xDC00)
				{
					// 상위 서로게이트
					hsg = ch;
					continue;
				}
			}


			size++;
		}
	}

	if (hsg)
	{
		// 하위 서로게이트가 남음...
		// 못함
		return 0;
	}

	if (destsize > 0 && dest)
	{
		uchar4* p = dest;
		uchar4 uc;

		hsg = 0;

		for (const uchar2* cp = src; p < dest + size; cp++)
		{
			const uchar2 ch = *cp;

			if (ch >= 0xDC00 && ch < 0xE000)
			{
				// 하위 서로게이트
				uc = _utf16_surrogate((uchar2)hsg, ch);
				hsg = 0;
			}
			else if (ch >= 0xD800 && ch < 0xDC00)
			{
				// 상위 서로게이트
				hsg = ch;
				continue;
			}
			else
			{
				// 일반 글자
				uc = ch;
			}

			*p = uc;
			p++;
		}

		*p = '\0';
	}

	return size;
}

//
size_t qn_u32to16(uchar2* dest, size_t destsize, const uchar4* src, const size_t srclen)
{
	qn_return_when_fail(src, 0);

	size_t size = 0;
	const intptr_t slen = (intptr_t)srclen;
	size_t z;
	intptr_t i;

	if (destsize > 0)
	{
		destsize--;
		for (i = 0; (slen == 0 || i < slen) && src[i]; i++)
		{
			const uchar4 uc = src[i];

			if (uc < 0xD800)
				z = 1;		// NOLINT(bugprone-branch-clone)
			else if (uc < 0xE000)
				return 0;	// NOLINT(bugprone-branch-clone)
			else if (uc < 0x010000)
				z = 1;
			else if (uc < 0x110000)
				z = 2;
			else
				return 0;

			z += size;
			if (z >= destsize)
				break;
			size = z;
		}
	}
	else
	{
		for (i = 0; (slen == 0 || i < slen) && src[i]; i++)
		{
			const uchar4 uc = src[i];

			if (uc < 0xD800)
				size++;		// NOLINT(bugprone-branch-clone)
			else if (uc < 0xE000)
				return 0;	// NOLINT(bugprone-branch-clone)
			else if (uc < 0x010000)
				size++;
			else if (uc < 0x110000)
				size += 2;
			else
				return 0;
		}
	}

	if (destsize > 0 && dest)
	{
		uchar2* p;
		for (p = dest, i = 0, z = 0; z < size; i++)
		{
			const uchar4 uc = src[i];

			if (uc < 0x010000)
				p[z++] = (uchar2)uc;
			else
			{
				p[z++] = (uchar2)((uc - 0x010000) / 0x0400 + 0xD800);
				p[z++] = (uchar2)((uc - 0x010000) % 0x0400 + 0xDC00);
			}
		}

		p[z] = (uchar2)'\0';
	}

	return size;
		}

#ifdef QS_NO_MEMORY_PROFILE
#define DEF_UTF_DUP(name, in_type, out_type)\
	out_type* qn_a_##name(const in_type* src, size_t srclen) {\
		size_t len=qn_##name(NULL,0,src,srclen)+1; qn_return_on_ok(len<2,NULL);\
		out_type* buf=qn_a_alloc(len*sizeof(out_type), false); qn_##name(buf,len,src,srclen);/* NOLINT */\
		return buf; }
#else
#define DEF_UTF_DUP(name, in_type, out_type)\
	out_type* qn_a_i_##name(const in_type* src, size_t srclen, const char* desc, size_t line) {\
		size_t len=qn_##name(NULL,0,src,srclen)+1; qn_return_on_ok(len<2,NULL);\
		out_type* buf=qn_a_i_alloc(len*sizeof(out_type), false, desc, line); qn_##name(buf,len,src,srclen);/* NOLINT */\
		return buf; }
#endif
DEF_UTF_DUP(mbstowcs, char, wchar)
DEF_UTF_DUP(wcstombs, wchar, char)
DEF_UTF_DUP(u8to32, char, uchar4)
DEF_UTF_DUP(u8to16, char, uchar2)
DEF_UTF_DUP(u32to8, uchar4, char)
DEF_UTF_DUP(u16to8, uchar2, char)
DEF_UTF_DUP(u16to32, uchar2, uchar4)
DEF_UTF_DUP(u32to16, uchar4, uchar2)
#undef DEF_UTF_DUP

#ifdef _MSC_VER
#pragma warning(pop)
#endif
