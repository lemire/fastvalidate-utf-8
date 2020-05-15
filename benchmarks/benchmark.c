#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "benchmark.h"
#include "hoehrmann.h"
#include "simdasciicheck.h"
#include "simdasciicheck_avx512.h"
#include "simdutf8check.h"
#include "simdutf8check_avx512.h"

#include <x86intrin.h>
/*
 * legal utf-8 byte sequence
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - page 94
 *
 *  Code Points        1st       2s       3s       4s
 * U+0000..U+007F     00..7F
 * U+0080..U+07FF     C2..DF   80..BF
 * U+0800..U+0FFF     E0       A0..BF   80..BF
 * U+1000..U+CFFF     E1..EC   80..BF   80..BF
 * U+D000..U+D7FF     ED       80..9F   80..BF
 * U+E000..U+FFFF     EE..EF   80..BF   80..BF
 * U+10000..U+3FFFF   F0       90..BF   80..BF   80..BF
 * U+40000..U+FFFFF   F1..F3   80..BF   80..BF   80..BF
 * U+100000..U+10FFFF F4       80..8F   80..BF   80..BF
 *
 */

// populate a string with null chars
void populate(char *data, size_t N) {
  for (size_t i = 0; i < N; i++)
    data[i] = rand() & 0x7f;
  // adding null termination
  data[N] = '\0';
}

#define GCC_COMPILER (defined(__GNUC__) && !defined(__clang__) && !defined(__llvm__) && !defined(__INTEL_COMPILER))

#ifdef GCC_COMPILER
__attribute__((optimize("no-tree-vectorize"))) // works only with GCC
static bool
validate_ascii_nosimd(const char *src, size_t len) {
  const char *end = src + len;
  uint64_t mask1 = 0, mask2 = 0, mask3 = 0, mask4 = 0;

  for (; src < end - 32; src += 32) {
    const uint64_t *p = (const uint64_t *)src;
    mask1 |= p[0];
    mask2 |= p[1];
    mask3 |= p[2];
    mask4 |= p[3];
  }
  for (; src < end - 8; src += 8) {
    const uint64_t *p = (const uint64_t *)src;
    mask1 |= p[0];
  }
  uint8_t tail_mask = 0;
  for (; src < end; src++) {
    tail_mask |= *(const uint8_t *)src;
  }
  uint64_t final_mask = mask1 | mask2 | mask3 | mask4 | tail_mask;
  return !(final_mask & 0x8080808080808080);
}

static bool validate_ascii_nointrin(const char *src, size_t len) {
  const char *end = src + len;
  uint64_t mask1 = 0, mask2 = 0, mask3 = 0, mask4 = 0;

  for (; src < end - 32; src += 32) {
    const uint64_t *p = (const uint64_t *)src;
    mask1 |= p[0];
    mask2 |= p[1];
    mask3 |= p[2];
    mask4 |= p[3];
  }
  for (; src < end - 8; src += 8) {
    const uint64_t *p = (const uint64_t *)src;
    mask1 |= p[0];
  }
  uint8_t tail_mask = 0;
  for (; src < end; src++) {
    tail_mask |= *(const uint8_t *)src;
  }
  uint64_t final_mask = mask1 | mask2 | mask3 | mask4 | tail_mask;
  return !(final_mask & 0x8080808080808080);
}
#endif

