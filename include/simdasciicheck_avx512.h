#ifdef __AVX512F__
#include <x86intrin.h>
// The function returns true (1) if all chars passed in src are
// 7-bit values (0x00..0x7F). Otherwise, it returns false (0).
static bool validate_ascii_fast_avx512(const char *src, size_t len) {
  size_t i = 0;
  __m512i has_error = _mm512_setzero_si512();
  if (len >= 64) {
    for (; i <= len - 64; i += 64) {
      __m512i current_bytes = _mm512_loadu_si512((const __m512i *)(src + i));
      has_error = _mm512_or_si512(has_error, current_bytes);
    }
  }
  int reduced = _mm512_reduce_or_epi32(has_error);

  for (; i < len; i++) {
    reduced |= src[i];
  }

  return !(reduced & 0x80808080);
}
#endif