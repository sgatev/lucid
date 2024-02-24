#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

#include "lucid/token.h"

namespace lucid {

// Converts a string of Lucid code into a stream of tokens.
class Lexer {
 public:
  // `buffer_` must end in `\n`.
  explicit Lexer(std::string_view buffer)
      : buffer_(buffer.data()), size_(buffer.size()), pos_(0) {}

  // Returns the next token in the buffer.
  inline Token next() {
    if (pos_ == size_) return Token(Token::Kind::End, pos_, pos_);

    const char c = buffer_[pos_];
    const std::size_t start_pos = pos_;
    if (whitespace[c]) {
      // Whitespace.
      do ++pos_;
      // `buffer_` is not empty as it must end in `\n`.
      while (whitespace[buffer_[pos_]]);
    } else if (alphanumeric[c]) {
      // Identifier or number.
      do ++pos_;
      // `buffer_` is not empty as it must end in `\n`.
      while (alphanumeric[buffer_[pos_]]);
    } else if (c == '"' || c == '#') {
      // String or comment.
      const char* pos = std::char_traits<char>::find(
          buffer_ + pos_ + 1, size_ - pos_ - 1, finishers[c]);
      if (pos == nullptr) {
        pos_ = size_;
        return Token(Token::Kind::IncompleteString, start_pos, pos_);
      }
      pos_ = pos - buffer_ + 1;
    } else {
      // Singleton.
      ++pos_;
    }
    return Token(kind[c], start_pos, pos_);
  }

 private:
  static constexpr std::array<bool, 256> whitespace = []() consteval {
    std::array<bool, 256> whitespace = {false};
    whitespace[' '] = true;
    whitespace['\n'] = true;
    whitespace['\t'] = true;
    return whitespace;
  }();

  static constexpr std::array<bool, 256> alphanumeric = []() consteval {
    std::array<bool, 256> alphanumeric = {false};
    for (char c = 'a'; c <= 'z'; ++c) alphanumeric[c] = true;
    for (char c = 'A'; c <= 'Z'; ++c) alphanumeric[c] = true;
    for (char c = '0'; c <= '9'; ++c) alphanumeric[c] = true;
    return alphanumeric;
  }();

  static constexpr std::array<Token::Kind, 256> kind = []() consteval {
    std::array<Token::Kind, 256> kind = {Token::Kind::End};
    kind['='] = Token::Kind::Equal;
    kind['('] = Token::Kind::OpenParen;
    kind[')'] = Token::Kind::CloseParen;
    kind['{'] = Token::Kind::OpenBrace;
    kind['}'] = Token::Kind::CloseBrace;
    kind['['] = Token::Kind::OpenBracket;
    kind[']'] = Token::Kind::CloseBracket;
    kind[':'] = Token::Kind::Colon;
    kind[','] = Token::Kind::Comma;
    kind['+'] = Token::Kind::Plus;
    kind['-'] = Token::Kind::Minus;
    kind['*'] = Token::Kind::Star;
    kind['/'] = Token::Kind::Slash;
    kind['>'] = Token::Kind::Greater;
    kind['<'] = Token::Kind::Less;
    kind['.'] = Token::Kind::Dot;
    kind['|'] = Token::Kind::Bar;
    kind['!'] = Token::Kind::Exclamation;
    kind['#'] = Token::Kind::Comment;
    kind['"'] = Token::Kind::String;
    kind['%'] = Token::Kind::Percent;
    kind[' '] = Token::Kind::Whitespace;
    kind['\t'] = Token::Kind::Whitespace;
    kind['\n'] = Token::Kind::Whitespace;
    for (char c = 'a'; c <= 'z'; ++c) kind[c] = Token::Kind::Ident;
    for (char c = 'A'; c <= 'Z'; ++c) kind[c] = Token::Kind::Ident;
    for (char c = '0'; c <= '9'; ++c) kind[c] = Token::Kind::Number;
    return kind;
  }();

  static constexpr std::array<char, 256> finishers = []() consteval {
    std::array<char, 256> finishers = {' '};
    finishers['"'] = '"';
    finishers['#'] = '\n';
    return finishers;
  }();

  const char* buffer_;
  const std::size_t size_;
  std::size_t pos_;
};

}  // namespace lucid
