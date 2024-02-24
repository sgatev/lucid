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
    Token token = lexer.next();
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
    let main = () -> Int32 {
      print("Hello, world!")
      return 0
    }
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "main"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenParen, "("),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Minus, "-"), Tok(Kind::Greater, ">"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Int32"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenBrace, "{"),
                  Tok(Kind::Whitespace, "\n      "), Tok(Kind::Ident, "print"),
                  Tok(Kind::OpenParen, "("),
                  Tok(Kind::String, "\"Hello, world!\""),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Whitespace, "\n      "),
                  Tok(Kind::Ident, "return"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Number, "0"), Tok(Kind::Whitespace, "\n    "),
                  Tok(Kind::CloseBrace, "}"), Tok(Kind::Whitespace, "\n  ")));
}

TEST(LexerTest, Tuple) {
  EXPECT_THAT(
      ReadTokens(R"(
    let Point = (
      x: Int32,
      y: Int32,
    )
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Point"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenParen, "("),
                  Tok(Kind::Whitespace, "\n      "), Tok(Kind::Ident, "x"),
                  Tok(Kind::Colon, ":"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "Int32"), Tok(Kind::Comma, ","),
                  Tok(Kind::Whitespace, "\n      "), Tok(Kind::Ident, "y"),
                  Tok(Kind::Colon, ":"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "Int32"), Tok(Kind::Comma, ","),
                  Tok(Kind::Whitespace, "\n    "), Tok(Kind::CloseParen, ")"),
                  Tok(Kind::Whitespace, "\n  ")));
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
      ElementsAre(
          Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
          Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "sortByLength"),
          Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
          Tok(Kind::Whitespace, " "), Tok(Kind::OpenParen, "("),
          Tok(Kind::Ident, "mut"), Tok(Kind::Whitespace, " "),
          Tok(Kind::Ident, "names"), Tok(Kind::Colon, ":"),
          Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "List"),
          Tok(Kind::OpenParen, "("), Tok(Kind::Ident, "String"),
          Tok(Kind::CloseParen, ")"), Tok(Kind::CloseParen, ")"),
          Tok(Kind::Whitespace, " "), Tok(Kind::OpenBrace, "{"),
          Tok(Kind::Whitespace, "\n      "), Tok(Kind::Ident, "sort"),
          Tok(Kind::OpenParen, "("), Tok(Kind::Ident, "mut"),
          Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "names"),
          Tok(Kind::Comma, ","), Tok(Kind::Whitespace, " "),
          Tok(Kind::OpenParen, "("), Tok(Kind::Ident, "a"),
          Tok(Kind::Colon, ":"), Tok(Kind::Whitespace, " "),
          Tok(Kind::Ident, "String"), Tok(Kind::Comma, ","),
          Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "b"),
          Tok(Kind::Colon, ":"), Tok(Kind::Whitespace, " "),
          Tok(Kind::Ident, "String"), Tok(Kind::CloseParen, ")"),
          Tok(Kind::Whitespace, " "), Tok(Kind::Minus, "-"),
          Tok(Kind::Greater, ">"), Tok(Kind::Whitespace, " "),
          Tok(Kind::Ident, "Bool"), Tok(Kind::Whitespace, " "),
          Tok(Kind::OpenBrace, "{"), Tok(Kind::Whitespace, "\n        "),
          Tok(Kind::Ident, "return"), Tok(Kind::Whitespace, " "),
          Tok(Kind::Ident, "a"), Tok(Kind::Dot, "."), Tok(Kind::Ident, "len"),
          Tok(Kind::Whitespace, " "), Tok(Kind::Less, "<"),
          Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "b"),
          Tok(Kind::Dot, "."), Tok(Kind::Ident, "len"),
          Tok(Kind::Whitespace, "\n      "), Tok(Kind::CloseBrace, "}"),
          Tok(Kind::CloseParen, ")"), Tok(Kind::Whitespace, "\n    "),
          Tok(Kind::CloseBrace, "}"), Tok(Kind::Whitespace, "\n  ")));
}

