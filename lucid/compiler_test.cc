#include <sys/wait.h>

#include <filesystem>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(CompilerTest, Works) {
  const std::filesystem::path runtime_path = testing::SrcDir() + "__main__";

  const auto compiler_path = runtime_path / "lucid" / "compiler";
  const int compiler_result = std::system(compiler_path.c_str());
  EXPECT_EQ(WEXITSTATUS(compiler_result), 0);

  const auto binary_path = runtime_path / "main";
  const int binary_result = std::system(binary_path.c_str());
  EXPECT_EQ(WEXITSTATUS(binary_result), 21);
}

}  // namespace
}  // namespace lucid
