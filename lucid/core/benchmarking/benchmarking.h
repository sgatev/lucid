#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include "lucid/core/meta/macros.h"

namespace lucid {

// The iterations of one benchmark run, and the clock that times them.
//
// A benchmark prepares whatever it needs first and then iterates over the
// state, so that only the work inside the loop is measured:
//
//   BENCHMARK(Push) {
//     Stack stack;
//     for (auto _ : state) stack.Push(1);
//   }
//
// The loop runs `MaxIterations()` times. A benchmark that needs one input per
// iteration can ask for that count before the loop starts, and prepare them
// all outside the measurement.
//
// The loop has to do something the compiler cannot see through, or an
// optimized build is free to delete it and measure nothing. Work whose result
// is otherwise unused goes through `DoNotOptimize`.
class BenchmarkState {
 public:
  explicit BenchmarkState(std::size_t max_iterations)
      : max_iterations_(max_iterations) {}

  // Marks the end of the iterations.
  class Sentinel {};

  // Hands out the iterations of a run, one per step.
  class Iterator {
   public:
    explicit Iterator(BenchmarkState& state)
        : state_(&state), remaining_(state.max_iterations_) {}

    // The loop variable of a benchmark carries nothing: the iterations are all
    // there is to hand out, and the benchmark counts them rather than reads
    // them.
    struct Iteration {};

    Iteration operator*() const { return {}; }

    Iterator& operator++() {
      --remaining_;
      return *this;
    }

    // Stops the clock as the loop ends, so that whatever the benchmark does
    // afterwards is left out of the measurement.
    //
    // This is what the range-based for loop evaluates before every iteration,
    // including the one that ends it, which is the last moment that belongs to
    // the measured work.
    bool operator!=(Sentinel) {
      if (remaining_ > 0) return true;
      state_->Stop();
      return false;
    }

   private:
    BenchmarkState* state_;
    std::size_t remaining_;
  };

  // Starts the clock and returns the iterations to measure.
  Iterator begin() {
    Iterator it(*this);
    // Started last, so that building the iterator is not measured.
    start_time_ = std::chrono::steady_clock::now();
    return it;
  }

  Sentinel end() const { return {}; }

  // Returns the number of iterations this run measures.
  std::size_t MaxIterations() const { return max_iterations_; }

  // Returns how long the measured iterations took.
  std::chrono::nanoseconds ElapsedTime() const { return elapsed_time_; }

  // Records how many bytes the measured iterations processed altogether, for
  // a benchmark whose work is reported better as a rate than as a time.
  void SetBytesProcessed(std::int64_t bytes_processed) {
    bytes_processed_ = bytes_processed;
  }

  // Returns how many bytes the measured iterations processed, or zero if the
  // benchmark does not count them.
  std::int64_t BytesProcessed() const { return bytes_processed_; }

 private:
  // Stops the clock. Called once, as the loop ends.
  void Stop() {
    elapsed_time_ = std::chrono::steady_clock::now() - start_time_;
  }

  std::size_t max_iterations_;
  std::chrono::steady_clock::time_point start_time_;
  std::chrono::nanoseconds elapsed_time_{0};
  std::int64_t bytes_processed_ = 0;
};

// Keeps the computation that produced `value` from being optimized away.
//
// A benchmark whose result is never used is free to be deleted entirely, which
// would measure nothing. Passing the result here makes it observable: the
// empty assembly block may read `value` from wherever it is and may touch
// memory, so the work that produced it has to happen.
template <typename T>
inline void DoNotOptimize(const T& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

// The base class of a benchmark.
class Benchmark {
 public:
  virtual ~Benchmark() = default;

  // Returns the name of the benchmark.
  virtual std::string_view Name() const = 0;

  // Runs the iterations of `state`.
  virtual void Run(BenchmarkState& state) = 0;
};

// Adds a new benchmark to the global suite of benchmarks.
int AddBenchmark(std::unique_ptr<Benchmark> benchmark);

// Defines a benchmark called `name`.
//
// The body is handed the run as `state` and has to iterate over it, because
// that loop is what is measured.
//
// A benchmark name has to be unique within a binary. The generated class is
// named after `name` and its `Run` has external linkage, so a repeated name
// fails the build.
#define BENCHMARK(name)                                                       \
  /* NOLINTNEXTLINE */                                                        \
  class LUCID_CONCAT(name, Benchmark) : public lucid::Benchmark {             \
   public:                                                                    \
    std::string_view Name() const final { return LUCID_STRINGIFY(name); }     \
    void Run(lucid::BenchmarkState& state) final;                             \
  };                                                                          \
  /* NOLINTNEXTLINE */                                                        \
  static auto LUCID_UNIQUE_VAR(b) =                                           \
      lucid::AddBenchmark(std::make_unique<LUCID_CONCAT(name, Benchmark)>()); \
  void LUCID_CONCAT(name, Benchmark)::Run(lucid::BenchmarkState& state)

}  // namespace lucid
