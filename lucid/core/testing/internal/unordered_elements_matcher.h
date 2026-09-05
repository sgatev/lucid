#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "lucid/core/meta/static_for.h"
#include "lucid/core/string/concat.h"
#include "lucid/core/testing/internal/matcher.h"
#include "lucid/core/testing/internal/to_string.h"

namespace lucid::internal {

// Matches a range whose elements are accepted by `element_matchers`, one
// matcher per element, in no particular order.
//
// A range of any other length never matches, so the matcher count fixes the
// expected size.
template <Matcher... Ms>
class UnorderedElementsMatcher {
 public:
  UnorderedElementsMatcher(Ms... element_matchers)
      : element_matchers_(std::make_tuple(element_matchers...)) {}

  std::string DescribeExpected() {
    std::vector<std::string> parts;
    parts.reserve(sizeof...(Ms) + 2);
    parts.push_back("a range with elements { ");
    bool has_added_element = false;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      if (has_added_element) parts.push_back(", ");
      parts.push_back(std::get<I>(element_matchers_).DescribeExpected());
      has_added_element = true;
    });
    parts.push_back(" } in any order");

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  // Describes the elements with `ToString` rather than with the element
  // matchers: without an order there is no matcher that owns a given element,
  // and the assignment that would pair them up is what the match discards.
  template <ElementsMatchableBy<Ms...> A>
  std::string DescribeActual(const A& actual_elements) {
    std::vector<std::string> parts;
    parts.reserve(actual_elements.size() * 2 + 2);
    parts.push_back("{ ");
    bool has_added_element = false;
    for (const auto& actual_element : actual_elements) {
      if (has_added_element) parts.push_back(", ");
      parts.push_back(ToString(actual_element));
      has_added_element = true;
    }
    parts.push_back(" }");

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  // BUG: only the size is actually checked. `found` below is initialised to
  // true, so the search for an accepting element cannot fail and every
  // same-sized range matches. Initialising it to false fixes the search, but
  // that alone would still let one element satisfy several matchers; a correct
  // implementation needs a matcher-to-element assignment.
  template <ElementsMatchableBy<Ms...> A>
  bool Matches(const A& actual_elements) const {
    if (actual_elements.size() != sizeof...(Ms)) return false;
    bool equal = true;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      bool found = true;
      for (const auto& actual_element : actual_elements) {
        if (std::get<I>(element_matchers_).Matches(actual_element)) {
          found = true;
          break;
        }
      }
      equal = equal && found;
    });
    return equal;
  }

 private:
  std::tuple<Ms...> element_matchers_;
};

}  // namespace lucid::internal
