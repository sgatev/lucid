#include "lucid/token.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(TokenTest, Size) { EXPECT_EQ(sizeof(Token), 24); }

TEST(FindLineTest, Works) {
  EXPECT_EQ(FindLine("", Token(Token::Kind::End, 0, 0)), 1);
  EXPECT_EQ(FindLine("foo", Token(Token::Kind::Ident, 0, 3)), 1);
  EXPECT_EQ(FindLine("foo\nbar", Token(Token::Kind::Ident, 4, 7)), 2);
  EXPECT_EQ(FindLine("foo\nlet = bar", Token(Token::Kind::Ident, 10, 13)), 2);
}

TEST(FindColumnTest, Works) {
  EXPECT_EQ(FindColumn("", Token(Token::Kind::End, 0, 0)), 1);
  EXPECT_EQ(FindColumn("foo", Token(Token::Kind::Ident, 0, 3)), 1);
  EXPECT_EQ(FindColumn("foo\nbar", Token(Token::Kind::Ident, 4, 7)), 1);
  EXPECT_EQ(FindColumn("foo\nlet = bar", Token(Token::Kind::Ident, 10, 13)), 7);
}

}  // namespace
}  // namespace lucid
