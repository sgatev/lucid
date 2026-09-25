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

// A name that answers to nothing was once read out of an empty lookup, which
// left the compiler walking a function definition that was never there.
TEST(CompilerTest, ReportsNamesThatAreNotThere) {
  const struct {
    std::string_view source;
    std::string_view message;
  } kCases[] = {
      {R"(fun main(): Int32 { return nope() })", "no function 'nope'"},
      {R"(fun main(): Int32 { return nope })", "no variable 'nope'"},
      {R"(fun main(): Int32 { &nope = 1 return 0 })", "no variable 'nope'"},
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

TEST(CompilerTest, PrintCfg) {
  ASSERT_TRUE(CreateFile("max.lu", R"(
    fun max(a: Int32, b: Int32): Int32 {
      val c: Int32 = 0
      if a > b {
        &c = a
      } else {
        &c = b
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
