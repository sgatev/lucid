#include "lucid/file.h"

#include <optional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

MATCHER_P(HasValue, matcher, "") {
  if (!arg.HasValue()) return false;
  return ExplainMatchResult(matcher, arg.GetValue(), result_listener);
}

namespace lucid {
namespace {

using namespace std::string_literals;

using ::testing::_;
using ::testing::VariantWith;

TEST(ReadFileTest, Works) {
  std::string path = testing::SrcDir() + "_main/lucid/testdata/foobarbaz";
  EXPECT_THAT(ReadFile(path), HasValue("foobarbaz\n"s));
  EXPECT_THAT(ReadFile(path, /*with_trailing_zero=*/true),
              HasValue("foobarbaz\n\0"s));
}

TEST(ReadFileTest, MissingFile) { EXPECT_TRUE(ReadFile("unknown").HasError()); }

}  // namespace
}  // namespace lucid
