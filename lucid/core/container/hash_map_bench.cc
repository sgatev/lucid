#include <cstdlib>
#include <vector>

#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/container/hash_map.h"

namespace lucid {
namespace {

BENCHMARK(EmplaceUnique) {
  HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Emplace(i++, 0);
}

BENCHMARK(EmplaceDuplicate) {
  static constexpr int kCount = 1'000'000;
  HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Emplace(i, 0);
  for (int i = 0; auto _ : state) map.Emplace(i++ % kCount, 0);
}

BENCHMARK(InsertUnique) {
  HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Insert(i++, 0);
}

BENCHMARK(InsertDuplicate) {
  static constexpr int kCount = 1'000'000;
  HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(i, 0);
  for (int i = 0; auto _ : state) map.Insert(i++ % kCount, 0);
}

BENCHMARK(SetUnique) {
  HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Set(i++, 0);
}

BENCHMARK(SetDuplicate) {
  static constexpr int kCount = 1'000'000;
  HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Set(i, 0);
  for (int i = 0; auto _ : state) map.Set(i++ % kCount, 0);
}

BENCHMARK(FindPresent) {
  static constexpr int kCount = 1'000'000;
  HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(i, 0);
  for (int i = 0; auto _ : state) {
    DoNotOptimize(map.Get(i++ % kCount));
  }
}

BENCHMARK(FindMissing) {
  static constexpr int kCount = 1'000'000;
  HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(i, 0);
  for (int i = kCount; auto _ : state) DoNotOptimize(map.Get(i++));
}

BENCHMARK(RemoveMissing) {
  HashMap<int, int> map;
  for (int i = 0; auto _ : state) map.Remove(i++);
}

BENCHMARK(Random) {
  static constexpr int kCount = 1'000'000;
  HashMap<int, int> map;
  for (int i = 0; i < kCount; ++i) map.Insert(rand(), rand());
  std::vector<int> reads;
  reads.reserve(kCount);
  for (int i = 0; i < kCount; ++i) reads.push_back(rand());
  int i = 0;
  for (auto _ : state) DoNotOptimize(map.Get(reads[i++ % kCount]));
}

}  // namespace
}  // namespace lucid
