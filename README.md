# fastvalidate-utf-8
[![Build Status](https://travis-ci.org/lemire/fastvalidate-utf-8.png)](https://travis-ci.org/lemire/fastvalidate-utf-8)

Most strings online are in unicode using the UTF-8 encoding. Validating strings
quickly before accepting them is important.

This is a header-only C library to validate UTF-8 strings at high speeds using SIMD instructions.
Specifically, this expects an x64 processor (capable of SSE instruction). It will not
work currently on ARM processors.

Quick usage:
```
make
./unit
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

### Command-line tool 

Adam Retter maintains a useful [command-line tool](https://github.com/adamretter/utf8-validator-c) related to this library.

### Experimental results

On a Skylake processor, using GCC, we get:

```
$ ./benchmark
string size = 65536
We are feeding ascii so it is always going to be ok.
validate_utf8_fast(data, N)                                     :  0.702 cycles per operation (best)     0.703 cycles per operation (avg)
validate_ascii_fast(data, N)                                    :  0.081 cycles per operation (best)     0.083 cycles per operation (avg)
validate_utf8_fast(data, N)                                      :  0.700 cycles per operation (best)     0.701 cycles per operation (avg)  (linux counter)
validate_ascii_fast(data, N)                                     :  0.081 cycles per operation (best)     0.085 cycles per operation (avg)  (linux counter)
```

Thus, after rounding, it takes 0.7 cycles per input byte to validate UTF-8 strings.

* Blog post: [Validating UTF-8 strings using as little as 0.7 cycles per byte](https://lemire.me/blog/2018/05/16/validating-utf-8-strings-using-as-little-as-0-7-cycles-per-byte/)

