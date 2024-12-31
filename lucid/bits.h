#pragma once

#include <cassert>
#include <cstdint>

namespace lucid {

// Returns the two's complement of `n` interpreted as 7-bit signed integer.
//
// Requires:
// - `n` must be in the range [0, 128).
constexpr std::uint8_t TwosComplement7(std::uint8_t n) {
  assert((n & 0b10000000) == 0);
  return (~n + 1) & 0b01111111;
}

}  // namespace lucid
