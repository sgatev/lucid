#include <cstdlib>
#include <vector>

#include "benchmark/benchmark.h"
#include "lucid/hash_set.h"

static void BM_InsertUnique(benchmark::State& state) {
  lucid::HashSet<int> set;
  for (int i = 0; auto _ : state) set.Insert(i++);
}
BENCHMARK(BM_InsertUnique);

static void BM_InsertDuplicate(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashSet<int> set;
  for (int i = 0; i < kCount; ++i) set.Insert(i);
  for (int i = 0; auto _ : state) set.Insert(i++ % kCount);
}
BENCHMARK(BM_InsertDuplicate);

static void BM_FindPresent(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashSet<int> set;
  for (int i = 0; i < kCount; ++i) set.Insert(i);
  for (int i = 0; auto _ : state) {
    benchmark::DoNotOptimize(set.Contains(i++ % kCount));
  }
}
BENCHMARK(BM_FindPresent);

static void BM_FindMissing(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashSet<int> set;
  for (int i = 0; i < kCount; ++i) set.Insert(i);
  for (int i = kCount; auto _ : state) {
    benchmark::DoNotOptimize(set.Contains(i++));
  }
}
BENCHMARK(BM_FindMissing);

static void BM_RemoveMissing(benchmark::State& state) {
  lucid::HashSet<int> set;
  for (int i = 0; auto _ : state) set.Remove(i++);
}
BENCHMARK(BM_RemoveMissing);

static void BM_Random(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashSet<int> set;
  for (int i = 0; i < kCount; ++i) set.Insert(rand());
  std::vector<int> reads;
  for (int i = 0; i < kCount; ++i) reads.push_back(rand());
  int i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(set.Contains(reads[i++ % kCount]));
  }
}
BENCHMARK(BM_Random);

BENCHMARK_MAIN();
