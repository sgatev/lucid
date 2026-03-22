#include "lucid/core/container/fixed_map.h"

#include <string_view>

#include "gtest/gtest.h"

namespace lucid {
namespace {

using namespace std::literals::string_view_literals;

TEST(FixedMapTest, PresentKey) {
  static constexpr FixedMap map(
      std::array{
          std::pair{"foo"sv, 1},
          std::pair{"bar"sv, 2},
      },
      21);
  EXPECT_EQ(map["foo"], 1);
  EXPECT_EQ(map["bar"], 2);
}

TEST(FixedMapTest, MissingKey) {
  static constexpr FixedMap map(
      std::array{
          std::pair{"foo"sv, 1},
          std::pair{"bar"sv, 2},
      },
      21);
  EXPECT_EQ(map["baz"], 21);
  EXPECT_EQ(map["qux"], 21);
}

}  // namespace
}  // namespace lucid
