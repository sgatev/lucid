#pragma once

#include <cstddef>
#include <filesystem>
#include <format>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <variant>

#include "lucid/core/meta/macros.h"
#include "lucid/core/testing/internal/all_matcher.h"
#include "lucid/core/testing/internal/boolean_matcher.h"
#include "lucid/core/testing/internal/elements_matcher.h"
#include "lucid/core/testing/internal/empty_matcher.h"
#include "lucid/core/testing/internal/equality_matcher.h"
#include "lucid/core/testing/internal/field_matcher.h"
#include "lucid/core/testing/internal/matcher.h"
#include "lucid/core/testing/internal/not_matcher.h"
#include "lucid/core/testing/internal/optional_matcher.h"
#include "lucid/core/testing/internal/pair_matcher.h"
#include "lucid/core/testing/internal/predicate_matcher.h"
#include "lucid/core/testing/internal/prefix_matcher.h"
#include "lucid/core/testing/internal/size_matcher.h"
#include "lucid/core/testing/internal/suffix_matcher.h"
#include "lucid/core/testing/internal/unordered_elements_matcher.h"
#include "lucid/core/testing/internal/variant_matcher.h"

namespace lucid {

template <typename T, class... Types>
inline bool operator==(const T& t, const std::variant<Types...>& v) {
  const T* c = std::get_if<T>(&v);

  return c && *c == t;
}

template <typename T, class... Types>
inline bool operator==(const std::variant<Types...>& v, const T& t) {
  return t == v;
}

// The base class of a test.
class Test {
 public:
  virtual ~Test() = default;

  // Returns the name of the test.
  virtual std::string_view Name() const = 0;

  // Runs the test and returns true if it succeeded.
  bool RunFull();

  // Fails the test with the given `reason`, reporting `location` as where it
  // happened.
  //
  // `location` defaults to the call site, which is what a caller almost always
  // wants: the assertion macros expand at the assertion, so the default picks
  // up the line under test rather than any line inside the framework.
  void Fail(std::string_view reason,
            std::source_location location = std::source_location::current());

 protected:
  // Defines the logic of the test.
  virtual void Run() = 0;

  // Returns the path to the temporary directory created for this test run.
  std::filesystem::path TempDir() const { return temp_dir_; }

