#pragma once

#include <functional>
#include <string>

namespace lucid::internal {

template <typename P>
class PredicateMatcher {
 public:
  explicit PredicateMatcher(P predicate) : predicate_(predicate) {}

  std::string DescribeExpected() { return "accepted by predicate"; }

  template <typename A>
  std::string DescribeActual(A&&) {
    return "not such";
  }

  template <typename A>
  bool Matches(A&& a) {
    return std::invoke(predicate_, a);
  }

 private:
  P predicate_;
};

}  // namespace lucid::internal
