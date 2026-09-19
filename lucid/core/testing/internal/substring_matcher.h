#pragma once

#include <string>
#include <string_view>

#include "lucid/core/testing/internal/to_string.h"

namespace lucid::internal {

// Matches a string that contains `expected_substring`.
class SubstringMatcher {
 public:
  explicit SubstringMatcher(std::string_view expected_substring)
      : expected_substring_(expected_substring) {}

  std::string DescribeExpected() {
    return "a string containing " +
           ToString(std::string_view(expected_substring_));
  }

  template <StringLike A>
  std::string DescribeActual(const A& actual_value) {
    return ToString(std::string_view(actual_value));
  }

  template <StringLike A>
  bool Matches(const A& actual_value) const {
    return std::string_view(actual_value).contains(expected_substring_);
  }

 private:
  std::string expected_substring_;
};

}  // namespace lucid::internal
