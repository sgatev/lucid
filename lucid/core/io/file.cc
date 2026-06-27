#include "lucid/core/io/file.h"

#include <expected>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>

namespace lucid {

std::expected<std::string, ReadFileError> ReadFile(
    const std::filesystem::path& path, bool with_trailing_zero) {
  std::ifstream file(path);
  if (!file) return std::unexpected(ReadFileError(path));

  file.seekg(0, std::ios::end);
  std::streamsize size = file.tellg();
  if (with_trailing_zero) ++size;

  std::string buffer(size, '\0');
  file.seekg(0);
  file.read(&buffer[0], size);

  return buffer;
}

}  // namespace lucid
