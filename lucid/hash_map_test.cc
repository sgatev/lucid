#include "lucid/hash_map.h"

#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {

struct CustomKey {
  std::uint8_t a;
  std::uint32_t b;
  std::uint16_t c;

  bool operator==(const CustomKey&) const = default;
};

inline std::size_t Hash(const CustomKey& v) {
  return HashCombine(Hash(v.a), Hash(v.b), Hash(v.c));
}

struct MoveOnly {
  std::uint32_t v;

  explicit MoveOnly(std::uint32_t v) : v(v) {}

  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;

  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  bool operator==(const MoveOnly&) const = default;
};

inline std::size_t Hash(const MoveOnly& v) { return Hash(v.v); }

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

TEST(HashMap, SetSameKey) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_FALSE(map.Set(21, 84));

  EXPECT_EQ(map.Size(), 1);
  EXPECT_THAT(map.Find(21), Optional(84));
  EXPECT_EQ(map.Find(84), std::nullopt);
}

TEST(HashMap, SetDifferentKey) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Set(13, 26));

  EXPECT_EQ(map.Size(), 2);
  EXPECT_THAT(map.Find(21), Optional(42));
  EXPECT_THAT(map.Find(13), Optional(26));
}

TEST(HashMap, Remove) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Remove(21));
  EXPECT_FALSE(map.Remove(42));

  EXPECT_EQ(map.Size(), 0);
  EXPECT_EQ(map.Find(21), std::nullopt);
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

TEST(HashMap, Move) {
  HashMap<std::int32_t, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));

  HashMap<std::int32_t, int> map_move = std::move(map);

  EXPECT_EQ(map_move.Size(), 2);
  EXPECT_THAT(map_move.Find(21), Optional(42));
  EXPECT_THAT(map_move.Find(13), Optional(26));
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

TEST(HashMap, CustomKey) {
  HashMap<CustomKey, int> map;

  EXPECT_TRUE(map.Insert(CustomKey{.a = 1, .b = 10, .c = 100}, 42));
  EXPECT_TRUE(map.Insert(CustomKey{.a = 2, .b = 20, .c = 200}, 26));
  EXPECT_FALSE(map.Insert(CustomKey{.a = 1, .b = 10, .c = 100}, 21));

  EXPECT_THAT(map.Find(CustomKey{.a = 1, .b = 10, .c = 100}), Optional(42));
  EXPECT_THAT(map.Find(CustomKey{.a = 2, .b = 20, .c = 200}), Optional(26));
}

TEST(HashMap, MoveOnly) {
  HashMap<MoveOnly, MoveOnly> map;

  EXPECT_TRUE(map.Insert(MoveOnly(1), MoveOnly(42)));
  EXPECT_TRUE(map.Insert(MoveOnly(2), MoveOnly(26)));
  EXPECT_FALSE(map.Insert(MoveOnly(1), MoveOnly(21)));

  EXPECT_EQ(map.Find(MoveOnly(1)), MoveOnly(42));
  EXPECT_EQ(map.Find(MoveOnly(2)), MoveOnly(26));
}

}  // namespace
}  // namespace lucid
