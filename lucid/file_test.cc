#include "lucid/file.h"

#include <optional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using namespace std::string_literals;

using ::testing::_;
using ::testing::VariantWith;

TEST(ReadFileTest, Works) {
  std::string path = testing::SrcDir() + "_main/lucid/testdata/foobarbaz";
  EXPECT_THAT(ReadFile(path), VariantWith<std::string>("foobarbaz\n"s));
  EXPECT_THAT(ReadFile(path, /*with_trailing_zero=*/true),
              VariantWith<std::string>("foobarbaz\n\0"s));
}

TEST(ReadFileTest, MissingFile) {
  EXPECT_THAT(ReadFile("unknown"), VariantWith<ReadFileError>(_));
}

}  // namespace
}  // namespace lucid
