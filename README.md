# luc✦d

Lucid is an experimental programming language.

## Requires

* Mac with ARM64 architecture
* C++ 20 compatible compiler
* [Bazel](https://bazel.build)

## Test

To test all targets execute

```
$ bazel test ...
```

## Build

To build the compiler execute

```
$ bazel build -c opt //lucid:compiler
```

## Run

To compile and run code execute 

```
$ bazel-bin/lucid/compiler build main examples/main.lu
$ ./main
```
