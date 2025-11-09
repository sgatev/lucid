#include "lucid/string_index.h"

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(StringIndexTest, SameStringRef) {
  StringIndex index;
  auto foo1 = index.ref("foo");
  auto foo2 = index.ref("foo");
  EXPECT_EQ(foo1, foo2);
}

TEST(StringIndexTest, PrefixRef) {
  StringIndex index;
  auto foobar = index.ref("foobar");
  auto foo = index.ref("foo");
  EXPECT_NE(foobar, foo);
}

TEST(StringIndexTest, DifferentStringRef) {
  StringIndex index;
  auto foo = index.ref("foo");
  auto bar = index.ref("bar");
  EXPECT_NE(foo, bar);
}

TEST(StringIndexTest, Deref) {
  StringIndex index;
  index.ref("foobar");
  auto foo = index.ref("foo");
  EXPECT_EQ(index.deref(foo), "foo");
}

TEST(StringIndexTest, RefSize) {
  StringIndex index;
  auto foo = index.ref("foo");
  EXPECT_EQ(sizeof(foo), 8);
}

}  // namespace
}  // namespace lucid
