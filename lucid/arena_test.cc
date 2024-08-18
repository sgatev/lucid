#include "lucid/arena.h"

#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

TEST(ArenaRefTest, Size) { EXPECT_EQ(sizeof(Arena<int>::Ref), 4); }

TEST(ArenaTest, StoresValues) {
  Arena<int> arena;

  Arena<int>::Ref three = arena.Add(3);
  ArenaRef<int> five = arena.Add(5);

  EXPECT_EQ(arena.Get(three), 3);
  EXPECT_EQ(arena.Get(five), 5);
}

TEST(ArenaTest, ConstAccess) {
  Arena<int> arena;
  Arena<int>::Ref three = arena.Add(3);

  const Arena<int> const_arena = std::move(arena);
  EXPECT_EQ(const_arena.Get(three), 3);
}

TEST(ArenaTest, MutableAccess) {
  Arena<int> arena;

  auto age = arena.Add(3);
  EXPECT_EQ(arena.Get(age), 3);

  arena.Get(age) = 4;
  EXPECT_EQ(arena.Get(age), 4);
}

TEST(ArenaTest, AliasAccess) {
  Arena<int> arena;
  Arena<int>::Ref three = arena.Add(3);
  Arena<int>::Ref three_alias = arena.Alias(three);

  EXPECT_EQ(arena.Get(three_alias), 3);
}

TEST(ArenaTest, Equiv) {
  Arena<int> arena;
  Arena<int>::Ref three = arena.Add(3);
  Arena<int>::Ref three_alias = arena.Alias(three);
  Arena<int>::Ref five = arena.Add(5);
  Arena<int>::Ref other_three = arena.Add(3);

  EXPECT_TRUE(arena.Equiv(three, three_alias));
  EXPECT_FALSE(arena.Equiv(three, five));
  EXPECT_FALSE(arena.Equiv(three, other_three));
}

TEST(ArenaTest, Size) {
  Arena<int> arena;

  auto three = arena.Add(3);
  auto five = arena.Add(5);
  auto eight = arena.Add(8);
  arena.Alias(three);
  arena.Alias(five);
  arena.Alias(eight);

  EXPECT_EQ(arena.Size(), 3);
}

TEST(ArenaTest, RangeFor) {
  Arena<int> arena;

  arena.Add(3);
  arena.Add(5);
  arena.Add(8);

  std::vector<int> elements;
  for (int e : arena) elements.push_back(e);

  EXPECT_THAT(elements, ElementsAre(3, 5, 8));
}

}  // namespace
}  // namespace lucid
