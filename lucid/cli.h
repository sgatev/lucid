#pragma once

#include <functional>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>

namespace lucid {

// Represents the result of a command.
struct CommandResult {
  // Return code of the command.
  int return_code;

  // String printed on stdout by the command.
  std::string out;

  // String printed on stderr by the command.
  std::string err;
};

// A command handler that takes a list of arguments and runs a command.
using CommandHandler =
    std::function<CommandResult(std::span<std::string_view>)>;

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
// The command is identified by the first element in `args`. The remaining
// elements of `args` are passed in the handler call.
//
// Returns an error if `args` is empty. Returns an error if `commands` does not
// include the given command.
//
// Requirements:
//
//   * `commands` must not contain more than one command with a given name.
CommandResult RunCommand(std::string_view root_name,
                         std::initializer_list<Command> commands,
                         std::span<std::string_view> args);

}  // namespace lucid
