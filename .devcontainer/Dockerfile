# Day 2 environment: AFL++ with clang/ASAN, gdb, and matplotlib for scripts/plot.py.
# Ubuntu 24.04's afl++ (4.09c) is built against LLVM 17, so it needs clang-17 and its ASAN runtime.
FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      afl++ clang-17 llvm-17 libclang-rt-17-dev make gdb xxd python3 python3-matplotlib \
      lld-17 flex bison libssl-dev \
 && rm -rf /var/lib/apt/lists/*

# Ubuntu sets DEBUGINFOD_URLS in login shells, which makes ASAN's llvm-symbolizer (and gdb)
# download debug info from the internet on the first crash report: minutes instead of 0.1 s.
RUN rm -f /etc/profile.d/debuginfod.sh /etc/profile.d/debuginfod.csh

# A real target, set up as in Fuzzing101 exercise 3 (CVE-2017-13028): tcpdump 4.9.2 and
# libpcap 1.8.0 from GitHub, built with afl-clang-lto and ASAN in ~/fuzzing_tcpdump.
# Two differences, because this is LLVM 17 and the exercise used LLVM 11:
# LLVM_CONFIG=llvm-config-17, and the -Wno-error flags (clang 17 rejects the old C in
# tcpdump's configure checks; clang 11 only warned). -g -O2 is what configure uses by default.
USER vscode
WORKDIR /home/vscode/fuzzing_tcpdump
ENV LLVM_CONFIG=llvm-config-17
RUN curl -fsSLo tcpdump-4.9.2.tar.gz https://github.com/the-tcpdump-group/tcpdump/archive/refs/tags/tcpdump-4.9.2.tar.gz \
 && curl -fsSLo libpcap-1.8.0.tar.gz https://github.com/the-tcpdump-group/libpcap/archive/refs/tags/libpcap-1.8.0.tar.gz \
 && tar xzf tcpdump-4.9.2.tar.gz && tar xzf libpcap-1.8.0.tar.gz \
 && mv libpcap-libpcap-1.8.0 libpcap-1.8.0 \
 && cd libpcap-1.8.0 \
 && CC=afl-clang-lto ./configure --enable-shared=no --prefix="/home/vscode/fuzzing_tcpdump/install/" \
 && AFL_USE_ASAN=1 make -j"$(nproc)" \
 && cd ../tcpdump-tcpdump-4.9.2 \
 && AFL_USE_ASAN=1 CC=afl-clang-lto CFLAGS="-g -O2 -Wno-error=implicit-function-declaration -Wno-error=implicit-int" \
      ./configure --prefix="/home/vscode/fuzzing_tcpdump/install/" \
 && AFL_USE_ASAN=1 make -j"$(nproc)" && AFL_USE_ASAN=1 make install
USER root
WORKDIR /
