#include "lucid/core/io/file.h"

#include <expected>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace lucid {
namespace {

using namespace std::string_literals;

TEST(ReadFileTest, WithoutTrailingZero) {
  std::string path =
      testing::SrcDir() + "_main/lucid/core/io/testdata/foobarbaz";

  std::expected<std::string, ReadFileError> res = ReadFile(path);
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(res.value(), "foobarbaz\n"s);
}

TEST(ReadFileTest, WithTrailingZero) {
  std::string path =
      testing::SrcDir() + "_main/lucid/core/io/testdata/foobarbaz";

  std::expected<std::string, ReadFileError> res =
      ReadFile(path, /*with_trailing_zero=*/true);
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(res.value(), "foobarbaz\n\0"s);
}

TEST(ReadFileTest, MissingFile) {
  std::expected<std::string, ReadFileError> res = ReadFile("unknown");
  ASSERT_FALSE(res.has_value());

  std::stringstream ss;
  ss << res.error();
  EXPECT_EQ(ss.str(), R"(could not read file "unknown")");
}

}  // namespace
}  // namespace lucid
