#include "lucid/core/container/hash_map.h"

#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>

#include "lucid/core/testing/testing.h"

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

struct ConstructOnly {
  std::uint32_t v;

  explicit ConstructOnly(std::uint32_t v) : v(v) {}

  ConstructOnly(ConstructOnly&&) = default;
  ConstructOnly& operator=(ConstructOnly&&) = delete;

  ConstructOnly(const ConstructOnly&) = delete;
  ConstructOnly& operator=(const ConstructOnly&) = delete;

  bool operator==(const ConstructOnly&) const = default;
};

inline std::size_t Hash(const ConstructOnly& v) { return Hash(v.v); }

namespace {

TEST(Test, HashMapFindMiss) {
  HashMap<int, int> map;

  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.Get(21), std::nullopt);
}

TEST(Test, HashMapEmplace) {
  HashMap<int, std::pair<int, int>> map;

  EXPECT_EQ(map.Emplace(21, 1, 2), std::make_pair(1, 2));
  EXPECT_EQ(map.Emplace(21, 2, 4), std::make_pair(1, 2));
}

TEST(Test, HashMapInsert) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));

  EXPECT_EQ(map.size(), 1);
  EXPECT_THAT(map.Get(21), Optional(Equals(42)));
  EXPECT_EQ(map.Get(42), std::nullopt);
}

TEST(Test, HashMapInsertSameKey) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_FALSE(map.Insert(21, 84));

  EXPECT_EQ(map.size(), 1);
  EXPECT_THAT(map.Get(21), Optional(Equals(42)));
}

TEST(Test, HashMapInsertDifferentKey) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));

  EXPECT_EQ(map.size(), 2);
  EXPECT_THAT(map.Get(21), Optional(Equals(42)));
  EXPECT_THAT(map.Get(13), Optional(Equals(26)));
}

TEST(Test, HashMapSetSameKey) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_FALSE(map.Set(21, 84));

  EXPECT_EQ(map.size(), 1);
  EXPECT_THAT(map.Get(21), Optional(Equals(84)));
  EXPECT_EQ(map.Get(84), std::nullopt);
}

TEST(Test, HashMapSetDifferentKey) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Set(13, 26));

  EXPECT_EQ(map.size(), 2);
  EXPECT_THAT(map.Get(21), Optional(Equals(42)));
  EXPECT_THAT(map.Get(13), Optional(Equals(26)));
}

TEST(Test, HashMapRemove) {
  HashMap<int, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_EQ(map.Remove(21), 42);
  EXPECT_EQ(map.Remove(42), std::nullopt);

  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.Get(21), std::nullopt);
}

TEST(Test, HashMapScaling) {
  HashMap<int, int> map;

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_TRUE(map.Insert(i, i * 2));
  }

  EXPECT_EQ(map.size(), 10000);

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_THAT(map.Get(i), Optional(Equals(i * 2)));
  }
}

TEST(Test, HashMapCopyConstruct) {
  HashMap<std::int32_t, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));

  HashMap<std::int32_t, int> map_copy = map;

  EXPECT_EQ(map_copy.size(), 2);
  EXPECT_THAT(map_copy.Get(21), Optional(Equals(42)));
  EXPECT_THAT(map_copy.Get(13), Optional(Equals(26)));
}

TEST(Test, HashMapCopyAssign) {
  HashMap<std::int32_t, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));

  HashMap<std::int32_t, int> map_copy;
  map_copy = map;

  EXPECT_EQ(map_copy.size(), 2);
  EXPECT_THAT(map_copy.Get(21), Optional(Equals(42)));
  EXPECT_THAT(map_copy.Get(13), Optional(Equals(26)));
}

TEST(Test, HashMapMoveConstruct) {
  HashMap<MoveOnly, int> map;

  EXPECT_TRUE(map.Insert(MoveOnly(21), 42));
  EXPECT_TRUE(map.Insert(MoveOnly(13), 26));

  HashMap<MoveOnly, int> map_move = std::move(map);

  EXPECT_EQ(map_move.size(), 2);
  EXPECT_THAT(map_move.Get(MoveOnly(21)), Optional(Equals(42)));
  EXPECT_THAT(map_move.Get(MoveOnly(13)), Optional(Equals(26)));
}

TEST(Test, HashMapMoveAssign) {
  HashMap<MoveOnly, int> map;

  EXPECT_TRUE(map.Insert(MoveOnly(21), 42));
  EXPECT_TRUE(map.Insert(MoveOnly(13), 26));

  HashMap<MoveOnly, int> map_move;
  map_move = std::move(map);

  EXPECT_EQ(map_move.size(), 2);
  EXPECT_THAT(map_move.Get(MoveOnly(21)), Optional(Equals(42)));
  EXPECT_THAT(map_move.Get(MoveOnly(13)), Optional(Equals(26)));
}

TEST(Test, HashMapEqual) {
  HashMap<int, int> map1;
  map1.Insert(21, 1);
  map1.Insert(13, 2);

  HashMap<int, int> map2;
  map2.Insert(13, 2);
  map2.Insert(21, 1);

  EXPECT_TRUE(map1 == map2);
}

TEST(Test, HashMapNotEqualDifferentValues) {
  HashMap<int, int> map1;
  map1.Insert(21, 1);
  map1.Insert(13, 2);

  HashMap<int, int> map2;
  map2.Insert(13, 3);
  map2.Insert(21, 4);

  EXPECT_TRUE(map1 != map2);
}

