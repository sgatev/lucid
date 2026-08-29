#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
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

}  // namespace lucid
