#pragma once

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include "lucid/core/meta/static_for.h"
#include "lucid/core/string/concat.h"
#include "lucid/core/testing/internal/boolean_matcher.h"
#include "lucid/core/testing/internal/equality_matcher.h"
#include "lucid/core/testing/internal/predicate_matcher.h"

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

namespace internal {

template <typename... Ms>
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

  template <typename A>
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

  template <typename A>
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

template <typename... Ms>
class UnorderedElementsMatcher {
 public:
  UnorderedElementsMatcher(Ms... element_matchers)
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

  template <typename A>
  std::string DescribeActual(const A& actual_elements) {
    std::vector<std::string> parts;
    parts.reserve(actual_elements.size() + 2);
    parts.push_back("{ ");
    bool has_added_element = false;
    StaticFor<0, sizeof...(Ms)>([&]<int I>() {
      if (has_added_element) parts.push_back(", ");
      /*parts.push_back(std::get<I>(element_matchers_)
                          .DescribeActual(*actual_elements.find(I)));*/
      has_added_element = true;
    });
    parts.push_back(" }");

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  template <typename A>
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

class SizeMatcher {
 public:
  SizeMatcher(std::size_t expected_size) : expected_size_(expected_size) {}

  std::string DescribeExpected() {
    return "size is " + std::to_string(expected_size_);
  }

  template <typename A>
  std::string DescribeActual(const A& actual_elements) {
    return std::to_string(actual_elements.size());
  }

  template <typename A>
  bool Matches(const A& actual_elements) const {
    return actual_elements.size() == expected_size_;
  }

 private:
  std::size_t expected_size_;
};

class EmptyMatcher {
 public:
  std::string DescribeExpected() { return "is empty"; }

  template <typename A>
  std::string DescribeActual(const A& actual_elements) {
    return actual_elements.empty() ? "is empty" : "is not empty";
  }

  template <typename A>
  bool Matches(const A& actual_elements) const {
    return actual_elements.empty();
  }
};

template <typename F, typename M>
class FieldMatcher {
 public:
  explicit FieldMatcher(F field, M field_matcher)
      : field_(field), field_matcher_(field_matcher) {}

  std::string DescribeExpected() {
    return "with a field that " + field_matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(A&& actual_value) {
    return "with a field that " +
           field_matcher_.DescribeActual(std::invoke(field_, actual_value));
  }

  template <typename A>
  bool Matches(A&& actual_value) const {
    return field_matcher_.Matches(std::invoke(field_, actual_value));
  }

 private:
  F field_;
  M field_matcher_;
};

template <typename M>
class OptionalMatcher {
 public:
  explicit OptionalMatcher(M value_matcher) : value_matcher_(value_matcher) {}

  std::string DescribeExpected() {
    return "contain a value " + value_matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    if (!actual_value.has_value()) return "nullopt";
    return "optional " + value_matcher_.DescribeActual(actual_value);
  }

  template <typename A>
  bool Matches(const A& actual_value) const {
    if (!actual_value.has_value()) return false;
    return value_matcher_.Matches(*actual_value);
  }

 private:
  M value_matcher_;
};

template <typename T, typename M>
class VariantMatcher {
 public:
  explicit VariantMatcher(M value_matcher) : value_matcher_(value_matcher) {}

  std::string DescribeExpected() {
    return "contain a value " + value_matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    const auto* actual = std::get_if<T>(&actual_value);
    if (actual == nullptr) return "";
    return "variant with " + value_matcher_.DescribeActual(*actual);
  }

  template <typename A>
  bool Matches(const A& actual_value) const {
    const auto* actual = std::get_if<T>(&actual_value);
    if (actual == nullptr) return false;
    return value_matcher_.Matches(*actual);
  }

 private:
  M value_matcher_;
};

template <typename FM, typename SM>
class PairMatcher {
 public:
  explicit PairMatcher(FM first_matcher, SM second_matcher)
      : first_matcher_(first_matcher), second_matcher_(second_matcher) {}

  std::string DescribeExpected() {
    return "contain a pair with key " + first_matcher_.DescribeExpected() +
           " and value " + second_matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return "pair with key " +
           first_matcher_.DescribeActual(actual_value.first) + " and value " +
           second_matcher_.DescribeActual(actual_value.second);
  }

  template <typename A>
  bool Matches(const A& actual_value) const {
    return first_matcher_.Matches(actual_value.first) &&
           second_matcher_.Matches(actual_value.second);
  }

 private:
  FM first_matcher_;
  SM second_matcher_;
};

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

template <typename M>
class NotMatcher {
 public:
  explicit NotMatcher(M matcher) : matcher_(matcher) {}

  std::string DescribeExpected() {
    return "not " + matcher_.DescribeExpected();
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return matcher_.DescribeActual(actual_value);
  }

  template <typename A>
  bool Matches(const A& actual_value) const {
    return !matcher_.Matches(actual_value);
  }

 private:
  M matcher_;
};

}  // namespace internal

// The base class of a test.
class Test {
 public:
  virtual ~Test() = default;

  // Returns the name of the test.
  virtual std::string_view Name() const = 0;

  // Runs the test and returns true if it succeeded.
  bool RunFull();

  // Fails the test with the given `reason`.
  void Fail(std::string_view reason);

 protected:
  // Defines the logic of the test.
  virtual void Run() = 0;

 private:
  bool failed_;
};

// Adds a new test to the global suite of tests.
int AddTest(std::unique_ptr<Test> test);

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define CONCAT_(prefix, suffix) prefix##suffix
#define CONCAT(prefix, suffix) CONCAT_(prefix, suffix)

#define UNIQUE_VAR(prefix) CONCAT(prefix##_, __LINE__)

#define TEST(base, name)                                            \
  /* NOLINTNEXTLINE */                                              \
  class CONCAT(name, Test) : public base {                          \
   public:                                                          \
    std::string_view Name() const final { return TO_STRING(name); } \
    void Run() final;                                               \
  };                                                                \
  /* NOLINTNEXTLINE */                                              \
  static auto UNIQUE_VAR(t) =                                       \
      lucid::AddTest(std::make_unique<CONCAT(name, Test)>());       \
  void CONCAT(name, Test)::Run()

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
template <typename... Ms>
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
template <typename... Ms>
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
template <typename F, typename M>
internal::FieldMatcher<F, M> Field(F field, M field_matcher) {
  return internal::FieldMatcher<F, M>(field, field_matcher);
}

// Matches an optional that contains a value accepted by `value_matcher`.
template <typename M>
internal::OptionalMatcher<M> Optional(M value_matcher) {
  return internal::OptionalMatcher<M>(value_matcher);
}

// Matches a variant that contains value of type `T` accepted by
// `value_matcher`.
template <typename T, typename M>
internal::VariantMatcher<T, M> Variant(M value_matcher) {
  return internal::VariantMatcher<T, M>(value_matcher);
}

// Matches a pair value whose first element is accepted by `first_matcher` and
// whose seccond element is accepted by `second_matcher`.
template <typename FM, typename SM>
internal::PairMatcher<FM, SM> Pair(FM first_matcher, SM second_matcher) {
  return internal::PairMatcher<FM, SM>(first_matcher, second_matcher);
}

// Matches a value that is accepted by all `matchers`.
template <typename... Ms>
internal::AllMatcher<Ms...> AllOf(Ms... matchers) {
  return internal::AllMatcher<Ms...>(matchers...);
}

// Matches a value not accepted by `matcher`.
template <typename M>
internal::NotMatcher<M> Not(M matcher) {
  return internal::NotMatcher<M>(matcher);
}

inline auto StartsWith(std::string_view prefix) {
  return Truly([prefix](std::string_view s) { return s.starts_with(prefix); });
}

inline auto EndsWith(std::string_view suffix) {
  return Truly([suffix](std::string_view s) { return s.ends_with(suffix); });
}

#define ASSERT_THAT(actual, matcher)                                       \
  if (!(matcher).Matches(actual)) {                                        \
    std::vector<std::string> parts = {                                     \
        "Expected ",                                                       \
        TO_STRING(actual),                                                 \
        "\n",                                                              \
        " to be ",                                                         \
        (matcher).DescribeExpected(),                                      \
        "\n",                                                              \
        " but was found ",                                                 \
        (matcher).DescribeActual(actual),                                  \
        ".",                                                               \
    };                                                                     \
    Fail(Concat(std::vector<std::string_view>(parts.begin(), parts.end()), \
                ""));                                                      \
    return;                                                                \
  }

#define ASSERT_TRUE(actual) ASSERT_THAT(actual, IsTrue())
#define ASSERT_FALSE(actual) ASSERT_THAT(actual, IsFalse())
#define ASSERT_EQ(actual, expected) ASSERT_THAT(actual, Equals(expected))
#define ASSERT_NE(actual, expected) ASSERT_THAT(actual, NotEquals(expected))

#define EXPECT_THAT(actual, matcher)                                       \
  if (!(matcher).Matches(actual)) {                                        \
    std::vector<std::string> parts = {                                     \
        "Expected ",                                                       \
        TO_STRING(actual),                                                 \
        "\n",                                                              \
        " to be ",                                                         \
        (matcher).DescribeExpected(),                                      \
        "\n",                                                              \
        " but was found ",                                                 \
        (matcher).DescribeActual(actual),                                  \
        ".",                                                               \
    };                                                                     \
    Fail(Concat(std::vector<std::string_view>(parts.begin(), parts.end()), \
                ""));                                                      \
  }

#define EXPECT_TRUE(actual) EXPECT_THAT(actual, IsTrue())
#define EXPECT_FALSE(actual) EXPECT_THAT(actual, IsFalse())
#define EXPECT_EQ(actual, expected) EXPECT_THAT(actual, Equals(expected))
#define EXPECT_NE(actual, expected) EXPECT_THAT(actual, NotEquals(expected))

}  // namespace lucid
