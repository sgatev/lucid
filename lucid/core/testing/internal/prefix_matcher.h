#pragma once

#include <string>
#include <string_view>

#include "lucid/core/testing/internal/to_string.h"

namespace lucid::internal {

// Matches a string that starts with `expected_prefix`.
class PrefixMatcher {
 public:
  explicit PrefixMatcher(std::string_view expected_prefix)
      : expected_prefix_(expected_prefix) {}

  std::string DescribeExpected() {
    return "a string starting with " +
           ToString(std::string_view(expected_prefix_));
  }

  template <StringLike A>
  std::string DescribeActual(const A& actual_value) {
    return ToString(std::string_view(actual_value));
  }

  template <StringLike A>
  bool Matches(const A& actual_value) const {
    return std::string_view(actual_value).starts_with(expected_prefix_);
  }

 private:
  std::string expected_prefix_;
};

}  // namespace lucid::internal
