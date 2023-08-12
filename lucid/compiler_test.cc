#include <sys/wait.h>

#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/writer.h"

namespace lucid {
namespace {

class CompilerTest : public testing::Test {
 protected:
  int Write(std::string_view src) {
    std::FILE* src_file = std::fopen(src_path_.c_str(), "w+");
    if (src_file == nullptr) return 1;

    Writer src_writer = FileWriter(src_file);
    src_writer(src);

    return std::fclose(src_file);
  }

  int Compile(std::string binary_name) {
    return ExecCommand(compiler_path_ + " " + binary_name + " " + src_path_);
  }

  int Run(std::string_view binary_name) {
    const std::string binary_path = runtime_path_ / binary_name;
    return ExecCommand(binary_path);
  }

 private:
  int ExecCommand(std::string_view command) {
    const int result = std::system(command.data());
    return WEXITSTATUS(result);
  }

  const std::filesystem::path runtime_path_ = testing::SrcDir() + "__main__";
  const std::string compiler_path_ = runtime_path_ / "lucid" / "compiler";
  const std::string src_path_ = runtime_path_ / "test.lucid";
};

TEST_F(CompilerTest, EmptyMain) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 0
      }
    )"),
            0);
  ASSERT_EQ(Compile("main"), 0);
  EXPECT_EQ(Run("main"), 0);
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
  ASSERT_EQ(Compile("main"), 0);
  EXPECT_EQ(Run("main"), 21);
}

TEST_F(CompilerTest, AddInts) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 2 + 3
      }
    )"),
            0);
  ASSERT_EQ(Compile("main"), 0);
  EXPECT_EQ(Run("main"), 5);
}

TEST_F(CompilerTest, SubInts) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 7 - 5
      }
    )"),
            0);
  ASSERT_EQ(Compile("main"), 0);
  EXPECT_EQ(Run("main"), 2);
}

TEST_F(CompilerTest, MulInts) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 3 * 7
      }
    )"),
            0);
  ASSERT_EQ(Compile("main"), 0);
  EXPECT_EQ(Run("main"), 21);
}

TEST_F(CompilerTest, DivInts) {
  ASSERT_EQ(Write(R"(
      let main = () -> Int {
        return 8 / 2
      }
    )"),
            0);
  ASSERT_EQ(Compile("main"), 0);
  EXPECT_EQ(Run("main"), 4);
}

}  // namespace
}  // namespace lucid
