#include "lucid/core/cli/cli.h"

#include <algorithm>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/core/container/hash_map.h"

namespace lucid {

CommandContext StandardRootCommandContext(std::span<std::string_view> args) {
  return {
      .path = {},
      .args = args,
      .flags = {},
      .out = std::cout,
      .err = std::cerr,
  };
}

int RunCommand(std::initializer_list<Command> commands, CommandContext ctx) {
  if (ctx.args.empty()) {
    ctx.out << "Usage: ";
    for (std::string_view e : ctx.path) ctx.out << e << " ";
    ctx.out << "<command> ...\n\n"
            << "Available commands:\n";

    int offset = 0;
    for (const auto& command : commands) {
      offset = std::max(offset, static_cast<int>(command.name.size()));
    }

    for (const auto& command : commands) {
      ctx.out << std::left << "  " << std::setw(offset) << command.name << " "
              << command.help << "\n";
    }
    return 0;
  }

  auto command_it = std::find_if(
      commands.begin(), commands.end(),
      [&](const auto& command) { return command.name == ctx.args[0]; });
  if (command_it == commands.end()) {
    PrintError(ctx.err) << "unknown command '" << ctx.args[0] << "'\n";
    return 1;
  }

  std::vector<std::string_view> path = ctx.path;
  path.push_back(command_it->name);

  auto args = ctx.args.subspan(1);
  auto first_non_flag_arg_it =
      std::find_if(args.begin(), args.end(),
                   [](auto arg) { return !arg.starts_with("--"); });

  HashMap<std::string_view, std::string_view> flags;
  for (auto it = args.begin(); it != first_non_flag_arg_it; ++it) {
    auto flag = it->substr(2);
    auto count = flag.find("=");
    flags.Set(flag.substr(0, count), flag.substr(count + 1));
  }

  return command_it->handler({
      .path = std::move(path),
      .args = {first_non_flag_arg_it, args.end()},
      .flags = flags,
      .out = ctx.out,
      .err = ctx.err,
  });
}

std::ostream& PrintError(std::ostream& out) {
  return out << "\033[31m"
             << "ERROR:"
             << "\033[0m ";
}

}  // namespace lucid
