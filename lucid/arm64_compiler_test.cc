#include <sys/wait.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <string>
#include <string_view>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/cli.h"
#include "lucid/file.h"

namespace lucid {
namespace {

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::StartsWith;

// Represents the result of a command.
struct CommandResult {
  // Return code of the command.
  int return_code;

  // String printed on stdout by the command.
  std::string out;

  // String printed on stderr by the command.
  std::string err;
};

[[maybe_unused]] std::ostream& operator<<(std::ostream& stream,
                                          const CommandResult& res) {
  return stream << "CommandResult{.return_code=" << res.return_code
                << ", .out=\"" << res.out << "\" "
                << ", .err=\"" << res.err << "\"}";
}

// Matches the return code of a command.
MATCHER_P(ReturnsCode, matcher, "") {
  return ExplainMatchResult(matcher, arg.return_code, result_listener);
}

// Matches the string printed on stdout by a command.
MATCHER_P(Prints, matcher, "") {
  return ExplainMatchResult(matcher, arg.out, result_listener);
}

// Matches the string printed on stderr by a command.
MATCHER_P(PrintsError, matcher, "") {
  return ExplainMatchResult(matcher, arg.err, result_listener);
}

class CompilerTest : public testing::Test {
 protected:
  bool CreateFile(std::string_view src_file_name, std::string_view src) {
    const std::string src_path = runtime_path_ / src_file_name;
    std::ofstream src_stream(src_path);
    src_stream << src;
    return true;
  }

  CommandResult RunCompiler(std::initializer_list<std::string_view> args) {
    std::string cmd = FullPath("lucid/compiler");
    for (auto arg : args) cmd += " " + std::string(arg);
    return ExecCommand(cmd);
  }

  CommandResult Run(std::string_view binary_name) {
    return ExecCommand(FullPath(binary_name));
  }

  std::string FullPath(std::string_view file_name) {
    return runtime_path_ / file_name;
  }

 private:
  CommandResult ExecCommand(std::string_view command) {
    const std::string out_path = runtime_path_ / "stdout";
    const std::string err_path = runtime_path_ / "stderr";
    std::string c = std::string(command) + " > " + out_path + " 2> " + err_path;
    const int result = std::system(c.c_str());
    const int return_code = WEXITSTATUS(result);
    return {
        .return_code = return_code,
        .out = ReadFile(out_path).value(),
        .err = ReadFile(err_path).value(),
    };
  }

  const std::filesystem::path runtime_path_ = testing::SrcDir() + "__main__";
};

TEST_F(CompilerTest, EmptyMain) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 0
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(0)));
}

TEST_F(CompilerTest, FunctionCall) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let id = (x: Int) -> Int {
        return x
      }

      let main = () -> Int {
        return id(21)
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, AddInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 2 + 3
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(5)));
}

TEST_F(CompilerTest, SubInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 7 - 5
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(2)));
}

TEST_F(CompilerTest, MulInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 3 * 7
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(21)));
}

TEST_F(CompilerTest, DivInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 8 / 2
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(4)));
}

TEST_F(CompilerTest, AddBools) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return true + false
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(Run("main"), ReturnsCode(Eq(1)));
}

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
