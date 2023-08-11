#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <string_view>

#include "lucid/token.h"

namespace lucid {

// Converts a string buffer into a stream of tokens.
class Lexer {
 public:
  // `buffer_` must end in `\n`.
  explicit Lexer(std::string_view buffer) : buffer_(buffer), pos_(0) {}

  // Returns the next token in the buffer.
  Token Next() {
    if (auto pos = buffer_.find_first_not_of(" \n\t");
        pos == std::string_view::npos) {
      return Token(Token::Kind::End, pos_ + buffer_.size(),
                   pos_ + buffer_.size());
    } else {
      Advance(pos);
    }

    const char c = buffer_.front();
    const size_t start_pos = pos_;
    if (alphanumeric[c]) {
      // Identifier or number.
      do Advance(1);
      while (!buffer_.empty() && alphanumeric[buffer_.front()]);
    } else if (c == '"' || c == '#') {
      // String or comment.
      auto pos = std::find(buffer_.begin() + 1, buffer_.end(), finishers[c]);
      size_t diff = pos - buffer_.begin() + 1;
      Advance(diff);
    } else {
      // Singleton.
      Advance(1);
    }
    return Token(singletons[c], start_pos, pos_);
  }

 private:
  static constexpr std::array<bool, 256> alphanumeric = []() {
    std::array<bool, 256> alphanumeric = {false};
    for (char c = 'a'; c <= 'z'; ++c) alphanumeric[c] = true;
    for (char c = 'A'; c <= 'Z'; ++c) alphanumeric[c] = true;
    for (char c = '0'; c <= '9'; ++c) alphanumeric[c] = true;
    return alphanumeric;
  }();

  static constexpr std::array<Token::Kind, 256> singletons = []() {
    std::array<Token::Kind, 256> singletons = {Token::Kind::End};
    singletons['='] = Token::Kind::Equal;
    singletons['('] = Token::Kind::OpenParen;
    singletons[')'] = Token::Kind::CloseParen;
    singletons['{'] = Token::Kind::OpenBrace;
    singletons['}'] = Token::Kind::CloseBrace;
    singletons[':'] = Token::Kind::Colon;
    singletons[','] = Token::Kind::Comma;
    singletons['+'] = Token::Kind::Plus;
    singletons['-'] = Token::Kind::Minus;
    singletons['>'] = Token::Kind::Greater;
    singletons['<'] = Token::Kind::Less;
    singletons['.'] = Token::Kind::Dot;
    singletons['|'] = Token::Kind::Bar;
    singletons['#'] = Token::Kind::Comment;
    singletons['"'] = Token::Kind::String;
    for (char c = 'a'; c <= 'z'; ++c) singletons[c] = Token::Kind::Ident;
    for (char c = 'A'; c <= 'Z'; ++c) singletons[c] = Token::Kind::Ident;
    for (char c = '0'; c <= '9'; ++c) singletons[c] = Token::Kind::Number;
    return singletons;
  }();

  static constexpr std::array<char, 256> finishers = []() {
    std::array<char, 256> finishers = {' '};
    finishers['"'] = '"';
    finishers['#'] = '\n';
    return finishers;
  }();

  inline void Advance(size_t pos) {
    buffer_.remove_prefix(pos);
    pos_ += pos;
  }

  std::string_view buffer_;
  size_t pos_;
};

}  // namespace lucid
