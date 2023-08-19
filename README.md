# Lucid

An experimental programming language.

## Requires

* Mac with ARM64 architecture
* C++ 20 compatible compiler
* [Bazel](https://bazel.build)

## Test

To test all targets execute

```bash
$ bazel test ...
```

## Build

To build the compiler execute

```bash
$ bazel build -c opt //lucid:compiler
```

## Run

To compile code and run it execute 

```bash
$ bazel-bin/lucid/compiler build main examples/main.lu
$ ./main
```
