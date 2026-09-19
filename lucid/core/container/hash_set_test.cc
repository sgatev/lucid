#include "lucid/core/container/hash_set.h"

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

TEST(Test, HashSetEmpty) {
  HashSet<int> set;

  EXPECT_EQ(set.size(), 0);
  EXPECT_FALSE(set.Contains(21));
}

TEST(Test, HashSetInsert) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));

  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.Contains(21));
  EXPECT_FALSE(set.Contains(42));
}

TEST(Test, HashSetInsertSame) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_FALSE(set.Insert(21));

  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.Contains(21));
}

TEST(Test, HashSetInsertDifferent) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  EXPECT_EQ(set.size(), 2);
  EXPECT_TRUE(set.Contains(21));
  EXPECT_TRUE(set.Contains(13));
}

TEST(Test, HashSetRemove) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_EQ(set.Remove(21), 21);
  EXPECT_EQ(set.Remove(42), std::nullopt);

  EXPECT_EQ(set.size(), 0);
  EXPECT_FALSE(set.Contains(21));
}

TEST(Test, HashSetSurvivesRepeatedRemoval) {
  HashSet<int> set;
  for (int i = 0; i < 10000; ++i) {
    set.Insert(i);
    if (i > 0) EXPECT_TRUE(set.Remove(i - 1).has_value());
  }
  EXPECT_EQ(set.size(), 1);
}

TEST(Test, HashSetScaling) {
  HashSet<int> set;

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_TRUE(set.Insert(i));
  }

  EXPECT_EQ(set.size(), 10000);

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_TRUE(set.Contains(i));
  }
}

TEST(Test, HashSetCopyConstruct) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  HashSet<int> set_copy = set;

  EXPECT_EQ(set_copy.size(), 2);
  EXPECT_TRUE(set_copy.Contains(21));
  EXPECT_TRUE(set_copy.Contains(13));
}

TEST(Test, HashSetCopyAssign) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  HashSet<int> set_copy;
  set_copy = set;

  EXPECT_EQ(set_copy.size(), 2);
  EXPECT_TRUE(set_copy.Contains(21));
  EXPECT_TRUE(set_copy.Contains(13));
}

TEST(Test, HashSetMoveConstruct) {
  HashSet<MoveOnly> set;

  EXPECT_TRUE(set.Insert(MoveOnly(21)));
  EXPECT_TRUE(set.Insert(MoveOnly(13)));

  HashSet<MoveOnly> set_move = std::move(set);

  EXPECT_EQ(set_move.size(), 2);
  EXPECT_TRUE(set_move.Contains(MoveOnly(21)));
  EXPECT_TRUE(set_move.Contains(MoveOnly(13)));
}

TEST(Test, HashSetMoveAssign) {
  HashSet<MoveOnly> set;

  EXPECT_TRUE(set.Insert(MoveOnly(21)));
  EXPECT_TRUE(set.Insert(MoveOnly(13)));

  HashSet<MoveOnly> set_move;
  set_move = std::move(set);

  EXPECT_EQ(set_move.size(), 2);
  EXPECT_TRUE(set_move.Contains(MoveOnly(21)));
  EXPECT_TRUE(set_move.Contains(MoveOnly(13)));
}

TEST(Test, HashSetEqual) {
  HashSet<int> set1;
  set1.Insert(21);
  set1.Insert(13);

  HashSet<int> set2;
  set2.Insert(13);
  set2.Insert(21);

  EXPECT_TRUE(set1 == set2);
}

TEST(Test, HashSetNotEqualSameSize) {
  HashSet<int> set1;
  set1.Insert(21);
  set1.Insert(13);

  HashSet<int> set2;
  set2.Insert(13);
  set2.Insert(42);

  EXPECT_TRUE(set1 != set2);
}

TEST(Test, HashSetNotEqualDifferentSize) {
  HashSet<int> set1;
  set1.Insert(21);

  HashSet<int> set2;
  set2.Insert(13);
  set2.Insert(21);

  EXPECT_TRUE(set1 != set2);
}

