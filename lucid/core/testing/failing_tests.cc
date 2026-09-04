#include <iostream>

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
  int tewnty_one = 21;
  EXPECT_THAT(tewnty_one, Truly([](int x) { return false; }));
}

TEST(Test, Boolean) {
  bool falsy = 21 == 42;
  EXPECT_THAT(falsy, IsTrue());
}

}  // namespace lucid
