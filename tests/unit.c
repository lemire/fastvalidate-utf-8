#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simdutf8check.h"
#include "simdasciicheck.h"


void test() {
  char *goodsequences[] = {"a", "\xc3\xb1", "\xe2\x82\xa1", "\xf0\x90\x8c\xbc"};
  char *badsequences[] = {"\xc3\x28",         // 0
                          "\xa0\xa1",         // 1
                          "\xe2\x28\xa1",     // 2
                          "\xe2\x82\x28",     // 3
                          "\xf0\x28\x8c\xbc", // 4
                          "\xf0\x90\x28\xbc", // 5
                          "\xf0\x28\x8c\x28", // 6
                          "\xc0\x9f",         // 7
                          "\xf5\xff\xff\xff", // 8
                          "\xed\xa0\x81"};    // 9
  for (size_t i = 0; i < 4; i++) {
    size_t len = strlen(goodsequences[i]);
    assert(validate_utf8_fast(goodsequences[i], len));
  }
  for (size_t i = 0; i < 10; i++) {
    size_t len = strlen(badsequences[i]);
    assert(!validate_utf8_fast(badsequences[i], len));
  }

  char ascii[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 0};
  char notascii[] = {128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 238, 255, 0};
  assert(validate_ascii_fast(ascii, strlen(ascii)));
  assert(!validate_ascii_fast(notascii, strlen(notascii)));
}

int main() {
  test();
  printf("Your code is ok.\n");
  return EXIT_SUCCESS;
}
