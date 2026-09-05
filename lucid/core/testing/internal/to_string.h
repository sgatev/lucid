#pragma once

#include <string>
#include <string_view>

namespace lucid::internal {

// Returns a string representation of `t`.
//
// This primary template is the fallback for types without a specialization
// below. It yields a placeholder rather than failing to compile, so that a
// value of any type can be matched; only the failure message suffers.
template <typename T>
inline std::string ToString(const T& t) {
  return "[unstringable]";
}

// Returns a string representation of `c`.
template <>
inline std::string ToString(const char& c) {
  return "'" + std::string(1, c) + "'";
}

// Returns a string representation of `s`.
template <>
inline std::string ToString(const std::string_view& s) {
  return std::string(s);
}

// Returns a string representation of `i`.
template <>
inline std::string ToString(const int& i) {
  return std::to_string(i);
}

}  // namespace lucid::internal