TEST(Test, HashSetInt32) {
  HashSet<std::int32_t> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  EXPECT_TRUE(set.Contains(21));
  EXPECT_TRUE(set.Contains(13));
  EXPECT_FALSE(set.Contains(42));
}

TEST(Test, HashSetUint32) {
  HashSet<std::uint32_t> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  EXPECT_TRUE(set.Contains(21));
  EXPECT_TRUE(set.Contains(13));
  EXPECT_FALSE(set.Contains(42));
}

TEST(Test, HashSetStringView) {
  HashSet<std::string_view> set;

  EXPECT_TRUE(set.Insert("foo"));
  EXPECT_TRUE(set.Insert("bar"));

  EXPECT_TRUE(set.Contains("foo"));
  EXPECT_TRUE(set.Contains("bar"));
  EXPECT_FALSE(set.Contains("baz"));
}

TEST(Test, HashSetCustomKey) {
  HashSet<CustomKey> set;

  EXPECT_TRUE(set.Insert(CustomKey{.a = 1, .b = 10, .c = 100}));
  EXPECT_TRUE(set.Insert(CustomKey{.a = 2, .b = 20, .c = 200}));

  EXPECT_TRUE(set.Contains(CustomKey{.a = 1, .b = 10, .c = 100}));
  EXPECT_TRUE(set.Contains(CustomKey{.a = 2, .b = 20, .c = 200}));
  EXPECT_FALSE(set.Contains(CustomKey{.a = 3, .b = 30, .c = 300}));
}

TEST(Test, HashSetMoveOnly) {
  HashSet<MoveOnly> set;

  EXPECT_TRUE(set.Insert(MoveOnly(1)));
  EXPECT_TRUE(set.Insert(MoveOnly(2)));

  EXPECT_TRUE(set.Contains(MoveOnly(1)));
  EXPECT_TRUE(set.Contains(MoveOnly(2)));
  EXPECT_FALSE(set.Contains(MoveOnly(3)));
}

TEST(Test, HashSetConstructOnly) {
  HashSet<ConstructOnly> set;

  EXPECT_TRUE(set.Insert(ConstructOnly(21)));
  EXPECT_TRUE(set.Insert(ConstructOnly(13)));

  HashSet<ConstructOnly> set_move;
  set_move = std::move(set);

  EXPECT_EQ(set_move.size(), 2);
  EXPECT_TRUE(set_move.Contains(ConstructOnly(21)));
  EXPECT_TRUE(set_move.Contains(ConstructOnly(13)));
}

TEST(Test, HashSetIteratorCompareDifferentSets) {
  HashSet<int> set1;
  set1.Insert(21);

  HashSet<int> set2;
  set2.Insert(21);

  EXPECT_NE(set1.begin(), set2.begin());
  EXPECT_NE(set1.end(), set2.end());
}

TEST(Test, HashSetIteratorCompareEmpty) {
  HashSet<int> set;

  EXPECT_EQ(set.begin(), set.begin());
  EXPECT_EQ(set.end(), set.end());
  EXPECT_EQ(set.begin(), set.end());
}

TEST(Test, HashSetIteratorCompareNonEmpty) {
  HashSet<int> set;
  set.Insert(21);

  EXPECT_EQ(set.begin(), set.begin());
  EXPECT_EQ(set.end(), set.end());
  EXPECT_EQ(++set.begin(), set.end());
}

TEST(Test, HashSetIteratorDeref) {
  HashSet<int> set;
  set.Insert(21);

  EXPECT_EQ(*set.begin(), 21);
}

TEST(Test, HashSetIteratorRange) {
  HashSet<int> set;
  set.Insert(21);
  set.Insert(13);

  EXPECT_THAT(set, UnorderedElementsEqual(13, 21));
}

}  // namespace
}  // namespace lucid
