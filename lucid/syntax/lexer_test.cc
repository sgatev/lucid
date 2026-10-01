#include "lucid/syntax/lexer.h"

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/core/testing/testing.h"
#include "lucid/syntax/token.h"

namespace lucid {
namespace {

using namespace std::string_literals;

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

    // Nothing is to be read after an error.
    if (token.kind == Kind::Error) break;
  }
  return tokens;
}

TEST(Test, LexerEmpty) { EXPECT_THAT(ReadTokens(""), IsEmpty()); }

TEST(Test, LexerIdent) {
  EXPECT_THAT(ReadTokens("foo  "),
              ElementsEqual(Tok(Kind::Ident, "foo"), Tok(Kind::Space, "  ")));
  EXPECT_THAT(
      ReadTokens("_foo_bar  "),
      ElementsEqual(Tok(Kind::Ident, "_foo_bar"), Tok(Kind::Space, "  ")));
  EXPECT_THAT(ReadTokens("Foo21  "),
              ElementsEqual(Tok(Kind::Ident, "Foo21"), Tok(Kind::Space, "  ")));
  EXPECT_THAT(
      ReadTokens("foo# bar\n"),
      ElementsEqual(Tok(Kind::Ident, "foo"), Tok(Kind::Comment, "# bar"),
                    Tok(Kind::Space, "\n")));
  EXPECT_THAT(ReadTokens("foo("), ElementsEqual(Tok(Kind::Ident, "foo"),
                                                Tok(Kind::OpenParen, "(")));
}

TEST(Test, LexerString) {
  EXPECT_THAT(ReadTokens(R"("foo")"),
              ElementsEqual(Tok(Kind::String, R"("foo")")));
}

TEST(Test, LexerSingleton) {
  EXPECT_THAT(ReadTokens("+"), ElementsEqual(Tok(Kind::Plus, "+")));
  EXPECT_THAT(ReadTokens("+Foo"),
              ElementsEqual(Tok(Kind::Plus, "+"), Tok(Kind::Ident, "Foo")));
  EXPECT_THAT(
      ReadTokens(R"(+"foo")"),
      ElementsEqual(Tok(Kind::Plus, R"(+)"), Tok(Kind::String, R"("foo")")));
}

TEST(Test, LexerNumber) {
  EXPECT_THAT(ReadTokens("21"), ElementsEqual(Tok(Kind::Number, "21")));
  EXPECT_THAT(ReadTokens("21foo"),
              ElementsEqual(Tok(Kind::Number, "21"), Tok(Kind::Ident, "foo")));
  EXPECT_THAT(ReadTokens("21   "),
              ElementsEqual(Tok(Kind::Number, "21"), Tok(Kind::Space, "   ")));
}

TEST(Test, LexerSpace) {
  EXPECT_THAT(ReadTokens("   21"),
              ElementsEqual(Tok(Kind::Space, "   "), Tok(Kind::Number, "21")));
  EXPECT_THAT(ReadTokens("   foo"),
              ElementsEqual(Tok(Kind::Space, "   "), Tok(Kind::Ident, "foo")));
  EXPECT_THAT(ReadTokens("   ["), ElementsEqual(Tok(Kind::Space, "   "),
                                                Tok(Kind::OpenBracket, "[")));
}

TEST(Test, LexerComment) {
  EXPECT_THAT(
      ReadTokens("# comment\n"),
      ElementsEqual(Tok(Kind::Comment, "# comment"), Tok(Kind::Space, "\n")));
}

TEST(Test, LexerError) {
  EXPECT_THAT(ReadTokens(R"("foo)"),
              ElementsEqual(Tok(Kind::Error, R"("foo)")));
}

// A carriage return is whitespace, so a line ending in one before its newline
// reads the same as one that does not.
TEST(Test, LexerCarriageReturn) {
  EXPECT_THAT(ReadTokens("foo\r\nbar"),
              ElementsEqual(Tok(Kind::Ident, "foo"), Tok(Kind::Space, "\r\n"),
                            Tok(Kind::Ident, "bar")));
}

// A byte from 0x80 up is none of the language's, wherever it stands, and
// ends whatever was being read before it.
TEST(Test, LexerBytePastAscii) {
  EXPECT_THAT(ReadTokens("\xc3\xa9"), ElementsEqual(Tok(Kind::Error, "\xc3")));
  EXPECT_THAT(ReadTokens("caf\xc3\xa9"),
              ElementsEqual(Tok(Kind::Ident, "caf"), Tok(Kind::Error, "\xc3")));
  EXPECT_THAT(ReadTokens("21\x80"),
              ElementsEqual(Tok(Kind::Number, "21"), Tok(Kind::Error, "\x80")));
  EXPECT_THAT(ReadTokens("  \xff"),
              ElementsEqual(Tok(Kind::Space, "  "), Tok(Kind::Error, "\xff")));
}

// A byte order mark is passed over at the head of the source, and is only
// bytes anywhere else.
TEST(Test, LexerByteOrderMark) {
  EXPECT_THAT(ReadTokens("\xef\xbb\xbf"
                         "foo"),
              ElementsEqual(Tok(Kind::Ident, "foo")));
  EXPECT_THAT(ReadTokens("foo\xef\xbb\xbf"),
              ElementsEqual(Tok(Kind::Ident, "foo"), Tok(Kind::Error, "\xef")));
}

}  // namespace
}  // namespace lucid
