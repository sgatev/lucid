// Finds the number below a million whose Collatz sequence takes the most steps
// to reach 1, and returns it and its steps combined modulo 251. collatz.lu is
// the same program in Lucid, statement for statement, and the comparison in
// lucid/benchmarks/comparison.h times the two against each other.

#include <cstdint>

int main() {
  const std::int64_t limit = 1000000;

  std::int64_t best_start = 1;
  std::int64_t best_steps = 0;

  for (std::int64_t start = 1; start != limit; ++start) {
    std::int64_t n = start;
    std::int64_t steps = 0;
    while (n != 1) {
      if (n % 2 == 0) {
        n = n / 2;
      } else {
        n = 3 * n + 1;
      }
      steps = steps + 1;
    }

    if (steps > best_steps) {
      best_steps = steps;
      best_start = start;
    }
  }

  // The exit status is the lowest byte. The steps are weighed in, so that a run
  // that found the right number but counted its steps wrong comes to another.
  return static_cast<int>((best_start * 31 + best_steps) % 251);
}
