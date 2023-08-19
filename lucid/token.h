#pragma once

#include <cstddef>
#include <ostream>
#include <string_view>

namespace lucid {

// A token in the Lucid language.
struct Token {
  enum class Kind : std::size_t {
    End,
    String,
    OpenParen,
    CloseParen,
    OpenBrace,
    CloseBrace,
    Equal,
    Ident,
    Colon,
    Comma,
    Plus,
    Minus,
    Star,
    Slash,
    Greater,
    Less,
    Dot,
    Bar,
    Comment,
    Number,
  };

  constexpr Token(Kind kind, std::size_t start_pos, std::size_t end_pos)
      : kind(kind), start_pos(start_pos), end_pos(end_pos) {}

  constexpr bool operator==(const Token& other) const {
    return kind == other.kind && start_pos == other.start_pos &&
           end_pos == other.end_pos;
  }

  friend std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "Token{.kind=" << static_cast<int>(token.kind)
       << ", .start_pos=" << token.start_pos << ", .end_pos=" << token.end_pos
       << "}";
    return os;
  }

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
