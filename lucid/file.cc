#include "lucid/file.h"

#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <variant>

namespace lucid {

std::variant<std::string, FileError> ReadFile(std::string_view path) {
  std::ifstream file(path);
  if (!file.good()) return FileError{};

  std::string content;
  file.seekg(0, std::ios::end);
  content.reserve(file.tellg());

  file.seekg(0, std::ios::beg);
  content.assign((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());

  return content;
}

}  // namespace lucid
