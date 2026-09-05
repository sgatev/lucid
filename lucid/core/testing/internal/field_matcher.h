#pragma once

#include <concepts>
#include <functional>
#include <string>

#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// An actual value with a field, read by `F`, that matcher `M` accepts.
template <typename A, typename F, typename M>
concept FieldMatchableBy = std::invocable<const F&, A&> and
                           MatcherFor<M, std::invoke_result_t<const F&, A&>>;

// Matches a value with a field, read by `field`, that `field_matcher`
// accepts.
//
// `field` is anything `std::invoke` can call on the actual value, most often a
// pointer to a data member, but equally a getter or a lambda.
template <typename F, Matcher M>
class FieldMatcher {
 public:
  explicit FieldMatcher(F field, M field_matcher)
      : field_(field), field_matcher_(field_matcher) {}

  std::string DescribeExpected() {
    return "with a field " + field_matcher_.DescribeExpected();
  }

  template <FieldMatchableBy<F, M> A>
  std::string DescribeActual(A&& actual_value) {
    return "with a field " +
           field_matcher_.DescribeActual(std::invoke(field_, actual_value));
  }

  template <FieldMatchableBy<F, M> A>
  bool Matches(A&& actual_value) const {
    return field_matcher_.Matches(std::invoke(field_, actual_value));
  }

 private:
  F field_;
  M field_matcher_;
};

}  // namespace lucid::internal
