#include "lucid/compiler/compiler_test_fixture.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(CompilerTest, Build) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", FullPath("main"), FullPath("main.lu")}),
              ReturnsCode(0));
  EXPECT_THAT(RunBinary(FullPath("main")), ReturnsCode(0));
}

TEST(CompilerTest, EmptyMain) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(0));
}

TEST(CompilerTest, Comment) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    # comment
    fun main(): Int32 {
      return 21 # comment
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST(CompilerTest, AddInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 2 + 3
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST(CompilerTest, AddInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      return 2 + 3
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST(CompilerTest, SubInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 7 - 5
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, SubInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      return 7 - 5
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, MulInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 3 * 7
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST(CompilerTest, MulInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      return 3 * 7
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST(CompilerTest, DivInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 8 / 2
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

TEST(CompilerTest, DivInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      return 8 / 2
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

TEST(CompilerTest, ModInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 23 % 7
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, ModInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      return 17 % 5
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, IfStmtThenBranch) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if true {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, IfStmtElseIfBranch) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val x: Int32 = 2
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

TEST(CompilerTest, IfStmtElseBranch) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if false {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST(CompilerTest, IfStmtBothBranches) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun foo(b: Bool, n: Int32): Int32 {
      if b {
        &n = n + 1
      } else {
        &n = n + 2
      }
      return n
    }

    fun main(): Int32 {
      return foo(true, 2) + foo(false, 3)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(8));
}

TEST(CompilerTest, GtInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if 7 > 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, GtInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      if 7 > 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, GtFalse) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if 1 > 7 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST(CompilerTest, LtInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if 1 < 7 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, LtInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      if 1 < 7 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, LtFalse) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if 7 < 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST(CompilerTest, EqInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if 1 == 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, EqInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      if 1 == 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, NotEqInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if 1 != 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST(CompilerTest, NotEqInt64) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      if 1 != 1 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST(CompilerTest, EqFalse) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if 1 == 2 {
        return 2
      } else {
        return 3
      }
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST(CompilerTest, VarDecl) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val x: Int32 = 2
      val y: Int32 = 3
      return x + y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST(CompilerTest, VarDeclFromVar) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val x: Int32 = 2
      val y: Int32 = x
      return y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, VarAssign) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun foo(n: Int32): Int32 {
      &n = 3
      return n
    }

    fun main(): Int32 {
      return foo(2)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST(CompilerTest, FuncCallSingleArg) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun id(x: Int32): Int32 {
      return x
    }

    fun main(): Int32 {
      return id(21)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST(CompilerTest, FuncCallArgsSameType) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun sum(x: Int32, y: Int32, z: Int32): Int32 {
      return x + y + z
    }

    fun main(): Int32 {
      return sum(2, 3, 5)
    }
  )"));
  ASSERT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(10));
}

TEST(CompilerTest, FactRec) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun fact(n: Int32): Int32 {
      if n == 1 {
        return 1
      } else {
        return fact(n-1) * n
      }
    }

    fun main(): Int32 {
      return fact(5)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(120));
}

TEST(CompilerTest, FibRec) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun fib(n: Int32): Int32 {
      if n < 2 {
        return n
      }
      return fib(n-1) + fib(n-2)
    }

    fun main(): Int32 {
      return fib(8)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST(CompilerTest, FibIter) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun fib(n: Int32): Int32 {
      val a: Int32 = 0
      val b: Int32 = 1
      loop {
        if n == 0 {
          return a
        }

        val c: Int32 = a
        &a = b
        &b = c + b
        &n = n - 1
      }
    }

    fun main(): Int32 {
      return fib(8)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST(CompilerTest, PrintInt32) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun printInt32(i: Int32): Int32 {
      if i > 9 {
        do printInt32(i / 10)
      }

      val j: Int32 = i % 10
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

    fun main(): Int32 {
      do printInt32(21509)
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Output(Equals("21509"))));
}

TEST(CompilerTest, PrintString) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun main(): Int32 {
      do printString("Hello, world!\n")
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Output(Equals("Hello, world!\n"))));
}

TEST(CompilerTest, PrintMultipleValues) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun printInt32(i: Int32): Int32 {
      if i > 9 { do printInt32(i / 10) }

      val j: Int32 = i % 10
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

    fun main(): Int32 {
      do printInt32(0)
      do printString(", ")
      do printInt32(1)
      do printString("\n")
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Output(Equals("0, 1\n"))));
}

TEST(CompilerTest, LoopAndBreak) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun four(): Int32 {
      val n: Int32 = 0
      loop {
        if n > 3 {
          break
        }

        &n = n + 1
      }
      return n
    }

    fun main(): Int32 {
      return four()
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

TEST(CompilerTest, Int32Array) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32[10]

      val i: Int32 = 0
      loop {
        if i == 10 {
          break
        }

        &a[i] = i

        &i = i + 1
      }

      val r: Int32 = 0
      &i = 0
      loop {
        if i == 10 {
          break
        }

        &r = r + a[i]

        &i = i + 1
      }

      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(45));
}

TEST(CompilerTest, Int64Array) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      val a: Int64[10]

      val i: Int64 = 0
      loop {
        if i == 10 {
          break
        }

        &a[i] = i

        &i = i + 1
      }

      val r: Int64 = 0
      &i = 0
      loop {
        if i == 10 {
          break
        }

        &r = r + a[i]

        &i = i + 1
      }

      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(45));
}

