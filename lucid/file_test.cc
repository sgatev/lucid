#include "lucid/file.h"

#include <optional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using ::testing::Optional;

TEST(ReadFileTest, Works) {
  std::string path = testing::SrcDir() + "_main/lucid/testdata/foobarbaz";
  EXPECT_THAT(ReadFile(path), Optional(std::string("foobarbaz\n")));
}

TEST(ReadFileTest, MissingFile) {
  EXPECT_EQ(ReadFile("unknown"), std::nullopt);
}

}  // namespace
}  // namespace lucid
