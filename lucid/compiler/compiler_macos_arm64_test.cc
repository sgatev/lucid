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
      mut val v: Int32 = 0
      if a == 3 {
        if a == 2 {
          if a == 1 { mut v = 1 } else { mut v = 2 }
        } else {
          if a == 1 { mut v = 3 } else { mut v = 4 }
        }
      } else {
        if a == 2 {
          if a == 1 { mut v = 5 } else { mut v = 6 }
        } else {
          if a == 1 { mut v = 7 } else { mut v = 8 }
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
      mut val v0: Int32 = 1
      mut val v1: Int32 = 2
      mut val v2: Int32 = 3
      mut val v3: Int32 = 4
      mut val v4: Int32 = 5
      mut val v5: Int32 = 6
      mut val v6: Int32 = 7
      mut val v7: Int32 = 8
      mut val v8: Int32 = 9
      if a == 1 {
      } else {
        mut v0 = v0 - 1
        mut v1 = v1 - 1
        mut v2 = v2 - 1
        mut v3 = v3 - 1
        mut v4 = v4 - 1
        mut v5 = v5 - 1
        mut v6 = v6 - 1
        mut v7 = v7 - 1
        mut v8 = v8 - 1
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
      mut val i: Int32 = 0
      mut val v0: Int32 = 0
      mut val v1: Int32 = 1
      mut val v2: Int32 = 2
      mut val v3: Int32 = 3
      mut val v4: Int32 = 4
      mut val v5: Int32 = 5
      mut val v6: Int32 = 6
      mut val v7: Int32 = 7
      loop {
        if i == 3 {
          break
        }
        mut v0 = v0 + 1
        mut v1 = v1 + 1
        mut v2 = v2 + 1
        mut v3 = v3 + 1
        mut v4 = v4 + 1
        mut v5 = v5 + 1
        mut v6 = v6 + 1
        mut v7 = v7 + 1
        mut i = i + 1
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

// A string is held as where it stands among the strings the program
// carries, so what puts a worked-out one in a register is the instruction
// that sets a string rather than the one that sets a number. Which of the
// two it is cannot be told from the value or its width, only from the type
// the call was written with.
TEST(CompilerTest, CompValueOfAString) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    comp fun pick(n: Int32): String {
      if n > 1 {
        return "big"
      }
      return "small"
    }

    fun main(): Int32 {
      val a: String = comp pick(5)
      val b: String = comp pick(0)
      do printString(a)
      do printString(b)
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Output(Equals("bigsmall"))));
}

// Work done during compilation can have no effect of its own, so leaving
// the right side unread there would not be worth a branch, and reading both
// is what lets the value be worked out where every other comp value is.
TEST(CompilerTest, ShortCircuitInACompValue) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun t(): Bool {
      return true
    }

    comp fun f(): Bool {
      return false
    }

    comp fun n(): Int32 {
      return 7
    }

    fun main(): Int32 {
      val both: Bool = comp (t() and f())
      val either: Bool = comp (t() or f())
      val mixed: Bool = comp (n() > 3 and t())
      mut val count: Int32 = 0
      if both {
        mut count = count + 1
      }
      if either {
        mut count = count + 2
      }
      if mixed {
        mut count = count + 4
      }
      return count
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(6));
}

// Both operators over both of their sides, counted into one number so that
// a single answer says all four came out right.
// A whole function is run to work out what a comp value is, and running one
// follows branches, so a short circuit inside a comp function is fine where
// one standing in the value itself is not.
TEST(CompilerTest, ShortCircuitInsideACompFunction) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun p(): Bool {
      return true
    }

    comp fun q(): Bool {
      return true
    }

    comp fun both(): Bool {
      return p() and q()
    }

    fun main(): Int32 {
      val c: Bool = comp both()
      if c {
        return 5
      }
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

TEST(CompilerTest, ConjunctionAndDisjunction) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val t: Bool = true
      val f: Bool = false
      mut val n: Int32 = 0
      if t and t {
        mut n = n + 1
      }
      if t and f {
        mut n = n + 2
      }
      if f or t {
        mut n = n + 4
      }
      if f or f {
        mut n = n + 8
      }
      if !f {
        mut n = n + 16
      }
      return n
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

// The right side is read only where the left side leaves the answer open,
// which is what each of these writes out as it goes.
TEST(CompilerTest, ShortCircuitSkipsTheRightSide) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun p(): Bool {
      do printString("p")
      return true
    }

    fun q(): Bool {
      do printString("q")
      return true
    }

    fun no(): Bool {
      do printString("n")
      return false
    }

    fun main(): Int32 {
      if no() and q() {
        return 0
      }
      if p() or q() {
        return 0
      }
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Output(Equals("np"))));
}

