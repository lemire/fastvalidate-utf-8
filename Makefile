
all: unit benchmark

test:unit
	./unit

unit: tests/unit.c include/simdutf8check.h include/simdutf8check_avx512.h
	$(CC) -std=c99 -march=native -O3 -Wall -o unit tests/unit.c -Iinclude -D_GNU_SOURCE

benchmark: benchmarks/benchmark.c include/simdutf8check.h include/simdutf8check_avx512.h
	$(CC) -std=c99 -march=native -O3 -Wall -o benchmark benchmarks/benchmark.c -Iinclude -Ibenchmarks -D_GNU_SOURCE

clean:
	rm -r -f benchmark unit
