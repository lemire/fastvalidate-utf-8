
all: unit benchmark

test:unit
	./unit

unit: tests/unit.c include/simdutf8check.h
	cc -std=c99 -march=native -O3 -Wall -o unit tests/unit.c -Iinclude

benchmark: benchmarks/benchmark.c include/simdutf8check.h
	cc -std=c99 -march=native -O3 -Wall -o benchmark benchmarks/benchmark.c -Iinclude -Ibenchmarks

clean:
	rm -r -f benchmark unit
