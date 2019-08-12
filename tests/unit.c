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

void test() {
  size_t N = 128;
  char buffer[N];
  char *goodsequences[] = {"a", "\xc3\xb1", "\xe2\x82\xa1", "\xf0\x90\x8c\xbc", "안녕하세요, 세상"};
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
                          "\xC2\x7F" // 14
                        };
  for (size_t i = 0; i < 5; i++) {
    printf("good sequence %zu \n", i);
    size_t len = strlen(goodsequences[i]);
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
      //printf("%*.*s\n",(int)N,(int)N,buffer);
      if(!validate_utf8_fast(buffer, N)) {
            printf("failing to validate good string %zu with offset %zu \n", i, N);
            for(size_t j = 0; j < len; j++) printf("0x%02x ", (unsigned char)goodsequences[i][j]);
            printf("\n");
            abort();
      }
    }
    printf("\n");
#ifdef __AVX512F__
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
  for (size_t i = 0; i < 15; i++) {
    printf("bad sequence %zu \n", i);

    size_t len = strlen(badsequences[i]);
    if(validate_utf8_fast(badsequences[i], len)) {
      printf("failing to invalidate bad string %zu \n", i);
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
#ifdef __AVX512F__
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
#ifdef __AVX512F__
  assert(validate_ascii_fast_avx512(ascii, strlen(ascii)));
#endif 
  assert(!validate_ascii_fast(notascii, strlen(notascii)));
#ifdef __AVX2__
  assert(!validate_ascii_fast_avx(notascii, strlen(notascii)));
#endif
#ifdef __AVX512F__
  assert(!validate_ascii_fast_avx512(notascii, strlen(notascii)));
#endif

  __m128i cont = _mm_setr_epi8(4,0,0,0,3,0,0,2,0,1,2,0,3,0,0,1);
  __m128i has_error = _mm_setzero_si128();
  __m128i carries = carryContinuations(cont, _mm_set1_epi8(1));
  checkContinuations(cont, carries, &has_error);
  assert(_mm_test_all_zeros(has_error, has_error));
  assert(_mm_test_all_ones(_mm_cmpeq_epi8(carries, _mm_setr_epi8(4,3,2,1,3,2,1,2,1,1,2,1,3,2,1,1))));

  __m512i cont512 = _mm512_set4_epi32(0x01000003, 0x00020100, 0x02000003, 0x00000004);
  __mmask64 has_error512 = 0;
  __m512i carries512 = avx512_carryContinuations(cont512, _mm512_set1_epi8(1));
  avx512_checkContinuations(cont512, carries512, &has_error512);
  assert(has_error512 == 0); // all zeros
  assert(_mm512_cmpeq_epi8_mask(carries512,_mm512_set4_epi32(0x01010203, 0x01020101, 0x02010203, 0x01020304)) == 0xFFFFFFFFFFFFFFFF); // all ones

  // overlap
  cont = _mm_setr_epi8(4,0,1,0,3,0,0,2,0,1,2,0,3,0,0,1);
  has_error = _mm_setzero_si128();
  carries = carryContinuations(cont, _mm_set1_epi8(1));
  checkContinuations(cont, carries, &has_error);
  assert(!_mm_test_all_zeros(has_error, has_error));

  cont512 = _mm512_set4_epi32(0x01000003, 0x00020100, 0x02000003, 0x00010004);
  has_error512 = 0;
  carries512 = avx512_carryContinuations(cont512, _mm512_set1_epi8(1));
  avx512_checkContinuations(cont512, carries512, &has_error512);
  assert(has_error512 != 0);


  // underlap
  cont = _mm_setr_epi8(4,0,0,0,0,0,0,2,0,1,2,0,3,0,0,1);
  has_error = _mm_setzero_si128();
  carries = carryContinuations(cont, _mm_set1_epi8(1));
  checkContinuations(cont, carries, &has_error);
  assert(!_mm_test_all_zeros(has_error, has_error));

  cont512 = _mm512_set4_epi32(0x01000003, 0x00020100, 0x02000000, 0x00000004);
  has_error512 = 0;
  carries512 = avx512_carryContinuations(cont512, _mm512_set1_epi8(1));
  avx512_checkContinuations(cont512, carries512, &has_error512);
  assert(has_error512 != 0);

    // register crossing
  cont = _mm_setr_epi8(0,2,0,3,0,0,2,0,1,2,0,3,0,0,1,1);
  __m128i prev = _mm_setr_epi8(3,2,1,3,2,1,2,1,1,2,1,3,2,4,3,2);
  has_error = _mm_setzero_si128();
  carries = carryContinuations(cont, prev);
  checkContinuations(cont, carries, &has_error);
  assert(_mm_test_all_zeros(has_error, has_error));

  cont512 = _mm512_set4_epi32(0x01010000, 0x03000201, 0x00020000, 0x03000200);
  __m512i prev512 = _mm512_set4_epi32(0x02030402, 0x03010201, 0x01020102, 0x03010203);
  has_error512 = 0;
  carries512 = avx512_carryContinuations(cont512, prev512);
  avx512_checkContinuations(cont512, carries512, &has_error512);
  assert(has_error512 != 0);
}

int main() {
  test();
  printf("Your code is ok.\n");
  return EXIT_SUCCESS;
}
