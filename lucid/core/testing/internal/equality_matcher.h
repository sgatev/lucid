#pragma once

#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

#include "lucid/core/string/concat.h"

namespace lucid::internal {

// Returns a string representation of `s`.
template <typename T>
inline std::string ToString(const T& t) {
  return "[unstringable]";
}

// Returns a string representation of `s`.
template <>
inline std::string ToString(const char& c) {
  return "'" + std::string(1, c) + "'";
}

// Returns a string representation of `s`.
template <>
inline std::string ToString(const std::string_view& s) {
  return std::string(s);
}

// Returns a string representation of `s`.
template <>
inline std::string ToString(const int& s) {
  return std::to_string(s);
}

template <typename E>
class EqualityMatcher {
 public:
  explicit EqualityMatcher(E expected_value, bool expect_equals)
      : expected_value_(std::forward<E>(expected_value)),
        expect_equals_(expect_equals) {}

  std::string DescribeExpected() { return DescribeValue(expected_value_); }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return DescribeValue(actual_value);
  }

  template <typename A>
  bool Matches(const A& actual_value) {
    return (actual_value == expected_value_) == expect_equals_;
  }

 private:
  template <typename V>
  std::string DescribeValue(const V& value) {
    std::string qualifier = expect_equals_ ? "equal" : "not equal";
    std::string stringified_value = ToString(value);
    return Concat(std::initializer_list<std::string_view>{qualifier, "to",
                                                          stringified_value},
                  " ");
  }

  const E expected_value_;
  bool expect_equals_;
};

}  // namespace lucid::internal
