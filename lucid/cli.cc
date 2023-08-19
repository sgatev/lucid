#include "lucid/cli.h"

#include <algorithm>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>

namespace lucid {

int RunCommand(std::string_view root_name,
               std::initializer_list<Command> commands, CommandContext ctx) {
  if (ctx.args.empty()) {
    ctx.out << "Usage: " << root_name << " <command> ...\n\n"
            << "Available commands:\n";
    for (const auto& command : commands) {
      ctx.out << "  " << command.name << " \t" << command.help << "\n";
    }
    return 0;
  }

  auto it = std::find_if(
      commands.begin(), commands.end(),
      [&](const auto& command) { return command.name == ctx.args[0]; });
  if (it != commands.end()) {
    return it->handler({ctx.args.subspan(1), ctx.out, ctx.err});
  }

  PrintError(ctx.err) << "unknown command '" << ctx.args[0] << "'\n";
  return 1;
}

std::ostream& PrintError(std::ostream& out) {
  return out << "\033[31m"
             << "ERROR:"
             << "\033[0m ";
}

}  // namespace lucid
