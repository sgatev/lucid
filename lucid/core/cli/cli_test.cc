#include "lucid/core/cli/cli.h"

#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

TEST(RunCommandTest, RunsCommand) {
  std::vector<std::string_view> args = {"foo", "--foo_flag=foo_value", "bar",
                                        "--bar_flag=bar_value", "baz"};

  std::span<std::string_view> foo_args;
  std::unordered_map<std::string_view, std::string_view> foo_flags;
  auto foo = [&foo_args, &foo_flags](CommandContext ctx) {
    foo_args = ctx.args;
    foo_flags = ctx.flags;
    return 0;
  };
  auto bar = [](CommandContext) { return 1; };

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
                       {.args = args, .flags = {}, .out = out, .err = err}),
            0);

  EXPECT_THAT(foo_args, ElementsAre("bar", "--bar_flag=bar_value", "baz"));
  EXPECT_THAT(foo_flags, UnorderedElementsAre(Pair("foo_flag", "foo_value")));
}

TEST(RunCommandTest, UnknownCommand) {
  auto bar = [](CommandContext) { return 0; };
  std::vector<std::string_view> args = {"foo", "bar", "baz"};
  std::unordered_map<std::string_view, std::string_view> flags = {};
  std::stringstream out, err;
  EXPECT_EQ(RunCommand("test",
                       {
                           {
                               .name = "bar",
                               .handler = bar,
                           },
                       },
                       {args, flags, out, err}),
            1);
  EXPECT_EQ(std::string(err.str()),
            "\033[31mERROR:\033[0m unknown command 'foo'\n");
}

TEST(RunCommandTest, EmptyArgs) {
  auto foo = [](CommandContext) { return 1; };
  std::vector<std::string_view> args = {};
  std::unordered_map<std::string_view, std::string_view> flags = {};
  std::stringstream out, err;
  EXPECT_EQ(RunCommand("test",
                       {
                           {.name = "foo", .handler = foo},
                       },
                       {args, flags, out, err}),
            0);
}

TEST(PrintErrorTest, Works) {
  std::stringstream out;
  PrintError(out) << "foo";
  EXPECT_EQ(std::string(out.str()), "\033[31mERROR:\033[0m foo");
}

}  // namespace
}  // namespace lucid
