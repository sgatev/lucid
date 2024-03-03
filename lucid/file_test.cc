#include "lucid/file.h"

#include <optional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using namespace std::string_literals;

using ::testing::Optional;

TEST(ReadFileTest, Works) {
  std::string path = testing::SrcDir() + "_main/lucid/testdata/foobarbaz";
  EXPECT_THAT(ReadFile(path), Optional("foobarbaz\n"s));
  EXPECT_THAT(ReadFile(path, /*with_trailing_zero=*/true),
              Optional("foobarbaz\n\0"s));
}

TEST(ReadFileTest, MissingFile) {
  EXPECT_EQ(ReadFile("unknown"), std::nullopt);
}

}  // namespace
}  // namespace lucid
