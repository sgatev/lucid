#include <string_view>

#include "lucid/compiler/compiler_test_fixture.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(CompilerTest, VersionIncludesCommitLine) {
  ASSERT_THAT(RunCompiler({"version"}),
              AllOf(ReturnsCode(0), Output(StartsWith("Commit:"))));
}

TEST(CompilerTest, PrintAstStatement) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 0
    }
  )"));

  EXPECT_THAT(RunCompiler({"print-ast", FullPath("main.lu"), "S0"}),
              AllOf(ReturnsCode(0),
                    Output(StartsWith("\033[34mS0: \033[mReturnStmt"))));
}

TEST(CompilerTest, PrintAstRejectsMalformedNode) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 0
    }
  )"));

  // An index that is not a number, one that no integer can hold, and one with
  // no kind in front of it: none of them names a node, and none of them is a
  // reason to stop running.
  for (std::string_view id : {"Sxyz", "S99999999999999999999", "X1", "S"}) {
    EXPECT_THAT(
        RunCompiler({"print-ast", FullPath("main.lu"), id}),
        AllOf(ReturnsCode(1), ErrorOutput(Contains(
                                  "must be either 'S<index>' or 'E<index>'"))));
  }
}

// A `break` outside every loop once read the top of an empty stack of the
// blocks a loop is left for.
TEST(CompilerTest, ReportsBreakWithNoLoop) {
  const std::string_view kCases[] = {
      R"(fun main(): Int32 { break return 0 })",
      R"(fun main(): Int32 { if true { break } return 0 })",
  };

  for (std::string_view source : kCases) {
    ASSERT_TRUE(CreateFile("main.lu", source));
    EXPECT_THAT(RunCompiler({"compile", FullPath("main.lu")}),
                AllOf(ReturnsCode(1),
                      ErrorOutput(Contains("break with no loop to leave"))));
  }
}

// A declaration is gone once the block holding it ends, which once left the
// name behind for whatever came after to read.
TEST(CompilerTest, ReportsVariablesOutOfScope) {
  const std::string_view kCases[] = {
      R"(fun main(): Int32 { if true { val y: Int32 = 2 } return y })",
      R"(fun main(): Int32 { loop { val y: Int32 = 2 break } return y })",
      R"(fun main(): Int32 { if true { mut val y: Int32 = 2 } mut y = 3 return 0 })",
  };

  for (std::string_view source : kCases) {
    ASSERT_TRUE(CreateFile("main.lu", source));
    EXPECT_THAT(
        RunCompiler({"compile", FullPath("main.lu")}),
        AllOf(ReturnsCode(1), ErrorOutput(Contains("no variable 'y'"))));
  }
}

// A write can only reach what said it could, which is what the `&` on a
// declaration says. The mark is the same one the write itself carries.
TEST(CompilerTest, ReportsWritesToWhatIsNotMarked) {
  const struct {
    std::string_view source;
    std::string_view message;
  } kCases[] = {
      {R"(fun main(): Int32 { val n: Int32 = 1 mut n = 2 return n })",
       "no 'mut' on the declaration of 'n'"},
      {R"(fun f(a: Int32): Int32 { mut a = 1 return a }
          fun main(): Int32 { return f(0) })",
       "no 'mut' on the declaration of 'a'"},
      {R"(fun main(): Int32 { val b: Int32[2] mut b[0] = 1 return b[0] })",
       "no 'mut' on the declaration of 'b'"},
      {R"(val P: Type = (x: Int32, y: Int32)
          fun main(): Int32 { val p: P mut p.x = 1 return p.x })",
       "no 'mut' on the declaration of 'p'"},
      {R"(val P: Type = (x: Int32, y: Int32)
          fun main(): Int32 { val ps: P[2] mut ps[0].x = 1 return ps[0].x })",
       "no 'mut' on the declaration of 'ps'"},
      {R"(comp fun two(): Int32 { return 2 }
          fun main(): Int32 { val c: Int32 = comp two() mut c = 5 return c })",
       "no 'mut' on the declaration of 'c'"},
  };

  for (const auto& [source, message] : kCases) {
    ASSERT_TRUE(CreateFile("main.lu", source));
    EXPECT_THAT(RunCompiler({"compile", FullPath("main.lu")}),
                AllOf(ReturnsCode(1), ErrorOutput(Contains(message))));
  }
}

