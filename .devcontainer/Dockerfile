# Day 2 environment: AFL++ with clang/ASAN, gdb, and matplotlib for scripts/plot.py.
# Ubuntu 24.04's afl++ (4.09c) is built against LLVM 17, so it needs clang-17 and its ASAN runtime.
FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      afl++ clang-17 llvm-17 libclang-rt-17-dev make gdb xxd python3 python3-matplotlib \
      flex bison libssl-dev \
 && rm -rf /var/lib/apt/lists/*

# A real target: tcpdump 4.9.2 (2017) on libpcap 1.8.0, built with AFL++ and ASAN as in
# Fuzzing101 exercise 3. Sources stay in /opt/src so gdb can show them.
# The -Wno-error flags are needed because clang 17 rejects the old C in their configure checks.
WORKDIR /opt/src
RUN curl -fsSLO https://www.tcpdump.org/release/libpcap-1.8.0.tar.gz \
 && curl -fsSLO https://www.tcpdump.org/release/tcpdump-4.9.2.tar.gz \
 && echo "f47b51533f9f060afb304010ea5cbf51d032707333bca70c36351d255754659c  libpcap-1.8.0.tar.gz" | sha256sum -c \
 && echo "798b3536a29832ce0cbb07fafb1ce5097c95e308a6f592d14052e1ef1505fe79  tcpdump-4.9.2.tar.gz" | sha256sum -c \
 && tar xzf libpcap-1.8.0.tar.gz && tar xzf tcpdump-4.9.2.tar.gz && rm *.tar.gz \
 && cd libpcap-1.8.0 \
 && CC=afl-clang-fast ./configure --enable-shared=no --prefix=/opt/tcpdump \
 && AFL_USE_ASAN=1 make -j"$(nproc)" && make install \
 && cd ../tcpdump-4.9.2 \
 && AFL_USE_ASAN=1 CC=afl-clang-fast CFLAGS="-g -O1 -Wno-error=implicit-function-declaration -Wno-error=implicit-int" \
      ./configure --prefix=/opt/tcpdump \
 && AFL_USE_ASAN=1 make -j"$(nproc)" && AFL_USE_ASAN=1 make install \
 && ln -s /opt/tcpdump/sbin/tcpdump /usr/local/bin/tcpdump
WORKDIR /
