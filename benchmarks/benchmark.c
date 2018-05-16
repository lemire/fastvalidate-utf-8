#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "benchmark.h"
#include "simdutf8check.h"
#include "simdasciicheck.h"

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

void populate(char *data, size_t N) {
  for (size_t i = 0; i < N; i++)
    data[i] = rand() & 0x7f;
}

void demo(size_t N) {
  printf("string size = %zu \n", N);
  char *data = (char *)malloc(N);
  bool expected = true; // it is all ascii?
  int repeat = 5;
  printf("We are feeding ascii so it is always going to be ok.\n");

  BEST_TIME(validate_utf8_fast(data, N), expected, populate(data, N), repeat, N,
            true);

  BEST_TIME(validate_ascii_fast(data, N), expected, populate(data, N),
            repeat, N, true);
#ifdef __linux__
  BEST_TIME_LINUX(validate_utf8_fast(data, N), expected, populate(data, N), repeat, N,
            true);

  BEST_TIME_LINUX(validate_ascii_fast(data, N), expected, populate(data, N),
            repeat, N, true);
#endif
  printf("\n\n");

  free(data);
}

int main() {
  demo(65536);
  printf("We are feeding ascii so it is always going to be ok.\n");
}
