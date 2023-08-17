#pragma once

#include <functional>
#include <initializer_list>
#include <ostream>
#include <span>
#include <string_view>

namespace lucid {

// The execution context of a command.
struct CommandContext {
  // Arguments passed to the command.
  std::span<std::string_view> args;

  // Standard output stream of the command.
  std::ostream& out;

  // Standard error output stream of the command.
  std::ostream& err;
};

// A handler that runs a command within a given context.
using CommandHandler = std::function<int(CommandContext)>;

// A command.
struct Command {
  // Name of the command.
  std::string_view name;

  // Help string for the command.
  std::string_view help;

  // Command handler.
  CommandHandler handler;
};

// Calls the handler for the given command.
//
// The command is identified by the first element in `ctx.args`. The remaining
// elements of `ctx.args` are passed in the handler call.
//
// Returns an error if `ctx.args` is empty. Returns an error if `commands` does
// not include the given command.
//
// Requirements:
//
//   * `commands` must not contain more than one command with a given name.
int RunCommand(std::string_view root_name,
               std::initializer_list<Command> commands, CommandContext ctx);

}  // namespace lucid
