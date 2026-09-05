#pragma once

#include <string>

namespace lucid::internal {

class EmptyMatcher {
 public:
  std::string DescribeExpected() { return "is empty"; }

  template <typename A>
  std::string DescribeActual(const A& actual_elements) {
    return actual_elements.empty() ? "is empty" : "is not empty";
  }

  template <typename A>
  bool Matches(const A& actual_elements) const {
    return actual_elements.empty();
  }
};

}  // namespace lucid::internal
