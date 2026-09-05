#include <iostream>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {

TEST(Test, Foo) {
  Fail("Foo failure");
  std::cout << "Foo LOG" << '\n';
}

TEST(Test, Bar) { std::cout << "Bar LOG" << '\n'; }

TEST(Test, Baz) {
  std::cout << "Baz LOG" << '\n';
  Fail("Baz failure");
}

TEST(Test, Predicate) {
  int twenty_one = 21;
  EXPECT_THAT(twenty_one, Truly([](int x) { return false; }));
}

TEST(Test, Boolean) {
  bool falsy = 21 == 42;
  EXPECT_THAT(falsy, IsTrue());
}

TEST(Test, Equality) {
  int twenty_one = 21;
  EXPECT_THAT(twenty_one, Equals(42));
}

TEST(Test, Inequality) {
  int twenty_one = 21;
  EXPECT_THAT(twenty_one, NotEquals(21));
}

TEST(Test, Size) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values, SizeIs(3));
}

TEST(Test, Empty) {
  std::vector<int> values = {1};
  EXPECT_THAT(values, IsEmpty());
}

TEST(Test, Elements) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values, ElementsEqual(1, 3));
}

TEST(Test, FewerElementsThanMatchers) {
  std::vector<int> values = {1};
  EXPECT_THAT(values, ElementsEqual(1, 2));
}

TEST(Test, MoreElementsThanMatchers) {
  std::vector<int> values = {1, 2, 3};
  EXPECT_THAT(values, ElementsEqual(1, 2));
}

TEST(Test, UnorderedElements) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values, UnorderedElementsEqual(2));
}

TEST(Test, Field) {
  struct Point {
    int x;
    int y;
  };
  Point point = {1, 2};
  EXPECT_THAT(point, Field(&Point::x, Equals(3)));
}

TEST(Test, Optional) {
  std::optional<int> value = 1;
  EXPECT_THAT(value, Optional(Equals(2)));
}

TEST(Test, EmptyOptional) {
  std::optional<int> value;
  EXPECT_THAT(value, Optional(Equals(2)));
}

TEST(Test, Variant) {
  std::variant<int, char> value = 1;
  EXPECT_THAT(value, Variant<int>(Equals(2)));
}

TEST(Test, OtherVariantAlternative) {
  std::variant<int, char> value = 'a';
  EXPECT_THAT(value, Variant<int>(Equals(2)));
}

TEST(Test, Pair) {
  std::pair<int, char> value = {1, 'a'};
  EXPECT_THAT(value, Pair(Equals(2), Equals('b')));
}

TEST(Test, All) {
  std::vector<int> values = {1};
  EXPECT_THAT(values, AllOf(IsEmpty(), SizeIs(2)));
}

TEST(Test, Not) {
  int twenty_one = 21;
  EXPECT_THAT(twenty_one, Not(Equals(21)));
}

}  // namespace lucid
