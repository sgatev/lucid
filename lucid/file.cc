#include "lucid/file.h"

#include <cstddef>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

namespace lucid {

std::optional<std::string> ReadFile(std::string_view path,
                                    bool with_trailing_zero) {
  std::ifstream file(path);
  if (!file) return std::nullopt;

  file.seekg(0, std::ios::end);
  std::size_t size = file.tellg();
  if (with_trailing_zero) ++size;

  std::string buffer(size, '\0');
  file.seekg(0);
  file.read(&buffer[0], size);

  return buffer;
}

}  // namespace lucid
