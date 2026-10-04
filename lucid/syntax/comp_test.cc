#include "lucid/syntax/comp.h"

#include <cassert>
#include <expected>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"

namespace lucid {
namespace {

using namespace std::string_literals;

class CompCheckTest : public Test, public AstFixture {
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

TEST(CompCheckTest, EmptyNonCompFunc) {
  std::string_view src = R"(
    fun test(): Int32 {
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, EmptyCompFunc) {
  std::string_view src = R"(
    comp fun test(): Int32 {
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, DoStmtInCompFunc) {
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

TEST(CompCheckTest, DoStmtOnCompInCompFunc) {
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

TEST(CompCheckTest, NestedDoStmtInConstFunc) {
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

TEST(CompCheckTest, NonCompVarDeclNonCompInit) {
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

TEST(CompCheckTest, CompVarDeclNonCompInit) {
  std::string_view src = R"(
    fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      val x: Int32 = comp foo()
      return 0
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

TEST(CompCheckTest, CompVarDeclCompFuncCallInit) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      val x: Int32 = comp foo()
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, CompVarDeclCompIntLitInit) {
  std::string_view src = R"(
    fun test(): Int32 {
      val x: Int32 = comp 21
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

TEST(CompCheckTest, CompVarDeclCompBoolLitInit) {
  std::string_view src = R"(
    fun test(): Int32 {
      val x: Bool = comp true
      return 0
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A comp value another comp value is worked out from has to be one that
// stays as it was initialized.
TEST(CompCheckTest, CompVarDeclReadByCompVarDecl) {
  std::string_view src = R"(
    fun test(): Int32 {
      val x: Int32 = comp 21
      val y: Int32 = comp (x + 1)
      return y
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A write can reach a variable marked `mut` after it is initialized, so what
// it holds is not known during compilation however it was initialized.
TEST(CompCheckTest, MutCompVarDeclReadByCompVarDecl) {
  std::string_view src = R"(
    fun test(): Int32 {
      mut val x: Int32 = comp 21
      val y: Int32 = comp (x + 1)
      return y
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

// Initializing a variable during compilation says nothing about what may be
// done to it afterwards.
TEST(CompCheckTest, MutCompVarDeclCompIntLitInit) {
  std::string_view src = R"(
    fun test(): Int32 {
      mut val x: Int32 = comp 21
      mut x = x + 1
      return x
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A variable whose initializer is only partly worked out during compilation
// does not hold what compilation worked out, so nothing can be read from it
// there.
TEST(CompCheckTest, PartlyCompVarDeclReadByCompVarDecl) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(n: Int32): Int32 {
      val x: Int32 = comp foo() + n
      val y: Int32 = comp (x + 1)
      return y
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

// A `comp` reaches one element, so what stands beside it is left for when the
// program runs and may read whatever is in scope.
TEST(CompCheckTest, CompExprBesideARuntimeValue) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(n: Int32): Int32 {
      return comp foo() + n
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A `comp` needs no declaration to stand on any more.
TEST(CompCheckTest, CompExprInReturn) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      return comp foo()
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// A comp function called without a `comp` on the call is called while the
// program runs, like any other.
TEST(CompCheckTest, CompFuncCalledWithoutComp) {
  std::string_view src = R"(
    comp fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      return foo()
    }
  )";

  EXPECT_TRUE(CheckComp(src).has_value());
}

// What a `comp` expression calls has to be a comp function wherever the
// expression stands.
TEST(CompCheckTest, CompExprCallingNonCompFunc) {
  std::string_view src = R"(
    fun foo(): Int32 {
      return 0
    }

    fun test(): Int32 {
      return comp foo()
    }
  )";

  EXPECT_FALSE(CheckComp(src).has_value());
}

}  // namespace
}  // namespace lucid
