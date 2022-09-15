# wombathasher
A Command Line Hash List and Hash Comparison tool compatible with wombatforensics' hash lists and hash matching using the BLAKE3 hash.

The code is compiled using c++17.

The blake3 library is statically linked in. to compile libblake3, i use the following two commands...

1. `gcc -c blake3.c blake3_dispatch.c blake3_portable.c blake3_sse2_x86-64_unix.S blake3_sse41_x86-64_unix.S blake3_avx2_x86-64_unix.S blake3_avx512_x86-64_unix.S`
2. `ar -rc libblake3.a blake3_avx2_x86-64_unix.o blake3_avx512_x86-64_unix.o blake3_dispatch.o blake3.o blake3_portable.o blake3_sse2_x86-64_unix.o blake3_sse41_x86-64_unix.o`
