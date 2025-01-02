#pragma once

#include <ostream>
#include <string_view>

namespace lucid {

// Interprets `s` as a Posix-encoded string and returns its length.
//
// Requires:
// - `s` must be of at least length 2, with '"' as its first and last character.
std::size_t EncodedStringLength(std::string_view s);

// Interprets `s` as a Posix-encoded string, writes it to `out`, and returns its
// length.
//
// Requires:
// - `s` must be of at least length 2, with '"' as its first and last character.
//
// Guarantees:
// - The string written to `out` is null-terminated.
std::size_t WriteEncodedString(std::string_view s, std::ostream& out);

}  // namespace lucid
