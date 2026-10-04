#include "lucid/core/dataflow/worklist.h"

#include <cstddef>
#include <numeric>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

class BoundedNatDomain {
 public:
  using element_type = int;

  explicit BoundedNatDomain(std::size_t size) : size_(size) {}

  std::size_t size() const { return size_; }

  std::size_t id(int i) const { return i; }

 private:
  std::size_t size_;
};

// The numbers below `size`, from the least up.
std::vector<int> Ascending(int size) {
  std::vector<int> order(size);
  std::iota(order.begin(), order.end(), 0);
  return order;
}

TEST(Test, WorklistEmpty) {
  Worklist worklist(BoundedNatDomain(6), Ascending(6));

  EXPECT_TRUE(worklist.empty());
}

TEST(Test, WorklistOrdered) {
  Worklist worklist(BoundedNatDomain(6), Ascending(6));

  worklist.push(5);
  worklist.push(1);
  worklist.push(4);
  worklist.push(2);
  worklist.push(3);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 1);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 2);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 3);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 4);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 5);

  EXPECT_TRUE(worklist.empty());
}

TEST(Test, WorklistNoDuplicates) {
  Worklist worklist(BoundedNatDomain(6), Ascending(6));

  worklist.push(2);
  worklist.push(1);
  worklist.push(2);
  worklist.push(3);
  worklist.push(1);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 1);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 2);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 3);

  EXPECT_TRUE(worklist.empty());
}

TEST(Test, WorklistPushRange) {
  Worklist worklist(BoundedNatDomain(6), Ascending(6));

  worklist.push_range(std::vector<int>{5, 1, 4});
  worklist.push_range(std::vector<int>{3, 2});

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 1);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 2);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 3);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 4);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 5);

  EXPECT_TRUE(worklist.empty());
}

// Elements come out in the order the worklist was made with, whatever order
// they went in.
TEST(Test, WorklistFollowsItsOrder) {
  Worklist worklist(BoundedNatDomain(6), std::vector<int>{4, 0, 5, 2, 1, 3});

  worklist.push(1);
  worklist.push(5);
  worklist.push(3);
  worklist.push(4);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 4);

  // One that comes earlier than what is left goes in ahead of it.
  worklist.push(0);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 0);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 5);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 1);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 3);

  EXPECT_TRUE(worklist.empty());
}

// Every element goes in at once, across more than one word of bits.
TEST(Test, WorklistPushAll) {
  Worklist worklist(BoundedNatDomain(70), Ascending(70));

  worklist.push_all();
  worklist.push(3);

  for (int i = 0; i < 70; ++i) {
    ASSERT_FALSE(worklist.empty());
    EXPECT_EQ(worklist.pop(), i);
  }
  EXPECT_TRUE(worklist.empty());
}

}  // namespace
}  // namespace lucid
