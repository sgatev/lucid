#include "lucid/worklist.h"

#include <functional>

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(WorklistTest, Empty) {
  Worklist<int, std::less<>> worklist(std::less<>{});

  EXPECT_TRUE(worklist.empty());
}

TEST(WorklistTest, Ordered) {
  Worklist<int, std::less<>> worklist(std::less<>{});

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
  Worklist<int, std::less<>> worklist(std::less<>{});

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
  Worklist<int, std::less<>> worklist(std::less<>{});

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
