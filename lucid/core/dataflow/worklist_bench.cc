#include <cstddef>
#include <cstdlib>
#include <functional>
#include <vector>

#include "benchmark/benchmark.h"
#include "lucid/core/dataflow/worklist.h"

class BoundedNatDomain {
 public:
  using element_type = int;

  explicit BoundedNatDomain(std::size_t size) : size_(size) {}

  std::size_t size() const { return size_; }

  std::size_t id(int i) const { return i; }

 private:
  std::size_t size_;
};

static void BM_Push(benchmark::State& state) {
  lucid::Worklist<int, BoundedNatDomain, std::less<>> worklist(
      BoundedNatDomain(1000), std::less());

  std::vector<int> inputs;
  for (int i = 0; i < state.max_iterations; ++i) {
    inputs.push_back(std::rand() % 1000);
  }

  int i = 0;
  for (auto _ : state) worklist.push(inputs[i++]);
  benchmark::DoNotOptimize(worklist.empty());
}
BENCHMARK(BM_Push);

static void BM_Pop(benchmark::State& state) {
  lucid::Worklist<int, BoundedNatDomain, std::less<>> worklist(
      BoundedNatDomain(1000), std::less());

  for (int i = 0; i < state.max_iterations; ++i) {
    worklist.push(std::rand() % 1000);
  }

  for (auto _ : state) worklist.pop();
  benchmark::DoNotOptimize(worklist.empty());
}
BENCHMARK(BM_Pop);

static void BM_PushPop(benchmark::State& state) {
  lucid::Worklist<int, BoundedNatDomain, std::less<>> worklist(
      BoundedNatDomain(1000), std::less());

  std::vector<int> inputs;
  for (int i = 0; i < state.max_iterations; ++i) {
    inputs.push_back(std::rand() % 1000);
  }

  int i = 0;
  for (auto _ : state) {
    worklist.push(inputs[i++]);
    worklist.pop();
  }
  benchmark::DoNotOptimize(worklist.empty());
}
BENCHMARK(BM_PushPop);

BENCHMARK_MAIN();
