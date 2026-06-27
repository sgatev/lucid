#include "lucid/core/cli/cli.h"

#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/core/container/hash_map.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

TEST(RunCommandTest, RootCommand) {
  std::vector<std::string_view> args = {
      "foo",
      "--foo_flag=foo_value",
  };

  std::optional<std::string_view> foo_flag_value;
  auto foo = [&](CommandContext ctx) {
    foo_flag_value = ctx.Flag("foo_flag");
    return 0;
  };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "foo",
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            0);
  EXPECT_EQ(foo_flag_value, "foo_value");
  EXPECT_EQ(out.str(), "");
  EXPECT_EQ(err.str(), "");
}

TEST(RunCommandTest, NestedCommand) {
  std::vector<std::string_view> args = {
      "foo",
      "--foo_flag=foo_value",
      "bar",
      "--bar_flag=bar_value",
  };

  std::optional<std::string_view> bar_flag_value;
  auto bar = [&](CommandContext ctx) {
    bar_flag_value = ctx.Flag("bar_flag");
    return 0;
  };

  std::optional<std::string_view> foo_flag_value;
  auto foo = [&](CommandContext ctx) {
    foo_flag_value = ctx.Flag("foo_flag");
    return RunCommand(
        {
            {
                .name = "bar",
                .handler = bar,
            },
        },
        ctx);
  };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "foo",
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            0);
  EXPECT_EQ(foo_flag_value, "foo_value");
  EXPECT_EQ(bar_flag_value, "bar_value");
  EXPECT_EQ(out.str(), "");
  EXPECT_EQ(err.str(), "");
}

TEST(RunCommandTest, UnknownCommand) {
  std::vector<std::string_view> args = {"foo", "bar", "baz"};

  auto bar = [](const CommandContext&) { return 0; };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "bar",
                        .handler = bar,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            1);
  EXPECT_EQ(out.str(), "");
  EXPECT_EQ(std::string(err.str()),
            "\33[31mERROR:\33[m unknown command 'foo'\n");
}

TEST(RunCommandTest, Output) {
  std::vector<std::string_view> args = {
      "foo",
  };

  auto foo = [&](CommandContext ctx) {
    ctx.Out() << "foo";
    return 0;
  };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "foo",
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            0);
  EXPECT_EQ(out.str(), "foo");
  EXPECT_EQ(err.str(), "");
}

TEST(RunCommandTest, Error) {
  std::vector<std::string_view> args = {
      "foo",
  };

  auto foo = [&](CommandContext ctx) {
    ctx.Err() << "foo";
    return 0;
  };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "foo",
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            0);
  EXPECT_EQ(out.str(), "");
  EXPECT_EQ(err.str(), "\33[31mERROR:\33[m foo");
}

TEST(RunCommandTest, CurrentCommand) {
  std::vector<std::string_view> args = {
      "foo",
      "--foo_flag=foo_value",
      "bar",
  };

  std::string bar_current_command;
  auto bar = [&](const CommandContext& ctx) {
    bar_current_command = ctx.CurrentCommand();
    return 0;
  };

  auto foo = [&](CommandContext ctx) {
    return RunCommand(
        {
            {
                .name = "bar",
                .handler = bar,
            },
        },
        std::move(ctx));
  };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "foo",
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            0);
  EXPECT_EQ(bar_current_command, "test foo bar");
}

TEST(RunCommandTest, EmptyArgs) {
  auto foo = [](const CommandContext&) { return 1; };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "foo",
                        .help = "bar",
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, {}, {}, out, err)),
            0);
  EXPECT_EQ(out.str(), R"(Usage: test <command> ...

Available commands:
  foo bar
)");
  EXPECT_EQ(err.str(), "");
}

}  // namespace
}  // namespace lucid
