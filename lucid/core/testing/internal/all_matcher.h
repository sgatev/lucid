#pragma once

#include <string>
#include <tuple>

#include "lucid/core/meta/static_for.h"
#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// Matches a value that every one of `matchers` accepts.
template <Matcher... Ms>
class AllMatcher {
 public:
  explicit AllMatcher(Ms... matchers)
      : matchers_(std::make_tuple(matchers...)) {}

  // Returns an empty description.
  //
  // TODO: combine the descriptions of `matchers_`. Until then a failure
  // reports which value was rejected but not what was wanted of it.
  std::string DescribeExpected() { return ""; }

  // Returns an empty description. See `DescribeExpected`.
  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return "";
  }

  template <typename A>
    requires(MatcherFor<Ms, A> and ...)
  bool Matches(const A& actual_value) const {
    bool equal = true;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      equal = equal && std::get<I>(matchers_).Matches(actual_value);
    });
    return equal;
  }

 private:
  std::tuple<Ms...> matchers_;
};

}  // namespace lucid::internal
