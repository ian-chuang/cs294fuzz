#!/usr/bin/env bash
# Our helper, not part of AFL++. Runs triage.sh on every input, then groups the
# inputs that report the same error at the same place, and prints one line per
# group: how many inputs, the error, where, and one example file.
#   scripts/group.sh <harness> <dir-or-files...>
# Example:
#   scripts/group.sh ini results/ini/default/crashes results/ini/default/hangs
cd "$(dirname "$0")/.."
harness=${1:?usage: group.sh <harness> <dir-or-files...>}
shift

# triage.sh prints 4 lines per input: the file, "ran:", "error:", "where:".
# Turn each input into one line: error | where | file. Then count equal error|where pairs.
scripts/triage.sh "$harness" "$@" \
  | awk '/^  error:/ { error = substr($0, 11) }
         /^  where:/ { print error " | " substr($0, 11) " | " file }
         /^[^ ]/     { file = $1 }' \
  | awk -F' \\| ' '{ key = $1 " | " $2; count[key]++; if (!(key in example)) example[key] = $3 }
                   END { for (k in count) printf "%3d inputs  %s\n            e.g. %s\n", count[k], k, example[k] }' \
  | paste - - | sort -rn | tr '\t' '\n'
