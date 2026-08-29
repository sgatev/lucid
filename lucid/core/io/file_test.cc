#include "lucid/core/io/file.h"

#include <expected>
#include <filesystem>
#include <sstream>
#include <string>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

using namespace std::string_literals;

TEST(Test, ReadFileWithoutTrailingZero) {
  std::string path =
      std::filesystem::current_path() / "lucid/core/io/testdata/foobarbaz";

  std::expected<std::string, ReadFileError> res = ReadFile(path);
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(res.value(), "foobarbaz\n"s);
}

TEST(Test, ReadFileWithTrailingZero) {
  std::string path =
      std::filesystem::current_path() / "lucid/core/io/testdata/foobarbaz";

  std::expected<std::string, ReadFileError> res =
      ReadFile(path, /*with_trailing_zero=*/true);
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(res.value(), "foobarbaz\n\0"s);
}

TEST(Test, ReadFileMissingFile) {
  std::expected<std::string, ReadFileError> res = ReadFile("unknown");
  ASSERT_FALSE(res.has_value());

  std::stringstream ss;
  ss << res.error();
  EXPECT_EQ(ss.str(), R"(could not read file "unknown")");
}

}  // namespace
}  // namespace lucid
