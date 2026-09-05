#pragma once

#include <concepts>
#include <string>
#include <utility>

#include "lucid/core/testing/internal/to_string.h"

namespace lucid::internal {

// A value that can be compared for equality with a value of type `E`.
//
// This is deliberately weaker than `std::equality_comparable_with`, which also
// requires a common reference type that the compared values convert to.
template <typename A, typename E>
concept ComparableTo = requires(const A& a, const E& e) {
  { a == e } -> std::convertible_to<bool>;
};

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

  std::string DescribeExpected() {
    return (expect_equals_ ? "equal to " : "not equal to ") +
           ToString(expected_value_);
  }

  // Returns just the value. The comparison belongs to the expectation, so a
  // failure reads "to be equal to 42 but was found 21".
  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return ToString(actual_value);
  }

  template <ComparableTo<E> A>
  bool Matches(const A& actual_value) const {
    return (actual_value == expected_value_) == expect_equals_;
  }

 private:
  const E expected_value_;
  bool expect_equals_;
};

}  // namespace lucid::internal
