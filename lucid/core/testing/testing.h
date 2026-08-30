#pragma once

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/core/string/concat.h"

namespace lucid {

// Returns a string representation of `s`.
template <typename T>
inline std::string ToString(const T& t) {
  return "[unstringable]";
}

// Returns a string representation of `s`.
template <>
inline std::string ToString(const std::string_view& s) {
  return std::string(s);
}

// Returns a string representation of `s`.
template <>
inline std::string ToString(const int& s) {
  return std::to_string(s);
}

namespace internal {

class BooleanMatcher {
 public:
  explicit BooleanMatcher(bool expected_value)
      : expected_value_(expected_value) {}

  std::string DescribeExpected() {
    return expected_value_ ? "be true" : "be false";
  }

  std::string DescribeActual(bool actual_value) {
    return actual_value ? "true" : "false";
  }

  bool Matches(bool actual_value) { return actual_value == expected_value_; }

 private:
  bool expected_value_;
};

template <typename E>
class EqualityMatcher {
 public:
  explicit EqualityMatcher(E expected_value, bool expect_equals)
      : expected_value_(expected_value), expect_equals_(expect_equals) {}

  std::string DescribeExpected() {
    std::string qualifier = expect_equals_ ? "equal " : "not equal ";
    std::string stringified_expected_value = ToString(expected_value_);
    return Concat(
        std::initializer_list<std::string_view>{qualifier,
                                                stringified_expected_value},
        "");
  }

  template <typename A>
  std::string DescribeActual(const A& actual_value) {
    return ToString(actual_value);
  }

  template <typename A>
  bool Matches(const A& actual_value) {
    return (actual_value == expected_value_) == expect_equals_;
  }

 private:
  const E expected_value_;
  bool expect_equals_;
};

template <typename T>
class ElementsMatcher {
 public:
  ElementsMatcher(std::initializer_list<const T> expected_elements)
      : expected_elements_(expected_elements) {}

  std::string DescribeExpected() {
    std::vector<std::string> parts;
    parts.reserve(expected_elements_.size() + 2);
    parts.push_back("have elements { ");
    for (bool has_added_element = false;
         const auto& element : expected_elements_) {
      if (has_added_element) parts.push_back(", ");
      parts.push_back(ToString(element));
      has_added_element = true;
    }
    parts.push_back(" }");

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  std::string DescribeActual(std::span<const T> actual_elements) {
    std::vector<std::string> parts;
    parts.reserve(actual_elements.size() + 2);
    parts.push_back("{ ");
    for (bool has_added_element = false;
         const auto& element : actual_elements) {
      if (has_added_element) parts.push_back(", ");
      parts.push_back(ToString(element));
      has_added_element = true;
    }
    parts.push_back(" }");

    return Concat(std::vector<std::string_view>(parts.begin(), parts.end()),
                  "");
  }

  bool Matches(std::span<const T> actual_elements) {
    return std::ranges::equal(actual_elements, expected_elements_);
  }

 private:
  std::initializer_list<const T> expected_elements_;
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

#define TEST(base, name)                                                \
  /* NOLINTNEXTLINE */                                                  \
  class name : public base {                                            \
   public:                                                              \
    std::string_view Name() const final { return TO_STRING(name); }     \
    void Run() final;                                                   \
  };                                                                    \
  /* NOLINTNEXTLINE */                                                  \
  static auto UNIQUE_VAR(t) = lucid::AddTest(std::make_unique<name>()); \
  void name::Run()

// Matches a value that is true.
inline internal::BooleanMatcher IsTrue() {
  return internal::BooleanMatcher(true);
}

// Matches a value that is false.
inline internal::BooleanMatcher IsFalse() {
  return internal::BooleanMatcher(false);
}

// Matches a value that is equal to `expected_value`.
template <typename T>
inline internal::EqualityMatcher<T> Equals(T expected_value) {
  return internal::EqualityMatcher<T>(expected_value, true);
}

// Matches a value that is not equal to `expected_value`.
template <typename T>
inline internal::EqualityMatcher<T> NotEquals(T expected_value) {
  return internal::EqualityMatcher<T>(expected_value, false);
}

// Matches a value that contains `expected_elements`.
template <typename T>
internal::ElementsMatcher<const T> ElementsAre(
    std::initializer_list<const T> expected_elements) {
  return internal::ElementsMatcher<const T>(expected_elements);
}

#define ASSERT_THAT(actual, matcher)                                       \
  if (!(matcher).Matches(actual)) {                                        \
    std::vector<std::string> parts = {                                     \
        "Expected ",   TO_STRING(actual),                                  \
        " to ",        (matcher).DescribeExpected(),                       \
        " but found ", (matcher).DescribeActual(actual),                   \
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
        "Expected ",   TO_STRING(actual),                                  \
        " to ",        (matcher).DescribeExpected(),                       \
        " but found ", (matcher).DescribeActual(actual),                   \
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
