#include "lucid/lexer.h"

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/token.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

using Kind = Token::Kind;

struct TestToken {
  TestToken(Kind kind, std::string_view text) : kind(kind), text(text) {}

  bool operator==(const TestToken& other) const {
    return kind == other.kind && text == other.text;
  }

  const Kind kind;
  const std::string text;
};

[[maybe_unused]] std::ostream& operator<<(std::ostream& stream,
                                          const TestToken& tok) {
  return stream << "TestToken{.kind=" << static_cast<int>(tok.kind)
                << ", .text=\"" << tok.text << "\"}";
}

TestToken Tok(Kind kind, std::string_view text) {
  return TestToken(kind, text);
}

std::vector<TestToken> ReadTokens(std::string_view code) {
  std::vector<TestToken> tokens;
  Lexer lexer(code);
  while (true) {
    Token token = lexer.Next();
    if (token.kind == Kind::End) break;

    tokens.push_back(
        {token.kind,
         code.substr(token.start_pos, token.end_pos - token.start_pos)});
  }
  return tokens;
}

TEST(LexerTest, Empty) { EXPECT_THAT(ReadTokens(""), IsEmpty()); }

TEST(LexerTest, Function) {
  EXPECT_THAT(
      ReadTokens(R"(
    let main = () -> Int {
      print("Hello, world!")
      return 0
    }
  )"),
      ElementsAre(Tok(Kind::Ident, "let"), Tok(Kind::Ident, "main"),
                  Tok(Kind::Equal, "="), Tok(Kind::OpenParen, "("),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Minus, "-"),
                  Tok(Kind::Greater, ">"), Tok(Kind::Ident, "Int"),
                  Tok(Kind::OpenBrace, "{"), Tok(Kind::Ident, "print"),
                  Tok(Kind::OpenParen, "("),
                  Tok(Kind::String, "\"Hello, world!\""),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Ident, "return"),
                  Tok(Kind::Number, "0"), Tok(Kind::CloseBrace, "}")));
}

TEST(LexerTest, Tuple) {
  EXPECT_THAT(ReadTokens(R"(
    let Point = (
      x: Int,
      y: Int,
    )
  )"),
              ElementsAre(Tok(Kind::Ident, "let"), Tok(Kind::Ident, "Point"),
                          Tok(Kind::Equal, "="), Tok(Kind::OpenParen, "("),
                          Tok(Kind::Ident, "x"), Tok(Kind::Colon, ":"),
                          Tok(Kind::Ident, "Int"), Tok(Kind::Comma, ","),
                          Tok(Kind::Ident, "y"), Tok(Kind::Colon, ":"),
                          Tok(Kind::Ident, "Int"), Tok(Kind::Comma, ","),
                          Tok(Kind::CloseParen, ")")));
}

TEST(LexerTest, Lambda) {
  EXPECT_THAT(
      ReadTokens(R"(
    let sortByLength = (mut names: List(String)) {
      sort(mut names, (a: String, b: String) -> Bool {
        return a.len < b.len
      })
    }
  )"),
      ElementsAre(Tok(Kind::Ident, "let"), Tok(Kind::Ident, "sortByLength"),
                  Tok(Kind::Equal, "="), Tok(Kind::OpenParen, "("),
                  Tok(Kind::Ident, "mut"), Tok(Kind::Ident, "names"),
                  Tok(Kind::Colon, ":"), Tok(Kind::Ident, "List"),
                  Tok(Kind::OpenParen, "("), Tok(Kind::Ident, "String"),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::CloseParen, ")"),
                  Tok(Kind::OpenBrace, "{"), Tok(Kind::Ident, "sort"),
                  Tok(Kind::OpenParen, "("), Tok(Kind::Ident, "mut"),
                  Tok(Kind::Ident, "names"), Tok(Kind::Comma, ","),
                  Tok(Kind::OpenParen, "("), Tok(Kind::Ident, "a"),
                  Tok(Kind::Colon, ":"), Tok(Kind::Ident, "String"),
                  Tok(Kind::Comma, ","), Tok(Kind::Ident, "b"),
                  Tok(Kind::Colon, ":"), Tok(Kind::Ident, "String"),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Minus, "-"),
                  Tok(Kind::Greater, ">"), Tok(Kind::Ident, "Bool"),
                  Tok(Kind::OpenBrace, "{"), Tok(Kind::Ident, "return"),
                  Tok(Kind::Ident, "a"), Tok(Kind::Dot, "."),
                  Tok(Kind::Ident, "len"), Tok(Kind::Less, "<"),
                  Tok(Kind::Ident, "b"), Tok(Kind::Dot, "."),
                  Tok(Kind::Ident, "len"), Tok(Kind::CloseBrace, "}"),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::CloseBrace, "}")));
}

TEST(LexerTest, Union) {
  EXPECT_THAT(
      ReadTokens(R"(
    let Nothing = ()

    let Optional = (T: Type) -> Type {
      return T | Nothing
    }
  )"),
      ElementsAre(Tok(Kind::Ident, "let"), Tok(Kind::Ident, "Nothing"),
                  Tok(Kind::Equal, "="), Tok(Kind::OpenParen, "("),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Ident, "let"),
                  Tok(Kind::Ident, "Optional"), Tok(Kind::Equal, "="),
                  Tok(Kind::OpenParen, "("), Tok(Kind::Ident, "T"),
                  Tok(Kind::Colon, ":"), Tok(Kind::Ident, "Type"),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Minus, "-"),
                  Tok(Kind::Greater, ">"), Tok(Kind::Ident, "Type"),
                  Tok(Kind::OpenBrace, "{"), Tok(Kind::Ident, "return"),
                  Tok(Kind::Ident, "T"), Tok(Kind::Bar, "|"),
                  Tok(Kind::Ident, "Nothing"), Tok(Kind::CloseBrace, "}")));
}

TEST(LexerTest, Comment) {
  EXPECT_THAT(
      ReadTokens(R"(
    # Returns the sum of two integers.
    let sum = (a: Int, b: Int) {
      return a + b # can overflow
    }
  )"),
      ElementsAre(
          Tok(Kind::Comment, "# Returns the sum of two integers.\n"),
          Tok(Kind::Ident, "let"), Tok(Kind::Ident, "sum"),
          Tok(Kind::Equal, "="), Tok(Kind::OpenParen, "("),
          Tok(Kind::Ident, "a"), Tok(Kind::Colon, ":"), Tok(Kind::Ident, "Int"),
          Tok(Kind::Comma, ","), Tok(Kind::Ident, "b"), Tok(Kind::Colon, ":"),
          Tok(Kind::Ident, "Int"), Tok(Kind::CloseParen, ")"),
          Tok(Kind::OpenBrace, "{"), Tok(Kind::Ident, "return"),
          Tok(Kind::Ident, "a"), Tok(Kind::Plus, "+"), Tok(Kind::Ident, "b"),
          Tok(Kind::Comment, "# can overflow\n"), Tok(Kind::CloseBrace, "}")));
}

TEST(LexerTest, Number) {
  EXPECT_THAT(ReadTokens(R"(
    let c = sum(21, 32)
  )"),
              ElementsAre(Tok(Kind::Ident, "let"), Tok(Kind::Ident, "c"),
                          Tok(Kind::Equal, "="), Tok(Kind::Ident, "sum"),
                          Tok(Kind::OpenParen, "("), Tok(Kind::Number, "21"),
                          Tok(Kind::Comma, ","), Tok(Kind::Number, "32"),
                          Tok(Kind::CloseParen, ")")));
}

}  // namespace
}  // namespace lucid
