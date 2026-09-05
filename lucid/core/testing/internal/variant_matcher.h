#pragma once

#include <concepts>
#include <string>
#include <variant>

#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// A value that may hold an alternative of type `T`.
template <typename A, typename T>
concept VariantLike = requires(const A& a) {
  { std::get_if<T>(&a) } -> std::convertible_to<const T*>;
};

// An actual variant holding an alternative of type `T` that `M` accepts.
template <typename A, typename T, typename M>
concept VariantMatchableBy = VariantLike<A, T> and MatcherFor<M, T>;

// Matches a variant that currently holds an alternative of type `T` accepted
// by `value_matcher`.
//
// A variant holding any other alternative never matches.
template <typename T, Matcher M>
class VariantMatcher {
 public:
  explicit VariantMatcher(M value_matcher) : value_matcher_(value_matcher) {}

  std::string DescribeExpected() {
    return "contain a value " + value_matcher_.DescribeExpected();
  }

  // Returns a description of the held alternative, or an empty description if
  // the variant holds some alternative other than `T`.
  template <VariantMatchableBy<T, M> A>
  std::string DescribeActual(const A& actual_value) {
    const auto* actual = std::get_if<T>(&actual_value);
    if (actual == nullptr) return "";
    return "variant with " + value_matcher_.DescribeActual(*actual);
  }

  template <VariantMatchableBy<T, M> A>
  bool Matches(const A& actual_value) const {
    const auto* actual = std::get_if<T>(&actual_value);
    if (actual == nullptr) return false;
    return value_matcher_.Matches(*actual);
  }

 private:
  M value_matcher_;
};

}  // namespace lucid::internal
