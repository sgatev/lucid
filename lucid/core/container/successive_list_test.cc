#include "lucid/core/container/successive_list.h"

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, SuccessiveListDefaultConstructedListIsEmpty) {
  SuccessiveList<int> list;
  EXPECT_THAT(list, IsEmpty());
  EXPECT_THAT(list, SizeIs(0));
}

TEST(Test, SuccessiveListNonEmptyListContainsSuccessiveValues) {
  SuccessiveList<int> list(6, 21);
  EXPECT_THAT(list, Not(IsEmpty()));
  EXPECT_THAT(list, SizeIs(6));
  EXPECT_THAT(list, ElementsAre(21, 22, 23, 24, 25, 26));
  EXPECT_EQ(list[3], 24);
}

}  // namespace
}  // namespace lucid
