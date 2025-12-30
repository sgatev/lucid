#pragma once

#include <utility>

namespace lucid {

// Returns the given argument unchanged.
template <typename T>
constexpr T&& identity(T&& i) noexcept {
  return std::forward<T>(i);
}

}  // namespace lucid
