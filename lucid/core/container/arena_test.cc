#include "lucid/core/container/arena.h"

#include <initializer_list>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, ArenaRefDefault) {
  EXPECT_TRUE(Arena<int>::Ref() == Arena<int>::kNullRef);
}

TEST(Test, ArenaRefSize) { EXPECT_EQ(sizeof(Arena<int>::Ref), 4); }

TEST(Test, ArenaRefUnconvertible) {
  EXPECT_FALSE((std::is_convertible_v<Arena<int>::Ref, Arena<double>::Ref>));
}

TEST(Test, ArenaRefEquality) {
  EXPECT_TRUE(Arena<int>::Ref(2) == Arena<int>::Ref(2));
  EXPECT_TRUE(Arena<int>::Ref(2) != Arena<int>::Ref(3));
}

TEST(Test, ArenaRefId) { EXPECT_EQ(Arena<int>::Ref(2).id(), 2); }

TEST(Test, ArenaRefIncrements) {
  Arena<int>::Ref ref(2);
  ++ref;
  EXPECT_EQ(ref.id(), 3);
  --ref;
  EXPECT_EQ(ref.id(), 2);
}

TEST(Test, ArenaRefOffsets) {
  Arena<int>::Ref ref(5);
  EXPECT_EQ((ref + 3).id(), 8);
  EXPECT_EQ((ref - 2).id(), 3);
}

TEST(Test, ArenaStoresValues) {
  Arena<int> arena;

  Arena<int>::Ref three = arena.Add(3);
  Arena<int>::Ref five = arena.Add(5);

  EXPECT_EQ(arena.Get(three), 3);
  EXPECT_EQ(arena.Get(five), 5);
}

TEST(Test, ArenaConstAccess) {
  Arena<int> arena;
  Arena<int>::Ref three = arena.Add(3);

  const Arena<int> const_arena = std::move(arena);
  EXPECT_EQ(const_arena.Get(three), 3);
}

TEST(Test, ArenaMutableAccess) {
  Arena<int> arena;

  auto age = arena.Add(3);
  EXPECT_EQ(arena.Get(age), 3);

  arena.Get(age) = 4;
  EXPECT_EQ(arena.Get(age), 4);
}

TEST(Test, ArenaAliasAccess) {
  Arena<int> arena;
  Arena<int>::Ref three = arena.Add(3);
  Arena<int>::Ref three_alias = arena.Alias(three);

  EXPECT_EQ(arena.Get(three_alias), 3);
}

TEST(Test, ArenaEquiv) {
  Arena<int> arena;
  Arena<int>::Ref three = arena.Add(3);
  Arena<int>::Ref three_alias = arena.Alias(three);
  Arena<int>::Ref five = arena.Add(5);
  Arena<int>::Ref other_three = arena.Add(3);

  EXPECT_TRUE(arena.Equiv(three, three_alias));
  EXPECT_FALSE(arena.Equiv(three, five));
  EXPECT_FALSE(arena.Equiv(three, other_three));
}

TEST(Test, ArenaSize) {
  Arena<int> arena;

  auto three = arena.Add(3);
  auto five = arena.Add(5);
  auto eight = arena.Add(8);
  arena.Alias(three);
  arena.Alias(five);
  arena.Alias(eight);

  EXPECT_EQ(arena.Size(), 3);
}

TEST(Test, ArenaRangeFor) {
  Arena<int> arena;

  arena.Add(3);
  arena.Add(5);
  arena.Add(8);

  std::vector<int> elements;
  for (int e : arena) elements.push_back(e);

  EXPECT_THAT(elements, ElementsEqual(3, 5, 8));
}

}  // namespace
}  // namespace lucid
