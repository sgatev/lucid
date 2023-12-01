#pragma once

#include <cstddef>
#include <ostream>
#include <string_view>

namespace lucid {

// A token in the Lucid language.
struct Token {
  enum class Kind : std::size_t {
    Bar,
    CloseBrace,
    CloseParen,
    Colon,
    Comma,
    Comment,
    Dot,
    DoubleEqual,
    End,
    Equal,
    Greater,
    Ident,
    IncompleteString,
    Less,
    Minus,
    NotEqual,
    Number,
    OpenBrace,
    OpenParen,
    Plus,
    Slash,
    Star,
    String,
  };

  constexpr Token(Kind kind, std::size_t start_pos, std::size_t end_pos)
      : kind(kind), start_pos(start_pos), end_pos(end_pos) {}

  constexpr bool operator==(const Token& other) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Token& token);

  Kind kind;
  std::size_t start_pos;
  std::size_t end_pos;
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
