all: main

main: tsp2opt_gcc tsp2opt_clang

CFLAGS=-O3 --std=c++17 -lfmt -ffast-math -march=native -mtune=native 
#CFLAGS=-O0 --std=c++17 -lfmt -fno-omit-frame-pointer -g

tsp2opt_gcc: Cpp/main.cpp
	g++ -o tsp2opt_gcc Cpp/main.cpp $(CFLAGS)

tsp2opt_clang: Cpp/main.cpp
	clang++ -o tsp2opt_clang  Cpp/main.cpp $(CFLAGS)

clean:
	rm tsp2opt*