#pragma once

#include <cstddef>
#include <string>

namespace lucid::internal {

class SizeMatcher {
 public:
  SizeMatcher(std::size_t expected_size) : expected_size_(expected_size) {}

  std::string DescribeExpected() {
    return "size is " + std::to_string(expected_size_);
  }

  template <typename A>
  std::string DescribeActual(const A& actual_elements) {
    return std::to_string(actual_elements.size());
  }

  template <typename A>
  bool Matches(const A& actual_elements) const {
    return actual_elements.size() == expected_size_;
  }

 private:
  std::size_t expected_size_;
};

}  // namespace lucid::internal
