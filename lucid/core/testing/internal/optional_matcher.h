#pragma once

#include <string>

namespace lucid::internal {

template <typename M>
class OptionalMatcher {
 public:
  explicit OptionalMatcher(M value_matcher) : value_matcher_(value_matcher) {}

  std::string DescribeExpected() {
    return "contain a value " + value_matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    if (!actual_value.has_value()) return "nullopt";
    return "optional " + value_matcher_.DescribeActual(actual_value);
  }

  template <typename A>
  bool Matches(const A& actual_value) const {
    if (!actual_value.has_value()) return false;
    return value_matcher_.Matches(*actual_value);
  }

 private:
  M value_matcher_;
};

}  // namespace lucid::internal
