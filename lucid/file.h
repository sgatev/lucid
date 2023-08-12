#pragma once

#include <string>
#include <string_view>
#include <variant>

namespace lucid {

// An error that occurred while working with a file.
struct FileError {
  bool operator==(const FileError&) const { return true; }
};

// Returns the content of the file located at `path`.
std::variant<std::string, FileError> ReadFile(std::string_view path);

}  // namespace lucid
