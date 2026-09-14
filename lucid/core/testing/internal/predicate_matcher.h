#pragma once

#include <concepts>
#include <functional>
#include <string>

#include "lucid/core/testing/internal/to_string.h"

namespace lucid::internal {

// Matches a value accepted by `predicate`.
//
// The predicate is opaque, so a failure reports the value it rejected but not
// which property of that value it objected to. Prefer a more specific matcher
// where one exists.
template <typename P>
class PredicateMatcher {
 public:
  explicit PredicateMatcher(P predicate) : predicate_(predicate) {}

  std::string DescribeExpected() { return "accepted by the predicate"; }

  template <typename A>
  std::string DescribeActual(A&& a) {
    return ToString(a);
  }

  template <typename A>
    requires std::predicate<const P&, A&>
  bool Matches(A&& a) const {
    return std::invoke(predicate_, a);
  }

 private:
  P predicate_;
};

}  // namespace lucid::internal
