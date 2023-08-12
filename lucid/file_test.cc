#include "lucid/file.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using ::testing::VariantWith;

TEST(ReadFileTest, Works) {
  const std::string path =
      testing::SrcDir() + "__main__/lucid/testdata/foobarbaz";
  EXPECT_THAT(ReadFile(path), VariantWith<std::string>("foobarbaz\n"));
}

TEST(ReadFileTest, MissingFile) {
  EXPECT_THAT(ReadFile("unknown"), VariantWith<FileError>(FileError{}));
}

}  // namespace
}  // namespace lucid
