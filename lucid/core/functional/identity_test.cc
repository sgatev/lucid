#include "lucid/core/functional/identity.h"

#include <string>
#include <utility>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, IdentityBasic) {
  int x = 42;
  int y = identity(x);
  EXPECT_EQ(y, 42);
}

TEST(Test, IdentityConstReference) {
  std::string x = "foo";
  const std::string& y = identity(x);
  EXPECT_EQ(y, "foo");
}

TEST(Test, IdentityLvalueReference) {
  int x = 42;
  int& y = identity(x);
  y = 21;
  EXPECT_EQ(x, 21);
}

TEST(Test, IdentityRvalueReference) {
  std::string x = "foo";
  std::string y = identity(std::move(x));
  EXPECT_EQ(y, "foo");
}

}  // namespace
}  // namespace lucid
