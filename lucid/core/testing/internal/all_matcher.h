#pragma once

#include <string>
#include <tuple>

#include "lucid/core/meta/static_for.h"

namespace lucid::internal {

template <typename... Ms>
class AllMatcher {
 public:
  explicit AllMatcher(Ms... matchers)
      : matchers_(std::make_tuple(matchers...)) {}

  std::string DescribeExpected() { return ""; }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return "";
  }

  template <typename A>
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
