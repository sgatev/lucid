#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace lucid {

inline std::size_t Hash(std::uint32_t v) {
  // Low bias 32-bit hash function discovered by
  // https://github.com/skeeto/hash-prospector.
  v ^= v >> 16;
  v *= 0x7feb352d;
  v ^= v >> 15;
  v *= 0x846ca68b;
  v ^= v >> 16;
  return v;
}

inline std::size_t Hash(std::string_view v) {
  std::uint32_t h = Hash(static_cast<std::uint32_t>(v.size()));
  const char* pos = v.data();
  std::size_t remaining = v.size();
  for (; remaining >= 4; remaining -= 4, pos += 4) {
    std::uint32_t word;
    std::memcpy(&word, pos, sizeof(word));
    h = Hash(h ^ word);
  }

  std::uint32_t tail = 0;
  for (std::size_t i = 0; i < remaining; ++i) {
    tail = (tail << 8) | static_cast<std::uint8_t>(pos[i]);
  }
  return Hash(h ^ tail);
}

// Returns a combination of the given hashes.
//
// Each hash is mixed in before the next one is folded on top of it, rather
// than all of them being folded together with XOR alone. XOR would give the
// same combination whichever order the hashes arrived in, and would drop any
// two of them that matched.
template <typename... Hs>
inline std::size_t HashCombine(Hs... hs) {
  std::size_t combined = 0;
  ((combined = Hash(static_cast<std::uint32_t>(combined ^ hs))), ...);
  return combined;
}

}  // namespace lucid
