#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/compiler/compiler_test_fixture.h"

namespace lucid {
namespace {

using ::testing::AllOf;

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

TEST_F(CompilerTest, IfStmtBothBranches) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let foo = (b: Bool, n: Int32) -> Int32 {
      if b {
        n = n + 1
      } else {
        n = n + 2
      }
      return n
    }

    let main = () -> Int32 {
      return foo(true, 2) + foo(false, 3)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(8));
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
      let a: Int32[10]

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
      let a: Int64[10]

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
      let a: Bool[10]

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

TEST_F(CompilerTest, LongDependencyChain) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let a1: Int32 = 1
      let a2: Int32 = a1 + 1
      let a3: Int32 = a2 + 1
      let a4: Int32 = a3 + 1
      let a5: Int32 = a4 + 1
      let a6: Int32 = a5 + 1
      let a7: Int32 = a6 + 1
      let a8: Int32 = a7 + 1
      let a9: Int32 = a8 + 1
      let a10: Int32 = a9 + 1
      let a11: Int32 = a10 + 1
      let a12: Int32 = a11 + 1
      let a13: Int32 = a12 + 1
      let a14: Int32 = a13 + 1
      let a15: Int32 = a14 + 1
      let a16: Int32 = a15 + 1
      let a17: Int32 = a16 + 1
      let a18: Int32 = a17 + 1
      let a19: Int32 = a18 + 1
      let a20: Int32 = a19 + 1
      return a20
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(20));
}

TEST_F(CompilerTest, ManyLiveVariables) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      let a1: Int32 = 1
      let a2: Int32 = 2
      let a3: Int32 = 3
      let a4: Int32 = 4
      let a5: Int32 = 5
      let a6: Int32 = 6
      let a7: Int32 = 7
      let a8: Int32 = 8
      let a9: Int32 = 9
      let a10: Int32 = 10
      let a11: Int32 = 11
      let a12: Int32 = 12
      let a13: Int32 = 13
      let a14: Int32 = 14
      let a15: Int32 = 15
      let a16: Int32 = 16
      let a17: Int32 = 17
      let a18: Int32 = 18
      let a19: Int32 = 19
      let a20: Int32 = 20
      return a1  + a2  + a3  + a4  + a5 +
             a6  + a7  + a8  + a9  + a10 +
             a11 + a12 + a13 + a14 + a15 +
             a16 + a17 + a18 + a19 + a20
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(210));
}

TEST_F(CompilerTest, Comp) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp let max = (a: Int32, b: Int32) -> Int32 {
      let m: Int32 = a
      if b > m {
        m = b
      }
      return m
    }

    let main = () -> Int32 {
      comp let round1: Int32 = max(21, 105)
      comp let round2: Int32 = max(210, round1)
      return round2
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(210));
}

TEST_F(CompilerTest, Tuple) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let Point = tuple (x: Int32, y: Int32)

    let main = () -> Int32 {
      let p: Point
      p.x = 21
      p.y = 42
      return p.x + p.y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(63));
}

}  // namespace
}  // namespace lucid
