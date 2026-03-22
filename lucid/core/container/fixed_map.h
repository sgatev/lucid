#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <utility>

namespace lucid {

// A fixed map from objects of type `KeyT` to objects of type `ValueT` that
// contains exaclty `size` pairs.
template <typename KeyT, typename ValueT, std::size_t kSize>
class FixedMap {
 public:
  // Constructs a map with the given `entries` and a `missing` object. The
  // `missing` object is returned on lookups with keys that are not present in
  // `entries`.
  constexpr FixedMap(std::array<std::pair<KeyT, ValueT>, kSize> entries,
                     ValueT missing)
      : entries_(std::move(entries)), missing_(std::move(missing)) {}

  // Returns the value that is mapped to `key`, if present, or the missing
  // object provided on construction of the map.
  [[nodiscard]] constexpr ValueT operator[](const KeyT& key) const {
    const auto it =
        std::find_if(std::begin(entries_), std::end(entries_),
                     [&key](const auto& p) { return p.first == key; });
    if (it == std::end(entries_)) [[unlikely]] {
      return missing_;
    }
    return it->second;
  }

 private:
  std::array<std::pair<KeyT, ValueT>, kSize> entries_;
  ValueT missing_;
};

}  // namespace lucid
