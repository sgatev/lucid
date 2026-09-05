#pragma once

#include <concepts>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

#include "lucid/core/string/concat.h"

namespace lucid::internal {

// A value that can be compared for equality with a value of type `E`.
//
// This is deliberately weaker than `std::equality_comparable_with`, which also
// requires a common reference type that the compared values convert to.
template <typename A, typename E>
concept ComparableTo = requires(const A& a, const E& e) {
  { a == e } -> std::convertible_to<bool>;
};

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

// Matches a value equal to `expected_value`, or, when `expect_equals` is
// false, a value that differs from it.
//
// One class covers both senses so that they share their description logic;
// `Equals` and `NotEquals` are the entry points.
template <typename E>
class EqualityMatcher {
 public:
  // Constructs a matcher of values equal to `expected_value`, or, if
  // `expect_equals` is false, of values that differ from it.
  explicit EqualityMatcher(E expected_value, bool expect_equals)
      : expected_value_(std::forward<E>(expected_value)),
        expect_equals_(expect_equals) {}

  std::string DescribeExpected() { return DescribeValue(expected_value_); }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return DescribeValue(actual_value);
  }

  template <ComparableTo<E> A>
  bool Matches(const A& actual_value) const {
    return (actual_value == expected_value_) == expect_equals_;
  }

 private:
  // Returns a description of `value` qualified by the sense of the match, so
  // that expected and actual values read alike in a failure message.
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
