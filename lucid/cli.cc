#include "lucid/cli.h"

#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace lucid {

std::optional<std::string> RunCommand(
    std::initializer_list<std::pair<std::string_view, CommandHandler>> handlers,
    std::span<std::string_view> args) {
  if (args.empty()) return "missing arguments";
  for (const auto& [command, handler] : handlers) {
    if (command == args[0]) return handler(args.subspan(1));
  }
  return "unknown command: " + std::string(args[0]);
}

}  // namespace lucid
