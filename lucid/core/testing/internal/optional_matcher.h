#pragma once

#include <concepts>
#include <string>
#include <utility>

#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// A value that may hold a value of another type.
template <typename A>
concept OptionalLike = requires(const A& a) {
  { a.has_value() } -> std::convertible_to<bool>;
  { *a };
};

// The type of the value held by `A`.
template <OptionalLike A>
using OptionalValueType = decltype(*std::declval<const A&>());

// An actual optional whose contained value `M` accepts.
template <typename A, typename M>
concept OptionalMatchableBy =
    OptionalLike<A> and MatcherFor<M, OptionalValueType<A>>;

// Matches an optional that holds a value accepted by `value_matcher`.
//
// An empty optional never matches, whatever `value_matcher` is.
template <Matcher M>
class OptionalMatcher {
 public:
  explicit OptionalMatcher(M value_matcher) : value_matcher_(value_matcher) {}

  std::string DescribeExpected() {
    return "contain a value " + value_matcher_.DescribeExpected();
  }

  template <OptionalMatchableBy<M> A>
  std::string DescribeActual(const A& actual_value) {
    if (!actual_value.has_value()) return "nullopt";
    return "optional " + value_matcher_.DescribeActual(*actual_value);
  }

  template <OptionalMatchableBy<M> A>
  bool Matches(const A& actual_value) const {
    if (!actual_value.has_value()) return false;
    return value_matcher_.Matches(*actual_value);
  }

 private:
  M value_matcher_;
};

}  // namespace lucid::internal
