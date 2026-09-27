#include "lucid/am/reg_set.h"

#include <cstddef>
#include <random>
#include <set>
#include <vector>

#include "lucid/am/instructions.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

Reg R(std::int32_t id) { return Reg{.id = id, .size = RegSize32}; }

std::vector<Reg> Held(const RegSet& set) {
  return std::vector<Reg>(set.begin(), set.end());
}

TEST(Test, RegSetHoldsNothingToBeginWith) {
  RegSet set(8);

  EXPECT_EQ(set.size(), 0);
  EXPECT_THAT(Held(set), IsEmpty());
  EXPECT_FALSE(set.Contains(R(0)));
  EXPECT_FALSE(set.Contains(R(7)));
}

TEST(Test, RegSetHoldsWhatIsPutIn) {
  RegSet set(8);

  set.Insert(R(3));
  set.Insert(R(6));
  set.Insert(R(0));

  EXPECT_EQ(set.size(), 3);
  EXPECT_TRUE(set.Contains(R(3)));
  EXPECT_TRUE(set.Contains(R(6)));
  EXPECT_TRUE(set.Contains(R(0)));
  EXPECT_FALSE(set.Contains(R(4)));
  EXPECT_THAT(Held(set), UnorderedElementsEqual(R(0), R(3), R(6)));
}

TEST(Test, RegSetTakesTheSameRegisterOnce) {
  RegSet set(4);

  set.Insert(R(2));
  set.Insert(R(2));

  EXPECT_EQ(set.size(), 1);
  EXPECT_THAT(Held(set), UnorderedElementsEqual(R(2)));
}

TEST(Test, RegSetForgetsWhatIsTakenOut) {
  RegSet set(4);

  set.Insert(R(1));
  set.Remove(R(1));

  EXPECT_EQ(set.size(), 0);
  EXPECT_FALSE(set.Contains(R(1)));
  EXPECT_THAT(Held(set), IsEmpty());
}

// Taking one out moves whichever was last into its place, so the one that
// moved has to be findable where it landed rather than where it was put.
TEST(Test, RegSetKeepsTheRestWhenOneIsTakenOut) {
  RegSet set(8);

  set.Insert(R(1));
  set.Insert(R(4));
  set.Insert(R(7));

  set.Remove(R(1));

  EXPECT_EQ(set.size(), 2);
  EXPECT_FALSE(set.Contains(R(1)));
  EXPECT_TRUE(set.Contains(R(4)));
  EXPECT_TRUE(set.Contains(R(7)));
  EXPECT_THAT(Held(set), UnorderedElementsEqual(R(4), R(7)));

  // The one that moved is still taken out by the register it is, not by
  // where it first stood.
  set.Remove(R(7));

  EXPECT_THAT(Held(set), UnorderedElementsEqual(R(4)));
}

// Nothing moves where what is taken out is already last.
TEST(Test, RegSetTakesOutTheOneAtTheEnd) {
  RegSet set(8);

  set.Insert(R(2));
  set.Insert(R(5));
  set.Remove(R(5));

  EXPECT_EQ(set.size(), 1);
  EXPECT_TRUE(set.Contains(R(2)));
  EXPECT_THAT(Held(set), UnorderedElementsEqual(R(2)));
}

TEST(Test, RegSetTakingOutWhatIsNotThereChangesNothing) {
  RegSet set(8);

  set.Insert(R(3));
  set.Remove(R(5));

  EXPECT_EQ(set.size(), 1);
  EXPECT_THAT(Held(set), UnorderedElementsEqual(R(3)));
}

TEST(Test, RegSetIsEmptyAndUsableAfterClearing) {
  RegSet set(8);

  set.Insert(R(1));
  set.Insert(R(6));
  set.Clear();

  EXPECT_EQ(set.size(), 0);
  EXPECT_FALSE(set.Contains(R(1)));
  EXPECT_FALSE(set.Contains(R(6)));
  EXPECT_THAT(Held(set), IsEmpty());

  // Clearing leaves the room it had, which is what lets one set serve block
  // after block.
  set.Insert(R(6));

  EXPECT_THAT(Held(set), UnorderedElementsEqual(R(6)));
}

// A register's id is what the set holds it by, which is what lets the id be
// an index. Two registers never share an id, so the width is not asked for.
TEST(Test, RegSetHoldsARegisterByItsId) {
  RegSet set(8);

  set.Insert(Reg{.id = 4, .size = RegSize32});

  EXPECT_TRUE(set.Contains(Reg{.id = 4, .size = RegSize64}));

  set.Remove(Reg{.id = 4, .size = RegSize64});

  EXPECT_EQ(set.size(), 0);
}

// Put in and taken out in whatever order, against a set that is known to be
// right, because what a register is held by moves as others are taken out.
TEST(Test, RegSetAgreesWithAPlainSetOverManyChanges) {
  static constexpr std::int32_t kRegs = 32;

  RegSet set(kRegs);
  std::set<std::int32_t> expected;
  std::mt19937 random(7);

  for (int step = 0; step < 4000; ++step) {
    const std::int32_t id = random() % kRegs;
    if (random() % 2 == 0) {
      set.Insert(R(id));
      expected.insert(id);
    } else {
      set.Remove(R(id));
      expected.erase(id);
    }

    ASSERT_EQ(set.size(), expected.size());
    ASSERT_TRUE(set.Contains(R(id)) == expected.contains(id));
  }

  std::set<std::int32_t> held;
  for (Reg reg : set) held.insert(reg.id);

  EXPECT_TRUE(held == expected);
}

}  // namespace
}  // namespace lucid
