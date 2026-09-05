#pragma once

#include <string>

namespace lucid::internal {

template <typename M>
class NotMatcher {
 public:
  explicit NotMatcher(M matcher) : matcher_(matcher) {}

  std::string DescribeExpected() {
    return "not " + matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return matcher_.DescribeActual(actual_value);
  }

  template <typename A>
  bool Matches(const A& actual_value) const {
    return !matcher_.Matches(actual_value);
  }

 private:
  M matcher_;
};

}  // namespace lucid::internal
