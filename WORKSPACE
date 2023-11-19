load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "bazel_skylib",
    remote = "https://github.com/bazelbuild/bazel-skylib",
    tag = "1.5.0",
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

git_repository(
    name = "gtest",
    remote = "https://github.com/google/googletest",
    tag = "v1.14.0",
)

git_repository(
    name = "gbench",
    remote = "https://github.com/google/benchmark",
    tag = "v1.8.3",
)
