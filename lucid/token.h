#pragma once

#include <cctype>
#include <cstddef>
#include <ostream>
#include <string_view>

namespace lucid {

// A token in the Lucid language.
struct Token {
  enum class Kind : std::size_t {
    End = 0,
    String = 1,
    OpenParen = 2,
    CloseParen = 3,
    OpenBrace = 4,
    CloseBrace = 5,
    Equal = 6,
    Ident = 7,
    Colon = 8,
    Comma = 9,
    Plus = 10,
    Minus = 11,
    Greater = 12,
    Less = 13,
    Dot = 14,
    Bar = 15,
    Comment = 16,
    Number = 17,
  };

  constexpr Token(Kind kind, std::size_t start_pos,
                  std::size_t end_pos) noexcept
      : kind(kind), start_pos(start_pos), end_pos(end_pos) {}

  constexpr bool operator==(const Token& other) const noexcept {
    return kind == other.kind && start_pos == other.start_pos &&
           end_pos == other.end_pos;
  }

  friend std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "Token(" << static_cast<int>(token.kind) << ", " << token.start_pos
       << ", " << token.end_pos << ")";
    return os;
  }

  Kind kind;
  std::size_t start_pos;
  std::size_t end_pos;
};

// Returns the index of the line in `buffer` that contains `token`.
//
// The result is undefined if `buffer` does not contain `token`.
std::size_t FindLine(std::string_view buffer, const Token& token);

// Returns the index of the column in `buffer` that contains `token`.
//
// The result is undefined if `buffer` does not contain `token`.
std::size_t FindColumn(std::string_view buffer, const Token& token);

}  // namespace lucid
