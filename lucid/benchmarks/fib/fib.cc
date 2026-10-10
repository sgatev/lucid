// Sums the Fibonacci numbers from the 25th to the 35th, each worked out by the
// recursion that calls itself twice, and returns the sum modulo 251. fib.lu is
// the same program in Lucid, statement for statement, and the comparison in
// lucid/benchmarks/comparison.h times the two against each other.

#include <cstdint>

namespace {

std::int32_t fib(std::int32_t n) {
  if (n < 2) {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}

}  // namespace

int main() {
  std::int32_t total = 0;

  // Bounded by a loop rather than written out, so that neither compiler is
  // handed a call it could work out while compiling.
  for (std::int32_t n = 25; !(n > 35); ++n) {
    total = total + fib(n);
  }

  // The exit status is the lowest byte.
  return total % 251;
}
