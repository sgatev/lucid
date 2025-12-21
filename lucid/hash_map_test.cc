#include "lucid/hash_map.h"

#include <cstdint>
#include <optional>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using ::testing::Optional;

TEST(HashMap, FindMiss) {
  HashMap<int, int> map;

  EXPECT_EQ(map.Size(), 0);
  EXPECT_EQ(map.Find(21), std::nullopt);
}

TEST(HashMap, Insert) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));

  EXPECT_EQ(map.Size(), 1);
  EXPECT_THAT(map.Find(21), Optional(42));
  EXPECT_EQ(map.Find(42), std::nullopt);
}

TEST(HashMap, InsertSameKey) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_FALSE(map.Insert(21, 84));

  EXPECT_EQ(map.Size(), 1);
  EXPECT_THAT(map.Find(21), Optional(42));
}

TEST(HashMap, InsertDifferentKey) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));

  EXPECT_EQ(map.Size(), 2);
  EXPECT_THAT(map.Find(21), Optional(42));
  EXPECT_THAT(map.Find(13), Optional(26));
}

TEST(HashMap, Scaling) {
  HashMap<int, int> map;

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_TRUE(map.Insert(i, i * 2));
  }

  EXPECT_EQ(map.Size(), 10000);

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_THAT(map.Find(i), Optional(i * 2));
  }
}

TEST(HashMap, Copy) {
  HashMap<std::int32_t, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));

  HashMap<std::int32_t, int> map_copy = map;

  EXPECT_EQ(map_copy.Size(), 2);
  EXPECT_THAT(map_copy.Find(21), Optional(42));
  EXPECT_THAT(map_copy.Find(13), Optional(26));
}

TEST(HashMap, Int32) {
  HashMap<std::int32_t, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));
  EXPECT_FALSE(map.Insert(21, 21));

  EXPECT_THAT(map.Find(21), Optional(42));
  EXPECT_THAT(map.Find(13), Optional(26));
}

TEST(HashMap, Uint32) {
  HashMap<std::uint32_t, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));
  EXPECT_FALSE(map.Insert(21, 21));

  EXPECT_THAT(map.Find(21), Optional(42));
  EXPECT_THAT(map.Find(13), Optional(26));
}

TEST(HashMap, StringView) {
  HashMap<std::string_view, int> map;

  EXPECT_TRUE(map.Insert("foo", 42));
  EXPECT_TRUE(map.Insert("bar", 26));
  EXPECT_FALSE(map.Insert("foo", 21));

  EXPECT_THAT(map.Find("foo"), Optional(42));
  EXPECT_THAT(map.Find("bar"), Optional(26));
}

}  // namespace
}  // namespace lucid
