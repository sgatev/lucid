#include "lucid/arena.h"

#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

TEST(ArenaTest, StoresValues) {
  Arena<int> arena;

  Arena<int>::Ref three = arena.add(3);
  ArenaRef<int> five = arena.add(5);

  EXPECT_EQ(arena.get(three), 3);
  EXPECT_EQ(arena.get(five), 5);
}

TEST(ArenaTest, ConstAccess) {
  Arena<int> arena;
  Arena<int>::Ref three = arena.add(3);

  const Arena<int> const_arena = std::move(arena);
  EXPECT_EQ(const_arena.get(three), 3);
}

TEST(ArenaTest, MutableAccess) {
  Arena<int> arena;

  auto age = arena.add(3);
  EXPECT_EQ(arena.get(age), 3);

  arena.get(age) = 4;
  EXPECT_EQ(arena.get(age), 4);
}

TEST(ArenaTest, Size) {
  Arena<int> arena;

  arena.add(3);
  arena.add(5);
  arena.add(8);

  EXPECT_EQ(arena.size(), 3);
}

TEST(ArenaTest, RangeFor) {
  Arena<int> arena;

  arena.add(3);
  arena.add(5);
  arena.add(8);

  std::vector<int> elements;
  for (int e : arena) elements.push_back(e);

  EXPECT_THAT(elements, ElementsAre(3, 5, 8));
}

}  // namespace
}  // namespace lucid
