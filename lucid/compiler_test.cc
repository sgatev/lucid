#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/compiler_test_fixture.h"

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
  version    Prints version information for lucid.
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
