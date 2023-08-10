#include <sys/wait.h>

#include <filesystem>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

class CompilerTest : public testing::Test {
 protected:
  int Compile(std::string_view input) {
    const std::string compiler_path = runtime_path_ / "lucid" / "compiler";
    return ExecCommand(compiler_path + " " + std::string(input));
  }

  int Run(std::string_view input) {
    const std::string binary_path = runtime_path_ / input;
    return ExecCommand(binary_path);
  }

 private:
  int ExecCommand(std::string_view command) {
    const int result = std::system(command.data());
    return WEXITSTATUS(result);
  }

  const std::filesystem::path runtime_path_ = testing::SrcDir() + "__main__";
};

TEST_F(CompilerTest, EmptyMain) {
  ASSERT_EQ(Compile("empty_main"), 0);
  EXPECT_EQ(Run("empty_main"), 0);
}

TEST_F(CompilerTest, FunctionCall) {
  ASSERT_EQ(Compile("func_call"), 0);
  EXPECT_EQ(Run("func_call"), 21);
}

TEST_F(CompilerTest, AddInts) {
  ASSERT_EQ(Compile("add_ints"), 0);
  EXPECT_EQ(Run("add_ints"), 5);
}

TEST_F(CompilerTest, SubInts) {
  ASSERT_EQ(Compile("sub_ints"), 0);
  EXPECT_EQ(Run("sub_ints"), 2);
}

TEST_F(CompilerTest, MulInts) {
  ASSERT_EQ(Compile("mul_ints"), 0);
  EXPECT_EQ(Run("mul_ints"), 21);
}

TEST_F(CompilerTest, DivInts) {
  ASSERT_EQ(Compile("div_ints"), 0);
  EXPECT_EQ(Run("div_ints"), 4);
}

}  // namespace
}  // namespace lucid
