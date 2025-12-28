#include "lucid/optional_ref.h"

#include <optional>
#include <string>
#include <utility>

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(OptionalRef, Default) {
  int v = 21;
  OptionalRef<int> o;

  EXPECT_FALSE(o.has_value());

  EXPECT_EQ(o, OptionalRef<int>());
  EXPECT_EQ(o, std::nullopt);

  EXPECT_NE(o, OptionalRef<int>(v));
  EXPECT_NE(o, 21);
}

TEST(OptionalRef, Unengaged) {
  int v = 21;
  OptionalRef<int> o = std::nullopt;

  EXPECT_FALSE(o.has_value());

  EXPECT_EQ(o, OptionalRef<int>());
  EXPECT_EQ(o, std::nullopt);

  EXPECT_NE(o, OptionalRef<int>(v));
  EXPECT_NE(o, 21);
}

TEST(OptionalRef, Engaged) {
  int v = 21;
  OptionalRef<int> o = v;

  ASSERT_TRUE(o.has_value());
  EXPECT_EQ(*o, 21);

  EXPECT_EQ(o, OptionalRef<int>(v));
  EXPECT_EQ(o, 21);

  EXPECT_NE(o, OptionalRef<int>());
  EXPECT_NE(o, std::nullopt);
}

TEST(OptionalRef, Arrow) {
  struct S {
    int a;
  } v = {.a = 21};
  OptionalRef<S> o = v;

  ASSERT_TRUE(o.has_value());
  EXPECT_EQ(o->a, 21);
}

TEST(OptionalRef, Copy) {
  int v = 21;
  OptionalRef<int> o1 = v;
  OptionalRef<int> o2 = o1;

  ASSERT_TRUE(o2.has_value());
  EXPECT_EQ(*o2, 21);

  EXPECT_EQ(o2, OptionalRef<int>(v));
  EXPECT_EQ(o2, 21);

  EXPECT_NE(o2, OptionalRef<int>());
  EXPECT_NE(o2, std::nullopt);
}

TEST(OptionalRef, Move) {
  int v = 21;
  OptionalRef<int> o1 = v;
  OptionalRef<int> o2 = std::move(o1);

  ASSERT_TRUE(o2.has_value());
  EXPECT_EQ(*o2, 21);

  EXPECT_EQ(o2, OptionalRef<int>(v));
  EXPECT_EQ(o2, 21);

  EXPECT_NE(o2, OptionalRef<int>());
  EXPECT_NE(o2, std::nullopt);
}

TEST(OptionalRef, Transform) {
  struct Foo {
    std::string bar;
  };

  Foo foo{.bar = "foobar"};
  OptionalRef<Foo> o1 = foo;

  OptionalRef<std::string> o2 = o1.transform<std::string>(
      [](Foo& foo) -> std::string& { return foo.bar; });

  ASSERT_TRUE(o2.has_value());
  EXPECT_EQ(*o2, "foobar");
}

}  // namespace
}  // namespace lucid
