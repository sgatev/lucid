#pragma once

#include <string>

namespace lucid::internal {

template <typename FM, typename SM>
class PairMatcher {
 public:
  explicit PairMatcher(FM first_matcher, SM second_matcher)
      : first_matcher_(first_matcher), second_matcher_(second_matcher) {}

  std::string DescribeExpected() {
    return "contain a pair with key " + first_matcher_.DescribeExpected() +
           " and value " + second_matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return "pair with key " +
           first_matcher_.DescribeActual(actual_value.first) + " and value " +
           second_matcher_.DescribeActual(actual_value.second);
  }

  template <typename A>
  bool Matches(const A& actual_value) const {
    return first_matcher_.Matches(actual_value.first) &&
           second_matcher_.Matches(actual_value.second);
  }

 private:
  FM first_matcher_;
  SM second_matcher_;
};

}  // namespace lucid::internal
