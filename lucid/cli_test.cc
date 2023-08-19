#include "lucid/cli.h"

#include <span>
#include <sstream>
#include <string>
#include <string_view>
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
  std::stringstream out, err;
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
  std::stringstream out, err;
  EXPECT_EQ(RunCommand("test",
                       {
                           {
                               .name = "bar",
                               .handler = bar,
                           },
                       },
                       {args, out, err}),
            1);
  EXPECT_EQ(std::string(err.str()),
            "\033[31mERROR:\033[0m unknown command 'foo'\n");
}

TEST(RunCommandTest, EmptyArgs) {
  auto foo = [](CommandContext) { return 1; };
  std::vector<std::string_view> args = {};
  std::stringstream out, err;
  EXPECT_EQ(RunCommand("test",
                       {
                           {.name = "foo", .handler = foo},
                       },
                       {args, out, err}),
            0);
}

TEST(PrintErrorTest, Works) {
  std::stringstream out;
  PrintError(out) << "foo";
  EXPECT_EQ(std::string(out.str()), "\033[31mERROR:\033[0m foo");
}

}  // namespace
}  // namespace lucid
