#ifndef HOEHRMANN_H
#define HOEHRMANN_H
// credit: @hoehrmann

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1
#define SHIFTLESS_UTF8_REJECT 16

static const uint8_t utf8d[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 00..1f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20..3f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 40..5f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60..7f
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   9,   9,   9,   9,   9,   9,
    9,   9,   9,   9,   9,   9,   9,   9,   9,   9, // 80..9f
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7, // a0..bf
    8,   8,   2,   2,   2,   2,   2,   2,   2,   2,   2,
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2, // c0..df
    0xa, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
    0x3, 0x3, 0x4, 0x3, 0x3, // e0..ef
    0xb, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8 // f0..ff
};
static const uint8_t utf8d_transition[] = {
    0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4,
    0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   0,   1,   1,   1,   1,
    1,   0,   1,   0,   1,   1,   1,   1,   1,   1, // s1..s2
    1,   2,   1,   1,   1,   1,   1,   2,   1,   2,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   2,   1,   1,   1,   1,   1,   1,   1,   1, // s3..s4
    1,   2,   1,   1,   1,   1,   1,   1,   1,   2,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   3,   1,   3,   1,   1,   1,   1,   1,   1, // s5..s6
    1,   3,   1,   1,   1,   1,   1,   3,   1,   3,   1,
    1,   1,   1,   1,   1,   1,   3,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, // s7..s8
};
static const uint8_t shifted_utf8d_transition[] = {
    0x0,  0x10, 0x20, 0x30, 0x50, 0x80, 0x70, 0x10, 0x10, 0x10, 0x40, 0x60,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0,  0x10, 0x10,
    0x10, 0x10, 0x10, 0x0,  0x10, 0x0,  0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x10, 0x20, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0x10, 0x30, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30,
    0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
static uint32_t inline decode(uint32_t *state, uint32_t *codep, uint32_t byte) {
  uint32_t type = utf8d[byte];
  *codep = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6)
                                   : (0xff >> type) & (byte);
  *state = utf8d_transition[16 * *state + type];
  return *state;
}

static uint32_t inline shiftless_decode(uint32_t *state, uint32_t *codep,
                                        uint32_t byte) {
  uint32_t type = utf8d[byte];
  *codep = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6)
                                   : (0xff >> type) & (byte);
  *state = shifted_utf8d_transition[*state + type];
  return *state;
}

static uint32_t inline updatestate(uint32_t *state, uint32_t byte) {
  uint32_t type = utf8d[byte];
  *state = utf8d_transition[16 * *state + type];
  return *state;
}

static uint32_t inline shiftless_updatestate(uint32_t *state, uint32_t byte) {
  uint32_t type = utf8d[byte];
  *state = shifted_utf8d_transition[*state + type];
  return *state;
}

static inline bool is_ascii(const char *c, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (c[i] < 0)
      return false;
  }
  return true;
}

static inline bool validate_dfa_utf8_branchless(const char *c, size_t len) {
  const unsigned char *cu = (const unsigned char *)c;
  uint32_t state = 0;
  for (size_t i = 0; i < len; i++) {
    uint32_t byteval = (uint32_t)cu[i];
    updatestate(&state, byteval);
  }
  return state != UTF8_REJECT;
}

static inline bool shiftless_validate_dfa_utf8_branchless(const char *c,
                                                          size_t len) {
  const unsigned char *cu = (const unsigned char *)c;
  uint32_t state = 0;
  for (size_t i = 0; i < len; i++) {
    uint32_t byteval = (uint32_t)cu[i];
    shiftless_updatestate(&state, byteval);
  }
  return state != SHIFTLESS_UTF8_REJECT;
}

static inline bool validate_dfa_utf8(const char *c, size_t len) {
  const unsigned char *cu = (const unsigned char *)c;
  uint32_t state = 0;
  for (size_t i = 0; i < len; i++) {
    uint32_t byteval = (uint32_t)cu[i];
    if (updatestate(&state, byteval) == UTF8_REJECT)
      return false;
  }
  return true;
}

static inline bool shiftless_validate_dfa_utf8(const char *c, size_t len) {
  const unsigned char *cu = (const unsigned char *)c;
  uint32_t state = 0;
  for (size_t i = 0; i < len; i++) {
    uint32_t byteval = (uint32_t)cu[i];
    if (shiftless_updatestate(&state, byteval) == SHIFTLESS_UTF8_REJECT)
      return false;
  }
  return true;
}

// credit: Travis Downs
// This is an experiment, the validation is not guarantee to work
static inline bool validate_dfa_utf8_double(const char *c, size_t len) {
  size_t half = len / 2;
  const unsigned char *cu = (const unsigned char *)c;

  while ((unsigned char)c[half] <= 0xBF && (unsigned char)c[half] > 0x80 &&
         half > 0) {
    half--;
  }
  uint32_t s1 = 0, s2 = 0;
  for (size_t i = 0, j = half; i < half; i++, j++) {
    updatestate(&s1, cu[i]);
    updatestate(&s2, cu[j]);
  }
  for (int j = half * 2; j < len; j++) {
    updatestate(&s2, cu[j]);
  }
  return (s1 != UTF8_REJECT) && (s2 != UTF8_REJECT);
}

// credit: Travis Downs
// This is an experiment, the validation is not guarantee to work
static inline bool shiftless_validate_dfa_utf8_double(const char *c,
                                                      size_t len) {
  size_t half = len / 2;
  const unsigned char *cu = (const unsigned char *)c;

  while ((unsigned char)c[half] <= 0xBF && (unsigned char)c[half] > 0x80 &&
         half > 0) {
    half--;
  }
  uint32_t s1 = 0, s2 = 0;
  for (size_t i = 0, j = half; i < half; i++, j++) {
    shiftless_updatestate(&s1, cu[i]);
    shiftless_updatestate(&s2, cu[j]);
  }
  for (int j = half * 2; j < len; j++) {
    shiftless_updatestate(&s2, cu[j]);
  }
  return (s1 != SHIFTLESS_UTF8_REJECT) && (s2 != SHIFTLESS_UTF8_REJECT);
}

#endif
