#pragma once

#include <functional>
#include <initializer_list>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/core/container/hash_map.h"

namespace lucid {

struct Command;

// The execution context of a command.
struct CommandContext {
 public:
  CommandContext(std::vector<std::string_view> path,
                 std::span<std::string_view> args,
                 HashMap<std::string_view, std::string_view> flags,
                 std::ostream& out, std::ostream& err);

  // Returns the full name of the command.
  std::string CurrentCommand() const;

  // Returns the first positional argument to the command, if any.
  std::optional<std::string_view> TakeArg();

  // Returns the value of the flag with the given name, if any.
  std::optional<std::string_view> Flag(std::string_view flag_name);

  // Standard output stream of the command.
  std::ostream& Out();

  // Standard error output stream of the command.
  std::ostream& Err();

 private:
  friend int RunCommand(std::initializer_list<Command> commands,
                        CommandContext ctx);

  std::vector<std::string_view> path_;
  std::span<std::string_view> args_;
  HashMap<std::string_view, std::string_view> flags_;
  std::ostream& out_;
  std::ostream& err_;
};

// Returns an execution context for a root command.
CommandContext StandardRootCommandContext(std::span<std::string_view> args);

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
// Requires:
// - `commands` must not contain more than one command with a given name.
int RunCommand(std::initializer_list<Command> commands, CommandContext ctx);

}  // namespace lucid
