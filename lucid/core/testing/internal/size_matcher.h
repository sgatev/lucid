#pragma once

#include <cstddef>
#include <string>

#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// Matches a value that holds exactly `expected_size` elements.
class SizeMatcher {
 public:
  SizeMatcher(std::size_t expected_size) : expected_size_(expected_size) {}

  std::string DescribeExpected() {
    return "of size " + std::to_string(expected_size_);
  }

  template <Sized A>
  std::string DescribeActual(const A& actual_elements) {
    return "of size " + std::to_string(actual_elements.size());
  }

  template <Sized A>
  bool Matches(const A& actual_elements) const {
    return actual_elements.size() == expected_size_;
  }

 private:
  std::size_t expected_size_;
};

}  // namespace lucid::internal
