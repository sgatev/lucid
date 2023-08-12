#include "lucid/file.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(ReadFileTest, Works) {
  const std::string path =
      testing::SrcDir() + "__main__/lucid/testdata/foobarbaz";
  EXPECT_EQ(ReadFile(path), "foobarbaz\n");
}

}  // namespace
}  // namespace lucid
