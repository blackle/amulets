#include "SHA256.h"
#include <cstring>
#include <sstream>
#include <iomanip>
#include <stdint.h>
#include <immintrin.h>

/*
This code is from StackOverflow user jww, posted here: https://stackoverflow.com/a/41495032
per StackOverflow, code is licensed under CC-BY-SA 3.0 - https://creativecommons.org/licenses/by-sa/3.0/
*/
void sha256_ishaext(uint32_t state[8], const uint8_t input[])
{
    __m128i STATE0, STATE1;
    __m128i MSG, TMP, MASK;
    __m128i TMSG0, TMSG1, TMSG2, TMSG3;
    __m128i ABEF_SAVE, CDGH_SAVE;

    // Load initial values
    TMP = _mm_loadu_si128((__m128i*) &state[0]);
    STATE1 = _mm_loadu_si128((__m128i*) &state[4]);
    MASK = _mm_set_epi64x(0x0c0d0e0f08090a0bULL, 0x0405060700010203ULL);

    TMP = _mm_shuffle_epi32(TMP, 0xB1); // CDAB
    STATE1 = _mm_shuffle_epi32(STATE1, 0x1B); // EFGH
    STATE0 = _mm_alignr_epi8(TMP, STATE1, 8); // ABEF
    STATE1 = _mm_blend_epi16(STATE1, TMP, 0xF0); // CDGH

    // Save current hash
    ABEF_SAVE = STATE0;
    CDGH_SAVE = STATE1;

    // Rounds 0-3
    MSG = _mm_loadu_si128((const __m128i*) (input+0));
    TMSG0 = _mm_shuffle_epi8(MSG, MASK);
    MSG = _mm_add_epi32(TMSG0, _mm_set_epi64x(0xE9B5DBA5B5C0FBCFULL, 0x71374491428A2F98ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

    // Rounds 4-7
    TMSG1 = _mm_loadu_si128((const __m128i*) (input+16));
    TMSG1 = _mm_shuffle_epi8(TMSG1, MASK);
    MSG = _mm_add_epi32(TMSG1, _mm_set_epi64x(0xAB1C5ED5923F82A4ULL, 0x59F111F13956C25BULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG0 = _mm_sha256msg1_epu32(TMSG0, TMSG1);

    // Rounds 8-11
    TMSG2 = _mm_loadu_si128((const __m128i*) (input+32));
    TMSG2 = _mm_shuffle_epi8(TMSG2, MASK);
    MSG = _mm_add_epi32(TMSG2, _mm_set_epi64x(0x550C7DC3243185BEULL, 0x12835B01D807AA98ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG1 = _mm_sha256msg1_epu32(TMSG1, TMSG2);

    // Rounds 12-15
    TMSG3 = _mm_loadu_si128((const __m128i*) (input+48));
    TMSG3 = _mm_shuffle_epi8(TMSG3, MASK);
    MSG = _mm_add_epi32(TMSG3, _mm_set_epi64x(0xC19BF1749BDC06A7ULL, 0x80DEB1FE72BE5D74ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG3, TMSG2, 4);
    TMSG0 = _mm_add_epi32(TMSG0, TMP);
    TMSG0 = _mm_sha256msg2_epu32(TMSG0, TMSG3);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG2 = _mm_sha256msg1_epu32(TMSG2, TMSG3);

    // Rounds 16-19
    MSG = _mm_add_epi32(TMSG0, _mm_set_epi64x(0x240CA1CC0FC19DC6ULL, 0xEFBE4786E49B69C1ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG0, TMSG3, 4);
    TMSG1 = _mm_add_epi32(TMSG1, TMP);
    TMSG1 = _mm_sha256msg2_epu32(TMSG1, TMSG0);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG3 = _mm_sha256msg1_epu32(TMSG3, TMSG0);

    // Rounds 20-23
    MSG = _mm_add_epi32(TMSG1, _mm_set_epi64x(0x76F988DA5CB0A9DCULL, 0x4A7484AA2DE92C6FULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG1, TMSG0, 4);
    TMSG2 = _mm_add_epi32(TMSG2, TMP);
    TMSG2 = _mm_sha256msg2_epu32(TMSG2, TMSG1);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG0 = _mm_sha256msg1_epu32(TMSG0, TMSG1);

    // Rounds 24-27
    MSG = _mm_add_epi32(TMSG2, _mm_set_epi64x(0xBF597FC7B00327C8ULL, 0xA831C66D983E5152ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG2, TMSG1, 4);
    TMSG3 = _mm_add_epi32(TMSG3, TMP);
    TMSG3 = _mm_sha256msg2_epu32(TMSG3, TMSG2);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG1 = _mm_sha256msg1_epu32(TMSG1, TMSG2);

    // Rounds 28-31
    MSG = _mm_add_epi32(TMSG3, _mm_set_epi64x(0x1429296706CA6351ULL,  0xD5A79147C6E00BF3ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG3, TMSG2, 4);
    TMSG0 = _mm_add_epi32(TMSG0, TMP);
    TMSG0 = _mm_sha256msg2_epu32(TMSG0, TMSG3);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG2 = _mm_sha256msg1_epu32(TMSG2, TMSG3);

    // Rounds 32-35
    MSG = _mm_add_epi32(TMSG0, _mm_set_epi64x(0x53380D134D2C6DFCULL, 0x2E1B213827B70A85ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG0, TMSG3, 4);
    TMSG1 = _mm_add_epi32(TMSG1, TMP);
    TMSG1 = _mm_sha256msg2_epu32(TMSG1, TMSG0);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG3 = _mm_sha256msg1_epu32(TMSG3, TMSG0);

    // Rounds 36-39
    MSG = _mm_add_epi32(TMSG1, _mm_set_epi64x(0x92722C8581C2C92EULL, 0x766A0ABB650A7354ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG1, TMSG0, 4);
    TMSG2 = _mm_add_epi32(TMSG2, TMP);
    TMSG2 = _mm_sha256msg2_epu32(TMSG2, TMSG1);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG0 = _mm_sha256msg1_epu32(TMSG0, TMSG1);

    // Rounds 40-43
    MSG = _mm_add_epi32(TMSG2, _mm_set_epi64x(0xC76C51A3C24B8B70ULL, 0xA81A664BA2BFE8A1ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG2, TMSG1, 4);
    TMSG3 = _mm_add_epi32(TMSG3, TMP);
    TMSG3 = _mm_sha256msg2_epu32(TMSG3, TMSG2);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG1 = _mm_sha256msg1_epu32(TMSG1, TMSG2);

    // Rounds 44-47
    MSG = _mm_add_epi32(TMSG3, _mm_set_epi64x(0x106AA070F40E3585ULL, 0xD6990624D192E819ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG3, TMSG2, 4);
    TMSG0 = _mm_add_epi32(TMSG0, TMP);
    TMSG0 = _mm_sha256msg2_epu32(TMSG0, TMSG3);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG2 = _mm_sha256msg1_epu32(TMSG2, TMSG3);

    // Rounds 48-51
    MSG = _mm_add_epi32(TMSG0, _mm_set_epi64x(0x34B0BCB52748774CULL, 0x1E376C0819A4C116ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG0, TMSG3, 4);
    TMSG1 = _mm_add_epi32(TMSG1, TMP);
    TMSG1 = _mm_sha256msg2_epu32(TMSG1, TMSG0);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);
    TMSG3 = _mm_sha256msg1_epu32(TMSG3, TMSG0);

    // Rounds 52-55
    MSG = _mm_add_epi32(TMSG1, _mm_set_epi64x(0x682E6FF35B9CCA4FULL, 0x4ED8AA4A391C0CB3ULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG1, TMSG0, 4);
    TMSG2 = _mm_add_epi32(TMSG2, TMP);
    TMSG2 = _mm_sha256msg2_epu32(TMSG2, TMSG1);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

    // Rounds 56-59
    MSG = _mm_add_epi32(TMSG2, _mm_set_epi64x(0x8CC7020884C87814ULL, 0x78A5636F748F82EEULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    TMP = _mm_alignr_epi8(TMSG2, TMSG1, 4);
    TMSG3 = _mm_add_epi32(TMSG3, TMP);
    TMSG3 = _mm_sha256msg2_epu32(TMSG3, TMSG2);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

    // Rounds 60-63
    MSG = _mm_add_epi32(TMSG3, _mm_set_epi64x(0xC67178F2BEF9A3F7ULL, 0xA4506CEB90BEFFFAULL));
    STATE1 = _mm_sha256rnds2_epu32(STATE1, STATE0, MSG);
    MSG = _mm_shuffle_epi32(MSG, 0x0E);
    STATE0 = _mm_sha256rnds2_epu32(STATE0, STATE1, MSG);

    // Add values back to state
    STATE0 = _mm_add_epi32(STATE0, ABEF_SAVE);
    STATE1 = _mm_add_epi32(STATE1, CDGH_SAVE);

    TMP = _mm_shuffle_epi32(STATE0, 0x1B); // FEBA
    STATE1 = _mm_shuffle_epi32(STATE1, 0xB1); // DCHG
    STATE0 = _mm_blend_epi16(TMP, STATE1, 0xF0); // DCBA
    STATE1 = _mm_alignr_epi8(STATE1, TMP, 8); // ABEF

    // Save state
    _mm_storeu_si128((__m128i*) &state[0], STATE0);
    _mm_storeu_si128((__m128i*) &state[4], STATE1);
}

SHA256::SHA256(): m_blocklen(0), m_bitlen(0) {
	m_state[0] = 0x6a09e667;
	m_state[1] = 0xbb67ae85;
	m_state[2] = 0x3c6ef372;
	m_state[3] = 0xa54ff53a;
	m_state[4] = 0x510e527f;
	m_state[5] = 0x9b05688c;
	m_state[6] = 0x1f83d9ab;
	m_state[7] = 0x5be0cd19;
}

void SHA256::update(const uint8_t * data, size_t length) {
	for (size_t i = 0 ; i < length ; i++) {
		m_data[m_blocklen++] = data[i];
		if (m_blocklen == 64) {
			transform();

			// End of the block
			m_bitlen += 512;
			m_blocklen = 0;
		}
	}
}

void SHA256::update(const std::string &data) {
	update(reinterpret_cast<const uint8_t*> (data.c_str()), data.size());
}

uint8_t * SHA256::digest() {
	uint8_t * hash = new uint8_t[32];

	pad();
	revert(hash);

	return hash;
}

uint32_t SHA256::rotr(uint32_t x, uint32_t n) {
	return (x >> n) | (x << (32 - n));
}

uint32_t SHA256::choose(uint32_t e, uint32_t f, uint32_t g) {
	return (e & f) ^ (~e & g);
}

uint32_t SHA256::majority(uint32_t a, uint32_t b, uint32_t c) {
	return (a & (b | c)) | (b & c);
}

uint32_t SHA256::sig0(uint32_t x) {
	return SHA256::rotr(x, 7) ^ SHA256::rotr(x, 18) ^ (x >> 3);
}

uint32_t SHA256::sig1(uint32_t x) {
	return SHA256::rotr(x, 17) ^ SHA256::rotr(x, 19) ^ (x >> 10);
}

void SHA256::transform() {
	sha256_ishaext(m_state, m_data);
}

void SHA256::pad() {

	uint64_t i = m_blocklen;
	uint8_t end = m_blocklen < 56 ? 56 : 64;

	m_data[i++] = 0x80; // Append a bit 1
	while (i < end) {
		m_data[i++] = 0x00; // Pad with zeros
	}

	if(m_blocklen >= 56) {
		transform();
		memset(m_data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	m_bitlen += m_blocklen * 8;
	m_data[63] = m_bitlen;
	m_data[62] = m_bitlen >> 8;
	m_data[61] = m_bitlen >> 16;
	m_data[60] = m_bitlen >> 24;
	m_data[59] = m_bitlen >> 32;
	m_data[58] = m_bitlen >> 40;
	m_data[57] = m_bitlen >> 48;
	m_data[56] = m_bitlen >> 56;
	transform();
}

void SHA256::revert(uint8_t * hash) {
	// SHA uses big endian byte ordering
	// Revert all bytes
	for (uint8_t i = 0 ; i < 4 ; i++) {
		for(uint8_t j = 0 ; j < 8 ; j++) {
			hash[i + (j * 4)] = (m_state[j] >> (24 - i * 8)) & 0x000000ff;
		}
	}
}

std::string SHA256::toString(const uint8_t * digest) {
	std::stringstream s;
	s << std::setfill('0') << std::hex;

	for(uint8_t i = 0 ; i < 32 ; i++) {
		s << std::setw(2) << (unsigned int) digest[i];
	}

	return s.str();
}
