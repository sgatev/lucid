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

// Returns the two's complement of `n` interpreted as 9-bit signed integer.
//
// Requires:
// - `n` must be in the range [0, 512).
constexpr std::uint16_t TwosComplement9(std::uint16_t n) {
  assert((n & 0b1111111000000000) == 0);
  return (~n + 1) & 0b0111111111;
}

}  // namespace lucid
