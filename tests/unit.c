#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simdutf8check.h"
#include "simdasciicheck.h"
#include "simdutf8check_avx512.h"
#include "simdasciicheck_avx512.h"
#include "../benchmarks/hoehrmann.h"// not part of the library, but let us test in any case

void test() {
  size_t N = 128;
  char buffer[N];
  // additional tests are from autobahn websocket testsuite https://github.com/crossbario/autobahn-testsuite/tree/master/autobahntestsuite/autobahntestsuite/case
  char *goodsequences[] = {"a", "\xc3\xb1", "\xe2\x82\xa1", "\xf0\x90\x8c\xbc", "안녕하세요, 세상",
                            "\xc2\x80", // 6.7.2
                            "\xf0\x90\x80\x80", // 6.7.4
                            "\xee\x80\x80" // 6.11.2
                            };
  char *badsequences[] = {"\xc3\x28",         // 0
                          "\xa0\xa1",         // 1
                          "\xe2\x28\xa1",     // 2
                          "\xe2\x82\x28",     // 3
                          "\xf0\x28\x8c\xbc", // 4
                          "\xf0\x90\x28\xbc", // 5
                          "\xf0\x28\x8c\x28", // 6
                          "\xc0\x9f",         // 7
                          "\xf5\xff\xff\xff", // 8
                          "\xed\xa0\x81", // 9
                          "\xf8\x90\x80\x80\x80", //10
                          "123456789012345\xed", //11
                          "123456789012345\xf1", //12
                          "123456789012345\xc2", //13
                          "\xC2\x7F", // 14
                          "\xce", // 6.6.1
                          "\xce\xba\xe1", // 6.6.3
                          "\xce\xba\xe1\xbd", // 6.6.4
                          "\xce\xba\xe1\xbd\xb9\xcf", // 6.6.6
                          "\xce\xba\xe1\xbd\xb9\xcf\x83\xce", // 6.6.8
                          "\xce\xba\xe1\xbd\xb9\xcf\x83\xce\xbc\xce", // 6.6.10
                          "\xdf", // 6.14.6
                          "\xef\xbf" // 6.14.7
                        };
  for (size_t i = 0; i < 8; i++) {
    printf("good sequence %zu \n", i);
    size_t len = strlen(goodsequences[i]);
    if(!validate_dfa_utf8(goodsequences[i], len+1)) {
      printf("failing to validate good string %zu using DFA: validate_dfa_utf8 \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    if(!validate_dfa_utf8_branchless(goodsequences[i], len+1)) {
      printf("failing to validate good string %zu using DFA: validate_dfa_utf8 \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    if(!shiftless_validate_dfa_utf8(goodsequences[i], len+1)) {
      printf("failing to validate good string %zu using DFA: shiftless_validate_dfa_utf8 \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    if(!shiftless_validate_dfa_utf8_branchless(goodsequences[i], len+1)) {
      printf("failing to validate good string %zu using DFA:shiftless_validate_dfa_utf8_branchless \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    if(!validate_utf8_fast(goodsequences[i], len)) {
      printf("failing to validate good string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      memset(buffer,0x20,N);
      printf(".");
      fflush(NULL);
      memcpy(buffer + offset, goodsequences[i], len);
      if(!validate_utf8_fast(buffer, N)) {
            printf("failing to validate good string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
#ifdef AVX512_IMPLEMENTATION
    if(!validate_utf8_fast_avx512(goodsequences[i], len)) {
      printf("(avx512) failing to validate good string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, goodsequences[i], len);
      if(!validate_utf8_fast_avx512(buffer, N)) {
            printf("(avx512) failing to validate good string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
    if(!validate_utf8_fast_avx512_asciipath(goodsequences[i], len)) {
      printf("(avx512) failing to validate good string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, goodsequences[i], len);
      if(!validate_utf8_fast_avx512_asciipath(buffer, N)) {
            printf("(avx512) failing to validate good string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
#endif
#ifdef __AVX2__
    if(!validate_utf8_fast_avx(goodsequences[i], len)) {
      printf("(avx) failing to validate good string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, goodsequences[i], len);
      if(!validate_utf8_fast_avx(buffer, N)) {
            printf("(avx) failing to validate good string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
    if(!validate_utf8_fast_avx_asciipath(goodsequences[i], len)) {
      printf("(avx) failing to validate good string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, goodsequences[i], len);
      if(!validate_utf8_fast_avx_asciipath(buffer, N)) {
            printf("(avx) failing to validate good string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
#endif
  }
  for (size_t i = 0; i < 23; i++) {
    printf("bad sequence %zu \n", i);

    size_t len = strlen(badsequences[i]);
    if(validate_dfa_utf8(badsequences[i], len+1)) {
      printf("failing to invalidate bad string %zu using DFA \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
      printf("\n");
      abort();
    }
    if(validate_dfa_utf8_branchless(badsequences[i], len+1)) {
      printf("failing to invalidate bad string %zu using DFA \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
      printf("\n");
      abort();
    }
    if(shiftless_validate_dfa_utf8(badsequences[i], len+1)) {
      printf("failing to invalidate bad string %zu using DFA \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
      printf("\n");
      abort();
    }
    if(shiftless_validate_dfa_utf8_branchless(badsequences[i], len+1)) {
      printf("failing to invalidate bad string %zu using DFA \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, badsequences[i], len);
      if(validate_utf8_fast(buffer, N)) {
            printf("failing to invalidate bad string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
#ifdef __AVX2__
    if(validate_utf8_fast_avx(badsequences[i], len)) {
      printf("(avx) failing to invalidate bad string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, badsequences[i], len);
      if(validate_utf8_fast_avx(buffer, N)) {
            printf("(avx) failing to invalidate bad string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
    if(validate_utf8_fast_avx_asciipath(badsequences[i], len)) {
      printf("(avx) failing to invalidate bad string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, badsequences[i], len);
      if(validate_utf8_fast_avx_asciipath(buffer, N)) {
            printf("(avx) failing to invalidate bad string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
#endif
#ifdef AVX512_IMPLEMENTATION
    if(validate_utf8_fast_avx512(badsequences[i], len)) {
      printf("(avx512) failing to invalidate bad string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, badsequences[i], len);
      if(validate_utf8_fast_avx512(buffer, N)) {
            printf("(avx512) failing to invalidate bad string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
    if(validate_utf8_fast_avx512_asciipath(badsequences[i], len)) {
      printf("(avx512) failing to invalidate bad string %zu \n", i);
      for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
      printf("\n");
      abort();
    }
    for(size_t offset = 0; offset < N - len; offset++) {
      printf(".");
      fflush(NULL);
      memset(buffer,0x20,N);
      memcpy(buffer + offset, badsequences[i], len);
      if(validate_utf8_fast_avx512_asciipath(buffer, N)) {
            printf("(avx512) failing to invalidate bad string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)badsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
#endif
  }

  char ascii[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 0};
  char notascii[] = {128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 238, 255, 0};
  assert(validate_ascii_fast(ascii, strlen(ascii)));
#ifdef __AVX2__
  assert(validate_ascii_fast_avx(ascii, strlen(ascii)));
#endif 
#ifdef AVX512_IMPLEMENTATION
  assert(validate_ascii_fast_avx512(ascii, strlen(ascii)));
#endif 
  assert(!validate_ascii_fast(notascii, strlen(notascii)));
#ifdef __AVX2__
  assert(!validate_ascii_fast_avx(notascii, strlen(notascii)));
#endif
#ifdef AVX512_IMPLEMENTATION
  assert(!validate_ascii_fast_avx512(notascii, strlen(notascii)));
#endif

  __m128i cont = _mm_setr_epi8(4,0,0,0,3,0,0,2,0,1,2,0,3,0,0,1);
  __m128i has_error = _mm_setzero_si128();
  __m128i carries = carryContinuations(cont, _mm_set1_epi8(1));
  checkContinuations(cont, carries, &has_error);
  assert(_mm_test_all_zeros(has_error, has_error));
  assert(_mm_test_all_ones(_mm_cmpeq_epi8(carries, _mm_setr_epi8(4,3,2,1,3,2,1,2,1,1,2,1,3,2,1,1))));

#ifdef AVX512_IMPLEMENTATION
  __m512i cont512 = _mm512_set4_epi32(0x01000003, 0x00020100, 0x02000003, 0x00000004);
  __mmask64 has_error512 = 0;
  __m512i carries512 = avx512_carryContinuations(cont512, _mm512_set1_epi8(1));
  avx512_checkContinuations(cont512, carries512, &has_error512);
  assert(has_error512 == 0); // all zeros
  assert(_mm512_cmpeq_epi8_mask(carries512,_mm512_set4_epi32(0x01010203, 0x01020101, 0x02010203, 0x01020304)) == 0xFFFFFFFFFFFFFFFF); // all ones
#endif

  // overlap
  cont = _mm_setr_epi8(4,0,1,0,3,0,0,2,0,1,2,0,3,0,0,1);
  has_error = _mm_setzero_si128();
  carries = carryContinuations(cont, _mm_set1_epi8(1));
  checkContinuations(cont, carries, &has_error);
  assert(!_mm_test_all_zeros(has_error, has_error));

#ifdef AVX512_IMPLEMENTATION
  cont512 = _mm512_set4_epi32(0x01000003, 0x00020100, 0x02000003, 0x00010004);
  has_error512 = 0;
  carries512 = avx512_carryContinuations(cont512, _mm512_set1_epi8(1));
  avx512_checkContinuations(cont512, carries512, &has_error512);
  assert(has_error512 != 0);
#endif

  // underlap
  cont = _mm_setr_epi8(4,0,0,0,0,0,0,2,0,1,2,0,3,0,0,1);
  has_error = _mm_setzero_si128();
  carries = carryContinuations(cont, _mm_set1_epi8(1));
  checkContinuations(cont, carries, &has_error);
  assert(!_mm_test_all_zeros(has_error, has_error));

#ifdef AVX512_IMPLEMENTATION
  cont512 = _mm512_set4_epi32(0x01000003, 0x00020100, 0x02000000, 0x00000004);
  has_error512 = 0;
  carries512 = avx512_carryContinuations(cont512, _mm512_set1_epi8(1));
  avx512_checkContinuations(cont512, carries512, &has_error512);
  assert(has_error512 != 0);
#endif

    // register crossing
  cont = _mm_setr_epi8(0,2,0,3,0,0,2,0,1,2,0,3,0,0,1,1);
  __m128i prev = _mm_setr_epi8(3,2,1,3,2,1,2,1,1,2,1,3,2,4,3,2);
  has_error = _mm_setzero_si128();
  carries = carryContinuations(cont, prev);
  checkContinuations(cont, carries, &has_error);
  assert(_mm_test_all_zeros(has_error, has_error));

#ifdef AVX512_IMPLEMENTATION
  cont512 = _mm512_set4_epi32(0x01010000, 0x03000201, 0x00020000, 0x03000200);
  __m512i prev512 = _mm512_set4_epi32(0x02030402, 0x03010201, 0x01020102, 0x03010203);
  has_error512 = 0;
  carries512 = avx512_carryContinuations(cont512, prev512);
  avx512_checkContinuations(cont512, carries512, &has_error512);
  assert(has_error512 != 0);
#endif
}

int main() {
  test();
  printf("Your code is ok.\n");
  return EXIT_SUCCESS;
}
