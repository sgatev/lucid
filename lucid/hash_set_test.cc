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

namespace {

using ::testing::Optional;

TEST(HashSet, Empty) {
  HashSet<int> set;

  EXPECT_EQ(set.Size(), 0);
  EXPECT_FALSE(set.Contains(21));
}

TEST(HashSet, Insert) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));

  EXPECT_EQ(set.Size(), 1);
  EXPECT_TRUE(set.Contains(21));
  EXPECT_FALSE(set.Contains(42));
}

TEST(HashSet, InsertSame) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_FALSE(set.Insert(21));

  EXPECT_EQ(set.Size(), 1);
  EXPECT_TRUE(set.Contains(21));
}

TEST(HashSet, InsertDifferent) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  EXPECT_EQ(set.Size(), 2);
  EXPECT_TRUE(set.Contains(21));
  EXPECT_TRUE(set.Contains(13));
}

TEST(HashSet, Remove) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Remove(21));
  EXPECT_FALSE(set.Remove(42));

  EXPECT_EQ(set.Size(), 0);
  EXPECT_FALSE(set.Contains(21));
}

TEST(HashSet, Scaling) {
  HashSet<int> set;

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_TRUE(set.Insert(i));
  }

  EXPECT_EQ(set.Size(), 10000);

  for (int i = 1; i <= 10000; ++i) {
    EXPECT_TRUE(set.Contains(i));
  }
}

TEST(HashSet, Copy) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  HashSet<int> set_copy = set;

  EXPECT_EQ(set_copy.Size(), 2);
  EXPECT_TRUE(set_copy.Contains(21));
  EXPECT_TRUE(set_copy.Contains(13));
}

TEST(HashSet, Move) {
  HashSet<int> set;

  EXPECT_TRUE(set.Insert(21));
  EXPECT_TRUE(set.Insert(13));

  HashSet<int> set_move = std::move(set);

  EXPECT_EQ(set_move.Size(), 2);
  EXPECT_TRUE(set_move.Contains(21));
  EXPECT_TRUE(set_move.Contains(13));
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

}  // namespace
}  // namespace lucid
