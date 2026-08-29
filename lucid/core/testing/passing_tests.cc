#include <iostream>

#include "lucid/core/testing/testing.h"

namespace lucid {

TEST(Test, Foo) { std::cout << "Foo LOG" << '\n'; }

TEST(Test, Bar) { std::cout << "Bar LOG" << '\n'; }

TEST(Test, Baz) { std::cout << "Baz LOG" << '\n'; }

}  // namespace lucid
