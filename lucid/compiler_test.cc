#include <sys/wait.h>

#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

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
  int Write(std::string_view src) {
    std::FILE* src_file = std::fopen(src_path_.c_str(), "w+");
    if (src_file == nullptr) return 1;

    Writer src_writer = FileWriter(src_file);
    src_writer(src);

    return std::fclose(src_file);
  }

  CommandResult Compile(std::string binary_name) {
    return ExecCommand(compiler_path_ + " " + binary_name + " " + src_path_);
  }

  CommandResult Run(std::string_view binary_name) {
    const std::string binary_path = runtime_path_ / binary_name;
    return ExecCommand(binary_path);
  }

 private:
  CommandResult ExecCommand(std::string_view command) {
    std::string c = std::string(command) + " 2> " + err_path_;
    const int result = std::system(c.c_str());
    const int return_code = WEXITSTATUS(result);
    return {
        .return_code = return_code,
        .err = ReadFile(err_path_),
    };
  }

  const std::filesystem::path runtime_path_ = testing::SrcDir() + "__main__";
  const std::string compiler_path_ = runtime_path_ / "lucid" / "compiler";
  const std::string src_path_ = runtime_path_ / "test.lucid";
  const std::string err_path_ = runtime_path_ / "stderr";
};

TEST_F(CompilerTest, EmptyMain) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 0
      }
    )"),
            0);
  ASSERT_THAT(Compile("main"), ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(0));
}

TEST_F(CompilerTest, FunctionCall) {
  ASSERT_EQ(Write(R"(
      let id = (x: Int) -> Int {
        return x
      }

      let main = () -> Int {
        return id(21)
      }
    )"),
            0);
  ASSERT_THAT(Compile("main"), ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(21));
}

TEST_F(CompilerTest, AddInts) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 2 + 3
      }
    )"),
            0);
  ASSERT_THAT(Compile("main"), ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(5));
}

TEST_F(CompilerTest, SubInts) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 7 - 5
      }
    )"),
            0);
  ASSERT_THAT(Compile("main"), ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(2));
}

TEST_F(CompilerTest, MulInts) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 3 * 7
      }
    )"),
            0);
  ASSERT_THAT(Compile("main"), ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(21));
}

TEST_F(CompilerTest, DivInts) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 8 / 2
      }
    )"),
            0);
  ASSERT_THAT(Compile("main"), ReturnsCode(0));
  EXPECT_THAT(Run("main"), ReturnsCode(4));
}

TEST_F(CompilerTest, ParseError) {
  ASSERT_EQ(Write(R"(
      let main = ( -> Int {
        return 0
      }
    )"),
            0);
  ASSERT_THAT(Compile("main"),
              AllOf(ReturnsCode(1),
                    PrintsError("parse error: expected closing parenthesis or "
                                "parameter at line 2, column 20\n")));
}

}  // namespace
}  // namespace lucid
