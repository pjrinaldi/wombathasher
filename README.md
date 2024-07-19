# wombathasher
A Command Line Hash List and Hash Comparison tool compatible with [wombatforensics](https://github.com/pjrinaldi/wombatforensics)' hash lists and hash matching using the BLAKE3 hash.

## Repository Change
* I am no longer building code on github. I have moved my code to the website www.wombatforensics.com and am hosting my repositories on a vps using fossil for the repositories rather than git. I decided to stop using github due to all the AI crap and scraping code. My code isn't fancy or great, and it is free, but I just don't like the idea of scraping without my ok and since github is free, that is part of the price for free access. So I am leaving the historical bits of my repositories, but moving them all to fossil repositories. Feel free to check them out, they aren't as fancy or featureful as github, but it fits my needs.

The code is compiled using c++17.

The blake3 library is statically linked in, so can just download the binary and it should run fine. If you want to compile yourself, compile blake3 first. To compile libblake3, i use the following two commands...

1. `gcc -c blake3.c blake3_dispatch.c blake3_portable.c blake3_sse2_x86-64_unix.S blake3_sse41_x86-64_unix.S blake3_avx2_x86-64_unix.S blake3_avx512_x86-64_unix.S`
2. `ar -rc libblake3.a blake3_avx2_x86-64_unix.o blake3_avx512_x86-64_unix.o blake3_dispatch.o blake3.o blake3_portable.o blake3_sse2_x86-64_unix.o blake3_sse41_x86-64_unix.o`

To compile blake3 and wombathasher, you simply have to run the "build.sh" script.
