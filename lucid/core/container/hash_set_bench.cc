#include <cstdlib>
#include <vector>

#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {
namespace {

BENCHMARK(InsertUnique) {
  HashSet<int> set;
  for (int i = 0; auto _ : state) set.Insert(i++);
}

BENCHMARK(InsertDuplicate) {
  static constexpr int kCount = 1'000'000;
  HashSet<int> set;
  for (int i = 0; i < kCount; ++i) set.Insert(i);
  for (int i = 0; auto _ : state) set.Insert(i++ % kCount);
}

BENCHMARK(FindPresent) {
  static constexpr int kCount = 1'000'000;
  HashSet<int> set;
  for (int i = 0; i < kCount; ++i) set.Insert(i);
  for (int i = 0; auto _ : state) {
    DoNotOptimize(set.Contains(i++ % kCount));
  }
}

BENCHMARK(FindMissing) {
  static constexpr int kCount = 1'000'000;
  HashSet<int> set;
  for (int i = 0; i < kCount; ++i) set.Insert(i);
  for (int i = kCount; auto _ : state) {
    DoNotOptimize(set.Contains(i++));
  }
}

BENCHMARK(RemoveMissing) {
  HashSet<int> set;
  for (int i = 0; auto _ : state) set.Remove(i++);
}

BENCHMARK(Random) {
  static constexpr int kCount = 1'000'000;
  HashSet<int> set;
  for (int i = 0; i < kCount; ++i) set.Insert(rand());
  std::vector<int> reads;
  reads.reserve(kCount);
  for (int i = 0; i < kCount; ++i) reads.push_back(rand());
  int i = 0;
  for (auto _ : state) {
    DoNotOptimize(set.Contains(reads[i++ % kCount]));
  }
}

}  // namespace
}  // namespace lucid
