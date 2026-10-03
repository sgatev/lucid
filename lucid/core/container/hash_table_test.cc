#include "lucid/core/container/hash_table.h"

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {

// A value that shares its slot with every other one, so that they queue along
// a single probe chain and a removal leaves a slot behind in the middle of it.
struct Colliding {
  int id;

  bool operator==(const Colliding&) const = default;
};

inline std::size_t Hash(const Colliding&) { return 7; }

namespace {

// A capacity is not something a table converts from: `HashTable<int> t = 64`
// would otherwise compile, and read like a copy while meaning nothing of the
// sort.
static_assert(!std::is_convertible_v<std::size_t, HashTable<int>>);

TEST(Test, HashTableTakesACapacityItCanAddress) {
  HashTable<int> table(100);

  // A mask of 99 reaches 16 of 100 slots, so the table takes the next power
  // of two over what it was asked for.
  EXPECT_EQ(table.capacity(), 128);

  // What it holds is reachable, where before it would fill the slots it could
  // reach and then search for a free one forever.
  for (int i = 0; i < 100; ++i) EXPECT_TRUE(table.Insert(i));
  EXPECT_EQ(table.size(), 100);
  for (int i = 0; i < 100; ++i) EXPECT_TRUE(table.Find(i) != table.end());
}

TEST(Test, HashTableHoldsNoSlotsUntilItIsGivenAValue) {
  HashTable<int> table;

  EXPECT_EQ(table.capacity(), 0);
  EXPECT_EQ(table.size(), 0);
  EXPECT_TRUE(table.begin() == table.end());

  // Nothing is there to find or to take out, and looking costs no storage.
  EXPECT_TRUE(table.Find(1) == table.end());
  EXPECT_THAT(table.Remove(1), Equals(std::nullopt));
  EXPECT_EQ(table.capacity(), 0);
}

TEST(Test, HashTableTakesSlotsForTheFirstValue) {
  HashTable<int> table;

  EXPECT_TRUE(table.Insert(1));

  EXPECT_TRUE(table.capacity() > 0);
  EXPECT_EQ(table.size(), 1);
  EXPECT_TRUE(table.Find(1) != table.end());
}

TEST(Test, HashTableCopiedFromAnEmptyOneHoldsNoSlots) {
  const HashTable<int> table;
  HashTable<int> copy = table;

  EXPECT_EQ(copy.capacity(), 0);
  EXPECT_TRUE(copy == table);

  // The copy takes its own slots when it is given something, rather than
  // sharing whatever the original would have had.
  EXPECT_TRUE(copy.Insert(1));
  EXPECT_EQ(table.size(), 0);
}

TEST(Test, HashTableMovedFromHoldsNoSlots) {
  HashTable<int> table;
  table.Insert(1);

  HashTable<int> moved = std::move(table);

  // The storage went with the values, so what is left describes itself: a
  // table with no slots, which can be walked and filled again.
  EXPECT_EQ(table.capacity(), 0);
  EXPECT_EQ(table.size(), 0);
  EXPECT_TRUE(table.begin() == table.end());

  EXPECT_TRUE(table.Insert(2));
  EXPECT_TRUE(table.Find(2) != table.end());
  EXPECT_TRUE(table.Find(1) == table.end());
  EXPECT_TRUE(moved.Find(1) != moved.end());
}

TEST(Test, HashTableGrowsWhenItsValuesFillIt) {
  HashTable<int> table;
  table.Insert(0);
  const std::size_t capacity = table.capacity();

  for (std::size_t i = 1; i < capacity; ++i) table.Insert(static_cast<int>(i));

  EXPECT_TRUE(table.capacity() > capacity);
  EXPECT_EQ(table.size(), capacity);
}

// Returns the values of `table` in the order it is walked in.
std::vector<int> WalkOrder(const HashTable<int>& table) {
  std::vector<int> order;
  for (int value : table) order.push_back(value);
  return order;
}

// A value the table already holds is found rather than inserted, so it leaves
// the table as it was: a rebuild moves every value, and with them the order the
// table is walked in. Checked at every size up to a few growths, which takes
// in the moment the table is just past what it holds before it grows.
TEST(Test, HashTableDoesNotRebuildForAValueItHolds) {
  HashTable<int> table;
  for (int count = 1; count < 300; ++count) {
    table.Insert(count - 1);
    const std::size_t capacity = table.capacity();
    const std::vector<int> order = WalkOrder(table);

    for (int i = 0; i < count; ++i) EXPECT_FALSE(table.Insert(i));

    EXPECT_EQ(table.capacity(), capacity);
    EXPECT_EQ(WalkOrder(table), order);
  }
}

TEST(Test, HashTableDoesNotGrowForTheSlotsARemovalEmptied) {
  HashTable<int> table;

  // The slots the table takes for the first value it is given.
  EXPECT_TRUE(table.Insert(0));
  EXPECT_THAT(table.Remove(0), Optional(Equals(0)));
  const std::size_t capacity = table.capacity();

  // Many times more pairs than the table has slots. Each one empties the slot
  // it filled, so the table is never holding more than a single value and has
  // no reason to ask for more room.
  for (int i = 1; i < 10000; ++i) {
    EXPECT_TRUE(table.Insert(i));
    EXPECT_THAT(table.Remove(i), Optional(Equals(i)));
  }

  EXPECT_EQ(table.size(), 0);
  EXPECT_EQ(table.capacity(), capacity);
}

TEST(Test, HashTableKeepsItsValuesWhenItRebuilds) {
  HashTable<int> table;

  // A few values that stay, against a stream that comes and goes and leaves
  // the table rebuilding itself over the slots that stream emptied.
  for (int i = 0; i < 8; ++i) table.Insert(i);
  const std::size_t capacity = table.capacity();
  for (int i = 1000; i < 5000; ++i) {
    table.Insert(i);
    table.Remove(i);
  }

  EXPECT_EQ(table.capacity(), capacity);
  EXPECT_EQ(table.size(), 8);

  for (int i = 0; i < 8; ++i) EXPECT_TRUE(table.Find(i) != table.end());
}

TEST(Test, HashTableFindsAValuePastASlotARemovalEmptied) {
  HashTable<Colliding> table;

  EXPECT_TRUE(table.Insert(Colliding{1}));
  EXPECT_TRUE(table.Insert(Colliding{2}));

  // The first of them held the head of the chain the second one is on, so
  // removing it leaves the second sitting behind an emptied slot.
  EXPECT_THAT(table.Remove(Colliding{1}), Optional(Equals(Colliding{1})));

  // The search has to run past that slot to the value still behind it, rather
  // than take the slot and leave the table holding the value twice.
  EXPECT_FALSE(table.Insert(Colliding{2}));
  EXPECT_EQ(table.size(), 1);

  // One removal takes it out, because there is only the one of it.
  EXPECT_THAT(table.Remove(Colliding{2}), Optional(Equals(Colliding{2})));
  EXPECT_EQ(table.size(), 0);
  EXPECT_TRUE(table.Find(Colliding{2}) == table.end());
}

TEST(Test, HashTableFindsValuesPastManySlotsRemovalsEmptied) {
  HashTable<Colliding> table;

  for (int i = 0; i < 8; ++i) EXPECT_TRUE(table.Insert(Colliding{i}));

  // Empty the front half of the chain, leaving the back half behind it.
  for (int i = 0; i < 4; ++i) {
    EXPECT_THAT(table.Remove(Colliding{i}), Optional(Equals(Colliding{i})));
  }
  EXPECT_EQ(table.size(), 4);

  for (int i = 4; i < 8; ++i) EXPECT_FALSE(table.Insert(Colliding{i}));
  EXPECT_EQ(table.size(), 4);
}

TEST(Test, HashTableGrowsForValuesThatStay) {
  HashTable<int> table;
  table.Insert(0);
  const std::size_t capacity = table.capacity();

  // The same stream, but nothing is removed, so the room a rebuild would hand
  // back is not there to hand back and the table has to grow.
  for (int i = 1; i < 5000; ++i) table.Insert(i);

  EXPECT_TRUE(table.capacity() > capacity);
  EXPECT_EQ(table.size(), 5000);

  for (int i = 0; i < 5000; ++i) EXPECT_TRUE(table.Find(i) != table.end());
}

}  // namespace
}  // namespace lucid