 private:
  bool failed_ = false;
  std::filesystem::path temp_dir_;
};

// Adds a new test to the global suite of tests.
int AddTest(std::unique_ptr<Test> test);

// Defines a test called `name` that runs with the fixture `base`.
//
// A test name has to be unique within a binary, whatever fixture it runs with.
// The generated class is named after `name` alone and its `Run` has external
// linkage, so a repeated name fails the build.
#define TEST(base, name)                                                  \
  /* NOLINTNEXTLINE */                                                    \
  class LUCID_CONCAT(name, Test) : public base {                          \
   public:                                                                \
    std::string_view Name() const final { return LUCID_STRINGIFY(name); } \
    void Run() final;                                                     \
  };                                                                      \
  /* NOLINTNEXTLINE */                                                    \
  static auto LUCID_UNIQUE_VAR(t) =                                       \
      lucid::AddTest(std::make_unique<LUCID_CONCAT(name, Test)>());       \
  void LUCID_CONCAT(name, Test)::Run()

// Matches a value that is accepted by `predicate`.
template <typename P>
inline internal::PredicateMatcher<P> Truly(P predicate) {
  return internal::PredicateMatcher<P>(predicate);
}

// Matches a value that is true.
inline internal::BooleanMatcher IsTrue() {
  return internal::BooleanMatcher(true);
}

// Matches a value that is false.
inline internal::BooleanMatcher IsFalse() {
  return internal::BooleanMatcher(false);
}

// Matches a value that is equal to `expected_value`.
template <typename E>
inline internal::EqualityMatcher<E> Equals(E expected_value) {
  return internal::EqualityMatcher<E>(std::forward<E>(expected_value), true);
}

// Matches a value that is not equal to `expected_value`.
template <typename E>
inline internal::EqualityMatcher<E> NotEquals(E expected_value) {
  return internal::EqualityMatcher<E>(expected_value, false);
}

// Matches a value whose size is `expected_size`.
inline internal::SizeMatcher SizeIs(std::size_t expected_size) {
  return internal::SizeMatcher(expected_size);
}

// Matches a value that is empty.
inline internal::EmptyMatcher IsEmpty() { return internal::EmptyMatcher(); }

// Matches a value that whose elements match `element_matchers` in the given
// order.
template <internal::Matcher... Ms>
internal::ElementsMatcher<Ms...> Elements(Ms... element_matchers) {
  return internal::ElementsMatcher<Ms...>(element_matchers...);
}

// Matches a value that contains `expected_elements` in the given order.
template <typename... Ts>
internal::ElementsMatcher<internal::EqualityMatcher<Ts>...> ElementsEqual(
    Ts... expected_elements) {
  return Elements(Equals(expected_elements)...);
}

// Matches a value that whose elements match `element_matchers` in no particular
// order.
template <internal::Matcher... Ms>
internal::UnorderedElementsMatcher<Ms...> UnorderedElements(
    Ms... element_matchers) {
  return internal::UnorderedElementsMatcher<Ms...>(element_matchers...);
}

// Matches a value that contains `expected_elements` in no particular order.
template <typename... Ts>
internal::UnorderedElementsMatcher<internal::EqualityMatcher<Ts>...>
UnorderedElementsEqual(Ts... expected_elements) {
  return UnorderedElements(Equals(expected_elements)...);
}

// Matches a value that has a field accepted by `field_matcher`.
template <typename F, internal::Matcher M>
internal::FieldMatcher<F, M> Field(F field, M field_matcher) {
  return internal::FieldMatcher<F, M>(field, field_matcher);
}

// Matches an optional that contains a value accepted by `value_matcher`.
template <internal::Matcher M>
internal::OptionalMatcher<M> Optional(M value_matcher) {
  return internal::OptionalMatcher<M>(value_matcher);
}

// Matches a variant that contains value of type `T` accepted by
// `value_matcher`.
template <typename T, internal::Matcher M>
internal::VariantMatcher<T, M> Variant(M value_matcher) {
  return internal::VariantMatcher<T, M>(value_matcher);
}

// Matches a pair value whose first element is accepted by `first_matcher` and
// whose seccond element is accepted by `second_matcher`.
template <internal::Matcher FM, internal::Matcher SM>
internal::PairMatcher<FM, SM> Pair(FM first_matcher, SM second_matcher) {
  return internal::PairMatcher<FM, SM>(first_matcher, second_matcher);
}

// Matches a value that is accepted by all `matchers`.
template <internal::Matcher... Ms>
internal::AllMatcher<Ms...> AllOf(Ms... matchers) {
  return internal::AllMatcher<Ms...>(matchers...);
}

// Matches a value not accepted by `matcher`.
template <internal::Matcher M>
internal::NotMatcher<M> Not(M matcher) {
  return internal::NotMatcher<M>(matcher);
}

// Matches a string that starts with `prefix`.
inline internal::PrefixMatcher StartsWith(std::string_view prefix) {
  return internal::PrefixMatcher(prefix);
}

// Matches a string that ends with `suffix`.
inline internal::SuffixMatcher EndsWith(std::string_view suffix) {
  return internal::SuffixMatcher(suffix);
}

// Checks `actual` against `matcher`, and on a mismatch fails the test with a
// description of the difference and then runs `on_mismatch`.
//
// Both operands are bound to locals so that each is evaluated exactly once:
// `actual` is often a call with side effects, and describing it must not run it
// a second time. Wrapping the whole thing in a loop makes an invocation a
// single statement, so it can be used as the body of an unbraced `if`.
#define LUCID_MATCH_OR_FAIL(actual, matcher, on_mismatch)                  \
  do {                                                                     \
    auto&& LUCID_UNIQUE_VAR(value) = (actual);                             \
    auto&& LUCID_UNIQUE_VAR(m) = (matcher);                                \
    if (!LUCID_UNIQUE_VAR(m).Matches(LUCID_UNIQUE_VAR(value))) {           \
      Fail(std::format(                                                    \
          "Expected {}\n to be {}\n but was found {}.",                    \
          LUCID_STRINGIFY(actual), LUCID_UNIQUE_VAR(m).DescribeExpected(), \
          LUCID_UNIQUE_VAR(m).DescribeActual(LUCID_UNIQUE_VAR(value))));   \
      on_mismatch;                                                         \
    }                                                                      \
  } while (false)

#define ASSERT_THAT(actual, matcher) \
  LUCID_MATCH_OR_FAIL(actual, matcher, return)

#define ASSERT_TRUE(actual) ASSERT_THAT(actual, IsTrue())
#define ASSERT_FALSE(actual) ASSERT_THAT(actual, IsFalse())
#define ASSERT_EQ(actual, expected) ASSERT_THAT(actual, Equals(expected))
#define ASSERT_NE(actual, expected) ASSERT_THAT(actual, NotEquals(expected))

#define EXPECT_THAT(actual, matcher) \
  LUCID_MATCH_OR_FAIL(actual, matcher, (void)0)

#define EXPECT_TRUE(actual) EXPECT_THAT(actual, IsTrue())
#define EXPECT_FALSE(actual) EXPECT_THAT(actual, IsFalse())
#define EXPECT_EQ(actual, expected) EXPECT_THAT(actual, Equals(expected))
#define EXPECT_NE(actual, expected) EXPECT_THAT(actual, NotEquals(expected))

}  // namespace lucid
