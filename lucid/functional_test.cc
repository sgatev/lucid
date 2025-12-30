#include "lucid/functional.h"

#include <string>
#include <utility>

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(IdentityTest, Basic) {
  int x = 42;
  int y = identity(x);
  EXPECT_EQ(y, 42);
}

TEST(IdentityTest, ConstReference) {
  std::string x = "foo";
  const std::string& y = identity(x);
  EXPECT_EQ(y, "foo");
}

TEST(IdentityTest, LvalueReference) {
  int x = 42;
  int& y = identity(x);
  y = 21;
  EXPECT_EQ(x, 21);
}

TEST(IdentityTest, RvalueReference) {
  std::string x = "foo";
  std::string y = identity(std::move(x));
  EXPECT_EQ(y, "foo");
}

}  // namespace
}  // namespace lucid
