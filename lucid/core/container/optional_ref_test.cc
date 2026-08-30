#include "lucid/core/container/optional_ref.h"

#include <optional>
#include <string>
#include <utility>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, OptionalRefDefault) {
  int v = 21;
  OptionalRef<int> o;

  EXPECT_FALSE(o.has_value());

  EXPECT_EQ(o, OptionalRef<int>());
  EXPECT_EQ(o, std::nullopt);

  EXPECT_NE(o, OptionalRef<int>(v));
  EXPECT_NE(o, 21);
}

TEST(Test, OptionalRefUnengaged) {
  int v = 21;
  OptionalRef<int> o = std::nullopt;

  EXPECT_FALSE(o.has_value());

  EXPECT_EQ(o, OptionalRef<int>());
  EXPECT_EQ(o, std::nullopt);

  EXPECT_NE(o, OptionalRef<int>(v));
  EXPECT_NE(o, 21);
}

TEST(Test, OptionalRefEngaged) {
  int v = 21;
  OptionalRef<int> o = v;

  ASSERT_TRUE(o.has_value());
  EXPECT_EQ(*o, 21);

  EXPECT_EQ(o, OptionalRef<int>(v));
  EXPECT_EQ(o, 21);

  EXPECT_NE(o, OptionalRef<int>());
  EXPECT_NE(o, std::nullopt);
}

TEST(Test, OptionalRefArrow) {
  struct S {
    int a;
  } v = {.a = 21};
  OptionalRef<S> o = v;

  ASSERT_TRUE(o.has_value());
  EXPECT_EQ(o->a, 21);
}

TEST(Test, OptionalRefCopy) {
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

TEST(Test, OptionalRefMove) {
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

TEST(Test, OptionalRefTransform) {
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

TEST(Test, OptionalRefValueOr) {
  OptionalRef<int> o1;
  EXPECT_EQ(o1.value_or(21), 21);

  int v = 42;
  OptionalRef<int> o2 = v;
  EXPECT_EQ(o2.value_or(21), 42);
}

}  // namespace
}  // namespace lucid
