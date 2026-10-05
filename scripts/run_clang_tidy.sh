#!/bin/bash
# Runs clang-tidy over every source in the tree with the checks in .clang-tidy,
# and fails if any of them finds something:
#
#   scripts/run_clang_tidy.sh
#
# clang-tidy is taken from Homebrew's LLVM, unless LLVM_PREFIX names another
# installation. Apple's toolchain does not ship it.

set -euo pipefail

llvm_prefix="${LLVM_PREFIX:-$(brew --prefix llvm)}"
"${llvm_prefix}/bin/clang-tidy" --version

# A source can include a header the build generates, such as
# lucid/compiler/version.h, and clang-tidy cannot read a source whose headers
# are missing. They are built without compiling anything else.
bazel build --output_groups=compilation_prerequisites_INTERNAL_ //lucid/...
bazel run @hedron_compile_commands//:refresh_all

# The extractor also lists each header, with the command of a source that
# includes it, and clang-tidy would then check that source in its place. Only
# the sources in the tree are kept, and clang-tidy works out each header's
# command itself when it reaches it through them. A source is also listed once
# for every target that compiles it, all with the same command, so it is kept
# once.
db_dir=$(mktemp -d)
trap 'rm -rf "${db_dir}"' EXIT
python3 - "${db_dir}/compile_commands.json" <<'EOF'
import json
import sys

with open("compile_commands.json") as f:
    commands = json.load(f)
sources = {}
for c in commands:
    if c["file"].startswith("lucid/") and c["file"].endswith(".cc"):
        sources.setdefault(c["file"], c)
with open(sys.argv[1], "w") as f:
    json.dump(list(sources.values()), f)
EOF

# Homebrew's clang does not know where the SDK the build uses lives, so it is
# told. Headers are reported once their path is in the tree, which leaves out
# the SDK's and those of external repositories.
"${llvm_prefix}/bin/run-clang-tidy" \
  -p "${db_dir}" \
  -clang-tidy-binary "${llvm_prefix}/bin/clang-tidy" \
  -extra-arg=-isysroot -extra-arg="$(xcrun --show-sdk-path)" \
  -header-filter="^(\./)?lucid/|^${PWD}/lucid/" \
  -warnings-as-errors='*' \
  -quiet
