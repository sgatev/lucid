#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "lucid/core/meta/static_for.h"
#include "lucid/core/string/concat.h"
#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// An actual value that every one of the matchers `Ms` accepts.
template <typename A, typename... Ms>
concept MatchableByAll = (MatcherFor<Ms, A> and ...);

// Matches a value that every one of `matchers` accepts.
template <Matcher... Ms>
class AllMatcher {
 public:
  explicit AllMatcher(Ms... matchers)
      : matchers_(std::make_tuple(matchers...)) {}

  std::string DescribeExpected() {
    std::vector<std::string> parts;
    parts.reserve(sizeof...(Ms) * 2);
    bool has_added_matcher = false;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      if (has_added_matcher) parts.push_back(" and ");
      parts.push_back(std::get<I>(matchers_).DescribeExpected());
      has_added_matcher = true;
    });

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  // Describes the value once per matcher. The descriptions overlap, but each
  // is phrased in the terms of the matcher that rejected it, which is what a
  // reader needs in order to see which conjunct failed.
  template <MatchableByAll<Ms...> A>
  std::string DescribeActual(const A& actual_value) {
    std::vector<std::string> parts;
    parts.reserve(sizeof...(Ms) * 2);
    bool has_added_matcher = false;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      if (has_added_matcher) parts.push_back(" and ");
      parts.push_back(std::get<I>(matchers_).DescribeActual(actual_value));
      has_added_matcher = true;
    });

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  template <MatchableByAll<Ms...> A>
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
