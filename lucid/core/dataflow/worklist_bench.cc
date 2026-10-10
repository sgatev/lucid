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

// How many random inputs a benchmark cycles through. A power of two, so that
// wrapping around is a mask, and few enough that preparing them costs nothing
// next to the iterations. Preparing one per iteration instead made the inputs
// take far longer than the loop that reads them: the fastest benchmark here
// settles on hundreds of millions of iterations.
constexpr std::size_t kInputCount = 1 << 16;

// Elements of a domain of 1000, in no order.
std::vector<int> RandomInputs() {
  std::vector<int> inputs;
  inputs.reserve(kInputCount);
  for (std::size_t i = 0; i < kInputCount; ++i) {
    inputs.push_back(std::rand() % 1000);
  }
  return inputs;
}

BENCHMARK(Push) {
  Worklist worklist(BoundedNatDomain(1000), Ascending(1000));

  const std::vector<int> inputs = RandomInputs();

  std::size_t i = 0;
  for (auto _ : state) worklist.push(inputs[i++ & (kInputCount - 1)]);
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

  const std::vector<int> inputs = RandomInputs();

  std::size_t i = 0;
  for (auto _ : state) {
    worklist.push(inputs[i++ & (kInputCount - 1)]);
    worklist.pop();
  }
  DoNotOptimize(worklist.empty());
}

}  // namespace
}  // namespace lucid
