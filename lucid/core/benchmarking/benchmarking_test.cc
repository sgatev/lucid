#include "lucid/core/benchmarking/benchmarking.h"

#include <chrono>
#include <cstddef>
#include <string>
#include <thread>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

// Long enough to measure with room to spare, short enough not to slow the
// suite down. The assertions below leave a wide margin either side of it,
// because a loaded machine can stretch a sleep but never shorten it.
constexpr std::chrono::milliseconds kSleepTime(50);

TEST(Test, BenchmarkStateRunsMaxIterations) {
  BenchmarkState state(7);

  std::size_t iterations = 0;
  for (auto _ : state) ++iterations;

  EXPECT_EQ(state.MaxIterations(), 7u);
  EXPECT_EQ(iterations, 7u);
}

TEST(Test, BenchmarkStateRunsNoIterations) {
  BenchmarkState state(0);

  std::size_t iterations = 0;
  for (auto _ : state) ++iterations;

  EXPECT_EQ(iterations, 0u);
}

TEST(Test, BenchmarkStateMeasuresNothingBeforeItIsIteratedOver) {
  BenchmarkState state(1);

  EXPECT_EQ(state.ElapsedTime(), std::chrono::nanoseconds(0));
}

TEST(Test, BenchmarkStateMeasuresTheLoop) {
  BenchmarkState state(2);

  for (auto _ : state) std::this_thread::sleep_for(kSleepTime);

  EXPECT_TRUE(state.ElapsedTime() >= 2 * kSleepTime);
}

TEST(Test, BenchmarkStateMeasuresNeitherSetupNorTeardown) {
  BenchmarkState state(1);

  std::this_thread::sleep_for(kSleepTime);
  for (auto _ : state) {
  }
  std::this_thread::sleep_for(kSleepTime);

  // Whatever the loop itself cost, it is nowhere near the sleeping either side
  // of it.
  EXPECT_TRUE(state.ElapsedTime() < kSleepTime);
}

TEST(Test, CountLinesSkipsLinesWithNothingOnThem) {
  EXPECT_EQ(CountLines(""), 0);
  EXPECT_EQ(CountLines("\n\n"), 0);
  EXPECT_EQ(CountLines(" \t \r\n"), 0);
  EXPECT_EQ(CountLines("a\n\nb\n"), 2);
  EXPECT_EQ(CountLines("  # a comment\n"), 1);
}

TEST(Test, CountLinesCountsALastLineWithoutANewline) {
  EXPECT_EQ(CountLines("a\nb"), 2);
  // The sources the compiler reads end in a NUL, which is not code.
  EXPECT_EQ(CountLines(std::string("a\n\0", 3)), 1);
}

TEST(Test, DoNotOptimizeAcceptsValuesOfAnySize) {
  bool small = true;
  std::string large = "a string that does not fit in a register";
  std::vector<int> container = {1, 2, 3};

  DoNotOptimize(small);
  DoNotOptimize(large);
  DoNotOptimize(container);
  DoNotOptimize(container.empty());
}

}  // namespace
}  // namespace lucid
