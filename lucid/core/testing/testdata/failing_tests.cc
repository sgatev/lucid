#include <cstdint>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/core/testing/testing.h"

namespace lucid {

// A printable value, for checking how a failure renders one.
struct Point {
  int x;

  bool operator==(const Point&) const = default;

  friend std::ostream& operator<<(std::ostream& out, const Point& point) {
    return out << "Point{" << point.x << "}";
  }
};

// An unprintable value, for checking the fallback.
struct Opaque {
  int x;

  bool operator==(const Opaque&) const = default;
};

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

TEST(Test, UnorderedElementsMismatch) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values, UnorderedElementsEqual(2, 3));
}

// Both matchers accept the single 1, but only one of them can have it.
TEST(Test, UnorderedElementsReusedElement) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values, UnorderedElementsEqual(1, 1));
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

// A type is rendered through its `operator<<` where it has one.
TEST(Test, PrintableValue) {
  Point point = {1};
  EXPECT_THAT(point, Equals(Point{2}));
}

// A type with neither an `operator<<` nor a string form falls back to a
// placeholder, so that a value of any type can still be matched.
TEST(Test, UnprintableValue) {
  Opaque opaque = {1};
  EXPECT_THAT(opaque, Equals(Opaque{2}));
}

TEST(Test, Falsy) {
  bool truthy = 21 == 21;
  EXPECT_THAT(truthy, IsFalse());
}

TEST(Test, ElementsWithMatchers) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values, Elements(Equals(1), Truly([](int x) { return x > 5; })));
}

TEST(Test, UnorderedElementsWithMatchers) {
  std::vector<int> values = {1, 2};
  EXPECT_THAT(values,
              UnorderedElements(Equals(3), Truly([](int x) { return x > 5; })));
}

TEST(Test, Prefix) {
  std::string greeting = "hello";
  EXPECT_THAT(greeting, StartsWith("bye"));
}

TEST(Test, Suffix) {
  std::string greeting = "hello";
  EXPECT_THAT(greeting, EndsWith("bye"));
}

// A failing ASSERT returns from the test, so the log below must not appear.
TEST(Test, AssertStopsTheTest) {
  bool falsy = 21 == 42;
  ASSERT_TRUE(falsy);
  std::cout << "AssertStopsTheTest unreachable LOG" << '\n';
}

// A failing EXPECT carries on, so both failures and the log must appear.
TEST(Test, ExpectContinuesAfterFailure) {
  int twenty_one = 21;
  EXPECT_EQ(twenty_one, 42);
  EXPECT_NE(twenty_one, 21);
  std::cout << "ExpectContinuesAfterFailure reached LOG" << '\n';
}

// Output on both standard streams is captured and kept in order, so that a
// failure reports everything the test printed rather than just its stdout.
TEST(Test, StandardErrorOutput) {
  std::cout << "StandardErrorOutput stdout LOG" << '\n';
  std::cerr << "StandardErrorOutput stderr LOG" << '\n';
  Fail("StandardErrorOutput failure");
}

// A scoped enumeration with no `operator<<`, for checking the numeric
// fallback.
enum class Fruit : std::uint8_t { kApple, kPear };

// A scoped enumeration has no `operator<<` of its own, so it is reported as
// its numeric value rather than as a placeholder.
TEST(Test, EnumValue) {
  Fruit fruit = Fruit::kApple;
  EXPECT_THAT(fruit, Equals(Fruit::kPear));
}

// A scoped enumeration with an `operator<<`, for checking that names win.
enum class Vegetable : std::uint8_t { kLeek, kBean };

inline std::ostream& operator<<(std::ostream& out, Vegetable vegetable) {
  return out << (vegetable == Vegetable::kLeek ? "kLeek" : "kBean");
}

// An enumeration that is printable keeps its names.
TEST(Test, PrintableEnumValue) {
  Vegetable vegetable = Vegetable::kLeek;
  EXPECT_THAT(vegetable, Equals(Vegetable::kBean));
}

}  // namespace lucid
