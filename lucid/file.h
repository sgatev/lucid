#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace lucid {

// Returns the content of the file located at `path` or nullopt in case of
// an error.
//
// If `with_trailing_zero` is set to `true`, appends a null terminating
// character at the end of the returned string.
std::optional<std::string> ReadFile(std::string_view path,
                                    bool with_trailing_zero = false);

}  // namespace lucid
