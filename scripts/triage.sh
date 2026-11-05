#!/usr/bin/env bash
# Our helper, not part of AFL++. For every saved crash (or hang) input, it runs
# the same command you would type by hand, and prints the three lines of the
# AddressSanitizer report that matter: what went wrong, and where.
#   scripts/triage.sh <harness> <dir-or-files...>
# Examples:
#   scripts/triage.sh ini results/ini/default/crashes
cd "$(dirname "$0")/.."
harness=${1:?usage: triage.sh <harness: ini|ini2|join> <dir-or-files...>}
shift

export ASAN_OPTIONS=detect_leaks=0

for target in "$@"; do
  if [ -d "$target" ]; then files=$(find "$target" -maxdepth 1 -type f ! -name README.txt ! -name '.*' | sort)
  else files=$target; fi
  for f in $files; do
    # 1. Run it, exactly as by hand (giving up after 5 seconds). Keep only what it prints to stderr.
    ran="build/fuzz_$harness < $f"
    out=$(timeout 5 "build/fuzz_$harness" < "$f" 2>&1 > /dev/null)
    rc=$?

    # 2. Pick out what went wrong and where.
    if [ $rc -eq 124 ]; then
      error="hang: still running after 5 seconds"
      where="(no report: the program never stopped)"
    elif echo "$out" | grep -q "ERROR: AddressSanitizer"; then
      # e.g. "heap-buffer-overflow on address 0x502000000098 at pc ..." -> "heap-buffer-overflow"
      error=$(echo "$out" | grep -m1 "ERROR: AddressSanitizer" | sed -E 's/.*AddressSanitizer: ([^ ]+).*/\1/')
      # "WRITE of size 1 at ..." (overflows) or "caused by a READ memory access" (SEGV)
      access=$(echo "$out" | grep -m1 -oE '^(READ|WRITE) of size [0-9]+|caused by a (READ|WRITE)' | sed 's/caused by a //')
      [ -n "$access" ] && error="$error, $access"
      # the first stack frame in a .c file: the program's own code, not libc or ASAN
      where=$(echo "$out" | grep -E '^ +#[0-9]+ .* in [^ ]+ [^ ]+\.c:[0-9]+' | head -1 \
              | sed -E "s/.* in ([^ ]+) ([^ ]+\.c):([0-9]+).*/\1  \2:\3/; s#$PWD/##; s#/\./#/#")
      where=${where:-"(no frame in the program code)"}
    elif echo "$out" | grep -q "Assertion"; then
      # "fuzz_ini: target/minilib.c:76: int parse_ini(...): Assertion `...' failed."
      error="assertion failed: $(echo "$out" | grep -m1 Assertion | sed -E "s/.*Assertion \`(.*)' failed.*/\1/")"
      where=$(echo "$out" | grep -m1 Assertion | sed -E 's/^[^:]+: ([^:]+:[0-9]+): .* ([a-z_]+)\(.*/\2  \1/')
    else
      error="no crash (exit code $rc)"
      where="-"
    fi

    echo "$f  ($(wc -c < "$f" | tr -d ' ') bytes)"
    echo "  ran:    $ran"
    echo "  error:  $error"
    echo "  where:  $where"
    echo
  done
done
