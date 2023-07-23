#include "lucid/string_builder.h"

#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(StringBuilderTest, BuildsString) {
  StringBuilder builder;
  builder.Append("Hello");
  builder.Append(", ");
  builder.Append("world");
  builder.Append("!");

  const std::string result = std::move(builder).Build();
  EXPECT_EQ(result, "Hello, world!");
}

}  // namespace
}  // namespace lucid