TEST(CompilerTest, BoolArray) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Bool[10]

      val i: Int32 = 0
      loop {
        if i == 10 {
          break
        }

        if i % 2 == 0 {
          &a[i] = true
        } else {
          &a[i] = false
        }

        &i = i + 1
      }

      val r: Int32 = 0
      &i = 0
      loop {
        if i == 10 {
          break
        }

        if a[i] {
          &r = r + 1
        }

        &i = i + 1
      }

      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST(CompilerTest, LongDependencyChain) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a1: Int32 = 1
      val a2: Int32 = a1 + 1
      val a3: Int32 = a2 + 1
      val a4: Int32 = a3 + 1
      val a5: Int32 = a4 + 1
      val a6: Int32 = a5 + 1
      val a7: Int32 = a6 + 1
      val a8: Int32 = a7 + 1
      val a9: Int32 = a8 + 1
      val a10: Int32 = a9 + 1
      val a11: Int32 = a10 + 1
      val a12: Int32 = a11 + 1
      val a13: Int32 = a12 + 1
      val a14: Int32 = a13 + 1
      val a15: Int32 = a14 + 1
      val a16: Int32 = a15 + 1
      val a17: Int32 = a16 + 1
      val a18: Int32 = a17 + 1
      val a19: Int32 = a18 + 1
      val a20: Int32 = a19 + 1
      return a20
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(20));
}

TEST(CompilerTest, ManyLiveVariables) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a1: Int32 = 1
      val a2: Int32 = 2
      val a3: Int32 = 3
      val a4: Int32 = 4
      val a5: Int32 = 5
      val a6: Int32 = 6
      val a7: Int32 = 7
      val a8: Int32 = 8
      val a9: Int32 = 9
      val a10: Int32 = 10
      val a11: Int32 = 11
      val a12: Int32 = 12
      val a13: Int32 = 13
      val a14: Int32 = 14
      val a15: Int32 = 15
      val a16: Int32 = 16
      val a17: Int32 = 17
      val a18: Int32 = 18
      val a19: Int32 = 19
      val a20: Int32 = 20
      return a1  + a2  + a3  + a4  + a5 +
             a6  + a7  + a8  + a9  + a10 +
             a11 + a12 + a13 + a14 + a15 +
             a16 + a17 + a18 + a19 + a20
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(210));
}

TEST(CompilerTest, Comp) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun max(a: Int32, b: Int32): Int32 {
      val m: Int32 = a
      if b > m {
        &m = b
      }
      return m
    }

    fun main(): Int32 {
      comp val round1: Int32 = max(21, 105)
      comp val round2: Int32 = max(210, round1)
      return round2
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(210));
}

TEST(CompilerTest, Tuple) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp val Point: Type = (x: Int32, y: Int32)

    fun main(): Int32 {
      val p: Point
      &p.x = 21
      &p.y = 42
      return p.x + p.y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(63));
}

TEST(CompilerTest, LargeInteger) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun count3s(i: Int32): Int32 {
      val count: Int32 = 0
      loop {
        if i == 0 {
          break
        }
        val r: Int32 = i % 10
        if r == 3 {
          &count = count + 1
        }
        &i = i / 10
      }
      return count
    }

    fun main(): Int32 {
      return count3s(73633723)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

TEST(CompilerTest, CompEvaluatesValuesTooLargeToCarry) {
  // The result of the call is larger than an instruction can hold, so it
  // becomes a constant the program loads rather than one it carries.
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun twice(a: Int32): Int32 {
      return a + a
    }

    fun main(): Int32 {
      comp val big: Int32 = twice(40000)
      return big - 79999
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(1));
}

TEST(CompilerTest, CompEvaluatesNegativeValues) {
  // A negative result is one no instruction can carry either.
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun diff(a: Int32, b: Int32): Int32 {
      return a - b
    }

    fun main(): Int32 {
      comp val neg: Int32 = diff(1, 5)
      return neg + 9
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

}  // namespace
}  // namespace lucid
