#pragma once

#include <string>

#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// Matches a value that `matcher` rejects.
template <Matcher M>
class NotMatcher {
 public:
  explicit NotMatcher(M matcher) : matcher_(matcher) {}

  std::string DescribeExpected() {
    return "not " + matcher_.DescribeExpected();
  }

  template <MatchableBy<M> A>
  std::string DescribeActual(const A& actual_value) {
    return matcher_.DescribeActual(actual_value);
  }

  template <MatchableBy<M> A>
  bool Matches(const A& actual_value) const {
    return !matcher_.Matches(actual_value);
  }

 private:
  M matcher_;
};

}  // namespace lucid::internal