TEST(LexerTest, Union) {
  EXPECT_THAT(
      ReadTokens(R"(
    let Nothing = ()

    let Optional = (T: Type) -> Type {
      return T | Nothing
    }
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Nothing"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenParen, "("),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Whitespace, "\n\n    "),
                  Tok(Kind::Ident, "let"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "Optional"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Equal, "="), Tok(Kind::Whitespace, " "),
                  Tok(Kind::OpenParen, "("), Tok(Kind::Ident, "T"),
                  Tok(Kind::Colon, ":"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "Type"), Tok(Kind::CloseParen, ")"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Minus, "-"),
                  Tok(Kind::Greater, ">"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "Type"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::OpenBrace, "{"), Tok(Kind::Whitespace, "\n      "),
                  Tok(Kind::Ident, "return"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "T"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Bar, "|"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "Nothing"), Tok(Kind::Whitespace, "\n    "),
                  Tok(Kind::CloseBrace, "}"), Tok(Kind::Whitespace, "\n  ")));
}

TEST(LexerTest, Comment) {
  EXPECT_THAT(
      ReadTokens(R"(
    # Returns the sum of two integers.
    let sum = (a: Int32, b: Int32) {
      return a + b # can overflow
    }
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "),
                  Tok(Kind::Comment, "# Returns the sum of two integers.\n"),
                  Tok(Kind::Whitespace, "    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "sum"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenParen, "("),
                  Tok(Kind::Ident, "a"), Tok(Kind::Colon, ":"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Int32"),
                  Tok(Kind::Comma, ","), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "b"), Tok(Kind::Colon, ":"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Int32"),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::OpenBrace, "{"), Tok(Kind::Whitespace, "\n      "),
                  Tok(Kind::Ident, "return"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "a"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Plus, "+"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "b"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Comment, "# can overflow\n"),
                  Tok(Kind::Whitespace, "    "), Tok(Kind::CloseBrace, "}"),
                  Tok(Kind::Whitespace, "\n  ")));
}

TEST(LexerTest, Number) {
  EXPECT_THAT(
      ReadTokens(R"(
    let c = sum(21, 32)
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "c"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "sum"),
                  Tok(Kind::OpenParen, "("), Tok(Kind::Number, "21"),
                  Tok(Kind::Comma, ","), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Number, "32"), Tok(Kind::CloseParen, ")"),
                  Tok(Kind::Whitespace, "\n  ")));
}

TEST(LexerTest, If) {
  EXPECT_THAT(
      ReadTokens(R"(
    let fact = (n: Int32): Int32 {
      if n == 1 {
        return 1
      }
      return n * fib(n-1)
    }
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "fact"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenParen, "("),
                  Tok(Kind::Ident, "n"), Tok(Kind::Colon, ":"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Int32"),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Colon, ":"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Int32"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenBrace, "{"),
                  Tok(Kind::Whitespace, "\n      "), Tok(Kind::Ident, "if"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "n"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::DoubleEqual, "=="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Number, "1"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenBrace, "{"),
                  Tok(Kind::Whitespace, "\n        "),
                  Tok(Kind::Ident, "return"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Number, "1"), Tok(Kind::Whitespace, "\n      "),
                  Tok(Kind::CloseBrace, "}"), Tok(Kind::Whitespace, "\n      "),
                  Tok(Kind::Ident, "return"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "n"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Star, "*"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "fib"), Tok(Kind::OpenParen, "("),
                  Tok(Kind::Ident, "n"), Tok(Kind::Minus, "-"),
                  Tok(Kind::Number, "1"), Tok(Kind::CloseParen, ")"),
                  Tok(Kind::Whitespace, "\n    "), Tok(Kind::CloseBrace, "}"),
                  Tok(Kind::Whitespace, "\n  ")));
}

TEST(LexerTest, IfNot) {
  EXPECT_THAT(
      ReadTokens(R"(
    let fact = (n: Int32): Int32 {
      if n != 1 {
        return 1
      }
      return n * fib(n-1)
    }
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "fact"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenParen, "("),
                  Tok(Kind::Ident, "n"), Tok(Kind::Colon, ":"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Int32"),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Colon, ":"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Int32"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenBrace, "{"),
                  Tok(Kind::Whitespace, "\n      "), Tok(Kind::Ident, "if"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "n"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::NotEqual, "!="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Number, "1"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenBrace, "{"),
                  Tok(Kind::Whitespace, "\n        "),
                  Tok(Kind::Ident, "return"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Number, "1"), Tok(Kind::Whitespace, "\n      "),
                  Tok(Kind::CloseBrace, "}"), Tok(Kind::Whitespace, "\n      "),
                  Tok(Kind::Ident, "return"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "n"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Star, "*"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "fib"), Tok(Kind::OpenParen, "("),
                  Tok(Kind::Ident, "n"), Tok(Kind::Minus, "-"),
                  Tok(Kind::Number, "1"), Tok(Kind::CloseParen, ")"),
                  Tok(Kind::Whitespace, "\n    "), Tok(Kind::CloseBrace, "}"),
                  Tok(Kind::Whitespace, "\n  ")));
}

TEST(LexerTest, IncompleteString) {
  EXPECT_THAT(
      ReadTokens(R"(
    let main = () -> Int32 {
      print("Hello, world!)
      return 0
    }
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "main"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Equal, "="),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenParen, "("),
                  Tok(Kind::CloseParen, ")"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Minus, "-"), Tok(Kind::Greater, ">"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "Int32"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::OpenBrace, "{"),
                  Tok(Kind::Whitespace, "\n      "), Tok(Kind::Ident, "print"),
                  Tok(Kind::OpenParen, "("),
                  Tok(Kind::IncompleteString, R"("Hello, world!)
      return 0
    }
  )")));
}

TEST(LexerTest, ArrayType) {
  EXPECT_THAT(
      ReadTokens(R"(
    let a: Int32[10]
  )"),
      ElementsAre(Tok(Kind::Whitespace, "\n    "), Tok(Kind::Ident, "let"),
                  Tok(Kind::Whitespace, " "), Tok(Kind::Ident, "a"),
                  Tok(Kind::Colon, ":"), Tok(Kind::Whitespace, " "),
                  Tok(Kind::Ident, "Int32"), Tok(Kind::OpenBracket, "["),
                  Tok(Kind::Number, "10"), Tok(Kind::CloseBracket, "]"),
                  Tok(Kind::Whitespace, "\n  ")));
}

}  // namespace
}  // namespace lucid
