#include <cstddef>
#include <cstdlib>
#include <numeric>
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

// The numbers below `size`, from the least up.
std::vector<int> Ascending(int size) {
  std::vector<int> order(size);
  std::iota(order.begin(), order.end(), 0);
  return order;
}

BENCHMARK(Push) {
  Worklist worklist(BoundedNatDomain(1000), Ascending(1000));

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
  Worklist worklist(BoundedNatDomain(1000), Ascending(1000));

  // Every element goes back in whenever the last is taken out: there are only
  // as many of them as the domain holds, far fewer than the iterations.
  for (auto _ : state) {
    if (worklist.empty()) worklist.push_all();
    DoNotOptimize(worklist.pop());
  }
}

BENCHMARK(PushPop) {
  Worklist worklist(BoundedNatDomain(1000), Ascending(1000));

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
