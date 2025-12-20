#include <cstdlib>
#include <vector>

#include "benchmark/benchmark.h"
#include "lucid/hash_map.h"

static void BM_InsertUnique(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Insert(i++, 0);
}
BENCHMARK(BM_InsertUnique);

static void BM_InsertDuplicate(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(i, 0);
  for (int i = 0; auto _ : state) map.Insert(i++ % kCount, 0);
}
BENCHMARK(BM_InsertDuplicate);

static void BM_FindPresent(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(i, 0);
  for (int i = 0; auto _ : state) {
    benchmark::DoNotOptimize(map.Find(i++ % kCount));
  }
}
BENCHMARK(BM_FindPresent);

static void BM_FindMissing(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(i, 0);
  for (int i = kCount; auto _ : state) benchmark::DoNotOptimize(map.Find(i++));
}
BENCHMARK(BM_FindMissing);

static void BM_Random(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(rand(), rand());
  std::vector<int> reads;
  for (int i = 0; i < kCount; ++i) reads.push_back(rand());
  int i = 0;
  for (auto _ : state) benchmark::DoNotOptimize(map.Find(reads[i++ % kCount]));
}
BENCHMARK(BM_Random);

BENCHMARK_MAIN();
