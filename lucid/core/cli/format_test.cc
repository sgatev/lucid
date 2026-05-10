#include "lucid/core/cli/format.h"

#include <sstream>

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(SetColorTest, Works) {
  std::stringstream ss;
  ss << SetColor(Color::Blue) << "foo";
  ss << "bar";
  ss << SetColor(Color::Green);
  ss << "baz";

  EXPECT_EQ(ss.str(), "\33[34mfoobar\33[32mbaz");
}

TEST(ResetColorTest, Works) {
  std::stringstream ss;
  ss << "foo" << ResetColor;
  ss << "bar";
  ss << ResetColor;
  ss << "baz";

  EXPECT_EQ(ss.str(), "foo\33[mbar\33[mbaz");
}

TEST(IndentTest, Works) {
  std::stringstream ss;
  ss << Indent(4) << "foo";
  ss << "bar";
  ss << Indent(2);
  ss << "baz";

  EXPECT_EQ(ss.str(), "    foobar  baz");
}

}  // namespace
}  // namespace lucid
