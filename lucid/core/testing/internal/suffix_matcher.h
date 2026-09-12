#pragma once

#include <string>
#include <string_view>

#include "lucid/core/testing/internal/matcher.h"
#include "lucid/core/testing/internal/to_string.h"

namespace lucid::internal {

// Matches a string that ends with `expected_suffix`.
class SuffixMatcher {
 public:
  explicit SuffixMatcher(std::string_view expected_suffix)
      : expected_suffix_(expected_suffix) {}

  std::string DescribeExpected() {
    return "a string ending with " +
           ToString(std::string_view(expected_suffix_));
  }

  template <StringLike A>
  std::string DescribeActual(const A& actual_value) {
    return ToString(std::string_view(actual_value));
  }

  template <StringLike A>
  bool Matches(const A& actual_value) const {
    return std::string_view(actual_value).ends_with(expected_suffix_);
  }

 private:
  std::string expected_suffix_;
};

}  // namespace lucid::internal
