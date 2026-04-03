#pragma once

#include <ostream>
#include <string_view>

namespace lucid {

// Interprets `s` as a Posix-encoded string and returns its length.
std::size_t EncodedStringLength(std::string_view s);

// Interprets `s` as a Posix-encoded string, writes it to `out`, and returns its
// length.
//
// Guarantees:
// - The string written to `out` is null-terminated.
std::size_t WriteEncodedString(std::string_view s, std::ostream& out);

}  // namespace lucid
