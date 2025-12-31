#include "hash.h"
#include <stdio.h>

size_t strnhash(const void *key, size_t key_size, const void *seed) {
	const unsigned char *str = key;
	const size_t *seed_buf = seed;
	size_t hash = 0;
	char h[sizeof(size_t)] = {0};
	for (size_t i = 0; i < key_size && str[i] != '\0'; i++) {
		hash ^= seed_buf[str[i] % 128U];
	}

	return hash;
}

// MurmurHash3-inspired hash for size_t values
size_t zhash2(const void *key, size_t key_size, const void *seed) {
	size_t n = *(const size_t *)key;
	const size_t *seed_ptr = (const size_t *)seed;
	size_t h = seed_ptr ? *seed_ptr : 0xc15d213aa4d7a795UL;

	// Mix bits
	n ^= n >> 33;
	n *= 0xff51afd7ed558ccdUL;
	n ^= n >> 33;

	return h ^ n;
}

size_t xhash(const void *key, size_t key_size, const void *seed) {
	size_t n = *(const size_t *)key;
	const size_t *seed_ptr = (const size_t *)seed;
	size_t h = seed_ptr ? *seed_ptr : 0xc15d213aa4d7a795UL;

	// Mix bits
	n ^= n >> 33;
	n *= 0xff51afd7ed558ccdUL;
	n ^= n >> 33;

	return n;
}



#ifdef __ARM_NEON__
#include <arm_neon.h>

poly8_t table[16] = {0, 27, 54, 45, 108, 119, 90, 65, 216, 195, 238, 245, 180, 175, 130, 153};

poly64_t pmul(poly64_t a, poly64_t b) {
	poly64_t p = (poly64_t)a, q = (poly64_t)b, r = (poly64_t)27;

	poly128_t z = vmull_p64(p, q);
	poly128_t w = vmull_p64(r, z>>64);

	return (poly64_t)z ^ (poly64_t)w ^ (poly64_t)table[(size_t)(w>>64)];
}

#elif defined(__i386__) || defined(__x86_64__)
#include <immintrin.h>

size_t pmul(size_t a, size_t b) {
	__m128i p = _mm_set1_epi64x((long long)a), q = _mm_set1_epi64x((long long)b), r = _mm_cvtsi64_si128(27);

	__i128m z = _mm_clmulepi64_si128(a, b);
	__i128m w = _mm_clmulepi64_si128(z, r);
	__m128i table = _mm_setr_epi8(0, 27, 54, 45, 108, 119, 90, 65, 216, 195, 238, 245, 180, 175, 130, 153);

	__m128i y = _mm_shuffle_epi8(table, _mm_srli_si128(z,8));
	__m128i temp1 = _mm_xor_si128(z, a);
	return (size_t)_mm_cvtsi128_si64(_mm_xor_si128(temp1, y));
}

#else

size_t pmul(size_t a, size_t b) {
	size_t p = 0;
	while (a != 0 && b != 0) {
        if (b & 1) /* if the polynomial for b has a constant term, add the corresponding a to p */
            p ^= a; /* addition in GF(2^m) is an XOR of the polynomial coefficients */

        if (a & 0x8000000000000000UL) /* GF modulo: if a has a nonzero term x^63, then must be reduced when it becomes x^64 */
            a = (a << 1) ^ 0x11b; /* subtract (XOR) the primitive polynomial x^64 + x^4 + x^3 + x + 1 (0b1_0001_1011) – you can change it but it must be irreducible */
        else
            a <<= 1; /* equivalent to a*x */
        b >>= 1;
	}

	return p;
}

#endif

/* default hash for integer keys */
size_t zhash(const void *key, size_t key_size, const void *seed) {
	size_t *a = seed;
	size_t p = *(size_t *)key;


	size_t q = (a[0] + (size_t)pmul(p, a[1]));
	//size_t q = a[0] ^ pmul(a[1] ^ pmul(a[2], p), p);
	size_t K = 3;
	
	return q;
}

/* default hash for integer keys */
size_t id_hash(const void *key, size_t key_size, const void *seed) {
	size_t *a = seed;
	size_t p = *(size_t *)key;


	//size_t q = (a[0] + (size_t)pmul(p, a[1]));
	//size_t q = a[0] ^ pmul(a[1] ^ pmul(a[2], p), p);
	
	
	return p;
}