// A call standing before the operator in the same expression is read before
// it, which is what taking the operator out of the expression could undo.
TEST(CompilerTest, ShortCircuitKeepsTheOrderOfCalls) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun p(): Bool {
      do printString("p")
      return true
    }

    fun q(): Bool {
      do printString("q")
      return true
    }

    fun no(): Bool {
      do printString("n")
      return false
    }

    fun first(): Int32 {
      do printString("f")
      return 1
    }

    fun pick(b: Bool): Int32 {
      do printString("k")
      return 1
    }

    fun main(): Int32 {
      val c: Int32 = first() + pick(p() and q())
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Output(Equals("fpqk"))));
}

// `or` binds looser than `and`, so the conjunction is what the disjunction
// holds and neither side of it is read where the left side already holds.
TEST(CompilerTest, ConjunctionBindsTighterThanDisjunction) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun p(): Bool {
      do printString("p")
      return true
    }

    fun q(): Bool {
      do printString("q")
      return true
    }

    fun no(): Bool {
      do printString("n")
      return false
    }

    fun main(): Int32 {
      if p() or no() and q() {
        return 0
      }
      return 0
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Output(Equals("p"))));
}

// Both sides of each comparison, and the point where they meet, which is
// the whole of what `>=` and `<=` add over `>` and `<`.
TEST(CompilerTest, GreaterOrEqualAndLessOrEqual) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val lo: Int32 = 1
      val hi: Int32 = 2
      mut val n: Int32 = 0
      if lo >= hi {
        mut n = n + 1
      }
      if hi >= lo {
        mut n = n + 2
      }
      if lo >= lo {
        mut n = n + 4
      }
      if lo <= hi {
        mut n = n + 8
      }
      if hi <= lo {
        mut n = n + 16
      }
      if lo <= lo {
        mut n = n + 32
      }
      return n
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(46));
}

// Held in a variable rather than branched on, which the backend sets with
// CSET instead of folding into the branch.
TEST(CompilerTest, ComparisonHeldInAVariable) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 5
      val at_least: Bool = a >= 5
      val at_most: Bool = a <= 4
      mut val n: Int32 = 0
      if at_least {
        mut n = n + 3
      }
      if at_most {
        mut n = n + 4
      }
      return n
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

TEST(CompilerTest, ComparisonOfWiderAndSignedValues) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int64 = 5000000000
      val b: Int64 = 4999999999
      val neg: Int32 = 0 - 5
      mut val n: Int32 = 0
      if a >= b {
        mut n = n + 1
      }
      if neg <= 0 {
        mut n = n + 2
      }
      return n
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

// An array or a tuple lives in the frame, which the interpreter that runs
// comp code once had none of: what it stored went nowhere and what it read
// back was zero.
TEST(CompilerTest, CompFunctionUsingAnArray) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun squares(n: Int32): Int32 {
      mut val a: Int32[4]
      mut val i: Int32 = 0
      loop {
        if i == 4 {
          break
        }
        mut a[i] = i * i
        mut i = i + 1
      }
      return a[n]
    }

    fun main(): Int32 {
      val c: Int32 = comp squares(3)
      return c
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(9));
}

