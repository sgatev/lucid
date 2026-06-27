#include "lucid/syntax/comp.h"

#include <cassert>
#include <expected>
#include <list>
#include <optional>
#include <utility>
#include <variant>

#include "gtest/gtest.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"

namespace lucid {
namespace {

using namespace std::string_literals;

class CompCheckTest : public testing::Test, public AstFixture {
 protected:
  std::expected<void, CompError> CheckComp(std::string_view src) {
    std::string code_with_null(src);
    code_with_null.append("\0"s);

    std::list<FuncDefStmt> func_defs;
    for (Parser parser(syn_ctx_, src, Lexer(code_with_null));;) {
      std::expected<std::optional<Def>, ParserError> def_or_error =
          parser.Parse();
      if (!def_or_error.has_value()) {
        return std::unexpected(CompError("failed to parse definition"));
      }

      std::optional<Def> maybe_def = std::move(def_or_error).value();
      if (!maybe_def.has_value()) break;

      Def def = std::move(maybe_def).value();
      assert(std::holds_alternative<FuncDefStmt>(def));

      auto& func_def = std::get<FuncDefStmt>(def);
      func_defs.push_back(func_def);
      syn_ctx_.AddFuncDef(func_defs.back());
    }

    FuncDefStmt* test_func_def = nullptr;
    for (auto& func_def : func_defs) {
      if (syn_ctx_.DerefIdent(func_def.name) == "test") {
        test_func_def = &func_def;
      }
    }

    return lucid::CheckComp(syn_ctx_, *test_func_def);
  }
};

TEST_F(CompCheckTest, EmptyNonCompFunc) {
  std::string_view src = R"(
    fun test(): Int32 {
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, EmptyCompFunc) {
  std::string_view src = R"(
    comp fun test(): Int32 {
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, DoStmtInCompFunc) {
  std::string_view src = R"(
    fun effect(): Int32 {
      return 0
    }

    comp fun test(): Int32 {
      do effect()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, DoStmtOnCompInCompFunc) {
  std::string_view src = R"(
    comp fun pure(): Int32 {
      return 0
    }

    comp fun test(): Int32 {
      do pure()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, NestedDoStmtInConstFunc) {
  std::string_view src = R"(
    fun effect(): Int32 {
      return 0
    }

    comp fun test(b1: Bool, b2: Bool): Int32 {
      loop {
        if b1 {
        } else {
          if b2 {
            do effect()
          }
        }
      }
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, NonCompVarDeclNonCompInit) {
  std::string_view src = R"(
    fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      val x: Int32 = foo()
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, CompVarDeclNonCompInit) {
  std::string_view src = R"(
    fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      comp val x: Int32 = foo()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, CompVarDeclCompFuncCallInit) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      comp val x: Int32 = foo()
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, CompVarDeclCompIntLitInit) {
  std::string_view src = R"(
    fun test(): Int32 {
      comp val x: Int32 = 21
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST_F(CompCheckTest, CompVarDeclCompBoolLitInit) {
  std::string_view src = R"(
    fun test(): Int32 {
      comp val x: Bool = true
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

}  // namespace
}  // namespace lucid
