#include "lucid/result.h"

#include <utility>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

struct FooErr {};
struct BarErr {};

TEST(ResultTest, Value) {
  Result<int, FooErr, BarErr> e = 21;

  EXPECT_FALSE(e.HasError());
  ASSERT_TRUE(e.HasValue());
  EXPECT_EQ(e.GetValue(), 21);
}

TEST(ResultTest, Error) {
  Result<int, FooErr, BarErr> e = FooErr{};

  EXPECT_FALSE(e.HasValue());
  EXPECT_TRUE(e.HasError());
  EXPECT_TRUE(e.HasError<FooErr>());
}

TEST(ResultTest, Conversion) {
  Result<int, FooErr> e1 = FooErr{};
  Result<int, FooErr, BarErr> e2 = e1;

  EXPECT_TRUE(e2.HasError());
  EXPECT_TRUE(e2.HasError<FooErr>());
}

}  // namespace
}  // namespace lucid
