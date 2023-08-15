#include "lucid/cli.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/test_cli.h"

namespace lucid {
namespace {

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Eq;

TEST(RunCommandTest, RunsCommand) {
  std::vector<std::string> foo_args;
  auto foo = [&foo_args](std::span<std::string_view> args) {
    foo_args.assign(args.begin(), args.end());
    return CommandResult{.return_code = 0};
  };
  auto bar = [](std::span<std::string_view> args) {
    return CommandResult{.return_code = 1};
  };
  std::vector<std::string_view> args = {"foo", "bar", "baz"};
  EXPECT_THAT(RunCommand("test",
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
                         args),
              ReturnsCode(Eq(0)));
  EXPECT_THAT(foo_args, ElementsAre("bar", "baz"));
}

TEST(RunCommandTest, UnknownCommand) {
  auto bar = [](std::span<std::string_view> args) {
    return CommandResult{.return_code = 0};
  };
  std::vector<std::string_view> args = {"foo", "bar", "baz"};
  EXPECT_THAT(
      RunCommand("test",
                 {
                     {
                         .name = "bar",
                         .handler = bar,
                     },
                 },
                 args),
      AllOf(ReturnsCode(Eq(1)), PrintsError(Eq("unknown command: foo"))));
}

TEST(RunCommandTest, EmptyArgs) {
  auto foo = [](std::span<std::string_view> args) {
    return CommandResult{.return_code = 1};
  };
  std::vector<std::string_view> args = {};
  EXPECT_THAT(RunCommand("test",
                         {
                             {.name = "foo", .handler = foo},
                         },
                         args),
              ReturnsCode(Eq(0)));
}

}  // namespace
}  // namespace lucid
