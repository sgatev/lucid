#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string_view>

namespace lucid {

// A token in the Lucid language.
struct Token {
  enum class Kind : std::uint8_t {
    Comment,
    Space,
    Bar,
    CloseBrace,
    CloseParen,
    Colon,
    Comma,
    Dot,
    End,
    Equal,
    Greater,
    Ident,
    Error,
    Less,
    Minus,
    Exclamation,
    Number,
    OpenBrace,
    OpenParen,
    Plus,
    Slash,
    Star,
    String,
    Percent,
    Ampersand,
    OpenBracket,
    CloseBracket,
  };

  static constexpr Kind kFirstSemanticKind = Kind::Bar;

  constexpr bool operator==(const Token& other) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Token& token);

  Kind kind;
  std::uint32_t start_pos;
  std::uint32_t end_pos;
};

// Returns the index of the line in `buffer` that contains `token`.
//
// The behavior is undefined if `buffer` does not contain `token`.
std::size_t FindLine(std::string_view buffer, const Token& token);

// Returns the index of the column in `buffer` that contains `token`.
//
// The behavior is undefined if `buffer` does not contain `token`.
std::size_t FindColumn(std::string_view buffer, const Token& token);

}  // namespace lucid
