# Lucid

Lucid is an experimental programming language and toolchain developed from scratch.

## Overview

- [Language](LANGUAGE.md)
- [Architecture](ARCHITECTURE.md)

## Requires

- ARM64 macOS
- [Xcode](https://developer.apple.com/xcode) 27
- [Bazel](https://bazel.build)

## Build

To build the compiler execute

```
bazel build -c opt //lucid/compiler:main
```

## Run

To compile and run code execute

```
bazel run -c opt //lucid/compiler:main -- run $PWD/examples/main.lu
```

## Test

To test all targets execute

```
bazel test ...
```

## Benchmark

To run every benchmark execute

```
scripts/run_benchmarks.sh
```

A single binary can be run on its own, and reports what it measured either for
a person to read or as the JSON a benchmark tracker reads

```
bazel run -c opt //lucid/am:reg_bench
bazel run -c opt //lucid/am:reg_bench -- --format=json
```

Every push to `main` records the results, which are charted at
[sgatev.github.io/lucid/dev/bench](https://sgatev.github.io/lucid/dev/bench/).

## Code

To enable syntax highlighting in your editor install

- [neovim-lucid](https://github.com/sgatev/nvim-lucid/tree/main) plugin for [Neovim](https://neovim.io)
- [tree-sitter-lucid](https://github.com/sgatev/tree-sitter-lucid/tree/main) grammar for [Tree-sitter](https://tree-sitter.github.io/tree-sitter)

## Debug

To print the abstract syntax tree derived from code execute

```
bazel run -c opt //lucid/compiler:main -- print-ast $PWD/examples/main.lu
```

To print the syntax control flow graph derived from code execute

```
bazel run -c opt //lucid/compiler:main -- print-syntax-cfg $PWD/examples/main.lu
```

To print the abstract machine control flow graph derived from code execute

```
bazel run -c opt //lucid/compiler:main -- print-am-cfg $PWD/examples/main.lu
```
