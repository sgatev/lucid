// The concepts shared by the matchers.
//
// A matcher decides whether an actual value is acceptable, and describes both
// what it expects and what it was given, so that a failing assertion can
// report the difference. Matchers are plain values: cheap to copy, and
// composed by handing one to another.
//
// Every matcher provides:
// - `DescribeExpected()`, describing the values it accepts.
// - `Matches(actual)`, deciding whether `actual` is one of them.
// - `DescribeActual(actual)`, describing `actual`.
//
// `Matches` and `DescribeActual` are member templates, so a matcher usually
// accepts more than one type of actual value: `IsEmpty()` matches anything
// with an `empty()` member, whatever its type. That is why there are two
// concepts below rather than one -- `Matcher` covers what can be checked
// knowing only the matcher, `MatcherFor` adds what needs an actual value type.
#pragma once

#include <concepts>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>

namespace lucid::internal {

// A matcher of values.
//
// Matchers are generic over the type of the actual value they are given, so
// this describes only the part of their interface that does not depend on it.
// Use `MatcherFor` to also require that a matcher accepts a given actual value
// type.
//
// Requires:
// - `DescribeExpected` member that describes the values the matcher accepts.
template <typename M>
concept Matcher = requires(M m) {
  { m.DescribeExpected() } -> std::convertible_to<std::string>;
};

// A matcher that accepts actual values of type `A`.
//
// Requires:
// - `Matches` member that decides whether an actual value is accepted.
// - `DescribeActual` member that describes an actual value.
template <typename M, typename A>
concept MatcherFor = Matcher<M> and requires(M m, const A& a) {
  { m.Matches(a) } -> std::same_as<bool>;
  { m.DescribeActual(a) } -> std::convertible_to<std::string>;
};

// A value that reports how many elements it holds.
template <typename A>
concept Sized = requires(const A& a) {
  { a.size() } -> std::convertible_to<std::size_t>;
};

// A sized value whose elements can be traversed from its beginning.
template <typename A>
concept ElementRange =
    Sized<A> and
    requires(const A& a, decltype(std::begin(std::declval<const A&>())) it) {
      { *it };
      { ++it };
      { it != std::end(a) } -> std::convertible_to<bool>;
    };

// The type of the elements of `A`.
template <ElementRange A>
using ElementType = decltype(*std::begin(std::declval<const A&>()));

// An actual value that matcher `M` accepts.
//
// This is `MatcherFor` with its arguments reversed, so that it can be used as
// a constrained template parameter: `template <MatchableBy<M> A>`.
template <typename A, typename M>
concept MatchableBy = MatcherFor<M, A>;

// An element range whose elements the matchers `Ms` accept.
template <typename A, typename... Ms>
concept ElementsMatchableBy =
    ElementRange<A> and (MatcherFor<Ms, ElementType<A>> and ...);

}  // namespace lucid::internal
