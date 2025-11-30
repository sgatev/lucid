#include <cstdlib>
#include <functional>
#include <vector>

#include "benchmark/benchmark.h"
#include "lucid/worklist.h"

static void BM_Push(benchmark::State &state) {
  lucid::Worklist<int, std::less<int>> worklist(std::less<int>{});

  std::vector<int> inputs;
  for (int i = 0; i < state.max_iterations; ++i) inputs.push_back(std::rand());

  int i = 0;
  for (auto _ : state) worklist.push(inputs[i++]);
  benchmark::DoNotOptimize(worklist.empty());
}
BENCHMARK(BM_Push)->Range(2 << 2, 2 << 24);

static void BM_Pop(benchmark::State &state) {
  lucid::Worklist<int, std::less<int>> worklist(std::less<int>{});

  for (int i = 0; i < state.max_iterations; ++i) worklist.push(std::rand());

  for (auto _ : state) worklist.pop();
  benchmark::DoNotOptimize(worklist.empty());
}
BENCHMARK(BM_Pop)->Range(2 << 2, 2 << 24);

BENCHMARK_MAIN();
