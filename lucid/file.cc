#include "lucid/file.h"

#include <cstddef>
#include <fstream>
#include <ios>
#include <string>
#include <string_view>

#include "lucid/result.h"

namespace lucid {

Result<std::string, ReadFileError> ReadFile(std::string_view path,
                                            bool with_trailing_zero) {
  std::ifstream file(path);
  if (!file) return ReadFileError(path);

  file.seekg(0, std::ios::end);
  std::streamsize size = file.tellg();
  if (with_trailing_zero) ++size;

  std::string buffer(size, '\0');
  file.seekg(0);
  file.read(&buffer[0], size);

  return buffer;
}

}  // namespace lucid
