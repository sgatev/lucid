#include <iostream>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {

TEST(Test, Foo) { std::cout << "Foo LOG" << '\n'; }

TEST(Test, Bar) { std::cout << "Bar LOG" << '\n'; }

TEST(Test, Baz) { std::cout << "Baz LOG" << '\n'; }

TEST(Test, UnorderedElements) {
  std::vector<int> values = {1, 2, 3};
  EXPECT_THAT(values, UnorderedElementsEqual(3, 1, 2));
}

TEST(Test, UnorderedElementsWithDuplicates) {
  std::vector<int> values = {1, 2, 1};
  EXPECT_THAT(values, UnorderedElementsEqual(1, 1, 2));
}

// The first matcher accepts either element and takes the 2, which is the only
// element the second accepts. Matching succeeds only by handing the 2 over and
// giving the first matcher the 1 instead.
TEST(Test, UnorderedElementsNeedingReassignment) {
  std::vector<int> values = {2, 1};
  EXPECT_THAT(values, UnorderedElements(Truly([](int x) { return x > 0; }),
                                        Equals(2)));
}

}  // namespace lucid
