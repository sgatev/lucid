#include <cstddef>
#include <cstdlib>
#include <functional>
#include <vector>

#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/dataflow/worklist.h"

namespace lucid {
namespace {

class BoundedNatDomain {
 public:
  using element_type = int;

  explicit BoundedNatDomain(std::size_t size) : size_(size) {}

  std::size_t size() const { return size_; }

  std::size_t id(int i) const { return i; }

 private:
  std::size_t size_;
};

BENCHMARK(Push) {
  Worklist<int, BoundedNatDomain, std::less<>> worklist(BoundedNatDomain(1000),
                                                        std::less());

  std::vector<int> inputs;
  inputs.reserve(state.MaxIterations());
  for (std::size_t i = 0; i < state.MaxIterations(); ++i) {
    inputs.push_back(std::rand() % 1000);
  }

  int i = 0;
  for (auto _ : state) worklist.push(inputs[i++]);
  DoNotOptimize(worklist.empty());
}

BENCHMARK(Pop) {
  Worklist<int, BoundedNatDomain, std::less<>> worklist(BoundedNatDomain(1000),
                                                        std::less());

  for (std::size_t i = 0; i < state.MaxIterations(); ++i) {
    worklist.push(std::rand() % 1000);
  }

  for (auto _ : state) worklist.pop();
  DoNotOptimize(worklist.empty());
}

BENCHMARK(PushPop) {
  Worklist<int, BoundedNatDomain, std::less<>> worklist(BoundedNatDomain(1000),
                                                        std::less());

  std::vector<int> inputs;
  inputs.reserve(state.MaxIterations());
  for (std::size_t i = 0; i < state.MaxIterations(); ++i) {
    inputs.push_back(std::rand() % 1000);
  }

  int i = 0;
  for (auto _ : state) {
    worklist.push(inputs[i++]);
    worklist.pop();
  }
  DoNotOptimize(worklist.empty());
}

}  // namespace
}  // namespace lucid
