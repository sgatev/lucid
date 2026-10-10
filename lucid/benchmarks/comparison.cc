#include "lucid/benchmarks/comparison.h"

#include <mach-o/dyld.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <system_error>

#include "lucid/compiler/compiler.h"
#include "lucid/core/benchmarking/benchmarking.h"

extern char** environ;

namespace lucid {
namespace {

// Returns where the files the running binary was built with are found, which
// is beside the binary rather than wherever it was run from.
std::filesystem::path Runfiles(std::string_view package) {
  std::uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::string path(size, '\0');
  if (_NSGetExecutablePath(path.data(), &size) != 0) std::abort();
  path.resize(path.find('\0'));
  return std::filesystem::path(path += ".runfiles") / "_main" / package;
}

// Runs the program at `path` and returns its exit status.
int Run(const std::filesystem::path& path) {
  std::string program = path.string();
  char* argv[] = {program.data(), nullptr};
  pid_t pid = 0;
  if (posix_spawn(&pid, program.c_str(), nullptr, nullptr, argv, environ) !=
      0) {
    std::abort();
  }
  int status = 0;
  if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status)) std::abort();
  return WEXITSTATUS(status);
}

}  // namespace

ComparedProgram::ComparedProgram(std::string_view package,
                                 std::string_view name)
    : runfiles_(Runfiles(package)), name_(name) {}

ComparedProgram::~ComparedProgram() {
  if (lucid_binary_.empty()) return;

  std::error_code ignored;
  std::filesystem::remove(lucid_binary_, ignored);
  // What the compiler wrote before linking, beside the binary.
  std::filesystem::remove(std::filesystem::path(lucid_binary_) += ".o",
                          ignored);
}

void ComparedProgram::RunLucid(BenchmarkState& state) {
  RunEach(state, LucidBinary());
}

void ComparedProgram::RunCpp(BenchmarkState& state) {
  RunEach(state, runfiles_ / std::format("{}_cc", name_));
}

const std::filesystem::path& ComparedProgram::LucidBinary() {
  if (!lucid_binary_.empty()) return lucid_binary_;

  lucid_binary_ = std::filesystem::temp_directory_path() /
                  std::format("{}-{}", name_, getpid());
  if (!BuildCode({.src_path = runfiles_ / std::format("{}.lu", name_),
                  .out_path = lucid_binary_})
           .has_value()) {
    std::abort();
  }
  return lucid_binary_;
}

int ComparedProgram::Expected() {
  if (expected_ < 0) expected_ = Run(runfiles_ / std::format("{}_cc", name_));
  return expected_;
}

// The first run of a binary is slower than the ones after it, by as much as a
// fifth of QuickSort, because the system checks a program it has not run
// before. That is the same for both versions and says nothing about either, so
// that run is not measured.
void ComparedProgram::RunEach(BenchmarkState& state,
                              const std::filesystem::path& path) {
  const int expected = Expected();
  if (Run(path) != expected) std::abort();
  for (auto _ : state) {
    if (Run(path) != expected) std::abort();
  }
}

}  // namespace lucid
