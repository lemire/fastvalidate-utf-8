#ifndef SIMDUTF8CHECK_AXV512_H
#define SIMDUTF8CHECK_AXV512_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
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

#ifdef __AVX512F__

/*****************************/
static inline __m512i avx512_push_last_byte_of_a_to_b(__m512i a, __m512i b) {
  __m512i indexes = _mm512_set_epi64(0x3E3D3C3B3A393837, 0x363534333231302F,
                                     0x2E2D2C2B2A292827, 0x262524232221201F,
                                     0x1E1D1C1B1A191817, 0x161514131211100F,
                                     0x0E0D0C0B0A090807, 0x060504030201007F);
  return _mm512_permutex2var_epi8(b, indexes, a);
}

static inline __m512i avx512_push_last_2bytes_of_a_to_b(__m512i a, __m512i b) {
  __m512i indexes = _mm512_set_epi64(0x3D3C3B3A39383736, 0x3534333231302F2E,
                                     0x2D2C2B2A29282726, 0x2524232221201F1E,
                                     0x1D1C1B1A19181716, 0x1514131211100F0E,
                                     0x0D0C0B0A09080706, 0x0504030201007F7E);
  return _mm512_permutex2var_epi8(b, indexes, a);
}

// all byte values must be no larger than 0xF4
static inline void avx512_checkSmallerThan0xF4(__m512i current_bytes,
                                               __mmask64 *has_error) {
  *has_error =
      _kor_mask64(*has_error, _mm512_cmpgt_epu8_mask(current_bytes,
                                                     _mm512_set1_epi8(0xF4)));
}

static inline __m512i avx512_continuationLengths(__m512i high_nibbles) {
  return _mm512_shuffle_epi8(
      _mm512_setr4_epi32(0x01010101, 0x01010101, 0x00000000,
                         0x04030202), // see avx2 version for clarity
      high_nibbles);
}

static inline __m512i avx512_carryContinuations(__m512i initial_lengths,
                                                __m512i previous_carries) {

  __m512i right1 = _mm512_subs_epu8(
      avx512_push_last_byte_of_a_to_b(previous_carries, initial_lengths),
      _mm512_set1_epi8(1));
  __m512i sum = _mm512_add_epi8(initial_lengths, right1);

  __m512i right2 =
      _mm512_subs_epu8(avx512_push_last_2bytes_of_a_to_b(previous_carries, sum),
                       _mm512_set1_epi8(2));
  return _mm512_add_epi8(sum, right2);
}

static inline void avx512_checkContinuations(__m512i initial_lengths,
                                             __m512i carries,
                                             __mmask64 *has_error) {
  // overlap || underlap
  // carry > length && length > 0 || !(carry > length) && !(length > 0)
  // (carries > length) == (lengths > 0)
  *has_error = _kor_mask64(
      *has_error,
      _knot_mask64(_kxor_mask64(
          _mm512_cmpgt_epi8_mask(carries, initial_lengths),
          _mm512_cmpgt_epi8_mask(initial_lengths, _mm512_setzero_si512()))));
}

// when 0xED is found, next byte must be no larger than 0x9F
// when 0xF4 is found, next byte must be no larger than 0x8F
// next byte must be continuation, ie sign bit is set, so signed < is ok
static inline void avx512_checkFirstContinuationMax(__m512i current_bytes,
                                                    __m512i off1_current_bytes,
                                                    __mmask64 *has_error) {
  __mmask64 maskED =
      _mm512_cmpeq_epi8_mask(off1_current_bytes, _mm512_set1_epi8(0xED));
  __mmask64 maskF4 =
      _mm512_cmpeq_epi8_mask(off1_current_bytes, _mm512_set1_epi8(0xF4));
  __mmask64 badfollowED = _kand_mask64(
      _mm512_cmpgt_epi8_mask(current_bytes, _mm512_set1_epi8(0x9F)), maskED);
  __mmask64 badfollowF4 = _kand_mask64(
      _mm512_cmpgt_epi8_mask(current_bytes, _mm512_set1_epi8(0x8F)), maskF4);

  *has_error = _kor_mask64(*has_error, _kor_mask64(badfollowED, badfollowF4));
}

// map off1_hibits => error condition
// hibits     off1    cur
// C       => < C2 && true
// E       => < E1 && < A0
// F       => < F1 && < 90
// else      false && false
static inline void avx512_checkOverlong(__m512i current_bytes,
                                        __m512i off1_current_bytes,
                                        __m512i hibits, __m512i previous_hibits,
                                        __mmask64 *has_error) {
  __m512i off1_hibits =
      avx512_push_last_byte_of_a_to_b(previous_hibits, hibits);
  __m512i initial_mins = _mm512_shuffle_epi8(
      _mm512_setr4_epi32(0x80808080, 0x80808080, 0x80808080,
                         0xF1E180C2), // see avx2 version for clarity
      off1_hibits);

  __mmask64 initial_under =
      _mm512_cmpgt_epi8_mask(initial_mins, off1_current_bytes);

  __m512i second_mins = _mm512_shuffle_epi8(
      _mm512_setr4_epi32(0x80808080, 0x80808080, 0x80808080,
                         0x90A07F7F), // see avx2 version for clarity
      off1_hibits);
  __mmask64 second_under = _mm512_cmpgt_epi8_mask(second_mins, current_bytes);
  *has_error =
      _kor_mask64(*has_error, _kand_mask64(initial_under, second_under));
}

struct avx512_processed_utf_bytes {
  __m512i rawbytes;
  __m512i high_nibbles;
  __m512i carried_continuations;
};

