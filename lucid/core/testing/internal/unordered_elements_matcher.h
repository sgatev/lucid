#pragma once

#include <array>
#include <cstddef>
#include <format>
#include <string>
#include <tuple>

#include "lucid/core/meta/static_for.h"
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
    std::string elements;
    bool has_added_element = false;
    StaticFor<0, kMatcherCount>([&]<int I>() {
      if (has_added_element) elements += ", ";
      elements += std::get<I>(element_matchers_).DescribeExpected();
      has_added_element = true;
    });

    return std::format("a range with elements {{ {} }} in any order", elements);
  }

  // Describes the elements with `ToString` rather than with the element
  // matchers: without an order there is no matcher that owns a given element,
  // and the assignment that would pair them up is local to `Matches`.
  template <ElementsMatchableBy<Ms...> A>
  std::string DescribeActual(const A& actual_elements) {
    std::string elements;
    bool has_added_element = false;
    for (const auto& actual_element : actual_elements) {
      if (has_added_element) elements += ", ";
      elements += ToString(actual_element);
      has_added_element = true;
    }

    return std::format("{{ {} }}", elements);
  }

  template <ElementsMatchableBy<Ms...> A>
  bool Matches(const A& actual_elements) const {
    if (actual_elements.size() != kMatcherCount) return false;

    // Every matcher accepting *some* element is not enough: each needs an
    // element of its own, or `UnorderedElementsEqual(1, 1)` would match
    // { 1, 2 }, both matchers claiming the single 1. So record which elements
    // each matcher accepts, then look for a one-to-one assignment.
    Acceptance accepts;
    StaticFor<0, kMatcherCount>([&]<int I>() {
      std::size_t element = 0;
      for (const auto& actual_element : actual_elements) {
        accepts[I][element] =
            std::get<I>(element_matchers_).Matches(actual_element);
        ++element;
      }
    });

    // The two sides are the same size, so the range matches exactly when every
    // matcher can be assigned. Assigning them in turn is safe because `Assign`
    // reshuffles earlier choices rather than committing to them.
    Assignment assignment;
    assignment.fill(kUnassigned);
    for (std::size_t matcher = 0; matcher < kMatcherCount; ++matcher) {
      VisitedElements visited = {};
      if (!Assign(accepts, matcher, visited, assignment)) return false;
    }
    return true;
  }

 private:
  static constexpr std::size_t kMatcherCount = sizeof...(Ms);

  // Marks an element that no matcher has been assigned yet.
  static constexpr std::size_t kUnassigned = static_cast<std::size_t>(-1);

  // Which elements each matcher accepts, indexed by matcher then by element.
  using Acceptance = std::array<std::array<bool, kMatcherCount>, kMatcherCount>;

  // The matcher each element is assigned to, indexed by element.
  using Assignment = std::array<std::size_t, kMatcherCount>;

  // The elements already considered by one search, indexed by element.
  using VisitedElements = std::array<bool, kMatcherCount>;

  // Assigns `matcher` an element of its own, and returns whether it could.
  //
  // Walks the elements `matcher` accepts. A free one is taken; a taken one is
  // claimed only if the matcher holding it can be re-assigned elsewhere, which
  // is the same question one step deeper. `visited` records the elements this
  // search has already tried, so that a cycle of re-assignments terminates.
  static bool Assign(const Acceptance& accepts, std::size_t matcher,
                     VisitedElements& visited, Assignment& assignment) {
    for (std::size_t element = 0; element < kMatcherCount; ++element) {
      if (!accepts[matcher][element] || visited[element]) continue;
      visited[element] = true;
      if (assignment[element] == kUnassigned ||
          Assign(accepts, assignment[element], visited, assignment)) {
        assignment[element] = matcher;
        return true;
      }
    }
    return false;
  }

  std::tuple<Ms...> element_matchers_;
};

}  // namespace lucid::internal