// A `comp` reaches one element, so a value worked out during compilation can
// stand beside one that is not without a declaration between them.
TEST(CompilerTest, CompExprBesideARuntimeValue) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun twenty(): Int32 {
      return 20
    }

    fun main(): Int32 {
      val n: Int32 = 1
      return comp twenty() + n
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

// Parentheses are what give a `comp` more than one element to work on.
TEST(CompilerTest, CompExprOverAGroup) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun twenty(): Int32 {
      return 20
    }

    comp fun one(): Int32 {
      return 1
    }

    fun main(): Int32 {
      return comp (twenty() + one())
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

// Nothing has to be declared for a value to be worked out during compilation:
// a `comp` stands where the expression it works out stands.
TEST(CompilerTest, CompExprAsAnArgument) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun twenty(): Int32 {
      return 20
    }

    fun plusOne(a: Int32): Int32 {
      return a + 1
    }

    fun main(): Int32 {
      return plusOne(comp twenty())
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

// A comp function is one that may be called during compilation rather than
// one that has to be, and without a `comp` the call is made while the program
// runs.
TEST(CompilerTest, CompFunctionCalledWhileRunning) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun twentyOne(): Int32 {
      return 21
    }

    fun main(): Int32 {
      return twentyOne()
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

TEST(CompilerTest, CompFunctionUsingATuple) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    val Point: Type = (x: Int32, y: Int32)

    comp fun area(): Int32 {
      mut val p: Point
      mut p.x = 3
      mut p.y = 7
      return p.x * p.y
    }

    fun main(): Int32 {
      val c: Int32 = comp area()
      return c
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

// A `break` leaves the loop standing closest over it. An unconditional one
// in an inner loop once took the answer the inner body gave for the outer
// sequence as well, which cut the outer loop's own back edge and left it
// running once.
TEST(CompilerTest, BreakLeavesTheInnermostLoop) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      mut val i: Int32 = 0
      mut val n: Int32 = 0
      loop {
        mut i = i + 1
        loop {
          mut n = n + 1
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
      mut val i: Int32 = 0
      mut val j: Int32 = 0
      mut val k: Int32 = 0
      loop {
        mut i = i + 1
        loop {
          mut j = j + 1
          loop {
            mut k = k + 1
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
      mut val ps: Point[3]
      mut ps[0].x = 7
      mut ps[0].y = 1
      mut ps[2].x = 9
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
      mut val ps: Point[3]
      val i: Int32 = 2
      mut ps[i].y = 6
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
      mut val o: Outer
      mut o.i.x = 5
      mut o.n = 2
      return o.i.x * o.n
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(10));
}

TEST(CompilerTest, TupleHoldingAnArray) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    val Row: Type = (v: Int32[4], n: Int32)

    fun main(): Int32 {
      mut val r: Row
      mut r.n = 5
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
    fun foo(b: Bool, mut n: Int32): Int32 {
      if b {
        mut n = n + 1
      } else {
        mut n = n + 2
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
    fun foo(mut n: Int32): Int32 {
      mut n = 3
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
    fun fib(mut n: Int32): Int32 {
      mut val a: Int32 = 0
      mut val b: Int32 = 1
      loop {
        if n == 0 {
          return a
        }

        val c: Int32 = a
        mut a = b
        mut b = c + b
        mut n = n - 1
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
      mut val n: Int32 = 0
      loop {
        if n > 3 {
          break
        }

        mut n = n + 1
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
      mut val a: Int32[10]

      mut val i: Int32 = 0
      loop {
        if i == 10 {
          break
        }

        mut a[i] = i

        mut i = i + 1
      }

      mut val r: Int32 = 0
      mut i = 0
      loop {
        if i == 10 {
          break
        }

        mut r = r + a[i]

        mut i = i + 1
      }

      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(45));
}

TEST(CompilerTest, Int64Array) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int64 {
      mut val a: Int64[10]

      mut val i: Int64 = 0
      loop {
        if i == 10 {
          break
        }

        mut a[i] = i

        mut i = i + 1
      }

      mut val r: Int64 = 0
      mut i = 0
      loop {
        if i == 10 {
          break
        }

        mut r = r + a[i]

        mut i = i + 1
      }

      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(45));
}

TEST(CompilerTest, BoolArray) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      mut val a: Bool[10]

      mut val i: Int32 = 0
      loop {
        if i == 10 {
          break
        }

        if i % 2 == 0 {
          mut a[i] = true
        } else {
          mut a[i] = false
        }

        mut i = i + 1
      }

      mut val r: Int32 = 0
      mut i = 0
      loop {
        if i == 10 {
          break
        }

        if a[i] {
          mut r = r + 1
        }

        mut i = i + 1
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
      mut val m: Int32 = a
      if b > m {
        mut m = b
      }
      return m
    }

    fun main(): Int32 {
      val round1: Int32 = comp max(21, 105)
      val round2: Int32 = comp max(210, round1)
      return round2
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(210));
}

TEST(CompilerTest, Tuple) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp val Point: Type = (x: Int32, y: Int32)

    fun main(): Int32 {
      mut val p: Point
      mut p.x = 21
      mut p.y = 42
      return p.x + p.y
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(63));
}

TEST(CompilerTest, LargeInteger) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun count3s(mut i: Int32): Int32 {
      mut val count: Int32 = 0
      loop {
        if i == 0 {
          break
        }
        val r: Int32 = i % 10
        if r == 3 {
          mut count = count + 1
        }
        mut i = i / 10
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
      val big: Int32 = comp twice(40000)
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
      val neg: Int32 = comp diff(1, 5)
      return neg + 9
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(5));
}

// Eleven parameters against ten registers. The eleventh arrives on the stack,
// because a parameter is live where the function is entered and spilling one
// cannot buy room there: the store it would put in comes after the entry.
TEST(CompilerTest, ParameterPastTheRegistersArrivesOnTheStack) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun total(a0: Int32, a1: Int32, a2: Int32, a3: Int32, a4: Int32,
              a5: Int32, a6: Int32, a7: Int32, a8: Int32, a9: Int32,
              a10: Int32): Int32 {
      return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10
    }

    fun main(): Int32 {
      return total(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(66));
}

TEST(CompilerTest, ManyParametersArriveOnTheStack) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun total(a0: Int32, a1: Int32, a2: Int32, a3: Int32, a4: Int32,
              a5: Int32, a6: Int32, a7: Int32, a8: Int32, a9: Int32,
              a10: Int32, a11: Int32, a12: Int32, a13: Int32, a14: Int32,
              a15: Int32, a16: Int32, a17: Int32, a18: Int32,
              a19: Int32): Int32 {
      return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 +
             a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19
    }

    fun main(): Int32 {
      return total(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                   18, 19, 20)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(210));
}

// A parameter on the stack is read where it is read, rather than held in a
// register from the entry, so reading it more than once reads it more than
// once.
TEST(CompilerTest, ParameterOnTheStackIsReadAtEveryUse) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun thrice(a0: Int32, a1: Int32, a2: Int32, a3: Int32, a4: Int32,
               a5: Int32, a6: Int32, a7: Int32, a8: Int32, a9: Int32,
               a10: Int32): Int32 {
      return a10 + a10 + a10
    }

    fun main(): Int32 {
      return thrice(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

// A parameter on the stack carried around a loop, where it reaches a phi
// function as one of its arguments.
TEST(CompilerTest, ParameterOnTheStackReachesAPhiFunction) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun count(a0: Int32, a1: Int32, a2: Int32, a3: Int32, a4: Int32,
              a5: Int32, a6: Int32, a7: Int32, a8: Int32, a9: Int32,
              a10: Int32): Int32 {
      mut val acc: Int32 = a10
      mut val i: Int32 = 0
      loop {
        if i >= a0 {
          break
        }
        mut acc = acc + 1
        mut i = i + 1
      }
      return acc
    }

    fun main(): Int32 {
      return count(4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(104));
}

// A function that both takes parameters on the stack and passes some on, so
// its frame holds room for what it was given and for what it gives.
TEST(CompilerTest, StackArgumentsPassedOnThroughAFrameThatHasItsOwn) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun inner(a0: Int32, a1: Int32, a2: Int32, a3: Int32, a4: Int32,
              a5: Int32, a6: Int32, a7: Int32, a8: Int32, a9: Int32,
              a10: Int32, a11: Int32): Int32 {
      return a10 * 10 + a11
    }

    fun outer(b0: Int32, b1: Int32, b2: Int32, b3: Int32, b4: Int32,
              b5: Int32, b6: Int32, b7: Int32, b8: Int32, b9: Int32,
              b10: Int32, b11: Int32): Int32 {
      return inner(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, b10, b11) + b0
    }

    fun main(): Int32 {
      return outer(3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 9)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(62));
}

// A caller with slots of its own, so that what it leaves for the call stands
// apart from what it keeps for itself.
TEST(CompilerTest, StackArgumentsBesideACallerSOwnSlots) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun last(a0: Int32, a1: Int32, a2: Int32, a3: Int32, a4: Int32,
             a5: Int32, a6: Int32, a7: Int32, a8: Int32, a9: Int32,
             a10: Int32): Int32 {
      return a10
    }

    fun main(): Int32 {
      mut val buf: Int32[4]
      mut buf[0] = 11
      mut buf[3] = 22
      return last(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40) + buf[0] + buf[3]
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(73));
}

// A parameter on the stack that is a pointer rather than a number, which is
// read back at its own width.
TEST(CompilerTest, ParameterOnTheStackKeepsItsWidth) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun tell(a0: Int32, a1: Int32, a2: Int32, a3: Int32, a4: Int32,
             a5: Int32, a6: Int32, a7: Int32, a8: Int32, a9: Int32,
             s: String, n: Int32): Int32 {
      do printString(s)
      return n + a0
    }

    fun main(): Int32 {
      return tell(2, 0, 0, 0, 0, 0, 0, 0, 0, 0, "", 5)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(7));
}

// Worked out while compiling, where nothing has been given a register and the
// arguments past the tenth are held wherever the interpreter holds them.
TEST(CompilerTest, CompEvaluatesACallWithArgumentsOnTheStack) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun total(a0: Int32, a1: Int32, a2: Int32, a3: Int32, a4: Int32,
                   a5: Int32, a6: Int32, a7: Int32, a8: Int32, a9: Int32,
                   a10: Int32, a11: Int32): Int32 {
      return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11
    }

    fun main(): Int32 {
      val c: Int32 = comp total(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)
      return c
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(78));
}

// A parameter the body never reads still arrives in a register of its own.
// The entry fills them one after another, so one sharing with a parameter it
// has not filled yet would write over it.
TEST(CompilerTest, AParameterThatIsNotReadKeepsItsOwnRegister) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun first(a: Int32, b: Int32): Int32 {
      return a
    }

    fun main(): Int32 {
      return first(7, 9)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(7));
}

// A slot of the frame narrower than a register once began where the last
// register the function hands back began, rather than where it ends, so the
// first of them stood inside it. The caller got that register back with its
// upper half written over, which shows where the caller was holding
// something wider than the half that survived.
TEST(CompilerTest, ASlotOfTheFrameStandsClearOfTheRegistersHandedBack) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun narrow(): Int32 {
      mut val buf: Int32[2]
      mut buf[0] = 12345
      mut buf[1] = 2
      return buf[0] + buf[1]
    }

    fun main(): Int32 {
      val a: String = ""
      val b: String = " "
      val c: String = "  "
      val d: String = "   "
      val e: String = "    "
      val f: String = "     "
      val g: String = "      "
      val h: String = "       "
      val i: String = "        "
      val r: Int32 = narrow()
      do printString(a)
      do printString(b)
      do printString(c)
      do printString(d)
      do printString(e)
      do printString(f)
      do printString(g)
      do printString(h)
      do printString(i)
      return r
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(59));
}

// A frame wider than the field that reserves it, and a slot further from the
// stack pointer than the field that reaches one. Both are asked for in as
// many instructions as they take, where a single instruction once carried
// whatever was left of the value after the field had taken what it could.
TEST(CompilerTest, AFrameWiderThanTheFieldThatReservesIt) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun wide(): Int32 {
      mut val buf: Int32[5000]
      mut buf[0] = 7
      mut buf[4999] = 11

      # More live at once than there are registers, so that what is put away
      # stands above the array, out of reach of the field.
      val a: Int32 = 1
      val b: Int32 = 2
      val c: Int32 = 3
      val d: Int32 = 4
      val e: Int32 = 5
      val f: Int32 = 6
      val g: Int32 = 7
      val h: Int32 = 8
      val i: Int32 = 9
      val j: Int32 = 10
      val k: Int32 = 11
      val l: Int32 = 12
      val sum: Int32 = a + b + c + d + e + f + g + h + i + j + k + l
      return buf[0] + buf[4999] + sum
    }

    fun main(): Int32 {
      return wide()
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(96));
}

// Ten values crossing a branch, which is as many as there are registers.
// A phi function's result sharing the register its argument is already in is
// what leaves room for them.
TEST(CompilerTest, TenValuesCrossABranch) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 1
      mut val v0: Int32 = 1
      mut val v1: Int32 = 2
      mut val v2: Int32 = 3
      mut val v3: Int32 = 4
      mut val v4: Int32 = 5
      mut val v5: Int32 = 6
      mut val v6: Int32 = 7
      mut val v7: Int32 = 8
      mut val v8: Int32 = 9
      mut val v9: Int32 = 10
      # Every one of them is written on both sides, so every one has a phi
      # function where the sides meet.
      if a == 1 {
        mut v0 = v0 + 10
        mut v1 = v1 + 10
        mut v2 = v2 + 10
        mut v3 = v3 + 10
        mut v4 = v4 + 10
        mut v5 = v5 + 10
        mut v6 = v6 + 10
        mut v7 = v7 + 10
        mut v8 = v8 + 10
        mut v9 = v9 + 10
      } else {
        mut v0 = v0 - 1
        mut v1 = v1 - 1
        mut v2 = v2 - 1
        mut v3 = v3 - 1
        mut v4 = v4 - 1
        mut v5 = v5 - 1
        mut v6 = v6 - 1
        mut v7 = v7 - 1
        mut v8 = v8 - 1
        mut v9 = v9 - 1
      }
      return v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(155));
}

// Two values that trade places every turn of a loop. Their phi functions
// take each other's registers, so the copies that settle them run in a
// circle and one of them has to stand somewhere else while the rest move.
TEST(CompilerTest, ValuesThatTradePlacesAroundALoop) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      mut val a: Int32 = 1
      mut val b: Int32 = 2
      mut val i: Int32 = 0
      loop {
        if i >= 5 {
          break
        }
        val t: Int32 = a
        mut a = b
        mut b = t
        mut i = i + 1
      }
      return a * 10 + b
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(21));
}

