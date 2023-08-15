#include "lucid/cli.h"

#include <initializer_list>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace lucid {

std::optional<std::string> RunCommand(std::string_view root_name,
                                      std::initializer_list<Command> commands,
                                      std::span<std::string_view> args) {
  if (args.empty()) {
    std::cout << "Usage: lucid <command> ...\n\n";
    std::cout << "Available commands:\n";
    for (const auto& command : commands) {
      std::cout << "  " << command.name << " \t" << command.help << "\n";
    }
    return std::nullopt;
  }

  for (const auto& command : commands) {
    if (command.name == args[0]) return command.handler(args.subspan(1));
  }
  return "unknown command: " + std::string(args[0]);
}

}  // namespace lucid
