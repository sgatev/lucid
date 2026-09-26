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

// Branches nested this deep leave blocks holding phi functions and no
// instructions, which the registers were once not coloured for. `a` is 3, so
// the branches taken are the first, the second and the second.
TEST(CompilerTest, NestedBranches) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 3
      val v: Int32 = 0
      if a == 3 {
        if a == 2 {
          if a == 1 { &v = 1 } else { &v = 2 }
        } else {
          if a == 1 { &v = 3 } else { &v = 4 }
        }
      } else {
        if a == 2 {
          if a == 1 { &v = 5 } else { &v = 6 }
        } else {
          if a == 1 { &v = 7 } else { &v = 8 }
        }
      }
      return v
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

// Nine values crossing a branch leave more live where the sides meet than
// there are registers, so the allocator spills one a phi function reads. A
// phi reads its argument where control leaves the block it comes from, so
// that is where the value is loaded back. `a` is 1, so nothing is taken off
// and the sum is of one through nine.
TEST(CompilerTest, BranchingValuesThatSpill) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 1
      val v0: Int32 = 1
      val v1: Int32 = 2
      val v2: Int32 = 3
      val v3: Int32 = 4
      val v4: Int32 = 5
      val v5: Int32 = 6
      val v6: Int32 = 7
      val v7: Int32 = 8
      val v8: Int32 = 9
      if a == 1 {
      } else {
        &v0 = v0 - 1
        &v1 = v1 - 1
        &v2 = v2 - 1
        &v3 = v3 - 1
        &v4 = v4 - 1
        &v5 = v5 - 1
        &v6 = v6 - 1
        &v7 = v7 - 1
        &v8 = v8 - 1
      }
      val sum0: Int32 = v0
      val sum1: Int32 = sum0 + v1
      val sum2: Int32 = sum1 + v2
      val sum3: Int32 = sum2 + v3
      val sum4: Int32 = sum3 + v4
      val sum5: Int32 = sum4 + v5
      val sum6: Int32 = sum5 + v6
      val sum7: Int32 = sum6 + v7
      val sum8: Int32 = sum7 + v8
      return sum8
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(45));
}

// Eight values carried around a loop crowd the ten registers, so the loop
// spills. Each turn adds one to every value and there are three turns, so
// the values end as three through ten and their sum is 52.
TEST(CompilerTest, LoopCarriedValuesThatSpill) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val i: Int32 = 0
      val v0: Int32 = 0
      val v1: Int32 = 1
      val v2: Int32 = 2
      val v3: Int32 = 3
      val v4: Int32 = 4
      val v5: Int32 = 5
      val v6: Int32 = 6
      val v7: Int32 = 7
      loop {
        if i == 3 {
          break
        }
        &v0 = v0 + 1
        &v1 = v1 + 1
        &v2 = v2 + 1
        &v3 = v3 + 1
        &v4 = v4 + 1
        &v5 = v5 + 1
        &v6 = v6 + 1
        &v7 = v7 + 1
        &i = i + 1
      }
      val s0: Int32 = v0
      val s1: Int32 = s0 + v1
      val s2: Int32 = s1 + v2
      val s3: Int32 = s2 + v3
      val s4: Int32 = s3 + v4
      val s5: Int32 = s4 + v5
      val s6: Int32 = s5 + v6
      val s7: Int32 = s6 + v7
      return s7
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(52));
}

// An operation whose two operands are both literals, standing under another
// operation, once reached the backend with no type at all and stopped the
// compiler. Neither the grouping nor the absence of it makes a difference.
TEST(CompilerTest, NestedLiteralArithmetic) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 1 + 2 * 3
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(7));
}

TEST(CompilerTest, NestedLiteralArithmeticGrouped) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return (1 + 2) * 3
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(9));
}

// Both sides of the comparison are written out of literals, so nothing in it
// has a type until the literals are given theirs.
TEST(CompilerTest, NestedLiteralComparison) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      if 1 + 2 * 3 > 5 {
        return 1
      }
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(1));
}

// A `break` leaves the loop standing closest over it. An unconditional one
// in an inner loop once took the answer the inner body gave for the outer
// sequence as well, which cut the outer loop's own back edge and left it
// running once.
TEST(CompilerTest, BreakLeavesTheInnermostLoop) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val i: Int32 = 0
      val n: Int32 = 0
      loop {
        &i = i + 1
        loop {
          &n = n + 1
          break
        }
        if i == 3 {
          break
        }
      }
      return i * 10 + n
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(33));
}

