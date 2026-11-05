# How to build the programs the fuzzer runs. Type `make` to build all of them,
# or `make build/fuzz_join` to build one.
#
# Every program is one harness (target/harness_*.c) plus the library
# (target/minilib.c), compiled with:
#   afl-clang-fast   AFL++'s version of clang: adds code that records which
#                    parts of the program each input reaches (coverage)
#   AFL_USE_ASAN=1   also turns on AddressSanitizer: memory errors crash loudly
#   -g               keep line numbers, so crash reports can say "minilib.c:92"
#   -O1              light optimization: faster, and line numbers stay mostly accurate
#   -fno-omit-frame-pointer   clearer stack traces in crash reports
#   -Itarget         look for minilib.h in target/
#
# Each rule below says: to make the file on the left, these files on the right
# are needed; if any of them changed, run the command on the next lines.

FLAGS = -g -O1 -fno-omit-frame-pointer -Itarget

all: build/fuzz_ini build/fuzz_ini2 build/fuzz_join

build/fuzz_ini: target/harness_ini.c target/minilib.c target/minilib.h
	mkdir -p build
	AFL_USE_ASAN=1 afl-clang-fast $(FLAGS) -o build/fuzz_ini target/harness_ini.c target/minilib.c

build/fuzz_ini2: target/harness_ini2.c target/minilib.c target/minilib.h
	mkdir -p build
	AFL_USE_ASAN=1 afl-clang-fast $(FLAGS) -o build/fuzz_ini2 target/harness_ini2.c target/minilib.c

build/fuzz_join: target/harness_join.c target/minilib.c target/minilib.h
	mkdir -p build
	AFL_USE_ASAN=1 afl-clang-fast $(FLAGS) -o build/fuzz_join target/harness_join.c target/minilib.c

# `make clean` deletes everything in build/
clean:
	rm -rf build