// generate a string having at least length N
// can exceed by up to 3 chars, returns the actual length
size_t populate_utf8(char *data, size_t N) {
  size_t i = 0;
  for (; i < N;) {
    int w = rand() & 0xFF;
    if (w < 0x80) {
      data[i++] = 0x20; // w;
    } else if (w < 0xE0) {
      data[i++] = 0xC2 + rand() % (0xDF - 0xC2 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    } else if (w == 0xE0) {
      data[i++] = w;
      data[i++] = 0xA0 + rand() % (0xBF - 0xA0 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    } else if (w <= 0xEC) {
      data[i++] = w;
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    } else if (w == 0xED) {
      data[i++] = w;
      data[i++] = 0x80 + rand() % (0x9F - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    } else if (w <= 0xEF) {
      data[i++] = w;
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    } else if (w < 0xF0) {
      data[i++] = 0xF1 + rand() % (0xF3 - 0xF1 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    } else if (w == 0xF0) {
      data[i++] = w;
      data[i++] = 0x90 + rand() % (0xBF - 0x90 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    } else if (w <= 0xF3) {
      data[i++] = 0xF1 + rand() % (0xF3 - 0xF1 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    } else if (w == 0xF4) {
      data[i++] = w;
      data[i++] = 0x80 + rand() % (0x8F - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
      data[i++] = 0x80 + rand() % (0xBF - 0x80 + 1);
    }
  }
  data[i] = '\0';
  return i;
}

// copied in part from Guava
static bool validate_utf8(const char *b, size_t length) {
  const unsigned char *bytes = (const unsigned char *)b;
  for (size_t index = 0;;) {
    unsigned char byte1;

    do { // fast ASCII Path
      if (index >= length) {
        return true;
      }
      byte1 = bytes[index++];
    } while (byte1 < 0x80);
    if (byte1 < 0xE0) {
      // Two-byte form.
      if (index == length) {
        return false;
      }
      if (byte1 < 0xC2 || bytes[index++] > 0xBF) {
        return false;
      }
    } else if (byte1 < 0xF0) {
      // Three-byte form.
      if (index + 1 >= length) {
        return false;
      }
      unsigned char byte2 = bytes[index++];
      if (byte2 > 0xBF
          // Overlong? 5 most significant bits must not all be zero.
          || (byte1 == 0xE0 && byte2 < 0xA0)
          // Check for illegal surrogate codepoints.
          || (byte1 == 0xED && 0xA0 <= byte2)
          // Third byte trailing-byte test.
          || bytes[index++] > 0xBF) {
        return false;
      }
    } else {

      // Four-byte form.
      if (index + 2 >= length) {
        return false;
      }
      int byte2 = bytes[index++];
      if (byte2 > 0xBF
          // Check that 1 <= plane <= 16. Tricky optimized form of:
          // if (byte1 > (byte) 0xF4
          //     || byte1 == (byte) 0xF0 && byte2 < (byte) 0x90
          //     || byte1 == (byte) 0xF4 && byte2 > (byte) 0x8F)
          || (((byte1 << 28) + (byte2 - 0x90)) >> 30) != 0
          // Third byte trailing-byte test
          || bytes[index++] > 0xBF
          // Fourth byte trailing-byte test
          || bytes[index++] > 0xBF) {
        return false;
      }
    }
  }
}

void demo(size_t N) {
  printf("string size = %zu \n", N);
  char *data = (char *)malloc(N);
  bool expected = true; // it is all ascii?
  int repeat = 5;
  printf("We are feeding ascii so it is always going to be ok.\n");
  printf("It favors schemes that skip ASCII characters.\n");

  BEST_TIME(validate_utf8(data, N), expected, populate(data, N), repeat, N,
            true);
  BEST_TIME(validate_utf8_fast(data, N), expected, populate(data, N), repeat, N,
            true);
  BEST_TIME(shiftless_validate_dfa_utf8_branchless(data, N + 1), expected,
            populate(data, N), repeat, N, true);
  BEST_TIME(shiftless_validate_dfa_utf8_branchless(data, N + 1), expected,
            populate(data, N), repeat, N, true);
  BEST_TIME(shiftless_validate_dfa_utf8(data, N + 1), expected, populate(data, N),
            repeat, N, true);
  BEST_TIME(shiftless_validate_dfa_utf8(data, N + 1), expected, populate(data, N),
            repeat, N, true);
  BEST_TIME(validate_dfa_utf8_double(data, N + 1), expected, populate(data, N),
            repeat, N, true);
  BEST_TIME(shiftless_validate_dfa_utf8_double(data, N + 1), expected, populate(data, N),
            repeat, N, true);
#ifdef __AVX2__
  BEST_TIME(validate_utf8_fast_avx(data, N), expected, populate(data, N),
            repeat, N, true);
  BEST_TIME(validate_utf8_fast_avx_asciipath(data, N), expected,
            populate(data, N), repeat, N, true);
#endif

#ifdef AVX512_IMPLEMENTATION
  BEST_TIME(validate_utf8_fast_avx512(data, N), expected, populate(data, N),
            repeat, N, true);
  BEST_TIME(validate_utf8_fast_avx512_asciipath(data, N), expected,
            populate(data, N), repeat, N, true);
#endif

  BEST_TIME(validate_ascii_fast(data, N), expected, populate(data, N), repeat,
            N, true);

#ifdef __AVX2__
  BEST_TIME(validate_ascii_fast_avx(data, N), expected, populate(data, N),
            repeat, N, true);
#endif
#ifdef AVX512_IMPLEMENTATION
  BEST_TIME(validate_ascii_fast_avx512(data, N), expected, populate(data, N),
            repeat, N, true);
#endif
#ifdef GCC_COMPILER
  BEST_TIME(validate_ascii_nosimd(data, N), expected, populate(data, N), repeat,
            N, true);
#endif
  BEST_TIME(validate_ascii_nointrin(data, N), expected, populate(data, N),
            repeat, N, true);
#ifdef __linux__
  BEST_TIME_LINUX(validate_utf8_fast(data, N), expected, populate(data, N),
                  repeat, N, true);

  BEST_TIME_LINUX(validate_ascii_fast(data, N), expected, populate(data, N),
                  repeat, N, true);
#endif
  printf("\n\n");

  free(data);
}

void demo_utf8(size_t N) {
  printf("string size (approx) = %zu \n", N);
  char *data = (char *)malloc(2 * N);
  size_t actualN;
  bool expected = true; // it is all utf8?
  int repeat = 5;
  printf("Producing random-looking UTF-8\n");

  BEST_TIME(validate_utf8(data, actualN), expected,
            actualN = populate_utf8(data, N), repeat, N, true);

  BEST_TIME(validate_utf8(data, actualN), expected,
            actualN = populate_utf8(data, N), repeat, N, true);

  BEST_TIME(shiftless_validate_dfa_utf8_branchless(data, actualN + 1), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
  BEST_TIME(shiftless_validate_dfa_utf8_branchless(data, actualN + 1), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
  BEST_TIME(shiftless_validate_dfa_utf8(data, actualN + 1), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
  BEST_TIME(shiftless_validate_dfa_utf8(data, actualN + 1), expected,
            actualN = populate_utf8(data, N), repeat, N, true);


  BEST_TIME(validate_dfa_utf8_double(data, actualN + 1), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
  BEST_TIME(shiftless_validate_dfa_utf8_double(data, actualN + 1), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
#ifdef __AVX2__
  BEST_TIME(validate_utf8_fast_avx(data, actualN), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
  BEST_TIME(validate_utf8_fast_avx_asciipath(data, actualN), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
#endif
#ifdef AVX512_IMPLEMENTATION
  BEST_TIME(validate_utf8_fast_avx512(data, actualN), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
  BEST_TIME(validate_utf8_fast_avx512_asciipath(data, actualN), expected,
            actualN = populate_utf8(data, N), repeat, N, true);
#endif
  printf("\n\n");
  free(data);
}

int main() {
  demo(65536);
  demo_utf8(65536);
  printf("Warning: the 'double' schemes are not guarantee to validate, they are an experiment in performance.\n");
}
