# fastvalidate-utf-8
[![Build Status](https://travis-ci.org/lemire/fastvalidate-utf-8.png)](https://travis-ci.org/lemire/fastvalidate-utf-8)
[![Code Quality: Cpp](https://img.shields.io/lgtm/grade/cpp/g/lemire/fastvalidate-utf-8.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/lemire/fastvalidate-utf-8/context:cpp)


Most strings online are in unicode using the UTF-8 encoding. Validating strings
quickly before accepting them is important.


### Want a production-ready function?

The fastvalidate-utf-8 repository is for demonstration purposes.

If you want access to a fast validation function for production use, you can rely on the simdjson library. [It is as simple as the following](https://github.com/simdjson/simdjson/blob/master/doc/basics.md#utf-8-validation-alone):

```C++
  const char * some_string = "[ 1, 2, 3, 4] ";
  size_t length = strlen(some_string);
  bool is_ok = simdjson::validate_utf8(some_string, length);
```

See https://github.com/simdjson/

The simdjson library supports a wide-range of platforms and offers runtime dispatching as well as the most up-to-date algorithms. It is not necessary that your data is made of JSON though this was the original motivation.


## Reference

- John Keiser, Daniel Lemire, [Validating UTF-8 In Less Than One Instruction Per Byte](https://arxiv.org/abs/2010.03090), Software: Practice & Experience 51 (5), 2021


## How to use fastvalidate-utf-8?

This is a header-only C library to validate UTF-8 strings at high speeds using SIMD instructions.
Specifically, this expects an x64 processor (capable of SSE instruction). It will not
work currently on ARM processors. It is not meant to be used in production as-is. Please see
the simdjson library and its corresponding `simdjson::validate_utf8` function.


Quick usage:
```
make
./unit
./benchmark
```

Code usage:

```C++
  #include "simdutf8check.h"

  char * mystring = ...
  bool is_it_valid = validate_utf8_fast(mystring, thestringlength);
```

It should be able to validate strings using less than 1 cycle per input byte.

If you expect your strings to be plain ASCII, you can spend less than 0.1 cycles per input byte to check whether that is the case using the ``validate_ascii_fast`` function found in the ``simdasciicheck.h`` header. There are even faster functions like ``validate_utf8_fast_avx``.

A modified version of this code improved the performance of [Scylla](https://github.com/scylladb/scylla/commit/6fadba56cc18bebe6648e6aec09be1b0b93f4817).



### Command-line tool 

Adam Retter maintains a useful [command-line tool](https://github.com/adamretter/utf8-validator-c) related to this library.

### Experimental results

On a Skylake processor, using GCC, we get:

```
$ ./benchmark
string size = 65536
We are feeding ascii so it is always going to be ok.
It favors schemes that skip ASCII characters.
validate_utf8(data, N)                                          :  1.256 cycles per operation (best)     1.316 cycles per operation (avg)
validate_utf8_fast(data, N)                                     :  0.704 cycles per operation (best)     0.706 cycles per operation (avg)
validate_utf8_fast_avx(data, N)                                 :  0.450 cycles per operation (best)     0.452 cycles per operation (avg)
validate_utf8_fast_avx_asciipath(data, N)                       :  0.088 cycles per operation (best)     0.091 cycles per operation (avg)
validate_ascii_fast(data, N)                                    :  0.082 cycles per operation (best)     0.084 cycles per operation (avg)
validate_ascii_fast_avx(data, N)                                :  0.050 cycles per operation (best)     0.074 cycles per operation (avg)
validate_ascii_nosimd(data, N)                                  :  0.104 cycles per operation (best)     0.106 cycles per operation (avg)
validate_ascii_nointrin(data, N)                                :  0.068 cycles per operation (best)     0.088 cycles per operation (avg)
validate_utf8_fast(data, N)                                      :  0.701 cycles per operation (best)     0.703 cycles per operation (avg)  (linux counter)
validate_ascii_fast(data, N)                                     :  0.083 cycles per operation (best)     0.085 cycles per operation (avg)  (linux counter)


string size (approx) = 65536
Producing random-looking UTF-8
validate_utf8(data, actualN)                                    :  10.967 cycles per operation (best)     11.005 cycles per operation (avg)
validate_utf8_fast(data, actualN)                               :  0.702 cycles per operation (best)     0.705 cycles per operation (avg)
validate_utf8_fast_avx(data, actualN)                           :  0.448 cycles per operation (best)     0.485 cycles per operation (avg)
validate_utf8_fast_avx_asciipath(data, actualN)                 :  0.480 cycles per operation (best)     0.594 cycles per operation (avg)
```

Thus, after rounding, it takes 0.7 cycles per input byte to validate UTF-8 strings.

* Blog post: [Validating UTF-8 strings using as little as 0.7 cycles per byte](https://lemire.me/blog/2018/05/16/validating-utf-8-strings-using-as-little-as-0-7-cycles-per-byte/)
* Blog post: [Validating UTF-8 bytes using only 0.45 cycles per byte (AVX edition)](https://lemire.me/blog/2018/10/19/validating-utf-8-bytes-using-only-0-45-cycles-per-byte-avx-edition/)

## In Go

[There is an assembly wrapper in Go](https://github.com/stuartcarnie/go-simd) by Stuart Carnie.

## ARM Neon and SSE4

[Fast UTF-8 validation with range algorithm (NEON+SSE4)](https://github.com/cyb70289/utf8/)

## License

This library is distributed under the terms of any of the following
licenses, at your option:

* Apache License (Version 2.0) [LICENSE-APACHE](LICENSE-APACHE),
* Boost Software License [LICENSE-BOOST](LICENSE-BOOST), or
* MIT License [LICENSE-MIT](LICENSE-MIT).
