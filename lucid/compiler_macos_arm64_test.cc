#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/compiler_test_fixture.h"

namespace lucid {
namespace {

using ::testing::AllOf;
using ::testing::Eq;

TEST_F(CompilerTest, EmptyMain) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
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
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, AddInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 2 + 3
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
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
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, SubInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      return 7 - 5
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, MulInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 3 * 7
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, MulInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      return 3 * 7
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, DivInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 8 / 2
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(4)));
}

TEST_F(CompilerTest, DivInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      return 8 / 2
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(4)));
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
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
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
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
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
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
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
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, GtFalse) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      if 1 > 7 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(3)));
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
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
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
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, LtFalse) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      if 7 < 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(3)));
}

TEST_F(CompilerTest, EqInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      if 1 == 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, EqInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      if 1 == 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, EqFalse) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      if 1 == 2 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(3)));
}

TEST_F(CompilerTest, VarDecl) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let x: Int32 = 2
      let y: Int32 = 3
      return x + y
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(5)));
}

TEST_F(CompilerTest, VarDeclFromVar) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let x: Int32 = 2
      let y: Int32 = x
      return y
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, VarAssign) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let foo = (n: Int32) -> Int32 {
      n = 3
      return n
    }

    let main = () -> Int32 {
      return foo(2)
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(3)));
}

TEST_F(CompilerTest, FactRec) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let fact = (n: Int32) -> Int32 {
      if n == 1 {
        return 1
      } else {
        return fact(n-1) * n
      }
    }

    let main = () -> Int32 {
      return fact(5)
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(120)));
}

TEST_F(CompilerTest, FibRec) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let fib = (n: Int32) -> Int32 {
      if n < 2 {
        return n
      }
      return fib(n-1) + fib(n-2)
    }

    let main = () -> Int32 {
      return fib(8)
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, PrintInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let print = (n: Int32) -> Int32 {
      return 0
    }

    let main = () -> Int32 {
      print(21)
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), AllOf(ReturnsCode(Eq(0)), Prints("21")));
}

TEST_F(CompilerTest, PrintString) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let prints = (n: String) -> Int32 {
      return 0
    }

    let main = () -> Int32 {
      prints("Hello, world!\n")
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"),
              AllOf(ReturnsCode(Eq(0)), Prints("Hello, world!\n")));
}

}  // namespace
}  // namespace lucid
