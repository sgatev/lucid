#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/compiler/compiler_test_fixture.h"

namespace lucid {
namespace {

using ::testing::AllOf;
using ::testing::EndsWith;
using ::testing::StartsWith;

TEST_F(CompilerTest, VersionIncludesCommitLine) {
  ASSERT_THAT(RunCompiler({"version"}),
              AllOf(ReturnsCode(0), Prints(StartsWith("Commit:"))));
}

TEST_F(CompilerTest, NoCommand) {
  ASSERT_THAT(RunCompiler({}),
              AllOf(ReturnsCode(0), Prints(R"(Usage: lucid <command> ...

Available commands:
  build      Compiles the specified target and builds a binary.
  compile    Compiles the specified target.
  run        Compiles the specified target, builds a binary, and runs it.
  parse      Parses the specified target.
  print-ast  Parses the specified target and prints the AST.
  print-cfg  Parses the specified target and prints the CFG.
  print-ami  Parses the specified target and prints the AMI.
  version    Prints version information for lucid.
)")));
}

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

// Matches a formatted error string.
MATCHER_P(FormattedError, matcher, "") {
  return ExplainMatchResult(
      matcher,
      std::string_view(arg).substr(sizeof("\033[31mERROR:\033[0m ") - 1),
      result_listener);
}

TEST_F(CompilerTest, UnknownCommand) {
  ASSERT_THAT(RunCompiler({"foo"}),
              AllOf(ReturnsCode(1),
                    PrintsError(FormattedError("unknown command 'foo'\n"))));
}

TEST_F(CompilerTest, NoBuildArguments) {
  ASSERT_THAT(RunCompiler({"build"}),
              AllOf(ReturnsCode(1),
                    PrintsError(FormattedError(
                        "'build' command requires exactly 2 arguments\n"))));
}

TEST_F(CompilerTest, UnknownFile) {
  ASSERT_THAT(
      RunCompiler({"build", "unknown", "unknown.lu"}),
      AllOf(ReturnsCode(1),
            PrintsError(AllOf(FormattedError(StartsWith("could not read file")),
                              EndsWith("unknown.lu\"\n")))));
}

TEST_F(CompilerTest, ParseError) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = ( -> Int32 {
      return 0
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              AllOf(ReturnsCode(1), PrintsError(FormattedError(
                                        "expected closing parenthesis or "
                                        "parameter at line 2, column 17\n"))));
}

TEST_F(CompilerTest, TypeError) {
  ASSERT_TRUE(CreateFile("main.lu", R"(
    let main = () -> Int32 {
      return true
    }
  )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lu")}),
              AllOf(ReturnsCode(1),
                    PrintsError(FormattedError("expected type Int32\n"))));
}

}  // namespace
}  // namespace lucid
