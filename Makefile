# Build the fuzz targets with AFL++'s compiler (adds coverage instrumentation)
# and AddressSanitizer (AFL_USE_ASAN=1: memory errors crash loudly).
CC     := afl-clang-fast
CFLAGS := -g -O1 -fno-omit-frame-pointer -Itarget
HARNESSES := ini ini_trunc ini_buggy checksum

all: $(addprefix build/fuzz_,$(HARNESSES))

build/fuzz_%: target/harness_%.c target/minilib.c target/minilib.h | build
	AFL_USE_ASAN=1 $(CC) $(CFLAGS) -o $@ target/harness_$*.c target/minilib.c

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: all clean
