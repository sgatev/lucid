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

using namespace std::string_literals;

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
  std::string code_with_null(code);
  code_with_null.append("\0"s);

  std::vector<TestToken> tokens;
  Lexer lexer(code_with_null);
  while (true) {
    const Token token = lexer.next();
    if (token.kind == Kind::End) break;

    tokens.emplace_back(
        token.kind,
        code.substr(token.start_pos, token.end_pos - token.start_pos));
  }
  return tokens;
}

TEST(LexerTest, Empty) { EXPECT_THAT(ReadTokens(""), IsEmpty()); }

TEST(LexerTest, Ident) {
  EXPECT_THAT(ReadTokens("foo  "),
              ElementsAre(Tok(Kind::Ident, "foo"), Tok(Kind::Space, "  ")));
  EXPECT_THAT(ReadTokens("Foo21  "),
              ElementsAre(Tok(Kind::Ident, "Foo21"), Tok(Kind::Space, "  ")));
  EXPECT_THAT(
      ReadTokens("foo# bar\n"),
      ElementsAre(Tok(Kind::Ident, "foo"), Tok(Kind::Comment, "# bar\n")));
  EXPECT_THAT(ReadTokens("foo("),
              ElementsAre(Tok(Kind::Ident, "foo"), Tok(Kind::OpenParen, "(")));
}

TEST(LexerTest, String) {
  EXPECT_THAT(ReadTokens(R"("foo")"),
              ElementsAre(Tok(Kind::String, R"("foo")")));
}

TEST(LexerTest, Singleton) {
  EXPECT_THAT(ReadTokens("+"), ElementsAre(Tok(Kind::Plus, "+")));
  EXPECT_THAT(ReadTokens("+Foo"),
              ElementsAre(Tok(Kind::Plus, "+"), Tok(Kind::Ident, "Foo")));
  EXPECT_THAT(
      ReadTokens(R"(+"foo")"),
      ElementsAre(Tok(Kind::Plus, R"(+)"), Tok(Kind::String, R"("foo")")));
}

TEST(LexerTest, Number) {
  EXPECT_THAT(ReadTokens("21"), ElementsAre(Tok(Kind::Number, "21")));
  EXPECT_THAT(ReadTokens("21foo"),
              ElementsAre(Tok(Kind::Number, "21"), Tok(Kind::Ident, "foo")));
  EXPECT_THAT(ReadTokens("21   "),
              ElementsAre(Tok(Kind::Number, "21"), Tok(Kind::Space, "   ")));
}

TEST(LexerTest, Space) {
  EXPECT_THAT(ReadTokens("   21"),
              ElementsAre(Tok(Kind::Space, "   "), Tok(Kind::Number, "21")));
  EXPECT_THAT(ReadTokens("   foo"),
              ElementsAre(Tok(Kind::Space, "   "), Tok(Kind::Ident, "foo")));
  EXPECT_THAT(ReadTokens("   ["), ElementsAre(Tok(Kind::Space, "   "),
                                              Tok(Kind::OpenBracket, "[")));
}

TEST(LexerTest, Comment) {
  EXPECT_THAT(ReadTokens("# comment\n"),
              ElementsAre(Tok(Kind::Comment, "# comment\n")));
}

TEST(LexerTest, Error) {
  EXPECT_THAT(ReadTokens(R"("foo)"), ElementsAre(Tok(Kind::Error, R"("foo)")));
}

}  // namespace
}  // namespace lucid
