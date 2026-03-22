#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace lucid {

// Returns a combination of two given hashes.
template <typename... Hs>
inline std::size_t HashCombine(Hs&&... hs) {
  return (std::rotl(hs, 7) ^ ...);
}

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
  std::size_t h = Hash(0);
  const std::size_t words_end = (v.size() >> 2) << 2;
  std::size_t i = 0;
  for (; i < words_end; i += 4) {
    const std::uint32_t word =
        (v[i] << 16) | (v[i + 1] << 8) | (v[i + 2] << 4) | v[i + 3];
    h = HashCombine(h, Hash(word));
  }
  for (; i < v.size(); ++i) h = HashCombine(h, Hash(v[i]));
  return h;
}

}  // namespace lucid
