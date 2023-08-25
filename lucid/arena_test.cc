#include "lucid/arena.h"

#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

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

}  // namespace
}  // namespace lucid
