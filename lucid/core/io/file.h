#pragma once

#include <filesystem>
#include <ostream>
#include <string>

#include "lucid/core/functional/result.h"

namespace lucid {

class ReadFileError {
 public:
  explicit ReadFileError(std::filesystem::path path) : path_(path) {}

  friend std::ostream& operator<<(std::ostream& out,
                                  const ReadFileError error) {
    return out << "could not read file " << error.path_;
  }

 private:
  std::filesystem::path path_;
};

// Returns the content of the file located at `path`.
//
// If `with_trailing_zero` is set to `true`, appends a null terminating
// character at the end of the returned string.
Result<std::string, ReadFileError> ReadFile(std::filesystem::path path,
                                            bool with_trailing_zero = false);

}  // namespace lucid
