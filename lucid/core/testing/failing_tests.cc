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
  EXPECT_THAT(21, Truly([](int x) { return false; }));
}

}  // namespace lucid
