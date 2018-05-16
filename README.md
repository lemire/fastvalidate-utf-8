# fastvalidate-utf-8

Most strings online are in unicode using the UTF-8 encoding. Validating strings
quickly before accepting them is important.

This is a header-only C library to validate UTF-8 strings at high speeds using SIMD instructions.
Specifically, this expects an x64 processor (capable of SSE instruction). It will not
work currently on ARM processors.

Quick usage:
```
make
./test
./benchmark
```

Code usage:

```
  #include "simdutf8check.h"

  char * mystring = ...
  bool is_it_valid = validate_utf8_fast(mystring, thestringlength);
```

It should be able to validate strings using close to 1 cycle per input byte.

If you expect your strings to be plain ASCII, you can spend less than 0.1 cycles per input byte to check whether that is the case using the ``validate_ascii_fast`` function found in the ``simdasciicheck.h`` header.

