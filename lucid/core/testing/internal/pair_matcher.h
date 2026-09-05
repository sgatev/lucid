#pragma once

#include <string>
#include <utility>

#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// A value that holds a first and a second element.
template <typename A>
concept PairLike = requires(const A& a) {
  { a.first };
  { a.second };
};

// The type of the first element of `A`.
template <PairLike A>
using FirstType = decltype(std::declval<const A&>().first);

// The type of the second element of `A`.
template <PairLike A>
using SecondType = decltype(std::declval<const A&>().second);

// An actual pair whose elements `FM` and `SM` accept.
template <typename A, typename FM, typename SM>
concept PairMatchableBy = PairLike<A> and MatcherFor<FM, FirstType<A>> and
                          MatcherFor<SM, SecondType<A>>;

// Matches a pair whose first and second elements are accepted by
// `first_matcher` and `second_matcher` respectively.
//
// Descriptions name the two elements "key" and "value", as pairs are most
// often matched as the entries of a map.
template <Matcher FM, Matcher SM>
class PairMatcher {
 public:
  explicit PairMatcher(FM first_matcher, SM second_matcher)
      : first_matcher_(first_matcher), second_matcher_(second_matcher) {}

  std::string DescribeExpected() {
    return "a pair with key " + first_matcher_.DescribeExpected() +
           " and value " + second_matcher_.DescribeExpected();
  }

  template <PairMatchableBy<FM, SM> A>
  std::string DescribeActual(const A& actual_value) {
    return "a pair with key " +
           first_matcher_.DescribeActual(actual_value.first) + " and value " +
           second_matcher_.DescribeActual(actual_value.second);
  }

  template <PairMatchableBy<FM, SM> A>
  bool Matches(const A& actual_value) const {
    return first_matcher_.Matches(actual_value.first) &&
           second_matcher_.Matches(actual_value.second);
  }

 private:
  FM first_matcher_;
  SM second_matcher_;
};

}  // namespace lucid::internal
