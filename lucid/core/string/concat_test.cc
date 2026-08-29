#include "lucid/core/string/concat.h"

#include <span>
#include <string_view>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, ConcatSameSizeParts) {
  EXPECT_EQ(Concat(std::vector<std::string_view>{"a", "b", "c"}, " "), "a b c");
}

TEST(Test, ConcatDifferentSizeParts) {
  EXPECT_EQ(Concat(std::vector<std::string_view>{"aaa", "b", "cc"}, "dddd"),
            "aaaddddbddddcc");
}

TEST(Test, ConcatEmptyGlue) {
  EXPECT_EQ(Concat(std::vector<std::string_view>{"a", "b", "c"}, ""), "abc");
}

TEST(Test, ConcatNoParts) {
  EXPECT_EQ(Concat(std::vector<std::string_view>{""}, "foo"), "");
}

}  // namespace
}  // namespace lucid
