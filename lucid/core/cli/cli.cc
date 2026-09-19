#include "lucid/core/cli/cli.h"

#include <algorithm>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/core/cli/format.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/concat.h"

namespace lucid {
namespace {

// Returns whether `command` declares a flag named `flag_name`.
bool AcceptsFlag(const Command& command, std::string_view flag_name) {
  return std::ranges::any_of(
      command.flags, [&](const Flag& flag) { return flag.name == flag_name; });
}

}  // namespace

CommandContext::CommandContext(
    std::vector<std::string_view> path, std::span<std::string_view> args,
    HashMap<std::string_view, std::string_view> flags,
    // NOLINTNEXTLINE: adjacent parameters of same type ok.
    std::ostream& out, std::ostream& err)
    : path_(std::move(path)),
      args_(args),
      flags_(std::move(flags)),
      out_(out),
      err_(err) {}

std::string CommandContext::CurrentCommand() const {
  return Concat(path_, " ");
}

std::optional<std::string_view> CommandContext::TakeArg() {
  if (args_.empty()) return std::nullopt;
  std::string_view arg = args_.front();
  args_ = args_.subspan(1);
  return arg;
}

std::optional<std::string_view> CommandContext::Flag(
    std::string_view flag_name) {
  auto flag_value = flags_.Get(flag_name);
  if (flag_value.has_value()) return *flag_value;
  return std::nullopt;
}

std::ostream& CommandContext::Out() { return out_; }

std::ostream& CommandContext::Err() {
  return err_ << SetColor(Color::Red) << "ERROR:" << ResetColor << " ";
}

CommandContext StandardRootCommandContext(std::span<std::string_view> args) {
  return CommandContext({}, args, {}, std::cout, std::cerr);
}

namespace internal {

int InvokeCommand(const Command& command, CommandContext ctx) {
  std::vector<std::string_view> path = ctx.path_;
  path.push_back(command.name);

  auto first_non_flag_arg_it =
      std::find_if(ctx.args_.begin(), ctx.args_.end(),
                   [](auto arg) { return !arg.starts_with("--"); });

  HashMap<std::string_view, std::string_view> flags;
  for (auto it = ctx.args_.begin(); it != first_non_flag_arg_it; ++it) {
    std::string_view flag = it->substr(2);
    std::size_t value_offset = flag.find('=');
    std::string_view flag_name = flag.substr(0, value_offset);
    if (!AcceptsFlag(command, flag_name)) {
      ctx.Err() << "unknown flag '--" << flag_name << "'\n";
      return 1;
    }
    // A flag given without a value gets an empty one, so that a command can
    // tell `--flag` from `--flag=value` while still being able to test for
    // mere presence.
    flags.Set(flag_name, value_offset == std::string_view::npos
                             ? std::string_view()
                             : flag.substr(value_offset + 1));
  }

  return command.handler(
      CommandContext(std::move(path), {first_non_flag_arg_it, ctx.args_.end()},
                     flags, ctx.out_, ctx.err_));
}

}  // namespace internal

int RunCommand(std::initializer_list<Command> commands, CommandContext ctx) {
  std::optional<std::string_view> invoked_command = ctx.TakeArg();
  if (!invoked_command) {
    ctx.Out() << "Usage: " << ctx.CurrentCommand() << " <command> ...\n\n"
              << "Available commands:\n";

    int offset = 0;
    for (const auto& command : commands) {
      offset = std::max(offset, static_cast<int>(command.name.size()));
    }

    for (const auto& command : commands) {
      ctx.Out() << std::left << "  " << std::setw(offset) << command.name << " "
                << command.help << "\n";
    }
    return 0;
  }

  auto command_it = std::find_if(
      commands.begin(), commands.end(),
      [&](const auto& command) { return command.name == *invoked_command; });
  if (command_it == commands.end()) {
    ctx.Err() << "unknown command '" << *invoked_command << "'\n";
    return 1;
  }

  return internal::InvokeCommand(*command_it, std::move(ctx));
}

int RunProgram(const Command& command, CommandContext ctx) {
  // The first argument is the path the program was invoked by. It names no
  // command -- the program is the command -- so it is dropped rather than
  // matched against one.
  ctx.TakeArg();

  return internal::InvokeCommand(command, std::move(ctx));
}

}  // namespace lucid
