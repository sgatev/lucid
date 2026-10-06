#!/bin/bash
# Runs the tests with coverage instrumentation and reports which lines of the
# tree they reach, as a page per source under coverage/:
#
#   scripts/run_coverage.sh
#   open coverage/index.html
#
# Arguments name the tests to run, and are every test in the tree when there
# are none. The page is drawn by genhtml, from Homebrew's lcov.

set -euo pipefail

# Bazel turns what a test recorded into a report with llvm-profdata and
# llvm-cov, but does not look for them itself. They are those of the toolchain
# the tests were compiled with, which is the one xcrun finds.
bazel coverage \
  --test_env=LLVM_PROFDATA="$(xcrun -f llvm-profdata)" \
  --test_env=LLVM_COV="$(xcrun -f llvm-cov)" \
  "${@:-...}"

report="$(bazel info output_path)/_coverage/_coverage_report.dat"

# Bazel merges the reports of the tests by function name, and can count a
# lambda as never called while counting the lines in it as reached. genhtml
# refuses a report like that unless told the lines are what to trust. It also
# cannot find where a function ends in a report from llvm-cov, and is told not
# to try.
rm -rf coverage
genhtml \
  --quiet \
  --ignore-errors inconsistent,inconsistent \
  --rc derive_function_end_line=0 \
  --output-directory coverage \
  "${report}"
