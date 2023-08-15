#include "lucid/cli.h"

#include <span>
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
  auto foo = [&foo_args](std::span<std::string_view> args) {
    foo_args.assign(args.begin(), args.end());
    return std::nullopt;
  };
  auto bar = [](std::span<std::string_view> args) { return "error"; };
  std::vector<std::string_view> args = {"foo", "bar", "baz"};
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
                       args),
            std::nullopt);
  EXPECT_THAT(foo_args, ElementsAre("bar", "baz"));
}

TEST(RunCommandTest, UnknownCommand) {
  auto bar = [](std::span<std::string_view> args) { return "error"; };
  std::vector<std::string_view> args = {"foo", "bar", "baz"};
  EXPECT_EQ(RunCommand("test",
                       {
                           {
                               .name = "bar",
                               .handler = bar,
                           },
                       },
                       args),
            "unknown command: foo");
}

TEST(RunCommandTest, EmptyArgs) {
  auto foo = [](std::span<std::string_view> args) { return std::nullopt; };
  std::vector<std::string_view> args = {};
  EXPECT_EQ(RunCommand("test",
                       {
                           {.name = "foo", .handler = foo},
                       },
                       args),
            std::nullopt);
}

}  // namespace
}  // namespace lucid
