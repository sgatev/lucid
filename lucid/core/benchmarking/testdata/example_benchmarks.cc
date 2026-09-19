#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "lucid/core/benchmarking/benchmarking.h"

namespace lucid {
namespace {

BENCHMARK(Nothing) {
  for (auto _ : state) {
  }
}

BENCHMARK(Sum) {
  std::vector<int> values;
  values.reserve(state.MaxIterations());
  for (std::size_t i = 0; i < state.MaxIterations(); ++i) {
    values.push_back(static_cast<int>(i));
  }

  int sum = 0;
  for (std::size_t i = 0; auto _ : state) sum += values[i++];
  DoNotOptimize(sum);
}

// A benchmark that reports what it got through, rather than only how long it
// took.
BENCHMARK(Copy) {
  const std::string source(1024, 'x');
  std::string destination;

  for (auto _ : state) {
    destination = source;
    DoNotOptimize(destination);
  }

  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(source.size()));
}

}  // namespace
}  // namespace lucid