TEST(CompilerTest, BreakLeavesTheInnermostOfThreeLoops) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val i: Int32 = 0
      val j: Int32 = 0
      val k: Int32 = 0
      loop {
        &i = i + 1
        loop {
          &j = j + 1
          loop {
            &k = k + 1
            break
          }
          break
        }
        if i == 2 {
          break
        }
      }
      return i * 100 + j * 10 + k
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(222));
}

// A declaration inside a block stands over an outer one of the same name
// only while that block lasts. Reading `y` after the branch once gave the
// two, because both declarations went by the one name.
TEST(CompilerTest, ShadowedVariable) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val y: Int32 = 1
      if true {
        val y: Int32 = 2
      }
      return y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(1));
}

// A parameter is declared where the function is, so a local of the same name
// stands over it for as long as its own block lasts.
TEST(CompilerTest, LocalShadowingAParameter) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun f(a: Int32): Int32 {
      val a: Int32 = 9
      return a
    }

    fun main(): Int32 {
      return f(1)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(9));
}

// Two blocks beside one another each declaring the same name, with different
// types, which one map from names to types could not hold at once.
TEST(CompilerTest, SameNameInBothBranches) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 1
      if a == 1 {
        val b: Int32 = 2
      } else {
        val b: Int64 = 3
      }
      return 4
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

// The initializer stands before the declaration it belongs to, so the name
// in it is the one that was in force until then.
TEST(CompilerTest, DeclarationReadingTheEarlierOne) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val x: Int32 = 2
      val x: Int32 = x + 3
      return x
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

// An array whose elements are tuples: the slots behind it run field by field
// and element by element, and reaching into it costs the element's whole size
// per step rather than one slot.
TEST(CompilerTest, ArrayOfTuples) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    val Point: Type = (x: Int32, y: Int32)

    fun main(): Int32 {
      val ps: Point[3]
      &ps[0].x = 7
      &ps[0].y = 1
      &ps[2].x = 9
      return ps[0].x + ps[2].x
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(16));
}

// The same, indexed by something only known as it runs.
TEST(CompilerTest, ArrayOfTuplesIndexedByVariable) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    val Point: Type = (x: Int32, y: Int32)

    fun main(): Int32 {
      val ps: Point[3]
      val i: Int32 = 2
      &ps[i].y = 6
      return ps[i].y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(6));
}

TEST(CompilerTest, TupleInTuple) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    val Inner: Type = (x: Int32)
    val Outer: Type = (i: Inner, n: Int32)

    fun main(): Int32 {
      val o: Outer
      &o.i.x = 5
      &o.n = 2
      return o.i.x * o.n
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(10));
}

TEST(CompilerTest, TupleHoldingAnArray) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    val Row: Type = (v: Int32[4], n: Int32)

    fun main(): Int32 {
      val r: Row
      &r.n = 5
      return r.n + r.v[0] - r.v[0]
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST(CompilerTest, Grouping) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 1
      val b: Int32 = 2
      val c: Int32 = 3
      return (a + b) * c
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(9));
}

TEST(CompilerTest, GroupingIsNotPrecedence) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 1
      val b: Int32 = 2
      val c: Int32 = 3
      return a + b * c
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(7));
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

TEST(CompilerTest, Int64LiteralBeyondInt32) {
  // A literal too wide for an `Int32` is still a value an `Int64` holds, and
  // it reaches the program whole rather than as its lower half.
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val big: Int64 = 4294967296
      val low: Int64 = 1
      if big > low {
        return 7
      }
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(7));
}

TEST(CompilerTest, Int64LiteralKeepsItsLowerHalf) {
  // 2^32 + 5 and 5 differ only above the 32nd bit.
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val big: Int64 = 4294967301
      val small: Int64 = 5
      if big == small {
        return 1
      }
      return 2
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(2));
}

TEST(CompilerTest, IntLiteralOutOfRangeForItsType) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val x: Int32 = 4294967296
      return x
    }
  )"));
  EXPECT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              AllOf(ReturnsCode(1),
                    ErrorOutput(Contains(
                        "integer literal out of range for type Int32"))));
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
