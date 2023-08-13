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

// Calls the first command handler that matches the given command.
//
// Returns nullopt if the command succeeds. Returns an error string otherwise.
//
// The command is identified by the first element in `args`. The remaining
// elements of `args` are passed in the handler call.
//
// Returns an error if `args` is empty. Returns an error if `handlers` does not
// include a handler for the given command.
std::optional<std::string> RunCommand(
    std::initializer_list<std::pair<std::string_view, CommandHandler>> handlers,
    std::span<std::string_view> args);

}  // namespace lucid
