#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/compiler_test_fixture.h"

namespace lucid {
namespace {

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::StartsWith;

TEST_F(CompilerTest, MissingArguments) {
  ASSERT_THAT(RunCompiler({}),
              AllOf(ReturnsCode(Eq(0)), Prints(Eq(R"(Usage: lucid <command> ...

Available commands:
  build 	Compiles the specified target and builds a binary.
  version 	Prints version information for lucid.
)"))));
}

TEST_F(CompilerTest, UnknownCommand) {
  ASSERT_THAT(
      RunCompiler({"foo"}),
      AllOf(ReturnsCode(Eq(1)), PrintsError(Eq("unknown command: foo\n"))));
}

TEST_F(CompilerTest, MissingBuildArguments) {
  ASSERT_THAT(
      RunCompiler({"build"}),
      AllOf(ReturnsCode(Eq(1)),
            PrintsError(Eq("'build' command requires exactly 2 arguments\n"))));
}

TEST_F(CompilerTest, UnknownFile) {
  ASSERT_THAT(
      RunCompiler({"build", "unknown", "unknown.lucid"}),
      AllOf(
          ReturnsCode(Eq(1)),
          PrintsError(Eq("file error: could not read file unknown.lucid\n"))));
}

TEST_F(CompilerTest, ParseError) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = ( -> Int {
        return 0
      }
    )"));
  ASSERT_THAT(
      RunCompiler({"build", "main", FullPath("main.lucid")}),
      AllOf(ReturnsCode(Eq(1)),
            PrintsError(Eq("parse error: expected closing parenthesis or "
                           "parameter at line 2, column 20\n"))));
}

TEST_F(CompilerTest, VersionIncludesCommitLine) {
  ASSERT_THAT(RunCompiler({"version"}),
              AllOf(ReturnsCode(Eq(0)), Prints(StartsWith("Commit:"))));
}

}  // namespace
}  // namespace lucid
