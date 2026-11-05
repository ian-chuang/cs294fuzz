# Day 2 environment: AFL++ with clang/ASAN, gdb, and matplotlib for scripts/plot.py.
# Ubuntu 24.04's afl++ (4.09c) is built against LLVM 17, so it needs clang-17 and its ASAN runtime.
FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      afl++ clang-17 llvm-17 libclang-rt-17-dev make gdb xxd python3 python3-matplotlib \
 && rm -rf /var/lib/apt/lists/*

# Ubuntu sets DEBUGINFOD_URLS in login shells, which makes ASAN's llvm-symbolizer (and gdb)
# download debug info from the internet on the first crash report: minutes instead of 0.1 s.
RUN rm -f /etc/profile.d/debuginfod.sh /etc/profile.d/debuginfod.csh
