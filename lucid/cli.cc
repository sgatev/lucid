#include "lucid/cli.h"

#include <algorithm>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>

namespace lucid {

CommandResult RunCommand(std::string_view root_name,
                         std::initializer_list<Command> commands,
                         std::span<std::string_view> args) {
  if (args.empty()) {
    CommandResult result;
    result.return_code = 0;
    result.out.append("Usage: lucid <command> ...\n\n");
    result.out.append("Available commands:\n");
    for (const auto& command : commands) {
      result.out.append("  ");
      result.out.append(command.name);
      result.out.append(" \t");
      result.out.append(command.help);
      result.out.append("\n");
    }
    return result;
  }

  auto it = std::find_if(
      commands.begin(), commands.end(),
      [&](const auto& command) { return command.name == args[0]; });
  if (it != commands.end()) return it->handler(args.subspan(1));

  return {
      .return_code = 1,
      .err = "unknown command: " + std::string(args[0]),
  };
}

}  // namespace lucid
