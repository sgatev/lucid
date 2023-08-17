#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace lucid {

// Returns the content of the file located at `path` or nullopt in case of
// error.
std::optional<std::string> ReadFile(std::string_view path);

}  // namespace lucid
