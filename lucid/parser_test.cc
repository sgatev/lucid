#include "lucid/parser.h"

#include <functional>
#include <string>
#include <string_view>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/lexer.h"

MATCHER_P(HoldsStmt, match_stmt, "") {
  auto* stmt = std::get_if<lucid::StmtRef>(&arg);
  if (stmt == nullptr) return false;
  return match_stmt(*stmt);
}

MATCHER_P(HoldsError, match_err, "") {
  auto* err = std::get_if<std::string>(&arg);
  if (err == nullptr) return false;
  return match_err == *err;
}

namespace lucid {
namespace {

struct FuncDefStmtPattern {
  std::string_view name;
  std::string_view result_type;
};

class ParserTest : public testing::Test {
 protected:
  std::variant<StmtRef, std::string> Parse(std::string_view code) {
    auto res = Parser(arena_, code, Lexer(code)).ParseFuncDef();
    if (auto* ref = std::get_if<StmtRef>(&res)) return *ref;
    return std::get<ParserError>(res).ToString();
  }

  std::function<bool(StmtRef)> MatchesFuncDefStmt(FuncDefStmtPattern pattern) {
    return [this, pattern](StmtRef ref) {
      const Stmt& stmt = arena_.get(ref);
      auto* func_def_stmt = std::get_if<FuncDefStmt>(&stmt);
      if (func_def_stmt == nullptr) return false;
      if (func_def_stmt->name != pattern.name) return false;
      if (func_def_stmt->result_type != pattern.result_type) return false;
      return true;
    };
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(ParserTest, EmptyFuncDef) {
  std::string_view src = R"(
    let main = () {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(MatchesFuncDefStmt({
                              .name = "main",
                          })));
}

TEST_F(ParserTest, FuncDefMissingLet) {
  std::string_view src = R"(
    = () {
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsError("parse error: expected `let` keyword at line 2, column 5"));
}

TEST_F(ParserTest, FuncDefMissingName) {
  std::string_view src = R"(
    let = () {
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsError("parse error: expected identifier at line 2, column 9"));
}

TEST_F(ParserTest, FuncDefMissingEqual) {
  std::string_view src = R"(
    let main () {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 2, column 14"));
}

TEST_F(ParserTest, FuncDefMissingOpeningParen) {
  std::string_view src = R"(
    let main = ) {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 2, column 16"));
}

TEST_F(ParserTest, FuncDefMissingClosingParen) {
  std::string_view src = R"(
    let main = ( {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 2, column 18"));
}

TEST_F(ParserTest, FuncDefMissingOpenBrace) {
  std::string_view src = R"(
    let main = ()
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 3, column 5"));
}

TEST_F(ParserTest, FuncDefMissingClosingBrace) {
  std::string_view src = R"(
    let main = () {
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 3, column 3"));
}

}  // namespace
}  // namespace lucid
