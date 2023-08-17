#include "lucid/file.h"

#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>

namespace lucid {

std::optional<std::string> ReadFile(std::string_view path) {
  std::ifstream file(path);
  if (!file) return std::nullopt;
  return std::string(std::istreambuf_iterator<char>(file),
                     std::istreambuf_iterator<char>());
}

}  // namespace lucid
