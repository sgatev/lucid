#pragma once

#include <memory>
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

#define EXPECT_EQ(actual, expected)    \
  if ((actual) != (expected))          \
    Fail(Concat(                       \
        std::vector<std::string_view>{ \
            "Expected ",               \
            TO_STRING(actual),         \
            " to equal ",              \
            "\"",                      \
            expected,                  \
            "\"",                      \
            " but found ",             \
            "\"",                      \
            actual,                    \
            "\"",                      \
            ".",                       \
        },                             \
        ""));

}  // namespace lucid
