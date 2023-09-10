#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/compiler_test_fixture.h"

namespace lucid {
namespace {

using ::testing::Eq;

TEST_F(CompilerTest, EmptyMain) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        return 0
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(0)));
}

TEST_F(CompilerTest, FunctionCall) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let id = (x: Int32) -> Int32 {
        return x
      }

      let main = () -> Int32 {
        return id(21)
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, AddInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        return 2 + 3
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(5)));
}

TEST_F(CompilerTest, AddInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int64 {
        return 2 + 3
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(5)));
}

TEST_F(CompilerTest, SubInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        return 7 - 5
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, SubInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int64 {
        return 7 - 5
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, MulInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        return 3 * 7
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, MulInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int64 {
        return 3 * 7
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, DivInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        return 8 / 2
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(4)));
}

TEST_F(CompilerTest, DivInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int64 {
        return 8 / 2
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(4)));
}

TEST_F(CompilerTest, AddBools) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        return true + false
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(1)));
}

TEST_F(CompilerTest, IfStmtThenBranch) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        if true {
          return 2 
        } else {
          return 3
        }
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, IfStmtElseBranch) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        if false {
          return 2 
        } else {
          return 3 
        }
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(3)));
}

TEST_F(CompilerTest, GtInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        if 7 > 1 {
          return 2
        } else {
          return 3
        }
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, GtInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int64 {
        if 7 > 1 {
          return 2
        } else {
          return 3
        }
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, LtInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        if 1 < 7 {
          return 2
        } else {
          return 3
        }
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, LtInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int64 {
        if 1 < 7 {
          return 2
        } else {
          return 3
        }
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, VarDecl) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
      let main = () -> Int32 {
        let x: Int32 = 2
        let y: Int32 = 3
        return x + y
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(5)));
}

}  // namespace
}  // namespace lucid
