#include "lucid/worklist.h"

#include <cstddef>
#include <functional>

#include "gtest/gtest.h"

namespace lucid {
namespace {

class BoundedNatDomain {
 public:
  explicit BoundedNatDomain(std::size_t size) : size_(size) {}

  std::size_t size() const { return size_; }

  std::size_t id(int i) const { return i; }

 private:
  std::size_t size_;
};

TEST(WorklistTest, Empty) {
  Worklist<int, BoundedNatDomain, std::less<>> worklist(BoundedNatDomain(6),
                                                        std::less());

  EXPECT_TRUE(worklist.empty());
}

TEST(WorklistTest, Ordered) {
  Worklist<int, BoundedNatDomain, std::less<>> worklist(BoundedNatDomain(6),
                                                        std::less());

  worklist.push(5);
  worklist.push(1);
  worklist.push(4);
  worklist.push(2);
  worklist.push(3);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 5);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 4);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 3);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 2);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 1);

  EXPECT_TRUE(worklist.empty());
}

TEST(WorklistTest, NoDuplicates) {
  Worklist<int, BoundedNatDomain, std::less<>> worklist(BoundedNatDomain(6),
                                                        std::less());

  worklist.push(2);
  worklist.push(1);
  worklist.push(2);
  worklist.push(3);
  worklist.push(1);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 3);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 2);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 1);

  EXPECT_TRUE(worklist.empty());
}

TEST(WorklistTest, PushRange) {
  Worklist<int, BoundedNatDomain, std::less<>> worklist(BoundedNatDomain(6),
                                                        std::less());

  worklist.push_range(std::vector<int>{5, 1, 4});
  worklist.push_range(std::vector<int>{3, 2});

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 5);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 4);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 3);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 2);

  ASSERT_FALSE(worklist.empty());
  EXPECT_EQ(worklist.pop(), 1);

  EXPECT_TRUE(worklist.empty());
}

}  // namespace
}  // namespace lucid
