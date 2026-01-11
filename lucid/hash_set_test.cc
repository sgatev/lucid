#include "lucid/hash_set.h"

#include <cstdint>
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

using ::testing::Optional;
using ::testing::UnorderedElementsAre;

TEST(HashSet, Empty) {
  HashSet<int> set;

  EXPECT_EQ(set.size(), 0);
  EXPECT_FALSE(set.Contains(21));
}

TEST(HashSet, Insert) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));

  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.Contains(21));
  EXPECT_FALSE(set.Contains(42));
}

TEST(HashSet, InsertSame) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_FALSE(set.Insert(21));

  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.Contains(21));
}

TEST(HashSet, InsertDifferent) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  EXPECT_EQ(set.size(), 2);
  EXPECT_TRUE(set.Contains(21));
  EXPECT_TRUE(set.Contains(13));
}

TEST(HashSet, Remove) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Remove(21));
  EXPECT_FALSE(set.Remove(42));

  EXPECT_EQ(set.size(), 0);
  EXPECT_FALSE(set.Contains(21));
}

TEST(HashSet, Scaling) {
  HashSet<int> set;

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_TRUE(set.Insert(i));
  }

  EXPECT_EQ(set.size(), 10000);

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_TRUE(set.Contains(i));
  }
}

TEST(HashSet, CopyConstruct) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  HashSet<int> set_copy = set;

  EXPECT_EQ(set_copy.size(), 2);
  EXPECT_TRUE(set_copy.Contains(21));
  EXPECT_TRUE(set_copy.Contains(13));
}

TEST(HashSet, CopyAssign) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  HashSet<int> set_copy;
  set_copy = set;

  EXPECT_EQ(set_copy.size(), 2);
  EXPECT_TRUE(set_copy.Contains(21));
  EXPECT_TRUE(set_copy.Contains(13));
}

TEST(HashSet, MoveConstruct) {
  HashSet<MoveOnly> set;

  EXPECT_TRUE(set.Insert(MoveOnly(21)));
  EXPECT_TRUE(set.Insert(MoveOnly(13)));

  HashSet<MoveOnly> set_move = std::move(set);

  EXPECT_EQ(set_move.size(), 2);
  EXPECT_TRUE(set_move.Contains(MoveOnly(21)));
  EXPECT_TRUE(set_move.Contains(MoveOnly(13)));
}

TEST(HashSet, MoveAssign) {
  HashSet<MoveOnly> set;

  EXPECT_TRUE(set.Insert(MoveOnly(21)));
  EXPECT_TRUE(set.Insert(MoveOnly(13)));

  HashSet<MoveOnly> set_move;
  set_move = std::move(set);

  EXPECT_EQ(set_move.size(), 2);
  EXPECT_TRUE(set_move.Contains(MoveOnly(21)));
  EXPECT_TRUE(set_move.Contains(MoveOnly(13)));
}

TEST(HashSet, Equal) {
  HashSet<int> set1;
  set1.Insert(21);
  set1.Insert(13);

  HashSet<int> set2;
  set2.Insert(13);
  set2.Insert(21);

  EXPECT_TRUE(set1 == set2);
}

TEST(HashSet, NotEqualSameSize) {
  HashSet<int> set1;
  set1.Insert(21);
  set1.Insert(13);

  HashSet<int> set2;
  set2.Insert(13);
  set2.Insert(42);

  EXPECT_TRUE(set1 != set2);
}

TEST(HashSet, NotEqualDifferentSize) {
  HashSet<int> set1;
  set1.Insert(21);

  HashSet<int> set2;
  set2.Insert(13);
  set2.Insert(21);

  EXPECT_TRUE(set1 != set2);
}

TEST(HashSet, Int32) {
  HashSet<std::int32_t> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  EXPECT_TRUE(set.Contains(21));
  EXPECT_TRUE(set.Contains(13));
  EXPECT_FALSE(set.Contains(42));
}

TEST(HashSet, Uint32) {
  HashSet<std::uint32_t> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  EXPECT_TRUE(set.Contains(21));
  EXPECT_TRUE(set.Contains(13));
  EXPECT_FALSE(set.Contains(42));
}

TEST(HashSet, StringView) {
  HashSet<std::string_view> set;

  EXPECT_TRUE(set.Insert("foo"));
  EXPECT_TRUE(set.Insert("bar"));

  EXPECT_TRUE(set.Contains("foo"));
  EXPECT_TRUE(set.Contains("bar"));
  EXPECT_FALSE(set.Contains("baz"));
}

TEST(HashSet, CustomKey) {
  HashSet<CustomKey> set;

  EXPECT_TRUE(set.Insert(CustomKey{.a = 1, .b = 10, .c = 100}));
  EXPECT_TRUE(set.Insert(CustomKey{.a = 2, .b = 20, .c = 200}));

  EXPECT_TRUE(set.Contains(CustomKey{.a = 1, .b = 10, .c = 100}));
  EXPECT_TRUE(set.Contains(CustomKey{.a = 2, .b = 20, .c = 200}));
  EXPECT_FALSE(set.Contains(CustomKey{.a = 3, .b = 30, .c = 300}));
}

TEST(HashSet, MoveOnly) {
  HashSet<MoveOnly> set;

  EXPECT_TRUE(set.Insert(MoveOnly(1)));
  EXPECT_TRUE(set.Insert(MoveOnly(2)));

  EXPECT_TRUE(set.Contains(MoveOnly(1)));
  EXPECT_TRUE(set.Contains(MoveOnly(2)));
  EXPECT_FALSE(set.Contains(MoveOnly(3)));
}

TEST(HashSet, ConstructOnly) {
  HashSet<ConstructOnly> set;

  EXPECT_TRUE(set.Insert(ConstructOnly(21)));
  EXPECT_TRUE(set.Insert(ConstructOnly(13)));

  HashSet<ConstructOnly> set_move;
  set_move = std::move(set);

  EXPECT_EQ(set_move.size(), 2);
  EXPECT_TRUE(set_move.Contains(ConstructOnly(21)));
  EXPECT_TRUE(set_move.Contains(ConstructOnly(13)));
}

TEST(HashSet, IteratorCompareDifferentSets) {
  HashSet<int> set1;
  set1.Insert(21);

  HashSet<int> set2;
  set2.Insert(21);

  EXPECT_NE(set1.begin(), set2.begin());
  EXPECT_NE(set1.end(), set2.end());
}

TEST(HashSet, IteratorCompareEmpty) {
  HashSet<int> set;

  EXPECT_EQ(set.begin(), set.begin());
  EXPECT_EQ(set.end(), set.end());
  EXPECT_EQ(set.begin(), set.end());
}

TEST(HashSet, IteratorCompareNonEmpty) {
  HashSet<int> set;
  set.Insert(21);

  EXPECT_EQ(set.begin(), set.begin());
  EXPECT_EQ(set.end(), set.end());
  EXPECT_EQ(++set.begin(), set.end());
}

TEST(HashSet, IteratorDeref) {
  HashSet<int> set;
  set.Insert(21);

  EXPECT_EQ(*set.begin(), 21);
}

TEST(HashSet, IteratorRange) {
  HashSet<int> set;
  set.Insert(21);
  set.Insert(13);

  EXPECT_THAT(set, UnorderedElementsAre(13, 21));
}

}  // namespace
}  // namespace lucid
