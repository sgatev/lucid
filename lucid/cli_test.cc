#include "lucid/cli.h"

#include <span>
#include <string>
#include <string_view>
#include <strstream>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

TEST(RunCommandTest, RunsCommand) {
  std::vector<std::string> foo_args;
  auto foo = [&foo_args](CommandContext ctx) {
    foo_args.assign(ctx.args.begin(), ctx.args.end());
    return 0;
  };
  auto bar = [](CommandContext) { return 1; };
  std::vector<std::string_view> args = {"foo", "bar", "baz"};
  std::strstream out, err;
  EXPECT_EQ(RunCommand("test",
                       {
                           {
                               .name = "foo",
                               .handler = foo,
                           },
                           {
                               .name = "bar",
                               .handler = bar,
                           },
                       },
                       {args, out, err}),
            0);
  EXPECT_THAT(foo_args, ElementsAre("bar", "baz"));
}

TEST(RunCommandTest, UnknownCommand) {
  auto bar = [](CommandContext) { return 0; };
  std::vector<std::string_view> args = {"foo", "bar", "baz"};
  std::strstream out, err;
  EXPECT_EQ(RunCommand("test",
                       {
                           {
                               .name = "bar",
                               .handler = bar,
                           },
                       },
                       {args, out, err}),
            1);
  EXPECT_EQ(std::string(err.str()), "unknown command: foo\n");
}

TEST(RunCommandTest, EmptyArgs) {
  auto foo = [](CommandContext) { return 1; };
  std::vector<std::string_view> args = {};
  std::strstream out, err;
  EXPECT_EQ(RunCommand("test",
                       {
                           {.name = "foo", .handler = foo},
                       },
                       {args, out, err}),
            0);
}

}  // namespace
}  // namespace lucid
