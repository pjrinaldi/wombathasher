#!/bin/bash

echo "Building libblake3.a for C++ program"

gcc -c ../blake3.c ../blake3_dispatch.c ../blake3_portable.c ../blake3_sse2_x86-64_unix.S ../blake3_sse41_x86-64_unix.S ../blake3_avx2_x86-64_unix.S ../blake3_avx512_x86-64_unix.S

ar -rcs libblake3.a blake3_avx2_x86-64_unix.o blake3_avx512_x86-64_unix.o blake3_dispatch.o blake3.o blake3_portable.o blake3_sse2_x86-64_unix.o blake3_sse41_x86-64_unix.o

echo "Building wombathasher as C++ program"

g++ -O3 -static -o wombathasher+ wombathasher.cpp -lpthread -L. -lblake3
