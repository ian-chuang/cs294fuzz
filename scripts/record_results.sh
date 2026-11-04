#!/usr/bin/env bash
# Instructor only: re-record the pre-made runs in results/ (about 50 minutes at the default).
# Run it in the Codespace, then commit results/.
#   scripts/record_results.sh [seconds-per-run]
#
# The tcpdump campaign is run separately, with Fuzzing101 exercise 3's command (several hours):
#   afl-fuzz -m none -i $HOME/fuzzing_tcpdump/tcpdump-tcpdump-4.9.2/tests/ -o $HOME/fuzzing_tcpdump/out/ -s 123 \
#     -- $HOME/fuzzing_tcpdump/install/sbin/tcpdump -vvvvXX -ee -nn -r @@
set -euo pipefail
cd "$(dirname "$0")/.."
secs=${1:-600}

make
for spec in ini:ini_run_a ini:ini_run_b ini_trunc:ini_trunc ini_buggy:ini_buggy checksum:checksum; do
  harness=${spec%%:*}
  name=${spec##*:}
  case $harness in
    ini_buggy) seeds=seeds/ini_small ;;   # this harness only has a 64-byte buffer
    ini*)      seeds=seeds/ini ;;
    *)         seeds=seeds/$harness ;;
  esac
  rm -rf "results/$name"
  AFL_NO_UI=1 afl-fuzz -i "$seeds" -o "results/$name" -V "$secs" -- "build/fuzz_$harness" > /dev/null
  echo "$name: $(grep -E 'saved_crashes|saved_hangs|edges_found' results/$name/default/fuzzer_stats | tr -s ' ' | paste -sd ' ' -)"
done
scripts/plot.py results/coverage.png results/ini_run_a results/ini_run_b results/ini_trunc results/checksum
