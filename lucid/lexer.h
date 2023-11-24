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
  explicit Lexer(std::string_view buffer) : buffer_(buffer), pos_(0) {}

  // Returns the next token in the buffer.
  Token next() {
  start:
    if (buffer_.empty()) return Token(Token::Kind::End, pos_, pos_);

    const char c = buffer_.front();
    const std::size_t start_pos = pos_;
    if (c == ' ' || c == '\n' || c == '\t') {
      // Whitespace.
      advance(1);
      goto start;
    } else if (alphanumeric[c]) {
      // Identifier or number.
      do advance(1);
      // `buffer_` is not empty as it must end in `\n`.
      while (alphanumeric[buffer_.front()]);
    } else if (c == '"' || c == '#') {
      // String or comment.
      auto pos = std::find(buffer_.begin() + 1, buffer_.end(), finishers[c]);
      if (pos == buffer_.end()) {
        advance(buffer_.size());
        return Token(Token::Kind::IncompleteString, start_pos, pos_);
      }
      advance(pos - buffer_.begin() + 1);
    } else if (c == '=') {
      advance(1);
      // `buffer_` is not empty as it must end in `\n`.
      if (buffer_.front() == '=') {
        advance(1);
        return Token(Token::Kind::DoubleEqual, start_pos, pos_);
      }
    } else {
      // Singleton.
      advance(1);
    }
    return Token(kind[c], start_pos, pos_);
  }

 private:
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
    kind['#'] = Token::Kind::Comment;
    kind['"'] = Token::Kind::String;
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

  void advance(std::size_t pos) {
    buffer_.remove_prefix(pos);
    pos_ += pos;
  }

  std::string_view buffer_;
  std::size_t pos_;
};

}  // namespace lucid
