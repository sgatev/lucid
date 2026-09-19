#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/cli/cli.h"

namespace lucid {
namespace {

std::vector<std::unique_ptr<Benchmark>> registered_benchmarks;

// How long a benchmark is measured for when its iteration count is left to be
// chosen: long enough that the resolution of the clock does not show up in the
// result, short enough that a suite of them still finishes quickly.
constexpr std::chrono::milliseconds kTargetRunTime(100);

// The iteration count the first run of a benchmark is measured with.
//
// One, because the cost of an iteration is exactly what is not known yet:
// starting any higher makes the first run of an expensive benchmark overshoot
// the target by as much as that first guess is wrong.
constexpr std::size_t kInitialIterations = 1;

// The iteration count no run goes past, so that a benchmark whose work is too
// fast to ever reach `kTargetRunTime` still finishes.
constexpr std::size_t kMaxIterations = 1'000'000'000;

// What one benchmark was measured to do.
struct BenchmarkResult {
  std::string_view name;
  std::size_t iterations;
  std::chrono::nanoseconds elapsed_time;
  std::int64_t bytes_processed;
};

// Runs `benchmark` over `iterations` iterations and reports what they
// measured.
BenchmarkResult Measure(Benchmark& benchmark, std::size_t iterations) {
  BenchmarkState state(iterations);
  benchmark.Run(state);
  return BenchmarkResult{
      .name = benchmark.Name(),
      .iterations = iterations,
      .elapsed_time = state.ElapsedTime(),
      .bytes_processed = state.BytesProcessed(),
  };
}

// Returns the iteration count to measure next, after `iterations` of them took
// `elapsed_time`.
//
// The count grows by the factor the run fell short of the target by, with a
// floor so that a run too fast to measure at all still grows, and a ceiling so
// that one unlucky measurement cannot ask for a run that takes minutes. The
// estimate is padded, because landing just under the target costs a whole
// further run.
std::size_t NextIterations(std::size_t iterations,
                           std::chrono::nanoseconds elapsed_time) {
  const double target = std::chrono::duration<double>(kTargetRunTime).count();
  const double elapsed = std::chrono::duration<double>(elapsed_time).count();
  const double factor =
      std::clamp(elapsed > 0 ? 1.4 * target / elapsed : 100.0, 2.0, 100.0);

  const double next = static_cast<double>(iterations) * factor;
  if (next >= static_cast<double>(kMaxIterations)) return kMaxIterations;
  return static_cast<std::size_t>(next);
}

// Measures `benchmark` with growing iteration counts until it runs for
// `kTargetRunTime`, and reports the last of those runs.
//
// Every run repeats whatever the benchmark itself sets up, so the count grows
// by the factor it fell short by rather than one step at a time.
BenchmarkResult MeasureUntilTargetRunTime(Benchmark& benchmark) {
  std::size_t iterations = kInitialIterations;
  while (true) {
    const BenchmarkResult result = Measure(benchmark, iterations);
    if (result.elapsed_time >= kTargetRunTime || iterations >= kMaxIterations) {
      return result;
    }
    iterations = NextIterations(iterations, result.elapsed_time);
  }
}

// Returns how fast `benchmark_result` got through its bytes, or nothing at
// all for a benchmark that does not count them.
std::string Throughput(const BenchmarkResult& benchmark_result) {
  if (benchmark_result.bytes_processed <= 0) return "";

  const double elapsed_seconds =
      std::chrono::duration<double>(benchmark_result.elapsed_time).count();
  if (elapsed_seconds <= 0) return "";

  static constexpr double kBytesPerMegabyte = 1'000'000;
  return std::format(", {:.1f}MB/s",
                     static_cast<double>(benchmark_result.bytes_processed) /
                         kBytesPerMegabyte / elapsed_seconds);
}

// Runs all benchmarks in the global suite and prints what they measured.
int HandleRunBenchmarksCommand(CommandContext ctx) {
  const std::optional<std::string_view> filter = ctx.Flag("filter");
  if (filter.has_value() && filter->empty()) {
    ctx.Err() << "the 'filter' flag requires a value\n";
    return 1;
  }

  // A fixed count trades the target run time for a repeatable one, which is
  // what a comparison between two runs of the same benchmark needs.
  std::optional<std::size_t> fixed_iterations;
  const std::optional<std::string_view> iterations_flag =
      ctx.Flag("iterations");
  if (iterations_flag.has_value()) {
    std::size_t iterations = 0;
    const char* begin = iterations_flag->data();
    const char* end = begin + iterations_flag->size();
    const auto [parsed_end, error] = std::from_chars(begin, end, iterations);
    if (error != std::errc() || parsed_end != end || iterations == 0) {
      ctx.Err() << "the 'iterations' flag requires a positive number\n";
      return 1;
    }
    fixed_iterations = iterations;
  }

  std::vector<Benchmark*> selected_benchmarks;
  selected_benchmarks.reserve(registered_benchmarks.size());
  for (const auto& benchmark : registered_benchmarks) {
    if (!filter.has_value() || benchmark->Name().contains(*filter)) {
      selected_benchmarks.push_back(benchmark.get());
    }
  }
  // A filter that selects nothing is reported rather than run, so that a
  // mistyped one fails instead of quietly measuring nothing.
  if (filter.has_value() && selected_benchmarks.empty()) {
    ctx.Err() << "no benchmark matches the filter '" << *filter << "'\n";
    return 1;
  }

  std::vector<BenchmarkResult> benchmark_results;
  benchmark_results.reserve(selected_benchmarks.size());
  for (Benchmark* benchmark : selected_benchmarks) {
    benchmark_results.push_back(fixed_iterations.has_value()
                                    ? Measure(*benchmark, *fixed_iterations)
                                    : MeasureUntilTargetRunTime(*benchmark));
  }

  std::chrono::nanoseconds suite_elapsed_time{0};
  for (const auto& benchmark_result : benchmark_results) {
    suite_elapsed_time += benchmark_result.elapsed_time;

    // The time per iteration is what one run says about the code; the total
    // and the count are what say how much to trust it.
    const double elapsed_ns_per_iteration =
        std::chrono::duration<double, std::nano>(benchmark_result.elapsed_time)
            .count() /
        static_cast<double>(benchmark_result.iterations);

    ctx.Out() << std::format(
        "│ {} {} iterations ({}, {:.1f}ns each{})\n", benchmark_result.name,
        benchmark_result.iterations,
        std::chrono::duration_cast<std::chrono::microseconds>(
            benchmark_result.elapsed_time),
        elapsed_ns_per_iteration, Throughput(benchmark_result));
  }

  ctx.Out() << std::format(
      "└─► {} {} ({})\n", benchmark_results.size(),
      benchmark_results.size() == 1 ? "benchmark" : "benchmarks",
      std::chrono::duration_cast<std::chrono::microseconds>(
          suite_elapsed_time));

  return 0;
}

}  // namespace

int AddBenchmark(std::unique_ptr<Benchmark> benchmark) {
  registered_benchmarks.push_back(std::move(benchmark));
  return 1;
}

// Runs the benchmarks in this binary, parsing `args` for the flags below.
int RunAllBenchmarks(std::vector<std::string_view> args) {
  return RunProgram(
      {
          .name = "benchmarks",
          .help = "Runs the benchmarks in this binary.",
          .flags = {{
                        .name = "filter",
                        .help = "Runs only the benchmarks whose name contains "
                                "this substring.",
                    },
                    {
                        .name = "iterations",
                        .help = "Runs every benchmark for this many iterations "
                                "instead of choosing a count.",
                    }},
          .handler = HandleRunBenchmarksCommand,
      },
      StandardRootCommandContext(args));
}

}  // namespace lucid

int main(int argc, char* argv[]) {
  return lucid::RunAllBenchmarks({argv, argv + argc});
}
