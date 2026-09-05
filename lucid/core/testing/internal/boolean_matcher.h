#pragma once

#include <string>

namespace lucid::internal {

// Matches a boolean equal to `expected_value`.
//
// Unlike most matchers this one is not generic over the actual value type: it
// takes a `bool`, so any value implicitly convertible to one is accepted.
class BooleanMatcher {
 public:
  explicit BooleanMatcher(bool expected_value)
      : expected_value_(expected_value) {}

  std::string DescribeExpected() { return expected_value_ ? "true" : "false"; }

  std::string DescribeActual(bool actual_value) {
    return actual_value ? "true" : "false";
  }

  bool Matches(bool actual_value) const {
    return actual_value == expected_value_;
  }

 private:
  bool expected_value_;
};

}  // namespace lucid::internal
