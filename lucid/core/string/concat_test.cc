#include "lucid/core/string/concat.h"

#include <span>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(ConcatTest, SameSizeParts) {
  EXPECT_EQ(Concat(std::vector<std::string_view>{"a", "b", "c"}, " "), "a b c");
}

TEST(ConcatTest, DifferentSizeParts) {
  EXPECT_EQ(Concat(std::vector<std::string_view>{"aaa", "b", "cc"}, "dddd"),
            "aaaddddbddddcc");
}

TEST(ConcatTest, EmptyGlue) {
  EXPECT_EQ(Concat(std::vector<std::string_view>{"a", "b", "c"}, ""), "abc");
}

TEST(ConcatTest, NoParts) {
  EXPECT_EQ(Concat(std::vector<std::string_view>{""}, "foo"), "");
}

}  // namespace
}  // namespace lucid