// What a declaration marked for writing allows, and what one not marked for
// it still allows: reading, and being read through.
TEST(CompilerTest, AllowsWritesToWhatIsMarked) {
  const std::string_view kCases[] = {
      R"(fun main(): Int32 { mut val n: Int32 = 1 mut n = 7 return n })",
      R"(fun f(mut a: Int32): Int32 { mut a = 7 return a }
         fun main(): Int32 { return f(0) })",
      R"(fun main(): Int32 { mut val b: Int32[2] mut b[0] = 7 return b[0] })",
      R"(val P: Type = (x: Int32, y: Int32)
         fun main(): Int32 { mut val p: P mut p.x = 7 return p.x })",
      R"(fun main(): Int32 { val n: Int32 = 7 return n })",
      R"(comp fun two(): Int32 { return 2 }
         fun main(): Int32 { mut val c: Int32 = comp two() mut c = 7 return c })",
      R"(val P: Type = (x: Int32, y: Int32)
         fun main(): Int32 { mut val p: P mut p.x = 7 return p.x })",
  };

  for (std::string_view source : kCases) {
    ASSERT_TRUE(CreateFile("main.lu", source));
    EXPECT_THAT(RunCompiler({"compile", FullPath("main.lu")}), ReturnsCode(0));
  }
}

// A name that answers to nothing was once read out of an empty lookup, which
// left the compiler walking a function definition that was never there.
// Following a call during compilation costs a frame of the compiler's own
// stack, so a program that recurses without end is told where compilation
// gave up rather than taking the compiler down with it.
TEST(CompilerTest, ReportsCompilationThatRunsTooDeep) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun down(n: Int32): Int32 {
      if n == 0 {
        return 0
      }
      return down(n - 1)
    }

    fun main(): Int32 {
      return comp down(100000)
    }
  )"));
  EXPECT_THAT(RunCompiler({"compile", FullPath("main.lu")}),
              AllOf(ReturnsCode(1),
                    ErrorOutput(Contains("compilation followed more than"))));
}

// A loop with no end to it is what compilation runs out of work it will do
// on, rather than out of stack. The count never reaches the number it is
// tested against until it has run round the whole width of an `Int32`, which
// is far more work than compilation will do.
TEST(CompilerTest, ReportsCompilationThatRunsTooLong) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun spin(end: Int32): Int32 {
      mut val i: Int32 = 0
      loop {
        if i == end {
          break
        }
        mut i = i + 1
      }
      return i
    }

    fun main(): Int32 {
      return comp spin(0 - 1)
    }
  )"));
  EXPECT_THAT(
      RunCompiler({"compile", FullPath("main.lu")}),
      AllOf(ReturnsCode(1),
            ErrorOutput(Contains("compilation worked through more than"))));
}

// A block is counted along with what it holds, so a loop that does next to
// nothing on each turn is caught as surely as one that does plenty.
TEST(CompilerTest, ReportsCompilationSpinningOverAlmostNothing) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    comp fun spin(end: Int32): Int32 {
      mut val i: Int32 = 0
      loop {
        if i == end {
          break
        }
      }
      return i
    }

    fun main(): Int32 {
      return comp spin(0 - 1)
    }
  )"));
  EXPECT_THAT(
      RunCompiler({"compile", FullPath("main.lu")}),
      AllOf(ReturnsCode(1),
            ErrorOutput(Contains("compilation worked through more than"))));
}

TEST(CompilerTest, ReportsNamesThatAreNotThere) {
  const struct {
    std::string_view source;
    std::string_view message;
  } kCases[] = {
      {R"(fun main(): Int32 { return nope() })", "no function 'nope'"},
      {R"(fun main(): Int32 { return nope })", "no variable 'nope'"},
      {R"(fun main(): Int32 { mut nope = 1 return 0 })", "no variable 'nope'"},
      {R"(fun main(): Nope { return 0 })", "no type of this name"},
      {R"(val P: Type = (x: Int32)
          fun main(): Int32 { val p: P return p.nope })",
       "no field 'nope'"},
      {R"(fun main(): Int32 { val n: Int32 = 1 return n.x })",
       "a value of this type has no fields"},
      {R"(fun main(): Int32 { val n: Int32 = 1 return n[0] })",
       "a value of this type cannot be indexed"},
  };

  for (const auto& [source, message] : kCases) {
    ASSERT_TRUE(CreateFile("main.lu", source));
    EXPECT_THAT(RunCompiler({"compile", FullPath("main.lu")}),
                AllOf(ReturnsCode(1), ErrorOutput(Contains(message))));
  }
}

