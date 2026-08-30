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

#define ASSERT_TRUE(actual)                                                \
  if (!(actual)) {                                                         \
    std::vector<std::string> parts = {                                     \
        "Expected ", TO_STRING(actual), " to be true ", " but found ",     \
        "\"",        ToString(actual),  "\"",           ".",               \
    };                                                                     \
    Fail(Concat(std::vector<std::string_view>(parts.begin(), parts.end()), \
                ""));                                                      \
    return;                                                                \
  }

#define ASSERT_FALSE(actual)                                               \
  if ((actual)) {                                                          \
    std::vector<std::string> parts = {                                     \
        "Expected ",                                                       \
        TO_STRING(actual),                                                 \
        " to be false ",                                                   \
        " but found ",                                                     \
        "\"",                                                              \
        ToString(actual),                                                  \
        "\"",                                                              \
        ".",                                                               \
    };                                                                     \
    Fail(Concat(std::vector<std::string_view>(parts.begin(), parts.end()), \
                ""));                                                      \
    return;                                                                \
  }

#define EXPECT_TRUE(actual)                                                \
  if (!(actual)) {                                                         \
    std::vector<std::string> parts = {                                     \
        "Expected ", TO_STRING(actual), " to be true ", " but found ",     \
        "\"",        ToString(actual),  "\"",           ".",               \
    };                                                                     \
    Fail(Concat(std::vector<std::string_view>(parts.begin(), parts.end()), \
                ""));                                                      \
  }

#define EXPECT_FALSE(actual)                                               \
  if ((actual)) {                                                          \
    std::vector<std::string> parts = {                                     \
        "Expected ",                                                       \
        TO_STRING(actual),                                                 \
        " to be false ",                                                   \
        " but found ",                                                     \
        "\"",                                                              \
        ToString(actual),                                                  \
        "\"",                                                              \
        ".",                                                               \
    };                                                                     \
    Fail(Concat(std::vector<std::string_view>(parts.begin(), parts.end()), \
                ""));                                                      \
  }

#define EXPECT_EQ(actual, expected)                                        \
  if ((actual) != (expected)) {                                            \
    std::vector<std::string> parts = {                                     \
        "Expected ",                                                       \
        TO_STRING(actual),                                                 \
        " to equal ",                                                      \
        "\"",                                                              \
        ToString(expected),                                                \
        "\"",                                                              \
        " but found ",                                                     \
        "\"",                                                              \
        ToString(actual),                                                  \
        "\"",                                                              \
        ".",                                                               \
    };                                                                     \
    Fail(Concat(std::vector<std::string_view>(parts.begin(), parts.end()), \
                ""));                                                      \
  }

#define EXPECT_NE(actual, expected)                                        \
  if ((actual) == (expected)) {                                            \
    std::vector<std::string> parts = {                                     \
        "Expected ",                                                       \
        TO_STRING(actual),                                                 \
        " to not equal ",                                                  \
        "\"",                                                              \
        ToString(expected),                                                \
        "\"",                                                              \
        " but found ",                                                     \
        "\"",                                                              \
        ToString(actual),                                                  \
        "\"",                                                              \
        ".",                                                               \
    };                                                                     \
    Fail(Concat(std::vector<std::string_view>(parts.begin(), parts.end()), \
                ""));                                                      \
  }

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

template <typename T>
ElementsMatcher<const T> ElementsAre(
    std::initializer_list<const T> expected_elements) {
  return ElementsMatcher<const T>(expected_elements);
}

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

}  // namespace lucid
