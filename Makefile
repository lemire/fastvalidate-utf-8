
all: unit benchmark

test:unit
	./unit

unit: tests/unit.c include/simdutf8check.h
	cc -march=native -O3 -o unit tests/unit.c -Iinclude

benchmark: benchmarks/benchmark.c include/simdutf8check.h
	cc -march=native -O3 -o benchmark benchmarks/benchmark.c -Iinclude -Ibenchmarks

clean:
	rm -r -f benchmark unit
