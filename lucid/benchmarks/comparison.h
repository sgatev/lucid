#pragma once

#include <filesystem>
#include <string_view>

#include "lucid/core/benchmarking/benchmarking.h"

namespace lucid {

// A program written once in Lucid and once in C++, statement for statement,
// whose two versions are timed against each other.
//
// Each is run whole as a process of its own, so that the two are measured the
// same way: what a run spends starting the process and generating its input
// is in both. The program reports what it worked out as its exit status, and
// the C++ version is the reference for it. A Lucid run that comes to anything
// else got the program wrong, and how long it took is no measurement of the
// program, so the benchmark stops instead of recording it.
//
// The two files are `<name>.lu` and `<name>.cc` in `package`, which is what
// `lucid_cpp_comparison` in comparison.bzl builds from.
class ComparedProgram {
 public:
  // `package` is the directory of the Bazel package holding the program, such
  // as "lucid/benchmarks/fib", and `name` is the name its files share.
  ComparedProgram(std::string_view package, std::string_view name);

  ComparedProgram(const ComparedProgram&) = delete;
  ComparedProgram& operator=(const ComparedProgram&) = delete;

  // Removes the Lucid binary, if it was ever built.
  ~ComparedProgram();

  // Times the Lucid version, once for every iteration of `state`.
  void RunLucid(BenchmarkState& state);

  // Times the C++ version, once for every iteration of `state`.
  void RunCpp(BenchmarkState& state);

 private:
  // Returns the Lucid binary, compiled into a temporary directory the first
  // time it is asked for.
  //
  // Once for all the runs of the benchmark rather than once for each, so that
  // every run times a binary that has been run before.
  const std::filesystem::path& LucidBinary();

  // Returns what the C++ version exits with, run once the first time it is
  // asked for.
  int Expected();

  // Runs the binary at `path` once for every iteration of `state`, after a
  // run of it that is not measured, and stops if any run of it exits with
  // anything other than `Expected()`.
  void RunEach(BenchmarkState& state, const std::filesystem::path& path);

  std::filesystem::path runfiles_;
  std::string_view name_;
  std::filesystem::path lucid_binary_;
  int expected_ = -1;
};

}  // namespace lucid
