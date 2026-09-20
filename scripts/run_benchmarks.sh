#!/bin/bash
# Runs every benchmark in the tree and prints the results as the single JSON
# array that github-action-benchmark reads. Bazel's own output goes to stderr,
# so the report is what stdout carries:
#
#   scripts/run_benchmarks.sh > benchmarks.json
#
# A benchmark name is only unique within the binary that declares it. Each one
# is prefixed here with the target it came from, which is what keeps them apart
# in the recorded history.

set -euo pipefail

targets=$(bazel query 'filter("_bench$", kind("cc_binary", //lucid/...))')

# Built in one step, so that the measurements are not spread across the time
# it takes to compile the rest of them.
bazel build -c opt ${targets}

objects=$(mktemp)
trap 'rm -f "${objects}"' EXIT

for target in ${targets}; do
  # //lucid/am:reg_bench is built to bazel-bin/lucid/am/reg_bench, and names
  # its benchmarks lucid/am/reg_bench/ColorChain64 and so on.
  name="${target#//}"
  name="${name/://}"

  echo "Running ${target}" >&2

  # One object per line, which is what lets the prefix go in and the brackets
  # around the array of a single binary drop out, in one pass.
  "bazel-bin/${name}" --format=json |
    sed -n "s|^  {\"name\": \"|  {\"name\": \"${name}/|p" >>"${objects}"
done

if [[ ! -s "${objects}" ]]; then
  echo "No benchmark reported a result." >&2
  exit 1
fi

# Every binary comma-separates the objects of its own array, so those commas
# come off before the ones that separate the objects of this one go in.
echo "["
awk '
  { sub(/,$/, "") }
  NR > 1 { print previous "," }
  { previous = $0 }
  END { print previous }
' "${objects}"
echo "]"
