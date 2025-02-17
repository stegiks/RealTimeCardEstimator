# Makefile for only main.cpp which needs Open MP library

CXX = g++
CXXFLAGS = -std=c++11 -fopenmp -O3

# Arguments : <seed> <total_rows> <total_ops> <num_threads>
ARGS = 6969 5000000 2000000

main: main.cpp
	$(CXX) $(CXXFLAGS) -o main main.cpp

clean:
	rm -f main

run: main
	./main $(ARGS)

run_data: main
	./main $(ARGS) data-only