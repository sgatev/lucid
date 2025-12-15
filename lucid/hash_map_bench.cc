#include "benchmark/benchmark.h"
#include "lucid/hash_map.h"

static void BM_InsertUnique(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Insert(i++, 0);
}
BENCHMARK(BM_InsertUnique);

static void BM_InsertDuplicate(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  map.Insert(0, 0);
  for (auto _ : state) map.Insert(0, 0);
}
BENCHMARK(BM_InsertDuplicate);

static void BM_FindPresent(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  map.Insert(0, 0);
  for (auto _ : state) benchmark::DoNotOptimize(map.Find(0));
}
BENCHMARK(BM_FindPresent);

static void BM_FindMissing(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  for (auto _ : state) benchmark::DoNotOptimize(map.Find(0));
}
BENCHMARK(BM_FindMissing);

static void BM_FindMany(benchmark::State& state) {
  static constexpr int kItemsCount = 100'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kItemsCount; ++i) map.Insert(i, 0);
  for (auto _ : state) {
    for (int i = 0; i < kItemsCount; ++i) benchmark::DoNotOptimize(map.Find(i));
  }
  state.SetItemsProcessed(kItemsCount);
}
BENCHMARK(BM_FindMany);

BENCHMARK_MAIN();
