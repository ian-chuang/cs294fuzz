#!/usr/bin/env bash
# Our helper, not part of AFL++: re-run every saved crash (or hang) input and
# print one line each: the kind of error and the first stack frame in the
# program's own code.
#   scripts/triage.sh <harness> <dir-or-files...>
# Examples:
#   scripts/triage.sh ini results/ini_run_a/default/crashes
#   scripts/triage.sh tcpdump tcpdump/crashes
cd "$(dirname "$0")/.."
harness=${1:?usage: triage.sh <harness: ini|ini_trunc|ini_buggy|checksum|tcpdump> <dir-or-files...>}
shift

export ASAN_OPTIONS=detect_leaks=0:symbolize=1

# Run the target on one input file, giving up after 5 seconds.
run() {
  if [ "$harness" = tcpdump ]; then
    timeout 5 tcpdump -vvvvXX -ee -nn -r "$1" 2>&1 > /dev/null   # same flags as the fuzzing run; keep only stderr
  else
    timeout 5 "build/fuzz_$harness" < "$1" 2>&1
  fi
}

for target in "$@"; do
  if [ -d "$target" ]; then files=$(find "$target" -maxdepth 1 -type f ! -name README.txt ! -name '.*' | sort)
  else files=$target; fi
  for f in $files; do
    out=$(run "$f"); rc=$?
    if [ $rc -eq 124 ]; then sig="HANG: still running after 5 seconds"
    elif echo "$out" | grep -q "SUMMARY: AddressSanitizer"; then
      kind=$(echo "$out" | grep -m1 "SUMMARY: AddressSanitizer" | sed 's/SUMMARY: AddressSanitizer: \([^ ]*\).*/\1/')
      # first stack frame in a .c file (ASAN's own frames are in .cpp/.inc files)
      frame=$(echo "$out" | grep -E '^ +#[0-9]+ ' | grep -E '\.c:[0-9]+' | head -1 \
              | sed -E 's/.* in ([^ ]+) ([^ ]*\/)?([^ /]+\.c:[0-9]+).*/\3 in \1/')
      sig="$kind  ${frame:-(no frame in the program code)}"
    elif echo "$out" | grep -qi "assert"; then sig=$(echo "$out" | grep -i -m1 "assert" | cut -c1-120)
    else sig="exit code $rc"; fi
    printf '%-42s %-6s %s\n' "$(basename "$f" | cut -c1-42)" "$(wc -c < "$f" | tr -d ' ')B" "$sig"
  done
done