// The arguments are taken one for one with the parameters, so a call that
// brings the wrong number of them once read a parameter never passed.
TEST(CompilerTest, ReportsTheWrongNumberOfArguments) {
  ASSERT_TRUE(CreateFile("few.lu", R"(
    fun f(a: Int32, b: Int32): Int32 {
      return a
    }

    fun main(): Int32 {
      return f(1)
    }
  )"));
  EXPECT_THAT(
      RunCompiler({"compile", FullPath("few.lu")}),
      AllOf(ReturnsCode(1),
            ErrorOutput(Contains("function 'f' takes 2 arguments, not 1"))));

  ASSERT_TRUE(CreateFile("many.lu", R"(
    fun f(a: Int32): Int32 {
      return a
    }

    fun main(): Int32 {
      return f(1, 2)
    }
  )"));
  EXPECT_THAT(
      RunCompiler({"compile", FullPath("many.lu")}),
      AllOf(ReturnsCode(1),
            ErrorOutput(Contains("function 'f' takes 1 argument, not 2"))));
}

// An array or a tuple lives on the stack and nothing lays one out on either
// side of a call, which is said rather than fallen over.
TEST(CompilerTest, RejectsCompositeParamsAndResults) {
  ASSERT_TRUE(CreateFile("param.lu", R"(
    val Point: Type = (x: Int32)

    fun f(p: Point): Int32 {
      return 0
    }
  )"));
  EXPECT_THAT(
      RunCompiler({"compile", FullPath("param.lu")}),
      AllOf(ReturnsCode(1), ErrorOutput(Contains(
                                "a parameter cannot be an array or a tuple"))));

  ASSERT_TRUE(CreateFile("result.lu", R"(
    val Point: Type = (x: Int32)

    fun f(): Point {
      val p: Point
      return p
    }
  )"));
  EXPECT_THAT(
      RunCompiler({"compile", FullPath("result.lu")}),
      AllOf(ReturnsCode(1),
            ErrorOutput(Contains("a result cannot be an array or a tuple"))));
}

TEST(CompilerTest, PrintAstRejectsNodeThatIsNotThere) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 0
    }
  )"));

  EXPECT_THAT(
      RunCompiler({"print-ast", FullPath("main.lu"), "S9999"}),
      AllOf(ReturnsCode(1), ErrorOutput(Contains("no statement 'S9999'"))));
  EXPECT_THAT(
      RunCompiler({"print-ast", FullPath("main.lu"), "E9999"}),
      AllOf(ReturnsCode(1), ErrorOutput(Contains("no expression 'E9999'"))));
}

TEST(CompilerTest, NoCommand) {
  ASSERT_THAT(RunCompiler({}),
              AllOf(ReturnsCode(0), Output(Equals(R"(Usage: lucid <command> ...

Available commands:
  build            Compiles the specified target and builds a binary.
  compile          Compiles the specified target.
  run              Compiles the specified target, builds a binary, and runs it.
  parse            Parses the specified target.
  print-ast        Parses the specified target and prints the AST.
  print-syntax-cfg Parses the specified target and prints the syntax CFG.
  print-am-cfg     Parses the specified target and prints the abstract machine CFG.
  version          Prints version information for lucid.
)"))));
}

TEST(CompilerTest, PrintAst) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"print-ast", FullPath("main.lu")}),
              AllOf(ReturnsCode(0), Output(Equals(R"(FuncDefStmt {
  .name = "main"
  .stmts = [
[34m    S0: [mReturnStmt {
      .value = {
[34m        E0: [mIntLitExpr {
          .value = 0
        }
      }
    }
  ]
}
)"))));
}

TEST(CompilerTest, PrintAstExpression) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"print-ast", FullPath("main.lu"), "E0"}),
              AllOf(ReturnsCode(0), Output(Equals(R"([34mE0: [mIntLitExpr {
  .value = 0
}
)"))));
}

// One blank line between functions, the same as the abstract machine graph
// is printed with.
TEST(CompilerTest, PrintCfgSeparatesFunctions) {
  ASSERT_TRUE(CreateFile("two.lu", R"(
    fun one(): Int32 {
      return 1
    }

    fun two(): Int32 {
      return 2
    }
  )"));
  ASSERT_THAT(RunCompiler({"print-syntax-cfg", FullPath("two.lu")}),
              AllOf(ReturnsCode(0), Output(Contains("}\n\n"))));
}

TEST(CompilerTest, PrintCfg) {
  ASSERT_TRUE(CreateFile("max.lu", R"(
    fun max(a: Int32, b: Int32): Int32 {
      mut val c: Int32 = 0
      if a > b {
        mut c = a
      } else {
        mut c = b
      }
      return c
    }
  )"));
  ASSERT_THAT(RunCompiler({"print-syntax-cfg", FullPath("max.lu")}),
              AllOf(ReturnsCode(0), Output(Equals(R"([34mmax($0, $1)[m {
  [34mB0:[m {
    .sequences = [
      [34mE0: [mIntLitExpr { .value = 0 }
      [34mS2: [mVarDeclStmt { .name = '$2', .init = E0 }
      [34mE1: [mIdentExpr { .name = '$0' }
      [34mE2: [mIdentExpr { .name = '$1' }
      [34mE3: [mBinaryOpExpr { .op = Gt, .lhs = E1, .rhs = E2 }
    ]
    .next = [
      [34mB3[m
      [34mB4[m
    ]
  }
  [34mB1:[m {
    .preds = [
      [34mB2[m
    ]
  }
  [34mB2:[m {
    .phis = [
      $5 = φ($4, $3)
    ]
    .sequences = [
      [34mE6: [mIdentExpr { .name = '$5' }
      [34mS4: [mReturnStmt { .value = E6 }
    ]
    .next = [
      [34mB1[m
    ]
    .preds = [
      [34mB3[m
      [34mB4[m
    ]
  }
  [34mB3:[m {
    .sequences = [
      [34mE4: [mIdentExpr { .name = '$0' }
      [34mS6: [mVarDeclStmt { .name = '$4', .init = E4 }
    ]
    .next = [
      [34mB2[m
    ]
    .preds = [
      [34mB0[m
    ]
  }
  [34mB4:[m {
    .sequences = [
      [34mE5: [mIdentExpr { .name = '$1' }
      [34mS5: [mVarDeclStmt { .name = '$3', .init = E5 }
    ]
    .next = [
      [34mB2[m
    ]
    .preds = [
      [34mB0[m
    ]
  }
}
)"))));
}

TEST(CompilerTest, UnknownCommand) {
  ASSERT_THAT(
      RunCompiler({"foo"}),
      AllOf(ReturnsCode(1), ErrorOutput(Contains("unknown command 'foo'"))));
}

TEST(CompilerTest, NoBuildArguments) {
  ASSERT_THAT(RunCompiler({"build"}),
              AllOf(ReturnsCode(1),
                    ErrorOutput(Contains(
                        ("'build' command requires exactly 2 arguments")))));
}

TEST(CompilerTest, UnknownFile) {
  ASSERT_THAT(
      RunCompiler({"build", "unknown", "unknown.lu"}),
      AllOf(ReturnsCode(1), ErrorOutput(AllOf(Contains("could not read file"),
                                              EndsWith("unknown.lu\"\n")))));
}

TEST(CompilerTest, ParseError) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(: Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              AllOf(ReturnsCode(1),
                    ErrorOutput(Contains("expected closing parenthesis or "
                                         "parameter at line 2, column 14\n"))));
}

TEST(CompilerTest, TypeError) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return true
    }
  )"));
  ASSERT_THAT(
      RunCompiler({"build", "main", FullPath("main.lu")}),
      AllOf(ReturnsCode(1), ErrorOutput(Contains("expected type Int32\n"))));
}

}  // namespace
}  // namespace lucid
