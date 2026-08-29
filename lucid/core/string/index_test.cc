#include "lucid/core/string/index.h"

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, StringIndexRefSize) {
  StringIndex index;
  auto foo = index.ref("foo");
  EXPECT_EQ(sizeof(foo), 8);
}

TEST(Test, StringIndexSameStringRef) {
  StringIndex index;
  auto foo1 = index.ref("foo");
  auto foo2 = index.ref("foo");
  EXPECT_EQ(foo1, foo2);
}

TEST(Test, StringIndexPrefixRef) {
  StringIndex index;
  auto foobar = index.ref("foobar");
  auto foo = index.ref("foo");
  EXPECT_NE(foobar, foo);
}

TEST(Test, StringIndexDifferentStringRef) {
  StringIndex index;
  auto foo = index.ref("foo");
  auto bar = index.ref("bar");
  EXPECT_NE(foo, bar);
}

TEST(Test, StringIndexPrefixDeref) {
  StringIndex index;
  index.ref("foobar");
  auto foo = index.ref("foo");
  EXPECT_EQ(index.deref(foo), "foo");
}

}  // namespace
}  // namespace lucid
