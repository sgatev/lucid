#include <string_view>

#include "lucid/compiler/compiler_test_fixture.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(CompilerTest, VersionIncludesCommitLine) {
  ASSERT_THAT(RunCompiler({"version"}),
              AllOf(ReturnsCode(0), Output(StartsWith("Commit:"))));
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

TEST(CompilerTest, PrintAstNode) {
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

// Matches a formatted error string.
template <typename M>
auto FormattedError(M matcher) {
  return Truly([matcher](std::string_view s) {
    return matcher.Matches(s.substr(sizeof("\33[31mERROR:\33[m ") - 1));
  });
}

TEST(CompilerTest, UnknownCommand) {
  ASSERT_THAT(
      RunCompiler({"foo"}),
      AllOf(ReturnsCode(1),
            ErrorOutput(FormattedError(StartsWith("unknown command 'foo'")))));
}

TEST(CompilerTest, NoBuildArguments) {
  ASSERT_THAT(RunCompiler({"build"}),
              AllOf(ReturnsCode(1),
                    ErrorOutput(FormattedError(StartsWith(
                        "'build' command requires exactly 2 arguments")))));
}

TEST(CompilerTest, UnknownFile) {
  ASSERT_THAT(
      RunCompiler({"build", "unknown", "unknown.lu"}),
      AllOf(ReturnsCode(1),
            ErrorOutput(AllOf(FormattedError(StartsWith("could not read file")),
                              EndsWith("unknown.lu\"\n")))));
}

TEST(CompilerTest, ParseError) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(: Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              AllOf(ReturnsCode(1), ErrorOutput(FormattedError(StartsWith(
                                        "expected closing parenthesis or "
                                        "parameter at line 2, column 14\n")))));
}

TEST(CompilerTest, TypeError) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    fun main(): Int32 {
      return true
    }
  )"));
  ASSERT_THAT(
      RunCompiler({"build", "main", FullPath("main.lu")}),
      AllOf(ReturnsCode(1),
            ErrorOutput(FormattedError(StartsWith("expected type Int32\n")))));
}

}  // namespace
}  // namespace lucid
