#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {

TEST(Test, Foo) { std::cout << "Foo LOG" << '\n'; }

TEST(Test, Bar) { std::cout << "Bar LOG" << '\n'; }

TEST(Test, Baz) { std::cout << "Baz LOG" << '\n'; }

TEST(Test, Boolean) {
  bool truthy = 21 == 21;
  EXPECT_THAT(truthy, IsTrue());
  EXPECT_THAT(!truthy, IsFalse());
}

TEST(Test, Predicate) {
  int twenty_one = 21;
  EXPECT_THAT(twenty_one, Truly([](int x) { return x % 3 == 0; }));
}

TEST(Test, Equality) {
  int twenty_one = 21;
  EXPECT_THAT(twenty_one, Equals(21));
  EXPECT_THAT(twenty_one, NotEquals(42));
}

TEST(Test, Size) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values, SizeIs(2));
}

TEST(Test, Empty) {
  std::vector<int> values;
  EXPECT_THAT(values, IsEmpty());
}

TEST(Test, Elements) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values, Elements(Equals(1), Truly([](int x) { return x > 1; })));
  EXPECT_THAT(values, ElementsEqual(1, 2));
}

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
  EXPECT_THAT(values,
              UnorderedElements(Truly([](int x) { return x > 0; }), Equals(2)));
}

TEST(Test, Field) {
  struct Point {
    int x;
    int y;
  };
  Point point = {1, 2};
  EXPECT_THAT(point, Field(&Point::x, Equals(1)));
}

TEST(Test, Optional) {
  std::optional<int> value = 1;
  EXPECT_THAT(value, Optional(Equals(1)));
}

TEST(Test, Variant) {
  std::variant<int, char> value = 1;
  EXPECT_THAT(value, Variant<int>(Equals(1)));
}

TEST(Test, Pair) {
  std::pair<int, char> value = {1, 'a'};
  EXPECT_THAT(value, Pair(Equals(1), Equals('a')));
}

TEST(Test, All) {
  std::vector<int> values = {1};
  EXPECT_THAT(values, AllOf(SizeIs(1), Not(IsEmpty())));
}

TEST(Test, Not) {
  int twenty_one = 21;
  EXPECT_THAT(twenty_one, Not(Equals(42)));
}

TEST(Test, Affixes) {
  std::string greeting = "hello";
  EXPECT_THAT(greeting, StartsWith("he"));
  EXPECT_THAT(greeting, EndsWith("lo"));
  EXPECT_THAT(greeting, Contains("ell"));
}

TEST(Test, ExpectMacros) {
  bool truthy = 21 == 21;
  int twenty_one = 21;
  EXPECT_TRUE(truthy);
  EXPECT_FALSE(!truthy);
  EXPECT_EQ(twenty_one, 21);
  EXPECT_NE(twenty_one, 42);
}

TEST(Test, AssertMacros) {
  bool truthy = 21 == 21;
  int twenty_one = 21;
  std::vector<int> values;
  ASSERT_TRUE(truthy);
  ASSERT_FALSE(!truthy);
  ASSERT_EQ(twenty_one, 21);
  ASSERT_NE(twenty_one, 42);
  ASSERT_THAT(values, IsEmpty());
}

}  // namespace lucid
