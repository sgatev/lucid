"""Builds a program written once in Lucid and once in C++, to time the two."""

load("@bazel_lib//lib:expand_template.bzl", "expand_template")
load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

def lucid_cpp_comparison(name):
    """Times `<name>.lu` against `<name>.cc`, both in the calling package.

    Defines `<name>_cc`, the C++ version built the way the benchmarks are,
    which is with `-c opt`, and `<name>_bench`, whose benchmarks `Lucid` and
    `Cpp` run each version whole and time it. The dashboard names a series by
    the binary and the benchmark, so these names are what keep a program's
    history together.

    Args:
      name: The name the two source files share.
    """
    cc_binary(
        name = name + "_cc",
        testonly = True,
        srcs = [name + ".cc"],
    )

    expand_template(
        name = name + "_bench_src",
        testonly = True,
        template = "//lucid/benchmarks:comparison_bench.cc.tpl",
        out = name + "_bench.cc",
        substitutions = {
            "{NAME}": name,
            "{PACKAGE}": native.package_name(),
        },
    )

    cc_binary(
        name = name + "_bench",
        testonly = True,
        srcs = [name + "_bench.cc"],
        data = [
            name + ".lu",
            ":" + name + "_cc",
        ],
        # What Lucid compiles to runs on nothing else.
        target_compatible_with = [
            "@platforms//os:macos",
            "@platforms//cpu:arm64",
        ],
        deps = [
            "//lucid/benchmarks:comparison",
            "//lucid/core/benchmarking",
        ],
    )