TEST(Test, HashMapNotEqualSameSize) {
  HashMap<int, int> map1;
  map1.Insert(21, 1);
  map1.Insert(13, 2);

  HashMap<int, int> map2;
  map2.Insert(13, 2);
  map2.Insert(42, 1);

  EXPECT_TRUE(map1 != map2);
}

TEST(Test, HashMapNotEqualDifferentSize) {
  HashMap<int, int> map1;
  map1.Insert(21, 1);

  HashMap<int, int> map2;
  map2.Insert(13, 2);
  map2.Insert(21, 1);

  EXPECT_TRUE(map1 != map2);
}

TEST(Test, HashMapInt32) {
  HashMap<std::int32_t, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));
  EXPECT_FALSE(map.Insert(21, 21));

  EXPECT_THAT(map.Get(21), Optional(Equals(42)));
  EXPECT_THAT(map.Get(13), Optional(Equals(26)));
}

TEST(Test, HashMapUint32) {
  HashMap<std::uint32_t, int> map;

  EXPECT_TRUE(map.Insert(21, 42));
  EXPECT_TRUE(map.Insert(13, 26));
  EXPECT_FALSE(map.Insert(21, 21));

  EXPECT_THAT(map.Get(21), Optional(Equals(42)));
  EXPECT_THAT(map.Get(13), Optional(Equals(26)));
}

TEST(Test, HashMapStringView) {
  HashMap<std::string_view, int> map;

  EXPECT_TRUE(map.Insert("foo", 42));
  EXPECT_TRUE(map.Insert("bar", 26));
  EXPECT_FALSE(map.Insert("foo", 21));

  EXPECT_THAT(map.Get("foo"), Optional(Equals(42)));
  EXPECT_THAT(map.Get("bar"), Optional(Equals(26)));
}

TEST(Test, HashMapCustomKey) {
  HashMap<CustomKey, int> map;

  EXPECT_TRUE(map.Insert(CustomKey{.a = 1, .b = 10, .c = 100}, 42));
  EXPECT_TRUE(map.Insert(CustomKey{.a = 2, .b = 20, .c = 200}, 26));
  EXPECT_FALSE(map.Insert(CustomKey{.a = 1, .b = 10, .c = 100}, 21));

  EXPECT_THAT(map.Get(CustomKey{.a = 1, .b = 10, .c = 100}),
              Optional(Equals(42)));
  EXPECT_THAT(map.Get(CustomKey{.a = 2, .b = 20, .c = 200}),
              Optional(Equals(26)));
}

TEST(Test, HashMapMoveOnly) {
  HashMap<MoveOnly, MoveOnly> map;

  EXPECT_TRUE(map.Insert(MoveOnly(1), MoveOnly(42)));
  EXPECT_TRUE(map.Insert(MoveOnly(2), MoveOnly(26)));
  EXPECT_FALSE(map.Insert(MoveOnly(1), MoveOnly(21)));

  EXPECT_EQ(map.Get(MoveOnly(1)), MoveOnly(42));
  EXPECT_EQ(map.Get(MoveOnly(2)), MoveOnly(26));
}

TEST(Test, HashMapConstructOnly) {
  HashMap<ConstructOnly, ConstructOnly> map;

  EXPECT_TRUE(map.Insert(ConstructOnly(1), ConstructOnly(42)));
  EXPECT_TRUE(map.Insert(ConstructOnly(2), ConstructOnly(26)));
  EXPECT_FALSE(map.Insert(ConstructOnly(1), ConstructOnly(21)));

  EXPECT_EQ(map.Get(ConstructOnly(1)), ConstructOnly(42));
  EXPECT_EQ(map.Get(ConstructOnly(2)), ConstructOnly(26));
}

TEST(Test, HashMapIteratorCompareDifferentMaps) {
  HashMap<int, int> map1;
  map1.Insert(21, 42);

  HashMap<int, int> map2;
  map2.Insert(21, 42);

  EXPECT_NE(map1.begin(), map2.begin());
  EXPECT_NE(map1.end(), map2.end());
}

TEST(Test, HashMapIteratorCompareEmpty) {
  HashMap<int, int> map;

  EXPECT_EQ(map.begin(), map.begin());
  EXPECT_EQ(map.end(), map.end());
  EXPECT_EQ(map.begin(), map.end());
}

TEST(Test, HashMapIteratorCompareNonEmpty) {
  HashMap<int, int> map;
  map.Insert(21, 42);

  EXPECT_EQ(map.begin(), map.begin());
  EXPECT_EQ(map.end(), map.end());
  EXPECT_EQ(++map.begin(), map.end());
}

TEST(Test, HashMapIteratorDeref) {
  HashMap<int, int> map;
  map.Insert(21, 42);

  EXPECT_EQ(*map.begin(), std::make_pair(21, 42));
}

TEST(Test, HashMapIteratorRange) {
  HashMap<int, int> map;
  map.Insert(21, 42);
  map.Insert(13, 26);

  EXPECT_THAT(map, UnorderedElementsEqual(
                       {std::make_pair(13, 26), std::make_pair(21, 42)}));
}

}  // namespace
}  // namespace lucid
