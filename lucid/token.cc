#include "lucid/token.h"

#include <algorithm>
#include <cstddef>
#include <string_view>

namespace lucid {

std::size_t FindLine(std::string_view buffer, const Token& token) {
  std::string_view prefix = buffer.substr(0, token.start_pos);
  return std::count(prefix.begin(), prefix.end(), '\n') + 1;
}

std::size_t FindColumn(std::string_view buffer, const Token& token) {
  std::string_view prefix = buffer.substr(0, token.start_pos);
  auto it = prefix.find_last_of('\n');
  prefix.remove_prefix(it == std::string_view::npos ? prefix.size() : it + 1);
  return prefix.size() + 1;
}

}  // namespace lucid
