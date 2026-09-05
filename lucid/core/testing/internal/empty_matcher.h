#pragma once

#include <concepts>
#include <string>

namespace lucid::internal {

// A value that reports whether it holds no elements.
template <typename A>
concept Emptiable = requires(const A& a) {
  { a.empty() } -> std::convertible_to<bool>;
};

// Matches a value that holds no elements.
class EmptyMatcher {
 public:
  std::string DescribeExpected() { return "empty"; }

  template <Emptiable A>
  std::string DescribeActual(const A& actual_elements) {
    return actual_elements.empty() ? "empty" : "not empty";
  }

  template <Emptiable A>
  bool Matches(const A& actual_elements) const {
    return actual_elements.empty();
  }
};

}  // namespace lucid::internal
