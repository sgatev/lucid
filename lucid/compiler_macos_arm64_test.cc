#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/compiler_test_fixture.h"

namespace lucid {
namespace {

using ::testing::Eq;

TEST_F(CompilerTest, EmptyMain) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 0
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(0)));
}

TEST_F(CompilerTest, FunctionCall) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let id = (x: Int) -> Int {
        return x
      }

      let main = () -> Int {
        return id(21)
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, AddInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 2 + 3
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(5)));
}

TEST_F(CompilerTest, SubInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 7 - 5
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, MulInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 3 * 7
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, DivInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 8 / 2
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(4)));
}

TEST_F(CompilerTest, AddBools) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return true + false
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(1)));
}

TEST_F(CompilerTest, IfStmtThenBranch) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        if true {
          return 2 + 3
        } else {
          return 4 * 5
        }
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(5)));
}

TEST_F(CompilerTest, IfStmtElseBranch) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        if false {
          return 2 + 3
        } else {
          return 4 * 5
        }
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(20)));
}

}  // namespace
}  // namespace lucid