// Three of them, so the circle is longer than a single trade.
TEST(CompilerTest, ValuesThatMoveAroundALoopInACircle) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      mut val a: Int32 = 1
      mut val b: Int32 = 2
      mut val c: Int32 = 3
      mut val i: Int32 = 0
      loop {
        if i >= 4 {
          break
        }
        val t: Int32 = a
        mut a = b
        mut b = c
        mut c = t
        mut i = i + 1
      }
      return a * 100 + b * 10 + c
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(231));
}

// Twelve values carried around a loop, which is more than there are
// registers to hold them, so some are put away in the frame. A phi function
// whose result goes there has its arguments go to the same slot, which
// leaves it nothing to settle and leaves the registers free.
TEST(CompilerTest, MoreValuesCarriedAroundALoopThanThereAreRegisters) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      mut val c0: Int32 = 0
      mut val c1: Int32 = 0
      mut val c2: Int32 = 0
      mut val c3: Int32 = 0
      mut val c4: Int32 = 0
      mut val c5: Int32 = 0
      mut val c6: Int32 = 0
      mut val c7: Int32 = 0
      mut val c8: Int32 = 0
      mut val c9: Int32 = 0
      mut val c10: Int32 = 0
      mut val c11: Int32 = 0
      mut val i: Int32 = 0
      loop {
        if i >= 3 {
          break
        }
        mut c0 = c0 + 1
        mut c1 = c1 + 2
        mut c2 = c2 + 3
        mut c3 = c3 + 4
        mut c4 = c4 + 5
        mut c5 = c5 + 6
        mut c6 = c6 + 7
        mut c7 = c7 + 8
        mut c8 = c8 + 9
        mut c9 = c9 + 10
        mut c10 = c10 + 11
        mut c11 = c11 + 12
        mut i = i + 1
      }
      return c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 + c11
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(234));
}

