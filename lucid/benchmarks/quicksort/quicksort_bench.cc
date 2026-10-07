// Times the same QuickSort written in Lucid and in C++, each as a program of
// its own run from start to end, so that the two are measured the same way.
//
// What a run spends starting the process and generating the numbers is in
// both, and small beside the sorting: a few percent of it.

#include <mach-o/dyld.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <string>
#include <system_error>

#include "lucid/compiler/compiler.h"
#include "lucid/core/benchmarking/benchmarking.h"

extern char** environ;

namespace lucid {
namespace {

// What both programs return, which is the checksum of what they sorted. A run
// that returns anything else sorted wrong, and how long it took is no
// measurement of sorting: the benchmark stops instead of recording it.
constexpr int kChecksum = 239;

// Returns where the files this binary was built with are found, which is
// beside the binary rather than wherever it was run from.
std::filesystem::path Runfiles() {
  std::uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::string path(size, '\0');
  if (_NSGetExecutablePath(path.data(), &size) != 0) std::abort();
  path.resize(path.find('\0'));
  return std::filesystem::path(path += ".runfiles") / "_main" / "lucid" /
         "benchmarks" / "quicksort";
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

// Runs the program at `path` once for every iteration of `state`, after a
// run of it that is not measured.
//
// The first run of a binary is slower than the ones after it, by as much as a
// fifth of this one, because the system checks a program it has not run
// before. That is the same for both programs and says nothing about either.
void RunEach(BenchmarkState& state, const std::filesystem::path& path) {
  if (Run(path) != kChecksum) std::abort();
  for (auto _ : state) {
    if (Run(path) != kChecksum) std::abort();
  }
}

// quicksort.lu, compiled into a temporary directory the first time it is
// asked for and removed as the benchmarks end.
//
// Once for all the runs of the benchmark rather than once for each, so that
// every run times a binary that has been run before.
class LucidProgram {
 public:
  static const std::filesystem::path& Path() {
    static const LucidProgram program;
    return program.path_;
  }

  LucidProgram(const LucidProgram&) = delete;
  LucidProgram& operator=(const LucidProgram&) = delete;

  ~LucidProgram() {
    std::error_code ignored;
    std::filesystem::remove(path_, ignored);
    // What the compiler wrote before linking, beside the binary.
    std::filesystem::remove(std::filesystem::path(path_) += ".o", ignored);
  }

 private:
  LucidProgram()
      : path_(std::filesystem::temp_directory_path() /
              std::format("quicksort-{}", getpid())) {
    if (!BuildCode({.src_path = Runfiles() / "quicksort.lu", .out_path = path_})
             .has_value()) {
      std::abort();
    }
  }

  std::filesystem::path path_;
};

BENCHMARK(Lucid) { RunEach(state, LucidProgram::Path()); }

// quicksort.cc, which Bazel built beside this binary.
BENCHMARK(Cpp) { RunEach(state, Runfiles() / "quicksort_cc"); }

}  // namespace
}  // namespace lucid
