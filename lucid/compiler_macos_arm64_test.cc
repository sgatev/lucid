#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/compiler_test_fixture.h"

namespace lucid {
namespace {

using ::testing::AllOf;

TEST_F(CompilerTest, PrintAst) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"print-ast", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Prints(R"(FuncDefStmt {
  .name = "main"
  .stmts = [
[34m    S0: [0mReturnStmt {
      .value = {
[34m        E0: [0mIntLitExpr {
          .value = 0
        }
      }
    }
  ]
}
)")));
}

TEST_F(CompilerTest, PrintAstNode) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"print-ast", FullPath("main.lu"), "E0"}),
              AllOf(ReturnsCode(0), Prints(R"([34mE0: [0mIntLitExpr {
  .value = 0
}
)")));
}

TEST_F(CompilerTest, PrintCfg) {
  ASSERT_TRUE(CreateFile("max.lu", R"(
    let max = (a: Int32, b: Int32) -> Int32 {
      let c: Int32 = 0
      if a > b {
        c = a
      } else {
        c = b
      }
      return c
    }
  )"));
  ASSERT_THAT(RunCompiler({"print-cfg", FullPath("max.lu")}),
              AllOf(ReturnsCode(0), Prints(R"([34mmax:
[0m[34m  B0: [0m{
    .sequences = [
[34m      E0: [0mIntLitExpr { .value = 0 }
[34m      S2: [0mVarDeclStmt { .name = 'c', .init = E0 }
[34m      E1: [0mIdentExpr { .name = 'a' }
[34m      E2: [0mIdentExpr { .name = 'b' }
[34m      E3: [0mBinaryOpExpr { .op = Gt, .lhs = E1, .rhs = E2 }
    ]
    .next = [
[34m      B3
[0m[34m      B4
[0m    ]
  }
[34m  B1: [0m{
    .preds = [
[34m      B2
[0m    ]
  }
[34m  B2: [0m{
    .sequences = [
[34m      E6: [0mIdentExpr { .name = 'c' }
[34m      S4: [0mReturnStmt { .value = E6 }
    ]
    .next = [
[34m      B1
[0m    ]
    .preds = [
[34m      B3
[0m[34m      B4
[0m    ]
  }
[34m  B3: [0m{
    .sequences = [
[34m      E4: [0mIdentExpr { .name = 'a' }
[34m      S0: [0mVarAssignStmt { .name = 'c', .expr = E4 }
    ]
    .next = [
[34m      B2
[0m    ]
    .preds = [
[34m      B0
[0m    ]
  }
[34m  B4: [0m{
    .sequences = [
[34m      E5: [0mIdentExpr { .name = 'b' }
[34m      S1: [0mVarAssignStmt { .name = 'c', .expr = E5 }
    ]
    .next = [
[34m      B2
[0m    ]
    .preds = [
[34m      B0
[0m    ]
  }

)")));
}

TEST_F(CompilerTest, Build) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(0));
  EXPECT_THAT(Run(FullPath("main")), ReturnsCode(0));
}

TEST_F(CompilerTest, EmptyMain) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(0));
}

TEST_F(CompilerTest, Comment) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    # comment
    let main = () -> Int32 {
      return 21 # comment
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST_F(CompilerTest, AddInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 2 + 3
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST_F(CompilerTest, AddInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      return 2 + 3
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST_F(CompilerTest, SubInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 7 - 5
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST_F(CompilerTest, SubInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      return 7 - 5
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST_F(CompilerTest, MulInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 3 * 7
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST_F(CompilerTest, MulInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      return 3 * 7
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST_F(CompilerTest, DivInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 8 / 2
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

TEST_F(CompilerTest, DivInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      return 8 / 2
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

TEST_F(CompilerTest, ModInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return 23 % 7
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST_F(CompilerTest, ModInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      return 17 % 5
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST_F(CompilerTest, IfStmtElseIfBranch) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let x: Int32 = 2
      if x == 1 {
        return 3
      } else if x == 2 {
        return 4
      } else {
        return 5
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST_F(CompilerTest, NotEqInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      if 1 != 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST_F(CompilerTest, NotEqInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      if 1 != 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST_F(CompilerTest, VarDecl) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let x: Int32 = 2
      let y: Int32 = 3
      return x + y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST_F(CompilerTest, VarDeclFromVar) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let x: Int32 = 2
      let y: Int32 = x
      return y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST_F(CompilerTest, FuncCallSingleArg) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let id = (x: Int32) -> Int32 {
      return x
    }

    let main = () -> Int32 {
      return id(21)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST_F(CompilerTest, FuncCallArgsSameType) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let sum = (x: Int32, y: Int32, z: Int32) -> Int32 {
      return x + y + z
    }

    let main = () -> Int32 {
      return sum(2, 3, 5)
    }
  )"));
  ASSERT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(10));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(120));
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
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST_F(CompilerTest, FibIter) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let fib = (n: Int32) -> Int32 {
      let a: Int32 = 0
      let b: Int32 = 1
      loop {
        if n == 0 {
          return a
        }

        let c: Int32 = a
        a = b
        b = c + b
        n = n - 1
      }
    }

    let main = () -> Int32 {
      return fib(8)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST_F(CompilerTest, PrintInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let printString = (s: String) -> Int32 {
      return 0
    }

    let printInt32 = (i: Int32) -> Int32 {
      if i > 9 {
        do printInt32(i / 10)
      }

      let j: Int32 = i % 10
      if      j == 0 { do printString("0") }
      else if j == 1 { do printString("1") }
      else if j == 2 { do printString("2") }
      else if j == 3 { do printString("3") }
      else if j == 4 { do printString("4") }
      else if j == 5 { do printString("5") }
      else if j == 6 { do printString("6") }
      else if j == 7 { do printString("7") }
      else if j == 8 { do printString("8") }
      else if j == 9 { do printString("9") }
      return 0
    }

    let main = () -> Int32 {
      do printInt32(21509)
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Prints("21509")));
}

TEST_F(CompilerTest, PrintString) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let printString = (s: String) -> Int32 {
      return 0
    }

    let main = () -> Int32 {
      do printString("Hello, world!\n")
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Prints("Hello, world!\n")));
}

TEST_F(CompilerTest, PrintMultipleValues) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let printString = (s: String) -> Int32 {
      return 0
    }

    let printInt32 = (i: Int32) -> Int32 {
      if i > 9 { do printInt32(i / 10) }

      let j: Int32 = i % 10
      if      j == 0 { do printString("0") }
      else if j == 1 { do printString("1") }
      else if j == 2 { do printString("2") }
      else if j == 3 { do printString("3") }
      else if j == 4 { do printString("4") }
      else if j == 5 { do printString("5") }
      else if j == 6 { do printString("6") }
      else if j == 7 { do printString("7") }
      else if j == 8 { do printString("8") }
      else if j == 9 { do printString("9") }
      return 0
    }

    let main = () -> Int32 {
      do printInt32(0)
      do printString(", ")
      do printInt32(1)
      do printString("\n")
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Prints("0, 1\n")));
}

TEST_F(CompilerTest, LoopAndBreak) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let four = () -> Int32 {
      let n: Int32 = 0
      loop {
        if n > 3 {
          break
        }

        n = n + 1
      }
      return n
    }

    let main = () -> Int32 {
      return four()
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

TEST_F(CompilerTest, Int32Array) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let a: Int32[10] = 8

      let i: Int32 = 0
      loop {
        if i == 10 {
          break
        }

        a[i] = i

        i = i + 1
      }

      let r: Int32 = 0
      i = 0
      loop {
        if i == 10 {
          break
        }

        r = r + a[i]

        i = i + 1
      }

      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(45));
}

TEST_F(CompilerTest, Int64Array) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int64 {
      let a: Int64[10] = 8

      let i: Int64 = 0
      loop {
        if i == 10 {
          break
        }

        a[i] = i

        i = i + 1
      }

      let r: Int64 = 0
      i = 0
      loop {
        if i == 10 {
          break
        }

        r = r + a[i]

        i = i + 1
      }

      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(45));
}

TEST_F(CompilerTest, BoolArray) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let a: Bool[10] = false

      let i: Int32 = 0
      loop {
        if i == 10 {
          break
        }

        if i % 2 == 0 {
          a[i] = true
        } else {
          a[i] = false
        }

        i = i + 1
      }

      let r: Int32 = 0
      i = 0
      loop {
        if i == 10 {
          break
        }

        if a[i] {
          r = r + 1
        }

        i = i + 1
      }

      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

}  // namespace
}  // namespace lucid
