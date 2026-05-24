#include <cstdlib>
#include <vector>

#include "benchmark/benchmark.h"
#include "lucid/core/container/hash_map.h"

static void BM_EmplaceUnique(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Emplace(i++, 0);
}
BENCHMARK(BM_EmplaceUnique);

static void BM_EmplaceDuplicate(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Emplace(i, 0);
  for (int i = 0; auto _ : state) map.Emplace(i++ % kCount, 0);
}
BENCHMARK(BM_EmplaceDuplicate);

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

static void BM_SetUnique(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Set(i++, 0);
}
BENCHMARK(BM_SetUnique);

static void BM_SetDuplicate(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Set(i, 0);
  for (int i = 0; auto _ : state) map.Set(i++ % kCount, 0);
}
BENCHMARK(BM_SetDuplicate);

static void BM_FindPresent(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(i, 0);
  for (int i = 0; auto _ : state) {
    benchmark::DoNotOptimize(map.Get(i++ % kCount));
  }
}
BENCHMARK(BM_FindPresent);

static void BM_FindMissing(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(i, 0);
  for (int i = kCount; auto _ : state) benchmark::DoNotOptimize(map.Get(i++));
}
BENCHMARK(BM_FindMissing);

static void BM_RemoveMissing(benchmark::State& state) {
  lucid::HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Remove(i++);
}
BENCHMARK(BM_RemoveMissing);

static void BM_Random(benchmark::State& state) {
  static constexpr int kCount = 1'000'000;
  lucid::HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(rand(), rand());
  std::vector<int> reads;
  for (int i = 0; i < kCount; ++i) reads.push_back(rand());
  int i = 0;
  for (auto _ : state) benchmark::DoNotOptimize(map.Get(reads[i++ % kCount]));
}
BENCHMARK(BM_Random);

BENCHMARK_MAIN();
