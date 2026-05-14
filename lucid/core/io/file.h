#pragma once

#include <expected>
#include <filesystem>
#include <ostream>
#include <string>

namespace lucid {

class ReadFileError {
 public:
  explicit ReadFileError(std::filesystem::path path) : path_(path) {}

  friend std::ostream& operator<<(std::ostream& out, const ReadFileError& err) {
    return out << "could not read file " << err.path_;
  }

 private:
  std::filesystem::path path_;
};

// Returns the content of the file located at `path`.
//
// If `with_trailing_zero` is set to `true`, appends a null terminating
// character at the end of the returned string.
std::expected<std::string, ReadFileError> ReadFile(
    std::filesystem::path path, bool with_trailing_zero = false);

}  // namespace lucid
