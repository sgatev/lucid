#pragma once

#include <concepts>
#include <functional>
#include <string>

namespace lucid::internal {

// Matches a value accepted by `predicate`.
//
// The predicate is opaque, so a failure can only report that it rejected the
// value, never which property of the value it objected to. Prefer a more
// specific matcher where one exists.
template <typename P>
class PredicateMatcher {
 public:
  explicit PredicateMatcher(P predicate) : predicate_(predicate) {}

  std::string DescribeExpected() { return "accepted by predicate"; }

  // Returns a placeholder, as a bare predicate cannot describe a value.
  template <typename A>
  std::string DescribeActual(A&&) {
    return "not such";
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
