#include "lucid/core/container/successive_list.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::SizeIs;

TEST(SuccessiveListTest, DefaultConstructedListIsEmpty) {
  SuccessiveList<int> list;
  EXPECT_THAT(list, IsEmpty());
  EXPECT_THAT(list, SizeIs(0));
  EXPECT_THAT(list, ElementsAre());
}

TEST(SuccessiveListTest, NonEmptyListContainsSuccessiveValues) {
  SuccessiveList<int> list(6, 21);
  EXPECT_THAT(list, Not(IsEmpty()));
  EXPECT_THAT(list, SizeIs(6));
  EXPECT_THAT(list, ElementsAre(21, 22, 23, 24, 25, 26));
  EXPECT_EQ(list[3], 24);
}

}  // namespace
}  // namespace lucid