static inline void
avx512_count_nibbles(__m512i bytes, struct avx512_processed_utf_bytes *answer) {
  answer->rawbytes = bytes;
  answer->high_nibbles =
      _mm512_and_si512(_mm512_srli_epi16(bytes, 4), _mm512_set1_epi8(0x0F));
}

// check whether the current bytes are valid UTF-8
// at the end of the function, previous gets updated
static struct avx512_processed_utf_bytes
avx512_checkUTF8Bytes(__m512i current_bytes,
                      struct avx512_processed_utf_bytes *previous,
                      __mmask64 *has_error) {
  struct avx512_processed_utf_bytes pb;
  avx512_count_nibbles(current_bytes, &pb);

  avx512_checkSmallerThan0xF4(current_bytes, has_error);

  __m512i initial_lengths = avx512_continuationLengths(pb.high_nibbles);

  pb.carried_continuations = avx512_carryContinuations(
      initial_lengths, previous->carried_continuations);

  avx512_checkContinuations(initial_lengths, pb.carried_continuations,
                            has_error);

  __m512i off1_current_bytes =
      avx512_push_last_byte_of_a_to_b(previous->rawbytes, pb.rawbytes);
  avx512_checkFirstContinuationMax(current_bytes, off1_current_bytes,
                                   has_error);

  avx512_checkOverlong(current_bytes, off1_current_bytes, pb.high_nibbles,
                       previous->high_nibbles, has_error);
  return pb;
}

// check whether the current bytes are valid UTF-8
// at the end of the function, previous gets updated
static struct avx512_processed_utf_bytes
avx512_checkUTF8Bytes_asciipath(__m512i current_bytes,
                                struct avx512_processed_utf_bytes *previous,
                                __mmask64 *has_error) {
  if (!_mm512_cmpge_epu8_mask(current_bytes,
                              _mm512_set1_epi8(0x80))) { // fast ascii path
    *has_error = _kor_mask64(
        *has_error,
        _mm512_cmpgt_epi8_mask(
            previous->carried_continuations,
            _mm512_setr_epi32(0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x01090909)));
    return *previous;
  }

  struct avx512_processed_utf_bytes pb;
  avx512_count_nibbles(current_bytes, &pb);

  avx512_checkSmallerThan0xF4(current_bytes, has_error);

  __m512i initial_lengths = avx512_continuationLengths(pb.high_nibbles);

  pb.carried_continuations = avx512_carryContinuations(
      initial_lengths, previous->carried_continuations);

  avx512_checkContinuations(initial_lengths, pb.carried_continuations,
                            has_error);

  __m512i off1_current_bytes =
      avx512_push_last_byte_of_a_to_b(previous->rawbytes, pb.rawbytes);
  avx512_checkFirstContinuationMax(current_bytes, off1_current_bytes,
                                   has_error);

  avx512_checkOverlong(current_bytes, off1_current_bytes, pb.high_nibbles,
                       previous->high_nibbles, has_error);
  return pb;
}

static bool validate_utf8_fast_avx512_asciipath(const char *src, size_t len) {
  size_t i = 0;
  __mmask64 has_error = 0;
  struct avx512_processed_utf_bytes previous = {
    .rawbytes = _mm512_setzero_si512(),
    .high_nibbles = _mm512_setzero_si512(),
    .carried_continuations = _mm512_setzero_si512()
  };
  if (len >= 64) {
    for (; i <= len - 64; i += 64) {
      __m512i current_bytes = _mm512_loadu_si512((const __m512i *)(src + i));
      previous =
          avx512_checkUTF8Bytes_asciipath(current_bytes, &previous, &has_error);
    }
  }

  // last part
  if (i < len) {
    char buffer[64];
    memset(buffer, 0, 64);
    memcpy(buffer, src + i, len - i);
    __m512i current_bytes = _mm512_loadu_si512((const __m512i *)(buffer));
    previous = avx512_checkUTF8Bytes(current_bytes, &previous, &has_error);
  } else {
    has_error = _kor_mask64(
        has_error,
        _mm512_cmpgt_epi8_mask(
            previous.carried_continuations,
            _mm512_setr_epi32(0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x01090909)));
  }

  return !has_error;
}

static bool validate_utf8_fast_avx512(const char *src, size_t len) {
  size_t i = 0;
  __mmask64 has_error = 0;
  struct avx512_processed_utf_bytes previous = {
    .rawbytes = _mm512_setzero_si512(),
    .high_nibbles = _mm512_setzero_si512(),
    .carried_continuations = _mm512_setzero_si512()
  };
  if (len >= 64) {
    for (; i <= len - 64; i += 64) {
      __m512i current_bytes = _mm512_loadu_si512((const __m512i *)(src + i));
      previous = avx512_checkUTF8Bytes(current_bytes, &previous, &has_error);
    }
  }

  // last part
  if (i < len) {
    char buffer[64];
    memset(buffer, 0, 64);
    memcpy(buffer, src + i, len - i);
    __m512i current_bytes = _mm512_loadu_si512((const __m512i *)(buffer));
    previous = avx512_checkUTF8Bytes(current_bytes, &previous, &has_error);
  } else {
    has_error = _kor_mask64(
        has_error,
        _mm512_cmpgt_epi8_mask(
            previous.carried_continuations,
            _mm512_setr_epi32(0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x09090909,
                              0x09090909, 0x09090909, 0x09090909, 0x01090909)));
  }

  return !has_error;
}

#endif // __AVX512F__

#endif // SIMDUTF8CHECK_AXV512_H
