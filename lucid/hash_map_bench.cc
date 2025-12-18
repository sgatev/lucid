#include "benchmark/benchmark.h"
#include "lucid/hash_map.h"

static void BM_InsertUnique(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Insert(i++, 0);
}
BENCHMARK(BM_InsertUnique);

static void BM_InsertDuplicate(benchmark::State& state) {
  static constexpr int kItemsCount = 100'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kItemsCount; ++i) map.Insert(i, 0);
  int i = 0;
  for (auto _ : state) map.Insert(i++ % kItemsCount, 0);
}
BENCHMARK(BM_InsertDuplicate);

static void BM_FindPresent(benchmark::State& state) {
  static constexpr int kItemsCount = 100'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kItemsCount; ++i) map.Insert(i, 0);
  for (int i = 0; auto _ : state) {
    benchmark::DoNotOptimize(map.Find(i++ % kItemsCount));
  }
}
BENCHMARK(BM_FindPresent);

static void BM_FindMissing(benchmark::State& state) {
  static constexpr int kItemsCount = 100'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kItemsCount; ++i) map.Insert(i, 0);
  int i = kItemsCount;
  for (auto _ : state) benchmark::DoNotOptimize(map.Find(i++));
}
BENCHMARK(BM_FindMissing);

BENCHMARK_MAIN();
