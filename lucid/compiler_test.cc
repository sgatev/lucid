#include <sys/wait.h>

#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <string_view>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/file.h"
#include "lucid/writer.h"

MATCHER_P(ReturnsCode, code, "") { return arg.return_code == code; }
MATCHER_P(PrintsError, err, "") { return arg.err == err; }

namespace lucid {
namespace {

using ::testing::AllOf;

struct CommandResult {
  int return_code;
  std::string err;
};

[[maybe_unused]] std::ostream& operator<<(std::ostream& stream,
                                          const CommandResult& res) {
  return stream << "CommandResult{.return_code=" << res.return_code
                << " .err=\"" << res.err << "\"}";
}

class CompilerTest : public testing::Test {
 protected:
  bool CreateFile(std::string_view src_file_name, std::string_view src) {
    const std::string src_path = runtime_path_ / src_file_name;
    std::FILE* src_file = std::fopen(src_path.c_str(), "w+");
    if (src_file == nullptr) return false;

    Writer src_writer = FileWriter(src_file);
    src_writer(src);

    return std::fclose(src_file) == 0;
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
    const std::string err_path = runtime_path_ / "stderr";
    std::string c = std::string(command) + " 2> " + err_path;
    const int result = std::system(c.c_str());
    const int return_code = WEXITSTATUS(result);
    return {
        .return_code = return_code,
        .err = std::get<std::string>(ReadFile(err_path)),
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
              ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(0));
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
              ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(21));
}

TEST_F(CompilerTest, AddInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 2 + 3
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(5));
}

TEST_F(CompilerTest, SubInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 7 - 5
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(2));
}

TEST_F(CompilerTest, MulInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 3 * 7
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(21));
}

TEST_F(CompilerTest, DivInts) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = () -> Int {
        return 8 / 2
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(4));
}

TEST_F(CompilerTest, MissingArguments) {
  ASSERT_THAT(RunCompiler({}),
              AllOf(ReturnsCode(1), PrintsError("missing arguments\n")));
}

TEST_F(CompilerTest, UnknownCommand) {
  ASSERT_THAT(RunCompiler({"foo"}),
              AllOf(ReturnsCode(1), PrintsError("unknown command: foo\n")));
}

TEST_F(CompilerTest, MissingBuildArguments) {
  ASSERT_THAT(
      RunCompiler({"build"}),
      AllOf(ReturnsCode(1),
            PrintsError("'build' command requires exactly 2 arguments\n")));
}

TEST_F(CompilerTest, UnknownFile) {
  ASSERT_THAT(
      RunCompiler({"build", "unknown", "unknown.lucid"}),
      AllOf(ReturnsCode(1),
            PrintsError("file error: could not read file unknown.lucid\n")));
}

TEST_F(CompilerTest, ParseError) {
  ASSERT_TRUE(CreateFile("main.lucid", R"(
      let main = ( -> Int {
        return 0
      }
    )"));
  ASSERT_THAT(RunCompiler({"build", "main", FullPath("main.lucid")}),
              AllOf(ReturnsCode(1),
                    PrintsError("parse error: expected closing parenthesis or "
                                "parameter at line 2, column 20\n")));
}

}  // namespace
}  // namespace lucid
