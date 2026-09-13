#include "lucid/core/cli/cli.h"

#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, RunCommandRootCommand) {
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
                        .flags = {{
                            .name = "foo_flag",
                        }},
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            0);
  EXPECT_EQ(foo_flag_value, "foo_value");
  EXPECT_EQ(out.str(), "");
  EXPECT_EQ(err.str(), "");
}

TEST(Test, RunCommandNestedCommand) {
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
                .flags = {{
                    .name = "bar_flag",
                }},
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
                        .flags = {{
                            .name = "foo_flag",
                        }},
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

TEST(Test, RunCommandUnknownCommand) {
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

TEST(Test, RunCommandFlagWithoutValue) {
  std::vector<std::string_view> args = {"foo", "--foo_flag"};

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
                        .flags = {{
                            .name = "foo_flag",
                        }},
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            0);
  // Present, but with no value, so that a command can tell the two apart.
  EXPECT_THAT(foo_flag_value, Optional(Equals("")));
  EXPECT_EQ(err.str(), "");
}

TEST(Test, RunCommandUnknownFlag) {
  std::vector<std::string_view> args = {"foo", "--baz_flag=baz_value"};

  bool has_run = false;
  auto foo = [&](const CommandContext&) {
    has_run = true;
    return 0;
  };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "foo",
                        .flags = {{
                            .name = "foo_flag",
                        }},
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            1);
  EXPECT_THAT(has_run, IsFalse());
  EXPECT_EQ(out.str(), "");
  EXPECT_EQ(std::string(err.str()),
            "\33[31mERROR:\33[m unknown flag '--baz_flag'\n");
}

TEST(Test, RunCommandFlagNotDeclaredByCommandWithNoFlags) {
  std::vector<std::string_view> args = {"foo", "--any_flag"};

  auto foo = [](const CommandContext&) { return 0; };

  std::stringstream out, err;
  EXPECT_EQ(RunCommand(
                {
                    {
                        .name = "foo",
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            1);
  EXPECT_EQ(std::string(err.str()),
            "\33[31mERROR:\33[m unknown flag '--any_flag'\n");
}

TEST(Test, RunCommandOutput) {
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

TEST(Test, RunCommandError) {
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

TEST(Test, RunCommandCurrentCommand) {
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
                        .flags = {{.name = "foo_flag"}},
                        .handler = foo,
                    },
                },
                CommandContext({"test"}, args, {}, out, err)),
            0);
  EXPECT_EQ(bar_current_command, "test foo bar");
}

TEST(Test, RunCommandEmptyArgs) {
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
