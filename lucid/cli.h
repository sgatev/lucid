#pragma once

#include <functional>
#include <initializer_list>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace lucid {

// A command handler that takes a list of arguments and runs a command.
//
// Returns nullopt if the command succeeds. Returns an error string otherwise.
using CommandHandler =
    std::function<std::optional<std::string>(std::span<std::string_view>)>;

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
// Returns nullopt if the command succeeds. Returns an error string otherwise.
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
std::optional<std::string> RunCommand(std::string_view root_name,
                                      std::initializer_list<Command> commands,
                                      std::span<std::string_view> args);

}  // namespace lucid
