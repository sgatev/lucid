# luc✦d

Lucid is an experimental programming language.

## Requires

* ARM64 macOS
* Apple Clang 15.0+
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

or simply

```
$ bazel-bin/lucid/compiler run examples/main.lu
```
