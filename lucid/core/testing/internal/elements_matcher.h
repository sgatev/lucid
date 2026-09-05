#pragma once

#include <iterator>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "lucid/core/meta/static_for.h"
#include "lucid/core/string/concat.h"
#include "lucid/core/testing/internal/matcher.h"

namespace lucid::internal {

// Matches a range whose elements are accepted by `element_matchers`, one
// matcher per element, in order.
//
// A range of any other length never matches, so the matcher count fixes the
// expected size.
template <Matcher... Ms>
class ElementsMatcher {
 public:
  ElementsMatcher(Ms... element_matchers)
      : element_matchers_(std::make_tuple(element_matchers...)) {}

  std::string DescribeExpected() {
    std::vector<std::string> parts;
    parts.reserve(sizeof...(Ms) + 2);
    parts.push_back("have elements { ");
    bool has_added_element = false;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      if (has_added_element) parts.push_back(", ");
      parts.push_back(std::get<I>(element_matchers_).DescribeExpected());
      has_added_element = true;
    });
    parts.push_back(" }");

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  template <ElementsMatchableBy<Ms...> A>
  std::string DescribeActual(const A& actual_elements) {
    std::vector<std::string> parts;
    parts.reserve(actual_elements.size() + 2);
    parts.push_back("{ ");
    auto it = std::begin(actual_elements);
    bool has_added_element = false;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      if (has_added_element) parts.push_back(", ");
      parts.push_back(std::get<I>(element_matchers_).DescribeActual(*it));
      ++it;
      has_added_element = true;
    });
    parts.push_back(" }");

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  template <ElementsMatchableBy<Ms...> A>
  bool Matches(const A& actual_elements) const {
    if (actual_elements.size() != sizeof...(Ms)) return false;
    auto it = std::begin(actual_elements);
    bool equal = true;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      equal = equal && std::get<I>(element_matchers_).Matches(*it);
      ++it;
    });
    return equal;
  }

 private:
  std::tuple<Ms...> element_matchers_;
};

}  // namespace lucid::internal
