# luc✦d

Lucid is an experimental programming language.

## Overview

* [Architecture](ARCHITECTURE.md)

## Requires

* ARM64 macOS
* Xcode 15.3+
* [Bazel](https://bazel.build)

## Test

To test all targets execute

```
bazel test ...
```

## Build

To build the compiler execute

```
bazel build -c opt //lucid:compiler
```

## Run

To compile and run code execute 

```
bazel run -c opt //lucid:compiler -- run examples/main.lu
```

## Code

To enable syntax highlighting in your editor install

* [neovim-lucid](https://github.com/sgatev/nvim-lucid/tree/main) plugin for [Neovim](https://neovim.io)
* [tree-sitter-lucid](https://github.com/sgatev/tree-sitter-lucid/tree/main) grammar for [Tree-sitter](https://tree-sitter.github.io/tree-sitter)
