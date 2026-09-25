#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <ostream>
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

// How many times a settled iteration count is measured, of which the fastest
// is what gets reported.
//
// What interferes with a run only ever costs it time: the scheduler takes the
// core away, the clock drops, something else wants the cache. So repeated
// runs of the same work spread out on one side alone, and the fastest of them
// is the one that was interfered with least. Measuring once leaves the figure
// to whether that single run was an unlucky one, which for the benchmarks
// here meant a fifth of a run's time appearing and disappearing between one
// invocation and the next.
constexpr std::size_t kMeasuredRuns = 3;

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

// Measures `benchmark` over `iterations` iterations `kMeasuredRuns` times and
// reports the fastest of those runs.
BenchmarkResult MeasureFastestRun(Benchmark& benchmark,
                                  std::size_t iterations) {
  BenchmarkResult fastest = Measure(benchmark, iterations);
  for (std::size_t run = 1; run < kMeasuredRuns; ++run) {
    const BenchmarkResult result = Measure(benchmark, iterations);
    if (result.elapsed_time < fastest.elapsed_time) fastest = result;
  }
  return fastest;
}

// Measures `benchmark` with growing iteration counts until it runs for
// `kTargetRunTime`, and reports the fastest run at the count it settled on.
//
// Every run repeats whatever the benchmark itself sets up, so the count grows
// by the factor it fell short by rather than one step at a time. The run that
// ends the growing is not the one reported: it is the first to cross the
// target, which is to say the runs that cross it by being slow are the ones
// that end it.
BenchmarkResult MeasureUntilTargetRunTime(Benchmark& benchmark) {
  std::size_t iterations = kInitialIterations;
  while (true) {
    const BenchmarkResult result = Measure(benchmark, iterations);
    if (result.elapsed_time >= kTargetRunTime || iterations >= kMaxIterations) {
      return MeasureFastestRun(benchmark, iterations);
    }
    iterations = NextIterations(iterations, result.elapsed_time);
  }
}

// Returns how fast `benchmark_result` got through its bytes, or nothing at
// all for a benchmark that does not count them.
std::optional<double> MegabytesPerSecond(
    const BenchmarkResult& benchmark_result) {
  if (benchmark_result.bytes_processed <= 0) return std::nullopt;

  const double elapsed_seconds =
      std::chrono::duration<double>(benchmark_result.elapsed_time).count();
  if (elapsed_seconds <= 0) return std::nullopt;

  static constexpr double kBytesPerMegabyte = 1'000'000;
  return static_cast<double>(benchmark_result.bytes_processed) /
         kBytesPerMegabyte / elapsed_seconds;
}

// Returns how long one iteration of `benchmark_result` took.
//
// The time per iteration is what one run says about the code; the total and
// the count are what say how much to trust it.
double ElapsedNanosPerIteration(const BenchmarkResult& benchmark_result) {
  return std::chrono::duration<double, std::nano>(benchmark_result.elapsed_time)
             .count() /
         static_cast<double>(benchmark_result.iterations);
}

// Returns the throughput of `benchmark_result` as the report writes it, or an
// empty string for a benchmark that does not count its bytes.
std::string Throughput(const BenchmarkResult& benchmark_result) {
  const std::optional<double> megabytes_per_second =
      MegabytesPerSecond(benchmark_result);
  if (!megabytes_per_second.has_value()) return "";

  return std::format(", {:.1f}MB/s", *megabytes_per_second);
}

// How the results of a run are reported.
enum class ReportFormat { kText, kJson };

// Prints `benchmark_results` as the report a person reads.
void PrintTextReport(std::ostream& out,
                     const std::vector<BenchmarkResult>& benchmark_results) {
  std::chrono::nanoseconds suite_elapsed_time{0};
  for (const auto& benchmark_result : benchmark_results) {
    suite_elapsed_time += benchmark_result.elapsed_time;

    out << std::format("│ {} {} iterations ({}, {:.1f}ns each{})\n",
                       benchmark_result.name, benchmark_result.iterations,
                       std::chrono::duration_cast<std::chrono::microseconds>(
                           benchmark_result.elapsed_time),
                       ElapsedNanosPerIteration(benchmark_result),
                       Throughput(benchmark_result));
  }

  out << std::format("└─► {} {} ({})\n", benchmark_results.size(),
                     benchmark_results.size() == 1 ? "benchmark" : "benchmarks",
                     std::chrono::duration_cast<std::chrono::microseconds>(
                         suite_elapsed_time));
}

// Prints `benchmark_results` as the JSON that github-action-benchmark reads:
// an array of one object per benchmark, measured in nanoseconds per iteration,
// where a smaller value is the better one.
//
// A name is the identifier the benchmark was declared with, so it goes into a
// JSON string as it is.
void PrintJsonReport(std::ostream& out,
                     const std::vector<BenchmarkResult>& benchmark_results) {
  out << "[\n";
  for (std::size_t i = 0; i < benchmark_results.size(); ++i) {
    const BenchmarkResult& benchmark_result = benchmark_results[i];

    std::string extra =
        std::format("{} iterations", benchmark_result.iterations);
    if (const std::optional<double> megabytes_per_second =
            MegabytesPerSecond(benchmark_result);
        megabytes_per_second.has_value()) {
      extra += std::format(", {:.1f}MB/s", *megabytes_per_second);
    }

    out << std::format(
        R"(  {{"name": "{}", "unit": "ns/iter", "value": {:.3f}, "extra": "{}"}}{})"
        "\n",
        benchmark_result.name, ElapsedNanosPerIteration(benchmark_result),
        extra, i + 1 < benchmark_results.size() ? "," : "");
  }
  out << "]\n";
}

// Runs all benchmarks in the global suite and prints what they measured.
int HandleRunBenchmarksCommand(CommandContext ctx) {
  const std::optional<std::string_view> filter = ctx.Flag("filter");
  if (filter.has_value() && filter->empty()) {
    ctx.Err() << "the 'filter' flag requires a value\n";
    return 1;
  }

  ReportFormat format = ReportFormat::kText;
  if (const std::optional<std::string_view> format_flag = ctx.Flag("format");
      format_flag.has_value()) {
    if (*format_flag == "text") {
      format = ReportFormat::kText;
    } else if (*format_flag == "json") {
      format = ReportFormat::kJson;
    } else {
      ctx.Err() << "the 'format' flag requires 'text' or 'json'\n";
      return 1;
    }
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
    benchmark_results.push_back(
        fixed_iterations.has_value()
            ? MeasureFastestRun(*benchmark, *fixed_iterations)
            : MeasureUntilTargetRunTime(*benchmark));
  }

  switch (format) {
    case ReportFormat::kText:
      PrintTextReport(ctx.Out(), benchmark_results);
      break;
    case ReportFormat::kJson:
      PrintJsonReport(ctx.Out(), benchmark_results);
      break;
  }

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
                    },
                    {
                        .name = "format",
                        .help = "Prints the results as 'text', the default, or "
                                "as the 'json' that a benchmark tracker reads.",
                    }},
          .handler = HandleRunBenchmarksCommand,
      },
      StandardRootCommandContext(args));
}

}  // namespace lucid

int main(int argc, char* argv[]) {
  return lucid::RunAllBenchmarks({argv, argv + argc});
}