// The numbers the language holds are signed, so what divides them has to
// read a negative one as the number it is rather than as the very large one
// its bits also stand for.
TEST(CompilerTest, DividesNegativeNumbers) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 0 - 8
      val b: Int32 = 4
      val q: Int32 = a / b
      val r: Int32 = a - q * b

      # -8 / 4 is -2, and nothing is left over. An unsigned divide would
      # make the quotient enormous and the comparison below go the other
      # way.
      if q == 0 - 2 {
        if r == 0 {
          return 7
        }
        return 8
      }
      return 9
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(7));
}

TEST(CompilerTest, TakesTheRemainderOfANegativeNumber) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      val a: Int32 = 0 - 9
      val b: Int32 = 4

      # What is left over keeps the sign of what was divided, so -9 % 4 is
      # -1, and adding the divisor brings it back into range.
      return a % b + 5
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(4));
}

// What is left over takes a division and a multiply-subtract, and the
// subtract reads both sides again after the division has written. The
// quotient therefore stands somewhere neither side is, rather than in the
// register the answer goes to.
//
// Holding the answer apart from both sides would do instead, but nothing
// that decides what to put away in memory counts that, so eight parameters
// and one remainder once could not be compiled at all.
TEST(CompilerTest, TakesARemainderBesideMoreValuesThanThereAreRegisters) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun callee(p0: Int32, p1: Int32, p2: Int32, p3: Int32, p4: Int32,
               p5: Int32, p6: Int32, p7: Int32, p8: Int32, p9: Int32,
               p10: Int32, p11: Int32): Int32 {
      val left: Int32 = 7 % 3
      return p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10 + p11 +
             left
    }

    fun main(): Int32 {
      return callee(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1)
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(13));
}

// A result is handed back in the width it is held in. A string is an
// address, twice the width a number is, and half of an address was once all
// that came back: the caller read a whole one out of a register holding half
// of one, and printed whatever that pointed at.
TEST(CompilerTest, HandsBackAStringWhole) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun printString(s: String): Int32 {
      return 0
    }

    fun choose(a: String, b: String, take: Int32): String {
      if take > 0 {
        return a
      }
      return b
    }

    fun main(): Int32 {
      val a: String = ""
      val b: String = ""
      do printString(choose(a, b, 1))
      do printString(choose(a, b, 0))
      return 3
    }
  )"));
  EXPECT_THAT(RunCompiler({"run", FullPath("main.lu")}), ReturnsCode(3));
}

}  // namespace
}  // namespace lucid
