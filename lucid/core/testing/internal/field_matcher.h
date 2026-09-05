#pragma once

#include <functional>
#include <string>

namespace lucid::internal {

template <typename F, typename M>
class FieldMatcher {
 public:
  explicit FieldMatcher(F field, M field_matcher)
      : field_(field), field_matcher_(field_matcher) {}

  std::string DescribeExpected() {
    return "with a field that " + field_matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(A&& actual_value) {
    return "with a field that " +
           field_matcher_.DescribeActual(std::invoke(field_, actual_value));
  }

  template <typename A>
  bool Matches(A&& actual_value) const {
    return field_matcher_.Matches(std::invoke(field_, actual_value));
  }

 private:
  F field_;
  M field_matcher_;
};

}  // namespace lucid::internal
