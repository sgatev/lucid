#include "lucid/core/container/hash_table.h"

#include <cstddef>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, HashTableGrowsWhenItsValuesFillIt) {
  HashTable<int> table;
  const std::size_t capacity = table.capacity();

  for (std::size_t i = 0; i < capacity; ++i) table.Insert(static_cast<int>(i));

  EXPECT_TRUE(table.capacity() > capacity);
  EXPECT_EQ(table.size(), capacity);
}

TEST(Test, HashTableDoesNotGrowForTheSlotsARemovalEmptied) {
  HashTable<int> table;
  const std::size_t capacity = table.capacity();

  // Many times more pairs than the table has slots. Each one empties the slot
  // it filled, so the table is never holding more than a single value and has
  // no reason to ask for more room.
  for (int i = 0; i < 10000; ++i) {
    EXPECT_TRUE(table.Insert(i));
    EXPECT_THAT(table.Remove(i), Optional(Equals(i)));
  }

  EXPECT_EQ(table.size(), 0);
  EXPECT_EQ(table.capacity(), capacity);
}

TEST(Test, HashTableKeepsItsValuesWhenItRebuilds) {
  HashTable<int> table;
  const std::size_t capacity = table.capacity();

  // A few values that stay, against a stream that comes and goes and leaves
  // the table rebuilding itself over the slots that stream emptied.
  for (int i = 0; i < 8; ++i) table.Insert(i);
  for (int i = 1000; i < 5000; ++i) {
    table.Insert(i);
    table.Remove(i);
  }

  EXPECT_EQ(table.capacity(), capacity);
  EXPECT_EQ(table.size(), 8);

  for (int i = 0; i < 8; ++i) EXPECT_TRUE(table.Find(i) != table.end());
}

TEST(Test, HashTableGrowsForValuesThatStay) {
  HashTable<int> table;
  const std::size_t capacity = table.capacity();

  // The same stream, but nothing is removed, so the room a rebuild would hand
  // back is not there to hand back and the table has to grow.
  for (int i = 0; i < 5000; ++i) table.Insert(i);

  EXPECT_TRUE(table.capacity() > capacity);
  EXPECT_EQ(table.size(), 5000);

  for (int i = 0; i < 5000; ++i) EXPECT_TRUE(table.Find(i) != table.end());
}

}  // namespace
}  // namespace lucid
