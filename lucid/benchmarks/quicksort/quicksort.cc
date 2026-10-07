// Sorts 50 arrays of 100000 pseudo-random numbers with an iterative QuickSort,
// and returns a checksum of the sorted arrays, or 255 if one came out
// unsorted. quicksort.lu is the same program in Lucid, statement for
// statement, and quicksort_bench.cc times the two against each other.

#include <cstdint>

int main() {
  // Both live on the stack, as they do in the Lucid program.
  std::int64_t a[100000];
  const std::int32_t n = 100000;
  const std::int32_t rounds = 50;

  // Pairs of bounds still to be sorted. The smaller side of each partition is
  // sorted first and the larger one waits here, so it never holds more than
  // log2(n) pairs.
  std::int32_t stack[128];

  std::int64_t seed = 42;
  std::int64_t checksum = 0;

  for (std::int32_t round = 0; round != rounds; ++round) {
    for (std::int32_t k = 0; k != n; ++k) {
      seed = (seed * 1103515245 + 12345) % 2147483648;
      a[k] = seed % 1000000;
    }

    std::int32_t sp = 2;
    stack[0] = 0;
    stack[1] = n - 1;
    while (sp != 0) {
      sp -= 2;
      std::int32_t lo = stack[sp];
      std::int32_t hi = stack[sp + 1];

      while (lo < hi) {
        const std::int64_t pivot = a[lo + (hi - lo) / 2];
        std::int32_t i = lo - 1;
        std::int32_t j = hi + 1;
        while (true) {
          do ++i;
          while (a[i] < pivot);
          do --j;
          while (a[j] > pivot);
          if (i >= j) break;
          const std::int64_t tmp = a[i];
          a[i] = a[j];
          a[j] = tmp;
        }

        if (j - lo < hi - j) {
          stack[sp] = j + 1;
          stack[sp + 1] = hi;
          sp += 2;
          hi = j;
        } else {
          stack[sp] = lo;
          stack[sp + 1] = j;
          sp += 2;
          lo = j + 1;
        }
      }
    }

    for (std::int32_t m = 1; m != n; ++m) {
      if (a[m - 1] > a[m]) return 255;
    }

    // Weighs every 997th position, so that a sort that lost or duplicated
    // values comes to another checksum.
    for (std::int32_t p = 0; p < n; p += 997) {
      checksum = (checksum * 31 + a[p]) % 1000000007;
    }
  }

  // The exit status is the lowest byte, and 255 means unsorted.
  return static_cast<int>(checksum % 251);
}
